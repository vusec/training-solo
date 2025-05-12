/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#include <string.h>
#include <assert.h>

#include "common.h"
#include "targets.h"
#include "helper.h"
#include "config.h"
#include "../../common/kmmap/mm.h"

// ----------------------------------------------------------------------------
// ASM

// 48 8b 06     mov    rax, qword ptr [rsi]
#define LOAD_RSI_ASM "\x48\x8b\x06"
#define LOAD_RSI_SIZE (sizeof(LOAD_RSI_ASM) - 1)

// 48 8b 86
// fe ca de 00  mov    rax, qword ptr [rsi + 0xdecafe]
#define LOAD_RSI_DECAFE_ASM "\x48\x8b\x86\xfe\xca\xde\x00"
#define LOAD_RSI_DECAFE_SIZE (sizeof(LOAD_RSI_DECAFE_ASM) - 1)

// 48 8b 02     mov    rax, qword ptr [rdx]
#define LOAD_RDX_ASM "\x48\x8b\x02"
#define LOAD_RDX_SIZE (sizeof(LOAD_RDX_ASM) - 1)

// 48 8b 82
// fe ca de 00  mov    rax, qword ptr [rdx + 0xdecafe]
#define LOAD_RDX_DECAFE_ASM "\x48\x8b\x82\xfe\xca\xde\x00"
#define LOAD_RDX_DECAFE_SIZE (sizeof(LOAD_RDX_DECAFE_ASM) - 1)

// 48 8b 01     mov    rax, qword ptr [rcx]
#define LOAD_RCX_ASM "\x48\x8b\x01"
#define LOAD_RCX_SIZE (sizeof(LOAD_RCX_ASM) - 1)

// 48 8b 81
// fe ca de 00  mov    rax, qword ptr [rcx + 0xdecafe]
#define LOAD_RCX_DECAFE_ASM "\x48\x8b\x81\xfe\xca\xde\x00"
#define LOAD_RCX_DECAFE_SIZE (sizeof(LOAD_RCX_DECAFE_ASM) - 1)

// 4d 8b 00     mov    r8, qword ptr [r8]
#define LOAD_R8_ASM "\x4d\x8b\x00"
#define LOAD_R8_SIZE (sizeof(LOAD_R8_ASM) - 1)

// 4d 8b 80
// fe ca de 00  mov    r8, qword ptr [r8 + 0xdecafe]
#define LOAD_R8_DECAFE_ASM "\x4d\x8b\x80\xfe\xca\xde\x00"
#define LOAD_R8_DECAFE_SIZE (sizeof(LOAD_R8_DECAFE_ASM) - 1)

// 4d 8b 09     mov    r9, qword ptr [r9]
#define LOAD_R9_ASM "\x4d\x8b\x09"
#define LOAD_R9_SIZE (sizeof(LOAD_R9_ASM) - 1)

// 4d 8b 89
// fe ca de 00  mov    r9, qword ptr [r9 + 0xdecafe]
#define LOAD_R9_DECAFE_ASM "\x4d\x8b\x89\xfe\xca\xde\x00"
#define LOAD_R9_DECAFE_SIZE (sizeof(LOAD_R9_DECAFE_ASM) - 1)


// 4d 8b 12     mov    r10, qword ptr [r10]
#define LOAD_R10_ASM "\x4d\x8b\x12"
#define LOAD_R10_SIZE (sizeof(LOAD_R10_ASM) - 1)

// 4d 8b 92
// fe ca de 00  mov    r10, qword ptr [r10 + 0xdecafe]
#define LOAD_R10_DECAFE_ASM "\x4d\x8b\x92\xfe\xca\xde\x00"
#define LOAD_R10_DECAFE_SIZE (sizeof(LOAD_R10_DECAFE_ASM) - 1)



// 0f ae e8     lfence
// 41 ff e3     jmp    r11
#define JMP_R11_ASM    "\x0f\xae\xe8\x41\xff\xe3\xcc"
#define JMP_R11_SIZE   (sizeof(JMP_R11_ASM) - 1)
#define JIT_EXIT_SIZE ((uint64_t) (jit_exit_end - jit_exit_start))


