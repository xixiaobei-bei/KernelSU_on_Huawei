/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode_remote.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lousong@huawei.com maojingjing@huawei.com lvhao13@huawei.com
 * Create: 2020-12-21
 *
 */

#include <linux/fs_stack.h>
#include <linux/namei.h>
#include <linux/xattr.h>
#include <linux/string.h>

#include "comm/socket_adapter.h"
#include "hmdfs.h"
#include "hmdfs_client.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"
#include "authority/authentication.h"
#include "stash.h"
#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif

struct hmdfs_lookup_ret *lookup_remote_dentry(struct dentry *child_dentry,
					      const struct qstr *qstr,
					      uint64_t dev_id)
{
	struct hmdfs_lookup_ret *lookup_ret;
	struct hmdfs_dentry *dentry = NULL;
	struct clearcache_item *cache_item = NULL;
	struct hmdfs_dcache_lookup_ctx ctx;
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);

	cache_item = hmdfs_find_cache_item(dev_id, child_dentry->d_parent);
	if (!cache_item)
		return NULL;

	lookup_ret = kmalloc(sizeof(*lookup_ret), GFP_KERNEL);
	if (!lookup_ret)
		goto out;

	hmdfs_init_dcache_lookup_ctx(&ctx, sbi, qstr, cache_item->filp);
	dentry = hmdfs_find_dentry(child_dentry, &ctx);
	if (!dentry) {
		kfree(lookup_ret);
		lookup_ret = NULL;
		goto out;
	}

	lookup_ret->i_mode = le16_to_cpu(dentry->i_mode);
	lookup_ret->i_size = le64_to_cpu(dentry->i_size);
	lookup_ret->i_mtime = le64_to_cpu(dentry->i_mtime);
	lookup_ret->i_mtime_nsec = le32_to_cpu(dentry->i_mtime_nsec);
	lookup_ret->i_ino = le64_to_cpu(dentry->i_ino);
#ifdef CONFIG_HMDFS_1_0
	lookup_ret->fno = INVALID_FILE_ID;
#endif
	hmdfs_unlock_file(ctx.filp, get_dentry_group_pos(ctx.bidx),
			  DENTRYGROUP_SIZE);
	kfree(ctx.page);
out:
	kref_put(&cache_item->ref, release_cache_item);
	return lookup_ret;
}

/* get_remote_inode_info - fill hmdfs_lookup_ret by info from remote getattr
 *
 * @dentry: local dentry
 * @hmdfs_peer: which remote devcie
 * @flags: lookup flags
 *
 * return allocaed and initialized hmdfs_lookup_ret on success, and NULL on
 * failure.
 */
struct hmdfs_lookup_ret *get_remote_inode_info(struct hmdfs_peer *con,
					       struct dentry *dentry,
					       unsigned int flags)
{
	int err = 0;
	struct hmdfs_lookup_ret *lookup_ret = NULL;
	struct hmdfs_getattr_ret *getattr_ret = NULL;
	unsigned int expected_flags = 0;

	lookup_ret = kmalloc(sizeof(*lookup_ret), GFP_KERNEL);
	if (!lookup_ret)
		return NULL;

	err = hmdfs_remote_getattr(con, dentry, flags, &getattr_ret);
	if (err) {
		hmdfs_debug("inode info get failed with err %d", err);
		kfree(lookup_ret);
		return NULL;
	}
	/* make sure we got everything we need */
	expected_flags = STATX_INO | STATX_SIZE | STATX_MODE | STATX_MTIME;
	if ((getattr_ret->stat.result_mask & expected_flags) !=
	    expected_flags) {
		hmdfs_debug("remote getattr failed with flag %x",
			    getattr_ret->stat.result_mask);
		kfree(lookup_ret);
		kfree(getattr_ret);
		return NULL;
	}

	lookup_ret->i_mode = getattr_ret->stat.mode;
	lookup_ret->i_size = getattr_ret->stat.size;
	lookup_ret->i_mtime = getattr_ret->stat.mtime.tv_sec;
	lookup_ret->i_mtime_nsec = getattr_ret->stat.mtime.tv_nsec;
	lookup_ret->i_ino = getattr_ret->stat.ino;
#ifdef CONFIG_HMDFS_1_0
	lookup_ret->fno = INVALID_FILE_ID;
#endif
	kfree(getattr_ret);
	return lookup_ret;
}

