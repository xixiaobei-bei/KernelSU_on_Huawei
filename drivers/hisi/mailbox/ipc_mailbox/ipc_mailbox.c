/*
 * ipc_mailbox.c
 *
 * mailbox driver
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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/semaphore.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <pr_log.h>
#include <securec.h>
#include <uapi/linux/sched/types.h>

#include "ipc_mailbox.h"
#include "ipc_mailbox_event.h"
#include "ipc_mailbox_mntn.h"

/*lint -e578 */
/*lint -e580 */
#define PR_LOG_TAG AP_MAILBOX_TAG

#define mbox_pr_err(fmt, args...)   pr_err(fmt "\n", ##args)

#define mbox_pr_info(fmt, args...)  pr_info(fmt "\n", ##args)

#define mbox_pr_debug(fmt, args...)

#define TX_FIFO_CELL_SIZE (sizeof(struct hisi_mbox_task *))
#define MAILBOX_MAX_TX_FIFO 256
/* tx_thread warn level to bug_on when tx_thread is blocked by some reasons */
#define TX_THREAD_BUFFER_WARN_LEVEL (156 * TX_FIFO_CELL_SIZE)
#define ERROR (-1)

#define WAIT_ACK_TIMES 10

enum {
	NOCOMPLETION = 0,
	COMPLETING,
	COMPLETED
};

enum {
	TX_TASK = 0,
	RX_TASK
};

static struct class *g_mbox_class;
static spinlock_t g_task_buffer_lock;
static struct hisi_mbox_task *g_txtaskbuffer;

/* mailbox device resource pool */
static LIST_HEAD(mdevices);

static int ipc_mbx_init_tx_task_buf(void)
{
	struct hisi_mbox_task *ptask = NULL;
	int i;

	if (g_txtaskbuffer)
		return 0;

	g_txtaskbuffer = kcalloc(TX_TASK_DDR_NODE_NUM, sizeof(*ptask),
		GFP_KERNEL);
	if (!g_txtaskbuffer)
		return -ENOMEM;

	if (memset_s(g_txtaskbuffer, sizeof(*ptask) * TX_TASK_DDR_NODE_NUM,
		0, sizeof(*ptask) * TX_TASK_DDR_NODE_NUM) != EOK)
		mbox_pr_err("memset g_txtaskbuffer failed");

	ptask = g_txtaskbuffer;
	for (i = 0; i < TX_TASK_DDR_NODE_NUM; i++) {
		/*
		 * Init the tx buffer's node, set the flag to available
		 */
		ptask->used_flag = TX_TASK_DDR_NODE_AVA;
		ptask++;
	}

	return 0;
}

struct hisi_mbox_task *ipc_mbox_node_alloc(void)
{
	int index;
	struct hisi_mbox_task *ptask = g_txtaskbuffer;
	unsigned long flags = 0;

	spin_lock_irqsave(&g_task_buffer_lock, flags);
	for (index = 0; index < TX_TASK_DDR_NODE_NUM; index++) {
		if (ptask->used_flag == TX_TASK_DDR_NODE_AVA)
			break;
		ptask++;
	}

	if (likely(index != TX_TASK_DDR_NODE_NUM))
		ptask->used_flag =
			TX_TASK_DDR_NODE_OPY; /* set the node occupied  */
	else
		ptask = NULL;

	spin_unlock_irqrestore(&g_task_buffer_lock, flags);

	return ptask;
}

void ipc_mbox_task_free(struct hisi_mbox_task **tx_task)
{
	unsigned long flags = 0;

	if (!tx_task || !(*tx_task)) {
		mbox_pr_err("null pointer");
		return;
	}
	spin_lock_irqsave(&g_task_buffer_lock, flags);
	(*tx_task)->used_flag = TX_TASK_DDR_NODE_AVA;
	(*tx_task)->ack_handle = NULL;
	spin_unlock_irqrestore(&g_task_buffer_lock, flags);
}
EXPORT_SYMBOL(ipc_mbox_task_free);

struct hisi_mbox_task *ipc_mbox_task_alloc(struct hisi_mbox *mbox,
	mbox_msg_t *tx_buffer, mbox_msg_len_t tx_buffer_len, int need_auto_ack)
{
	int ret;
	struct hisi_mbox_task *tx_task = NULL;

	if (!mbox || !mbox->tx || !tx_buffer) {
		if (!mbox)
			mbox_pr_err("null pointer mbox!");
		else
			mbox_pr_err("rproc_id:%d no tx ability or no tx_buffer",
				mbox->rproc_id);
		goto out;
	}
	tx_task = ipc_mbox_node_alloc();
	if (!tx_task) {
		mbox_pr_err("tx task no mem");
		goto out;
	}
	ret = memcpy_s((void *)tx_task->tx_buffer,
			tx_buffer_len * (sizeof(*(tx_task->tx_buffer))),
			(void *)tx_buffer,
			tx_buffer_len * (sizeof(*tx_buffer)));
	if (ret != EOK) {
		ipc_mbox_task_free(&tx_task);
		tx_task = NULL;
		goto out;
	}
	tx_task->tx_buffer_len = tx_buffer_len;
	tx_task->need_auto_ack = need_auto_ack;
out:
	return tx_task;
}
EXPORT_SYMBOL(ipc_mbox_task_alloc);

