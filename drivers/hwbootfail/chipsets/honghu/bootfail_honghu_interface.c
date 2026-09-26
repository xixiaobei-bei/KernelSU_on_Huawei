/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the external interface for BootDetector kernel stage
 * Author: hucangjun
 * Create: 2020-12-25
 */

#include <linux/time.h>
#include <linux/securec.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/bootfail_honghu.h>

#define DEFAULT_DETAIL_INFO "Bootfail in kernel stage"

static void get_boot_fail_time(unsigned long *prtc_time,
	unsigned int *pboot_time)
{
	struct timeval tv;

	if (prtc_time == NULL || pboot_time == NULL)
		return;

	do_gettimeofday(&tv);
	*prtc_time = tv.tv_sec;
	*pboot_time = 0;
}

int process_bootfail(enum bootfail_errno bootfail_errno,
	enum suggest_recovery_method suggested_method,
	const char *detail_info)
{
	struct bootfail_proc_param pparam;

	print_err("%s: %d\n", __func__, __LINE__);
	if (memset_s(&pparam, sizeof(pparam), 0, sizeof(pparam)) != EOK)
		print_err("%s: memset_s param failed\n", __func__);
	pparam.magic = BF_SW_MAGIC_NUM;
	pparam.binfo.bootfail_errno = bootfail_errno;
	pparam.binfo.stage = KERNEL_STAGE;
	pparam.binfo.suggest_recovery_method = suggested_method;
	if (suggested_method != METHOD_DO_NOTHING) {
		pparam.binfo.post_action = PA_REBOOT;
		print_info("%s: ready do action reboot\n", __func__);
	}

	get_boot_fail_time((unsigned long *)&pparam.binfo.rtc_time,
		(unsigned int *)&pparam.binfo.bootup_time);
	if (detail_info != NULL) {
		if (strncpy_s(pparam.detail_info,
			sizeof(pparam.detail_info),
			detail_info, strlen(detail_info)) != EOK)
			print_err("strncpy_s detail_info failed\n");
	} else {
		if (strncpy_s(pparam.detail_info,
			sizeof(pparam.detail_info),
			DEFAULT_DETAIL_INFO, strlen(DEFAULT_DETAIL_INFO)) != EOK)
			print_err("strncpy_s default detail_info failed\n");
	}

	pparam.binfo.is_updated = 0;
	pparam.binfo.is_rooted = 0;
	(void)boot_fail_error(&pparam);

	return 0;
}