#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>     // for std::chrono::high_resolution_clock
#include <string.h>
#include <cassert>
#include <cmath>
#include <stdint.h>
#include <sstream>
#include <unordered_set>


#include "filter.hpp" 

//FILTER CLASS

// STATIC VARIABLES 
#define MEM_UNIT 64ULL
#define MET_UNIT 3ULL
#define OFF_POS 0ULL
#define OCC_POS 1ULL
#define RUN_POS 2ULL
#define SCALE_INPUT 8388608ULL

using namespace std;

Cqf::Cqf(){
    elements_inside = 0;
}

Cqf::Cqf(uint64_t q_size, uint64_t r_size, bool verbose) : verbose(verbose), debug(false) {
    elements_inside = 0;
    quotient_size = q_size;
    remainder_size = r_size;
    uint64_t num_quots = 1ULL << quotient_size; 
    uint64_t num_of_words = num_quots * (MET_UNIT + remainder_size) / MEM_UNIT; 

    // In machine words
    this->block_size = (3 + this->remainder_size);
    number_blocks = std::ceil(num_quots / MEM_UNIT);
    
    //if (this->verbose) {
    std::cout << "[FS]: QUOTIENT SIZE " << quotient_size << std::endl;
    std::cout << "[FS]: REMAINDER SIZE " << remainder_size << std::endl;
    std::cout << "[FS]: NUMBER OF QUOTIENTS " << num_quots << std::endl;
    //}

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}


Cqf::Cqf(uint64_t max_memory, bool verbose) : verbose(verbose), debug(false) {

    elements_inside = 0;
    
    // Size of the quotient/remainder to fit into max_memory MB
    quotient_size = find_quotient_given_memory(max_memory);
    remainder_size = MEM_UNIT - quotient_size;

    // Number of quotients must be >= MEM_UNIT
    uint64_t num_quots = 1ULL << quotient_size; //524.288
    uint64_t num_of_words = num_quots * (MET_UNIT + remainder_size) / MEM_UNIT; //393.216

    // In machine words
    this->block_size = (3 + this->remainder_size);
    number_blocks = std::ceil(num_quots / MEM_UNIT);
    
    
    //if (this->verbose) {
    std::cout << "[FILTER STATS]" << max_memory << std::endl;
    std::cout << "[FS]: MAX MEMORY " << max_memory << std::endl;
    std::cout << "[FS]: QUOTIENT SIZE " << quotient_size << std::endl;
    std::cout << "[FS]: REMAINDER SIZE " << remainder_size << std::endl;
    std::cout << "[FS]: NUMBER OF QUOTIENTS " << num_quots << std::endl;
    //}

    cqf = std::vector<uint64_t>(num_of_words);
    m_num_bits = num_of_words*MEM_UNIT;
}

