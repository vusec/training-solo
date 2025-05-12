#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#include "../config.h"
#include "../flush_and_reload.h"
#include "../jitting.h"
#include "../branch_evict.h"

/**
 * This template tests Indirect Target Selection (ITS) training by evicting
 * the indirect branch BTB entry
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)   9996 (100.0%)      0 (  0.0%)
 * | Intel Core i7-11700    |      0 (  0.0%)   9998 (100.0%)      0 (  0.0%)
 * |                        |
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

    evict_btb_entry_caller1(cfg);
}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    // place on upper half of the cache-line
    cfg->paths[P_TRAINING].branch_addr = (void *) (((uint64_t) cfg->paths[P_TRAINING].branch_addr) & ~((1LLU << 6) - 1));
    cfg->paths[P_TRAINING].branch_addr += 0x20 + (rand() % 0x20);

    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    // place on lower half of the cache-line
    cfg->paths[P_VICTIM].branch_addr = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr) & ~((1LLU << 6) - 1));
    cfg->paths[P_VICTIM].branch_addr += rand() % 0x20;

    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);

    // If we train/test in kernel, we can use an evicting branch.
    // Train/test in user we have to use a eviction set.
    if(cfg->paths[P_TRAINING].domain == DOMAIN_KERNEL) {
        init_address_evicting_branch(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
    } else {
        init_addresses_btb_eviction_set(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
    }
}

static void initialize_config(config * cfg)
{
    cfg->histories[P_TRAINING].type = HISTORY_NONE;
    cfg->histories[P_VICTIM].type = HISTORY_NONE;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_DIRECT_UNCOND;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    cfg->paths[P_TRAINING].entry_offset = 0;
    cfg->paths[P_VICTIM].entry_offset = 0;

    randomize_branch_addresses(cfg);
}
