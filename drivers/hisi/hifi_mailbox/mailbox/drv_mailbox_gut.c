/*
 * drv_mailbox_gut.c
 *
 * this file implement main process of mailbox driver
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

#include "drv_mailbox_gut.h"

#include <linux/io.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/hisi/audio_log.h>

#undef _MAILBOX_FILE_
#define _MAILBOX_FILE_ "gut"

#define LOG_TAG "mailbox_gut"

#define TIMER_REG_SIZE 4
#define MAILBOX_PROTECT_WORD_NUM 2

/* Mailbox module master control handle */
struct mb g_mailbox_handle = {MAILBOX_MAILCODE_INVALID};
static void __iomem *g_share_mem_base;
static void *g_slice_reg;

/*
 * Define the physical memory and hardware resource configuration
 * of all existing shared memory channels, globally defined
 */
static struct mb_cfg g_mailbox_cfg_tbl[MAILBOX_GLOBAL_CHANNEL_NUM + 1] = {
	/* Channel from CCPU to other cores */
	MAILBOX_CHANNEL_COMPOSE(CCPU, HIFI, MSG),

	/* Channel from ACPU to other cores */
	MAILBOX_CHANNEL_COMPOSE(ACPU, HIFI, MSG),

	/* HIFI to other core channels */
	MAILBOX_CHANNEL_COMPOSE(HIFI, CCPU, MSG),
	MAILBOX_CHANNEL_COMPOSE(HIFI, ACPU, MSG),

	/* End sign */
	{MAILBOX_MAILCODE_INVALID, 0, 0, 0},
};

/* Memory pool space for mailbox channel handle */
static struct mb_buff g_mailbox_channel_handle_pool[MAILBOX_CHANNEL_NUM];

/* Memory pool space of mailbox user callback handle */
static struct mb_cb g_mailbox_user_cb_pool[MAILBOX_USER_NUM];

static int mailbox_init_mem(const struct mb_cfg *cfg)
{
	struct mb_head *head = (struct mb_head *)(uintptr_t)cfg->head_addr;
	uintptr_t data_addr = (uintptr_t)cfg->data_addr;
	uint32_t data_size = cfg->data_size;

	if ((data_addr == 0) || (head == NULL) || (data_size == 0)) {
		return mb_logerro_p1(MB_E_GUT_INIT_CORESHARE_MEM,
			cfg->butt_id);
	}

	mailbox_memset((void *)data_addr, 0, data_size);

	/* Initialize the mailbox header protection word */
	mailbox_write_reg(data_addr, MAILBOX_PROTECT1);
	mailbox_write_reg(data_addr + MAILBOX_PROTECT_LEN, MAILBOX_PROTECT2);

	/* Initialize the protection word at the end of the mailbox */
	mailbox_write_reg((data_addr + data_size) -
		(MAILBOX_PROTECT_WORD_NUM * MAILBOX_PROTECT_LEN),
		MAILBOX_PROTECT1);
	mailbox_write_reg((data_addr + data_size) - MAILBOX_PROTECT_LEN,
		MAILBOX_PROTECT2);

	/* Initialize mailbox header */
	head->front = 0;
	head->front_slice = 0;
	head->rear = head->front;
	head->rear_slice = 0;
	head->protect_word4 = MAILBOX_PROTECT2;
	head->protect_word3 = MAILBOX_PROTECT1;
	head->protect_word2 = MAILBOX_PROTECT2;
	head->protect_word1 = MAILBOX_PROTECT1;

	return 0;
}

/* Write data to the queue and update the write pointer */
static uint32_t mailbox_queue_write(struct mb_queue *queue, const char *data,
	uint32_t size)
{
	uint32_t to_bottom;

	/*
	 * Calculate the length of the write pointer position
	 * from the tail of the ring buffer
	 */
	to_bottom = (uint32_t)((queue->base + queue->length) - queue->front);

	/*
	 * If the length of the write pointer from the tail of the ring buffer
	 * is greater than the length of the content to be written,
	 * the content is copied directly and the write pointer is updated
	 */
	if (to_bottom > size) {
		/* Write data to the write pointer */
		mailbox_memcpy((void *)(uintptr_t)queue->front, data, size);
		queue->front += size;
	} else {
		/* writing data front R length to write pointer */
		mailbox_memcpy((void *)(uintptr_t)(queue->front),
			data, to_bottom);

		/* Write pucData + R to the beginning of the ring buffer */
		mailbox_memcpy((void *)(uintptr_t)(queue->base),
			(data + to_bottom), (size - to_bottom));

		queue->front = (queue->base + size) - to_bottom;
	}

	return size;
}

/*
 * The mailbox user call in the data receiving callback function
 * to read an email that arrives first from the mailbox
 */
