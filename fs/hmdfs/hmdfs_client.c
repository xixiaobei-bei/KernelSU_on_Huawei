/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_client.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: maojingjing@huawei.com lousong@huawei.com
 * Create: 2020-04-09
 *
 */

#include "hmdfs_client.h"
#include "hmdfs_server.h"

#include <linux/highmem.h>
#include <linux/sched/signal.h>
#include <linux/statfs.h>

#include "comm/socket_adapter.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"
#include "comm/node_cb.h"
#include "stash.h"
#include "authority/authentication.h"

#define HMDFS_SYNC_WPAGE_RETRY_MS 2000

static inline void free_sm_outbuf(struct hmdfs_send_command *sm)
{
	if (sm->out_buf && sm->out_len != 0)
		kfree(sm->out_buf);
	sm->out_len = 0;
	sm->out_buf = NULL;
}

int hmdfs_send_open(struct hmdfs_peer *con, const char *send_buf,
		    __u8 file_type, struct hmdfs_open_ret *open_ret)
{
	int ret;
	int path_len = strlen(send_buf);
	size_t send_len = sizeof(struct open_request) + path_len + 1;
	struct open_request *open_req = kzalloc(send_len, GFP_KERNEL);
	struct open_response *resp;
	struct hmdfs_send_command sm = {
		.data = open_req,
		.len = send_len,
	};
	hmdfs_init_cmd(&sm.operations, F_OPEN);

	if (!open_req) {
		ret = -ENOMEM;
		goto out;
	}
	open_req->file_type = file_type;
	open_req->path_len = cpu_to_le32(path_len);
	strcpy(open_req->buf, send_buf);
	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(open_req);

	if (!ret && (sm.out_len == 0 || !sm.out_buf))
		ret = -ENOENT;
	if (ret)
		goto out;
	resp = sm.out_buf;

	open_ret->ino = le64_to_cpu(resp->ino);
	open_ret->fid.ver = le64_to_cpu(resp->file_ver);
	open_ret->fid.id = le32_to_cpu(resp->file_id);
	open_ret->file_size = le64_to_cpu(resp->file_size);
	open_ret->remote_ctime.tv_sec = le64_to_cpu(resp->ctime);
	open_ret->remote_ctime.tv_nsec = le32_to_cpu(resp->ctime_nsec);
	open_ret->stable_ctime.tv_sec = le64_to_cpu(resp->stable_ctime);
	open_ret->stable_ctime.tv_nsec = le32_to_cpu(resp->stable_ctime_nsec);

out:
	free_sm_outbuf(&sm);
	return ret;
}

void hmdfs_send_close(struct hmdfs_peer *con, const struct hmdfs_fid *fid)
{
	size_t send_len = sizeof(struct release_request);
	struct release_request *release_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = release_req,
		.len = send_len,
	};
	hmdfs_init_cmd(&sm.operations, F_RELEASE);

	if (!release_req)
		return;

	release_req->file_ver = cpu_to_le64(fid->ver);
	release_req->file_id = cpu_to_le32(fid->id);

	hmdfs_sendmessage_request(con, &sm);
	kfree(release_req);
}

int hmdfs_send_fsync(struct hmdfs_peer *con, const struct hmdfs_fid *fid,
		     __s64 start, __s64 end, __s32 datasync)
{
	int ret;
	struct fsync_request *fsync_req =
		kzalloc(sizeof(struct fsync_request), GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = fsync_req,
		.len = sizeof(struct fsync_request),
	};

	hmdfs_init_cmd(&sm.operations, F_FSYNC);
	if (!fsync_req)
		return -ENOMEM;

	fsync_req->file_ver = cpu_to_le64(fid->ver);
	fsync_req->file_id = cpu_to_le32(fid->id);
	fsync_req->datasync = cpu_to_le32(datasync);
	fsync_req->start = cpu_to_le64(start);
	fsync_req->end = cpu_to_le64(end);

	ret = hmdfs_sendmessage_request(con, &sm);

	free_sm_outbuf(&sm);
	kfree(fsync_req);
	return ret;
}

int hmdfs_client_readpage(struct hmdfs_peer *con, const struct hmdfs_fid *fid,
			  struct page *page)
{
	int ret;
	size_t send_len = sizeof(struct readpage_request);
	struct readpage_request *read_data = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = read_data,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_READPAGE);
	if (!read_data) {
		unlock_page(page);
		return -ENOMEM;
	}

	sm.out_buf = page;
	read_data->file_ver = cpu_to_le64(fid->ver);
	read_data->file_id = cpu_to_le32(fid->id);
	read_data->size = cpu_to_le32(HMDFS_PAGE_SIZE);
	read_data->index = cpu_to_le64(page->index);
	ret = hmdfs_sendpage_request(con, &sm);
	kfree(read_data);
	return ret;
}

