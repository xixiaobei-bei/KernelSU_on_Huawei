/* SPDX-License-Identifier: GPL-2.0 */
/*
 * socket_adapter.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: transport adapt for hmdfs
 * Author: maojingjing1@huawei.com
 * Create: 2020-03-26
 *
 */

#ifndef SOCKET_ADAPTER_H
#define SOCKET_ADAPTER_H

#include <linux/net.h>
#include <linux/pagemap.h>

#include "connection.h"
#include "hmdfs.h"
#include "protocol.h"

#define HMDFS_KEY_SIZE	  32
#define HMDFS_IV_SIZE	  12
#define HMDFS_TAG_SIZE	  16
#define HMDFS_CID_SIZE	  64
#define INVALID_SOCKET_FD (-1)

#define HMDFS_IDR_RESCHED_COUNT 512

struct connection_operations {
	void (*recvmsg)(struct hmdfs_peer *con, void *head, void *buf);
	void (*recvpage)(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			 int err, void *data);
	const struct file_operations *remote_file_fops;
	const struct inode_operations *remote_file_iops;
	const struct address_space_operations *remote_file_aops;
	int (*remote_unlink)(struct hmdfs_peer *con, struct dentry *dentry);
	int (*remote_readdir)(struct hmdfs_peer *con, struct file *file,
			      struct dir_context *ctx);
	struct hmdfs_lookup_ret *(*remote_lookup)(struct hmdfs_peer *con,
						  const char *relative_path,
						  const char *d_name);
};

/*****************************************************************************
 * connections(TCP, UDP, .etc) adapter for RPC
 *****************************************************************************/

struct work_handler_desp {
	struct work_struct work;
	struct hmdfs_peer *peer;
	struct hmdfs_head_cmd *head;
	void *buf;
};

struct work_readfile_request_async {
	struct work_struct work;
	struct hmdfs_peer *con;
	struct hmdfs_send_command sm;
};

static inline void hmdfs_init_cmd(struct hmdfs_cmd *op, u8 cmd)
{
	op->reserved = 0;
	op->cmd_flag = C_REQUEST;
	op->command = cmd;
	op->reserved2 = 0;
}

int hmdfs_send_async_request(struct hmdfs_peer *peer,
			     const struct hmdfs_req *req);
int hmdfs_sendmessage_request(struct hmdfs_peer *con,
			      struct hmdfs_send_command *msg);
int hmdfs_sendpage_request(struct hmdfs_peer *con,
			   struct hmdfs_send_command *msg);

int hmdfs_sendmessage_response(struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *cmd, __u32 data_len,
			       void *buf, __u32 ret_code);
int hmdfs_readfile_response(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			    struct file *filp);
const struct connection_operations *hmdfs_get_peer_operation(__u8 version);

void hmdfs_recv_page_work_fn(struct work_struct *ptr);

/*****************************************************************************
 * statistics info for RPC
 *****************************************************************************/

enum hmdfs_resp_type {
	HMDFS_RESP_NORMAL,
	HMDFS_RESP_DELAY,
	HMDFS_RESP_TIMEOUT
};

struct server_statistic {
	unsigned long long cnt;		 /* request received */
	unsigned long long max;		 /* max processing time */
	unsigned long long total;	 /* total processing time */
	unsigned long long snd_cnt;      /* resp send to client */
	unsigned long long snd_fail_cnt; /* send resp to client failed cnt */
};

struct client_statistic {
	unsigned long long snd_cnt;	   /* request send to server */
	unsigned long long resp_cnt;	   /* response receive from server */
	unsigned long long timeout_cnt;    /* no respone from server */
	unsigned long long delay_resp_cnt; /* delay response from server */
	unsigned long long max;            /* max waiting time */
	unsigned long long total;	   /* total waiting time */
	unsigned long long snd_fail_cnt;   /* request send failed to server */
};


static inline void hmdfs_statistic(struct hmdfs_sb_info *sbi, u8 cmd,
				   unsigned long jiff)
{
	if (cmd >= F_SIZE)
		return;

	sbi->s_server_statis[cmd].cnt++;
	sbi->s_server_statis[cmd].total += jiff;
	if (jiff > sbi->s_server_statis[cmd].max)
		sbi->s_server_statis[cmd].max = jiff;
}

static inline void hmdfs_server_snd_statis(struct hmdfs_sb_info *sbi,
					   u8 cmd, int ret)
{
	if (cmd >= F_SIZE)
		return;
	ret ? sbi->s_server_statis[cmd].snd_fail_cnt++ :
	      sbi->s_server_statis[cmd].snd_cnt++;
}

static inline void hmdfs_client_snd_statis(struct hmdfs_sb_info *sbi,
					   u8 cmd, int ret)
{
	if (cmd >= F_SIZE)
		return;
	ret ? sbi->s_client_statis[cmd].snd_fail_cnt++ :
	      sbi->s_client_statis[cmd].snd_cnt++;
}

extern void hmdfs_client_resp_statis(struct hmdfs_sb_info *sbi, u8 cmd,
				     enum hmdfs_resp_type type,
				     unsigned long start, unsigned long end);

/*****************************************************************************
 * timeout configuration for RPC
 *****************************************************************************/

enum HMDFS_TIME_OUT {
	TIMEOUT_NONE = 0,
	TIMEOUT_COMMON = 4,
	TIMEOUT_6S = 6,
	TIMEOUT_30S = 30,
	TIMEOUT_1M = 60,
	TIMEOUT_90S = 90,
	TIMEOUT_CONFIG = UINT_MAX - 1, // for hmdfs_req to read from config
	TIMEOUT_UNINIT = UINT_MAX,
};

static inline int get_cmd_timeout(struct hmdfs_sb_info *sbi, enum FILE_CMD cmd)
{
	return sbi->s_cmd_timeout[cmd];
}

static inline void set_cmd_timeout(struct hmdfs_sb_info *sbi, enum FILE_CMD cmd,
				   unsigned int value)
{
	sbi->s_cmd_timeout[cmd] = value;
}

void hmdfs_response_wakeup(struct sendmsg_wait_queue *msg_info,
			   __u32 ret_code, __u32 data_len, void *buf);

void hmdfs_wakeup_parasite(struct hmdfs_msg_parasite *mp);

void hmdfs_wakeup_async_work(struct hmdfs_async_work *async_work);

void msg_put(struct sendmsg_wait_queue *msg_wq);
void head_put(struct hmdfs_msg_idr_head *head);
void mp_put(struct hmdfs_msg_parasite *mp);
void asw_put(struct hmdfs_async_work *asw);
static inline void asw_done(struct hmdfs_async_work *asw)
{
	if (asw->page)
		unlock_page(asw->page);
	asw_put(asw);
}

static inline void asw_get(struct hmdfs_async_work *asw)
{
	kref_get(&asw->head.ref);
}
#endif
