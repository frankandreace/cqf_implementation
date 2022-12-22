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

/*
RANK AND SELECT
*/

// find first unused slot given a position



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
