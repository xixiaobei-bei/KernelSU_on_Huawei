/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transport.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg for hmdfs
 * Author: maojingjing1@huawei.com
 *         wangminmin4@huawei.com
 * Create: 2020-03-26
 *
 */

#ifndef HMDFS_TRANSPORT_H
#define HMDFS_TRANSPORT_H

#include "connection.h"

#define ADAPTER_MESSAGE_LENGTH (1024 * 1024 + 1024) // 1M + 1K
#define MAX_RECV_SIZE sizeof(struct hmdfs_head_cmd)

#define TCP_KVEC_HEAD 0
#define TCP_KVEC_DATA 1

enum TCP_KVEC_FILE_ELE_INDEX {
	TCP_KVEC_FILE_PARA = 1,
	TCP_KVEC_FILE_CONTENT = 2,
};

enum TCP_KVEC_TYPE {
	TCP_KVEC_ELE_SINGLE = 1,
	TCP_KVEC_ELE_DOUBLE = 2,
	TCP_KVEC_ELE_TRIPLE = 3,
};

#define TCP_RECV_TIMEOUT     2
#define MAX_RECV_RETRY_TIMES 2

#ifndef SO_RCVTIMEO
#define SO_RCVTIMEO SO_RCVTIMEO_OLD
#endif

struct tcp_handle {
	struct connection *connect;
	int recvbuf_maxsize;
	struct mutex close_mutex;
	/*
	 * To achieve atomicity.
	 *
	 * The sock lock held at the tcp layer may be temporally released at
	 * `sk_wait_event()` when waiting for sock buffer. From this point on,
	 * threads serialized at the initial call to `lock_sock()` contained
	 * in `tcp_sendmsg()` can proceed, resuling in intermixed messages.
	 */
	struct mutex send_mutex;
	struct socket *sock;
	int fd;
	struct kmem_cache *recv_cache;
	struct task_struct *recv_task;
};

void hmdfs_get_connection(struct hmdfs_peer *peer);
void hmdfs_reget_connection(struct connection *conn);
struct connection *hmdfs_get_conn_tcp(struct hmdfs_peer *node, int socket_fd,
				      uint8_t *master_key, uint8_t status);
void tcp_stop_connect(struct connection *connect);
uint32_t hmdfs_tcpi_rtt(struct hmdfs_peer *node);
void tcp_close_socket(struct tcp_handle *tcp);

#ifdef CONFIG_HMDFS_CRYPTO
int tcp_send_rekey_request(struct connection *connect);
#endif

#endif
