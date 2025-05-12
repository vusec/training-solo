#ifndef _LEAKAGE_RATE_H
#define _LEAKAGE_RATE_H

#include "common.h"

void leak_test_leakage_rate(struct config * cfg);
void test_kernel_leakage_rate(struct config * cfg);
void leak_dummy_secret(struct config * cfg);


#endif //_LEAKAGE_RATE_H
