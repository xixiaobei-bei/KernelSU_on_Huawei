/*
 * power_firmware.c
 *
 * firmware for power module
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/types.h>
#include <chipset_common/hwpower/common_module/power_firmware.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_fw
HWLOG_REGIST();

static int g_power_fw_enable;
static struct list_head g_power_fw_list;
static struct proc_dir_entry *g_power_fw_dir;

static int power_fw_template_read(struct seq_file *s, void *d)
{
	struct power_fw_attr *pattr = s->private;
	char *buf = NULL;
	int ret;

	if (!g_power_fw_enable)
		return -EINVAL;

	if (!pattr || !pattr->read) {
		hwlog_err("invalid read\n");
		return -EINVAL;
	}

	buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = pattr->read(pattr->dev_data, buf, PAGE_SIZE);
	seq_printf(s, "%s", buf);
	kfree(buf);

	hwlog_info("read: name=%s ret=%d\n", pattr->name, ret);
	return 0;
}

static ssize_t power_fw_template_write(struct file *file,
	const char __user *data, size_t size, loff_t *ppos)
{
	struct power_fw_attr *pattr = NULL;
	char *buf = NULL;
	int ret;

	if (!g_power_fw_enable)
		return -EINVAL;

	pattr = ((struct seq_file *)file->private_data)->private;
	if (!pattr || !pattr->write) {
		hwlog_err("invalid write\n");
		return -EINVAL;
	}

	buf = kzalloc(PAGE_SIZE * POWER_FW_MAX_BLOCKS, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (size >= PAGE_SIZE * POWER_FW_MAX_BLOCKS) {
		hwlog_err("input too long\n");
		kfree(buf);
		return -ENOMEM;
	}

	if (copy_from_user(buf, data, size)) {
		hwlog_err("can not copy data form user to kernel\n");
		kfree(buf);
		return -ENOSPC;
	}
	buf[size] = '\0';

	ret = pattr->write(pattr->dev_data, buf, size);
	kfree(buf);

	hwlog_info("write: name=%s ret=%d\n", pattr->name, ret);
	return ret;
}

static int power_fw_template_open(struct inode *inode, struct file *file)
{
	return single_open(file, power_fw_template_read, PDE_DATA(inode));
}

static const struct file_operations power_fw_template_fops = {
	.open = power_fw_template_open,
	.read = seq_read,
	.write = power_fw_template_write,
	.release = single_release,
};

void power_fw_ops_register(char *name, void *dev_data,
	power_fw_read read, power_fw_write write)
{
	struct power_fw_attr *new_attr = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return;
	}

	if (!g_power_fw_list.next) {
		hwlog_info("list init\n");
		INIT_LIST_HEAD(&g_power_fw_list);
	}

	new_attr = kzalloc(sizeof(*new_attr), GFP_KERNEL);
	if (!new_attr)
		return;

	strncpy(new_attr->name, name, POWER_FW_ITEM_NAME_MAX - 1);
	new_attr->name[POWER_FW_ITEM_NAME_MAX - 1] = '\0';
	new_attr->dev_data = dev_data;
	new_attr->read = read;
	new_attr->write = write;
	list_add(&new_attr->list, &g_power_fw_list);

	hwlog_info("%s ops register ok\n", name);
}

static int power_fw_set_enable(unsigned int val)
{
	if (!g_power_fw_dir)
		return -1;

	g_power_fw_enable = val;
	return 0;
}

static int power_fw_get_enable(unsigned int *val)
{
	if (!val)
		return -1;

	*val = g_power_fw_enable;
	return 0;
}

static struct power_if_ops power_fw_if_ops = {
	.set_enable = power_fw_set_enable,
	.get_enable = power_fw_get_enable,
	.type_name = "power_firmware",
};

static int __init power_fw_init(void)
{
	int ret;
	struct proc_dir_entry *file = NULL;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct power_fw_attr *pattr = NULL;

	if (!g_power_fw_list.next) {
		hwlog_info("list init\n");
		INIT_LIST_HEAD(&g_power_fw_list);
	}

	g_power_fw_dir = proc_mkdir_mode("power_fw", POWER_FW_DIR_PERMS, NULL);
	if (IS_ERR_OR_NULL(g_power_fw_dir)) {
		hwlog_err("node create failed\n");
		ret = -EINVAL;
		goto fail_create_dir;
	}

	list_for_each(pos, &g_power_fw_list) {
		pattr = list_entry(pos, struct power_fw_attr, list);
		file = proc_create_data(pattr->name, POWER_FW_FILE_PERMS,
			g_power_fw_dir, &power_fw_template_fops, pattr);
		if (!file) {
			hwlog_err("%s register fail\n", pattr->name);
			ret = -ENOMEM;
			goto fail_create_file;
		}

		hwlog_info("%s register ok\n", pattr->name);
	}

	power_if_ops_register(&power_fw_if_ops);
	return 0;

fail_create_file:
	remove_proc_entry("power_fw", NULL);
	g_power_fw_dir = NULL;
fail_create_dir:
	list_for_each_safe(pos, next, &g_power_fw_list) {
		pattr = list_entry(pos, struct power_fw_attr, list);
		list_del(&pattr->list);
		kfree(pattr);
	}
	INIT_LIST_HEAD(&g_power_fw_list);

	return ret;
}

static void __exit power_fw_exit(void)
{
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct power_fw_attr *pattr = NULL;

	remove_proc_entry("power_fw", NULL);
	g_power_fw_dir = NULL;

	list_for_each_safe(pos, next, &g_power_fw_list) {
		pattr = list_entry(pos, struct power_fw_attr, list);
		list_del(&pattr->list);
		kfree(pattr);
	}
	INIT_LIST_HEAD(&g_power_fw_list);
}

late_initcall_sync(power_fw_init);
module_exit(power_fw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("firmware for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
