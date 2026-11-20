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
#include <linux/device.h>
#include <linux/module.h>
#include "dpu_chrdev.h"
#include "dpu_utils.h"

static struct class *g_dpu_cdev_class;
static atomic_t g_dpu_cdev_ref = ATOMIC_INIT(0);
static dev_t g_dpu_devno;

#define DPU_CHR_DEV_CLASS_NAME    "display_dpu"

static struct class *dpu_get_chrdev_class(void)
{
	if (!g_dpu_cdev_class) {
		g_dpu_cdev_class = class_create(THIS_MODULE, DPU_CHR_DEV_CLASS_NAME);
		if (!g_dpu_cdev_class)
			return NULL;
	}

	return g_dpu_cdev_class;
}

static dev_t dpu_get_devno(void)
{
	int ret;

	if (!g_dpu_devno) {
		ret = alloc_chrdev_region(&g_dpu_devno, 0, 1, "dpu_chrdev");
		if (ret) {
			dpu_pr_err("get chrdev devno error");
			return g_dpu_devno;
		}
	}

	return MKDEV(MAJOR(g_dpu_devno), atomic_read(&g_dpu_cdev_ref));
}

void dpu_create_chrdev(struct dpu_chrdev *chrdev, void *dev_data)
{
	struct class *dpu_cdev_class = NULL;
	int ret;

	dpu_cdev_class = dpu_get_chrdev_class();
	if (!dpu_cdev_class)
		return;

	chrdev->devno = dpu_get_devno();
	dpu_pr_info("start major=%d, minor=%d", MAJOR(chrdev->devno), MINOR(chrdev->devno));

	chrdev->dpu_cdevice = device_create(dpu_cdev_class, NULL, chrdev->devno, dev_data, "%s", chrdev->name);
	if (!chrdev->dpu_cdevice) {
		dpu_pr_err("create device fail, major=%d, minor=%d", MAJOR(chrdev->devno), MINOR(chrdev->devno));
		return;
	}

	cdev_init(&chrdev->cdev, chrdev->chrdev_fops);
	chrdev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&chrdev->cdev, chrdev->devno, 1);
	if (ret) {
		dpu_pr_err("cdev_add fail, major=%d, minor=%d", MAJOR(chrdev->devno), MINOR(chrdev->devno));
		goto cdev_add_fail;
	}

	chrdev->dev_class = dpu_cdev_class;
	atomic_inc(&g_dpu_cdev_ref);

	dpu_pr_info("end major=%d, minor=%d, name=%s, id=%d",
			MAJOR(chrdev->devno), MINOR(chrdev->devno), chrdev->name, chrdev->id);
	return;

cdev_add_fail:
	device_destroy(chrdev->dev_class, chrdev->devno);
}

void dpu_destroy_chrdev(struct dpu_chrdev *chrdev)
{
	if (!chrdev->dev_class)
		return;

	cdev_del(&chrdev->cdev);
	device_destroy(chrdev->dev_class, chrdev->devno);
	unregister_chrdev_region(chrdev->devno, 1);

	if (atomic_dec_return(&g_dpu_cdev_ref) == 0) {
		class_destroy(g_dpu_cdev_class);
		chrdev->dev_class = NULL;
		g_dpu_cdev_class = NULL;
	}

	chrdev->dpu_cdevice = NULL;
	chrdev->chrdev_fops = NULL;
}

void dpu_create_attrs(struct device *dpu_cdevice, struct device_attribute *device_attrs, uint32_t len)
{
	uint32_t i;
	int32_t error = 0;

	for (i = 0; i < len; i++) {
		error = device_create_file(dpu_cdevice, &device_attrs[i]);
		if (error)
			break;
	}

	if (error) {
		while (--i >= 0)
			device_remove_file(dpu_cdevice, &device_attrs[i]);
	}
}

void dpu_cleanup_attrs(struct device *dpu_cdevice, struct device_attribute *device_attrs, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		device_remove_file(dpu_cdevice, &device_attrs[i]);
}

MODULE_LICENSE("GPL");