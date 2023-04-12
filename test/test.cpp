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
#include <vector>
#include <algorithm>

#include "filter.hpp" 
#include "bcqf_ec.hpp" 
#include "bcqf_oom.hpp" 

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
  Rsqf small_qf(qsize, 64-qsize, false);
  Rsqf usual_qf(4); //524288, qsize=19

  for (size_t j=0 ; j<1000 ; j++) {
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    
    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    cout << "j " << j << endl;
    Rsqf small_qf(qsize, 64-qsize, false);

    for (size_t i=0 ; i<(1ULL<<qsize) ; i++) { //fill to 2^qsize elements (100%-1)
      //cout << "llooooopppp" << endl;
      //cout << i << " ";
      uint64_t val = distributionTMP(generatorTMP);      
      small_qf.insert(val);
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
    Rsqf small_qf(qsize, 64-qsize, false);
    std::unordered_set<uint64_t> verif;

    for (size_t i=0 ; i<(1ULL<<qsize)-1 ; i++) { //fill to 2^qsize elements (100%-1)
      uint64_t val = distributionTMP(generatorTMP);      
      small_qf.insert(val);
      verif.insert(val);
    }   

    if (verif != small_qf.enumerate()) {
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

  int qsize = 19; 
  uint64_t val;
  std::unordered_set<uint64_t> enu;

  for (size_t j=0 ; j<10000 ; j++) {
    
    uint64_t seedTMP = distribution(generator);
    cout << "\nseed " << seedTMP << endl;

    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    cout << "j " << j << endl;
    Rsqf small_qf(qsize, 64-qsize, false);
    std::vector<uint64_t> verif;

    //INSERT
    for (size_t i=0 ; i<(1ULL<<qsize)-1 ; i++) { //fill to 2^qsize elements (100%-1) (1ULL<<qsize)-1
      val = distributionTMP(generatorTMP);      
      small_qf.insert(val);
      verif.push_back(val);
    }   

    //REMOVE ELEMS
    for (size_t i=0 ; i<(1ULL<<qsize)-1 ; i++) { //(1ULL<<qsize)/2
      val = verif.back();
      verif.pop_back();
      small_qf.remove(val);
    } 

    //CHECK ENUMERATE
    enu = small_qf.enumerate();
    if (verif.size() != enu.size()) {
      cout << "error verif != enum" << endl;
      exit(0);
    }
    for ( auto it = verif.begin(); it != verif.end(); ++it ){
      if (enu.find(*it) == enu.end()){
        cout << "error verif != enum" << endl;
        exit(0);
      }
    }
  }  
}




void test_one_cqf(){
  uint64_t seed = 9915022754138594861ULL; 
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;


  int qsize = 7;
  std::map<uint64_t, uint64_t> enu;
  Bcqf_ec cqf(qsize, 64-qsize, 5, true);

  std::map<uint64_t, uint64_t> verif;

  

  for (size_t i=0 ; i<1 ; i++) {
    //std::cout << "\ni " << i << endl;
    uint64_t val = distribution(generator);
    //val &= mask_right(qsize);
    //if (val == 0) { val += (1ULL << 45); }
    //else { val += (val<<qsize); }
    
    cout << "inserting " << val << " => " << val%63 << endl; 
    cqf.insert(val, val%63);
    verif.insert({ val, val%63 });
  }

  

  //REMOVE ELEMS

  
  //CHECK ENUMERATE
  enu = cqf.enumerate();
  cout << "done inserting, verif size " << verif.size() << " enum size " << enu.size() << endl;

  std::cout << cqf.block2string(0, 1) << "\n" << cqf.block2string(1, 1);

  if (verif.size() != enu.size()) {
    cout << "error verif != enum" << endl;
    exit(0);
  }
  for ( auto it = verif.begin(); it != verif.end(); ++it ){
    cout << (*it).first << endl;
    cout << enu[(*it).first] << endl;
    if (enu.find((*it).first) == enu.end()){
      cout << "error verif != enum (diff)" << endl;
      exit(0);
    }
  }

  

   //REMOVE ELEMS
  for (std::map<uint64_t,uint64_t>::iterator it = verif.begin(); it != verif.end(); it++){
    cout << "removing " << (*it).first << " => " << (*it).second << endl; 
    cout << cqf.query((*it).first) << endl;
    cqf.remove((*it).first, (*it).second);
  }
  verif.clear();


  std::cout << cqf.block2string(0, true) << "\n" << cqf.block2string(1, true);

  //CHECK ENUMERATE
  enu = cqf.enumerate();
  if (verif.size() != enu.size()) {
    cout << "error verif != enum (post remove, size) verif:" << verif.size() << "  " << enu.size() << endl;
    exit(0);
  }
  for ( auto it = verif.begin(); it != verif.end(); ++it ){
    if (enu.find((*it).first) == enu.end()){
      cout << "error verif != enum (post remove, diff)" << endl;
      exit(0);
    }
  }

  
}




int main(int argc, char** argv) {
    //test_one_cqf();

    //test_lots_of_full_cqf();

    //test_lots_of_full_cqf_enumerate();

    //test_lots_of_full_cqf_remove();

  Bcqf_oom cqf(7, 64-7, 5, true);

  cqf.insert((22ULL << 7) + 16, 32150);

  std::cout << cqf.block2string(0, true) << "\n" << cqf.block2string(1, true);
  
  cout << cqf.query((22ULL << 7) + 16) << endl;

  cqf.remove((22ULL << 7) + 16);

  std::cout << cqf.block2string(0, true) << "\n" << cqf.block2string(1, true);

  return 0;
}