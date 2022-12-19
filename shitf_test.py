#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on 21/12/2021

@author: francescoandreace
"""

import sys

MEM_UNIT = 64

def print_bits(x: int):
    # Convert the integer to a binary string
    binary_str = bin(x)[2:]
    # Print the binary string, padding with zeros if necessary
    print(binary_str.zfill(64))

def mask_right(num_bits: int) -> int:
    return (1 << num_bits) - 1

def mask_left(num_bits: int) -> int:
    return ~((1 << (MEM_UNIT - num_bits)) - 1)


def shift_bits_left_and_insert(start_word: int,start_pos: int, end_word: int, end_pos: int,bit_to_insert:int):
    #there are 3 possible cases, the first one is in the same word, 

    # while they are not in the same
    #while start_word != end_word:
    pass

if __name__ == '__main__':
    print_bits(mask_right(5))

    print_bits(mask_left(4))