static uint32_t mailbox_queue_read(struct mb_queue *queue, char *buff, uint32_t size)
{
	uint32_t to_bottom;

	/*
	 * Calculate the length of the read pointer position
	 * from the tail of the ring buffer
	 */
	to_bottom = (uint32_t)((queue->base + queue->length) - queue->rear);

	/*
	 * If the length of the read pointer from the tail of the ring buffer
	 * is greater than the length of the content to be written,
	 * the content is copied directly and the read pointer is updated
	 */
	if (to_bottom > size) {
		/* Copy the data at the read pointer */
		mailbox_memcpy(buff,
			(const void *)(uintptr_t)(queue->rear), size);

		queue->rear += size;
	} else {
		/* Copy several bytes before the data at the read pointer */
		mailbox_memcpy(buff,
			(const void *)(uintptr_t)(queue->rear), to_bottom);

		/* Copy the remaining from the beginning of the ring buffer */
		mailbox_memcpy((buff + to_bottom),
			(const void *)(uintptr_t)(queue->base),
			(size - to_bottom));

		queue->rear = (queue->base + size) - to_bottom;
	}

	return size;
}

static int mailbox_check_mail(struct mb_buff *mbuff, struct mb_mail *msg_head,
	unsigned long data_addr)
{
	uint32_t time_stamp;
	uint32_t seq_num;
	int ret = 0;

	/* First check the message header protection word */
	if (msg_head->partition != MAILBOX_MSGHEAD_NUMBER)
		ret = mb_logerro_p1(MB_F_GUT_MSG_CHECK_FAIL, msg_head->mail_code);

	seq_num = mbuff->seq_num;

	/*
	 * If the SN number is not continuous
	 * (discard the two CPUs may be powered off separately)
	 */
	if (msg_head->seq_num == MAILBOX_SEQNUM_START) {
		/* Receiver receives for the first time */
		ret = mb_logerro_p1(MB_I_RECEIVE_FIRST_MAIL,
			msg_head->mail_code);
	} else if ((seq_num + 1) != msg_head->seq_num) {
		/* Not a loop back error */
		ret = mb_logerro_p1(MB_E_GUT_MAILBOX_SEQNUM_CHECK_FAIL,
			msg_head->mail_code);
	}

	mbuff->seq_num = msg_head->seq_num;

	/* Check message retention time in shared memory */
	time_stamp = (uint32_t)mailbox_get_timestamp();

	msg_head->read_slice = time_stamp;

#ifdef MAILBOX_OPEN_MNTN
	mailbox_record_transport(&(mbuff->mntn), msg_head->mail_code,
		msg_head->write_slice, msg_head->read_slice, data_addr);
#endif
	return ret;
}

static struct mb *mailbox_get_mb(void)
{
	if (g_mailbox_handle.init_flag == MAILBOX_INIT_MAGIC)
		return &g_mailbox_handle;

	AUDIO_LOGE("error: mb not init");
	return NULL;
}

static struct mb_buff *mailbox_get_channel_handle(const struct mb *mailbox,
	uint32_t mcode)
{
	const struct mb_link *link_array = NULL;
	struct mb_buff *mbuff = NULL;
	uint32_t src_id;
	uint32_t dst_id;
	uint32_t carrier_id;

	src_id = mailbox_get_src_id(mcode);
	dst_id = mailbox_get_dst_id(mcode);
	carrier_id = mailbox_get_carrier_id(mcode);

	if (src_id == mailbox->local_id) {
		if (dst_id < MAILBOX_CPUID_BUTT) {
			link_array = &mailbox->send_tbl[dst_id];
		} else {
			(void)mb_logerro_p1(MB_E_GUT_INVALID_CHANNEL_ID, mcode);
			return NULL;
		}
	} else if (dst_id == mailbox->local_id) {
		if (src_id < MAILBOX_CPUID_BUTT) {
			link_array = &mailbox->recv_tbl[src_id];
		} else {
			(void)mb_logerro_p1(MB_E_GUT_INVALID_CHANNEL_ID, mcode);
			return NULL;
		}
	} else {
		/* judged at the entrance of sending and receiving */
	}

	if ((link_array == NULL) || (link_array->carrier_butt == 0)) {
		/* There is currently no mailbox channel between the two CPUs */
		(void)mb_logerro_p1(MB_E_GUT_INVALID_CPU_LINK, mcode);
		return NULL;
	}

	/* Get channel operation handle */
	if (carrier_id < link_array->carrier_butt) {
		mbuff = &link_array->channel_buff[carrier_id];
	} else {
		(void)mb_logerro_p1(MB_E_GUT_INVALID_CARRIER_ID, mcode);
		return NULL;
	}

	if (mbuff->cfg == NULL)
		return NULL;

	if (mailbox_get_use_id(mcode) >=
		mailbox_get_use_id(mbuff->cfg->butt_id)) {
		(void)mb_logerro_p1(MB_E_GUT_INVALID_USER_ID, mcode);

		/*
		 * The PC project allows sending illegal use IDs
		 * to test the code branch of the receiving channel
		 */
		return NULL;
	}

	return mbuff;
}

/*
 * Open a mailbox channel, prepare to start writing or an email.
 * Create a mailbox operation symbol based on the mailbox channel number,
 * and fill in the mailbox operator information based on the mailbox header
 */
