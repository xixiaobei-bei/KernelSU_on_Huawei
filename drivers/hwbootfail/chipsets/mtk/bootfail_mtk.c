/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: wangsiwen
 * Create: 2020-09-20
 */
#include <hwbootfail/chipsets/bootfail_mtk.h>

#include <linux/printk.h>
#include <linux/kdebug.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/stacktrace.h>
#include <soc/mediatek/log_store_kernel.h>
#include <linux/sched/clock.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include "adapter_mtk.h"

#ifdef CONFIG_BOOT_DETECTOR_MTK_4G
#include <hwbootfail/chipsets/bootfail_mtk_4g.h>
#endif

#define NS_PER_SEC 1000000000
#define BOOT_SUCC_TIME 90 /* unit: second */
#define SHORT_TIMER_TIMEOUT_VALUE (1000 * 60 * 10)
#define LONG_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define ACTION_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define UFS_TYPE_OFFSET 0x10
#define UFS_FAULT_SIMULATION_SIZE 0x20

static bool g_is_poweroff_charge;
static uintptr_t g_panic_tag_addr;

static void save_long_press_info_func(struct work_struct *work)
{
	set_valid_long_press_flag();
	mtk_save_long_press_logs();
}

static DECLARE_WORK(save_long_press_info_work, &save_long_press_info_func);

void save_long_press_info(void)
{
	if (!is_boot_detector_enabled()) {
		print_err("%s:BootDetector disabled\n", __func__);
		return;
	}

	if (is_boot_success()) {
		print_err("%s:already boot success\n", __func__);
	} else {
		print_err("%s:begin\n", __func__);
		schedule_work(&save_long_press_info_work);
	}
}

unsigned int get_bootup_time(void)
{
	unsigned long long ts;

	ts = sched_clock();

	return (unsigned int)(ts / (NS_PER_SEC));
}

static int __init early_parse_power_off_charge_cmdline(char *p)
{
	if (p != NULL) {
		g_is_poweroff_charge = (strncmp(p, "charger",
			strlen("charger")) == 0) ? true : false;
	}

	return 0;
}
early_param("androidboot.mode", early_parse_power_off_charge_cmdline);

void set_valid_long_press_flag(void)
{
	union bfi_part_header hdr;
	enum boot_stage bootstage;

	(void)memset_s(&hdr, sizeof(hdr), 0, sizeof(hdr));
	if (is_erecovery_mode()) {
		print_info("save long press log in erecovery mode\n");
		if (strncpy_s(hdr.misc_msg.command,
			sizeof(hdr.misc_msg.command),
			ERECOVERY_LONG_PRESS_TAG,
			min(strlen(ERECOVERY_LONG_PRESS_TAG),
			sizeof(hdr.misc_msg.command) - 1)) != EOK) {
			print_info("ERECOVERY_TAG strncpy_s failed\n");
			return;
		}
	} else {
		if (check_recovery_mode()) {
			print_info("recovery mode\n");
			return;
		}

		if (g_is_poweroff_charge) {
			print_info("charger mode\n");
			return;
		}

		get_boot_stage(&bootstage);
		if (!((get_bootup_time() > BOOT_SUCC_TIME) &&
			(bootstage < BOOT_SUCC_STAGE))) {
			print_info("mistakenly press\n");
			return;
		}
		print_info("save long press log in normal mode\n");
		if (strncpy_s(hdr.misc_msg.command,
			sizeof(hdr.misc_msg.command),
			NORMAL_LONG_PRESS_TAG,
			min(strlen(NORMAL_LONG_PRESS_TAG),
			sizeof(hdr.misc_msg.command) - 1)) != EOK) {
			print_info("NORMAL_TAG strncpy_s failed\n");
			return;
		}
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

/*
 * pbuf points to a memory block with device attribute, we should access
 * pbuf by alignment. We set pbuf "volatile" to prevent any optimization
 * on the access to pbuf, and make sure we write to it byte-by-byte in the
 * for statement.
 */
static noinline void dump_stacktrace(volatile char *pbuf, size_t buf_size)
{
	int i;
	size_t stack_len = 0;
	size_t com_len = 0;
	struct stack_trace trace;
	unsigned long entries[20] = {0}; /* max backword entries */
	char tmp_buf[384] = {0}; /* max size of tmp_buf */
	char com_buf[40] = {0}; /* max size of com_buf */

	if (pbuf == NULL || buf_size == 0) {
		print_invalid_params("buf_size: %zu\n", buf_size);
		return;
	}

	for (i = 0; i < (int)buf_size; i++)
		pbuf[i] = 0;

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
			print_err("stack_len: %zu, overflow\n", stack_len);
			tmp_buf[sizeof(tmp_buf) - 1] = '\0';
			break;
		}
		scnprintf(tmp_buf + stack_len, sizeof(tmp_buf) - stack_len,
			"%pS\n", (void *)(uintptr_t)trace.entries[i]);
	}

	com_len = min(buf_size - 1, strlen(com_buf));
	for (i = 0; i < (int)com_len; i++)
		pbuf[i] = com_buf[i];
	if (com_len >= (buf_size - 1))
		return;
	stack_len = min(buf_size - com_len - 1, strlen(tmp_buf));
	for (i = 0; i < (int)stack_len; i++)
		pbuf[com_len + i] = tmp_buf[i];
}

