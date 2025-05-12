/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _FLIP_TESTS_H_
#define _FLIP_TESTS_H_

#include "common.h"
#include "config.h"

void flip_single_bits_of_caller(config * cfg, int can_flip[3][FLIPPING_BITS_END]);
void flip_double_bits_of_caller(config * cfg, int can_flip_double[3][FLIPPING_BITS_END][FLIPPING_BITS_END]);

void print_can_flip_single(int can_flip[3][FLIPPING_BITS_END], int min_hits);
void print_can_flip_double(int can_flip[3][FLIPPING_BITS_END][FLIPPING_BITS_END], int bits_filter[3][FLIPPING_BITS_END], int min_hits);

void do_hit_test(struct config * old_cfg);

#endif //_FLIP_TESTS_H_
