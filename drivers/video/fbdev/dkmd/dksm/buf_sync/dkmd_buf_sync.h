/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef DKMD_BUF_SYNC_H
#define DKMD_BUF_SYNC_H

#include <linux/list.h>
#include <linux/types.h>

#include "dkmd_timeline.h"

struct dkmd_dma_buf {
	struct dma_buf *buf_handle;
	int32_t shared_fd;
};

int dkmd_buf_sync_lock_dma_buf(struct dkmd_timeline *timeline, struct dkmd_dma_buf *layer_dma_buf, int shared_fd);

#endif