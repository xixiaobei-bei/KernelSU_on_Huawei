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
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>

#include "res_mgr.h"
#include "dpu_utils.h"
#include "dpu_config.h"
#include "opr_mgr.h"
#include "dvfs.h"

#define DPU_RM_DEV_NAME "dpu_rm"

static struct dpu_rm *g_rm_dev;

static ssize_t dpu_rm_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t dpu_rm_debug_store(struct device *device,
			struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static struct device_attribute rm_attrs[] = {
	__ATTR(rm_debug, S_IRUGO | S_IWUSR, dpu_rm_debug_show, dpu_rm_debug_store),

	/* TODO: other attrs */
};

static int dpu_rm_open(struct inode *inode, struct file *filp)
{
	struct dpu_rm *rm_dev = g_rm_dev;

	dpu_pr_info("rm_dev: 0x%p", rm_dev);

	if (!rm_dev)
		return -1;

	if (atomic_read(&rm_dev->ref_cnt) > 0)
		return 0;

	filp->private_data = g_rm_dev;
	atomic_inc(&rm_dev->ref_cnt);
	return 0;
}

static int dpu_rm_release(struct inode *inode, struct file *filp)
{
	struct dpu_rm *rm_dev = NULL;

	rm_dev = filp->private_data;
	if (!rm_dev) {
		dpu_pr_err("rm_dev is null");
		return -1;
	}

	if (atomic_read(&rm_dev->ref_cnt) == 0)
		return 0;

	if (!atomic_sub_and_test(1, &rm_dev->ref_cnt))
		return 0;

	filp->private_data = NULL;
	return 0;
}

static int rm_check_ioctl(const struct dpu_rm *rm_dev, unsigned int cmd, void __user *argp)
{
	if (_IOC_TYPE(cmd) != RES_IOCTL_MAGIC) {
		dpu_pr_err("cmd magic is err");
		return -EINVAL;
	}

	if (_IOC_NR(cmd) > RES_IOCTL_CMD_MAX) {
		dpu_pr_err("cmd max  is err");
		return -EINVAL;
	}

	if (!argp || !rm_dev) {
		dpu_pr_err("argp or rm_dev is null");
		return -EINVAL;
	}

	return 0;
}
static long dpu_rm_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct dpu_rm *rm_dev = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct dpu_rm_resouce_node *_node_ = NULL;
	struct dpu_rm_resouce_node *res_node = NULL;

	rm_dev = f->private_data;
	if (rm_check_ioctl(rm_dev, cmd, argp) != 0)
		return -EINVAL;

	list_for_each_entry_safe(res_node, _node_, &rm_dev->resource_list, list_node) {
		if (!res_node)
			continue;

		if (!res_node->ioctl)
			continue;

		if (res_node->ioctl(res_node->data, cmd, argp) == 0)
			return 0;
	}

	return -EINVAL;
}

static struct file_operations dpu_rm_fops = {
	.owner = THIS_MODULE,
	.open = dpu_rm_open,
	.release = dpu_rm_release,
	.unlocked_ioctl = dpu_rm_ioctl,
	.compat_ioctl =  dpu_rm_ioctl,
};

static void dpu_rm_create_chrdev(struct dpu_rm *rm_dev)
{
	/* init chrdev info */
	rm_dev->rm_chrdev.name = DPU_RM_DEV_NAME;
	rm_dev->rm_chrdev.id = 1;
	rm_dev->rm_chrdev.chrdev_fops = &dpu_rm_fops;

	dpu_create_chrdev(&rm_dev->rm_chrdev, rm_dev);
	dpu_create_attrs(rm_dev->rm_chrdev.dpu_cdevice, rm_attrs, ARRAY_SIZE(rm_attrs));
}

static void dpu_rm_init_data(struct dpu_rm_data *data)
{
	data->lbuf_size = dpu_config_get_lbuf_size();
	if (data->lbuf_size == 0)
		dpu_pr_err("get lbuf size fail, is 0");

	// todo: get other data
}

static void dpu_rm_init_resource_list(struct list_head *res_head, struct dpu_rm_data *rm_data)
{
	struct dpu_rm_resouce_node *_node_ = NULL;
	struct dpu_rm_resouce_node *res_node = NULL;

	INIT_LIST_HEAD(res_head);

	dpu_rm_register_opr_mgr(res_head);
	dpu_rm_register_dvfs(res_head);

	list_for_each_entry_safe(res_node, _node_, res_head, list_node) {
		if (!res_node)
			continue;

		if (res_node->init) {
			res_node->data = res_node->init(rm_data);
			if (!res_node->data)
				dpu_pr_err("init resource node data fail");
		}
	}
}

static void dpu_rm_deinit_resouce_list(struct list_head *res_head)
{
	struct dpu_rm_resouce_node *_node_ = NULL;
	struct dpu_rm_resouce_node *res_node = NULL;

	list_for_each_entry_safe(res_node, _node_, res_head, list_node) {
		if (!res_node)
			continue;

		list_del(&res_node->list_node);
		if (res_node->deinit)
			res_node->deinit(res_node->data);

		kfree(res_node);
		res_node = NULL;
	}
}

static int dpu_rm_probe(struct platform_device *pdev)
{
	struct dpu_rm *rm_dev = NULL;
	int ret;

	dpu_pr_info("resource manager enter ++");

	/* 1, read config from dtsi */
	ret = dpu_init_config(pdev);
	if (ret) {
		dpu_pr_err("init dpu config fail");
		return -1;
	}

	rm_dev = devm_kzalloc(&pdev->dev, sizeof(*rm_dev), GFP_KERNEL);
	if (!rm_dev) {
		dpu_pr_err("alloc rm device data fail");
		return -1;
	}

	dpu_pr_info("rm_dev: 0x%p", rm_dev);

	dpu_rm_init_data(&rm_dev->data);
	dpu_rm_init_resource_list(&rm_dev->resource_list, &rm_dev->data);

	atomic_set(&(rm_dev->ref_cnt), 0);
	/* create chrdev */
	dpu_rm_create_chrdev(rm_dev);

	dev_set_drvdata(&pdev->dev, rm_dev);
	g_rm_dev = rm_dev;

	dpu_pr_info("resource manager enter --");
	return 0;
}

static int dpu_rm_remove(struct platform_device *pdev)
{
	/* TODO: remove device */
	struct dpu_rm *rm_dev = NULL;

	rm_dev = platform_get_drvdata(pdev);
	if (!rm_dev)
		return -1;

	dpu_rm_deinit_resouce_list(&rm_dev->resource_list);
	dpu_destroy_chrdev(&rm_dev->rm_chrdev);
	g_rm_dev = NULL;

	return 0;
}

#define DTS_COMP_DISP_RM_NAME "hisilicon,disp_res_mgr"

static const struct of_device_id dpu_rm_match_table[] = {
	{
		.compatible = DTS_COMP_DISP_RM_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, dpu_rm_match_table);

static struct platform_driver g_dpu_rm_driver = {
	.probe = dpu_rm_probe,
	.remove = dpu_rm_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DPU_RM_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dpu_rm_match_table),
	},
};

static int dpu_rm_init(void)
{
	int ret;

	dpu_pr_info("enter +++++++ \n");

	ret = platform_driver_register(&g_dpu_rm_driver);
	if (ret) {
		dpu_pr_info("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	dpu_pr_info("enter -------- \n");
	return ret;
}


module_init(dpu_rm_init);

MODULE_DESCRIPTION("Display Resource Manager Driver");
MODULE_LICENSE("GPL v2");

