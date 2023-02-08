#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>     // for std::chrono::high_resolution_clock
#include <string.h>
#include <cassert>
#include <cmath>
#include <stdint.h>
#include <sstream>


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
    //std::cout << "[INSERT] inserting " << number << " |q/r " << quot << " " << rem << std::endl;


    if (debug) {
        cout << "[INSERT] debug: q/r " << quot << " " << rem << endl;
    }


    std::cout << "[INSERT] quot " << quot << std::endl;
    std::cout << "[INSERT] rem " << rem << std::endl;


    // GET FIRST UNUSED SLOT
    //if (debug || verbose) {cout << "[INSERT] before first unused slot " << endl;}
    uint64_t fu_slot = first_unused_slot(quot);
    cout << "[INSERT] first unused slot " << fu_slot << endl;
    
    if (debug || verbose)  {cout << "[INSERT] first unused slot " << fu_slot << endl;}
    // IF THE QUOTIENT HAS NEVER BEEN USED BEFORE
    //PUT THE REMAINDER AT THE END OF THE RUN OF THE PREVIOUS USED QUOTIENT OR AT THE POSITION OF THE QUOTIENT
    uint64_t starting_position = get_next_quot(get_previous_runend2(get_prev_quot(quot)));
    if (debug or verbose)
        cout << "[INSERT] starting_position " << starting_position << endl;

    if (starting_position < quot) starting_position = quot;
    //comment gérer le cas 0, savoir si starting_position > 0 car c'est shifted
    // ou starting_position > 0 car on a fait un tour dans le filter ?

    if (this->verbose) {
        std::cout << "[INSERT] starting_position " << starting_position << std::endl;
    }

    if (!is_occupied(quot)){
        if (this->verbose or debug) {
            std::cout << "[INSERT] not occupied " << std::endl;
            std::cout << "[INSERT] SHIFT METADATA starting_position: " << starting_position << " | end_position " << fu_slot << std::endl;
        }

        shift_bits_left_metadata(quot, 1,starting_position,fu_slot);
        elements_inside++;
        if (this->verbose) {
            std::cout << "[INSERT] elements_inside " << elements_inside << std::endl;
            std::cout << "[INSERT] shifting " << std::endl;
            std::cout << "[INSERT] SHIFT METADATA starting_position: " << starting_position << " | end_position " << fu_slot << std::endl;
        }
        return shift_left_and_set_circ(starting_position, fu_slot, rem);
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else{
        cout << "[INSERT] occupied" << endl;
        //getting boundaries of the run
        if (this->verbose) {
            std::cout << "[INSERT] occupied " << std::endl;
        }
        std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);

        if (this->verbose) {
            std::cout << "[INSERT] boundary start " << boundary.first << std::endl;
            std::cout << "[INSERT] boundary end " << boundary.second << std::endl;
        }
        //find the place where the remainder should be inserted / all similar to a query
        //getting position where to start shifting right
        starting_position = boundary.first;

        while(starting_position <= boundary.second){
            if (this->verbose) std::cout << "[INSERT] looping trough  " << starting_position << std::endl;
            uint64_t remainder_in_filter = get_remainder(starting_position); 
            if (remainder_in_filter > rem) {
                if (this->verbose) std::cout << "[INSERT] looping trough, rem_filter > rem  " << starting_position << std::endl;
                break;
            }
            starting_position++;
        }
        
        if (this->verbose) std::cout << "[INSERT] element should be placed in  " << starting_position << std::endl;

        uint64_t metadata_starting_position = boundary.first; //boundary.first;
        //if((metadata_starting_position == boundary.second) && (metadata_starting_position != 0)) metadata_starting_position--;

        if (this->verbose) {
            std::cout << "[INSERT] shift_bits_left_metadata " << quot << " " << 0 << " " << metadata_starting_position << " " << fu_slot << " " << std::endl;
        }    

        shift_bits_left_metadata(quot, 0, metadata_starting_position, fu_slot);
        // SHIFT EVERYTHING RIGHT AND INSERTING THE NEW REMINDER
        elements_inside++;
        if (this->verbose) {
            std::cout << "[INSERT] elements_inside " << elements_inside << std::endl;
            std::cout << "[INSERT] SHIFTING REMAINDERS FROM " << starting_position << " to " << fu_slot << std::endl;
            std::cout << "[INSERT] rem to insert " << rem << std::endl;
        }
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

    if (debug) {
        cout << "debug: q/r " << quot << " " << rem << endl;
    }

    if (!is_occupied(quot)) return 0;

    std::pair<uint64_t,uint64_t> boundary = get_run_boundaries(quot);
    if (debug)
        cout << "boundaries " << boundary.first << " " << boundary.second << endl;

    // TODO:
    // OPTIMIZE TO LOG LOG SEARCH ?

    uint64_t position = boundary.first;

    while(position != boundary.second){
        if (debug)
            cout << position << " ";
        uint64_t remainder_in_filter = get_remainder(position);
        if (debug)
            cout << "remainder_in_filter " << remainder_in_filter << endl;
        if (remainder_in_filter == rem) return 1;
        else if (remainder_in_filter > rem) return 0;
        position = get_next_quot(position);
    }
    uint64_t remainder_in_filter = get_remainder(boundary.second); 
    if (debug)
        cout << "remainder_in_filter " << remainder_in_filter << endl;
    this->debug = false;

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

// REMAINDER OPERATIONS

uint64_t Cqf::get_remainder(uint64_t position){

    uint64_t block = get_block_id(position);
    uint64_t pos_in_block = get_shift_in_block(position);

    uint64_t pos = (block*((MET_UNIT+remainder_size)*MEM_UNIT)+MET_UNIT*MEM_UNIT+pos_in_block*remainder_size); 

    return get_bits(cqf, pos, remainder_size);
}

/*
uint64_t Cqf::get_remainder_func(uint64_t position){

    uint64_t block = get_remainder_word_position(position);
    uint64_t pos_in_block = get_remainder_shift_position(position);

    uint64_t pos = block * MEM_UNIT + pos_in_block; 

    return get_bits(cqf, pos, remainder_size);
}
*/

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

    if(this->verbose){
        std::cout << "SLS remainder to insert: " << next_remainder << std::endl;

        std::cout << "SLS current word position: " << curr_word_pos << std::endl;
        std::cout << "SLS current word shift: " << curr_word_shift << std::endl;
        std::cout << "SLS end word position: " << end_word_pos << std::endl;
        std::cout << "SLS end word shift: " << end_word_shift << std::endl;
    }
    uint64_t to_shift = 0;
    
    // WHILE CURR_WORD != END_WORD
    if ((curr_word_pos != end_word_pos) && (curr_word_shift + remainder_size >= MEM_UNIT)){
        if(this->verbose){
            std::cout << "SLS 1st IF " << std::endl;
        }
        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, remainder_size);
        if(this->verbose){
            std::cout << "SLS 1st IF to_shift " << to_shift << std::endl;
            print_bits(to_shift);
        }
        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);
        if(this->verbose){
            std::cout << "SLS 1st IF setbits " << next_remainder << std::endl;
            print_bits(next_remainder);
            cout << endl << block2string(start_quotient/64, false) << endl << endl;
        }
        next_remainder = to_shift;
        curr_word_shift = remainder_size - (MEM_UNIT - curr_word_shift);
        curr_word_pos = get_next_remainder_word(curr_word_pos);
        //cout << endl << block2string(start_quotient/64, false) << endl << endl;
    }

    while (curr_word_pos != end_word_pos){
        if(this->verbose == true){
            std::cout << "WHILE " << std::endl;
        }
        to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, MEM_UNIT - curr_word_shift);
        if(this->verbose == true){
            std::cout << "to_shift: ";
            print_bits(to_shift);
            std::cout << "nbits: " << ( MEM_UNIT - curr_word_shift) << std::endl;
        }

        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);
        if(this->verbose){
            std::cout << "SLS setbits next rem" << next_remainder << std::endl;
            print_bits(next_remainder);
        }
        set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(MEM_UNIT - curr_word_shift - remainder_size)) , MEM_UNIT - curr_word_shift - remainder_size);
        if(this->verbose){
            std::cout << "SLS 2nd set bits ";
            print_bits((to_shift & mask_right(MEM_UNIT - curr_word_shift - remainder_size)));
            std::cout << "SLS 2nd set bits ";
            print_bits((to_shift & mask_right(MEM_UNIT - curr_word_shift - remainder_size)));
        }
        next_remainder = to_shift >> (MEM_UNIT - curr_word_shift - remainder_size);
        curr_word_shift = 0;
        curr_word_pos = get_next_remainder_word(curr_word_pos);
        if(this->verbose){
            std::cout << "SLS current word position: " << curr_word_pos << std::endl;
            cout << endl << block2string(start_quotient/64, false) << endl << endl;
        }
        //cout << endl << block2string(start_quotient/64, false) << endl << endl;
    }
    if(this->verbose){
        std::cout << "SLS SHIFT END " << std::endl;
        cout << endl << block2string(start_quotient/64, false) << endl << endl;
    }
    //cout << endl << block2string(start_quotient/64, false) << endl << endl;
    //save the bits that are gonna be moved.
    to_shift = get_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, end_word_shift - curr_word_shift);

    //set the new_remainder bits into the word
    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift, next_remainder, remainder_size);


    set_bits(cqf, curr_word_pos * MEM_UNIT + curr_word_shift + remainder_size, (to_shift & mask_right(end_word_shift - curr_word_shift)), end_word_shift - curr_word_shift);
    //cout << endl << block2string(start_quotient/64, false) << endl << endl;
    if(this->verbose){
        std::cout << "SLS to_shift ";
        print_bits(to_shift);
        std::cout << "SLS next_remainder ";
        print_bits(next_remainder);
    }

    if(this->verbose){
        std::cout << "END OF SHIFTING" << std::endl;
        cout << endl << block2string(start_quotient/64, false) << endl << endl;
    }

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

