/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode_local.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lousong@huawei.com maojingjing@huawei.com
 * Create: 2020-03-26
 *
 */

#include <linux/file.h>
#include <linux/fs_stack.h>
#include <linux/kernel.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/string.h>

#include "authority/authentication.h"
#include "comm/socket_adapter.h"
#include "comm/transport.h"
#include "hmdfs_client.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#include "hmdfs_trace.h"

#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif

extern struct kmem_cache *hmdfs_dentry_cachep;

static const char *const symlink_tgt_white_list[] = {
	"/storage/",
	"/sdcard/",
};

struct hmdfs_name_data {
	struct dir_context ctx;
	const struct qstr *to_find;
	char *name;
	bool found;
};

int init_hmdfs_dentry_info(struct hmdfs_sb_info *sbi, struct dentry *dentry,
			   int dentry_type)
{
	struct hmdfs_dentry_info *info =
		kmem_cache_zalloc(hmdfs_dentry_cachep, GFP_ATOMIC);

	if (!info)
		return -ENOMEM;
	dentry->d_fsdata = info;
	INIT_LIST_HEAD(&info->cache_list_head);
	INIT_LIST_HEAD(&info->remote_cache_list_head);
	spin_lock_init(&info->cache_list_lock);
	mutex_init(&info->remote_cache_list_lock);
	mutex_init(&info->cache_pull_lock);
	spin_lock_init(&info->lock);
	info->dentry_type = dentry_type;
	info->device_id = 0;
	if (dentry_type == HMDFS_LAYER_ZERO ||
	    dentry_type == HMDFS_LAYER_FIRST_DEVICE ||
	    dentry_type == HMDFS_LAYER_SECOND_LOCAL ||
	    dentry_type == HMDFS_LAYER_SECOND_REMOTE)
		d_set_d_op(dentry, &hmdfs_dev_dops);
	else
		d_set_d_op(dentry, &hmdfs_dops);
	return 0;
}

static inline void set_symlink_flag(struct hmdfs_dentry_info *gdi)
{
	gdi->file_type = HM_SYMLINK;
}

struct inode *fill_inode_local(struct super_block *sb,
			       struct inode *lower_inode)
{
	struct inode *inode;
	struct hmdfs_inode_info *info;

	if (!igrab(lower_inode))
		return ERR_PTR(-ESTALE);

	inode = hmdfs_iget5_locked_local(sb, lower_inode);
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
#ifdef CONFIG_HMDFS_1_0
	info->file_no = hmdfs_read_file_id(lower_inode);
	info->adapter_dentry_flag = hmdfs_adapter_read_dentry_flag(lower_inode);
#endif
#ifdef CONFIG_HMDFS_ANDROID
	info->perm = hmdfs_read_perm(lower_inode);
#endif
	if (S_ISDIR(lower_inode->i_mode))
		inode->i_mode = (lower_inode->i_mode & S_IFMT) | S_IRWXU |
				S_IRWXG | S_IXOTH;
	else if (S_ISREG(lower_inode->i_mode))
		inode->i_mode = (lower_inode->i_mode & S_IFMT) | S_IRUSR |
				S_IWUSR | S_IRGRP | S_IWGRP;
	else if (S_ISLNK(lower_inode->i_mode))
		inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

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
	inode->i_generation = lower_inode->i_generation;

	info->inode_type = HMDFS_LAYER_OTHER_LOCAL;
	if (S_ISDIR(lower_inode->i_mode)) {
		inode->i_op = &hmdfs_dir_inode_ops_local;
		inode->i_fop = &hmdfs_dir_ops_local;
		inode->i_mode |= S_IXUGO;
	} else if (S_ISREG(lower_inode->i_mode)) {
		inode->i_op = &hmdfs_file_iops_local;
		inode->i_fop = &hmdfs_file_fops_local;
	} else if (S_ISLNK(lower_inode->i_mode)) {
		inode->i_op = &hmdfs_symlink_iops_local;
		inode->i_fop = &hmdfs_file_fops_local;
	}

	fsstack_copy_inode_size(inode, lower_inode);
	unlock_new_inode(inode);
	return inode;
}

/* hmdfs_convert_lookup_flags - covert hmdfs lookup flags to vfs lookup flags
 *
 * @hmdfs_flags: hmdfs lookup flags
 * @vfs_flags: pointer to converted flags
 *
 * return 0 on success, or err code on failure.
 */
int hmdfs_convert_lookup_flags(unsigned int hmdfs_flags,
			       unsigned int *vfs_flags)
{
	*vfs_flags = 0;

	/* currently only support HMDFS_LOOKUP_REVAL */
	if (hmdfs_flags & ~HMDFS_LOOKUP_REVAL)
		return -EINVAL;

	if (hmdfs_flags & HMDFS_LOOKUP_REVAL)
		*vfs_flags |= LOOKUP_REVAL;

	return 0;
}

