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

#ifndef DPU_CHRDEV_H
#define DPU_CHRDEV_H

#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/cdev.h>

struct dpu_chrdev {
	const char *name;
	int id;
	dev_t devno;
	struct class *dev_class;
	struct device *dpu_cdevice;
	struct cdev cdev;
	struct file_operations *chrdev_fops;
};

void dpu_create_chrdev(struct dpu_chrdev *chrdev, void *dev_data);
void dpu_destroy_chrdev(struct dpu_chrdev *chrdev);
void dpu_create_attrs(struct device *dte_cdevice, struct device_attribute *device_attrs, uint32_t len);
void dpu_cleanup_attrs(struct device *dte_cdevice, struct device_attribute *device_attrs, uint32_t len);


#endif /* DISP_CHRDEV_H */