static int mailbox_request_channel(struct mb *mb, struct mb_buff **mb_buf,
	uint32_t mailcode)
{
	struct mb_head *head = NULL;
	struct mb_queue *queue = NULL;
	struct mb_buff *mbuff = NULL;
	int ret;

	mailbox_dpm_device_get();

	*mb_buf = NULL;
	mbuff = mailbox_get_channel_handle(mb, mailcode);
	if (mbuff == NULL) {
		ret = (int)MAILBOX_ERRO;
		goto request_err;
	}

	/* Check if the channel is initialized */
	head = (struct mb_head *)(uintptr_t)mbuff->cfg->head_addr;
	if (head == NULL) {
		ret = (int)MAILBOX_ERRO;
		goto request_err;
	}

	if (((head->protect_word1 != MAILBOX_PROTECT1)) ||
		(head->protect_word2 != MAILBOX_PROTECT2) ||
		(head->protect_word3 != MAILBOX_PROTECT1) ||
		(head->protect_word4 != MAILBOX_PROTECT2)) {
		/*
		 * Protection word is incorrect and re-init the mailbox header,
		 * do not return to affect the communication process
		 */
		mailbox_init_mem(mbuff->cfg);

		mb_logerro_p1(MAILBOX_NOT_READY, mailcode);
	}

	if (mailbox_get_src_id(mailcode) == mb->local_id) {
		/* Disallow sending emails during interruption */
		if (mailbox_int_context() == MAILBOX_TRUE) {
			/*
			 * (IFC for low power consumption) may need to support
			 * sending emails during the interrupt,
			 * here you need to lock the interrupt
			 */
			ret = mb_logerro_p1(
				MB_E_GUT_SEND_MAIL_IN_INT_CONTEXT, mailcode);
			goto request_err;
		} else {
			mailbox_mutex_lock(&mbuff->mutex);
		}
	}

	mbuff->mailcode = mailcode;
	/* Need to fill the queue operator based on the mailbox header info */
	queue = &mbuff->mail_queue;
	queue->front = queue->base + head->front * sizeof(uint32_t);
	queue->rear = queue->base + head->rear * sizeof(uint32_t);
	mbuff->mb = mb;
	*mb_buf = mbuff;

	return 0;
request_err:
	AUDIO_LOGE("err");
	mailbox_dpm_device_put();

	return ret;
}

static void mailbox_release_channel(const struct mb *mb, struct mb_buff *mbuff)
{
	uint32_t channel_id = mbuff->channel_id;

	/* Need to distinguish between reading and writing */
	if (mb->local_id == mailbox_get_src_id(channel_id)) {
		if (mailbox_int_context() != MAILBOX_TRUE)
			mailbox_mutex_unlock(&mbuff->mutex);
	}

	mailbox_dpm_device_put();
}

static void save_temp_state(struct mb_queue *tmp, const struct mb_queue *usr)
{
	tmp->base = usr->base;
	tmp->length = usr->length;
	tmp->front = usr->rear;
	tmp->rear = usr->front;
}

/*
 * Read and process the text data content of a mailbox,
 * And call the mail data processing callback function
 */
static int mailbox_read_mail(struct mb_buff *mbuff)
{
	uint32_t use_id, slice, to_bottom;
	struct mb_mail mail;
	/* Function callback function queue of this mailbox channel */
	struct mb_cb *read_cb = NULL;
	struct mb_queue tmp_queue;
	struct mb_queue *usr_queue = &(mbuff->usr_queue);
	struct mb_queue *m_queue = &(mbuff->mail_queue);

	mailbox_memcpy(usr_queue, m_queue, sizeof(struct mb_queue));

	save_temp_state(&tmp_queue, usr_queue);

	/* Read the message header */
	mailbox_queue_read(usr_queue, (char *)&mail, sizeof(struct mb_mail));

	/*
	 * Check the mail header to determine if the read message is abnormal,
	 * including whether the SN number is continuous
	 * and whether the message stays for too long
	 */
	mailbox_check_mail(mbuff, &mail, m_queue->rear);

	/* Write the read time back to the mailbox queue */
	mailbox_queue_write(&tmp_queue, (char *)&mail, sizeof(struct mb_mail));
	use_id = mailbox_get_use_id(mail.mail_code);
	if (use_id >= mailbox_get_use_id(mbuff->cfg->butt_id)) {
		(void)mb_logerro_p1(MB_E_GUT_INVALID_USER_ID, mail.mail_code);
		goto EXIT;
	}

	read_cb = &mbuff->read_cb[use_id];
	mailbox_mutex_lock(&mbuff->mutex);

	mailbox_mutex_unlock(&mbuff->mutex);
	if (read_cb->func != NULL) {
		usr_queue->size = mail.msg_length;
		slice = (uint32_t)mailbox_get_timestamp();
		read_cb->func(mbuff, read_cb->handle, read_cb->data);

		/* Recording OM information */
#ifdef MAILBOX_OPEN_MNTN
		mailbox_record_receive(&mbuff->mntn, mail.mail_code, slice);
#endif
	} else {
		(void)mb_logerro_p1(MB_E_GUT_READ_CALLBACK_NOT_FIND,
			mail.mail_code);
	}

EXIT:
	/*
	 * Regardless of whether the user uses the callback to read the data
	 * and how much data is read, the read pointer of the mailbox queue
	 * needs to be shifted according to the actual size
	 * to ensure the correctness of the subsequent mail reading.
	 */
	to_bottom = (uint32_t)((m_queue->base + m_queue->length) -
		m_queue->rear);
	if (to_bottom > (mail.msg_length + sizeof(struct mb_mail))) {
		m_queue->rear += (mail.msg_length + sizeof(struct mb_mail));
	} else {
		m_queue->rear = m_queue->base + ((mail.msg_length +
			sizeof(struct mb_mail)) - to_bottom);
	}

	/* Read guarantees pointer alignment of 4 bytes */
	m_queue->rear = mailbox_align_size(m_queue->rear, MB_ALIGN);

	return (int)(mailbox_align_size(mail.msg_length, MB_ALIGN) +
		sizeof(struct mb_mail));
}