// 90 90 90 90  nop; nop; nop; nop
// ff 17        call rdi
// c3           ret
#define CALL_RDI_ASM "\x90\x90\x90\x90\xff\x17\xc3\xcc"
#define CALL_RDI_SIZE (sizeof(CALL_RDI_ASM) - 1)

// 90 90 90 90  nop; nop; nop; nop
// ff 27        jmp rdi
#define JMP_RDI_ASM "\x90\x90\x90\x90\xff\x27\xcc"
#define JMP_RDI_SIZE (sizeof(JMP_RDI_ASM) - 1)

// 90               nop
// e9 __ __ __ __   jmp ...
#define REL_BRANCH_ASM    "\x90\xe9\x00\x00\x00\x00\xcc"
#define REL_BRANCH_OFFSET   6
#define REL_BRANCH_ASM_SIZE (sizeof(REL_BRANCH_ASM) - 1)

// 90               nop
// e8 __ __ __ __   call ...
#define REL_CALL_ASM    "\x90\xe8\x00\x00\x00\x00\xc3"
#define REL_CALL_OFFSET   6
#define REL_CALL_ASM_SIZE (sizeof(REL_CALL_ASM) - 1)


// 0f 82 __ __ __ __ jnz ...
#define REL_JC_BRANCH_ASM    "\x0f\x82\x00\x00\x00\x00\xcc"
#define REL_JC_BRANCH_OFFSET   6
#define REL_JC_BRANCH_ASM_SIZE (sizeof(REL_JC_BRANCH_ASM) - 1)

// c3                ret
#define RET_ASM    "\x90\x90\x90\x90\xc3\xcc"
#define RET_ASM_SIZE   (sizeof(RET_ASM) - 1)

// 90 90 90 90      nop; nop; nop; nop
// ff 37            push [rdi]
// c3               ret
#define PUSH_RDI_MEM_ASM "\x90\x90\xff\x37\xc3\xcc"
#define PUSH_RDI_MEM_SIZE (sizeof(PUSH_RDI_MEM_ASM) - 1)



static void unmap_function(void * jit_addr, uint64_t len) {
    uint8_t *start_address, *end_address;
    jit_addr = jit_addr - ENTRY_OFFSET;

    memset((uint8_t *) jit_addr, 0, len);

    start_address = (uint8_t *) ((uint64_t) jit_addr & ~0xfff);
    end_address = (uint8_t *) ((uint64_t) (jit_addr + len) & ~0xfff);

    assert(unmap_address(start_address) == 0);

    if (start_address != end_address) {
        assert(unmap_address(end_address) == 0);
    }

}


static void jit_function(void * rwx, void * jit_addr, void * copy_start, void * copy_end) {
    uint8_t *start_address, *end_address;
    uint64_t len = (uint64_t) (copy_end - copy_start);
    jit_addr = jit_addr - ENTRY_OFFSET;

    start_address = (uint8_t *) ((uint64_t) jit_addr & ~0xfff);
    end_address = (uint8_t *) ((uint64_t) (jit_addr + len) & ~0xfff);

    assert(map_address(start_address) == 0);


    if (start_address != end_address) {
        assert(map_address(end_address) == 0);
    }

    memcpy((uint8_t *) jit_addr, copy_start, len);

}

