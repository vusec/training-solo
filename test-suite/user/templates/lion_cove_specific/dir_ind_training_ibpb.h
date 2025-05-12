/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

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
 * This template tests Indirect Target Selection (ITS) training on Lion Cove by
 * aliasing a direct and indirect branch.
 * We execute IBPB after the training, which is bypassed.
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Ultra 7 258V           |      0 (  0.0%)   9987 ( 99.9%)      0 (  0.0%)
 * |                        |
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

    set_ibpb(cfg->cpu_nr);

}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);

}

static void initialize_config(config * cfg)
{
    cfg->histories[P_TRAINING].type = HISTORY_NONE;
    cfg->histories[P_VICTIM].type = HISTORY_NONE;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    if (rand() % 2 == 0) {
        cfg->paths[P_TRAINING].branch_type = rand() % 2 == 0 ? TRIGGER_DIRECT_COND : TRIGGER_DIRECT_UNCOND;
        cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;
    } else {
        cfg->paths[P_TRAINING].branch_type = TRIGGER_DIRECT_CALL;
        cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_CALL;
    }

    cfg->paths[P_TRAINING].entry_offset = 0;
    cfg->paths[P_VICTIM].entry_offset = 0;

    randomize_branch_addresses(cfg);
}
