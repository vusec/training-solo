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
 * This template tests if the BTB is updated if BTB is incorrect and iBTB a hit
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)   5584 ( 55.8%)      1 (  0.0%)
 * | Intel Core i7-11700    |      0 (  0.0%)   4740 ( 47.4%)      0 (  0.0%)
 * | Intel Core i9-14900K P |      0 (  0.0%)   1176 ( 11.8%)    125 (  1.2%)
 * | Intel Core 7 258V P    |      0 (  0.0%)   2196 ( 22.0%)    115 (  1.1%)
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_ibtb_entry_caller1(cfg, 0);
    evict_ibtb_entry_caller1(cfg, 1);
    evict_btb_entry_caller1(cfg);

    // We have a clean state

    // Insert correct iBTB Entry (iBTB=hit)
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);

    // we need to evict the pc entry, to make sure incorrect BTB entry is
    // inserted later
    evict_btb_entry_caller1(cfg);

    // Insert incorrect BTB entry (BTB=incorrect)
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target1_mock);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target1_mock);

    // Status: BTB = incorrect, iBTB = hit
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);

    // Now we have to evict the iBTB entry to infer if BTB was updated
    evict_ibtb_entry_caller1(cfg, 1);

    // Did we update BTB?
    // if hit on near: yes
    // If no hit: no
}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);

    init_address_evicting_branch(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
}

static void initialize_config(config * cfg)
{
    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain =   DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);
}
