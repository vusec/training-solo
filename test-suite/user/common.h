/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "targets.h"

#define OPTION_GET_HOST_ADDRESS 0
#define OPTION_GET_VICTIM_ADDRESS 2
#define OPTION_GET_SPECULATION_TARGET_ADDRESS 3
#define OPTION_TRIGGER_GADGET   10

#define MANY_ITERATIONS 1000
#define BRUTE_FORCE_ITERATIONS 10
#define CHECK_ITERATIONS 1000

#define MAX_ENTRY_OFFSET 128

#define RANDOMIZE_BIT_START 0
#define RANDOMIZE_BIT_END   44
#define RANDOMIZE_SPACE (1LU << (RANDOMIZE_BIT_END - RANDOMIZE_BIT_START))

#define FLIPPING_BITS_END 48


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

#ifdef LION_COVE
    // This is a hack for LION_COVE: on LION_COVE the BTB is indexed by the
    // address of the prior target (aka entry point). With this hack we align
    // the branch addr with the entrypoint instead of last branch instruction.
    // In this way we flipping bits works as expected.

    // The default entry point (last target) for a caller
    #define CALLER_DEFAULT_LAST_TARGET_OFFSET (-5 -(CALLER_PATCH_START_OFFSET - MAX_ENTRY_OFFSET))
    #define CALLER_PATCH_END_OFFSET (caller_patch - caller_entry + 5 + CALLER_DEFAULT_LAST_TARGET_OFFSET)
#else
    #define CALLER_PATCH_END_OFFSET (caller_patch - caller_entry + 5)
#endif

#define CALLER_SIZE (caller_end - caller_start)

#define TARGET_SIZE (target_end - target_start)

#define ENTRY_OFFSET 2


#endif //_COMMON_H_
