/*
 * Training Solo
 * Friday, March 22th 2024
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _EVICT_SYS_TABLE_L2_H_
#define _EVICT_SYS_TABLE_L2_H_

#if defined(INTEL_10_GEN)
    #define L1_WAYS (8)
    #define L2_WAYS (4)
    #define L2_SETS (1024)
    #define L2_EVICT_SIZE (4 + 6)

#elif defined(INTEL_11_GEN)
    #define L1_WAYS 12
    #define L2_WAYS 8
    #define L2_SETS 1024
    #define L2_EVICT_SIZE (8 + 6)

#elif defined(INTEL_13_GEN)
    #define L1_WAYS (12)
    #define L2_WAYS (16)
    #define L2_SETS (2048)
    #define L2_EVICT_SIZE (16 + 8)
#else
    #error "Not supported micro-architecture"
    // silence undefined errors
    #define L1_WAYS 1
    #define L2_WAYS 1
    #define L2_SETS 1
#endif

#include <assert.h>


void build_ev_set_l2(uint64_t target, void ** ev_set);

__always_inline void evict(void **ev_set)
{
	void **start = (void **)ev_set[0];
	void **p = start;
	do {
		p = *p;
	} while (p != start);
}

#endif //_EVICT_SYS_TABLE_L2_H_
