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


MODULE_AUTHOR("Sander Wiebing");
MODULE_DESCRIPTION("KMMAP Kernel Module");
MODULE_LICENSE("GPL");

// #define VERBOSE


static unsigned long *get_pte(void *virt_addr)
{
	unsigned long va = (unsigned long)virt_addr;
	unsigned long *r = NULL;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset(current->mm, va);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return NULL;

	p4d = p4d_offset(pgd, va);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return NULL;

	pud = pud_offset(p4d, va);
	if (pud_none(*pud))
		return NULL;

	if (pud_bad(*pud))
		return NULL;


	pmd = pmd_offset(pud, va);
	if (pmd_none(*pmd))
		return NULL;


	if (pmd_trans_huge(*pmd)) {
		return (unsigned long *)pmd;
	}


	if (pmd_bad(*pmd))
		return NULL;


	pte = pte_offset_kernel(pmd, va);
	if (!pte_none(*pte)) {
		r = (unsigned long *)pte;
	}

	pte_unmap(pte);

	return r;
}

static int insert_pte(void *virt_addr, struct page * p, pgprot_t pgprot, uint8_t set_user_flag)
{
	unsigned long va = (unsigned long)virt_addr;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	pte_t pte;

	pgd = pgd_offset(current->mm, va);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		return -1;
	}

	p4d = p4d_offset(pgd, va);
	if (p4d_none(*p4d)) {

		unsigned long new_pud_page = get_zeroed_page(GFP_KERNEL);
		if (!new_pud_page)
			return -1;
		set_p4d(p4d, __p4d(__pa(new_pud_page) | _KERNPG_TABLE));
	}

	if (p4d_bad(*p4d)) {
		return -1;
	}


	pud = pud_offset(p4d, va);
	if (pud_none(*pud)) {

		unsigned long new_pmd_page  = get_zeroed_page(GFP_KERNEL);

		if (!new_pmd_page)
			return -1;
		set_pud(pud, __pud(__pa(new_pmd_page) | _KERNPG_TABLE));
	}

	if (pud_bad(*pud)) {
		return -1;
	}


	pmd = pmd_offset(pud, va);
	if (pmd_none(*pmd)) {
		unsigned long new_pte_page = get_zeroed_page(GFP_KERNEL);
		if (!new_pte_page)
			return -1;
		set_pmd(pmd, __pmd(__pa(new_pte_page) | _KERNPG_TABLE));
	}


	if (pmd_trans_huge(*pmd)) {
		return -1;
	}


	if (pmd_bad(*pmd)) {
		return -1;
	}

	ptep = pte_offset_kernel(pmd, va);

	if (!pte_none(*ptep)) {
		return -1;
	}

	pte = mk_pte(p, pgprot);
	// pr_info("%lu\n", pte_val(pte));

	set_pte(ptep, pte);

	if(set_user_flag) {
		set_pgd(pgd, __pgd(pgd_val(*pgd) | _PAGE_USER));
		set_p4d(p4d, __p4d(p4d_val(*p4d) | _PAGE_USER));
		set_pud(pud, __pud(pud_val(*pud) | _PAGE_USER));
		set_pmd(pmd, __pmd(pmd_val(*pmd) | _PAGE_USER));
	}

	pte_unmap(ptep);

	return 0;

}

static int remove_pte(void *va) {
	pte_t * ptep = (pte_t *) get_pte(va);

	if(!ptep) {
		return -1;
	}

	pte_clear(current->mm, (unsigned long)va, ptep);

	// x86 specific:
	asm volatile("invlpg (%0)" ::"r" (va) : "memory");

	return 0;
}

#if 0
static void read_page_flags_of_addr(uint8_t * vaddr) {

    unsigned long *pte;

	pr_info("VADDR: %px\n", vaddr);

	pte = get_pte(vaddr);

    pr_info("PTE ENTRY: %px \n", pte);
	if (!pte) {
		return;
	}
	pr_info("PTE VALUE: %lx \n", *pte);
	pr_info("PTE FLAGS: %lx \n", pte_flags(*(pte_t *)pte));
	pr_info("PTE WRITABLE: %lx \n", *pte & _PAGE_RW);
	pr_info("PTE PAGE_USER: %lx \n", *pte & _PAGE_USER);
	pr_info("PTE NOT EXEC: %lx \n", *pte & _PAGE_NX);

}
#endif


