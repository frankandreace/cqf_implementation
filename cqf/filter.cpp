#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>     // for std::chrono::high_resolution_clock
#include <string.h>
#include <cassert>
#include <cmath>
#include <stdint.h> 


#include "filter.hpp" 

//FILTER CLASS

// STATIC VARIABLES 
#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL
#define SCALE_INPUT 8000000ULL


/*
Cqf::Cqf( uint64_t quotient_s ){
    assert(quotient_s < MEM_UNIT);

    elements_inside = 0;
    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    uint64_t num_bits_quot = 1ULL << quotient_size;
    number_blocks = std::ceil(num_bits_quot/MEM_UNIT);
    block_size = remainder_size + MET_UNIT;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}
*/

Cqf::Cqf(uint64_t quotient_s, uint64_t n_blocks){
    assert(quotient_s < MEM_UNIT);
    
    elements_inside = 0;
    quotient_size = quotient_s;
    remainder_size = MEM_UNIT - quotient_size;
    max_encoded_value = (1ULL << remainder_size) -1;
    block_size = remainder_size + MET_UNIT;
    number_blocks = n_blocks;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}

Cqf::Cqf(uint64_t max_memory){

    elements_inside = 0;
    
    std::cout << "max_memory " << max_memory << std::endl;
    quotient_size = find_quotient_given_memory(max_memory);
    std::cout << "QUOTIENT SIZE " << quotient_size << std::endl;
    remainder_size = MEM_UNIT - quotient_size;
    uint64_t num_bits_quot = 1ULL << quotient_size;
    number_blocks = std::ceil(num_bits_quot/MEM_UNIT);
    block_size = remainder_size + MET_UNIT;
    uint64_t num_of_words = number_blocks * (MEM_UNIT * block_size);

    std::cout << "remainder_size " << remainder_size << std::endl;
    std::cout << "num_bits_quot " << num_bits_quot << std::endl;
    std::cout << "number_blocks " << number_blocks << std::endl;

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}


/*

HIGH LEVEL OPERATIONS

*/
uint64_t Cqf::get_quot_size(){
    return quotient_size;
}

uint64_t Cqf::get_num_el_inserted(){
    return elements_inside;
}

uint64_t Cqf::find_quotient_given_memory(uint64_t max_memory){
    uint64_t quotinet_s;
    uint64_t curr_m;
    
    for (int i = MEM_UNIT - 1; i > 0; --i){
        //std::cout << "i " << i << std::endl;
        quotinet_s = i;
        //std::cout << "quotinet_s " << quotinet_s << std::endl;
        if (quotinet_s >= (MEM_UNIT - MET_UNIT)){
            curr_m = ((1ULL << quotinet_s)/SCALE_INPUT) * (MEM_UNIT + MET_UNIT - quotinet_s);
            //std::cout << "max_memory " << (max_memory*MEM_UNIT) << std::endl;
            //std::cout << "curr_m (/SCALE)" << curr_m << std::endl;
            if (max_memory >= curr_m) return quotinet_s;
        }
        else{
            curr_m = ((1ULL << quotinet_s)) * (MEM_UNIT + MET_UNIT - quotinet_s);
            //std::cout << "max_memory " << (max_memory * SCALE_INPUT) << std::endl;
            //std::cout << "curr_m " << curr_m << std::endl;
            if (max_memory*SCALE_INPUT >= curr_m) return quotinet_s;
        }

    }
    return 0;

}


void Cqf::insert(uint64_t number){

    if (elements_inside == number_blocks*MEM_UNIT) return;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    std::cout << "quot " << quot << std::endl;
    std::cout << "rem " << rem << std::endl;

    // GET FIRST UNUSED SLOT
    uint64_t fu_slot = first_unused_slot(quot);
    
    std::cout << "fu_slot " << fu_slot << std::endl;

    // IF THE QUOTIENT HAS NEVER BEEN USED BEFORE
    //PUT THE REMAINDER AT THE END OF THE RUN OF THE PREVIOUS USED QUOTIENT OR AT THE POSITION OF THE QUOTIENT
    uint64_t starting_position = sel_rank_filter(get_prev_quot(quot));

    std::cout << "sel_rank " << starting_position << std::endl;

    if (starting_position < quot) starting_position = quot;

    std::cout << "starting_position " << starting_position << std::endl;

    if (!is_occupied(quot)){
        std::cout << "not occupied " << std::endl;
        shift_bits_left_metadata(quot, 1,starting_position,fu_slot);
        elements_inside++;
        std::cout << "elements_inside " << elements_inside << std::endl;
        return shift_left_and_set_circ(starting_position, fu_slot, rem);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
    //getting boundaries of the run
        std::cout << "occupied " << std::endl;
        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

        std::cout << "boundary start " << boundary.first << std::endl;
        std::cout << "boundary end " << boundary.second << std::endl;
    //find the place where the remainder should be inserted / all similar to a query
    //getting position where to start shifting right
        starting_position = boundary.first;

        while(starting_position <= boundary.second){
            uint64_t remainder_in_filter = get_remainder(starting_position); 
            if (remainder_in_filter > rem) break;
            starting_position++;
        }
    std::cout << "element should be placed in  " << starting_position << std::endl;

    uint64_t metadata_starting_position = boundary.first;
    //if((metadata_starting_position == boundary.second) && (metadata_starting_position != 0)) metadata_starting_position--;

    std::cout << "metadata_starting_position " << metadata_starting_position << std::endl;    

    shift_bits_left_metadata(quot, 0, metadata_starting_position, fu_slot);
    // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
    elements_inside++;
    std::cout << "elements_inside " << elements_inside << std::endl; 
    return shift_left_and_set_circ(starting_position, fu_slot, rem);
    }

}



uint64_t Cqf::query(uint64_t number){
    //std::cout << "elements_inside " << elements_inside << std::endl;
    if (elements_inside == 0) return 0;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (!is_occupied(quot)) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?

    uint64_t position = boundary.first;

    while(position != boundary.second){
        uint64_t remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) return 1;
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }
    uint64_t remainder_in_filter = get_remainder(boundary.second); 
    if (remainder_in_filter == rem) return 1;
    return 0;

}


