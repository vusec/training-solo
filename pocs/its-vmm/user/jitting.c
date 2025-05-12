#include <string.h>
#include <assert.h>

#include "common.h"
#include "targets.h"
#include "helper.h"
#include "config.h"
#include "../../../common/kmmap/mm.h"

// ----------------------------------------------------------------------------
// ASM

// 48 8b 06     mov    rax, qword ptr [rsi]
#define LOAD_RSI_ASM "\x48\x8b\x06"
#define LOAD_RSI_SIZE (sizeof(LOAD_RSI_ASM) - 1)

// 48 8b 86
// fe ca de 00  mov    rax, qword ptr [rsi + decafe]
#define LOAD_RSI_DECAFE_ASM "\x48\x8b\x86\xfe\xca\xde\x00"
#define LOAD_RSI_DECAFE_SIZE (sizeof(LOAD_RSI_DECAFE_ASM) - 1)

// 48 8b 02     mov    rax, qword ptr [rdx]
#define LOAD_RDX_ASM "\x48\x8b\x02"
#define LOAD_RDX_SIZE (sizeof(LOAD_RDX_ASM) - 1)

// 48 8b 82
// fe ca de 00  mov    rax, qword ptr [rdx + decafe]
#define LOAD_RDX_DECAFE_ASM "\x48\x8b\x82\xfe\xca\xde\x00"
#define LOAD_RDX_DECAFE_SIZE (sizeof(LOAD_RDX_DECAFE_ASM) - 1)

// 48 8b 01     mov    rax, qword ptr [rcx]
#define LOAD_RCX_ASM "\x48\x8b\x01"
#define LOAD_RCX_SIZE (sizeof(LOAD_RCX_ASM) - 1)

// 48 8b 81
// fe ca de 00  mov    rax, qword ptr [rcx + decafe]
#define LOAD_RCX_DECAFE_ASM "\x48\x8b\x81\xfe\xca\xde\x00"
#define LOAD_RCX_DECAFE_SIZE (sizeof(LOAD_RCX_DECAFE_ASM) - 1)



// 0f ae e8     lfence
// 41 ff e3     jmp    r11
#define JMP_R11_ASM    "\x0f\xae\xe8\x41\xff\xe3\xcc"
#define JMP_R11_SIZE   (sizeof(JMP_R11_ASM) - 1)
#define JIT_EXIT_SIZE ((uint64_t) (jit_exit_end - jit_exit_start))


// e9 __ __ __ __   jmp ...
#define REL_BRANCH_ASM    "\xe9\x00\x00\x00\x00\xcc"
#define REL_BRANCH_OFFSET   5
#define REL_BRANCH_ASM_SIZE (sizeof(REL_BRANCH_ASM) - 1)

// 0f 82 __ __ __ __ jnz ...
#define REL_JC_BRANCH_ASM    "\x0f\x82\x00\x00\x00\x00\xcc"
#define REL_JC_BRANCH_OFFSET   6
#define REL_JC_BRANCH_ASM_SIZE (sizeof(REL_JC_BRANCH_ASM) - 1)

// c3                ret
#define RET_ASM    "\x90\x90\x90\x90\xc3\xcc"
#define RET_ASM_SIZE   (sizeof(RET_ASM) - 1)


#define PUSH_RDI_MEM_ASM "\xff\x37\xc3"
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

    unmap_function(cfg->caller0, CALLER_SIZE);
    unmap_function(cfg->caller1, CALLER_SIZE);
    unmap_function(cfg->target0, TARGET_SIZE);
    unmap_function(cfg->target1_mock, TARGET_SIZE);
    unmap_function(cfg->target1_near, TARGET_SIZE);
    unmap_function(cfg->target1_short, TARGET_SIZE);

}


void * get_random_address(uint8_t * rwx) {
    uint64_t rnd = get_45bit_random_value();

    return (void *) rwx + ((rnd % RANDOMIZE_SPACE) << RANDOMIZE_BIT_START) + ENTRY_OFFSET;
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

    for (size_t i = 0; i < MAX_CALLER_EVICT_PC_SET; i++)
    {
        if(cfg->caller_evict_list_pc[i]) {
            jit_function(0, cfg->target_evict_list_pc[i], ret_target_start, ret_target_end);
        }
    }

}

void jit_callers(config * cfg) {

    jit_function(cfg->base0, cfg->caller0, caller_start, caller_end);
    jit_function(cfg->base1, cfg->caller1, caller_start, caller_end);

    for (size_t i = 0; i < MAX_CALLER_EVICT_PC_SET; i++)
    {
        if(cfg->caller_evict_list_pc[i]) {
            jit_function(0, cfg->caller_evict_list_pc[i], caller_start, caller_end);
        }
    }

}

