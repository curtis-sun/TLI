#include <iostream>
#include "include/function.h"
#include "learnedindex/learned_index.h"
#include "learnedindex/learned_index_impl.h"

struct alignas(CACHELINE_SIZE) ThreadParam;

typedef LearnedIndex<key_type, val_type> learnedindex_type;
typedef ThreadParam thread_param_t;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    learnedindex_type *li;
    uint64_t throughput;
    uint32_t thread_id;
};

void prepare(learnedindex_type *&li);
void run_benchmark(learnedindex_type *li);
void *run_fg(void *param);

int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    learnedindex_type *li;
    prepare(li);
    run_benchmark(li);
    if(li!=nullptr) delete li;
}

void prepare(learnedindex_type *&li){
    COUT_THIS("[Training learned index]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    li = new learnedindex_type();
    li->train(exist_keys, exist_keys, 250000, 64);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
}

void run_benchmark(learnedindex_type *li) {
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
        thread_params[worker_i].li = li;
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
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    learnedindex_type *li = thread_param.li;

    size_t key_n_per_thread = YCSBconfig.operate_num / Config.thread_num;
    size_t key_start = thread_id * key_n_per_thread;
    size_t key_end = (thread_id + 1) * key_n_per_thread;
    std::vector<std::pair<key_type, val_type>> result;
    

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    ready_threads++;
    volatile bool res = false;
    val_type dummy_value = 1234;

    while (!running)
        ;
    for(int i=key_start; i<key_end; i++) {
        operation_item opi = YCSBconfig.operate_queue[i];
        if(opi.op == 0){     // read
            res = li->find(opi.key, dummy_value);
        } else if (opi.op == 1) {
            li->insert(opi.key, opi.key);
        } else if (opi.op == 2) {
            res = li->update(opi.key, opi.key);
        } else if (opi.op == 3) {
            res = li->remove(opi.key);
        } else if (opi.op == 4) {
            result.clear();
            int n = li->scan(opi.key, opi.range, result);
        } else {
            COUT_THIS("Wrong operator");
            exit(1);
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}