static int set_status(struct hisi_mbox_device *mdev, int status)
{
	int ret = 0;
	unsigned long flags = 0;
	int old_status = mdev->status;

	spin_lock_irqsave(&mdev->status_lock, flags);
	if ((MDEV_DEACTIVATED & mdev->status)) {
		spin_unlock_irqrestore(&mdev->status_lock, flags);
		mbox_pr_info("an unexpected ipc caused by %s", mdev->name);
		ret = -ENODEV;
		goto out;
	} else if ((MDEV_DEACTIVATED & (unsigned int)status)) {
		mdev->status |= (unsigned int)status;

		while ((MDEV_SYNC_SENDING & mdev->status) ||
			(MDEV_ASYNC_ENQUEUE & mdev->status)) {
			spin_unlock_irqrestore(&mdev->status_lock, flags);
			usleep_range(5000, 6000); /* wait for sync_send */
			spin_lock_irqsave(&mdev->status_lock, flags);
		}
	} else {
		mdev->status |= (unsigned int)status;
	}

	spin_unlock_irqrestore(&mdev->status_lock, flags);
	ipc_notify_mailbox_event(EVENT_IPC_MBX_STATUS_CHANGE, mdev,
		"change status: 0x%X -> 0x%X", old_status, mdev->status);
out:
	return ret;
}

static void clr_status(struct hisi_mbox_device *mdev, int status)
{
	unsigned long flags = 0;
	int old_status = mdev->status;

	spin_lock_irqsave(&mdev->status_lock, flags);
	mdev->status &= ~(unsigned int)status;
	spin_unlock_irqrestore(&mdev->status_lock, flags);
	ipc_notify_mailbox_event(EVENT_IPC_MBX_STATUS_CHANGE, mdev,
		"change status: 0x%X -> 0x%X", old_status, mdev->status);
}

static int ipc_mbox_task_send_async(
	struct hisi_mbox_device *mdev, struct hisi_mbox_task *tx_task)
{
	int ret;

	mdev->ops->ensure_channel(mdev);

	mdev->tx_task = tx_task;
	ret = mdev->ops->send(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len,
		tx_task->need_auto_ack);
	if (ret)
		mbox_pr_err("mdev %s can not be sent", mdev->name);
	mdev->tx_task = NULL;
	return ret;
}

static void ipc_mbox_sync_fail_print(
	struct hisi_mbox_device *mdev, int need_irq_enable, int receipted)
{
	if (receipted && need_irq_enable)
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_TASKLET_JAM, mdev, "");
	else if (receipted && !need_irq_enable)
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_ISR_JAM, mdev, "");
	else if (!receipted)
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_ACK_LOST, mdev, "");
}

static int ipc_mbox_recv_ack_last_chance(
	struct hisi_mbox_device *mdev, struct hisi_mbox_task *tx_task,
	int need_irq_enable, int *receipted)
{
	mbox_msg_t *rx_buffer = NULL;
	mbox_msg_len_t rx_len;

	if (!mdev->ops->is_stm(mdev, ACK_STATUS) && !need_irq_enable)
		return -ETIMEOUT;

	rx_len = mdev->ops->recv(mdev, &rx_buffer);
	tx_task->ack_buffer = rx_buffer;
	tx_task->ack_buffer_len = rx_len;
	*receipted = 1;

	if (need_irq_enable)
		mdev->ops->enable_irq(mdev);

	return 0;
}

static void ipc_mbox_set_complete(struct hisi_mbox_device *mdev,
	int new_status, const char *func)
{
	ipc_notify_mailbox_event(EVENT_MBOX_COMPLETE_STATUS_CHANGE, mdev,
		"in func:%s complete status change: %d -> %d", func,
		mdev->completed, new_status);
	mdev->completed = new_status;
}

static int ipc_mbox_sync_process_record(struct hisi_mbox_device *mdev,
	struct hisi_mbox_task *tx_task, unsigned long *flags)
{
	int need_irq_enable = 0;
	int receipted = 0;
	int ret = 0;
	int wait = WAIT_ACK_TIMES;

	spin_lock_irqsave(&mdev->complete_lock, *flags);
	switch (mdev->completed) {
	case NOCOMPLETION:
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_PROC_RECORD,
			mdev, "NOCOMPLETION");
		ipc_mbox_set_complete(mdev, COMPLETED, __func__);
		break;

	case COMPLETING:
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_PROC_RECORD,
			mdev, "COMPLETING");
		/*
		 * Wait for ACK reception in behind half in 50ms
		 * Both ACK reception and irq restore will be handled
		 * here instead of in behind half, in the case of
		 * tasklet jam in irq-affinity core
		 */
		do {
			spin_unlock_irqrestore(&mdev->complete_lock, *flags);
			usleep_range(5000, 6000); /* wait for irqstore unlock */
			spin_lock_irqsave(&mdev->complete_lock, *flags);
		} while (mdev->completed != COMPLETED && wait--);

		/* Tasklet jam */
		if (mdev->completed != COMPLETED) {
			ipc_mbox_set_complete(mdev, COMPLETED, __func__);
			need_irq_enable = 1;
			break;
		}
	/* fall through */
	case COMPLETED:
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_PROC_RECORD,
			mdev, "COMPLETED");
		receipted = 1;
	/* fall through */
	default:
		goto unlock;
	}
	/* dump ipc regs */
	mdev->ops->dump_regs(mdev);
	/* Handle ack & irq */
	ret = ipc_mbox_recv_ack_last_chance(
		mdev, tx_task, need_irq_enable, &receipted);
