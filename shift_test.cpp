#include <iostream>
#include <bitset>
#include <stdint.h> 

#define MEM_UNIT  64ULL

void print_bits(uint64_t x) {
  std::bitset<64> bits(x);
  std::cout << bits << std::endl;
}

uint64_t mask_right(uint64_t num_bits) {
    uint64_t mask = -(num_bits >= MEM_UNIT) | ((1ULL << num_bits) - 1ULL);
    return mask;
}

uint64_t mask_left(uint64_t num_bits) {
    //uint64_t mask = ~(-(num_bits >= MEM_UNIT)) | ~((1ULL << (MEM_UNIT - num_bits)) - 1ULL);
    return ~(mask_right(MEM_UNIT-num_bits));
}

