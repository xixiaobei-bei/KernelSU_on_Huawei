/*
 * npu_dfx_cq.h
 *
 * about npu dfx cq
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
#ifndef __NPU_DFX_CQ_H
#define __NPU_DFX_CQ_H

#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_dfx_sq.h"
#include "npu_platform.h"
#include "npu_common.h"

#define DEVDRV_MAX_DFX_CQ_DEPTH             256

#define DEVDRV_DFX_PHASE_ZERO               0
#define DEVDRV_DFX_PHASE_ONE                1
#define DEVDRV_DFX_BRIEF_CQ_OFFSET          4
#define DEVDRV_DFX_DETAILED_CQ_OFFSET       8
#define DEVDRV_DFX_CQ_FIRST_INDEX           116
#define DEVDRV_DFX_MAX_SQ_SLOT_LEN          128
#define DEVDRV_DFX_MAX_CQ_SLOT_LEN          128

#define PROFILING_AICPU_SIZE                0x40000
#define PROFILING_AICORE_SIZE               0x40000
#define PROFILING_TSFW_SIZE                 0x40000

enum cq_info_type {
	DFX_BRIEF_CQ = 0,
	DFX_DETAILED_CQ
};

enum dfx_sq_type {
	DFX_DBG_SQ = 0,
	DFX_LOG_SQ = 1,
	DFX_PROF_SQ = 2,
	DFX_HEART_BEAT_SQ = 3,
	DFX_MAX_SQ = DEVDRV_MAX_DFX_SQ_NUM
};

enum dfx_cq_type {
	DFX_DBG_REPORT_CQ = 0,
	DFX_DBG_TS_CALLBACK_CQ = 1,
	DFX_LOG_REPORT_CQ = 2,
	DFX_LOG_CALLBACK_CQ = 3,
	DFX_PROF_REPORT_CQ = 4,
	DFX_PROF_TSCPU_CALLBACK_CQ = 5,
	DFX_PROF_AICORE_CALLBACK_CQ = 6,
	DFX_PROF_AICPU_CALLBACK_CQ = 7,
	DFX_HEART_BEAT_REPORT_CQ = 8,
	DFX_MAX_CQ = DEVDRV_MAX_DFX_CQ_NUM
};

struct devdrv_dfx_sq_info {
	u32 devid;
	u32 index;
	u32 depth;
	u32 slot_len;
	u8 *addr;
	phys_addr_t phy_addr;
	u32 head;
	u32 tail;
	u32 credit;
	u32 *doorbell;
	enum dfx_cqsq_func_type function;
};

struct devdrv_dfx_create_cq_para {
	u32 cq_type;
	u32 cq_index;
	u32 slot_len;
	enum dfx_cqsq_func_type function;
	void (*callback) (const u8 *cq_slot, const u8 *sq_slot);
	u64 *addr;
};

struct devdrv_dfx_cq_report {
	u8 phase;
	u8 reserved[3];
	u16 sq_index;
	u16 sq_head;
};

struct devdrv_mailbox_cqsq {
	u16 valid;       /* validity judgement, 0x5a5a is valid */
	u16 cmd_type;     /* command type */
	u32 result;       /* TS's process result succ or fail: no error: 0, error: not 0 */
	u64 sq_addr;       /* invalid addr: 0x0 */
	u64 cq0_addr;
	u64 cq1_addr;
	u64 cq2_addr;
	u64 cq3_addr;
	u16 sq_index;       /* invalid index: 0xFFFF */
	u16 cq0_index;       /* sq's return */
	u16 cq1_index;       /* ts's return */
	u16 cq2_index;       /* ai cpu's return */
	u16 cq3_index;       /* reserved */
	u16 stream_id;       /* binding stream id for sq and cq (for calculation task) */
	u8 plat_type;       /* inform TS, msg is sent from host or device, device: 0 host: 1 */
	u8 cq_slot_size;    /* calculation cq's slot size, default: 12 bytes */
	u16 group_id;
};

struct devdrv_dfx_int_context {
	int irq_num;
	struct workqueue_struct *wq;
	spinlock_t spinlock;
	struct list_head int_list_header;
};

struct devdrv_dfx_cq_info {
	u32 devid;
	u32 index;
	u32 depth;
	u32 slot_len;
	u32 type;
	struct work_struct work;
	void *dfx_cqsq_ptr;  // pointer struct devdrv_dfx_cqsq
	u8 *addr;
	phys_addr_t phy_addr;
	u32 head;
	u32 tail;
	u32 phase;
	u32 *doorbell;
	struct list_head int_list_node;
	void (*callback) (const u8 *cq_slot, const u8 *sq_slot);
	enum dfx_cqsq_func_type function;
	struct mutex lock;
};

struct devdrv_dfx_cqsq {
	struct devdrv_dfx_sq_info *sq_info;
	struct devdrv_dfx_cq_info *cq_info;
	u32 sq_num;
	u32 cq_num;
	struct mutex dfx_mutex;
	struct devdrv_dfx_int_context int_context;
};

int devdrv_create_dfx_cq(struct devdrv_dev_ctx *cur_dev_ctx, struct devdrv_dfx_create_cq_para *cq_para);

void devdrv_destroy_dfx_cq(struct devdrv_dev_ctx *cur_dev_ctx, u32 cq_index);

int devdrv_dfx_send_mailbox(const struct devdrv_dev_ctx *cur_dev_ctx, struct devdrv_mailbox_cqsq *cqsq);

int devdrv_get_core_num(struct devdrv_dev_ctx *cur_dev_ctx, u32 *ai_cpu_num, u32 *ai_core_num);

int devdrv_dfx_cqsq_init(struct devdrv_dev_ctx *cur_dev_ctx);

void devdrv_destroy_dfx_cqsq(struct devdrv_dev_ctx *cur_dev_ctx);

void devdrv_destroy_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx, u32 sq_index);

struct devdrv_dfx_cqsq *devdrv_get_dfx_cqsq_info(struct devdrv_dev_ctx *cur_dev_ctx);

#endif
