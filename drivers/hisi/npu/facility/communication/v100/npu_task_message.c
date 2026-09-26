/*
 * npu_task_message.c
 *
 * about npu task message
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 */
#include "npu_task_message.h"
#include "npu_log.h"

void npu_set_report_timeout(struct npu_dev_ctx *dev_ctx, int *timeout_out)
{
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(timeout_out == NULL, "timeout_out is null\n");
	unused(dev_ctx);
	unused(timeout_out);
	npu_drv_debug("npu_set_report_timeout timeout_out is %d\n", *timeout_out);
	return;
}

void npu_exception_timeout(
	struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx)
{
	npu_drv_debug("drv receive response wait timeout\n");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(proc_ctx == NULL, "proc_ctx is null\n");

	unused(dev_ctx);
	unused(proc_ctx);
	return;
}

int npu_task_set_init(u8 dev_ctx_id)
{
	unused(dev_ctx_id);
	return 0;
}

int npu_task_set_destroy(u8 dev_ctx_id)
{
	unused(dev_ctx_id);
	return 0;
}

int npu_task_set_insert(struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id)
{
	unused(dev_ctx);
	unused(stream_id);
	unused(task_id);
	return 0;
}

int npu_task_set_remove(struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id)
{
	unused(dev_ctx);
	unused(stream_id);
	unused(task_id);
	return 0;
}

int npu_task_set_clear(struct npu_dev_ctx *dev_ctx)
{
	unused(dev_ctx);
	return 0;
}