static int hmdfs_name_match(struct dir_context *ctx, const char *name,
			    int namelen, loff_t offset, u64 ino,
			    unsigned int d_type)
{
	struct hmdfs_name_data *buf =
		container_of(ctx, struct hmdfs_name_data, ctx);
	struct qstr candidate = QSTR_INIT(name, namelen);

	if (qstr_case_eq(buf->to_find, &candidate)) {
		memcpy(buf->name, name, namelen);
		buf->name[namelen] = 0;
		buf->found = true;
		return 1;
	}
	return 0;
}

static int __lookup_nosensitive(struct path *lower_parent_path,
				struct dentry *child_dentry, unsigned int flags,
				struct path *lower_path)
{
	struct file *file;
	const struct cred *cred = current_cred();
	const struct qstr *name = &child_dentry->d_name;
	int err;
	struct hmdfs_name_data buffer = {
		.ctx.actor = hmdfs_name_match,
		.to_find = name,
		.name = __getname(),
		.found = false,
	};

	if (!buffer.name) {
		err = -ENOMEM;
		goto out;
	}
	file = dentry_open(lower_parent_path, O_RDONLY, cred);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		goto put_name;
	}
	err = iterate_dir(file, &buffer.ctx);
	fput(file);
	if (err)
		goto put_name;
	if (buffer.found)
		err = vfs_path_lookup(lower_parent_path->dentry,
				      lower_parent_path->mnt, buffer.name,
				      flags, lower_path);
	else
		err = -ENOENT;
put_name:
	__putname(buffer.name);
out:
	return err;
}

struct dentry *hmdfs_lookup_local(struct inode *parent_inode,
				  struct dentry *child_dentry,
				  unsigned int flags)
{
	const char *d_name = child_dentry->d_name.name;
	int err = 0;
	struct path lower_path, lower_parent_path;
	struct dentry *lower_dentry = NULL, *parent_dentry = NULL, *ret = NULL;
	struct hmdfs_dentry_info *gdi = NULL;
	struct inode *child_inode = NULL;
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);

	trace_hmdfs_lookup_local(parent_inode, child_dentry, flags);
	if (child_dentry->d_name.len > NAME_MAX) {
		ret = ERR_PTR(-ENAMETOOLONG);
		goto out;
	}
#ifdef CONFIG_HMDFS_1_0
	dentry_syncer_wakeup();
#endif

	/* local device */
	parent_dentry = dget_parent(child_dentry);
	hmdfs_get_lower_path(parent_dentry, &lower_parent_path);
	err = init_hmdfs_dentry_info(sbi, child_dentry,
				     HMDFS_LAYER_OTHER_LOCAL);
	if (err) {
		ret = ERR_PTR(err);
		goto out_err;
	}

	gdi = hmdfs_d(child_dentry);

	flags &= ~LOOKUP_FOLLOW;
	err = vfs_path_lookup(lower_parent_path.dentry, lower_parent_path.mnt,
			      (child_dentry->d_name.name), 0, &lower_path);
	if (err == -ENOENT && !sbi->s_case_sensitive)
		err = __lookup_nosensitive(&lower_parent_path, child_dentry, 0,
					   &lower_path);
	if (err && err != -ENOENT) {
		ret = ERR_PTR(err);
		goto out_err;
	} else if (!err) {
		hmdfs_set_lower_path(child_dentry, &lower_path);
		child_inode = fill_inode_local(parent_inode->i_sb,
					       d_inode(lower_path.dentry));
		if (S_ISLNK(d_inode(lower_path.dentry)->i_mode))
			set_symlink_flag(gdi);
		if (IS_ERR(child_inode)) {
			err = PTR_ERR(child_inode);
			ret = ERR_PTR(err);
			hmdfs_put_reset_lower_path(child_dentry);
			goto out_err;
		}
		ret = d_splice_alias(child_inode, child_dentry);
		if (IS_ERR(ret)) {
			err = PTR_ERR(ret);
			hmdfs_put_reset_lower_path(child_dentry);
			goto out_err;
		}

		check_and_fixup_ownership(parent_inode, child_inode,
					  lower_path.dentry,
					  child_dentry->d_name.name);
		goto out_err;
	}
	/*
	 * return 0 here, so that vfs can continue the process of making this
	 * negative dentry to a positive one while creating a new file.
	 */
	err = 0;
	ret = 0;

	lower_dentry = lookup_one_len_unlocked(d_name, lower_parent_path.dentry,
					       child_dentry->d_name.len);
	if (IS_ERR(lower_dentry)) {
		err = PTR_ERR(lower_dentry);
		ret = lower_dentry;
		goto out_err;
	}
	lower_path.dentry = lower_dentry;
	lower_path.mnt = mntget(lower_parent_path.mnt);
	hmdfs_set_lower_path(child_dentry, &lower_path);

