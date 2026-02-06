/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_server.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 *         chenyi77@huawei.com
 * Create: 2020-04-01
 *
 */

#ifndef HMDFS_ADAPTER_SERVER_H
#define HMDFS_ADAPTER_SERVER_H

#include "adapter_protocol.h"
#include "comm/transport.h"

void server_recv_handshake(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *data);
void server_recv_handshake_response(struct hmdfs_peer *con,
				    struct hmdfs_adapter_head *head,
				    void *data);
void server_recv_read(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		      void *data);
void server_recv_writepages(struct hmdfs_peer *con,
			    struct hmdfs_adapter_head *head, void *buf);
void server_recv_close(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		       void *buf);
void server_recv_delete(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
			void *buf);
void server_recv_read_meta(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *buf);
void server_recv_sync(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		      void *buf);
void server_recv_set_fsize(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *buf);
void server_request_recv(struct hmdfs_peer *con,
			 struct hmdfs_adapter_head *head, void *buf);

typedef void (*adapter_server_request_callback)(struct hmdfs_peer *,
						struct hmdfs_adapter_head *,
						void *);
static const adapter_server_request_callback
	adapter_s_recv_callbacks[META_PULL_END] = {
		[HANDSHAKE_REQUEST] = server_recv_handshake,
		[HANDSHAKE_RESPONSE] = server_recv_handshake_response,
		[READ_REQUEST] = server_recv_read,
		[WRITE_REQUEST] = server_recv_writepages,
		[CLOSE_REQUEST] = server_recv_close,
		[DELETE_REQUEST] = server_recv_delete,
		[READ_META_REQUEST] = server_recv_read_meta,
		[SYNC_REQUEST] = server_recv_sync,
		[SET_FSIZE_REQUEST] = server_recv_set_fsize,
	};

#endif
