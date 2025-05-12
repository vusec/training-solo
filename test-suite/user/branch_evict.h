/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _BRANCH_EVICT_H_
#define _BRANCH_EVICT_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int init_address_evicting_branch(void ** evict_list, void * evict_target);
int init_addresses_btb_eviction_set(void ** evict_list, void * evict_target);
int init_addresses_ibtb_eviction_set(void ** evict_list, void * evict_target);


#endif // _BRANCH_EVICT_H_