/*
 * Get the unread data length of the current mailbox.
 * If the length is greater than 0, read and process an email in sequence,
 * Until all data is read
 */
static int mailbox_receive_all_mails(struct mb_buff *mbuf)
{
	struct mb_queue *queue = NULL;
	int size_left; /* Mailbox unreceived message length */
	uint32_t mail_len;
	int ret = 0;

	queue = &(mbuf->mail_queue);

	/*
	 * Calculate the length of the data to be fetched in the mailbox,
	 * unit is byte, The length here has been aligned by 4byte when writing
	 */
	if (queue->front >= queue->rear) {
		size_left = (int)(queue->front - queue->rear);
	} else {
		size_left = (int)((queue->length) -
			((queue->rear) - (queue->front)));
	}

	/*
	 * If the length of the data to be fetched is not 0,
	 * the mailbox content is not empty
	 */
	while (size_left > 0) {
		/* Each time read an email, including the header */
		mail_len = (uint32_t)mailbox_read_mail(mbuf);
		if (mail_len == 0)
			ret = mb_logerro_p1(MB_F_GUT_READ_MAIL, mbuf->channel_id);

		/* Update the length of data to be fetched */
		size_left -= (int)(mail_len);
	}

	if (size_left < 0)
		return mb_logerro_p1(MB_F_GUT_RECEIVE_MAIL, mbuf->channel_id);

	return ret;
}

/* Write the head and tail pointer of the message queue to the mailbox head */
static void mailbox_flush_buff(struct mb_buff *mbuff)
{
	struct mb_head *head = NULL;
	struct mb_queue *queue = NULL;
	uint32_t channel_id = mbuff->channel_id;
	struct mb *mb = mbuff->mb;

	head = (struct mb_head *)(uintptr_t)mbuff->cfg->head_addr;
	queue = &mbuff->mail_queue;

	/* Need to distinguish between reading and writing */
	if (mb->local_id == mailbox_get_src_id(channel_id)) {
		/* Write close, update the mailbox write pointer */
		head->front = (uint32_t)(queue->front - queue->base) /
			sizeof(uint32_t);
		head->front_slice = (uint32_t)mailbox_get_timestamp();
	} else if (mb->local_id == mailbox_get_dst_id(channel_id)) {
		/* Read off, update the mailbox read pointer */
		head->rear = (uint32_t)(queue->rear - queue->base) /
			sizeof(uint32_t);
		head->rear_slice = (uint32_t)mailbox_get_timestamp();
	}
}

/*
 * callback function for when receives new data.
 * Differentiate according to the mailbox channel level described
 * by the incoming parameter (MailBoxID),
 * and use different scanning mailbox reading strategies:
 * High 24bit effective notification to which CPU and
 * which channel has mail arrived: scan this mailbox channel to process all mail
 */
static int mailbox_read_channel(uint32_t channel_id)
{
	struct mb_buff *mbuf = NULL; /* Mailbox operation handle */
	struct mb *mb = NULL;
	int ret;

	mb = mailbox_get_mb();
	if (mb == NULL)
		return (int)MAILBOX_ERRO;

	/* Determine whether it is the receiving channel */
	if (mb->local_id != mailbox_get_dst_id(channel_id))
		return mb_logerro_p1(MB_E_GUT_INVALID_TARGET_CPU, channel_id);

	/* Open this mailbox channel */
	ret = mailbox_request_channel(mb, &mbuf, channel_id);
	if (ret != 0)
		return ret;

	/* Receive all mail from this mailbox channel */
	ret = mailbox_receive_all_mails(mbuf);

	/* Write the info of the mailbox channel back to the mailbox header */
	mailbox_flush_buff(mbuf);

	/* After receiving, close the mailbox */
	mailbox_release_channel(mb, mbuf);

	return ret;
}

static void update_carrier_butt(struct mb_link *link, uint32_t link_id,
	uint32_t carrier_id)
{
	/* Record the largest channel connected by this CPU */
	if ((carrier_id + 1) > link[link_id].carrier_butt)
		link[link_id].carrier_butt = (carrier_id + 1);
}

