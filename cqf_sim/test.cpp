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

uint64_t get_block_id(uint64_t number){
  return number/MEM_UNIT;
}

uint64_t get_shift_in_block(uint64_t number){
  return number%MEM_UNIT;
}


uint64_t get_next_block_id(std::vector<uint64_t>& vec, uint64_t current_block){
  uint64_t num_blocks = vec.size()/3;
  //std::cout << "FLI " << filter_length << std::endl;
  //std::cout << "CBI " << current_block << std::endl;
  if (current_block < num_blocks - 1 ) return ++current_block;
  else return 0;
}

uint64_t get_runend_word(std::vector<uint64_t>& vec, uint64_t current_block){
  uint64_t runend_id = (current_block * MET_UNIT) + RUN_POS;
  return vec[runend_id];
}

uint64_t get_occupied_word(std::vector<uint64_t>& vec, uint64_t current_block){
  uint64_t occupied_id = (current_block * MET_UNIT) + OCC_POS;
  return vec[occupied_id];
}

uint64_t get_offset_word(std::vector<uint64_t>& vec, uint64_t current_block){
  uint64_t offset_id = (current_block * MET_UNIT) + OFF_POS;
  return vec[offset_id];
}

void set_runend_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value ){
  uint64_t runend_id = (current_block * MET_UNIT) + RUN_POS;
  vec[runend_id] = value;
}

void set_offset_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value ){
  uint64_t offset_id = (current_block * MET_UNIT) + OFF_POS;
  vec[offset_id] = value;
}

void set_occupied_bit(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value ,uint64_t bit_pos){
  uint64_t occupied_id = (current_block * MET_UNIT) + OCC_POS;
  uint64_t occ_word = get_occupied_word(vec,current_block);
  std::cout << "STR ";
  print_bits(occ_word);
  value &= mask_right(1);
  value <<= bit_pos;
  uint64_t out_value = ((occ_word & mask_right(bit_pos)) | value);
  std::cout << "MSK ";
  print_bits((occ_word & mask_right(bit_pos - 1)));
  std::cout << "MID ";
  print_bits(out_value);
  out_value |= (occ_word & mask_left(MEM_UNIT-bit_pos-1));
  vec[occupied_id] = out_value;
  std::cout << "END ";
  print_bits(out_value);
}

void shift_bits_left_metadata( std::vector<uint64_t>& vect, uint64_t new_bit, uint64_t start_position, uint64_t end_position){
  uint64_t overflow_bit = new_bit;
  uint64_t current_block = get_block_id(start_position);
  uint64_t current_shift_in_block = get_shift_in_block(start_position);
  uint64_t end_block = get_block_id(end_position);
  uint64_t end_shift_in_block = get_shift_in_block(end_position);

  uint64_t word_to_shift = 0;
  uint64_t save_right = 0;
  uint64_t to_shift = 0;

  //std::cout << "ENP " << end_position << std::endl;
  //std::cout << "CB  " << current_block << std::endl;
  //std::cout << "CSB " << current_shift_in_block << std::endl;
  //std::cout << "EB  " << end_block << std::endl;
  //std::cout << "ESB " << end_shift_in_block << std::endl;

  /*
  if (value_occupied == 1){
        set_occupieds_bit(curr_block,curr_pos_in_block,quot_value);
    }
  */
  set_occupied_bit(vect, current_block,1,3);
  set_occupied_bit(vect, current_block,1,4);
  set_occupied_bit(vect, current_block,1,5);

  

  if ((current_block == end_block) && (start_position > end_position)){
    //show(iteration, "iteration counter");
    word_to_shift = get_runend_word(vect, current_block);
    
    //show(word_to_shift,"word_to_shift");
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    

    overflow_bit <<= current_shift_in_block;
    //show(save_right,"save_right");
    //show(overflow_bit,"overflow_bit");

    to_shift |= (save_right | overflow_bit);

    set_runend_word(vect, current_block, to_shift); 
    //show(to_shift,"to_shift after");
    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    current_block = get_next_block_id(vect,current_block);
    //std::cout << "CBA " << current_block << std::endl;
    current_shift_in_block = 0;
    //iteration++;
  }

  while ( current_block != end_block ){
    //std::cout << "CB  " << current_block << std::endl;
    //show(iteration, "iteration counter");
    word_to_shift = get_runend_word(vect, current_block);
    
    //show(word_to_shift,"word_to_shift");
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    
    overflow_bit <<= current_shift_in_block;
    //show(save_right,"save_right");
    //show(overflow_bit,"overflow_bit");

    to_shift |= (save_right | overflow_bit);

    set_runend_word(vect, current_block, to_shift); 
    //show(to_shift,"to_shift after");
    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    current_block = get_next_block_id(vect,current_block);
    //std::cout << "CBA " << current_block << std::endl;
    current_shift_in_block = 0;
    //iteration++;
  }
  //show(iteration, "iteration counter");
  word_to_shift = get_runend_word(vect, current_block);
  //show(word_to_shift,"word_to_shift");

  uint64_t save_left = (word_to_shift & mask_left(MEM_UNIT-end_shift_in_block));
  to_shift = ((word_to_shift & mask_right(end_shift_in_block)) << 1);
  
  //show(save_left,"save_left");
  //show(to_shift,"to_shift");

  to_shift |= (save_left | overflow_bit);

  set_runend_word(vect, current_block, to_shift); 
  //show(to_shift,"to_shift after");
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
