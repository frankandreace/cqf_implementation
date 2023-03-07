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


  int qsize = 19; 
  Cqf small_cqf(qsize, 64-qsize, false);
  Cqf usual_cqf(4); //524288, qsize=19

  for (size_t j=0 ; j<1000 ; j++) {
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    
    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    cout << "j " << j << endl;
    Cqf small_cqf(qsize, 64-qsize, false);

    for (size_t i=0 ; i<(1ULL<<qsize) ; i++) { //fill to 2^qsize elements (100%-1)
      //cout << "llooooopppp" << endl;
      //cout << i << " ";
      uint64_t val = distributionTMP(generatorTMP);      
      small_cqf.insert(val);
    }     
  }
}


void test_lots_of_full_cqf_enumerate() {
  uint64_t seed = time(NULL);
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;

  int qsize = 24; 

  for (size_t j=0 ; j<1000 ; j++) {
    
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    cout << "j " << j << endl;
    Cqf small_cqf(qsize, 64-qsize, false);
    std::unordered_set<uint64_t> verif;

    for (size_t i=0 ; i<(1ULL<<qsize)-1 ; i++) { //fill to 2^qsize elements (100%-1)
      uint64_t val = distributionTMP(generatorTMP);      
      small_cqf.insert(val);
      verif.insert(val);
    }   

    if (verif != small_cqf.enumerate()) {
      cout << "error verif != enum" << endl;
      exit(0);
    }
  }  
}



void test_lots_of_full_cqf_remove() {
  uint64_t seed = time(NULL);
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;

  int qsize = 7; 
  uint64_t val;

  for (size_t j=0 ; j<1000 ; j++) {
    
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    cout << "j " << j << endl;
    Cqf small_cqf(qsize, 64-qsize, false);
    std::unordered_set<uint64_t> verif;




    for (size_t i=0 ; i<(1ULL<<qsize)-1 ; i++) { //fill to 2^qsize elements (100%-1)
      val = distributionTMP(generatorTMP);      
      small_cqf.insert(val);
      verif.insert(val);
    }   

    if (verif != small_cqf.enumerate()) {
      cout << "error verif != enum" << endl;
      exit(0);
    }

    for (size_t i=0 ; i<(1ULL<<qsize)/2 ; i++) { 
      val = *verif.begin();
      verif.extract(val);
      small_cqf.remove(val);
    } 

    if (verif != small_cqf.enumerate()) {
      cout << "error verif != enum (post remove)" << endl;
      exit(0);
    }
  }  
}






void test_one_cqf(){
  uint64_t seed = 6030154409310195692ULL; 
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;


  int qsize = 11;
  Cqf small_cqf(qsize, 64-qsize, false);

  std::unordered_set<uint64_t> verif;

  for (size_t i=0 ; i<1023 ; i++) {
    std::cout << "\ni " << i << endl;
    uint64_t val = distribution(generator);
    val &= mask_right(qsize);
    if (val == 0) { val += (1ULL << 45); }
    else { val += (val<<qsize); }
    
    small_cqf.insert(val);
    verif.insert(val);
  }

  
  //cout << small_cqf.enumerate().size() << endl;
  //cout << verif.size() << endl;
  cout << (verif == small_cqf.enumerate()) << endl;

  
}




int main(int argc, char** argv) {
    //test_one_cqf();

    //test_lots_of_full_cqf();

    //test_lots_of_full_cqf_enumerate();

    test_lots_of_full_cqf_remove();


  return 0;
}