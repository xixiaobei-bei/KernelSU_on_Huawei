/**
 * @file cmdlist_dev.c
 * @brief Provide equipment related interfaces
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/uaccess.h>
#include <linux/syscalls.h>

#include "cmdlist_dev.h"
#include "cmdlist_drv.h"
#include "dkmd_cmdlist.h"
#include "cmdlist_priv.h"

struct cmdlist_ioctl_data {
	uint32_t ioctl_cmd;
	int (* ioctl_func)(struct cmdlist_node_client *);
};

static struct cmdlist_ioctl_data g_ioctl_data[] = {
	{ CMDLIST_CREATE_CLIENT, cmdlist_create_client },
	{ CMDLIST_SIGNAL_CLIENT, cmdlist_signal_client },
	{ CMDLIST_LINK_NEXT_CLIENT, cmdlist_link_next_client },
	{ CMDLIST_APPEND_NEXT_CLIENT, cmdlist_append_next_client },
	{ CMDLIST_DUMP_USER_CLIENT, cmdlist_dump_user_client },
	{ CMDLIST_DUMP_SCENE_CLIENT, cmdlist_dump_scene_client },
};

static int cmdlist_open(struct inode *inode, struct file *file)
{
	struct cmdlist_private *priv = &g_cmdlist_priv;

	if (!file) {
		pr_err("fail to open!\n");
		return -EINVAL;
	}

	file->private_data = NULL;
	if (priv->device_initialized == 0) {
		pr_err("device is not initail!\n");
		return -EINVAL;
	}
	file->private_data = priv;

	return cmdlist_dev_open(g_cmdlist_priv.scene_num);
}

static int cmdlist_close(struct inode *inode, struct file *file)
{
	struct cmdlist_private *priv = NULL;

	if (!file) {
		pr_err("fail to open!\n");
		return -EINVAL;
	}

	priv = (struct cmdlist_private *)file->private_data;
	if (!priv) {
		pr_err("priv is null!\n");
		return -EINVAL;
	}

	return cmdlist_dev_close();
}

static int cmdlist_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct cmdlist_private *priv = NULL;

	if (!file) {
		pr_err("fail to open!\n");
		return -EINVAL;
	}

	priv = (struct cmdlist_private *)file->private_data;
	if (!priv) {
		pr_err("priv is null!\n");
		return -EINVAL;
	}

	return cmdlist_dev_mmap(vma);
}

static int get_cmdlist_device_info(struct file *file, unsigned long arg)
{
	struct cmdlist_info info = {0};
	struct cmdlist_private *priv = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (!file) {
		pr_err("fail to open!\n");
		return -EINVAL;
	}

	priv = (struct cmdlist_private *)file->private_data;
	if (!priv) {
		pr_err("priv is null!\n");
		return -EINVAL;
	}

	info.pool_size = priv->sum_pool_size;
	info.viraddr_base = (uint64_t)(uintptr_t)priv->pool_vir_addr;
	if (copy_to_user(argp, &info, sizeof(info))) {
		pr_err("copy_to_user failed!\n");
		return -EFAULT;
	}

	pr_info("pool_size=%d, viraddr_base=0x%x\n", info.pool_size, info.viraddr_base);

	return 0;
}

static int cmdlist_ioctl_hander(unsigned int cmd, unsigned long arg)
{
	int i = 0;
	int ret = -1;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct cmdlist_node_client user_client;

	if (!argp) {
		pr_err("argp is null\n");
		return -EINVAL;
	}

	ret = copy_from_user(&user_client, argp, sizeof(user_client));
	if (ret) {
		pr_err("copy for user failed! ret=%d\n", ret);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(g_ioctl_data); i++) {
		if (cmd == g_ioctl_data[i].ioctl_cmd) {
			ret = g_ioctl_data[i].ioctl_func(&user_client);
			break;
		}
	}

	if (copy_to_user((struct cmdlist_node_client __user *)argp, &user_client, sizeof(user_client))) {
		if (user_client.fence_fd > 0)
			sys_close(user_client.fence_fd);
		pr_err("copy_to_user failed!\n");
		ret = -EFAULT;
	}

	return ret;
}

static long cmdlist_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (!argp) {
		pr_err("argp is null\n");
		return -EINVAL;
	}

	switch (cmd) {
	case CMDLIST_INFO_GET:
		ret = get_cmdlist_device_info(file, arg);
		break;
	case CMDLIST_DUMP_ALL_CLIENT:
		cmdlist_dump_all_client();
		break;
	default:
		ret = cmdlist_ioctl_hander(cmd, arg);
	}

	return ret;
}

static const struct file_operations cmdlist_fops = {
	.owner = THIS_MODULE,
	.mmap = cmdlist_mmap,
	.open = cmdlist_open,
	.release = cmdlist_close,
	.unlocked_ioctl = cmdlist_ioctl,
};

int cmdlist_device_setup(struct cmdlist_private *priv)
{
	int ret = 0;

	priv->chr_major = register_chrdev(0, DEV_NAME_CMDLIST, &cmdlist_fops);
	if (priv->chr_major < 0 ) {
		pr_err("error %d registering cmdlist chrdev!\n", ret);
		return -ENXIO;
	}

	priv->chr_class = class_create(THIS_MODULE, "cmdlist");
	if (IS_ERR_OR_NULL(priv->chr_class)) {
		pr_err("create cmdlist class fail!\n");
		ret = PTR_ERR(priv->chr_class);
		goto err_chr_class;
	}

	priv->chr_dev = device_create(priv->chr_class, 0, MKDEV(priv->chr_major, 0), NULL, DEV_NAME_CMDLIST);
	if (IS_ERR_OR_NULL(priv->chr_dev)) {
		pr_err("create cmdlist char device fail!\n");
		ret = PTR_ERR(priv->chr_dev);
		goto err_chr_device;
	}
	dev_set_drvdata(priv->chr_dev, priv);

	pr_info("add cmdlist chr_dev device succ!\n");
	return ret;

err_chr_device:
	class_destroy(priv->chr_class);
err_chr_class:
	unregister_chrdev(priv->chr_major, DEV_NAME_CMDLIST);
	return ret;
}