uint64_t Cqf::remove(uint64_t number){

    if (elements_inside == 0) return 0;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    //std::cout << "quot " << quot << std::endl;
    //std::cout << "rem " << rem << std::endl;

    if (is_occupied(quot) == false) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);
    
    //std::cout << "boundary.first " << boundary.first << std::endl;
    //std::cout << "boundary.second " << boundary.second << std::endl;

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?
    uint64_t pos_element;
    uint64_t position = boundary.first;
    bool found = false;
    uint64_t remainder_in_filter;

    // GET POSITION
    while(position != boundary.second){
        remainder_in_filter = get_remainder(position); 
        if (remainder_in_filter == rem) {
            pos_element = position;
            found = true;
            //std::cout << "FOUND el at pos " << pos_element << std::endl;
            break;
        }
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }
    remainder_in_filter = get_remainder(boundary.second); 
    if (remainder_in_filter == rem) {
        pos_element = position;
        found = true;
        //std::cout << "FOUND el at pos " << pos_element << std::endl;
        }
    if (!found) return 0; //not found

    // GET FIRST UNUSED SLOT
    uint64_t end_slot = first_unused_slot(quot);

    //std::cout << "fus " << end_slot << std::endl;

    // FIND THE EXACT RIGHTMOST ELEMENT TO SHIFT LEFT
    end_slot = find_boundary_shift_deletion(pos_element,get_prev_quot(end_slot));

    //std::cout << "end_slot_shift " << end_slot << std::endl;

    // REMOVE ELEMENT BY SHIFTING EVERYTHING LEFT OF ONE REMINDER
    //std::cout << "shifting remainders  " << std::endl;
    //std::cout << "from " << pos_element << " to " << end_slot << std::endl;
    shift_right_and_rem_circ(pos_element,end_slot);

    if(boundary.first == boundary.second) {
        // LAST ELEMENT, ZERO OCC & RUNEND
        set_occupied_bit(get_block_id(quot),0,get_shift_in_block(quot));
        set_runend_bit(get_block_id(quot),0,get_shift_in_block(quot));
    }

    else{
        //std::cout << "shifting metadata " << std::endl;
        //std::cout << "from " << pos_element << " to " << end_slot << std::endl;
        shift_bits_right_metadata(quot,0,pos_element,end_slot);
    }

    elements_inside--;
    //std::cout << "elements_inside " << elements_inside << std::endl;
    return 1;
}



//
uint64_t Cqf::quotient(uint64_t num) const{
    return num & mask_right(quotient_size);
}

uint64_t Cqf::remainder(uint64_t num) const{
    return num >> (MEM_UNIT - remainder_size);
}


// INSERT, REMOVE AND QUERY OF THE CQF

void Cqf::cinsert(uint64_t number){

    if (elements_inside == number_blocks*MEM_UNIT) return;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    std::cout << "quot " << quot << std::endl;
    std::cout << "rem " << rem << std::endl;

    // GET FIRST UNUSED SLOT
    uint64_t fu_slot = first_unused_slot(quot);
    
    //std::cout << "fu_slot " << fu_slot << std::endl;

    // IF THE QUOTIENT HAS NEVER BEEN USED BEFORE
    //PUT THE REMAINDER AT THE END OF THE RUN OF THE PREVIOUS USED QUOTIENT OR AT THE POSITION OF THE QUOTIENT
    uint64_t starting_position = sel_rank_filter(get_prev_quot(quot));

    //std::cout << "sel_rank " << starting_position << std::endl;

    if (starting_position < quot) starting_position = quot;

    //std::cout << "starting_position " << starting_position << std::endl;

    if (!is_occupied(quot)){
        //std::cout << "not occupied " << std::endl;
        shift_bits_left_metadata(quot,1,starting_position,fu_slot);
        elements_inside++;
        //std::cout << "elements_inside " << elements_inside << std::endl;
        counter_el run_info;
        run_info.start = starting_position;
        run_info.end = starting_position;
        run_info.count = 0;
        std::cout << "increase counter now " << std::endl;
        return increase_counter(rem,run_info,fu_slot);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
        //getting boundaries of the run
        //std::cout << "occupied " << std::endl;
        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

        //std::cout << "boundary start " << boundary.first << std::endl;
        //std::cout << "boundary end " << boundary.second << std::endl;
        //find the place where the remainder should be inserted / all similar to a query
        //getting position where to start shifting right
        starting_position = boundary.first;

        counter_el run_info = scan_for_elements(boundary.first,boundary.second,rem);
       
        uint64_t metadata_starting_position = run_info.start;
        //if((metadata_starting_position == boundary.second) && (metadata_starting_position != 0)) metadata_starting_position--;

        //std::cout << "metadata_starting_position " << metadata_starting_position << std::endl;    

        shift_bits_left_metadata(quot, 0, metadata_starting_position, fu_slot);
        // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
        elements_inside++;
        //std::cout << "elements_inside " << elements_inside << std::endl; 
        return increase_counter(rem,run_info,fu_slot);
    }

}

uint64_t Cqf::cquery(uint64_t number){
    //std::cout << "elements_inside " << elements_inside << std::endl;
    if (elements_inside == 0) return 0;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if (!is_occupied(quot)) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?
    counter_el run_info = scan_for_elements(boundary.first,boundary.second,rem);
    return run_info.count;
}

