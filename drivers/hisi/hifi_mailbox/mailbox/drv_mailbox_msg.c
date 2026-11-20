/*
 * drv_mailbox_msg.c
 *
 * this file implement mailbox interfaces
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

#include "drv_mailbox_msg.h"

#include "drv_mailbox_debug.h"
#include "drv_mailbox_gut.h"

#undef _MAILBOX_FILE_
#define _MAILBOX_FILE_ "msg"

void mailbox_msg_receiver(void *mb_buf, void *handle, void *data)
{
	/* Temporary handle of mailbox buffer, used to pass callback to user */
	struct mb_queue *queue = NULL;
	struct mb_buff *mbuf = mb_buf;
	mb_msg_cb func = (mb_msg_cb)handle;

	if(mbuf == NULL)
		return;

	queue = &mbuf->usr_queue;
	if (func)
		func(data, queue, queue->size);
	else
		(void)mb_logerro_p1(MB_E_GUT_READ_CALLBACK_NOT_FIND,
			mbuf->mailcode);
}

int mailbox_reg_msg_cb(uint32_t mailcode, mb_msg_cb func, void *data)
{
	return mailbox_register_cb(mailcode, mailbox_msg_receiver, func, data);
}

uint32_t mailbox_try_send_msg(uint32_t mailcode, const void *pdata,
	uint32_t length)
{
	struct mb_buff *mb_buf = NULL;
	struct mb_queue *queue = NULL;
	int ret;

	if ((pdata == NULL) || (length == 0)) {
		ret = mb_logerro_p1(MAILBOX_ERRO, mailcode);
		goto exit_out;
	}

	/* Get mailbox buffer */
	ret = mailbox_request_buff(mailcode, (void *)&mb_buf);
	if (ret != 0)
		goto exit_out;

	/* Fill user data */
	queue = &mb_buf->usr_queue;
	if (length != mailbox_write_buff(queue, pdata, length)) {
		ret = mb_logerro_p1(MAILBOX_FULL, mailcode);
		goto exit_out;
	}

	/* Package letter */
	ret = mailbox_sealup_buff(mb_buf, length);
	if (ret == 0)
		ret = mailbox_send_buff(mb_buf);

exit_out:
	if (mb_buf != NULL)
		mailbox_release_buff(mb_buf);

	return (uint32_t)ret;
}

uint32_t mailbox_read_msg_data(struct mb_queue *mail_handle, char *buff,
	uint32_t *size)
{
	struct mb_queue *q = mail_handle;

	if ((q == NULL) || (buff == NULL) || (size == NULL)) {
		return (uint32_t)mb_logerro_p1(
			MB_E_GUT_INPUT_PARAMETER, 0);
	}

	if (q->size > *size) {
		return (uint32_t)mb_logerro_p1(
			MB_E_GUT_USER_BUFFER_SIZE_TOO_SMALL, *size);
	}

	/* Check the validity of the queue handle returned by the user */
	if ((q->length == 0) ||
		((uint32_t)(q->front - q->base) > q->length) ||
		((uint32_t)(q->rear - q->base) > q->length)) {
		return (uint32_t)mb_logerro_p1(
			MB_F_GUT_INVALID_USER_MAIL_HANDLE, q);
	}

	*size = mailbox_read_buff(q, buff, q->size);

	return 0;
}

uint32_t mailbox_send_msg(uint32_t mailcode, const void *data,
	uint32_t length)
{
	int ret;
	int try_go_on;
	int try_times = 0;

#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	if (!is_dsp_img_loaded())
		return MAILBOX_HIFI_NOT_LOAD;
#endif

	ret = (int)mailbox_try_send_msg(mailcode, data, length);

	if (mailbox_int_context() == MAILBOX_FALSE) {
		while (ret == MAILBOX_FULL) {
			mailbox_delivery(mailbox_get_channel_id(mailcode));
			try_go_on = mailbox_scene_delay(
				MAILBOX_DELAY_SCENE_MSG_FULL, &try_times);
			if (try_go_on != MAILBOX_TRUE)
				break;

			ret = (int)mailbox_try_send_msg(mailcode, data, length);
		}
	}

	if (ret != 0 && ret != MAILBOX_FULL)
		return (uint32_t)MAILBOX_ERRO;

	return (uint32_t)ret;
}
