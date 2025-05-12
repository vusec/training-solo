#include "config.h"

void initialize_config(config * cfg) {

    // default settings

    cfg->train_in_kernel = 1;
    cfg->test_in_kernel = 2;

    cfg->caller0_jump_type = TRIGGER_DIRECT;
    cfg->caller1_jump_type = TRIGGER_INDIRECT;


    cfg->caller0_history_type = HISTORY_NONE;
    cfg->caller1_history_type = HISTORY_NONE;

    // N nops (offset) executed before caller branch
    // max is MAX_ENTRY_OFFSET (64)
    cfg->caller0_entry_offset = 0x0;
    cfg->caller1_entry_offset = 0x0;

#if defined(TEST_USER_USER)

    cfg->train_in_kernel = 0;
    cfg->test_in_kernel = 0;

#elif defined(TEST_USER_KERNEL)

    cfg->train_in_kernel = 0;
    cfg->test_in_kernel = 1;

#elif defined(TEST_USER_VMM)

    cfg->caller0_history_type = HISTORY_NONE;

    cfg->train_in_kernel = 0;
    cfg->test_in_kernel = 2;

#elif defined(TEST_KERNEL_KERNEL)

    cfg->train_in_kernel = 1;
    cfg->test_in_kernel = 1;

#elif defined(TEST_KERNEL_VMM)

    cfg->caller0_history_type = HISTORY_NONE;

    cfg->train_in_kernel = 1;
    cfg->test_in_kernel = 2;

#else
    #error "Test not defined or unkown"

#endif


}

