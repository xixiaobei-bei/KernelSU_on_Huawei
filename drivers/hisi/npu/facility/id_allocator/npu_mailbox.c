/*
 * npu_mailbox.c
 *
 * about npu mailbox
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_mailbox.h"

#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <securec.h>
#include "npu_common.h"
#include "npu_doorbell.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_shm.h"
#include "npu_adapter.h"
#include "npu_pm_framework.h"

extern struct npu_mem_info g_shm_desc[NPU_DEV_NUM][NPU_MAX_MEM];

static int npu_mailbox_send_message_check(
	const struct npu_mailbox *mailbox,
	const struct npu_mailbox_message *message_info, const int *result)
{
	if (mailbox == NULL || message_info == NULL || result == NULL) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	if (mailbox->working == 0) {
		npu_drv_err("mailbox not working\n");
		return -EINVAL;
	}

	if (message_info->message_length > NPU_MAILBOX_PAYLOAD_LENGTH) {
		npu_drv_err("message length is too long\n");
		return -EINVAL;
	}
	return 0;
}

static int npu_mailbox_message_send_trigger(struct npu_mailbox *mailbox,
	struct npu_mailbox_message *message)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_mailbox_message_header *header = NULL;

	if (mailbox == NULL || message == NULL) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	dev_ctx = container_of(mailbox, struct npu_dev_ctx, mailbox);
	if (dev_ctx == NULL) {
		npu_drv_err("get dev_ctx failed\n");
		return -EINVAL;
	}
	header = (struct npu_mailbox_message_header *)mailbox->send_sram;

	if (dev_ctx->power_stage != NPU_PM_UP) {
		npu_drv_err("npu is powered off, power_stage[%u]\n",
			dev_ctx->power_stage);
		return -EINVAL;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_ops failed\n");
		return -EINVAL;
	}

	ret = plat_info->adapter.res_ops.npu_mailbox_send(mailbox->send_sram,
		NPU_MAILBOX_PAYLOAD_LENGTH,
		message->message_payload, message->message_length);
	if (ret != 0) {
		npu_drv_err("npu_mailbox_send failed. ret[%d], valid[%u], cmd_type[%u], result[%u]\n",
			ret, header->valid, header->cmd_type, header->result);
		return ret;
	}

	message->is_sent = 1;
	ret = npu_write_doorbell_val(DOORBELL_RES_MAILBOX,
		DOORBELL_MAILBOX_MAX_SIZE - 1, 0);
	npu_drv_info("mailbox send ret[%d], valid[%u], cmd_type[%u], result[%u]\n",
		ret, header->valid, header->cmd_type, header->result);

	return ret;
}

static void npu_delete_message(struct npu_mailbox_message *message)
{
	message->process_result = -1;
	if (message->abandon == NPU_MAILBOX_RECYCLE_MESSAGE)
		goto out;

	message->abandon = NPU_MAILBOX_RECYCLE_MESSAGE;

	up(&message->wait);
	return;

out:
	kfree(message->message_payload);
	message->message_payload = NULL;
	kfree(message);
	message = NULL;
}

static int npu_mailbox_message_create(const struct npu_mailbox *mailbox,
	const u8 *buf, u32 len, struct npu_mailbox_message **message_ptr)
{
	int i;
	int ret;
	struct npu_mailbox_message *message = NULL;

	if (mailbox == NULL || buf == NULL ||
		len < sizeof(struct npu_mailbox_message_header) ||
		len > NPU_MAILBOX_PAYLOAD_LENGTH) {
		npu_drv_err("input argument invalid\n");
		return -EINVAL;
	}

	message = kzalloc(sizeof(struct npu_mailbox_message), GFP_KERNEL);
	if (message == NULL) {
		npu_drv_err("kmalloc failed\n");
		return -ENOMEM;
	}

	message->message_payload = NULL;
	message->message_payload = kzalloc(NPU_MAILBOX_PAYLOAD_LENGTH,
		GFP_KERNEL);
	if (message->message_payload == NULL) {
		kfree(message);
		message = NULL;
		npu_drv_err("kmalloc message_payload failed\n");
		return -ENOMEM;
	}

	ret = memcpy_s(message->message_payload, NPU_MAILBOX_PAYLOAD_LENGTH,
		buf, len);
	if (ret != 0)
		npu_drv_err("memcpy_s message_payload failed\n");

	for (i = len; i < NPU_MAILBOX_PAYLOAD_LENGTH; i++)
		message->message_payload[i] = 0;

	message->message_length = NPU_MAILBOX_PAYLOAD_LENGTH;
	message->process_result = 0;
	message->sync_type = NPU_MAILBOX_SYNC;
	message->cmd_type = 0;
	message->message_index = 0;
	message->message_pid = 0;
	message->mailbox = (struct npu_mailbox *)mailbox;
	message->abandon = NPU_MAILBOX_VALID_MESSAGE;

	*message_ptr = message;

	return 0;
}

irqreturn_t npu_mailbox_ack_irq(int irq, void *data)
{
	struct npu_mailbox_message *message = NULL;
	struct npu_mailbox *mailbox = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	unsigned long flags;

	mailbox = (struct npu_mailbox *)data;
	cur_dev_ctx = container_of(mailbox, struct npu_dev_ctx, mailbox);
	if (cur_dev_ctx->power_stage != NPU_PM_UP)
		return IRQ_NONE;

	// protect message
	spin_lock_irqsave(&(mailbox->send_queue.spinlock), flags);
	npu_plat_handle_irq_tophalf(NPU_IRQ_MAILBOX_ACK);
	if (mailbox->send_queue.message != NULL) {
		message = (struct npu_mailbox_message *)(
			mailbox->send_queue.message);
		up(&(message->wait));
	}

	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(npu_mailbox_ack_irq);

int npu_mailbox_init(u8 dev_id)
{
	int ret;
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_mailbox *mailbox = NULL;
	struct npu_platform_info *plat_info = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(dev_ctx == NULL, -ENODATA,
		"get device context by device id failed\n");

	mailbox = &dev_ctx->mailbox;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	cond_return_error(mailbox == NULL, -EINVAL, "input argument error\n");

	// init
	spin_lock_init(&mailbox->send_queue.spinlock);
	INIT_LIST_HEAD(&mailbox->send_queue.list_header);

	// init send queue
	spin_lock(&mailbox->send_queue.spinlock);
	mailbox->send_queue.mailbox_type = NPU_MAILBOX_SRAM;
	mailbox->send_queue.status = NPU_MAILBOX_FREE;
	spin_unlock(&mailbox->send_queue.spinlock);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		cond_return_error(
			g_shm_desc[dev_id][NPU_INFO_MEM].size < NPU_MAILBOX_PAYLOAD_LENGTH * 4,
			-EINVAL, "NPU_INFO_MEM size invalid, size = %zu, len = %d\n",
			g_shm_desc[dev_id][NPU_INFO_MEM].size, NPU_MAILBOX_PAYLOAD_LENGTH * 4);
		mailbox->send_sram = (u8 *)(uintptr_t)(
			g_shm_desc[dev_id][NPU_INFO_MEM].virt_addr +
			g_shm_desc[dev_id][NPU_INFO_MEM].size -
			NPU_MAILBOX_PAYLOAD_LENGTH * 4);
		mailbox->receive_sram = mailbox->send_sram +
			NPU_MAILBOX_PAYLOAD_LENGTH;
	} else {
		mailbox->send_sram =
			(u8 *)plat_info->dts_info.reg_vaddr[NPU_REG_TS_SRAM];
		mailbox->receive_sram = mailbox->send_sram +
			NPU_MAILBOX_PAYLOAD_LENGTH;
	}

	mailbox->send_queue.wq = create_workqueue("devdrv-mb-send");
	if (mailbox->send_queue.wq == NULL) {
		npu_drv_err("create send workqueue error\n");
		ret = -ENOMEM;
		return ret;
	}
	// register irq handler
	ret = request_irq(plat_info->dts_info.irq_mailbox_ack,
		npu_mailbox_ack_irq, IRQF_TRIGGER_NONE, "devdrv-ack", mailbox);
	cond_return_error(ret != 0, ret, "request_irq ack irq failed ret %d\n",
		ret);

	mailbox->working = 1;
	mutex_init(&mailbox->send_mutex);
	mailbox->send_queue.message = NULL;

	return 0;
}
EXPORT_SYMBOL(npu_mailbox_init);

int npu_mailbox_message_send_for_res(u8 dev_id, const u8 *buf, u32 len,
	int *result)
{
	int ret;
	struct npu_mailbox_message *message = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_mailbox *mailbox = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		npu_drv_err("get device context by device id failed\n");
		return -ENODATA;
	}

	mailbox = &dev_ctx->mailbox;
	// create message
	ret = npu_mailbox_message_create(mailbox, buf, len, &message);
	if (ret != 0) {
		npu_drv_err("create mailbox message failed\n");
		return -1;
	}
	// send message
	ret = npu_mailbox_message_send_ext(mailbox, message, result);
	if (ret != 0) {
		npu_drv_err("npu_mailbox_message_send_ext failed\n");
		ret = -1;
	} else {
		ret = 0;
	}

	if (message != NULL) {
		if (message->message_payload != NULL) {
			kfree(message->message_payload);
			message->message_payload = NULL;
		}
		kfree(message);
		message = NULL;
	}

	return ret;
}
EXPORT_SYMBOL(npu_mailbox_message_send_for_res);

int npu_mailbox_message_send_ext(struct npu_mailbox *mailbox,
	struct npu_mailbox_message *message, int *result)
{
	int ret;
	unsigned long flags;
	u64 jiffy;
	struct npu_mailbox_message_header *header = NULL;

	npu_drv_debug("enter\n");

	// check input para
	ret = npu_mailbox_send_message_check(mailbox, message, result);
	if (ret != 0) {
		npu_drv_err("create mailbox message faled\n");
		return ret;
	}

	// fill message
	header = (struct npu_mailbox_message_header *)message->message_payload;
	header->result = 0;
	header->valid = NPU_MAILBOX_MESSAGE_VALID;

	message->process_result = 0;
	message->is_sent = 0;
	sema_init(&message->wait, 0);

	// send message
	// protect send, avoid multithread problem

	mutex_lock(&mailbox->send_mutex);

	// protect message
	spin_lock_irqsave(&(mailbox->send_queue.spinlock), flags);
	mailbox->send_queue.message = message;
	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	ret = npu_mailbox_message_send_trigger(mailbox, message);
	if (ret != 0)
		goto failed;
	jiffy = msecs_to_jiffies(NPU_MAILBOX_SEND_TIMEOUT_SECOND);
	ret = down_timeout(&message->wait, jiffy);
	if (ret == 0) {
		*result = message->process_result;
	} else {
		header = (struct npu_mailbox_message_header *)mailbox->send_sram;
		npu_drv_err("mailbox down timeout. ret[%d], valid[0x%x], cmd_type[%u], result[%u]\n",
			ret, header->valid, header->cmd_type, header->result);
	}

failed:
	// protect message
	spin_lock_irqsave(&mailbox->send_queue.spinlock, flags);
	mailbox->send_queue.message = NULL;
	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	mutex_unlock(&mailbox->send_mutex);

	return ret;
}

void npu_mailbox_recycle(struct npu_mailbox *mailbox)
{
	struct npu_mailbox_message *message = NULL;
	struct list_head *pos = NULL, *n = NULL;

	spin_lock(&mailbox->send_queue.spinlock);
	if (!list_empty_careful(&mailbox->send_queue.list_header)) {
		list_for_each_safe(pos, n, &mailbox->send_queue.list_header) {
			message = list_entry(pos, struct npu_mailbox_message,
				send_list_node);
			list_del(pos);
			npu_delete_message(message);
		}
	}
	spin_unlock(&mailbox->send_queue.spinlock);
}

void npu_mailbox_exit(struct npu_mailbox *mailbox)
{
	struct npu_platform_info *plat_info = NULL;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return;
	}

	if (mailbox == NULL) {
		npu_drv_err("input argument error\n");
		return;
	}

	// register irq handler
	free_irq(plat_info->dts_info.irq_mailbox_ack, mailbox);
	destroy_workqueue(mailbox->send_queue.wq);
	mailbox->working = 0;
	npu_mailbox_recycle(mailbox);
}

void npu_mailbox_destroy(int dev_id)
{
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_mailbox *mailbox = NULL;
	struct npu_platform_info *plat_info = NULL;

	if ((dev_id < 0) || (dev_id >= NPU_DEV_NUM)) {
		npu_drv_err("device id is illegal\n");
		return;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		npu_drv_err("get device context by device id %d failed\n", dev_id);
		return;
	}

	mailbox = &dev_ctx->mailbox;
	if (mailbox == NULL) {
		npu_drv_err("npu devid %d mailbox argument error\n", dev_id);
		return;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return;
	}

	// register irq handler
	free_irq(plat_info->dts_info.irq_mailbox_ack, mailbox);
	destroy_workqueue(mailbox->send_queue.wq);
	mailbox->working = 0;
	mutex_destroy(&mailbox->send_mutex);
	mailbox->send_queue.message = NULL;
}