unlock:
	spin_unlock_irqrestore(&mdev->complete_lock, *flags);
	ipc_mbox_sync_fail_print(mdev, need_irq_enable, receipted);
	return ret;
}

static unsigned long ipc_mbox_get_tx_timeout(struct hisi_mbox_device *mdev)
{
	unsigned int mdev_timeout = mdev->ops->get_timeout(mdev);

	return msecs_to_jiffies(mdev_timeout);
}

static int ipc_mbox_task_send_sync(
	struct hisi_mbox_device *mdev, struct hisi_mbox_task *tx_task)
{
	unsigned long tx_timeout;
	long timeout;
	int ret;
	unsigned long flags = 0;

	mdev->ops->ensure_channel(mdev);
	spin_lock_irqsave(&mdev->complete_lock, flags);
	mdev->complete.done = 0;
	ipc_mbox_set_complete(mdev, NOCOMPLETION, __func__);
	mdev->tx_task = tx_task;
	spin_unlock_irqrestore(&mdev->complete_lock, flags);

	ret = mdev->ops->send(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len,
		tx_task->need_auto_ack);
	if (ret) {
		mbox_pr_err("mdev %s can not be sent", mdev->name);
		goto out;
	}

	if (tx_task->need_auto_ack != AUTO_ACK &&
		tx_task->need_auto_ack != MANUAL_ACK) {
		mbox_pr_err("%s invalid ack mode", mdev->name);
		goto refresh;
	}

	tx_timeout = ipc_mbox_get_tx_timeout(mdev);

	ipc_notify_mailbox_event(EVENT_IPC_SYNC_SEND_BEGIN, mdev, "");

	timeout = wait_for_completion_timeout(&mdev->complete, tx_timeout);
	if (unlikely(timeout == 0)) {
		ipc_notify_mailbox_event(EVENT_IPC_SYNC_SEND_TIMEOUT, mdev,
			"%s ipc_timeout\n<INFO> MSG[0]:0x%08x MSG[1]:0x%08x\n",
			mdev->name,
			tx_task->tx_buffer[0], tx_task->tx_buffer[1]);

		/* Synchronization for isr */
		ret = ipc_mbox_sync_process_record(mdev, tx_task, &flags);
	} else {
		/* once success, clear the g_continuous_fail_cnt */
		ipc_mntn_reset_continuous_fail_cnt();
	}

refresh:
	mdev->ops->refresh(mdev);
out:
	/* completion */
	ipc_notify_mailbox_event(EVENT_IPC_SYNC_SEND_END, mdev,
		"mdev %s completion", mdev->name);
	mdev->tx_task = NULL;

	return ret;
}

int ipc_mbox_msg_send_sync(struct hisi_mbox *mbox, mbox_msg_t *tx_buffer,
	mbox_msg_len_t tx_buffer_len, int need_auto_ack, mbox_msg_t *ack_buffer,
	mbox_msg_len_t ack_buffer_len)
{
	struct hisi_mbox_device *mdev = NULL;
	struct hisi_mbox_task tx_task;
	size_t src_len, copy_len;
	int ret;

	tx_task.ack_buffer = NULL;
	if (!mbox || !mbox->tx || !tx_buffer) {
		if (!mbox)
			mbox_pr_err("null pointer mbox!");
		else
			mbox_pr_err("rproc_id:%d no tx ability or no tx_buffer",
				mbox->rproc_id);
		ret = -EINVAL;
		goto out;
	}
	ret = memcpy_s((void *)tx_task.tx_buffer,
			tx_buffer_len * (sizeof(*(tx_task.tx_buffer))),
			(void *)tx_buffer,
			tx_buffer_len * (sizeof(*tx_buffer)));
	if (ret != EOK) {
		mbox_pr_err("%s: memcpy_s failed", __func__);
		ret = -EINVAL;
		goto out;
	}
	tx_task.tx_buffer_len = tx_buffer_len;
	tx_task.need_auto_ack = need_auto_ack;

	mdev = mbox->tx;

	/* SYNC_SENDING start */
	ret = set_status(mdev, MDEV_SYNC_SENDING);
	if (ret) {
		mbox_pr_err("MSG{0x%08x, 0x%08x}", tx_task.tx_buffer[0],
			tx_task.tx_buffer[1]);
		goto out;
	}

	/* send */
	mutex_lock(&mdev->dev_lock);
	ret = ipc_mbox_task_send_sync(mdev, &tx_task);
	if (!ret && ack_buffer && tx_task.ack_buffer) {
		src_len = (size_t)(sizeof(*ack_buffer) / sizeof(u8) *
			  ack_buffer_len);
		copy_len = (size_t)(sizeof(*(tx_task.ack_buffer)) / sizeof(u8) *
			  ack_buffer_len);
		ret = memcpy_s((void *)ack_buffer, src_len,
			(void *)tx_task.ack_buffer, copy_len);
		if (ret != EOK)
			mbox_pr_err("%s: memcpy_s failed", __func__);
	}

	mutex_unlock(&mdev->dev_lock);

	/* SYNC_SENDING end */
	clr_status(mdev, MDEV_SYNC_SENDING);

out:
	return ret;
}
EXPORT_SYMBOL(ipc_mbox_msg_send_sync);

