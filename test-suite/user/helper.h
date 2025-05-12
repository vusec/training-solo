/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _HELPER_H_
#define _HELPER_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "config.h"
#include "targets.h"
#include "common.h"

void print_hits_header(config * cfg);
void print_hits(config * cfg, uint64_t hits[], char * text, uint64_t iterations);

uint32_t get_tag_for_address(void * address);
uint32_t get_set_for_address(void * address);
uint32_t get_offset_for_address(void * address);
void print_branch_info(void * branch_addr, char * desc);

void print_config(config * cfg);
void print_global_settings(config * cfg);


static __always_inline __attribute__((always_inline)) void randomize_history_cond_skip_last_n(uint8_t * cond_array, int skip_n) {
    for(int i=0; i<MAX_HISTORY_SIZE - skip_n; i++) cond_array[i] = rand()&1;
}

static __always_inline __attribute__((always_inline)) void randomize_history_cond(uint8_t * cond_array) {
    return randomize_history_cond_skip_last_n(cond_array, 0);
}

static __always_inline __attribute__((always_inline)) void set_history_cond_static(uint8_t * cond_array) {
    for(int i=0; i<MAX_HISTORY_SIZE; i++) cond_array[i] = i % 2;
}


static __always_inline uint64_t get_45bit_random_value() {
    return (((uint64_t) rand() & 0x7fffLU) << 30) |
            (((uint64_t) rand() & 0x7fffLU) << 15) |
            ((uint64_t) rand() & 0x7fffLU);

}

#define PAGE_OFFSET  (0xffff888000000000ULL)   // (0xffff888000000000ULL) // 0xffff888000000000ULL
static uint64_t virt_to_physmap(uint64_t virtual_address, uint64_t page_offset) {
    int pagemap;
    uint64_t value;
    int got;
    uint64_t page_frame_number;

    if (!page_offset) {
        page_offset = PAGE_OFFSET;

    } else if (page_offset == 0xffff888100000000UL) {
        // We are running in a VM (KASLR disabled), this is a hack
        // to still let virt_to_physmap work
        page_offset = 0xffff888000000000UL;
    }


    pagemap = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap < 0) {
        printf("ERROR: Pagemap %d", pagemap);
        exit(1);
    }

    got = pread(pagemap, &value, 8, (virtual_address / 0x1000) * 8);
    if (got != 8) {
        exit(2);
    }

    page_frame_number = value & ((1ULL << 54) - 1);
    if (page_frame_number == 0) {
        printf("ERROR: page_frame_number %ld\n", page_frame_number);
        exit(3);
    }

    close(pagemap);

    return page_offset + (page_frame_number * 0x1000 + virtual_address % 0x1000);
}

#define HUGE_PAGE_SIZE (1 << 21) // 2 MB

static int get_rss_of_addr(void * addr)
{
    void * start = 0;
    int rss = -1;
    char line[256];

    FILE* file = fopen("/proc/self/smaps","r");

    if (!file) {
        printf("Error opening /proc/self/smaps file!\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(line, sizeof(line), file)) {
        // Find the address
        if(sscanf(line, "%p-%*[^\n]\n", &start) != 1 && start == addr)
        {
            // Find the RSS field
            while(fgets(line, sizeof(line), file)) {

                if(sscanf(line, "Rss: %d kB\n", &rss) == 1) {
                    break;
                }
            }

            break;

        }

    }

    fclose(file);

    return rss;
}

static uint8_t * allocate_huge_page()
{
    uint8_t * addr = memalign(HUGE_PAGE_SIZE, HUGE_PAGE_SIZE);

    madvise(addr, HUGE_PAGE_SIZE, MADV_HUGEPAGE);
    *(volatile uint8_t *) addr = 1;

    assert(get_rss_of_addr(addr) == 2048);

    return addr;
}



#endif //_HELPER_H_