uint64_t Cqf::get_previous_runend3(uint64_t quotient){ //const
    if (debug || verbose)
        cout << "- get_previous_runend3 " << quotient << endl;

    // starting pieces of information required
    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);
    uint64_t runend;


    // the RSQF uses RANK to count the number d of occupied slots between slots i and j
    uint64_t d = bitrankasm(occupied, pos_in_block);
    //uint64_t searched_position = bitrankasm(occupied, pos_in_block) + offset;
    if (verbose || debug) std::cout << "[SR]: d " << (d) << ";" << std::endl;

    while(d == 0){
        if(block == 0) return 0;
        block = get_prev_block_id(block);
        pos_in_block = MEM_UNIT - 1;
        offset = get_offset_word(block);
        occupied = get_occupied_word(block);
        //runend = get_runend_word(block);

        // rank of starting word
        d = bitrankasm(occupied, pos_in_block) + offset;

        if (verbose || debug) std::cout << "[SR]: (WHILE-SP=0) d " << (d) << ";" << std::endl;
    }

    uint64_t ones_rend;
    while(d >= MEM_UNIT){
        runend = get_runend_word(block);
        ones_rend = bitrankasm(runend,MEM_UNIT - 1);
        d -= ones_rend;
        block = get_next_block_id(block);
        //offset = get_offset_word(block);
        //occupied = get_occupied_word(block);

        if (verbose || debug) std::cout << "[SR]: (WHILE-SP>=64) d " << (d) << ";" << std::endl;
    }

    // 0 < d < MEM_UNIT
    runend = get_runend_word(block);
    uint64_t select_value = bitselectasm(runend,d - 1);
    if (verbose || debug) std::cout << "[SR]: SELECT_VALUE " << (select_value) << ";" << std::endl;

    while (select_value >= MEM_UNIT){
        ones_rend = bitrankasm(runend, MEM_UNIT - 1);
        d -= ones_rend;
        block = get_next_block_id(block);
        runend = get_runend_word(block);
        select_value = bitselectasm(runend,d - 1);
        if (verbose || debug) std::cout << "[SR]: BLOCK " << (block) << " | SELECT_VALUE " << select_value << ";" << std::endl;
    }
    if (verbose || debug) std::cout << "[SR]: SELECT_VALUE " << (select_value) << ";" << std::endl;
    if (verbose || debug) std::cout << "[SR]: RETURNED_VALUE " << ((block * MEM_UNIT) + select_value) << ";" << std::endl;
    return ((block * MEM_UNIT) + select_value); 

}

