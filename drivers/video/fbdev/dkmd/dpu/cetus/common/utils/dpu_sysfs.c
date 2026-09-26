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

#include "dpu_sysfs.h"
#include "dpu_utils.h"

void dpu_sysfs_init(struct dpu_attr *attrs)
{
	int i;

	dpu_pr_info(" ++++ ");

	attrs->sysfs_index = 0;
	for (i = 0; i < DPU_SYSFS_ATTRS_NUM; i++)
		attrs->sysfs_attrs[i] = NULL;

	attrs->sysfs_attr_group.attrs = attrs->sysfs_attrs;

	dpu_pr_info(" ---- ");
}
EXPORT_SYMBOL(dpu_sysfs_init);

void dpu_sysfs_attrs_append(struct dpu_attr *attrs, struct attribute *attr)
{

	dpu_pr_info(" ++++ ");

	if (attrs->sysfs_index >= DPU_SYSFS_ATTRS_NUM) {
		dpu_pr_err("sysfs_atts_num %d is out of range %d!\n", attrs->sysfs_index, DPU_SYSFS_ATTRS_NUM);
		return;
	}

	attrs->sysfs_attrs[attrs->sysfs_index] = attr;
	attrs->sysfs_index++;

	dpu_pr_info(" ---- ");
}
EXPORT_SYMBOL(dpu_sysfs_attrs_append);

int dpu_sysfs_create(struct platform_device *pdev, struct dpu_attr *attrs)
{
	int ret;

	dpu_pr_info(" ++++ ");

	ret = sysfs_create_group(&pdev->dev.kobj, &(attrs->sysfs_attr_group));
	if (ret)
		dpu_pr_err("sysfs group creation failed, error=%d!\n", ret);

	dpu_pr_info(" ---- ");

	return ret;
}
EXPORT_SYMBOL(dpu_sysfs_create);

void dpu_sysfs_remove(struct platform_device *pdev, struct dpu_attr *attrs)
{
	dpu_pr_info(" ++++ ");

	sysfs_remove_group(&pdev->dev.kobj, &(attrs->sysfs_attr_group));

	dpu_sysfs_init(attrs);

	dpu_pr_info(" ---- ");
}
EXPORT_SYMBOL(dpu_sysfs_remove);