std::string Cqf::block2string(size_t block_id, bool bit_format) {
    std::stringstream stream;

    // Init the position in cqf to the first machine word of the block
    uint64_t cqf_position = block_id * this->block_size;
    // --- Offset ---
    stream << "offset : " << this->cqf[cqf_position++] << endl;
    
    // --- Occurences ---
    stream << "occ    : ";
    uint64_t occ_register = this->cqf[cqf_position++];
    for (size_t bit=0 ; bit<MEM_UNIT ; bit++) {
        stream << ((occ_register & 0b1) ? '1' : '0');
        occ_register >>= 1;
        if (bit % 4 == 3)
            stream << "  ";
    } stream << endl;

    // --- Runends ---
    stream << "runends: ";
    uint64_t runs_register = this->cqf[cqf_position++];
    for (size_t bit=0 ; bit<MEM_UNIT ; bit++) {
        stream << ((runs_register & 0b1) ? '1' : '0');
        runs_register >>= 1;
        if (bit % 4 == 3)
            stream << "  ";
    } stream << endl;

    stream << "BLOCK " << block_id << endl;
    
    // --- Remainders ---
    // Read all bits of the block one by one
    uint64_t bit_position = 0;
    bool end_remainder = false;
    for (size_t remainder_id=0 ; remainder_id<MEM_UNIT ; remainder_id++) {
        uint64_t current_remainder = 0;

        // Get the remainder bit by bit
        for (size_t remainder_bit=0 ; remainder_bit<this->remainder_size ; remainder_bit++) {
            // Add one bit to the remainder
            size_t byte_position = bit_position / MEM_UNIT;
            uint64_t bit_value = (this->cqf[cqf_position + byte_position] >> (bit_position % MEM_UNIT)) & 0b1;
            current_remainder += bit_value << remainder_bit;

            // Pretty print the bit format
            if (bit_format) {
                // Beginning of the line
                if (bit_position % MEM_UNIT == 0) {
                    stream << "         ";
                }
                // Remainder separation symbol
                if (end_remainder and remainder_bit == 0)
                    stream << '|';
                // Current bit value
                stream << (bit_value ? '1' : '0');
                // End of the line
                if (bit_position % MEM_UNIT == MEM_UNIT - 1) {
                    stream << endl;
                    end_remainder = false;
                }
                // Separation between bits
                else if (bit_position % 4 == 3) {
                    stream << (end_remainder ? " " : "  ");
                    end_remainder = false;
                }
            }

            // Update the loop
            bit_position += 1;
        }
        // Take into account the end of the remainder for bit pretty printing
        end_remainder = true;

        // Format the string using decimal numbers
        if (not bit_format) {
            if (remainder_id % 4 == 0)
                stream << "         ";
            stream << current_remainder << '\t';
            if (remainder_id % 4 == 3)
                stream << endl;
        }
    }

    return stream.str();
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
    uint64_t quotient_size;
    uint64_t curr_m;
    
    for (int i = MEM_UNIT - 1; i > 0; --i){
        quotient_size = i;

        if (quotient_size >= (MEM_UNIT - MET_UNIT)){
            curr_m = ((1ULL << quotient_size)/SCALE_INPUT) * (MEM_UNIT + MET_UNIT - quotient_size);
            if (max_memory >= curr_m) return quotient_size;
        }
        else{
            curr_m = ((1ULL << quotient_size)) * (MEM_UNIT + MET_UNIT - quotient_size);
            if (max_memory*SCALE_INPUT >= curr_m) return quotient_size;
        }

    }
    return 0;

}

using namespace std;

void Cqf::insert(uint64_t number){
    if (elements_inside == number_blocks*MEM_UNIT) return;


    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    std::cout << "[INSERT] quot " << quot << std::endl;
    std::cout << "[INSERT] rem " << rem << std::endl;


    // GET FIRST UNUSED SLOT
    //if (debug || verbose) {cout << "[INSERT] before first unused slot " << endl;}
    uint64_t fu_slot = first_unused_slot(quot);
    cout << "[INSERT] first unused slot " << fu_slot << endl;
    
    uint64_t starting_position = get_runstart(quot);

    if (!is_occupied(quot)){
        set_occupied_bit(get_block_id(quot), 1, get_shift_in_block(quot));
        shift_bits_left_metadata(quot, 1, starting_position,fu_slot);
        elements_inside++;
        return shift_left_and_set_circ(starting_position, fu_slot, rem);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
        cout << "[INSERT] occupied" << endl;
        //getting boundaries of the run

        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);
        cout << "boundaries " << boundary.first << " " << boundary.second <<endl;

        //find the place where the remainder should be inserted / all similar to a query
        //getting position where to start shifting right
        starting_position = boundary.first;
        

        while(starting_position <= boundary.second){ //bug with this condition ("<=" avant)
            //cout << "loop" << endl;
            uint64_t remainder_in_filter = get_remainder(starting_position); 
            if (remainder_in_filter > rem) {
                break;
            }
            starting_position = get_next_quot(starting_position);
        }
        
        uint64_t metadata_starting_position = boundary.first; //boundary.first;
        //if((metadata_starting_position == boundary.second) && (metadata_starting_position != 0)) metadata_starting_position--;


        shift_bits_left_metadata(quot, 0, metadata_starting_position, fu_slot);
        // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
        elements_inside++;
        cout << "starting_position " << starting_position << endl;
        return shift_left_and_set_circ(starting_position, fu_slot, rem);
    }
}


