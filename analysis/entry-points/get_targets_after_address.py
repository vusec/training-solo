#!/usr/bin/env python3
import sys

def get_mask(start_bit, end_bit):
    return (((1 << (end_bit - start_bit + 1)) - 1) << (start_bit))

MAX_DISTANCE_BITS = 12
MAX_DISTANCE_MASK = get_mask(0, MAX_DISTANCE_BITS - 1)

CACHE_LINE_MSB = 5
CACHE_LINE_MSB_MASK  = (1 << CACHE_LINE_MSB)

CACHE_LINE_MASK = get_mask(0, 5)



def output_targets_for_address(ind_address):

    ind_address = int(ind_address, 16)

    # CACHE_LINE MSB should be 0 for ind addresses
    if (ind_address & CACHE_LINE_MSB_MASK) >> CACHE_LINE_MSB != 0:
        return

    # The training indirect branch has a PC with PC[5] == 1.
    dir_address = (ind_address & ~CACHE_LINE_MASK) | CACHE_LINE_MSB_MASK
    target = dir_address + 3

    # output target as long as we can still jump to it as a short branch
    while dir_address & ~MAX_DISTANCE_MASK == target & ~MAX_DISTANCE_MASK:
        print(f'{hex(ind_address)},{hex(target)}')
        target += 1


try:
    for line in sys.stdin:
        output_targets_for_address(line)
except (BrokenPipeError, IOError):
    pass
