/*
 *
 * IPC mailbox driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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
#ifndef __IPC_MAILBOX_H__
#define __IPC_MAILBOX_H__

#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <linux/hisi/hisi_rproc.h>

#define RPUNCERTAIN     2
#define RPACCESSIBLE    1
#define RPUNACCESSIBLE  0

#define MDEV_SYNC_SENDING       (1 << 0)
#define MDEV_ASYNC_ENQUEUE      (1 << 1)
#define MDEV_DEACTIVATED        (1 << 2)

/**
 * Alloc a static buffer for the tx_task, the num can be adjust according to
 * different platform.
 * 512 allow 4 mdev-kfifo to be full fille (one mdev-kfifo can cache
 * 256 tx_task node).
 */
#define TX_TASK_DDR_NODE_NUM   585

#define TX_TASK_DDR_NODE_AVA    0xA5
#define TX_TASK_DDR_NODE_OPY    0x5A

#define MAILBOX_AUTOACK_TIMEOUT msecs_to_jiffies(300)

#define IDLE_STATUS (1 << 4)
#define SOURCE_STATUS (1 << 5)
#define DESTINATION_STATUS (1 << 6)
#define ACK_STATUS (1 << 7)

enum mbox_ack_type_t {
	MANUAL_ACK = 0,
	AUTO_ACK,
};

enum mbox_mail_type_t {
	TX_MAIL = 0,
	RX_MAIL,
	MAIL_TYPE_MAX,
};

struct hisi_mbox_task;

typedef int (*mbox_irq_handler_t)(int irq, void *p);

typedef void (*mbox_complete_t)(struct hisi_mbox_task *task);
typedef void (*rproc_complete_t)(rproc_msg_t *ack_buffer,
	rproc_msg_len_t ack_buffer_len,
	int error,
	void *data);

struct hisi_mbox_task {
	/* use static memory to cache the async tx buffer */
	mbox_msg_t tx_buffer[MBOX_CHAN_DATA_SIZE];
	/*
	 * alloc by mailbox core, shouldn't be free when a tx
	 * task complete by mailbox users
	 */
	mbox_msg_t *ack_buffer;
	mbox_msg_len_t tx_buffer_len;
	mbox_msg_len_t ack_buffer_len;
	int need_auto_ack;
	unsigned char used_flag;
	void (*ack_handle)(
		mbox_msg_t *ack_buffer, mbox_msg_len_t ack_buffer_len);
};

struct hisi_mbox_device {
	const char *name;
	struct list_head node;
	int configured;
	struct device *dev;
	void *priv;
	struct ipc_mbox_dev_ops *ops;
	int cur_task;
	int cur_irq;

	volatile unsigned int status;
	spinlock_t status_lock;
	mbox_irq_handler_t irq_handler;

	/* tx attributes */
	spinlock_t fifo_lock;
	struct kfifo fifo;
	struct mutex dev_lock;
	struct completion complete;
	spinlock_t complete_lock;
	int completed;
	struct hisi_mbox_task *tx_task;

	/* rx attributes */
	mbox_msg_t *rx_buffer;
	mbox_msg_t *ack_buffer;
	struct atomic_notifier_head notifier;
	struct tasklet_struct rx_bh;
	struct tasklet_struct tx_bh;
	struct task_struct *tx_kthread;

	wait_queue_head_t tx_wait;
};

struct ipc_mbox_dev_ops {
	/* get ready */
	int (*startup)(struct hisi_mbox_device *mdev);
	void (*shutdown)(struct hisi_mbox_device *mdev);

	int (*check)(struct hisi_mbox_device *mdev,
		enum mbox_mail_type_t mtype, int rproc_id);

	/* communication */
	mbox_msg_len_t (*recv)(struct hisi_mbox_device *mdev, mbox_msg_t **msg);
	int (*send)(struct hisi_mbox_device *mdev,
		mbox_msg_t *msg,
		mbox_msg_len_t len,
		int ack_mode);
	void (*ack)(struct hisi_mbox_device *mdev,
		mbox_msg_t *msg,
		mbox_msg_len_t len);
	void (*refresh)(struct hisi_mbox_device *mdev);
	unsigned int (*get_timeout)(struct hisi_mbox_device *mdev);
	unsigned int (*get_fifo_size)(struct hisi_mbox_device *mdev);
	unsigned int (*get_sched_priority)(struct hisi_mbox_device *mdev);
	unsigned int (*get_sched_policy)(struct hisi_mbox_device *mdev);
	int (*get_rproc_id)(struct hisi_mbox_device *mdev);
	int (*get_ipc_version)(struct hisi_mbox_device *mdev);
	int (*get_channel_size)(struct hisi_mbox_device *mdev);
	int (*get_channel_id)(struct hisi_mbox_device *mdev);
	void (*show_mdev_info)(struct hisi_mbox_device *mdev, int idx);
	int (*get_slice)(struct hisi_mbox_device *mdev, u64 *slice);
	/* irq */
	int (*request_irq)(
		struct hisi_mbox_device *mdev, irq_handler_t handler, void *p);
	void (*free_irq)(struct hisi_mbox_device *mdev, void *p);
	void (*enable_irq)(struct hisi_mbox_device *mdev);
	void (*disable_irq)(struct hisi_mbox_device *mdev);
	struct hisi_mbox_device *(*irq_to_mdev)(struct hisi_mbox_device *mdev,
		struct list_head *list, int irq);
	int (*is_stm)(struct hisi_mbox_device *mdev, unsigned int stm);
	void (*clr_ack)(struct hisi_mbox_device *mdev);
	void (*ensure_channel)(struct hisi_mbox_device *mdev);

	/* mntn */
	void (*status)(struct hisi_mbox_device *mdev);
	void (*dump_regs)(struct hisi_mbox_device *mdev);
};

struct hisi_mbox {
	int rproc_id;
	struct hisi_mbox_device *tx;
	struct hisi_mbox_device *rx;
	struct notifier_block *nb;
};

void ipc_mbox_task_free(struct hisi_mbox_task **tx_task);
struct hisi_mbox_task *ipc_mbox_task_alloc(struct hisi_mbox *mbox,
	mbox_msg_t *tx_buffer,
	mbox_msg_len_t tx_buffer_len,
	int need_auto_ack
	);

/*
 * might sleep function
 * guarantee function called not in atomic context
 */
int ipc_mbox_msg_send_sync(struct hisi_mbox *mbox,
	mbox_msg_t *tx_buffer,
	mbox_msg_len_t tx_buffer_len,
	int need_auto_ack,
	mbox_msg_t *ack_buffer,
	mbox_msg_len_t ack_buffer_len);

/*
 * atomic context function
 */
int ipc_mbox_msg_send_async(struct hisi_mbox *mbox,
	struct hisi_mbox_task *tx_task);

struct hisi_mbox *ipc_mbox_get(int rproc_id, struct notifier_block *nb);

void ipc_mbox_put(struct hisi_mbox **mbox);

void ipc_mbox_device_activate(struct hisi_mbox_device **mdevs);
void ipc_mbox_device_deactivate(struct hisi_mbox_device **mdevs);

int ipc_mbox_device_register(struct device *parent,
	struct hisi_mbox_device **mdevs);
int ipc_mbox_device_unregister(struct hisi_mbox_device **list);

void ipc_mbox_empty_task(struct hisi_mbox_device *mdev);

#endif /* __IPC_MAILBOX_H__ */
