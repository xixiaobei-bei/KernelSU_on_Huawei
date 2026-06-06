#ifndef __NPU_RECYCLE_H
#define __NPU_RECYCLE_H

#include "npu_mailbox.h"
#include "npu_proc_ctx.h"

bool devdrv_is_proc_resource_leaks(struct devdrv_proc_ctx *proc_ctx);

void devdrv_resource_leak_print(struct devdrv_proc_ctx *proc_ctx);

void devdrv_recycle_npu_resources(struct devdrv_proc_ctx *proc_ctx);

#endif
