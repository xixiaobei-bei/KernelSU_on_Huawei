/*
 * ipc_rproc.c
 *
 * ipc rproc communication interfacer
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
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/hisi/hisi_rproc.h>
#include <pr_log.h>
#include "ipc_mailbox.h"
#include "ipc_rproc_inner.h"

/*lint -e580 */
#define PR_LOG_TAG AP_MAILBOX_TAG

#define rproc_pr_err(fmt, args...)   pr_err(fmt "\n", ##args)

#define rproc_pr_debug(fmt, args...) pr_debug(fmt "\n", ##args)

enum call_type_t {
	ASYNC_CALL = 0,
	SYNC_CALL
};

struct ipc_rproc_info {
	struct atomic_notifier_head notifier;
	struct notifier_block nb;
	struct hisi_mbox *mbox;
};

struct ipc_rproc_info_arr {
	struct ipc_rproc_info rproc_arr[HISI_RPROC_MAX_MBX_ID];
	unsigned int rproc_cnt;
};

static struct ipc_rproc_info_arr g_rproc_table;


static struct ipc_rproc_info *find_rproc(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;
	unsigned int i;

	for (i = 0; i < g_rproc_table.rproc_cnt; i++) {
		if (g_rproc_table.rproc_arr[i].mbox &&
			rproc_id == g_rproc_table.rproc_arr[i].mbox->rproc_id) {
			rproc = &g_rproc_table.rproc_arr[i];
			break;
		}
	}
	return rproc;
}

static int get_rproc_channel_size(struct ipc_rproc_info *rproc)
{
	struct hisi_mbox_device *mdev = NULL;

	mdev = rproc->mbox->tx ? rproc->mbox->tx : rproc->mbox->rx;
	if (!mdev)
		return -EINVAL;

	return mdev->ops->get_channel_size(mdev);
}

static int check_channel_size(struct ipc_rproc_info *rproc, int len)
{
	int mail_box_size = get_rproc_channel_size(rproc);

	if (mail_box_size <= 0 || len > mail_box_size) {
		rproc_pr_err("len is invalid, %d:%d", mail_box_size, len);
		return -EINVAL;
	}

	return 0;
}

int hisi_rproc_xfer_async_with_ack(
	rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len,
	void (*ack_handle)(rproc_msg_t *ack_buffer,
		rproc_msg_len_t ack_buffer_len))
{
	struct ipc_rproc_info *rproc = NULL;
	struct hisi_mbox_task *tx_task = NULL;
	struct hisi_mbox *mbox = NULL;
	int ret;

	rproc = find_rproc(rproc_id);
	if (!rproc) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	if (check_channel_size(rproc, len))
		return -EINVAL;

	mbox = rproc->mbox;

	tx_task = ipc_mbox_task_alloc(mbox, msg, len, MANUAL_ACK);
	if (!tx_task) {
		rproc_pr_err("alloc task failed");
		return -EINVAL;
	}

	tx_task->ack_handle = ack_handle;
	ret = ipc_mbox_msg_send_async(mbox, tx_task);
	if (ret) {
		/* -12:tx_fifo full */
		rproc_pr_err(
			"%s async send failed, errno: %d", mbox->tx->name, ret);
		ipc_mbox_task_free(&tx_task);
	}

	return ret;
}
EXPORT_SYMBOL(hisi_rproc_xfer_async_with_ack);

int hisi_rproc_xfer_async(
	rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len)
{
	struct ipc_rproc_info *rproc = NULL;
	struct hisi_mbox_task *tx_task = NULL;
	struct hisi_mbox *mbox = NULL;
	enum mbox_ack_type_t ack_type = AUTO_ACK;
	int ret;

	rproc = find_rproc(rproc_id);
	if (!rproc) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	if (check_channel_size(rproc, len))
		return -EINVAL;

	mbox = rproc->mbox;

	tx_task = ipc_mbox_task_alloc(mbox, msg, len, ack_type);
	if (!tx_task) {
		rproc_pr_err("alloc task failed");
		return -EINVAL;
	}

	ret = ipc_mbox_msg_send_async(mbox, tx_task);
	if (ret) {
		/* -12:tx_fifo full */
		rproc_pr_err(
			"%s async send failed, errno: %d", mbox->tx->name, ret);
		ipc_mbox_task_free(&tx_task);
	}

	return ret;
}
EXPORT_SYMBOL(hisi_rproc_xfer_async);