/* Statistics channel information, calculate the memory space to be allocated */
static int mailbox_calculate_space(struct mb *mb, const struct mb_cfg *cfg,
	uint32_t cpu_id)
{
	struct mb_link *send_link = NULL;
	struct mb_link *recev_link = NULL;
	uint32_t src_id;
	uint32_t dst_id;
	uint32_t ca_id;
	int ret = 0;

	send_link = mb->send_tbl;
	recev_link = mb->recv_tbl;

	while (cfg->butt_id != MAILBOX_MAILCODE_INVALID) {
		src_id = mailbox_get_src_id(cfg->butt_id);
		dst_id = mailbox_get_dst_id(cfg->butt_id);
		ca_id = mailbox_get_carrier_id(cfg->butt_id);

		if (cpu_id == src_id) {
			if (dst_id < MAILBOX_CPUID_BUTT) {
				update_carrier_butt(send_link, dst_id, ca_id);
			} else {
				ret = mb_logerro_p1(MB_E_GUT_INVALID_CHANNEL_ID,
					cfg->butt_id);
			}
		}

		if (cpu_id == dst_id) {
			if (src_id < MAILBOX_CPUID_BUTT) {
				update_carrier_butt(recev_link, src_id, ca_id);
			} else {
				ret = mb_logerro_p1(MB_E_GUT_INVALID_CHANNEL_ID,
					cfg->butt_id);
			}
		}

		cfg++;
	}

	return ret;
}

static uint32_t mailbox_register_channel(struct mb_cfg *cfg,
	struct mb_buff *mbuf_cur, uint32_t direct)
{
	struct mb_queue *queue = NULL;

	/* Assign mailbox ID number to control handle */
	mbuf_cur->channel_id = mailbox_get_channel_id(cfg->butt_id);
	mbuf_cur->seq_num = MAILBOX_SEQNUM_START;
#ifdef MAILBOX_OPEN_MNTN
	mbuf_cur->mntn.peak_traffic_left = MAILBOX_QUEUE_LEFT_INVALID;
	mbuf_cur->mntn.mbuff = mbuf_cur;
#endif
	/* Configure physical resources for the mailbox */
	mbuf_cur->cfg = cfg;

	/* Initialize channel resources */
	queue = &(mbuf_cur->mail_queue);

	/*
	 * Initialize the ring queue control symbol of the mailbox channel,
	 * the following two items will not change after initialization
	 */
	queue->length = mbuf_cur->cfg->data_size -
		(MAILBOX_DATA_LEN_PROTECT_NUM * MAILBOX_PROTECT_LEN);
	queue->base = mbuf_cur->cfg->data_addr +
		(MAILBOX_DATA_BASE_PROTECT_NUM * MAILBOX_PROTECT_LEN);

	/* Register this channel to a specific platform */
	return (uint32_t)mailbox_channel_register(
		mailbox_get_channel_id(cfg->butt_id), cfg->int_src,
		mailbox_get_dst_id(cfg->butt_id), direct, &mbuf_cur->mutex);
}

static uint32_t mailbox_update_channel_pool(struct mb_link *link,
	uint32_t link_id, struct mb_buff **mbuf_prob,
	uint32_t *channel_sum)
{
	if (link[link_id].channel_buff != NULL)
		return 0;

	link[link_id].channel_buff = *mbuf_prob;
	*mbuf_prob += (link[link_id].carrier_butt);
	*channel_sum += (link[link_id].carrier_butt);

	if (*channel_sum > MAILBOX_CHANNEL_NUM) {
		return (uint32_t)mb_logerro_p1(
			MB_F_GUT_INIT_CHANNEL_POOL_TOO_SMALL, *channel_sum);
	}

	return 0;
}

static uint32_t mailbox_init_all_check(uint32_t channel_sum,
	uint32_t use_sum)
{
	uint32_t ret = 0;

	if (channel_sum != (uint32_t)MAILBOX_CHANNEL_NUM) {
		ret = (uint32_t)mb_logerro_p1(
			MB_E_GUT_INIT_CHANNEL_POOL_TOO_LARGE,
			((MAILBOX_CHANNEL_NUM << 16) | channel_sum));
	}

	if (use_sum != MAILBOX_USER_NUM)
		ret = (uint32_t)mb_logerro_p1(MB_E_GUT_INIT_USER_POOL_TOO_LARGE, use_sum);

	return ret;
}

static uint32_t mailbox_use_sum_overflow(uint32_t use_sum)
{
	if (use_sum > MAILBOX_USER_NUM) {
		return (uint32_t)mb_logerro_p1(
			MB_F_GUT_INIT_USER_POOL_TOO_SMALL, use_sum);
	}

	return 0;
}

