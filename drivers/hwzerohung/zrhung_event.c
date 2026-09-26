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
#include <log/hiview_hievent.h>
#include <asm/current.h>
#include "huawei_platform/log/hw_log.h"
#include "chipset_common/hwzrhung/zrhung.h"
#include "zrhung_common.h"
#include "zrhung_transtation.h"

#define HWLOG_TAG zrhung
#define WP_TO_RAW_ID_BASE 10000

HWLOG_REGIST();

int zrhung_send_hievent(enum zrhung_wp_id id, const char *cmd_buf, const char *msg_buf)
{
	int ret;
	int raw_id = id + WP_TO_RAW_ID_BASE;

	ret = hiview_send_raw_event(raw_id, "%d%d%s%s%s%s", current->pid,
				    current->tgid, current->comm,
				    current->comm, msg_buf, cmd_buf);
	if (ret < 0)
		hwlog_err("failed to send hievent");
	return ret;
}

int zrhung_send_event(enum zrhung_wp_id id, const char *cmd_buf, const char *msg_buf)
{
	int ret;
	int cmd_len = 0;
	int msg_len = 0;
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

	if (msg_buf)
		msg_len = strlen(msg_buf);
	if (msg_len > ZRHUNG_MSG_LEN_MAX) {
		hwlog_err("watchpoint msg buffer too long");
		return -EINVAL;
	}

	ret = zrhung_send_hievent(id, cmd_buf, msg_buf);
	hwlog_info("zrhung send event from kernel: wp=%d, ret=%d", id, ret);
	return ret;
}
EXPORT_SYMBOL(zrhung_send_event);
