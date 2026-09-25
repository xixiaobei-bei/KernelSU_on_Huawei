/*
 * npu_proc_ctx.h
 *
 * about npu proc ctx
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_PROC_CTX_H
#define __NPU_PROC_CTX_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include "devdrv_user_common.h"

struct devdrv_para {
	struct list_head list;
	pid_t pid;
	u32 cq_slot_size;
	u16 group_id; /* docker */
	u16 tflops;
	u16 disable_wakelock;
};

struct devdrv_proc_ctx {
	pid_t pid;
	u8 devid;
	u32 sink_stream_num;
	u32 stream_num;
	u32 event_num;
	u32 cq_num;
	u32 model_num;
	u32 task_num;
	u32 send_count;
	u32 receive_count;
	u32 last_ts_status;
	struct file* filep;
	bool map_ops_flag; /* for forbidding the mmap in user space */
	/* add for sub info in LTE */
	void *proc_ctx_sub;
	/* ipc receive process will check this and find proc_context */
	int ipc_port;
	struct list_head sink_stream_list;
	struct list_head stream_list;
	struct list_head event_list;
	struct list_head model_list;
	struct list_head task_list;
	struct list_head dev_ctx_list;
	struct list_head cq_list;
	atomic_t mailbox_message_count;
	u32 should_stop_thread;
	spinlock_t mailbox_wait_spinlock;
	struct semaphore mailbox_wait;
	struct list_head message_list_header;
	struct list_head ipc_msg_send_head;
	struct list_head ipc_msg_return_head;
	struct list_head l2_vma_list;
	struct list_head cma_vma_list;
	struct list_head sqcq_vma_list;

	wait_queue_head_t ipc_wait;
	struct work_struct recycle_work;
	int cq_tail_updated;
	wait_queue_head_t report_wait;
	struct mutex stream_mutex;
	struct mutex event_mutex;
	struct mutex model_mutex;
	struct mutex task_mutex;
	struct mutex map_mutex;
	struct devdrv_para para;
	DECLARE_BITMAP(stream_bitmap, DEVDRV_MAX_STREAM_ID);
	DECLARE_BITMAP(event_bitmap, DEVDRV_MAX_EVENT_ID);
	DECLARE_BITMAP(model_bitmap, DEVDRV_MAX_MODEL_ID);
	DECLARE_BITMAP(task_bitmap, DEVDRV_MAX_TASK_ID);
};

/* for get report phase byte */
struct devdrv_report {
	u32 a;
	u32 b;
	u32 c;
};

// update in cq report interrupt
#define CQ_HEAD_UPDATED_FLAG 0x1
#define CQ_HEAD_INITIAL_UPDATE_FLAG 0x0
#define DEVDRV_REPORT_PHASE 0x8000
#define devdrv_get_phase_from_report(report) ((report->c & DEVDRV_REPORT_PHASE) >> 15)
#define devdrv_clr_phase_in_report(report)  ((report->c) &= (~DEVDRV_REPORT_PHASE))
#define DEVDRV_CQ_PER_IRQ          1
#define DEVDRV_CQ_UPDATE_IRQ_SUM   1

struct devdrv_cq_report_int_ctx {
	struct devdrv_proc_ctx *proc_ctx;
	int first_cq_index;
	struct tasklet_struct find_cq_task;
};

typedef enum {
	RREPORT_FROM_CQ_HEAD = 0x0,
	RREPORT_FROM_CQ_TAIL
} cq_report_pos_t;

int devdrv_proc_ctx_init(struct devdrv_proc_ctx *proc_ctx);

void devdrv_proc_ctx_destroy(struct devdrv_proc_ctx **proc_ctx_ptr);

struct devdrv_ts_cq_info *devdrv_proc_alloc_cq(struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_free_cq(struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_send_alloc_stream_mailbox(struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_clr_sqcq_info(struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_alloc_stream(struct devdrv_proc_ctx *proc_ctx, u32 *stream_id, u32 strategy);

int devdrv_proc_free_stream(struct devdrv_proc_ctx *proc_ctx, u32 stream_id);

u32 devdrv_proc_get_cq_tail_report_phase(struct devdrv_proc_ctx *proc_ctx);

u32 devdrv_proc_get_cq_head_report_phase(struct devdrv_proc_ctx *proc_ctx);

bool devdrv_proc_is_valid_report_received(struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_report_wait(struct devdrv_proc_ctx *proc_ctx, int timeout);

int devdrv_request_cq_report_irq_bh(void);

int devdrv_free_cq_report_irq_bh(void);

void devdrv_bind_proc_ctx_with_cq_int_ctx(struct devdrv_proc_ctx *proc_ctx);

void devdrv_unbind_proc_ctx_with_cq_int_ctx(const struct devdrv_proc_ctx *proc_ctx);

int devdrv_proc_alloc_event(struct devdrv_proc_ctx *proc_ctx, u32* event_id_ptr);

int devdrv_proc_free_event(struct devdrv_proc_ctx *proc_ctx, u32 event_id);

int devdrv_proc_alloc_model(struct devdrv_proc_ctx *proc_ctx, u32* model_id_ptr);

int devdrv_proc_free_model(struct devdrv_proc_ctx *proc_ctx, u32 model_id);

int devdrv_proc_alloc_task(struct devdrv_proc_ctx *proc_ctx, u32* task_id_ptr);

int devdrv_proc_free_task(struct devdrv_proc_ctx *proc_ctx, u32 task_id);

bool devdrv_vma_valid(const struct vm_area_struct *vma);


#endif /* __DEVDRV_MANAGER_H */