uint64_t Cqf::query(uint64_t number){
    //std::cout << "elements_inside " << elements_inside << std::endl;
    if (elements_inside == 0) return 0;
    //get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);

    if ((number == 15638732828105257660ULL) || (number == 455783342229697213ULL)) this->debug = true;

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


std::unordered_set<uint64_t> Cqf::enumerate(){
    std::unordered_set<uint64_t> finalSet;
    uint64_t curr_occ;
    
    std::pair<uint64_t, uint64_t> bounds;
    uint64_t cursor;

    uint64_t quotient;
    uint64_t number;

    for(uint block = 0; block < number_blocks; ++block){
        curr_occ = get_occupied_word(block);
        if (curr_occ == 0) continue;

        for (uint64_t i=0; i<MEM_UNIT; i++){
            if (curr_occ & 1ULL){ //occupied
                quotient = block*MEM_UNIT + i;
                bounds = get_run_boundaries(quotient);
                cursor = bounds.first;
                while (cursor != (bounds.second)){
                    number = rebuild_number(quotient, get_remainder(quotient), quotient_size);
                    finalSet.insert(number);
                    cursor = get_next_quot(cursor);
                }
                number = rebuild_number(quotient, get_remainder(quotient), quotient_size);
                finalSet.insert(number); 
            }

            curr_occ >>= 1ULL;
        }
    }


    return finalSet;
}



//
uint64_t Cqf::quotient(uint64_t num) const{
    return num & mask_right(quotient_size);
}

uint64_t Cqf::remainder(uint64_t num) const{
    return num >> (MEM_UNIT - remainder_size);
}

// REMAINDER OPERATIONS

uint64_t Cqf::get_remainder(uint64_t position){

    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((MET_UNIT+remainder_size)*MEM_UNIT)+MET_UNIT*MEM_UNIT+pos_in_block*remainder_size); 

    return get_bits(cqf, pos, remainder_size);
}


void Cqf::set_remainder(uint64_t position, uint64_t value){
    assert(position < number_blocks*MEM_UNIT);

    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((3+remainder_size)*MEM_UNIT)+3*MEM_UNIT+pos_in_block*remainder_size);

    set_bits(cqf, pos, value, remainder_size);
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

    uint64_t to_shift = 0;
    
    // WHILE CURR_WORD != END_WORD
    if ((curr_word_pos != end_word_pos) && (curr_word_shift + remainder_size >= MEM_UNIT)){
        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, remainder_size);
        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);

        next_remainder = to_shift;
        curr_word_shift = remainder_size - (MEM_UNIT - curr_word_shift);
        curr_word_pos = get_next_remainder_word(curr_word_pos);
        //cout << endl << block2string(start_quotient/64, false) << endl << endl;
    }

    while (curr_word_pos != end_word_pos){

        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, MEM_UNIT - curr_word_shift);

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);
        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(MEM_UNIT - curr_word_shift - remainder_size)) , MEM_UNIT - curr_word_shift - remainder_size);
        next_remainder = to_shift >> (MEM_UNIT - curr_word_shift - remainder_size);
        curr_word_shift = 0;
        curr_word_pos = get_next_remainder_word(curr_word_pos);
    }

    //save the bits that are gonna be moved.
    to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, end_word_shift - curr_word_shift);

    //set the new_remainder bits into the word
    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);


    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(end_word_shift - curr_word_shift)), end_word_shift - curr_word_shift);
}