static void hmdfs_remote_readdir_work(struct work_struct *work)
{
	struct hmdfs_readdir_work *rw =
		container_of(to_delayed_work(work), struct hmdfs_readdir_work,
			     dwork);
	struct dentry *dentry = rw->dentry;
	struct hmdfs_peer *con = rw->con;
	const struct cred *old_cred = hmdfs_override_creds(con->sbi->cred);
	bool empty = false;

	get_remote_dentry_file(dentry, con);
	hmdfs_d(dentry)->async_readdir_in_progress = false;
	hmdfs_revert_creds(old_cred);

	dput(dentry);
	peer_put(con);
	spin_lock(&con->sbi->async_readdir_work_lock);
	list_del(&rw->head);
	empty = list_empty(&con->sbi->async_readdir_work_list);
	spin_unlock(&con->sbi->async_readdir_work_lock);
	kfree(rw);

	if (empty)
		wake_up_interruptible(&con->sbi->async_readdir_wq);
}

static void get_remote_dentry_file_in_wq(struct dentry *dentry,
					 struct hmdfs_peer *con)
{
	struct hmdfs_readdir_work *rw = NULL;

	/* do nothing if async readdir is already in progress */
	if (cmpxchg_relaxed(&hmdfs_d(dentry)->async_readdir_in_progress, false,
			     true))
		return;

	rw = kmalloc(sizeof(*rw), GFP_KERNEL);
	if (!rw) {
		hmdfs_d(dentry)->async_readdir_in_progress = false;
		return;
	}

	dget(dentry);
	peer_get(con);
	rw->dentry = dentry;
	rw->con = con;
	spin_lock(&con->sbi->async_readdir_work_lock);
	INIT_DELAYED_WORK(&rw->dwork, hmdfs_remote_readdir_work);
	list_add(&rw->head, &con->sbi->async_readdir_work_list);
	spin_unlock(&con->sbi->async_readdir_work_lock);
	queue_delayed_work(con->dentry_wq, &rw->dwork, 0);
}

void get_remote_dentry_file_sync(struct dentry *dentry, struct hmdfs_peer *con)
{
	get_remote_dentry_file_in_wq(dentry, con);
	flush_workqueue(con->dentry_wq);
}

struct hmdfs_lookup_ret *hmdfs_lookup_by_con(struct hmdfs_peer *con,
					     struct dentry *dentry,
					     struct qstr *qstr,
					     unsigned int flags,
					     const char *relative_path)
{
	struct hmdfs_lookup_ret *result = NULL;

	if (con->version > USERSPACE_MAX_VER) {
		/*
		 * LOOKUP_REVAL means we found stale info from dentry file, thus
		 * we need to use remote getattr.
		 */
		if (flags & LOOKUP_REVAL) {
			/*
			 * HMDFS_LOOKUP_REVAL means we need to skip dentry cache
			 * in lookup, because dentry cache in server might have
			 * stale data.
			 */
			result = get_remote_inode_info(con, dentry,
						       HMDFS_LOOKUP_REVAL);
			get_remote_dentry_file_in_wq(dentry->d_parent, con);
			return result;
		}

		/* If cache file is still valid */
		if (hmdfs_cache_revalidate(READ_ONCE(con->conn_time),
					   con->device_id, dentry->d_parent)) {
			result = lookup_remote_dentry(dentry, qstr,
						      con->device_id);
			/*
			 * If lookup from cache file failed, use getattr to see
			 * if remote have created the file.
			 */
			if (!(flags & (LOOKUP_CREATE | LOOKUP_RENAME_TARGET)) &&
			    !result)
				result = get_remote_inode_info(con, dentry, 0);
			/* If cache file expired, use getattr directly
			 * except create and rename opt
			 */
		} else {
			result = get_remote_inode_info(con, dentry, 0);
			get_remote_dentry_file_in_wq(dentry->d_parent, con);
		}
	} else {
		if (!relative_path)
			return NULL;

		result = con->conn_operations->remote_lookup(
			con, relative_path, dentry->d_name.name);
	}

	return result;
}

