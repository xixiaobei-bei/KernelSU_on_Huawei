/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_server.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: maojingjing@huawei.com
 * Create: 2020-04-09
 *
 */

#ifndef HMDFS_SERVER_H
#define HMDFS_SERVER_H

#include "hmdfs.h"
#include "comm/transport.h"
#include "comm/socket_adapter.h"

static inline void hmdfs_send_err_response(struct hmdfs_peer *con,
					   struct hmdfs_head_cmd *cmd, int err)
{
	if (hmdfs_sendmessage_response(con, cmd, 0, NULL, (__u32)err))
		hmdfs_warning("send err failed");
}

void hmdfs_server_open(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
		       void *data);
void hmdfs_server_atomic_open(struct hmdfs_peer *con,
			      struct hmdfs_head_cmd *cmd, void *data);
void hmdfs_server_fsync(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data);
void hmdfs_server_release(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data);
void hmdfs_server_readpage(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   void *data);
void hmdfs_server_readpages(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    void *data);
void hmdfs_server_readpages_open(struct hmdfs_peer *con,
				 struct hmdfs_head_cmd *cmd, void *data);
void hmdfs_server_writepage(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    void *data);

void hmdfs_server_readdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data);

void hmdfs_server_mkdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data);

void hmdfs_server_create(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data);

void hmdfs_server_rmdir(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			void *data);

void hmdfs_server_unlink(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data);

void hmdfs_server_rename(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data);

void hmdfs_server_setattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data);
void hmdfs_server_getattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data);
void hmdfs_server_statfs(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data);
void hmdfs_server_syncfs(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			 void *data);
void hmdfs_server_getxattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   void *data);
void hmdfs_server_setxattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   void *data);
void hmdfs_server_listxattr(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    void *data);
void hmdfs_server_get_drop_push(struct hmdfs_peer *con,
				struct hmdfs_head_cmd *cmd, void *data);

void __init hmdfs_server_add_node_evt_cb(void);
int is_hidden_dir(const char *dir);
#endif
