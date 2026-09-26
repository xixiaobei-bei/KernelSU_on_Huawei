/*
 * drv_mailbox_debug.c
 *
 * this file implement mailbox debug utils
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

#include "drv_mailbox_debug.h"

#include <linux/hisi/audio_log.h>

#include "drv_mailbox_cfg.h"
#include "drv_mailbox_gut.h"

#undef _MAILBOX_FILE_
#define _MAILBOX_FILE_ "dbg"

#define LOG_TAG "mailbox_debug"

#define QUEUE_LEN_SHIFT 3u

#if (MAILBOX_LOG_LEVEL != MAILBOX_LOG_NONE)
int mailbox_log_erro(uint32_t err_no, unsigned long param1,
	unsigned long param2, uint32_t line_no, const char *file_name)
{
	struct mb *mb = &g_mailbox_handle;
	bool print = false;
	char *erro_str = NULL;

	mb->log_array[mb->log_prob].erro_num = err_no;
	mb->log_array[mb->log_prob].line = line_no;
	mb->log_array[mb->log_prob].param1 = param1;
	mb->log_array[mb->log_prob].param2 = param2;
	mb->log_array[mb->log_prob].file = (const char *)file_name;
	mb->log_prob = ((MAILBOX_ERRO_ARRAY_NUM - 1) == mb->log_prob) ?
			(0) : (mb->log_prob + 1);

#if (MAILBOX_LOG_LEVEL >= MAILBOX_LOG_CRITICAL)
	if ((err_no > MB_F_RET_START) && (err_no < MB_F_RET_END)) {
		erro_str = "mb Critical!";
		print = true;
		mailbox_assert(err_no);
	}
#endif

#if (MAILBOX_LOG_LEVEL >= MAILBOX_LOG_ERROR)
	if ((err_no > MB_E_RET_START) && (err_no < MB_E_RET_END)) {
		erro_str = "mb error!";
		print = true;
	}
#endif

#if (MAILBOX_LOG_LEVEL >= MAILBOX_LOG_WARNING)
	if ((err_no > MB_W_RET_START) && (err_no < MB_W_RET_END)) {
		erro_str = "mb warning!";
		print = true;
	}
#endif

#if (MAILBOX_LOG_LEVEL >= MAILBOX_LOG_INFO)
	if ((err_no > MB_I_RET_START) && (err_no < MB_I_RET_END)) {
		erro_str = "mb info!";
		print = true;
	}
#endif

	if (err_no == MAILBOX_FULL) {
		AUDIO_LOGE("mb: %lu full", param1);
		print = true;
	} else if (err_no == MAILBOX_NOT_READY) {
		AUDIO_LOGE("remote mb: %lu not ready", param1);
		print = true;
	}

	if (!print)
		return (int)err_no;

	AUDIO_LOGE("%s:0x%08x, param1:%lu, param2:%lu, line:%u, file:%s",
		erro_str, err_no, param1, param2, line_no, file_name);

	return (int)err_no;
}
#endif

#ifdef MAILBOX_OPEN_MNTN
uint32_t g_mb_trans_time_limit = MAILBOX_MAIL_TRANS_TIME_LIMIT;
uint32_t g_mb_deal_time_limit = MAILBOX_MAIL_DEAL_TIME_LIMIT;
uint32_t g_mb_sche_limit = MAILBOX_MAIL_SCHE_TIME_LIMIT;

static uint32_t mailbox_get_slice_diff(uint32_t start, uint32_t end)
{
	if (end >= start)
		return end - start;
	else
		return MAILBOX_MAX_SLICE - start + end;
}

static void mailbox_statistic_slice(struct mb_slice *slice,
	uint32_t mailcode, uint32_t threslhold, uint32_t erro_code)
{
	uint32_t slice_diff;
	uint32_t slice_end = (uint32_t)mailbox_get_timestamp();
	uint32_t slice_start = slice->start;

	slice_diff = mailbox_get_slice_diff(slice_start, slice_end);

	slice_end = slice->total;
	slice->total += slice_diff;
	if (slice_end > slice->total)
		slice->overflow = MAILBOX_TRUE;

	if (slice_diff > slice->max) {
		slice->max = slice_diff;
		slice->code = mailcode;
	}

	if (slice_diff >= threslhold)
		mb_logerro_p2(erro_code, slice_diff, mailcode);
}

void mailbox_record_sche_send(void *priv)
{
	struct mb_buff *mbuf = priv;

	mbuf->mntn.sche.start = (uint32_t)mailbox_get_timestamp();
}

void mailbox_record_sche_recv(void *priv)
{
	struct mb_buff *mbuf = priv;

	mailbox_statistic_slice(&mbuf->mntn.sche, mbuf->channel_id,
		g_mb_sche_limit, MB_W_SCHE_TIME_OUT);
}


void mailbox_record_send(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t time_stamp, unsigned long mail_addr)
{
	struct mb_queue *m_queue = &mntn->mbuff->mail_queue;
	uint32_t size_left = (uint32_t)mailbox_queue_left(m_queue->rear,
		m_queue->front, m_queue->length);
	if (size_left < mntn->peak_traffic_left)
		mntn->peak_traffic_left = size_left;

	mntn->track_array[mntn->track_prob].send_slice = time_stamp;
	mntn->track_array[mntn->track_prob].addr = mail_addr;
	mntn->track_array[mntn->track_prob].use_id =
		mailbox_get_use_id(mailcode);
	mntn->track_prob = ((MAILBOX_RECORD_USEID_NUM - 1) == mntn->track_prob) ?
		0 : (mntn->track_prob + 1);
}

void mailbox_record_transport(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t write_slice, uint32_t read_slice,
	unsigned long mail_addr)
{
	struct mb_queue *m_queue = &mntn->mbuff->mail_queue;
	uint32_t size_left = (uint32_t)mailbox_queue_left(
		m_queue->rear, m_queue->front, m_queue->length);
	if (size_left < mntn->peak_traffic_left)
		mntn->peak_traffic_left = size_left;

	if (size_left < (m_queue->length >> QUEUE_LEN_SHIFT))
		mb_logerro_p2(MB_E_GUT_MAILBOX_RECEIVE_FULL, size_left, mailcode);

	mntn->track_array[mntn->track_prob].use_id =
		(unsigned short)mailbox_get_use_id(mailcode);
	mntn->track_array[mntn->track_prob].send_slice = write_slice;
	mntn->track_array[mntn->track_prob].recv_slice = read_slice;
	mntn->track_array[mntn->track_prob].addr = mail_addr;
	mntn->track_prob = ((MAILBOX_RECORD_USEID_NUM - 1) == mntn->track_prob)
		? (0) : (mntn->track_prob + 1);

	mntn->trans.start = write_slice;
	mailbox_statistic_slice(&mntn->trans, mailcode, g_mb_trans_time_limit,
		MB_W_TRANSPORT_TIME_OUT);
}

void mailbox_record_receive(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t slice_start)
{
	mntn->deal.start = slice_start;
	mailbox_statistic_slice(&mntn->deal, mailcode, g_mb_deal_time_limit,
		MB_W_RECEIVE_TIME_OUT);
}

#else

#endif