// METHOD TO INCREASE THE COUNTER OF A VALUE. IT IS USED AFTER CALLING THE SCAN_COUNTERS
// FUNCTION. 
void Cqf::increase_counter(uint64_t value, counter_el run_info, uint64_t fus){ 
    std::cout << "value: " << value << std::endl;
    std::cout << "rinfo.start: " << run_info.start << std::endl;
    std::cout << "rinfo.end: " << run_info.end << std::endl;
    std::cout << "rinfo.count: " << run_info.count << std::endl;
    std::cout << "fus: " << fus << std::endl;
    if (value == 0){
        if (run_info.count == 0){ // COUNT --> 1  // Y,Z ---> Y,0,Z
            shift_left_and_set_circ(run_info.start,fus,0);
        }
        else if(run_info.count == 1){ // COUNT --> 2  // Y,0,Z ---> Y,0,0,Z
            shift_left_and_set_circ(run_info.start,fus,0);
        }
        else if(run_info.count == 2){ // COUNT --> 3  // Y,0,0,Z ---> Y,0,0,0,Z
            shift_left_and_set_circ(run_info.start,fus,0);
        }
        else if(run_info.count == 3){ // COUNT --> 4  // Y,0,0,0,Z ---> Y,0,1,0,0,Z
            shift_left_and_set_circ(get_next_quot(run_info.start),fus,1);
        }
        else{   // JUST CHECK THAT WHEN THE LAST SLOT OF THE ONES WRAPPED BY THE ZEROS DOES NOT OVERFLOW;
            uint64_t last_encoded_counter_slot = get_prev_quot(get_prev_quot(run_info.end));
            uint64_t value_les = get_remainder(last_encoded_counter_slot);
            if(value_les == (max_encoded_value - 1)) shift_left_and_set_circ(get_next_quot(last_encoded_counter_slot),fus,1);
            else set_remainder(last_encoded_counter_slot,value_les + 1);
        }
    }
    else{ // NOT A ZERO
        if(run_info.count == 0){    // COUNT --> 1 // Y,Z ---> Y,X,Z
            std::cout << "0 to 1" << std::endl;
            shift_left_and_set_circ(run_info.start,fus,value);
        }
        else if(run_info.count == 1){    // COUNT --> 2 // Y,X,Z ---> Y,X,X,Z
            std::cout << "1 to 2" << std::endl;
            shift_left_and_set_circ(run_info.end,fus,value);
        }
        else if(run_info.count == 2){    // COUNT --> 3 // Y,X,X,Z ---> Y,X,1,X,Z
            std::cout << "2 to 3" << std::endl;
            if (value != 1) shift_left_and_set_circ(get_prev_quot(run_info.end),fus,1);
            else {
                shift_left_and_set_circ(get_prev_quot(run_info.end),fus,0);
                shift_left_and_set_circ(run_info.end,get_next_quot(fus),2);
            }

        }
        else{
            std::cout << ">3" << std::endl;
            uint64_t encoded_counter = run_info.count-2;
            std::cout << "encoded_counter: " << encoded_counter << std::endl;
            if (encoded_counter == value - 1){ // ADD THE ZERO AND JUMP OF 2
                std::cout << "add zero and jump" << std::endl;
                shift_left_and_set_circ(get_next_quot(run_info.start),fus,0);
                set_remainder(run_info.end,get_remainder(run_info.end) + 2);
            }
            else if(encoded_counter%max_encoded_value == value - 1){ // JUMP OF 2 WHEN COUNTER_SLOT GETS TO VALUE
                std::cout << "jump" << std::endl;
                set_remainder(run_info.end,get_remainder(run_info.end) + 2);
            }
            else if(encoded_counter == max_encoded_value){ // add new slot to the counter when reaching max_value
                std::cout << "new slot" << std::endl;
                shift_left_and_set_circ(get_prev_quot(run_info.end),fus,1);
            }
            else{// in every othrer case just increase of 1
                std::cout << "just increase" << std::endl;
                 set_remainder(get_prev_quot(run_info.end),get_remainder(get_prev_quot(run_info.end)) + 1);
            }

        }

    }
}

void Cqf::decrease_counter(uint64_t value, counter_el run_info, uint64_t fus){
    
}

// return the start,end and counter of the searched value. If not found, start==end correspond to the position
// where it should be inserted. and counter is 0.
// this method should work for all counter operations (query,insert,remove).
counter_el Cqf::scan_for_elements(uint64_t position, uint64_t end, uint64_t searched_value){
    uint64_t current_rem = get_remainder(position);
    uint64_t last_rem;
    uint64_t value = current_rem;
    uint64_t value_start = position;
    uint64_t next_pos;

    bool reset_count = false;

    counter_el run_info;
    uint64_t counter = 1;

    //std::cout << "reading VALUE " << value << std::endl;
    //std::cout << "at POSITION " << value_start << std::endl;

    while ((position != end)){ // this while works till it finds the searched element or it does not. 
        // adjour next element
        position = get_next_quot(position);
        last_rem = current_rem;
        current_rem = get_remainder(position);
        
        if (current_rem == value){
            // IF VALUE IS ZERO 
            if (value == 0){
                // TWO CONSECUTIVE ZEROS ARE AN ESCAPE SEQUENCE
                if (last_rem == current_rem) {
                    counter++;
                    //std::cout << "VALUE " << value << ", COUNTER " << counter << ", atPOS " << position << std::endl;
                    reset_count = true;
                }
                else{ // IF NEXT ONE IS A ZERO TOO CONTINUE
                    next_pos = get_next_quot(position);
                    if (get_remainder(next_pos) == 0) {
                        counter++;
                        //std::cout << "VALUE " << value << ", COUNTER " << counter << ", atPOS " << position << std::endl;
                        continue;
                    }
                    else{ // THERE WAS JUST 1 ZERO, RESET BACK TO POSITION OF THE ELEMENT AFTER THE 1ST ZERO
                        position = get_next_quot(value_start);
                        counter = 1;
                        reset_count = true;
                        if (value == searched_value){
                            //std::cout << "FOUND " << value << std::endl;
                            run_info.start = value_start;
                            run_info.end = value_start;
                            run_info.count = counter;
                            return run_info;
                        }
                    }
                }
            }
            // IF SLOT IS EQUAL TO REMAINDER
            else{
                // ESCAPE OF VALUE
                counter ++;
                //std::cout << "VALUE " << value << ", COUNTER " << counter << ", atPOS " << position << std::endl;
                reset_count = true;
                //reset_count = true;
            }
        }
        // INCREASE. RESET COUNTER
        else if ((current_rem > last_rem) && (last_rem != 0)){ // this element is a remainder
            reset_count = true;
        }
        else if ((current_rem < last_rem) || ((current_rem > last_rem) && (last_rem == 0))){
            if((current_rem < value) || (value == 0)) counter += current_rem;
            else counter += (current_rem - 1);
            //std::cout << "VALUE " << value << ", COUNTER " << counter << ", atPOS " << position << std::endl;
        }
        // reset everything if reset is set to true
        if (reset_count == true){
            
            if (value == searched_value){ // resetting
                //std::cout << "FOUND " << value << std::endl;
                run_info.start = value_start;
                run_info.end = position;
                run_info.count = counter;
                return run_info;
            } 
            // the slot in which I am is a remainder and not a counter. Adj value, value start, counter and reset_count
            else{
                current_rem = get_remainder(position);
                value = current_rem;
                value_start = position;
                counter = 1;
                reset_count = false;
                //std::cout << "reading VALUE " << value << std::endl;
                //std::cout << "at POSITION " << value_start << std::endl;
            }

            if (value > searched_value) { // the element is not here, return start==end=pos-- (useful for insertions),count=0
                //std::cout << "NOT FOUND " << std::endl;
                run_info.start = position;
                run_info.end = run_info.start;
                run_info.count = 0;
                return run_info;
            }
        }
        
    }

    if (value == searched_value){ // in case there is just 1 el in the run
        //std::cout << "FOUND " << value << std::endl;
        run_info.start = value_start;
        run_info.end = position;
        run_info.count = counter;
        return run_info;
    }
    else{
        //std::cout << "NOT FOUND " << std::endl;
        run_info.start = position;
        run_info.end = run_info.start;
        run_info.count = 0;
        return run_info;
    }
    

}

