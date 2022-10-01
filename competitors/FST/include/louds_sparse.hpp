#ifndef LOUDSSPARSE_H_
#define LOUDSSPARSE_H_

#include <string>

#include "config.hpp"
#include "fst_builder.hpp"
#include "label_vector.hpp"
#include "rank.hpp"
#include "select.hpp"

namespace fst {

class LoudsSparse {
 public:
  class Iter {
   public:
    Iter()
        : is_valid_(false),
          trie_(0),
          start_level_(0),
          start_node_num_(0),
          key_len_(0),
          is_at_terminator_(false) {};

    Iter(LoudsSparse *trie)
        : is_valid_(false),
          trie_(trie),
          start_node_num_(0),
          key_len_(0),
          is_at_terminator_(false) {
      start_level_ = trie_->getStartLevel();
      for (level_t level = start_level_; level < trie_->getHeight(); level++) {
        key_.push_back(0);
        pos_in_trie_.push_back(0);
        value_pos_.push_back(0);
        value_pos_initialized_.push_back(false);
      }
    }

    void clear();

    bool isValid() const { return is_valid_; };

    int compare(const std::string &key) const;

    std::string getKey() const;

    int getSuffix(word_t *suffix) const;

    std::string getKeyWithSuffix(unsigned *bitlen) const;

    position_t getStartNodeNum() const { return start_node_num_; };

    void setStartNodeNum(position_t node_num) { start_node_num_ = node_num; };

    void setToFirstLabelInRoot();

    void setToLastLabelInRoot();

    void moveToLeftMostKey();

    void moveToRightMostKey();

    uint64_t getValue() const;

    uint64_t getLastIteratorPosition() const;

    void rankValuePosition(size_t pos);

    void operator++(int);

    void operator--(int);

   private:
    void append(const position_t pos);

    void append(const label_t label, const position_t pos);

    void set(const level_t level, const position_t pos);

   private:
    bool is_valid_;  // True means the iter currently points to a valid key
    LoudsSparse *trie_;
    level_t start_level_;
    position_t start_node_num_;  // Passed in by the dense iterator; default = 0
    level_t
        key_len_;  // Start counting from start_level_; does NOT include suffix

    std::vector<label_t> key_;
    std::vector<position_t> pos_in_trie_;

    // stores the index of the current (sparse) value
    std::vector<position_t> value_pos_;
    std::vector<bool> value_pos_initialized_;
    bool is_at_terminator_;

    friend class LoudsSparse;
  };

 public:
  LoudsSparse() {};

  LoudsSparse(const SuRFBuilder *builder);

  ~LoudsSparse() {}

  // point query: trie walk starts at node "in_node_num" instead of root
  // in_node_num is provided by louds-dense's lookupKey function
  bool lookupKey(const std::string &key, const position_t in_node_num,
                 uint64_t &value) const;

  void moveToKeyGreaterThan(const std::string &key, const bool inclusive,
                            LoudsSparse::Iter &iter) const;

  level_t getHeight() const { return height_; };

  level_t getStartLevel() const { return start_level_; };

  uint64_t serializedSize() const;

  uint64_t getMemoryUsage() const;

  void serialize(char *&dst) const {
    memcpy(dst, &height_, sizeof(height_));
    dst += sizeof(height_);
    memcpy(dst, &start_level_, sizeof(start_level_));
    dst += sizeof(start_level_);
    memcpy(dst, &node_count_dense_, sizeof(node_count_dense_));
    dst += sizeof(node_count_dense_);
    memcpy(dst, &child_count_dense_, sizeof(child_count_dense_));
    dst += sizeof(child_count_dense_);
    align(dst);
    labels_->serialize(dst);
    child_indicator_bits_->serialize(dst);
    louds_bits_->serialize(dst);
    align(dst);
  }

  static std::unique_ptr<LoudsSparse> deSerialize(char *&src) {
    std::unique_ptr<LoudsSparse> louds_sparse = std::make_unique<LoudsSparse>();
    memcpy(&(louds_sparse->height_), src, sizeof(louds_sparse->height_));
    src += sizeof(louds_sparse->height_);
    memcpy(&(louds_sparse->start_level_), src,
           sizeof(louds_sparse->start_level_));
    src += sizeof(louds_sparse->start_level_);
    memcpy(&(louds_sparse->node_count_dense_), src,
           sizeof(louds_sparse->node_count_dense_));
    src += sizeof(louds_sparse->node_count_dense_);
    memcpy(&(louds_sparse->child_count_dense_), src,
           sizeof(louds_sparse->child_count_dense_));
    src += sizeof(louds_sparse->child_count_dense_);
    align(src);
    louds_sparse->labels_ = LabelVector::deSerialize(src);
    louds_sparse->child_indicator_bits_ = BitvectorRank::deSerialize(src);
    louds_sparse->louds_bits_ = BitvectorSelect::deSerialize(src);
    align(src);
    return louds_sparse;
  }

