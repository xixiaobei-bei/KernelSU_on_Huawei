/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode_root.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: lvhao13@huawei.com chenjinglong1@huawei.com
 * Create: 2021-01-11
 *
 */

#include <linux/fs_stack.h>
#include <linux/mount.h>
#include <linux/namei.h>

#include "authority/authentication.h"
#include "comm/socket_adapter.h"
#include "comm/transport.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#include "hmdfs_merge_view.h"
#include "hmdfs_trace.h"
#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif

static struct inode *fill_device_local_inode(struct super_block *sb,
					     struct inode *lower_inode)
{
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info = NULL;

	if (!igrab(lower_inode))
		return ERR_PTR(-ESTALE);

	inode = hmdfs_iget_locked_root(sb, HMDFS_ROOT_DEV_LOCAL, lower_inode,
				       NULL);
	if (!inode) {
		hmdfs_err("iget5_locked get inode NULL");
		iput(lower_inode);
		return ERR_PTR(-ENOMEM);
	}
	if (!(inode->i_state & I_NEW)) {
		iput(lower_inode);
		return inode;
	}

	info = hmdfs_i(inode);
	info->inode_type = HMDFS_LAYER_SECOND_LOCAL;

	inode->i_mode =
		(lower_inode->i_mode & S_IFMT) | S_IRWXU | S_IRWXG | S_IXOTH;

	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);

	inode->i_atime = lower_inode->i_atime;
	inode->i_ctime = lower_inode->i_ctime;
	inode->i_mtime = lower_inode->i_mtime;

	inode->i_op = &hmdfs_dir_inode_ops_local;
	inode->i_fop = &hmdfs_dir_ops_local;

	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}

static struct inode *fill_device_inode_remote(struct super_block *sb,
					      uint64_t dev_id)
{
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info = NULL;
	struct hmdfs_peer *con = NULL;

	con = hmdfs_lookup_from_devid(sb->s_fs_info, dev_id);
	if (!con)
		return ERR_PTR(-ENOENT);

	inode = hmdfs_iget_locked_root(sb, HMDFS_ROOT_DEV_REMOTE, NULL, con);
	if (!inode) {
		hmdfs_err("get inode NULL");
		inode = ERR_PTR(-ENOMEM);
		goto out;
	}
	if (!(inode->i_state & I_NEW))
		goto out;

	info = hmdfs_i(inode);
	info->inode_type = HMDFS_LAYER_SECOND_REMOTE;

	inode->i_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH;

	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);
	inode->i_op = &hmdfs_dev_dir_inode_ops_remote;
	inode->i_fop = &hmdfs_dev_dir_ops_remote;

	unlock_new_inode(inode);

out:
	peer_put(con);
	return inode;
}