int hisi_rproc_xfer_sync(rproc_id_t rproc_id, rproc_msg_t *msg,
	rproc_msg_len_t len, rproc_msg_t *ack_buffer,
	rproc_msg_len_t ack_buffer_len)
{
	struct ipc_rproc_info *rproc = NULL;
	struct hisi_mbox *mbox = NULL;
	enum mbox_ack_type_t ack_type = MANUAL_ACK;
	int ret;

	rproc = find_rproc(rproc_id);
	if (!rproc) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	if (check_channel_size(rproc, len))
		return -EINVAL;

	mbox = rproc->mbox;

	ret = ipc_mbox_msg_send_sync(
		mbox, msg, len, ack_type, ack_buffer, ack_buffer_len);
	if (ret)
		rproc_pr_err("fail to sync send");

	return ret;
}
EXPORT_SYMBOL(hisi_rproc_xfer_sync);

static int ipc_rproc_rx_notifier(
	struct notifier_block *nb, unsigned long len, void *msg)
{
	struct ipc_rproc_info *rproc =
		container_of(nb, struct ipc_rproc_info, nb);

	atomic_notifier_call_chain(&rproc->notifier, len, msg);
	return 0;
}

int hisi_rproc_rx_register(rproc_id_t rproc_id, struct notifier_block *nb)
{
	struct ipc_rproc_info *rproc = NULL;

	if (!nb) {
		rproc_pr_err("invalid notifier block");
		return -EINVAL;
	}

	rproc = find_rproc(rproc_id);
	if (!rproc) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}
	atomic_notifier_chain_register(&rproc->notifier, nb);
	return 0;
}
EXPORT_SYMBOL(hisi_rproc_rx_register);

int hisi_rproc_rx_unregister(rproc_id_t rproc_id, struct notifier_block *nb)
{
	struct ipc_rproc_info *rproc = NULL;

	if (!nb) {
		rproc_pr_err("invalid notifier block");
		return -EINVAL;
	}

	rproc = find_rproc(rproc_id);
	if (!rproc) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return  -EINVAL;
	}
	atomic_notifier_chain_unregister(&rproc->notifier, nb);
	return 0;
}
EXPORT_SYMBOL(hisi_rproc_rx_unregister);

/*
 * Release the ipc channel's structure, it's usually called by
 * module_exit function, but the module_exit function should never be used.
 * @rproc_id_t
 * @return   0:succeed, other:failed
 */
int hisi_rproc_put(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;
	unsigned int i;

	for (i = 0; i < g_rproc_table.rproc_cnt; i++) {
		rproc = &g_rproc_table.rproc_arr[i];
		if (rproc->mbox && rproc_id == rproc->mbox->rproc_id) {
			ipc_mbox_put(&rproc->mbox);
			break;
		}
	}

	if (unlikely(g_rproc_table.rproc_cnt == i)) {
		if (!rproc)
			rproc_pr_err("[%s]rproc pointer is null!\n", __func__);
		else
			rproc_pr_err("\nrelease the ipc channel %d's failed\n",
				rproc_id);

		return -ENODEV;
	}
	return 0;
}

/*
 * Flush the tx_work queue.
 * @rproc_id_t
 * @return   0:succeed, other:failed
 */
int hisi_rproc_flush_tx(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;
	unsigned int i;

	for (i = 0; i < g_rproc_table.rproc_cnt; i++) {
		rproc = &g_rproc_table.rproc_arr[i];
		if (!rproc->mbox)
			continue;
		if (rproc->mbox->tx && rproc_id == rproc->mbox->rproc_id) {
			ipc_mbox_empty_task(rproc->mbox->tx);
			break;
		}
	}

	if (unlikely(g_rproc_table.rproc_cnt == i))
		return -ENODEV;

	return 0;
}

/*
 * Judge the mailbox is suspend.
 * @rproc_id_t
 * @return   0:succeed, other:failed
 */
