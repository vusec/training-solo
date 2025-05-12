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
 * This template tests if the iBTB is updated if both the BTB and iBTB
 * contain an incorrect entry.
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)    486 (  4.9%)   9513 ( 95.1%)
 * | Intel Core i7-11700    |      0 (  0.0%)     72 (  0.7%)   9919 ( 99.2%)
 * | Intel Core i9-14900K P |      0 (  0.0%)     21 (  0.2%)   9979 ( 99.8%)
 * | Intel Core 7 258V P    |      0 (  0.0%)     15 (  0.1%)   9962 ( 99.6%)
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_btb_entry_caller1(cfg);
    evict_ibtb_entry_caller1(cfg, 1);

    // We have a clean state

    // Insert incorrect BTB and iBTB entry (BTB=incorrect, iBTB=incorrect)
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target1_mock);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target1_mock);

    // Status: BTB = incorrect, iBTB = incorrect
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);

    // Did we update iBTB?
    // if hit on far: yes
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

