/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_server.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: maojingjing@huawei.com lousong@huawei.com
 * Create: 2020-04-09
 *
 */

#include "hmdfs_server.h"

#include <linux/file.h>
#include <linux/xattr.h>
#include <linux/namei.h>
#include <linux/statfs.h>
#include <linux/mount.h>

#include "authority/authentication.h"
#include "comm/fault_inject.h"
#include "hmdfs.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"
#include "server_writeback.h"
#include "comm/node_cb.h"

#define HMDFS_MAX_HIDDEN_DIR	1

enum {
	HMDFS_NOT_HIDDEN_DIR = 0,
	HMDFS_IS_HIDDEN_DIR,
	HMDFS_IN_HIDDEN_DIR,
};

struct hmdfs_open_info {
	struct file *file;
	struct inode *inode;
	bool stat_valid;
	struct kstat stat;
	uint64_t real_ino;
	int file_id;
};

static int insert_file_into_conn(struct hmdfs_peer *conn, struct file *file)
{
	struct idr *idr = &(conn->file_id_idr);
	int ret;

	idr_preload(GFP_KERNEL);
	spin_lock(&(conn->file_id_lock));
	ret = idr_alloc_cyclic(idr, file, 0, 0, GFP_NOWAIT);
	spin_unlock(&(conn->file_id_lock));
	idr_preload_end();
	return ret;
}

/*
 * get_file_from_conn - get file from conn by file_id. It should be noted that
 * an aditional reference will be acquired for returned file, the called should
 * put it after the file is not used anymore.
 */
static struct file *get_file_from_conn(struct hmdfs_peer *conn, __u32 file_id)
{
	struct file *file;
	struct idr *idr = &(conn->file_id_idr);

	rcu_read_lock();
	file = idr_find(idr, file_id);
	if (file && !get_file_rcu(file))
		file = NULL;
	rcu_read_unlock();
	return file;
}

void remove_file_from_conn(struct hmdfs_peer *conn, __u32 file_id)
{
	spinlock_t *lock = &(conn->file_id_lock);
	struct idr *idr = &(conn->file_id_idr);

	spin_lock(lock);
	idr_remove(idr, file_id);
	spin_unlock(lock);
}

struct file *hmdfs_open_photokit_path(struct hmdfs_sb_info *sbi,
				      const char *path)
{
	struct file *file;
	int err;
	const char *root_name = sbi->local_dst;
	char *real_path;
	int path_len;

	path_len = strlen(root_name) + strlen(path) + 2;
	if (path_len >= PATH_MAX) {
		err = -EINVAL;
		return ERR_PTR(err);
	}
	real_path = kzalloc(path_len, GFP_KERNEL);
	if (!real_path) {
		err = -ENOMEM;
		return ERR_PTR(err);
	}

	sprintf(real_path, "%s/%s", root_name, path);
	file = filp_open(real_path, O_RDWR | O_LARGEFILE, 0644);
	if (IS_ERR(file)) {
		hmdfs_info("filp_open failed: %ld", PTR_ERR(file));
	} else {
		hmdfs_info("get file with magic %lu",
			   file->f_inode->i_sb->s_magic);
	}

	kfree(real_path);
	return file;
}

struct file *hmdfs_open_path(struct hmdfs_sb_info *sbi, const char *path)
{
	struct path root_path;
	struct file *file;
	int err;
	const char *root_name = sbi->local_dst;

	err = kern_path(root_name, 0, &root_path);
	if (err) {
		hmdfs_info("kern_path failed: %d", err);
		return ERR_PTR(err);
	}
	file = file_open_root(root_path.dentry, root_path.mnt, path,
			      O_RDWR | O_LARGEFILE, 0644);
	path_put(&root_path);
	if (IS_ERR(file)) {
		hmdfs_err(
			"GRAPERR sb->s_readonly_remount %d sb_flag %lu mnt %px sb %px",
			sbi->sb->s_readonly_remount, sbi->sb->s_flags,
			root_path.mnt, sbi->sb);
		hmdfs_info("file_open_root failed: %ld", PTR_ERR(file));
	} else {
		hmdfs_info("get file with magic %lu",
			   file->f_inode->i_sb->s_magic);
	}
	return file;
}

inline void hmdfs_close_path(struct file *file)
{
	fput(file);
}

/* After offline server close all files opened by client */
void hmdfs_server_offline_notify(struct hmdfs_peer *conn, int evt,
				 unsigned int seq)
{
	int id;
	int count = 0;
	unsigned int next;
	struct file *filp = NULL;
	struct idr *idr = &conn->file_id_idr;

	/* wait all async work complete */
	flush_workqueue(conn->req_handle_wq);
	flush_workqueue(conn->async_wq);

	/* If there is some open requests in processing,
	 * Maybe, we need to close file when peer offline
	 */
	idr_for_each_entry(idr, filp, id) {
		hmdfs_debug("[%d]Server close: id=%d", count, id);
		hmdfs_close_path(filp);
		count++;
		if (count % HMDFS_IDR_RESCHED_COUNT == 0)
			cond_resched();
	}

	/* Reinitialize idr */
	next = idr_get_cursor(idr);
	idr_destroy(idr);

	idr_init(idr);
	idr_set_cursor(idr, next);

	/* Make old file id to be stale */
	conn->fid_cookie++;
}

static struct hmdfs_node_cb_desc server_cb[] = {
	{
		.evt = NODE_EVT_OFFLINE,
		.sync = true,
		.min_version = DFS_2_0,
		.fn = hmdfs_server_offline_notify
	},
};

void __init hmdfs_server_add_node_evt_cb(void)
{
	hmdfs_node_add_evt_cb(server_cb, ARRAY_SIZE(server_cb));
}

static int hmdfs_get_inode_by_name(struct hmdfs_peer *con, const char *filename,
				   uint64_t *ino)
{
	int ret = 0;
	struct path root_path;
	struct path dst_path;
	struct inode *inode = NULL;

	ret = kern_path(con->sbi->local_dst, 0, &root_path);
	if (ret) {
		hmdfs_err("kern_path failed err = %d", ret);
		return ret;
	}

	ret = vfs_path_lookup(root_path.dentry, root_path.mnt, filename, 0,
			      &dst_path);
	if (ret) {
		path_put(&root_path);
		return ret;
	}

	inode = d_inode(dst_path.dentry);
	if (con->sbi->sb == inode->i_sb)
		inode = hmdfs_i(inode)->lower_inode;
	*ino = generate_u64_ino(inode->i_ino, inode->i_generation);

	path_put(&dst_path);
	path_put(&root_path);

	return 0;
}

static struct file *hmdfs_open_file(struct hmdfs_peer *con,
				    const char *filename, uint8_t file_type,
				    int *file_id)
{
	struct file *file = NULL;
	int id;

	if (!filename) {
		hmdfs_err("filename is NULL");
		return ERR_PTR(-EINVAL);
	}

	if (hm_islnk(file_type))
		file = hmdfs_open_photokit_path(con->sbi, filename);
	else
		file = hmdfs_open_path(con->sbi, filename);
	if (IS_ERR(file))
		return file;

	id = insert_file_into_conn(con, file);
	if (id < 0) {
		hmdfs_err("file_id alloc failed! err=%d", id);
		hmdfs_close_path(file);
		return ERR_PTR(id);
	}
	*file_id = id;

	return file;
}

static struct hmdfs_time_t msec_to_timespec(unsigned int msec)
{
	struct hmdfs_time_t timespec = {
		.tv_sec = msec / MSEC_PER_SEC,
		.tv_nsec = (msec % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	return timespec;
}

static struct hmdfs_time_t hmdfs_current_kernel_time(void)
{
	struct hmdfs_time_t time;

#if KERNEL_VERSION(4, 18, 0) < LINUX_VERSION_CODE
	ktime_get_coarse_real_ts64(&time);
#else
	time = current_kernel_time();
#endif
	return time;
}

/*
 * Generate fid version like following format:
 *
 * |     boot cookie     |  con cookie |
 * |---------------------|-------------|
 *           49                15       (bits)
 */
static uint64_t hmdfs_server_pack_fid_ver(struct hmdfs_peer *con,
					  struct hmdfs_head_cmd *cmd)
{
	uint64_t boot_cookie = con->sbi->boot_cookie;
	uint16_t con_cookie = con->fid_cookie;

	if (hmdfs_should_fake_fid_ver(&con->sbi->fault_inject, con,
				      cmd, T_BOOT_COOKIE))
		boot_cookie = hmdfs_gen_boot_cookie();