bool hmdfs_usr_sig_pending(struct task_struct *p)
{
	sigset_t *sig = &p->pending.signal;

	if (likely(!signal_pending(p)))
		return false;
	return sigismember(sig, SIGINT) || sigismember(sig, SIGTERM) ||
	       sigismember(sig, SIGKILL);
}

void hmdfs_client_writepage_done(struct hmdfs_inode_info *info,
				 struct hmdfs_writepage_context *ctx)
{
	struct page *page = ctx->page;
	bool unlock = ctx->rsem_held;

	SetPageUptodate(page);
	end_page_writeback(page);
	if (unlock)
		up_read(&info->wpage_sem);
	unlock_page(page);
}

static void hmdfs_client_writepage_err(struct hmdfs_peer *peer,
				       struct hmdfs_inode_info *info,
				       struct hmdfs_writepage_context *ctx,
				       int err)
{
	struct page *page = ctx->page;
	bool unlock = ctx->rsem_held;

	if (err == -ENOMEM || err == -EAGAIN || err == -ESHUTDOWN ||
	    err == -ETIME)
		SetPageUptodate(page);
	else
		hmdfs_info("Page %ld of file %u writeback err %d devid %llu",
			   page->index, ctx->fid.id, err, peer->device_id);

	/*
	 * Current and subsequent writebacks have been canceled by the
	 * user, leaving these pages' states in chaos. Read pages in
	 * the future to update these pages.
	 */
	if (ctx->sync_all && hmdfs_usr_sig_pending(ctx->caller))
		ClearPageUptodate(page);

	if (ctx->sync_all || !time_is_after_eq_jiffies(ctx->timeout) ||
	    !(err == -ETIME || hmdfs_need_redirty_page(info, err))) {
		SetPageError(page);
		mapping_set_error(page->mapping, -EIO);
	} else {
		__set_page_dirty_nobuffers(page);
		account_page_redirty(page);
	}

	end_page_writeback(page);
	if (unlock)
		up_read(&info->wpage_sem);
	unlock_page(page);
}

static inline bool
hmdfs_no_timedout_sync_write(struct hmdfs_writepage_context *ctx)
{
	return ctx->sync_all && time_is_after_eq_jiffies(ctx->timeout);
}

static inline bool
hmdfs_client_rewrite_for_timeout(struct hmdfs_writepage_context *ctx, int err)
{
	return (err == -ETIME && hmdfs_no_timedout_sync_write(ctx) &&
		!hmdfs_usr_sig_pending(ctx->caller));
}

static inline bool
hmdfs_client_rewrite_for_offline(struct hmdfs_sb_info *sbi,
				 struct hmdfs_writepage_context *ctx, int err)
{
	struct hmdfs_inode_info *info = hmdfs_i(ctx->page->mapping->host);
	unsigned int status = READ_ONCE(info->stash_status);

	/*
	 * No retry if offline occurs during inode restoration.
	 *
	 * Do retry if local file cache is ready even it is not
	 * a WB_SYNC_ALL write, else no-sync_all writeback will
	 * return -EIO, mapping_set_error(mapping, -EIO) will be
	 * called and it will make the concurrent calling of
	 * filemap_write_and_wait() in hmdfs_flush_stash_file_data()
	 * return -EIO.
	 */
	return (hmdfs_is_stash_enabled(sbi) &&
		status != HMDFS_REMOTE_INODE_RESTORING &&
		(hmdfs_no_timedout_sync_write(ctx) ||
		 status == HMDFS_REMOTE_INODE_STASHING) &&
		hmdfs_is_offline_or_timeout_err(err));
}

static inline bool
hmdfs_client_redo_writepage(struct hmdfs_sb_info *sbi,
			    struct hmdfs_writepage_context *ctx, int err)
{
	return hmdfs_client_rewrite_for_timeout(ctx, err) ||
	       hmdfs_client_rewrite_for_offline(sbi, ctx, err);
}

static bool hmdfs_remote_write_to_remote(struct hmdfs_inode_info *info)
{
	unsigned int status = READ_ONCE(info->stash_status);
	bool stashing;

	if (status != HMDFS_REMOTE_INODE_STASHING)
		return true;

	/* Ensure it's OK to use info->cache afterwards */
	spin_lock(&info->stash_lock);
	stashing = (info->stash_status == HMDFS_REMOTE_INODE_STASHING);
	spin_unlock(&info->stash_lock);

	return !stashing;
}

