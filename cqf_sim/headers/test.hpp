#ifndef TEST_HPP
#define TEST_HPP
#include <stdint.h> 

void test_cqf_metadata();
void test_metadata_shift();
void test_metadata_shift1();
void test_set_get_reminder();
void test_shift_left_add_reminder();
void test_boundaries();

void test_insert();
void test_fus();
void test_sel_rank_filter();
void test_cqf_bound_del();

void shift_bits_left_metadata( std::vector<uint64_t>& vect,uint64_t quotient, uint64_t new_bit, uint64_t start_position, uint64_t end_position);
void print_vector(std::vector<uint64_t>& vect);

void set_offset_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value );
void set_occupied_bit(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value ,uint64_t bit_pos);
void set_runend_word(std::vector<uint64_t>& vec, uint64_t current_block, uint64_t value );

uint64_t get_offset_word(std::vector<uint64_t>& vec, uint64_t current_block);
uint64_t get_occupied_word(std::vector<uint64_t>& vec, uint64_t current_block);
uint64_t get_runend_word(std::vector<uint64_t>& vec, uint64_t current_block);

#endif