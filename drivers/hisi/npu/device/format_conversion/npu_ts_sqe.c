/*
 * npu_ts_sqe.c
 *
 * about npu ts sqe
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
#include "npu_ts_sqe.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#include "npu_log.h"
#include "devdrv_user_common.h"

typedef int (*format_func)(void *hwts_sqe_addr, devdrv_ts_task_t *ts_sqe_addr);
#define TS_SQE_ENTRY(ts_sq_addr, offset) ((ts_sq_addr) + (offset) * sizeof(devdrv_ts_task_t))
#define HWTS_SQE_ENTRY(hwts_sq_addr, offset) ((hwts_sq_addr) + (offset) * DEVDRV_HWTS_SQ_SLOT_SIZE)

int format_aicore_sqe(void *hwts_sqe_addr, devdrv_ts_task_t *ts_sqe_addr)
{
	devdrv_hwts_kernel_sqe_t *kernel_sqe = (devdrv_hwts_kernel_sqe_t *)hwts_sqe_addr;
	kernel_sqe->type = DEVDRV_HWTS_SQE_TASK_KICKSTART;
	kernel_sqe->ie = 0;
	kernel_sqe->pre_p = 0;
	kernel_sqe->post_p = 0;
	kernel_sqe->wr_cqe = 0;
	kernel_sqe->rd_cond = 0;
	kernel_sqe->res0 = 0;
	kernel_sqe->l2_lock = 0;
	kernel_sqe->l2_unlock = 0;
	kernel_sqe->blh_dim = ts_sqe_addr->u.kernel_task.block_dim;
	kernel_sqe->stream_id = ts_sqe_addr->stream_id;
	kernel_sqe->task_id = ts_sqe_addr->task_id;
	kernel_sqe->pc_addr_low = (u32)(ts_sqe_addr->u.kernel_task.pc_start);
	kernel_sqe->pc_addr_high = (u16)((ts_sqe_addr->u.kernel_task.pc_start) >> 32);
	kernel_sqe->kernel_credit = 255;
	kernel_sqe->res2 = 0;
	kernel_sqe->prefetch_cnt = 1;
	kernel_sqe->param_addr_low = (u32)(ts_sqe_addr->u.kernel_task.param_base);
	kernel_sqe->param_addr_high = (u16)((ts_sqe_addr->u.kernel_task.param_base) >> 32);
	kernel_sqe->l2_in_main = 0xFF; // dirty_code
	kernel_sqe->res3 = 0;
	kernel_sqe->literal_addr_low = (u32)(ts_sqe_addr->u.kernel_task.literal_src_addr);
	kernel_sqe->literal_addr_high = (u16)(ts_sqe_addr->u.kernel_task.literal_src_addr >> 32);
	kernel_sqe->res4 = 0;
	kernel_sqe->literal_base_ub = ts_sqe_addr->u.kernel_task.literal_dst_base;
	kernel_sqe->res5 = 0;
	kernel_sqe->literal_buff_len = ts_sqe_addr->u.kernel_task.literal_size;
	kernel_sqe->res6 = 0;

	return 0;
}

int format_evt_rec_sqe(void *hwts_sqe_addr, devdrv_ts_task_t *ts_sqe_addr)
{
	devdrv_hwts_event_sqe_t *evt_rec_sqe = (devdrv_hwts_event_sqe_t *)hwts_sqe_addr;

	NPU_DRV_DEBUG("format_evt_rec_sqe enter\n");
	evt_rec_sqe->type = DEVDRV_HWTS_SQE_EVENT_RECORD;
	evt_rec_sqe->ie = 0;
	evt_rec_sqe->pre_p = 0;
	evt_rec_sqe->post_p = 0;
	evt_rec_sqe->wr_cqe = 0;
	evt_rec_sqe->res0 = 0;
	evt_rec_sqe->res1 = 0;
	evt_rec_sqe->stream_id = ts_sqe_addr->stream_id;
	evt_rec_sqe->task_id = ts_sqe_addr->task_id;
	evt_rec_sqe->event_id = ts_sqe_addr->u.event_task.event_id;
	evt_rec_sqe->res2 = 0;
	NPU_DRV_DEBUG("format_evt_rec_sqe end\n");
	return 0;
}

int format_wait_evt_sqe(void *hwts_sqe_addr, devdrv_ts_task_t *ts_sqe_addr)
{
	devdrv_hwts_event_sqe_t *evt_wait_sqe = (devdrv_hwts_event_sqe_t *)hwts_sqe_addr;

	NPU_DRV_DEBUG("format_wait_evt_sqe enter\n");
	evt_wait_sqe->type = DEVDRV_HWTS_SQE_WAIT_EVENT;
	evt_wait_sqe->ie = 0;
	evt_wait_sqe->pre_p = 0;
	evt_wait_sqe->post_p = 0;
	evt_wait_sqe->wr_cqe = 0;
	evt_wait_sqe->res0 = 0;
	evt_wait_sqe->res1 = 0;
	evt_wait_sqe->stream_id = ts_sqe_addr->stream_id;
	evt_wait_sqe->task_id = ts_sqe_addr->task_id;
	evt_wait_sqe->event_id = ts_sqe_addr->u.event_task.event_id;
	evt_wait_sqe->res2 = 0;
	NPU_DRV_DEBUG("format_wait_evt_sqe end\n");
	return 0;
}

int format_ph_sqe(void *hwts_sqe_addr, devdrv_ts_task_t *ts_sqe_addr)
{
	devdrv_hwts_ph_sqe_t *ph_sqe = (devdrv_hwts_ph_sqe_t *)hwts_sqe_addr;

	NPU_DRV_DEBUG("format_ph_sqe enter\n");
	ph_sqe->type = DEVDRV_HWTS_SQE_PLACE_HOLDER;
	ph_sqe->ie = 1;
	ph_sqe->pre_p = 0;
	ph_sqe->post_p = 0;
	ph_sqe->wr_cqe = 0;
	ph_sqe->res0 = 0;
	ph_sqe->res1 = 0;
	ph_sqe->stream_id = ts_sqe_addr->stream_id;
	ph_sqe->task_id   = ts_sqe_addr->task_id;

	NPU_DRV_DEBUG("format_ph_sqe end\n");
	return 0;
}

format_func formate_hwts_sqe_map[] = {
/*	0					1				2					3			*/
	format_aicore_sqe,	format_ph_sqe,	format_evt_rec_sqe,	format_wait_evt_sqe,
