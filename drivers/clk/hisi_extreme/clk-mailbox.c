/*
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "clk-mailbox.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/hisi/hisi_rproc.h>

#define MAX_SEND_IPC_TRY	3
#define MAX_SEND_MSG_TRY	3

#define CLK_MBX_ID HISI_ACPU_LPM3_MBX_1
#define MAX_MBOX_CHAN_DATA_SIZE RPROC_GET_MBOX_CHAN_DATA_SIZE(CLK_MBX_ID)

struct mailbox_clk {
	struct hisi_mbox *mbox;
};

static int mailbox_disable;

static int mailbox_pm_callback(struct notifier_block *nb,
	unsigned long action, void *ptr)
{
	switch (action) {
	case PM_SUSPEND_PREPARE:
		mailbox_disable = 1;
		break;

	case PM_POST_SUSPEND:
		mailbox_disable = 0;
		break;

	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static struct notifier_block mailbox_pm_notif_block = {
	.notifier_call = mailbox_pm_callback,
};

static int clkmbox_send_ipc(mbox_msg_t *msg, int length)
{
	int loop = MAX_SEND_IPC_TRY;
	int ret, rproc_id;

	rproc_id = CLK_MBX_ID;

	if (RPROC_IS_SUSPEND(rproc_id))
		return -ENODEV;
	/* try again if failing to send */
	do {
		ret = RPROC_ASYNC_SEND(rproc_id, (rproc_msg_t *)msg, length);
		loop--;
	} while (ret == -ENOMEM && loop > 0);
	if (ret) {
		pr_err("[%s] fail to send mbox msg, ret = %d!\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int clkmbox_send_ipc_sync(mbox_msg_t *msg, int length)
{
	int loop = MAX_SEND_IPC_TRY;
	int ret, rproc_id;

	rproc_id = CLK_MBX_ID;

	if (RPROC_IS_SUSPEND(rproc_id))
		return -ENODEV;
	/* try again if failing to send */
	do {
		ret = RPROC_SYNC_SEND(rproc_id, (rproc_msg_t *)msg, length, NULL, 0);
		loop--;
	} while (ret == -ENOMEM && loop > 0);
	if (ret) {
		pr_err("[%s] fail to send mbox msg, ret = %d!\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

/*
 * Please pay attention to the following points:
 * (1) This function can be called from atomic and non-atomic context.
 * (2) Please check the return variable, as the mailbox may not ready.
 */
int clkmbox_send_msg(mbox_msg_t *msg, int length)
{
	int ret;
	int loop = MAX_SEND_MSG_TRY;

	if (msg == NULL || length > MAX_MBOX_CHAN_DATA_SIZE) {
		pr_err("fail to send msg/mbox_length!\n");
		return -EINVAL;
	}

	/* try again when the mail box is full */
	do {
		ret = clkmbox_send_ipc(msg, length);
		loop--;
	} while (ret != 0 && loop > 0);

	return ret;
}

int clkmbox_send_msg_sync(mbox_msg_t *msg, int length)
{
	int ret;
	int loop = MAX_SEND_MSG_TRY;

	if (msg == NULL || length > MAX_MBOX_CHAN_DATA_SIZE)
		return -EINVAL;

	/* try again when the mail box is full */
	do {
		ret = clkmbox_send_ipc_sync(msg, length);
		loop--;
	} while (ret != 0 && loop > 0);

	return ret;
}

static int __init clkmbox_init(void)
{
	int ret = 0;

	mailbox_disable = 0;
	register_pm_notifier(&mailbox_pm_notif_block);

	return ret;
}

static void __exit clkmbox_exit(void)
{
	int rproc_id = CLK_MBX_ID;

	/*
	 * the RPROC_LPM3 is a shared channel by many IP,
	 * but the exit function should never be used
	 */
	RPROC_PUT(rproc_id);
	unregister_pm_notifier(&mailbox_pm_notif_block);
}

fs_initcall(clkmbox_init);
module_exit(clkmbox_exit);
