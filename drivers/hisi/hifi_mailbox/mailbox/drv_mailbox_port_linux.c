/*
 * drv_mailbox_port_linux.c
 *
 * mailbox implement on linux platform
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

#include "drv_mailbox_port_linux.h"

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/types.h>
#include <uapi/linux/sched/types.h>
#include <linux/hisi/audio_log.h>

#include "drv_mailbox_gut.h"
#include "drv_mailbox_cfg.h"
#include "bsp_drv_ipc.h"
#include "drv_mailbox_debug.h"
#include "drv_mailbox_platform.h"

#undef _MAILBOX_FILE_
#define _MAILBOX_FILE_ "linux"

#define LOG_TAG "mailbox_port_linux"

#define MAILBOX_MILLISEC_PER_SECOND 1000
#define PROC_NAME_LEN 16

/* Send full delay in milliseconds */
#define MAILBOX_LINUX_SEND_FULL_DELAY_MS 10

/* Number of retry attempts after sending full delay */
#define MAILBOX_LINUX_SEND_FULL_DELAY_TIMES 0
#define MAILBOX_NORMAL_PRORERTY 86
#define MAILBOX_HIGH_PRORERTY 99

enum mailbox_lock_type {
	MAILBOX_LOCK_SEMAPHORE = 0x00010000,
	MAILBOX_LOCK_SPINLOCK = 0x00020000
};

/* Define the calling method of the mailbox data processing interface */
#define MAILBOX_PROC_MASK 0x0000ffff
enum mailbox_linux_proc_style {
	MAILBOX_SEND = 0,

	MAILBOX_RECEV_START,

	/* The mail processing queue attached to the task starts */
	MAILBOX_RECV_TASK_START,
	MAILBOX_RECV_TASK_NORMAL,
	MAILBOX_RECV_TASK_HIGH,

	/* The mail processing queue attached to the task ends */
	MAILBOX_RECV_TASK_END,

	/* Messages processed in tasklet */
	MAILBOX_RECV_TASKLET,
	MAILBOX_RECV_TASKLET_HI,

	/* Mail processing queue processed in interrupt */
	MAILBOX_RECV_INT_IRQ,
	MAILBOX_RECV_END
};

/* Define the mailbox work queue and its data attached to a single task */
struct mb_local_work {
	/* May be the connection ID between cores or the physical channel ID */
	uint32_t channel_id;
	/* Does this mailbox have data flags */
	uint32_t data_flag;
	int (*cb)(uint32_t channel_id);
	struct mb_local_work *next;
#ifdef MAILBOX_OPEN_MNTN
	void *mb_priv;
#endif
};

/* Define the mailbox task related data */
struct mb_local_proc {
	char proc_name[PROC_NAME_LEN];
	uint32_t proc_id;
	int priority;
	struct mb_local_work *work_list;
	wait_queue_head_t wait;
	struct tasklet_struct tasklet;
	struct task_struct *task;
	int incoming;
};

/*
 * Define the correspondence between mailbox physical channels and tasks,
 * inter-core interrupts and receive callback functions
 */
struct mb_local_cfg {
	uint32_t channel_id;
	uint32_t property;
	uint32_t int_src;
	uint32_t dst_id;
};

/* Prevents sleep during wake-up and email processing */
static struct wakeup_source g_mb_lpwr_lock;
static bool g_is_usb_suspend;

/* Mailbox task attribute list */
static struct mb_local_proc g_mailbox_local_proc_tbl[] = {
	/* Task-based mail data processing */
	{
		"mailboxNormal",
		MAILBOX_RECV_TASK_NORMAL,
		MAILBOX_NORMAL_PRORERTY,
		NULL,
	},
	{
		"mailboxHigh",
		MAILBOX_RECV_TASK_HIGH,
		MAILBOX_HIGH_PRORERTY,
		NULL,
	},

	/* Process messages with tasklet */
	{
		"mailboxTasklet",
		MAILBOX_RECV_TASKLET,
		0,
		NULL,
	},
	{
		"mailboxTasklet",
		MAILBOX_RECV_TASKLET_HI,
		0,
		NULL,
	},