// REMAINDER OPERATIONS

uint64_t Cqf::get_remainder(uint64_t position){

    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((MET_UNIT+remainder_size)*MEM_UNIT)+MET_UNIT*MEM_UNIT+pos_in_block*remainder_size); 

    return get_bits(cqf, pos, remainder_size);
}

uint64_t Cqf::get_remainder_func(uint64_t position){

    uint64_t block = get_remainder_word_position(position);
    uint64_t pos_in_block = get_remainder_shift_position(position);

    uint64_t pos = block * MEM_UNIT + pos_in_block; 

    return get_bits(cqf, pos, remainder_size);
}


void Cqf::set_remainder(uint64_t position, uint64_t value){
    assert(position < number_blocks*MEM_UNIT);

    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((3+remainder_size)*MEM_UNIT)+3*MEM_UNIT+pos_in_block*remainder_size);

    //std::cout << "block: " << block << std::endl;
    //std::cout << "pos_in_block: " << pos_in_block << std::endl;
    //std::cout << "pos: " << pos << std::endl;
    set_bits(cqf, pos, value, remainder_size);
    //std::cout << "FIN_OUT " << std::endl;
}

uint64_t Cqf::get_remainder_word_position(uint64_t quotient){
    return (get_block_id(quotient) * (MET_UNIT + remainder_size) + MET_UNIT + ((remainder_size * get_shift_in_block(quotient)) / MEM_UNIT)); 
}

uint64_t Cqf::get_remainder_shift_position(uint64_t quotient){
    return (get_shift_in_block(quotient) * remainder_size ) % MEM_UNIT;
}

void Cqf::shift_left_and_set_circ(uint64_t start_quotient,uint64_t end_quotient, uint64_t next_remainder){
    assert(start_quotient < ( 1ULL << quotient_size));
    assert(end_quotient < ( 1ULL << quotient_size)); //( 1ULL << quotient_size));

    uint64_t curr_word_pos = get_remainder_word_position(start_quotient);
    uint64_t curr_word_shift = get_remainder_shift_position(start_quotient);

    uint64_t end_word_pos = get_remainder_word_position(end_quotient);
    uint64_t end_word_shift = get_remainder_shift_position(end_quotient);
    /*
    std::cout << "SLS cwp: " << curr_word_pos << std::endl;
    std::cout << "SLS cws: " << curr_word_shift << std::endl;
    std::cout << "SLS ewp: " << end_word_pos << std::endl;
    std::cout << "SLS ews: " << end_word_shift << std::endl;
    */
    uint64_t to_shift = 0;
    
    // WHILE CURR_WORD != END_WORD
    if ((curr_word_pos != end_word_pos) && (curr_word_shift + remainder_size >= MEM_UNIT)){
        //std::cout << "1st IF " << std::endl;

        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, remainder_size);

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);

        next_remainder = to_shift;
        curr_word_shift = remainder_size - (MEM_UNIT - curr_word_shift);
        curr_word_pos = get_next_remainder_word(curr_word_pos);
    }

    while (curr_word_pos != end_word_pos){
        //std::cout << "WHILE " << std::endl;

        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, MEM_UNIT - curr_word_shift);
        //std::cout << "to_shift: ";
        //print_bits(to_shift);
        //std::cout << "nbits: " << ( MEM_UNIT - curr_word_shift) << std::endl;


        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(MEM_UNIT - curr_word_shift - remainder_size)) , MEM_UNIT - curr_word_shift - remainder_size);
        /*
        std::cout << "SLS to_shift ";
        print_bits(to_shift);
        std::cout << "SLS next_remainder ";
        print_bits(next_remainder);
        */
        next_remainder = to_shift >> (MEM_UNIT - curr_word_shift - remainder_size);
        curr_word_shift = 0;
        curr_word_pos = get_next_remainder_word(curr_word_pos);
        //std::cout << "cwp: " << curr_word_pos << std::endl;
    }
    //std::cout << "END " << std::endl;
    //save the bits that are gonna be moved.
    to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, end_word_shift - curr_word_shift);

    //set the new_remainder bits into the word
    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);


    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(end_word_shift - curr_word_shift)), end_word_shift - curr_word_shift);
    /*
    std::cout << "SLS to_shift ";
    print_bits(to_shift);
    std::cout << "SLS next_remainder ";
    print_bits(next_remainder);
    */
}


void Cqf::shift_right_and_rem_circ(uint64_t start_quotient,uint64_t end_quotient){
    assert(start_quotient < ( 1ULL << quotient_size));
    assert(end_quotient < ( 1ULL << quotient_size)); 

    uint64_t curr_word_pos = get_remainder_word_position(start_quotient);
    uint64_t curr_word_shift = get_remainder_shift_position(start_quotient);

    uint64_t end_word_pos = get_remainder_word_position(end_quotient);
    uint64_t end_word_shift = get_remainder_shift_position(end_quotient);
    /*
    std::cout << "cwp: " << curr_word_pos << std::endl;
    std::cout << "cws: " << curr_word_shift << std::endl;
    std::cout << "ewp: " << end_word_pos << std::endl;
    std::cout << "ews: " << end_word_shift << std::endl;
    */
    uint64_t to_shift;
    uint64_t new_word_pos;
    uint64_t overflow;
    uint64_t new_word_shift;

    // UNTIL CURR_W_POS != END_W_POS
    if ((curr_word_pos != end_word_pos) && (curr_word_shift + remainder_size >= MEM_UNIT)){

        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, remainder_size);

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, to_shift, remainder_size);

        new_word_shift = remainder_size - (MEM_UNIT - curr_word_shift);
        new_word_pos = get_next_remainder_word(curr_word_pos);
        overflow = get_bits(cqf,new_word_pos * MEM_UNIT + new_word_shift, remainder_size);
        set_bits(cqf,curr_word_pos * MEM_UNIT + MEM_UNIT - remainder_size, overflow, remainder_size);

        curr_word_shift = new_word_shift;
        curr_word_pos = new_word_pos;
    }

    while (curr_word_pos != end_word_pos){
        
        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, MEM_UNIT - (curr_word_shift + remainder_size));

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, to_shift, MEM_UNIT - (curr_word_shift + remainder_size));

        //update values for next round
        new_word_shift = 0;    
        new_word_pos = get_next_remainder_word(curr_word_pos);
        overflow = get_bits(cqf,new_word_pos * MEM_UNIT + new_word_shift, remainder_size);
        set_bits(cqf,curr_word_pos * MEM_UNIT + MEM_UNIT - remainder_size, overflow, remainder_size);

        curr_word_shift = new_word_shift;
        curr_word_pos = new_word_pos;

    }

    // WHEN CURR_W_POS == END_W_POS
    to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, end_word_shift - curr_word_shift);

    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, to_shift, end_word_shift - curr_word_shift);
    
    //safety measure, cleaning the last (and so now unused) remainder spot
    set_bits(cqf, end_word_pos * MEM_UNIT + end_word_shift, 0, remainder_size);

}