out_err:
	if (!err)
		hmdfs_set_time(child_dentry, jiffies);
	hmdfs_put_lower_path(&lower_parent_path);
	dput(parent_dentry);
out:
	trace_hmdfs_lookup_local_end(parent_inode, child_dentry, err);
	return ret;
}

int hmdfs_mkdir_local_dentry(struct inode *dir, struct dentry *dentry,
			     umode_t mode)
{
	struct inode *lower_dir = hmdfs_i(dir)->lower_inode;
	struct dentry *lower_dir_dentry = NULL;
	struct super_block *sb = dir->i_sb;
#ifdef CONFIG_HMDFS_1_0
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	char *relative_dir_path = NULL;
#endif
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	int error = 0;
	struct inode *lower_inode = NULL;
	struct inode *child_inode = NULL;
	bool local_res = false;
	struct cache_fs_override or;
	__u16 child_perm;
	kuid_t tmp_uid;

#ifdef CONFIG_HMDFS_1_0
	relative_dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!relative_dir_path) {
		error = -ENOMEM;
		goto path_err;
	}
#endif

	error = hmdfs_override_dir_id_fs(&or, dir, dentry, &child_perm);
	if (error)
		goto cleanup;

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_dir_dentry = lock_parent(lower_dentry);

	tmp_uid = hmdfs_override_inode_uid(lower_dir);
	mode = (mode & S_IFMT) | 00771;

	error = vfs_mkdir(lower_dir, lower_dentry, mode);
	hmdfs_revert_inode_uid(lower_dir, tmp_uid);
	if (error) {
		hmdfs_err("vfs_mkdir() error:%d", error);
		goto out;
	}
	local_res = true;
	lower_inode = d_inode(lower_dentry);
#ifdef CONFIG_HMDFS_ANDROID
	error = hmdfs_persist_perm(lower_dentry, &child_perm);
#endif
	child_inode = fill_inode_local(sb, lower_inode);
	if (IS_ERR(child_inode)) {
		error = PTR_ERR(child_inode);
		goto out;
	}
	d_add(dentry, child_inode);
	set_nlink(dir, hmdfs_i(dir)->lower_inode->i_nlink);
out:
	unlock_dir(lower_dir_dentry);
	if (local_res) {
		hmdfs_drop_remote_cache_dents(dentry->d_parent);
#ifdef CONFIG_HMDFS_1_0
		if (adapter_identify_dentry_flag(dir, dentry, lower_dentry) ==
		    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
			hmdfs_i(d_inode(dentry))->file_no = INVALID_FILE_ID;
			hmdfs_adapter_create(sbi, dentry, relative_dir_path);
		}
#endif
	}
	if (error) {
		hmdfs_clear_drop_flag(dentry->d_parent);
		d_drop(dentry);
	}
	hmdfs_put_lower_path(&lower_path);
	hmdfs_revert_dir_id_fs(&or);
cleanup:
#ifdef CONFIG_HMDFS_1_0
	kfree(relative_dir_path);
path_err:
#endif
	return error;
}

int hmdfs_mkdir_local(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	int err = 0;

	if (check_filename(dentry->d_name.name, dentry->d_name.len)) {
		err = -EINVAL;
		return err;
	}

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		err = -EACCES;
		return err;
	}
	err = hmdfs_mkdir_local_dentry(dir, dentry, mode);
	trace_hmdfs_mkdir_local(dir, dentry, err);
	return err;
}

