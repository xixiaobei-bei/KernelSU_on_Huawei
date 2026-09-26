/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_socket.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 *         koujilong@huawei.com
 * Create: 2020-04-22
 *
 */

#include "adapter_client.h"
#include "adapter_server.h"

enum ADAPTER_CMD_FLAG { ADAPTER_C_REQUEST = 0, ADAPTER_C_RESPONSE = 1 };

void hmdfs_adapter_recv_mesg_callback(struct hmdfs_peer *con, void *head,
				      void *buf)
{
	struct hmdfs_adapter_head *adapter_head =
		(struct hmdfs_adapter_head *)head;

	if (adapter_head->request_id == ADAPTER_C_REQUEST)
		server_request_recv(con, adapter_head, buf);
	else
		client_response_recv(con, adapter_head, buf);
	kfree(buf);
}
