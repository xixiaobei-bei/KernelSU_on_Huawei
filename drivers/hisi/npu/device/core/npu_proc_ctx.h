/*
 * npu_proc_ctx.h
 *
 * about npu proc ctx
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

#include "npu_user_common.h"

struct npu_para {
	struct list_head list;
	pid_t pid;
	u32 cq_slot_size;
	u16 group_id; /* docker */
	u16 tflops;
	u16 disable_wakelock;
};

struct npu_proc_ctx {
	pid_t pid;
	u8 devid;
	u32 sink_stream_num;
	u32 stream_num;
	u32 event_num;
	u32 hwts_event_num;
	u32 cq_num;
	u32 sq_num;
	u32 model_num;
	u32 task_num;
	u32 send_count;
	u32 receive_count;
	u32 last_ts_status;
	int sq_round_index; /* refers to round-robin index in sq multiplexing */
	struct file *filep;
	/* add for sub info in LTE */
	void *proc_ctx_sub;
	/* ipc receive process will check this and find proc_context */
	int ipc_port;
	struct list_head sink_stream_list;
	struct list_head stream_list;
	struct list_head event_list;
	struct list_head hwts_event_list;
	struct list_head model_list;
	struct list_head task_list;
	struct list_head dev_ctx_list;
	struct list_head cq_list;
	struct list_head sq_list; /* refers to aval sq list for round-robin */
	atomic_t mailbox_message_count;
	u32 should_stop_thread;
	spinlock_t mailbox_wait_spinlock;
	struct semaphore mailbox_wait;
	struct list_head message_list_header;
	struct list_head ipc_msg_send_head;
	struct list_head ipc_msg_return_head;
	struct list_head l2_vma_list;
	struct list_head common_vma_list; /* save common vma list , use to delete list */

	wait_queue_head_t ipc_wait;
	struct work_struct recycle_work;
	int cq_tail_updated;
	wait_queue_head_t report_wait;
	struct mutex stream_mutex;
	struct mutex event_mutex;
	struct mutex model_mutex;
	struct mutex task_mutex;
	struct mutex map_mutex;
	struct npu_para para;
	DECLARE_BITMAP(stream_bitmap, NPU_MAX_STREAM_ID);
	DECLARE_BITMAP(event_bitmap, NPU_MAX_EVENT_ID);
	DECLARE_BITMAP(hwts_event_bitmap, NPU_MAX_HWTS_EVENT_ID);
	DECLARE_BITMAP(model_bitmap, NPU_MAX_MODEL_ID);
	DECLARE_BITMAP(task_bitmap, NPU_MAX_SINK_TASK_ID);
};

/* for get report phase byte */
struct npu_report {
	volatile u16 sop        : 1; /* unused,v100/v200, time:2020.2.17. start of packet, indicates this is the first 32bit return payload */
	volatile u16 mop        : 1; /* unused,v100/v200, time:2020.2.17. middle of packet, indicates the payload is a continuation of previous task return payload */
	volatile u16 eop        : 1; /* unused,v100/v200, time:2020.2.17. end of packet, indicates this is the last 32bit return payload.
								  * SOP & EOP can appear in the same packet, MOP & EOP can also appear on the same packet.
								  */
	volatile u16 report_type : 3;
	volatile u16 stream_id   : 10;
	volatile u16 task_id;
	volatile u16 sq_id      : 9;
	volatile u16 reserved   : 6;
	volatile u16 phase      : 1;
	volatile u16 sq_head;
	volatile u64 pay_load;
};

struct npu_report_payload {
	volatile uint64_t err_code : 32;
	volatile uint64_t persist_task_id : 16;
	volatile uint64_t persist_stream_id : 16;
};

// update in cq report interrupt
#define CQ_HEAD_UPDATED_FLAG 0x1
#define CQ_HEAD_INITIAL_FLAG 0x0
#define NPU_REPORT_PHASE 0x8000
#define NPU_CQ_PER_IRQ          1
#define NPU_CQ_UPDATE_IRQ_SUM   1
#define CQ_INVALID_PHASE 0xff

struct npu_cq_report_int_ctx {
	struct npu_proc_ctx *proc_ctx;
	int first_cq_index;
	struct tasklet_struct find_cq_task;
};

typedef enum {
	RREPORT_FROM_CQ_HEAD = 0x0,
	RREPORT_FROM_CQ_TAIL
} cq_report_pos_t;

int npu_proc_ctx_init(struct npu_proc_ctx *proc_ctx);

void npu_proc_ctx_destroy(struct npu_proc_ctx **proc_ctx_ptr);

struct npu_ts_cq_info *npu_proc_alloc_cq(
	struct npu_proc_ctx *proc_ctx);

int npu_proc_free_cq(struct npu_proc_ctx *proc_ctx);

int npu_proc_get_cq_id(struct npu_proc_ctx *proc_ctx, u32 *cq_id);

int npu_proc_send_alloc_stream_mailbox(struct npu_proc_ctx *proc_ctx);

int npu_proc_clear_sqcq_info(struct npu_proc_ctx *proc_ctx);

int npu_proc_alloc_stream(struct npu_proc_ctx *proc_ctx,
	struct npu_stream_info **stream_info, u16 strategy, u16 priority);

int npu_proc_free_stream(struct npu_proc_ctx *proc_ctx, u32 stream_id);

u32 npu_proc_get_cq_tail_report_phase(struct npu_proc_ctx *proc_ctx);

u32 npu_proc_get_cq_head_report_phase(struct npu_proc_ctx *proc_ctx);

int npu_request_cq_report_irq_bh(void);

int npu_free_cq_report_irq_bh(void);

void npu_bind_proc_ctx_with_cq_int_ctx(struct npu_proc_ctx *proc_ctx);

void npu_unbind_proc_ctx_with_cq_int_ctx(
	const struct npu_proc_ctx *proc_ctx);

struct npu_proc_ctx *npu_get_proc_ctx_with_int_ctx(void);

int npu_proc_alloc_event(struct npu_proc_ctx *proc_ctx,
	u32 *event_id_ptr, u16 strategy);

int npu_proc_free_event(struct npu_proc_ctx *proc_ctx, u32 event_id,
	u16 strategy);

int npu_proc_alloc_model(struct npu_proc_ctx *proc_ctx,
	u32 *model_id_ptr);

int npu_proc_free_model(struct npu_proc_ctx *proc_ctx, u32 model_id);

int npu_proc_alloc_task(struct npu_proc_ctx *proc_ctx, u32 *task_id_ptr);

int npu_proc_free_task(struct npu_proc_ctx *proc_ctx, u32 task_id);

inline u16 npu_get_phase_from_report(struct npu_report *report);

inline void npu_set_phase_to_report(struct npu_report *report, u16 val);

bool npu_vma_valid(const struct vm_area_struct *vma);

#endif /* __NPU_MANAGER_H */
