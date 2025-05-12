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
* The prefether does not check if the branch address matches, so prefetches
* the P_EXTRA target in our case.
*/


static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
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

    cfg->paths[P_EXTRA].branch_addr += 0x40;
    cfg->paths[P_EXTRA].entry_offset = 0x40;
}

static void initialize_config(config * cfg)
{
    cfg->prefetch_side_channel = 1;

    cfg->histories[P_TRAINING].type = HISTORY_JIT;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_EXTRA].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_DIRECT_UNCOND;
    cfg->paths[P_EXTRA].branch_type = TRIGGER_DIRECT_COND;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_DIRECT_UNCOND;

    randomize_branch_addresses(cfg);
}
