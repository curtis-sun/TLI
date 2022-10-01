#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <climits>
#include <immintrin.h>
#include <cassert>
#include <random>
#include <memory>
#include <array>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <getopt.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>

#include "function.h"
#include "finedex.h"
#include "util.h"


using namespace std;
using namespace finedex;

#define BUF_SIZE 2048


int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data(); 
    

    FINEdex<key_type, val_type> finedex;
    finedex.train(exist_keys, exist_keys, 32);
    finedex.self_check();
    LOG(2)<<"FINEdex Self check: OK";

    // check find
    val_type dummy_val=0;
    int find_count=0;
    for(int i=0; i<exist_keys.size(); i++) {
        auto res = finedex.find(exist_keys[i], dummy_val);
        if(res==Result::ok) {
            find_count++;
            assert(dummy_val==exist_keys[i]);
        } else{
            LOG(5)<<"Non-find " <<i<<" : "<<exist_keys[i]<<endl;
            finedex.find_debug(exist_keys[i]);
            exit(0);
        }
    }
    LOG(3)<<"find ok: " << find_count;

    // check update
    for(int i=0; i<exist_keys.size(); i++) {
        auto res = finedex.update(exist_keys[i], exist_keys[i]+1);
        assert(res==Result::ok);
    }
    val_type update_val=0;
    int update_count=0;
    for(int i=0; i<exist_keys.size(); i++) {
        auto res = finedex.find(exist_keys[i], update_val);
        if(res==Result::ok) {
            update_count++;
            assert(update_val==exist_keys[i]+1);
        } else{
            LOG(5)<<"Non-update " <<i<<" : "<<exist_keys[i]<<endl;
            finedex.find_debug(exist_keys[i]);
            exit(0);
        }
    }
    LOG(3)<<"update ok: " << update_count;

    // check delete
    int remove_count=0;
    for(int i=0; i<exist_keys.size(); i=i+2) {
        auto res = finedex.remove(exist_keys[i]);
        remove_count++;
    }
    val_type remove_val=0;
    int remove_find_count=0;
    for(int i=0; i<exist_keys.size(); i++) {
        auto res = finedex.find(exist_keys[i], remove_val);
        if(res==Result::ok) {
            remove_find_count++;
            assert(remove_val==exist_keys[i]+1);
        }
    }
    LOG(3)<<"remove ok, [remove, remove_find]: " << remove_count<<", "<<remove_find_count;

    // check insert
    for(int i=0; i<non_exist_keys.size(); i++) {
        finedex.insert(non_exist_keys[i], non_exist_keys[i]);
    }
    val_type insert_dummy_val=0;
    int insert_count=0;
    for(int i=0; i<non_exist_keys.size(); i++) {
        auto res = finedex.find(non_exist_keys[i], insert_dummy_val);
        if(res==Result::ok) {
            insert_count++;
            assert(insert_dummy_val==non_exist_keys[i]);
        } else{
            LOG(5)<<"Non-find " <<i<<" : "<<non_exist_keys[i]<<endl;
            finedex.find_debug(non_exist_keys[i]);
            exit(0);
        }
    }
    LOG(3)<<"find insert ok: " << insert_count;


    // check non update
    for(int i=0; i<non_exist_keys.size(); i++) {
        auto res = finedex.update(non_exist_keys[i], non_exist_keys[i]+1);
        assert(res==Result::ok);
    }
    val_type non_update_val=0;
    int non_update_count=0;
    for(int i=0; i<non_exist_keys.size(); i++) {
        auto res = finedex.find(non_exist_keys[i], non_update_val);
        if(res==Result::ok) {
            non_update_count++;
            assert(non_update_val==non_exist_keys[i]+1);
        } else{
            LOG(5)<<"Non-update " <<i<<" : "<<non_exist_keys[i]<<endl;
            finedex.find_debug(non_exist_keys[i]);
            exit(0);
        }
    }
    LOG(3)<<"non_update ok: " << non_update_count;

    
    return 0;
}