/*
 * Training Solo
 * Friday, March 22th 2024
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */


#ifndef _COMMON_H_
#define _COMMON_H_

#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>

#define HUGE_PAGE_SIZE (1 << 21)

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


#endif //_COMMON_H_