int hmdfs_create_local_dentry(struct inode *dir, struct dentry *dentry,
			      umode_t mode, bool want_excl)
{
	struct inode *lower_dir = NULL;
	struct dentry *lower_dir_dentry = NULL;
	struct super_block *sb = dir->i_sb;
#ifdef CONFIG_HMDFS_1_0
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	char *relative_dir_path = NULL;
#endif
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	int error = 0;
	struct inode *lower_inode = NULL;
	struct inode *child_inode = NULL;
	kuid_t tmp_uid;
#ifdef CONFIG_HMDFS_ANDROID
	const struct cred *saved_cred = NULL;
	struct fs_struct *saved_fs = NULL, *copied_fs = NULL;
	__u16 child_perm;
#endif

#ifdef CONFIG_HMDFS_ANDROID
	saved_cred = hmdfs_override_file_fsids(dir, &child_perm);
	if (!saved_cred) {
		error = -ENOMEM;
		goto path_err;
	}

	saved_fs = current->fs;
	copied_fs = hmdfs_override_fsstruct(saved_fs);
	if (!copied_fs) {
		error = -ENOMEM;
		goto revert_fsids;
	}
#endif
	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	mode = (mode & S_IFMT) | 00660;
	lower_dir_dentry = lock_parent(lower_dentry);
	lower_dir = d_inode(lower_dir_dentry);
	tmp_uid = hmdfs_override_inode_uid(lower_dir);
	error = vfs_create(lower_dir, lower_dentry, mode, want_excl);
	hmdfs_revert_inode_uid(lower_dir, tmp_uid);
	unlock_dir(lower_dir_dentry);
	if (error)
		goto out;

	lower_inode = d_inode(lower_dentry);
#ifdef CONFIG_HMDFS_ANDROID
	error = hmdfs_persist_perm(lower_dentry, &child_perm);
#endif
	child_inode = fill_inode_local(sb, lower_inode);
	if (IS_ERR(child_inode)) {
		error = PTR_ERR(child_inode);
		goto out_created;
	}
	d_add(dentry, child_inode);

out_created:
	hmdfs_drop_remote_cache_dents(dentry->d_parent);
#ifdef CONFIG_HMDFS_1_0
	relative_dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!relative_dir_path) {
		error = -ENOMEM;
		hmdfs_put_lower_path(&lower_path);
		goto path_err;
	}
	if (adapter_identify_dentry_flag(dir, dentry, lower_dentry) ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG)
		hmdfs_adapter_create(sbi, dentry, relative_dir_path);
	kfree(relative_dir_path);
#endif
out:
	if (error) {
		hmdfs_clear_drop_flag(dentry->d_parent);
		d_drop(dentry);
	}
	hmdfs_put_lower_path(&lower_path);

#ifdef CONFIG_HMDFS_ANDROID
	hmdfs_revert_fsstruct(saved_fs, copied_fs);
revert_fsids:
	hmdfs_revert_fsids(saved_cred);
#endif
#if (defined CONFIG_HMDFS_ANDROID) || (defined CONFIG_HMDFS_1_0)
path_err:
#endif
	return error;
}

int hmdfs_create_local(struct inode *dir, struct dentry *child_dentry,
		       umode_t mode, bool want_excl)
{
	int err = 0;

	if (check_filename(child_dentry->d_name.name,
			   child_dentry->d_name.len)) {
		err = -EINVAL;
		return err;
	}

	if (hmdfs_file_type(child_dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		err = -EACCES;
		return err;
	}

	err = hmdfs_create_local_dentry(dir, child_dentry, mode, want_excl);
	trace_hmdfs_create_local(dir, child_dentry, err);
	return err;
}

int hmdfs_rmdir_local_dentry(struct inode *dir, struct dentry *dentry)
{
	struct inode *lower_dir = NULL;
	struct dentry *lower_dir_dentry = NULL;
	kuid_t tmp_uid;
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	int error = 0;
#ifdef CONFIG_HMDFS_1_0
	char *relative_dir_path = NULL;
#endif

#ifdef CONFIG_HMDFS_1_0
	relative_dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!relative_dir_path) {
		error = -ENOMEM;
		goto path_err;
	}
#endif
	hmdfs_clear_cache_dents(dentry, true);
	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_dir_dentry = lock_parent(lower_dentry);
	lower_dir = d_inode(lower_dir_dentry);
	tmp_uid = hmdfs_override_inode_uid(lower_dir);

	error = vfs_rmdir(lower_dir, lower_dentry);
	hmdfs_revert_inode_uid(lower_dir, tmp_uid);
	unlock_dir(lower_dir_dentry);
	hmdfs_put_lower_path(&lower_path);
	if (error)
		goto path_err;
#ifdef CONFIG_HMDFS_1_0
	if (hmdfs_i(d_inode(dentry))->adapter_dentry_flag ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		struct hmdfs_sb_info *sbi = dir->i_sb->s_fs_info;

		hmdfs_adapter_remove(sbi, relative_dir_path,
				     dentry->d_name.name);
	}
#endif
	hmdfs_drop_remote_cache_dents(dentry->d_parent);
path_err:
	if (error)
		hmdfs_clear_drop_flag(dentry->d_parent);
#ifdef CONFIG_HMDFS_1_0
	kfree(relative_dir_path);
#endif
	return error;
}

int hmdfs_rmdir_local(struct inode *dir, struct dentry *dentry)
{
	int err = 0;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		err = -EACCES;
		goto out;
	}

	err = hmdfs_rmdir_local_dentry(dir, dentry);
	if (err != 0) {
		hmdfs_err("rm dir failed:%d", err);
		goto out;
	}

	/* drop dentry even remote failed
	 * it maybe cause that one remote devices disconnect
	 * when doing remote rmdir
	 */
	d_drop(dentry);