	if (hmdfs_should_fake_fid_ver(&con->sbi->fault_inject, con,
				      cmd, T_CON_COOKIE))
		con_cookie++;

	return (boot_cookie |
		(con_cookie & ((1 << HMDFS_FID_VER_BOOT_COOKIE_SHIFT) - 1)));
}

static struct file *get_file_by_fid_and_ver(struct hmdfs_peer *con,
					    struct hmdfs_head_cmd *cmd,
					    __u32 file_id, __u64 file_ver)
{
	struct file *file = NULL;
	__u64 cur_file_ver = hmdfs_server_pack_fid_ver(con, cmd);

	if (file_ver != cur_file_ver) {
		hmdfs_warning("Stale file version %llu for fid %u (ver %llu)",
			      file_ver, file_id, cur_file_ver);
		return ERR_PTR(-EBADF);
	}

	file = get_file_from_conn(con, file_id);
	if (!file)
		return ERR_PTR(-EBADF);

	return file;
}

static void hmdfs_update_open_response(struct hmdfs_peer *con,
				       struct hmdfs_head_cmd *cmd,
				       struct hmdfs_open_info *info,
				       struct open_response *resp)
{
	struct hmdfs_time_t current_time = hmdfs_current_kernel_time();
	struct hmdfs_time_t ctime = info->stat_valid ? info->stat.ctime :
						       info->inode->i_ctime;
	struct hmdfs_time_t precision =
		msec_to_timespec(con->sbi->dcache_precision);
	loff_t size = info->stat_valid ? info->stat.size :
					 i_size_read(info->inode);

	resp->ino = cpu_to_le64(info->real_ino);
	resp->file_ver = cpu_to_le64(hmdfs_server_pack_fid_ver(con, cmd));
	resp->file_id = cpu_to_le32(info->file_id);
	resp->file_size = cpu_to_le64(size);
	resp->ctime = cpu_to_le64(ctime.tv_sec);
	resp->ctime_nsec = cpu_to_le32(ctime.tv_nsec);

	/*
	 * In server, ctime might stay the same after coverwrite. We introduce a
	 * new value stable_ctime to handle the problem.
	 * - if open rpc time < ctime, stable_ctime = 0;
	 * - if ctime <= open rpc time < ctime + dcache_precision, stable_ctime
	 *   = ctime
	 * - else, stable_ctime = ctime + dcache_precision;
	 */
	precision = hmdfs_time_add(ctime, precision);
	if (hmdfs_time_compare(&current_time, &ctime) < 0) {
		resp->stable_ctime = cpu_to_le64(0);
		resp->stable_ctime_nsec = cpu_to_le32(0);
	} else if (hmdfs_time_compare(&current_time, &ctime) >= 0 &&
		   hmdfs_time_compare(&current_time, &precision) < 0) {
		resp->stable_ctime = resp->ctime;
		resp->stable_ctime_nsec = resp->ctime_nsec;
	} else {
		resp->stable_ctime = cpu_to_le64(precision.tv_sec);
		resp->stable_ctime_nsec = cpu_to_le32(precision.tv_nsec);
	}
}

static int hmdfs_get_open_info(struct hmdfs_peer *con, uint8_t file_type,
			       const char *filename,
			       struct hmdfs_open_info *info)
{
	int ret = 0;

	info->inode = file_inode(info->file);
	info->stat_valid = false;
	if (con->sbi->sb == info->inode->i_sb) {
		/* if open a regular file */
		info->inode = hmdfs_i(info->inode)->lower_inode;
	} else if (con->sbi->lower_sb != info->inode->i_sb) {
		/* It's possible that inode is not from lower, for example:
		 * 1. touch /f2fs/file
		 * 2. ln -s /sdcard_fs/file /f2fs/link
		 * 3. cat /hmdfs/link -> generate dentry cache in sdcard_fs
		 * 4. echo hi >> /hmdfs/file -> append write not through
		 *    sdcard_fs
		 * 5. cat /hmdfs/link -> got inode in sdcard, which size is
		 *    still 0
		 *
		 * If src file isn't in lower, use getattr to get
		 * information.
		 */
		ret = vfs_getattr(&info->file->f_path, &info->stat, STATX_ALL,
				  0);
		if (ret) {
			hmdfs_err("call vfs_getattr failed, err %d", ret);
			return ret;
		}
		info->stat_valid = true;
	}

	/* if open a link file, get ino from link inode */
	if (hm_islnk(file_type)) {
		ret = hmdfs_get_inode_by_name(con, filename, &info->real_ino);
		if (ret)
			return ret;
	} else {
		info->real_ino = generate_u64_ino(info->inode->i_ino,
						  info->inode->i_generation);
	}

	return 0;
}

void hmdfs_server_open(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
		       void *data)
{
	struct open_request *recv = data;
	int sizeread = sizeof(struct open_response);
	struct open_response *resp = NULL;
	struct hmdfs_open_info *info = NULL;
	int ret = 0;

	trace_hmdfs_server_open_enter(con, recv);
	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &ret))
		goto out_err;

	resp = kzalloc(sizeread, GFP_KERNEL);
	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!resp || !info) {
		ret = -ENOMEM;
		goto err_free;
	}

	info->file = hmdfs_open_file(con, recv->buf, recv->file_type,
				     &info->file_id);
	if (IS_ERR(info->file)) {
		ret = PTR_ERR(info->file);
		goto err_free;
	}

	ret = hmdfs_get_open_info(con, recv->file_type, recv->buf, info);
	if (ret)
		goto err_close;

	hmdfs_update_open_response(con, cmd, info, resp);

	trace_hmdfs_server_open_exit(con, resp, info->file, 0);
	ret = hmdfs_sendmessage_response(con, cmd, sizeread, resp, 0);
	if (ret) {
		hmdfs_err("sending msg response failed, file_id %d, err %d",
			  info->file_id, ret);
		remove_file_from_conn(con, info->file_id);
		hmdfs_close_path(info->file);
	}
	kfree(resp);
	kfree(info);
	return;

err_close:
	remove_file_from_conn(con, info->file_id);
	hmdfs_close_path(info->file);
err_free:
	kfree(resp);
	kfree(info);
out_err:
	trace_hmdfs_server_open_exit(con, NULL, NULL, ret);
	hmdfs_send_err_response(con, cmd, ret);
}

static int hmdfs_check_and_create(struct path *path_parent,
				  struct dentry *dentry, uint64_t device_id,
				  umode_t mode, bool is_excl)
{
	int err = 0;

	/* if inode doesn't exist, create it */
	if (d_is_negative(dentry)) {
		hmdfs_mark_drop_flag(device_id, path_parent->dentry);
		err = vfs_create(d_inode(path_parent->dentry), dentry, mode,
				 is_excl);
		if (err)
			hmdfs_err("create failed, err %d", err);
	} else {
		if (is_excl)
			err = -EEXIST;
		/* if inode aready exist, see if it's symlink */
		else if (S_ISREG(d_inode(dentry)->i_mode) &&
			 hm_islnk(hmdfs_d(dentry)->file_type))
			err = -EINVAL;
		else if (S_ISDIR(d_inode(dentry)->i_mode))
			err = -EISDIR;
	}

	return err;
}
static int hmdfs_lookup_create(struct hmdfs_peer *con,
			       struct atomic_open_request *recv,
			       struct path *child_path, bool *truncate)
{
	int err = 0;
	struct path path_root;
	struct path path_parent;
	uint32_t open_flags = le32_to_cpu(recv->open_flags);
	char *path = recv->buf;
	char *filename = recv->buf + le32_to_cpu(recv->path_len) + 1;
	struct dentry *dentry = NULL;

	err = kern_path(con->sbi->local_dst, LOOKUP_DIRECTORY, &path_root);
	if (err) {
		hmdfs_err("no path for %s, err %d", con->sbi->local_dst, err);
		return err;
	}

	err = vfs_path_lookup(path_root.dentry, path_root.mnt, path,
			      LOOKUP_DIRECTORY, &path_parent);
	if (err) {
		hmdfs_info("no dir in %s, err %d", con->sbi->local_dst, err);
		goto put_path_root;
	}

