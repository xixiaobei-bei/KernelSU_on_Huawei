/*
 * npu_debug_resource_count.h
 *
 * about npu debug resource count
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
#ifndef __NPU_DEBUG_RESOURCE_COUNT_H
#define __NPU_DEBUG_RESOURCE_COUNT_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_npu_debug_init(void);

int list_count(struct list_head *list);

static int hisi_npu_resource_debugfs_show(struct seq_file *s, void *data);

static int hisi_npu_resource_debugfs_open(struct inode *inode, struct file *file);
#endif