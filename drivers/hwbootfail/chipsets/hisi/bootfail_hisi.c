/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/stacktrace.h>
#include <mntn_public_interface.h>
#include <hwbootfail/chipsets/bootfail_hisi.h>
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include "adapter_hisi.h"
#include "ocp_hisi.h"

/* ----local macroes ---- */
#define BFI_PART_SIZE (0x7E0000) /* 8MB -128 KB */
#define NS_PER_SEC 1000000000
#define BOOT_SUCC_TIME (90) /* unit: second */
#define SHORT_TIMER_TIMEOUT_VALUE (1000 * 60 * 10)
#define LONG_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define ACTION_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define UFS_TYPE_OFFSET 0x10
#define UFS_FAULT_SIMULATION_SIZE (0x20)

/* ---- local prototypes ---- */
/* ---- local static variables ---- */
static bool is_poweroff_charge;
static uintptr_t panic_tag_addr;

/* ---- local function definitions --- */
unsigned int get_bootup_time(void)
{
	unsigned long long ts;

#ifdef CONFIG_HISI_TIME
	ts = hisi_getcurtime(); /* ns */
#else
	ts = sched_clock();
#endif

	return (unsigned int)(ts / (NS_PER_SEC));
}

static int __init early_parse_power_off_charge_cmdline(char *p)
{
	if (p != NULL) {
		is_poweroff_charge = (strncmp(p, "charger",
			strlen("charger")) == 0) ? true : false;
	}

	return 0;
}
early_param("androidboot.mode", early_parse_power_off_charge_cmdline);

void set_valid_long_press_flag(void)
{
	union bfi_part_header hdr;
	enum boot_stage bootstage;

	memset(&hdr, 0, sizeof(hdr));
	if (is_erecovery_mode()) {
		print_info("save long press log in erecovery mode!\n");
		/*lint -e666*/
		strncpy(hdr.misc_msg.command, ERECOVERY_LONG_PRESS_TAG,
			min(strlen(ERECOVERY_LONG_PRESS_TAG),
			sizeof(hdr.misc_msg.command) - 1));
		/*lint +e666*/
	} else {
		if (check_recovery_mode()) {
			print_info("recovery mode!\n");
			return;
		}

		if (is_poweroff_charge) {
			print_info("charger mode!\n");
			return;
		}

		get_boot_stage(&bootstage);
		if (!((get_bootup_time() > BOOT_SUCC_TIME) &&
			(bootstage < BOOT_SUCC_STAGE))) {
			print_info("mistakenly press!\n");
			return;
		}
		print_info("save long press log in normal mode!\n");
		/*lint -e666*/
		strncpy(hdr.misc_msg.command, NORMAL_LONG_PRESS_TAG,
			min(strlen(NORMAL_LONG_PRESS_TAG),
			sizeof(hdr.misc_msg.command) - 1));
		/*lint +e666*/
	}
	(void)write_part(BFI_DEV_PATH, 0, (const char *)&hdr, sizeof(hdr));
}

unsigned int get_long_timer_timeout_value(void)
{
	return LONG_TIMER_TIMEOUT_VALUE;
}

unsigned int get_short_timer_timeout_value(void)
{
	return SHORT_TIMER_TIMEOUT_VALUE;
}

unsigned int get_action_timer_timeout_value(void)
{
	return ACTION_TIMER_TIMEOUT_VALUE;
}

static void dump_stacktrace(char *pbuf, size_t buf_size)
{
	int i;
	size_t stack_len = 0;
	size_t com_len = 0;
	struct stack_trace trace;
	unsigned long entries[20]; /* max backword entries*/
	char tmp_buf[384]; /* max size of tmp_buf */
	char com_buf[40]; /* max size of com_buf */
	bool find_panic = false;

	if (pbuf == NULL || buf_size == 0)
		return;

	for (i = 0; i < (int)buf_size; i++)
		writeb(0, pbuf + i);
	memset(tmp_buf, 0, sizeof(tmp_buf));
	memset(com_buf, 0, sizeof(com_buf));
	trace.nr_entries = 0;
	trace.max_entries = (unsigned int)ARRAY_SIZE(entries);
	trace.entries = entries;
	trace.skip = 0;
	save_stack_trace(&trace);
	scnprintf(com_buf, sizeof(com_buf), "Comm:%s,CPU:%d\n",
		current->comm, raw_smp_processor_id());
	for (i = 0; i < (int)trace.nr_entries; i++) {
		stack_len = strlen(tmp_buf);
		if (stack_len >= sizeof(tmp_buf)) {
			print_err("stack_len: %d, overflow\n", (int)stack_len);
			tmp_buf[sizeof(tmp_buf) - 1] = '\0';
			break;
		}
		scnprintf(tmp_buf + stack_len, sizeof(tmp_buf) - stack_len,
			"%pS\n", (void *)(uintptr_t)trace.entries[i]);
		if (!find_panic) {
			if (strncmp(tmp_buf, "panic", strlen("panic")) == 0)
				find_panic = true;
			else
				memset(tmp_buf, 0, sizeof(tmp_buf));
		}
	}

	com_len = min(buf_size - 1, strlen(com_buf)); //lint !e666
	for (i = 0; i < (int)com_len; i++)
		writeb(com_buf[i], pbuf + i);
	if (com_len >= buf_size - 1)
		return;
	stack_len = min(buf_size - com_len - 1, strlen(tmp_buf)); //lint !e666
	for (i = 0; i < (int)stack_len; i++)
		writeb(tmp_buf[i], pbuf + com_len + i);
}

