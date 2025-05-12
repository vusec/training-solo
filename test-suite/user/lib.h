/*
 * Training Solo test suite
 * Friday, March 6th 2025
 *
 * Sander Wiebing - s.j.wiebing@vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _LIB_H_
#define _LIB_H_

#define THR 80

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static __always_inline __attribute__((always_inline)) void cpuid(void) { asm volatile ("xor %%rax, %%rax\ncpuid\n\t" ::: "%rax", "%rbx", "%rcx", "%rdx"); }
static __always_inline __attribute__((always_inline)) void flush(void * addr) { asm volatile ("clflush (%0)\n\t" :: "r"(addr):); }
static __always_inline __attribute__((always_inline)) void mfence(void) { asm volatile ("mfence\n\t":::); }
static __always_inline __attribute__((always_inline)) void lfence(void) { asm volatile ("lfence\n\t":::); }
static __always_inline __attribute__((always_inline)) void sfence(void) { asm volatile ("sfence\n\t":::); }

static __always_inline __attribute__((always_inline)) void maccess(void *p) {
        *(volatile unsigned char *)p;
}

static __always_inline __attribute__((always_inline)) uint64_t rdtscp(void) {
        uint64_t lo, hi;
        asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");
        return (hi << 32) | lo;
}

static __always_inline __attribute__((always_inline)) uint64_t load_time(void *p)
{
    uint64_t t0 = rdtscp();
    maccess(p);
    return rdtscp() - t0;
}

/* Write the MSR "reg" on cpu "cpu" */
static void wrmsr(uint32_t reg, int cpu, uint64_t data)
{
    int fd;
    char msr_file_name[128];

    sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);

    fd = open(msr_file_name, O_WRONLY);
    if (fd < 0) {
        printf( "wrmsr: can't open %s\n", msr_file_name);
        exit(1);
    }

    if (pwrite(fd, &data, sizeof(data), reg) != sizeof(data) ) {
        printf( "wrmsr: cannot write %s/0x%08x Err: %d\n", msr_file_name, reg, errno);
        exit(2);
     }

    close(fd);

    return;
}

#define MSR_IA32_PRED_CMD       0x00000049 /* Prediction Command */

static inline __attribute__((always_inline)) void set_ibpb(int cpu)
{
     wrmsr(MSR_IA32_PRED_CMD, cpu, 1);
     asm volatile("mfence");

}

#endif //_LIB_H_