out:
	/* return connect device's errcode */
	trace_hmdfs_rmdir_local(dir, dentry, err);
	return err;
}

int hmdfs_unlink_local_dentry(struct inode *dir, struct dentry *dentry)
{
	struct inode *lower_dir = hmdfs_i(dir)->lower_inode;
	struct dentry *lower_dir_dentry = NULL;
#ifdef CONFIG_HMDFS_1_0
	struct hmdfs_sb_info *sbi = dir->i_sb->s_fs_info;
	char *relative_dir_path = NULL;
#endif
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	int error;
	kuid_t tmp_uid;

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	dget(lower_dentry);
	lower_dir_dentry = lock_parent(lower_dentry);
	tmp_uid = hmdfs_override_inode_uid(lower_dir);
	error = vfs_unlink(lower_dir, lower_dentry, NULL);
	hmdfs_revert_inode_uid(lower_dir, tmp_uid);
	set_nlink(d_inode(dentry),
		  hmdfs_i(d_inode(dentry))->lower_inode->i_nlink);
	unlock_dir(lower_dir_dentry);
	dput(lower_dentry);
	if (error)
		goto path_err;

#ifdef CONFIG_HMDFS_1_0
	relative_dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!relative_dir_path) {
		error = -ENOMEM;
		goto path_err;
	}
	if (hmdfs_i(d_inode(dentry))->adapter_dentry_flag ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		hmdfs_adapter_remove_file_id_async(
			hmdfs_sb(dir->i_sb), hmdfs_i(d_inode(dentry))->file_no);
		hmdfs_adapter_remove(sbi, relative_dir_path,
				     dentry->d_name.name);
	}
	kfree(relative_dir_path);
#endif
	hmdfs_drop_remote_cache_dents(dentry->d_parent);
	d_drop(dentry);
	hmdfs_put_lower_path(&lower_path);

path_err:
	if (error)
		hmdfs_clear_drop_flag(dentry->d_parent);
	return error;
}

int hmdfs_unlink_local(struct inode *dir, struct dentry *dentry)
{
	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON)
		return -EACCES;

	return hmdfs_unlink_local_dentry(dir, dentry);
}

int hmdfs_rename_local_dentry(struct inode *old_dir, struct dentry *old_dentry,
			      struct inode *new_dir, struct dentry *new_dentry,
			      unsigned int flags)
{
	struct path lower_old_path;
	struct path lower_new_path;
	struct dentry *lower_old_dentry = NULL;
	struct dentry *lower_new_dentry = NULL;
	struct dentry *lower_old_dir_dentry = NULL;
	struct dentry *lower_new_dir_dentry = NULL;
	struct dentry *trap = NULL;
	int rc = 0;
#ifdef CONFIG_HMDFS_1_0
	uint32_t fno = hmdfs_i(d_inode(old_dentry))->file_no;
#endif
	kuid_t old_dir_uid, new_dir_uid;

	if (flags)
		return -EINVAL;

	hmdfs_get_lower_path(old_dentry, &lower_old_path);
	lower_old_dentry = lower_old_path.dentry;
	if (!lower_old_dentry) {
		hmdfs_err("lower_old_dentry as NULL");
		rc = -EACCES;
		goto out_put_old_path;
	}

	hmdfs_get_lower_path(new_dentry, &lower_new_path);
	lower_new_dentry = lower_new_path.dentry;
	if (!lower_new_dentry) {
		hmdfs_err("lower_new_dentry as NULL");
		rc = -EACCES;
		goto out_put_new_path;
	}

	lower_old_dir_dentry = dget_parent(lower_old_dentry);
	lower_new_dir_dentry = dget_parent(lower_new_dentry);
	trap = lock_rename(lower_old_dir_dentry, lower_new_dir_dentry);
	old_dir_uid = hmdfs_override_inode_uid(d_inode(lower_old_dir_dentry));
	new_dir_uid = hmdfs_override_inode_uid(d_inode(lower_new_dir_dentry));

	/* source should not be ancestor of target */
	if (trap == lower_old_dentry) {
		rc = -EINVAL;
		goto out_lock;
	}
	/* target should not be ancestor of source */
	if (trap == lower_new_dentry) {
		rc = -ENOTEMPTY;
		goto out_lock;
	}

	rc = vfs_rename(d_inode(lower_old_dir_dentry), lower_old_dentry,
			d_inode(lower_new_dir_dentry), lower_new_dentry, NULL,
			flags);
out_lock:
	dget(old_dentry);

	hmdfs_revert_inode_uid(d_inode(lower_old_dir_dentry), old_dir_uid);
	hmdfs_revert_inode_uid(d_inode(lower_new_dir_dentry), new_dir_uid);

	unlock_rename(lower_old_dir_dentry, lower_new_dir_dentry);
	if (rc == 0) {
		hmdfs_drop_remote_cache_dents(old_dentry->d_parent);
		if (old_dentry->d_parent != new_dentry->d_parent)
			hmdfs_drop_remote_cache_dents(new_dentry->d_parent);
	} else {
		hmdfs_clear_drop_flag(old_dentry->d_parent);
		if (old_dentry->d_parent != new_dentry->d_parent)
			hmdfs_clear_drop_flag(old_dentry->d_parent);
		d_drop(new_dentry);
	}

#ifdef CONFIG_HMDFS_1_0
	if (hmdfs_i(d_inode(old_dentry))->adapter_dentry_flag ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		struct dentry *parent_dentry = NULL;
		struct path lower_parent_path;

		parent_dentry = dget_parent(new_dentry);
		hmdfs_get_lower_path(parent_dentry, &lower_parent_path);
		hmdfs_adapter_add_rename_task(
			hmdfs_sb(old_dir->i_sb),
			&lower_parent_path,
			new_dentry->d_name.name, fno,
			d_inode(old_dentry)->i_mode);
		hmdfs_put_lower_path(&lower_parent_path);
		dput(parent_dentry);
	}
#endif
	dput(old_dentry);
	dput(lower_old_dir_dentry);
	dput(lower_new_dir_dentry);

out_put_new_path:
	hmdfs_put_lower_path(&lower_new_path);
out_put_old_path:
	hmdfs_put_lower_path(&lower_old_path);
	return rc;
}