int hmdfs_remote_do_writepage(struct hmdfs_peer *con,
			      struct hmdfs_writepage_context *ctx)
{
	struct hmdfs_inode_info *info = hmdfs_i(ctx->page->mapping->host);
	bool to_remote = false;
	int err = 0;

	to_remote = hmdfs_remote_write_to_remote(info);
	if (to_remote)
		err = hmdfs_client_writepage(info->conn, ctx);
	else
		err = hmdfs_stash_writepage(info->conn, ctx);
	if (!err)
		return 0;

	if (!(to_remote &&
	      hmdfs_client_rewrite_for_offline(con->sbi, ctx, err)))
		return err;

	queue_delayed_work(con->retry_wb_wq, &ctx->retry_dwork,
			   msecs_to_jiffies(HMDFS_SYNC_WPAGE_RETRY_MS));

	return 0;
}

void hmdfs_remote_writepage_retry(struct work_struct *work)
{
	struct hmdfs_writepage_context *ctx =
		container_of(work, struct hmdfs_writepage_context,
			     retry_dwork.work);
	struct hmdfs_inode_info *info = hmdfs_i(ctx->page->mapping->host);
	struct hmdfs_peer *peer = info->conn;
	const struct cred *old_cred = NULL;
	int err;

	old_cred = hmdfs_override_creds(peer->sbi->cred);
	err = hmdfs_remote_do_writepage(peer, ctx);
	hmdfs_revert_creds(old_cred);
	if (err) {
		hmdfs_client_writepage_err(peer, info, ctx, err);
		put_task_struct(ctx->caller);
		kfree(ctx);
	}
}

void hmdfs_writepage_cb(struct hmdfs_peer *peer, const struct hmdfs_req *req,
			const struct hmdfs_resp *resp)
{
	struct hmdfs_writepage_context *ctx = req->private;
	struct hmdfs_inode_info *info = hmdfs_i(ctx->page->mapping->host);
	int ret = resp->ret_code;
	unsigned long page_index = ctx->page->index;

	trace_hmdfs_writepage_cb_enter(peer, info->remote_ino, page_index, ret);

	if (!ret) {
		hmdfs_client_writepage_done(info, ctx);
		atomic64_inc(&info->write_counter);
		goto cleanup_all;
	}

	if (hmdfs_client_redo_writepage(peer->sbi, ctx, ret)) {
		ret = hmdfs_remote_do_writepage(peer, ctx);
		if (!ret)
			goto cleanup_req;
		WARN_ON(ret == -ETIME);
	}

	hmdfs_client_writepage_err(peer, info, ctx, ret);

cleanup_all:
	put_task_struct(ctx->caller);
	kfree(ctx);
cleanup_req:
	kfree(req->data);

	trace_hmdfs_writepage_cb_exit(peer, info->remote_ino, page_index, ret);
}

int hmdfs_client_writepage(struct hmdfs_peer *con,
			   struct hmdfs_writepage_context *param)
{
	int ret = 0;
	size_t send_len = sizeof(struct writepage_request) + HMDFS_PAGE_SIZE;
	struct writepage_request *write_data = kzalloc(send_len, GFP_NOFS);
	struct hmdfs_req req;
	char *data = NULL;

	if (unlikely(!write_data))
		return -ENOMEM;

	WARN_ON(!PageLocked(param->page)); // VFS
	WARN_ON(PageDirty(param->page)); // VFS
	WARN_ON(!PageWriteback(param->page)); // hmdfs

	write_data->file_ver = cpu_to_le64(param->fid.ver);
	write_data->file_id = cpu_to_le32(param->fid.id);
	write_data->index = cpu_to_le64(param->page->index);
	write_data->count = cpu_to_le32(param->count);
	data = kmap(param->page);
	memcpy((char *)write_data->buf, data, HMDFS_PAGE_SIZE);
	kunmap(param->page);
	req.data = write_data;
	req.data_len = send_len;

	req.private = param;
	req.private_len = sizeof(*param);

	req.timeout = TIMEOUT_CONFIG;
	hmdfs_init_cmd(&req.operations, F_WRITEPAGE);
	ret = hmdfs_send_async_request(con, &req);
	if (unlikely(ret))
		kfree(write_data);
	return ret;
}

void hmdfs_client_recv_readpage(struct hmdfs_head_cmd *head, int err,
				struct hmdfs_async_work *async_work)
{
	struct page *page = async_work->page;
	int ret = le32_to_cpu(head->ret_code);
	struct hmdfs_inode_info *info = hmdfs_i(page->mapping->host);
	unsigned long page_index = page->index;

