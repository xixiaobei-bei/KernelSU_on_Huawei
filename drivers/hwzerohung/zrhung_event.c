/*
 * zrhung_event.c
 *
 * Interfaces implementation for sending hung event from kernel
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include "huawei_platform/log/hw_log.h"
#include "chipset_common/hwzrhung/zrhung.h"
#include "zrhung_common.h"
#include "zrhung_transtation.h"

#define HWLOG_TAG zrhung
#define BUF_SIZE (sizeof(struct zrhung_write_event) + ZRHUNG_CMD_LEN_MAX + ZRHUNG_MSG_LEN_MAX + 2)

HWLOG_REGIST();
static uint8_t g_buf[BUF_SIZE];
static DEFINE_SPINLOCK(lock);

int zrhung_send_event(enum zrhung_wp_id id, const char *cmd_buf, const char *msg_buf)
{
	int ret;
	int cmd_len = 0;
	int msg_len = 0;
	char *p = NULL;
	char *out_buf = NULL;
	struct zrhung_write_event evt = {0};
	int total_len = sizeof(evt);
	unsigned long flags;

	memset(&evt, 0, sizeof(evt));
	if (zrhung_is_id_valid(id) < 0) {
		hwlog_err("Bad watchpoint id");
		return -EINVAL;
	}

	if (cmd_buf)
		cmd_len = strlen(cmd_buf);
	if (cmd_len > ZRHUNG_CMD_LEN_MAX) {
		hwlog_err("watchpoint cmd too long");
		return -EINVAL;
	}
	total_len += cmd_len + 1;

	if (msg_buf)
		msg_len = strlen(msg_buf);
	if (msg_len > ZRHUNG_MSG_LEN_MAX) {
		hwlog_err("watchpoint msg buffer too long");
		return -EINVAL;
	}
	total_len += msg_len + 1;

	spin_lock_irqsave(&lock, flags);
	out_buf = g_buf;

	/* construct the message */
	evt.magic = MAGIC_NUM;
	evt.len = total_len;
	evt.wp_id = id;
	evt.cmd_len = cmd_len + 1;
	evt.msg_len = msg_len + 1;

	memset(out_buf, 0, total_len);
	p = out_buf;
	memcpy(p, &evt, sizeof(evt));
	p += sizeof(evt);

	if (cmd_len > 0)
		memcpy(p, cmd_buf, cmd_len);

	p += cmd_len;
	*p = 0;
	p++;

	if (msg_buf)
		memcpy(p, msg_buf, msg_len);

	p += msg_len;
	*p = 0;

	/* send the message */
	ret = htrans_write_event_kernel(out_buf);
	spin_unlock_irqrestore(&lock, flags);

	hwlog_info("zrhung send event from kernel: wp=%d, ret=%d", evt.wp_id,
		   ret);

	return ret;
}
EXPORT_SYMBOL(zrhung_send_event);
