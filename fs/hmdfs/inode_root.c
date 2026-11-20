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
#include "comm/device_node.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#include "hmdfs_merge_view.h"
#include "hmdfs_trace.h"
#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif

int hmdfs_generic_get_inode(struct super_block *sb, uint64_t root_ino,
			    struct inode *lo_i, struct inode **inode,
			    struct hmdfs_peer *peer, bool is_inode_local)
{
	if (lo_i && !igrab(lo_i)) {
		*inode = ERR_PTR(-ESTALE);
		return -ESTALE;
	}

	if (is_inode_local)
		*inode = hmdfs_iget5_locked_local(sb, lo_i);
	else
		*inode = hmdfs_iget_locked_root(sb, root_ino, lo_i, peer);
	if (!(*inode)) {
		hmdfs_err("iget5_locked get inode NULL");
		iput(lo_i);
		*inode = ERR_PTR(-ENOMEM);
		return -ENOMEM;
	}
	if (!((*inode)->i_state & I_NEW)) {
		iput(lo_i);
		return -ESTALE;
	}
	return 0;
}

void hmdfs_generic_fill_inode(struct inode *inode, struct inode *lower_inode,
			      int inode_type, bool is_inode_local)
{
	struct hmdfs_inode_info *info = NULL;

	info = hmdfs_i(inode);
	info->inode_type = inode_type;
	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);

	if (is_inode_local) {
		if (S_ISDIR(lower_inode->i_mode))
			inode->i_mode = (lower_inode->i_mode & S_IFMT) |
					S_IRWXU | S_IRWXG | S_IXOTH;
		else if (S_ISREG(lower_inode->i_mode))
			inode->i_mode = (lower_inode->i_mode & S_IFMT) |
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
		else if (S_ISLNK(lower_inode->i_mode))
			inode->i_mode =
				S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
		inode->i_generation = lower_inode->i_generation;
#ifdef CONFIG_HMDFS_ANDROID
		inode->i_uid = lower_inode->i_uid;
		inode->i_gid = lower_inode->i_gid;
#endif
	} else if (lower_inode && !is_inode_local) {
		inode->i_mode = (lower_inode->i_mode & S_IFMT) | S_IRWXU |
				S_IRWXG | S_IXOTH;
	} else {
		inode->i_mode = S_IRWXU | S_IRWXG | S_IXOTH;
	}
	if (lower_inode) {
		inode->i_atime = lower_inode->i_atime;
		inode->i_ctime = lower_inode->i_ctime;
		inode->i_mtime = lower_inode->i_mtime;
	}
}

static struct inode *fill_device_local_inode(struct super_block *sb,
					     struct inode *lower_inode)
{
	struct inode *inode = NULL;
	int err = 0;

	err = hmdfs_generic_get_inode(sb, HMDFS_ROOT_DEV_LOCAL, lower_inode,
				      &inode, NULL, false);
	if (err)
		return inode;

	hmdfs_generic_fill_inode(inode, lower_inode, HMDFS_LAYER_SECOND_LOCAL,
				 false);

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
	struct hmdfs_peer *con = NULL;
	int err;

	con = hmdfs_lookup_from_devid(sb->s_fs_info, dev_id);
	if (!con)
		return ERR_PTR(-ENOENT);
	err = hmdfs_generic_get_inode(sb, HMDFS_ROOT_DEV_REMOTE, NULL, &inode,
				      con, false);
	if (err)
		goto out;

	hmdfs_generic_fill_inode(inode, NULL, HMDFS_LAYER_SECOND_REMOTE, false);

	inode->i_mode |= S_IFDIR;
	inode->i_op = &hmdfs_dev_dir_inode_ops_remote;
	inode->i_fop = &hmdfs_dev_dir_ops_remote;

	unlock_new_inode(inode);

out:
	peer_put(con);
	return inode;
}

static int hmdfs_check_connect_status(struct hmdfs_peer *peer)
{
	struct connection *connect = get_conn_impl(peer, C_REQUEST);
	if (connect) {
		if (connect->link_type == LINK_TYPE_WLAN) {
			connection_put(connect);
			return 0;
		}
		if (!hmdfs_pair_conn_and_cmd_flag(connect, C_REQUEST))
			hmdfs_try_get_p2p_session_async(peer);
		connection_put(connect);
		return 0;
	}

	return hmdfs_try_get_p2p_session_sync(peer);
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
	struct hmdfs_peer *peer = NULL;
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
		strncpy(cid, d_name, HMDFS_CID_SIZE);
		cid[HMDFS_CID_SIZE] = '\0';
		peer = hmdfs_lookup_from_cid(sbi, cid);
		if (!peer || !hmdfs_is_node_online_or_shaking(peer)) {
			kfree(cid);
			err = -ENOENT;
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		err = hmdfs_check_connect_status(peer);
		if (err) {
			kfree(cid);
			err = -ENOENT;
			ret_dentry = ERR_PTR(err);
			goto out;
		}
		di->device_id = peer->device_id;
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
	if (peer)
		peer_put(peer);
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
	int err;

	err = hmdfs_generic_get_inode(sb, HMDFS_ROOT_DEV, NULL, &inode,
				      NULL, false);
	if (err)
		return inode;

	hmdfs_generic_fill_inode(inode, lower_inode, HMDFS_LAYER_FIRST_DEVICE,
				 false);

	inode->i_op = &hmdfs_device_ops;
	inode->i_fop = &hmdfs_device_fops;

	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}
struct inode *fill_root_inode(struct super_block *sb, struct inode *lower_inode)
{
	int err;
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info = NULL;

	err = hmdfs_generic_get_inode(sb, HMDFS_ROOT_ANCESTOR, lower_inode,
				      &inode, NULL, false);
	if (err)
		return inode;

	hmdfs_generic_fill_inode(inode, lower_inode, HMDFS_LAYER_ZERO, false);

	info = hmdfs_i(inode);
#ifdef CONFIG_HMDFS_1_0
	info->file_no = INVALID_FILE_ID;
	info->adapter_dentry_flag = ADAPTER_OTHER_DENTRY_FLAG;
#endif

	inode->i_op = &hmdfs_root_ops;
	inode->i_fop = &hmdfs_root_fops;
	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}
