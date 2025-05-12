/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "targets.h"
#include "common.h"
#include "helper.h"
#include "jitting.h"

int init_address_evicting_branch(void ** evict_list, void * to_evict_branch_addr) {
    uint64_t evict_address;

    evict_address = 0x100000000000 + ((get_45bit_random_value() % RANDOMIZE_SPACE));
    evict_address &= ~TAG_SET_OFFSET_MASK;
    evict_address += (((uint64_t) to_evict_branch_addr) & TAG_SET_OFFSET_MASK);
    evict_address -= CALLER_PATCH_END_OFFSET;

    *evict_list = (void *) evict_address;

    return 1;
}


int init_addresses_btb_eviction_set(void ** evict_list, void * to_evict_branch_addr) {
    uint64_t evict_address;
    uint8_t ok;

    for (int i = 0; i < BTB_EVICTION_SET_SIZE; i++)
    {
        ok = 0;

        while (ok == 0)
        {
            ok = 1;
            evict_address = 0x200000000000 + ((get_45bit_random_value() % (1LU << (24))) << (SET_END + 1));
            evict_address |= (get_45bit_random_value() & OFFSET_MASK);
            evict_address |= (uint64_t) to_evict_branch_addr & SET_MASK;

            evict_address -= CALLER_PATCH_END_OFFSET;

            if (get_tag_for_address(to_evict_branch_addr) == get_tag_for_address((void *) evict_address)) {
                ok = 0;
                continue;
            }

            // test self collision
            for (size_t j = 0; j < i; j++) {
                if (get_tag_for_address(evict_list[j]) == get_tag_for_address((void *) evict_address)) {
                    ok = 0;
                    break;
                }
            }

        }

        evict_list[i] = (void *) evict_address;

    }

    return BTB_EVICTION_SET_SIZE;
}

int init_addresses_ibtb_eviction_set(void ** evict_list, void * to_evict_branch_addr) {
    uint64_t evict_address;

    for (int i = 0; i < iBTB_EVICTION_SET_SIZE; i++)
    {
            // As shown by Indirector, IP[15:6] are used for the iBTB tag,
            // IP is not involved for indexing.
            // We set the offset equal, as the last jump target matters
            // evict_address = (0x300000000000 + get_45bit_random_value());

            evict_address = (uint64_t) get_random_address((void *) 0x300000000000);
            evict_address &= ~OFFSET_MASK;
            evict_address += (uint64_t) to_evict_branch_addr & OFFSET_MASK;
            evict_address -= CALLER_PATCH_END_OFFSET;

            evict_list[i] = (void *) evict_address;

    }

    return iBTB_EVICTION_SET_SIZE;
}
