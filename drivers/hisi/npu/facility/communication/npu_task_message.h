/*
 * npu_task_message.h
 *
 * about npu task message
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 */

#ifndef __NPU_TASK_MESSAGE_H
#define __NPU_TASK_MESSAGE_H

#include "npu_proc_ctx.h"
#include "npu_common.h"

#define TS_TASK_BUFF_SIZE 1500

void npu_set_report_timeout(
	struct npu_dev_ctx *dev_ctx, int *timeout_out);
void npu_exception_timeout(
	struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx);

int npu_task_set_init(u8 dev_ctx_id);

int npu_task_set_destroy(u8 dev_ctx_id);

int npu_task_set_insert(
	struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id);

int npu_task_set_remove(
	struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id);

int npu_task_set_clear(struct npu_dev_ctx *dev_ctx);


#endif