void clean_all(config * cfg) {

    unmap_function(cfg->paths[P_TRAINING].caller_start, CALLER_SIZE);
    unmap_function(cfg->paths[P_VICTIM].caller_start, CALLER_SIZE);
    unmap_function(cfg->target0, TARGET_SIZE);
    unmap_function(cfg->target1_mock, TARGET_SIZE);
    unmap_function(cfg->target1_near, TARGET_SIZE);
    unmap_function(cfg->target1_short, TARGET_SIZE);


    if (cfg->paths[P_EXTRA].branch_addr) {
        unmap_function(cfg->paths[P_EXTRA].caller_start, CALLER_SIZE);
        unmap_function(cfg->target2, TARGET_SIZE);
        unmap_function(cfg->target2_short, TARGET_SIZE);
        unmap_function(cfg->target2_near, TARGET_SIZE);

    }

    for (size_t i = 0; i < BTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].btb_eviction_set[i]) {
            unmap_function(cfg->paths[P_TRAINING].btb_eviction_set[i], CALLER_SIZE);
            unmap_function(cfg->paths[P_TRAINING].btb_eviction_targets[i], RET_TARGET_SIZE);
        }

        if(cfg->paths[P_VICTIM].btb_eviction_set[i]) {
            unmap_function(cfg->paths[P_VICTIM].btb_eviction_set[i], CALLER_SIZE);
            unmap_function(cfg->paths[P_VICTIM].btb_eviction_targets[i], RET_TARGET_SIZE);
        }
    }

    for (size_t i = 0; i < iBTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].ibtb_eviction[i]) {
            unmap_function(cfg->paths[P_TRAINING].ibtb_eviction[i], CALLER_SIZE);
        }

        if(cfg->paths[P_VICTIM].ibtb_eviction[i]) {
            unmap_function(cfg->paths[P_VICTIM].ibtb_eviction[i], CALLER_SIZE);
        }
    }

    assert_no_present_allocations();

}


void * get_random_address(uint8_t * rwx) {
    uint64_t rnd = get_45bit_random_value();

    return (void *) rwx + ((rnd % RANDOMIZE_SPACE) << RANDOMIZE_BIT_START) + ENTRY_OFFSET;
}

void * get_random_address_btb_alias(uint8_t * rwx, void * to_alias) {
    uint64_t address;

    address = (uint64_t) get_random_address(rwx);

    address &= ~TAG_SET_OFFSET_MASK;
    address += (uint64_t) to_alias & (TAG_SET_OFFSET_MASK);

    return (uint8_t *) address;
}

void * flip_bits_preserve_btb_tag_set(void * branch_address) {

    #if defined(INTEL_10_GEN) || defined(INTEL_9_GEN)
        branch_address = (void *) ((uint64_t) branch_address ^ ((1 << 16) | (1 << 24)));
    #elif defined(INTEL_11_GEN)
        branch_address = (void *) ((uint64_t) branch_address ^ ((1 << 16) | (1 << 26)));
    #endif

    return branch_address;

}

void update_path_caller_addresses(branch_path_t * path) {
    path->caller_start = path->branch_addr - CALLER_PATCH_END_OFFSET;
    path->caller_entry = path->caller_start + MAX_ENTRY_OFFSET - path->entry_offset;
}