int ipc_mbox_msg_send_async(
	struct hisi_mbox *mbox, struct hisi_mbox_task *tx_task)
{
	struct hisi_mbox_device *mdev = NULL;
	int ret;
	unsigned long flags = 0;

	if (!tx_task || !mbox || !mbox->tx) {
		mbox_pr_err("invalid parameters");
		ret = -EINVAL;
		goto out;
	}

	mdev = mbox->tx;

	/* ASYNC_ENQUEUE start */
	ret = set_status(mdev, MDEV_ASYNC_ENQUEUE);
	if (ret) {
		mbox_pr_err("MSG{0x%08x, 0x%08x}",
			tx_task->tx_buffer[0], tx_task->tx_buffer[1]);
		goto out;
	}

	/* enqueue */
	spin_lock_irqsave(&mdev->fifo_lock, flags);
	if (kfifo_avail(&mdev->fifo) < TX_FIFO_CELL_SIZE) {
		spin_unlock_irqrestore(&mdev->fifo_lock, flags);

		ipc_notify_mailbox_event(
			EVENT_IPC_ASYNC_IN_QUEUE_FAIL, mdev, "");

		mbox_pr_err("%s : kfifo is full", __func__);
		ret = -ENOMEM;
		goto clearstatus;
	}

	if (kfifo_in(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE) !=
		TX_FIFO_CELL_SIZE)
		mbox_pr_err("%s : no enough kfifo", __func__);

	spin_unlock_irqrestore(&mdev->fifo_lock, flags);
	ipc_notify_mailbox_event(EVENT_IPC_ASYNC_TASK_IN_QUEUE, mdev, "");

	wake_up_interruptible(&mdev->tx_wait);

clearstatus:
	/* ASYNC_ENQUEUE end */
	clr_status(mdev, MDEV_ASYNC_ENQUEUE);
out:
	return ret;
}
EXPORT_SYMBOL(ipc_mbox_msg_send_async);

static struct hisi_mbox_task *ipc_mbox_dequeue_task(
	struct hisi_mbox_device *mdev)
{
	struct hisi_mbox_task *tx_task = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->fifo_lock, flags);
	if (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) {
		if (!kfifo_out(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE))
			tx_task = NULL;
	}

	spin_unlock_irqrestore(&mdev->fifo_lock, flags);
	return tx_task;
}

void ipc_mbox_empty_task(struct hisi_mbox_device *mdev)
{
	struct hisi_mbox_task *tx_task = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->fifo_lock, flags);
	while (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) {
		if (kfifo_out(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE))
			ipc_mbox_task_free(&tx_task);
	}
	spin_unlock_irqrestore(&mdev->fifo_lock, flags);

	mutex_lock(&mdev->dev_lock);
	/*
	 * do nothing here just to wait for the already-kfifo-out's
	 * tx_task finish
	 */
	mutex_unlock(&mdev->dev_lock);
}
EXPORT_SYMBOL(ipc_mbox_empty_task);

static inline void ipc_do_async_ack_work(struct hisi_mbox_task *tx_task)
{
	if (!tx_task->ack_handle)
		return;

	tx_task->ack_handle(tx_task->ack_buffer, tx_task->ack_buffer_len);
}

static int ipc_mbox_tx_thread(void *context)
{
	struct hisi_mbox_device *mdev = (struct hisi_mbox_device *)context;
	struct hisi_mbox_task *tx_task = NULL;
	int ret;

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(mdev->tx_wait,
			(kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE));
		if (unlikely(ret)) {
			mbox_pr_err("%s wait event failed", __func__);
			continue;
		}

		mutex_lock(&mdev->dev_lock);
		/*
		 * kick out the async send request from  mdev's kfifo one by one
		 * and send it out
		 */
		tx_task = ipc_mbox_dequeue_task(mdev);
		while (tx_task) {
			ipc_notify_mailbox_event(
				EVENT_IPC_ASYNC_TASK_SEND, mdev, "");
			if (tx_task->need_auto_ack != MANUAL_ACK) {
				ret = ipc_mbox_task_send_async(mdev, tx_task);
			} else {
				ret = ipc_mbox_task_send_sync(mdev, tx_task);
				ipc_do_async_ack_work(tx_task);
			}

			ipc_mbox_task_free(&tx_task);
			/* current task unlinked */
			mdev->tx_task = NULL;
			tx_task = ipc_mbox_dequeue_task(mdev);
		}

		mutex_unlock(&mdev->dev_lock);
	}
	return 0;
}

