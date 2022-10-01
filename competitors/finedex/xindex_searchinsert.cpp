#include <iostream>
#include "include/function.h"
#include "xindex/xindex.h"
#include "xindex/xindex_impl.h"

struct alignas(CACHELINE_SIZE) ThreadParam;
class Key;

typedef ThreadParam thread_param_t;
typedef Key index_key_t;
typedef xindex::XIndex<index_key_t, uint64_t> xindex_t;


volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::vector<index_key_t> xindex_exist_keys;

struct alignas(CACHELINE_SIZE) ThreadParam {
    xindex_t *xi;
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(xindex_t *xi, size_t sec);
void *run_fg(void *param);
void prepare(xindex_t *&xi);
void *run_read(void *param);

class Key {
  typedef std::array<double, 1> model_key_t;

 public:
  static constexpr size_t model_key_size() { return 1; }
  static Key max() {
    static Key max_key(std::numeric_limits<uint64_t>::max());
    return max_key;
  }
  static Key min() {
    static Key min_key(std::numeric_limits<uint64_t>::min());
    return min_key;
  }

  Key() : key(0) {}
  Key(uint64_t key) : key(key) {}
  Key(const Key &other) { key = other.key; }
  Key &operator=(const Key &other) {
    key = other.key;
    return *this;
  }

  model_key_t to_model_key() const {
    model_key_t model_key;
    model_key[0] = key;
    return model_key;
  }

  friend bool operator<(const Key &l, const Key &r) { return l.key < r.key; }
  friend bool operator>(const Key &l, const Key &r) { return l.key > r.key; }
  friend bool operator>=(const Key &l, const Key &r) { return l.key >= r.key; }
  friend bool operator<=(const Key &l, const Key &r) { return l.key <= r.key; }
  friend bool operator==(const Key &l, const Key &r) { return l.key == r.key; }
  friend bool operator!=(const Key &l, const Key &r) { return l.key != r.key; }

  uint64_t key;
} PACKED;

int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    xindex_t *xi;
    prepare(xi);
    run_benchmark(xi, Config.runtime);
    if(xi!=nullptr) delete xi;
    return 0;
}

void prepare(xindex_t *&xi){
    xindex_exist_keys.reserve(exist_keys.size());
    for (size_t i = 0; i < exist_keys.size(); ++i) {
        xindex_exist_keys.push_back(index_key_t(exist_keys[i]));
    }
    std::vector<uint64_t> vals(xindex_exist_keys.size(), 1);

    COUT_THIS("[Training xindex]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    xi = new xindex_t(xindex_exist_keys, vals, Config.thread_num, 1);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
}

void run_benchmark(xindex_t *xi, size_t sec) {
    pthread_t threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
    // check if parameters are cacheline aligned
    for (size_t i = 0; i < Config.thread_num; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }

    running = false;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].xi = xi;
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
                                (void *)&thread_params[worker_i]);
        if (ret) {
            COUT_N_EXIT("Error:" << ret);
        }
    }

    COUT_THIS("[micro] prepare data ...");
    while (ready_threads < Config.thread_num) sleep(0.5);

    double time_ns;
    double time_s;
    TIMER_DECLARE(1);
    TIMER_BEGIN(1);
    running = true;
    void *status;
    for (size_t i = 0; i < Config.thread_num; i++) {
        int rc = pthread_join(threads[i], &status);
        if (rc) {
            COUT_N_EXIT("Error:unable to join," << rc);
        }
    }
    TIMER_END_NS(1,time_ns);
    TIMER_END_S(1,time_s);

    size_t throughput = 0;
    for (auto &p : thread_params) {
        throughput += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput / time_s);


    running = false;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].xi = xi;
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        int ret = pthread_create(&threads[worker_i], nullptr, run_read,
                                (void *)&thread_params[worker_i]);
        if (ret) {
            COUT_N_EXIT("Error:" << ret);
        }
    }

    COUT_THIS("[micro] prepare data ...");
    while (ready_threads < Config.thread_num) sleep(0.5);

    running = true;
    std::vector<size_t> tput_history1(Config.thread_num, 0);
    size_t current_sec = 0;
    while (current_sec < sec) {
        sleep(1);
        uint64_t tput = 0;
        for (size_t i = 0; i < Config.thread_num; i++) {
            tput += thread_params[i].throughput - tput_history1[i];
            tput_history1[i] = thread_params[i].throughput;
        }
        COUT_THIS("[micro] >>> sec " << current_sec << " throughput: " << tput);
        ++current_sec;
    }

    running = false;
    //void *status;
    for (size_t i = 0; i < Config.thread_num; i++) {
        int rc = pthread_join(threads[i], &status);
        if (rc) {
            COUT_N_EXIT("Error:unable to join," << rc);
        }
    }

    size_t throughput1 = 0;
    for (auto &p : thread_params) {
        throughput1 += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput1 / sec);
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    xindex_t *xi = thread_param.xi;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);

    size_t non_exist_key_n_per_thread = non_exist_keys.size() / Config.thread_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread;
    size_t non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    std::vector<key_type> op_keys(non_exist_keys.begin() + non_exist_key_start,
                                   non_exist_keys.begin() + non_exist_key_end);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    ready_threads++;
    volatile bool res = false;
    uint64_t dummy_value = 1234;

    while (!running)
        ;
  for(size_t i=0; i<op_keys.size(); i++) {
    key_type dummy_key = op_keys[i];
      //std::cout << "========insert: " << dummy_key << std::endl;
      res = xi->put(dummy_key, dummy_key, thread_id);
    thread_param.throughput++;
  }
  pthread_exit(nullptr);
}

void *run_read(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    xindex_t *xi = thread_param.xi;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    // exsiting keys fall within range [delete_i, insert_i)
    ready_threads++;
    volatile bool res = false;
    uint64_t dummy_value = 1234;

    while (!running)
        ;
    while (running) {
        double d = ratio_dis(gen);
        if (d <= 0.2) {  // get
            key_type dummy_key = exist_keys[query_i % exist_keys.size()];
            res = xi->get(dummy_key, dummy_value, thread_id);
            query_i++;
            if (unlikely(query_i == exist_keys.size())) {
                query_i = 0;
            }
        } else {  // insert
            key_type dummy_key = non_exist_keys[insert_i % non_exist_keys.size()];
            res = xi->get(dummy_key, dummy_value, thread_id);
            insert_i++;
            if (unlikely(insert_i == non_exist_keys.size())) {
                insert_i = 0;
            }
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}