	if (!err)
		SetPageUptodate(page);
	else if (err == -EBADF)
		/* There may be a stale fd caused by fid version, need reopen */
		set_bit(HMDFS_FID_NEED_OPEN, &info->fid_flags);

	hmdfs_client_resp_statis(async_work->head.peer->sbi, F_READPAGE,
				 HMDFS_RESP_NORMAL, async_work->start, jiffies);

	trace_hmdfs_client_recv_readpage(async_work->head.peer,
					 info->remote_ino, page_index, ret);

	asw_done(async_work);
}

/* read cache dentry file at path and write them into filp */
int hmdfs_client_start_readdir(struct hmdfs_peer *con, struct file *filp,
			       const char *path, int path_len,
			       struct hmdfs_dcache_header *header)
{
	int ret;
	size_t send_len = sizeof(struct readdir_request) + path_len + 1;
	struct readdir_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
		.local_filp = filp,
	};

	hmdfs_init_cmd(&sm.operations, F_ITERATE);
	if (!req)
		return -ENOMEM;

	/* add ref or it will be release at msg put */
	get_file(sm.local_filp);
	req->path_len = cpu_to_le32(path_len);
	strncpy(req->path, path, path_len);

	/*
	 * Is we already have a cache file, verify it. If it is
	 * uptodate, then we don't have to transfer a new one
	 */
	if (header) {
		req->dcache_crtime = header->dcache_crtime;
		req->dcache_crtime_nsec = header->dcache_crtime_nsec;
		req->dentry_ctime = header->dentry_ctime;
		req->dentry_ctime_nsec = header->dentry_ctime_nsec;
		req->num = header->num;
		req->verify_cache = cpu_to_le32(1);
	}

	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(req);
	return ret;
}

int hmdfs_client_start_mkdir(struct hmdfs_peer *con,
			     const char *path, const char *name,
			     umode_t mode, struct hmdfs_lookup_ret *mkdir_ret)
{
	int ret = 0;
	int path_len = strlen(path);
	int name_len = strlen(name);
	size_t send_len = sizeof(struct mkdir_request) + path_len + 1 +
			  name_len + 1;
	struct mkdir_request *mkdir_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_inodeinfo_response *resp = NULL;
	struct hmdfs_send_command sm = {
		.data = mkdir_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_MKDIR);
	if (!mkdir_req)
		return -ENOMEM;

	mkdir_req->path_len = cpu_to_le32(path_len);
	mkdir_req->name_len = cpu_to_le32(name_len);
	mkdir_req->mode = cpu_to_le16(mode);
	strncpy(mkdir_req->path, path, path_len);
	strncpy(mkdir_req->path + path_len + 1, name, name_len);

	ret = hmdfs_sendmessage_request(con, &sm);
	if (ret == -ENOENT || ret == -ETIME || ret == -ENOTSUPP)
		goto out;
	if (!sm.out_buf) {
		ret = -ENOENT;
		goto out;
	}
	resp = sm.out_buf;
	mkdir_ret->i_mode = le16_to_cpu(resp->i_mode);
	mkdir_ret->i_size = le64_to_cpu(resp->i_size);
	mkdir_ret->i_mtime = le64_to_cpu(resp->i_mtime);
	mkdir_ret->i_mtime_nsec = le32_to_cpu(resp->i_mtime_nsec);
	mkdir_ret->i_ino = le64_to_cpu(resp->i_ino);
#ifdef CONFIG_HMDFS_1_0
	mkdir_ret->fno = INVALID_FILE_ID;
#endif

out:
	free_sm_outbuf(&sm);
	kfree(mkdir_req);
	return ret;
}

int hmdfs_client_start_create(struct hmdfs_peer *con,
			      const char *path, const char *name,
			      umode_t mode, bool want_excl,
			      struct hmdfs_lookup_ret *create_ret)
{
	int ret = 0;
	int path_len = strlen(path);
	int name_len = strlen(name);
	size_t send_len = sizeof(struct create_request) + path_len + 1 +
			  name_len + 1;
	struct create_request *create_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_inodeinfo_response *resp = NULL;
	struct hmdfs_send_command sm = {
		.data = create_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_CREATE);
	if (!create_req)
		return -ENOMEM;

	create_req->path_len = cpu_to_le32(path_len);
	create_req->name_len = cpu_to_le32(name_len);
	create_req->mode = cpu_to_le16(mode);
	create_req->want_excl = want_excl;
	strncpy(create_req->path, path, path_len);
	strncpy(create_req->path + path_len + 1, name, name_len);

