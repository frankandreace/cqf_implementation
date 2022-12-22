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

void print_bits(uint64_t x) {
  std::bitset<MEM_UNIT> bits(x);
  std::cout << bits << std::endl;
}

void print_pair(std::pair<uint64_t,uint64_t> bound){
  std::cout << bound.first << "-" << bound.second << " ";
}

void show(uint64_t value, std::string name){
  std::cout << name << std::endl;
  print_bits(value);
}

void test_masks(){
  std::cout << "ML 00: ";
  print_bits(mask_left(0));
  std::cout << "ML 01: ";
  print_bits(mask_left(1));
  std::cout << "ML 63: ";
  print_bits(mask_left(63));
  std::cout << "ML 64: ";
  print_bits(mask_left(64));
  std::cout << "MR 00: ";
  print_bits(mask_right(0));
  std::cout << "MR 01: ";
  print_bits(mask_right(1));
  std::cout << "MR 63: ";
  print_bits(mask_right(63));
  std::cout << "MR 64: ";
  print_bits(mask_right(64));
}

void test_bitselect(){
  print_bits(mask_left(0));
  std::cout << "BITSELECT 0,0: " << bitselectasm(mask_left(0),0) << std::endl;
  std::cout << "BITSELECT 0,1: " << bitselectasm(mask_left(0),1) << std::endl;

  print_bits(mask_left(64));
  std::cout << "BITSELECT 64,0: " << bitselectasm(mask_left(64),0) << std::endl;
  std::cout << "BITSELECT 64,1: " << bitselectasm(mask_left(64),1) << std::endl;
  std::cout << "BITSELECT 64,2: " << bitselectasm(mask_left(64),2) << std::endl;
  std::cout << "BITSELECT 64,3: " << bitselectasm(mask_left(64),3) << std::endl;
  std::cout << "BITSELECT 64,63: " << bitselectasm(mask_left(64),63) << std::endl;

  print_bits(mask_right(1));
  std::cout << "BITSELECT 1,0: " << bitselectasm(mask_right(1),0) << std::endl;
  std::cout << "BITSELECT 1,1: " << bitselectasm(mask_right(1),1) << std::endl;
  std::cout << "BITSELECT 1,2: " << bitselectasm(mask_right(1),2) << std::endl;
  std::cout << "BITSELECT 1,3: " << bitselectasm(mask_right(1),3) << std::endl;
  std::cout << "BITSELECT 1,63: " << bitselectasm(mask_right(1),63) << std::endl;
  
  print_bits(mask_right(63));
  std::cout << "BITSELECT 63,0: " << bitselectasm(mask_right(63),0) << std::endl;
  std::cout << "BITSELECT 63,1: " << bitselectasm(mask_right(63),1) << std::endl;
  std::cout << "BITSELECT 63,2: " << bitselectasm(mask_right(63),2) << std::endl;
  std::cout << "BITSELECT 63,3: " << bitselectasm(mask_right(63),3) << std::endl;
  std::cout << "BITSELECT 63,63: " << bitselectasm(mask_right(63),63) << std::endl;
}