static uint32_t mailbox_init_all_handle(struct mb *mb, struct mb_cfg *cfg,
	uint32_t cpu_id)
{
	uint32_t ret = 0;
	struct mb_link *send_link = mb->send_tbl;
	struct mb_link *recv_link = mb->recv_tbl;
	struct mb_buff *mbuf_prob = &g_mailbox_channel_handle_pool[0];
	struct mb_cb *cb_prob = &g_mailbox_user_cb_pool[0];
	/* Point to the mailbox channel being processed */
	struct mb_buff *mbuf_cur = NULL;
	uint32_t channel_sum = 0;
	uint32_t use_sum = 0;
	uint32_t src_id, dst_id, carrier_id, use_max, direct;

	/* Initialize the control handle of each mailbox */
	while (cfg->butt_id != MAILBOX_MAILCODE_INVALID) {
		direct = MB_DIRECTION_INVALID;
		src_id = mailbox_get_src_id(cfg->butt_id);
		dst_id = mailbox_get_dst_id(cfg->butt_id);
		carrier_id = mailbox_get_carrier_id(cfg->butt_id);
		use_max = mailbox_get_use_id(cfg->butt_id);

		AUDIO_LOGI("src_id: %u dst_id: %u", src_id, dst_id);

		/* Check if this CPU sends the channel */
		if (cpu_id == src_id) {
			direct = MB_DIRECTION_SEND;

			ret = mailbox_update_channel_pool(send_link, dst_id,
				&mbuf_prob, &channel_sum);
			if (ret != 0)
				return ret;

			mbuf_cur = &send_link[dst_id].channel_buff[carrier_id];
		}

		/* Check if this CPU receiving channel */
		if (cpu_id == dst_id) {
			direct = MB_DIRECTION_RECEIVE;

			ret = mailbox_update_channel_pool(recv_link, src_id,
				&mbuf_prob, &channel_sum);
			if (ret != 0)
				return ret;

			mbuf_cur = &recv_link[src_id].channel_buff[carrier_id];
			mbuf_cur->read_cb = cb_prob;
			cb_prob += use_max;
			use_sum += use_max;

			ret = mailbox_use_sum_overflow(use_sum);
			if (ret != 0)
				return ret;

			/*
			 * Registered mailbox thread callback interface,
			 * used to process shared memory mailbox data
			 */
			ret = mailbox_process_register(
				mailbox_get_channel_id(cfg->butt_id),
				mailbox_read_channel, mbuf_cur);

			/* Initialize mailbox shared memory, set flag bit */
			ret |= mailbox_init_mem(cfg);
		}

		/*
		 * Send channel: mutually exclusive when sending,
		 * Receive channel: Mutual exclusion when callback registering
		 */
		if ((direct != MB_DIRECTION_INVALID) && (mbuf_cur != NULL))
			ret = mailbox_register_channel(cfg, mbuf_cur, direct);

		cfg++;
	}

	ret |= mailbox_init_all_check(channel_sum, use_sum);

	return ret;
}

static void mailbox_destory_box(void)
{
	mailbox_local_work_free();
}

static uint32_t mailbox_create_box(struct mb *mb, struct mb_cfg *cfg,
	uint32_t cpu_id)
{
	mb->local_id = cpu_id;

	AUDIO_LOGI("cpu_id: %u", cpu_id);

	/*
	 * The first cycle checks the validity of the channel number,
	 * and counts the space of the channel array between each core
	 */
	if (mailbox_calculate_space(mb, cfg, cpu_id) != 0)
		return (uint32_t)mb_logerro_p0(MB_E_GUT_CALCULATE_SPACE);

	/* The second loop allocates space for each channel handle */
	return mailbox_init_all_handle(mb, cfg, cpu_id);
}

static void *mem_remap_type(unsigned long phys_addr, size_t size,
	pgprot_t pgprot)
{
	int i;
	u8 *vaddr = NULL;
	int npages;
	unsigned long offset = phys_addr & (PAGE_SIZE - 1);
	struct page **pages = NULL;

	npages = PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT;
	pages = vmalloc(sizeof(char *) * npages);
	if (pages == NULL)
		return NULL;

	pages[0] = phys_to_page(phys_addr);
	for (i = 0; i < npages - 1; i++)
		pages[i + 1] = pages[i] + 1;

	vaddr = (u8 *)vmap(pages, npages, VM_MAP, pgprot);
	if (vaddr == NULL)
		AUDIO_LOGE("vmap return NULL");
	else
		vaddr += offset;

	vfree(pages);

	AUDIO_LOGD("phys_addr: 0x%pK size: 0x%08lx",
		(void *)(uintptr_t)phys_addr, (unsigned long)size);
	AUDIO_LOGD("npages: %d vaddr: %pK offset: 0x%08lx",
		npages, vaddr, offset);

	return (void *)vaddr;
}

static int mailbox_check_memory_config(void)
{
#ifdef MAILBOX_OPEN_MNTN
	uintptr_t act_head_buttom = MAILBOX_MEM_BASEADDR + MAILBOX_MEM_HEAD_LEN;
	uintptr_t head_buttom = MAILBOX_HEAD_BOTTOM_ADDR;
	uintptr_t act_data_buttom = MAILBOX_MEM_BASEADDR + MAILBOX_MEM_LENGTH;
	uintptr_t data_buttom = MAILBOX_MEMORY_BOTTOM_ADDR;

	if (act_head_buttom < head_buttom || act_data_buttom < data_buttom) {
		AUDIO_LOGE("mailbox address overflow:");
		AUDIO_LOGE("headbuttom valid: 0x%pK, cfg: 0x%pK",
			(void *)act_head_buttom, (void *)head_buttom);
		AUDIO_LOGE("databuttom valid: 0x%pK, cfg: 0x%pK",
			(void *)act_data_buttom, (void *)data_buttom);

		return mb_logerro_p0(MB_F_GUT_MEMORY_CONFIG);
	}
#endif
	return 0;
}