	inode_lock(d_inode(path_parent.dentry));
	dentry = lookup_one_len(filename, path_parent.dentry, strlen(filename));
	if (IS_ERR(dentry)) {
		err = PTR_ERR(dentry);
		inode_unlock(d_inode(path_parent.dentry));
		goto put_path_parent;
	}
	/* only truncate if inode alredy exists */
	*truncate = ((open_flags & HMDFS_O_TRUNC) && d_is_positive(dentry));
	err = hmdfs_check_and_create(&path_parent, dentry, con->device_id,
				     le16_to_cpu(recv->mode),
				     open_flags & HMDFS_O_EXCL);
	inode_unlock(d_inode(path_parent.dentry));
	if (err) {
		dput(dentry);
	} else {
		child_path->dentry = dentry;
		child_path->mnt = mntget(path_parent.mnt);
	}

put_path_parent:
	path_put(&path_parent);
put_path_root:
	path_put(&path_root);
	return err;
}

static int hmdfs_dentry_open(struct hmdfs_peer *con,
			     const struct path *path,
			     struct hmdfs_open_info *info)
{
	int err = 0;

	info->file = dentry_open(path, O_RDWR | O_LARGEFILE, current_cred());
	if (IS_ERR(info->file)) {
		err = PTR_ERR(info->file);
		hmdfs_err("open file failed, err %d", err);
		return err;
	}

	info->file_id = insert_file_into_conn(con, info->file);
	if (info->file_id < 0) {
		err = info->file_id;
		hmdfs_err("file_id alloc failed! err %d", err);
		hmdfs_close_path(info->file);
		return err;
	}

	return 0;
}

static int hmdfs_server_do_atomic_open(struct hmdfs_peer *con,
				       struct hmdfs_head_cmd *cmd,
				       struct atomic_open_request *recv,
				       struct hmdfs_open_info *info,
				       struct atomic_open_response *resp)
{
	struct path child_path;
	bool truncate = false;
	int err = 0;

	err = hmdfs_lookup_create(con, recv, &child_path, &truncate);
	if (err)
		return err;

	err = hmdfs_dentry_open(con, &child_path, info);
	if (err)
		goto put_child;

	err = hmdfs_get_open_info(con, HM_REG, NULL, info);
	if (err)
		goto fail_close;

	if (truncate) {
		err = vfs_truncate(&child_path, 0);
		if (err) {
			hmdfs_err("truncate failed, err %d", err);
			goto fail_close;
		}
	}
	hmdfs_update_open_response(con, cmd, info, &resp->open_resp);
	resp->i_mode = cpu_to_le16(file_inode(info->file)->i_mode);

fail_close:
	if (err) {
		remove_file_from_conn(con, info->file_id);
		hmdfs_close_path(info->file);
	}
put_child:
	path_put(&child_path);
	return err;
}

void hmdfs_server_atomic_open(struct hmdfs_peer *con,
			      struct hmdfs_head_cmd *cmd, void *data)
{
	int err;
	struct atomic_open_request *recv = data;
	struct atomic_open_response *resp = NULL;
	struct hmdfs_open_info *info = NULL;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	if (is_hidden_dir(recv->buf)) {
		err = -EPERM;
		goto out;
	}

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	resp = kzalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp || !info) {
		err = -ENOMEM;
		goto out;
	}

	err = hmdfs_server_do_atomic_open(con, cmd, recv, info, resp);

out:
	if (err) {
		hmdfs_send_err_response(con, cmd, err);
	} else {
		err = hmdfs_sendmessage_response(con, cmd, sizeof(*resp), resp,
						 0);
		if (err) {
			hmdfs_err("sending msg response failed, file_id %d, err %d",
				  info->file_id, err);
			remove_file_from_conn(con, info->file_id);
			hmdfs_close_path(info->file);
		}
	}
	kfree(info);
	kfree(resp);
}

void hmdfs_server_release(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data)
{
	struct release_request *release_recv = data;
	struct file *file = NULL;
	__u32 file_id;
	__u64 file_ver;
	int ret = 0;

	file_id = le32_to_cpu(release_recv->file_id);
	file_ver = le64_to_cpu(release_recv->file_ver);
	file = get_file_by_fid_and_ver(con, cmd, file_id, file_ver);
	if (IS_ERR(file)) {
		hmdfs_err("cannot find %u", file_id);
		ret = PTR_ERR(file);
		goto out;
	}
	/* put the reference acquired by get_file_by_fid_and_ver() */
	hmdfs_close_path(file);
	hmdfs_info("close %u", file_id);
	remove_file_from_conn(con, file_id);

	hmdfs_close_path(file);

out:
	trace_hmdfs_server_release(con, file_id, file_ver, ret);
	set_conn_sock_quickack(con);
}

void hmdfs_server_fsync(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data)
{
	struct fsync_request *fsync_recv = data;
	__s32 datasync = le32_to_cpu(fsync_recv->datasync);
	__s64 start = le64_to_cpu(fsync_recv->start);
	__s64 end = le64_to_cpu(fsync_recv->end);
	struct file *file = NULL;
	__u32 file_id;
	__u64 file_ver;
	int ret = 0;

	file_id = le32_to_cpu(fsync_recv->file_id);
	file_ver = le64_to_cpu(fsync_recv->file_ver);
	file = get_file_by_fid_and_ver(con, cmd, file_id, file_ver);
	if (IS_ERR(file)) {
		hmdfs_err("cannot find %u", file_id);
		ret = PTR_ERR(file);
		goto out;
	}

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &ret))
		goto out_put_file;

	ret = vfs_fsync_range(file, start, end, datasync);
	if (ret)
		hmdfs_err("fsync fail, ret %d", ret);

out_put_file:
	hmdfs_close_path(file);
out:
	hmdfs_send_err_response(con, cmd, ret);
}

void hmdfs_server_readpage(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   void *data)
{
	struct readpage_request *readpage_recv = data;
	__u64 file_ver;
	__u32 file_id;
	struct file *file = NULL;
	loff_t pos;
	struct readpage_response *readpage = NULL;
	int ret = 0;
	size_t read_len;

	file_id = le32_to_cpu(readpage_recv->file_id);
	file_ver = le64_to_cpu(readpage_recv->file_ver);
	file = get_file_by_fid_and_ver(con, cmd, file_id, file_ver);
	if (unlikely(IS_ERR(file))) {
		hmdfs_info(
			"file with id %u does not exist, pgindex %llu, devid %llu",
			file_id, le64_to_cpu(readpage_recv->index),
			con->device_id);
		ret = PTR_ERR(file);
		goto fail;
	}

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &ret))
		goto fail_put_file;

	read_len = (size_t)le32_to_cpu(readpage_recv->size);
	if (read_len == 0)
		goto fail_put_file;

	readpage = kmalloc(read_len, GFP_KERNEL);
	if (!readpage) {
		ret = -ENOMEM;
		goto fail_put_file;
	}

	pos = (loff_t)le64_to_cpu(readpage_recv->index) << HMDFS_PAGE_OFFSET;
	ret = kernel_read(file, readpage->buf, read_len, &pos);
	if (ret < 0) {
		hmdfs_send_err_response(con, cmd, -EIO);
	} else {
		if (ret != read_len)
			memset(readpage->buf + ret, 0, read_len - ret);
		hmdfs_sendmessage_response(con, cmd, read_len, readpage, 0);
	}

	hmdfs_close_path(file);
	kfree(readpage);
	return;

fail_put_file:
	hmdfs_close_path(file);
fail:
	hmdfs_send_err_response(con, cmd, ret);
}

static struct readpages_response *alloc_readpages_resp(unsigned int len)
{
	struct readpages_response *resp = NULL;

	if (len > HMDFS_PAGE_SIZE)
		resp = vmalloc(len);
	else
		resp = kmalloc(len, GFP_KERNEL);

	return resp;
}

static void free_readpages_resp(struct readpages_response *resp,
				unsigned int len)
{
	if (len > HMDFS_PAGE_SIZE)
		vfree(resp);
	else
		kfree(resp);
}

