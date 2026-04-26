/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dentry_syncer.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: wangminmin4@huawei.com
 * Create: 2020-04-25
 *
 */

#ifndef HMDFS_DENTRY_SYNCER_H
#define HMDFS_DENTRY_SYNCER_H

#include "hmdfs_dentryfile.h"

#define DENTRY_SYNCER_CHRDEV_NAME "hmdfs_dentry_syncer"

enum syncer_req_type {
	INVALID_ = 0,
	// From dentry to metaservice
	DENTRY_LOOKUP_REQUEST,
	DENTRY_LOOKUP_RESPONSE,
	DENTRY_ITERATE_REQUEST,
	DENTRY_ITERATE_RESPONSE,

	META_CREATE_REQUEST,
	META_CREATE_RESPONSE,
	META_REMOVE_REQUEST,
	META_REMOVE_RESPONSE,
	META_RENAME_REQUEST,
	META_RENAME_RESPONSE,
	META_UPDATE_REQUEST,
	META_UPDATE_RESPONSE,

	WAKEUP_REQUEST,
	WAKEUP_RESPONSE,
	SYNCER_REQ_TYPE_MAX,
};

enum syncer_req_status {
	START = 0,
	WORKING,
	SUCCESS,
	FAIL,
};

struct metabase {
	__u64 ctime;
	__u64 mtime;
	__u64 atime;
	__u64 size;
	__u32 mode;
	uid_t uid;
	gid_t gid;
	int32_t nlink;
	__u32 rdev;
	__u64 dno;
	__u32 fno;
	/* Length of account and file's parent dir relative path in hmdfs */
	__u32 path_len;
	/* Length of file name */
	__u32 name_len;
	/* Whether the file is symlink */
	__u32 is_symlink;
	/* Length of source device path (local src) */
	__u32 src_path_len;
	/* Length of file's parent dir relative path in hmdfs */
	__u32 relative_dir_len;
	/*
	 * Account + relative file path in hmdfs. If @is_symlink is not 0,
	 * source device path (local src) is also appended after file path.
	 */
	char path[];
};

struct lookup_request {
	__u64 device_id;
	__u32 path_len;
	__u32 name_len;
	char path[];
};

struct iterate_request {
	__u32 start;
	__u32 count;
	__u64 device_id;
	__u32 path_len;
	char path[];
};

struct remove_request {
	__u32 path_len;
	__u32 name_len;
	char path[];
};

struct _rename_request {
	int32_t is_dir;
	__u32 flags;
	__u32 oldparent_len;
	__u32 oldname_len;
	__u32 newparent_len;
	__u32 newname_len;
	char path[];
};

struct syncer_req {
	enum syncer_req_type type;
	enum syncer_req_status status;
	__u32 reqid;
	__u32 len;
	char req_buf[];
};

int dentry_syncer_lookup(struct lookup_request *lookup_req,
			 struct metabase *result, int len);
int dentry_syncer_iterate(struct file *file, __u64 dev_id, const char *path,
			  struct dir_context *ctx);
void dentry_syncer_create(struct metabase *create_req);
void dentry_syncer_update(struct metabase *update_req);
void dentry_syncer_remove(struct remove_request *remove_req);
void dentry_syncer_rename(struct _rename_request *rename_req);
void dentry_syncer_wakeup(void);

int dentry_syncer_init(void);
void dentry_syncer_exit(void);

#endif