void jit_targets(config * cfg) {

    assert(FR_BUF_SID == 0xdecafe);

    jit_function(cfg->base0, cfg->target0, target_start, target_end);
    memcpy((uint8_t *) cfg->target0 + TARGET_PATCH_OFF, LOAD_RCX_DECAFE_ASM, LOAD_RCX_DECAFE_SIZE);

    jit_function(cfg->base1, cfg->target1_mock, target_start, target_end);

    jit_function(cfg->base1, cfg->target1_short, target_start, target_end);
    memcpy((uint8_t *) cfg->target1_short + TARGET_PATCH_OFF, LOAD_RSI_DECAFE_ASM, LOAD_RSI_DECAFE_SIZE);

    jit_function(cfg->base1, cfg->target1_near, target_start, target_end);
    memcpy((uint8_t *) cfg->target1_near + TARGET_PATCH_OFF, LOAD_RDX_DECAFE_ASM, LOAD_RDX_DECAFE_SIZE);

    cfg->ftable1[0] = cfg->target1_mock;
    cfg->ftable1[1] = cfg->target1_short;
    cfg->ftable1[2] = cfg->target1_near;

    if (cfg->paths[P_EXTRA].branch_addr) {
        jit_function(cfg->base1, cfg->target2, target_start, target_end);
        memcpy((uint8_t *) cfg->target2 + TARGET_PATCH_OFF, LOAD_R10_DECAFE_ASM, LOAD_R10_DECAFE_SIZE);

        jit_function(cfg->base1, cfg->target2_short, target_start, target_end);
        memcpy((uint8_t *) cfg->target2_short + TARGET_PATCH_OFF, LOAD_R8_DECAFE_ASM, LOAD_R8_DECAFE_SIZE);

        jit_function(cfg->base1, cfg->target2_near, target_start, target_end);
        memcpy((uint8_t *) cfg->target2_near + TARGET_PATCH_OFF, LOAD_R9_DECAFE_ASM, LOAD_R9_DECAFE_SIZE);

    }

    if(cfg->prefetch_side_channel) {
        cfg->fr_buf[0] = (uint8_t *) cfg->target1_short;
        cfg->fr_buf[1] = (uint8_t *) cfg->target1_near;
        cfg->fr_buf[2] = (uint8_t *) cfg->target0;

        if (cfg->paths[P_EXTRA].branch_addr) {
            cfg->fr_buf[3] = (uint8_t *) cfg->target2_short;
            cfg->fr_buf[4] = (uint8_t *) cfg->target2_near;
            cfg->fr_buf[5] = (uint8_t *) cfg->target2;
        }
    }


    for (size_t i = 0; i < BTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].btb_eviction_set[i]) {
            jit_function(0, cfg->paths[P_TRAINING].btb_eviction_targets[i], ret_target_start, ret_target_end);
        }

        if(cfg->paths[P_VICTIM].btb_eviction_set[i]) {
            jit_function(0, cfg->paths[P_VICTIM].btb_eviction_targets[i], ret_target_start, ret_target_end);
        }
    }

}

void jit_callers(config * cfg) {

    update_path_caller_addresses(&cfg->paths[P_TRAINING]);
    update_path_caller_addresses(&cfg->paths[P_VICTIM]);
    if (cfg->paths[P_EXTRA].branch_addr) {
        update_path_caller_addresses(&cfg->paths[P_EXTRA]);
    }

    jit_function(cfg->base0, cfg->paths[P_TRAINING].caller_start, caller_start, caller_end);
    jit_function(cfg->base1, cfg->paths[P_VICTIM].caller_start, caller_start, caller_end);
    if (cfg->paths[P_EXTRA].branch_addr) {
        jit_function(cfg->base1, cfg->paths[P_EXTRA].caller_start, caller_start, caller_end);
    }

    for (size_t i = 0; i < BTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].btb_eviction_set[i]) {
            jit_function(0, cfg->paths[P_TRAINING].btb_eviction_set[i], caller_start, caller_end);
        }

        if(cfg->paths[P_VICTIM].btb_eviction_set[i]) {
            jit_function(0, cfg->paths[P_VICTIM].btb_eviction_set[i], caller_start, caller_end);
        }
    }

    for (size_t i = 0; i < iBTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].ibtb_eviction[i]) {
            jit_function(0, cfg->paths[P_TRAINING].ibtb_eviction[i], caller_start, caller_end);
        }

        if(cfg->paths[P_VICTIM].ibtb_eviction[i]) {
            jit_function(0, cfg->paths[P_VICTIM].ibtb_eviction[i], caller_start, caller_end);
        }
    }

}

static void jit_indirect_branch(void * caller, branch_type_t branch_type) {

    assert(branch_type == TRIGGER_INDIRECT_CALL || branch_type == TRIGGER_INDIRECT_JMP);

    if (branch_type == TRIGGER_INDIRECT_JMP) {
        memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, JMP_RDI_ASM, JMP_RDI_SIZE);
    } else if (branch_type == TRIGGER_INDIRECT_CALL) {
        memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, CALL_RDI_ASM, CALL_RDI_SIZE);
    }

}


