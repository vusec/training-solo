#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "mitigations.h"

#define BIT_CET_IBT_SUPPORT 20


// Extended Feature Flags Enumeration Leaf | IA32_SPEC_CTRL
// LEAF 7, ECX = 0, OUT=EDX
#define BIT_IA32_ARCH_CAPABILITIES 29
#define BIT_IBRS_IBPB              26


// Extended Feature Flags Enumeration Leaf | IA32_SPEC_CTRL
// LEAF 7, ECX = 2, OUT=EDX
#define BIT_PSFD        0
#define BIT_IPRED_CTRL  1
#define BIT_RRSBA_CTRL  2
#define BIT_DDP_CTRL    3
#define BIT_BHI_CTRL    4
#define BIT_MCDT_NO     5

// IA32_ARCH_CAPABILITIES MSR
#define MSR_IA32_ARCH_CAPABILITIES 0x10a

#define BIT_IBRS_ALL    1
#define BIT_RSBA        2
#define BIT_RRSBA       19
#define BIT_BHI_NO      20

// IA32_SPEC_CTRL MSR
#define MSR_IA32_SPEC_CTRL 0x48

#define BIT_IBRS_MSR    0
#define BIT_IPRED_DIS_U 3
#define BIT_IPRED_DIS_S 4
#define BIT_RRSBA_DIS_U 5
#define BIT_RRSBA_DIS_S 6
#define BIT_PSFD_MSR        7
#define BIT_DDPD_U      8
#define BIT_BHI_DIS_S   10

#define MSR_IA32_U_CET			0x6a0 // user mode cet
#define MSR_IA32_S_CET			0x6a2 // kernel mode cet
#define CET_ENDBR_EN			2


static inline void read_cpuid( uint32_t eax_in, uint32_t ecx_in,
    uint32_t *eax_out, uint32_t *ebx_out, uint32_t *ecx_out, uint32_t *edx_out)
{

    asm volatile("cpuid"
          : "=a" (*eax_out), "=b" (*ebx_out), "=c" (*ecx_out), "=d" (*edx_out)
          : "a" (eax_in), "c" (ecx_in)
          // : "memory"
          );
}

static inline int get_bit(uint32_t value, uint8_t bit) {
    return (value >> bit) & 0x1;
}


/* Read the MSR "reg" on cpu "cpu" */
uint64_t rdmsr(uint32_t reg, int cpu)
{
	int fd;
	char msr_file_name[128];
	uint64_t data;

	sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);

	fd = open(msr_file_name, O_RDONLY);
	if (fd < 0) {
		printf( "rdmsr: can't open %s\n", msr_file_name);
		exit(1);
	}

	if ( pread(fd, &data, sizeof(data), reg) != sizeof(data) ) {
		printf( "rdmsr: cannot read %s/0x%08x\n", msr_file_name, reg);
		exit(2);
	}

	close(fd);

	return data;
}


void print_mitigation_info() {
    unsigned int eax, ebx, ecx, edx;

    printf("================== ENVIRONMENT INFO ===================\n");

    (system("lscpu | grep 'Model name' | awk '{$1=$1}1'") + 1);
    (system("echo -n 'Linux version: ' && uname -r") + 1);
    (system("echo 'Linux spectre_v2 mitigation info:' && echo -n '- ' && cat /sys/devices/system/cpu/vulnerabilities/spectre_v2 | cut -d' ' -f2-") + 1);

    printf("============== HARDWARE MITIGATION INFO ===============\n");


    read_cpuid(7, 0, &eax, &ebx, &ecx, &edx);

    if (get_bit(edx, BIT_IA32_ARCH_CAPABILITIES) != 1) {
        printf("IA32_ARCH_CAPABILITIES not supported\n");
        return;
    }


    // ------------------------------------------------------------------------
    // INTEL CET
    // LEAF 7, ECX = 0

    read_cpuid(7, 0, &eax, &ebx, &ecx, &edx);

    printf("%22s: %2d | %22s  %2s\n",
        "IBRS/IBPB Support",  get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_RRSBA_DIS_S),
        "", "");

    printf("%22s: %2d | %22s: %2d\n",
        "eIBRS Support", get_bit((uint32_t) rdmsr(MSR_IA32_ARCH_CAPABILITIES, 0), BIT_IBRS_ALL),
        "eIBRS Enabled", get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_IBRS_MSR));

    printf("%22s: %2d | %22s  %2s\n",
        "RSBA", get_bit((uint32_t) rdmsr(MSR_IA32_ARCH_CAPABILITIES, 0), BIT_RSBA),
        "", "");

    printf("%22s: %2d | %22s  %2s\n",
        "RRSBA", get_bit((uint32_t) rdmsr(MSR_IA32_ARCH_CAPABILITIES, 0), BIT_RRSBA),
        "", "");


    int ibt_support = get_bit(edx, BIT_CET_IBT_SUPPORT);
    printf("%22s: %2d | %22s: %2d\n",
        "CET_IBT Support", get_bit(edx, BIT_CET_IBT_SUPPORT) ,
        "IBT Kernel-mode", ibt_support ? get_bit((uint32_t) rdmsr(MSR_IA32_S_CET, 0), CET_ENDBR_EN) : 0);

    printf("%22s  %2s | %22s: %2d\n",
        "", "",
        "IBT User-mode", ibt_support ? get_bit((uint32_t) rdmsr(MSR_IA32_U_CET, 0), CET_ENDBR_EN) : 0);




    // ------------------------------------------------------------------------
    // Extended Feature Flags Enumeration Leaf | IA32_SPEC_CTRL
    // LEAF 7, ECX = 2

    read_cpuid(7, 2, &eax, &ebx, &ecx, &edx);

    printf("%22s: %2d | %22s: %2d\n", "IPRED_CTRL",
        get_bit(edx, BIT_IPRED_CTRL) , "IPRED_DIS_S", 0);

    printf("%22s: %2d | %22s: %2d\n",
        "RRSBA_CTRL", get_bit(edx, BIT_RRSBA_CTRL) ,
        "RRSBA_DIS_S", get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_RRSBA_DIS_S));

    printf("%22s  %2s | %22s: %2d\n",
        "", "",
        "RRSBA_DIS_U", get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_RRSBA_DIS_U));

    printf("%22s: %2d | %22s: %2d\n",
        "DDP_CTRL", get_bit(edx, BIT_DDP_CTRL),
        "DDPD_U", get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_DDPD_U));

    printf("%22s: %2d | %22s: %2d\n",
        "BHI_CTRL", get_bit(edx, BIT_BHI_CTRL),
        "BHI_DIS_S", get_bit((uint32_t) rdmsr(MSR_IA32_SPEC_CTRL, 0), BIT_BHI_DIS_S));

    printf("%22s: %2d | %22s  %2s\n",
        "BHI_NO", get_bit((uint32_t) rdmsr(MSR_IA32_ARCH_CAPABILITIES, 0), BIT_BHI_NO),
        "", "");

}
