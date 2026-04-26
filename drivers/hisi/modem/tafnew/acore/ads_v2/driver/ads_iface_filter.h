/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef __ADS_IFACE_FILTER__
#define __ADS_IFACE_FILTER__

#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/in6.h>
#include <linux/icmp.h>
#include <linux/byteorder/generic.h>

#include "ps_iface_global_def.h"
#include "ads_iface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define ADS_FILTER_SHARED_IFACE_MAX_NUM 2
#define ADS_FILTER_LIST_MAX_NUM 256
#define ADS_FILTER_DEFAULT_AGING_TIMELEN 30
#define ADS_FILTER_SECOND_TO_JIFFIES 1000

#define ADS_FILTER_CTX() (&ads_filter_ctx);

#define ADS_FILTER_ARRAY_SIZE(a) (sizeof((a)) / sizeof((a[0])))

#define ADS_FILTER_GET_ICMP_DECODE_FUNC_TBL_SIZE() \
	(ADS_FILTER_ARRAY_SIZE(ads_ipv4_icmp_decode_func_tbl))

#define ADS_FILTER_GET_ICMP_DECODE_FUNC_TBL_ADDR(type) \
	(&(ads_ipv4_icmp_decode_func_tbl[type]))

/* 过滤表索引号:
    TCP\UCP包, 源端口号低8位;
    ICMP包, Sequence低8位;
    分片包(非首片), IP Identification低8位. */
#define ADS_GET_FILTER_INDEX(filter_info) \
	((uint8_t)(ntohs((filter_info)->filter.filter_id[0]) & 0xff))

#define ADS_IS_FILTER_IPHDR_MATCH(ip_info1, ip_info2) \
	(((ip_info1)->src_addr == (ip_info2)->src_addr) && \
	((ip_info1)->dst_addr == (ip_info2)->dst_addr) && \
	((ip_info1)->protocol == (ip_info2)->protocol))

#define ADS_IS_FILTER_TCP_INFO_MATCH(tcp_info1, tcp_info2) \
	(((tcp_info1)->src_port == (tcp_info2)->src_port) && \
	((tcp_info1)->dst_port == (tcp_info2)->dst_port))

#define ADS_IS_FILTER_UDP_INFO_MATCH(udp_info1, udp_info2) \
	(((udp_info1)->src_port == (udp_info2)->src_port) && \
	((udp_info1)->dst_port == (udp_info2)->dst_port))

#define ADS_IS_FILTER_ICMP_INFO_MATCH(icmp_info1, icmp_info2) \
	(((icmp_info1)->sequence == (icmp_info2)->sequence) && \
	((icmp_info1)->id == (icmp_info2)->id))

#define ADS_IS_FILTER_FRAG_INFO_MATCH(frag_info1, frag_info2) \
	((frag_info1)->ip_id == (frag_info2)->ip_id)

#define ADS_FILTER_IS_IPV6_ADDR_MATCH(ipv6_addr1, ipv6_addr2) \
      ((((ipv6_addr1)->in6_u.u6_addr32[0] ^ (ipv6_addr2)->in6_u.u6_addr32[0]) | \
	((ipv6_addr1)->in6_u.u6_addr32[1] ^ (ipv6_addr2)->in6_u.u6_addr32[1]) | \
	((ipv6_addr1)->in6_u.u6_addr32[2] ^ (ipv6_addr2)->in6_u.u6_addr32[2]) | \
	((ipv6_addr1)->in6_u.u6_addr32[3] ^ (ipv6_addr2)->in6_u.u6_addr32[3])) == 0)

enum ads_filter_ip_pkt_type
{
	ADS_FILTER_PKT_TYPE_TCP,
	ADS_FILTER_PKT_TYPE_UDP,
	ADS_FILTER_PKT_TYPE_ICMP,
	ADS_FILTER_PKT_TYPE_FRAGMENT, /* 数据包类型为分片包(非首片) */
	ADS_FILTER_PKT_TYPE_BUTT
};

enum ads_filter_orig_ip_pkt_type
{
	ADS_FILTER_ORIG_UL_IPV4_TCP,
	ADS_FILTER_ORIG_UL_IPV4_UDP,
	ADS_FILTER_ORIG_UL_IPV4_ECHOREQ,
	ADS_FILTER_ORIG_UL_IPV4_NON_FIRST_FRAG,
	ADS_FILTER_ORIG_UL_IPV4_NOT_SUPPORT,
	ADS_FILTER_ORIG_UL_IPV6_PKT,

	ADS_FILTER_ORIG_DL_IPV4_TCP,
	ADS_FILTER_ORIG_DL_IPV4_UDP,
	ADS_FILTER_ORIG_DL_IPV4_ECHOREPLY,
	ADS_FILTER_ORIG_DL_IPV4_ICMPERROR,
	ADS_FILTER_ORIG_DL_IPV4_FIRST_FRAG,
	ADS_FILTER_ORIG_DL_IPV4_NON_FIRST_FRAG,
	ADS_FILTER_ORIG_DL_IPV6_PKT,

	ADS_FILTER_ORIG_BUTT
};

struct ads_filter_ctx_s {
	struct list_head ipv4_list[ADS_FILTER_LIST_MAX_NUM];
	struct in6_addr ipv6_saddr[PS_IFACE_ID_BUTT];
	uint64_t aging_time;
};

struct ads_ipv4_head_info_s {
	uint32_t src_addr;
	uint32_t dst_addr;
	uint8_t protocol;
	uint8_t rsv[3];
};

struct ads_ipv4_tcp_info_s {
	uint16_t src_port;
	uint16_t dst_port;
};

struct ads_ipv4_udp_info_s {
	uint16_t src_port;
	uint16_t dst_port;
};

struct ads_ipv4_icmp_info_s {
	uint16_t sequence;	/* Icmp Sequence number */
	uint16_t id;		/* Icmp Identifier */
};

struct ads_ipv4_fragment_info_s {
	uint16_t ip_id;		/* IP identification */
	uint8_t rsv[2];
};

struct ads_ipv4_filter_info_s {
	uint8_t pkt_type;
	uint8_t rsv[3];
	uint64_t time_cnt;
	struct ads_ipv4_head_info_s ip_info;
	union {
		struct ads_ipv4_tcp_info_s tcp_info;
		struct ads_ipv4_udp_info_s udp_info;
		struct ads_ipv4_icmp_info_s icmp_info;
		struct ads_ipv4_fragment_info_s frag_info;
		uint16_t filter_id[2];
	} filter;
};

struct ads_ipv4_filter_node_s {
	uint8_t iface_id;
	uint8_t rsv[3];
	struct ads_ipv4_filter_info_s filter_info;
	struct list_head list;
};

typedef int (*ads_ipv4_icmp_decode_func_t) (const struct icmphdr *icmph,
	struct ads_ipv4_filter_info_s *filter_info);

struct ads_ipv4_icmp_decode_handler_s {
	ads_ipv4_icmp_decode_func_t func;
	uint8_t pkt_type;
	uint8_t rsv[7];
};

void ads_iface_filter_ctx_init(void);
void ads_iface_filter_ctx_reset(void);
void ads_iface_ul_data_filter_info_save(uint8_t iface_id,
	const struct sk_buff *skb);

uint8_t ads_iface_filter_dl_data(struct ads_iface_priv_s *priv,
	const struct sk_buff *skb);

struct ads_iface_priv_s *ads_iface_shared_iface_select(
	struct ads_iface_priv_s *priv, struct sk_buff *skb);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_IFACE_FILTER__ */

