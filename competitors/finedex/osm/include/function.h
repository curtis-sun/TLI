#include "util.h"
#include "logging.hh"

namespace finedex{

typedef uint64_t key_type;
typedef uint64_t val_type;

std::vector<key_type> exist_keys;
std::vector<key_type> non_exist_keys;

inline void load_data();
inline void parse_args(int, char **);

void normal_data();
void lognormal_data();
void osm_data();
template<typename key_type>
std::vector<key_type> read_osm(const char *path);


// parameters
struct config{
	double read_ratio = 0;
	double insert_ratio = 1;
	double update_ratio = 0;
	double delete_ratio = 0;
	size_t item_num  = 10000;
	size_t exist_num = 1000;
	size_t runtime = 10;
	size_t thread_num = 1;
	size_t benchmark = 0;  
	size_t insert_factor = 1;
	double skewness = 0.05;
}Config;



inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"read", required_argument, 0, 'a'},
      {"insert", required_argument, 0, 'b'},
      {"remove", required_argument, 0, 'c'},
      {"update", required_argument, 0, 'd'},
      {"item_num", required_argument, 0, 'e'},
      {"runtime", required_argument, 0, 'f'},
      {"thread_num", required_argument, 0, 'g'},
      {"benchmark", required_argument, 0, 'h'},
      {"insert_factor", required_argument, 0, 'i'},
      {"skewness", required_argument, 0, 'j'},
      {"exist_num", required_argument, 0, 'k'},
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:f:g:h:i:j:k:";
  int option_index = 0;

  while (1) {
    int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        if (long_options[option_index].flag != 0) break;
        abort();
        break;
      case 'a':
        Config.read_ratio = strtod(optarg, NULL);
        INVARIANT(Config.read_ratio >= 0 && Config.read_ratio <= 1);
        break;
      case 'b':
        Config.insert_ratio = strtod(optarg, NULL);
        INVARIANT(Config.insert_ratio >= 0 && Config.insert_ratio <= 1);
        break;
      case 'c':
        Config.delete_ratio = strtod(optarg, NULL);
        INVARIANT(Config.delete_ratio >= 0 && Config.delete_ratio <= 1);
        break;
      case 'd':
        Config.update_ratio = strtod(optarg, NULL);
        INVARIANT(Config.update_ratio >= 0 && Config.update_ratio <= 1);
        break;
      case 'e':
        Config.item_num = strtoul(optarg, NULL, 10);
        INVARIANT(Config.item_num > 0);
        break;
      case 'f':
        Config.runtime = strtoul(optarg, NULL, 10);
        INVARIANT(Config.runtime > 0);
        break;
      case 'g':
        Config.thread_num = strtoul(optarg, NULL, 10);
        INVARIANT(Config.thread_num > 0);
        break;
      case 'h':
        Config.benchmark = strtoul(optarg, NULL, 10);
        INVARIANT(Config.benchmark >= 0 && Config.benchmark<3);
        break;
      case 'i':
        Config.insert_factor = strtoul(optarg, NULL, 10);
        INVARIANT(Config.insert_factor >= 0);
        break;
      case 'j':
        Config.skewness = strtod(optarg, NULL);
        INVARIANT(Config.skewness > 0 && Config.skewness <= 1);
        break;
      case 'k':
        Config.exist_num = strtoul(optarg, NULL, 10);
        INVARIANT(Config.exist_num > 0);
        break;
      default:
        abort();
    }
  }

  LOG(4)<<"[micro] Read:Insert:Update:Delete:Scan = "
            << Config.read_ratio << ":" << Config.insert_ratio << ":" << Config.update_ratio << ":"
            << Config.delete_ratio;
  double ratio_sum =
      Config.read_ratio + Config.insert_ratio + Config.delete_ratio + Config.update_ratio;
  INVARIANT(ratio_sum > 0.9999 && ratio_sum < 1.0001);  // avoid precision lost
  LOG(4)<<"Config.runtime: "<<Config.runtime;
  LOG(4)<<"Config.thread_num: "<<Config.thread_num;
  LOG(4)<<"Config.benchmark: "<<Config.benchmark;
}


void load_data(){
  LOG(2)<<"read data ...";
  switch (Config.benchmark) {
    case 0:
      normal_data();
			break;
		case 1:
			lognormal_data();
			break;
    case 2:
      osm_data();
      break;
		default:
			abort();
  }

    // initilize XIndex (sort keys first)
  LOG(2)<<"sort data ...";
  std::sort(exist_keys.begin(), exist_keys.end());
  exist_keys.erase(std::unique(exist_keys.begin(), exist_keys.end()), exist_keys.end());
  for(size_t i=1; i<exist_keys.size(); i++){
    assert(exist_keys[i]>exist_keys[i-1]);
  }

  LOG(3) << "exist_keys.size(): "<<exist_keys.size();
  LOG(3) << "non_exist_keys.size(): " << non_exist_keys.size();
}


void normal_data(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> rand_normal(4, 2);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        key_type a = rand_normal(gen)*1000000000000;
        if(a<0) {
            i--;
            continue;
        }
        exist_keys.push_back(a);
    }
    if (Config.insert_ratio > 0) {
        non_exist_keys.reserve(Config.item_num);
        for (size_t i = 0; i < Config.item_num; ++i) {
            key_type a = rand_normal(gen)*1000000000000;
            if(a<0) {
                i--;
                continue;
            }
            non_exist_keys.push_back(a);
        }
    }
}
void lognormal_data(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::lognormal_distribution<double> rand_lognormal(0, 2);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        assert(a>0);
        exist_keys.push_back(a);
    }
    if (Config.insert_ratio > 0) {
        non_exist_keys.reserve(Config.item_num);
        for (size_t i = 0; i < Config.item_num; ++i) {
            key_type a = rand_lognormal(gen)*1000000000000;
            assert(a>0);
            non_exist_keys.push_back(a);
        }
    }
}

void osm_data() {
    std::vector<key_type> read_keys = read_osm<key_type>("/home/user/lpf/FINEdex/osm/osm_cellids_800M_uint64");
    size_t size = read_keys.size();
    LOG(2)<< "read osm: "<<size;

    size_t num = Config.exist_num < read_keys.size()? Config.exist_num:read_keys.size();
    exist_keys.reserve(num);
    for(int i=0; i<num; i++){
      //exist_keys.push_back(read_keys[size-num+i+1]);
      exist_keys.push_back(read_keys[i]);
    }
}

#define BUF_SIZE 2048

template<typename key_type>
std::vector<key_type> read_osm(const char *path) 
{
  std::vector<key_type> vec;
  FILE *fin = fopen(path, "rb");
  key_type buf[BUF_SIZE];

  fread(buf, sizeof(uint64_t), 1, fin);
  uint64_t osm_size = buf[0];

  while (true) {
    size_t num_read = fread(buf, sizeof(key_type), BUF_SIZE, fin);
    for (int i = 0; i < num_read; i++) {
      vec.push_back(buf[i]);
    }
    if (num_read < BUF_SIZE) break;
  }
  fclose(fin);
  assert(osm_size==vec.size());
  return vec;
}

} //namespace finedex