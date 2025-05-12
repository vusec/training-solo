#ifndef _TEMPLATES_H_
#define _TEMPLATES_H_

#if defined(ind_ind_history_collision)
    #include "templates/ind_ind_history_collision.h"
#elif defined(ind_ind_history_evict_btb_ibtb)
    #include "templates/ind_ind_history_evict_btb_ibtb.h"
#elif defined(ind_ind_history_evict_btb)
    #include "templates/ind_ind_history_evict_btb.h"
#elif defined(ind_ind_history_evict_ibtb)
    #include "templates/ind_ind_history_evict_ibtb.h"
#elif defined(dir_ind_evict_ind_btb)
    #include "templates/dir_ind_evict_ind_btb.h"
#elif defined(ind_ind_call_jmp_collision)
    #include "templates/ind_ind_call_jmp_collision.h"

#elif defined(test_btb_tag_set)
    #include "templates/table3_btb_properties/test_btb_tag_set.h"
#elif defined(test_btb_tag_set_prefetch)
    #include "templates/table3_btb_properties/test_btb_tag_set_prefetch.h"
#elif defined(test_btb_identify_set_bits)
    #include "templates/table3_btb_properties/test_btb_identify_set_bits.h"
#elif defined(test_btb_identify_set_bits_prefetch)
    #include "templates/table3_btb_properties/test_btb_identify_set_bits_prefetch.h"
#elif defined(test_btb_tag_set_evicting_branch)
    #include "templates/table3_btb_properties/test_btb_tag_set_evicting_branch.h"
// -----------------------------------------------------------------------------
// Table 2 - Update policy BTB/iBTB

#elif defined(ind_ind_btb_hit_ibtb_incorrect)
    #include "templates/table2_update_policy/ind_ind_btb_hit_ibtb_incorrect.h"
#elif defined(ind_ind_btb_hit_ibtb_miss)
    #include "templates/table2_update_policy/ind_ind_btb_hit_ibtb_miss.h"
#elif defined(ind_ind_btb_incorrect_ibtb_hit)
    #include "templates/table2_update_policy/ind_ind_btb_incorrect_ibtb_hit.h"
#elif defined(ind_ind_btb_incorrect_ibtb_incorrect_test_btb)
    #include "templates/table2_update_policy/ind_ind_btb_incorrect_ibtb_incorrect_test_btb.h"
#elif defined(ind_ind_btb_incorrect_ibtb_incorrect_test_ibtb)
    #include "templates/table2_update_policy/ind_ind_btb_incorrect_ibtb_incorrect_test_ibtb.h"
#elif defined(ind_ind_btb_incorrect_ibtb_miss_test_btb)
    #include "templates/table2_update_policy/ind_ind_btb_incorrect_ibtb_miss_test_btb.h"
#elif defined(ind_ind_btb_incorrect_ibtb_miss_test_ibtb)
    #include "templates/table2_update_policy/ind_ind_btb_incorrect_ibtb_miss_test_ibtb.h"
#elif defined(ind_ind_btb_mis_ibtb_hit)
    #include "templates/table2_update_policy/ind_ind_btb_mis_ibtb_hit.h"
#elif defined(ind_ind_btb_miss_ibtb_miss_test_btb)
    #include "templates/table2_update_policy/ind_ind_btb_miss_ibtb_miss_test_btb.h"
#elif defined(ind_ind_btb_miss_ibtb_miss_test_ibtb)
    #include "templates/table2_update_policy/ind_ind_btb_miss_ibtb_miss_test_ibtb.h"
#elif defined(ind_ind_btb_miss_ibtb_incorrect_test_btb)
    #include "templates/table2_update_policy/ind_ind_btb_miss_ibtb_incorrect_test_btb.h"
#elif defined(ind_ind_btb_miss_ibtb_incorrect_test_ibtb)
    #include "templates/table2_update_policy/ind_ind_btb_miss_ibtb_incorrect_test_ibtb.h"

// -----------------------------------------------------------------------------
// Table 4 - Indirect Target Selection (ITS) Prediction scenarios

#elif defined(dir_ind_btb_hit_ibtb_hit)
    #include "templates/table4_its_prediction/dir_ind_btb_hit_ibtb_hit.h"
#elif defined(dir_ind_btb_hit_ibtb_miss)
    #include "templates/table4_its_prediction/dir_ind_btb_hit_ibtb_miss.h"
#elif defined(dir_ind_btb_miss_ibtb_hit)
    #include "templates/table4_its_prediction/dir_ind_btb_miss_ibtb_hit.h"
#elif defined(dir_ind_btb_miss_ibtb_miss)
    #include "templates/table4_its_prediction/dir_ind_btb_miss_ibtb_miss.h"

// -----------------------------------------------------------------------------
// Test explicit for kernel domain

// BHI_DIS_S Testing
#elif defined(bhi_dis_s_ind_ind_near)
#include "templates/kernel_domain/bhi_dis_s_ind_ind_near.h"
#elif defined(bhi_dis_s_ind_ind_far)
#include "templates/kernel_domain/bhi_dis_s_ind_ind_far.h"
#elif defined(evict_ibtb_with_evict_branch)
#include "templates/kernel_domain/evict_ibtb_with_evict_branch.h"
#elif defined(evict_ibtb_with_evict_set)
#include "templates/kernel_domain/evict_ibtb_with_evict_set.h"

// -----------------------------------------------------------------------------
// Lion Cove Experiments

#elif defined(lion_cove_multiple_sets)
    #include "templates/lion_cove_specific/lion_cove_multiple_sets.h"
#elif defined(lion_cove_multiple_sets_prefetch)
    #include "templates/lion_cove_specific/lion_cove_multiple_sets_prefetch.h"
#elif defined(lion_cove_eviction)
    #include "templates/lion_cove_specific/lion_cove_eviction.h"
#elif defined(lion_cove_eviction_prefetch)
    #include "templates/lion_cove_specific/lion_cove_eviction_prefetch.h"
#elif defined(dir_ind_training)
    #include "templates/lion_cove_specific/dir_ind_training.h"
#elif defined(dir_ind_training_ibpb)
    #include "templates/lion_cove_specific/dir_ind_training_ibpb.h"



// -----------------------------------------------------------------------------
// Randomization experiments

#elif defined(randomize_set_bits)
    #include "templates/randomization/randomize_set_bits.h"
#elif defined(randomize_tag_bits)
    #include "templates/randomization/randomize_tag_bits.h"
#elif defined(randomize_35_bits)
    #include "templates/randomization/randomize_35_bits.h"
#elif defined(randomize_evict_ibtb)
    #include "templates/randomization/randomize_evict_ibtb.h"
#elif defined(randomize_call_jmp)
    #include "templates/randomization/randomize_call_jmp.h"
#else
    #define initialize_config(cfg) 1
    #define do_train_function(cfg) 1
    #define randomize_branch_addresses(cfg) 1
    #error "Test not defined or unkown"
#endif


#endif // _TEMPLATES_H_
