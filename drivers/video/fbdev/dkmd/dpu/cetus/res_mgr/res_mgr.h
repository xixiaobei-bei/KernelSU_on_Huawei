/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef DKMD_DPU_RES_MGR_H
#define DKMD_DPU_RES_MGR_H

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include "dpu_chrdev.h"

struct dpu_rm_data {
	uint32_t lbuf_size;
};

struct dpu_rm {
	struct platform_device *pdev;
	struct dpu_chrdev rm_chrdev;

	atomic_t ref_cnt;

	struct dpu_rm_data data;

	struct list_head resource_list;
};

/* resource_list node */
struct dpu_rm_resouce_node {
	struct list_head list_node;
	void *data;

	void* (*init)(struct dpu_rm_data *rm_data);
	void (*deinit)(void *data);
	long (*ioctl)(void *data, unsigned int cmd, void __user *argp);
};

#endif /* DKMD_DPU_RES_MGR_H */
