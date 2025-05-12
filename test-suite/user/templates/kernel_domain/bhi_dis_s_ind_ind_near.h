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
 * This template test the indirect branch prediction if BHI_DIS_S is enabled
 * and using a near offset (<32 bits)
 * We test with a different history for training and victim.
 *
 * For reference, single run hit results with BHI_DIS_S ENABLED:
 * | CPU                       |   SHORT            NEAR             FAR
 * |---------------------------|------------------------------------------------|
 * | Intel Core i9-14900K P    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)   (Raptor Cove)
 * | Intel Core i9-14900K E    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)   (Gracemont)
 * | Intel Core Ultra 7 155H P |      0 (  0.0%)   9990 ( 99.9%)      0 (  0.0%)   (Redwood Cove)
 * | Intel Core Ultra 7 155H E |      0 (  0.0%)   9998 (100.0%)      0 (  0.0%)   (Crestmont)
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);
}

static void initialize_config(config * cfg) {
    cfg->test_function = TEST_SINGLE_TEST;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_KERNEL;
    cfg->paths[P_VICTIM].domain = DOMAIN_KERNEL;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_CALL;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_CALL;

    cfg->target0_offset = 1LLU << 22;

    randomize_branch_addresses(cfg);
}
