/*
 * drv_mailbox.c
 *
 * this file encapsulate mailbox interfaces
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

#include "drv_mailbox_cfg.h"
#include "drv_mailbox_msg.h"

/* funcions below use by others, just keep captial case */
uint32_t DRV_MAILBOX_SENDMAIL(uint32_t mail_code, const void *data,
	uint32_t length)
{
	return mailbox_send_msg(mail_code, data, length);
}

uint32_t DRV_MAILBOX_REGISTERRECVFUNC(uint32_t mail_code, mb_msg_cb func,
	void *data)
{
	return (uint32_t)mailbox_reg_msg_cb(mail_code, func, data);
}

uint32_t DRV_MAILBOX_READMAILDATA(struct mb_queue *mail_handle,
	unsigned char *data, uint32_t *size)
{
	return mailbox_read_msg_data(mail_handle, (char *)data, size);
}