	/* Interrupted mail data processing */
	{
		"mailboxInt",
		MAILBOX_RECV_INT_IRQ,
		0,
		NULL,
	},
};

/*
 * Correspondence configuration table of A-core mailbox channel resources
 * and platform system resources
 */
static struct mb_local_cfg g_mb_local_cfg_tbl[] = {
	/* Receive channel configuration */
	{
		MAILBOX_MAILCODE_RESERVED(MCU, ACPU, MSG),
		MAILBOX_RECV_TASKLET_HI,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(HIFI, ACPU, MSG),
		MAILBOX_RECV_TASKLET_HI,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(CCPU, ACPU, MSG),
		MAILBOX_RECV_TASKLET_HI,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(CCPU, ACPU, IFC),
		MAILBOX_RECV_TASK_NORMAL,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(MCU, ACPU, IFC),
		MAILBOX_RECV_TASK_HIGH,
		0,
	},

	/* Send channel configuration */
	{
		MAILBOX_MAILCODE_RESERVED(ACPU, MCU, MSG),
		MAILBOX_SEND | MAILBOX_LOCK_SPINLOCK,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(ACPU, HIFI, MSG),
		MAILBOX_SEND | MAILBOX_LOCK_SPINLOCK,
		0,
	},

	/*
	 * If the message channel is used as the return channel of the IFC,
	 * and the function of the IFC executor has a sleep action,
	 * the message channel can only be protected by the semaphore
	 */
	{
		MAILBOX_MAILCODE_RESERVED(ACPU, CCPU, MSG),
		MAILBOX_SEND | MAILBOX_LOCK_SEMAPHORE,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(ACPU, CCPU, IFC),
		MAILBOX_SEND | MAILBOX_LOCK_SEMAPHORE,
		0,
	},
	{
		MAILBOX_MAILCODE_RESERVED(ACPU, MCU, IFC),
		MAILBOX_SEND | MAILBOX_LOCK_SEMAPHORE,
		0,
	},

	{
		MAILBOX_MAILCODE_INVALID,
		0,
		0,
	},
};

/* Create different locks according to different channel types */
static void mailbox_mutex_init(const struct mb_local_cfg *local_cfg,
	struct mb_mutex *mtx)
{
	if ((local_cfg->property > MAILBOX_RECEV_START) &&
		(local_cfg->property < MAILBOX_RECV_END)) {
		spin_lock_init(&mtx->splock);
		mtx->type = MAILBOX_LOCK_SPINLOCK;
	} else {
		if (local_cfg->property & MAILBOX_LOCK_SEMAPHORE) {
			sema_init(&mtx->sem, 1);
			mtx->type = MAILBOX_LOCK_SEMAPHORE;
		} else if (local_cfg->property & MAILBOX_LOCK_SPINLOCK) {
			spin_lock_init(&mtx->splock);
			mtx->type = MAILBOX_LOCK_SPINLOCK;
		} else {
			AUDIO_LOGE("unknow property: %u, use spin_lock",
				local_cfg->property);
			spin_lock_init(&mtx->splock);
			mtx->type = MAILBOX_LOCK_SPINLOCK;
		}
	}
}

void mailbox_usb_suspend(bool is_suspend)
{
	AUDIO_LOGI("mailbox usb set suspend: %d", is_suspend);
	g_is_usb_suspend = is_suspend;
}

static void mailbox_receive_process(unsigned long data)
{
	struct mb_local_proc *proc = (struct mb_local_proc *)(uintptr_t)data;
	struct mb_local_work *work = proc->work_list;

	while (work != NULL) {
		if (work->data_flag != MAILBOX_TRUE) {
			work = work->next;
			continue;
		}

		work->data_flag = MAILBOX_FALSE;
#ifdef MAILBOX_OPEN_MNTN
		mailbox_record_sche_recv(work->mb_priv);
#endif
		if (work->cb != NULL) {
			if (work->cb(work->channel_id) != 0)
				mb_logerro_p1(MB_E_LINUX_CALLBACK_ERRO, work->channel_id);
		} else {
			mb_logerro_p1(MB_E_LINUX_CALLBACK_NOT_FIND, work->channel_id);
		}
		work = work->next;
	}
	if (g_mb_lpwr_lock.active)
		__pm_relax(&g_mb_lpwr_lock);
}

