#ifndef ADDITIONAL_METHODS_HPP
#define ADDITIONAL_METHODS_HPP
#include <stdint.h> 

void print_bits(uint64_t x);

uint64_t mask_left(uint64_t num_bits);
uint64_t mask_right(uint64_t num_bits);

uint64_t get_block_id(uint64_t position);
uint64_t get_shift_in_block(uint64_t position);
uint64_t get_quot_from_block_shift(uint64_t block, uint64_t shift);

uint64_t bitselectasm(uint64_t num, uint64_t rank);
uint64_t bitrankasm(uint64_t val, uint64_t pos);

uint64_t get_bits(std::vector<uint64_t>& vec, uint64_t pos, uint64_t len);
void set_bits(std::vector<uint64_t>& vec, uint64_t pos, uint64_t value, uint64_t len);

#endif