static void jit_jump_to_target(void * caller, branch_type_t branch_type, void * target) {
    uint64_t offset;

    assert(branch_type == TRIGGER_DIRECT_UNCOND || branch_type == TRIGGER_DIRECT_COND || branch_type == TRIGGER_DIRECT_CALL);

    // e9: JMP, e8: CALL
    if (branch_type == TRIGGER_DIRECT_COND) {
        memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, REL_JC_BRANCH_ASM, REL_JC_BRANCH_ASM_SIZE);
    } else if (branch_type == TRIGGER_DIRECT_UNCOND) {
        memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, REL_BRANCH_ASM, REL_BRANCH_ASM_SIZE);
    } else if (branch_type == TRIGGER_DIRECT_CALL) {
        memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, REL_CALL_ASM, REL_CALL_ASM_SIZE);
    }

    offset = ((uint64_t) target) - ((uint64_t) caller + CALLER_PATCH_END_OFFSET + 1);
#ifdef LION_COVE
    // This is a hack to deal with LION_COVE btb indexing, see common.h
    offset += CALLER_DEFAULT_LAST_TARGET_OFFSET;
#endif
    memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 2, &offset, sizeof(uint32_t));

}

static int  jit_ret_to_target(void * caller) {
    memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET, PUSH_RDI_MEM_ASM, PUSH_RDI_MEM_SIZE);

}

static void jit_branch(branch_path_t * path, void * target) {

    switch (path->branch_type)
    {
    case TRIGGER_INDIRECT_CALL:
    case TRIGGER_INDIRECT_JMP:
        jit_indirect_branch(path->caller_start, path->branch_type);
        break;
    case TRIGGER_DIRECT_UNCOND:
    case TRIGGER_DIRECT_CALL:
    case TRIGGER_DIRECT_COND:
        jit_jump_to_target(path->caller_start, path->branch_type, target);
        break;
    case TRIGGER_RET:
        jit_ret_to_target(path->caller_start);
        break;
    default:
        break;
    }

}