static int mailbox_receive_task(void *data)
{
	struct mb_local_proc *proc = data;
	struct sched_param param;

	param.sched_priority = (proc->priority < MAX_RT_PRIO) ?
		proc->priority : (MAX_RT_PRIO - 1);
	(void)sched_setscheduler(current, SCHED_FIFO, &param);

	do {
		wait_event(proc->wait, proc->incoming);
		proc->incoming = MAILBOX_FALSE;

		mailbox_receive_process((uintptr_t)data);
	} while (!kthread_should_stop());

	return 0;
}

void mailbox_destory_platform(void)
{
	uint32_t proc_id;
	struct mb_local_proc *local_proc = &g_mailbox_local_proc_tbl[0];
	uint32_t count = ARRAY_SIZE(g_mailbox_local_proc_tbl);

	while (count) {
		proc_id = local_proc->proc_id;
		if ((proc_id > MAILBOX_RECV_TASK_START) &&
			(proc_id < MAILBOX_RECV_TASK_END)) {
			if (local_proc->task != NULL) {
				kthread_stop(local_proc->task);
				local_proc->task = NULL;
			}
		}

		count--;
		local_proc++;
	}

	wakeup_source_trash(&g_mb_lpwr_lock);
}

int mailbox_init_platform(void)
{
	int ret = 0;
	uint32_t proc_id;
	struct mb_local_proc *local_proc = &g_mailbox_local_proc_tbl[0];
	uint32_t count = ARRAY_SIZE(g_mailbox_local_proc_tbl);

	wakeup_source_init(&g_mb_lpwr_lock, "mailbox_low_power_wake_lock");

	/* Create platform task interrupt semaphore part */
	while (count) {
		proc_id = local_proc->proc_id;

		if ((proc_id > MAILBOX_RECV_TASK_START) &&
			(proc_id < MAILBOX_RECV_TASK_END)) {
			init_waitqueue_head(&local_proc->wait);

			local_proc->task = kthread_run(mailbox_receive_task,
				(void *)local_proc, "%s",
				local_proc->proc_name);
			if (IS_ERR(local_proc->task)) {
				ret = mb_logerro_p1(MB_E_LINUX_TASK_CREATE,
					proc_id);
				mailbox_destory_platform();
				return ret;
			}
		}

		if ((proc_id == MAILBOX_RECV_TASKLET) || (proc_id == MAILBOX_RECV_TASKLET_HI))
			tasklet_init(&local_proc->tasklet, mailbox_receive_process, (uintptr_t)local_proc);

		count--;
		local_proc++;
	}

	return 0;
}

static int mailbox_ipc_process(struct mb_local_work *local_work,
	struct mb_local_proc *local_proc, uint32_t channel_id,
	uint32_t proc_id)
{
	uint32_t is_find = MAILBOX_TRUE;

	while (local_work) {
		if (channel_id == local_work->channel_id) {
			local_work->data_flag = MAILBOX_TRUE;

#ifdef MAILBOX_OPEN_MNTN
			mailbox_record_sche_send(local_work->mb_priv);
#endif

			/*
			 * Usb driver may use mailbox in suspend context
			 * wake_lock will make suspend flow aborted
			 */
			if (!g_is_usb_suspend)
				__pm_stay_awake(&g_mb_lpwr_lock);

			if ((proc_id > MAILBOX_RECV_TASK_START) &&
				(proc_id < MAILBOX_RECV_TASK_END)) {
				local_proc->incoming = MAILBOX_TRUE;
				wake_up(&local_proc->wait);
			} else if (proc_id == MAILBOX_RECV_TASKLET_HI) {
				tasklet_hi_schedule(&local_proc->tasklet);
			} else if (proc_id == MAILBOX_RECV_TASKLET) {
				tasklet_schedule(&local_proc->tasklet);
			} else if (proc_id == MAILBOX_RECV_INT_IRQ) {
				mailbox_receive_process((uintptr_t)local_proc);
			} else {
				is_find = MAILBOX_FALSE;
			}
		}

		local_work = local_work->next;
	}

	return is_find;
}

