/*
PRINTING, DEBUGGING AND TESTING
*/
#include <iostream>
#include <bitset>
#include <stdint.h> 
#include <limits>
#include <random>
#include <ctime>
#include <getopt.h>

#include "filter.hpp" 
#include "ext_methods.hpp"
#include "test.hpp"


#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL

using namespace std;


void print_pair(std::pair<uint64_t,uint64_t> bound){
  std::cout << bound.first << "-" << bound.second << " ";
}

void show(uint64_t value, std::string name){
  std::cout << name << std::endl;
  print_bits(value);
}



void test_lots_of_full_cqf(){
  uint64_t seed = time(NULL);
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;


  Cqf small_cqf(7, 64-7, false);
  //Cqf small_cqf(9, 64-9, false);
  Cqf usual_cqf(4);

  for (size_t j=0 ; j<500000 ; j++) {
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    
    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    std::cout << "j " << j << endl;
    Cqf small_cqf(7, 64-7, false);

    for (size_t i=0 ; i<128 ; i++) {
      cout << "llooooopppp" << endl;
      cout << i << " ";
      uint64_t val = distributionTMP(generatorTMP);      
      small_cqf.insert(val);
    }     

  }
}




int main(int argc, char** argv) {
    /* uint64_t seed = 11454997566774698965ULL;
    default_random_engine generator;
    generator.seed(seed);
    uniform_int_distribution<uint64_t> distribution;


    Cqf small_cqf(7, 64-7, true);
    Cqf usual_cqf(4);


    for (size_t i=0 ; i<128 ; i++) {
      std::cout << "i " << i << endl;
      uint64_t val = distribution(generator);
      val &= mask_right(7);
      if (val == 0) { val += (1ULL << 15); }
      else { val += (val<<7); }
    
      
      small_cqf.insert(val);

      std::cout << small_cqf.block2string(0) << "\n\n" << small_cqf.block2string(1);

    } */


    test_lots_of_full_cqf();


  return 0;
}