static int jit_jump_to_target(void * caller, void * target) {

    uint64_t offset = 0;

    offset = ((uint64_t) target) - ((uint64_t) caller + CALLER_PATCH_END_OFFSET + 1);

    // e9: JMP, e8: CALL
    memset((uint8_t *) caller + CALLER_PATCH_START_OFFSET, '\xe9', 1);
    memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 1, &offset, sizeof(uint32_t));

    // eb: 1 byte offset. Max 0x7e
    // assert(offset <= 0x70);
    // memset((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 3, '\xeb', 1);
    // memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 3 + 1, &offset, sizeof(uint8_t));

    // 74: je; 75: jne
    // memset((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 3, '\x74', 1);
    // memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 3 + 1, &offset, sizeof(uint8_t));

}

static int  jit_ret_to_target(void * caller) {

    memcpy((uint8_t *) caller + CALLER_PATCH_START_OFFSET + 2, PUSH_RDI_MEM_ASM, PUSH_RDI_MEM_SIZE);

}

void insert_targets_at_free_addresses(config * cfg, uint64_t target0_offset) {

    // select target0 such that we can place target1_short and
    // target1_near at the correct position
    uint64_t try = 0;
    while (1)
    {
        assert(try < 10000);

        if (cfg->kvm_speculation_target_pc) {
            cfg->target0 = (void *) ((uint64_t) cfg->caller0 & ~((1LU << 32) - 1)) + ((uint64_t) cfg->kvm_speculation_target_pc & ((1LU << 32) - 1));
        } else {
            cfg->target0 = cfg->caller0 + CALLER_PATCH_END_OFFSET + target0_offset + (try * 0x10);
        }

        cfg->target1_short = (void *) (((uint64_t) cfg->caller1 + CALLER_PATCH_END_OFFSET) & SHORT_CALLER_MASK) + ((uint64_t) cfg->target0 & SHORT_TARGET_MASK);
        cfg->target1_near = (void *) (((uint64_t) cfg->caller1 + CALLER_PATCH_END_OFFSET)  & NEAR_CALLER_MASK) + ((uint64_t) cfg->target0 & NEAR_TARGET_MASK);

        // find spots for caller evicts
        for (size_t i = 0; i < MAX_CALLER_EVICT_PC_SET; i++)
        {
            if (cfg->caller_evict_list_pc[i]) {
                cfg->target_evict_list_pc[i] = cfg->caller_evict_list_pc[i] + (1LU << 20);
            }
        }

        if (labs(cfg->caller1  - cfg->target1_short) <= CALLER_SIZE){
            try += 1;
            continue;
        }

        if (labs(cfg->caller1  - cfg->target1_near) <= CALLER_SIZE){
            try += 1;
            continue;
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
        cfg->target1_mock = cfg->caller1 + TARGET1_MOCK_OFFSET + (try * 0x10);

        if (labs(cfg->caller0  - cfg->target1_mock) <= CALLER_SIZE){
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

    if(cfg->caller0_jump_type == TRIGGER_DIRECT) {
        jit_jump_to_target(cfg->caller0, cfg->target0);
    } else if (cfg->caller0_jump_type == TRIGGER_RET) {
        jit_ret_to_target(cfg->caller0);
    }

    if(cfg->caller1_jump_type == TRIGGER_DIRECT) {
        jit_jump_to_target(cfg->caller1, cfg->target1_mock);
    } else if (cfg->caller1_jump_type == TRIGGER_RET)
    {
        jit_ret_to_target(cfg->caller1);
    }

    for (size_t i = 0; i < MAX_CALLER_EVICT_PC_SET; i++)
    {
        if(cfg->caller_evict_list_pc[i]) {
            jit_jump_to_target(cfg->caller_evict_list_pc[i], cfg->target_evict_list_pc[i]);
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


uint8_t * insert_history_jmp_chain(uint8_t * jit_buf, uint64_t * history)
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
    return &jit_buf[history[0]];
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


void randomize_history(config * cfg, uint8_t history_idx) {

    assert(history_idx <= 1);

    uint8_t history_type = history_idx == 0 ? cfg->caller0_history_type : cfg->caller1_history_type;

    switch (history_type)
    {
    case HISTORY_NONE:
        break;

    case HISTORY_STATIC_CONDITIONAL:
        randomize_history_static(history_idx == 0 ? cfg->history_0 : cfg->history_1);
        break;

    case HISTORY_JIT:

        if (history_idx == 0) {

            clean_history_jit_chain(cfg->history_jit_rwx_0, (uint64_t *) cfg->history_jit_0);
            create_random_jump_history((uint64_t *) cfg->history_jit_0, 0);
            cfg->history_jit_entry_0 = insert_history_jmp_chain(cfg->history_jit_rwx_0, (uint64_t *) cfg->history_jit_0);

        } else {

            clean_history_jit_chain(cfg->history_jit_rwx_1, (uint64_t *) cfg->history_jit_1);
            create_random_jump_history((uint64_t *) cfg->history_jit_1, 0);
            cfg->history_jit_entry_1 = insert_history_jmp_chain(cfg->history_jit_rwx_1, (uint64_t *) cfg->history_jit_1);

        }

        break;
    }

    return;
}