int mailbox_register_cb(uint32_t mailcode,
	void (*cb)(void *mbuf, void *handle, void *data), void *usr_handle,
	void *usr_data)
{
	struct mb_cb *read_cb = NULL;
	struct mb_buff *mbuf = NULL;
	struct mb *mb = NULL;
	uint32_t dst_id;

	/* possible to register during the initialization process */
	mb = &g_mailbox_handle;

	/* Register callback for a mail app in a mailbox channel of a CPU */
	dst_id = mailbox_get_dst_id(mailcode);

	AUDIO_LOGI("local_id = %u dst_id = %u", mb->local_id, dst_id);

	if (mb->local_id != dst_id)
		return mb_logerro_p1(MB_E_GUT_INVALID_TARGET_CPU, mailcode);

	mbuf = mailbox_get_channel_handle(mb, mailcode);
	if (mbuf == NULL)
		return mb_logerro_p1(MB_E_GUT_INVALID_CHANNEL_ID, mailcode);

	read_cb = &mbuf->read_cb[mailbox_get_use_id(mailcode)];

	mailbox_mutex_lock(&mbuf->mutex);
	read_cb->handle = usr_handle;
	read_cb->data = usr_data;
	read_cb->func = cb;
	mailbox_mutex_unlock(&mbuf->mutex);

	return 0;
}

/* Initialize the ring fifo descriptor left to user space */
static int mailbox_init_usr_queue(struct mb_buff *mb_buf)
{
	uint32_t hsize = sizeof(struct mb_mail);
	struct mb_queue *uq = &mb_buf->usr_queue;

	/* skip the header part of the mailbox message */
	mailbox_memcpy(uq, &mb_buf->mail_queue, sizeof(struct mb_queue));

	if (hsize > mailbox_queue_left(uq->rear, uq->front, uq->length))
		return mb_logerro_p1(MAILBOX_FULL, mb_buf->mailcode);

	if (hsize + uq->front >= uq->base + uq->length)
		uq->front = uq->front + hsize - uq->length;
	else
		uq->front = uq->front + hsize;

	return 0;
}

/* Get a mailbox physical channel buff */
int mailbox_request_buff(uint32_t mailcode, void *mb_buf)
{
	struct mb *mailbox = NULL;
	int ret;
	struct mb_buff **mbuf = (struct mb_buff **)mb_buf;

	mailbox = mailbox_get_mb();
	*mbuf = NULL;
	if (mailbox == NULL)
		return mb_logerro_p1(MB_E_GUT_INPUT_PARAMETER, mailcode);

	/* Check if it is a sending channel */
	if (mailbox->local_id != mailbox_get_src_id(mailcode))
		return mb_logerro_p1(MB_E_GUT_INVALID_SRC_CPU, mailcode);

	ret = mailbox_request_channel(mailbox, mbuf, mailcode);
	if (ret != 0)
		return ret;

	(*mbuf)->mb = mailbox;
	ret = mailbox_init_usr_queue(*mbuf);
	if (ret == 0)
		return 0;

	mailbox_release_channel(mailbox, *mbuf);
	*mbuf = NULL;

	return ret;
}

/* Fill user data into mailbox physical channel buff */
uint32_t mailbox_write_buff(struct mb_queue *queue, const char *data,
	uint32_t size)
{
	unsigned long rear = queue->rear;
	unsigned long front = queue->front;
	uint32_t length = queue->length;
	int mail_size = sizeof(struct mb_mail);

	if ((size <= mailbox_queue_left(rear, front, length)) &&
		(size + mail_size <= mailbox_queue_left(rear, front, length))) {
		return mailbox_queue_write(queue, data, size);
	}

	return 0;
}

/* Read mailbox physical channel data */
uint32_t mailbox_read_buff(struct mb_queue *queue, char *data, uint32_t size)
{
	return mailbox_queue_read(queue, (char *)data, size);
}

