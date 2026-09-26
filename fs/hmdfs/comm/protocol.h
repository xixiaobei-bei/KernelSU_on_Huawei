/* SPDX-License-Identifier: GPL-2.0 */
/*
 * protocol.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: transport adapt for hmdfs
 * Author: maojingjing1@huawei.com
 * Create: 2020-03-26
 *
 */

#ifndef HMDFS_PROTOCOL_H
#define HMDFS_PROTOCOL_H

#include <linux/kref.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/namei.h>

struct hmdfs_cmd {
	__u8 reserved;
	__u8 cmd_flag;
	__u8 command;
	__u8 reserved2;
} __packed;

#define HMDFS_MSG_MAGIC	      0xF7
#define HMDFS_MAX_MESSAGE_LEN (8 * 1024 * 1024)

struct hmdfs_head_cmd {
	__u8 magic;
	__u8 version;
	__le16 reserved;
	__le32 data_len;
	struct hmdfs_cmd operations;
	__le32 ret_code;
	__le32 msg_id;
	__le32 reserved1;
} __packed;

enum FILE_RECV_STATE {
	FILE_RECV_PROCESS = 0,
	FILE_RECV_SUCC,
	FILE_RECV_ERR_NET,
	FILE_RECV_ERR_SPC,
};

struct file_recv_info {
	void *local_filp;
	atomic_t local_fslices;
	atomic_t state;
};

enum MSG_IDR_TYPE {
	MSG_IDR_1_0_NONE = 0,
	MSG_IDR_1_0_MESSAGE_SYNC,
	MSG_IDR_1_0_PAGE,
	MSG_IDR_MESSAGE_SYNC,
	MSG_IDR_MESSAGE_ASYNC,
	MSG_IDR_PAGE,
	MSG_IDR_MAX,
};

struct hmdfs_msg_idr_head {
	__u32 type;
	__u32 msg_id;
	struct kref ref;
	struct hmdfs_peer *peer;
};

struct sendmsg_wait_queue {
	struct hmdfs_msg_idr_head head;
	wait_queue_head_t response_q;
	struct list_head async_msg;
	atomic_t valid;
	__u32 size;
	void *buf;
	__u32 ret;
	unsigned long start;
	struct file_recv_info recv_info;
};

struct hmdfs_send_command {
	struct hmdfs_cmd operations;
	void *data;
	size_t len;
	void *local_filp;
	void *out_buf;
	size_t out_len;
	__u32 ret_code;
};

struct hmdfs_req {
	struct hmdfs_cmd operations;
	/*
	 * Normally, the caller ought set timeout to TIMEOUT_CONFIG, so that
	 * hmdfs_send_async_request will search s_cmd_timeout for the user-
	 * configured timeout values.
	 *
	 * However, consider the given scenery:
	 * The caller may want to issue multiple requests sharing the same
	 * timeout value, but the users may update the value during the gap.
	 * To ensure the "atomicty" of timeout-using for these requests, we
	 * provide the timeout field for hacking.
	 */
	unsigned int timeout;
	void *data;
	size_t data_len;

	void *private; // optional
	size_t private_len; // optional
};

struct hmdfs_resp {
	void *out_buf;
	size_t out_len;
	__u32 ret_code;
};

struct hmdfs_msg_parasite {
	struct hmdfs_msg_idr_head head;
	struct delayed_work d_work;
	bool wfired;
	struct hmdfs_req req;
	struct hmdfs_resp resp;
	unsigned long start;
};

struct hmdfs_send_data {
	// sect1: head
	void *head;
	size_t head_len;

	// sect2: <optional> slice descriptor
	void *sdesc;
	size_t sdesc_len;

	// sect3: request / response / file slice
	void *data;
	size_t len;
};

struct slice_descriptor {
	__le32 num_slices;
	__le32 slice_size;
	__le32 slice_sn;
	__le32 content_size;
} __packed;

enum DFS_VERSION {
	INVALID_VERSION = 0,
	DFS_1_0,