void Cqf::shift_right_and_rem_circ(uint64_t start_quotient,uint64_t end_quotient){
    assert(start_quotient < ( 1ULL << quotient_size));
    assert(end_quotient < ( 1ULL << quotient_size)); 

    uint64_t curr_word_pos = get_remainder_word_position(start_quotient);
    uint64_t curr_word_shift = get_remainder_shift_position(start_quotient);

    uint64_t end_word_pos = get_remainder_word_position(end_quotient);
    uint64_t end_word_shift = get_remainder_shift_position(end_quotient);

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
    assert(cqf[offset_id] >= 0);
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

void Cqf::decrement_offset(uint64_t current_block){
    uint64_t old_offset = get_offset_word(current_block);
    set_offset_word(current_block, (0 == old_offset)? 0 : old_offset-1);
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
    return get_bit_from_word(get_occupied_word(block) ,pos_in_block);
}
bool Cqf::is_runend(uint64_t position){
    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);
    return get_bit_from_word(get_runend_word(block) ,pos_in_block);
}

uint64_t Cqf::first_unused_slot(uint64_t curr_quotient){ //const
    //if ((rend_pos == 0) && (curr_quotient != 0)) return curr_quotient;
    //preventing erroneous stop when it jumps from the end of the filter to the beginning 
    // and curr_quot > rend_pos for the circularity and not beacuse there is free space.
    uint64_t rend_pos = get_runend3(curr_quotient);
    cout << "RUNEND start FUS " << rend_pos << endl;
    uint64_t loop_counter = 0;


    //the conditions are enforced to handle the toricity of the filter as 
    // (current_quotient < runend_position) does not consider the fact that I could move 
    // form the end of the filter to the beginning and the aforemention condition would never be met.

    // the idea is that if I move from the end of the filter to the beginning, 
    // in the block I should see that occ+offset > 0
    while(curr_quotient <= rend_pos){
        curr_quotient = get_next_quot(rend_pos);
        rend_pos = get_runend3(curr_quotient);
        cout << "loop new currquot + runend " << curr_quotient << " + " << rend_pos << endl;
        if (get_shift_in_block(rend_pos) == 0){
            cout << "FUS case rendpos=0" << endl;
            uint64_t curr_block = get_block_id(curr_quotient);
            uint64_t offset = get_offset_word(curr_block);
            return offset == 0 ? curr_quotient : (curr_block*64 + offset + 1);
        }
        loop_counter++;
    }

    return curr_quotient;
}

uint64_t Cqf::get_runend3(uint64_t quotient){
    cout << "getrunend3" << endl;
    uint64_t current_block = get_block_id(quotient);
    uint64_t current_shift = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(current_block);
    
    if (current_shift == 0) { return quotient + offset; }

    uint64_t nb_runs = bitrankasm(get_occupied_word(current_block),
                                  current_shift);

    if (nb_runs == 0) { return (quotient - current_shift + offset); }

    uint64_t pos_after_jump = ((current_block * MEM_UNIT) + offset) % (1ULL << quotient_size);
    current_block = get_block_id(pos_after_jump);
    current_shift = get_shift_in_block(pos_after_jump);

    uint64_t runend_mask = (current_shift == 0) ? mask_right(MEM_UNIT) : mask_right(current_shift);

    uint64_t select_val = bitselectasm(get_runend_word(current_block) & runend_mask, 
                                       nb_runs);


    while (select_val == MEM_UNIT){    
        current_block = get_next_block_id(current_block);
        select_val = bitselectasm(get_runend_word(current_block), nb_runs);
        nb_runs -= bitrankasm(get_runend_word(current_block), MEM_UNIT-1);
    }
    return current_block*MEM_UNIT + select_val;
}