int hisi_rproc_is_suspend(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;
	struct hisi_mbox_device *mdev = NULL;
	unsigned long flags = 0;
	int ret = 0;

	rproc = find_rproc(rproc_id);
	if (!rproc || !rproc->mbox || !rproc->mbox->tx) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	mdev = rproc->mbox->tx;
	spin_lock_irqsave(&mdev->status_lock, flags);
	if ((MDEV_DEACTIVATED & mdev->status))
		ret = -ENODEV;
	spin_unlock_irqrestore(&mdev->status_lock, flags);
	return ret;
}
EXPORT_SYMBOL(hisi_rproc_is_suspend);

static void ipc_add_rproc_info(struct hisi_mbox_device *mdev)
{
	struct ipc_rproc_info *rproc = NULL;
	int rproc_id;

	if (g_rproc_table.rproc_cnt >= ARRAY_SIZE(g_rproc_table.rproc_arr)) {
		rproc_pr_debug("rproc table is overload");
		return;
	}

	rproc_id = mdev->ops->get_rproc_id(mdev);
	rproc = &g_rproc_table.rproc_arr[g_rproc_table.rproc_cnt];
	if (rproc->mbox) {
		rproc_pr_debug("rproc-%d has init", rproc_id);
		return;
	}

	ATOMIC_INIT_NOTIFIER_HEAD(&rproc->notifier);

	rproc->nb.next = NULL;
	rproc->nb.notifier_call = ipc_rproc_rx_notifier;
	/*
	 * rproc_id as mdev_index to get the right mailbox-dev
	 */
	rproc->mbox = ipc_mbox_get(rproc_id, &rproc->nb);
	if (!rproc->mbox) {
		rproc_pr_debug("%s rproc[%d] mbox is not exist",
			__func__, rproc_id);
		return;
	}

	g_rproc_table.rproc_cnt++;
	rproc_pr_debug("%s get mbox rproc[%d]", __func__, rproc_id);
}

int ipc_rproc_get_ipc_version(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;
	struct hisi_mbox_device *mdev = NULL;

	rproc = find_rproc(rproc_id);
	if (!rproc || !rproc->mbox) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	mdev = rproc->mbox->tx ? rproc->mbox->tx : rproc->mbox->rx;
	if (!mdev)
		return -EINVAL;

	return mdev->ops->get_ipc_version(mdev);
}
EXPORT_SYMBOL(ipc_rproc_get_ipc_version);

int hisi_rproc_get_channel_size(rproc_id_t rproc_id)
{
	struct ipc_rproc_info *rproc = NULL;

	rproc = find_rproc(rproc_id);
	if (!rproc || !rproc->mbox) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	return get_rproc_channel_size(rproc);
}
EXPORT_SYMBOL(hisi_rproc_get_channel_size);

int ipc_rproc_get_mailbox_id(rproc_id_t rproc_id)
{
	struct hisi_mbox_device *mdev = NULL;
	struct ipc_rproc_info *rproc = NULL;

	rproc = find_rproc(rproc_id);
	if (!rproc || !rproc->mbox) {
		rproc_pr_err("%s:invalid rproc xfer:%u", __func__, rproc_id);
		return -EINVAL;
	}

	mdev = rproc->mbox->tx ? rproc->mbox->tx : rproc->mbox->rx;
	if (!mdev)
		return -EINVAL;

	return mdev->ops->get_channel_id(mdev);
}
EXPORT_SYMBOL(ipc_rproc_get_mailbox_id);

int ipc_rproc_init(struct hisi_mbox_device **mdev_list)
{
	struct hisi_mbox_device *mdev = NULL;
	unsigned int i;

	for (i = 0; (mdev = mdev_list[i]); i++)
		ipc_add_rproc_info(mdev);

	return 0;
}
EXPORT_SYMBOL(ipc_rproc_init);

static void __exit ipc_rproc_exit(void)
{
	struct ipc_rproc_info *rproc = NULL;
	unsigned int i;

	for (i = 0; i < g_rproc_table.rproc_cnt; i++) {
		rproc = &g_rproc_table.rproc_arr[i];
		if (rproc->mbox)
			ipc_mbox_put(&rproc->mbox);
	}
	g_rproc_table.rproc_cnt = 0;
}

module_exit(ipc_rproc_exit);
/*lint +e580 */
