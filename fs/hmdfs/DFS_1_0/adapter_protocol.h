/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_proctol.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: hmdfs EMUI adapter - header for message protocol
 * Author: koujilong@huawei.com
 * Create: 2020-04-01
 *
 */

#ifndef HMDFS_ADAPTER_PROTOCOL_H
#define HMDFS_ADAPTER_PROTOCOL_H

#include <linux/types.h>

#define HMDFS_FLAG_ENCRYPTED	     0x1
#define ADAPTER_LOCK_DEFAULT_TIMEOUT 3000 // ms

enum ADAPTER_MSG_OPERATION {
	INVALID_MSG = 0,
	HANDSHAKE_REQUEST,
	HANDSHAKE_RESPONSE,
	READ_REQUEST,
	READ_RESPONSE,
	WRITE_REQUEST,
	WRITE_RESPONSE,
	CLOSE_REQUEST,
	CLOSE_RESPONSE,
	DELETE_REQUEST,
	DELETE_RESPONSE,
	OPEN_REQUEST,
	OPEN_RESPONSE,
	READ_META_REQUEST,
	READ_META_RESPONSE,
	GET_WLOCK_REQUEST,
	GET_WLOCK_RESPONSE,
	GET_RLOCK_REQUEST,
	GET_RLOCK_RESPONSE,
	RELEASE_LOCK_REQUEST,
	RELEASE_LOCK_RESPONSE,
	SYNC_REQUEST,
	SYNC_RESPONSE,
	SET_FSIZE_REQUEST,
	SET_FSIZE_RESPONSE,
	DB_SYNC_MSG, /* Unused in kernel filesystem */
	MOVE_REQUEST,
	MOVE_RESPONSE,
	PULL_REQUEST,
	PULL_RESPONSE,
	HEART_BEAT_MSG, /* Unused in kernel filesystem */
	FLUSH_REQUEST,
	FLUSH_RESPONSE,
	CLEAN_CACHE_REQUEST,
	CLEAN_CACHE_RESPONSE,
	CREATE_REQUEST,
	CREATE_RESPONSE,
	RENAME_REQUEST,
	RENAME_RESPONSE,

	META_PULL_BEGIN = 0x40,
	/* Unused in kernel filesystem */
	META_PULL_REQUEST = META_PULL_BEGIN,
	META_PULL_END = 0x4f
};

enum MSG_TIME_OUT {
	NO_WAIT = 0,
	MSG_DEFAULT_TIMEOUT = 4,
};

struct hmdfs_adapter_head {
	__u8 magic;
	__u8 version;
	__u8 operations;
	__u8 flags;
	__u32 datasize;
	__u64 source;
	__u16 msg_id;
	__u16 request_id;
};

struct hmdfs_adapter_msg {
	struct hmdfs_adapter_head hdr;
	char buf[0];
};

struct adapter_req_info {
	__u32 fno;
	__u64 src_dno;
};

struct adapter_handshake_ack {
	__u32 len;
	char dev_id[0];
};

struct adapter_read_request {
	struct adapter_req_info req_info;
	struct {
		__u32 lock_id;
		__u64 offset;
		__u64 size;
		__u32 timeout;
		__u8 block;
	};
} __packed;

struct adapter_read_ack {
	__u32 status;
	__u64 size;
	char data_buf[0];
} __packed;

struct adapter_write_data_buffer {
	__u32 size;
	__u32 payload_size;
	__u64 offset;
	char buf[0];
};

struct adapter_write_request {
	struct adapter_req_info req_info;
	struct {
		__u32 lock_id;
		__u64 size;
		__u32 timeout;
		__u8 block;
	};
	char data_buf[0];
} __packed;

struct adapter_write_ack {
	__u32 status;
	__u64 size;
} __packed;

/* Close request only contains req_info, do not need close_request */
struct adapter_close_ack {
	__u32 status;
	struct {
		__u64 size;
		__u64 mtime;
		__u64 atime;
	};
} __packed;

struct adapter_block_meta_base {
	__u32 block_index;
	__u64 mtime;
	__u64 size;
};

/* Read_meta request only contains fileId, do not need read_meta_request */
struct adapter_read_meta_ack {
	__u32 status;
	struct {
		__u64 mtime;
		__u64 size;
	};
	__u32 block_count;
	char block_list[0];
} __packed;

/* Set_fsize response only contains status, do not need set_fsize_ack */
struct adapter_set_fsize_request {
	struct adapter_req_info req_info;
	__u32 lock_id;
	__u64 size;
} __packed;

/*
 * Sync request only contains req_info & sync response is the same as
 * close_ack, so we do not need sync_request/sync_ack
 */

struct adapter_sendmsg {
	void *data;
	size_t len;
	__u8 operations;
	int timeout;
	void *outmsg;
};

enum { HMDFS_MSG_LEN_ANY = 1, HMDFS_MSG_LEN_MIN, HMDFS_MSG_LEN_FIXED };
#endif
