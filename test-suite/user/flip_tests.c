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
#include <string.h>

#include "config.h"
#include "common.h"
#include "jitting.h"
#include "flush_and_reload.h"
#include "targets.h"
#include "helper.h"


#define FLIP_TEST_ITERATIONS 50
#define FLIP_TEST_THR (FLIP_TEST_ITERATIONS * 0.1)


void print_can_flip_single(int can_flip[3][FLIPPING_BITS_END], int min_hits)
{
    for (int i = 0; i < 3; i++)
    {
        switch (i)
        {
        case 0:
            printf("%10s ", "TRAINING:");
            break;

        case 1:
            printf("%10s ", "VICTIM:");
            break;

        case 2:
            printf("%10s ", "BOTH:");
            break;
        }

        printf("{");
        for (int bit = 0; bit < (FLIPPING_BITS_END); bit++)
        {
            if(can_flip[i][bit] >= min_hits) {
                printf("%2d, ", bit);
            }
        }
        printf("}\n");

    }

}

void print_can_flip_double(int can_flip[3][FLIPPING_BITS_END][FLIPPING_BITS_END], int bits_filter[3][FLIPPING_BITS_END], int min_hits)
{

    for (int i = 0; i < 3; i++)
    {
        switch (i)
        {
        case 0:
            printf("%10s ", "TRAINING:");
            break;

        case 1:
            printf("%10s ", "VICTIM:");
            break;

        case 2:
            printf("%10s ", "BOTH:");
            break;
        }

        printf("{");

        for (int bit_x = 0; bit_x < (FLIPPING_BITS_END); bit_x++)
        {
            if (bits_filter[0][bit_x]) { continue; }

            uint8_t any_hit = 0;

            for (int bit_y = 0; bit_y < (FLIPPING_BITS_END); bit_y++)
            {
                if (bits_filter[0][bit_y]) { continue; }

                if(can_flip[i][bit_x][bit_y] >= min_hits) {
                    printf("%2d_%-2d, ", bit_x, bit_y);
                    any_hit = 1;
                }
            }
            if (i == 2 && any_hit) {
                printf("\n%6s", "");
            }
        }
        printf("}\n");

    }

}


// #define VERBOSE

void flip_single_bits_of_caller(config * cfg, int can_flip[3][FLIPPING_BITS_END]) {

    uint8_t enable_short = 1;
    if (cfg->prefetch_side_channel == 1) {
        enable_short = 0;
    }


#ifdef VERBOSE
    printf("%10s TESTING FLIPPING SINGLE BITS %10s \n","==========", "==========");
#endif

    uint64_t hits[N_FR_BUF] = {};
    uint64_t old_training, old_victim;

    old_victim = (uint64_t) cfg->paths[P_TRAINING].branch_addr;
    old_training = (uint64_t) cfg->paths[P_VICTIM].branch_addr;
    clean_all(cfg);

#ifdef VERBOSE
    printf("%-10s | %9s %-7s %9s | %9s %-7s %9s | %9s %-7s %9s |\n","", "", "CALLER0", "", "", "CALLER1", "", "", "BOTH", "");
    printf("%-10s | %-7s | %-7s | %-7s | %-7s | %-7s | %-7s | %-7s | %-7s | %-7s |\n","FLIPPING:", "SHORT", "NEAR", "FAR", "SHORT", "NEAR", "FAR", "SHORT", "NEAR", "FAR");
#endif

    for (int bit = 0; bit < (FLIPPING_BITS_END); bit++)
    {

#ifdef VERBOSE
        printf("%7s %2d ",
            "BIT", bit);
#endif

        // Test TRAINING
        cfg->paths[P_VICTIM].branch_addr = (void *) old_victim;
        cfg->paths[P_TRAINING].branch_addr = (void *)(old_training ^ (1LLU << bit));
        if (bit == 47) {  cfg->paths[P_TRAINING].branch_addr = (void *) ((uint64_t) cfg->paths[P_TRAINING].branch_addr | 0xffff000000000000);  }

        if (cfg->paths[P_VICTIM].branch_addr == cfg->paths[P_TRAINING].branch_addr) {
#ifdef VERBOSE
            printf("\n");
#endif
            continue;
        }

        // printf("BIT %2d 0x%lx %p %p\n", bit, old_training, cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);

        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

#ifdef VERBOSE
        printf("| %7lu | %7lu | %7lu ",
            hits[0], hits[1], hits[2]);
        // printf("| %p %p", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);
#endif

        if((enable_short && hits[0] >  FLIP_TEST_THR) || hits[1] > FLIP_TEST_THR || hits[2] > FLIP_TEST_THR) {
            can_flip[0][bit] += 1;
        }


        clean_all(cfg);

        // Test caller1
        cfg->paths[P_TRAINING].branch_addr = (void *) old_training;
        cfg->paths[P_VICTIM].branch_addr = (void *)(old_victim ^ (1LLU << bit));
        if (bit == 47) {  cfg->paths[P_VICTIM].branch_addr = (void *) ((uint64_t) cfg->paths[P_VICTIM].branch_addr | 0xffff000000000000);  }

        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

#ifdef VERBOSE
        printf("| %7lu | %7lu | %7lu ",
            hits[0], hits[1], hits[2]);
        // printf("| %p %p", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);
#endif

        if((enable_short && hits[0] >  FLIP_TEST_THR)  || hits[1] > FLIP_TEST_THR || hits[2] > FLIP_TEST_THR) {
            can_flip[1][bit] += 1;
        }


        clean_all(cfg);

        // Test both

        cfg->paths[P_TRAINING].branch_addr = (void *)(old_training ^ (1LLU << bit));
        cfg->paths[P_VICTIM].branch_addr = (void *)(old_victim ^ (1LLU << bit));
        if (bit == 47) {  cfg->paths[P_TRAINING].branch_addr = (void *) ((uint64_t) cfg->paths[P_TRAINING].branch_addr | 0xffff000000000000);  }
        if (bit == 47) {  cfg->paths[P_VICTIM].branch_addr = (void *) ((uint64_t) cfg->paths[P_VICTIM].branch_addr | 0xffff000000000000);  }

        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

#ifdef VERBOSE
        printf("| %7lu | %7lu | %7lu ",
            hits[0], hits[1], hits[2]);
        // printf("| %p %p\n", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);
        printf("\n");
#endif

        if((enable_short && hits[0] >  FLIP_TEST_THR)  || hits[1] > FLIP_TEST_THR || hits[2] > FLIP_TEST_THR) {
            can_flip[2][bit] += 1;
        }


        clean_all(cfg);
        fflush(stdout);

    }

    cfg->paths[P_TRAINING].branch_addr = (void *)old_training;
    cfg->paths[P_VICTIM].branch_addr = (void *)old_victim;
    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);

    }