static void ipc_mbox_tx_bh(unsigned long context)
{
	struct hisi_mbox_device *mdev =
		(struct hisi_mbox_device *)(uintptr_t)context;
	mbox_msg_t *ack_buffer = NULL;
	mbox_msg_len_t ack_len;
	unsigned long flags = 0;

	ipc_notify_mailbox_event(EVENT_IPC_SEND_BH, mdev, "");
	/*
	 * check msg type
	 * - if ack interrupt occur,
	 *   an entire ipc have completed, and a completion should be excuted;
	 */
	if (unlikely(mdev->tx_task == NULL)) {
		/*
		 * the tasklet is blocked by after scheduled, after
		 * timeout for TASKLET jam,
		 * the tx_task is set to NULL,
		 * but then this tasklet has it's turn, then abort.
		 */
		return;
	}
	/*
	 * sometimes when TASKLET jam, tx_bh may be sched before
	 * tx_task== NULL,
	 * and this may make if (unlikely(NULL == mdev->tx_task))
	 * judeg failed
	 */
	spin_lock_irqsave(&mdev->complete_lock, flags);
	if (unlikely(mdev->completed != COMPLETING)) {
		spin_unlock_irqrestore(&mdev->complete_lock, flags);
		return;
	}
	ack_len = mdev->ops->recv(mdev, &ack_buffer);
	mdev->tx_task->ack_buffer = ack_buffer;
	mdev->tx_task->ack_buffer_len = ack_len;
	ipc_mbox_set_complete(mdev, COMPLETED, __func__);
	complete(&mdev->complete);
	spin_unlock_irqrestore(&mdev->complete_lock, flags);
	ipc_notify_mailbox_event(EVENT_IPC_SYNC_RECV_ACK, mdev,
		"ack_len:%d, completed_status:%d", ack_len, mdev->completed);

	mdev->ops->enable_irq(mdev);
	mbox_pr_debug("mdev %s ack leave", mdev->name);
}

static void ipc_mbox_rx_bh(unsigned long context)
{
	struct hisi_mbox_device *mdev =
		(struct hisi_mbox_device *)(uintptr_t)context;
	mbox_msg_t *rx_buffer = NULL;
	mbox_msg_len_t rx_len;

	ipc_notify_mailbox_event(EVENT_IPC_RECV_BH, mdev, "");
	mbox_pr_debug("mdev %s rx_bh enter\n", mdev->name);

	/*
	 * check msg type
	 * - if ack interrupt occur,
	 *   an entire ipc have completed, and a completion should be excuted;
	 * - if msg interrupt occur,
	 *   we need broadcast msgs to useres only after submit an ack.
	 */
	rx_len = mdev->ops->recv(mdev, &rx_buffer);
	atomic_notifier_call_chain(&mdev->notifier, rx_len, (void *)rx_buffer);
	mdev->ops->ack(mdev, NULL, 0);

	mdev->ops->enable_irq(mdev);
	mbox_pr_debug("mdev %s rx_bh leave", mdev->name);
}

static irqreturn_t ipc_mbox_interrupt(int irq, void *p)
{
	struct hisi_mbox_device *mdev = (struct hisi_mbox_device *)p;
	const char *err_msg =
		"has been handled\n caused of current IPC timeout(ISR)";

	if (!mdev->ops || !mdev->ops->irq_to_mdev) {
		mbox_pr_err("an unexpected interrupt");
		return IRQ_NONE;
	}

	ipc_notify_mailbox_event(EVENT_IPC_BEFORE_IRQ_TO_MDEV, mdev,
		"before irq_to_mdev: rproc_id:%d",
		mdev->ops->get_rproc_id(mdev));
	mdev = mdev->ops->irq_to_mdev(mdev, &mdevices, irq);
	if (!mdev) {
		mbox_pr_err("an unexpected interrupt %d occured", irq);
		return IRQ_NONE;
	}
	ipc_notify_mailbox_event(EVENT_IPC_AFTER_IRQ_TO_MDEV, mdev,
		"after irq_to_mdev: rproc_id:%d",
		mdev->ops->get_rproc_id(mdev));

	if (!mdev->configured) {
		mbox_pr_err("mdev %s has not startup yet", mdev->name);
		return IRQ_NONE;
	}

	/* ipc */
	if (mdev->ops->is_stm(mdev, DESTINATION_STATUS)) {
		mbox_pr_debug("mdev %s ipc", mdev->name);
		mdev->cur_task = RX_TASK;
		mdev->cur_irq = irq;
		mdev->ops->disable_irq(mdev);
		tasklet_schedule(&mdev->rx_bh);
		return IRQ_HANDLED;
	}

	/* ack */
	spin_lock(&mdev->complete_lock);
	if (mdev->tx_task && mdev->ops->is_stm(mdev, ACK_STATUS)) {
		if (unlikely(mdev->completed == COMPLETED)) {
			spin_unlock(&mdev->complete_lock);
			/*
			 * need to clear the ack if the ack is reached after
			 * 300ms' timeout, otherwise the ack will trigger all
			 * the time
			 */
			mdev->ops->clr_ack(mdev);
			mbox_pr_err("%s--%d %s", mdev->name, irq, err_msg);
			return IRQ_HANDLED;
		}

		mbox_pr_debug("mdev %s ack", mdev->name);
		ipc_mbox_set_complete(mdev, COMPLETING, __func__);
		mdev->cur_task = TX_TASK;
		mdev->cur_irq = irq;
		mdev->ops->disable_irq(mdev);
		tasklet_schedule(&mdev->tx_bh);
	}

	spin_unlock(&mdev->complete_lock);

	mbox_pr_debug("mdev %s interrupt leave, irq %d", mdev->name, irq);
	return IRQ_HANDLED;
}

