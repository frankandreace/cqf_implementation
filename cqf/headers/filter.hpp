#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>
#include <stdint.h> 
#include <unordered_set>

#include "additional_methods.hpp"

#endif


class Cqf {
    
    public:
    /*
    OPERATIONS ON THE ENTIRE CQF
    */
    // constructor

    Cqf();
    //Cqf(uint64_t quotient_s);
    Cqf(uint64_t q_size, uint64_t r_size, bool verbose = false); // deprecated, used for early tests, back for debug

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

    std::unordered_set<uint64_t> enumerate(); 



    /** returns the size in bits of the quotient
     */    
    uint64_t get_quot_size();

    /** returns the number of numbers inserted 
     */ 
    uint64_t get_num_el_inserted();

    /** Prints the entire cqf, block by block at the bit level 
     */ 
    void show() const;

    /** Prints a slice of the cqf, block by block at the bit level
     * @param start starting block 
     * @param end ending block 
     */ 
    void show_slice(uint64_t start, uint64_t end) const;

    // FUNCTIONS

    /** check if the occupied bit of the given quotient is 1 or 0.
     * @param position quotient to query
     * @return true if 1, false if 0.
     * It extracts the related bit of the occupied word in the block of the quotient
     */
    bool is_occupied(uint64_t position);
    bool is_runend(uint64_t position);

    /** returns the remainder slot associated to the requested quotient
     * @param position quotient 
     * @return an uint64 with the value stored in the slot
     */
    uint64_t get_remainder(uint64_t position);

    /** deprecated version of the get_remainder function. Not used anymore in the filter
     */
    uint64_t get_remainder_func(uint64_t position);

    /** sets the remainder slot associated to the requested quotient to a given value
     * @param position quotient associated to the slot to set 
     * @param value value to store in the slot
     * If I want to set the 155th remainder slot to 1, I will use position=155 and value=1.
     */
    void set_remainder(uint64_t position, uint64_t value);

    /** returns the id of the word in the filter that contains (or starts containing) the
     * remainder slot associated to a certain quotient (by this I mean quotient 155 is associated to
     * the 155th slot (since there is a 0th)).
     * @param quotient the *th quotient
     * @return uint64 returning the id of *th word in the vector.
     * from the block of the quotient and the position in the block it gets the word where the slot starts
     */
    uint64_t get_remainder_word_position(uint64_t quotient);

    /** complementary to "get_remainder_word_position", it gives back the bit in the word where the requested slot
     * starts.
     * @param quotient the *th slot requested
     * @return uint64 with the shift in the word where the slot starts
     * It returns the bit of word where the slot starts.
     */
    uint64_t get_remainder_shift_position(uint64_t quotient);

    /** this function shifts the remainders on 1 slot from start to end and insert a new remainder in the start position
     * @param start_quotient quotient where to start the shifting
     * @param end_quotient quotient where to end the shifting
     * @param next_remainder the value to store in the remainder slot at 'start_quotient' position
     * It shifts (left) bits in words to leave space for a new insertion. Then inserts the value.
     */
    void shift_left_and_set_circ(uint64_t start_quotient,uint64_t end_quotient, uint64_t next_remainder);

    /** this function shift the remainders of 1 slot from end to start and clears the 'end' slot. 
     * It is used to remove remainders.
     * @param start_quotient position that will be overwritten when shifting
     * @param end_quotient position that will be zeroed when shifting
     * It shifts bits right in the words to overwrite the 'start_quotient' slot that has to be removed.
     */
    void shift_right_and_rem_circ(uint64_t start_quotient,uint64_t end_quotient);