/*
 * hmdfs_update_inode_size - update inode size when finding aready existed
 * inode.
 *
 * First of all, if the file is opened for writing, we don't update inode size
 * here, because inode size is about to be changed after writing.
 *
 * If the file is not opened, simply update getattr_isize(not actual inode size,
 * just a value showed to user). This is safe because inode size will be
 * up-to-date after open.
 *
 * If the file is opened for read:
 * a. getattr_isize == HMDFS_STALE_REMOTE_ISIZE
 *   1) i_size == new_size, nothing need to be done.
 *   2) i_size > new_size, we keep the i_size and set getattr_isize to new_size,
 *      stale data might be readed in this case, which is fine because file is
 *      opened before remote truncate the file.
 *   3) i_size < new_size, we drop the last page of the file if i_size is not
 *      aligned to PAGE_SIZE, clear getattr_isize, and update i_size to
 *      new_size.
 * b. getattr_isize != HMDFS_STALE_REMOTE_ISIZE, getattr_isize will only be set
 *    after 2).
 *   4) getattr_isize > i_size, this situation is impossible.
 *   5) i_size >= new_size, this case is the same as 2).
 *   6) i_size < new_size, this case is the same as 3).
 */
static void hmdfs_update_inode_size(struct inode *inode, uint64_t new_size)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	int writecount;
	uint64_t size;

	inode_lock(inode);
	size = info->getattr_isize;
	if (size == HMDFS_STALE_REMOTE_ISIZE)
		size = i_size_read(inode);
	if (size == new_size) {
		inode_unlock(inode);
		return;
	}

	writecount = atomic_read(&inode->i_writecount);
	/* check if writing is in progress */
	if (writecount > 0) {
		info->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
		inode_unlock(inode);
		return;
	}

	/* check if there is no one who opens the file */
	if (kref_read(&info->ref) == 0)
		goto update_info;

	/* check if there is someone who opens the file for read */
	if (writecount == 0) {
		uint64_t aligned_size;

		/* use inode size here instead of getattr_isize */
		size = i_size_read(inode);
		if (new_size <= size)
			goto update_info;
		/*
		 * if the old inode size is not aligned to HMDFS_PAGE_SIZE, we
		 * need to drop the last page of the inode, otherwise zero will
		 * be returned while reading the new range in the page after
		 * chaning inode size.
		 */
		aligned_size = round_down(size, HMDFS_PAGE_SIZE);
		if (aligned_size != size)
			truncate_inode_pages(inode->i_mapping, aligned_size);
		i_size_write(inode, new_size);
		info->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
		inode_unlock(inode);
		return;
	}

update_info:
	info->getattr_isize = new_size;
	inode_unlock(inode);
}

static void hmdfs_update_inode(struct inode *inode,
			       struct hmdfs_lookup_ret *lookup_result)
{
	struct hmdfs_time_t remote_mtime = {
		.tv_sec = lookup_result->i_mtime,
		.tv_nsec = lookup_result->i_mtime_nsec,
	};

	/*
	 * We only update mtime if the file is not opened for writing. If we do
	 * update it before writing is about to start, user might see the mtime
	 * up-and-down if system time in server and client do not match. However
	 * mtime in client will eventually match server after timeout without
	 * writing.
	 */
	if (!inode_is_open_for_write(inode))
		inode->i_mtime = remote_mtime;

	/*
	 * We don't care i_size of dir, and lock inode for dir
	 * might cause deadlock.
	 */
	if (S_ISREG(inode->i_mode))
		hmdfs_update_inode_size(inode, lookup_result->i_size);
}

static void hmdfs_fill_inode_android(struct inode *inode, struct inode *dir,
				     umode_t mode)
{
#ifdef CONFIG_HMDFS_ANDROID
	inode->i_uid = dir->i_uid;
	inode->i_gid = dir->i_gid;
#endif
}

struct inode *fill_inode_remote(struct super_block *sb, struct hmdfs_peer *con,
				struct hmdfs_lookup_ret *res, struct inode *dir)
{
	struct inode *inode = NULL;
	struct hmdfs_inode_info *info;
	umode_t mode = res->i_mode;