void hmdfs_server_readpages(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    void *data)
{
	struct readpages_request *req = data;
	__u64 file_ver;
	__u32 file_id;
	struct file *file = NULL;
	loff_t pos;
	struct readpages_response *resp = NULL;
	ssize_t ret = 0;
	size_t read_len;

	file_id = le32_to_cpu(req->file_id);
	file_ver = le64_to_cpu(req->file_ver);
	file = get_file_by_fid_and_ver(con, cmd, file_id, file_ver);
	if (unlikely(IS_ERR(file))) {
		ret = PTR_ERR(file);
		goto fail;
	}

	read_len = (size_t)le32_to_cpu(req->size);
	if (read_len == 0)
		goto fail_put_file;

	resp = alloc_readpages_resp(read_len);
	if (!resp) {
		ret = -ENOMEM;
		goto fail_put_file;
	}

	pos = (loff_t)le64_to_cpu(req->index) << HMDFS_PAGE_OFFSET;
	ret = kernel_read(file, resp->buf, read_len, &pos);
	if (ret < 0) {
		ret = -EIO;
		goto fail_free_resp;
	}

	hmdfs_sendmessage_response(con, cmd, ret, resp, 0);
	hmdfs_close_path(file);
	free_readpages_resp(resp, read_len);
	return;

fail_free_resp:
	free_readpages_resp(resp, read_len);
fail_put_file:
	hmdfs_close_path(file);
fail:
	hmdfs_send_err_response(con, cmd, ret);
}

static int hmdfs_do_readpages_open(struct hmdfs_peer *con,
				   struct hmdfs_head_cmd *cmd,
				   struct readpages_open_request *recv,
				   struct hmdfs_open_info *info,
				   struct readpages_open_response *resp)
{
	int ret = 0;
	loff_t pos = 0;

	info->file = hmdfs_open_file(con, recv->buf, recv->file_type,
				     &info->file_id);
	if (unlikely(IS_ERR(info->file)))
		return PTR_ERR(info->file);

	ret = hmdfs_get_open_info(con, recv->file_type, recv->buf, info);
	if (ret)
		goto fail_close;

	pos = (loff_t)le64_to_cpu(recv->index) << HMDFS_PAGE_OFFSET;
	ret = kernel_read(info->file, resp->buf, le32_to_cpu(recv->size), &pos);
	if (ret < 0)
		goto fail_close;

	hmdfs_update_open_response(con, cmd, info, &resp->open_resp);
	memset(resp->reserved, 0, sizeof(resp->reserved));
	ret = hmdfs_sendmessage_response(con, cmd, sizeof(*resp) + ret, resp,
					 0);
	if (ret) {
		hmdfs_err("sending msg response failed, file_id %d, err %d",
			  info->file_id, ret);
		ret = 0;
		goto fail_close;
	}
	return 0;

fail_close:
	remove_file_from_conn(con, info->file_id);
	hmdfs_close_path(info->file);
	return ret;
}

void hmdfs_server_readpages_open(struct hmdfs_peer *con,
				 struct hmdfs_head_cmd *cmd, void *data)
{
	struct readpages_open_request *recv = data;
	struct readpages_open_response *resp = NULL;
	int ret = -EINVAL;
	size_t read_len = 0;
	size_t resp_len = 0;
	struct hmdfs_open_info *info = NULL;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &ret))
		goto fail;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		ret = -ENOMEM;
		goto fail;
	}

	read_len = (size_t)le32_to_cpu(recv->size);
	if (read_len == 0) {
		ret = -EINVAL;
		goto fail_free_info;
	}
	resp_len = read_len + sizeof(*resp);
	resp = vmalloc(resp_len);
	if (!resp) {
		ret = -ENOMEM;
		goto fail_free_info;
	}

	ret = hmdfs_do_readpages_open(con, cmd, recv, info, resp);

	vfree(resp);
fail_free_info:
	kfree(info);
fail:
	if (ret)
		hmdfs_send_err_response(con, cmd, ret);
}

static bool need_rebuild_dcache(struct hmdfs_dcache_header *h,
				struct hmdfs_time_t time,
				unsigned int precision)
{
	struct hmdfs_time_t crtime = { .tv_sec = le64_to_cpu(h->dcache_crtime),
				       .tv_nsec = le64_to_cpu(
					       h->dcache_crtime_nsec) };
	struct hmdfs_time_t ctime = { .tv_sec = le64_to_cpu(h->dentry_ctime),
				      .tv_nsec = le64_to_cpu(
					      h->dentry_ctime_nsec) };
	struct hmdfs_time_t pre_time = { .tv_sec = precision / MSEC_PER_SEC,
					 .tv_nsec = precision % MSEC_PER_SEC *
						    NSEC_PER_MSEC };

	if (hmdfs_time_compare(&time, &ctime) != 0)
		return true;

	pre_time = hmdfs_time_add(time, pre_time);
	if (hmdfs_time_compare(&crtime, &pre_time) < 0)
		return true;

	return false;
}

static bool hmdfs_server_cache_validate(struct file *filp, struct inode *inode,
					unsigned long precision)
{
	struct hmdfs_dcache_header header;
	int overallpage;
	ssize_t bytes;
	loff_t pos = 0;

	overallpage = get_dentry_group_cnt(file_inode(filp));
	if (overallpage == 0) {
		hmdfs_err("cache file size is 0");
		return false;
	}

	bytes = kernel_read(filp, &header, sizeof(header), &pos);
	if (bytes != sizeof(header)) {
		hmdfs_err("read file failed, err:%ld", bytes);
		return false;
	}

	return !need_rebuild_dcache(&header, inode->i_ctime, precision);
}

struct file *hmdfs_server_cache_revalidate(struct hmdfs_sb_info *sbi,
					   const char *recvpath,
					   struct path *path)
{
	struct cache_file_node *cfn = NULL;
	struct file *file;

	cfn = find_cfn(sbi, HMDFS_SERVER_CID, recvpath, true);
	if (!cfn)
		return NULL;

	if (!hmdfs_server_cache_validate(cfn->filp, path->dentry->d_inode,
					 sbi->dcache_precision)) {
		remove_cfn(cfn);
		release_cfn(cfn);
		return NULL;
	}
	file = cfn->filp;
	get_file(cfn->filp);
	release_cfn(cfn);

	return file;
}

bool hmdfs_client_cache_validate(struct hmdfs_sb_info *sbi,
				 struct readdir_request *readdir_recv,
				 struct path *path)
{
	struct inode *inode = path->dentry->d_inode;
	struct hmdfs_dcache_header header;

	/* always rebuild dentryfile for small dir */
	if (le64_to_cpu(readdir_recv->num) < sbi->dcache_threshold)
		return false;

	header.dcache_crtime = readdir_recv->dcache_crtime;
	header.dcache_crtime_nsec = readdir_recv->dcache_crtime_nsec;
	header.dentry_ctime = readdir_recv->dentry_ctime;
	header.dentry_ctime_nsec = readdir_recv->dentry_ctime_nsec;

	return !need_rebuild_dcache(&header, inode->i_ctime,
				    sbi->dcache_precision);
}

static char *server_lower_dentry_path_raw(struct hmdfs_peer *peer,
					  struct dentry *lo_d)
{
	struct hmdfs_dentry_info *di = hmdfs_d(peer->sbi->sb->s_root);
	struct dentry *lo_d_root = di->lower_path.dentry;
	struct dentry *lo_d_tmp = NULL;
	char *lo_p_buf = NULL;
	char *buf_head = NULL;
	char *buf_tail = NULL;
	size_t path_len = 0;

	lo_p_buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(!lo_p_buf))
		return ERR_PTR(-ENOMEM);

	/* To generate a reversed path str */
	for (lo_d_tmp = lo_d; lo_d_tmp != lo_d_root && !IS_ROOT(lo_d_tmp);
	     lo_d_tmp = lo_d_tmp->d_parent) {
		u32 dlen = lo_d_tmp->d_name.len;
		int reverse_index = dlen - 1;

		/* Considering the appended slash and '\0' */
		if (unlikely(path_len + dlen + 1 > PATH_MAX - 1)) {
			kfree(lo_p_buf);
			return ERR_PTR(-ENAMETOOLONG);
		}
		for (; reverse_index >= 0; --reverse_index)
			lo_p_buf[path_len++] =
				lo_d_tmp->d_name.name[reverse_index];
		lo_p_buf[path_len++] = '/';
	}

	/* Reverse the reversed path str to get the real path str */
	for (buf_head = lo_p_buf, buf_tail = lo_p_buf + path_len - 1;
	     buf_head < buf_tail; ++buf_head, --buf_tail)
		swap(*buf_head, *buf_tail);

	if (path_len == 0)
		lo_p_buf[0] = '/';
	return lo_p_buf;
}

static int server_lookup(struct hmdfs_peer *peer, const char *req_path,
			 struct path *path)
{
	struct path root_path;
	int err = 0;

	err = kern_path(peer->sbi->local_dst, 0, &root_path);
	if (err)
		goto out_noroot;

