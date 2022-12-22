/*
ADDITIONAL METHODS NOT USED BY THE CQF CLASS
*/
#include <iostream>
#include <bitset>
#include <stdint.h> 
#include <cassert>


#include "filter.hpp" 
#include "ext_methods.hpp"
#include "test.hpp"

#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL

uint64_t get_next_quot(std::vector<uint64_t>& vec, uint64_t current_quot){
    uint64_t num_blocks = vec.size()/3;
    if (current_quot < num_blocks*MEM_UNIT - 1) return ++current_quot;
    else return 0;
}

uint64_t get_prev_quot(std::vector<uint64_t>& vec, uint64_t current_quot){
    uint64_t num_blocks = vec.size()/3;
    if (current_quot > 0 ) return --current_quot;
    else return num_blocks*MEM_UNIT - 1;
}

uint64_t mask_left(uint64_t numbits){
    return ~(mask_right(MEM_UNIT-numbits));
}

uint64_t mask_right(uint64_t numbits){
    uint64_t mask = -(numbits >= MEM_UNIT) | ((1ULL << numbits) - 1ULL);
    return mask;
}

uint64_t get_block_id(uint64_t position){
    return position / MEM_UNIT;
}

uint64_t get_shift_in_block(uint64_t position){
    return position % MEM_UNIT;
}

uint64_t get_quot_from_block_shift(uint64_t block, uint64_t shift){
    return block*MEM_UNIT + shift;
}

uint64_t get_prev_block_id(std::vector<uint64_t>& vec, uint64_t current_block){
  uint64_t num_blocks = vec.size()/3;
  //std::cout << "FLI " << filter_length << std::endl;
  //std::cout << "CBI " << current_block << std::endl;
  if (current_block > 0 ) return --current_block;
  else return num_blocks - 1;
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
  //std::cout << "STR ";
  //print_bits(occ_word);
  value &= mask_right(1);
  value <<= bit_pos;
  uint64_t out_value = ((occ_word & mask_right(bit_pos)) | value);
  //std::cout << "MSK ";
  //print_bits((occ_word & mask_right(bit_pos - 1)));
  //std::cout << "MID ";
  //print_bits(out_value);
  out_value |= (occ_word & mask_left(MEM_UNIT-bit_pos-1));
  vec[occupied_id] = out_value;

}

/*
RANK AND SELECT
*/

