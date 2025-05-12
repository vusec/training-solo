/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#include "targets.h"

#define DEFAULT_TARGET0_OFFSET ((1LU << 9))
#define TARGET1_MOCK_OFFSET 0x11000

#define N_FR_BUF 6

// Branch paths
enum path_type {
    P_TRAINING,
    P_VICTIM,
    P_EXTRA,
    P_EVICTING,
    N_PATHS,
} typedef path_type;

// Note: evicting path type does
// not have its own branch path
#define N_BRANCH_PATHS  (N_PATHS - 1)
#define N_HISTORIES     (N_PATHS)

// History types
enum history_type_t {
    HISTORY_NONE,
    HISTORY_CONDITIONAL,
    HISTORY_JIT,
    HISTORY_STATIC,
    N_HISTORY_TYPES
} typedef history_type_t;

static char *history_type_str[N_HISTORY_TYPES] = {
    "NONE", "COND BRS", "JIT BRS", "STATIC"};

// Jump types
enum branch_type_t {
    TRIGGER_INDIRECT_JMP,
    TRIGGER_INDIRECT_CALL,
    TRIGGER_DIRECT_UNCOND,
    TRIGGER_DIRECT_COND,
    TRIGGER_DIRECT_CALL,
    TRIGGER_RET,
    N_BRANCH_TYPES
} typedef branch_type_t;

static char *branch_type_str[N_BRANCH_TYPES] = {
    "IND_JMP", "IND_CALL", "DIR_UNCO", "DIR_COND", "DIR_CALL", "RET"};

// Jump types
enum domain_type {
    DOMAIN_USER,
    DOMAIN_KERNEL,
    N_DOMAINS
} typedef domain_type;

static char *domain_str[N_DOMAINS] = {"USER", "KERNEL"};


// Test functions
enum test_function_t {
    TEST_SINGLE_TEST,   // default
    TEST_BTB_PROPERTIES,
    TEST_BTB_SET_BITS,
    TEST_RANDOMIZE
} typedef test_function_t;

struct history_t {
    history_type_t type;
    uint8_t id;                 // Unique id, used for HISTORY_NONE
    uint8_t * cond_array;       // Conditional history array
    uint64_t * jit_array;       // Array of randomized branch addresses
    void * jit_entry;           // Entry address for the JIT history
    uint8_t * jit_rwx;          // Read/write/execute region for JIT history
} typedef history_t;

struct branch_path_t {
    path_type id;
    domain_type domain;

    void * caller_start;
    void * caller_entry;
    void * branch_addr;

    uint64_t entry_offset;
    branch_type_t branch_type;

    void * btb_eviction_set[BTB_EVICTION_SET_SIZE];
    void * btb_eviction_targets[BTB_EVICTION_SET_SIZE];
    void * ibtb_eviction[iBTB_EVICTION_SET_SIZE];
} typedef branch_path_t;



struct config {
    int fd_kernel_exec;

    int cpu_nr;

    uint8_t * base0;
    uint8_t * base1;
    uint8_t * base2;

    uint8_t * phys_start;

    branch_path_t paths[N_BRANCH_PATHS];
    history_t histories[N_HISTORIES];
    uint64_t target0_offset;

    void * target0; // the target of caller0

    void * target1_mock;    // the arch. target of caller1 (not monitored)
    void * target1_short;   // Collides with target0 if short target is stored
                            // NOTE: With VMM as victim, only short is used
    void *target1_near;     // Collides with target0 if 32-bit is stored

    void * target2;         // TARGET OF CALLER EXTRA
    void * target2_short;
    void * target2_near;

    void **ftable1;

    test_function_t test_function;
    uint8_t prefetch_side_channel;

    uint8_t * fr_buf[N_FR_BUF];
    uint8_t * fr_buf_p[N_FR_BUF];

    uint8_t * ind_map;
    uint8_t * secret_page;

    uint8_t * host_fr_buf;
    uint8_t * host_secret_page;

    uint8_t * host_arg_fr_buf;
    uint8_t * host_arg_secret;

} typedef config;

#endif //_CONFIG_H_