	if (con->version > USERSPACE_MAX_VER)
		inode = hmdfs_iget5_locked_remote(sb, con, res->i_ino);
	else
		inode = hmdfs_iget_locked_dfs_1_0(sb, con);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	info = hmdfs_i(inode);
	info->inode_type = HMDFS_LAYER_OTHER_REMOTE;
	if (con->version > USERSPACE_MAX_VER) {
		/* the inode was found in cache */
		if (!(inode->i_state & I_NEW)) {
			hmdfs_fill_inode_android(inode, dir, mode);
			hmdfs_update_inode(inode, res);
			return inode;
		}

		hmdfs_remote_init_stash_status(con, inode, mode);
	}

#ifdef CONFIG_HMDFS_1_0
	info->file_no = res->fno;
#endif
	inode->i_ctime.tv_sec = 0;
	inode->i_ctime.tv_nsec = 0;
	inode->i_mtime.tv_sec = res->i_mtime;
	inode->i_mtime.tv_nsec = res->i_mtime_nsec;

	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);

	if (S_ISDIR(mode))
		inode->i_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH;
	else if (S_ISREG(mode))
		inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	else if (S_ISLNK(mode))
		inode->i_mode = S_IFREG | S_IRWXU | S_IRWXG;

	if (S_ISREG(mode) || S_ISLNK(mode)) { // Reguler file
		inode->i_op = con->conn_operations->remote_file_iops;
		inode->i_fop = con->conn_operations->remote_file_fops;
		inode->i_size = res->i_size;
		set_nlink(inode, 1);
	} else if (S_ISDIR(mode)) { // Directory
		inode->i_op = &hmdfs_dev_dir_inode_ops_remote;
		inode->i_fop = &hmdfs_dev_dir_ops_remote;
		set_nlink(inode, 2);
	}
	inode->i_mapping->a_ops = con->conn_operations->remote_file_aops;

	hmdfs_fill_inode_android(inode, dir, mode);
	unlock_new_inode(inode);
	return inode;
}

static struct dentry *hmdfs_lookup_remote_dentry(struct inode *parent_inode,
						 struct dentry *child_dentry,
						 int flags)
{
	struct dentry *ret = NULL;
	struct inode *inode = NULL;
	struct super_block *sb = parent_inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	struct hmdfs_lookup_ret *lookup_result = NULL;
	struct hmdfs_peer *con = NULL;
	char *file_name = NULL;
	int file_name_len = child_dentry->d_name.len;
	struct qstr qstr;
	struct hmdfs_dentry_info *gdi = hmdfs_d(child_dentry);
	uint64_t device_id = 0;
	char *relative_path = NULL;

	file_name = kzalloc(NAME_MAX + 1, GFP_KERNEL);
	if (!file_name)
		return ERR_PTR(-ENOMEM);
	strncpy(file_name, child_dentry->d_name.name, file_name_len);

	qstr.name = file_name;
	qstr.len = strlen(file_name);

	device_id = gdi->device_id;
	con = hmdfs_lookup_from_devid(sbi, device_id);
	if (!con) {
		ret = ERR_PTR(-ESHUTDOWN);
		goto done;
	}

	relative_path = hmdfs_get_dentry_relative_path(child_dentry->d_parent);
	if (unlikely(!relative_path)) {
		ret = ERR_PTR(-ENOMEM);
		hmdfs_err("get relative path failed %d", -ENOMEM);
		goto done;
	}

	lookup_result = hmdfs_lookup_by_con(con, child_dentry, &qstr, flags,
					    relative_path);
	if (lookup_result != NULL) {
		if (S_ISLNK(lookup_result->i_mode))
			gdi->file_type = HM_SYMLINK;
		inode = fill_inode_remote(sb, con, lookup_result, parent_inode);
		ret = d_splice_alias(inode, child_dentry);
		if (!IS_ERR_OR_NULL(ret))
			child_dentry = ret;
		if (!IS_ERR(ret))
			check_and_fixup_ownership_remote(parent_inode,
							 child_dentry);
	} else {
		ret = ERR_PTR(-ENOENT);
	}

done:
	if (con)
		peer_put(con);
	kfree(relative_path);
	kfree(lookup_result);
	kfree(file_name);
	return ret;
}

struct dentry *hmdfs_lookup_remote(struct inode *parent_inode,
				   struct dentry *child_dentry,
				   unsigned int flags)
{
	int err = 0;
	struct dentry *ret = NULL;
	struct hmdfs_dentry_info *gdi = NULL;
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);

	trace_hmdfs_lookup_remote(parent_inode, child_dentry, flags);
	if (child_dentry->d_name.len > NAME_MAX) {
		err = -ENAMETOOLONG;
		ret = ERR_PTR(-ENAMETOOLONG);
		goto out;
	}