static void ipc_mbox_shutdown(
	struct hisi_mbox_device *mdev, enum mbox_mail_type_t mail_type)
{
	mutex_lock(&mdev->dev_lock);
	if (!--mdev->configured) {
		mbox_pr_debug("%s shutdown", mdev->name);

		mdev->ops->free_irq(mdev, (void *)mdev);
		mdev->ops->shutdown(mdev);

		switch (mail_type) {
		case TX_MAIL:
			kthread_stop(mdev->tx_kthread);
			kfifo_free(&mdev->fifo);
			tasklet_kill(&mdev->tx_bh);
			break;
		case RX_MAIL:
			tasklet_kill(&mdev->rx_bh);
			break;
		default:
			break;
		}
	}

	mutex_unlock(&mdev->dev_lock);
}

static void ipc_mbox_free(struct hisi_mbox **mbox)
{
	struct hisi_mbox *_mbox = *mbox;

	kfree(_mbox);
	_mbox = NULL;

	*mbox = _mbox;
}

void ipc_mbox_put(struct hisi_mbox **mbox)
{
	struct hisi_mbox *_mbox = NULL;
	struct hisi_mbox_device *mdev[MAIL_TYPE_MAX] = { NULL };
	int i;

	if (!mbox) {
		mbox_pr_err("null pointer");
		return;
	}

	_mbox = *mbox;
	if (!_mbox) {
		mbox_pr_err("null pointer");
		return;
	}

	/* tx & rx mailbox devices deinit */
	mdev[TX_MAIL] = _mbox->tx;
	mdev[RX_MAIL] = _mbox->rx;
	for (i = TX_MAIL; i < MAIL_TYPE_MAX; i++) {
		if (mdev[i])
			ipc_mbox_shutdown(mdev[i], i);
	}

	if (mdev[RX_MAIL] && _mbox->nb)
		atomic_notifier_chain_unregister(
			&mdev[RX_MAIL]->notifier, _mbox->nb);

	ipc_mbox_free(mbox);
}
EXPORT_SYMBOL(ipc_mbox_put);

static int ipc_tx_mbox_startup(struct hisi_mbox_device *mdev)
{
	int tx_buff = mdev->ops->get_fifo_size(mdev) * TX_FIFO_CELL_SIZE;
	unsigned int sched_policy;
	struct sched_param param;

	mbox_pr_debug("tx mdev fifo_size is %u", tx_buff);
	if (kfifo_alloc(&mdev->fifo, tx_buff, GFP_KERNEL)) {
		mbox_pr_err("tx mdev %s alloc kfifo failed", mdev->name);
		return -ENOMEM;
	}

	init_waitqueue_head(&mdev->tx_wait);
	/* create the async tx thread */
	mdev->tx_kthread = kthread_create(ipc_mbox_tx_thread,
		(void *)mdev, "%s", mdev->name);
	if (unlikely(IS_ERR(mdev->tx_kthread))) {
		mbox_pr_err("create kthread tx_kthread failed!");
		kfifo_free(&mdev->fifo);
		return -EINVAL;
	}

	/*
	 * mailbox-13 needs to set the sched_priority
	 * adjust to the max 99,
	 * and set the policy adjust to the SCHED_FIFO,
	 * to solve the schedule nervous problem
	 */
	param.sched_priority =
		(MAX_RT_PRIO - mdev->ops->get_sched_priority(mdev));
	sched_policy = mdev->ops->get_sched_policy(mdev);
	mbox_pr_debug("tx mdev sched_priority is %u \tsched_policy is %u",
		param.sched_priority, sched_policy);
	(void)sched_setscheduler(mdev->tx_kthread, sched_policy, &param);
	wake_up_process(mdev->tx_kthread);

	/* tx mdev owns rx tasklet as well, for ipc ack msg. */
	tasklet_init(&mdev->tx_bh, ipc_mbox_tx_bh, (uintptr_t)mdev);

	return 0;
}

static void ipc_tx_mbox_startup_rollback(struct hisi_mbox_device *mdev)
{
	kfifo_free(&mdev->fifo);
	tasklet_kill(&mdev->tx_bh);
}

static void ipc_mbox_deinit_work(
	struct hisi_mbox_device *mdev, enum mbox_mail_type_t mail_type)
{
	switch (mail_type) {
	case TX_MAIL:
		ipc_tx_mbox_startup_rollback(mdev);
		break;
	case RX_MAIL:
		tasklet_kill(&mdev->rx_bh);
		break;
	default:
		break;
	}
}

static int ipc_mbox_startup(
	struct hisi_mbox_device *mdev, enum mbox_mail_type_t mail_type)
{
	int ret = 0;

