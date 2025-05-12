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
 * This template tests if the iBTB can be evicted from user domain with a
 * single aliasing branch.
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |
 * | Intel Core i7-11700    |
 * | Intel Core i9-14900K P |      0 (  0.0%)   9988 ( 99.9%)      0 (  0.0%)
 * | Intel Core 7 258V P    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core 7 258V E    |      0 (  0.0%)   2404 ( 24.0%)      0 (  0.0%)
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    evict_btb_entry_caller1(cfg);
    evict_ibtb_entry_caller1(cfg, 1);

    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);

    init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
    init_address_evicting_branch(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);
}

static void initialize_config(config * cfg)
{
    cfg->test_function = TEST_SINGLE_TEST;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_KERNEL;
    cfg->paths[P_VICTIM].domain = DOMAIN_KERNEL;

    // we can use either CALL or JUMP, the eviction set (JUMPS) evicts anyways
    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_CALL;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_CALL;

    randomize_branch_addresses(cfg);
}