// find first unused slot given a position
uint64_t first_unused_slot(std::vector<uint64_t>& vec, uint64_t curr_quotient){
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

std::pair<uint64_t,uint64_t> get_run_boundaries(std::vector<uint64_t>& vec, uint64_t quotient){
    //assert(quotient < m_num_bits);

    //uint64_t block = get_block_id(quotient);
    //uint64_t pos_in_block = get_shift_in_block(quotient);

    /*
    temporary here
    uint64_t occupied = get_occupied_word(vec,block);
    uint64_t bit_occ = (occupied >> pos_in_block) & 1;
    if (bit_occ == 0){
        boundaries.first = 0;
        boundaries.second = 0;
        return boundaries;
    }
    to here
    */
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
/*
    uint64_t position_in_rend = bitrankasm(occupied,pos_in_block) + offset - 1;

    // the start of the run is either the position in the block of the quotient or 
    // just after the end of the previous run. I use 'start' to keep track where is the start
    // of the previous run. If at the end it is 'after' the quotient, I use it.
    uint64_t start_pos_select = bitselectasm(runends,position_in_rend - 1);

    uint64_t end_pos_select = bitselectasm(runends,position_in_rend);

    // while the end is not in this word
    while (end_pos_select == MEM_UNIT){

        block = get_next_block_id(vec, block);
        position_in_rend -= bitrankasm(runends,MEM_UNIT-1);
        runends = get_runend_word(vec,block);
        end_pos_select = bitselectasm(runends,position_in_rend);

        if (start_pos_select == MEM_UNIT){
            start_pos_select = bitselectasm(runends,position_in_rend - 1);
        }
        
    }
*/


uint64_t sel_rank_filter(std::vector<uint64_t>& vec, uint64_t quotient){
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

uint64_t bitselectasm(uint64_t num, uint64_t rank){
    assert(rank < MEM_UNIT);
    uint64_t i = 1ULL << rank; // i = 2^rank

    // SELECT(v,i) = TZCNT(PDEP(2^rank,num))     
	asm("pdep %[num], %[mask], %[num]"
			: [num] "+r" (num)
			: [mask] "r" (i));

	asm("tzcnt %[bit], %[index]"
			: [index] "=r" (i)
			: [bit] "g" (num)
			: "cc");

	return i;
}

uint64_t bitrankasm(uint64_t val, uint64_t pos) {
    assert(pos < MEM_UNIT);
	val = val & ((2ULL << pos) - 1);
	asm("popcnt %[val], %[val]"
			: [val] "+r" (val)
			:
			: "cc");
	return val;
}

/*
uint64_t popcntasm(uint64_t num){
    
    // RANK(v,i) = POPCOUNT(v & (2^(i) -1))
    asm("popcnt %[num], %[num]"
                : [num] "+r" (num)
                :
                : "cc");
        return num;
}
*/


void shift_bits_left_metadata( std::vector<uint64_t>& vect, uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    // METHOD FOR INSERTION

  uint64_t overflow_bit = flag_bit;
  uint64_t current_block = get_block_id(start_position);
  uint64_t current_shift_in_block = get_shift_in_block(start_position);
  uint64_t end_block = get_block_id(end_position);
  uint64_t end_shift_in_block = get_shift_in_block(end_position);

  uint64_t word_to_shift = 0;
  uint64_t save_right = 0;
  uint64_t to_shift = 0;
  uint64_t next_block = 0;
  
  // IF FLAG == 1: I HAVE TO SET THE OCCUPIED BIT OF THE QUOTIENT TO 1
  if (flag_bit == 1){
    uint64_t quot_block = get_block_id(quotient);
    uint64_t quot_shift_in_block = get_shift_in_block(quotient);
    set_occupied_bit(vect, quot_block,flag_bit,quot_shift_in_block);
  }

  if ((current_block == end_block) && (start_position > end_position)){

    word_to_shift = get_runend_word(vect, current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    
    overflow_bit <<= current_shift_in_block;
    to_shift |= (save_right | overflow_bit);
    set_runend_word(vect, current_block, to_shift); 

    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    next_block = get_next_block_id(vect,current_block);

    if (overflow_bit == 1){
        uint64_t old_offset = get_offset_word(vect, current_block);

        if ((bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) && (old_offset > 0)) set_offset_word(vect, current_block, old_offset - 1);
        set_offset_word(vect, next_block, get_offset_word(vect, next_block) + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
    
    // IF OVERFLOW BIT IS 1, OFFSET OF THE NEXT BLOCK SHOULD BE INCREASED
    // AND OFFSET OF THIS WORD SHOULD BE DECREASED IF THE BIT WAS PART OF THAT

  }

  while ( current_block != end_block ){

    word_to_shift = get_runend_word(vect, current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    
    overflow_bit <<= current_shift_in_block;
    to_shift |= (save_right | overflow_bit);
    set_runend_word(vect, current_block, to_shift); 

    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    next_block = get_next_block_id(vect,current_block);

    if (overflow_bit == 1){
        uint64_t old_offset = get_offset_word(vect, current_block);

        if ((bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) && (old_offset > 0)) set_offset_word(vect, current_block, old_offset - 1);
        set_offset_word(vect, next_block, get_offset_word(vect, next_block) + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
  }

  word_to_shift = get_runend_word(vect, current_block);
  uint64_t save_left = (word_to_shift & mask_left(MEM_UNIT-end_shift_in_block));

  to_shift = ((word_to_shift & mask_right(end_shift_in_block)) << 1);
  to_shift |= (save_left | overflow_bit);
  set_runend_word(vect, current_block, to_shift); 

}


void shift_bits_right_metadata( std::vector<uint64_t>& vect, uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    // METHOD FOR DELETION

  uint64_t overflow_bit = flag_bit;
  uint64_t current_block = get_block_id(start_position);
  uint64_t current_shift_in_block = get_shift_in_block(start_position);
  uint64_t end_block = get_block_id(end_position);
  uint64_t end_shift_in_block = get_shift_in_block(end_position);

  uint64_t word_to_shift = 0;
  uint64_t save_right = 0;
  uint64_t to_shift = 0;
  uint64_t next_block = 0;
  

  if ((current_block == end_block) && (start_position > end_position)){

    word_to_shift = get_runend_word(vect, current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));
    //std::cout << "SAVE  ";
    //print_bits(save_right);
    //std::cout << "SHIFT ";
    //print_bits(to_shift);
    next_block = get_next_block_id(vect,current_block);
    
    overflow_bit = ((get_runend_word(vect, next_block) & 1ULL) << (MEM_UNIT - 1));
    //std::cout << "OB    ";
    //print_bits(overflow_bit);
    to_shift |= (save_right | overflow_bit);
    set_runend_word(vect, current_block, to_shift);
    //std::cout << "FIN   ";
    //print_bits(to_shift);

    overflow_bit >>= (MEM_UNIT - 1);

    uint64_t new_offset = get_offset_word(vect, next_block);
    if ((overflow_bit == 1) && (new_offset > 0)){
        set_offset_word(vect, next_block, new_offset - 1);

        uint64_t old_offset = get_offset_word(vect, current_block);
        if (bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) set_offset_word(vect, current_block, old_offset + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
    
    // IF OVERFLOW BIT IS 1, OFFSET OF THE NEXT BLOCK SHOULD BE INCREASED
    // AND OFFSET OF THIS WORD SHOULD BE DECREASED IF THE BIT WAS PART OF THAT

  }

  while ( current_block != end_block ){

    word_to_shift = get_runend_word(vect, current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));
    //std::cout << "SAVE  ";
    //print_bits(save_right);
    //std::cout << "SHIFT ";
    //print_bits(to_shift);

    next_block = get_next_block_id(vect,current_block);
    
    overflow_bit = ((get_runend_word(vect, next_block) & 1ULL) << (MEM_UNIT - 1));
    //std::cout << "OB    ";
    //print_bits(overflow_bit);
    to_shift |= (save_right | overflow_bit);
    set_runend_word(vect, current_block, to_shift); 
    //std::cout << "FIN   ";
    //print_bits(to_shift);

    overflow_bit >>= (MEM_UNIT - 1);

    uint64_t new_offset = get_offset_word(vect, next_block);
    if ((overflow_bit == 1) && (new_offset > 0)){
        set_offset_word(vect, next_block, new_offset - 1);

        uint64_t old_offset = get_offset_word(vect, current_block);
        if (bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) set_offset_word(vect, current_block, old_offset + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
  }

  word_to_shift = get_runend_word(vect, current_block);
  uint64_t save_left = (word_to_shift & mask_left(MEM_UNIT-end_shift_in_block));
  to_shift = ((word_to_shift & mask_right(end_shift_in_block)) >> 1);
  //std::cout << "SAVEL ";
  //print_bits(save_left);
  //std::cout << "SHIFT ";
  //print_bits(to_shift);
  to_shift |= (save_left | overflow_bit);
  set_runend_word(vect, current_block, to_shift); 
  //std::cout << "FIN   ";
  //print_bits(to_shift);
}

uint64_t find_boundary_shift_deletion(std::vector<uint64_t>& vect, uint64_t start_pos, uint64_t end_pos){
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