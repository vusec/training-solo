/*
 * Training Solo test suite
 * Friday, March 6th 2025
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
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <sys/syscall.h>
#include <math.h>

#include "lib.h"
#include "common.h"
#include "templates.h"
#include "flush_and_reload.h"
#include "jitting.h"
#include "targets.h"
#include "helper.h"
#include "mitigations.h"
#include "config.h"
#include "flip_tests.h"
#include "../../common/kmmap/mm.h"

#define PATH_DISABLE_SMAP_SMEP "/proc/training_solo/disable_smap_smep"
#define PATH_KERNEL_EXECUTE "/proc/training_solo/do_execute"

void disable_smap_smep() {

    int fd;
    char buf[8];
    uint8_t * address;

    if (access(PATH_DISABLE_SMAP_SMEP, F_OK) == 0) {
        fd = open(PATH_DISABLE_SMAP_SMEP, O_RDONLY);
        assert(fd);
    } else {
        printf("Error: File %s not found. Please insert the kernel module\n", PATH_DISABLE_SMAP_SMEP);
        exit(EXIT_FAILURE);
    }

    assert(read(fd, buf, 8) == 3);

    close(fd);

}


static int test_current_config(config * cfg, uint8_t do_print_config) {

    uint64_t hits[N_FR_BUF] = {};

    for (size_t inner = 0; inner < 1; inner++)
    {

        do_flush_and_reload(cfg, hits, BRUTE_FORCE_ITERATIONS);

        if(
            hits[0] > 1 || hits[1] > 1 || hits[2] > 1 ||
            hits[3] > 0 || hits[4] > 0 || hits[5] > 0
            ) {

            if (do_print_config) {
                printf("\n");
                print_global_settings(cfg);
                print_config(cfg);
            }

            print_hits_header(cfg);
            print_hits(cfg, hits, "INITIAL", BRUTE_FORCE_ITERATIONS);


            for (size_t i = 0; i < 5; i++)
            {
                do_flush_and_reload(cfg, hits, MANY_ITERATIONS * 10);
                print_hits(cfg, hits, "VERIFY", MANY_ITERATIONS * 10);
            }


            fflush(stdout);

            return 1;

        }

    }
    return 0;

}

static int do_single_test(config * cfg) {

    uint64_t outer = 0;

    for (size_t i = 0; i < N_HISTORIES; i++) {
        randomize_history(&cfg->histories[i]);
    }


    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);
    print_global_settings(cfg);
    print_config(cfg);


    while (1)
    {

        outer++;
        if(test_current_config(cfg, 0)) {
            printf("Total iterations: %lu\n", outer);
            return 0;
        }

        if(outer % (100 * 100) == 0) {
            printf("\rIteration: %lu ", outer);
            fflush(stdout);
        }

        if (outer > 500 * 100) {
            printf("No signal... exiting\n");
            exit(0);
        }

    }

    clean_all(cfg);

    return 0;
}


static void do_btb_properties_test(config * cfg) {
    int iter;
    int can_flip_single[3][FLIPPING_BITS_END] = {0};
    int can_flip_double[3][FLIPPING_BITS_END][FLIPPING_BITS_END] = {0};


    for (size_t i = 0; i < N_HISTORIES; i++) {
        randomize_history(&cfg->histories[i]);
    }

    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);
    print_global_settings(cfg);
    print_config(cfg);

    printf("Initial test:\n");

    for (iter = 0; iter < 10; iter++) {
        if(test_current_config(cfg, 0)) {
            break;
        }
    }

    if (iter >= 10) {
        printf("No signal! Please try with two colliding addresses as base\n");
    }

    printf("\n[+] This test shows which bits can be flipped without affecting the\n"
            "    aliasing between the training and victim branch\n"
            " -- Single bits: If we can flip a single bit, then this bit is \n"
            "    likely not used for neither the tag or set\n"
            " -- Two bits: If we can flip two bits at the same time, then those bit are \n"
            "    likely part of a XOR function in either the set or tag algorithm\n"
            "    Note: Flips that we can flip individually are filtered out\n\n");

    printf("-- TESTING FLIPPING SINGLE BITS [%20s]", "");

    int outer_iterations = 20;
    if (cfg->prefetch_side_channel){
        outer_iterations = 50;
    }

    for (int i = 0; i < outer_iterations; i++) {

        flip_single_bits_of_caller(cfg, can_flip_single);

        clean_all(cfg);
        randomize_branch_addresses(cfg);
        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        printf("\r-- TESTING FLIPPING SINGLE BITS [%.*s", (int) (i * (20 / (double) outer_iterations) + 1), "..................................................");
        fflush(stdout);
    }

    printf("\n");
    print_can_flip_single(can_flip_single, 2);

    printf("-- TESTING FLIPPING TWO BITS    [%20s]", "");

    for (int i = 0; i < outer_iterations; i++) {

        flip_double_bits_of_caller(cfg, can_flip_double);

        clean_all(cfg);
        randomize_branch_addresses(cfg);
        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        printf("\r-- TESTING FLIPPING TWO BITS    [%.*s", (int) (i * (20 / (double) outer_iterations) + 1), "..................................................");
        fflush(stdout);

    }

    printf("\n");
    print_can_flip_double(can_flip_double, can_flip_single, 2);


}

static void do_set_bits_test(config * cfg) {
    int iter;
    int can_flip_single[3][FLIPPING_BITS_END] = {0};
    int can_flip_double[3][FLIPPING_BITS_END][FLIPPING_BITS_END] = {0};


    for (size_t i = 0; i < N_HISTORIES; i++) {
        randomize_history(&cfg->histories[i]);
    }

    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);
    print_global_settings(cfg);
    print_config(cfg);

    printf("Initial test:\n");

    for (iter = 0; iter < 10; iter++) {
        if(test_current_config(cfg, 0)) {
            break;
        }
    }

    if (iter >= 10) {
        printf("No signal, this is good! We have a correct eviction\n");
    } else {
        printf(">> If we have a signal on short, this is probably a 'fallthrough' of the prefetcher\n");
        printf(">> If on NEAR/FAR, please fix the eviction set first.\n");

    }

    printf("\n[+] We have setup an BTB eviction set which we walk after execution of\n"
        "    the training branch. Without flipping any training/victim bits,\n"
        "    the eviction set should evict the inserted entry with no signal as result.\n"
        "    In this test we flip training and victim branch bits to check which flips\n"
        "    result in indexing in a different set, making the eviction set ineffective\n"
        "    with as result a signal.\n"
        " -- Both: For this test the 'BOTH' results matter: where we flip the bit\n"
        "    for both the training branch and the victim branch.\n"
        "    The bits we can flip are the bits that are used for set indexing.\n\n");

    printf("-- TESTING FLIPPING SINGLE BITS [%20s]", "");

    int outer_iterations = 50;

    for (int i = 0; i < outer_iterations; i++) {

        flip_single_bits_of_caller(cfg, can_flip_single);
        // printf("\n");
        // print_can_flip_single(can_flip_single, 0);

        clean_all(cfg);
        randomize_branch_addresses(cfg);
        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);

        printf("\r-- TESTING FLIPPING SINGLE BITS [%.*s", (int) (i * (20 / (double) outer_iterations) + 1), "..................................................");
        fflush(stdout);
    }

    printf("\n");


    print_can_flip_single(can_flip_single, 5);
}

static int do_randomization_test(config * cfg) {

    uint64_t outer = 0;

    for (size_t i = 0; i < N_HISTORIES; i++) {
        randomize_history(&cfg->histories[i]);
    }


    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, cfg->target0_offset);

    while (1)
    {

        outer++;
        if(test_current_config(cfg, 1)) {
            do_hit_test(cfg);
            printf("Total iterations: %lu\n", outer);
        }

        if(outer % (100 * 100) == 0) {
            printf("\rIteration: %lu ", outer);
            fflush(stdout);
        }

        clean_all(cfg);
        randomize_branch_addresses(cfg);
        jit_callers(cfg);
        insert_targets_at_free_addresses(cfg, cfg->target0_offset);


    }

    clean_all(cfg);

    return 0;
}


static int init(config * cfg) {
    uint64_t seed;

    seed = time(0);
    srand(seed);
    printf("Seed: %lu\n", seed);

    disable_smap_smep();

    initialize_memory_controller();

    // Initialize global settings

    cfg->base0 =(uint8_t *) 0x500000000000LLU;
    cfg->base1 =(uint8_t *) 0x600000000000LLU;
    cfg->base2 =(uint8_t *) 0x400000000000LLU;


    cfg->ind_map = (uint8_t *) allocate_huge_page();


    cfg->ftable1 = (void **) malloc(0x2000);

    for (unsigned i = 0; i < N_FR_BUF; i++)
    {
        cfg->fr_buf[i] = cfg->ind_map + (0x1000 * (i + 1));
        memset(cfg->fr_buf[i], 0x94, 0x1000);
        *(((uint8_t **)cfg->fr_buf_p) + i) = (uint8_t *) cfg->fr_buf[i] - FR_BUF_SID;
    }

    cfg->secret_page = cfg->ind_map + 0x10000;
    memset(cfg->secret_page, 0x0, 0x1000);

    // ------------------------------------------------------------------------
    // History setup

    for (int i = 0; i < N_HISTORIES; i++)
    {
        cfg->histories[i].cond_array = calloc(sizeof(uint64_t), MAX_HISTORY_SIZE);
        cfg->histories[i].jit_array = calloc(sizeof(uint64_t), HISTORY_JMP_SIZE);
        cfg->histories[i].jit_rwx = mmap((void *) 0x120000000000 + (0x40000000000 * i), HISTORY_RWX_SIZE,
            PROT_WRITE|PROT_READ|PROT_EXEC,  MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, -1, 0);
        cfg->histories[i].id = i;
    }

    // ------------------------------------------------------------------------

    if (access(PATH_KERNEL_EXECUTE, F_OK) == 0) {
        cfg->fd_kernel_exec = open(PATH_KERNEL_EXECUTE, O_WRONLY);
        assert(cfg->fd_kernel_exec);
    } else {
        printf("Error: File %s not found. Please insert the kernel module\n", PATH_KERNEL_EXECUTE);
        exit(EXIT_FAILURE);
    }


    // Initialize config based on template
    cfg->target0_offset = DEFAULT_TARGET0_OFFSET;
    cfg->histories[P_EVICTING].type = HISTORY_CONDITIONAL;
    initialize_config(cfg);

    return 0;

}


int main(int argc, char **argv)
{
    struct config cfg = {0};

    assert(syscall(SYS_getcpu, &cfg.cpu_nr, NULL, NULL) == 0);
    printf("Running on CPU: %d\n", cfg.cpu_nr);

#ifndef SUPSRESS_MITIGATION_INFO
    print_mitigation_info(cfg.cpu_nr);
#endif

    if (init(&cfg)) {
        return 1;
    }

    switch (cfg.test_function)
    {
    case TEST_SINGLE_TEST:
        do_single_test(&cfg);
        break;

    case TEST_BTB_PROPERTIES:
        do_btb_properties_test(&cfg);
        break;

    case TEST_BTB_SET_BITS:
        do_set_bits_test(&cfg);
        break;

    case TEST_RANDOMIZE:
        do_randomization_test(&cfg);
        break;

    default:
        break;
    }

}