uint64_t Cqf::get_runend2(uint64_t quotient){ //const
    // starting pieces of information required
    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);
    uint64_t runend;


    // rank of starting word
    uint64_t searched_position = bitrankasm(occupied, pos_in_block) + offset;

    while(searched_position == 0){
        //case nothing inserted and we insert at quot 0 (we do next_quot(-1))
        if(block == 0) { return quotient == MEM_UNIT*number_blocks-1 ? -1 : 0; } 
        block = get_prev_block_id(block);
        pos_in_block = MEM_UNIT - 1;
        offset = get_offset_word(block);
        occupied = get_occupied_word(block);
        //runend = get_runend_word(block);

        // rank of starting word
        searched_position = bitrankasm(occupied, pos_in_block) + offset;
    }

    uint64_t ones_rend;
    while(searched_position >= MEM_UNIT){
        runend = get_runend_word(block);
        ones_rend = bitrankasm(runend,MEM_UNIT - 1);
        searched_position -= ones_rend;
        block = get_next_block_id(block);
    }

    // 0 < searched_position < MEM_UNIT
    runend = get_runend_word(block);
    uint64_t select_value = bitselectasm(runend,searched_position);

    while (select_value >= MEM_UNIT){
        ones_rend = bitrankasm(runend, MEM_UNIT - 1);
        searched_position -= ones_rend;
        block = get_next_block_id(block);
        runend = get_runend_word(block);
        select_value = bitselectasm(runend,searched_position);
    }
    return ((block * MEM_UNIT) + select_value); 

}

uint64_t Cqf::get_previous_runend(uint64_t quotient){ //const

    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);


    // --- Get the begining of the previous run ---
    uint64_t position = bitrankasm(occupied, pos_in_block) + offset;

    //std::cout << "SR position " << position << std::endl;

    // --- go back through blocks until I get a number > 0 ---
    while (position == 0){
        // --- for first insertion ---
        if((block == 0) && (position == 0)){
            return 0;
        }
        block = get_prev_block_id(block);

        offset = get_offset_word(block);
        occupied = get_occupied_word(block);

        position = bitrankasm(occupied,MEM_UNIT-1) + offset;
        //std::cout << "SR position " << position << std::endl;
    }

    while (position >= MEM_UNIT){
        position -= bitrankasm(get_runend_word(block),MEM_UNIT-1);
        block = get_next_block_id(block);
    }

    // --- Get the end of the previous run (from the beginning) ---
    uint64_t select = bitselectasm(get_runend_word(block), position);

    while (select == MEM_UNIT){
        block = get_next_block_id(block);
        position -= bitrankasm(get_runend_word(block),MEM_UNIT);
        select = bitselectasm(get_runend_word(block),position);
    }

    return ((block * MEM_UNIT) + select);
}

uint64_t Cqf::get_runstart(uint64_t quotient){
    uint64_t current_block = get_block_id(quotient);
    uint64_t current_shift = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(current_block);

    // === COMPUTE d ===
    uint64_t nb_runs; 
    //nb_runs = d in the paper, number of runs that have to end before ours can start
    if (current_shift == 0){
        nb_runs = 0;
    } else {
        nb_runs = bitrankasm(get_occupied_word(current_block),
                             current_shift-1);
    }

    // === JUMP W/ OFFSET ===
    uint64_t select_val;
    uint64_t pos_after_jump = ((current_block * MEM_UNIT) + offset) % (1ULL << quotient_size);
    // (pos of block[0] + offset) % nbQuotsMax
    current_shift = get_shift_in_block(pos_after_jump);
    uint64_t runend_mask = (current_shift == 0) ? mask_right(MEM_UNIT) : mask_right(current_shift);

    // === FIRST BLOCK WE JUMPED INTO ===
    if (offset < MEM_UNIT){
        // jumped into the same block
        if (nb_runs == 0){
            return pos_after_jump <= quotient ? quotient : get_next_quot(pos_after_jump);
        }

        select_val = bitselectasm(get_runend_word(current_block) & runend_mask, 
                                  nb_runs);
        
        if (select_val < quotient){ return quotient; }
    }
    else {
        // jumped w/ offset into further block
        if (nb_runs == 0){
            return get_next_quot(pos_after_jump);
        }

        current_block = get_block_id(pos_after_jump);
        select_val = bitselectasm(get_runend_word(current_block) & runend_mask, 
                                  nb_runs);
    }

    nb_runs -= bitrankasm(get_runend_word(current_block) & runend_mask, 
                          MEM_UNIT-1);

    // === OTHER BLOCKS TO FIND THE END OF dth RUN === 
    while (select_val == MEM_UNIT){     
            current_block = get_next_block_id(current_block);
            select_val = bitselectasm(get_runend_word(current_block), nb_runs);
            nb_runs -= bitrankasm(get_runend_word(current_block), MEM_UNIT-1);
    }

    return current_block*MEM_UNIT + select_val + 1; 
    //+1 because select_val is the end of the dth run before our quot's one
} 