// CIRCULAR FILTER OPERATIONS

uint64_t Cqf::get_next_remainder_word(uint64_t current_word) const{
    uint64_t current_block = current_word / (MET_UNIT + remainder_size);
    uint64_t pos_in_block = current_word % (MET_UNIT + remainder_size);
    /*
    std::cout << "mu+rm: " << (MET_UNIT + remainder_size) << std::endl;
    std::cout << "cb: " << current_block << std::endl;
    std::cout << "pib: " << pos_in_block << std::endl;
    */
    if (pos_in_block != (MET_UNIT + remainder_size - 1)) return ++current_word;
    else{
        uint64_t next_block = get_next_block_id(current_block);
        return next_block * (MET_UNIT + remainder_size) + (MET_UNIT); 
    }
}

uint64_t Cqf::get_prev_remainder_word(uint64_t current_word) const{
    uint64_t current_block = current_word / (MET_UNIT + remainder_size);
    uint64_t pos_in_block = current_word % (MET_UNIT + remainder_size);

    if (pos_in_block > MET_UNIT) return --current_word;
    else{
        uint64_t prev_block = get_prev_block_id(current_block);
        return prev_block * (MET_UNIT + remainder_size) + (MET_UNIT + remainder_size - 1); 
    }
}


uint64_t Cqf::get_next_quot(uint64_t current_quot) const{
    //std::cout << "IO position " << current_quot << std::endl;
    //std::cout << "IO position " << ++current_quot << std::endl;
    if (current_quot < number_blocks*MEM_UNIT - 1) return ++current_quot;
    else return 0;
}

uint64_t Cqf::get_prev_quot(uint64_t current_quot) const{
    if (current_quot > 0 ) return --current_quot;
    else return number_blocks*MEM_UNIT - 1;
}


uint64_t Cqf::get_prev_block_id(uint64_t current_block) const{
  if (current_block > 0 ) return --current_block;
  else return number_blocks - 1;
}

uint64_t Cqf::get_next_block_id(uint64_t current_block) const{
  if (current_block < number_blocks - 1 ) return ++current_block;
  else return 0;
}

uint64_t Cqf::get_runend_word(uint64_t current_block) const{
  uint64_t runend_id = (current_block *(MET_UNIT+remainder_size)) + RUN_POS;
  return cqf[runend_id];
}

uint64_t Cqf::get_occupied_word(uint64_t current_block) const{
  uint64_t occupied_id = (current_block *(MET_UNIT+remainder_size)) + OCC_POS;
  return cqf[occupied_id];
}

uint64_t Cqf::get_offset_word(uint64_t current_block) const{
  uint64_t offset_id = (current_block *(MET_UNIT+remainder_size)) + OFF_POS;
  return cqf[offset_id];
}

void Cqf::set_runend_word(uint64_t current_block, uint64_t value ){
  uint64_t runend_id = (current_block *(MET_UNIT+remainder_size)) + RUN_POS;
  cqf[runend_id] = value;
}

void Cqf::set_offset_word(uint64_t current_block, uint64_t value ){
  uint64_t offset_id = (current_block *(MET_UNIT+remainder_size)) + OFF_POS;
  cqf[offset_id] = value;
}

void Cqf::set_occupied_bit(uint64_t current_block, uint64_t value ,uint64_t bit_pos){
  uint64_t occupied_id = (current_block *(MET_UNIT+remainder_size)) + OCC_POS;
  uint64_t occ_word = get_occupied_word(current_block);

  value &= mask_right(1ULL);
  value <<= bit_pos;
  uint64_t out_value = ((occ_word & mask_right(bit_pos)) | value);

  out_value |= (occ_word & mask_left(MEM_UNIT-bit_pos-1));
  cqf[occupied_id] = out_value;

}

void Cqf::set_runend_bit(uint64_t current_block, uint64_t value ,uint64_t bit_pos){
  uint64_t runend_id = (current_block *(MET_UNIT+remainder_size)) + RUN_POS;
  uint64_t rend_word = get_runend_word(current_block);

  value &= mask_right(1ULL);
  value <<= bit_pos;
  uint64_t out_value = ((rend_word & mask_right(bit_pos)) | value);

  out_value |= (rend_word & mask_left(MEM_UNIT-bit_pos-1));
  cqf[runend_id] = out_value;

}

//BITVECTOR AND METADATA OPERATIONS

bool Cqf::is_occupied(uint64_t position){
    
    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);
    /*
    std::cout << "IO position " << position << std::endl;
    std::cout << "IO block " << block << std::endl;
    std::cout << "IO pos_in_block " << pos_in_block << std::endl;
    std::cout << "IO occupied_id " << get_occupied_word(block) << std::endl;
    */
    return get_bit_from_word(get_occupied_word(block) ,pos_in_block);
}

