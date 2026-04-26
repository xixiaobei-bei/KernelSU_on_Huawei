/*
 * hw_kpg.h
 *
 * monitor kernel wakelock
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include <uapi/linux/android/binder.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>
#include <linux/time.h>
#include <huawei_platform/power/hw_kstate.h>

typedef enum {
	SET_WAKELOCK_TIMEOUT = 0,
	CANCEL_WAKELOCK_TIMEOUT = 1,
	SET_ALL_WAKELOCK_TIMEOUT = 2,
	CANCEL_ALL_WAKELOCK_TIMEOUT = 3,
	STOP_WAKELOCK = 4,
	STOP_ALL_KERNEL_WAKELOCK = 5,
	FREEZED_PID = 6
} pg_command_order;

#define HW_PG_LOCK_NAME_MAX_LEN 64
#define MAX_KERNEL_WAKELOCK_TIME 1800000
#define BASE 10

/* command received */
typedef struct {
	u8 cmd;
	u8 lock_timeout;
	char name[HW_PG_LOCK_NAME_MAX_LEN];
} pg_command;

/*
 * manage wakelock
 * @return 0 for success
 */
static int pg_cb(channel_id src, packet_tag tag, const char *data, size_t len)
{
	int ret = 0;
	pg_command pg_cmd;

	if (IS_ERR_OR_NULL(data)) {
		pr_err("pg_cb %s: invalid data or len:%d\n", __func__, (int)len);
		return -1;
	}

	/* set wakelock */
	if (memset_s(&pg_cmd, sizeof(pg_command), 0, sizeof(pg_command)) != EOK) {
		pr_err("pg_cb %s: failed to memset_s\n", __func__);
		return -1;
	}

	if (memcpy_s(&pg_cmd, sizeof(pg_command), data,
		(len < (sizeof(pg_command) - 1)) ? len : (sizeof(pg_command) - 1)) != EOK) {
		pr_err("pg_cb %s: failed to memcpy_s\n", __func__);
		return -1;
	}

	switch (pg_cmd.cmd) {
	case SET_WAKELOCK_TIMEOUT:
		ret = wakeup_source_set(pg_cmd.name, pg_cmd.lock_timeout);
		break;
	case CANCEL_WAKELOCK_TIMEOUT:
		ret = wakeup_source_set(pg_cmd.name, 0); /* 0 is default to cancel */
		break;
	case SET_ALL_WAKELOCK_TIMEOUT:
		break;
	case CANCEL_ALL_WAKELOCK_TIMEOUT:
		ret = wakeup_source_set_all(0); /* 0 is default to cancel */
		break;
	case STOP_WAKELOCK:
		ret = wake_unlockByName(pg_cmd.name);
		break;
	case STOP_ALL_KERNEL_WAKELOCK:
		ret = wake_unlockAll(MAX_KERNEL_WAKELOCK_TIME);
		break;
	case FREEZED_PID: {
		int taget_pid = simple_strtol(pg_cmd.name, NULL, BASE);
		if (taget_pid > 0)
			check_binder_calling_work(taget_pid);
		break;
	}
	default:
		return -1;
	}

	pr_debug("pg_cb %s: src=%d tag=%d len=%d \n", __func__, src, tag, (int) len);

	return ret;
}

static struct kstate_opt kpg_opt = {
	.name = "kpg",
	.tag = PACKET_TAG_KPG,
	.dst = CHANNEL_ID_NETLINK,
	.hook = pg_cb,
};

static int __init kpg_init(void)
{
	int ret = -1;

	ret = kstate_register_hook(&kpg_opt);
	if (ret < 0) {
		pr_err("hw_kpg %s: kstate_register_hook error\n", __func__);
	} else {
		pr_info("hw_kpg %s: kstate_register_hook success\n", __func__);
	}

	return ret;
}

static void __exit kpg_exit(void)
{
	kstate_unregister_hook(&kpg_opt);
}

late_initcall(kpg_init);
module_exit(kpg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("monitor kernel wakelock");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