	mutex_lock(&mdev->dev_lock);
	if (!mdev->configured++) {
		switch (mail_type) {
		case TX_MAIL:
			ret = ipc_tx_mbox_startup(mdev);
			if (ret)
				goto deconfig;
			break;

		case RX_MAIL:
			tasklet_init(
				&mdev->rx_bh, ipc_mbox_rx_bh, (uintptr_t)mdev);
			break;
		default:
			ret = -EINVAL;
			goto deconfig;
		}

		ret = mdev->ops->startup(mdev);
		if (ret) {
			mbox_pr_err("mdev %s startup failed", mdev->name);
			ret = -ENODEV;
			goto deinit_work;
		}

		ret = mdev->ops->request_irq(
			mdev, ipc_mbox_interrupt, (void *)mdev);
		if (ret) {
			mbox_pr_err("mdev %s request irq failed", mdev->name);
			ret = -ENODEV;
			goto shutdown;
		}
	}

	/* startup success */
	mutex_unlock(&mdev->dev_lock);
	return ret;

shutdown:
	mdev->ops->shutdown(mdev);
deinit_work:
	ipc_mbox_deinit_work(mdev, mail_type);
deconfig:
	mdev->configured--;
	mutex_unlock(&mdev->dev_lock);
	return ret;
}

static struct hisi_mbox *ipc_mbox_alloc(struct hisi_mbox_device *tx_mdev,
	struct hisi_mbox_device *rx_mdev, int rproc_id)
{
	struct hisi_mbox *mbox = NULL;

	mbox = kzalloc(sizeof(*mbox), GFP_KERNEL);
	if (!mbox)
		return NULL;

	mbox->rproc_id = rproc_id;
	mbox->tx = tx_mdev;
	mbox->rx = rx_mdev;
	mbox->nb = NULL;

	return mbox;
}

/*
 * search a certain mdev,
 * which could access the remote proccessor, "rp_name", in the list.
 */
static struct hisi_mbox_device *ipc_mbox_device_get(
	struct list_head *list, enum mbox_mail_type_t mtype, int rproc_id)
{
	struct hisi_mbox_device *mdev = NULL;
	struct hisi_mbox_device *_mdev = NULL;

	list_for_each_entry(_mdev, list, node) {
		/*
		 * though remote processor of mailboxes could be uncertain,
		 * when mailboxes prepare for use, it won't be an uncertain one
		 * or an unaccessible one, when a tx task of mailbox is alloced.
		 *
		 * the regulation is compatible for some mailboxes, whose remote
		 * processor could not be assigned, until mailboxes is used
		 * in an ipc.
		 */
		if (_mdev->ops->check(_mdev, mtype, rproc_id) !=
			RPUNACCESSIBLE) {
			mdev = _mdev;
			break;
		}
	}

	return mdev;
}

static void shutdown_mbox(int i, struct notifier_block *nb,
	struct hisi_mbox_device **mdev)
{
	if ((i == RX_MAIL) && nb)
		atomic_notifier_chain_unregister(&mdev[i]->notifier, nb);
	while (i--) {
		if (mdev[i])
			ipc_mbox_shutdown(mdev[i], i);
	}
}

struct hisi_mbox *ipc_mbox_get(int rproc_id, struct notifier_block *nb)
{
	struct hisi_mbox *mbox = NULL;
	struct list_head *list = &mdevices;
	struct hisi_mbox_device *mdev[MAIL_TYPE_MAX] = { NULL };
	int i;

	if (list_empty(list)) {
		mbox_pr_err("mailboxes not ready");
		goto out;
	}

	mdev[TX_MAIL] = ipc_mbox_device_get(list, TX_MAIL, rproc_id);
	mdev[RX_MAIL] = ipc_mbox_device_get(list, RX_MAIL, rproc_id);
	if ((!mdev[TX_MAIL]) && (!mdev[RX_MAIL]))
		goto out;

	for (i = TX_MAIL; i < MAIL_TYPE_MAX; i++) {
		if ((i == RX_MAIL) && mdev[i] && nb)
			atomic_notifier_chain_register(&mdev[i]->notifier, nb);

		if (mdev[i] && ipc_mbox_startup(mdev[i], i)) {
			mbox_pr_err("%s mdev %s startup failed",
				((i == TX_MAIL) ? "tx" : "rx"), mdev[i]->name);
			goto shutdown;
		}
	}

	mbox = ipc_mbox_alloc(mdev[TX_MAIL], mdev[RX_MAIL], rproc_id);
	if (!mbox) {
		mbox_pr_err("failed to alloc mbox");
		goto shutdown;
	}
	mbox->nb = nb;

	return mbox;

shutdown:
	shutdown_mbox(i, nb, mdev);
out:
	return mbox;
}
EXPORT_SYMBOL(ipc_mbox_get);