static int process_panic_event(struct notifier_block *self,
	unsigned long val, void *data)
{
	g_panic_tag_addr = (uintptr_t)get_phys_mem_base();
	if (check_recovery_mode() && g_panic_tag_addr != 0) {
		print_err("panic in recovery mode!\n");
		writel(is_erecovery_mode() ? PANIC_IN_ERECOVERY : PANIC_IN_RECOVERY,
			(char *)g_panic_tag_addr);
	} else if (g_panic_tag_addr != 0) {
		print_err("dump stack trace for panic @ %p!\n",
			g_panic_tag_addr);
		dump_stacktrace((volatile char *)g_panic_tag_addr,
			BF_SIZE_1K - ROFA_MEM_SIZE);
		print_err("stack trace:\n%s\n", (char *)g_panic_tag_addr);
	}

	return NOTIFY_DONE;
}

char *get_bfi_dev_path(void)
{
	return BFI_DEV_PATH;
}

static struct notifier_block panic_event_nb = {
	.notifier_call = process_panic_event,
	.priority = INT_MAX,
};

static struct notifier_block die_event_nb = {
	.notifier_call = process_panic_event,
	.priority = INT_MAX,
};

#ifdef CONFIG_BOOT_DETECTOR_DBG
long simulate_storge_rdonly(unsigned long arg)
{
	int ufs_type;
	char *paddr = NULL;
	unsigned int *value = NULL;

	if (copy_from_user(&ufs_type, (int *)(uintptr_t)arg,
		sizeof(int)) != 0) {
		print_err("Failed to copy flag from user\n");
		return -EFAULT;
	}

	paddr = get_phys_mem_base();
	value = (unsigned int *)paddr;
	writel(0x0, value++);
	writel(0x0, value++);
	writel(0x0, value++);
	writel(ufs_type, (paddr + UFS_TYPE_OFFSET));
	iounmap(paddr);
	print_err("simulate storge rdonly successfully\n");

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
	enum bootfail_errorcode errcode;
	struct common_init_params init_param;
	struct adapter adp;

	g_panic_tag_addr = (uintptr_t)get_phys_mem_base();
	print_err("Register panic notifier in bootfail\n");
	atomic_notifier_chain_register(&panic_notifier_list, &panic_event_nb);
	register_die_notifier(&die_event_nb);
#ifdef CONFIG_BOOT_DETECTOR_MTK_4G
	init_boot_context();
#endif

	(void)memset_s(&init_param, sizeof(init_param), 0, sizeof(init_param));
	if (bootfail_common_init(&init_param) != 0) {
		print_err("init bootfail common failed\n");
		return -1;
	}

	(void)memset_s(&adp, sizeof(adp), 0, sizeof(adp));
	if (mtk_adapter_init(&adp) != 0) {
		print_err("init adapter failed\n");
		return -1;
	}

	errcode = boot_detector_init(&adp);
	if (errcode != BF_OK) {
		print_err("init boot detector failed, errcode: %d\n", errcode);
		return -1;
	}

	return 0;
}

module_init(bootfail_init);
MODULE_LICENSE("GPL");
