/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the dev.c provides API for kernel stp module
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include "common.h"
#include "hook_charact.h"
#include "hook_spray.h"
#include "hook_sock_spray.h"

#define KSHIELD_CONFIG_MAX 64
#define THIRD_APP_UID_MIN 10000
#define PRIV_UID 2000
#define SPLIT_STR ";"
#define BASE_DECIMAL 10

static int ks_inspection_switch(uid_t uid, bool enable)
{
	struct task_struct *g = NULL;
	struct task_struct *p = NULL;
	int ret = -EINVAL;

	if (!(uid > THIRD_APP_UID_MIN || uid == PRIV_UID))
		return ret;

	read_lock(&tasklist_lock);
	for_each_process_thread(g, p) {
		if (task_uid(p).val == uid) {
			p->inspected = enable;
			ret = 0;
		}
	}
	read_unlock(&tasklist_lock);
	return ret;
}

static int ks_dev_cha_enable(unsigned int value)
{
	int ret;
	bool enable = value ? true : false;

	ret = hook_cha_enable(enable);
	if (ret != 0)
		return ret;

	ks_enable_heap_spray(enable);
	ks_enable_sock_spray(enable);

	return ret;
}

static int ks_dev_config_update(unsigned int cmd, char *cur, unsigned int value)
{
	unsigned int data[KSHIELD_CONFIG_MAX];
	int count = 0;
	char *conf = NULL;
	int ret;
	struct cha_conf_meta meta;

	(void)cmd;
	ret = memset_s(data, sizeof(data), 0, sizeof(data));
	if (ret != 0)
		return ret;
	conf = strsep(&cur, SPLIT_STR);
	while ((conf != NULL) && *conf) {
		if (count == KSHIELD_CONFIG_MAX)
			return -EINVAL;
		if (kstrtou32(conf, BASE_DECIMAL, &value))
			return -EINVAL;
		data[count] = value;
		count++;
		conf = strsep(&cur, SPLIT_STR);
	}

	ret = memcpy_s(&meta, sizeof(meta), data, sizeof(meta));
	if (ret != 0)
		return ret;

	ret = hook_cha_update(meta);
	if (ret != 0)
		return ret;

	ks_heap_spray_update(meta.tmeta.heap_spray);
	ks_sock_spray_update(meta.tmeta.sock_spray);

	/*auto enable after update conf*/
	ret = ks_dev_cha_enable(1);

	return ret;
}

long ks_dev_ioctl(unsigned int cmd, char *cur)
{
	int ret;
	char *conf = NULL;
	unsigned int value = 0;

	if (cmd == KS_START_INSPECTION || cmd == KS_STOP_INSPECTION ||
		cmd == KS_CHA_ALL_ENABLE) {
		conf = strsep(&cur, SPLIT_STR);
		if (!conf || !*conf)
			return -EINVAL;
		if (kstrtou32(conf, BASE_DECIMAL, &value))
			return -EINVAL;
		ks_debug("ks dev ioctl cmd = %u, value = %u", cmd, value);
	}
	switch (cmd) {
	case KS_START_INSPECTION:
		ret = ks_inspection_switch((uid_t)value, true);
		break;
	case KS_STOP_INSPECTION:
		ret = ks_inspection_switch((uid_t)value, false);
		break;
	case KS_CHA_ALL_ENABLE:
		ret = ks_dev_cha_enable(value);
		break;
	case KS_CHA_CONFIG_UPDATE:
		ret = ks_dev_config_update(KS_CHA_CONFIG_UPDATE, cur, value);
		break;
	default:
		ret = -ENOTTY;
		break;
	}
	return ret;
}
