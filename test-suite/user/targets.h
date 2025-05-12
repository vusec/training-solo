/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _TARGETS_H_
#define _TARGETS_H_

#define MAX_HISTORY_SIZE 512

#if defined(INTEL_10_GEN) || defined(INTEL_9_GEN)

    // [14:21]
    #define TAG_PART_1_START 14
    #define TAG_PART_1_END   21

    // [29:22]
    #define TAG_PART_2_START 22
    #define TAG_PART_2_END   29

    // [13:5]
    #define SET_START 5
    #define SET_END 13

    #define N_SHORT_BITS 10
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 29

    #define BTB_EVICTION_SET_SIZE 8
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(SUNNY_COVE) || defined(CYPRESS_COVE) || defined(WILLOW_COVE) || \
        defined(INTEL_11_GEN)
    #define TAG_PART_1_START 14
    #define TAG_PART_1_END   23

    #define TAG_PART_2_START 24
    #define TAG_PART_2_END   33

    // [13:5]
    #define SET_START 5
    #define SET_END 13

    #define N_SHORT_BITS 12
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 66

    #define BTB_EVICTION_SET_SIZE 6
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(GOLDEN_COVE) || defined(INTEL_14_GEN) || defined(RAPTOR_COVE) || \
        defined(INTEL_ULTRA_1) || defined(REDWOOD_COVE)
    #define TAG_PART_1_START 15
    #define TAG_PART_1_END   23

    #define TAG_PART_2_START 0
    #define TAG_PART_2_END   0

    // [13:5]
    #define SET_START 5
    #define SET_END 14

    #define N_SHORT_BITS 12     // No short bits
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 194

    #define BTB_EVICTION_SET_SIZE 12
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(GRACEMONT)  || defined(INTEL_14_GEN_E_CORE)
    #define TAG_PART_1_START 15
    #define TAG_PART_1_END   24

    #define TAG_PART_2_START 0
    #define TAG_PART_2_END   0

    // [13:5]
    #define SET_START 5
    #define SET_END 14

    #define N_SHORT_BITS 12     // No short bits
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 194

    #define BTB_EVICTION_SET_SIZE (5 + 2)
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(CRESTMONT)  || defined(INTEL_ULTRA_1_E_CORE)
    #define TAG_PART_1_START 16
    #define TAG_PART_1_END   25

    #define TAG_PART_2_START 0
    #define TAG_PART_2_END   0

    // [13:5]
    #define SET_START 6
    #define SET_END 15

    #define N_SHORT_BITS 12     // No short bits
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 194

    #define BTB_EVICTION_SET_SIZE (12 + 8)
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(LION_COVE)  || defined(INTEL_ULTRA_2)
    #define TAG_PART_1_START 13
    #define TAG_PART_1_END   21

    #define TAG_PART_2_START 0
    #define TAG_PART_2_END   0

    // [13:5]
    #define SET_START 4
    #define SET_END 12

    #define N_SHORT_BITS 12     // No short bits
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 194

    #define BTB_EVICTION_SET_SIZE (24 + 2)
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(SKYMONT)  || defined(INTEL_ULTRA_2_E_CORE)
    #define TAG_PART_1_START 16
    #define TAG_PART_1_END   25

    #define TAG_PART_2_START 0
    #define TAG_PART_2_END   0

    // [13:5]
    #define SET_START 6
    #define SET_END 15

    #define N_SHORT_BITS 12     // No short bits
    #define N_NEAR_BITS 32

    #define HISTORY_JMP_SIZE 194

    #define BTB_EVICTION_SET_SIZE 8
    #define iBTB_EVICTION_SET_SIZE 8
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#else
    #error "Not supported micro-architecture"
    // silence undefined errors
    #define TAG_PART_1_START 1
    #define TAG_PART_1_END 1
    #define TAG_PART_2_START 1
    #define TAG_PART_2_END   1
    #define SET_START 1
    #define SET_END 1
    #define N_SHORT_BITS 1
    #define N_NEAR_BITS 1
    #define HISTORY_JMP_SIZE 1

    #define BTB_EVICTION_SET_SIZE 1
    #define iBTB_EVICTION_SET_SIZE 1
    #define BTB_CONTENTION_SET_SIZE 1
    #define MAX_CALLER_EVICT_PC_SET 1
#endif

#if(TAG_PART_2_START == 0)
    #define TAG_MASK_PART_1 (((1LLU << (TAG_PART_1_END - TAG_PART_1_START + 1)) - 1) << (TAG_PART_1_START))
    #define TAG_MASK_PART_2 (0)
    #define TAG_MASK (TAG_MASK_PART_1)
#else
    #define TAG_MASK_PART_1 (((1LLU << (TAG_PART_1_END - TAG_PART_1_START + 1)) - 1) << (TAG_PART_1_START))
    #define TAG_MASK_PART_2 (((1LLU << (TAG_PART_2_END - TAG_PART_2_START + 1)) - 1) << (TAG_PART_2_START))
    #define TAG_MASK (TAG_MASK_PART_1 | TAG_MASK_PART_2)

    #if (TAG_PART_1_END - TAG_PART_1_START != TAG_PART_2_END - TAG_PART_2_START)
        #error "Invalid Tag: Part 1 and part 2 should be of equal lengths"
    #endif
#endif


#define OFFSET_MASK ((1LLU << SET_START) - 1)
#define SET_MASK (((1LLU << (SET_END - SET_START + 1)) - 1) << (SET_START))
#define SET_OFFSET_MASK (SET_MASK | OFFSET_MASK)
#define TAG_SET_MASK (TAG_MASK | SET_MASK)
#define TAG_SET_OFFSET_MASK (TAG_SET_MASK | OFFSET_MASK)


#define SHORT_CALLER_MASK (((1LU << (64 - N_SHORT_BITS)) - 1) << N_SHORT_BITS)
#define SHORT_TARGET_MASK ((1LU << N_SHORT_BITS) - 1)

#define NEAR_CALLER_MASK (((1LU << (64 - N_NEAR_BITS)) - 1) << N_NEAR_BITS)
#define NEAR_TARGET_MASK ((1LU << N_NEAR_BITS) - 1)


#endif //_TARGETS_H_