/*	4					5				6					7			*/
	format_ph_sqe,		format_ph_sqe,	format_ph_sqe,		format_ph_sqe,
/*	8					9				10					11			*/
	format_ph_sqe,		format_ph_sqe,	format_ph_sqe,		format_ph_sqe,
/*	12					13				14					15			*/
	format_ph_sqe,		format_ph_sqe,	format_ph_sqe,		format_ph_sqe,
/*	16					17				18					19			*/
	format_ph_sqe,		NULL,			format_ph_sqe,		format_ph_sqe,
/*	20					21				22					23			*/
	format_ph_sqe,		NULL,			NULL,				NULL,
/*	24					25				26					27			*/
	NULL,				NULL,			NULL,				NULL,
/*	28					29				30					31			*/
	NULL,				NULL,			NULL,				NULL,
/*	32					33				34					35			*/
	NULL,				NULL,			NULL,				NULL,
/*	36					37				38					39			*/
	NULL,				NULL,			NULL,				NULL,
/*	40					41				42					43			*/
	NULL,				NULL,			NULL,				NULL,
/*	44					45				46					47			*/
	NULL,				NULL,			NULL,				NULL,
/*	48					49				50					51			*/
	NULL,				NULL,			NULL,				NULL,
/*	52					53				54					55			*/
	NULL,				NULL,			NULL,				NULL,
/*	56					57				58					59			*/
	NULL,				NULL,			NULL,				NULL,
/*	60					61				62					63			*/
	NULL,				NULL,			NULL,				NULL,
/*	64					65				66					67			*/
	format_ph_sqe,		format_ph_sqe,	NULL,				NULL,

};

static inline void devdrv_format_head_sqe(devdrv_hwts_sqe_hdr_t *hwts_sqe_hdr)
{
	hwts_sqe_hdr->l2_lock = 1;
	return;
}

static inline void devdrv_format_tail_sqe(devdrv_hwts_sqe_hdr_t *hwts_sqe_hdr)
{
	hwts_sqe_hdr->l2_unlock = 1;
	hwts_sqe_hdr->wr_cqe = 1;
	return;
}


int devdrv_format_hwts_sqe(void *hwts_sq_addr, void *ts_sq_addr, u64 ts_sq_len)
{
	u64 ts_sqe_num = ts_sq_len / sizeof(devdrv_ts_task_t);
	format_func func = NULL;
	u8 *hwts_sq_base = hwts_sq_addr;
	u8 *hwts_sqe = NULL;
	u64 i;
	int ret;
	devdrv_ts_task_t *ts_sqe = NULL;

	if (hwts_sq_addr == NULL) {
		NPU_DRV_ERR("hwts_sq_addr is null\n");
		return -1;
	}
	if (ts_sq_addr == NULL) {
		NPU_DRV_ERR("ts_sq_addr is null\n");
		return -1;
	}

	NPU_DRV_DEBUG("hwts_sq_addr:0x%lx, ts_sq_addr:0x%lx, ts_sq_len:%d, ts_sqe_num:%d",
		hwts_sq_addr, ts_sq_addr, ts_sq_len, ts_sqe_num);
	for (i = 0; i < ts_sqe_num; i++) {
		ts_sqe = (devdrv_ts_task_t *)TS_SQE_ENTRY(ts_sq_addr, i);
		hwts_sqe = HWTS_SQE_ENTRY(hwts_sq_base, i);

		func = formate_hwts_sqe_map[ts_sqe->type];
		if (func == NULL) {
			NPU_DRV_ERR("invalid task:%d, type:%d\n", ts_sqe->task_id, ts_sqe->type);
			func = format_ph_sqe;
		}

		ret = func((void *)hwts_sqe, ts_sqe);
		if (ret != 0)
			NPU_DRV_ERR("formate_hwts_sqe failed, ret:%d, task:%d, type:%d\n",
				ts_sqe->task_id, ts_sqe->type, ret);
	}
	devdrv_format_head_sqe((devdrv_hwts_sqe_hdr_t *)HWTS_SQE_ENTRY(hwts_sq_base, 0));
	devdrv_format_tail_sqe((devdrv_hwts_sqe_hdr_t *)HWTS_SQE_ENTRY(hwts_sq_base, ts_sqe_num - 1));
	NPU_DRV_DEBUG("devdrv_format_hwts_sqe end\n");
	return ts_sqe_num;
}
EXPORT_SYMBOL(devdrv_format_hwts_sqe);