int mailbox_sealup_buff(struct mb_buff *mb_buf, uint32_t usr_size)
{
	struct mb_mail mail = {0};
	struct mb_queue *queue = NULL;
	uint32_t ts;
	unsigned long mail_addr;

	queue = &mb_buf->mail_queue;

	/* Determine if the maximum size of a single transmission is exceeded */
	if (usr_size > (mb_buf->cfg->single_max - sizeof(struct mb_mail))) {
		return mb_logerro_p2(MB_E_GUT_WRITE_EXCEED_MAX_SIZE,
			usr_size, mb_buf->mailcode);
	}

	ts = (uint32_t)mailbox_get_timestamp();
	/* Fill the message header */
	mail.partition = MAILBOX_MSGHEAD_NUMBER;
	mail.write_slice = ts;
	mail.read_slice = 0;
	mail.priority = 0; /* deprecated */
	mail.seq_num = mb_buf->seq_num;
	mail.msg_length = usr_size;
	mail.mail_code = mb_buf->mailcode;

	if (mb_buf->seq_num == MAILBOX_SEQNUM_START)
		(void)mb_logerro_p1(MB_I_SEND_FIRST_MAIL, mb_buf->mailcode);

	mb_buf->seq_num++;

	/*
	 * Write a message header to the mailbox queue
	 * and update the temporary structure of the queue status record
	 */
	mail_addr = queue->front;
	mailbox_queue_write(queue, (char *)(&mail), sizeof(struct mb_mail));

	/* Update mailbox ring buffer information */
	queue->front = mailbox_align_size(mb_buf->usr_queue.front, MB_ALIGN);

	/*
	 * Write the information of the mailbox channel operator
	 * back to the mailbox header.
	 */
	mailbox_flush_buff(mb_buf);

#ifdef MAILBOX_OPEN_MNTN
	mailbox_record_send(&(mb_buf->mntn), mb_buf->mailcode, ts, mail_addr);
#endif

	/* prepare to write the next message */
	return mailbox_init_usr_queue(mb_buf);
}

int mailbox_send_buff(const struct mb_buff *mbuf)
{
	return mailbox_delivery(mbuf->channel_id);
}

void mailbox_release_buff(struct mb_buff *mbuf)
{
	mailbox_release_channel(mbuf->mb, mbuf);
}

static void mailbox_update_cfg(void)
{
	int i;
	struct mb_head *head = NULL;
	struct mb_cfg *tbl = g_mailbox_cfg_tbl;
	unsigned long offset;

	offset = (uintptr_t)g_share_mem_base - MAILBOX_MEM_BASEADDR;

	for (i = 0; i < MAILBOX_GLOBAL_CHANNEL_NUM; i++) {
		tbl[i].head_addr = tbl[i].head_addr + offset;
		tbl[i].data_addr = tbl[i].data_addr + offset;

		AUDIO_LOGI("i: %d, head addr: 0x%pK, data addr: 0x%pK", i,
			(void *)(uintptr_t)(tbl[i].head_addr),
			(void *)(uintptr_t)(tbl[i].data_addr));

		/* Initialize mailbox header */
		head = (struct mb_head *)(uintptr_t)tbl[i].head_addr;
		head->front = 0;
		head->rear = 0;
		head->front_slice = 0;
		head->rear_slice = 0;
	}
}

void *get_slice_reg(void)
{
	return g_slice_reg;
}

void mailbox_destory(void)
{
	if (g_slice_reg) {
		iounmap(g_slice_reg);
		g_slice_reg = NULL;
	}

	mailbox_destory_platform();

	mailbox_destory_box();

	if (g_share_mem_base) {
		vunmap(g_share_mem_base);
		g_share_mem_base = NULL;
	}
}

int mailbox_init(void)
{
	int ret;
	struct mb_cfg *tbl = g_mailbox_cfg_tbl;
	struct mb *handle = &g_mailbox_handle;

	if (handle->init_flag == MAILBOX_INIT_MAGIC)
		return mb_logerro_p1(MB_E_GUT_ALREADY_INIT, handle->init_flag);

	AUDIO_LOGI("baseaddr: 0x%pK", (void *)(uintptr_t)MAILBOX_MEM_BASEADDR);

	g_share_mem_base = mem_remap_type(MAILBOX_MEM_BASEADDR,
		MAILBOX_MEM_LENGTH, pgprot_writecombine(PAGE_KERNEL));
	if (g_share_mem_base == NULL) {
		AUDIO_LOGE("mmap mailbox mem error");
		return mb_logerro_p0(MB_F_GUT_MEMORY_CONFIG);
	}

	mailbox_update_cfg();

	/* This line needs to be deleted in single-core restart feature */
	mailbox_memset(handle, 0, sizeof(*handle));

	ret = mailbox_check_memory_config();
	if (ret != 0)
		goto error_remap;

	if (mailbox_create_box(handle, &tbl[0], MAILBOX_LOCAL_CPUID) != 0) {
		ret = mb_logerro_p0(MB_E_GUT_CREATE_BOX);
		goto error_create_box;
	}

	/* Initialize platform differences */
	if (mailbox_init_platform() != 0) {
		ret = mb_logerro_p0(MB_E_GUT_INIT_PLATFORM);
		goto error_create_box;
	}

	g_slice_reg = (void *)ioremap(SYS_TIME_STAMP_REG, TIMER_REG_SIZE);
	if (g_slice_reg == NULL) {
		AUDIO_LOGE("ioremap of slice reg fail");
		ret = -1;
		goto error_init_platform;
	}

	handle->init_flag = MAILBOX_INIT_MAGIC;

	AUDIO_LOGI("mb init OK");

	return 0;
error_init_platform:
	mailbox_destory_platform();
error_create_box:
	mailbox_destory_box();
error_remap:
	vunmap(g_share_mem_base);
	g_share_mem_base = NULL;

	return ret;
}

