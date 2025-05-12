/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

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
#include "templates.h"


extern void fill_bhb_jmp(uint8_t *history, void * caller_function,
                     void * target_function, void * arg1);

extern void enter_jit_chain(void * jit_entry, void * caller_function,
                    void * target_function, void * fr_buf_p, void * do_rsb_underflow,
                    void * jmp_history);


static void execute_caller_in_kernel(config * cfg, history_t * history, uint8_t * caller_entry, void * target) {
    char buf[17];
    void * args[7];
    int ret;

    args[0] = enter_jit_chain;
    args[3] = target;
    args[4] = cfg->fr_buf_p;
    args[5] = 0;

    switch (history->type)
    {
    case HISTORY_NONE:
        args[1] = caller_entry;
        args[2] = NULL;
        args[5] = (void *) (uint64_t) history->id;
        args[6] = NULL;
        break;
    case HISTORY_STATIC:
    case HISTORY_CONDITIONAL:
        args[1] = fill_bhb_jmp;
        args[2] = caller_entry;
        args[6] = (void *) history->cond_array;
        break;
    case HISTORY_JIT:
        args[1] = history->jit_entry;
        args[2] = caller_entry;
        args[6] = NULL;
        break;
    }

    snprintf(buf, 17, "%lx", (size_t) args);

    ret = pwrite(cfg->fd_kernel_exec, buf, sizeof(buf), 0);

    if (ret <= 0) {
        printf("[ERROR] Kernel executing address %p failed: %s (%d)\n", caller_entry, strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

}

static void execute_caller_in_user(config * cfg, history_t * history, void * caller_entry, void * target) {

    switch (history->type)
    {
    case HISTORY_NONE:
        enter_jit_chain(caller_entry, NULL, target, cfg->fr_buf_p, (void *) (uint64_t) history->id, NULL);
        break;
    case HISTORY_STATIC:
    case HISTORY_CONDITIONAL:
        enter_jit_chain(fill_bhb_jmp, caller_entry, target, cfg->fr_buf_p, (void *) (uint64_t) history->id, history->cond_array);
        break;
    case HISTORY_JIT:
        enter_jit_chain(history->jit_entry, caller_entry, target, cfg->fr_buf_p, (void *) (uint64_t) history->id, NULL);
        break;
    }

    cpuid();
}

static void walk_caller_evict_list(struct config * cfg, history_t * history, void * caller_evict_list[], uint64_t entry_offset, int set_size) {

    for (size_t i = 0; i < set_size; i++)
    {
        if (caller_evict_list[i]) {
            void * entry = caller_evict_list[i] + MAX_ENTRY_OFFSET - entry_offset;
            execute_caller_in_user(cfg, history, entry,  &cfg->ftable1[0]);
            execute_caller_in_user(cfg, history, entry,  &cfg->ftable1[0]);
            execute_caller_in_user(cfg, history, entry,  &cfg->ftable1[0]);
            execute_caller_in_user(cfg, history, entry,  &cfg->ftable1[0]);
        }
    }

}

static void walk_caller_evict_list_target(struct config * cfg, history_t * history, void * caller_evict_list[], uint64_t entry_offset, int set_size, void * target_evict_list[]) {

    for (size_t i = 0; i < set_size * 2; i++)
    {
        int set_index = i % set_size;
        if (caller_evict_list[set_index]) {
            void * entry = caller_evict_list[set_index] + MAX_ENTRY_OFFSET - entry_offset;
            execute_caller_in_user(cfg, history, entry,  &target_evict_list[set_index]);
            execute_caller_in_user(cfg, history, entry,  &target_evict_list[set_index]);
        }
    }

}

void execute_path_in_kernel(config * cfg, history_t * history, branch_path_t * path, void * target)
{
    return execute_caller_in_kernel(cfg, history, path->caller_entry, target);
}

void execute_path_in_user(config * cfg, history_t * history, branch_path_t * path, void * target)
{
    return execute_caller_in_user(cfg, history, path->caller_entry, target);
}

void execute_path(config * cfg, history_t * history, branch_path_t * path, void * target) {
    if (path->domain == DOMAIN_USER) {
        execute_path_in_user(cfg, history, path, target);
    } else {
        execute_path_in_kernel(cfg, history, path, target);
    }
}

void evict_btb_entry_caller0(struct config * cfg) {
    // If set is empty, you should not call this function
    assert(cfg->paths[P_TRAINING].btb_eviction_set[0]);

    walk_caller_evict_list_target(cfg, &cfg->histories[P_EVICTING], cfg->paths[P_TRAINING].btb_eviction_set, cfg->paths[P_TRAINING].entry_offset, BTB_EVICTION_SET_SIZE, cfg->paths[P_TRAINING].btb_eviction_targets);
}

void evict_btb_entry_caller1(struct config * cfg) {
    // If set is empty, you should not call this function
    assert(cfg->paths[P_VICTIM].btb_eviction_set[0]);

    walk_caller_evict_list_target(cfg, &cfg->histories[P_EVICTING], cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].entry_offset, BTB_EVICTION_SET_SIZE, cfg->paths[P_VICTIM].btb_eviction_targets);
}

void evict_ibtb_entry_caller0(struct config * cfg, int path_type) {
    // If set is empty, you should not call this function
    assert(cfg->paths[P_TRAINING].ibtb_eviction[0]);

    walk_caller_evict_list(cfg, &cfg->histories[path_type], cfg->paths[P_TRAINING].ibtb_eviction, cfg->paths[P_TRAINING].entry_offset, iBTB_EVICTION_SET_SIZE);
}

void evict_ibtb_entry_caller1(struct config * cfg, int path_type) {
    // If set is empty, you should not call this function
    assert(cfg->paths[P_VICTIM].ibtb_eviction[0]);

    walk_caller_evict_list(cfg, &cfg->histories[path_type], cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].entry_offset, iBTB_EVICTION_SET_SIZE);
}



void do_flush_and_reload(config * cfg, uint64_t hits[], uint64_t iterations)
{

    uint64_t t;

    for (int i = 0; i < N_FR_BUF; i++){ hits[i] = 0; }

    set_ibpb(cfg->cpu_nr);

    for(int iter=0; iter < iterations; iter++) {

        do_train_function(cfg);

        flush(&cfg->ftable1[0]);
        for (int i = 0; i < N_FR_BUF; i++){ flush(cfg->fr_buf[i]); }

        execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_VICTIM], &cfg->ftable1[0]);

        for (int i = 0; i < N_FR_BUF; i++)
        {
            if (cfg->paths[P_EXTRA].branch_addr == 0 && i >= 3) {
                continue;
            }

            if (i >= 1 && cfg->fr_buf[i- 1] == cfg->fr_buf[i]) {
                continue;
            }
            t = load_time(cfg->fr_buf[i]);
            if(t < THR) {
                hits[i]++;
            }
        }

    }

}
