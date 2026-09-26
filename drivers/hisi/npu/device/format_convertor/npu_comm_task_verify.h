/*
 * npu_comm_task_verify.h
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
#ifndef _NPU_COMM_TASK_VERIFY_H_
#define _NPU_COMM_TASK_VERIFY_H_

#include <linux/types.h>

#include "npu_rt_task.h"

int npu_verify_ts_sqe(void *ts_task);

#endif