 private:
  position_t getChildNodeNum(const position_t pos) const;

  position_t getFirstLabelPos(const position_t node_num) const;

  position_t getLastLabelPos(const position_t node_num) const;

  position_t getSuffixPos(const position_t pos) const;

  position_t nodeSize(const position_t pos) const;

  bool isEndofNode(const position_t pos) const;

  void moveToLeftInNextSubtrie(position_t pos, const position_t node_size,
                               const label_t label,
                               LoudsSparse::Iter &iter) const;

  // return value indicates potential false positive
  bool compareSuffixGreaterThan(const position_t pos, const std::string &key,
                                const level_t level, const bool inclusive,
                                LoudsSparse::Iter &iter) const;

 private:
  static const position_t kRankBasicBlockSize;
  static const position_t kSelectSampleInterval;

  std::vector<uint64_t> positions_sparse;

  level_t height_;       // trie height
  level_t start_level_;  // louds-sparse encoding starts at this level
  // number of nodes in louds-dense encoding
  position_t node_count_dense_;
  // number of children(1's in child indicator bitmap) in louds-dense encoding
  position_t child_count_dense_;

  std::unique_ptr<LabelVector> labels_;
  std::unique_ptr<BitvectorRank> child_indicator_bits_;
  std::unique_ptr<BitvectorSelect> louds_bits_;
};

LoudsSparse::LoudsSparse(const SuRFBuilder *builder) {
  height_ = builder->getLabels().size();
  start_level_ = builder->getSparseStartLevel();

  node_count_dense_ = 0;
  for (level_t level = 0; level < start_level_; level++) {
    node_count_dense_ += builder->getNodeCounts()[level];
  }
  if (start_level_ == 0) {
    child_count_dense_ = 0;
  } else {
    child_count_dense_ =
        node_count_dense_ + builder->getNodeCounts()[start_level_] - 1;
  }
  labels_ = std::make_unique<LabelVector>(builder->getLabels(),
                                          start_level_,
                                          height_);

  std::vector<position_t> num_items_per_level;
  for (level_t level = 0; level < height_; level++) {
    num_items_per_level.push_back(builder->getLabels()[level].size());
  }
  child_indicator_bits_ = std::make_unique<BitvectorRank>(kRankBasicBlockSize,
                                                          builder->getChildIndicatorBits(),
                                                          num_items_per_level,
                                                          start_level_,
                                                          height_);
  louds_bits_ = std::make_unique<BitvectorSelect>(kSelectSampleInterval,
                                                  builder->getLoudsBits(),
                                                  num_items_per_level,
                                                  start_level_,
                                                  height_);

  positions_sparse = builder->getSparseValues();
}

bool LoudsSparse::lookupKey(const std::string &key,
                            const position_t in_node_num,
                            uint64_t &value) const {
  position_t node_num = in_node_num;
  position_t pos = getFirstLabelPos(node_num);
  level_t level = 0;
  for (level = start_level_; level < key.length(); level++) {
    // child_indicator_bits_->prefetch(pos);
    if (!labels_->search((label_t) key[level],
                         pos,
                         nodeSize(pos)))
      return false;

    // if trie branch terminates
    if (!child_indicator_bits_->readBit(pos)) {
      uint64_t value_pos = pos - child_indicator_bits_->rank(pos);
      value = positions_sparse[value_pos];
      return true;
    }
    // move to child
    node_num = getChildNodeNum(pos);
    pos = getFirstLabelPos(node_num);
  }
  return false;
}

void LoudsSparse::moveToKeyGreaterThan(const std::string &key,
                                       const bool inclusive,
                                       LoudsSparse::Iter &iter) const {
  position_t node_num = iter.getStartNodeNum();
  position_t pos = getFirstLabelPos(node_num);

  level_t level;
  for (level = start_level_; level < key.length(); level++) {
    position_t node_size = nodeSize(pos);
    // if no exact match
    if (!labels_->search((label_t) key[level], pos, node_size)) {
      // todo -> do not return false, but just move to the next bigger key?
      moveToLeftInNextSubtrie(pos, node_size, key[level], iter);
      return;
    }

    iter.append(key[level], pos);

    // if trie branch terminates
    if (!child_indicator_bits_->readBit(pos)) {
      if (inclusive) {
        iter.is_valid_ = true;
      } else {
        iter++;
      }
      iter.rankValuePosition(pos);
      return;
    }
    // move to child
    node_num = getChildNodeNum(pos);
    pos = getFirstLabelPos(node_num);
  }

  if ((labels_->read(pos) == kTerminator) &&
      (!child_indicator_bits_->readBit(pos)) && !isEndofNode(pos)) {
    iter.append(kTerminator, pos);
    iter.is_at_terminator_ = true;
    if (!inclusive) iter++;
    iter.is_valid_ = true;
    return;
  }

  if (key.length() <= level) {
    iter.moveToLeftMostKey();
    return;
  }

  iter.is_valid_ = true;
  return;
}

uint64_t LoudsSparse::serializedSize() const {
  uint64_t size =
      sizeof(height_) + sizeof(start_level_) + sizeof(node_count_dense_) +
          sizeof(child_count_dense_) + labels_->serializedSize() +
          child_indicator_bits_->serializedSize()
          + louds_bits_->serializedSize();
  sizeAlign(size);
  return size;
}

uint64_t LoudsSparse::getMemoryUsage() const {
  return (sizeof(*this) + labels_->size() + child_indicator_bits_->size() +
      louds_bits_->size() + positions_sparse.size() * 8);
}

position_t LoudsSparse::getChildNodeNum(const position_t pos) const {
  return (child_indicator_bits_->rank(pos) + child_count_dense_);
}

position_t LoudsSparse::getFirstLabelPos(const position_t node_num) const {
  return louds_bits_->select(node_num + 1 - node_count_dense_);
}

position_t LoudsSparse::getLastLabelPos(const position_t node_num) const {
  position_t next_rank = node_num + 2 - node_count_dense_;
  if (next_rank > louds_bits_->numOnes()) return (louds_bits_->numBits() - 1);
  return (louds_bits_->select(next_rank) - 1);
}

position_t LoudsSparse::getSuffixPos(const position_t pos) const {
  return (pos - child_indicator_bits_->rank(pos));
}

position_t LoudsSparse::nodeSize(const position_t pos) const {
  assert(louds_bits_->readBit(pos));
  return louds_bits_->distanceToNextSetBit(pos);
}

bool LoudsSparse::isEndofNode(const position_t pos) const {
  return ((pos == louds_bits_->numBits() - 1) || louds_bits_->readBit(pos + 1));
}

void LoudsSparse::moveToLeftInNextSubtrie(position_t pos,
                                          const position_t node_size,
                                          const label_t label,
                                          LoudsSparse::Iter &iter) const {
  // if no label is greater than key[level] in this node
  if (!labels_->searchGreaterThan(label, pos, node_size)) {
    iter.append(pos + node_size - 1);
    return iter++;
  } else {
    iter.append(pos);
    return iter.moveToLeftMostKey();
  }
}

bool LoudsSparse::compareSuffixGreaterThan(const position_t pos,
                                           const std::string &key,
                                           const level_t level,
                                           const bool inclusive,
                                           LoudsSparse::Iter &iter) const {
  // position_t suffix_pos = getSuffixPos(pos);
  // int compare = suffixes_->compare(suffix_pos, key, level);
  // if ((compare != kCouldBePositive) && (compare < 0)) {
  //    iter++;
  //    return false;
  //}
  iter.is_valid_ = true;
  return true;
}

//============================================================================

void LoudsSparse::Iter::clear() {
  is_valid_ = false;
  key_len_ = 0;
  is_at_terminator_ = false;
}

int LoudsSparse::Iter::compare(const std::string &key) const {
  if (is_at_terminator_ && (key_len_ - 1) < (key.length() - start_level_))
    return -1;
  std::string iter_key = getKey();
  std::string key_sparse = key.substr(start_level_);
  std::string key_sparse_same_length = key_sparse.substr(0, iter_key.length());
  int compare = iter_key.compare(key_sparse_same_length);
  if (compare != 0) return compare;
  return compare;
}

std::string LoudsSparse::Iter::getKey() const {
  if (!is_valid_) return std::string();
  level_t len = key_len_;
  if (is_at_terminator_) len--;
  return std::string((const char *) key_.data(), (size_t) len);
}

void LoudsSparse::Iter::append(const position_t pos) {
  assert(key_len_ < key_.size());
  key_[key_len_] = trie_->labels_->read(pos);
  pos_in_trie_[key_len_] = pos;
  key_len_++;
}

void LoudsSparse::Iter::append(const label_t label, const position_t pos) {
  assert(key_len_ < key_.size());
  key_[key_len_] = label;
  pos_in_trie_[key_len_] = pos;
  key_len_++;
}

void LoudsSparse::Iter::set(const level_t level, const position_t pos) {
  assert(level < key_.size());
  key_[level] = trie_->labels_->read(pos);
  pos_in_trie_[level] = pos;
}

void LoudsSparse::Iter::setToFirstLabelInRoot() {
  assert(start_level_ == 0);
  pos_in_trie_[0] = 0;
  key_[0] = trie_->labels_->read(0);
}

void LoudsSparse::Iter::setToLastLabelInRoot() {
  assert(start_level_ == 0);
  pos_in_trie_[0] = trie_->getLastLabelPos(0);
  key_[0] = trie_->labels_->read(pos_in_trie_[0]);
}

void LoudsSparse::Iter::moveToLeftMostKey() {
  if (key_len_ == 0) {
    position_t pos = trie_->getFirstLabelPos(start_node_num_);
    label_t label = trie_->labels_->read(pos);
    append(label, pos);
  }

  level_t level = key_len_ - 1;
  position_t pos = pos_in_trie_[level];
  label_t label = trie_->labels_->read(pos);

  if (!trie_->child_indicator_bits_->readBit(pos)) {
    if ((label == kTerminator) && !trie_->isEndofNode(pos))
      is_at_terminator_ = true;
    is_valid_ = true;
    rankValuePosition(pos);
    return;
  }

  while (level < trie_->getHeight()) {
    position_t node_num = trie_->getChildNodeNum(pos);
    pos = trie_->getFirstLabelPos(node_num);
    label = trie_->labels_->read(pos);
    // if trie branch terminates
    if (!trie_->child_indicator_bits_->readBit(pos)) {
      append(label, pos);
      if ((label == kTerminator) && !trie_->isEndofNode(pos))
        is_at_terminator_ = true;
      rankValuePosition(pos);
      is_valid_ = true;
      return;
    }
    append(label, pos);
    level++;
  }
  assert(false);  // shouldn't reach here
}

void LoudsSparse::Iter::moveToRightMostKey() {
  if (key_len_ == 0) {
    // todo can we remove the following statement since it has no effect?
    trie_->getFirstLabelPos(start_node_num_);
    position_t pos = trie_->getLastLabelPos(start_node_num_);
    label_t label = trie_->labels_->read(pos);
    append(label, pos);
  }

  level_t level = key_len_ - 1;
  position_t pos = pos_in_trie_[level];
  label_t label = trie_->labels_->read(pos);

  if (!trie_->child_indicator_bits_->readBit(pos)) {
    if ((label == kTerminator) && !trie_->isEndofNode(pos))
      is_at_terminator_ = true;
    is_valid_ = true;
    return;
  }

  while (level < trie_->getHeight()) {
    position_t node_num = trie_->getChildNodeNum(pos);
    pos = trie_->getLastLabelPos(node_num);
    label = trie_->labels_->read(pos);
    // if trie branch terminates
    if (!trie_->child_indicator_bits_->readBit(pos)) {
      append(label, pos);
      if ((label == kTerminator) && !trie_->isEndofNode(pos))
        is_at_terminator_ = true;
      is_valid_ = true;
      return;
    }
    append(label, pos);
    level++;
  }
  assert(false);  // shouldn't reach here
}

uint64_t LoudsSparse::Iter::getValue() const {
  return trie_->positions_sparse[value_pos_[key_len_ - 1]];
}

uint64_t LoudsSparse::Iter::getLastIteratorPosition() const {
  return pos_in_trie_[key_len_ - 1];
};

void LoudsSparse::Iter::rankValuePosition(size_t pos) {
  if (value_pos_initialized_[key_len_ - 1]) {
    value_pos_[key_len_ - 1]++;
  } else {
    value_pos_initialized_[key_len_ - 1] = true;
    uint64_t value_pos = pos - trie_->child_indicator_bits_->rank(pos);
    value_pos_[key_len_ - 1] = value_pos;
  }
}

void LoudsSparse::Iter::operator++(int) {
  assert(key_len_ > 0);
  is_at_terminator_ = false;
  position_t pos = pos_in_trie_[key_len_ - 1];
  pos++;
  // trie_->louds_bits_ is set for last label in a node -> node terminates here
  while (pos >= trie_->louds_bits_->numBits() ||
      trie_->louds_bits_->readBit(pos)) {
    key_len_--;
    if (key_len_ == 0) {
      is_valid_ = false;
      return;
    }
    pos = pos_in_trie_[key_len_ - 1];
    pos++;
  }

  set(key_len_ - 1, pos);
  return moveToLeftMostKey();
}

void LoudsSparse::Iter::operator--(int) {
  assert(key_len_ > 0);
  is_at_terminator_ = false;
  position_t pos = pos_in_trie_[key_len_ - 1];
  if (pos == 0) {
    is_valid_ = false;
    return;
  }
  while (trie_->louds_bits_->readBit(pos)) {
    key_len_--;
    if (key_len_ == 0) {
      is_valid_ = false;
      return;
    }
    pos = pos_in_trie_[key_len_ - 1];
  }
  pos--;
  set(key_len_ - 1, pos);
  return moveToRightMostKey();
}

const position_t LoudsSparse::kRankBasicBlockSize = 512;
const position_t LoudsSparse::kSelectSampleInterval = 64;

}  // namespace fst

#endif  // LOUDSSPARSE_H_