static int process_panic_event(struct notifier_block *self,
	unsigned long val,
	void *data)
{
	if (check_recovery_mode() && panic_tag_addr != 0) {
		print_err("panic in recovery mode!\n");
		writel(is_erecovery_mode() ? PANIC_IN_ERECOVERY : PANIC_IN_RECOVERY,
			(char *)panic_tag_addr);
	} else if (panic_tag_addr != 0) {
		print_err("dump stack trace for panic @ %p!\n",
			(void *)panic_tag_addr);
		dump_stacktrace((char *)panic_tag_addr, BF_SIZE_1K);
		print_err("stack trace:\n%s\n", (char *)panic_tag_addr);
	}

	return NOTIFY_DONE;
}

char *get_bfi_dev_path(void)
{
	return BFI_DEV_PATH;
}

size_t get_bfi_part_size(void)
{
	return (size_t)BFI_PART_SIZE;
}

static struct notifier_block panic_event_nb = {
	.notifier_call	= process_panic_event,
	.priority	= INT_MAX,
};

#ifdef CONFIG_BOOT_DETECTOR_DBG
long simulate_storge_rdonly(unsigned long arg)
{
	int ufs_type;
	char *paddr = NULL;
	unsigned int *value = NULL;

	if (copy_from_user(&ufs_type, (int *)(uintptr_t)arg,
		sizeof(int)) != 0) {
		print_err("Failed to copy flag from user!\n");
		return -EFAULT;
	}

	paddr = ioremap_nocache(HISI_SUB_RESERVED_UNUSED_PHYMEM_BASE,
		UFS_FAULT_SIMULATION_SIZE);
	if (paddr != NULL) {
		value = (unsigned int *)paddr;
		writel(0x0, value++);
		writel(0x0, value++);
		writel(0x0, value++);
		writel(ufs_type, (paddr + UFS_TYPE_OFFSET));
		iounmap(paddr);
		print_err("simulate storge rdonly successfully!\n");
	}

	return 0;
}
#else
long simulate_storge_rdonly(unsigned long arg)
{
	return -EFAULT;
}
#endif

int __init bootfail_init(void)
{
	int ret;
	enum bootfail_errorcode errcode;
	struct ocp_init_param ocp_param;
	struct common_init_params init_param;
	struct adapter adp;
	/*lint -e446*/
	panic_tag_addr = (uintptr_t)ioremap_nocache((phys_addr_t)
		HISI_SUB_RESERVED_UNUSED_PHYMEM_BASE, BF_SIZE_1K);
	/*lint +e446*/
	if (panic_tag_addr != 0) {
		print_err("Register panic notifier in bootfail!\n");
		atomic_notifier_chain_register(&panic_notifier_list,
			&panic_event_nb);
	}

	memset(&init_param, 0, sizeof(init_param));
	if (bootfail_common_init(&init_param) != 0) {
		print_err("int bootfail common failed!\n");
		return -1;
	}

	memset(&adp, 0, sizeof(adp));
	if (hisi_adapter_init(&adp) != 0) {
		print_err("int adapter failed!\n");
		return -1;
	}

	errcode = boot_detector_init(&adp);
	if (errcode != BF_OK) {
		print_err("init boot detector failed [ret: %d]\n", errcode);
		return -1;
	}

	/* set kernel stage */
	errcode = set_boot_stage(KERNEL_STAGE);
	print_err("set KERNEL_STAGE [%d]\n", errcode);

	/* process OCP for HISI */
	memset(&ocp_param, 0, sizeof(ocp_param));
	ret = ocp_init(&ocp_param);
	if (ret != 0)
		print_err("device_ocp_init failed!\n");

	return 0;
}

module_init(bootfail_init);
MODULE_LICENSE("GPL");