// #define VERBOSE

void flip_double_bits_of_caller(config * cfg, int can_flip[3][FLIPPING_BITS_END][FLIPPING_BITS_END]) {

    uint8_t enable_short = 1;
    if (cfg->prefetch_side_channel == 1) {
        enable_short = 0;
    }

#ifdef VERBOSE
    printf("%10s TESTING FLIPPING DOUBLE BITS %10s \n","==========", "==========");
#endif

    uint64_t hits[N_FR_BUF] = {};
    uint64_t old_training, old_victim;

    old_training = (uint64_t) cfg->paths[P_TRAINING].branch_addr;
    old_victim = (uint64_t) cfg->paths[P_VICTIM].branch_addr;
    clean_all(cfg);
#ifdef VERBOSE

    printf("%-10s | %9s %-7s %9s | %9s %-7s %9s | \n","", "", "CALLER0", "", "", "CALLER1", "");
    printf("%-10s | %-7s | %-7s | %-7s | %-7s | %-7s | %-7s |\n","FLIPPING:", "SHORT", "NEAR", "FAR", "SHORT", "NEAR", "FAR");

#endif

    for (int bit_x = 0; bit_x < (FLIPPING_BITS_END); bit_x++)
    {

        for (int bit_y = 0; bit_y < (FLIPPING_BITS_END); bit_y++)
        {

            if (bit_x >= bit_y) {
                continue;
            }

            #ifdef VERBOSE
            printf("%2d and %2d",
                bit_x, bit_y);
            #endif
#if 1
            // Test caller0
            cfg->paths[P_VICTIM].branch_addr = (void *) old_victim;
            cfg->paths[P_TRAINING].branch_addr = (void *)((old_training + CALLER_PATCH_END_OFFSET) ^ ((1LLU << bit_x) | (1LLU << bit_y))) - CALLER_PATCH_END_OFFSET;
            if (bit_x == 47 || bit_y == 47) {  cfg->paths[P_TRAINING].branch_addr = (void *) ((uint64_t) cfg->paths[P_TRAINING].branch_addr | 0xffff000000000000);  }

            if (cfg->paths[P_VICTIM].branch_addr == cfg->paths[P_TRAINING].branch_addr) {
            #ifdef VERBOSE
                printf("\n");
            #endif
                continue;

            }
            jit_callers(cfg);
            insert_targets_at_free_addresses(cfg, cfg->target0_offset);

            do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

            #ifdef VERBOSE
            printf("| %7lu | %7lu | %7lu ",
                hits[0], hits[1], hits[2]);
            printf("| %18p %18p", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);

            fflush(stdout);
            #endif


            if((enable_short && hits[0] >  FLIP_TEST_THR) || hits[1] > FLIP_TEST_THR || hits[2] > FLIP_TEST_THR) {
                can_flip[0][bit_x][bit_y] += 1;
            }



            clean_all(cfg);

            // Test caller1

            cfg->paths[P_TRAINING].branch_addr = (void *) old_training;
            cfg->paths[P_VICTIM].branch_addr = (void *)((old_victim + CALLER_PATCH_END_OFFSET) ^ ((1LLU << bit_x) | (1LLU << bit_y))) - CALLER_PATCH_END_OFFSET;
            if (bit_x == 47 || bit_y == 47) {  cfg->paths[P_VICTIM].branch_addr = (void *) ((uint64_t) cfg->paths[P_VICTIM].branch_addr | 0xffff000000000000);  }


            jit_callers(cfg);
            insert_targets_at_free_addresses(cfg, cfg->target0_offset);

            do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

            #ifdef VERBOSE
            printf("| %7lu | %7lu | %7lu ",
                hits[0], hits[1], hits[2]);
            // printf("| %p %p", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);
            #endif

            if((enable_short && hits[0] >  FLIP_TEST_THR)  || hits[1] > FLIP_TEST_THR || hits[2] > FLIP_TEST_THR) {
                can_flip[1][bit_x][bit_y] += 1;
            }

            clean_all(cfg);
#endif
            // Test both

#if 0
            cfg->paths[P_TRAINING].branch_addr = (void *)((old_training + CALLER_PATCH_END_OFFSET) ^ ((1LLU << bit_x) | (1LLU << bit_y))) - CALLER_PATCH_END_OFFSET;
            cfg->paths[P_VICTIM].branch_addr = (void *)((old_victim + CALLER_PATCH_END_OFFSET) ^ ((1LLU << bit_x) | (1LLU << bit_y))) - CALLER_PATCH_END_OFFSET;
            if (bit_x == 47 || bit_y == 47) {  cfg->paths[P_TRAINING].branch_addr = (void *) ((uint64_t) cfg->paths[P_TRAINING].branch_addr | 0xffff000000000000);  }
            if (bit_x == 47 || bit_y == 47) {  cfg->paths[P_VICTIM].branch_addr = (void *) ((uint64_t) cfg->paths[P_VICTIM].branch_addr | 0xffff000000000000);  }


            jit_callers(cfg);
            insert_targets_at_free_addresses(cfg, cfg->target0_offset);

            do_flush_and_reload(cfg, hits, FLIP_TEST_ITERATIONS);

            #ifdef VERBOSE
            printf("| %7lu | %7lu | %7lu ",
                hits[0], hits[1], hits[2]);
            printf("| %p %p", cfg->paths[P_TRAINING].branch_addr, cfg->paths[P_VICTIM].branch_addr);
            #endif

            if(hits[0] > 1 || hits[1] > 1 || hits[2] > 1) {
                can_flip[2][bit_x][bit_y] += 1;
            }

            clean_all(cfg);
#endif
        #ifdef VERBOSE
            printf("\n");
        #endif
            fflush(stdout);

        }

    }

    cfg->paths[P_TRAINING].branch_addr = (void *) old_training;
    cfg->paths[P_VICTIM].branch_addr = (void *) old_victim;
    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);


}

