#include <iostream>
#include "include/function.h"
#include "include/aidel.h"
#include "include/aidel_impl.h"

struct alignas(CACHELINE_SIZE) ThreadParam;


typedef ThreadParam thread_param_t;
typedef aidel::AIDEL<key_type, val_type> aidel_type;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    aidel_type *ai;
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(aidel_type *ai, size_t sec);
void *run_fg(void *param);
void prepare(aidel_type *&ai);
void *run_read(void *param);

int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    aidel_type *ai;
    prepare(ai);
    run_benchmark(ai, Config.runtime);
    if(ai!=nullptr) delete ai;
}

void prepare(aidel_type *&ai){
    COUT_THIS("[Training aidel]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    ai = new aidel_type();
    ai->train(exist_keys, exist_keys, 64);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
    ai->self_check();
    COUT_THIS("check aidel: OK");
}

void run_benchmark(aidel_type *ai, size_t sec) {
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
        thread_params[worker_i].ai = ai;
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
        thread_params[worker_i].ai = ai;
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
    aidel_type *ai = thread_param.ai;

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
    volatile result_t res = result_t::failed;
    val_type dummy_value = 1234;

    while (!running)
        ;
	for(size_t i=0; i<op_keys.size(); i++) {
		key_type dummy_key = op_keys[i];
    	//std::cout << "========insert: " << dummy_key << std::endl;
    	res = ai->insert(dummy_key, dummy_key);
		thread_param.throughput++;
	}
	pthread_exit(nullptr);
}

void *run_read(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    aidel_type *ai = thread_param.ai;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    // exsiting keys fall within range [delete_i, insert_i)
    ready_threads++;
    volatile result_t res = result_t::failed;
    val_type dummy_value = 1234;

    while (!running)
        ;
    /*while (running) {
        double d = ratio_dis(gen);
        if (d <= 0.2) {  // get
            key_type dummy_key = exist_keys[query_i % exist_keys.size()];
            res = ai->find(dummy_key, dummy_value);
            query_i++;
            if (unlikely(query_i == exist_keys.size())) {
                query_i = 0;
            }
        } else {  // insert
            key_type dummy_key = non_exist_keys[insert_i % non_exist_keys.size()];
            res = ai->find(dummy_key, dummy_value);
            insert_i++;
            if (unlikely(insert_i == non_exist_keys.size())) {
                insert_i = 0;
            }
        }
        thread_param.throughput++;
    }*/
    // range query
    std::vector<std::pair<key_type, val_type>> result;
    while (running) {
        double d = ratio_dis(gen);
        if (d <= 0.2) {  // get
            key_type dummy_key = exist_keys[query_i % exist_keys.size()];
            result.clear();
            int n = ai->scan(dummy_key, 10, result);
            query_i++;
            if (unlikely(query_i == exist_keys.size())) {
                query_i = 0;
            }
        } else {  // insert
            key_type dummy_key = non_exist_keys[insert_i % non_exist_keys.size()];
            result.clear();
            int n = ai->scan(dummy_key, 10, result);
            insert_i++;
            if (unlikely(insert_i == non_exist_keys.size())) {
                insert_i = 0;
            }
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}