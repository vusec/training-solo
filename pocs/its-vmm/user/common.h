#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define OPTION_GET_HOST_ADDRESS 0
#define OPTION_GET_VICTIM_ADDRESS 2
#define OPTION_GET_SPECULATION_TARGET_ADDRESS 3
#define OPTION_TRIGGER_GADGET   10

#define MANY_ITERATIONS 1000
#define BRUTE_FORCE_ITERATIONS 10
#define CHECK_ITERATIONS 1000

#define MAX_ENTRY_OFFSET 64

#define RANDOMIZE_BIT_START 0
#define RANDOMIZE_BIT_END   33
#define RANDOMIZE_SPACE (1LU << (RANDOMIZE_BIT_END - RANDOMIZE_BIT_START))

#define HISTORY_RWX_SIZE_BITS 20
#define HISTORY_RWX_SIZE (1LU << HISTORY_RWX_SIZE_BITS)

#define FR_BUF_SID 0xdecafe


extern void target_start(void);
extern void target_entry(void);
extern void target_patch(void);
extern void target_end(void);
#define TARGET_PATCH_OFF (target_patch - target_entry)
#define TARGET_SIZE (target_end - target_start)

extern void ret_target_start(void);
extern void ret_target_entry(void);
extern void ret_target_end(void);
#define RET_TARGET_SIZE (ret_target_end - ret_target_start)

extern void caller_start(void);
extern void caller_entry(void);
extern void caller_patch(void);
extern void caller_end(void);

extern void jit_exit_start(void);
extern void jit_exit_end(void);

#define CALLER_PATCH_START_OFFSET (caller_patch - caller_entry)
#define CALLER_PATCH_END_OFFSET (caller_patch - caller_entry + 4)
#define CALLER_SIZE (caller_end - caller_start)

#define TARGET_SIZE (target_end - target_start)

#define ENTRY_OFFSET 2


#endif //_COMMON_H_
