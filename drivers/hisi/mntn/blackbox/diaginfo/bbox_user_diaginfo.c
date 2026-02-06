/*
 * bbox_user_diaginfo.c
 *
 * DMD driver for user-mode application
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "bbox_user_diaginfo.h"
#include "bbox_diaginfo.h"
#include <bbox_diaginfo_id_def.h>
#include "bbox_user_diaginfo_data.h"
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <securec.h>

#define USER_DMD_FILE_LIMIT    (S_IWUSR | S_IWGRP)

static int check_user_data(unsigned int err_id)
{
	if (err_id < SoC_DIAGINFO_START || err_id > BBOX_DIAGINFO_END) {
		pr_err("[%s]err_id invalid\n", __func__);
		return -1;
	}

	return 0;
}

static int get_user_msg(unsigned int err_id, char **err_msg)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_user_diaginfo_tbl); i++) {
		if (err_id == g_user_diaginfo_tbl[i].err_id) {
			*err_msg = g_user_diaginfo_tbl[i].err_msg;
			return 0;
		}
	}

	pr_err("err_id[%u] is not registered\n", err_id);
	return -1;
}

static ssize_t user_diaginfo_write(struct file *file, const char __user *userbuf, size_t bytes, loff_t *off)
{
	ssize_t bytes_writen;
	unsigned int err_id;
	int ret;
	unsigned int node_nums;
	char *err_msg = NULL;

	if (bytes > sizeof(err_id)) {
		pr_err("[%s]invalid input\n", __func__);
		return -EPERM;
	}

	bytes_writen = simple_write_to_buffer(&err_id,
			sizeof(err_id),
			off, userbuf, bytes);
	if (bytes_writen <= 0) {
		pr_err("[%s]simple_write_to_buffer error\n", __func__);
		return -EPERM;
	}

	ret = check_user_data(err_id);
	if (ret < 0) {
		pr_err("user data invalid\n");
		return -EPERM;
	}

	node_nums = get_diaginfo_nodes_num();
	if (node_nums >= MAX_USER_NODES_NUM) {
		pr_err("[%s]add to diaginfo list busy\n", __func__);
		return -EPERM;
	}

	ret = get_user_msg(err_id, &err_msg);
	if (ret == 0)
		ret = bbox_diaginfo_record(err_id, NULL, "%s", err_msg);
	if (ret != 0)
		return -EPERM;

	return bytes_writen;
}

static const struct file_operations user_diaginfo_fops = {
	.write = user_diaginfo_write,
};

static int user_diaginfo_create_procfs(void)
{
	struct proc_dir_entry *root = NULL;
	struct proc_dir_entry *user_diaginfo = NULL;

	root = proc_mkdir("user_diaginfo", NULL);
	if (!root) {
		pr_err("[%s]create root dir error\n", __func__);
		return -ENOENT;
	}

	user_diaginfo = proc_create_data("user_diaginfo", USER_DMD_FILE_LIMIT, root, &user_diaginfo_fops, NULL);
	if (!user_diaginfo) {
		pr_err("[%s] user_diaginfo procfs create failed\n", __func__);
		proc_remove(root);
		return -ENOENT;
	}

	return 0;
}

static int __init bbox_user_diaginfo_init(void)
{
	int ret;

	ret = user_diaginfo_create_procfs();
	if (ret < 0) {
		pr_err("[%s]user diaginfo create procfs failed\n", __func__);
		return ret;
	}

	return 0;
}
late_initcall(bbox_user_diaginfo_init);