	ret = hmdfs_sendmessage_request(con, &sm);
	if (ret == -ENOENT || ret == -ETIME || ret == -ENOTSUPP)
		goto out;
	if (!sm.out_buf) {
		ret = -ENOENT;
		goto out;
	}
	resp = sm.out_buf;
	create_ret->i_mode = le16_to_cpu(resp->i_mode);
	create_ret->i_size = le64_to_cpu(resp->i_size);
	create_ret->i_mtime = le64_to_cpu(resp->i_mtime);
	create_ret->i_mtime_nsec = le32_to_cpu(resp->i_mtime_nsec);
	create_ret->i_ino = le64_to_cpu(resp->i_ino);
#ifdef CONFIG_HMDFS_1_0
	create_ret->fno = INVALID_FILE_ID;
#endif

out:
	free_sm_outbuf(&sm);
	kfree(create_req);
	return ret;
}

int hmdfs_client_start_rmdir(struct hmdfs_peer *con, const char *path,
			     const char *name)
{
	int ret;
	int path_len = strlen(path);
	int name_len = strlen(name);
	size_t send_len = sizeof(struct rmdir_request) + path_len + 1 +
			  name_len + 1;
	struct rmdir_request *rmdir_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = rmdir_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_RMDIR);
	if (!rmdir_req)
		return -ENOMEM;

	rmdir_req->path_len = cpu_to_le32(path_len);
	rmdir_req->name_len = cpu_to_le32(name_len);
	strncpy(rmdir_req->path, path, path_len);
	strncpy(rmdir_req->path + path_len + 1, name, name_len);

	ret = hmdfs_sendmessage_request(con, &sm);
	free_sm_outbuf(&sm);
	kfree(rmdir_req);
	return ret;
}

int hmdfs_client_start_unlink(struct hmdfs_peer *con, const char *path,
			      const char *name)
{
	int ret;
	int path_len = strlen(path);
	int name_len = strlen(name);
	size_t send_len = sizeof(struct unlink_request) + path_len + 1 +
			  name_len + 1;
	struct unlink_request *unlink_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = unlink_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_UNLINK);
	if (!unlink_req)
		return -ENOMEM;

	unlink_req->path_len = cpu_to_le32(path_len);
	unlink_req->name_len = cpu_to_le32(name_len);
	strncpy(unlink_req->path, path, path_len);
	strncpy(unlink_req->path + path_len + 1, name, name_len);

	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(unlink_req);
	free_sm_outbuf(&sm);
	return ret;
}

int hmdfs_client_start_rename(struct hmdfs_peer *con, const char *old_path,
			      const char *old_name, const char *new_path,
			      const char *new_name, unsigned int flags)
{
	int ret;
	int old_path_len = strlen(old_path);
	int new_path_len = strlen(new_path);
	int old_name_len = strlen(old_name);
	int new_name_len = strlen(new_name);

	size_t send_len = sizeof(struct rename_request) + old_path_len + 1 +
			  new_path_len + 1 + old_name_len + 1 + new_name_len +
			  1;
	struct rename_request *rename_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = rename_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_RENAME);
	if (!rename_req)
		return -ENOMEM;

	rename_req->old_path_len = cpu_to_le32(old_path_len);
	rename_req->new_path_len = cpu_to_le32(new_path_len);
	rename_req->old_name_len = cpu_to_le32(old_name_len);
	rename_req->new_name_len = cpu_to_le32(new_name_len);
	rename_req->flags = cpu_to_le32(flags);

	strncpy(rename_req->path, old_path, old_path_len);
	strncpy(rename_req->path + old_path_len + 1, new_path, new_path_len);

	strncpy(rename_req->path + old_path_len + 1 + new_path_len + 1,
		old_name, old_name_len);
	strncpy(rename_req->path + old_path_len + 1 + new_path_len + 1 +
			old_name_len + 1,
		new_name, new_name_len);

	ret = hmdfs_sendmessage_request(con, &sm);
	free_sm_outbuf(&sm);
	kfree(rename_req);
	return ret;
}

int hmdfs_send_setattr(struct hmdfs_peer *con, const char *send_buf,
		       struct setattr_info *attr_info)
{
	int ret;
	int path_len = strlen(send_buf);
	size_t send_len = path_len + 1 + sizeof(struct setattr_request);
	struct setattr_request *setattr_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = setattr_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_SETATTR);
	if (!setattr_req)
		return -ENOMEM;

	strcpy(setattr_req->buf, send_buf);
	setattr_req->path_len = cpu_to_le32(path_len);
	setattr_req->valid = cpu_to_le32(attr_info->valid);
	setattr_req->size = cpu_to_le64(attr_info->size);
	setattr_req->mtime = cpu_to_le64(attr_info->mtime);
	setattr_req->mtime_nsec = cpu_to_le32(attr_info->mtime_nsec);
	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(setattr_req);
	return ret;
}

