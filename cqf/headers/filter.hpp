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
    //Cqf(uint64_t quotient_s, uint64_t n_blocks); // deprecated, used for early tests

    /** Constructor that deduces quotient and reminder sizes from the desired struct size
     * @param max_memory The desired (maximum) size of the CQF (in MBytes)
     */
    Cqf(uint64_t max_memory, bool verbose = false);
    
    /*
    HIGH LEVEL PUBLIC OPERATIONS
    */

    /** Deduce a quotient size from the memory occupation limit
     * The filter is divided into blocks: each block contains 64 quotients, and occupies 3 words of metadata and 'r' words of remainders (since ther are 64 remainders inside) --> (r + 3) words == ((64 - q) + 3) words;
     * There are 2^(q) quotients in the filter. Each block contains 64 quotients --> #blocks = 2^(q)/64. Since I cannot analitycally get q from the equation MAX_MEM = (2^(q) / 64)((64 - q) + 3),I try values in a for loop. Starting with q = 63, I lower the size of q till I get the mem of the filter <= of max memory.
     * P.S.: To solve some problems with the comparison of numbers, for large values of q, I divide the 2^(q) by the number of bytes in a MB. For the smaller values I multiply the MAX_MEM value by the same amount.
     * 
     * @param max_memory Max size to occupy with the CQF (in MBytes). The memory of the CQF is given by 1 parameter: the quotient size(q), the remainder size(r) is (64 - q).
     * 
     * @return Quotient size in bits
     **/
    uint64_t find_quotient_given_memory(uint64_t max_memory);
    
    /** insert a new number in the filter. TODO: What if already present ? ADD COUNTERS.
     * @param number to insert
     * This function inserts a number in the RSQF. IF the number is already present, it just adds another copy of the number.
     * If a number has the same quotient but different reminders, it stores reminders in a monothonic way
     * (i.e. each reminder in a run is greater or equal than the predecessor).
     * If the filter is full, new insertions are authomatically discarded.
     * When adding a new element, all reminders and runend bits are shifted right of 1 position.
     */
    void insert(uint64_t number);
    
    /** query a number from the filter.
     * @param number Number to query
     * @return TODO: ADD COUNTERS
     * it queries a number. It first checks if the occupied bit of the quotient is set to 1. If so it scans
     * in a linear way the remainders of the run associated to this element. If it find the remainder it returns 1 else 0.
     * Stops immediately if the filter is empty
     */
    uint64_t query(uint64_t number);

    /** remove (if present) a number from the filter
     * @param number value to remove
     * @return TODO: ADD COUNTERS
     * This method removes an element from the filter. At the beginning it works like a query. If it finds the 
     * searched element, it find the rightmost remainder it has to shift to mantain the QF organized and then shift
     * everything to remove the element. The rightmost remainder can be the one before the FUS (First Unused Slot) or another remainder 
     * saved in a slot before it. There are some edge cases where the remainder slots are all occupied up to a certain
     * slot but I cannot shift left part of the remainders beacuse they would be moved in a position that is smaller
     * than the one of their quotient. This vould violate one of the rules of the QF.
     * 
     */
    uint64_t remove(uint64_t number);




    uint64_t get_quot_size();

    uint64_t get_num_el_inserted();

    void show() const;
    void show_slice(uint64_t start, uint64_t end) const;

    // FUNCTIONS

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
    uint64_t get_previous_runend(uint64_t quotient) const;
    
    std::vector<uint64_t> cqf; // uint64_t vector to store the cqf
    uint64_t get_prev_quot(uint64_t current_quot) const;

    private:
    // VALUES

    bool verbose;
    bool debug;
    uint64_t m_num_bits;    // max number of bits occupied by the cqf to check no memory leaks
    uint64_t quotient_size; // value of q
    uint64_t remainder_size;    // value of r
    uint64_t number_blocks; // number of blocks the cqf is divided into
    // uint64_t block_size;
    uint64_t elements_inside;



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

