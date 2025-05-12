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
    #define BTB_CONTENTION_SET_SIZE 4
    #define MAX_CALLER_EVICT_PC_SET (BTB_EVICTION_SET_SIZE + BTB_CONTENTION_SET_SIZE)

#elif defined(INTEL_11_GEN)
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

    #define BTB_EVICTION_SET_SIZE 8
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
    #define BTB_CONTENTION_SET_SIZE 1
    #define MAX_CALLER_EVICT_PC_SET 1
#endif

#define TAG_MASK_PART_1 (((1LLU << (TAG_PART_1_END - TAG_PART_1_START + 1)) - 1) << (TAG_PART_1_START))
#define TAG_MASK_PART_2 (((1LLU << (TAG_PART_2_END - TAG_PART_2_START + 1)) - 1) << (TAG_PART_2_START))


#define SET_MASK (((1LLU << (SET_END - SET_START + 1)) - 1) << (SET_START))
#define TAG_SET_MASK (TAG_MASK_PART_1 | TAG_MASK_PART_2 | SET_MASK)

#define SHORT_CALLER_MASK (((1LU << (64 - N_SHORT_BITS)) - 1) << N_SHORT_BITS)
#define SHORT_TARGET_MASK ((1LU << N_SHORT_BITS) - 1)

#define NEAR_CALLER_MASK (((1LU << (64 - N_NEAR_BITS)) - 1) << N_NEAR_BITS)
#define NEAR_TARGET_MASK ((1LU << N_NEAR_BITS) - 1)


#endif //_TARGETS_H_
