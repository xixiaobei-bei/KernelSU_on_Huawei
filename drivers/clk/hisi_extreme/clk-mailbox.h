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
#ifndef __CLK_MAILBOX_H_
#define __CLK_MAILBOX_H_

#include <linux/completion.h>
#include <linux/hisi/hisi_rproc.h>

#define LPM3_CMD_LEN	2

#define NO_AUTO_ACK	0

struct clk_mbox_context {
	struct completion complete;
	unsigned long freq;
};

int clkmbox_is_ready(void);
int clkmbox_send_msg(mbox_msg_t *msg, int length);
int clkmbox_send_msg_sync(mbox_msg_t *msg, int length);
#endif
