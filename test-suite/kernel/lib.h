
#define THR 80

__always_inline static void cpuid_fence(void) { asm volatile ("xor %%rax, %%rax\ncpuid\n\t" ::: "%rax", "%rbx", "%rcx", "%rdx"); }
__always_inline static void flush(void * addr) { asm volatile ("clflush (%0)\n\t" :: "r"(addr):); }
__always_inline static void mfence(void) { asm volatile ("mfence\n\t":::); }
__always_inline static void lfence(void) { asm volatile ("lfence\n\t":::); }
__always_inline static void sfence(void) { asm volatile ("sfence\n\t":::); }


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
