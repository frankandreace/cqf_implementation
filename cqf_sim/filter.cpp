#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>     // for std::chrono::high_resolution_clock
#include <string.h>
#include <cassert>
#include <cmath>
#include <stdint.h> 

#include "ext_methods.hpp"
#include "test.hpp"
#include "filter.hpp" 

//FILTER CLASS

// STATIC VARIABLES 
#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL

Cqf::Cqf( uint64_t quotient_s ){
    assert(quotient_s < MEM_UNIT);

    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    uint64_t num_bits_quot = 1ULL << quotient_size;
    number_blocks = std::ceil(num_bits_quot/MEM_UNIT);
    block_size = remainder_size + MET_UNIT;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}

Cqf::Cqf(uint64_t quotient_s, uint64_t n_blocks){
    assert(quotient_s < MEM_UNIT);

    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    block_size = remainder_size + MET_UNIT;
    number_blocks = n_blocks;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}


/*

HIGH LEVEL OPERATIONS



uint64_t Cqf::insert(uint64_t number){
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    // GET FIRST UNUSED SLOT
    uint64_t fu_slot = first_unused_slot(quot);
    
    // IF THE QUOTIENT HAS NEVER BEEN USED BEFORE
    //PUT THE REMAINDER AT THE END OF THE RUN OF THE PREVIOUS USED QUOTIENT OR AT THE POSITION OF THE QUOTIENT
    uint64_t starting_position = sel_rank_filter(get_prev_quot(quot));
    if (starting_position < quot) starting_position = quot;

    if (!is_occupied(quot)){
        update_metadata_insertion(1,starting_position,fu_slot);

        return shift_right_and_set(starting_position, fu_slot, rem);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
    //getting boundaries of the run
        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    //find the place where the remainder should be inserted / all similar to a query
    //getting position where to start shifting right
        starting_position = boundary.first;

        while(starting_position <= boundary.second){
            uint64_t remainder_in_filter = get_remainder(starting_position); 
            if (remainder_in_filter > rem) break;
            starting_position++;
    }

    update_metadata_insertion(0,starting_position,fu_slot);
    // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
    return shift_right_and_set_circ(starting_position, fu_slot, rem);
    }

}


uint64_t Cqf::query(uint64_t number) const{
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (is_occupied(quot) == false) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?

    uint64_t position = boundary.first;

    while(position <= boundary.second){
        uint64_t remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) return 1;
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }

    return 0;

}


uint64_t Cqf::remove(uint64_t number){
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (is_occupied(quot) == false) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?
    uint64_t pos_element = 0;
    uint64_t position = boundary.first;

    // GET POSITION
    while(position <= boundary.second){
        uint64_t remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) {
            pos_element = position;
            break;
        }
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }
    if (pos_element == 0) return 0; //not found

    // GET FIRST UNUSED SLOT
    uint64_t end_slot = first_unused_slot(quot);

    // FIND THE EXACT RIGHTMOST ELEMENT TO SHIFT LEFT
    end_slot = find_rightmost_to_shift_left(position,end_slot);

    // REMOVE ELEMENT BY SHIFTING EVERYTHING LEFT OF ONE REMINDER
    shift_left_and_rem_circ(position,end_slot);

    // MOVE 1 LEFT THE RUNEND OR CLEAR IT WITH OCCUPIED
    //update_metadata_deletion(boundary,position,end_slot);

}

*/