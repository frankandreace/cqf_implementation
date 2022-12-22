#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>     // for std::chrono::high_resolution_clock
#include <string.h>
#include <cassert>
#include <cmath>
#include <stdint.h> 

#include "ext_methods.hpp"
#include "test.hpp"
#include "filter.hpp" 

//FILTER CLASS

// STATIC VARIABLES 
#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL

Cqf::Cqf( uint64_t quotient_s ){
    assert(quotient_s < MEM_UNIT);

    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    uint64_t num_bits_quot = 1ULL << quotient_size;
    number_blocks = std::ceil(num_bits_quot/MEM_UNIT);
    block_size = remainder_size + MET_UNIT;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}

Cqf::Cqf(uint64_t quotient_s, uint64_t n_blocks){
    assert(quotient_s < MEM_UNIT);

    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    block_size = remainder_size + MET_UNIT;
    number_blocks = n_blocks;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}


/*

HIGH LEVEL OPERATIONS



uint64_t Cqf::insert(uint64_t number){
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    // GET FIRST UNUSED SLOT
    uint64_t fu_slot = first_unused_slot(quot);
    
    // IF THE QUOTIENT HAS NEVER BEEN USED BEFORE
    //PUT THE REMAINDER AT THE END OF THE RUN OF THE PREVIOUS USED QUOTIENT OR AT THE POSITION OF THE QUOTIENT
    uint64_t starting_position = sel_rank_filter(get_prev_quot(quot));
    if (starting_position < quot) starting_position = quot;

    if (!is_occupied(quot)){
        update_metadata_insertion(1,starting_position,fu_slot);

        return shift_right_and_set(starting_position, fu_slot, rem);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
    //getting boundaries of the run
        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    //find the place where the remainder should be inserted / all similar to a query
    //getting position where to start shifting right
        starting_position = boundary.first;

        while(starting_position <= boundary.second){
            uint64_t remainder_in_filter = get_remainder(starting_position); 
            if (remainder_in_filter > rem) break;
            starting_position++;
    }
    uint64_t metadata_starting_position = starting_position;
    if(metadata_starting_position == boundary.second) metadata_starting_position--;
    update_metadata_insertion(0,starting_position,fu_slot);
    // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
    return shift_right_and_set_circ(starting_position, fu_slot, rem);
    }

}


uint64_t Cqf::query(uint64_t number) const{
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (is_occupied(quot) == false) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?

    uint64_t position = boundary.first;

    while(position <= boundary.second){
        uint64_t remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) return 1;
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }

    return 0;

}


uint64_t Cqf::remove(uint64_t number){
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (is_occupied(quot) == false) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?
    uint64_t pos_element = 0;
    uint64_t position = boundary.first;

    // GET POSITION
    while(position <= boundary.second){
        uint64_t remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) {
            pos_element = position;
            break;
        }
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }
    if (pos_element == 0) return 0; //not found

    // GET FIRST UNUSED SLOT
    uint64_t end_slot = first_unused_slot(quot);

    // FIND THE EXACT RIGHTMOST ELEMENT TO SHIFT LEFT
    end_slot = find_rightmost_to_shift_left(position,end_slot);

    // REMOVE ELEMENT BY SHIFTING EVERYTHING LEFT OF ONE REMINDER
    shift_left_and_rem_circ(position,end_slot);

    // MOVE 1 LEFT THE RUNEND OR CLEAR IT WITH OCCUPIED
    //update_metadata_deletion(boundary,position,end_slot);

}

*/

//
uint64_t Cqf::quotient(uint64_t num) const{
    return num & mask_right(quotient_size);
}

uint64_t Cqf::remainder(uint64_t num) const{
    return num >> (MEM_UNIT - remainder_size);
}

// CIRCULAR FILTER OPERATIONS


uint64_t Cqf::get_next_quot(uint64_t current_quot) const{
    if (current_quot < number_blocks*MEM_UNIT - 1) return ++current_quot;
    else return 0;
}