struct dentry *hmdfs_device_lookup(struct inode *parent_inode,
				   struct dentry *child_dentry,
				   unsigned int flags)
{
	const char *d_name = child_dentry->d_name.name;
	struct inode *root_inode = NULL;
	struct super_block *sb = parent_inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	struct dentry *ret_dentry = NULL;
	int err = 0;
	struct hmdfs_peer *con = NULL;
	struct hmdfs_dentry_info *di = NULL;
	uint8_t *cid = NULL;
	struct path *root_lower_path = NULL;

	trace_hmdfs_device_lookup(parent_inode, child_dentry, flags);
	if (!strncmp(d_name, DEVICE_VIEW_LOCAL,
		     sizeof(DEVICE_VIEW_LOCAL) - 1)) {
		err = init_hmdfs_dentry_info(sbi, child_dentry,
					     HMDFS_LAYER_SECOND_LOCAL);
		if (err) {
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		di = hmdfs_d(sb->s_root);
		root_lower_path = &(di->lower_path);
		hmdfs_set_lower_path(child_dentry, root_lower_path);
		path_get(root_lower_path);
		root_inode = fill_device_local_inode(
			sb, d_inode(root_lower_path->dentry));
		if (IS_ERR(root_inode)) {
			err = PTR_ERR(root_inode);
			ret_dentry = ERR_PTR(err);
			hmdfs_put_reset_lower_path(child_dentry);
			goto out;
		}
		ret_dentry = d_splice_alias(root_inode, child_dentry);
		if (IS_ERR(ret_dentry)) {
			err = PTR_ERR(ret_dentry);
			ret_dentry = ERR_PTR(err);
			hmdfs_put_reset_lower_path(child_dentry);
			goto out;
		}
	} else {
		err = init_hmdfs_dentry_info(sbi, child_dentry,
					     HMDFS_LAYER_SECOND_REMOTE);
		di = hmdfs_d(child_dentry);
		if (err) {
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		cid = kzalloc(HMDFS_CID_SIZE + 1, GFP_KERNEL);
		if (!cid) {
			err = -ENOMEM;
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		memcpy(cid, d_name, HMDFS_CID_SIZE);
		cid[HMDFS_CID_SIZE] = '\0';
		con = hmdfs_lookup_from_cid(sbi, cid);
		if (!con) {
			kfree(cid);
			err = -ENOENT;
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		di->device_id = con->device_id;
		root_inode = fill_device_inode_remote(sb, di->device_id);
		if (IS_ERR(root_inode)) {
			kfree(cid);
			err = PTR_ERR(root_inode);
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		ret_dentry = d_splice_alias(root_inode, child_dentry);
		kfree(cid);
	}
	if (root_inode)
		hmdfs_root_inode_perm_init(root_inode);
	if (!err)
		hmdfs_set_time(child_dentry, jiffies);
out:
	if (con)
		peer_put(con);
	trace_hmdfs_device_lookup_end(parent_inode, child_dentry, err);
	return ret_dentry;
}

struct dentry *hmdfs_root_lookup(struct inode *parent_inode,
				 struct dentry *child_dentry,
				 unsigned int flags)
{
	const char *d_name = child_dentry->d_name.name;
	struct inode *root_inode = NULL;
	struct super_block *sb = parent_inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	struct dentry *ret = ERR_PTR(-ENOENT);
	struct path root_path;

	trace_hmdfs_root_lookup(parent_inode, child_dentry, flags);
	if (sbi->s_merge_switch && !strcmp(d_name, MERGE_VIEW_ROOT)) {
		ret = hmdfs_lookup_merge(parent_inode, child_dentry, flags);
		if (ret && !IS_ERR(ret))
			child_dentry = ret;
		root_inode = d_inode(child_dentry);
	} else if (!strcmp(d_name, DEVICE_VIEW_ROOT)) {
		ret = ERR_PTR(init_hmdfs_dentry_info(
			sbi, child_dentry, HMDFS_LAYER_FIRST_DEVICE));
		if (IS_ERR(ret))
			goto out;
		ret = ERR_PTR(kern_path(sbi->local_src, 0, &root_path));
		if (IS_ERR(ret))
			goto out;
		root_inode = fill_device_inode(sb, d_inode(root_path.dentry));
		ret = d_splice_alias(root_inode, child_dentry);
		path_put(&root_path);
	}
	if (!IS_ERR(ret) && root_inode)
		hmdfs_root_inode_perm_init(root_inode);

out:
	trace_hmdfs_root_lookup_end(parent_inode, child_dentry,
				    PTR_ERR_OR_ZERO(ret));
	return ret;
}

const struct inode_operations hmdfs_device_ops = {
	.lookup = hmdfs_device_lookup,
};

const struct inode_operations hmdfs_root_ops = {
	.lookup = hmdfs_root_lookup,
};

struct inode *fill_device_inode(struct super_block *sb,
				struct inode *lower_inode)
{
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info = NULL;

	inode = hmdfs_iget_locked_root(sb, HMDFS_ROOT_DEV, NULL, NULL);
	if (!inode) {
		hmdfs_err("iget5_locked get inode NULL");
		return ERR_PTR(-ENOMEM);
	}
	if (!(inode->i_state & I_NEW))
		return inode;

	info = hmdfs_i(inode);
	info->inode_type = HMDFS_LAYER_FIRST_DEVICE;

	inode->i_atime = lower_inode->i_atime;
	inode->i_ctime = lower_inode->i_ctime;
	inode->i_mtime = lower_inode->i_mtime;

	inode->i_mode = (lower_inode->i_mode & S_IFMT) | S_IRUSR | S_IXUSR |
			S_IRGRP | S_IXGRP | S_IXOTH;
	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);
	inode->i_op = &hmdfs_device_ops;
	inode->i_fop = &hmdfs_device_fops;

	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}

struct inode *fill_root_inode(struct super_block *sb, struct inode *lower_inode)
{
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info = NULL;

	if (!igrab(lower_inode))
		return ERR_PTR(-ESTALE);

	inode = hmdfs_iget_locked_root(sb, HMDFS_ROOT_ANCESTOR, lower_inode,
				       NULL);
	if (!inode) {
		hmdfs_err("iget5_locked get inode NULL");
		iput(lower_inode);
		return ERR_PTR(-ENOMEM);
	}
	if (!(inode->i_state & I_NEW)) {
		iput(lower_inode);
		return inode;
	}

	info = hmdfs_i(inode);
	info->inode_type = HMDFS_LAYER_ZERO;
#ifdef CONFIG_HMDFS_1_0
	info->file_no = INVALID_FILE_ID;
	info->adapter_dentry_flag = ADAPTER_OTHER_DENTRY_FLAG;
#endif
	inode->i_mode = (lower_inode->i_mode & S_IFMT) | S_IRUSR | S_IXUSR |
			S_IRGRP | S_IXGRP | S_IXOTH;

#ifdef CONFIG_HMDFS_ANDROID
	inode->i_uid = lower_inode->i_uid;
	inode->i_gid = lower_inode->i_gid;
#else
	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);
#endif
	inode->i_atime = lower_inode->i_atime;
	inode->i_ctime = lower_inode->i_ctime;
	inode->i_mtime = lower_inode->i_mtime;

	inode->i_op = &hmdfs_root_ops;
	inode->i_fop = &hmdfs_root_fops;
	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}
