/*
ADDITIONAL METHODS NOT USED BY THE CQF CLASS
*/
#include <stdint.h>

#include "ext_methods.hpp"

#define MEM_UNIT  64ULL

uint64_t mask_left(uint64_t numbits){
    return ~(mask_right(MEM_UNIT-numbits));
}

uint64_t mask_right(uint64_t numbits){
    uint64_t mask = -(numbits >= MEM_UNIT) | ((1ULL << numbits) - 1ULL);
    return mask;
}
/*
uint64_t get_block_id(uint64_t position){
    return position / MEM_UNIT;
}

uint64_t get_shift_in_block(uint64_t position){
    return position % MEM_UNIT;
}
*/
uint64_t get_quot_from_block_shift(uint64_t block, uint64_t shift){
    return block*MEM_UNIT + shift;
}

/*
ROB PATRO'S PAPER IMPLEMENTATIONS FOR ASM BIT OPERATIONS
*/

uint64_t bitselectasm(uint64_t num, uint64_t rank){

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