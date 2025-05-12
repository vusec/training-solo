#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "../mm.h"


int main(int argc, char **argv)
{
    uint8_t * address;
    int ret;

    initialize_memory_controller();

    address = (uint8_t *) 0xffff900000000000;

    for (size_t i = 0; i < 32; i++)
    {
        assert(map_address(address + (i * 0x1000)) == 0);
        assert(map_address(address + (i * 0x1000)) == 0);
    }

    *address = 0x91;

    printf("Wrote successfully to kern address! %x \n", *address);

    for (size_t i = 0; i < 32; i++)
    {
        assert(unmap_address(address + (i * 0x1000)) == 0);
        assert(unmap_address(address + (i * 0x1000)) == 0);
    }


    address = (uint8_t *) 0x610000000000;

    for (size_t i = 0; i < 32; i++)
    {
        assert(map_address(address + (i * 0x1000)) == 0);
        assert(map_address(address + (i * 0x1000)) == 0);
    }

    *address = 0x91;

    printf("Wrote successfully to user address! %x \n", *address);

    for (size_t i = 0; i < 32; i++)
    {
        assert(unmap_address(address + (i * 0x1000)) == 0);
        assert(unmap_address(address + (i * 0x1000)) == 0);
    }

    ret = map_address((uint8_t *) main);

    printf("Mapping existing address returns: %s (%d) \n", strerror(ret), ret);

}
