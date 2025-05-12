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
 * This template shows on LION_COVE a branch can have multiple entires in
 * multiple BTB sets. Depending on which entry_offset you give to VICTIM path,
 * it will use the TRAINING path entry, EXTRA path entry, or none.
 * Showing that the aliasing EXTRA branch path does not evict the TRAINING path.
 *
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_btb_entry_caller1(cfg);
    evict_ibtb_entry_caller1(cfg, P_VICTIM);
    evict_ibtb_entry_caller1(cfg, P_TRAINING);
    evict_ibtb_entry_caller1(cfg, P_EVICTING);


    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

    execute_path(cfg, &cfg->histories[P_EVICTING], &cfg->paths[P_EXTRA], &cfg->target2);
    execute_path(cfg, &cfg->histories[P_EVICTING], &cfg->paths[P_EXTRA], &cfg->target2);

}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);

    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_EXTRA].branch_addr = get_random_address_btb_alias(cfg->base2, cfg->paths[P_VICTIM].branch_addr);


    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
    init_address_evicting_branch(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
}

static void initialize_config(config * cfg)
{
    cfg->prefetch_side_channel = 0;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_EXTRA].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_EXTRA].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    cfg->paths[P_TRAINING].entry_offset = 0x1;
    cfg->paths[P_EXTRA].entry_offset = 0x0;

    // Now select the offset for the victim, 0x1 for TRAINING entry and
    // 0x0 for VICTIM entry.
    cfg->paths[P_VICTIM].entry_offset = 0x1;

    randomize_branch_addresses(cfg);
}
