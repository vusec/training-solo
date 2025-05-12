#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#include "../../config.h"
#include "../../helper.h"
#include "../../flush_and_reload.h"
#include "../../jitting.h"
#include "../../branch_evict.h"

/**
 * Randomize experiment while we evict the iBTB to monitor the BTB
 *
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_btb_entry_caller1(cfg);

    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

    evict_ibtb_entry_caller1(cfg, 1);

}

static void randomize_branch_addresses(config * cfg)
{
    // randomize branch addresses
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    uint64_t victim = (uint64_t) cfg->base1;
    victim += ((uint64_t) cfg->paths[P_TRAINING].branch_addr & ((1LLU << RANDOMIZE_BIT_END) - 1));

    // randomize tag + set + offset bits
    uint64_t mask = rand() % 2 == 0 ? SET_OFFSET_MASK : TAG_SET_OFFSET_MASK;
    victim &= ~(mask);
    victim += get_45bit_random_value() & (mask);
    cfg->paths[P_VICTIM].branch_addr = (void *) victim;
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);

    // randomize branch type

    do {
        cfg->paths[P_TRAINING].branch_type = (branch_type_t) (rand() % N_BRANCH_TYPES);
        cfg->paths[P_VICTIM].branch_type = (branch_type_t) (rand() % N_BRANCH_TYPES);
    } while (
    #if 1
        // force two different branch types
        cfg->paths[P_TRAINING].branch_type == cfg->paths[P_VICTIM].branch_type
    #else
        0
    #endif
    );

    // randomize entry offset
    cfg->paths[P_TRAINING].entry_offset = rand() % MAX_ENTRY_OFFSET;
    cfg->paths[P_VICTIM].entry_offset = rand() % MAX_ENTRY_OFFSET;

    // randomize jump offset, either short or near
    if (rand() % 2 == 0) {
        cfg->target0_offset = rand() % (1LU << (N_SHORT_BITS - 1));
    } else {
        cfg->target0_offset = rand() % (1LU << (N_NEAR_BITS - 1));
    }

    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
    init_address_evicting_branch(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);


}

static void initialize_config(config * cfg)
{
    cfg->test_function = TEST_RANDOMIZE;

    cfg->histories[P_TRAINING].type = HISTORY_STATIC;
    cfg->histories[P_VICTIM].type = HISTORY_STATIC;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    randomize_branch_addresses(cfg);
}
