/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: the event.c uses the kernel stp API to upload the information to stp engine
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include "event.h"

#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/sched/user.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <chipset_common/security/hw_kernel_stp_interface.h>
#include "common.h"

#define ADDITION_INFO_LEN 64
#define CHA_STP_INFO_LEN 256

static bool need_exp_report(void)
{
	struct user_struct *user = get_current_user();

	if (unlikely(in_interrupt()))
		goto ret_false;

	if (!user->kshield_exp_reported) {
		user->kshield_exp_reported = true;
		ks_debug("[kshield] make kshield_exp_reported = true\n");
		free_uid(user);
		return true;
	}

ret_false:
	free_uid(user);
	return false;
}

static struct stp_item config_stp_item(void)
{
	struct stp_item item;
	int ret;

	item.id = item_info[KSHIELD].id;
	item.status = STP_RISK;
	item.credible = STP_REFERENCE;
	item.version = 0;
	ret = strncpy_s(item.name, STP_ITEM_NAME_LEN - 1,
		item_info[KSHIELD].name, strlen(item_info[KSHIELD].name) + 1);
	if (ret != EOK)
		ks_debug("[kshield] config stp item strncpy_s error\n");

	return item;
}

void gen_exp_event(unsigned int hook_id)
{
	char addition_info[ADDITION_INFO_LEN];
	char stp_info[ADDITION_INFO_LEN];
	int ret;

	ks_debug("[kshield] gen exp event hook id = %u\n", hook_id);
	snprintf_s(addition_info, sizeof(addition_info), sizeof(addition_info) - 1,
		"%u:%u:1", current_uid().val, current->tgid);  // malicious->1
	ret = snprintf_s(stp_info, sizeof(stp_info), sizeof(stp_info) - 1,
		"type:exp proc_name:%s hook_id:%u", current->comm, hook_id);
	if (ret == -1) {
		ks_debug("[kshield] gen exp event snprintf_s error\n");
		return;
	}
	if (!need_exp_report())
		return;
	kernel_stp_upload(config_stp_item(), stp_info);
	kernel_stp_kshield_upload(config_stp_item(), addition_info);
}

void gen_cha_event(unsigned int hook_id, unsigned long data, unsigned long tick,
			const char *extend_info)
{
	char addition_info[ADDITION_INFO_LEN];
	char stp_info[CHA_STP_INFO_LEN];
	int ret;

	ks_debug("[kshield] gen cha event hook id = %u, data = %lu, tick = %lu\n",
		hook_id, data, tick);
	snprintf_s(addition_info, sizeof(addition_info), sizeof(addition_info) - 1,
		"%u:%u:0", current_uid().val, current->tgid); // suspicious->0
	ret = snprintf_s(stp_info, sizeof(stp_info), sizeof(stp_info) - 1,
		"type:cha proc_name:%s hook_id:%u %s", current->comm, hook_id, extend_info);
	if (ret == -1) {
		ks_debug("[kshield] gen cha event snprintf_s error\n");
		return;
	}
	kernel_stp_upload(config_stp_item(), stp_info);
	kernel_stp_kshield_upload(config_stp_item(), addition_info);
}