static void hmdfs_update_getattr_ret(struct getattr_response *resp,
				     struct hmdfs_getattr_ret *result)
{
	struct kstat *stat = &result->stat;

	stat->result_mask = le32_to_cpu(resp->result_mask);
	if (stat->result_mask == 0)
		return;

	stat->ino = le64_to_cpu(resp->ino);
	stat->mode = le16_to_cpu(resp->mode);
	stat->nlink = le32_to_cpu(resp->nlink);
	stat->uid.val = le32_to_cpu(resp->uid);
	stat->gid.val = le32_to_cpu(resp->gid);
	stat->size = le64_to_cpu(resp->size);
	stat->blocks = le64_to_cpu(resp->blocks);
	stat->blksize = le32_to_cpu(resp->blksize);
	stat->atime.tv_sec = le64_to_cpu(resp->atime);
	stat->atime.tv_nsec = le32_to_cpu(resp->atime_nsec);
	stat->mtime.tv_sec = le64_to_cpu(resp->mtime);
	stat->mtime.tv_nsec = le32_to_cpu(resp->mtime_nsec);
	stat->ctime.tv_sec = le64_to_cpu(resp->ctime);
	stat->ctime.tv_nsec = le32_to_cpu(resp->ctime_nsec);
	stat->btime.tv_sec = le64_to_cpu(resp->crtime);
	stat->btime.tv_nsec = le32_to_cpu(resp->crtime_nsec);
	result->fsid = le64_to_cpu(resp->fsid);
	/* currently not used */
	result->i_flags = 0;
}

int hmdfs_send_getattr(struct hmdfs_peer *con, const char *send_buf,
		       unsigned int lookup_flags,
		       struct hmdfs_getattr_ret *result)
{
	int path_len = strlen(send_buf);
	size_t send_len = path_len + 1 + sizeof(struct getattr_request);
	int ret = 0;
	struct getattr_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_GETATTR);
	if (!req)
		return -ENOMEM;

	req->path_len = cpu_to_le32(path_len);
	req->lookup_flags = cpu_to_le32(lookup_flags);
	strncpy(req->buf, send_buf, path_len);
	ret = hmdfs_sendmessage_request(con, &sm);
	if (!ret && (sm.out_len == 0 || !sm.out_buf))
		ret = -ENOENT;
	if (ret)
		goto out;

	hmdfs_update_getattr_ret(sm.out_buf, result);

out:
	kfree(req);
	free_sm_outbuf(&sm);
	return ret;
}

static void hmdfs_update_statfs_ret(struct statfs_response *resp,
				    struct kstatfs *buf)
{
	buf->f_type = le64_to_cpu(resp->f_type);
	buf->f_bsize = le64_to_cpu(resp->f_bsize);
	buf->f_blocks = le64_to_cpu(resp->f_blocks);
	buf->f_bfree = le64_to_cpu(resp->f_bfree);
	buf->f_bavail = le64_to_cpu(resp->f_bavail);
	buf->f_files = le64_to_cpu(resp->f_files);
	buf->f_ffree = le64_to_cpu(resp->f_ffree);
	buf->f_fsid.val[0] = le32_to_cpu(resp->f_fsid_0);
	buf->f_fsid.val[1] = le32_to_cpu(resp->f_fsid_1);
	buf->f_namelen = le64_to_cpu(resp->f_namelen);
	buf->f_frsize = le64_to_cpu(resp->f_frsize);
	buf->f_flags = le64_to_cpu(resp->f_flags);
	buf->f_spare[0] = le64_to_cpu(resp->f_spare_0);
	buf->f_spare[1] = le64_to_cpu(resp->f_spare_1);
	buf->f_spare[2] = le64_to_cpu(resp->f_spare_2);
	buf->f_spare[3] = le64_to_cpu(resp->f_spare_3);
}

int hmdfs_send_statfs(struct hmdfs_peer *con, const char *path,
		      struct kstatfs *buf)
{
	int ret;
	int path_len = strlen(path);
	size_t send_len = sizeof(struct statfs_request) + path_len + 1;
	struct statfs_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_STATFS);
	if (!req)
		return -ENOMEM;

	req->path_len = cpu_to_le32(path_len);
	strncpy(req->path, path, path_len);

	ret = hmdfs_sendmessage_request(con, &sm);

	if (ret == -ETIME)
		ret = -EIO;
	if (!ret && (sm.out_len == 0 || !sm.out_buf))
		ret = -ENOENT;
	if (ret)
		goto out;

	hmdfs_update_statfs_ret(sm.out_buf, buf);
out:
	kfree(req);
	free_sm_outbuf(&sm);
	return ret;
}

