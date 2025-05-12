#include "helper.h"
#include "targets.h"


void print_hits_header() {
    printf("\r%-14s %7s %7s %7s %7s %7s\n","", "SHORT","", "NEAR","", " FAR");
}


void print_hits(uint64_t hits[], char * text, uint64_t iterations) {

    uint64_t total_hits = 0;
    for (size_t i = 0; i < 3; i++)
    {
        total_hits += hits[i];
    }

    if(!total_hits) total_hits = iterations;


    printf("%2s %11s: %6ld (%5.1f%%) %6ld (%5.1f%%) %6ld (%5.1f%%) "
           " (/%lu)"
        "\n", "", text,
        hits[0], (double) hits[0] / (double) iterations * 100,
        hits[1], (double) hits[1] / (double) iterations * 100,
        hits[2], (double) hits[2] / (double) iterations * 100
        ,iterations
        );
}


uint32_t get_tag_for_address(void * caller) {
    uint64_t part1 = ((uint64_t) (caller) & TAG_MASK_PART_1) >> (TAG_PART_1_START);
    uint64_t part2 = ((uint64_t) (caller) & TAG_MASK_PART_2) >> (TAG_PART_2_START);
    return (uint32_t) (part1 ^ part2);

}

uint32_t get_set_for_address(void * caller) {
    return (uint32_t) ((((uint64_t) caller) & SET_MASK) >> (SET_START));
}

uint32_t get_offset_for_address(void * caller) {
    return (uint64_t) (caller) & ((1LU << 6) - 1);
}



void print_config(config * cfg)
{

    printf("\n");
    if (cfg->kvm_victim_indirect_branch_pc) {
        printf("%15s: %p \n", "KVM ind branch", cfg->kvm_victim_indirect_branch_pc);
        printf("%15s: %p \n", "KVM spec target", cfg->kvm_speculation_target_pc);
    }

    printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", "caller0",
        cfg->caller0 + CALLER_PATCH_END_OFFSET,
        get_tag_for_address(cfg->caller0 + CALLER_PATCH_END_OFFSET),
        get_set_for_address(cfg->caller0 + CALLER_PATCH_END_OFFSET),
        get_offset_for_address(cfg->caller0 + CALLER_PATCH_END_OFFSET));

    printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", "caller1", cfg->caller1 + CALLER_PATCH_END_OFFSET,
        get_tag_for_address(cfg->caller1  + CALLER_PATCH_END_OFFSET),
        get_set_for_address(cfg->caller1  + CALLER_PATCH_END_OFFSET),
        get_offset_for_address(cfg->caller1 + CALLER_PATCH_END_OFFSET));


    printf("%15s: %p (offset: 0x%lx)\n", "target0", cfg->target0, labs(cfg->target0 - cfg->caller0 - CALLER_PATCH_END_OFFSET));
    printf("%15s: %p (offset: 0x%lx)\n", "target1_mock", cfg->target1_mock, labs(cfg->target1_mock - cfg->caller1 - CALLER_PATCH_END_OFFSET));
    printf("%15s: %p (offset: 0x%lx)\n", "target1_short", cfg->target1_short, labs(cfg->target1_short - cfg->caller1 - CALLER_PATCH_END_OFFSET));
    printf("%15s: %p (offset: 0x%lx)", "target1_near", cfg->target1_near, labs(cfg->target1_near - cfg->caller1 - CALLER_PATCH_END_OFFSET));
    if (cfg->target1_short == cfg->target1_near) {
        printf("   (== target1_short)");
    }
    printf("\n");
    printf("%15s: 0x%lx\n", "entry_off_c0", cfg->caller0_entry_offset + 3);
    printf("%15s: 0x%lx\n", "entry_off_c1", cfg->caller1_entry_offset + 3);
}


void print_global_settings(config * cfg)
{
    printf("=================== GLOBAL SETTINGS ===================\n");

    printf("%22s: %2d | %22s: %2d\n", "Train in kernel", cfg->train_in_kernel, "Test in kernel", cfg->test_in_kernel);
    printf("%22s: %2d | %22s: %2d\n", "Caller0 History type", cfg->caller0_history_type, "Caller1 History type", cfg->caller1_history_type);
    printf("%22s: %2d | %22s: %2d\n", "Caller0 direct jump", cfg->caller0_jump_type, "Caller1 direct jump", cfg->caller1_jump_type);

    printf("=======================================================\n");

}