void insert_targets_at_free_addresses(config * cfg, uint64_t target0_offset) {

    // select target0 such that we can place target1_short and
    // target1_near at the correct position
    uint64_t try = 0;
    while (1)
    {
        assert(try < 10000);

        cfg->target0 = cfg->paths[P_TRAINING].branch_addr + target0_offset - (try * 0x10);

        cfg->target1_short = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr) & SHORT_CALLER_MASK) + ((uint64_t) cfg->target0 & SHORT_TARGET_MASK);
        cfg->target1_near = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr)  & NEAR_CALLER_MASK) + ((uint64_t) cfg->target0 & NEAR_TARGET_MASK);

        if (cfg->paths[P_EXTRA].branch_addr) {
            // We want to set target2 at different lower bits then target0
            cfg->target2 = cfg->paths[P_EXTRA].branch_addr + (target0_offset ^ (1LU < 6 | 1LU < 7 | 1LU < 8)) + (try * 0x20);

            cfg->target2_short = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr) & SHORT_CALLER_MASK) + ((uint64_t) cfg->target2 & SHORT_TARGET_MASK);
            cfg->target2_near = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr)  & NEAR_CALLER_MASK) + ((uint64_t) cfg->target2 & NEAR_TARGET_MASK);

            if ((labs(cfg->target2_short - cfg->target1_short) <= CALLER_SIZE) ||
                (labs(cfg->target2_near - cfg->target1_near) <= CALLER_SIZE)) {
                try += 1;
                continue;
            }

        }

        // find spots for caller evicts
        for (size_t i = 0; i < BTB_EVICTION_SET_SIZE; i++)
        {
            if (cfg->paths[P_TRAINING].btb_eviction_set[i]) {
                cfg->paths[P_TRAINING].btb_eviction_targets[i] = cfg->paths[P_TRAINING].btb_eviction_set[i] + (1LU << 20);
            }
            if (cfg->paths[P_VICTIM].btb_eviction_set[i]) {
                cfg->paths[P_VICTIM].btb_eviction_targets[i] = cfg->paths[P_VICTIM].btb_eviction_set[i] + (1LU << 20);
            }
        }

        if (labs(cfg->paths[P_TRAINING].caller_start - cfg->target0) <= CALLER_SIZE) {
            try += 1;
            continue;
        }

        if (labs(cfg->paths[P_VICTIM].caller_start - cfg->target1_short) <= CALLER_SIZE){
            try += 1;
            continue;
        }

        if (labs(cfg->paths[P_VICTIM].caller_start  - cfg->target1_near) <= CALLER_SIZE){
            try += 1;
            continue;
        }

        if(cfg->prefetch_side_channel) {
            // place targets short at least a cache lines away
            // from other gadgets

            // target short
            if (labs(cfg->target1_short  - cfg->target1_mock) <= (CALLER_SIZE + 0x40)) {
                try += 1;
                continue;
            }

            if (labs(cfg->target1_short  - cfg->paths[P_VICTIM].caller_start) <= (CALLER_SIZE + 0x40)) {
                try += 1;
                continue;
            }

            if (cfg->target1_short != cfg->target1_near) {
                if (labs(cfg->target1_short  - cfg->target1_near) <= (CALLER_SIZE + 0x40)) {
                    try += 1;
                    continue;
                }
            }

            // target near
            if (labs(cfg->target1_near  - cfg->target1_mock) <= (CALLER_SIZE + 0x40)) {
                try += 1;
                continue;
            }

            if (labs(cfg->target1_near  - cfg->paths[P_VICTIM].caller_start) <= (CALLER_SIZE + 0x40)) {
                try += 1;
                continue;
            }

        }


        break;

    }

    // test if target1_short and target1_near collide
    if (labs(cfg->target1_near  - cfg->target1_short) <= TARGET_SIZE){
        cfg->target1_short = cfg->target1_near;
    }

    // place target1_mock at a free place
    try = 0;
    while (1)
    {
        cfg->target1_mock = cfg->paths[P_VICTIM].branch_addr + TARGET1_MOCK_OFFSET + (try * 0x10);

        if (labs(cfg->paths[P_VICTIM].caller_start - cfg->target1_mock) <= CALLER_SIZE){
            try += 1;
            continue;
        }

        if (labs(cfg->target1_short  - cfg->target1_mock) <= TARGET_SIZE){
            try += 1;
            continue;
        }

        if (labs(cfg->target1_near  - cfg->target1_mock) <= TARGET_SIZE){
            try += 1;
            continue;
        }


        break;

    }

    jit_targets(cfg);

    // JIT Jumps to targets

    jit_branch(&cfg->paths[P_TRAINING], cfg->target0);
    jit_branch(&cfg->paths[P_VICTIM], cfg->target1_mock);
    if (cfg->paths[P_EXTRA].branch_addr) {
        jit_branch(&cfg->paths[P_EXTRA], cfg->target2);
    }


    for (size_t i = 0; i < BTB_EVICTION_SET_SIZE; i++)
    {
        if(cfg->paths[P_TRAINING].btb_eviction_set[i]) {
            jit_jump_to_target(cfg->paths[P_TRAINING].btb_eviction_set[i], TRIGGER_DIRECT_UNCOND, cfg->paths[P_TRAINING].btb_eviction_targets[i]);
        }

        if(cfg->paths[P_VICTIM].btb_eviction_set[i]) {
            jit_jump_to_target(cfg->paths[P_VICTIM].btb_eviction_set[i], TRIGGER_DIRECT_UNCOND, cfg->paths[P_VICTIM].btb_eviction_targets[i]);
        }
    }

}

// ----------------------------------------------------------------------------
// JIT History

