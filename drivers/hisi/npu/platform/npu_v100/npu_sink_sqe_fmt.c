/*
 * npu_sink_sqe_fmt.c
 *
 * about npu sink sqe format
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
#include "npu_sink_sqe_fmt.h"

#include <securec.h>

#include "npu_common.h"
#include "npu_log.h"
#include "npu_sink_task_verify.h"
#include "npu_shm.h"

extern struct npu_mem_info g_shm_desc[NPU_DEV_NUM][NPU_MAX_MEM];
int npu_format_sink_sqe(npu_model_desc_t *model_desc, void *stream_buf_addr,
		u16 *first_task_id, u8 devid, int stream_idx)
{
	int task_idx;
	int ret;

	npu_rt_task_t *task = (npu_rt_task_t *)stream_buf_addr;
	npu_drv_debug("stream_cnt %u, stream_idx = %d, taskid:%u, type:%u\n",
		model_desc->stream_cnt, stream_idx,
		task->task_id, task->type);
	for (task_idx = 0; task_idx < model_desc->stream_tasks[stream_idx]; task_idx++) {
		// 1. set next stream's first task id
		if (task_idx == 0) {
			task[task_idx].next_stream_id = *first_task_id;
			*first_task_id = task[task_idx].task_id;
		}
		// 2. set next task id
		if (task_idx < model_desc->stream_tasks[stream_idx] - 1)
			task[task_idx].next_task_id = task[task_idx + 1].task_id;
		else
			task[task_idx].next_task_id = MAX_UINT16_NUM;

		// 3. cpy task
		ret = memcpy_s(((u8 *)(uintptr_t)g_shm_desc[devid][NPU_PERSISTENT_TASK_BUFF].virt_addr
						+ task[task_idx].task_id * NPU_RT_TASK_SIZE),
					   NPU_RT_TASK_SIZE,
					   &task[task_idx],
					   NPU_RT_TASK_SIZE);
		if (ret != 0) {
			npu_drv_err("memcpy_s fail, task_id= %d, ret= %d\n",
						task[task_idx].task_id, ret);
			return -1;
		}
	}

	return 0;
}