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

    for (int i = 0; i < 16; i++){ small_cqf.insert((1ULL<<11)+ 20); } 

    for (int i = 0; i < 28; i++){ small_cqf.insert((1ULL<<13)+ 40); } 

    small_cqf.insert((1ULL<<32)+ 99);
    
    //for (int i = 0; i < 12; i++){ small_cqf.insert((1ULL<<15)+ 100); }

    for (int i = 0; i < 55; i++){ small_cqf.insert((1ULL<<17)+ 96); }

    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);

    small_cqf.insert((1ULL<<17)+ 96); //fus=36 when it shd be 64
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);

    
    pair<uint64_t, uint64_t> bnd;
    bnd = small_cqf.get_run_boundaries(99); cout << bnd.first << " || " << bnd.second << endl;
    //bnd = small_cqf.get_run_boundaries(100); cout << bnd.first << " || " << bnd.second << endl;
    bnd = small_cqf.get_run_boundaries(96); cout << bnd.first << " || " << bnd.second << endl;
    bnd = small_cqf.get_run_boundaries(20); cout << bnd.first << " || " << bnd.second << endl;
    bnd = small_cqf.get_run_boundaries(40); cout << bnd.first << " || " << bnd.second << endl;

    

    return 0;
}