int hmdfs_send_syncfs(struct hmdfs_peer *con, int syncfs_timeout)
{
	int ret;
	struct hmdfs_req req;
	struct hmdfs_sb_info *sbi = con->sbi;
	struct syncfs_request *syncfs_req =
		kzalloc(sizeof(struct syncfs_request), GFP_KERNEL);

	if (!syncfs_req) {
		hmdfs_err("cannot allocate syncfs_request");
		return -ENOMEM;
	}

	hmdfs_init_cmd(&req.operations, F_SYNCFS);
	req.timeout = syncfs_timeout;

	syncfs_req->version = cpu_to_le64(sbi->hsi.version);
	req.data = syncfs_req;
	req.data_len = sizeof(*syncfs_req);

	ret = hmdfs_send_async_request(con, &req);
	if (ret) {
		kfree(syncfs_req);
		hmdfs_err("ret fail with %d", ret);
	}

	return ret;
}

static void hmdfs_update_getxattr_ret(struct getxattr_response *resp,
				     void *value, size_t o_size, int *ret)
{
	ssize_t size = le32_to_cpu(resp->size);

	if (o_size && o_size < size) {
		*ret = -ERANGE;
		return;
	}

	if (o_size)
		memcpy(value, resp->value, size);

	*ret = size;
}

int hmdfs_send_getxattr(struct hmdfs_peer *con, const char *send_buf,
			const char *name, void *value, size_t size)
{
	size_t path_len = strlen(send_buf);
	size_t name_len = strlen(name);
	size_t send_len = path_len + name_len +
			  sizeof(struct getxattr_request) + 2;
	int ret = 0;
	struct getxattr_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_GETXATTR);
	if (!req)
		return -ENOMEM;

	req->path_len = cpu_to_le32(path_len);
	req->name_len = cpu_to_le32(name_len);
	req->size = cpu_to_le32(size);
	strncpy(req->buf, send_buf, path_len);
	strncpy(req->buf + path_len + 1, name, name_len);
	ret = hmdfs_sendmessage_request(con, &sm);
	if (!ret && (sm.out_len == 0 || !sm.out_buf))
		ret = -ENOENT;
	if (ret)
		goto out;

	hmdfs_update_getxattr_ret(sm.out_buf, value, size, &ret);

out:
	kfree(req);
	free_sm_outbuf(&sm);
	return ret;
}

int hmdfs_send_setxattr(struct hmdfs_peer *con, const char *send_buf,
			const char *name, const void *value,
			size_t size, int flags)
{
	size_t path_len = strlen(send_buf);
	size_t name_len = strlen(name);
	size_t send_len = path_len + name_len + size + 2 +
			  sizeof(struct setxattr_request);
	int ret = 0;
	struct setxattr_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_SETXATTR);
	if (!req)
		return -ENOMEM;

	req->path_len = cpu_to_le32(path_len);
	req->name_len = cpu_to_le32(name_len);
	req->size = cpu_to_le32(size);
	req->flags = cpu_to_le32(flags);
	strncpy(req->buf, send_buf, path_len);
	strncpy(req->buf + path_len + 1, name, name_len);
	memcpy(req->buf + path_len + name_len + 2, value, size);
	if (!value)
		req->del = true;
	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(req);
	return ret;
}

static void hmdfs_update_listxattr_ret(struct listxattr_response *resp,
				       char *list, size_t o_size, ssize_t *ret)
{
	ssize_t size = le32_to_cpu(resp->size);

	if (o_size && o_size < size) {
		*ret = -ERANGE;
		return;
	}

	/* multi name split with '\0', use memcpy */
	if (o_size)
		memcpy(list, resp->list, size);

	*ret = size;
}

ssize_t hmdfs_send_listxattr(struct hmdfs_peer *con, const char *send_buf,
			     char *list, size_t size)
{
	size_t path_len = strlen(send_buf);
	size_t send_len = path_len + 1 + sizeof(struct listxattr_request);
	ssize_t ret = 0;
	struct listxattr_request *req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_LISTXATTR);
	if (!req)
		return -ENOMEM;

	req->path_len = cpu_to_le32(path_len);
	req->size = cpu_to_le32(size);
	strncpy(req->buf, send_buf, path_len);
	ret = hmdfs_sendmessage_request(con, &sm);
	if (!ret && (sm.out_len == 0 || !sm.out_buf))
		ret = -ENOENT;
	if (ret)
		goto out;

	hmdfs_update_listxattr_ret(sm.out_buf, list, size, &ret);

out:
	kfree(req);
	free_sm_outbuf(&sm);
	return ret;
}