	USERSPACE_MAX_VER = 0x3F,
	DFS_2_0,

	MAX_VERSION = 0xFF
};

enum CONN_OPERATIONS_VERSION { USERDFS_VERSION, PROTOCOL_VERSION };

enum CMD_FLAG { C_REQUEST = 0, C_RESPONSE = 1, C_FLAG_SIZE };

enum FILE_CMD {
	F_OPEN = 0,
	F_RELEASE = 1,
	F_READPAGE = 2,
	F_WRITEPAGE = 3,
	F_ITERATE = 4,
	F_RESERVED_1 = 5,
	F_RESERVED_2 = 6,
	F_RESERVED_3 = 7,
	F_RESERVED_4 = 8,
	F_MKDIR = 9,
	F_RMDIR = 10,
	F_CREATE = 11,
	F_UNLINK = 12,
	F_RENAME = 13,
	F_SETATTR = 14,
	F_CONNECT_ECHO = 15,
	F_STATFS = 16,
	F_CONNECT_REKEY = 17,
	F_DROP_PUSH = 18,
	F_RESERVED_0 = 19,
	F_GETATTR = 20,
	F_FSYNC = 21,
	F_SYNCFS = 22,
	F_GETXATTR = 23,
	F_SETXATTR = 24,
	F_LISTXATTR = 25,
	F_READPAGES = 26,
	F_READPAGES_OPEN = 27,
	F_ATOMIC_OPEN = 28,
	F_SIZE,
};

struct open_request {
	__u8 file_type;
	__le32 flags;
	__le32 path_len;
	char buf[0];
} __packed;

struct open_response {
	__le32 change_detect_cap;
	__le64 file_ver;
	__le32 file_id;
	__le64 file_size;
	__le64 ino;
	__le64 ctime;
	__le32 ctime_nsec;
	__le64 mtime;
	__le32 mtime_nsec;
	__le64 stable_ctime;
	__le32 stable_ctime_nsec;
	__le64 ichange_count;
} __packed;

enum hmdfs_open_flags {
	HMDFS_O_TRUNC = O_TRUNC,
	HMDFS_O_EXCL = O_EXCL,
};

struct atomic_open_request {
	__le32 open_flags;
	__le16 mode;
	__le16 reserved1;
	__le32 path_len;
	__le32 file_len;
	__le64 reserved2[4];
	char buf[0];
} __packed;

struct atomic_open_response {
	__le32 fno;
	__le16 i_mode;
	__le16 reserved1;
	__le32 i_flags;
	__le32 reserved2;
	__le64 reserved3[4];
	struct open_response open_resp;
} __packed;

struct release_request {
	__le64 file_ver;
	__le32 file_id;
} __packed;

struct fsync_request {
	__le64 file_ver;
	__le32 file_id;
	__le32 datasync;
	__le64 start;
	__le64 end;
} __packed;

struct readpage_request {
	__le64 file_ver;
	__le32 file_id;
	__le32 size;
	__le64 index;
} __packed;

struct readpage_response {
	char buf[0];
} __packed;

struct readpages_request {
	__le64 file_ver;
	__le32 file_id;
	__le32 size;
	__le64 index;
	__le64 reserved;
} __packed;

struct readpages_response {
	char buf[0];
} __packed;

struct readpages_open_request {
	__u8 file_type;
	__u8 reserved1[3];
	__le32 flags;
	__le32 path_len;
	__le32 size;
	__le64 index;
	__le64 reserved2;
	char buf[0];
} __packed;

struct readpages_open_response {
	struct open_response open_resp;
	__le64 reserved[4];
	char buf[0];
} __packed;

struct writepage_request {
	__le64 file_ver;
	__le32 file_id;
	__le64 index;
	__le32 count;
	char buf[0];
} __packed;

struct writepage_response {
	__le64 ichange_count;
	__le64 ctime;
	__le32 ctime_nsec;
} __packed;