#ifdef CONFIG_HMDFS_1_0
	dentry_syncer_wakeup();
#endif
	err = init_hmdfs_dentry_info(sbi, child_dentry,
				     HMDFS_LAYER_OTHER_REMOTE);
	if (err) {
		ret = ERR_PTR(err);
		goto out;
	}
	gdi = hmdfs_d(child_dentry);
	gdi->device_id = hmdfs_d(child_dentry->d_parent)->device_id;

	if (is_current_hmdfs_server_ctx())
		goto out;

	ret = hmdfs_lookup_remote_dentry(parent_inode, child_dentry, flags);
	/*
	 * don't return error if inode do not exist, so that vfs can continue
	 * to create it.
	 */
	if (IS_ERR_OR_NULL(ret)) {
		err = PTR_ERR(ret);
		if (err == -ENOENT)
			ret = NULL;
	} else {
		child_dentry = ret;
	}

out:
	if (!err)
		hmdfs_set_time(child_dentry, jiffies);
	trace_hmdfs_lookup_remote_end(parent_inode, child_dentry, err);
	return ret;
}

/* delete dentry in cache file */
void delete_in_cache_file(uint64_t dev_id, struct dentry *dentry)
{
	struct clearcache_item *item = NULL;

	item = hmdfs_find_cache_item(dev_id, dentry->d_parent);
	if (item) {
		hmdfs_delete_dentry(dentry, item->filp);
		kref_put(&item->ref, release_cache_item);
	} else {
		hmdfs_info("find cache item failed, con:%llu", dev_id);
	}
}

int hmdfs_mkdir_remote_dentry(struct hmdfs_peer *conn, struct dentry *dentry,
			      umode_t mode)
{
	int err = 0;
	char *dir_path = NULL;
	struct dentry *parent_dentry = dentry->d_parent;
	struct inode *parent_inode = d_inode(parent_dentry);
	struct super_block *sb = parent_inode->i_sb;
	const unsigned char *d_name = dentry->d_name.name;
	struct hmdfs_lookup_ret *mkdir_ret = NULL;
	struct inode *inode = NULL;

	mkdir_ret = kmalloc(sizeof(*mkdir_ret), GFP_KERNEL);
	if (!mkdir_ret) {
		err = -ENOMEM;
		return err;
	}
	dir_path = hmdfs_get_dentry_relative_path(parent_dentry);
	if (!dir_path) {
		err = -EACCES;
		goto mkdir_out;
	}
	err = hmdfs_client_start_mkdir(conn, dir_path, d_name, mode, mkdir_ret);
	if (err) {
		hmdfs_err("hmdfs_client_start_mkdir failed err = %d", err);
		goto mkdir_out;
	}
	if (mkdir_ret) {
		inode = fill_inode_remote(sb, conn, mkdir_ret, parent_inode);
		if (!IS_ERR(inode))
			d_add(dentry, inode);
		else
			err = PTR_ERR(inode);
		check_and_fixup_ownership_remote(parent_inode, dentry);
	} else {
		err = -ENOENT;
	}

mkdir_out:
	kfree(dir_path);
	kfree(mkdir_ret);
	return err;
}

int hmdfs_mkdir_remote(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	int err = 0;
	struct hmdfs_inode_info *info = hmdfs_i(dir);
	struct hmdfs_peer *con = info->conn;

	if (!con) {
		hmdfs_warning("qpb_debug: con is null!");
		goto out;
	}
	if (con->version <= USERSPACE_MAX_VER) {
		err = -EPERM;
		goto out;
	}
	err = hmdfs_mkdir_remote_dentry(con, dentry, mode);
	if (!err)
		create_in_cache_file(con->device_id, dentry);
	else
		hmdfs_err("remote mkdir failed err = %d", err);

out:
	trace_hmdfs_mkdir_remote(dir, dentry, err);
	return err;
}

