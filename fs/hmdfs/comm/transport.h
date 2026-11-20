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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
#include "d2dp/d2d.h"
#include <crypto/aead.h>
#endif

#define ADAPTER_MESSAGE_LENGTH (1024 * 1024 + 1024) // 1M + 1K
#define MAX_RECV_SIZE sizeof(struct hmdfs_head_cmd)


#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
#ifdef CONFIG_HMDFS_D2DP_TX_BINDING
#define D2D_BANDWIDTH_LOW_THRESHOLD  50   // Mbit/s
#define D2D_BANDWIDTH_HIGH_THRESHOLD 150  // Mbit/s
#define D2D_BANDWIDTH_INTERVAL_MS    1000

#define HISI_FAST_CORE_6 6
#define HISI_FAST_CORE_7 7
#endif
#endif

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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	/*
	 * UDP ports and D2DP version are negotiated over TCP session during
	 * HMDFS handshake. When negotiated data about remote side is sent to
	 * the userspace to connect UDP socket.
	 */
	uint16_t d2dp_local_udp_port;
	uint16_t d2dp_remote_udp_port;
	uint8_t d2dp_local_version;
	uint8_t d2dp_remote_version;
	uint8_t local_device_type;
	uint8_t remote_device_type;
#endif
};

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
struct d2dp_handle {
	struct connection   *connect;
	struct socket       *sock;
	struct d2d_protocol *proto;
	struct crypto_aead  *send_tfm;
	struct crypto_aead  *recv_tfm;
	struct d2d_security security;
	struct connection   *tcp_connect;
#ifdef CONFIG_HMDFS_D2DP_TX_BINDING
	struct mutex        bind_lock;
	atomic_long_t       last_time;
	uint64_t            last_bw;
	uint64_t            last_bytes;
#endif
};
#endif

struct connection_init_info {
	int           fd;
	struct socket *sock;
	uint8_t       *master_key;
	uint8_t       status;
	uint8_t       protocol;
	uint32_t      link_type;

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	uint16_t      d2dp_udp_port;
	uint8_t       d2dp_version;
	uint8_t       device_type;
	int32_t       binded_fd;
#endif
};

void hmdfs_handle_release(struct connection *conn);
void hmdfs_handle_shutdown(struct connection *conn);
struct socket *hmdfs_handle_get_socket(struct connection *conn);

void hmdfs_get_connection(struct hmdfs_peer *peer, int flag);
void hmdfs_reget_connection(struct connection *conn);
void hmdfs_delete_connection_by_sock(struct hmdfs_peer *peer,
				     struct socket *sock);
struct connection *hmdfs_get_conn(struct hmdfs_peer *node,
				  struct connection_init_info *info);
void hmdfs_stop_connect(struct connection *conn);
uint32_t hmdfs_get_connection_rtt(struct hmdfs_peer *node);

#ifdef CONFIG_HMDFS_CRYPTO
int hmdfs_send_rekey_request(struct connection *connect);
#endif

#endif
