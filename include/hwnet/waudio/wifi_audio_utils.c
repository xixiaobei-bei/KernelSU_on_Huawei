/*
 * wifi_audio_utils.c
 *
 * wifi audio kernel module tool function implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "wifi_audio_utils.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <securec.h>

static int log_flag = WIFI_AUDIO_LOG_HI;

int get_log_flag(void)
{
	return log_flag;
}
struct list_node *alloc_list_node(const void *data, unsigned int len)
{
	struct list_node *node = NULL;
	int sec_ret;
	unsigned int size;

	if (len > LIST_NODE_DATA_LEN_MAX) {
		wifi_audio_log_err("len %u > LIST_NODE_DATA_LEN_MAX", len);
		return NULL;
	}
	if (data == NULL) {
		wifi_audio_log_err("data is NULL");
		return NULL;
	}

	size = len + sizeof(*node);
	node = (struct list_node *)kmalloc(size, GFP_ATOMIC);
	if (node == NULL) {
		wifi_audio_log_err("kmalloc failed");
		return NULL;
	}
	node->len = len;
	sec_ret = memcpy_s(node->data, len, data, len);
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy data failed! sec_ret = %d", sec_ret);
		kfree(node);
		return NULL;
	}
	return node;
}

void free_list_node(struct list_node *node)
{
	if (node != NULL)
		kfree(node);
	return;
}

void list_queue_init(struct list_queue *queue)
{
	if (queue == NULL) {
		wifi_audio_log_err("queue is NULL");
		return;
	}
	INIT_LIST_HEAD(&(queue->queue));
	queue->len = 0;
	spin_lock_init(&(queue->lock));
	return;
}

void list_queue_enqueue(struct list_queue *queue, struct list_node *node)
{
	unsigned long flags = 0;

	if ((queue == NULL) || (node == NULL)) {
		wifi_audio_log_err("queue or node is NULL");
		return;
	}
	spin_lock_irqsave(&(queue->lock), flags);
	list_add_tail(&node->list, &(queue->queue));
	queue->len++;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return;
}

struct list_node *list_queue_dequeue(struct list_queue *queue)
{
	struct list_node *node = NULL;
	unsigned long flags = 0;

	if (queue == NULL) {
		wifi_audio_log_err("queue is NULL");
		return NULL;
	}
	spin_lock_irqsave(&(queue->lock), flags);
	node = list_first_entry(&(queue->queue), struct list_node, list);
	if (node == NULL) {
		spin_unlock_irqrestore(&(queue->lock), flags);
		wifi_audio_log_err("get list first entry is NULL");
		return NULL;
	}
	list_del(&node->list);
	queue->len--;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return node;
}

int list_queue_is_empty(struct list_queue *queue)
{
	unsigned long flags = 0;
	int ret;

	if (queue == NULL) {
		wifi_audio_log_err("queue is NULL");
		return 1; /* when queue is invalid, return 1 */
	}
	spin_lock_irqsave(&(queue->lock), flags);
	ret = list_empty(&(queue->queue));
	spin_unlock_irqrestore(&(queue->lock), flags);
	return ret;
}

unsigned int list_queue_len(struct list_queue *queue)
{
	unsigned long flags = 0;
	unsigned int queue_len;

	if (queue == NULL) {
		wifi_audio_log_err("queue is NULL");
		return 0; /* when queue is invalid, return 0 */
	}
	spin_lock_irqsave(&(queue->lock), flags);
	queue_len = queue->len;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return queue_len;
}

void list_queue_clean(struct list_queue *queue)
{
	struct list_node *node = NULL;

	if (queue == NULL) {
		wifi_audio_log_err("queue is NULL");
		return;
	}

	while (list_queue_is_empty(queue) == 0) {
		node = list_queue_dequeue(queue);
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		free_list_node(node);
		node = NULL;
	}
	return;
}
