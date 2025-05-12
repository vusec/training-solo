#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#include "targets.h"

#define TARGET0_OFFSET ((1LU << 7))
#define TARGET1_MOCK_OFFSET 0x11000

#define N_FR_BUF 3

struct config {
    int fd_kernel_exec;
    int fd_vmcall;
    int fd_leak_page;

    int cpu_nr;

    uint8_t * base0;
    uint8_t * base1;

    uint8_t * phys_start;

    void (*caller0)(void); // also known as the 'train' branch
    void (*caller1)(void); // also known as the 'victim' or 'test' branch

    void (*target0)(void); // the target of caller0

    void (*target1_mock)(void);  // the arch. target of caller1 (not monitored)
    void (*target1_short)(void); // Collides with target0 if short target is stored
                                 // NOTE: With VMM as victim, only short is used
    void (*target1_near)(void);  // Collides with target0 if 32-bit is stored
    void **ftable1;

    uint8_t * kvm_speculation_target_pc;
    uint8_t * kvm_victim_indirect_branch_pc;

    uint8_t * fr_buf[N_FR_BUF];
    uint8_t * fr_buf_p[N_FR_BUF];

    uint8_t * ind_map;
    uint8_t * secret_page;

    uint8_t * host_fr_buf;
    uint8_t * host_secret_page;

    uint8_t * host_arg_fr_buf;
    uint8_t * host_arg_secret;

    void * caller_evict_list_pc[MAX_CALLER_EVICT_PC_SET];
    void * target_evict_list_pc[MAX_CALLER_EVICT_PC_SET];

    // -- static conditional branch history
    uint8_t * history_0; // conditional branch history caller0
    uint8_t * history_1; // conditional branch history caller1

    // -- jitted history
    uint64_t * history_jit_0;
    uint64_t * history_jit_1;

    uint8_t * history_jit_entry_0; // conditional branch history caller0
    uint8_t * history_jit_entry_1;

    uint8_t * history_jit_rwx_0;
    uint8_t * history_jit_rwx_1;

    // -- option flags
    uint64_t caller0_entry_offset; // N nops (offset) executed before caller0 branch
    uint64_t caller1_entry_offset; // N nops (offset) execute before caller0 branch

    uint8_t caller0_jump_type; // Type of branch for caller0
    uint8_t caller1_jump_type; // Type of branch for caller1

    uint8_t train_in_kernel;  // Is caller0 in kernel executed?
    uint8_t test_in_kernel;   // Is caller1 in kernel executed? 2 == in VMM

    uint8_t caller0_history_type; // history type for caller0
    uint8_t caller1_history_type; // history type


} typedef config;

// History types
enum {
    HISTORY_NONE,
    HISTORY_STATIC_CONDITIONAL,
    HISTORY_JIT,
    HISTORY_STATIC
};

// Jump types
enum {
    TRIGGER_INDIRECT,
    TRIGGER_DIRECT,
    TRIGGER_RET
};

void initialize_config(config * cfg);

#endif //_CONFIG_H_

