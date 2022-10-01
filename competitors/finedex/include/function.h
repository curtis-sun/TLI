#include "util.h"

typedef uint64_t key_type;
typedef uint64_t val_type;

std::vector<key_type> exist_keys;
std::vector<key_type> non_exist_keys;

inline void load_data();
inline void parse_args(int, char **);

void uniform_data();
void normal_data();
void lognormal_data();
void timestamp_data();
void documentid_data();
void lognormal_data_with_insert_factor();
void skew_data();
void load_ycsb(const char *path);
void run_ycsb(const char *path);
std::vector<int> read_data(const char *path);
template<typename key_type>
std::vector<key_type> read_timestamp(const char *path);
template<typename key_type>
std::vector<key_type> read_document(const char *path);

typedef struct operation_item {
    key_type key;
    int32_t range;
    uint8_t op;
} operation_item;

// parameters
struct config{
	double read_ratio = 1;
	double insert_ratio = 0;
	double update_ratio = 0;
	double delete_ratio = 0;
	size_t item_num  = 2000000;
	size_t exist_num = 1000000;
	size_t runtime = 10;
	size_t thread_num = 1;
	size_t benchmark = 0;  
	size_t insert_factor = 1;
	double skewness = 0.05;
}Config;

struct {
    size_t operate_num = 10000000;
    operation_item* operate_queue;
}YCSBconfig;

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
        INVARIANT(Config.benchmark >= 0 && Config.benchmark<13);
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

  COUT_THIS("[micro] Read:Insert:Update:Delete:Scan = "
            << Config.read_ratio << ":" << Config.insert_ratio << ":" << Config.update_ratio << ":"
            << Config.delete_ratio );
  double ratio_sum =
      Config.read_ratio + Config.insert_ratio + Config.delete_ratio + Config.update_ratio;
  INVARIANT(ratio_sum > 0.9999 && ratio_sum < 1.0001);  // avoid precision lost
  COUT_VAR(Config.runtime);
  COUT_VAR(Config.thread_num);
  COUT_VAR(Config.benchmark);
}

void load_data(){
    switch (Config.benchmark) {
        case 0:
            normal_data();
			break;
		case 1:
			lognormal_data();
			break;
        case 2:
			timestamp_data();
			break;
		case 3:
			documentid_data();
			break;
        // === used for insert_factor and skew_data
        case 4:
            lognormal_data_with_insert_factor();
            break;
        case 5:
            skew_data();
            break;
        // ===== case 6--11 YCSB ===================
        case 6:
            //load_ycsb("../../ycsb/workloads/run.100K");
            load_ycsb("../FINEdex2/workloads/ycsb_data/workloada/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloada/run.10M");
            COUT_THIS("load workloada");
			break;
		case 7:
			load_ycsb("../FINEdex2/workloads/ycsb_data/workloadb/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloadb/run.10M");
            COUT_THIS("load workloadb");
			break;
        case 8:
			load_ycsb("../FINEdex2/workloads/ycsb_data/workloadc/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloadc/run.10M");
            COUT_THIS("load workloadc");
			break;
		case 9:
			load_ycsb("../FINEdex2/workloads/ycsb_data/workloadd/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloadd/run.10M");
            COUT_THIS("load workloadd");
			break;
        case 10:
			load_ycsb("../FINEdex2/workloads/ycsb_data/workloade/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloade/run.10M");
            COUT_THIS("load workloade");
			break;
        case 11:
            //YCSBconfig.operate_num = 14999193;
            YCSBconfig.operate_num = 15000749;
			load_ycsb("../FINEdex2/workloads/ycsb_data/workloadf/load.10M");
            run_ycsb("../FINEdex2/workloads/ycsb_data/workloadf/run.10M");
            COUT_THIS("load workloadf");
			break;
        case 12:
            uniform_data();
            break;
		default:
			abort();
    }

    // initilize XIndex (sort keys first)
    COUT_THIS("[processing data]");
    std::sort(exist_keys.begin(), exist_keys.end());
    exist_keys.erase(std::unique(exist_keys.begin(), exist_keys.end()), exist_keys.end());
    std::sort(exist_keys.begin(), exist_keys.end());
    for(size_t i=1; i<exist_keys.size(); i++){
        assert(exist_keys[i]>=exist_keys[i-1]);
    }
    //std::vector<val_type> vals(exist_keys.size(), 1);

    COUT_VAR(exist_keys.size());
    COUT_VAR(non_exist_keys.size());
}


