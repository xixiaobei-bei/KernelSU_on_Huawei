/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_fb.h"
#include "hisi_fb_sysfs.h"


/*******************************************************************************
 * fb sys fs
 */
void dpufb_sysfs_init(struct dpu_fb_data_type *dpufd)
{
	int i;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	dpufd->sysfs_index = 0;
	for (i = 0; i < HISI_FB_SYSFS_ATTRS_NUM; i++)
		dpufd->sysfs_attrs[i] = NULL;

	dpufd->sysfs_attr_group.attrs = dpufd->sysfs_attrs;
}

void dpufb_sysfs_attrs_append(struct dpu_fb_data_type *dpufd, struct attribute *attr)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	dpu_check_and_no_retval(!attr, ERR, "attr is NULL\n");

	if (dpufd->sysfs_index >= HISI_FB_SYSFS_ATTRS_NUM) {
		DPU_FB_ERR("fb%d, sysfs_atts_num %d is out of range %d!\n",
			dpufd->index, dpufd->sysfs_index, HISI_FB_SYSFS_ATTRS_NUM);
		return;
	}

	dpufd->sysfs_attrs[dpufd->sysfs_index] = attr;
	dpufd->sysfs_index++;
}

int dpufb_sysfs_create(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	ret = sysfs_create_group(&dpufd->fbi->dev->kobj, &(dpufd->sysfs_attr_group));
	if (ret)
		DPU_FB_ERR("fb%d sysfs group creation failed, error=%d!\n",
			dpufd->index, ret);


	return ret;
}

void dpufb_sysfs_remove(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval(!pdev, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	sysfs_remove_group(&dpufd->fbi->dev->kobj, &(dpufd->sysfs_attr_group));


	dpufb_sysfs_init(dpufd);
}
