/*
 * npu_comm_sqe_fmt.h
 *
 * about npu communication sqe format
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
#ifndef _NPU_COMM_SQE_FMT_H_
#define _NPU_COMM_SQE_FMT_H_

#include <linux/types.h>

#include "npu_rt_task.h"


/**
 * @ response receive control para
 */
struct npu_receive_response_info {
	u64 response_addr;
	u32 response_count;
	int wait_result;
	int timeout;
};

int npu_format_ts_sqe(void *ts_sqe_addr, npu_rt_task_t *ts_task,
	u32 index);

#endif