void uniform_data(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rand_uniform(0, Config.exist_num*100);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        exist_keys.push_back(i*100);
    }
    if (Config.insert_ratio > 0) {
        non_exist_keys.reserve(Config.item_num);
        for (size_t i = 0; i < Config.item_num; ++i) {
            key_type a = rand_uniform(gen);
            non_exist_keys.push_back(a);
        }
    }
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
void timestamp_data(){
    //assert(Config.read_ratio == 1);
    //std::vector<key_type> read_keys = read_timestamp<key_type>("../FINEdex2/workloads/timestamp.sorted.200M");
    std::vector<key_type> read_keys = read_timestamp<key_type>("osm/osm_cellids_800M_uint64");
    size_t size = read_keys.size();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_int32(
        0, size);

    if(Config.insert_ratio > 0) {
        size_t train_size = size/10;
        exist_keys.reserve(train_size);
        for(int i=0; i<train_size; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }

        non_exist_keys.reserve(size);
        for(int i=0; i<size; i++){
            non_exist_keys.push_back(read_keys[i]);
        }
    } else {
        size_t num = Config.exist_num < read_keys.size()? Config.exist_num:read_keys.size();
        exist_keys.reserve(num);
        for(int i=0; i<num; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }
    }
}
void documentid_data(){
    //assert(Config.read_ratio == 1);
    std::vector<key_type> read_keys = read_timestamp<key_type>("../FINEdex2/workloads/document-id.sorted.10M");
    size_t size = read_keys.size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_int32(
        0, size);

    if(Config.insert_ratio > 0) {
        size_t train_size = size/10;
        exist_keys.reserve(train_size);
        for(int i=0; i<train_size; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }

        non_exist_keys.reserve(size);
        for(int i=0; i<size; i++){
            non_exist_keys.push_back(read_keys[i]);
        }
    } else {
        size_t num = Config.exist_num < read_keys.size()? Config.exist_num:read_keys.size();
        exist_keys.reserve(num);
        for(int i=0; i<num; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }
    }
}
void lognormal_data_with_insert_factor(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::lognormal_distribution<double> rand_lognormal(0, 2);

    COUT_THIS("[gen_data] insert_factor: " <<Config.insert_factor);
    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        if(a<0) {
            i--;
            continue;
        }
        exist_keys.push_back(a);
    }

    int insert_num = Config.item_num*Config.insert_factor;
    non_exist_keys.reserve(insert_num);
    for (size_t i = 0; i < insert_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        if(a<0) {
            i--;
            continue;
        }
        non_exist_keys.push_back(a);
    }
}
void skew_data(){
    key_type maxdata = 987654321;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<key_type> rand_int(0, 987654321);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        exist_keys.push_back(rand_int(gen));
    }
    key_type upbound = (key_type)(Config.skewness*maxdata);
    COUT_THIS("[skewness upbound: ]"<<upbound);
    std::uniform_int_distribution<key_type> rand_skew(0, upbound);
    non_exist_keys.reserve(Config.item_num);
    for (size_t i = 0; i < Config.item_num; ++i) {
        non_exist_keys.push_back(rand_skew(gen));
    }
}



void load_ycsb(const char *path) {
    FILE *ycsb, *ycsb_read;
	char *buf = NULL;
	size_t len = 0;
    size_t item_num = 0;
    char key[16];
    key_type dummy_key = 1234;
    
    
    if((ycsb = fopen(path,"r")) == NULL)
    {
        perror("fail to read");
    }
    COUT_THIS("load data");
    int n = 10000000;
    exist_keys.reserve(n);
	while(getline(&buf,&len,ycsb) != -1){
	  	if(strncmp(buf, "INSERT", 6) == 0){
              memcpy(key, buf+7, 16);
              dummy_key = strtoul(key, NULL, 10);
              exist_keys.push_back(dummy_key);
	  	}
        item_num++;
	}
	fclose(ycsb);
    assert(exist_keys.size()==item_num);
    std::cerr<<"load number : " << item_num <<std::endl;
}

