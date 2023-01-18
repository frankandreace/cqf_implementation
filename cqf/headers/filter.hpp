#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>
#include <stdint.h> 

#include "additional_methods.hpp"

#endif


class Cqf {
    
    public:
    
    /*
    OPERATIONS ON THE ENTIRE CQF
    */
    // constructor
    //Cqf(uint64_t quotient_s);
    Cqf(uint64_t quotient_s, uint64_t n_blocks);
    Cqf(uint64_t max_memory);
    /*
    HIGH LEVEL PUBLIC OPERATIONS
    */
    uint64_t find_quotient_given_memory(uint64_t max_memory);
    // insert a new number in the filter
    void insert(uint64_t number);
    // query a number from the filter
    uint64_t query(uint64_t number);
    // remove(if present) a number from the filter
    uint64_t remove(uint64_t number);

    void cinsert(uint64_t number);
    // query a number from the filter
    uint64_t cquery(uint64_t number);
    // remove(if present) a number from the filter
    uint64_t cremove(uint64_t number);

    uint64_t get_quot_size();

    uint64_t get_num_el_inserted();

    void show() const;
    void show_slice(uint64_t start, uint64_t end) const;

    // FUNCTIONS
    void increase_counter(uint64_t value, counter_el run_info, uint64_t fus);
    void decrease_counter(uint64_t value, counter_el run_info, uint64_t fus);
    
    void print_counter(uint64_t value, uint64_t start, uint64_t end);
    counter_el scan_for_elements(uint64_t start, uint64_t end, uint64_t searched_value);

    bool is_occupied(uint64_t position);

    uint64_t get_remainder(uint64_t position);
    uint64_t get_remainder_func(uint64_t position);
    void set_remainder(uint64_t position, uint64_t value);

    uint64_t get_remainder_word_position(uint64_t quotient);
    uint64_t get_remainder_shift_position(uint64_t quotient);

    void shift_left_and_set_circ(uint64_t start_quotient,uint64_t end_quotient, uint64_t next_remainder);
    void shift_right_and_rem_circ(uint64_t start_quotient,uint64_t end_quotient);

    void shift_bits_right_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);
    void shift_bits_left_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);

    uint64_t find_boundary_shift_deletion(uint64_t start_pos, uint64_t end_pos) const;
    uint64_t first_unused_slot(uint64_t curr_quotient) const;
    std::pair<uint64_t,uint64_t> get_run_boundaries(uint64_t quotient) const;
    uint64_t sel_rank_filter(uint64_t quotient) const;
    
    std::vector<uint64_t> cqf; // uint64_t vector to store the cqf
    uint64_t get_prev_quot(uint64_t current_quot) const;

    private:
    // VALUES

    
    uint64_t m_num_bits;    // max number of bits occupied by the cqf to check no memory leaks
    uint64_t quotient_size; // value of q
    uint64_t remainder_size;    // value of r
    uint64_t number_blocks; // number of blocks the cqf is divided into
    uint64_t block_size;
    uint64_t elements_inside;
    uint64_t max_encoded_value;



    uint64_t quotient(uint64_t num) const;
    uint64_t remainder(uint64_t num) const;

    uint64_t get_next_remainder_word(uint64_t current_word) const;
    uint64_t get_prev_remainder_word(uint64_t current_word) const;

    uint64_t get_next_quot(uint64_t current_quot) const;

    
    uint64_t get_prev_block_id(uint64_t current_block) const;
    uint64_t get_next_block_id(uint64_t current_block) const;
    
    uint64_t get_runend_word(uint64_t current_block) const;
    uint64_t get_occupied_word(uint64_t current_block) const;
    uint64_t get_offset_word(uint64_t current_block) const;

    void set_runend_word(uint64_t current_block, uint64_t value);
    void set_offset_word(uint64_t current_block, uint64_t value);
    void set_occupied_bit(uint64_t current_block, uint64_t value, uint64_t bit_pos);
    void set_runend_bit(uint64_t current_block, uint64_t value ,uint64_t bit_pos);
};  