std::pair<uint64_t,uint64_t> Cqf::get_run_boundaries(uint64_t quotient){ //const
    //SUB OPTI
    std::pair<uint64_t, uint64_t> boundaries;

    boundaries.first = get_runstart(quotient);
    boundaries.second = get_runend3(quotient);

    return boundaries;
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

    //case shift almost all filter
    if ((curr_block == end_block) && (start_pos > end_pos)){
      s_int = bitrankasm(occ_word,curr_pos_in_block);
      e_int = bitrankasm(occ_word,MEM_UNIT - 1);

      for (uint64_t i=s_int; i < e_int; ++i){
          
          sel_occ = bitselectasm(occ_word,i + 1); //i + 1
          sel_rend = (i==0 ? 0 : bitselectasm(rend_word,i)) + offset;
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

        for (uint64_t i=s_int; i <= e_int; ++i){
            
            sel_occ = bitselectasm(occ_word,i + 1); //i + 1
            sel_rend = (i==0 ? 0 : bitselectasm(rend_word,i)) + offset;

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
    // curr_block = end_block
    s_int = bitrankasm(occ_word,curr_pos_in_block);
    e_int = bitrankasm(occ_word,end_pos_in_block);

    for (uint64_t i=s_int; i <= e_int; ++i){
        
        sel_occ = bitselectasm(occ_word,i + 1);
        sel_rend = (i==0 ? 0 : bitselectasm(rend_word,i)) + offset;

        if (sel_occ >= sel_rend){
            return get_quot_from_block_shift(curr_block,sel_rend);
        }
    }
    return end_pos;
}


void Cqf::shift_bits_left_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    cout << "- shift_bits_left_metadata Q " << quotient << " SP " << start_position << " EP " << end_position << endl;

    // METHOD FOR INSERTION
    uint64_t current_block = get_block_id(quotient);
    uint64_t current_shift_in_block = get_shift_in_block(start_position);

    uint64_t start_block = get_block_id(start_position);
    
    uint64_t end_block = get_block_id(end_position);
    uint64_t end_shift_in_block = get_shift_in_block(end_position);
    //(end_position == first unused slot)

    uint64_t overflow_bit = flag_bit;
    uint64_t word_to_shift;
    uint64_t save_right;
    uint64_t to_shift;
    uint64_t next_block;
    uint64_t save_left;

    //OFFSET case quotient is in first slot (TEST_F(CqfTest, offset2))
    if (get_shift_in_block(quotient) == 0) {
        if (!flag_bit){ // quotient occupied, so is the slot (its run or a previous one)
            set_offset_word(current_block, get_offset_word(current_block)+1);
        }
    }

    //OFFSET case everyblock we cross until runstart 
    while (current_block != start_block){
        current_block = get_next_block_id(current_block);
        if (end_position != current_block*64){
            //if end_pos not at block 0 of block (wd mean a remainder at pos 0 => offset still 0)
            set_offset_word(current_block, get_offset_word(current_block)+1);
        }
    } 
    
    // current_block == start_block
    //RUNEND case runstart at the end of filter (shift almost all filter)
    if ((current_block == end_block) && (start_position > end_position)) {

        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift,current_shift_in_block),(current_shift_in_block + 1));

        overflow_bit = shift_left(overflow_bit,current_shift_in_block);
        to_shift |= (save_right | overflow_bit);
        set_runend_word(current_block, to_shift); 


        current_block = get_next_block_id(current_block);
        current_shift_in_block = 0;
    }


    while ( current_block != end_block ) {
        word_to_shift = get_runend_word(current_block);

        save_right = word_to_shift & mask_right(current_shift_in_block);

        to_shift = shift_left(shift_right(word_to_shift,current_shift_in_block),(current_shift_in_block + 1));
        //((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));

        overflow_bit = shift_left(overflow_bit,current_shift_in_block);

        to_shift |= (save_right | overflow_bit);

        set_runend_word(current_block, to_shift); 

        overflow_bit = shift_right(word_to_shift,(MEM_UNIT - 1));
        next_block = get_next_block_id(current_block);

        current_block = next_block;

        //OFFSET case everyblock we cross until runend 
        if (end_position != current_block*64){
            set_offset_word(current_block, get_offset_word(current_block)+1);
        }
        current_shift_in_block = 0;
    }

    word_to_shift = get_runend_word(current_block);
    save_left = (word_to_shift & mask_left(MEM_UNIT-end_shift_in_block));
    save_right = word_to_shift & mask_right(current_shift_in_block);

    to_shift = (((word_to_shift & mask_right(end_shift_in_block)) & mask_left(MEM_UNIT - current_shift_in_block)) << 1);
    to_shift |= ((save_left | shift_left(overflow_bit,current_shift_in_block)) | save_right);
    //((save_left | (overflow_bit << current_shift_in_block)) | save_right);

    set_runend_word(current_block, to_shift); 
}