uint64_t Cqf::get_previous_runend2(uint64_t quotient){ //const
    cout << "- get_previous_runend2 " << quotient << endl;

    // starting pieces of information required
    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);
    uint64_t runend;


    // rank of starting word
    uint64_t searched_position = bitrankasm(occupied, pos_in_block) + offset;
    std::cout << "[SR]: FIRST SEARCHED_POSITION / rank of quot " << (searched_position) << ";" << std::endl;

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

        if (verbose || debug) std::cout << "[SR]: (WHILE-SP=0) SEARCHED_POSITION " << (searched_position) << ";" << std::endl;
    }

    uint64_t ones_rend;
    while(searched_position >= MEM_UNIT){
        runend = get_runend_word(block);
        ones_rend = bitrankasm(runend,MEM_UNIT - 1);
        searched_position -= ones_rend;
        block = get_next_block_id(block);
        //offset = get_offset_word(block);
        //occupied = get_occupied_word(block);

        if (verbose || debug) std::cout << "[SR]: (WHILE-SP>=64) SEARCHED_POSITION " << (searched_position) << ";" << std::endl;
    }

    // 0 < searched_position < MEM_UNIT
    runend = get_runend_word(block);
    uint64_t select_value = bitselectasm(runend,searched_position - 1);
    std::cout << "[SR]: SELECT_VALUE " << (select_value) << ";" << std::endl;

    while (select_value >= MEM_UNIT){
        ones_rend = bitrankasm(runend, MEM_UNIT - 1);
        searched_position -= ones_rend;
        block = get_next_block_id(block);
        runend = get_runend_word(block);
        select_value = bitselectasm(runend,searched_position - 1);
        if (verbose || debug) std::cout << "[SR]: BLOCK " << (block) << " | SELECT_VALUE " << select_value << ";" << std::endl;
    }
    if (verbose || debug) std::cout << "[SR]: SELECT_VALUE " << (select_value) << ";" << std::endl;
    if (verbose || debug) std::cout << "[SR]: RETURNED_VALUE " << ((block * MEM_UNIT) + select_value) << ";" << std::endl;
    return ((block * MEM_UNIT) + select_value); 

}

