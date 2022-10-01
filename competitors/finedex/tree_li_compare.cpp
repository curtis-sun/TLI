#include <iostream>
#include "include/function.h"
#include "learnedindex/learned_index.h"
#include "learnedindex/learned_index_impl.h"
#include "stx/btree_map"
#include "include/levelindex.h"
#include "util.h"

#define QUERIES_PER_TRIAL (5 * 100 * 1000)

typedef LearnedIndex<key_type, val_type> learnedindex_type;
typedef stx::btree_map<key_type, val_type> btree_type;
typedef LevelIndex<key_type> levelindex_type;
typedef TwoLevelIndex<key_type> twolevel_type;

template<class key_t>
std::vector<key_t> random_data(int num){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::lognormal_distribution<double> rand_lognormal(0, 2);

    std::vector<key_t> keys;
    keys.reserve(num);
    for (size_t i = 0; i < num; ++i) {
        key_t a = rand_lognormal(gen)*1000000;
        assert(a>0);
        keys.push_back(a);
    }
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    std::sort(keys.begin(), keys.end());
    return keys;
}


int main(int argc, char **argv){
	std::vector<int32_t> keys = random_data<int32_t>(8);
    std::vector<val_type> vals(keys.size(), 1);
    for(int i=0; i<keys.size(); i++){
        vals[i] = i;
    }
    std::vector<key_type> keys_clone(keys.begin(), keys.end());
    int n = keys_clone.size();
    int num_trials = 1;
    TIMER_DECLARE(0);
    double time_bt, time_si, time_li;
    std::cout<<"[keys_num]: "<<n<<std::endl;

	


    uint32_t seed = std::random_device()();
    std::mt19937 rng;
    std::uniform_int_distribution<> dist(0, n - 1);
    std::vector<key_type> queries(QUERIES_PER_TRIAL);


    // btree search baseline
    printf("Running binary\n");
    rng.seed(seed);
    long check_bt = 0;
    for (int t = 0; t < num_trials; t++) {
      for (key_type &query : queries) {
        query = keys_clone[dist(rng)];
      }

      
      TIMER_BEGIN(0);
      for (const key_type& key : queries) {
        int pos = binary_search_branchless(&keys[0], n, key);
        check_bt += pos;
      }
      TIMER_END_NS(0,time_bt);
    }
    printf("binary checksum = %ld\n", check_bt);
    printf("binary time = %8.1lf ns\n", time_bt/QUERIES_PER_TRIAL);

    printf("Running simd\n");
    rng.seed(seed);
    long check_si = 0;
    for (int t = 0; t < num_trials; t++) {
      for (key_type &query : queries) {
        query = keys_clone[dist(rng)];
      }

      
      TIMER_BEGIN(0);
      for (const key_type& key : queries) {
        int pos = binary_search_branchless(&keys[0], n, key);
        check_si += pos;
      }
      TIMER_END_NS(0,time_si);
    }
    printf("binary checksum = %ld\n", check_si);
    printf("binary time = %8.1lf ns\n", time_si/QUERIES_PER_TRIAL);
	
}

/*
int main(int argc, char **argv)
{
	parse_args(argc, argv);
    load_data();
    std::vector<val_type> vals(exist_keys.size(), 1);
    for(int i=0; i<exist_keys.size(); i++){
    	vals[i] = i;
    }

	learnedindex_type *li = new learnedindex_type();
	li->train(exist_keys, vals, 10000, 32);

	btree_type btree;
	for(int i=0; i<exist_keys.size(); i++){
		btree.insert(exist_keys[i], vals[i]);
	}

	levelindex_type level(exist_keys);


	// Clone vec so we don't bring pages from it into cache when selecting random keys
  	std::vector<key_type> keys_clone(exist_keys.begin(), exist_keys.end());
  	int n = keys_clone.size();
  	int num_trials = 1;
  	TIMER_DECLARE(0);
  	double time_bt, time_btopt, time_li;

  	uint32_t seed = std::random_device()();
  	std::mt19937 rng;
  	std::uniform_int_distribution<> dist(0, n - 1);
  	std::vector<key_type> queries(QUERIES_PER_TRIAL);


  	// btree search baseline
  	printf("Running btree\n");
  	rng.seed(seed);
  	long check_bt = 0;
  	for (int t = 0; t < num_trials; t++) {
  	  for (key_type &query : queries) {
  	    query = keys_clone[dist(rng)];
  	  }

  	  
      TIMER_BEGIN(0);
  	  for (const key_type& key : queries) {
  	    int pos = btree.find(key).data();
  	    check_bt += pos;
  	  }
  	  TIMER_END_NS(0,time_bt);
  	}
  	printf("btree checksum = %ld\n", check_bt);
  	printf("btree time = %8.1lf ns\n", time_bt/QUERIES_PER_TRIAL);

  	// btree_opt search baseline
  	printf("Running btree_opt\n");
  	rng.seed(seed);
  	long check_btopt = 0;
  	for (int t = 0; t < num_trials; t++) {
  	  for (key_type &query : queries) {
  	    query = keys_clone[dist(rng)];
  	  }

  	  
      TIMER_BEGIN(0);
  	  for (const key_type& key : queries) {
  	    int pos = level.find(key);
  	    check_btopt += pos;
  	  }
  	  TIMER_END_NS(0,time_btopt);
  	}
  	printf("btree_opt checksum = %ld\n", check_btopt);
  	printf("btree_opt time = %8.1lf ns\n", time_btopt/QUERIES_PER_TRIAL);


  	// learned indexes search baseline
  	printf("Running learned indexes\n");
  	rng.seed(seed);
  	long check_li = 0;
  	for (int t = 0; t < num_trials; t++) {
  	  for (key_type &query : queries) {
  	    query = keys_clone[dist(rng)];
  	  }

  	  
      TIMER_BEGIN(0);
  	  for (const key_type& key : queries) {
  	    val_type pos = 0;
  	    li->find(key, pos);
  	    check_li += pos;
  	  }
  	  TIMER_END_NS(0,time_li);
  	}
  	printf("learned index checksum = %ld\n", check_li);
  	printf("learned index time = %8.1lf ns\n", time_li/QUERIES_PER_TRIAL);


  	std::cout<<n<<", "<<time_bt/QUERIES_PER_TRIAL<<", "<<time_btopt/QUERIES_PER_TRIAL<<", "<<time_li/QUERIES_PER_TRIAL<<std::endl;




	return 0;
}
*/