int hmdfs_rename_local(struct inode *old_dir, struct dentry *old_dentry,
		       struct inode *new_dir, struct dentry *new_dentry,
		       unsigned int flags)
{
	int err = 0;
	int ret = 0;
#ifdef CONFIG_HMDFS_1_0
	struct super_block *sb = old_dir->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	char *relative_old_dir_path = 0;
	char *relative_new_dir_path = 0;
#endif

	trace_hmdfs_rename_local(old_dir, old_dentry, new_dir, new_dentry,
				 flags);
	if (hmdfs_file_type(old_dentry->d_name.name) != HMDFS_TYPE_COMMON ||
	    hmdfs_file_type(new_dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		err = -EACCES;
		goto rename_out;
	}

	if (S_ISREG(old_dentry->d_inode->i_mode)) {
		err = hmdfs_rename_local_dentry(old_dir, old_dentry, new_dir,
						new_dentry, flags);
	} else if (S_ISDIR(old_dentry->d_inode->i_mode)) {
		ret = hmdfs_rename_local_dentry(old_dir, old_dentry, new_dir,
						new_dentry, flags);
		if (ret != 0) {
			err = ret;
			goto rename_out;
		}
	}

	if (!err)
		d_invalidate(old_dentry);
#ifdef CONFIG_HMDFS_1_0
	relative_old_dir_path =
		hmdfs_get_dentry_relative_path(old_dentry->d_parent);
	relative_new_dir_path =
		hmdfs_get_dentry_relative_path(new_dentry->d_parent);
	if (!relative_old_dir_path || !relative_new_dir_path) {
		err = -EACCES;
		goto out_err;
	}

	if (hmdfs_i(d_inode(old_dentry))->adapter_dentry_flag ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		if (err == 0)
			hmdfs_adapter_rename(sbi, relative_old_dir_path,
					     old_dentry, relative_new_dir_path,
					     new_dentry);
	}
out_err:
	kfree(relative_old_dir_path);
	kfree(relative_new_dir_path);
#endif

rename_out:
	return err;
}

static bool symname_is_allowed(const char *symname)
{
	size_t symname_len = strlen(symname);
	const char *prefix = NULL;
	int i, total;

	/**
	 * Adjacent dots are prohibited.
	 * Note that vfs has escaped back slashes yet.
	 */
	for (i = 0; i < symname_len - 1; ++i)
		if (symname[i] == '.' && symname[i + 1] == '.')
			goto out_fail;

	/**
	 * Check if the symname is included in the whitelist
	 * Note that we skipped cmping strlen because symname is end with '\0'
	 */
	total = sizeof(symlink_tgt_white_list) /
		sizeof(*symlink_tgt_white_list);
	for (i = 0; i < total; ++i) {
		prefix = symlink_tgt_white_list[i];
		if (!strncmp(symname, prefix, strlen(prefix)))
			goto out_succ;
	}

out_fail:
	hmdfs_err("Prohibited link path");
	return false;
out_succ:
	return true;
}

int hmdfs_symlink_local(struct inode *dir, struct dentry *dentry,
			const char *symname)
{
	int err;
	struct dentry *lower_dentry = NULL;
	struct dentry *lower_parent_dentry = NULL;
	struct path lower_path;
	struct inode *child_inode = NULL;
	struct inode *lower_dir_inode = hmdfs_i(dir)->lower_inode;
	struct hmdfs_dentry_info *gdi = hmdfs_d(dentry);
	kuid_t tmp_uid;
#ifdef CONFIG_HMDFS_1_0
	struct super_block *sb = dir->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	char *relative_dir_path = NULL;
#endif
#ifdef CONFIG_HMDFS_ANDROID
	const struct cred *saved_cred = NULL;
	struct fs_struct *saved_fs = NULL, *copied_fs = NULL;
	__u16 child_perm;
#endif

	if (unlikely(!symname_is_allowed(symname))) {
		err = -EPERM;
		goto path_err;
	}
	/* With the help of xattr info(storage src in photokit secne)
	 * by device_view dentry info,
	 * not support symlink in merge_view except xxc/photokit dir. also,
	 * old PC Collaboration path need to be confirmed
	 */
#ifdef CONFIG_HMDFS_1_0
	if (hmdfs_adapter_read_dentry_flag(hmdfs_i(dir)->lower_inode) !=
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		err = -EPERM;
		goto path_err;
	}
#endif

#ifdef CONFIG_HMDFS_ANDROID
	saved_cred = hmdfs_override_file_fsids(dir, &child_perm);
	if (!saved_cred) {
		err = -ENOMEM;
		goto path_err;
	}

	saved_fs = current->fs;
	copied_fs = hmdfs_override_fsstruct(saved_fs);
	if (!copied_fs) {
		err = -ENOMEM;
		goto revert_fsids;
	}
#endif
	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_parent_dentry = lock_parent(lower_dentry);
	tmp_uid = hmdfs_override_inode_uid(lower_dir_inode);
	err = vfs_symlink(lower_dir_inode, lower_dentry, symname);
	hmdfs_revert_inode_uid(lower_dir_inode, tmp_uid);
	unlock_dir(lower_parent_dentry);
	if (err)
		goto out_err;
	set_symlink_flag(gdi);
#ifdef CONFIG_HMDFS_ANDROID
	err = hmdfs_persist_perm(lower_dentry, &child_perm);
#endif
	child_inode = fill_inode_local(dir->i_sb, d_inode(lower_dentry));
	if (IS_ERR(child_inode)) {
		err = PTR_ERR(child_inode);
		goto out_created;
	}
	d_add(dentry, child_inode);
	fsstack_copy_attr_times(dir, lower_dir_inode);
	fsstack_copy_inode_size(dir, lower_dir_inode);

out_created:
#ifdef CONFIG_HMDFS_1_0
	relative_dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!relative_dir_path) {
		err = -ENOMEM;
		goto out_err;
	}
	if (adapter_identify_dentry_flag(dir, dentry, lower_dentry) ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG)
		hmdfs_adapter_create(sbi, dentry, relative_dir_path);
	kfree(relative_dir_path);
#endif
out_err:
	hmdfs_put_lower_path(&lower_path);
#ifdef CONFIG_HMDFS_ANDROID
	hmdfs_revert_fsstruct(saved_fs, copied_fs);
revert_fsids:
	hmdfs_revert_fsids(saved_cred);
#endif
path_err:
	trace_hmdfs_symlink_local(dir, dentry, err);
	return err;
}

