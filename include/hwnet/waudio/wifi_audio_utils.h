/*
 * wifi_audio_utils.h
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

#ifndef _WIFI_AUDIO_UTILS_H_
#define _WIFI_AUDIO_UTILS_H_

#include <linux/kernel.h>
#include <linux/spinlock.h>

/* return code */
#define ERROR (-1)
#define SUCCESS 0

#define LIST_NODE_DATA_LEN_MAX 16384

int get_log_flag(void);
#define wifi_audio_log_print(level, fmt, ...) \
	do { \
		if (get_log_flag() < level) { \
			break; \
		} \
		pr_err(fmt, ##__VA_ARGS__); \
	}while(0)

#define wifi_audio_log_info(fmt, ...) \
	wifi_audio_log_print(WIFI_AUDIO_LOG_LOW, "wifi_audio<INFO>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define wifi_audio_log_warn(fmt, ...) \
	wifi_audio_log_print(WIFI_AUDIO_LOG_MID, "wifi_audio<WARNNG>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define wifi_audio_log_err(fmt, ...) \
	wifi_audio_log_print(WIFI_AUDIO_LOG_HI, "wifi_audio<ERROR>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum wifi_audio_log_level {
	WIFI_AUDIO_LOG_NULL = 0,
	WIFI_AUDIO_LOG_LOW = 1,
	WIFI_AUDIO_LOG_MID = 2,
	WIFI_AUDIO_LOG_HI = 4,
};

struct list_node {
	struct list_head list;
	unsigned int len;
	char data[0];
};

struct list_queue {
	struct list_head queue;
	unsigned int len;
	spinlock_t lock;
};

struct list_node *alloc_list_node(const void *data, unsigned int len);
void free_list_node(struct list_node *node);
void list_queue_init(struct list_queue *queue);
void list_queue_enqueue(struct list_queue *queue, struct list_node *node);
struct list_node *list_queue_dequeue(struct list_queue *queue);
int list_queue_is_empty(struct list_queue *queue);
unsigned int list_queue_len(struct list_queue *queue);
void list_queue_clean(struct list_queue *queue);
#endif /* _WIFI_AUDIO_UTILS_H_ */
