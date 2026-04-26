/*
 * power_debug.c
 *
 * debug for power module
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
#ifndef __NPU_RECYCLE_H
#define __NPU_RECYCLE_H

#include "npu_mailbox.h"
#include "npu_proc_ctx.h"

bool npu_is_proc_resource_leaks(const struct npu_proc_ctx *proc_ctx);

void npu_resource_leak_print(const struct npu_proc_ctx *proc_ctx);

void npu_recycle_npu_resources(struct npu_proc_ctx *proc_ctx);

#endif
