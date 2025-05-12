/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#include "helper.h"
#include "targets.h"


void print_hits_header(config * cfg) {

    if (cfg->paths[P_EXTRA].branch_addr != 0) {
        printf("\r%-18s %7s %7s %7s %7s %7s "
            "%7s | %11s %3s %11s %3s %11s\n",
            "","SHORT","", "NEAR","", "FAR",
            "","SHORT EXTRA","", "NEAR EXTRA","", "FAR EXTRA");

    } else {
        printf("\r%-18s %7s %7s %7s %7s %7s\n","", "SHORT","", "NEAR","", "FAR");
    }
}


void print_hits(config * cfg, uint64_t hits[], char * text, uint64_t iterations) {

    // check if extra path is enabled
    if (cfg->paths[P_EXTRA].branch_addr != 0) {
        printf("%2s %15s: %6ld (%5.1f%%) %6ld (%5.1f%%) %6ld (%5.1f%%)"
            "| %6ld (%5.1f%%) %6ld (%5.1f%%) %6ld (%5.1f%%) "
            " (/%lu)"
            "\n", "", text,
            hits[0], (double) hits[0] / (double) iterations * 100,
            hits[1], (double) hits[1] / (double) iterations * 100,
            hits[2], (double) hits[2] / (double) iterations * 100,
            hits[3], (double) hits[3] / (double) iterations * 100,
            hits[4], (double) hits[4] / (double) iterations * 100,
            hits[5], (double) hits[5] / (double) iterations * 100,
            iterations
            );


    } else {
        printf("%2s %15s: %6ld (%5.1f%%) %6ld (%5.1f%%) %6ld (%5.1f%%) "
            " (/%lu)"
            "\n", "", text,
            hits[0], (double) hits[0] / (double) iterations * 100,
            hits[1], (double) hits[1] / (double) iterations * 100,
            hits[2], (double) hits[2] / (double) iterations * 100,
            iterations
            );

    }
}


uint32_t get_tag_for_address(void * address) {
    uint64_t part1 = ((uint64_t) (address) & TAG_MASK_PART_1) >> (TAG_PART_1_START);

    if (TAG_PART_2_START != 0) {
        uint64_t part2 = ((uint64_t) (address) & TAG_MASK_PART_2) >> (TAG_PART_2_START);
        return (uint32_t) (part1 ^ part2);
    } else {
        return part1;
    }

}

uint32_t get_set_for_address(void * address) {
    return (uint32_t) ((((uint64_t) address) & SET_MASK) >> (SET_START));
}

uint32_t get_offset_for_address(void * address) {
    return (uint64_t) (address) & ((1LU << 6) - 1);
}

void print_branch_info(void * branch_addr, char * desc) {

    printf("%15s: %p (tag: 0x%03x, set: 0x%03x, PC[5:0]: 0x%03x)\n", desc, branch_addr,
        get_tag_for_address(branch_addr),
        get_set_for_address(branch_addr),
        get_offset_for_address(branch_addr));
}

void print_eviction_set(void ** evict_list, int size, char * desc) {
    for (int i = 0; i < size; i++)
    {
        if (evict_list[i]) {
            print_branch_info(evict_list[i] + CALLER_PATCH_END_OFFSET, desc);
        }
    }
}



void print_config(config * cfg)
{

    printf("\n");

    print_branch_info(cfg->paths[P_TRAINING].branch_addr, "training");
    if (cfg->paths[P_EXTRA].branch_addr) {
        print_branch_info(cfg->paths[P_EXTRA].branch_addr, "extra_training");
    }
    print_branch_info(cfg->paths[P_VICTIM].branch_addr, "victim");

    printf("\n");
    printf("%15s: %p (offset: 0x%lx)\n", "target0", cfg->target0, labs(cfg->target0 - cfg->paths[P_TRAINING].branch_addr));
    printf("%15s: %p (offset: 0x%lx)\n", "target1_mock", cfg->target1_mock, labs(cfg->target1_mock - cfg->paths[P_VICTIM].branch_addr));
    printf("%15s: %p (offset: 0x%lx)\n", "target1_short", cfg->target1_short, labs(cfg->target1_short - cfg->paths[P_VICTIM].branch_addr));
    printf("%15s: %p (offset: 0x%lx)", "target1_near", cfg->target1_near, labs(cfg->target1_near - cfg->paths[P_VICTIM].branch_addr));
    if (cfg->target1_short == cfg->target1_near) {
        printf("   (== target1_short)");
    }
    printf("\n");

    if (cfg->paths[P_EXTRA].branch_addr) {
        printf("%15s: %p (offset: 0x%lx)\n", "target2", cfg->target2, labs(cfg->target2 - cfg->paths[P_EXTRA].branch_addr));
        printf("%15s: %p (offset: 0x%lx)\n", "target2_short", cfg->target2_short, labs(cfg->target2_short - cfg->paths[P_VICTIM].branch_addr));
        printf("%15s: %p (offset: 0x%lx)", "target2_near", cfg->target2_near, labs(cfg->target2_near - cfg->paths[P_VICTIM].branch_addr));
        if (cfg->target1_short == cfg->target1_near) {
            printf("   (== target2_short)");
        }
        printf("\n");
    }
    printf("%15s: 0x%lx\n", "entry_off_c0", cfg->paths[P_TRAINING].entry_offset + 3);
    printf("%15s: 0x%lx\n", "entry_off_c1", cfg->paths[P_VICTIM].entry_offset + 3);
    if (cfg->paths[P_EXTRA].branch_addr) {
        printf("%15s: 0x%lx\n", "entry_off_c2", cfg->paths[P_EXTRA].entry_offset + 3);
    }

    printf("\n");
    print_eviction_set(cfg->paths[P_TRAINING].btb_eviction_set, BTB_EVICTION_SET_SIZE, "BTB Evict set");
    print_eviction_set(cfg->paths[P_VICTIM].btb_eviction_set, BTB_EVICTION_SET_SIZE, "BTB Evict set");
    print_eviction_set(cfg->paths[P_TRAINING].ibtb_eviction, iBTB_EVICTION_SET_SIZE, "iBTB Evict set");
    print_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, iBTB_EVICTION_SET_SIZE, "iBTB Evict set");

    printf("\n");


}


void print_global_settings(config * cfg)
{
    printf("=========================== MAIN CONFIGURATION ===========================\n");

    printf("%22s: %8s | %22s: %8s\n", "Training Domain", domain_str[cfg->paths[P_TRAINING].domain], "Victim Domain", domain_str[cfg->paths[P_VICTIM].domain]);
    printf("%22s: %8s | %22s: %8s\n", "Training History type", history_type_str[cfg->histories[P_TRAINING].type], "Victim History type", history_type_str[cfg->histories[P_VICTIM].type]);
    printf("%22s: %8s | %22s: %8s\n", "Training Branch Type", branch_type_str[cfg->paths[P_TRAINING].branch_type], "Victim Branch Type", branch_type_str[cfg->paths[P_VICTIM].branch_type]);
    printf("%22s: %8d | %22s  %8s\n", "Prefetch side-channel", cfg->prefetch_side_channel, "", "");

    printf("==========================================================================\n");

}