// -----------------------------------------------------------------------------
// Randomization tests

enum {
    RANDOM_C0_SRC,
    RANDOM_C1_SRC,
    RANDOM_C0_HIS,
    RANDOM_C1_HIS,
    RANDOM_C0_TGT,
    RANDOM_C0_ENTRY_OFFSET,
    RANDOM_C1_ENTRY_OFFSET,
    NULL_TEST,
    RANDOM_ALL,
    RANDOM_NUMBER_OF_OPTIONS
};


static void copy_cfg(struct config * cfg_dst, struct config * src_cfg) {

    uint8_t * dst_cond_array[N_HISTORIES];
    uint64_t * dst_jit_array[N_HISTORIES];

    // preserve the pointers to the arrays
    for (size_t i = 0; i < N_HISTORIES; i++) {
        dst_cond_array[i] = cfg_dst->histories[i].cond_array;
        dst_jit_array[i] = cfg_dst->histories[i].jit_array;
    }

    memcpy(cfg_dst, src_cfg, sizeof(config));

    // restore array pointers and copy the content
    for (size_t i = 0; i < N_HISTORIES; i++) {
        cfg_dst->histories[i].cond_array = dst_cond_array[i];
        memcpy(cfg_dst->histories[i].cond_array, src_cfg->histories[i].cond_array, MAX_HISTORY_SIZE * sizeof(uint64_t));

        cfg_dst->histories[i].jit_array = dst_jit_array[i];
        memcpy(cfg_dst->histories[i].jit_array, src_cfg->histories[i].jit_array, HISTORY_JMP_SIZE * sizeof(uint64_t));

    }

}