uint64_t Cqf::get_previous_runend(uint64_t quotient){ //const
    if (debug)
        cout << "- get_previous_runend " << quotient << endl;
    uint64_t block = get_block_id(quotient);
    uint64_t pos_in_block = get_shift_in_block(quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);

    if (verbose) {
        std::cout << "SR quotient " << quotient << std::endl; 
        std::cout << "SR block " << block << std::endl;
        std::cout << "SR pos_in_block " << pos_in_block << std::endl;
        std::cout << "SR offset " << offset << std::endl;
        std::cout << "SR occupied " << occupied << std::endl;

        std::cout << "[DB]: block" << block-1 << " (before)" <<  std::endl;
        std::cout << std::endl << block2string(block-1, false) << std::endl << std::endl;
        std::cout << "[DB]: block" << block << " (the one of the quotient)" <<  std::endl;
        std::cout << std::endl << block2string(block, false) << std::endl << std::endl;
        std::cout << "[DB]: block" << block+1 << " (after)" << std::endl;
        std::cout << std::endl << block2string(block+1, false) << std::endl << std::endl;

    }

    // --- Get the begining of the previous run ---
    if (verbose) cout << "pos_in_block " << pos_in_block << endl;
    if (verbose) cout << "offset " << offset << endl;
    uint64_t position = bitrankasm(occupied, pos_in_block) + offset;
    if (debug) cout << "position " << position << endl;

    if (verbose) cout << "bitrankasm " << bitrankasm(occupied, pos_in_block) << endl;
    if (verbose) cout << "position " << position << endl;

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
        if (verbose){
            std::cout << "pos >= MEM_UNIT " << position << std::endl;
        }
    }

    // --- Get the end of the previous run (from the beginning) ---
    if (verbose){
        std::cout << "rend_w" << std::endl;
        print_bits( get_runend_word(block) );
        std::cout << "offset: " << offset << std::endl;
        std::cout << "pos - 1: " << (position -1) << std::endl;
    }
    uint64_t select = bitselectasm(get_runend_word(block), position - 1);

    if (verbose){
        std::cout << "SR p-1 " << (position - 1) << std::endl;
        std::cout << "SR runends "<< std::endl;
        print_bits(get_runend_word(block));
        std::cout << "SR select " << select << std::endl;
    }

    while (select == MEM_UNIT){
        if (verbose) std::cout << "SR select == MEM_UNIT LOOP " << std::endl;
        block = get_next_block_id(block);

        position -= bitrankasm(get_runend_word(block),MEM_UNIT-1);
        if (verbose) std::cout << "SR position (== MEM_UNIT) " << position << std::endl;

        select = bitselectasm(get_runend_word(block),position - 1);
        if (verbose) std::cout << "SR select == MEM_UNIT " << select << std::endl;

    }

    if (verbose) std::cout << "SR FINAL " << (block * MEM_UNIT + select) << std::endl;
    if (debug)
        cout << "select " << select << endl;

    return ((block * MEM_UNIT) + select);
}