	err = vfs_path_lookup(root_path.dentry, root_path.mnt, req_path,
			      LOOKUP_DIRECTORY, path);
	path_put(&root_path);
out_noroot:
	return err;
}

/**
 * server_lookup_lower - lookup lower file-system
 * @peer: target device node
 * @req_path: abs path (mount point as the root) from the request
 * @lo_o: the lower path to return
 *
 * return the lower path's name, with characters' cases matched
 */
static char *server_lookup_lower(struct hmdfs_peer *peer, const char *req_path,
				 struct path *lo_p)
{
	char *lo_p_name = ERR_PTR(-ENOENT);
	struct path up_p;
	int err = 0;

	err = server_lookup(peer, req_path, &up_p);
	if (err)
		goto out;

	hmdfs_get_lower_path(up_p.dentry, lo_p);
	path_put(&up_p);

	lo_p_name = server_lower_dentry_path_raw(peer, lo_p->dentry);
	if (IS_ERR(lo_p_name)) {
		err = PTR_ERR(lo_p_name);
		path_put(lo_p);
	}
out:
	return err ? ERR_PTR(err) : lo_p_name;
}

void hmdfs_server_readdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data)
{
	struct readdir_request *readdir_recv = data;
	struct path lo_p;
	struct file *filp = NULL;
	int err = 0;
	unsigned long long num = 0;
	char *lo_p_name = NULL;

	trace_hmdfs_server_readdir(readdir_recv);

	lo_p_name = server_lookup_lower(con, readdir_recv->path, &lo_p);
	if (IS_ERR(lo_p_name)) {
		err = PTR_ERR(lo_p_name);
		hmdfs_info("Failed to get lower path: %d", err);
		goto send_err;
	}

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto err_lookup_path;

	if (le32_to_cpu(readdir_recv->verify_cache)) {
		if (hmdfs_client_cache_validate(con->sbi, readdir_recv, &lo_p))
			goto out_response;
	}

	filp = hmdfs_server_cache_revalidate(con->sbi, lo_p_name, &lo_p);
	if (IS_ERR_OR_NULL(filp)) {
		filp = hmdfs_server_rebuild_dents(con->sbi, &lo_p, &num,
						  lo_p_name);
		if (IS_ERR_OR_NULL(filp)) {
			err = PTR_ERR(filp);
			goto err_lookup_path;
		}
	}

out_response:
	err = hmdfs_readfile_response(con, cmd, filp);
	if (!err)
		hmdfs_add_remote_cache_list(con, lo_p_name);
	if (num >= con->sbi->dcache_threshold)
		cache_file_persistent(con, filp, lo_p_name, true);
	if (filp)
		fput(filp);
err_lookup_path:
	path_put(&lo_p);
	kfree(lo_p_name);
send_err:
	if (err)
		hmdfs_send_err_response(con, cmd, err);
}

int is_hidden_dir(const char *dir)
{
	int i;
	int hidden_len;
	int dir_len;
	static const char *hidden_dir[HMDFS_MAX_HIDDEN_DIR] = {
		"/Android",
	};

	for (i = 0; i < HMDFS_MAX_HIDDEN_DIR; i++) {
		if (!strcmp(dir, hidden_dir[i]))
			return HMDFS_IS_HIDDEN_DIR;

		hidden_len = strlen(hidden_dir[i]);
		dir_len = strlen(dir);
		if (dir_len <= hidden_len)
			continue;
		else if (dir[hidden_len] == '/' &&
			 !strncmp(dir, hidden_dir[i], hidden_len))
			return HMDFS_IN_HIDDEN_DIR;
	}

	return HMDFS_NOT_HIDDEN_DIR;
}

void hmdfs_server_mkdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data)
{
	int err = 0;
	struct mkdir_request *mkdir_recv = data;
	struct inode *child_inode = NULL;
	struct dentry *dent = NULL;
	char *mkdir_dir = NULL;
	char *mkdir_name = NULL;
	struct hmdfs_inodeinfo_response *mkdir_resp = NULL;
	int respsize = sizeof(struct hmdfs_inodeinfo_response);
	int path_len = le32_to_cpu(mkdir_recv->path_len);

	mkdir_resp = kzalloc(respsize, GFP_KERNEL);
	if (!mkdir_resp) {
		err = -ENOMEM;
		goto mkdir_out;
	}

	mkdir_dir = mkdir_recv->path;
	mkdir_name = mkdir_recv->path + path_len + 1;

	if (is_hidden_dir(mkdir_dir)) {
		err = -EPERM;
		goto mkdir_out;
	}

	dent = hmdfs_root_mkdir(con->device_id, con->sbi->local_dst,
				mkdir_dir, mkdir_name,
				le16_to_cpu(mkdir_recv->mode));
	if (IS_ERR(dent)) {
		err = PTR_ERR(dent);
		hmdfs_err("hmdfs_root_mkdir failed err = %d", err);
		goto mkdir_out;
	}
	child_inode = d_inode(dent);
	mkdir_resp->i_mode = cpu_to_le16(child_inode->i_mode);
	mkdir_resp->i_size = cpu_to_le64(child_inode->i_size);
	mkdir_resp->i_mtime = cpu_to_le64(child_inode->i_mtime.tv_sec);
	mkdir_resp->i_mtime_nsec = cpu_to_le32(child_inode->i_mtime.tv_nsec);
	mkdir_resp->i_ino = cpu_to_le64(child_inode->i_ino);
#ifdef CONFIG_HMDFS_1_0
	mkdir_resp->fno = cpu_to_le32(INVALID_FILE_ID);
#endif
	dput(dent);
mkdir_out:
	hmdfs_sendmessage_response(con, cmd, respsize, mkdir_resp, err);
	kfree(mkdir_resp);
}

void hmdfs_server_create(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data)
{
	int err = 0;
	struct create_request *create_recv = data;
	struct inode *child_inode = NULL;
	struct dentry *dent = NULL;
	char *create_dir = NULL;
	char *create_name = NULL;
	struct hmdfs_inodeinfo_response *create_resp = NULL;
	int respsize = sizeof(struct hmdfs_inodeinfo_response);
	int path_len = le32_to_cpu(create_recv->path_len);

	create_resp = kzalloc(respsize, GFP_KERNEL);
	if (!create_resp) {
		err = -ENOMEM;
		goto create_out;
	}

	create_dir = create_recv->path;
	create_name = create_recv->path + path_len + 1;

	if (is_hidden_dir(create_dir)) {
		err = -EPERM;
		goto create_out;
	}

	dent = hmdfs_root_create(con->device_id, con->sbi->local_dst,
				 create_dir, create_name,
				 le16_to_cpu(create_recv->mode),
				 create_recv->want_excl);
	if (IS_ERR(dent)) {
		err = PTR_ERR(dent);
		hmdfs_err("hmdfs_root_create failed err = %d", err);
		goto create_out;
	}
	child_inode = d_inode(dent);
	create_resp->i_mode = cpu_to_le16(child_inode->i_mode);
	create_resp->i_size = cpu_to_le64(child_inode->i_size);
	create_resp->i_mtime = cpu_to_le64(child_inode->i_mtime.tv_sec);
	create_resp->i_mtime_nsec = cpu_to_le32(child_inode->i_mtime.tv_nsec);
	/*
	 * keep same as hmdfs_server_open,
	 * to prevent hmdfs_open_final_remote from judging ino errors.
	 */
	create_resp->i_ino = cpu_to_le64(
		generate_u64_ino(hmdfs_i(child_inode)->lower_inode->i_ino,
				 child_inode->i_generation));
#ifdef CONFIG_HMDFS_1_0
	create_resp->fno = cpu_to_le32(INVALID_FILE_ID);
#endif
	dput(dent);
create_out:
	hmdfs_sendmessage_response(con, cmd, respsize, create_resp, err);
	kfree(create_resp);
}

void hmdfs_server_rmdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data)
{
	int err = 0;
	struct path root_path;
	char *path = NULL;
	char *name = NULL;
	struct rmdir_request *rmdir_recv = data;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	path = rmdir_recv->path;
	name = rmdir_recv->path + le32_to_cpu(rmdir_recv->path_len) + 1;
	err = kern_path(con->sbi->local_dst, 0, &root_path);
	if (!err) {
		err = hmdfs_root_rmdir(con->device_id, &root_path, path, name);
		path_put(&root_path);
	}
out:
	hmdfs_send_err_response(con, cmd, err);
}

