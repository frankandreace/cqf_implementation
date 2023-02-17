/*
PRINTING, DEBUGGING AND TESTING
*/
#include <iostream>
#include <bitset>
#include <stdint.h> 

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

void test_cqf_size(){
  Cqf cqf(1);
  uint64_t to_insert;
  std::pair<uint64_t, uint64_t> x;
  // uint64_t show;
  uint64_t q_size = cqf.get_quot_size();

  cqf.show_slice(0,5);
  std::cout << "q_size" << q_size << std::endl;
  to_insert = 31ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);


  to_insert = 9ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);


  to_insert = 1ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);


  to_insert = ((10ULL << q_size) | 188ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);


  to_insert = ((11ULL << q_size) | 188ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);


  to_insert = ((1ULL << q_size) | 188ULL) ;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((3ULL << q_size) | 188ULL) ;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);
  cqf.show_slice(0,3);

  to_insert = ((5ULL << q_size) | 188ULL) ;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 188ULL) ;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = 3ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 15ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 157ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = 3ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 15ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 157ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = 3ULL << q_size;
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 15ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  to_insert = ((4ULL << q_size) | 157ULL);
  std::cout << "inserting" << to_insert << std::endl;
  cqf.insert(to_insert);

  cqf.show_slice(0,5);

  std::cout << cqf.query((4ULL << q_size) | 188ULL) << std::endl;

  std::cout << cqf.query((15ULL << q_size) | 188ULL) << std::endl;

  std::cout << cqf.query((4ULL << q_size)) << std::endl;

  std::cout << cqf.query((1ULL << q_size)) << std::endl;
  

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((1ULL << q_size)) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((4ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((9ULL << q_size)) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((5ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((10ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((1ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((4ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((3ULL << q_size) | 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((31ULL << q_size)) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((11ULL << q_size)| 188ULL) << std::endl;

  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((3ULL << q_size)) << std::endl;
  cqf.show_slice(0,5);
}


using namespace std;

int main(int argc, char** argv) {
    //test_cqf_size();

    Cqf small_cqf(7, 64-7, false);

    print_bits(rebuild_number(30, 16, 7));
    
    return 0;
}