void test_bitrank(){
  print_bits(mask_left(0));
  std::cout << "BITRANK 0,0: " << bitrankasm(mask_left(0),0) << std::endl;
  std::cout << "BITRANK 0,1: " << bitrankasm(mask_left(0),1) << std::endl;

  print_bits(mask_left(64));
  std::cout << "BITRANK 64,0: " << bitrankasm(mask_left(64),0) << std::endl;
  std::cout << "BITRANK 64,1: " << bitrankasm(mask_left(64),1) << std::endl;
  std::cout << "BITRANK 64,2: " << bitrankasm(mask_left(64),2) << std::endl;
  std::cout << "BITRANK 64,3: " << bitrankasm(mask_left(64),3) << std::endl;
  std::cout << "BITRANK 64,63: " << bitrankasm(mask_left(64),63) << std::endl;

  print_bits(mask_right(1));
  std::cout << "BITRANK 1,0: " << bitrankasm(mask_right(1),0) << std::endl;
  std::cout << "BITRANK 1,1: " << bitrankasm(mask_right(1),1) << std::endl;
  std::cout << "BITRANK 1,2: " << bitrankasm(mask_right(1),2) << std::endl;
  std::cout << "BITRANK 1,3: " << bitrankasm(mask_right(1),3) << std::endl;
  std::cout << "BITRANK 1,63: " << bitrankasm(mask_right(1),63) << std::endl;
  
  print_bits(mask_right(63));
  std::cout << "BITRANK 63,0: " << bitrankasm(mask_right(63),0) << std::endl;
  std::cout << "BITRANK 63,1: " << bitrankasm(mask_right(63),1) << std::endl;
  std::cout << "BITRANK 63,2: " << bitrankasm(mask_right(63),2) << std::endl;
  std::cout << "BITRANK 63,3: " << bitrankasm(mask_right(63),3) << std::endl;
  std::cout << "BITRANK 63,63: " << bitrankasm(mask_right(63),63) << std::endl;
}

void print_vector(std::vector<uint64_t>& vect){
    uint16_t count = 0;
    for (uint64_t i: vect){
      if (count % 3 == 0){
        std::cout << i << std::endl;
      }
      else{
        print_bits(i);
      }
      count++;
    }
  }

void test_rank_select_operations(){
  uint64_t num_of_words = 9;
  std::vector<uint64_t> cqf(num_of_words);

  //setto offsets
  cqf[0] = 0ULL;//1ULL
  cqf[3] = 0ULL;
  cqf[6] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[1] = 0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf[4] = 0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf[7] = 0b0100000000000000000000000010000000000000000000000000000001000000ULL;

  //setto runends
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[2] = 0b0000000000000100000000000001000000000000000000000001000000000000ULL;
  cqf[5] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf[8] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  print_vector(cqf);

  std::cout << "TESTING 3, " << "SELRANK: " << sel_rank_filter(cqf,3) << " EXPECTED: 0" << std::endl;

  //std::cout << "TESTING 9, " << "SELRANK: " << sel_rank_filter(cqf,9) << " EXPECTED: 2" << std::endl;

  //std::cout << "TESTING 10, " << "SELRANK: " << sel_rank_filter(cqf,10) << " EXPECTED: 12" << std::endl;
 
  //std::cout << "TESTING 11, " << "SELRANK: " << sel_rank_filter(cqf,11) << " EXPECTED: 12" << std::endl;

  //std::cout << "TESTING 12, " << "SELRANK: " << sel_rank_filter(cqf,12) << " EXPECTED: 12" << std::endl;

  //std::cout << "TESTING 20, " << "SELRANK: " << sel_rank_filter(cqf,20) << " EXPECTED: 12" << std::endl;
  
  //std::cout << "TESTING 48, " << "SELRANK: " << sel_rank_filter(cqf,48) << " EXPECTED: 36" << std::endl;

  //std::cout << "TESTING 49, " << "SELRANK: " << sel_rank_filter(cqf,49) << " EXPECTED: 50" << std::endl;

  //std::cout << "TESTING 50, " << "SELRANK: " << sel_rank_filter(cqf,50) << " EXPECTED: 50" << std::endl;

  std::cout << "TESTING 51, " << "SELRANK: " << sel_rank_filter(cqf,51) << " EXPECTED: 50" << std::endl;

  std::cout << "TESTING 78, " << "SELRANK: " << sel_rank_filter(cqf,78) << " EXPECTED: 50?" << std::endl;

  std::cout << "TESTING 79, " << "SELRANK: " << sel_rank_filter(cqf,79) << " EXPECTED: 50?" << std::endl;

  std::cout << "TESTING 80, " << "SELRANK: " << sel_rank_filter(cqf,80) << " EXPECTED: 86" << std::endl;

  std::cout << "TESTING 81, " << "SELRANK: " << sel_rank_filter(cqf,81) << " EXPECTED: 86" << std::endl;

  std::cout << "TESTING 124, " << "SELRANK: " << sel_rank_filter(cqf,124) << " EXPECTED: 138" << std::endl;
 
  std::cout << "TESTING 126, " << "SELRANK: " << sel_rank_filter(cqf,126) << " EXPECTED: 138" << std::endl;

  std::cout << "TESTING 138, " << "SELRANK: " << sel_rank_filter(cqf,138) << " EXPECTED: 140" << std::endl;
  
  std::cout << "TESTING 189, " << "SELRANK: " << sel_rank_filter(cqf,189) << " EXPECTED: 168" << std::endl;

  std::cout << "TESTING 190, " << "SELRANK: " << sel_rank_filter(cqf,190) << " EXPECTED: 2" << std::endl;

  std::cout << "TESTING 191, " << "SELRANK: " << sel_rank_filter(cqf,191) << " EXPECTED: 2" << std::endl;


}



