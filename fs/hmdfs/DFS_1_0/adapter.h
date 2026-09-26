/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: hmdfs 1.0 adapter
 * Author: chenyi77@huawei.com
 * Create: 2020-04-17
 *
 */

#ifndef HMDFS_ADAPTER_H
#define HMDFS_ADAPTER_H

#include "adapter_client.h"
#include "adapter_dentry_limitation.h"
#include "adapter_file_id_generator.h"
#include "adapter_server.h"

extern const struct file_operations hmdfs_adapter_remote_file_fops;
extern const struct address_space_operations hmdfs_adapter_remote_file_aops;
extern const struct inode_operations hmdfs_adapter_remote_file_iops;

void hmdfs_adapter_recv_mesg_callback(struct hmdfs_peer *con, void *head,
				      void *buf);
extern int hmdfs_adapter_remote_unlink(struct hmdfs_peer *con,
				       struct dentry *dentry);
extern int hmdfs_adapter_remote_readdir(struct hmdfs_peer *con,
					struct file *file,
					struct dir_context *ctx);
extern struct hmdfs_lookup_ret *
hmdfs_adapter_remote_lookup(struct hmdfs_peer *con, const char *relative_path,
			    const char *d_name);
extern int hmdfs_adapter_update(struct inode *inode, struct file *file);
extern int hmdfs_adapter_create(struct hmdfs_sb_info *sbi,
				struct dentry *dentry,
				const char *relative_dir_path);
extern int hmdfs_adapter_rename(struct hmdfs_sb_info *sbi, const char *old_path,
				struct dentry *old_dentry, const char *new_path,
				struct dentry *new_dentry);
extern int hmdfs_adapter_remove(struct hmdfs_sb_info *sbi,
				const char *relative_dir_path,
				const char *name);
#endif
