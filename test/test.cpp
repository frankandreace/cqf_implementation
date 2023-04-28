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
#include <chrono>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "filter.hpp" 
#include "bcqf_ec.hpp" 
#include "bcqf_oom.hpp" 

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <iostream>
#include <fstream>

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

  std::map<string, uint64_t> enu;
  std::map<string, uint64_t> verif;

  int qsize = 16;
  int hashsize = 56;
  Bcqf_ec cqf(qsize, hashsize-qsize, 5, false);

  

  //INSERT ELEMS
  for (size_t i=0 ; i<(1ULL << qsize) - 150 ; i++) {
    //std::cout << "\ni " << i << endl;
    uint64_t val = distribution(generator);
    //val &= mask_right(qsize);
    //if (val == 0) { val += (1ULL << 45); }
    //else { val += (val<<qsize); }

    val &= mask_right(hashsize); 

    //print_bits(val);
    std::cout << "inserting " << val << " => " << (val%63) << endl; 
    cqf.insert(val, val%63);
    verif.insert({ hash_to_kmer(val, hashsize/2), val%63 });
  }

  

  //REMOVE ELEMS

  
  //CHECK ENUMERATE
  enu = cqf.enumerate();
  cout << "done inserting, verif size " << verif.size() << " enum size " << enu.size() << endl;

  std::cout << cqf.block2string(0) << "\n" << cqf.block2string(1) << "\n" << cqf.block2string(2) << "\n" << cqf.block2string(3);

  if (verif.size() != enu.size()) {
    cout << "error verif != enum" << endl;
    exit(0);
  }
  for ( auto it = verif.begin(); it != verif.end(); ++it ){
    cout << "checking if " << (*it).first << " => " << verif[(*it).first] << " is in enu " << endl;
    if (enu.find((*it).first) == enu.end()){
      cout << "error verif != enum (diff)" << endl;
      /* cout << "enu atm:" << endl;
      for ( auto ite = enu.begin(); ite != enu.end(); ++ite ){
        cout << (*ite).first << " => " << (*ite).second << endl;
      } */
      exit(0);
    }
  }


   //REMOVE ELEMS
  for (std::map<string,uint64_t>::iterator it = verif.begin(); it != verif.end(); it++){
    cout << "removing " << (*it).first << " => " << (*it).second << endl; 
    cqf.remove((*it).first, (*it).second);
  }
  verif.clear();


  //std::cout << cqf.block2string(0) << "\n" << cqf.block2string(1);

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

  cout << "all good 👍" << endl;
  
}


void test_time_fill_cqf(int q, int n){
  uint64_t seed = time(NULL);
  default_random_engine generator;
  generator.seed(seed);
  uniform_int_distribution<uint64_t> distribution;

  ofstream myfile;
  myfile.open ("/tmp/tmp");
  
  auto ttot = std::chrono::high_resolution_clock::now();

  

  for (int j=0 ; j<n ; j++) {

    uint64_t seedTMP = distribution(generator);
    default_random_engine generatorTMP;
    generatorTMP.seed(seedTMP);
    uniform_int_distribution<uint64_t> distributionTMP;

    Rsqf small_qf(q, 64-q, false);

    for (size_t i=0 ; i<(1ULL<<q)-1 ; i++) { //fill to 2^qsize elements (100%-1)

      auto t1 = std::chrono::high_resolution_clock::now();

      uint64_t val = distributionTMP(generatorTMP);      
      small_qf.insert(val);

      myfile << to_string( std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t1
      ).count()) << "\n";
    }     
  }

  myfile.close();

  cout << to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms\n";
}


void test_8GB_cqf(){
  /* 
  Results : 
  q 31 r 25 remainder_size 30 count_size 5
  4869.097392 ms (build) = 4.87s
  1295757.885060 ms (1B3 inserts) = 1295s = 21.6min
  1462827.337473 ms (1B3 28-mers query) = 24.4min
  36.443450 ms (10k 32-mers query)
  */

  std::string cwd = TEST_DIR;
  cout << "hello genouest\n";

  auto ttot = std::chrono::high_resolution_clock::now();

  Bcqf_ec cqf(31, 56-31, 5, false);

  cout << to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (build)\n";
  ttot = std::chrono::high_resolution_clock::now();

  cqf.insert(cwd + "../examples/data/AHX_ACXIOSF_6_1occs.txt"); //big file

  cout << to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (1B3 inserts)\n";
  ttot = std::chrono::high_resolution_clock::now();

  for (int i = 0; i<2000; i++){
    cqf.query("CAACAGCGGTGTTTTTGTGGGTTGGTGGCTGG", 32);
    cqf.query("ATGTAGCAGAAGGGGTGTAATCATGGCTAAGA", 32);
    cqf.query("AACTGCTGGCAGTGGGGCATTAGCTCGAATCT", 32);
    cqf.query("GCTTCTTCTGGACTGAACGAAGATGAAATCCA", 32);
    cqf.query("TTAATTTATATATTTAATGCATTAATTCTCAA", 32);
  }

  cout << to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (10k 32-mers query)\n";

}


int main(int argc, char** argv) {
    test_one_cqf();

    //test_lots_of_full_cqf();

    //test_lots_of_full_cqf_enumerate();

    ////test_lots_of_full_cqf_remove();

    //test_time_fill_cqf(22, 1);

    //test_8GB_cqf();
}