    /** Metadata function that shifts the bits right in the runend word when there is a deletion.
     * It also handles cases when occupieds and offsets have to be adjourned.
     * @param quotient quotient of the number to delete.
     * @param flag_bit flags (with 0) if the occupied bit has to be zeroed, else it is 1.
     * @param start_position starting position of the shifting.
     * @param end_position end position of the shifting.
     */
    void shift_bits_right_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);

    /** Metadata function that shifts the bits left in the runend word when there is an insertion.
     * It also handles cases when occupieds and offsets have to be adjourned.
     * @param quotient quotient of the number to add.
     * @param flag_bit flags (with 1) if the occupied bit has to be set to 1, else it is 0.
     * @param start_position starting position of the shifting.
     * @param end_position end position of the shifting.
     */
    void shift_bits_left_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position);

    /** This function finds the rightmost slot to shift when deleting a runend slot. It is needed because there are
     * cases where I cannot shift from the slot to the first unused slot. (for example there are no free slots for a lot
     * of slots in the right but I cannot shift the block next to the one I am deleting because it is exactly in the position
     * associated to it quotient (i.e. quotient of remainder is 5 and the block is the 5th (there is a 0th))).
     * @param start_position position of the slot to delete
     * @param end_position position of the first unused slot
     * @return the last position I have to shift.
     */
    uint64_t find_boundary_shift_deletion(uint64_t start_pos, uint64_t end_pos) const;

    /** Returns the first unused slot as described in the paper. 
     * Used on insertion and to find the shifting window of deletions.
     * @param curr_quotient the quotient from where to find the first unused slot
     * @return uint64 of the position of the first unused slot.
     * It extracts the related bit of the occupied word in the block of the quotient
     */
    uint64_t first_unused_slot(uint64_t curr_quotient); //const

    /** returns the start and the end of the run that contains all the remainders of the numbers inserted that have 
     * the given quotient. I.e. I have quotient 1 and want to see where the run that contains the remainders that are linked to it in the cqf
     * start and ends.
     * I have the start and end of the run of the remainders of the numbers that have that quotient.
     * @param quotient quotient of the numbers where we want to have the associated
     * @return a std::pair with start and end position of the run
     */
    std::pair<uint64_t,uint64_t> get_run_boundaries(uint64_t quotient); //const

    /** it gets the end of the previous run of the selected quotient. It is the equivalent of doing 
     * select(runend_vector, rank(occupieds_vector, quotient))
     * @param quotient quotient to give to the function
     * @return the position of the end of the previous run
     */
    uint64_t get_previous_runend(uint64_t quotient); //const
    uint64_t get_runend2(uint64_t quotient);
    uint64_t get_runend3(uint64_t quotient);

    uint64_t get_runstart(uint64_t quotient);

    
    
    /** Generate a string to represent a block. The 3 first lines are offset, occ and runend metadata. Then the other lines are reminders.
     * @param block_id The block to print.
     * @param bitformat a flag to print reminders as bitvector instead of numbers.
     * @return The block represented as string
     **/
    
    std::string block2string(size_t block_id, bool bit_format = false);

    uint64_t remainder(uint64_t num) const;


    bool verbose; //temporary public
    //private:
    // VALUES

    std::vector<uint64_t> cqf; // uint64_t vector to store the cqf
    
    bool debug;
    uint64_t m_num_bits;    // max number of bits occupied by the cqf to check no memory leaks
    uint64_t quotient_size; // value of q
    uint64_t remainder_size;    // value of r
    uint64_t block_size; // Block size (in machine words (64 bits multiples))
    uint64_t number_blocks; // number of blocks the cqf is divided into
    uint64_t elements_inside;



    uint64_t quotient(uint64_t num) const;
    
    /** For circular CQF, it gives back the runend_word after the one given. If the one given is at the end, 
     * it gives back the one at the beginning of the filter. It also skips the metadata words. It is used for 
     * remainder slot shifting
     * @param current_quot current quotient
     * @return uint64 with position of the one after
     */
    uint64_t get_next_remainder_word(uint64_t current_word) const;

    /** For circular CQF, it gives back the runend_word before the one given. If the one given is 0, 
     * it gives back the one at the end of the filter. It also skips the metadata words. It is used for 
     * remainder slot shifting
     * @param current_quot current quotient
     * @return uint64 with position of the one before
     */
    uint64_t get_prev_remainder_word(uint64_t current_word) const;

    /** For circular CQF, it gives back the quotient after the one given. If the one given is at the end, 
     * it gives back the one at the begining of the filter (0). 
     * @param current_quot current quotient
     * @return uint64 with position of the one after
     */    
    uint64_t get_next_quot(uint64_t current_quot) const;

    /** For circular CQF, it gives back the quotient before the one given. If the one given is 0, 
     * it gives back the one at the end of the filter. 
     * @param current_quot current quotient
     * @return uint64 with position of the one before
     */
    uint64_t get_prev_quot(uint64_t current_quot) const;

    /** For circular CQF, it gives back the block_id (a block is made by the metadata and remainder slots words)
     *  before the one given. If the one given is 0, it gives back the id of the one at the end of the filter. 
     * @param current_quot current quotient
     * @return uint64 with position of the one before
     */  
    uint64_t get_prev_block_id(uint64_t current_block) const;

    /** For circular CQF, it gives back the block_id (a block is made by the metadata and remainder slots words)
     *  after the one given. If the one given is 0, it gives back the id of the one at the end of the filter. 
     * @param current_quot current quotient
     * @return uint64 with position of the one before
     */  
    uint64_t get_next_block_id(uint64_t current_block) const;

    /** It returns the word containing the slice of the runends bitvector corresponing
     * to the requested block. 
     * @param current_block chosen block 
     * @return uint64 with the runend word of the particular block.
     */
    uint64_t get_runend_word(uint64_t current_block) const;

    /** It returns the word containing the slice of the occupieds bitvector corresponing
     * to the requested block. 
     * @param current_block chosen block 
     * @return uint64 with the occupieds word of the particular block.
     */
    uint64_t get_occupied_word(uint64_t current_block) const;

    /** It returns the word containing the slice of the occupieds bitvector corresponing
     * to the requested block.  
     * @param current_block chosen block 
     * @return uint64 with the offset word of the particular block.
     */
    uint64_t get_offset_word(uint64_t current_block) const;

    /** It sets the word containing the slice of the runends bitvector corresponing
     * to the requested block to a chosen value
     * @param current_block chosen block
     * @param value new runend word value
     */
    void set_runend_word(uint64_t current_block, uint64_t value);

    /** It sets the word containing the offset corresponing
     * to the requested block to a chosen value
     * @param current_block chosen block;
     * @param value new offset value;
     * @return uint64 with the runend word of the particular block.
     */
    void set_offset_word(uint64_t current_block, uint64_t value);

    void decrement_offset(uint64_t current_block);

    /** It sets the bit of the occupieds bitvector corresponing to the chosen block and (bit position in the word).
     * e.g. set 5th bit of 3rd block to 1 or 0
     * @param current_block chosen block 
     * @param value has to be 0 or 1 as it sets a bit
     * @param bit_pos the position of the bit in the occupied word 
     * @return uint64 with the runend word of the particular block.
     */
    void set_occupied_bit(uint64_t current_block, uint64_t value, uint64_t bit_pos);

    /** It sets the bit of the runend bitvector corresponing to the chosen block and (bit position in the word).
     * e.g. set 5th bit of 3rd block to 1 or 0
     * @param current_block chosen block 
     * @param value has to be 0 or 1 as it sets a bit
     * @param bit_pos the position of the bit in the occupied word 
     * @return uint64 with the runend word of the particular block.
     */
    void set_runend_bit(uint64_t current_block, uint64_t value ,uint64_t bit_pos);
};  

