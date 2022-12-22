#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>
#include <stdint.h> 

#endif


class Cqf {
    
    public:
    
    /*
    
    OPERATIONS ON THE ENTIRE CQF
    
    */
    // constructor
    Cqf(uint64_t quotient_s);
    Cqf(uint64_t quotient_s, uint64_t n_blocks);
    
    /*

    HIGH LEVEL OPERATIONS
    
    */

    // insert a new number in the filter
    uint64_t insert(uint64_t number);

    // query a number from the filter
    uint64_t query(uint64_t number) const;

    // remove(if present) a number from the filter
    uint64_t remove(uint64_t number);

    
    private:
    // VALUES

    std::vector<uint64_t> cqf; // uint64_t vector to store the cqf
    uint64_t m_num_bits;    // max number of bits occupied by the cqf to check no memory leaks
    uint64_t quotient_size; // value of q
    uint64_t remainder_size;    // value of r
    uint64_t number_blocks; // number of blocks the cqf is divided into
    uint64_t block_size;

    // FUNCTIONS
    uint64_t quotient(uint64_t num) const;
    uint64_t remainder(uint64_t num) const;

    uint64_t get_next_quot(uint64_t current_quot) const;
    uint64_t get_prev_quot(uint64_t current_quot) const;
    
    uint64_t get_prev_block_id(uint64_t current_block) const;
    uint64_t get_next_block_id(uint64_t current_block) const;
    
    uint64_t get_runend_word(uint64_t current_block) const;
    uint64_t get_occupied_word(uint64_t current_block) const;
    uint64_t get_offset_word(uint64_t current_block) const;

    void Cqf::set_runend_word(uint64_t current_block, uint64_t value);
    void Cqf::set_offset_word(uint64_t current_block, uint64_t value);
    void Cqf::set_occupied_bit(uint64_t current_block, uint64_t value, uint64_t bit_pos);
};  