static int passes_address_checks(uint8_t * address) {

	if ((uint64_t) address < TASK_SIZE) {
        pr_info("ERROR: Address is a user address (%px)\n", address);
        return -EFAULT;
	}

	if (((uint64_t) address & 0xffff800000000000) != 0xffff800000000000) {
        pr_info("ERROR: Address is non-canonical (%px)\n", address);
        return -EFAULT;
	}

	if ((uint64_t) address & (PAGE_SIZE - 1)) {
        pr_info("ERROR: Address is not page aligned (%px)\n", address);
        return -EFAULT;
	}

	return 0;
}


static ssize_t mod_kmunmap(struct file *filp, const char *buf, size_t len, loff_t *off) {

    char kbuf[256] = {0};
	uint8_t * address;
	struct page *p;
	int ret;
	unsigned long *pte;

    if (copy_from_user(kbuf, buf, min(len, (size_t) 255))) {
		return -EFAULT;
    }

    if (sscanf(kbuf, "%llx", (uint64_t *) &address) != 1) {
        return -EFAULT;
    }

	// Perform checks on the address
	ret = passes_address_checks(address);

	if (ret < 0) {
		pr_info("Failed address checks (%px)\n", address);
		return ret;
	}

	pte = get_pte(address);

	if (pte == NULL) {
#ifdef VERBOSE
        pr_info("ERROR: Address is not mapped (%px)\n", address);
#endif
        return -EFAULT;
	}

	// we only allow to unmap our own (user) pages
	if ((*pte & (_PAGE_RW | _PAGE_USER | _PAGE_NX)) != (_PAGE_RW | _PAGE_USER)) {
#ifdef VERBOSE
		pr_info("ERROR: Page flag mismatch (%px)\n", address);
#endif
        return -EINVAL;
	}


	// Start with unmapping

	p = pte_page(*(pte_t *)pte);
	__free_page(p);

	remove_pte(address);

    len = strlen(kbuf);
    *off += len;

    return len;
}

static ssize_t mod_kmmap(struct file *filp, const char *buf, size_t len, loff_t *off) {

    char kbuf[256] = {0};
	uint8_t * address;
	struct page *p;
	int ret;

    if (copy_from_user(kbuf, buf, min(len, (size_t) 255))) {
		return -EFAULT;
    }

    if (sscanf(kbuf, "%llx", (uint64_t *) &address) != 1) {
        return -EFAULT;
    }

	// Perform checks on the address
	ret = passes_address_checks(address);

	if (ret < 0) {
		pr_info("Failed address checks (%px)\n", address);
		return ret;
	}

	if (get_pte(address) != NULL) {
#ifdef VERBOSE
        pr_info("ERROR: Address is already mapped (%px)\n", address);
#endif
        return -EINVAL;
	}

	// Start with mapping

	p = alloc_page(GFP_KERNEL);

	ret = insert_pte(address, p, PAGE_SHARED_EXEC, 1);

    len = strlen(kbuf);
    *off += len;

    return len;
}



static struct proc_ops map_kmmap_fops = {
	.proc_write = mod_kmmap
};

static struct proc_ops map_kmunmap_fops = {
	.proc_write = mod_kmunmap
};


static struct proc_dir_entry *proc_dir;

static int __init mmap_kernel_init(void)
{

	pr_info("initializing\n");

	proc_dir = proc_mkdir("kmemory", NULL);
	proc_create("kmmap", 0666, proc_dir, &map_kmmap_fops);
	proc_create("kmunmap", 0666, proc_dir, &map_kmunmap_fops);


	return 0;
}

static void __exit mmap_kernel_exit(void)
{
	pr_info("exiting\n");

	proc_remove(proc_dir);
}

module_init(mmap_kernel_init);
module_exit(mmap_kernel_exit);
