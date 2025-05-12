#ifndef _MM_H_
#define _MM_H_

#include <stdint.h>

int initialize_memory_controller();

int unmap_address(uint8_t * address);
int map_address(uint8_t * address);

void assert_no_present_allocations(void);


#endif //_MM_H_

