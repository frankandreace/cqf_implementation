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


void test_cqf_metadata(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 1ULL;//1ULL
  cqf.cqf[8] = 0ULL;
  cqf.cqf[16] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf.cqf[9] =  0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf.cqf[17] = 0b0100000000000000000000000000000000000000000000000010000001000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000101000000000000000000101000000000000001000000000100ULL;
  cqf.cqf[10] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "TESTING RBS" << std::endl;

  std::cout << "TESTING 31, " << "RB: ";
  print_pair(cqf.get_run_boundaries(31)); 
  std::cout << " EXPECTED: 31-48" << std::endl;

  std::cout << "TESTING 49, " << "RB: ";
  print_pair(cqf.get_run_boundaries(49)); 
  std::cout << " EXPECTED: 49-50" << std::endl;

  std::cout << "TESTING 120, " << "RB: ";
  print_pair(cqf.get_run_boundaries(120)); 
  std::cout << " EXPECTED: 120-131" << std::endl;

  std::cout << "TESTING 124, " << "RB: ";
  print_pair(cqf.get_run_boundaries(124)); 
  std::cout << " EXPECTED: 132-138" << std::endl;

  std::cout << "TESTING 134, " << "RB: ";
  print_pair(cqf.get_run_boundaries(134)); 
  std::cout << " EXPECTED: 139-140" << std::endl;

  std::cout << "TESTING 141, " << "RB: ";
  print_pair(cqf.get_run_boundaries(141)); 
  std::cout << " EXPECTED: 141-168" << std::endl;

  std::cout << "TESTING 190, " << "RB: ";
  print_pair(cqf.get_run_boundaries(190)); 
  std::cout << " EXPECTED: 190-2" << std::endl;

  std::cout << std::endl << std::endl;
  std::cout << "TESTING FIRST UNUSED SLOT" << std::endl;

  std::cout << "TESTING 31, " << "FUS: " << cqf.first_unused_slot(31) << " EXPECTED: 51" << std::endl;

  std::cout << "TESTING 38, " << "FUS: " << cqf.first_unused_slot(38) << " EXPECTED: 51" << std::endl;

  std::cout << "TESTING 90, " << "FUS: " << cqf.first_unused_slot(90) << " EXPECTED: 90" << std::endl;

  std::cout << "TESTING 122, " << "FUS: " << cqf.first_unused_slot(122) << " EXPECTED: 169" << std::endl;

  std::cout << "TESTING 138, " << "FUS: " << cqf.first_unused_slot(132) << " EXPECTED: 169" << std::endl;

  std::cout << "TESTING 191, " << "FUS: " << cqf.first_unused_slot(191) << " EXPECTED: 3" << std::endl;


  std::cout << std::endl << std::endl;
  std::cout << "TESTING SHIFT BOUNDARIES" << std::endl;

  std::cout << "TESTING 31, " << "RB: " << cqf.find_boundary_shift_deletion(31, cqf.first_unused_slot(31) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 35, " << "RB: " << cqf.find_boundary_shift_deletion(35, cqf.first_unused_slot(35) - 1) << " EXPECTED: 48" << std::endl;

  std::cout << "TESTING 122, " << "RB: " << cqf.find_boundary_shift_deletion(122, cqf.first_unused_slot(122) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 134, " << "RB: " << cqf.find_boundary_shift_deletion(134, cqf.first_unused_slot(134) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 135, " << "RB: " << cqf.find_boundary_shift_deletion(135, cqf.first_unused_slot(135) - 1) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 190, " << "RB: " << cqf.find_boundary_shift_deletion(190, cqf.first_unused_slot(190) - 1) << " EXPECTED: 2" << std::endl;

}

void test_fus(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 0ULL;//1ULL
  cqf.cqf[8] = 0ULL;
  cqf.cqf[16] = 0ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "fist unused slot 0: " << cqf.first_unused_slot(0) << std::endl;
  std::cout << "fist unused slot 1: " << cqf.first_unused_slot(1) << std::endl;
  std::cout << "fist unused slot 10: " << cqf.first_unused_slot(10) << std::endl;
  std::cout << "fist unused slot 100: " << cqf.first_unused_slot(100) << std::endl;
  std::cout << "fist unused slot 175: " << cqf.first_unused_slot(175) << std::endl;

    //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "fist unused slot 0: " << cqf.first_unused_slot(0) << std::endl;
  std::cout << "fist unused slot 1: " << cqf.first_unused_slot(1) << std::endl;
  std::cout << "fist unused slot 10: " << cqf.first_unused_slot(10) << std::endl;
  std::cout << "fist unused slot 100: " << cqf.first_unused_slot(100) << std::endl;
  std::cout << "fist unused slot 175: " << cqf.first_unused_slot(175) << std::endl;

      //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000010ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "fist unused slot 0: " << cqf.first_unused_slot(0) << std::endl;
  std::cout << "fist unused slot 1: " << cqf.first_unused_slot(1) << std::endl;
  std::cout << "fist unused slot 10: " << cqf.first_unused_slot(10) << std::endl;
  std::cout << "fist unused slot 100: " << cqf.first_unused_slot(100) << std::endl;
  std::cout << "fist unused slot 175: " << cqf.first_unused_slot(175) << std::endl;

        //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000001000ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "fist unused slot 0: " << cqf.first_unused_slot(0) << std::endl;
  std::cout << "fist unused slot 1: " << cqf.first_unused_slot(1) << std::endl;
  std::cout << "fist unused slot 10: " << cqf.first_unused_slot(10) << std::endl;
  std::cout << "fist unused slot 100: " << cqf.first_unused_slot(100) << std::endl;
  std::cout << "fist unused slot 175: " << cqf.first_unused_slot(175) << std::endl;

        //setto occupieds
  //            0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000001000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000100ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000001000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "fist unused slot 0: " << cqf.first_unused_slot(0) << std::endl;
  std::cout << "fist unused slot 1: " << cqf.first_unused_slot(1) << std::endl;
  std::cout << "fist unused slot 10: " << cqf.first_unused_slot(10) << std::endl;
  std::cout << "fist unused slot 100: " << cqf.first_unused_slot(100) << std::endl;
  std::cout << "fist unused slot 174: " << cqf.first_unused_slot(174) << std::endl;
  std::cout << "fist unused slot 175: " << cqf.first_unused_slot(175) << std::endl;
  std::cout << "fist unused slot 176: " << cqf.first_unused_slot(176) << std::endl;
}

void test_metadata_shift(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 1ULL;//1ULL
  cqf.cqf[8] = 4ULL;
  cqf.cqf[16] = 0ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[9] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[17] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000011110000011110000011110000011110000011110000011110000011111ULL;
  cqf.cqf[10] = 0b1000000000001000000000000000010000000000000000000000000000000010ULL;
  cqf.cqf[18] = 0b1101010101010101010101010101010111100011101010101010101010101010ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  cqf.shift_bits_left_metadata(100,0,101,86);
  std::cout << std::endl;
  cqf.show();
  std::cout << std::endl << std::endl;
  cqf.shift_bits_right_metadata(100,0,101,86);
  cqf.show();
}

void test_metadata_shift1(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 0ULL;//1ULL
  cqf.cqf[8] = 0ULL;
  cqf.cqf[16] = 0ULL;
  
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000001000ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;


  //print_vector(cqf.cqf);
  cqf.show();
  cqf.shift_bits_left_metadata(188,1,188,188);
  std::cout << std::endl;
  cqf.show();
  std::cout << std::endl << std::endl;
  cqf.shift_bits_right_metadata(188,0,188,188);
  cqf.show();
}

void test_set_get_reminder(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 1ULL;//1ULL
  cqf.cqf[8] = 4ULL;
  cqf.cqf[16] = 0ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[9] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[17] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000011110000011110000011110000011110000011110000011110000011111ULL;
  cqf.cqf[10] = 0b1000000000001000000000000000010000000000000000000000000000000010ULL;
  cqf.cqf[18] = 0b1101010101010101010101010101010111100011101010101010101010101010ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "setting" << std::endl;
  std::cout << "0" << std::endl;
  cqf.set_remainder(0,11);
  std::cout << "end 0" << std::endl;
  std::cout << "1" << std::endl;
  cqf.set_remainder(1,12);
  std::cout << "12" << std::endl;
  cqf.set_remainder(12,31);
  std::cout << "63" << std::endl;
  cqf.set_remainder(63,1);
  std::cout << "64" << std::endl;
  cqf.set_remainder(64,2);
  std::cout << "65" << std::endl;
  cqf.set_remainder(65,3);
  std::cout << "128" << std::endl;
  cqf.set_remainder(128,10);
  std::cout << "190" << std::endl;
  cqf.set_remainder(190,13);
  std::cout << "191" << std::endl;
  cqf.set_remainder(191,14);
  std::cout << "189" << std::endl;
  cqf.set_remainder(189,5);
  std::cout << "188" << std::endl;
  cqf.set_remainder(188,7);

  cqf.show();

  std::cout << "getting" << std::endl;
  std::cout << cqf.get_remainder(0) << std::endl;
  std::cout << cqf.get_remainder(1) << std::endl;
  std::cout << cqf.get_remainder(12) << std::endl;
  std::cout << cqf.get_remainder(63) << std::endl;
  std::cout << cqf.get_remainder(64) << std::endl;
  std::cout << cqf.get_remainder(65) << std::endl;
  std::cout << cqf.get_remainder(128) << std::endl;
  std::cout << cqf.get_remainder(190) << std::endl;
  std::cout << cqf.get_remainder(191) << std::endl;
  std::cout << cqf.get_remainder(192) << std::endl;
  std::cout << cqf.get_remainder(188) << std::endl;
  std::cout << cqf.get_remainder(189) << std::endl;

  std::cout << cqf.get_remainder_func(0) << std::endl;
  std::cout << cqf.get_remainder_func(1) << std::endl;
  std::cout << cqf.get_remainder_func(12) << std::endl;
  std::cout << cqf.get_remainder_func(63) << std::endl;
  std::cout << cqf.get_remainder_func(64) << std::endl;
  std::cout << cqf.get_remainder_func(65) << std::endl;
  std::cout << cqf.get_remainder_func(128) << std::endl;
  std::cout << cqf.get_remainder_func(190) << std::endl;
  std::cout << cqf.get_remainder_func(191) << std::endl;
  std::cout << cqf.get_remainder_func(192) << std::endl;
  std::cout << cqf.get_remainder_func(188) << std::endl;
  std::cout << cqf.get_remainder_func(189) << std::endl;
}

void test_shift_left_add_reminder(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  //setto offsets
  cqf.cqf[0] = 1ULL;//1ULL
  cqf.cqf[8] = 4ULL;
  cqf.cqf[16] = 0ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[9] =  0b0000000000001000000000000000010000000000000100000000000000000010ULL;
  cqf.cqf[17] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000011110000011110000011110000011110000011110000011110000011111ULL;
  cqf.cqf[10] = 0b1000000000001000000000000000010000000000000000000000000000000010ULL;
  cqf.cqf[18] = 0b1101010101010101010101010101010111100011101010101010101010101010ULL;

  //print_vector(cqf.cqf);
  cqf.show();
  std::cout << "setting" << std::endl;
  std::cout << "0" << std::endl;
  cqf.set_remainder(0,11);
  std::cout << "end 0" << std::endl;
  std::cout << "1" << std::endl;
  cqf.set_remainder(1,12);
  cqf.set_remainder(2,1);
  cqf.set_remainder(3,2);
  cqf.set_remainder(4,3);

  cqf.set_remainder(10,17);
  cqf.set_remainder(11,18);
  cqf.set_remainder(13,19);
  cqf.set_remainder(14,20);
  cqf.set_remainder(15,21);
  cqf.set_remainder(16,22);
  std::cout << "12" << std::endl;
  cqf.set_remainder(12,31);
  std::cout << "63" << std::endl;
  cqf.set_remainder(63,1);
  std::cout << "64" << std::endl;
  cqf.set_remainder(64,2);
  std::cout << "65" << std::endl;
  cqf.set_remainder(65,3);
  std::cout << "128" << std::endl;
  cqf.set_remainder(128,10);
  std::cout << "190" << std::endl;
  cqf.set_remainder(190,13);
  std::cout << "191" << std::endl;
  cqf.set_remainder(191,14);
  std::cout << "189" << std::endl;
  cqf.set_remainder(189,5);
  std::cout << "188" << std::endl;
  cqf.set_remainder(188,7);

  //cqf.show();

  //cqf.shift_left_and_set_circ(10,17,23);

  cqf.show();

  cqf.shift_left_and_set_circ(12,17,21);

  cqf.show();

  cqf.shift_right_and_rem_circ(12,17);

  cqf.show();
}

void test_boundaries(){

  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);
  cqf.show();

  //            0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000000000000001000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000100ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000001000000000000000000000000000000000000000000000000ULL;

  //print_vector(cqf.cqf);
  cqf.show();

  std::cout << "TESTING 0, " << "RB: ";
  print_pair(cqf.get_run_boundaries(0));
  std::cout << "TESTING 1, " << "RB: ";
  print_pair(cqf.get_run_boundaries(1));
  std::cout << std::endl;

  std::cout << "TESTING 2, " << "RB: ";
  print_pair(cqf.get_run_boundaries(2));
  std::cout << "TESTING 190, " << "RB: ";
  print_pair(cqf.get_run_boundaries(190));
  std::cout << std::endl;
  std::cout << "TESTING 191, " << "RB: ";
  print_pair(cqf.get_run_boundaries(191));
  std::cout << std::endl;
  std::cout << "TESTING 174, " << "RB: ";
  print_pair(cqf.get_run_boundaries(174));
  std::cout << std::endl;
  std::cout << "TESTING 175, " << "RB: ";
  print_pair(cqf.get_run_boundaries(175));
  std::cout << std::endl;  
  std::cout << "TESTING 176, " << "RB: ";
  print_pair(cqf.get_run_boundaries(176));
  std::cout << std::endl;
}

void test_sel_rank_filter(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  std::cout << "SL 0, empty vect: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, empty vect: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, empty vect: " << cqf.sel_rank_filter(63) << std::endl;

  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  std::cout << "SL 0, 1pos0: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, 1pos0: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, 1pos0: " << cqf.sel_rank_filter(63) << std::endl;

  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000010ULL;
  std::cout << "SL 0, 1pos0: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, 1pos0: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, 1pos0: " << cqf.sel_rank_filter(63) << std::endl;
  
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000010ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000010000000ULL;
  std::cout << "SL 0, 1pos1: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, 1pos1: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, 1pos1: " << cqf.sel_rank_filter(63) << std::endl;

  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000011ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000010001000ULL;
  std::cout << "SL 0, 1pos01: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, 1pos01: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, 1pos01: " << cqf.sel_rank_filter(63) << std::endl;

  cqf.cqf[1] =  0b1000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[8] =  1ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[10] =  0b0000000000000000000000000000000000000000000000000000000000001000ULL;
  std::cout << "SL 0, 1end: " << cqf.sel_rank_filter(0) << std::endl;
  std::cout << "SL 1, 1end: " << cqf.sel_rank_filter(1) << std::endl;
  std::cout << "SL 63, 1end: " << cqf.sel_rank_filter(63) << std::endl;
}

void test_insert(){

  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

  uint64_t to_insert;
  std::pair<uint64_t, uint64_t> x;
  uint64_t show;

  cqf.show();
  //show = cqf.first_unused_slot(0);
  //std::cout << "fist unused slot of 0: " << show << std::endl;

  to_insert = 31ULL << 59;
  cqf.insert(to_insert);
  cqf.show();
  //x = cqf.get_run_boundaries(0);
  //std::cout << "TESTING 0, " << "RB: ";
  //print_pair(x);
  //std::cout << std::endl;

  to_insert = 9ULL << 59;
  cqf.insert(to_insert);
  cqf.show();
  //x = cqf.get_run_boundaries(0);
  //std::cout << "TESTING 0, " << "RB: ";
  //print_pair(x);
  //std::cout << std::endl;

  to_insert = 1ULL << 59;
  cqf.insert(to_insert);
  cqf.show();

  to_insert = ((10ULL << 59) | 188ULL);
  //show = cqf.first_unused_slot(188);
  //std::cout << "---FUS" << show << std::endl;
  cqf.insert(to_insert);
  //show = cqf.first_unused_slot(188);
  //std::cout << "---FUS" << show << std::endl;
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;


  to_insert = ((11ULL << 59) | 188ULL);
  //std::cout << "---FUS" << cqf.first_unused_slot(188) << std::endl;
  cqf.insert(to_insert);
  //std::cout << "---FUS" << cqf.first_unused_slot(188) << std::endl;
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;


  to_insert = ((1ULL << 59) | 188ULL) ;
  cqf.insert(to_insert);
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;


  to_insert = ((3ULL << 59) | 188ULL) ;
  cqf.insert(to_insert);
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;


  to_insert = ((5ULL << 59) | 188ULL) ;
  cqf.insert(to_insert);
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;


  to_insert = ((4ULL << 59) | 188ULL) ;
  cqf.insert(to_insert);
  cqf.show();
  x = cqf.get_run_boundaries(188);
  std::cout << "TESTING 0, " << "RB: ";
  print_pair(x);
  std::cout << std::endl;

  to_insert = 3ULL << 59;
  cqf.insert(to_insert);
  cqf.show();

  std::cout << cqf.query((4ULL << 59) | 188ULL) << std::endl;

  std::cout << cqf.query((15ULL << 59) | 188ULL) << std::endl;

  std::cout << cqf.query((4ULL << 59)) << std::endl;

  std::cout << cqf.query((1ULL << 59)) << std::endl;
  
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((1ULL << 59)) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((4ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((9ULL << 59)) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((5ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((10ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((1ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((4ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((3ULL << 59) | 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((31ULL << 59)) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((11ULL << 59)| 188ULL) << std::endl;
  cqf.show();
  std::cout << "REMOVE" << std::endl;
  std::cout << cqf.remove((3ULL << 59)) << std::endl;
  cqf.show();

}

void test_cqf_bound_del(){
  uint64_t num_of_blocks = 3;
  Cqf cqf(59,3);

    //setto offsets
  cqf.cqf[0] = 0ULL;//1ULL
  cqf.cqf[8] = 0ULL;
  cqf.cqf[16] = 0ULL;
  
  //setto occupieds
  //            0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000000000000000000000000000000000000000000000000000001ULL;
  cqf.cqf[9] =  0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[17] = 0b0000100000000000000000000000000000000000000000000000000000000000ULL;

  //setto runends
  //            0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000000000000000000000000000000000000000000000000001000ULL;
  cqf.cqf[10] = 0b0000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.cqf[18] = 0b1000000000000000000000000000000000000000000000000000000000000000ULL;
  cqf.show();

  std::cout << "TESTING 0, " << "RB: " << cqf.find_boundary_shift_deletion(0,cqf.get_prev_quot(cqf.first_unused_slot(0))) << " EXPECTED: 3" << std::endl;

  std::cout << "TESTING 187, " << "RB: " << cqf.find_boundary_shift_deletion(187,cqf.get_prev_quot(cqf.first_unused_slot(187))) << " EXPECTED: 191" << std::endl;


  //setto offsets
  cqf.cqf[0] = 1ULL;//1ULL
  cqf.cqf[8] = 0ULL;
  cqf.cqf[16] = 2ULL;
  
  //setto occupieds
  //       0b1000000010000000100000001000000010000000100000001000000010000000ULL;
  cqf.cqf[1] =  0b0000000000000010000000000000000010000000000000000000010000000000ULL;
  cqf.cqf[9] =  0b0001000100000000000000000000001000000000000000010000000000000000ULL;
  cqf.cqf[17] = 0b0100000000000000000000000000000000000000000000000010000001000000ULL;

  //setto runends
  //       0b0000000000000101000000000000000000000000000000000001000000000100ULL;
  cqf.cqf[2] =  0b0000000000000101000000000001000000000000000000000001000000000100ULL;
  cqf.cqf[10] = 0b0000000000000000000000000100000000000000010000000000000000000000ULL;
  cqf.cqf[18] = 0b0000000000000000000000010000000000000000000000000001010000001000ULL;

  //print_vector(cqf.cqf);
  cqf.show();

  std::cout << "TESTING 31, " << "RB: " << cqf.find_boundary_shift_deletion(31,cqf.get_prev_quot(cqf.first_unused_slot(31))) << " EXPECTED: 36" << std::endl;

  std::cout << "TESTING 35, " << "RB: " << cqf.find_boundary_shift_deletion(35, cqf.get_prev_quot(cqf.first_unused_slot(35))) << " EXPECTED: 36" << std::endl;

  std::cout << "TESTING 122, " << "RB: " << cqf.find_boundary_shift_deletion(122, cqf.get_prev_quot(cqf.first_unused_slot(122))) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 134, " << "RB: " << cqf.find_boundary_shift_deletion(134, cqf.get_prev_quot(cqf.first_unused_slot(134))) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 135, " << "RB: " << cqf.find_boundary_shift_deletion(135, cqf.get_prev_quot(cqf.first_unused_slot(135))) << " EXPECTED: 140" << std::endl;

  std::cout << "TESTING 190, " << "RB: " << cqf.find_boundary_shift_deletion(190, cqf.get_prev_quot(cqf.first_unused_slot(190))) << " EXPECTED: 2" << std::endl;

}