#include <iostream>
#include <bitset>

#include "filter.hpp" 
#include "ext_methods.hpp"
#include "test.hpp"

#define MEM_UNIT 64ULL

int main(int argc, char** argv) {
    /*
    uint64_t num_of_words = 9;
    std::vector<uint64_t> cqf(num_of_words);

    //setto offsets
    cqf[0] = 1ULL;
    cqf[3] = 4ULL;
    cqf[6] = 0ULL;
    
    //setto occupieds
    cqf[1] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;
    cqf[4] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;
    cqf[7] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;

    //setto runends
    cqf[2] = 0b0000011110000011110000011110000011110000011110000011110000011111ULL;
    cqf[5] = 0b1000000000001000000000000000010000000000000000000000000000000010ULL;
    cqf[8] = 0b1101010101010101010101010101010111100011101010101010101010101010ULL;
    
    std::cout << cqf.size() << std::endl;
    std::cout << std::endl;
    print_vector(cqf);
    std::cout << std::endl;
    //std::cout << "hello" << std::endl;
    //test_masks();
    shift_bits_left_metadata(cqf,100,0,101,86);
    std::cout << std::endl;
    print_vector(cqf);
    std::cout << std::endl << std::endl;
    shift_bits_right_metadata(cqf,100,0,101,86);
    std::cout << std::endl;
    print_vector(cqf);
    //test_bitrank();
    */
    //test_first_unused_slot();
    //test_rank_select_operations();
    //test_run_boundaries();
    //test_boundary_shift_deletion();
    //test_empty_case();

    //test_cqf_metadata();
    //test_metadata_shift();
    //test_metadata_shift1();
    //test_set_get_reminder();
    //test_shift_left_add_reminder();

    //test_boundaries();

    //test_fus();
    //test_sel_rank_filter();
    //test_cqf_bound_del();
    //test_insert();

    test_cqf_size();
}