#include <iostream>
#include <bitset>

#include "filter.hpp" 
#include "ext_methods.hpp"
#include "test.hpp"

#define MEM_UNIT 64ULL

int main(int argc, char** argv) {
    uint64_t num_of_words = 9;
    std::vector<uint64_t> cqf(num_of_words);

    //setto offsets
    cqf[0] = 1ULL;
    cqf[3] = 5ULL;
    cqf[6] = 0ULL;
    
    //setto occupieds
    cqf[1] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;
    cqf[4] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;
    cqf[7] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;

    //setto runends
    cqf[2] = 0b0000011110000011110000011110000011110000011110000011110000011111ULL;
    cqf[5] = 0b0000000000001000000000000000010000000000000100000000000000000010ULL;
    cqf[8] = 0b0101010101010101010101010101010111100011101010101010101010101010ULL;
    
    std::cout << cqf.size() << std::endl;
    std::cout << std::endl;
    print_vector(cqf);
    std::cout << std::endl;
    //std::cout << "hello" << std::endl;
    //test_masks();
    shift_bits_left_metadata(cqf,1,101,86);
    std::cout << std::endl;
    print_vector(cqf);
}