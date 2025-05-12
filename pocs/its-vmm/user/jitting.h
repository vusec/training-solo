#ifndef _JITTING_H_
#define _JITTING_H_

#include <stdint.h>
#include "common.h"

void insert_targets_at_free_addresses(config * cfg, uint64_t target0_offset);
void jit_callers(config * cfg);

void create_random_jump_history(uint64_t * jmp_history, uint64_t * his_to_avoid);
void create_random_jump_history_start_to_end(uint64_t * jmp_history, uint64_t * his_to_avoid, int start, int end);
void clean_history_jit_chain(uint8_t * jit_buf, uint64_t * history);

uint8_t * insert_history_jmp_chain(uint8_t * jit_buf, uint64_t * history);
uint8_t * insert_history_jmp_chain_length_n(uint8_t * jit_buf,  uint64_t * history, int len);

void randomize_history(config * cfg, uint8_t history_idx);

void * get_random_address(uint8_t * rwx);

void clean_all(config * cfg);

#endif //_JITTING_H_
