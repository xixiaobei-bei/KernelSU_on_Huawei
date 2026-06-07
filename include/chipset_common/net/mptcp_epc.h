/*
 * mptcp_epc.h
 *
 * EPC Feature depending on MPTCP
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MPTCP_EPC_H
#define _MPTCP_EPC_H

#include <linux/skbuff.h>

#include <net/mptcp.h>

#define MUTP_HEADER_LEN		8
#define MUTP_MSGTYPE_DATA	1
#define MUTP_VERSION		1

/* option teid for MP-GW */
#define MPTCP_SUB_TEID				11
#define MPTCP_SUB_LEN_TEID			7
#define MPTCP_SUB_LEN_TEID_ALIGN	8

#define SOCKS4_VERSON				0x04 /* Socks version 4 */
#define SOCKS4_CMD_CONNECT			0x01 /* CONNECT */
#define SOCKS4_REQUEST_GRANTED		0x5a /* Request granted */
#define SOCKS4_ECHO_LEN				8

#define DEV_IF_WIFI			0
#define DEV_IF_RMNET		1
#define DEV_IF_OTHER		2

#define RMNET_STR_LEN		5

struct mutp_header {
	uint8_t icheck:1,
		spare:4,
		version:3;
	uint8_t type;
	uint16_t len;
	uint32_t tunnel_id;
}  __packed;

/* Tunnel Endpoint Identifier(teid) for MP-GW */
struct mp_option_teid {
	__u8	kind;
	__u8	len;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	rsv:4,
		sub:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	sub:4,
		rsv:4;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif
	__u32 teid;
}  __packed;

/* SOCKS v4 client request message */
struct socks4_request {
	uint8_t ver;
	uint8_t cmd;
	uint16_t port;
	uint32_t addr;
	uint8_t user;
}  __packed;

void mptcp_hw_ext_set_hag_gw_info(struct mptcp_hw_ext *ext);
bool mptcp_hw_is_enable_hag_gw(struct sock *sk);
bool mptcp_hw_is_enable_hag_config(void);
void mptcp_teid_syn_options(const struct sock *sk, struct tcp_out_options *opts,
	unsigned int *remaining);
void mptcp_teid_options_write(__be32 *ptr, struct tcp_sock *tp,
			 const struct tcp_out_options *opts,
			 struct sk_buff *skb);
void mptcp_gw_rewrite_dst_addr(struct sock *sk, struct sockaddr *uaddr);


/* EPC MPTCP proxy functions */
int mptcp_hw_socks_recv(struct sock *sk, struct sk_buff *skb);
void mptcp_hw_socks4_send(struct sock *sk);
void mutp_fill_mutp_header(struct sock *sk, struct sk_buff *skb);
bool mutp_decode_recv(struct sk_buff *skb, bool is_udp4, int *off);
void mutp_rewrite_dst_addr(struct sock *sk, struct sockaddr *uaddr);
void mutp_rewrite_msg_addr(bool is_mutp, struct sk_buff *skb,
			struct sockaddr_in *sin);
void mutp_rewrite_msg_addrv6(bool is_mutp, struct sk_buff *skb,
			struct sockaddr_in6 *sin6);

#endif /* _MPTCP_EPC_H */
