/*
 * configfs_unlink_funs.c
 *
 * Unlink functions of all configs in kernel
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/fs.h>
#include <linux/hisi/usb/android_gadget.h>

#define CONFIGFS_ITEM_LINK 0x0020

struct configfs_dirent {
	atomic_t s_count;
	int s_dependent_count;
	struct list_head s_sibling;
	struct list_head s_children;
	struct list_head s_links;
	void *s_element;
	int s_type;
	umode_t s_mode;
	struct dentry *s_dentry;
	struct iattr *s_iattr;
#ifdef CONFIG_LOCKDEP
	int s_depth;
#endif
};

struct configfs_symlink {
	struct list_head sl_list;
	struct config_item *sl_target;
};

static void gadget_config_unlink_functions(struct dentry *p_dentry)
{
	struct configfs_dirent *p_sd = p_dentry->d_fsdata;

	if (!list_empty(&p_sd->s_children)) {
		struct configfs_dirent *sd = NULL, *tmp = NULL;

		list_for_each_entry_safe(sd, tmp,
			&p_sd->s_children, s_sibling) {
			if ((unsigned int)sd->s_type & CONFIGFS_ITEM_LINK) {
				struct configfs_symlink *sl = sd->s_element;
				struct config_item *item = sl->sl_target;

				pr_info("config unlink %s\n", item->ci_name);
				configfs_unlink(0, sd->s_dentry);
			}
		}
	}
}

static void gadget_unlink_functions(struct gadget_info *gi)
{
	if (!list_empty(&gi->configs_group.cg_children)) {
		struct config_item *item = NULL;

		list_for_each_entry(item,
			&gi->configs_group.cg_children, ci_entry) {
			pr_info("unlink %s\n", item->ci_name);
			mutex_unlock(&gi->lock);
			gadget_config_unlink_functions(item->ci_dentry);
			mutex_lock(&gi->lock);
		}
	}
}

