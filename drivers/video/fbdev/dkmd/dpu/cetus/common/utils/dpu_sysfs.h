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
#ifndef DPU_SYSFS_H
#define DPU_SYSFS_H

#include <linux/platform_device.h>
#include <linux/device.h>

#define DPU_SYSFS_ATTRS_NUM 64

struct dpu_attr {
	int sysfs_index;
	struct attribute *sysfs_attrs[DPU_SYSFS_ATTRS_NUM];
	struct attribute_group sysfs_attr_group;
};


void dpu_sysfs_init(struct dpu_attr *attrs);
void dpu_sysfs_attrs_append(struct dpu_attr *attrs, struct attribute *attr);
int dpu_sysfs_create(struct platform_device *pdev, struct dpu_attr *attrs);
void dpu_sysfs_remove(struct platform_device *pdev, struct dpu_attr *attrs);

#endif