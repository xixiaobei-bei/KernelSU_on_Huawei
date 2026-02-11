/*
 * drivers/inputhub/contexthub_ext_log.h
 *
 * sensors sysfs header
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

#ifndef __IOMCU_EXT_LOG_H
#define __IOMCU_EXT_LOG_H

struct inputhub_ext_notifier_node {
	struct list_head entry;
	int tag;
	int (*notify)(const pkt_header_t *data);
};

struct inputhub_ext_log_notifier {
	struct list_head head;
	spinlock_t lock;
};

typedef struct {
	pkt_header_t hd;
	uint8_t tag;
	uint8_t data[];
} ext_logger_req_t;

typedef struct {
	uint8_t type;
	uint16_t len;
	uint8_t data[];
} pedo_ext_logger_req_t;

int inputhub_ext_log_init(void);
int is_inputhub_ext_log_notify(const pkt_header_t *head);
int inputhub_ext_log_register_handler(int tag,
		int (*notify)(const pkt_header_t *head));

#endif