uint64_t Cqf::get_prev_quot(uint64_t current_quot) const{
    if (current_quot > 0 ) return --current_quot;
    else return number_blocks*MEM_UNIT - 1;
}


uint64_t Cqf::get_prev_block_id(uint64_t current_block) const{
  if (current_block > 0 ) return --current_block;
  else return number_blocks - 1;
}

uint64_t Cqf::get_next_block_id(uint64_t current_block) const{
  if (current_block < number_blocks - 1 ) return ++current_block;
  else return 0;
}

uint64_t Cqf::get_runend_word(uint64_t current_block) const{
  uint64_t runend_id = (current_block *(MET_UNIT+remainder_size)) + RUN_POS;
  return cqf[runend_id];
}

uint64_t Cqf::get_occupied_word(uint64_t current_block) const{
  uint64_t occupied_id = (current_block *(MET_UNIT+remainder_size)) + OCC_POS;
  return cqf[occupied_id];
}

uint64_t Cqf::get_offset_word(uint64_t current_block) const{
  uint64_t offset_id = (current_block *(MET_UNIT+remainder_size)) + OFF_POS;
  return cqf[offset_id];
}

void Cqf::set_runend_word(uint64_t current_block, uint64_t value ){
  uint64_t runend_id = (current_block *(MET_UNIT+remainder_size)) + RUN_POS;
  cqf[runend_id] = value;
}

void Cqf::set_offset_word(uint64_t current_block, uint64_t value ){
  uint64_t offset_id = (current_block *(MET_UNIT+remainder_size)) + OFF_POS;
  cqf[offset_id] = value;
}

void Cqf::set_occupied_bit(uint64_t current_block, uint64_t value ,uint64_t bit_pos){
  uint64_t occupied_id = (current_block *(MET_UNIT+remainder_size)) + OCC_POS;
  uint64_t occ_word = get_occupied_word(current_block);

  value &= mask_right(1ULL);
  value <<= bit_pos;
  uint64_t out_value = ((occ_word & mask_right(bit_pos)) | value);

  out_value |= (occ_word & mask_left(MEM_UNIT-bit_pos-1));
  cqf[occupied_id] = out_value;

}

//BITVECTOR AND METADATA OPERATIONS

uint64_t Cqf::sel_rank_filter(std::vector<uint64_t>& vec, uint64_t quotient) const{
    //assert(quotient < ( 1ULL << quotient_size));

    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(vec,block);
    uint64_t occupied = get_occupied_word(vec,block);

    uint64_t position = bitrankasm(occupied,pos_in_block) + offset;
    
    while (position == 0){

        if((block == 0) && (position == 0)){
            return 0;
        }
        block = get_prev_block_id(vec,block);

        offset = get_offset_word(vec,block);
        occupied = get_occupied_word(vec,block);

        position = bitrankasm(occupied,MEM_UNIT-1) + offset;
    }

    uint64_t runends = get_runend_word(vec,block);
    uint64_t select = bitselectasm(runends,position - 1);


    while (select == MEM_UNIT){

        block = get_next_block_id(vec,block);
        position -= bitrankasm(runends,MEM_UNIT-1);
        select = bitselectasm(get_runend_word(vec,block),position - 1);
        
    }

    return block * MEM_UNIT + select;
}

std::pair<uint64_t,uint64_t> Cqf::get_run_boundaries(std::vector<uint64_t>& vec, uint64_t quotient) const{
    //assert(quotient < m_num_bits);

    std::pair<uint64_t, uint64_t> boundaries;
    // the end of the run is found like this
    uint64_t end_pos = sel_rank_filter(vec, quotient);

    uint64_t start_pos = sel_rank_filter(vec, get_prev_quot(vec, quotient));

    //this should work for circular filter
    if (((start_pos < quotient) && (start_pos < end_pos) && (end_pos > quotient)) 
    || ((start_pos < quotient) && (start_pos > end_pos) && (end_pos < quotient)) ) boundaries.first = quotient;
    // it is the position just after the end of the previous
    else boundaries.first = get_next_quot(vec,start_pos); 

    boundaries.second = end_pos;

    return boundaries;
}

