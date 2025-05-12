#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#include "../../config.h"
#include "../../flush_and_reload.h"
#include "../../jitting.h"
#include "../../branch_evict.h"

/**
 * Test BTB properties set bits
 * We create a eviction set. Next we flip the bits of the training and victim.
 * Once a bit flip results in a different set, we see a signal (its not evicted
 * by the eviction set anymore). We evict the iBTB to observe the BTB
 * speculation.
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_ibtb_entry_caller1(cfg, P_VICTIM);
    evict_ibtb_entry_caller1(cfg, P_TRAINING);

    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

    evict_btb_entry_caller1(cfg);
}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);

    uint64_t address = (uint64_t) cfg->base1;
    address += ((uint64_t) cfg->paths[P_TRAINING].branch_addr & ((1LLU << RANDOMIZE_BIT_END) - 1));
    cfg->paths[P_VICTIM].branch_addr = (void *) address;

    init_addresses_btb_eviction_set(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);

}

static void initialize_config(config * cfg)
{
    cfg->prefetch_side_channel = 0;
    cfg->test_function = TEST_BTB_SET_BITS;
    cfg->target0_offset = 1LLU << 16;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);

}