std::pair<uint64_t,uint64_t> Cqf::get_run_boundaries(uint64_t quotient){ //const
    //assert(quotient < m_num_bits);
    if(debug || verbose) std::cout << "GRB quotient " << quotient << std::endl;
    std::pair<uint64_t, uint64_t> boundaries;
    // the end of the run is found like this
    uint64_t end_pos = get_previous_runend2(quotient);
    if(debug || verbose) std::cout << "GRB end_pos " << end_pos << std::endl;
    uint64_t start_pos;

    uint64_t temp = get_previous_runend2(get_prev_quot(quotient));
    start_pos = get_next_quot(temp);
    if(debug || verbose) std::cout << "GRB start_pos " << start_pos << std::endl;
    //this should work for circular filter
    // I REMOVED THE EQUALS OF START POSITION AND QUOTIENT
    if (((start_pos < quotient) && (start_pos < end_pos) && (end_pos >= quotient)) 
    || ((start_pos < quotient) && (start_pos > end_pos) && (end_pos < quotient)) 
    || ((start_pos > end_pos) && (start_pos > quotient))) {
        if(debug || verbose) std::cout << "GRB IN IF " << std::endl;
        if(debug || verbose) std::cout << "GRB IN IF 1st CONDITION " << ((start_pos < quotient) && (start_pos < end_pos) && (end_pos >= quotient)) << std::endl;
        if(debug || verbose) std::cout << "GRB IN IF 2nd CONDITION " << ((start_pos < quotient) && (start_pos > end_pos) && (end_pos < quotient)) << std::endl;
        if(debug || verbose) std::cout << "GRB IN IF 3rd CONDITION " << ((start_pos >= end_pos) && (start_pos > quotient)) << std::endl;
        boundaries.first = quotient;
    }
    else boundaries.first = start_pos;
    // it is the position just after the end of the previous

    boundaries.second = end_pos;

    if(debug || verbose) std::cout << "GRB boundaries.first " << boundaries.first << std::endl;
    if(debug || verbose) std::cout << "GRB boundaries.second " << boundaries.second << std::endl;

    return boundaries;
}

