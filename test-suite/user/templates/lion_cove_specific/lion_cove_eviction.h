#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#include "../../config.h"
#include "../../flush_and_reload.h"
#include "../../jitting.h"
#include "../../branch_evict.h"

/*
* This template shows on LION_COVE the set is determined by the entry address.
* However, if the branch address is not matching (like P_EXTRA), it is a mismatch
* and thus evicts it.
*/

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_ibtb_entry_caller1(cfg, P_VICTIM);
    evict_ibtb_entry_caller1(cfg, P_EXTRA);
    evict_btb_entry_caller1(cfg);

    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);

    execute_path(cfg, &cfg->histories[P_EVICTING], &cfg->paths[P_EXTRA], &cfg->target2);
    execute_path(cfg, &cfg->histories[P_EVICTING], &cfg->paths[P_EXTRA], &cfg->target2);


    evict_ibtb_entry_caller1(cfg, P_VICTIM);

}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_EXTRA].branch_addr = get_random_address_btb_alias(cfg->base2, cfg->paths[P_VICTIM].branch_addr);

    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
    init_addresses_btb_eviction_set(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);

    cfg->paths[P_VICTIM].entry_offset += 0;

    cfg->paths[P_EXTRA].entry_offset += 40;
    cfg->paths[P_EXTRA].branch_addr += 40;

}

static void initialize_config(config * cfg)
{
    cfg->prefetch_side_channel = 0;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_JIT;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_EXTRA].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_EXTRA].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);
}
