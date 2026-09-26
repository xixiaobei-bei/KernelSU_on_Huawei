/*
 * power_ioctl.c
 *
 * ioctl for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/common_module/power_ioctl.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_ioctl
HWLOG_REGIST();

static struct list_head g_power_ioctl_list;
static struct proc_dir_entry *g_power_ioctl_dir;

static long power_ioctl_template_rw(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct power_ioctl_attr *pattr = PDE_DATA(file_inode(file));

	if (!pattr || !pattr->rw) {
		hwlog_err("invalid rw\n");
		return -EINVAL;
	}

	hwlog_info("rw: name=%s cmd=%u\n", pattr->name, cmd);
	return pattr->rw(pattr->dev_data, cmd, arg);
}

#ifdef CONFIG_COMPAT
static const struct file_operations power_ioctl_template_fops = {
	.unlocked_ioctl = power_ioctl_template_rw,
	.compat_ioctl = power_ioctl_template_rw,
};
#else
static const struct file_operations power_ioctl_template_fops = {
	.unlocked_ioctl = power_ioctl_template_rw,
};
#endif /* CONFIG_COMPAT */

void power_ioctl_ops_register(char *name, void *dev_data, power_ioctl_rw rw)
{
	struct power_ioctl_attr *new_attr = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return;
	}

	if (!g_power_ioctl_list.next) {
		hwlog_info("list init\n");
		INIT_LIST_HEAD(&g_power_ioctl_list);
	}

	new_attr = kzalloc(sizeof(*new_attr), GFP_KERNEL);
	if (!new_attr)
		return;

	strncpy(new_attr->name, name, POWER_IOCTL_ITEM_NAME_MAX - 1);
	new_attr->name[POWER_IOCTL_ITEM_NAME_MAX - 1] = '\0';
	new_attr->dev_data = dev_data;
	new_attr->rw = rw;
	list_add(&new_attr->list, &g_power_ioctl_list);

	hwlog_info("%s ops register ok\n", name);
}

static int __init power_ioctl_init(void)
{
	int ret;
	struct proc_dir_entry *file = NULL;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct power_ioctl_attr *pattr = NULL;

	if (!g_power_ioctl_list.next) {
		hwlog_info("list init\n");
		INIT_LIST_HEAD(&g_power_ioctl_list);
	}

	g_power_ioctl_dir = proc_mkdir_mode("power_ioctl", POWER_IOCTL_DIR_PERMS, NULL);
	if (IS_ERR_OR_NULL(g_power_ioctl_dir)) {
		hwlog_err("node create failed\n");
		ret = -EINVAL;
		goto fail_create_dir;
	}

	list_for_each(pos, &g_power_ioctl_list) {
		pattr = list_entry(pos, struct power_ioctl_attr, list);
		file = proc_create_data(pattr->name, POWER_IOCTL_FILE_PERMS,
			g_power_ioctl_dir, &power_ioctl_template_fops, pattr);
		if (!file) {
			hwlog_err("%s register fail\n", pattr->name);
			ret = -ENOMEM;
			goto fail_create_file;
		}

		hwlog_info("%s register ok\n", pattr->name);
	}

	return 0;

fail_create_file:
	remove_proc_entry("power_ioctl", NULL);
	g_power_ioctl_dir = NULL;
fail_create_dir:
	list_for_each_safe(pos, next, &g_power_ioctl_list) {
		pattr = list_entry(pos, struct power_ioctl_attr, list);
		list_del(&pattr->list);
		kfree(pattr);
	}
	INIT_LIST_HEAD(&g_power_ioctl_list);

	return ret;
}

static void __exit power_ioctl_exit(void)
{
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct power_ioctl_attr *pattr = NULL;

	remove_proc_entry("power_ioctl", NULL);
	g_power_ioctl_dir = NULL;

	list_for_each_safe(pos, next, &g_power_ioctl_list) {
		pattr = list_entry(pos, struct power_ioctl_attr, list);
		list_del(&pattr->list);
		kfree(pattr);
	}
	INIT_LIST_HEAD(&g_power_ioctl_list);
}

late_initcall_sync(power_ioctl_init);
module_exit(power_ioctl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ioctl for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