static int mailbox_ipc_int_handle(uint32_t int_num)
{
	struct mb_local_cfg *local_cfg = &g_mb_local_cfg_tbl[0];
	struct mb_local_proc *l_proc = NULL;
	struct mb_local_work *l_work = NULL;
	uint32_t count;
	uint32_t proc_id;
	uint32_t channel_id = 0;
	uint32_t is_find = MAILBOX_FALSE;
	uint32_t ret = 0;

	while (local_cfg->channel_id != MAILBOX_MAILCODE_INVALID) {
		proc_id = local_cfg->property;

		if ((int_num != local_cfg->int_src) ||
			((local_cfg->property & MAILBOX_PROC_MASK) ==
			MAILBOX_SEND)) {
			local_cfg++;
			continue;
		}

		channel_id = local_cfg->channel_id;

		l_proc = &g_mailbox_local_proc_tbl[0];
		count = ARRAY_SIZE(g_mailbox_local_proc_tbl);
		while (count) {
			if (proc_id == l_proc->proc_id) {
				l_work = l_proc->work_list;
				is_find = mailbox_ipc_process(l_work, l_proc,
						channel_id, proc_id);
				break;
			}
			count--;
			l_proc++;
		}

		if (count == 0)
			ret = mb_logerro_p1(MB_E_LINUX_MAIL_TASK_NOT_FIND, channel_id);

		local_cfg++;
	}

	if (is_find != MAILBOX_TRUE)
		ret = mb_logerro_p1(MB_E_LINUX_MAIL_INT_NOT_FIND, channel_id);

	return ret;
}

void mailbox_local_work_free(void)
{
	struct mb_local_proc *l_proc = &g_mailbox_local_proc_tbl[0];
	uint32_t count = ARRAY_SIZE(g_mailbox_local_proc_tbl);
	struct mb_local_work *cur = NULL;
	struct mb_local_work *tmp = NULL;

	while (count) {
		cur = l_proc->work_list;

		if (cur == NULL) {
			count--;
			l_proc++;
			continue;
		}

		while (cur->next != NULL) {
			tmp = cur;
			cur = cur->next;

			kfree(tmp);
		}

		kfree(cur);
		l_proc->work_list = NULL;

		count--;
		l_proc++;
	}
}

int mailbox_process_register(uint32_t channel_id,
	int (*cb)(uint32_t channel_id), void *priv)
{
	struct mb_local_work *l_work = NULL;
	struct mb_local_cfg *local_cfg = &g_mb_local_cfg_tbl[0];
	struct mb_local_proc *l_proc = &g_mailbox_local_proc_tbl[0];
	struct mb_local_cfg *find_cfg = NULL;
	uint32_t count = ARRAY_SIZE(g_mailbox_local_proc_tbl);

	while (local_cfg->channel_id != MAILBOX_MAILCODE_INVALID) {
		if (local_cfg->channel_id == channel_id) {
			find_cfg = local_cfg;
			break;
		}
		local_cfg++;
	}

	if (find_cfg == NULL)
		return mb_logerro_p1(MB_E_LINUX_CHANNEL_NOT_FIND, channel_id);

	while (count) {
		if (find_cfg->property != l_proc->proc_id) {
			count--;
			l_proc++;
			continue;
		}

		if (l_proc->work_list) {
			l_work = l_proc->work_list;
			while (l_work->next != NULL)
				l_work = l_work->next;

			l_work->next = kcalloc(
				sizeof(struct mb_local_work), 1, GFP_KERNEL);
			if (l_work->next == NULL) {
				AUDIO_LOGE("memory alloc error");
				return (int)MB_E_LINUX_ALLOC_MEMORY;
			}
			l_work->next->channel_id = find_cfg->channel_id;
			l_work->next->cb = cb;
			l_work->next->next = NULL;
			l_work->next->mb_priv = priv;
		} else {
			l_proc->work_list = kcalloc(
				sizeof(struct mb_local_work), 1, GFP_KERNEL);
			if (l_proc->work_list == NULL) {
				AUDIO_LOGE("memory alloc error");
				return (int)MB_E_LINUX_ALLOC_MEMORY;
			}
			l_proc->work_list->channel_id = find_cfg->channel_id;
			l_proc->work_list->cb = cb;
			l_proc->work_list->next = NULL;
			l_proc->work_list->mb_priv = priv;
		}

		count--;
		l_proc++;
	}

	return 0;
}

