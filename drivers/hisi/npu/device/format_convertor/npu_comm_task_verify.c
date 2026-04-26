/*
 * npu_comm_task_verify.c
 *
 * about npu communication task verify
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
#include "npu_comm_task_verify.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <securec.h>

#include "npu_common.h"
#include "npu_log.h"
#include "npu_user_common.h"


typedef int (*verify_ts_task_func)(npu_rt_task_t *ts_task);

int verify_nonsink_task_comm_field(npu_rt_task_t *ts_task)
{
	if (ts_task->type >= NPU_TASK_RESERVED) {
		npu_drv_err("invalid task type:%u, task_id:%u\n",
			ts_task->type, ts_task->task_id);
		return -1;
	}
	if (ts_task->stream_id > NPU_MAX_NON_SINK_STREAM_ID) {
		npu_drv_err("invalid task stream_id:%u, task_id:%u\n",
			ts_task->stream_id, ts_task->task_id);
		return -1;
	}
	if (ts_task->task_id < NPU_MAX_TASK_START_ID ||
		ts_task->task_id > UINT16_MAX) {
		npu_drv_err("invalid task_id:%u\n", ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_evt_rec_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.event_task.event_id >= NPU_MAX_EVENT_ID) {
		npu_drv_err("invalid event_id:%u, task_id:%u\n",
			ts_task->u.event_task.event_id, ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_wait_evt_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.event_task.event_id >= NPU_MAX_EVENT_ID) {
		npu_drv_err("invalid event_id:%u, task_id:%u\n",
			ts_task->u.event_task.event_id, ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_fusion_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.fusion_task.flag > NPU_FUSION_END) {
		npu_drv_err("invalid fusion_task.flag:%u, task_id:%u\n",
			ts_task->u.fusion_task.flag, ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_memcpy_comm_task(npu_rt_task_t *rt_task)
{
	if (rt_task->u.memcpy_task.src_addr & npu_bit_mask(4) ||
		rt_task->u.memcpy_task.src_addr == 0) {
		npu_drv_err("invalid src_addr not 16B align or is 0, sdma task_id:%u\n",
			rt_task->task_id);
		return -1;
	}
	if (rt_task->u.memcpy_task.dst_addr & npu_bit_mask(4) ||
		rt_task->u.memcpy_task.dst_addr == 0) {
		npu_drv_err("invalid dst_addr not 16B align or is 0, sdma task_id:%u\n",
			rt_task->task_id);
		return -1;
	}
	if (rt_task->u.memcpy_task.length & npu_bit_mask(4) ||
		rt_task->u.memcpy_task.length == 0) {
		npu_drv_err("invalid length:%llu not 16B align or is 0, sdma task_id:%u\n",
			rt_task->u.memcpy_task.length, rt_task->task_id);
		return -1;
	}
	return 0;
}

int verify_maintenance_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.maintenance_task.goal > NPU_MT_EVENT_DESTROY) {
		npu_drv_err("invalid maintenance_task.goal:%u, task_id:%u\n",
			ts_task->u.maintenance_task.goal, ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_create_stream_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.create_stream_task.sq_id >= NPU_MAX_SQ_NUM) {
		npu_drv_err("invalid create_stream_task.sq_id:%u, task_id:%u\n",
			ts_task->u.create_stream_task.sq_id, ts_task->task_id);
		return -1;
	}
	if (ts_task->u.create_stream_task.priority > NPU_MAX_STREAM_PRIORITY) {
		npu_drv_err("invalid create_stream_task.priority:%u, task_id:%u\n",
			ts_task->u.create_stream_task.priority, ts_task->task_id);
		return -1;
	}
	return 0;
}

int verify_model_maintenance_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.model_maintenance_task.model_id >= NPU_MAX_MODEL_ID) {
		npu_drv_err("invalid model_maintenance_task.model_id:%u, task_id:%u\n",
			ts_task->u.model_maintenance_task.model_id, ts_task->task_id);
		return -1;
	}
	if (ts_task->u.model_maintenance_task.operation_type >=
		NPU_MMT_RESERVED) {
		npu_drv_err("invalid model_maintenance_task.operation_type:%u, task_id:%u\n",
			ts_task->u.model_maintenance_task.operation_type, ts_task->task_id);
		return -1;
	}
	/* no need to verify ts_task->u.model_maintenance_task.first_task_id,
	 * since it's not used after the first edition
	 */
	return 0;
}