struct readdir_request {
	__le64 dcache_crtime;
	__le64 dcache_crtime_nsec;
	__le64 dentry_ctime;
	__le64 dentry_ctime_nsec;
	__le64 num;
	__le32 verify_cache;
	__le32 path_len;
	char path[0];
} __packed;

struct hmdfs_inodeinfo_response {
	__le64 i_size;
	__le64 i_mtime;
	__le32 i_mtime_nsec;
	__le32 fno;
	__le16 i_mode;
	__le64 i_ino;
	__le32 i_flags;
	__le32 i_reserved;
} __packed;

struct mkdir_request {
	__le32 path_len;
	__le32 name_len;
	__le16 mode;
	char path[0];
} __packed;

struct create_request {
	__le32 path_len;
	__le32 name_len;
	__le16 mode;
	__u8 want_excl;
	char path[0];
} __packed;

struct rmdir_request {
	__le32 path_len;
	__le32 name_len;
	char path[0];
} __packed;

struct unlink_request {
	__le32 path_len;
	__le32 name_len;
	char path[0];
} __packed;

struct rename_request {
	__le32 old_path_len;
	__le32 new_path_len;
	__le32 old_name_len;
	__le32 new_name_len;
	__le32 flags;
	char path[0];
} __packed;

struct drop_push_request {
	__le32 path_len;
	char path[0];
} __packed;

struct setattr_request {
	__le64 size;
	__le32 valid;
	__le16 mode;
	__le32 uid;
	__le32 gid;
	__le64 atime;
	__le32 atime_nsec;
	__le64 mtime;
	__le32 mtime_nsec;
	__le32 path_len;
	char buf[0];
} __packed;

struct getattr_request {
	__le32 lookup_flags;
	__le32 path_len;
	char buf[0];
} __packed;

struct getattr_response {
	__le32 change_detect_cap;
	__le32 result_mask;
	__le32 flags;
	__le64 fsid;
	__le16 mode;
	__le32 nlink;
	__le32 uid;
	__le32 gid;
	__le32 rdev;
	__le64 ino;
	__le64 size;
	__le64 blocks;
	__le32 blksize;
	__le64 atime;
	__le32 atime_nsec;
	__le64 mtime;
	__le32 mtime_nsec;
	__le64 ctime;
	__le32 ctime_nsec;
	__le64 crtime;
	__le32 crtime_nsec;
	__le64 ichange_count;
} __packed;

struct statfs_request {
	__le32 path_len;
	char path[0];
} __packed;

struct statfs_response {
	__le64 f_type;
	__le64 f_bsize;
	__le64 f_blocks;
	__le64 f_bfree;
	__le64 f_bavail;
	__le64 f_files;
	__le64 f_ffree;
	__le32 f_fsid_0;
	__le32 f_fsid_1;
	__le64 f_namelen;
	__le64 f_frsize;
	__le64 f_flags;
	__le64 f_spare_0;
	__le64 f_spare_1;
	__le64 f_spare_2;
	__le64 f_spare_3;
} __packed;

struct syncfs_request {
	__le64 version;
	__le32 flags;
} __packed;

struct getxattr_request {
	__le32 path_len;
	__le32 name_len;
	__le32 size;
	char buf[0];
} __packed;

struct getxattr_response {
	__le32 size;
	char value[0]; /* xattr value may non-printable */
} __packed;

struct setxattr_request {
	__le32 path_len;
	__le32 name_len;
	__le32 size;
	__le32 flags;
	__u8 del; /* remove xattr */
	char buf[0];
} __packed;

struct listxattr_request {
	__le32 path_len;
	__le32 size;
	char buf[0];
} __packed;

struct listxattr_response {
	__le32 size;
	char list[0];
} __packed;

struct connection_rekey_request {
	__le32 update_request;
} __packed;

enum CONNECTION_KEY_UPDATE_REQUEST {
	UPDATE_NOT_REQUESTED = 0,
	UPDATE_REQUESTED = 1
};

enum MSG_QUEUE_STATUS {
	MSG_Q_SEND = 0,
	MSG_Q_END_RECV,
};
#endif
