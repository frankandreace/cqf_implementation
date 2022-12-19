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

};