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
 * This template tests a prediction is still served from the iBTB if there is a
 * miss on the BTB
 *
 * For reference, single run hit results:
 * | CPU                    |   SHORT            NEAR             FAR
 * |------------------------|--------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)      0 (  0.0%)   9998 (100.0%)
 * | Intel Core i7-11700    |      0 (  0.0%)      0 (  0.0%)   9979 ( 99.8%)
 * | Intel Core i9-14900K P |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core 7 258V P    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 * | Intel Core 7 258V E    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_TRAINING], &cfg->target0);

    evict_btb_entry_caller1(cfg);

    // It looks like iBTB is still served on 11th gen. However, in fact,
    // the BTB is not correctly evicted on 11th if there is still an iBTB entry.
    // To test, uncomment below and see the BTB entry is still there.
    // On 10th gen it seems the BTB is correctly evicted.
    // To correctly evict both: first evict ibtb then btb (see evict_btb_ibtb).

    // evict_ibtb_entry_caller1(cfg, P_VICTIM);
}

static void randomize_branch_addresses(config * cfg)
{
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);


    init_addresses_btb_eviction_set(cfg->paths[P_VICTIM].btb_eviction_set, cfg->paths[P_VICTIM].branch_addr);

    // init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);

}

static void initialize_config(config * cfg)
{
    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_JMP;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);
}