void hmdfs_recv_syncfs_cb(struct hmdfs_peer *peer, const struct hmdfs_req *req,
			  const struct hmdfs_resp *resp)
{
	struct hmdfs_sb_info *sbi = peer->sbi;
	struct syncfs_request *syncfs_req = (struct syncfs_request *)req->data;

	WARN_ON(!syncfs_req);
	spin_lock(&sbi->hsi.v_lock);
	if (le64_to_cpu(syncfs_req->version) != sbi->hsi.version) {
		hmdfs_info(
			"Recv stale syncfs resp[ver: %llu] from device %llu, current ver %llu",
			le64_to_cpu(syncfs_req->version), peer->device_id,
			sbi->hsi.version);
		spin_unlock(&sbi->hsi.v_lock);
		goto out;
	}

	if (!sbi->hsi.remote_ret)
		sbi->hsi.remote_ret = resp->ret_code;

	if (resp->ret_code) {
		hmdfs_err("Recv syncfs error code %d from device %llu",
			  resp->ret_code, peer->device_id);
	} else {
		/*
		 * Set @sb_dirty_count to zero if no one else produce
		 * dirty data on remote server during remote sync.
		 */
		atomic64_cmpxchg(&peer->sb_dirty_count,
				 peer->old_sb_dirty_count, 0);
	}

	atomic_dec(&sbi->hsi.wait_count);
	spin_unlock(&sbi->hsi.v_lock);
	wake_up_interruptible(&sbi->hsi.wq);

out:
	kfree(syncfs_req);
}

void hmdfs_send_drop_push(struct hmdfs_peer *con, const char *path)
{
	int path_len = strlen(path);
	size_t send_len = sizeof(struct drop_push_request) + path_len + 1;
	struct drop_push_request *dp_req = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = dp_req,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_DROP_PUSH);
	if (!dp_req)
		return;

	dp_req->path_len = cpu_to_le32(path_len);
	strncpy(dp_req->path, path, path_len);

	hmdfs_sendmessage_request(con, &sm);
	kfree(dp_req);
}

static void *hmdfs_get_msg_next(struct hmdfs_peer *peer, int *id)
{
	struct hmdfs_msg_idr_head *head = NULL;

	spin_lock(&peer->idr_lock);
	head = idr_get_next(&peer->msg_idr, id);
	if (head && head->type < MSG_IDR_MAX && head->type >= 0)
		kref_get(&head->ref);

	spin_unlock(&peer->idr_lock);

	return head;
}

void hmdfs_client_offline_notify(struct hmdfs_peer *conn, int evt,
				 unsigned int seq)
{
	int id;
	int count = 0;
	struct hmdfs_msg_idr_head *head = NULL;

	for (id = 0; (head = hmdfs_get_msg_next(conn, &id)) != NULL; ++id) {
		switch (head->type) {
		case MSG_IDR_1_0_NONE:
			head_put(head);
			head_put(head);
			break;
		case MSG_IDR_MESSAGE_SYNC:
		case MSG_IDR_1_0_MESSAGE_SYNC:
			hmdfs_response_wakeup((struct sendmsg_wait_queue *)head,
					      -ETIME, 0, NULL);
			hmdfs_debug("wakeup id=%d", head->msg_id);
			msg_put((struct sendmsg_wait_queue *)head);
			break;
		case MSG_IDR_MESSAGE_ASYNC:
			hmdfs_wakeup_parasite(
				(struct hmdfs_msg_parasite *)head);
			hmdfs_debug("wakeup parasite id=%d", head->msg_id);
			mp_put((struct hmdfs_msg_parasite *)head);
			break;
		case MSG_IDR_PAGE:
		case MSG_IDR_1_0_PAGE:
			hmdfs_wakeup_async_work(
				(struct hmdfs_async_work *)head);
			hmdfs_debug("wakeup async work id=%d", head->msg_id);
			asw_put((struct hmdfs_async_work *)head);
			break;
		default:
			hmdfs_err("Bad type=%d id=%d", head->type,
				  head->msg_id);
			break;
		}

		count++;
		/* If there are too many idr to process, avoid to soft lockup,
		 * process every 512 message we resched
		 */
		if (count % HMDFS_IDR_RESCHED_COUNT == 0)
			cond_resched();
	}
}

static struct hmdfs_node_cb_desc client_cb[] = {
	{
		.evt = NODE_EVT_OFFLINE,
		.sync = true,
		.min_version = DFS_1_0,
		.fn = hmdfs_client_offline_notify,
	},
};

void __init hmdfs_client_add_node_evt_cb(void)
{
	hmdfs_node_add_evt_cb(client_cb, ARRAY_SIZE(client_cb));
}