void run_ycsb(const char *path){
    FILE *ycsb, *ycsb_read;
	char *buf = NULL;
	size_t len = 0;
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0, scan_i = 0;
    size_t item_num = 0;
    char key[16];
    key_type dummy_key = 1234;
    

    if((ycsb = fopen(path,"r")) == NULL)
    {
        perror("fail to read");
    }
    YCSBconfig.operate_queue = (operation_item *)malloc(YCSBconfig.operate_num*sizeof(operation_item));

	while(getline(&buf,&len,ycsb) != -1){
		if(strncmp(buf, "READ", 4) == 0){
            memcpy(key, buf+5, 16);
            dummy_key = strtoul(key, NULL, 10);
            YCSBconfig.operate_queue[item_num].key = dummy_key;
            YCSBconfig.operate_queue[item_num].op = 0;
            item_num++;
            query_i++; 
		} else if(strncmp(buf, "INSERT", 6) == 0) {
            memcpy(key, buf+7, 16);
            dummy_key = strtoul(key, NULL, 10);
            YCSBconfig.operate_queue[item_num].key = dummy_key;
            YCSBconfig.operate_queue[item_num].op = 1;
            item_num++;
            insert_i++;
        } else if(strncmp(buf, "UPDATE", 6) == 0) {
            memcpy(key, buf+7, 16);
            dummy_key = strtoul(key, NULL, 10);
            YCSBconfig.operate_queue[item_num].key = dummy_key;
            YCSBconfig.operate_queue[item_num].op = 2;
            item_num++;
            update_i++;
        } else if(strncmp(buf, "REMOVE", 6) == 0) {
            memcpy(key, buf+7, 16);
            dummy_key = strtoul(key, NULL, 10);
            YCSBconfig.operate_queue[item_num].key = dummy_key;
            YCSBconfig.operate_queue[item_num].op = 3;
            item_num++;
            delete_i++;
        } else if(strncmp(buf, "SCAN", 4) == 0) {
            int range_start= 6;
            while(strncmp(&buf[range_start], " ", 1) != 0)
                range_start++;
            memcpy(key, buf+5, range_start-5);
            dummy_key = strtoul(key, NULL, 10);
            range_start++;
            int range = atoi(&buf[range_start]);

            YCSBconfig.operate_queue[item_num].key = dummy_key;
            YCSBconfig.operate_queue[item_num].range = range;
            YCSBconfig.operate_queue[item_num].op = 4;
            item_num++;
            scan_i++;
        } else {
            continue;
        }
	}
	fclose(ycsb);
    std::cerr<<"  read: " << query_i <<std::endl;
    std::cerr<<"insert: " << insert_i <<std::endl;
    std::cerr<<"update: " << update_i <<std::endl;
    std::cerr<<"remove: " << delete_i <<std::endl;
    std::cerr<<"  scan: " << scan_i <<std::endl;
    assert(item_num == YCSBconfig.operate_num);
}

// ===== read data ======
#define BUF_SIZE 2048

std::vector<int> read_data(const char *path) {
    std::vector<int> vec;
    FILE *fin = fopen(path, "rb");
    int buf[BUF_SIZE];
    while (true) {
        size_t num_read = fread(buf, sizeof(int), BUF_SIZE, fin);
        for (int i = 0; i < num_read; i++) {
            vec.push_back(buf[i]);
        }
        if (num_read < BUF_SIZE) break;
    }
    fclose(fin);
    return vec;
}

template<typename key_type>
std::vector<key_type> read_timestamp(const char *path) {
    std::vector<key_type> vec;
    FILE *fin = fopen(path, "rb");
    key_type buf[BUF_SIZE];
    while (true) {
        size_t num_read = fread(buf, sizeof(key_type), BUF_SIZE, fin);
        for (int i = 0; i < num_read; i++) {
            vec.push_back(buf[i]);
        }
        if (num_read < BUF_SIZE) break;
    }
    fclose(fin);
    return vec;
}

template<typename key_type>
std::vector<key_type> read_document(const char *path){
    std::vector<key_type> vec;
    std::ifstream fin(path);
    if(!fin){
        std::cout << "Erro message: " << strerror(errno) << std::endl;
        exit(0);
    }

    key_type a;
    char buffer[50];
    while(!fin.eof()){
      fin.getline(buffer, 50);
      if(strlen(buffer) == 0) break;
      sscanf(buffer, "%ld", &a);
      vec.push_back(a);
    }
    std::cout<< "read data number: " << vec.size() << std::endl;
    fin.close();
    return vec;
}