void Cqf::shift_bits_right_metadata(uint64_t quotient, uint64_t flag_bit, uint64_t start_position, uint64_t end_position){
    // METHOD FOR DELETION
    //Redundancy with what's done in find_boundary_shift_deletion => twice the iteration over blocks,
    //maybe possible in 1 iteration (shift everything until end or runstart at linked quotient)
    uint64_t overflow_bit = flag_bit;

    uint64_t current_block = get_block_id(quotient);
    uint64_t current_shift_in_block = get_shift_in_block(quotient);

    uint64_t start_block = get_block_id(start_position); 
    uint64_t end_block = get_block_id(end_position);
    uint64_t end_shift_in_block = get_shift_in_block(end_position);

    uint64_t word_to_shift;
    uint64_t save_right;
    uint64_t to_shift;
    uint64_t next_block;

    if (current_shift_in_block == 0) {
        decrement_offset(current_block);
    }


    while (current_block != start_block){
        current_block = get_next_block_id(current_block);
        decrement_offset(current_block);
    }
  

    //case shift almost all filter
    if ((current_block == end_block) && (start_position > end_position)){
        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift,(current_shift_in_block + 1)),current_shift_in_block);

        next_block = get_next_block_id(current_block);

        overflow_bit = shift_left((get_runend_word(next_block) & 1ULL),(MEM_UNIT - 1));

        to_shift |= (save_right | overflow_bit);
        set_runend_word(current_block, to_shift);

        overflow_bit >>= (MEM_UNIT - 1);

        current_block = next_block;
        current_shift_in_block = 0;
    }

    while ( current_block != end_block ){

        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift,(current_shift_in_block + 1)),current_shift_in_block);
        //((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));

        next_block = get_next_block_id(current_block);

        overflow_bit = shift_left((get_runend_word(next_block) & 1ULL),(MEM_UNIT - 1));
 
        to_shift |= (save_right | overflow_bit);
        set_runend_word(current_block, to_shift); 


        overflow_bit >>= (MEM_UNIT - 1);

        current_block = next_block;

        decrement_offset(current_block);
        current_shift_in_block = 0;
    }

    word_to_shift = get_runend_word(current_block);
    uint64_t save_left = (word_to_shift & mask_left(MEM_UNIT - end_shift_in_block - 1));
    save_right = word_to_shift & mask_right(current_shift_in_block);


    to_shift = (((word_to_shift & mask_right(end_shift_in_block + 1)) & mask_left(MEM_UNIT - current_shift_in_block )) >> 1);

    to_shift |= (save_left | save_right);


    //to_shift |= (save_left | overflow_bit);
    set_runend_word(current_block, to_shift); 
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