int hmdfs_create_remote_dentry(struct hmdfs_peer *conn, struct dentry *dentry,
			       umode_t mode, bool want_excl)
{
	int err = 0;
	char *dir_path = NULL;
	struct dentry *parent_dentry = dentry->d_parent;
	struct inode *parent_inode = d_inode(parent_dentry);
	struct super_block *sb = parent_inode->i_sb;
	const unsigned char *d_name = dentry->d_name.name;
	struct hmdfs_lookup_ret *create_ret = NULL;
	struct inode *inode = NULL;

	create_ret = kmalloc(sizeof(*create_ret), GFP_KERNEL);
	if (!create_ret) {
		err = -ENOMEM;
		return err;
	}
	dir_path = hmdfs_get_dentry_relative_path(parent_dentry);
	if (!dir_path) {
		err = -EACCES;
		goto create_out;
	}
	err = hmdfs_client_start_create(conn, dir_path, d_name, mode,
					want_excl, create_ret);
	if (err) {
		hmdfs_err("hmdfs_client_start_create failed err = %d", err);
		goto create_out;
	}
	if (create_ret) {
		inode = fill_inode_remote(sb, conn, create_ret, parent_inode);
		if (!IS_ERR(inode))
			d_add(dentry, inode);
		else
			err = PTR_ERR(inode);
		check_and_fixup_ownership_remote(parent_inode, dentry);
	} else {
		err = -ENOENT;
		hmdfs_err("get remote inode info failed err = %d", err);
	}

create_out:
	kfree(dir_path);
	kfree(create_ret);
	return err;
}

int hmdfs_create_remote(struct inode *dir, struct dentry *dentry, umode_t mode,
			bool want_excl)
{
	int err = 0;
	struct hmdfs_inode_info *info = hmdfs_i(dir);
	struct hmdfs_peer *con = info->conn;

	if (!con) {
		hmdfs_warning("qpb_debug: con is null!");
		goto out;
	}
	if (con->version <= USERSPACE_MAX_VER) {
		err = -EPERM;
		goto out;
	}
	err = hmdfs_create_remote_dentry(con, dentry, mode, want_excl);
	if (!err)
		create_in_cache_file(con->device_id, dentry);
	else
		hmdfs_err("remote create failed err = %d", err);

out:
	trace_hmdfs_create_remote(dir, dentry, err);
	return err;
}

int hmdfs_rmdir_remote_dentry(struct hmdfs_peer *conn, struct dentry *dentry)
{
	int error = 0;
	char *dir_path = NULL;
	const char *dentry_name = dentry->d_name.name;

	dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!dir_path) {
		error = -EACCES;
		goto rmdir_out;
	}

	error = hmdfs_client_start_rmdir(conn, dir_path, dentry_name);
	if (!error)
		delete_in_cache_file(conn->device_id, dentry);

rmdir_out:
	kfree(dir_path);
	return error;
}

int hmdfs_rmdir_remote(struct inode *dir, struct dentry *dentry)
{
	int err = 0;
	struct hmdfs_inode_info *info = hmdfs_i(dentry->d_inode);
	struct hmdfs_peer *con = info->conn;

	if (!con)
		goto out;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		err = -EACCES;
		goto out;
	}
	if (con->version <= USERSPACE_MAX_VER) {
		err = -EPERM;
		goto out;
	}
	err = hmdfs_rmdir_remote_dentry(con, dentry);
	/* drop dentry even remote failed
	 * it maybe cause that one remote devices disconnect
	 * when doing remote rmdir
	 */
	d_drop(dentry);
out:
	/* return connect device's errcode */
	trace_hmdfs_rmdir_remote(dir, dentry, err);
	return err;
}

int hmdfs_dev_unlink_from_con(struct hmdfs_peer *conn, struct dentry *dentry)
{
	int error = 0;
	char *dir_path = NULL;
	const char *dentry_name = dentry->d_name.name;

	dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!dir_path) {
		error = -EACCES;
		goto unlink_out;
	}
	error = hmdfs_client_start_unlink(conn, dir_path, dentry_name);
	if (!error) {
		delete_in_cache_file(conn->device_id, dentry);
		drop_nlink(d_inode(dentry));
		d_drop(dentry);
	}
unlink_out:
	kfree(dir_path);
	return error;
}

int hmdfs_unlink_remote(struct inode *dir, struct dentry *dentry)
{
	struct hmdfs_inode_info *info = hmdfs_i(dentry->d_inode);
	struct hmdfs_peer *conn = info->conn;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON)
		return -EACCES;

	if (!conn)
		return 0;

	if (conn->status != NODE_STAT_ONLINE)
		return 0;

	return conn->conn_operations->remote_unlink(conn, dentry);
}