void hmdfs_server_unlink(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data)
{
	int err = 0;
	struct path root_path;
	char *path = NULL;
	char *name = NULL;
	struct unlink_request *unlink_recv = data;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	path = unlink_recv->path;
	name = unlink_recv->path + le32_to_cpu(unlink_recv->path_len) + 1;
	err = kern_path(con->sbi->local_dst, 0, &root_path);
	if (!err) {
		err = hmdfs_root_unlink(con->device_id, &root_path, path, name);
		path_put(&root_path);
	}
out:
	hmdfs_send_err_response(con, cmd, err);
}

void hmdfs_server_rename(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data)
{
	int err = 0;
	int old_path_len;
	int new_path_len;
	int old_name_len;
	int new_name_len;
	unsigned int flags;
	char *path_old = NULL;
	char *name_old = NULL;
	char *path_new = NULL;
	char *name_new = NULL;
	struct rename_request *recv = data;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	old_path_len = le32_to_cpu(recv->old_path_len);
	new_path_len = le32_to_cpu(recv->new_path_len);
	old_name_len = le32_to_cpu(recv->old_name_len);
	new_name_len = le32_to_cpu(recv->new_name_len);
	flags = le32_to_cpu(recv->flags);

	path_old = recv->path;
	path_new = recv->path + old_path_len + 1;
	name_old = recv->path + old_path_len + 1 + new_path_len + 1;
	name_new = recv->path + old_path_len + 1 + new_path_len + 1 +
		   old_name_len + 1;

	if (is_hidden_dir(path_new)) {
		err = -EPERM;
		goto out;
	}
	err = hmdfs_root_rename(con->sbi, con->device_id, path_old, name_old,
				path_new, name_new, flags);
out:
	hmdfs_send_err_response(con, cmd, err);
}

static int hmdfs_lookup_symlink(struct path *link_path, const char *path_fmt,
				...)
{
	int ret;
	va_list args;
	char *path = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!path)
		return -ENOMEM;

	va_start(args, path_fmt);
	ret = vsnprintf(path, PATH_MAX, path_fmt, args);
	va_end(args);

	if (ret >= PATH_MAX) {
		ret = -ENAMETOOLONG;
		goto out;
	}

	/*
	 * Todo: when rebuild dentryfile, there maybe deadlock
	 *       because iterate_dir already hold the parent
	 *       lock, but now, we didn't know the symlink
	 *       src's parent.
	 */
	ret = kern_path(path, LOOKUP_FOLLOW, link_path);
	if (ret) {
		hmdfs_err("kern_path failed err = %d", ret);
		goto out;
	}

	if (!S_ISREG(d_inode(link_path->dentry)->i_mode)) {
		hmdfs_err("path is dir symlink");
		path_put(link_path);
		ret = -ENOTSUPP;
		goto out;
	}
out:
	kfree(path);
	return ret;
}

static int hmdfs_filldir_real(struct dir_context *ctx, const char *name,
			      int name_len, loff_t offset, u64 ino,
			      unsigned int d_type)
{
	int res = 0;
	char namestr[NAME_MAX + 1];
	struct getdents_callback_real *gc = NULL;
	struct dentry *child = NULL;

	if (name_len > NAME_MAX) {
		hmdfs_err("name_len:%d NAME_MAX:%u", name_len, NAME_MAX);
		goto out;
	}

	gc = container_of(ctx, struct getdents_callback_real, ctx);

	memcpy(namestr, name, name_len);
	namestr[name_len] = '\0';

	if (hmdfs_file_type(namestr) != HMDFS_TYPE_COMMON)
		goto out;

	/* parent lock already hold by iterate_dir */
	child = lookup_one_len(name, gc->parent_path->dentry, name_len);
	if (IS_ERR(child)) {
		res = PTR_ERR(child);
		hmdfs_err("lookup failed because %d", res);
		goto out;
	}

	if (d_really_is_negative(child)) {
		dput(child);
		hmdfs_err("lookup failed because negative dentry");
		/* just do not fill this entry and continue for next entry */
		goto out;
	}

	if (d_type == DT_REG || d_type == DT_DIR) {
		create_dentry(child, d_inode(child), gc->file, gc->sbi);
		gc->num++;
	} else if (d_type == DT_LNK) {
		struct path link_path;

		res = hmdfs_lookup_symlink(&link_path, "%s/%s/%s",
					   gc->sbi->local_src, gc->dir,
					   namestr);
		if (!res) {
			create_dentry(child, d_inode(link_path.dentry),
				      gc->file, gc->sbi);
			path_put(&link_path);
			gc->num++;
		} else if (res == -ENOENT) {
			/*
			 * If source file do not exist, use the info from link
			 * inode.
			 */
			create_dentry(child, d_inode(child), gc->file, gc->sbi);
			gc->num++;
		}
	}

	dput(child);

out:
	/*
	 * we always return 0 here, so that the caller can continue to next
	 * dentry even if failed on this dentry somehow.
	 */
	return 0;
}

static void hmdfs_server_set_header(struct hmdfs_dcache_header *header,
				    struct file *file, struct file *dentry_file)
{
	struct inode *inode = NULL;
	struct hmdfs_time_t cur_time;

	inode = file_inode(file);
	cur_time = current_time(file_inode(dentry_file));
	header->dcache_crtime = cpu_to_le64(cur_time.tv_sec);
	header->dcache_crtime_nsec = cpu_to_le64(cur_time.tv_nsec);
	header->dentry_ctime = cpu_to_le64(inode->i_ctime.tv_sec);
	header->dentry_ctime_nsec = cpu_to_le64(inode->i_ctime.tv_nsec);
}

// Get the dentries of target directory
struct file *hmdfs_server_rebuild_dents(struct hmdfs_sb_info *sbi,
					struct path *path, loff_t *num,
					const char *dir)
{
	int err = 0;
	struct getdents_callback_real gc = {
		.ctx.actor = hmdfs_filldir_real,
		.ctx.pos = 0,
		.num = 0,
		.sbi = sbi,
		.dir = dir,
	};
	struct file *file = NULL;
	struct file *dentry_file = NULL;
	struct hmdfs_dcache_header header;

	dentry_file = create_local_dentry_file_cache(sbi);
	if (IS_ERR(dentry_file)) {
		hmdfs_err("file create failed err=%ld", PTR_ERR(dentry_file));
		return dentry_file;
	}

	file = dentry_open(path, O_RDONLY | O_DIRECTORY, current_cred());
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		hmdfs_err("dentry_open failed");
		goto out;
	}

	hmdfs_server_set_header(&header, file, dentry_file);

	gc.parent_path = path;
	gc.file = dentry_file;

	if (is_hidden_dir(dir))
		goto write_header;

	err = iterate_dir(file, &(gc.ctx));
	if (err) {
		hmdfs_err("iterate_dir failed");
		goto out;
	}

write_header:
	header.case_sensitive = sbi->s_case_sensitive ? 1 : 0;
	header.num = cpu_to_le64(gc.num);
	if (num)
		*num = gc.num;

	err = write_header(dentry_file, &header);
out:
	if (!IS_ERR_OR_NULL(file))
		fput(file);

	if (err) {
		fput(dentry_file);
		dentry_file = ERR_PTR(err);
	}

	trace_hmdfs_server_rebuild_dents(&header, err);
	return dentry_file;
}

void hmdfs_server_writepage(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    void *data)
{
	struct writepage_request *writepage_recv = data;
	struct hmdfs_server_writeback *hswb = NULL;
	__u64 file_ver;
	__u32 file_id;
	struct file *file = NULL;
	loff_t pos;
	__u32 count;
	ssize_t ret;
	int err = 0;

	file_id = le32_to_cpu(writepage_recv->file_id);
	file_ver = le64_to_cpu(writepage_recv->file_ver);
	file = get_file_by_fid_and_ver(con, cmd, file_id, file_ver);
	if (unlikely(IS_ERR(file))) {
		hmdfs_info(
			"file with id %u does not exist, pgindex %llu, devid %llu",
			file_id, le64_to_cpu(writepage_recv->index),
			con->device_id);
		err = PTR_ERR(file);
		goto out;
	}

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out_put_file;

	pos = (loff_t)le64_to_cpu(writepage_recv->index) << HMDFS_PAGE_OFFSET;
	count = le32_to_cpu(writepage_recv->count);
	ret = kernel_write(file, writepage_recv->buf, count, &pos);
	if (ret != count)
		err = -EIO;

out_put_file:
	hmdfs_close_path(file);
out:
	hmdfs_send_err_response(con, cmd, err);

	hswb = con->sbi->h_swb;
	if (!err && hswb->dirty_writeback_control)
		hmdfs_server_check_writeback(hswb);
}

