#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define PATH_K_MMAP "/proc/kmemory/kmmap"
#define PATH_K_MUNMAP "/proc/kmemory/kmunmap"

#define MAX_ALLOCATIONS 128
#define TASK_SIZE (1LU << 47)

// #define DEBUG


struct allocation {
    uint8_t * address;
    uint64_t ref;
} typedef allocation;

allocation allocations[MAX_ALLOCATIONS];
uint64_t n_allocations = 0;
uint64_t allocation_highest_idx = 0;


static int fd_k_map;
static int fd_k_munmap;

static int map_kernel_address(uint8_t * address)
{
    char buf[17];
    int ret;

    snprintf(buf, 17, "%lx", (size_t) address);
    ret = pwrite(fd_k_map, buf, 16, 0);

    if (ret <= 0) {
        printf("[ERROR] Mapping kernel address %p failed: %s (%d)\n", address, strerror(errno), errno);
        return errno;
    }

    return 0;

}

static int unmap_kernel_address(uint8_t * address)
{
    char buf[17];
    int ret;

    snprintf(buf, 17, "%lx", (size_t) address);
    ret = pwrite(fd_k_munmap, buf, 16, 0);

    if (ret <= 0) {
        printf("[ERROR] Unmapping kernel address %p failed: %s (%d)\n", address, strerror(errno), errno);
        return errno;
    }

    return 0;

}

static int map_user_address(uint8_t * address)
{
    void * ret;

    ret = mmap((void *) address, 0x1000, PROT_WRITE|PROT_READ|PROT_EXEC,
				MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE, -1, 0);

    if (ret == MAP_FAILED) {
        printf("[ERROR] Mapping user address %p failed: %s (%d)\n", address, strerror(errno), errno);
        return errno;
    }

    return 0;
}

static int unmap_user_address(uint8_t * address)
{
    int ret;

    ret = munmap((void *) address, 0x1000);

    if (ret != 0) {
        printf("[ERROR] Unmapping user address %p failed: %s (%d)\n", address, strerror(errno), errno);
        return errno;
    }

    return 0;
}

void assert_no_present_allocations(void) {
    uint8_t is_empty = 1;


    for (size_t idx = 0; idx < MAX_ALLOCATIONS; idx++)
    {
        // keep track of highest occupied idx to be faster in new calls
        if (allocations[idx].address) {
            printf("Existing allocation, address: %p ref: %lu", allocations[idx].address, allocations[idx].ref);
            is_empty = 0;
        }
    }

    assert(is_empty);
}



int map_address(uint8_t * address)
{
    // first we check if the allocation already exists
    int ret;
    uint64_t idx_highest_seen = 0;

#ifdef DEBUG
    printf("[INFO] Mapping address %p\n", address);
#endif

    for (size_t idx = 0; idx <= allocation_highest_idx; idx++)
    {
        // keep track of highest occupied idx to be faster in new calls
        if (allocations[idx].address) {
            idx_highest_seen = idx;
        }

        if (allocations[idx].address == address) {
            allocations[idx].ref += 1;
            return EXIT_SUCCESS;
        }
    }

    allocation_highest_idx = idx_highest_seen;

    // we have to create a new allocation
    assert(n_allocations < MAX_ALLOCATIONS);

    if ((uint64_t) address < TASK_SIZE) {
        ret = map_user_address(address);
    } else {
        ret = map_kernel_address(address);
    }

    if (ret != 0) {
        return ret;
    }

    // update allocation table, fist search for a empty spot

    for (size_t idx = 0; idx < MAX_ALLOCATIONS; idx++)
    {
        // keep track of highest occupied idx to be faster in new calls
        if (!allocations[idx].address) {
            allocations[idx].address = address;
            allocations[idx].ref = 1;
            n_allocations += 1;

            if (idx > allocation_highest_idx) {
                allocation_highest_idx = idx;
            }

            return EXIT_SUCCESS;
        }
    }

    printf("n_allocations: %lu\n", n_allocations);

    // we did not found a free spot, fail
    assert(0);

    return EXIT_FAILURE;

}

int unmap_address(uint8_t * address)
{
    int ret;

#ifdef DEBUG
    printf("[INFO] unmapping address %p\n", address);
#endif


    for (size_t idx = 0; idx <= allocation_highest_idx; idx++)
    {
        if (allocations[idx].address == address) {

            if (--allocations[idx].ref == 0) {

                allocations[idx].address = 0;
                n_allocations--;

                if(allocation_highest_idx == idx) {
                    allocation_highest_idx--;
                }

                if ((uint64_t) address < TASK_SIZE) {
                    ret = unmap_user_address(address);
                } else {
                    ret = unmap_kernel_address(address);
                }

                return ret;

            } else {
                return EXIT_SUCCESS;
            }
        }
    }

    return EINVAL;

}

int initialize_memory_controller()
{

    if (access(PATH_K_MMAP, F_OK) == 0) {
        fd_k_map = open(PATH_K_MMAP, O_WRONLY);
        assert(fd_k_map);
    } else {
        printf("Error: File %s not found\n", PATH_K_MMAP);
        exit(-1);
    }

    if (access(PATH_K_MUNMAP, F_OK) == 0) {
        fd_k_munmap = open(PATH_K_MUNMAP, O_WRONLY);
        assert(fd_k_munmap);
    } else {
        printf("Error: File %s not found\n", PATH_K_MUNMAP);
        exit(-1);
    }

}
