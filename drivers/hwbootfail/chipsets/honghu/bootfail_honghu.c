/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/stacktrace.h>
#include <linux/sched/clock.h>
#include <linux/securec.h>
#include <asm/cacheflush.h>
#include <linux/mntn/bfmr/hisi_partition.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <hwbootfail/chipsets/bootfail_honghu.h>
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include "adapter_honghu.h"

/* ----local macroes ---- */
#define NS_PER_SEC 1000000000
#define SHORT_TIMER_TIMEOUT_VALUE (1000 * 60 * 10)
#define LONG_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define ACTION_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)

#define ENTRIES_MAX_SIZE 20
#define TEMP_BUF_MAX_SIZE 384
#define COMMON_BUF_MAX_SIZE 40

/* ---- local prototypes ---- */
/* ---- local static variables ---- */
static uintptr_t panic_tag_addr;

/* ---- local function definitions --- */
unsigned int get_bootup_time(void)
{
	unsigned long long ts;

	ts = sched_clock();

	return (unsigned int)(ts / (NS_PER_SEC));
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
	unsigned long entries[ENTRIES_MAX_SIZE]; /* max backword entries */
	char tmp_buf[TEMP_BUF_MAX_SIZE]; /* max size of tmp_buf */
	char com_buf[COMMON_BUF_MAX_SIZE]; /* max size of com_buf */
	bool find_panic = false;

	if (pbuf == NULL || buf_size == 0)
		return;

	(void)memset_s(pbuf, buf_size, 0, buf_size);
	(void)memset_s(tmp_buf, sizeof(tmp_buf), 0, sizeof(tmp_buf));
	(void)memset_s(com_buf, sizeof(com_buf), 0, sizeof(com_buf));
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
				(void)memset_s(tmp_buf, sizeof(tmp_buf), 0, sizeof(tmp_buf));
		}
	}

	com_len = min(buf_size - 1, strlen(com_buf)); // lint !e666
	if (memcpy_s(pbuf, buf_size, com_buf, com_len) != EOK)
		print_err("dump_stacktrace memcpy com_buf to pbuf faild!\n");
	if (com_len >= buf_size - 1)
		return;
	stack_len = min(buf_size - com_len - 1, strlen(tmp_buf)); // lint !e666
	if (memcpy_s(pbuf + com_len, buf_size - com_len,
		tmp_buf, stack_len) != EOK)
		print_err("dump_stacktrace memcpy tmp_buf to pbuf faild!\n");
}

static int process_panic_event(struct notifier_block *self,
	unsigned long val, void *data)
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

static struct notifier_block panic_event_nb = {
	.notifier_call	= process_panic_event,
	.priority	= INT_MAX,
};

#ifdef CONFIG_BOOT_DETECTOR_DBG
long simulate_storge_rdonly(unsigned long arg)
{
	print_info("bootfail Enter %s, arg: %lu\n", __func__, arg);
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
	struct common_init_params init_param;
	struct adapter adp;

	panic_tag_addr = (uintptr_t)bf_map(HISI_RESERVED_MNTN_DDR_BFMR_BASE,
		BF_SIZE_1K);
	print_err("bootfail_init: panic_tag_addr = 0x%lx \n", panic_tag_addr);
	if (panic_tag_addr != 0) {
		print_err("Register panic notifier in bootfail!\n");
		atomic_notifier_chain_register(&panic_notifier_list,
			&panic_event_nb);
	}

	(void)memset_s(&init_param, sizeof(init_param), 0, sizeof(init_param));
	if (bootfail_common_init(&init_param) != 0) {
		print_err("int bootfail common failed!\n");
		return -1;
	}

	(void)memset_s(&adp, sizeof(adp), 0, sizeof(adp));
	if (honghu_adapter_init(&adp) != 0) {
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

	return 0;
}

module_init(bootfail_init);
MODULE_LICENSE("GPL");