static const char *hmdfs_get_link_local(struct dentry *dentry,
					struct inode *inode,
					struct delayed_call *done)
{
	const char *link = NULL;
	struct dentry *lower_dentry = NULL;
	struct inode *lower_inode = NULL;
	struct path lower_path;

	if (!dentry) {
		hmdfs_err("dentry NULL");
		link = ERR_PTR(-ECHILD);
		goto link_out;
	}

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_inode = d_inode(lower_dentry);
	if (!lower_inode->i_op || !lower_inode->i_op->get_link) {
		hmdfs_err("The lower inode doesn't support get_link i_op");
		link = ERR_PTR(-EINVAL);
		goto out;
	}

	link = lower_inode->i_op->get_link(lower_dentry, lower_inode, done);
	if (IS_ERR_OR_NULL(link))
		goto out;
	fsstack_copy_attr_atime(inode, lower_inode);
out:
	hmdfs_put_lower_path(&lower_path);
	trace_hmdfs_get_link_local(inode, dentry, PTR_ERR_OR_ZERO(link));
link_out:
	return link;
}

static int hmdfs_setattr_local(struct dentry *dentry, struct iattr *ia)
{
	struct inode *inode = d_inode(dentry);
	struct inode *lower_inode = hmdfs_i(inode)->lower_inode;
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	struct iattr lower_ia;
	unsigned int ia_valid = ia->ia_valid;
	int err = 0;
	kuid_t tmp_uid;

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	memcpy(&lower_ia, ia, sizeof(lower_ia));
	if (ia_valid & ATTR_FILE)
		lower_ia.ia_file = hmdfs_f(ia->ia_file)->lower_file;
	lower_ia.ia_valid &= ~(ATTR_UID | ATTR_GID | ATTR_MODE);
	if (ia_valid & ATTR_SIZE) {
		err = inode_newsize_ok(inode, ia->ia_size);
		if (err)
			goto out;
		truncate_setsize(inode, ia->ia_size);
	}
	inode_lock(lower_inode);
	tmp_uid = hmdfs_override_inode_uid(lower_inode);

	err = notify_change(lower_dentry, &lower_ia, NULL);
	i_size_write(inode, i_size_read(lower_inode));
	inode->i_atime = lower_inode->i_atime;
	inode->i_mtime = lower_inode->i_mtime;
	inode->i_ctime = lower_inode->i_ctime;
	err = update_inode_to_dentry(dentry, inode);
	hmdfs_revert_inode_uid(lower_inode, tmp_uid);

	inode_unlock(lower_inode);
out:
	hmdfs_put_lower_path(&lower_path);
	return err;
}

