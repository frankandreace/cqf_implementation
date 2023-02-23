/*
PRINTING, DEBUGGING AND TESTING
*/
#include <iostream>
#include <bitset>
#include <stdint.h> 
#include <limits>

#include "filter.hpp" 
#include "ext_methods.hpp"
#include "test.hpp"


#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL


void print_pair(std::pair<uint64_t,uint64_t> bound){
  std::cout << bound.first << "-" << bound.second << " ";
}

void show(uint64_t value, std::string name){
  std::cout << name << std::endl;
  print_bits(value);
}


using namespace std;

int main(int argc, char** argv) {
    


    Cqf small_cqf(7, 64-7, false);

    //small_cqf.insert((1ULL<<32)+ 99);

    for (int i = 0; i < 12; i++){ small_cqf.insert((1ULL<<15)+ 100); }


    for (int i = 0; i < 5; i++){ small_cqf.insert((1ULL<<17)+ 96); } //48
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);

    small_cqf.insert((1ULL<<17)+ 96);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);


    

    return 0;
}