uint64_t Cqf::sel_rank_filter(uint64_t quotient) const{
    //assert(quotient < ( 1ULL << quotient_size));

    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);

    //std::cout << "SR block " << block << std::endl;
    //std::cout << "SR pos_in_block " << pos_in_block << std::endl;
    //std::cout << "SR offset " << offset << std::endl;
    //std::cout << "SR occupied " << occupied << std::endl;

    uint64_t position = bitrankasm(occupied,pos_in_block) + offset;

    //std::cout << "SR position " << position << std::endl;
    
    while (position == 0){

        if((block == 0) && (position == 0)){
            return 0;
        }
        block = get_prev_block_id(block);

        offset = get_offset_word(block);
        occupied = get_occupied_word(block);

        position = bitrankasm(occupied,MEM_UNIT-1) + offset;
        //std::cout << "SR position " << position << std::endl;
    }

    uint64_t runends = get_runend_word(block);
    uint64_t select = bitselectasm(runends,position - 1);

    //std::cout << "SR p-1 " << (position - 1) << std::endl;
    //std::cout << "SR runends " << runends << std::endl;
    //std::cout << "SR select " << select << std::endl;


    while (select == MEM_UNIT){

        block = get_next_block_id(block);
        position -= bitrankasm(runends,MEM_UNIT-1);
        select = bitselectasm(get_runend_word(block),position - 1);
        //std::cout << "SR select " << select << std::endl;
        
    }
    //std::cout << "SR FINAL " << (block * MEM_UNIT + select) << std::endl;

    return ((block * MEM_UNIT) + select);
}

std::pair<uint64_t,uint64_t> Cqf::get_run_boundaries(uint64_t quotient) const{
    //assert(quotient < m_num_bits);
    //std::cout << "GRB quotient " << quotient << std::endl;
    std::pair<uint64_t, uint64_t> boundaries;
    // the end of the run is found like this
    uint64_t end_pos = sel_rank_filter(quotient);
    //std::cout << "GRB end_pos " << end_pos << std::endl;
    uint64_t start_pos;

    uint64_t temp = sel_rank_filter(get_prev_quot(quotient));
    start_pos = get_next_quot(temp);
    //std::cout << "GRB start_pos " << start_pos << std::endl;
    //this should work for circular filter
    if (((start_pos <= quotient) && (start_pos < end_pos) && (end_pos >= quotient)) 
    || ((start_pos <= quotient) && (start_pos > end_pos) && (end_pos < quotient)) 
    || ((start_pos >= end_pos) && (start_pos >= quotient))) {
        //std::cout << "GRB IN IF " << std::endl;
        boundaries.first = quotient;
    }
    else boundaries.first = start_pos;
    // it is the position just after the end of the previous

    boundaries.second = end_pos;

    //std::cout << "GRB boundaries.first " << boundaries.first << std::endl;
    //std::cout << "GRB boundaries.second " << boundaries.second << std::endl;

    return boundaries;
}

uint64_t Cqf::first_unused_slot(uint64_t curr_quotient) const{
    //assert(curr_quotient < ( 1ULL << quotient_size));
    uint64_t rend_pos = sel_rank_filter(curr_quotient);
    std::cout << "FUS curr_quotient " << curr_quotient << std::endl;
    std::cout << "FUS out of sel_rank " << rend_pos << std::endl;
    //if ((rend_pos == 0) && (curr_quotient != 0)) return curr_quotient;
    //preventing erroneous stop when it jumps from the end of the filter to the beginning 
    // and curr_quot > rend_pos for the circularity and not beacuse there is free space.
    uint64_t block = get_block_id(curr_quotient);
    uint64_t pos_in_block = get_shift_in_block(curr_quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);

    // why ((bitrankasm(occupied,pos_in_block) + offset) != 0) ?
    while((curr_quotient <= rend_pos) || ((get_block_id(curr_quotient) > get_block_id(rend_pos)) && ((bitrankasm(occupied,get_shift_in_block(curr_quotient)) + offset) != 0))){
        curr_quotient = get_next_quot(rend_pos);
        rend_pos = sel_rank_filter(curr_quotient);
        //std::cout << "FUS IN WHILE curr_q " << curr_quotient << std::endl;
        //std::cout << "FUS IN WHILE rend_pos " << rend_pos << std::endl;

        //std::cout << "FUS IN WHILE get_block_id(curr_quotient) " << get_block_id(curr_quotient) << std::endl;
        //std::cout << "FUS IN WHILE get_block_id(rend_pos) " << get_block_id(rend_pos) << std::endl;
        //std::cout << "FUS IN WHILE (bitrankasm(occupied,pos_in_block) + offset) " << (bitrankasm(occupied,pos_in_block) + offset) << std::endl;
    }
    std::cout << "FUS curr_quotient " << curr_quotient << std::endl;
    return curr_quotient;
}