static int hmdfs_getattr_local(const struct path *path, struct kstat *stat,
			       u32 request_mask, unsigned int flags)
{
	struct path lower_path;
	int ret;

	hmdfs_get_lower_path(path->dentry, &lower_path);
	ret = vfs_getattr(&lower_path, stat, request_mask, flags);
	stat->ino = d_inode(path->dentry)->i_ino;
	hmdfs_put_lower_path(&lower_path);

	return ret;
}

int hmdfs_permission(struct inode *inode, int mask)
{
#ifdef CONFIG_HMDFS_ANDROID
	unsigned int mode = inode->i_mode;
	struct hmdfs_inode_info *hii = hmdfs_i(inode);
	kuid_t cur_uid = current_fsuid();

	if (uid_eq(cur_uid, ROOT_UID) || uid_eq(cur_uid, SYSTEM_UID))
		return 0;

	if (uid_eq(cur_uid, inode->i_uid)) {
		mode >>= 6;
	} else if (in_group_p(inode->i_gid)) {
		mode >>= 3;
	} else if (is_pkg_auth(hii->perm)) {
		kuid_t appid = get_appid_from_uid(cur_uid);

		if (uid_eq(appid, inode->i_uid))
			return 0;
	} else if (is_system_auth(hii->perm)) {
		if (in_group_p(MEDIA_RW_GID))
			return 0;
	}

	if ((mask & ~mode & (MAY_READ | MAY_WRITE | MAY_EXEC)) == 0)
		return 0;

	trace_hmdfs_permission(inode->i_ino);
	return -EACCES;
#else

	return 0;
#endif
}

static ssize_t hmdfs_local_listxattr(struct dentry *dentry, char *list,
				     size_t size)
{
	struct path lower_path;
	ssize_t res = 0;
	size_t r_size = size;

	if (!hmdfs_support_xattr(dentry))
		return -EOPNOTSUPP;

	if (size > HMDFS_LISTXATTR_SIZE_MAX)
		r_size = HMDFS_LISTXATTR_SIZE_MAX;

	hmdfs_get_lower_path(dentry, &lower_path);
	res = vfs_listxattr(lower_path.dentry, list, r_size);
	hmdfs_put_lower_path(&lower_path);

	if (res == -ERANGE && r_size != size) {
		hmdfs_info("no support listxattr size over than %d",
			   HMDFS_LISTXATTR_SIZE_MAX);
		res = -E2BIG;
	}

	return res;
}

const struct inode_operations hmdfs_symlink_iops_local = {
	.get_link = hmdfs_get_link_local,
	.permission = hmdfs_permission,
	.setattr = hmdfs_setattr_local,
};

const struct inode_operations hmdfs_dir_inode_ops_local = {
	.lookup = hmdfs_lookup_local,
	.mkdir = hmdfs_mkdir_local,
	.create = hmdfs_create_local,
	.rmdir = hmdfs_rmdir_local,
	.unlink = hmdfs_unlink_local,
	.symlink = hmdfs_symlink_local,
	.rename = hmdfs_rename_local,
	.permission = hmdfs_permission,
};

const struct inode_operations hmdfs_file_iops_local = {
	.setattr = hmdfs_setattr_local,
	.getattr = hmdfs_getattr_local,
	.permission = hmdfs_permission,
	.listxattr = hmdfs_local_listxattr,
};
