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

uint64_t get_absolute_position(uint64_t slot_idx){
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
REMAINDER OPERATIONS
*/
uint64_t Cqf::get_remainder(uint64_t position){

     uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((MET_UNIT+remainder_size)*MEM_UNIT)+MET_UNIT*MEM_UNIT+pos_in_block*remainder_size); 

    return get_bits_custom(pos, remainder_size);
}


uint64_t Cqf::set_remainder(uint64_t position, uint64_t value){
    int64_t block = get_block(position);
    uint64_t pos_in_block = get_shift(position);

    uint64_t pos = (block*((3+remainder_size)*MEM_UNIT)+3*MEM_UNIT+pos_in_block*remainder_size);

    set_bits(pos, value, remainder_size);




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