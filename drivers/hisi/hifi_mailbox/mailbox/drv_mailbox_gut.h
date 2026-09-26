/*
 * drv_mailbox_gut.h
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

#ifndef __DRV_MAILBOX_GUT_H__
#define __DRV_MAILBOX_GUT_H__

#include "drv_mailbox_cfg.h"
#include "drv_mailbox_platform.h"
#include "drv_mailbox_debug.h"

#define MAILBOX_GLOBAL_CHANNEL_NUM 4

/* Conversion macro composed and obtained by mailbox ID number */
/* Single element number (8bit) mask bit of mailbox number */
#define MAILBOX_SINGLE_ID_MASK 0xFF
/* Get mailbox ID from mail ID */
#define mailbox_get_channel_id(id) ((id) & (~MAILBOX_SINGLE_ID_MASK))

/* Get the source CPUID from the mail ID */
#define mailbox_get_src_id(id) \
	(unsigned char)(((id) >> MAILBOX_ID_SRC_CPU_OFFSET) & \
	MAILBOX_SINGLE_ID_MASK)

/* Get the destination CPUID from the mail ID */
#define mailbox_get_dst_id(id) \
	(unsigned char)(((id) >> MAILBOX_ID_DST_CPU_OFFSET) & \
	MAILBOX_SINGLE_ID_MASK)

/* Get the physical carrier ID of the channel from the mail ID */
#define mailbox_get_carrier_id(id) \
	(unsigned char)(((id) >> MAILBOX_ID_CHANNEL_OFFSET) & \
	MAILBOX_SINGLE_ID_MASK)

/* Get mail application ID from mail ID */
#define mailbox_get_use_id(id) (unsigned char)((id) & MAILBOX_SINGLE_ID_MASK)

/* Magic number indicating that the mailbox has been initialized */
#define MAILBOX_INIT_MAGIC 0x87654312

/* a is aligned by p bytes */
#define MB_ALIGN sizeof(uint32_t)
#define mailbox_align_size(a, p) (((a) + ((p)-1)) & ~((p)-1))

/* Write physical address */
#define mailbox_write_reg(addr, value) \
	(*((volatile uint32_t*)(addr)) = (value))

/* Define the number of CPUs */
#define MAILBOX_CPU_NUM (MAILBOX_CPUID_BUTT)

/* Define the number of channels */
#define MAILBOX_CHANNEL_BUTT(src, dst) \
	(MAILBOX_CHANNEL_##src##2##dst##_BUTT & 0xff)

/* Define the upper limit of user id */
#define MAILBOX_USER_BUTT(src, dst, channel) \
	((uint32_t)MAILBOX_MAILCODE_ITEM_END(src, dst, channel) & 0xff)

/* Define the elements in the data structure struct mb_cfg */
#define MAILBOX_CHANNEL_COMPOSE(src, dst, ch)                                 \
/* channel */                                                                 \
{                                                                             \
	(uint32_t)MAILBOX_MAILCODE_ITEM_END(src, dst, ch), /* DataSize */     \
	(uint32_t)MAILBOX_QUEUE_SIZE(src, dst, ch), /* SingleMax */           \
	(uint32_t)MAILBOX_MAILSIZE_MAX(src, dst, ch), /* HeadAddr */          \
	(unsigned long)MAILBOX_HEAD_ADDR(src, dst, ch), /* DataAddr */        \
	(unsigned long)MAILBOX_QUEUE_ADDR(src, dst, ch), /* IPC INT NUMBER */ \
	(uint32_t)MAILBOX_IPC_INT_NUM(src, dst, ch)                           \
}

#define MAILBOX_USER_QUEUE(mbuf) (&((struct mb_buff *)(mbuf))->usr_queue)

/*
 * Calculate the remaining space of the mailbox.
 * In order to prevent the head and tail of the queue from overlapping,
 * 4 bytes of unused space are removed.
 */
#define mailbox_queue_left(rear, front, length)                       \
	(((rear) > (front)) ? (((rear) - (front)) - sizeof(uint32_t)) \
		: (((length) - ((front) - (rear))) - sizeof(uint32_t)))

struct mb_cb {
	void (*func)(void *mbuf, void *handle, void *data);
	void *handle;
	void *data;
};

/* Mailbox hardware resource configuration of a physical mailbox channel */
struct mb_cfg {
	/*
	 * The end number of the mailbox ID,
	 * including the maximum ID information of the largest current mailbox
	 */
	uint32_t butt_id;
	/* Mailbox data size */
	uint32_t data_size;
	/* Maximum size of a single mail */
	uint32_t single_max;
	/* Mail header memory address */
	unsigned long head_addr;
	/* Mail data memory address */
	unsigned long data_addr;
	/* Interrupt number corresponding to the physical channel ID */
	uint32_t int_src;
};

/* Resource collection of mailbox channel */
struct mb_buff {
	/*
	 * The mailbox channel ID is determined by
	 * the link between the two cores of the mailbox,
	 * the link direction,
	 * and the physical channel number
	 */
	uint32_t channel_id;
	/*
	 * The mail ID is determined by link between two cores of the mailbox,
	 * the link direction, the physical channel number, and the app number
	 */
	uint32_t mailcode;
	uint32_t seq_num; /* Mailbox message serial number (SN number) */
	/* Prevent multiple threads from sending data to the same mailbox */
	struct mb_mutex mutex;
	struct mb *mb;
	/* Function callback function queue of this mailbox channel */
	struct mb_cb *read_cb;
	/* Shared memory channel information for this mailbox */
	struct mb_cfg *cfg;
#ifdef MAILBOX_OPEN_MNTN
	/* OM information for this mailbox */
	struct mb_mntn mntn;
#endif
	/* Circular queue operation handle for data processing */
	struct mb_queue mail_queue;
	/* The data used by the user, ie, mail_queue with header size removed */
	struct mb_queue usr_queue;
};

/* Single mailbox channel configuration table entry */
struct mb_link {
	/* How many mailbox channels are currently connected to the CPU */
	uint32_t carrier_butt;
	/* The collection of control resources for this mailbox */
	struct mb_buff *channel_buff;
};

/*
 * The overall control handle data structure of the mailbox module,
 * the collection of control handles of the mailbox group
 */
struct mb {
	uint32_t local_id;  /* Current CPU ID */
	uint32_t init_flag; /* Initialization flag */
	uint32_t log_prob;  /* Point to the next error record */
	/* Record the last few error messages */
	struct mb_log log_array[MAILBOX_ERRO_ARRAY_NUM];
	/* Mailbox send channel control resource list */
	struct mb_link send_tbl[MAILBOX_CPUID_BUTT];
	/* Mailbox receiving channel control resource list */
	struct mb_link recv_tbl[MAILBOX_CPUID_BUTT];
};

/* Mailbox module master control handle */
extern struct mb g_mailbox_handle;

int mailbox_register_cb(uint32_t mail_code,
	void (*cb)(void *mbuf, void *handle, void *data), void *usr_handle,
	void *usr_data);

int mailbox_request_buff(uint32_t mailcode, void *mbuf);

int mailbox_sealup_buff(struct mb_buff *mb_buf, uint32_t usr_size);

int mailbox_send_buff(const struct mb_buff *mb_buf);

void mailbox_release_buff(struct mb_buff *mb_buf);

uint32_t mailbox_write_buff(struct mb_queue *queue, const char *data, uint32_t size);

uint32_t mailbox_read_buff(struct mb_queue *queue, char *data, uint32_t size);

void *get_slice_reg(void);

#endif /* __DRV_MAILBOX_GUT_H__ */