uint64_t Cqf::first_unused_slot(uint64_t curr_quotient){ //const
    if (this->verbose) {
        std::cout << "IN FUS "  << std::endl;
    }


    uint64_t rend_pos = get_previous_runend2(curr_quotient);
    if (this->verbose) {
        std::cout << "FUS curr_quotient " << curr_quotient << std::endl;
        std::cout << "FUS out of sel_rank " << rend_pos << std::endl;
    }
    //if ((rend_pos == 0) && (curr_quotient != 0)) return curr_quotient;
    //preventing erroneous stop when it jumps from the end of the filter to the beginning 
    // and curr_quot > rend_pos for the circularity and not beacuse there is free space.
    uint64_t block = get_block_id(curr_quotient);
    // uint64_t pos_in_block = get_shift_in_block(curr_quotient);
    uint64_t offset = get_offset_word(block);
    uint64_t occupied = get_occupied_word(block);
    uint64_t loop_counter = 0;

    // why ((bitrankasm(occupied,pos_in_block) + offset) != 0) ?
    if (debug || verbose) {
        std::cout << "BEFORE WHILE " << std::endl;
        cout << "x <= s ? " << (((curr_quotient <= rend_pos))) << endl;
        cout << this->block2string(block);
        cout << "============================" << endl;
        cout << this->block2string(block+1);
    }

    while((curr_quotient <= rend_pos) || 
          ((get_block_id(curr_quotient) > get_block_id(rend_pos)) && 
           ((bitrankasm(
                get_occupied_word(get_block_id(curr_quotient)),
                get_shift_in_block(curr_quotient)) + get_offset_word(get_block_id(curr_quotient))) 
                    != 0))){

        
        curr_quotient = get_next_quot(rend_pos);
        rend_pos = get_previous_runend2(curr_quotient);
        loop_counter++;
        if (this->verbose) {
            std::cout << "FUS curr_quotient " << curr_quotient << std::endl;
            std::cout << "FUS rend_pos " << rend_pos << std::endl;
            std::cout << "LOOP COUNTER " << loop_counter << std::endl;
            cout << "x <= s ? " << (((curr_quotient <= rend_pos))) << endl;
            if (loop_counter > 2) {exit(0);}
            
        }
    }
    if (this->verbose) {
        std::cout << "AFTER WHILE " << curr_quotient << std::endl;
        std::cout << "FUS curr_quotient " << curr_quotient << std::endl;
    }

    return curr_quotient;
}