int verify_model_execute_comm_task(npu_rt_task_t *ts_task)
{
	if (ts_task->u.model_execute_task.model_id >= NPU_MAX_MODEL_ID) {
		npu_drv_err("invalid model_execute_task.model_id:%u, task_id:%u\n",
			ts_task->u.model_execute_task.model_id, ts_task->task_id);
		return -1;
	}
	// verify pid
	if (ts_task->u.model_execute_task.pid == 0)
		npu_drv_err("invalid model_execute_task.pid:%u, task_id:%u\n",
			ts_task->u.model_execute_task.pid, ts_task->task_id);

	/* no need to verify ts_task->u.model_maintenance_task.first_task_id,
	 * since it's not used after the first edition
	 */
	return 0;
}

int verify_profiling_enable_comm_task(npu_rt_task_t *ts_task)
{
	return 0;
}

int verify_profiling_disable_comm_task(npu_rt_task_t *ts_task)
{
	return 0;
}

int verify_bypass_comm_task(npu_rt_task_t *ts_task)
{
	return 0;
}

verify_ts_task_func verify_ts_sqe_map[] = {
	/* 0  1                  2                       3 */
	NULL, NULL,              verify_evt_rec_comm_task,     verify_wait_evt_comm_task,
	/* 4  5                  6                       7 */
	NULL, verify_memcpy_comm_task, verify_maintenance_comm_task, verify_create_stream_comm_task,
	/* 8                          9                         10    11 */
	NULL,                         NULL,                     NULL, NULL,
	/* 12                         13                        14    15 */
	verify_model_maintenance_comm_task, verify_model_execute_comm_task, NULL, NULL,
	/* 16                         17                        18    19 */
	NULL,                         NULL,                     NULL, NULL,
	/* 20                         21          22    23 */
	NULL,                         NULL,       NULL, verify_bypass_comm_task,
	/* 24                         25                            26    27 */
	verify_bypass_comm_task,          NULL,                         NULL, NULL,
	/* 28                         29                            30    31 */
	NULL,                         NULL,                         NULL, NULL,
	/* 32                         33                            34    35 */
	NULL,                         NULL,                         NULL, NULL,
	/* 36                         37                            38    39 */
	NULL,                         NULL,                         NULL, NULL,
	/* 40                         41                            42    43 */
	NULL,                         NULL,                         NULL, NULL,
	/* 44                         45                            46    47 */
	NULL,                         NULL,                         NULL, NULL,
	/* 48                         49                            50    51 */
	NULL,                         NULL,                         NULL, NULL,
	/* 52                         53                            54    55 */
	NULL,                         NULL,                         NULL, NULL,
	/* 56                         57                            58    59 */
	NULL,                         NULL,                         NULL, NULL,
	/* 60                         61                            62    63 */
	NULL,                         NULL,                         NULL, NULL,
	/* 64                         65                            66    67 */
	verify_profiling_enable_comm_task,  verify_profiling_disable_comm_task, NULL, NULL,
};

int npu_verify_ts_sqe(void *ts_task)
{
	verify_ts_task_func verify_func = NULL;
	int ret;
	npu_rt_task_t *ts_sqe = (npu_rt_task_t *)ts_task;

	npu_drv_debug("ts_task_addr:%pK", ts_task);
	ret = verify_nonsink_task_comm_field(ts_sqe);
	if (ret != 0) {
		npu_drv_err("verify_sqe_comm_field failed, ret:%d, task_id:%u, type:%u\n",
			ret, ts_sqe->task_id, ts_sqe->type);
		return -1;
	}

	verify_func = verify_ts_sqe_map[ts_sqe->type];
	if (verify_func == NULL) {
		npu_drv_err("invalid task_id:%u, type:%u\n", ts_sqe->task_id,
			ts_sqe->type);
		return -1;
	}
	ret = verify_func(ts_sqe);
	if (ret != 0) {
		npu_drv_err("verify_ts_sqe failed, ret:%d, task_id:%u, type:%u\n",
			ret, ts_sqe->task_id, ts_sqe->type);
		return -1;
	}

	npu_drv_debug("end\n");
	return 0;
}