int mailbox_channel_register(uint32_t channel_id, uint32_t int_src,
	uint32_t dst_id, uint32_t direct, struct mb_mutex *mutex)
{
	struct mb_local_cfg *local_cfg = &g_mb_local_cfg_tbl[0];

	if (int_src >= INTSRC_NUM) {
		AUDIO_LOGE("invalid int level %u", int_src);
		return -1;
	}

	while (local_cfg->channel_id != MAILBOX_MAILCODE_INVALID) {
		if (channel_id == local_cfg->channel_id) {
			mailbox_mutex_init(local_cfg, mutex);

			local_cfg->int_src = int_src;
			local_cfg->dst_id = dst_id;

			if (direct == MB_DIRECTION_RECEIVE)
				ipc_int_connect((enum ipc_int_lev)int_src, mailbox_ipc_int_handle, int_src);

			break;
		}

		local_cfg++;
	}

	/* Cannot find the corresponding configuration */
	if (local_cfg->channel_id == MAILBOX_MAILCODE_INVALID)
		return mb_logerro_p1(MB_E_LINUX_CHANNEL_NOT_FIND, channel_id);

	return 0;
}

int mailbox_delivery(uint32_t channel_id)
{
	struct mb_local_cfg *local_cfg = &g_mb_local_cfg_tbl[0];
	struct mb_local_cfg *find_cfg = NULL;

	while (local_cfg->channel_id != MAILBOX_MAILCODE_INVALID) {
		if (local_cfg->channel_id == channel_id) {
			find_cfg = local_cfg;
			break;
		}
		local_cfg++;
	}

	if (find_cfg != NULL) {
		return (int)ipc_int_send((enum ipc_int_core)find_cfg->dst_id,
			(enum ipc_int_lev)find_cfg->int_src);
	}

	return mb_logerro_p1(MB_E_LINUX_CHANNEL_NOT_FIND, channel_id);
}

void mailbox_mutex_lock(struct mb_mutex *mtx)
{
	if (mtx->type == MAILBOX_LOCK_SEMAPHORE)
		down(&mtx->sem);
	else if (mtx->type == MAILBOX_LOCK_SPINLOCK)
		spin_lock_irqsave(&mtx->splock, mtx->flags);
}

void mailbox_mutex_unlock(struct mb_mutex *mtx)
{
	if (mtx->type == MAILBOX_LOCK_SEMAPHORE)
		up(&mtx->sem);
	else if (mtx->type == MAILBOX_LOCK_SPINLOCK)
		spin_unlock_irqrestore(&mtx->splock, mtx->flags);
}

void *mailbox_memcpy(void *dst, const void *src, uint32_t size)
{
	return memcpy(dst, src, size); /* unsafe_function_ignore: memcpy */
}

void *mailbox_memset(void *m, int c, uint32_t size)
{
	return memset(m, c, size); /* unsafe_function_ignore: memset */
}

void mailbox_assert(uint32_t err_no)
{
}

int mailbox_int_context(void)
{
	return in_interrupt();
}

int mailbox_scene_delay(uint32_t scene_id, int *try_times)
{
	uint32_t go_on = MAILBOX_FALSE;
	uint32_t delay_ms = 0;

	switch (scene_id) {
	case MAILBOX_DELAY_SCENE_MSG_FULL:
	case MAILBOX_DELAY_SCENE_IFC_FULL:
		delay_ms = MAILBOX_LINUX_SEND_FULL_DELAY_MS;

		go_on = (*try_times >= MAILBOX_LINUX_SEND_FULL_DELAY_TIMES) ?
			MAILBOX_FALSE : MAILBOX_TRUE;
		break;
	default:
		break;
	}

	if (go_on == MAILBOX_TRUE)
		msleep(delay_ms);

	*try_times = *try_times + 1;

	return go_on;
}

int mailbox_get_timestamp(void)
{
	void *slice_reg = get_slice_reg();

	if (slice_reg != NULL)
		return readl(slice_reg);

	return 0;
}
