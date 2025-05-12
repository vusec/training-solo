/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _FLUSH_AND_RELOAD_H_
#define _FLUSH_AND_RELOAD_H_

#include <stdint.h>
#include "config.h"

void do_flush_and_reload(config * cfg, uint64_t hits[], uint64_t iterations);

void execute_path_in_user(config * cfg, history_t * history, branch_path_t * path, void * target);
void execute_path_in_kernel(config * cfg, history_t * history, branch_path_t * path, void * target);
void execute_path(config * cfg, history_t * history, branch_path_t * path, void * target);

void evict_btb_entry_caller0(struct config * cfg);
void evict_btb_entry_caller1(struct config * cfg);

void evict_ibtb_entry_caller0(struct config * cfg, int path_type);
void evict_ibtb_entry_caller1(struct config * cfg, int path_type);

#endif //_FLUSH_AND_RELOAD_H_