uint64_t Cqf::find_boundary_shift_deletion(uint64_t start_pos, uint64_t end_pos) const{
    //assert(start_pos < ( 1ULL << quotient_size));
    //assert(end_pos < ( 1ULL << quotient_size)); 

    uint64_t curr_block = get_block_id(start_pos);
    uint64_t curr_pos_in_block = get_shift_in_block(start_pos);

    uint64_t end_block = get_block_id(end_pos);
    uint64_t end_pos_in_block = get_shift_in_block(end_pos);
    /*
    std::cout << "FB curr_block " << curr_block << std::endl;
    std::cout << "FB curr_pos_in_block " << curr_pos_in_block << std::endl;
    std::cout << "FB end_block " << end_block << std::endl;
    std::cout << "FB end_pos_in_block " << end_pos_in_block << std::endl;
    */
    // #1 get occupieds, offset and runend word
    uint64_t rend_word = get_runend_word(curr_block);
    uint64_t occ_word = get_occupied_word(curr_block);
    uint64_t offset = get_offset_word(curr_block);


    uint64_t sel_occ;
    uint64_t sel_rend;
    uint64_t s_int; // start of the interval for select
    uint64_t e_int; //end of the interval for select

    if ((curr_block == end_block) && (start_pos > end_pos)){
      s_int = bitrankasm(occ_word,curr_pos_in_block);
      e_int = bitrankasm(occ_word,MEM_UNIT - 1);

      for (uint64_t i=s_int; i < e_int; ++i){
          
          sel_occ = bitselectasm(occ_word,i); //i + 1
          sel_rend = bitselectasm(rend_word,i + offset - 1);
          if (sel_occ > sel_rend){
              return get_quot_from_block_shift(curr_block,sel_rend);
          }
      }
      // adjourning block and words for next search (in or outside loop)
      curr_pos_in_block = 0;
      curr_block = get_next_block_id(curr_block);
      rend_word = get_runend_word(curr_block);
      occ_word = get_occupied_word(curr_block);
      offset = get_offset_word(curr_block);
    }

    while (curr_block != end_block)
    {   
        s_int = bitrankasm(occ_word,curr_pos_in_block);
        e_int = bitrankasm(occ_word,MEM_UNIT - 1);
        /*
        std::cout << "FB-W occ_word  ";
        print_bits(occ_word);
        std::cout << "FB-W rend_word ";
        print_bits(rend_word);
        std::cout << "FB offset " << offset << std::endl;

        std::cout << "FB-W s_int " << s_int << std::endl;
        std::cout << "FB-W e_int " << e_int << std::endl;
        */
        for (uint64_t i=s_int; i <= e_int; ++i){
            
            sel_occ = bitselectasm(occ_word,i); //i + 1
            sel_rend = bitselectasm(rend_word,i + offset - 1);
            /*
            std::cout << "FB-WF occ_word  ";
            print_bits(occ_word);
            std::cout << "FB-WF rend_word ";
            print_bits(rend_word);
            std::cout << "FB-WF sel_occ " << sel_occ << std::endl;
            std::cout << "FB-WF sel_rend " << sel_rend << std::endl;
            */
            if (sel_occ > sel_rend){
                return get_quot_from_block_shift(curr_block,sel_rend);
            }
        }
        // adjourning block and words for next search (in or outside loop)
        curr_pos_in_block = 0;
        curr_block = get_next_block_id(curr_block);
        rend_word = get_runend_word(curr_block);
        occ_word = get_occupied_word(curr_block);
        offset = get_offset_word(curr_block);

        //std::cout << "FB-W curr_block " << curr_block << std::endl;
        //std::cout << "FB-W offset " << offset << std::endl;
    }
    // curr_block = end_block
    s_int = bitrankasm(occ_word,curr_pos_in_block);
    e_int = bitrankasm(occ_word,end_pos_in_block);

    /*
    std::cout << "FB-E occ_word ";
    print_bits(occ_word);
    std::cout << "FB-E s_int " << s_int << std::endl;
    std::cout << "FB-E e_int " << e_int << std::endl;
    */
    for (uint64_t i=s_int; i <= e_int; ++i){
        
        sel_occ = bitselectasm(occ_word,i);
        sel_rend = bitselectasm(rend_word,i+offset-1);
        /*
        std::cout << "FB-E occ_word  ";
        print_bits(occ_word);
        std::cout << "FB-E rend_word ";
        print_bits(rend_word);
        
        std::cout << "FB-E sel_occ " << sel_occ << std::endl;
        std::cout << "FB-E sel_rend " << sel_rend << std::endl;
        */
        if (sel_occ >= sel_rend){
            return get_quot_from_block_shift(curr_block,sel_rend);
        }
    }
    return end_pos;
}


