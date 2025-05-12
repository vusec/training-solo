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

void print_hits_header();
void print_hits(uint64_t hits[], char * text, uint64_t iterations);

uint32_t get_tag_for_address(void * caller);
uint32_t get_set_for_address(void * caller);
uint32_t get_offset_for_address(void * caller);

void print_config(config * cfg);
void print_global_settings(config * cfg);


static __always_inline __attribute__((always_inline)) void randomize_history_static_skip_last_n(uint8_t * history, int skip_n) {
    for(int i=0; i<MAX_HISTORY_SIZE - skip_n; i++) history[i] = rand()&1;
}

static __always_inline __attribute__((always_inline)) void randomize_history_static(uint8_t * history) {
    return randomize_history_static_skip_last_n(history, 0);
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



static __attribute__((__noinline__))
uint64_t do_vmcall(int fd_vmcall, uint64_t option, uint64_t arg2, uint64_t arg3 ) {

    char buf[17];
    void * args[4];
    uint64_t ret;

    args[0] = (void *) 0;
    args[1] = (void *)  option;
    args[2] = (void *)  arg2;
    args[3] = (void *)  arg3;


    snprintf(buf, 17, "%lx", (size_t) args);

    ret = pread(fd_vmcall, buf, sizeof(buf), 2);

    if (ret == 0) {
        printf("[ERROR] Kernel executing vmcall failed: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    return ret;

}



#endif //_HELPER_H_
