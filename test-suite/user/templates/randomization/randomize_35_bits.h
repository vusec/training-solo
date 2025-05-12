#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#include "../../config.h"
#include "../../helper.h"
#include "../../flush_and_reload.h"
#include "../../jitting.h"
#include "../../branch_evict.h"

/**
 * Randomize experiment.
 * Randomize bits >35 between train and victim
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg)
{
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
}

static void randomize_branch_addresses(config * cfg)
{
    // randomize branch addresses
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    uint64_t victim = (uint64_t) cfg->base1;
    victim += ((uint64_t) cfg->paths[P_TRAINING].branch_addr & ((1LLU << RANDOMIZE_BIT_END) - 1));

    // randomize bits
    victim &= ~((1LLU << 35) - 1);
    victim += get_45bit_random_value() & ((1LLU << 35) - 1);
    cfg->paths[P_VICTIM].branch_addr = (void *) victim;

    // randomize branch types
    cfg->paths[P_TRAINING].branch_type = (branch_type_t) (rand() % N_BRANCH_TYPES);
    cfg->paths[P_VICTIM].branch_type = (branch_type_t) (rand() % N_BRANCH_TYPES);

    // randomize history types
    cfg->histories[P_TRAINING].type = (history_type_t) (rand() % N_HISTORY_TYPES);
    cfg->histories[P_VICTIM].type = (history_type_t) (rand() % N_HISTORY_TYPES);

    // randomize entry offset
    cfg->paths[P_TRAINING].entry_offset = rand() % MAX_ENTRY_OFFSET;
    cfg->paths[P_VICTIM].entry_offset = rand() % MAX_ENTRY_OFFSET;

    // randomize jump offset, either short or near
    if (rand() % 2 == 0) {
        cfg->target0_offset = rand() % (1LU << (N_SHORT_BITS - 1));
    } else {
        cfg->target0_offset = rand() % (1LU << (N_NEAR_BITS - 1));
    }

    if (rand() % 2 == 0) {
        randomize_history(&cfg->histories[P_TRAINING]);
        randomize_history(&cfg->histories[P_VICTIM]);
    } else {
        randomize_history(&cfg->histories[P_TRAINING]);
        memcpy(cfg->histories[P_VICTIM].cond_array, cfg->histories[P_TRAINING].cond_array, MAX_HISTORY_SIZE * sizeof(uint64_t));
    }


}

static void initialize_config(config * cfg)
{
    cfg->test_function = TEST_RANDOMIZE;

    cfg->histories[P_TRAINING].type = HISTORY_CONDITIONAL;
    cfg->histories[P_VICTIM].type = HISTORY_CONDITIONAL;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    randomize_branch_addresses(cfg);
}
