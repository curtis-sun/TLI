#include "include/level_bin.h"

typedef aidel::LevelBinCon<int, int> levelbin_type;


int main(int argc, char **argv)
{
    levelbin_type lb1;
    bool f = true;
    int i=0;
    while(f){
        f=lb1.con_insert(i, 0);
        i++;
    }
    std::cout<<"seq: "<<i<<std::endl;
    lb1.print(std::cout);


    levelbin_type lb2;
    f = true;
    int n=200;
    i=0;
    while(f){
        f=lb2.con_insert(n, 0);
        n--;
        i++;
    }
    std::cout<<"==========================="<<std::endl;
    std::cout<<"reseq: "<<i<<std::endl;
    lb2.print(std::cout);


    levelbin_type lb3;
    f = true;
    i=0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rand_uniform(0, 1000000);
    while(f){
        f=lb3.con_insert(rand_uniform(gen), 0);
        i++;
    }
    std::cout<<"==========================="<<std::endl;
    std::cout<<"random: "<<i<<std::endl;
    lb3.print(std::cout);

}


