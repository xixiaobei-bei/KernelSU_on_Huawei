/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_client.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: maojingjing@huawei.com
 * Create: 2020-04-09
 *
 */

#ifndef HMDFS_CLIENT_H
#define HMDFS_CLIENT_H

#include "comm/transport.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"

struct hmdfs_open_ret {
	struct hmdfs_fid fid;
	__u64 file_size;
	__u64 ino;
	struct hmdfs_time_t remote_ctime;
	struct hmdfs_time_t stable_ctime;
};

struct hmdfs_writepage_context {
	struct hmdfs_fid fid;
	uint32_t count;
	bool sync_all;
	bool rsem_held;
	unsigned long timeout;
	struct task_struct *caller;
	struct page *page;
	struct delayed_work retry_dwork;
};

int hmdfs_client_start_readdir(struct hmdfs_peer *con, struct file *filp,
			       const char *path, int path_len,
			       struct hmdfs_dcache_header *header);
int hmdfs_client_start_mkdir(struct hmdfs_peer *con,
			     const char *path, const char *name,
			     umode_t mode, struct hmdfs_lookup_ret *mkdir_ret);
int hmdfs_client_start_create(struct hmdfs_peer *con,
			      const char *path, const char *name,
			      umode_t mode, bool want_excl,
			      struct hmdfs_lookup_ret *create_ret);
int hmdfs_client_start_rmdir(struct hmdfs_peer *con, const char *path,
			     const char *name);
int hmdfs_client_start_unlink(struct hmdfs_peer *con, const char *path,
			      const char *name);
int hmdfs_client_start_rename(struct hmdfs_peer *con, const char *old_path,
			      const char *old_name, const char *new_path,
			      const char *new_name, unsigned int flags);

static inline bool hmdfs_is_offline_err(int err)
{
	/*
	 * writepage() will get -EBADF if peer is online
	 * again during offline stash, and -EBADF also
	 * needs redo.
	 */
	return (err == -EAGAIN || err == -ESHUTDOWN || err == -EBADF);
}

static inline bool hmdfs_is_offline_or_timeout_err(int err)
{
	return hmdfs_is_offline_err(err) || err == -ETIME;
}

static inline bool hmdfs_need_redirty_page(const struct hmdfs_inode_info *info,
					   int err)
{
	/*
	 * 1. stash is enabled
	 * 2. offline related error
	 * 3. no restore
	 */
	return hmdfs_is_stash_enabled(info->conn->sbi) &&
	       hmdfs_is_offline_err(err) &&
	       READ_ONCE(info->stash_status) != HMDFS_REMOTE_INODE_RESTORING;
}

bool hmdfs_usr_sig_pending(struct task_struct *p);
void hmdfs_writepage_cb(struct hmdfs_peer *peer, const struct hmdfs_req *req,
			const struct hmdfs_resp *resp);
int hmdfs_client_writepage(struct hmdfs_peer *con,
			   struct hmdfs_writepage_context *param);
int hmdfs_remote_do_writepage(struct hmdfs_peer *con,
			      struct hmdfs_writepage_context *ctx);
void hmdfs_remote_writepage_retry(struct work_struct *work);

void hmdfs_client_writepage_done(struct hmdfs_inode_info *info,
				 struct hmdfs_writepage_context *ctx);

int hmdfs_send_open(struct hmdfs_peer *con, const char *send_buf,
		    __u8 file_type, struct hmdfs_open_ret *open_ret);
void hmdfs_send_close(struct hmdfs_peer *con, const struct hmdfs_fid *fid);
int hmdfs_send_fsync(struct hmdfs_peer *con, const struct hmdfs_fid *fid,
		     __s64 start, __s64 end, __s32 datasync);
int hmdfs_client_readpage(struct hmdfs_peer *con, const struct hmdfs_fid *fid,
			  struct page *page);

int hmdfs_send_setattr(struct hmdfs_peer *con, const char *send_buf,
		       struct setattr_info *attr_info);
int hmdfs_send_getattr(struct hmdfs_peer *con, const char *send_buf,
		       unsigned int lookup_flags,
		       struct hmdfs_getattr_ret *getattr_result);
int hmdfs_send_statfs(struct hmdfs_peer *con, const char *path,
		      struct kstatfs *buf);
void hmdfs_client_recv_readpage(struct hmdfs_head_cmd *head, int err,
				struct hmdfs_async_work *async_work);
int hmdfs_send_syncfs(struct hmdfs_peer *con, int syncfs_timeout);
int hmdfs_send_getxattr(struct hmdfs_peer *con, const char *send_buf,
			const char *name, void *value, size_t size);
int hmdfs_send_setxattr(struct hmdfs_peer *con, const char *send_buf,
			const char *name, const void *val,
			size_t size, int flags);
ssize_t hmdfs_send_listxattr(struct hmdfs_peer *con, const char *send_buf,
			     char *list, size_t size);
void hmdfs_recv_syncfs_cb(struct hmdfs_peer *peer, const struct hmdfs_req *req,
			  const struct hmdfs_resp *resp);

void __init hmdfs_client_add_node_evt_cb(void);
#endif