void create_random_jump_history_start_to_end(uint64_t * jmp_history, uint64_t * his_to_avoid, int start, int end) {

    assert(JIT_EXIT_SIZE >= REL_JC_BRANCH_ASM_SIZE);

    // creates random history, return jump entry

    uint8_t ok;
    uint64_t rnd;

    //Create a random ret_history
    for(int i=start; i< end; i++)
    {
        ok = 0;
        while(!ok)
        {
            ok = 1;
            rnd = get_45bit_random_value();
            rnd = rnd % (HISTORY_RWX_SIZE - JIT_EXIT_SIZE );

            // printf("RND: %lu\n", rnd);


            //Avoid self collision
            for(int j=0; j<i; j++) {
                if(labs((int64_t)(rnd - jmp_history[j])) <= JIT_EXIT_SIZE) {
                    ok = 0;
                    break;
                }
            }

            for(int j=end; j<HISTORY_JMP_SIZE; j++) {
                if(labs((int64_t)(rnd - jmp_history[j])) <= JIT_EXIT_SIZE) {
                    ok = 0;
                    break;
                }
            }

            for(int j=0; j<start; j++) {
                if(labs((int64_t)(rnd - jmp_history[j])) <= JIT_EXIT_SIZE) {
                    ok = 0;
                    break;
                }
            }


            if (his_to_avoid) {
                //Avoid collision with other history
                for(int j=0; j<HISTORY_JMP_SIZE; j++) {
                    if(labs((int64_t)(rnd - his_to_avoid[j])) <= JIT_EXIT_SIZE) {
                        ok = 0;
                        break;
                    }
                }

            }

        }

        jmp_history[i] = rnd;
    }

}

void create_random_jump_history(uint64_t * jmp_history, uint64_t * his_to_avoid) {
    return create_random_jump_history_start_to_end(jmp_history, his_to_avoid, 0, HISTORY_JMP_SIZE);
}

void clean_history_jit_chain(uint8_t * jit_buf, uint64_t * history) {

    for(int i=0; i<HISTORY_JMP_SIZE-1; i++)
    {
        memset(&jit_buf[history[i]], 0xcc, REL_JC_BRANCH_ASM_SIZE);
    }

    memset(&jit_buf[history[HISTORY_JMP_SIZE-1]], 0xcc, (uint64_t) (jit_exit_end - jit_exit_start));

}


void * insert_history_jmp_chain(uint8_t * jit_buf, uint64_t * history)
{

    for(int i=0; i<HISTORY_JMP_SIZE-1; i++)
    {

        uint32_t off = ((uint32_t) history[i+1]) - ((uint32_t) history[i] + REL_JC_BRANCH_OFFSET);
        memcpy(&jit_buf[history[i]], REL_JC_BRANCH_ASM, REL_JC_BRANCH_ASM_SIZE);
        memcpy(&jit_buf[history[i]+2], &off, sizeof(uint32_t));
    }

    // Insert indirect jmp
    uint8_t * last_jmp_addr =  &jit_buf[history[HISTORY_JMP_SIZE-1]];

    memcpy(last_jmp_addr, jit_exit_start, JIT_EXIT_SIZE);


    // jump chain entry
    return (void *) &jit_buf[history[0]];
}

uint8_t * insert_history_jmp_chain_length_n(uint8_t * jit_buf,  uint64_t * history, int len)
{

    for(int i=0; i < len-1; i++)
    {
        uint32_t off = ((uint32_t) history[i+1]) - ((uint32_t) history[i] + REL_JC_BRANCH_OFFSET);

        memcpy(&jit_buf[history[i]], REL_JC_BRANCH_ASM, REL_JC_BRANCH_ASM_SIZE);
        memcpy(&jit_buf[history[i]+2], &off, sizeof(uint32_t));
    }

    // Insert indirect jmp
    uint8_t * last_jmp_addr =  &jit_buf[history[len - 1]];

    memcpy(last_jmp_addr, jit_exit_start, JIT_EXIT_SIZE);


    // jump chain entry
    return &jit_buf[history[0]];
}


void randomize_history(history_t * history) {


    switch (history->type)
    {
    case HISTORY_NONE:
        break;

    case HISTORY_CONDITIONAL:
        randomize_history_cond(history->cond_array);
        break;

    case HISTORY_STATIC:
        set_history_cond_static(history->cond_array);
        break;

    case HISTORY_JIT:

        clean_history_jit_chain(history->jit_rwx, history->jit_array);
        create_random_jump_history(history->jit_array, 0);
        history->jit_entry = insert_history_jmp_chain(history->jit_rwx, history->jit_array);
        break;
    }

    return;
}

