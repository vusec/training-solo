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
 * This template shows you cannot train CALLS with JUMPS (and vice versa) for
 * both BTB and iBTB if the history of both is equal. Note it is possible
 * while randomizing the history (see randomization tests).
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core i7-11700    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core i9-14900K P |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core 7 258V P    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core 7 258V E    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
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

    cfg->histories[P_TRAINING].type = HISTORY_STATIC;
    cfg->histories[P_VICTIM].type = HISTORY_STATIC;

    cfg->paths[P_TRAINING].domain = DOMAIN_KERNEL;
    cfg->paths[P_VICTIM].domain = DOMAIN_KERNEL;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_CALL;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);
}