static int hmdfs_lookup_linkpath(struct hmdfs_sb_info *sbi,
				 const char *path_name, struct path *dst_path)
{
	struct path link_path;
	int err;

	err = hmdfs_lookup_symlink(&link_path, "%s/%s", sbi->local_dst,
				   path_name);
	if (err)
		return err;

	if (d_inode(link_path.dentry)->i_sb != sbi->sb) {
		path_put(dst_path);
		*dst_path = link_path;
	} else {
		path_put(&link_path);
	}

	return 0;
}

static struct inode *hmdfs_verify_path(struct dentry *dentry, char *recv_buf,
				       struct super_block *sb)
{
	struct inode *inode = d_inode(dentry);
	struct hmdfs_inode_info *info = NULL;

	/* if we found path from wrong fs */
	if (inode->i_sb != sb) {
		hmdfs_err("super block do not match");
		return NULL;
	}

	info = hmdfs_i(inode);
	/* make sure lower inode is not NULL */
	if (info->lower_inode)
		return info->lower_inode;

	/*
	 * we don't expect lower inode to be NULL in server. However, it's
	 * possible because dentry cache can contain stale data.
	 */
	hmdfs_info("lower inode is NULL, is remote file: %d",
		   info->conn != NULL);
	return NULL;
}

static int hmdfs_notify_change(struct vfsmount *mnt, struct dentry * dentry,
			       struct iattr * attr,
			       struct inode **delegated_inode)
{
#ifdef CONFIG_SDCARD_FS
	/* sdcard_fs need to call setattr2, notify_change will call setattr */
	return notify_change2(mnt, dentry, attr, delegated_inode);
#else
	return notify_change(dentry, attr, delegated_inode);
#endif
}

void hmdfs_server_setattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data)
{
	int err = 0;
	struct dentry *dentry = NULL;
	struct inode *inode = NULL;
	struct setattr_request *recv = data;
	struct path root_path, dst_path;
	struct iattr attr;
	__u32 valid = le32_to_cpu(recv->valid);

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	err = kern_path(con->sbi->local_dst, 0, &root_path);
	if (err) {
		hmdfs_err("kern_path failed err = %d", err);
		goto out;
	}

	err = vfs_path_lookup(root_path.dentry, root_path.mnt, recv->buf, 0,
			      &dst_path);
	if (err)
		goto out_put_root;

	inode = hmdfs_verify_path(dst_path.dentry, recv->buf, con->sbi->sb);
	if (!inode) {
		err = -ENOENT;
		goto out_put_dst;
	}

	/* We need to follow if symlink was found */
	if (S_ISLNK(inode->i_mode)) {
		err = hmdfs_lookup_linkpath(con->sbi, recv->buf, &dst_path);
		/* if source file doesn't exist, use link inode */
		if (err == -ENOENT)
			err = 0;
		else if (err)
			goto out_put_dst;
	}

	dentry = dst_path.dentry;
	memset(&attr, 0, sizeof(attr));
	/* only support size and mtime */
	if (valid & (ATTR_SIZE | ATTR_MTIME))
		attr.ia_valid =
			(valid & (ATTR_MTIME | ATTR_MTIME_SET | ATTR_SIZE));
	attr.ia_size = le64_to_cpu(recv->size);
	attr.ia_mtime.tv_sec = le64_to_cpu(recv->mtime);
	attr.ia_mtime.tv_nsec = le32_to_cpu(recv->mtime_nsec);

	inode_lock(dentry->d_inode);
	err = hmdfs_notify_change(dst_path.mnt, dentry, &attr, NULL);
	inode_unlock(dentry->d_inode);

out_put_dst:
	path_put(&dst_path);
out_put_root:
	path_put(&root_path);
out:
	hmdfs_send_err_response(con, cmd, err);
}

static void update_getattr_response(struct hmdfs_peer *con, struct inode *inode,
				    struct kstat *ks,
				    struct getattr_response *resp)
{
	/* if getattr for link, get ino and mode from actual lower inode */
	resp->ino = cpu_to_le64(
		generate_u64_ino(inode->i_ino, inode->i_generation));
	resp->mode = cpu_to_le16(inode->i_mode);

	/* get other information from vfs_getattr() */
	resp->result_mask = cpu_to_le32(STATX_ALL);
	resp->fsid = cpu_to_le64(ks->dev);
	resp->nlink = cpu_to_le32(ks->nlink);
	resp->uid = cpu_to_le32(ks->uid.val);
	resp->gid = cpu_to_le32(ks->gid.val);
	resp->size = cpu_to_le64(ks->size);
	resp->blocks = cpu_to_le64(ks->blocks);
	resp->blksize = cpu_to_le32(ks->blksize);
	resp->atime = cpu_to_le64(ks->atime.tv_sec);
	resp->atime_nsec = cpu_to_le32(ks->atime.tv_nsec);
	resp->mtime = cpu_to_le64(ks->mtime.tv_sec);
	resp->mtime_nsec = cpu_to_le32(ks->mtime.tv_nsec);
	resp->ctime = cpu_to_le64(ks->ctime.tv_sec);
	resp->ctime_nsec = cpu_to_le32(ks->ctime.tv_nsec);
	resp->crtime = cpu_to_le64(ks->btime.tv_sec);
	resp->crtime_nsec = cpu_to_le32(ks->btime.tv_nsec);
}

void hmdfs_server_getattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data)
{
	int err = 0;
	struct getattr_request *recv = data;
	int size_read = sizeof(struct getattr_response);
	struct getattr_response *resp = NULL;
	struct kstat ks;
	struct path root_path, dst_path;
	struct inode *inode = NULL;
	unsigned int recv_flags = le32_to_cpu(recv->lookup_flags);
	unsigned int lookup_flags = 0;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto err;

	err = hmdfs_convert_lookup_flags(recv_flags, &lookup_flags);
	if (err)
		goto err;

	resp = kzalloc(size_read, GFP_KERNEL);
	if (!resp) {
		err = -ENOMEM;
		goto err;
	}
	err = kern_path(con->sbi->local_dst, 0, &root_path);
	if (err) {
		hmdfs_err("kern_path failed err = %d", err);
		goto err_free_resp;
	}
	//TODO: local_dst -->local_src
	err = vfs_path_lookup(root_path.dentry, root_path.mnt, recv->buf,
			      lookup_flags, &dst_path);
	if (err)
		goto out_put_root;

	err = is_hidden_dir(recv->buf);
	if (err == HMDFS_IN_HIDDEN_DIR) {
		err = -ENOENT;
		goto out_put_dst;
	}

	inode = hmdfs_verify_path(dst_path.dentry, recv->buf, con->sbi->sb);
	if (!inode) {
		err = -ENOENT;
		goto out_put_dst;
	}
	/* We need to follow if symlink was found */
	if (S_ISLNK(inode->i_mode)) {
		err = hmdfs_lookup_linkpath(con->sbi, recv->buf, &dst_path);
		/* if source file doesn't exist, use link inode */
		if (err && err != -ENOENT)
			goto out_put_dst;
	}

	err = vfs_getattr(&dst_path, &ks, STATX_ALL, 0);
	if (err)
		goto err_put_dst;
	update_getattr_response(con, inode, &ks, resp);

out_put_dst:
	path_put(&dst_path);
out_put_root:
	/*
	 * if path lookup failed, we return with result_mask setting to
	 * zero. So we can be aware of such situation in caller.
	 */
	if (err)
		resp->result_mask = cpu_to_le32(0);
	path_put(&root_path);
	hmdfs_sendmessage_response(con, cmd, size_read, resp, err);
	kfree(resp);
	return;

err_put_dst:
	path_put(&dst_path);
	path_put(&root_path);
err_free_resp:
	kfree(resp);
err:
	hmdfs_send_err_response(con, cmd, err);
}

