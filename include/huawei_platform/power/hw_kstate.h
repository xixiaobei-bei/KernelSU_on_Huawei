/*
 * hw_kstate.h
 *
 * This file use to send kernel state
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HW_KSTATE_H
#define _HW_KSTATE_H

#include <linux/types.h>
#include <linux/list.h>

#define KSTATE_NAME_LEN_MAX 20

typedef enum {
	CHANNEL_ID_NONE         = 0x0,
	CHANNEL_ID_LOCAL_SERVER = 0x1 << 0,
	CHANNEL_ID_LOCAL_CLIENT = 0x1 << 1,
	CHANNEL_ID_NETLINK      = 0x1 << 2,
	CHANNEL_ID_KCOLLECT     = 0x1 << 3,
	CHANNEL_ID_END
} channel_id;

typedef enum {
	PACKET_TAG_NONE = 0,
	PACKET_TAG_VALIDATE_CHANNEL,
	PACKET_TAG_MONITOR_CMD,
	PACKET_TAG_MONITOR_INFO,
	PACKET_TAG_KCOLLECT,
	PACKET_TAG_KPG,
	PACKET_TAG_END
} packet_tag;

/* data format between user and kernel */
struct ksmsg {
	packet_tag tag;
	channel_id src;
	channel_id dst;
	unsigned int length;
	char buffer[0];
};

/* used for register */
struct kstate_opt {
	struct list_head list;
	channel_id dst; /* used to mark client */
	packet_tag tag;
	char name[KSTATE_NAME_LEN_MAX];
	int (*hook)(channel_id src, packet_tag tag, const char *data, size_t len);
};

int kstate(channel_id channel, packet_tag tag, const char *data, size_t len);
int kstate_register_hook(struct kstate_opt *opt);
void kstate_unregister_hook(struct kstate_opt *opt);

#endif
