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

#include <linux/list.h>
#include <linux/slab.h>

#include "dvfs.h"
#include "res_mgr.h"


static void* dvfs_init(struct dpu_rm_data *rm_data)
{
	return NULL;
}

static void dvfs_deinit(void *dvfs)
{

}

static long dvfs_ioctl(void *dvfs, unsigned int cmd, void __user *argp)
{
	return 0;
}

void dpu_rm_register_dvfs(struct list_head *res_head)
{
	struct dpu_rm_resouce_node *dvfs_node = NULL;

	dvfs_node = kzalloc(sizeof(*dvfs_node), GFP_KERNEL);
	if (!dvfs_node)
		return;

	dvfs_node->init = dvfs_init;
	dvfs_node->deinit = dvfs_deinit;
	dvfs_node->ioctl = dvfs_ioctl;

	list_add_tail(&dvfs_node->list_node, res_head);
}