void Cqf::shift_bits_left_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    // METHOD FOR INSERTION
  
  uint64_t overflow_bit = flag_bit;
  uint64_t current_block = get_block_id(start_position);
  uint64_t current_shift_in_block = get_shift_in_block(start_position);
  uint64_t end_block = get_block_id(end_position);
  uint64_t end_shift_in_block = get_shift_in_block(end_position);

  /*
  std::cout << "SBLM quotient " << quotient << std::endl; 
  std::cout << "SBLM flag_bit " << flag_bit << std::endl; 
  std::cout << "SBLM start_position " << start_position << std::endl; 
  std::cout << "SBLM end_position " << end_position << std::endl; 

  std::cout << "SBLM overflow_bit " << overflow_bit << std::endl; 
  std::cout << "SBLM current_block " << current_block << std::endl; 
  std::cout << "SBLM current_shift_in_block " << current_shift_in_block << std::endl; 
  std::cout << "SBLM end_block " << end_block << std::endl;
  std::cout << "SBLM end_shift_in_block " << end_shift_in_block << std::endl;
  */

  uint64_t word_to_shift;
  uint64_t save_right;
  uint64_t to_shift;
  uint64_t next_block;
  uint64_t old_offset;
  uint64_t save_left;
  
  // IF FLAG == 1: I HAVE TO SET THE OCCUPIED BIT OF THE QUOTIENT TO 1
  if (flag_bit == 1){
    uint64_t quot_block = get_block_id(quotient);
    uint64_t quot_shift_in_block = get_shift_in_block(quotient);
    set_occupied_bit(quot_block,flag_bit,quot_shift_in_block);
  }

  if ((current_block == end_block) && (start_position > end_position)){

    word_to_shift = get_runend_word(current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    
    overflow_bit <<= current_shift_in_block;
    to_shift |= (save_right | overflow_bit);
    set_runend_word(current_block, to_shift); 

    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    next_block = get_next_block_id(current_block);

    if (overflow_bit == 1){
        old_offset = get_offset_word(current_block);

        if ((bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) && (old_offset > 0)) set_offset_word(current_block, old_offset - 1);
        set_offset_word(next_block, get_offset_word(next_block) + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
    
    // IF OVERFLOW BIT IS 1, OFFSET OF THE NEXT BLOCK SHOULD BE INCREASED
    // AND OFFSET OF THIS WORD SHOULD BE DECREASED IF THE BIT WAS PART OF THAT

  }

  while ( current_block != end_block ){

    word_to_shift = get_runend_word(current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));
    
    overflow_bit <<= current_shift_in_block;
    to_shift |= (save_right | overflow_bit);
    set_runend_word(current_block, to_shift); 

    overflow_bit = word_to_shift >> (MEM_UNIT - 1);
    next_block = get_next_block_id(current_block);

    if (overflow_bit == 1){
        old_offset = get_offset_word(current_block);

        if ((bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) && (old_offset > 0)) set_offset_word(current_block, old_offset - 1);
        set_offset_word(next_block, get_offset_word(next_block) + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
  }

  word_to_shift = get_runend_word(current_block);
  save_left = (word_to_shift & mask_left(MEM_UNIT-end_shift_in_block));
  save_right = word_to_shift & mask_right(current_shift_in_block);

  to_shift = (((word_to_shift & mask_right(end_shift_in_block)) & mask_left(MEM_UNIT - current_shift_in_block)) << 1);
  to_shift |= ((save_left | (overflow_bit << current_shift_in_block)) | save_right);
  /*
  std::cout << "SBLM word_to_shift ";
  print_bits(word_to_shift);
  std::cout << "SBLM save_left ";
  print_bits(save_left);
  std::cout << "SBLM save_right ";
  print_bits(save_right);
  std::cout << "SBLM to_shift ";
  print_bits(to_shift);
  */

  set_runend_word(current_block, to_shift); 

}


void Cqf::shift_bits_right_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    // METHOD FOR DELETION

  uint64_t overflow_bit = flag_bit;
  uint64_t current_block = get_block_id(start_position);
  uint64_t current_shift_in_block = get_shift_in_block(start_position);
  uint64_t end_block = get_block_id(end_position);
  uint64_t end_shift_in_block = get_shift_in_block(end_position);
  /*
  std::cout << "SBRM overflow_bit " << overflow_bit << std::endl; 
  std::cout << "SBRM current_block " << current_block << std::endl; 
  std::cout << "SBRM current_shift_in_block " << current_shift_in_block << std::endl; 
  std::cout << "SBRM end_block " << end_block << std::endl;
  std::cout << "SBRM end_shift_in_block " << end_shift_in_block << std::endl;
  */
  uint64_t word_to_shift;
  uint64_t save_right;
  uint64_t to_shift;
  uint64_t next_block;
  uint64_t new_offset;
  uint64_t old_offset;
  

  if ((current_block == end_block) && (start_position > end_position)){

    word_to_shift = get_runend_word(current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));
    //std::cout << "SAVE  ";
    //print_bits(save_right);
    //std::cout << "SHIFT ";
    //print_bits(to_shift);
    next_block = get_next_block_id(current_block);
    
    overflow_bit = ((get_runend_word(next_block) & 1ULL) << (MEM_UNIT - 1));
    //std::cout << "OB    ";
    //print_bits(overflow_bit);
    to_shift |= (save_right | overflow_bit);
    set_runend_word(current_block, to_shift);
    //std::cout << "FIN   ";
    //print_bits(to_shift);

    overflow_bit >>= (MEM_UNIT - 1);

    new_offset = get_offset_word(next_block);
    if ((overflow_bit == 1) && (new_offset > 0)){
        set_offset_word(next_block, new_offset - 1);

        old_offset = get_offset_word(current_block);
        if (bitrankasm(word_to_shift,MEM_UNIT-1) == old_offset) set_offset_word(current_block, old_offset + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
    
    // IF OVERFLOW BIT IS 1, OFFSET OF THE NEXT BLOCK SHOULD BE INCREASED
    // AND OFFSET OF THIS WORD SHOULD BE DECREASED IF THE BIT WAS PART OF THAT

  }

  while ( current_block != end_block ){

    word_to_shift = get_runend_word(current_block);
    save_right = word_to_shift & mask_right(current_shift_in_block);
    to_shift = ((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));

    /*
    std::cout << "SBRM mask_right ";
    print_bits(mask_right(current_shift_in_block));  
    std::cout << "SBRM word_to_shift ";
    print_bits(word_to_shift);
    std::cout << "SBRM save_right ";
    print_bits(save_right);
    std::cout << "SBRM to_shift ";
    print_bits(to_shift);
    */
    next_block = get_next_block_id(current_block);
    
    overflow_bit = ((get_runend_word(next_block) & 1ULL) << (MEM_UNIT - 1));
    //std::cout << "OB    ";
    //print_bits(overflow_bit);
    to_shift |= (save_right | overflow_bit);
    set_runend_word(current_block, to_shift); 
    //std::cout << "FIN   ";
    //print_bits(to_shift);

    overflow_bit >>= (MEM_UNIT - 1);

    new_offset = get_offset_word(next_block);
    if ((overflow_bit == 1) && (new_offset > 0)){
        set_offset_word(next_block, new_offset - 1);

        old_offset = get_offset_word(current_block);
        if ((bitrankasm(word_to_shift,MEM_UNIT-1) == (bitrankasm(get_occupied_word(current_block),MEM_UNIT - 1) + old_offset))) set_offset_word(current_block, old_offset + 1);
    }
    current_block = next_block;
    current_shift_in_block = 0;
  }

  word_to_shift = get_runend_word(current_block);
  uint64_t save_left = (word_to_shift & mask_left(MEM_UNIT - end_shift_in_block - 1));
  save_right = word_to_shift & mask_right(current_shift_in_block);
  /*
  std::cout << "SBRM word_to_shift ";
  print_bits(word_to_shift);
  std::cout << "SBRM mask_right ";
  print_bits(mask_right(current_shift_in_block));  
  std::cout << "SBRM mask_left  ";
  print_bits(mask_left(MEM_UNIT - end_shift_in_block - 1));  
  std::cout << "SBRM save_right ";
  print_bits(save_right);
  std::cout << "SBRM save_left ";
  print_bits(save_left);
  */

  to_shift = (((word_to_shift & mask_right(end_shift_in_block + 1)) & mask_left(MEM_UNIT - current_shift_in_block )) >> 1);
  /*
  std::cout << "SBRM mask_right ";
  print_bits(mask_right(end_shift_in_block + 1));  
  std::cout << "SBRM mask_left  ";
  print_bits(mask_left(MEM_UNIT - current_shift_in_block));  
  std::cout << "SBRM to_shift ";
  print_bits(to_shift);
  */
  to_shift |= (save_left | save_right);

  //std::cout << "SBRM to_shift ";
  //print_bits(to_shift);
  //to_shift |= (save_left | overflow_bit);
  set_runend_word(current_block, to_shift); 
  //std::cout << "FIN   ";
  //print_bits(to_shift);
}

void Cqf::show() const{
    for(uint i = 0; i < number_blocks; ++i){
        std::cout << get_offset_word(i) << std::endl;
        print_bits(get_occupied_word(i));
        print_bits(get_runend_word(i));
        std::cout << std::endl;
        for(uint j = 0; j < remainder_size; ++j){
            print_bits(cqf[i*(MET_UNIT + remainder_size) + MET_UNIT + j]);
        }
        std::cout << std::endl << std::endl;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
}

void Cqf::show_slice(uint64_t start, uint64_t end) const{
    std::cout << "start" << start << std::endl;
    std::cout << "end" << end << std::endl;
    for(uint i = start; i < end; ++i){
        std::cout << "start" << start << std::endl;
        std::cout << "end" << end << std::endl;
        std::cout << "i" << i << std::endl;
        std::cout << get_offset_word(i) << std::endl;
        print_bits(get_occupied_word(i));
        print_bits(get_runend_word(i));
        std::cout << std::endl;
        for(uint j = 0; j < remainder_size; ++j){
            print_bits(cqf[i*(MET_UNIT + remainder_size) + MET_UNIT + j]);
        }
        std::cout << std::endl << std::endl;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
}


