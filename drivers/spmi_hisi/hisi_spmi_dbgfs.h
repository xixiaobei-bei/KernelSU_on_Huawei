/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * spmi_dbgfs.h
 *
 * SPMI Debug fs support
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

#ifndef _SPMI_DBGFS_H
#define _SPMI_DBGFS_H

#include <linux/hisi-spmi.h>

#ifdef CONFIG_HISI_SPMI_DEBUG_FS
int spmi_dfs_add_controller(struct spmi_controller *ctrl);
int spmi_dfs_del_controller(struct spmi_controller *ctrl);
struct dentry *spmi_dfs_create_file(struct spmi_controller *ctrl,
	const char *name, void *data, const struct file_operations *fops);

#else
static inline int spmi_dfs_add_controller(struct spmi_controller *ctrl)
{
	return 0;
}
static inline int spmi_dfs_del_controller(struct spmi_controller *ctrl)
{
	return 0;
}

static inline struct dentry *spmi_dfs_create_file(struct spmi_controller *ctrl,
	const char *name, void *data, const struct file_operations *fops)
{
	return NULL;
}
#endif

#endif /* _SPMI_DBGFS_H */