uint64_t Cqf::find_boundary_shift_deletion(uint64_t start_pos, uint64_t end_pos) const{
    if (debug)
        cout << "- find_boundary_shift_deletion " << start_pos << " " << end_pos << endl;
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
    if (debug)
        cout << "- shift_bits_left_metadata " << quotient << " " << flag_bit << " " << start_position << " " << end_position << endl;

    // METHOD FOR INSERTION
    uint64_t overflow_bit = flag_bit;
    uint64_t current_block = get_block_id(start_position);
    uint64_t current_shift_in_block = get_shift_in_block(start_position);
    uint64_t end_block = get_block_id(end_position);
    uint64_t end_shift_in_block = get_shift_in_block(end_position);

    // if (debug) {
    //     std::cout << "SBLM quotient " << quotient << std::endl; 
    //     std::cout << "SBLM flag_bit " << flag_bit << std::endl; 
    //     std::cout << "SBLM start_position " << start_position << std::endl; 
    //     std::cout << "SBLM end_position " << end_position << std::endl; 

    //     std::cout << "SBLM overflow_bit " << overflow_bit << std::endl; 
    //     std::cout << "SBLM current_block " << current_block << std::endl; 
    //     std::cout << "SBLM current_shift_in_block " << current_shift_in_block << std::endl; 
    //     std::cout << "SBLM end_block " << end_block << std::endl;
    //     std::cout << "SBLM end_shift_in_block " << end_shift_in_block << std::endl;
    // }

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

        if (this->verbose) {
            std::cout << "FLAG IS 1" << std::endl;
            cout << "set_occupied_bit " << quot_block << " " << flag_bit << " " << quot_shift_in_block << endl;
        }   

    }

    if ((current_block == end_block) && (start_position > end_position)) {

        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift,current_shift_in_block),(current_shift_in_block + 1));

        overflow_bit = shift_left(overflow_bit,current_shift_in_block);
        to_shift |= (save_right | overflow_bit);
        set_runend_word(current_block, to_shift); 

        if (this->verbose) {
            std::cout << "OLD RUNEND_W" << std::endl;
            print_bits(word_to_shift);
            std::cout << "NEW RUNEND_W" << std::endl;
            print_bits(to_shift);
            std::cout << "OVERFLOW BIT" << std::endl;
            print_bits(overflow_bit);
            std::cout << "SAVE RIGHT" << std::endl;
            print_bits(save_right);
        } 

        overflow_bit = word_to_shift >> (MEM_UNIT - 1);
        next_block = get_next_block_id(current_block);

        if (this->verbose) {
            std::cout << "NEW OVERFLOW BIT" << std::endl;
            print_bits(overflow_bit);
        } 

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

    while ( current_block != end_block ) {
        if (this->verbose) {
            std::cout << "CURRENT: " << current_block << " |END: " << end_block << std::endl;
        } 

        word_to_shift = get_runend_word(current_block);

        if (this->verbose) {
        std::cout << "OLD RUNEND_W" << std::endl;
        print_bits(word_to_shift);
        }

        save_right = word_to_shift & mask_right(current_shift_in_block);

        if (this->verbose) {
        std::cout << "MASKED OLD RUNEND_W" << std::endl;
        print_bits(save_right);
        }

        to_shift = shift_left(shift_right(word_to_shift,current_shift_in_block),(current_shift_in_block + 1));
        //((word_to_shift >> current_shift_in_block) << (current_shift_in_block + 1));

        if (this->verbose) {
        std::cout << "OLD RUNEND_W SHIFTED RIGHT OF " << current_shift_in_block << std::endl;
        print_bits((word_to_shift >> current_shift_in_block));
        std::cout << "AND THEN SHIFTED LEFT OF " << (current_shift_in_block + 1) << std::endl;
        print_bits(to_shift);
        }

        overflow_bit = shift_left(overflow_bit,current_shift_in_block);

        if (this->verbose) {
        std::cout << "OVERFLOW BIT" << std::endl;
        print_bits(overflow_bit);
        }

        to_shift |= (save_right | overflow_bit);

        if (this->verbose) {
        std::cout << "NEW RUNEND_W" << std::endl;
        print_bits(to_shift);
        }

        set_runend_word(current_block, to_shift); 

        overflow_bit = shift_right(word_to_shift,(MEM_UNIT - 1));
        next_block = get_next_block_id(current_block);

        if (this->verbose) {
            std::cout << "NEW OVERFLOW BIT" << std::endl;
            print_bits(overflow_bit);
        } 

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
    to_shift |= ((save_left | shift_left(overflow_bit,current_shift_in_block)) | save_right);
    //((save_left | (overflow_bit << current_shift_in_block)) | save_right);

    set_runend_word(current_block, to_shift); 

    if (this->verbose) {
            std::cout << "CURRENT: " << current_block << " |END: " << end_block << std::endl;
            std::cout << "OLD RUNEND_W" << std::endl;
            print_bits(word_to_shift);
            std::cout << "NEW RUNEND_W" << std::endl;
            print_bits(to_shift);
            std::cout << "OVERFLOW BIT" << std::endl;
            print_bits(overflow_bit);
            std::cout << "SAVE RIGHT" << std::endl;
            print_bits(save_right);
        } 

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
    to_shift = shift_left(shift_right(word_to_shift,(current_shift_in_block + 1)),current_shift_in_block);
    //std::cout << "SAVE  ";
    //print_bits(save_right);
    //std::cout << "SHIFT ";
    //print_bits(to_shift);
    next_block = get_next_block_id(current_block);
    
    overflow_bit = shift_left((get_runend_word(next_block) & 1ULL),(MEM_UNIT - 1));
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
    to_shift = shift_left(shift_right(word_to_shift,(current_shift_in_block + 1)),current_shift_in_block);
    //((word_to_shift >> (current_shift_in_block + 1)) << (current_shift_in_block));

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
    
    overflow_bit = shift_left((get_runend_word(next_block) & 1ULL),(MEM_UNIT - 1));
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


