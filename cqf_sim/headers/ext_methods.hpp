#ifndef EXT_METHODS_HPP
#define EXT_METHODS_HPP
#include <stdint.h> 

uint64_t get_prev_quot(std::vector<uint64_t>& vec, uint64_t current_quot);
uint64_t get_next_quot(std::vector<uint64_t>& vec, uint64_t current_quot);

uint64_t mask_left(uint64_t num_bits);

uint64_t mask_right(uint64_t num_bits);

uint64_t get_next_block_id(std::vector<uint64_t>& vec, uint64_t current_block);

uint64_t get_block_id(uint64_t position);

uint64_t get_shift_in_block(uint64_t position);

uint64_t get_quot_from_block_shift(uint64_t block, uint64_t shift);

void shift_bits_right_metadata( std::vector<uint64_t>& vect, uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);
void shift_bits_right_metadata( std::vector<uint64_t>& vect, uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);

uint64_t find_boundary_shift_deletion(std::vector<uint64_t>& vect, uint64_t start_pos, uint64_t end_pos);

uint64_t first_unused_slot(std::vector<uint64_t>& vec, uint64_t curr_quotient);
std::pair<uint64_t,uint64_t> get_run_boundaries(std::vector<uint64_t>& vec, uint64_t quotient);

uint64_t sel_rank_filter(std::vector<uint64_t>& vec, uint64_t quotient);

uint64_t bitselectasm(uint64_t num, uint64_t rank);
uint64_t bitrankasm(uint64_t val, uint64_t pos);

void set_offset_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value );
void set_occupied_bit(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value ,uint64_t bit_pos);
void set_runend_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value );

uint64_t get_offset_word(std::vector<uint64_t>& vec, uint64_t current_block);
uint64_t get_occupied_word(std::vector<uint64_t>& vec, uint64_t current_block);
uint64_t get_runend_word(std::vector<uint64_t>& vec, uint64_t current_block);

#endif