void test_first_unused_slot(){
  uint64_t num_of_words = 9;
  std::vector<uint64_t> cqf(num_of_words);

  //setto offsets
  cqf[0] = 1ULL;//1ULL
  cqf[3] = 0ULL;
  cqf[6] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[1] = 0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf[4] = 0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf[7] = 0b0100000000000000000000000010000000000000000000000000000001000000ULL;

  //setto runends
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[2] = 0b0000000000000100000000000001000000000000000000000001000000000100ULL;
  cqf[5] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf[8] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  print_vector(cqf);


  std::cout << "TESTING 5, " << "FUS: " << first_unused_slot(cqf,5) << " EXPECTED: 5" << std::endl;

  std::cout << "TESTING 31, " << "FUS: " << first_unused_slot(cqf,31) << " EXPECTED: 37" << std::endl;

  std::cout << "TESTING 38, " << "FUS: " << first_unused_slot(cqf,38) << " EXPECTED: 38" << std::endl;

  std::cout << "TESTING 90, " << "FUS: " << first_unused_slot(cqf,90) << " EXPECTED: 90" << std::endl;

  std::cout << "TESTING 122, " << "FUS: " << first_unused_slot(cqf,122) << " EXPECTED: 141" << std::endl;

  std::cout << "TESTING 132, " << "FUS: " << first_unused_slot(cqf,132) << " EXPECTED: 141" << std::endl;

  std::cout << "TESTING 191, " << "FUS: " << first_unused_slot(cqf,191) << " EXPECTED: 3" << std::endl;

}



void test_run_boundaries(){
  uint64_t num_of_words = 9;
  std::vector<uint64_t> cqf(num_of_words);

  //setto offsets
  cqf[0] = 1ULL;//1ULL
  cqf[3] = 0ULL;
  cqf[6] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[1] = 0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf[4] = 0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf[7] = 0b0100000000000000000000000010000000000000000000000000000001000000ULL;

  //setto runends
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[2] = 0b0000000000000100000000000001000000000000000000000001000000000100ULL;
  cqf[5] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf[8] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  print_vector(cqf);

  std::cout << "TESTING 5, " << "RB: ";
  print_pair(get_run_boundaries(cqf,5)); 
  std::cout << " EXPECTED: 0-0" << std::endl;

  std::cout << "TESTING 10, " << "RB: ";
  print_pair(get_run_boundaries(cqf,10)); 
  std::cout << " EXPECTED: 10-12" << std::endl;

  std::cout << "TESTING 70, " << "RB: ";
  print_pair(get_run_boundaries(cqf,70)); 
  std::cout << " EXPECTED: 0-0" << std::endl;

  std::cout << "TESTING 80, " << "RB: ";
  print_pair(get_run_boundaries(cqf,80)); 
  std::cout << " EXPECTED: 80-86" << std::endl;

  std::cout << "TESTING 83, " << "RB: ";
  print_pair(get_run_boundaries(cqf,83)); 
  std::cout << " EXPECTED: 0-0" << std::endl;

  std::cout << "TESTING 120, " << "RB: ";
  print_pair(get_run_boundaries(cqf,120)); 
  std::cout << " EXPECTED: 120-131" << std::endl;

  std::cout << "TESTING 124, " << "RB: ";
  print_pair(get_run_boundaries(cqf,124)); 
  std::cout << " EXPECTED: 132-138" << std::endl;

  std::cout << "TESTING 134, " << "RB: ";
  print_pair(get_run_boundaries(cqf,134)); 
  std::cout << " EXPECTED: 139-140" << std::endl;

  std::cout << "TESTING 165, " << "RB: ";
  print_pair(get_run_boundaries(cqf,165)); 
  std::cout << " EXPECTED: 165-168" << std::endl;

  std::cout << "TESTING 190, " << "RB: ";
  print_pair(get_run_boundaries(cqf,190)); 
  std::cout << " EXPECTED: 190-2" << std::endl;

}

