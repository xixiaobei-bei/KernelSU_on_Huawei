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
#include <linux/file.h>
#include <linux/sync_file.h>
#include <linux/fs.h>

#include "dkmd_fence_utils.h"
#include "dkmd_utils.h"

int dkmd_fence_get_fence_fd(struct dma_fence *fence)
{
	int fd;
	struct sync_file *sync_file = NULL;

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		dpu_pr_err("fail to get unused fd\n");
		return fd;
	}

	sync_file = sync_file_create(fence);
	if (!sync_file) {
		put_unused_fd(fd);
		dpu_pr_err("failed to create sync file\n");
		return -ENOMEM;
	}

	fd_install(fd, sync_file->file);

	return fd;
}
