/*
 * npu_message.h
 *
 * about npu sq/cq msg
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

#ifndef __NPU_MESSAGE_H
#define __NPU_MESSAGE_H

#include "npu_proc_ctx.h"
#include "npu_rt_task.h"
#include "npu_comm_sqe_fmt.h"

int npu_send_request(struct npu_proc_ctx *proc_ctx,
	npu_rt_task_t *comm_task);
int npu_receive_response(struct npu_proc_ctx *proc_ctx,
	struct npu_receive_response_info *report_info);

#endif