#define HIT_TEST_ITERATIONS 1000


static void randomize_axis(struct config * cfg,  uint64_t total_hits[], int rnd_option) {

    uint64_t hits[N_FR_BUF] = {};
    uint64_t round = 0;
    for (int i = 0; i < N_FR_BUF; i++){ total_hits[i] = 0; }

    while (round < HIT_TEST_ITERATIONS)
    {
        round++;

        switch (rnd_option)
        {
        case RANDOM_C0_SRC:
            cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
            break;

        case RANDOM_C1_SRC:
            cfg->paths[P_VICTIM].branch_addr = get_random_address(cfg->base1);
            break;

        case RANDOM_C0_HIS:
            randomize_history(&cfg->histories[P_TRAINING]);
            break;

        case RANDOM_C1_HIS:
            randomize_history(&cfg->histories[P_VICTIM]);
            break;

        case RANDOM_C0_TGT:
            cfg->target0_offset = rand() % (1LU << (N_NEAR_BITS - 1));
            break;

        case RANDOM_C0_ENTRY_OFFSET:
            cfg->paths[P_TRAINING].entry_offset = rand() % MAX_ENTRY_OFFSET;
            break;

        case RANDOM_C1_ENTRY_OFFSET:
            cfg->paths[P_VICTIM].entry_offset = rand() % MAX_ENTRY_OFFSET;
            break;

        case NULL_TEST:
            break;
        default:
            break;
        }

        // we always the jit history, it may be overwritten by previous iterations
        if (cfg->histories[P_TRAINING].type == HISTORY_JIT && rnd_option != RANDOM_C0_HIS) {
            cfg->histories[P_TRAINING].jit_entry = insert_history_jmp_chain(cfg->histories[P_TRAINING].jit_rwx, cfg->histories[P_TRAINING].jit_array);
        }
        if (cfg->histories[P_VICTIM].type == HISTORY_JIT && rnd_option != RANDOM_C1_HIS) {
            cfg->histories[P_VICTIM].jit_entry = insert_history_jmp_chain(cfg->histories[P_VICTIM].jit_rwx, cfg->histories[P_VICTIM].jit_array);
        }

        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        do_flush_and_reload(cfg, hits, 100);

        for (size_t idx = 0; idx < N_FR_BUF; idx++)
        {
            if(hits[idx] > 10) {
                total_hits[idx] += 1;
            }
        }

        clean_all(cfg);

    }

}

void do_hit_test(struct config * old_cfg) {

    uint64_t hits[N_FR_BUF] = {};
    struct config new_cfg;
    memcpy(&new_cfg, old_cfg, sizeof(config));

    for (int i = 0; i < N_HISTORIES; i++)
    {
        new_cfg.histories[i].cond_array = calloc(sizeof(uint64_t), MAX_HISTORY_SIZE);
        new_cfg.histories[i].jit_array = calloc(sizeof(uint64_t), HISTORY_JMP_SIZE);
    }

    printf("\n");

    copy_cfg(&new_cfg, old_cfg);

    clean_all(old_cfg);

    randomize_axis(&new_cfg, hits, NULL_TEST);
    print_hits(&new_cfg, hits, "BEFORE RANDOM", HIT_TEST_ITERATIONS);

// -----------------------------------------------------------------------------
// Randomize all axises

    randomize_axis(&new_cfg, hits, RANDOM_C0_HIS);
    print_hits(&new_cfg, hits, "RANDOM C0_HIS", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C1_HIS);
    print_hits(&new_cfg, hits, "RANDOM C1_HIS", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C0_TGT);
    print_hits(&new_cfg, hits, "RANDOM C0_TGT", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C0_SRC);
    print_hits(&new_cfg, hits, "RANDOM C0_SRC", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C1_SRC);
    print_hits(&new_cfg, hits, "RANDOM C1_SRC", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C0_ENTRY_OFFSET);
    print_hits(&new_cfg, hits, "RANDOM C0_ENTRY", HIT_TEST_ITERATIONS);

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(&new_cfg, hits, RANDOM_C1_ENTRY_OFFSET);

    print_hits(&new_cfg, hits, "RANDOM C1_ENTRY", HIT_TEST_ITERATIONS);


// -----------------------------------------------------------------------------

    copy_cfg(&new_cfg, old_cfg);

    randomize_axis(old_cfg, hits, NULL_TEST);

    print_hits(old_cfg, hits, "AFTER RESTORE", HIT_TEST_ITERATIONS);

    // cleanup
    jit_callers(old_cfg);
    insert_targets_at_free_addresses(old_cfg, old_cfg->target0_offset);

    for (int i = 0; i < N_HISTORIES; i++) {
        free(new_cfg.histories[i].cond_array);
        free(new_cfg.histories[i].jit_array);
    }

}
