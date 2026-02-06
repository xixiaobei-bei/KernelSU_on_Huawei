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

#ifndef HISI_FB_SYSFS_H
#define HISI_FB_SYSFS_H

/*******************************************************************************
 * fb sys fs
 */
void dpufb_sysfs_init(struct dpu_fb_data_type *dpufd);

void dpufb_sysfs_attrs_append(struct dpu_fb_data_type *dpufd, struct attribute *attr);

int dpufb_sysfs_create(struct platform_device *pdev);

void dpufb_sysfs_remove(struct platform_device *pdev);

#endif /* HISI_FB_SYSFS_H */