void test_empty_case(){
  uint64_t num_of_words = 9;
  std::vector<uint64_t> cqf(num_of_words);

  //set offsets
  cqf[0] = 0ULL;//1ULL
  cqf[3] = 0ULL;
  cqf[6] = 0ULL;
  
  //set occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[1] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf[4] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf[7] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //set runends
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[2] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf[5] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf[8] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  print_vector(cqf);


  std::cout << "TESTING 5, " << "FUS: " << first_unused_slot(cqf,5) << " EXPECTED: 5" << std::endl;

  std::cout << "TESTING 31, " << "FUS: " << first_unused_slot(cqf,31) << " EXPECTED: 37" << std::endl;

  std::cout << "TESTING 38, " << "FUS: " << first_unused_slot(cqf,38) << " EXPECTED: 38" << std::endl;

  std::cout << "TESTING 90, " << "FUS: " << first_unused_slot(cqf,90) << " EXPECTED: 90" << std::endl;

  std::cout << "TESTING 122, " << "FUS: " << first_unused_slot(cqf,122) << " EXPECTED: 141" << std::endl;

  std::cout << "TESTING 132, " << "FUS: " << first_unused_slot(cqf,132) << " EXPECTED: 141" << std::endl;

  std::cout << "TESTING 191, " << "FUS: " << first_unused_slot(cqf,191) << " EXPECTED: 3" << std::endl;

  std::cout << "TESTING 5, " << "RB: ";
  print_pair(get_run_boundaries(cqf,5)); 
  std::cout << " EXPECTED: 0-0" << std::endl;

  std::cout << "TESTING 10, " << "RB: ";
  print_pair(get_run_boundaries(cqf,10)); 
  std::cout << " EXPECTED: 10-12" << std::endl;

  std::cout << "TESTING 70, " << "RB: ";
  print_pair(get_run_boundaries(cqf,70)); 
  std::cout << " EXPECTED: 0-0" << std::endl;

  std::cout << "TESTING 80, " << "RB: ";
  print_pair(get_run_boundaries(cqf,80)); 
  std::cout << " EXPECTED: 80-86" << std::endl;

  std::cout << "TESTING 124, " << "SELRANK: " << sel_rank_filter(cqf,124) << " EXPECTED: 138" << std::endl;
 
  std::cout << "TESTING 126, " << "SELRANK: " << sel_rank_filter(cqf,126) << " EXPECTED: 138" << std::endl;

  std::cout << "TESTING 138, " << "SELRANK: " << sel_rank_filter(cqf,138) << " EXPECTED: 140" << std::endl;
  
  std::cout << "TESTING 189, " << "SELRANK: " << sel_rank_filter(cqf,189) << " EXPECTED: 168" << std::endl;

  std::cout << "TESTING 190, " << "SELRANK: " << sel_rank_filter(cqf,190) << " EXPECTED: 2" << std::endl;

  std::cout << "TESTING 191, " << "SELRANK: " << sel_rank_filter(cqf,191) << " EXPECTED: 2" << std::endl;

  std::cout << "TESTING 31, " << "RB: " << find_boundary_shift_deletion(cqf, 31, first_unused_slot(cqf,31) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 35, " << "RB: " << find_boundary_shift_deletion(cqf, 35, first_unused_slot(cqf,35) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 122, " << "RB: " << find_boundary_shift_deletion(cqf, 122, first_unused_slot(cqf,122) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 134, " << "RB: " << find_boundary_shift_deletion(cqf, 134, first_unused_slot(cqf,134) - 1) << " EXPECTED: 140" << std::endl;

}

void test_boundary_shift_deletion(){
  uint64_t num_of_words = 9;
  std::vector<uint64_t> cqf(num_of_words);

  //setto offsets
  cqf[0] = 1ULL;//1ULL
  cqf[3] = 0ULL;
  cqf[6] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[1] = 0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf[4] = 0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf[7] = 0b0100000000000000000000000000000000000000000000000010000001000000ULL;

  //setto runends
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf[2] = 0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf[5] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf[8] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  print_vector(cqf);
  std::cout << "TESTING RBS" << std::endl;

  std::cout << "TESTING 31, " << "RB: ";
  print_pair(get_run_boundaries(cqf,31)); 
  std::cout << " EXPECTED: 31-48" << std::endl;

  std::cout << "TESTING 49, " << "RB: ";
  print_pair(get_run_boundaries(cqf,49)); 
  std::cout << " EXPECTED: 49-50" << std::endl;

  std::cout << "TESTING 120, " << "RB: ";
  print_pair(get_run_boundaries(cqf,120)); 
  std::cout << " EXPECTED: 120-131" << std::endl;

  std::cout << "TESTING 124, " << "RB: ";
  print_pair(get_run_boundaries(cqf,124)); 
  std::cout << " EXPECTED: 132-138" << std::endl;

  std::cout << "TESTING 134, " << "RB: ";
  print_pair(get_run_boundaries(cqf,134)); 
  std::cout << " EXPECTED: 139-140" << std::endl;

  std::cout << "TESTING 141, " << "RB: ";
  print_pair(get_run_boundaries(cqf,141)); 
  std::cout << " EXPECTED: 141-168" << std::endl;

  std::cout << "TESTING 190, " << "RB: ";
  print_pair(get_run_boundaries(cqf,190)); 
  std::cout << " EXPECTED: 190-2" << std::endl;

  std::cout << std::endl << std::endl;
  std::cout << "TESTING FIRST UNUSED SLOT" << std::endl;

  std::cout << "TESTING 31, " << "FUS: " << first_unused_slot(cqf,31) << " EXPECTED: 51" << std::endl;

  std::cout << "TESTING 38, " << "FUS: " << first_unused_slot(cqf,38) << " EXPECTED: 51" << std::endl;

  std::cout << "TESTING 90, " << "FUS: " << first_unused_slot(cqf,90) << " EXPECTED: 90" << std::endl;

  std::cout << "TESTING 122, " << "FUS: " << first_unused_slot(cqf,122) << " EXPECTED: 169" << std::endl;

  std::cout << "TESTING 138, " << "FUS: " << first_unused_slot(cqf,132) << " EXPECTED: 169" << std::endl;

  std::cout << "TESTING 191, " << "FUS: " << first_unused_slot(cqf,191) << " EXPECTED: 3" << std::endl;


  std::cout << std::endl << std::endl;
  std::cout << "TESTING SHIFT BOUNDARIES" << std::endl;

  std::cout << "TESTING 31, " << "RB: " << find_boundary_shift_deletion(cqf, 31, first_unused_slot(cqf,31) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 35, " << "RB: " << find_boundary_shift_deletion(cqf, 35, first_unused_slot(cqf,35) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 122, " << "RB: " << find_boundary_shift_deletion(cqf, 122, first_unused_slot(cqf,122) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 134, " << "RB: " << find_boundary_shift_deletion(cqf, 134, first_unused_slot(cqf,134) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 135, " << "RB: " << find_boundary_shift_deletion(cqf, 135, first_unused_slot(cqf,135) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 190, " << "RB: " << find_boundary_shift_deletion(cqf, 190, first_unused_slot(cqf,190) - 1) << " EXPECTED: 2" << std::endl;

}