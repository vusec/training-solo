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
 * Test Indirect Target Selection (ITS) training by evicting
 * the indirect branch iBTB entry
 *
 * For reference, single run hit results:
 * | CPU                    |  SHORT            NEAR             FAR         | SHORT EXTRA      NEAR EXTRA       FAR EXTRA
 * |------------------------|-------------------------------------------------------------------------------------------------|
 * | Intel Core i7-10700K   |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)|      0 (  0.0%)   9991 ( 99.9%)      0 (  0.0%)
 * | Intel Core i7-11700    |      0 (  0.0%)      0 (  0.0%)      0 (  0.0%)|      0 (  0.0%)   1133 ( 11.3%)      0 (  0.0%)
 * |                        |
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
   // First insert indirect BTB and iBTB entries from EXTRA training path
   execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_EXTRA], &cfg->target2);
   execute_path(cfg, &cfg->histories[P_VICTIM], &cfg->paths[P_EXTRA], &cfg->target2);

   execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
   execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

   evict_ibtb_entry_caller1(cfg, P_VICTIM);
}

static void randomize_branch_addresses(config * cfg)
{
   // place on upper half of the cache-line
   cfg->paths[P_TRAINING].branch_addr = (void *) (((uint64_t) cfg->paths[P_TRAINING].branch_addr) & ~((1LLU << 6) - 1));
   cfg->paths[P_TRAINING].branch_addr += 0x20 + (rand() % 0x20);
   cfg->paths[P_VICTIM].branch_addr = get_random_address_btb_alias(cfg->base1, cfg->paths[P_TRAINING].branch_addr);

   // place on lower half of the cache-line
   cfg->paths[P_VICTIM].branch_addr = (void *) (((uint64_t) cfg->paths[P_VICTIM].branch_addr) & ~((1LLU << 6) - 1));
   cfg->paths[P_VICTIM].branch_addr += rand() % 0x20;
   cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);

   // Extra training path
   cfg->paths[P_EXTRA].branch_addr = get_random_address_btb_alias(cfg->base2, cfg->paths[P_VICTIM].branch_addr);
   cfg->paths[P_EXTRA].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_EXTRA].branch_addr);
   init_addresses_ibtb_eviction_set(cfg->paths[P_VICTIM].ibtb_eviction, cfg->paths[P_VICTIM].branch_addr);
}

static void initialize_config(config * cfg)
{
   cfg->histories[P_TRAINING].type = HISTORY_NONE;
   cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

   cfg->paths[P_TRAINING].domain = DOMAIN_USER;
   cfg->paths[P_EXTRA].domain = DOMAIN_USER;
   cfg->paths[P_VICTIM].domain = DOMAIN_USER;

   cfg->paths[P_TRAINING].branch_type = TRIGGER_DIRECT_UNCOND;
   cfg->paths[P_EXTRA].branch_type = TRIGGER_INDIRECT_JMP;
   cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

   cfg->paths[P_TRAINING].entry_offset = 0;
   cfg->paths[P_EXTRA].entry_offset =   0;
   cfg->paths[P_VICTIM].entry_offset = 0;
   cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);

   randomize_branch_addresses(cfg);
}