/* rename dentry in cache file */
static void rename_in_cache_file(uint64_t dev_id, struct dentry *old_dentry,
				 struct dentry *new_dentry)
{
	struct clearcache_item *old_item = NULL;
	struct clearcache_item *new_item = NULL;

	old_item = hmdfs_find_cache_item(dev_id, old_dentry->d_parent);
	new_item = hmdfs_find_cache_item(dev_id, new_dentry->d_parent);
	if (old_item != NULL && new_item != NULL) {
		hmdfs_rename_dentry(old_dentry, new_dentry, old_item->filp,
				    new_item->filp);
	} else if (old_item != NULL) {
		hmdfs_err("new cache item find failed!");
	} else if (new_item != NULL) {
		hmdfs_err("old cache item find failed!");
	} else {
		hmdfs_err("both cache item find failed!");
	}

	if (old_item)
		kref_put(&old_item->ref, release_cache_item);
	if (new_item)
		kref_put(&new_item->ref, release_cache_item);
}

int hmdfs_rename_remote(struct inode *old_dir, struct dentry *old_dentry,
			struct inode *new_dir, struct dentry *new_dentry,
			unsigned int flags)
{
	int err = 0;
	int ret = 0;
	const char *old_dentry_d_name = old_dentry->d_name.name;
	char *relative_old_dir_path = 0;
	const char *new_dentry_d_name = new_dentry->d_name.name;
	char *relative_new_dir_path = 0;
	struct hmdfs_inode_info *info = hmdfs_i(old_dentry->d_inode);
	struct hmdfs_peer *con = info->conn;
#ifdef CONFIG_HMDFS_1_0
	struct super_block *sb = old_dir->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
#endif

	trace_hmdfs_rename_remote(old_dir, old_dentry, new_dir, new_dentry,
				  flags);

	if (flags & ~RENAME_NOREPLACE)
		return -EINVAL;

	if (hmdfs_file_type(old_dentry->d_name.name) != HMDFS_TYPE_COMMON ||
	    hmdfs_file_type(new_dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		return -EACCES;
	}

	relative_old_dir_path =
		hmdfs_get_dentry_relative_path(old_dentry->d_parent);
	relative_new_dir_path =
		hmdfs_get_dentry_relative_path(new_dentry->d_parent);
	if (!relative_old_dir_path || !relative_new_dir_path) {
		err = -EACCES;
		goto rename_out;
	}
	if (S_ISREG(old_dentry->d_inode->i_mode)) {
		if (con->version > USERSPACE_MAX_VER) {
			hmdfs_debug("send MSG to remote devID %llu",
				    con->device_id);
			err = hmdfs_client_start_rename(
				con, relative_old_dir_path, old_dentry_d_name,
				relative_new_dir_path, new_dentry_d_name,
				flags);
			if (!err)
				rename_in_cache_file(con->device_id, old_dentry,
						     new_dentry);
		}
	} else if (S_ISDIR(old_dentry->d_inode->i_mode)) {
		if ((con->status == NODE_STAT_ONLINE) &&
		    (con->version > USERSPACE_MAX_VER)) {
			ret = hmdfs_client_start_rename(
				con, relative_old_dir_path, old_dentry_d_name,
				relative_new_dir_path, new_dentry_d_name,
				flags);
			if (!ret)
				rename_in_cache_file(con->device_id, old_dentry,
						     new_dentry);
			else
				err = ret;
		}
	}
	if (!err)
		d_invalidate(old_dentry);
#ifdef CONFIG_HMDFS_1_0
	if (con->version <= USERSPACE_MAX_VER)
		hmdfs_adapter_rename(sbi, relative_old_dir_path,
				     old_dentry, relative_new_dir_path,
				     new_dentry);
#endif
rename_out:
	kfree(relative_old_dir_path);
	kfree(relative_new_dir_path);
	return err;
}

static int hmdfs_dir_setattr_remote(struct dentry *dentry, struct iattr *ia)
{
	// Do not support dir setattr
	return 0;
}

const struct inode_operations hmdfs_dev_dir_inode_ops_remote = {
	.lookup = hmdfs_lookup_remote,
	.mkdir = hmdfs_mkdir_remote,
	.create = hmdfs_create_remote,
	.rmdir = hmdfs_rmdir_remote,
	.unlink = hmdfs_unlink_remote,
	.rename = hmdfs_rename_remote,
	.setattr = hmdfs_dir_setattr_remote,
	.permission = hmdfs_permission,
};

static int hmdfs_setattr_remote(struct dentry *dentry, struct iattr *ia)
{
	struct hmdfs_inode_info *info = hmdfs_i(d_inode(dentry));
	struct hmdfs_peer *conn = info->conn;
	struct inode *inode = d_inode(dentry);
	char *send_buf = NULL;
	int err = 0;

	if (hmdfs_inode_is_stashing(info))
		return -EAGAIN;

	send_buf = hmdfs_get_dentry_relative_path(dentry);
	if (!send_buf) {
		err = -ENOMEM;
		goto out_free;
	}
	if (ia->ia_valid & ATTR_SIZE) {
		err = inode_newsize_ok(inode, ia->ia_size);
		if (err)
			goto out_free;
		truncate_setsize(inode, ia->ia_size);
		info->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
	}
	if (ia->ia_valid & ATTR_MTIME)
		inode->i_mtime = ia->ia_mtime;

	if ((ia->ia_valid & ATTR_SIZE) || (ia->ia_valid & ATTR_MTIME)) {
		struct setattr_info send_setattr_info = {
			.size = cpu_to_le64(ia->ia_size),
			.valid = cpu_to_le32(ia->ia_valid),
			.mtime = cpu_to_le64(ia->ia_mtime.tv_sec),
			.mtime_nsec = cpu_to_le32(ia->ia_mtime.tv_nsec),
		};
		err = hmdfs_send_setattr(conn, send_buf, &send_setattr_info);
	}
out_free:
	kfree(send_buf);
	return err;
}

int hmdfs_remote_getattr(struct hmdfs_peer *conn, struct dentry *dentry,
			 unsigned int lookup_flags,
			 struct hmdfs_getattr_ret **result)
{
	char *send_buf = NULL;
	struct hmdfs_getattr_ret *attr = NULL;
	int err = 0;

	if (dentry->d_sb != conn->sbi->sb || !result)
		return -EINVAL;

	attr = kzalloc(sizeof(*attr), GFP_KERNEL);
	if (!attr)
		return -ENOMEM;

	send_buf = hmdfs_get_dentry_relative_path(dentry);
	if (!send_buf) {
		kfree(attr);
		return -ENOMEM;
	}

	err = hmdfs_send_getattr(conn, send_buf, lookup_flags, attr);
	kfree(send_buf);

	if (err) {
		kfree(attr);
		return err;
	}

	*result = attr;
	return 0;
}

static int hmdfs_get_cached_attr_remote(const struct path *path,
					struct kstat *stat, u32 request_mask,
					unsigned int flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	uint64_t size = info->getattr_isize;

	stat->ino = inode->i_ino;
	stat->mtime = inode->i_mtime;
	stat->mode = inode->i_mode;
	stat->uid.val = inode->i_uid.val;
	stat->gid.val = inode->i_gid.val;
	if (size == HMDFS_STALE_REMOTE_ISIZE)
		size = i_size_read(inode);

	stat->size = size;
	return 0;
}

ssize_t hmdfs_remote_listxattr(struct dentry *dentry, char *list, size_t size)
{
	struct inode *inode = d_inode(dentry);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_peer *conn = info->conn;
	char *send_buf = NULL;
	ssize_t res = 0;
	size_t r_size = size;

	if (!hmdfs_support_xattr(dentry))
		return -EOPNOTSUPP;

	if (size > HMDFS_LISTXATTR_SIZE_MAX)
		r_size = HMDFS_LISTXATTR_SIZE_MAX;

	send_buf = hmdfs_get_dentry_relative_path(dentry);
	if (!send_buf)
		return -ENOMEM;

	res = hmdfs_send_listxattr(conn, send_buf, list, r_size);
	kfree(send_buf);

	if (res == -ERANGE && r_size != size) {
		hmdfs_info("no support listxattr size over than %d",
			   HMDFS_LISTXATTR_SIZE_MAX);
		res = -E2BIG;
	}

	return res;
}

const struct inode_operations hmdfs_dev_file_iops_remote = {
	.setattr = hmdfs_setattr_remote,
	.permission = hmdfs_permission,
	.getattr = hmdfs_get_cached_attr_remote,
	.listxattr = hmdfs_remote_listxattr,
};
