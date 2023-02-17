#ifndef ADDITIONAL_METHODS_HPP
#define ADDITIONAL_METHODS_HPP
#include <stdint.h> 

/** Print the bits of a given uint64 word
 * @param x the uint64 of which you want to see the bits
 * It prints on the standard output the bits of the uint64 word
 */
void print_bits(uint64_t x);

/** returns a mask of 1s of the leftmost Nth bits
 * e.g. mask_left(5) gives 11111000000000000...
 * @param num_bits number of 1s from the left in the mask
 * @return the uint64 corresponding to the mask
 */
uint64_t mask_left(uint64_t num_bits);

/** returns a mask of 1s of the rightmost Nth bits
 * e.g. mask_right(5) gives 000...00011111
 * @param num_bits number of 1s from the right in the mask
 * @return the uint64 corresponding to the mask
 */
uint64_t mask_right(uint64_t num_bits);

/** returns the value shifted left of shift bits
 * if shift is 64 returns a 0
 * @param value to shift
 * @param shift applied to value
 * @return the uint64 corresponding to the shifted value
 */
uint64_t shift_left(uint64_t value, uint64_t shift);

/** returns the value shifted right of shift bits
 * if shift is 64 returns a 0
 * @param value to shift
 * @param shift applied to value
 * @return the uint64 corresponding to the shifted value
 */
uint64_t shift_right(uint64_t value, uint64_t shift);

uint64_t rebuild_number(uint64_t quotient, uint64_t remainder, uint64_t shift);

/** returns the id of the block associated to the requested quotient
 * @param position the quotient of wich you want to know the block
 * @return the block id
 * e.g. if blocks have 64 quotients inside each, 
 * get_block_id(155) = 2 (there is block 0)
 */
uint64_t get_block_id(uint64_t position);

/** returns the position in the (corresponding) block of the quotient
 * @param position the quotient of which you want to know the position in its block
 * @return the position in which it is in the block
 * e..g. if blocks have 64 quotients inside,
 * get_shit_in_block(155) = 28
 */
uint64_t get_shift_in_block(uint64_t position);

/** gives back the quotient corresponding to a particular block and position in the block
 * @param block the particular block 
 * @param shift the shitft in the block (eg the position in the block)
 * @return a uint64 corresponding to the quotient associated with them
 * e.g. get_quot_from_block_shift(2,28) = 155
 */
uint64_t get_quot_from_block_shift(uint64_t block, uint64_t shift);

/** bitwise select operation
 * @param num   word of the bitvector of runends
 * @param rank  rank position
 * @return the select operation
 */
uint64_t bitselectasm(uint64_t num, uint64_t rank);

/** bitwise rank operation
 * @param val   word of the bitvector of occupieds
 * @param pos   position of the bit in the word
 * @return the rank operation
 */
uint64_t bitrankasm(uint64_t val, uint64_t pos);

/** get a specific bit from a uint64 word given its position in it
 * @param word  word from which you want a bit
 * @param pos_bit   position of the bit in the word
 * @return uint64 with the bit (in the rightmost pos) corresponding to the extracted bit
 */
uint64_t get_bit_from_word(uint64_t word, uint64_t pos_bit);

/** get 'len' bits of the vector from the bitposition pos 
 * @param vec   vector from which you want to extract bits
 * @param pos   starting position of the bits you want to extract (in bits)
 * @param len   how many bits you want to extract (up to 64)
 * @return a uint64 with the bits you extracted
 */
uint64_t get_bits(std::vector<uint64_t>& vec, uint64_t pos, uint64_t len);

/** set 'len' bits of the vector from the bitposition pos. Up to 64 bits at a time (value is a uint64)
 * @param vec vector you want to modify
 * @param pos   position of the bit where you want to start editing the vector
 * @param value uint64 with bits that you want to cast in the vector
 * @param len   number of bits that are gonna be effectively casted in the vector
 */
void set_bits(std::vector<uint64_t>& vec, uint64_t pos, uint64_t value, uint64_t len);

#endif