static void init_statfs_response(struct statfs_response *resp,
				 struct kstatfs *st)
{
	resp->f_type = cpu_to_le64(HMDFS_SUPER_MAGIC);
	resp->f_bsize = cpu_to_le64(st->f_bsize);
	resp->f_blocks = cpu_to_le64(st->f_blocks);
	resp->f_bfree = cpu_to_le64(st->f_bfree);
	resp->f_bavail = cpu_to_le64(st->f_bavail);
	resp->f_files = cpu_to_le64(st->f_files);
	resp->f_ffree = cpu_to_le64(st->f_ffree);
	resp->f_fsid_0 = cpu_to_le32(st->f_fsid.val[0]);
	resp->f_fsid_1 = cpu_to_le32(st->f_fsid.val[1]);
	resp->f_namelen = cpu_to_le64(st->f_namelen);
	resp->f_frsize = cpu_to_le64(st->f_frsize);
	resp->f_flags = cpu_to_le64(st->f_flags);
	/* f_spare is not used in f2fs or ext4 */
	resp->f_spare_0 = cpu_to_le64(st->f_spare[0]);
	resp->f_spare_1 = cpu_to_le64(st->f_spare[1]);
	resp->f_spare_2 = cpu_to_le64(st->f_spare[2]);
	resp->f_spare_3 = cpu_to_le64(st->f_spare[3]);
}

void hmdfs_server_statfs(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data)
{
	struct statfs_request *recv = data;
	struct statfs_response *resp = NULL;
	struct path root_path, path;
	struct kstatfs *st = NULL;
	int err = 0;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &err))
		goto out;

	st = kzalloc(sizeof(*st), GFP_KERNEL);
	if (!st) {
		err = -ENOMEM;
		goto out;
	}

	resp = kmalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp) {
		err = -ENOMEM;
		goto free_st;
	}

	err = kern_path(con->sbi->local_src, 0, &root_path);
	if (err) {
		hmdfs_info("kern_path failed err = %d", err);
		goto free_st;
	}

	err = vfs_path_lookup(root_path.dentry, root_path.mnt, recv->path, 0,
			      &path);
	if (err) {
		hmdfs_info("recv->path found failed err = %d", err);
		goto put_root;
	}

	err = vfs_statfs(&path, st);
	if (err)
		hmdfs_info("statfs local dentry failed, err = %d", err);
	init_statfs_response(resp, st);
	path_put(&path);

put_root:
	path_put(&root_path);
free_st:
	kfree(st);
out:
	if (err)
		hmdfs_send_err_response(con, cmd, err);
	else
		hmdfs_sendmessage_response(con, cmd, sizeof(*resp), resp, 0);

	kfree(resp);
}

void hmdfs_server_syncfs(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data)
{
	/*
	 * Reserved interface. There is a difference compared with traditional
	 * syncfs process. Remote syncfs process in client:
	 * 1. Remote writepages by async call
	 * 2. Remote syncfs calling
	 * 3. Wait all remote async calls(writepages) return in step 1
	 */
	int ret = 0;

	if (hmdfs_should_fail_req(&con->sbi->fault_inject, con, cmd, &ret)) {
		hmdfs_send_err_response(con, cmd, ret);
		return;
	}

	hmdfs_send_err_response(con, cmd, ret);
}

void hmdfs_server_getxattr(struct hmdfs_peer *con,
			   struct hmdfs_head_cmd *cmd, void *data)
{
	struct getxattr_request *recv = data;
	size_t size = le32_to_cpu(recv->size);
	size_t size_read = sizeof(struct getxattr_response) + size;
	struct getxattr_response *resp = NULL;
	struct path root_path;
	struct path path;
	char *file_path = recv->buf;
	char *name = recv->buf + recv->path_len + 1;
	int err = -ENOMEM;

	resp = kzalloc(size_read, GFP_KERNEL);
	if (!resp)
		goto err;

	err = kern_path(con->sbi->local_dst, LOOKUP_DIRECTORY, &root_path);
	if (err) {
		hmdfs_info("kern_path failed err = %d", err);
		goto err_free_resp;
	}

	err = vfs_path_lookup(root_path.dentry, root_path.mnt,
			      file_path, 0, &path);
	if (err) {
		hmdfs_info("path found failed err = %d", err);
		goto err_put_root;
	}

	if (!size)
		err = vfs_getxattr(path.dentry, name, NULL, size);
	else
		err = vfs_getxattr(path.dentry, name, resp->value, size);
	if (err < 0) {
		hmdfs_info("getxattr failed err %d", err);
		goto err_put_path;
	}

	resp->size = cpu_to_le32(err);
	hmdfs_sendmessage_response(con, cmd, size_read, resp, 0);
	path_put(&path);
	path_put(&root_path);
	kfree(resp);
	return;

err_put_path:
	path_put(&path);
err_put_root:
	path_put(&root_path);
err_free_resp:
	kfree(resp);
err:
	hmdfs_send_err_response(con, cmd, err);
}

void hmdfs_server_setxattr(struct hmdfs_peer *con,
			   struct hmdfs_head_cmd *cmd, void *data)
{
	struct setxattr_request *recv = data;
	size_t size = le32_to_cpu(recv->size);
	int flags = le32_to_cpu(recv->flags);
	bool del = recv->del;
	struct path root_path;
	struct path path;
	const char *file_path = NULL;
	const char *name = NULL;
	const void *value = NULL;
	int err;

	err = kern_path(con->sbi->local_dst, LOOKUP_DIRECTORY, &root_path);
	if (err) {
		hmdfs_info("kern_path failed err = %d", err);
		goto err;
	}

	file_path = recv->buf;
	name = recv->buf + recv->path_len + 1;
	value = name + recv->name_len + 1;
	err = vfs_path_lookup(root_path.dentry, root_path.mnt,
			      file_path, 0, &path);
	if (err) {
		hmdfs_info("path found failed err = %d", err);
		goto err_put_root;
	}

	if (del) {
		WARN_ON(flags != XATTR_REPLACE);
		err = vfs_removexattr(path.dentry, name);
	} else {
		err = vfs_setxattr(path.dentry, name, value, size, flags);
	}

	path_put(&path);
err_put_root:
	path_put(&root_path);
err:
	hmdfs_send_err_response(con, cmd, err);
}

void hmdfs_server_listxattr(struct hmdfs_peer *con,
			    struct hmdfs_head_cmd *cmd, void *data)
{
	struct listxattr_request *recv = data;
	size_t size = le32_to_cpu(recv->size);
	int size_read = sizeof(struct listxattr_response) + size;
	struct listxattr_response *resp = NULL;
	const char *file_path = NULL;
	struct path root_path;
	struct path path;
	int err = 0;

	resp = kzalloc(size_read, GFP_KERNEL);
	if (!resp) {
		err = -ENOMEM;
		goto err;
	}

	err = kern_path(con->sbi->local_dst, LOOKUP_DIRECTORY, &root_path);
	if (err) {
		hmdfs_info("kern_path failed err = %d", err);
		goto err_free_resp;
	}

	file_path = recv->buf;
	err = vfs_path_lookup(root_path.dentry, root_path.mnt,
			      file_path, 0, &path);
	if (err) {
		hmdfs_info("path found failed err = %d", err);
		goto err_put_root;
	}

	if (!size)
		err = vfs_listxattr(path.dentry, NULL, size);
	else
		err = vfs_listxattr(path.dentry, resp->list, size);
	if (err < 0) {
		hmdfs_info("listxattr failed err = %d", err);
		goto err_put_path;
	}

	resp->size = cpu_to_le32(err);
	hmdfs_sendmessage_response(con, cmd, size_read, resp, 0);
	path_put(&root_path);
	path_put(&path);
	kfree(resp);
	return;

err_put_path:
	path_put(&path);
err_put_root:
	path_put(&root_path);
err_free_resp:
	kfree(resp);
err:
	hmdfs_send_err_response(con, cmd, err);
}

void hmdfs_server_get_drop_push(struct hmdfs_peer *con,
				struct hmdfs_head_cmd *cmd, void *data)
{
	struct drop_push_request *dp_recv = data;
	struct path root_path, path;
	int err;
	char *tmp_path = NULL;

	err = kern_path(con->sbi->real_dst, 0, &root_path);
	if (err) {
		hmdfs_err("kern_path failed err = %d", err);
		goto quickack;
	}
	tmp_path = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!tmp_path)
		goto out;
	snprintf(tmp_path, PATH_MAX, "/" DEVICE_VIEW_ROOT "/%s%s",
		 con->cid, dp_recv->path);

	err = vfs_path_lookup(root_path.dentry, root_path.mnt, tmp_path, 0,
			      &path);
	if (err) {
		hmdfs_info("path found failed err = %d", err);
		goto free;
	}
	hmdfs_remove_cache_filp(con, path.dentry);

	path_put(&path);
free:
	kfree(tmp_path);
out:
	path_put(&root_path);
quickack:
	set_conn_sock_quickack(con);
}
