/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include "ocp_hisi.h"
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include <hwbootfail/chipsets/bootfail_hisi.h>
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>

/* ----local macroes ---- */
#define WAIT_FILE_NODE_TIMES  180   /* seconds */
/* ---- local prototypes ---- */
/* ---- local static variables ---- */
static struct semaphore proc_ocp_sem;
static struct ocp_proc_param proc_ocp_param;

/* ---- local function definitions --- */
static void format_ocp_proc_param(struct bootfail_proc_param *pparam,
	struct boot_fail_ocp_info *pocp_info)
{
	pparam->magic = BF_HW_MAGIC_NUM;
	pparam->binfo.rtc_time = get_sys_rtc_time();
	pparam->binfo.bootfail_errno = HARDWARE_NON_CORE_DEV_FAULT;
	pparam->binfo.bootup_time = get_bootup_time();
	pparam->binfo.suggest_recovery_method = METHOD_NO_SUGGESTION;
	strncpy(pparam->detail_info, pocp_info->binfo.ldo_num,
		min(sizeof(pparam->detail_info) - 1,
		sizeof(pocp_info->binfo.ldo_num)));
	strncpy(pparam->hardware_detail, pocp_info->binfo.ldo_num,
		min(sizeof(pparam->hardware_detail) - 1,
		sizeof(pocp_info->binfo.ldo_num)));
	get_boot_stage((enum boot_stage *)&pparam->binfo.stage);
}

static void __boot_fail_process_ocp(void)
{
	int i;

	for (i = 0; i < (int)array_size(proc_ocp_param.ocp_info); i++) {
		if (down_trylock(&(proc_ocp_param.ocp_info[i].sem)))
			continue;

		if (strcmp(proc_ocp_param.ocp_info[i].binfo.ldo_num, "") != 0) {
			enum bootfail_errorcode errorcode;
			struct bootfail_proc_param *pparam;

			pparam = vmalloc(sizeof(*pparam));
			if (pparam == NULL) {
				print_err("vmalloc failed!\n");
				up(&(proc_ocp_param.ocp_info[i].sem));
				continue;
			}
			memset((void *)pparam, 0, sizeof(*pparam));
			format_ocp_proc_param(pparam,
				 &proc_ocp_param.ocp_info[i]);
			errorcode = boot_fail_error(pparam);
			print_info("boot_fail_error ret: %d\n", errorcode);
			memset(&proc_ocp_param.ocp_info[i].binfo, 0,
				sizeof(struct ocp_basic_info));
			vfree(pparam);
		}
		up(&(proc_ocp_param.ocp_info[i].sem));
	}
}

static int process_ocp_func(void *param)
{
	print_info("param: %p\n", param);

	if (!wait_file_node_ready(get_bfi_dev_path(), WAIT_FILE_NODE_TIMES)) {
		print_err("%s is not ready, ending ocp mechanism!\n", get_bfi_dev_path());
		return 0;
	}

	while (1) {
		if (down_interruptible(&proc_ocp_sem) == 0)
			__boot_fail_process_ocp();
	}

	return 0;
}

int process_ocp_callback(struct notifier_block *self,
	unsigned long val, void *data)
{
	int i;

	for (i = 0; i < (int)array_size(proc_ocp_param.ocp_info); i++) {
		if (down_trylock(&(proc_ocp_param.ocp_info[i].sem))) {
			print_err("There's an OCP is being processed!\n");
			continue;
		}

		if (strcmp(proc_ocp_param.ocp_info[i].binfo.ldo_num, "") == 0) {
			memset((void *)&proc_ocp_param.ocp_info[i].binfo,
				0x0, sizeof(struct ocp_basic_info));
			memcpy((void *)&proc_ocp_param.ocp_info[i].binfo,
				data, sizeof(struct ocp_basic_info));
			proc_ocp_param.ocp_info[i].type = HW_FAULT_OCP;
			up(&(proc_ocp_param.ocp_info[i].sem));
			break;
		}
		up(&(proc_ocp_param.ocp_info[i].sem));
	}
	up(&proc_ocp_sem);

	return NOTIFY_DONE;
}

static struct notifier_block ocp_event_nb = {
	.notifier_call = process_ocp_callback,
	.priority = INT_MAX,
};

static int boot_fail_process_ocp(void)
{
	int ret;
	struct task_struct *tsk = NULL;

	ret = hisi_pmic_mntn_register_notifier(&ocp_event_nb);
	if (ret != 0) {
		print_err("ocp_event_nb register fail!\n");
		return ret;
	}

	tsk = kthread_run(process_ocp_func, NULL, "process_ocp");
	if (IS_ERR(tsk)) {
		print_err("Failed to create ocp process thread!\n");
		return -1;
	}

	return 0;
}

/**
 * ocp_init - init a worker to detect device ocp
 * @pparam: init parameter for ocp worker, for expand, not used now
 * return 0 if init success, -1 when fail
 */
int ocp_init(struct ocp_init_param *pparam)
{
	int i;

	if (unlikely(pparam == NULL)) {
		print_invalid_params("param is NULL!\n");
		return -1;
	}

	memset((void *)&proc_ocp_param, 0, sizeof(proc_ocp_param));
	for (i = 0; i < (int)array_size(proc_ocp_param.ocp_info); i++)
		sema_init(&proc_ocp_param.ocp_info[i].sem, 1);
	sema_init(&proc_ocp_sem, 0);
	(void)boot_fail_process_ocp();

	return 0;
}
