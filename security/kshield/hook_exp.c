/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: the hook_exp.c implements thread count code
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include <asm/pgtable.h>
#include <linux/file.h>
#include <linux/kallsyms.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "common.h"
#include "event.h"

#ifdef CONFIG_KSHIELD_PERMISSIVE
#define KS_RET_VAL 0
#else
#define KS_RET_VAL (-EINVAL)
#endif

static const char g_exp_action = IGNORE;

#ifdef CONFIG_KSHIELD_PERMISSIVE
static int take_action(struct task_struct *p, int action)
{
	return 0;
}
#else /* !CONFIG_KSHIELD_PERMISSIVE */
static int take_action(struct task_struct *p, int action)
{
	if (!p)
		return -EINVAL;

	switch (action) {
	case PERMISSIVE:
		ks_debug("Permissive task");
		do_send_sig_info(SIGCONT, SEND_SIG_FORCED, p, true);
		break;
	case FREEZE:
		ks_debug("Freeze task");
		do_send_sig_info(SIGSTOP, SEND_SIG_FORCED, p, true);
		break;
	case KILL:
		ks_debug("Kill task");
		do_send_sig_info(SIGKILL, SEND_SIG_FORCED, p, true);
		break;
	default:
		break;
	}
	return 0;
}
#endif

#if defined(CONFIG_ARM64_SW_TTBR0_PAN) || defined(CONFIG_ARM64_PAN)
static int check_ctl_table(const struct ctl_table *table)
{
	(void)table;
	return 0;
}
#else
static int _check_ctl_table(const struct ctl_table *ptr)
{
	unsigned long kaddr;

	kaddr = (unsigned long)((uintptr_t)ptr);
	if (unlikely(kaddr < TASK_SIZE))
		return -EINVAL;
	return 0;
}

static int check_ctl_table(struct ctl_table *table)
{
	struct ctl_table *entry = table;

	while (entry->procname) {
		if (unlikely(_check_ctl_table(entry)))
			return -EINVAL;
		entry++;
	}
	return 0;
}
#endif

int sub_chk_user_mem(const char __user *buf, size_t copy)
{
	(void)copy;
	if (unlikely(uaccess_mask_ptr(buf)))
		return 0;
	ks_debug("Detect malicious iovec");
	gen_exp_event(HOOK_EXP_IOVEC);
	take_action(current, g_exp_action);
	return KS_RET_VAL;
}

/**
 * The following function will trigger this hook
 * set_memory_{ro, rw, nx, x}
 *
 * These function can be called from different path in kernel
 * set_memory_{ro, rw}: module/cfi/power/bpf
 * set_memory_{nx, x}: module
 */
int sub_chk_change_mem(unsigned long start, unsigned long size,
	pgprot_t set_mask, pgprot_t clear_mask)
{
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned long text_start = (unsigned long)(uintptr_t)_text;
	unsigned long init_data_end = (unsigned long)(uintptr_t)__init_end;

	mem_start = start;
	mem_end = start + size;
	if (mem_start >= mem_end)
		return -EINVAL;
	if (mem_end < TASK_SIZE)
		goto report;

	/* set_memory_rw() */
	if (pgprot_val(set_mask) & PTE_WRITE)
		/* text/rodata(vdso)/init */
		if (mem_end >= text_start && mem_start < init_data_end)
			goto report;

	/* set_memory_x() */
	if (pgprot_val(clear_mask) & PTE_PXN)
		if (mem_start >= MODULES_END || mem_end < MODULES_VADDR)
			goto report;
	return 0;
report:
	ks_debug("Detect tamper memory attr\n");
	gen_exp_event(HOOK_EXP_CHG_MEM);
	take_action(current, g_exp_action);
	return KS_RET_VAL;
}

int sub_chk_heap_ret2dir(const void *x)
{
	struct page *page = NULL;

	if ((unsigned long)((uintptr_t)x) < VA_START) {
		ks_debug("Detect malicious fops\n");
		gen_exp_event(HOOK_EXP_FOPS);
		goto act;
	}
	page = virt_to_head_page(x);
	if (unlikely(!PageSlab(page))) {
		ks_debug("Detect ret2dir\n");
		gen_exp_event(HOOK_EXP_RET2DIR);
		goto act;
	}
	return 0;
act:
	take_action(current, g_exp_action);
	return KS_RET_VAL;
}

#ifdef CONFIG_KALLSYMS
static int check_fops(const void *ops, size_t ops_sz)
{
	unsigned long symbol_size;
	unsigned long offset;
	char name_buf[KSYM_NAME_LEN];
	const void *name = kallsyms_lookup((uintptr_t)ops, &symbol_size,
			&offset, NULL, name_buf);

	if (unlikely(!name || offset != 0 || symbol_size != ops_sz))
		return -EINVAL;
	return 0;
}
#else
static int check_fops(const void *ops, size_t ops_sz)
{
	unsigned long data;
	unsigned long data_end;
	unsigned long ro_begin;
	unsigned long ro_end;

	data = (unsigned long)((uintptr_t)ops);
	data_end = data + ops_sz;
	ro_begin = (unsigned long)_etext;
	ro_end = (unsigned long)__init_begin;
	if (likely(data >= ro_begin && data_end < ro_end))
		return 0;
	return -EINVAL;
}
#endif

int sub_chk_fops(const void *ops, size_t ops_sz)
{
	if (check_fops(ops, ops_sz)) {
		ks_debug("Detect malicious fops\n");
		gen_exp_event(HOOK_EXP_FOPS);
		take_action(current, g_exp_action);
		return KS_RET_VAL;
	}
	return 0;
}

int sub_chk_sysctl(const struct ctl_table *table)
{
	if (check_ctl_table(table)) {
		ks_debug("Detect malicious sysctl table");
		gen_exp_event(HOOK_EXP_REG_SYSCTL);
		take_action(current, g_exp_action);
		return KS_RET_VAL;
	}
	return 0;
}

#ifdef CONFIG_SECURITY_SELINUX
int sub_chk_sel_write_load(void)
{
	/* Only init can load policy. */
	if (!is_global_init(current)) {
		ks_debug("Load SELinux policy");
		gen_exp_event(HOOK_EXP_LOAD_POLICY);
		take_action(current, g_exp_action);
		return KS_RET_VAL;
	}
	return 0;
}
#endif

int sub_chk_set_procattr(const char *value, size_t size)
{
	int i;
	int ret = 0;
	char *s = NULL;
	char *tmp = NULL;
	char *s_back = NULL;
	char *domain_init = "init";
	int index = 3; // 3 is the init in selinux context domain index(example u:r:init:so)
	if (is_global_init(current))
		return 0;

	// check selinux context domain(example u:r:init:so)
	if (!strnstr(value, domain_init, size))
		return 0;
	s = kstrdup(value, GFP_KERNEL);
	if (!s)
		return 0;
	s_back = s;
	for (i = 0; i < index; i++) {
		tmp = strsep(&s, ":");
		if (!tmp)
			goto out_free;
	}
	if (!strcmp(tmp, domain_init)) {
		ks_debug("[%s] get setprocattr event\n", current->comm);
		gen_exp_event(HOOK_EXP_CHG_DOMAIN);
		take_action(current, g_exp_action);
		ret = KS_RET_VAL;
	}
out_free:
	kfree(s_back);
	return ret;
}
