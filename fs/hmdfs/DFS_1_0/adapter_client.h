/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_client.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 *         chenyi77@huawei.com
 * Create: 2020-04-17
 *
 */

#ifndef HMDFS_ADAPTER_CLIENT_H
#define HMDFS_ADAPTER_CLIENT_H

#include "adapter_protocol.h"
#include "comm/socket_adapter.h"

void client_recv_read(struct sendmsg_wait_queue *msg_info, void *buf,
		      size_t len);
void client_recv_writepage(struct sendmsg_wait_queue *msg_info, void *buf,
			   size_t len);
void client_recv_close(struct sendmsg_wait_queue *msg_info, void *buf,
		       size_t len);
void client_recv_delete(struct sendmsg_wait_queue *msg_info, void *buf,
			size_t len);
void client_recv_setfssize(struct sendmsg_wait_queue *msg_info, void *buf,
			   size_t len);
int client_sendpage_request(struct hmdfs_peer *con, struct adapter_sendmsg *sm);
int client_sendmessage_request(struct hmdfs_peer *con,
			       struct adapter_sendmsg *sm);
void client_response_recv(struct hmdfs_peer *con,
			  struct hmdfs_adapter_head *head, void *buf);

typedef void (*adapter_client_response_callback)(struct sendmsg_wait_queue *,
						 void *, size_t);
static const adapter_client_response_callback recv_callbacks[META_PULL_END] = {
	[READ_RESPONSE] = client_recv_read,
	[WRITE_RESPONSE] = client_recv_writepage,
	[CLOSE_RESPONSE] = client_recv_close,
	[DELETE_RESPONSE] = client_recv_delete,
	[SET_FSIZE_RESPONSE] = client_recv_setfssize,
};

#endif
