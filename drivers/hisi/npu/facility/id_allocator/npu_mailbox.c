/*
 * npu_mailbox.c
 *
 * about npu mailbox
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "npu_common.h"
#include "npu_doorbell.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_shm.h"
#include "npu_adapter.h"

extern struct devdrv_mem_info g_shm_desc[NPU_DEV_NUM][DEVDRV_MAX_MEM];

static int devdrv_mailbox_send_message_check(const struct devdrv_mailbox *mailbox,
                                             const struct devdrv_mailbox_message *message_info,
                                             const int *result)
{
	if (mailbox == NULL || message_info == NULL || result == NULL) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	if (mailbox->working == 0) {
		NPU_DRV_ERR("mailbox not working\n");
		return -EINVAL;
	}

	if (message_info->message_length > DEVDRV_MAILBOX_PAYLOAD_LENGTH) {
		NPU_DRV_ERR("message length is too long\n");
		return -EINVAL;
	}
	return 0;
}

static int devdrv_mailbox_message_send_trigger(struct devdrv_mailbox *mailbox,
                                               struct devdrv_mailbox_message *message)
{
	int ret;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_mailbox_message_header *header = (struct devdrv_mailbox_message_header *)mailbox->send_sram;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_ops failed\n");
		return -EINVAL;
	}

	ret = DEVDRV_PLAT_GET_RES_MAILBOX_SEND(plat_info) (mailbox->send_sram, DEVDRV_MAILBOX_PAYLOAD_LENGTH,
		message->message_payload, message->message_length);
	if (ret != 0) {
		NPU_DRV_ERR("npu_mailbox_send failed. ret[%d], valid[%u], cmd_type[%u], result[%u]\n",
			ret, header->valid, header->cmd_type, header->result);
		return ret;
	}

	message->is_sent = 1;
	ret =  devdrv_write_doorbell_val(DOORBELL_RES_MAILBOX, DOORBELL_MAILBOX_MAX_SIZE-1, 0);
	NPU_DRV_INFO("mailbox send ret[%d], valid[%u], cmd_type[%u], result[%u]\n",
		ret, header->valid, header->cmd_type, header->result);

	return ret;
}

static void devdrv_mailbox_ack_send_work(struct work_struct *work)
{
	struct devdrv_mailbox_message_header *header = NULL;
	struct devdrv_mailbox_message *next_message = NULL;
	struct devdrv_mailbox_message *message = NULL;
	int ret;

	message = container_of(work, struct devdrv_mailbox_message, send_work);

	if (message == NULL) {
		NPU_DRV_ERR("message is null\n");
		return;
	}

	list_del(&message->send_list_node);

	message->mailbox->send_queue.status = DEVDRV_MAILBOX_BUSY;
	if (!list_empty_careful(&message->mailbox->send_queue.list_header)) {
		next_message = list_first_entry(&message->mailbox->send_queue.list_header,
			struct devdrv_mailbox_message, send_list_node);
		ret = devdrv_mailbox_message_send_trigger(message->mailbox, next_message);
		if (ret != 0) {
			NPU_DRV_ERR("message length is too long\n");
			kfree(message->message_payload);
			message->message_payload = NULL;
			kfree(message);
			message = NULL;
			return;
		}
	} else {
		message->mailbox->send_queue.status = DEVDRV_MAILBOX_FREE;
	}

	// recycle message if it's abandoned
	if (message->abandon == DEVDRV_MAILBOX_RECYCLE_MESSAGE) {
		kfree(message->message_payload);
		message->message_payload = NULL;
		kfree(message);
		message = NULL;
		return;
	}
	// read process result add by TS
	header = (struct devdrv_mailbox_message_header *)message->mailbox->send_sram;
	message->process_result = header->result;

	// distribute message
	if (message->message_type == DEVDRV_MAILBOX_SYNC_MESSAGE)
		up(&message->wait);
}

static void devdrv_delete_message(struct devdrv_mailbox_message *message)
{
	message->process_result = -1;
	if (message->abandon == DEVDRV_MAILBOX_RECYCLE_MESSAGE)
		goto out;

	message->abandon = DEVDRV_MAILBOX_RECYCLE_MESSAGE;

	up(&message->wait);
	return;

out:
	kfree(message->message_payload);
	message->message_payload = NULL;
	kfree(message);
	message = NULL;
}

static int devdrv_mailbox_message_create(const struct devdrv_mailbox *mailbox,
                                         const u8 *buf, u32 len,
                                         struct devdrv_mailbox_message **message_ptr)
{
	int i;
	struct devdrv_mailbox_message *message = NULL;

	if (mailbox == NULL || buf == NULL ||
		len < sizeof(struct devdrv_mailbox_message_header) ||
		len > DEVDRV_MAILBOX_PAYLOAD_LENGTH) {
		NPU_DRV_ERR("input argument invalid\n");
		return -EINVAL;
	}

	message = kzalloc(sizeof(struct devdrv_mailbox_message), GFP_KERNEL);
	if (message == NULL) {
		NPU_DRV_ERR("kmalloc failed\n");
		return -ENOMEM;
	}

	message->message_payload = NULL;
	message->message_payload = kzalloc(DEVDRV_MAILBOX_PAYLOAD_LENGTH, GFP_KERNEL);
	if (message->message_payload == NULL) {
		kfree(message);
		message = NULL;
		NPU_DRV_ERR("kmalloc message_payload failed\n");
		return -ENOMEM;
	}

	memcpy(message->message_payload, buf, len);
	for (i = len; i < DEVDRV_MAILBOX_PAYLOAD_LENGTH; i++)
		message->message_payload[i] = 0;

	message->message_length = DEVDRV_MAILBOX_PAYLOAD_LENGTH;
	message->process_result = 0;
	message->sync_type = DEVDRV_MAILBOX_SYNC;
	message->cmd_type = 0;
	message->message_index = 0;
	message->message_pid = 0;
	message->mailbox = (struct devdrv_mailbox *)mailbox;
	message->abandon = DEVDRV_MAILBOX_VALID_MESSAGE;

	*message_ptr = message;

	return 0;
}

static irqreturn_t devdrv_mailbox_ack_irq(int irq, void *data)
{
	struct devdrv_mailbox_message *message = NULL;
	struct devdrv_mailbox *mailbox = NULL;
	unsigned long flags;

	mailbox = (struct devdrv_mailbox *)data;

	spin_lock_irqsave(&(mailbox->send_queue.spinlock), flags); // protect message
	if (mailbox->send_queue.message != NULL) {
		message = (struct devdrv_mailbox_message *)(mailbox->send_queue.message);
		up(&(message->wait));
	}

	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	return IRQ_HANDLED;
}

int devdrv_mailbox_init(u8 dev_id)
{
	int ret;
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox *mailbox = NULL;
	struct devdrv_platform_info *plat_info = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(dev_ctx == NULL, -ENODATA, "get device context by device id failed\n");

	mailbox = &dev_ctx->mailbox;

	plat_info = devdrv_plat_get_info();
	COND_RETURN_ERROR(plat_info == NULL, -EINVAL, "devdrv_plat_get_info\n");

	COND_RETURN_ERROR(mailbox == NULL, -EINVAL, "input argument error\n");

	// init
	spin_lock_init(&mailbox->send_queue.spinlock);
	INIT_LIST_HEAD(&mailbox->send_queue.list_header);

	// init send queue
	spin_lock(&mailbox->send_queue.spinlock);
	mailbox->send_queue.mailbox_type = DEVDRV_MAILBOX_SRAM;
	mailbox->send_queue.status = DEVDRV_MAILBOX_FREE;
	spin_unlock(&mailbox->send_queue.spinlock);

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		mailbox->send_sram = (u8 *)(uintptr_t)(g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr +
			g_shm_desc[dev_id][DEVDRV_INFO_MEM].size - DEVDRV_MAILBOX_PAYLOAD_LENGTH * 4);
		mailbox->receive_sram = mailbox->send_sram + DEVDRV_MAILBOX_PAYLOAD_LENGTH;
	} else {
		mailbox->send_sram = (u8 *) DEVDRV_PLAT_GET_REG_VADDR(plat_info, DEVDRV_REG_TS_SRAM);
		mailbox->receive_sram = mailbox->send_sram + DEVDRV_MAILBOX_PAYLOAD_LENGTH;
	}

	mailbox->send_queue.wq = create_workqueue("devdrv-mb-send");
	if (mailbox->send_queue.wq == NULL) {
		NPU_DRV_ERR("create send workqueue error\n");
		ret = -ENOMEM;
		return ret;
	}
	// register irq handler
	ret = request_irq(DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info), devdrv_mailbox_ack_irq,
		IRQF_TRIGGER_NONE, "devdrv-ack", mailbox);
	COND_RETURN_ERROR(ret != 0, ret, "request_irq ack irq failed ret %d\n", ret);

	mailbox->working = 1;
	mutex_init(&mailbox->send_mutex);
	mailbox->send_queue.message = NULL;

	return 0;
}
EXPORT_SYMBOL(devdrv_mailbox_init);

int devdrv_mailbox_message_send_for_res(u8 dev_id, const u8 *buf, u32 len, int *result)
{
	int ret;
	struct devdrv_mailbox_message *message = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox *mailbox = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("device id is illegal\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id failed\n");
		return -ENODATA;
	}

	mailbox = &dev_ctx->mailbox;
	// create message
	ret = devdrv_mailbox_message_create(mailbox, buf, len, &message);
	if (ret != 0) {
		NPU_DRV_ERR("create mailbox message failed\n");
		return -1;
	}
	// send message
	ret = devdrv_mailbox_message_send_ext(mailbox, message, result);
	if (ret != 0) {
		NPU_DRV_ERR("devdrv_mailbox_message_send failed\n");
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
EXPORT_SYMBOL(devdrv_mailbox_message_send_for_res);

static int devdrv_mailbox_sync_message_timeout_send(struct devdrv_mailbox *mailbox,
                                                    struct devdrv_mailbox_message *message)
{
	int ret;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct devdrv_mailbox_message *next_message = NULL;

	spin_lock(&mailbox->send_queue.spinlock);
	message->abandon = DEVDRV_MAILBOX_RECYCLE_MESSAGE;
	if (!list_empty_careful(&mailbox->send_queue.list_header)) {
		list_for_each_safe(pos, n, &mailbox->send_queue.list_header) {
			if (pos == &message->send_list_node) {
				list_del(&message->send_list_node);
				break;
			}
		}
	}

	if (message->is_sent) {
		message->mailbox->send_queue.status = DEVDRV_MAILBOX_BUSY;
		if (!list_empty_careful(&message->mailbox->send_queue.list_header)) {
			next_message = list_first_entry(&message->mailbox->send_queue.list_header,
				struct devdrv_mailbox_message, send_list_node);
			ret = devdrv_mailbox_message_send_trigger(message->mailbox, next_message);
			spin_unlock(&mailbox->send_queue.spinlock);
			return ret;
		} else {
			message->mailbox->send_queue.status = DEVDRV_MAILBOX_FREE;
		}
	}
	spin_unlock(&mailbox->send_queue.spinlock);

	return -1;
}

static int devdrv_mailbox_sync_message_send(struct devdrv_mailbox *mailbox,
                                            struct devdrv_mailbox_message *message, int *result)
{
	u64 jiffy;
	int ret;

	jiffy = msecs_to_jiffies(DEVDRV_MAILBOX_SEMA_TIMEOUT_SECOND * 100);

	ret = down_timeout(&message->wait, jiffy);
	if (ret != 0) {
		NPU_DRV_ERR("mailbox down timeout\n");
		return devdrv_mailbox_sync_message_timeout_send(mailbox, message);
	} else {
		if (message->abandon == DEVDRV_MAILBOX_VALID_MESSAGE) {
			*result = message->process_result;
			ret = 0;
		} else {
			NPU_DRV_ERR("message is not valid\n");
			ret = -1;
		}
	}

	return ret;
}

int devdrv_mailbox_message_send_ext(struct devdrv_mailbox *mailbox,
                                    struct devdrv_mailbox_message *message, int *result)
{
	int ret;
	unsigned long flags;
	u64 jiffy;
	struct devdrv_mailbox_message_header *header = NULL;

	NPU_DRV_DEBUG("enter\n");

	// check input para
	ret = devdrv_mailbox_send_message_check(mailbox, message, result);
	if (ret != 0) {
		NPU_DRV_ERR("create mailbox message faled\n");
		return ret;
	}

	// fill message
	header = (struct devdrv_mailbox_message_header *)message->message_payload;
	header->result = 0;
	header->valid = DEVDRV_MAILBOX_MESSAGE_VALID;

	message->process_result = 0;
	message->is_sent = 0;
	sema_init(&message->wait, 0);

	// send message
	mutex_lock(&mailbox->send_mutex);  // protect send, avoid multithread problem

	spin_lock_irqsave(&(mailbox->send_queue.spinlock), flags);  // protect message
	mailbox->send_queue.message = message;
	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	ret = devdrv_mailbox_message_send_trigger(mailbox, message);
	if (ret != 0)
		goto failed;
	jiffy = msecs_to_jiffies(DEVDRV_MAILBOX_SEMA_TIMEOUT_SECOND * 100);
	ret = down_timeout(&message->wait, jiffy);
	if (ret == 0) {
		*result = message->process_result;
	} else {
		header = (struct devdrv_mailbox_message_header *)mailbox->send_sram;
		NPU_DRV_ERR("mailbox down timeout. ret[%d], valid[0x%x], cmd_type[%u], result[%u]\n",
			ret, header->valid, header->cmd_type, header->result);
	}

failed:
	spin_lock_irqsave(&mailbox->send_queue.spinlock, flags);  // protect message
	mailbox->send_queue.message = NULL;
	spin_unlock_irqrestore(&(mailbox->send_queue.spinlock), flags);

	mutex_unlock(&mailbox->send_mutex);

	return ret;
}

int devdrv_mailbox_message_send(struct devdrv_mailbox *mailbox,
                                struct devdrv_mailbox_message *message, int *result)
{
	int ret;
	struct devdrv_mailbox_message_header *header = NULL;

	if (mailbox == NULL || message == NULL) {
		NPU_DRV_ERR("mailbox or message is null\n");
		return -1;
	}

	// check input para
	ret = devdrv_mailbox_send_message_check(mailbox, message, result);
	if (ret != 0) {
		NPU_DRV_ERR("create mailbox message faled\n");
		return ret;
	}
	// get mailbox message head
	header = (struct devdrv_mailbox_message_header *)message->message_payload;
	header->result = 0;
	header->valid = DEVDRV_MAILBOX_MESSAGE_VALID;

	message->process_result = 0;
	message->is_sent = 0;
	if (message->sync_type == DEVDRV_MAILBOX_SYNC) {
		sema_init(&message->wait, 0);
		message->message_type = DEVDRV_MAILBOX_SYNC_MESSAGE;
	} else {
		message->message_type = DEVDRV_MAILBOX_ASYNC_MESSAGE;
	}

	// init work
	INIT_WORK(&message->send_work, devdrv_mailbox_ack_send_work);

	// send message
	list_add_tail(&message->send_list_node, &mailbox->send_queue.list_header);

	if (mailbox->send_queue.status == DEVDRV_MAILBOX_FREE) {
		mailbox->send_queue.status = DEVDRV_MAILBOX_BUSY;
		(void)devdrv_mailbox_message_send_trigger(mailbox, message);
	}

	if (message->sync_type == DEVDRV_MAILBOX_SYNC)
		return devdrv_mailbox_sync_message_send(mailbox, message, result);

	return 0;
}
EXPORT_SYMBOL(devdrv_mailbox_message_send);

void devdrv_mailbox_recycle(struct devdrv_mailbox *mailbox)
{
	struct devdrv_mailbox_message *message = NULL;
	struct list_head *pos = NULL, *n = NULL;

	spin_lock(&mailbox->send_queue.spinlock);
	if (!list_empty_careful(&mailbox->send_queue.list_header)) {
		list_for_each_safe(pos, n, &mailbox->send_queue.list_header) {
			message = list_entry(pos, struct devdrv_mailbox_message, send_list_node);
			list_del(pos);
			devdrv_delete_message(message);
		}
	}
	spin_unlock(&mailbox->send_queue.spinlock);
}

void devdrv_mailbox_exit(struct devdrv_mailbox *mailbox)
{
	struct devdrv_platform_info *plat_info = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return;
	}

	if (mailbox == NULL) {
		NPU_DRV_ERR("input argument error\n");
		return;
	}

	// register irq handler
	free_irq(DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info), mailbox);
	destroy_workqueue(mailbox->send_queue.wq);
	mailbox->working = 0;
	devdrv_mailbox_recycle(mailbox);
}

void devdrv_mailbox_destroy(int dev_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox *mailbox = NULL;
	struct devdrv_platform_info *plat_info = NULL;

	if ((dev_id < 0) || (dev_id >= NPU_DEV_NUM)) {
		NPU_DRV_ERR("device id is illegal\n");
		return;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("get device context by device id %d failed\n", dev_id);
		return;
	}

	mailbox = &dev_ctx->mailbox;
	if (mailbox == NULL) {
		NPU_DRV_ERR("npu devid %d mailbox argument error\n", dev_id);
		return;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return;
	}

	// register irq handler
	free_irq(DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info), mailbox);
	destroy_workqueue(mailbox->send_queue.wq);
	mailbox->working = 0;
	mutex_destroy(&mailbox->send_mutex);
	mailbox->send_queue.message = NULL;
}

