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
 * This template shows you can confuse indirect CALL/JMP/RET
 * if you randomize the history. Note: by randomizing the entry offset
 * or lower bits of the caller, as a result you also change the BHB value
 */

static __always_inline __attribute__((always_inline))
void do_train_function(config * cfg) {

    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);
    execute_path(cfg, &cfg->histories[P_TRAINING], &cfg->paths[P_TRAINING], &cfg->target0);

}


static void randomize_branch_addresses(config * cfg)
{
    // randomize branch addresses
    cfg->paths[P_TRAINING].branch_addr = get_random_address(cfg->base0);
    uint64_t victim = (uint64_t) cfg->base1;
    victim += ((uint64_t) cfg->paths[P_TRAINING].branch_addr & ((1LLU << RANDOMIZE_BIT_END) - 1));

    // randomize set + offset bits
    victim &= ~(SET_MASK | OFFSET_MASK);
    victim += get_45bit_random_value() & (SET_MASK | OFFSET_MASK);
    cfg->paths[P_VICTIM].branch_addr = (void *) victim;
    cfg->paths[P_VICTIM].branch_addr = flip_bits_preserve_btb_tag_set(cfg->paths[P_VICTIM].branch_addr);


    // randomize branch types
    branch_type_t branch_types_set[] = {TRIGGER_INDIRECT_CALL, TRIGGER_INDIRECT_JMP, TRIGGER_RET};
    do
    {
        cfg->paths[P_TRAINING].branch_type = branch_types_set[rand() % 3];
        cfg->paths[P_VICTIM].branch_type = branch_types_set[rand() % 3];
    } while (cfg->paths[P_TRAINING].branch_type == cfg->paths[P_VICTIM].branch_type);

    // randomize history types
    cfg->histories[P_TRAINING].type = (history_type_t) (rand() % N_HISTORY_TYPES);
    cfg->histories[P_VICTIM].type = (history_type_t) (rand() % N_HISTORY_TYPES);

    // randomize entry offset
    cfg->paths[P_TRAINING].entry_offset = rand() % MAX_ENTRY_OFFSET;
    cfg->paths[P_VICTIM].entry_offset = rand() % MAX_ENTRY_OFFSET;

    randomize_history(&cfg->histories[P_TRAINING]);
    randomize_history(&cfg->histories[P_VICTIM]);
}

static void initialize_config(config * cfg) {
    cfg->test_function = TEST_RANDOMIZE;

    cfg->paths[P_TRAINING].domain = DOMAIN_USER;
    cfg->paths[P_VICTIM].domain = DOMAIN_USER;

    cfg->paths[P_TRAINING].branch_type = TRIGGER_INDIRECT_CALL;
    cfg->paths[P_VICTIM].branch_type = TRIGGER_INDIRECT_JMP;

    randomize_branch_addresses(cfg);
}