void ipc_mbox_device_deactivate(struct hisi_mbox_device **list)
{
	struct hisi_mbox_device *mdev = NULL;
	struct hisi_mbox_task *tx_task = NULL;
	int i;
	int ret;

	if (!list) {
		mbox_pr_err("no mboxes registered");
		return;
	}

	ipc_notify_mailbox_event(EVENT_IPC_DEACTIVATE_BEGIN, mdev, "");
	for (i = 0; (mdev = list[i]); i++) {
		ret = set_status(mdev, MDEV_DEACTIVATED);
		if (ret)
			continue;
		mutex_lock(&mdev->dev_lock);
		if (!mdev->configured) {
			mutex_unlock(&mdev->dev_lock);
			continue;
		}
		mutex_unlock(&mdev->dev_lock);

		mutex_lock(&mdev->dev_lock);
		tx_task = ipc_mbox_dequeue_task(mdev);
		while (tx_task) {
			ret = ipc_mbox_task_send_sync(mdev, tx_task);
			/* current task unlinked */
			mdev->tx_task = NULL;
			tx_task = ipc_mbox_dequeue_task(mdev);
		}
		mutex_unlock(&mdev->dev_lock);
	}
}
EXPORT_SYMBOL(ipc_mbox_device_deactivate);

void ipc_mbox_device_activate(struct hisi_mbox_device **list)
{
	struct hisi_mbox_device *mdev = NULL;
	int i;

	if (!list) {
		mbox_pr_err("no mboxes registered");
		return;
	}

	for (i = 0; (mdev = list[i]); i++)
		clr_status(mdev, MDEV_DEACTIVATED);
}
EXPORT_SYMBOL(ipc_mbox_device_activate);

int ipc_mbox_device_unregister(struct hisi_mbox_device **list)
{
	struct hisi_mbox_device *mdev = NULL;
	int i;

	if (!list) {
		mbox_pr_err("no mboxes registered");
		return -EINVAL;
	}

	for (i = 0; (mdev = list[i]); i++) {
		mutex_destroy(&mdev->dev_lock);
		device_destroy(g_mbox_class, (dev_t)i);
		list_del(&mdev->node);
	}

	return 0;
}
EXPORT_SYMBOL(ipc_mbox_device_unregister);

static int ipc_mbox_is_ops_invalid(struct ipc_mbox_dev_ops *ops)
{
	return !ops->startup || !ops->shutdown || !ops->check ||
		!ops->recv || !ops->send || !ops->refresh ||
		!ops->request_irq || !ops->free_irq || !ops->irq_to_mdev ||
		!ops->is_stm || !ops->get_ipc_version || !ops->get_rproc_id ||
		!ops->get_channel_size || !ops->get_fifo_size ||
		!ops->get_channel_id || !ops->show_mdev_info;
}

static int ipc_mbox_device_is_valid(struct hisi_mbox_device *mdev)
{
	if (ipc_mbox_is_ops_invalid(mdev->ops)) {
		mbox_pr_err("mdev ops invalid");
		return 0;
	}

	return 1;
}

int ipc_mbox_device_register(
	struct device *parent, struct hisi_mbox_device **list)
{
	struct hisi_mbox_device *mdev = NULL;
	int ret;
	int i;

	if (!list) {
		mbox_pr_err("null pointer");
		return -EINVAL;
	}

	for (i = 0; (mdev = list[i]); i++) {
		if (!ipc_mbox_device_is_valid(mdev)) {
			mbox_pr_err("invalid mdev");
			ret = -EINVAL;
			goto err_out;
		}

		mdev->dev = device_create(g_mbox_class, parent, (dev_t)i,
			mdev, "%s", mdev->name);
		if (IS_ERR(mdev->dev)) {
			mbox_pr_err(
				"mdev %s can not create device", mdev->name);
			ret = PTR_ERR(mdev->dev);
			goto err_out;
		}

		spin_lock_init(&mdev->fifo_lock);
		mdev->status = 0;
		spin_lock_init(&mdev->status_lock);
		spin_lock_init(&mdev->complete_lock);
		mutex_init(&mdev->dev_lock);

		init_completion(&mdev->complete);

		ATOMIC_INIT_NOTIFIER_HEAD(&mdev->notifier);
		list_add_tail(&mdev->node, &mdevices);
	}

	return 0;

err_out:
	while (i--) {
		mdev = list[i];
		list_del(&mdev->node);
		mutex_destroy(&mdev->dev_lock);
		device_destroy(g_mbox_class, (dev_t)i);
	}

	return ret;
}
EXPORT_SYMBOL(ipc_mbox_device_register);

static int __init ipc_mbox_init(void)
{
	int ret;

	mbox_pr_debug("%s: init", __func__);

	g_mbox_class = class_create(THIS_MODULE, "ipc-mailbox");
	if (IS_ERR(g_mbox_class))
		return PTR_ERR(g_mbox_class);

	spin_lock_init(&g_task_buffer_lock);
	ret = ipc_mbx_init_tx_task_buf();
	if (ret)
		return ret;

	ipc_mntn_register_mdevs(&mdevices);

	return 0;
}

postcore_initcall(ipc_mbox_init);

static void __exit ipc_mbox_exit(void)
{
	if (g_mbox_class)
		class_destroy(g_mbox_class);

	if (g_txtaskbuffer)
		kzfree(g_txtaskbuffer);
}

module_exit(ipc_mbox_exit);
/*lint +e580 */
/*lint +e578 */