uint64_t Cqf::first_unused_slot(std::vector<uint64_t>& vec, uint64_t curr_quotient) const{
    //assert(curr_quotient < ( 1ULL << quotient_size));
    uint64_t rend_pos = sel_rank_filter(vec, curr_quotient);
    if (rend_pos == 0) return curr_quotient;
    //preventing erroneous stop when it jumps from the end of the filter to the beginning 
    // and curr_quot > rend_pos for the circularity and not beacuse there is free space.
    while((curr_quotient < rend_pos) || (get_block_id(curr_quotient) > get_block_id(rend_pos))){
        curr_quotient = get_next_quot(vec, rend_pos);
        rend_pos = sel_rank_filter(vec, curr_quotient);
    }

    return curr_quotient;
}

uint64_t Cqf::find_boundary_shift_deletion(std::vector<uint64_t>& vect, uint64_t start_pos, uint64_t end_pos) const{
    //assert(start_pos < ( 1ULL << quotient_size));
    //assert(end_pos < ( 1ULL << quotient_size)); 

    uint64_t curr_block = get_block_id(start_pos);
    uint64_t curr_pos_in_block = get_shift_in_block(start_pos);

    uint64_t end_block = get_block_id(end_pos);
    uint64_t end_pos_in_block = get_shift_in_block(end_pos);

    // #1 get occupieds, offset and runend word
    uint64_t rend_word = get_runend_word(vect, curr_block);
    uint64_t occ_word = get_occupied_word(vect, curr_block);
    uint64_t offset = get_offset_word(vect, curr_block);

    uint64_t sel_occ = 0;
    uint64_t sel_rend = 0;
    uint64_t s_int = 0; // start of the interval for select
    uint64_t e_int = 0; //end of the interval for select

    if ((curr_block == end_block) && (start_pos > end_pos)){
      s_int = bitrankasm(occ_word,curr_pos_in_block);
      e_int = bitrankasm(occ_word,MEM_UNIT - 1);

      for (uint64_t i=s_int; i < e_int; ++i){
          
          sel_occ = bitselectasm(occ_word,i); //i + 1
          sel_rend = bitselectasm(rend_word,i + offset - 1);
          if (sel_occ > sel_rend){
              return get_quot_from_block_shift(curr_block,sel_rend);
          }
      }
      // adjourning block and words for next search (in or outside loop)
      curr_pos_in_block = 0;
      curr_block = get_next_block_id(vect, curr_block);
      rend_word = get_runend_word(vect, curr_block);
      occ_word = get_occupied_word(vect, curr_block);
      offset = get_offset_word(vect, curr_block);
    }

    while (curr_block != end_block)
    {   
        s_int = bitrankasm(occ_word,curr_pos_in_block);
        e_int = bitrankasm(occ_word,MEM_UNIT - 1);

        for (uint64_t i=s_int; i < e_int; ++i){
            
            sel_occ = bitselectasm(occ_word,i); //i + 1
            sel_rend = bitselectasm(rend_word,i + offset - 1);
            if (sel_occ > sel_rend){
                return get_quot_from_block_shift(curr_block,sel_rend);
            }
        }
        // adjourning block and words for next search (in or outside loop)
        curr_pos_in_block = 0;
        curr_block = get_next_block_id(vect, curr_block);
        rend_word = get_runend_word(vect, curr_block);
        occ_word = get_occupied_word(vect, curr_block);
        offset = get_offset_word(vect, curr_block);
    }
    // curr_block = end_block
    s_int = bitrankasm(occ_word,curr_pos_in_block);
    e_int = bitrankasm(occ_word,end_pos_in_block);

    for (uint64_t i=s_int; i < e_int; ++i){
        
        sel_occ = bitselectasm(occ_word,i);
        sel_rend = bitselectasm(rend_word,i+offset-1);
        if (sel_occ >= sel_rend){
            return get_quot_from_block_shift(curr_block,sel_rend);
        }
    }
    return end_pos;
}