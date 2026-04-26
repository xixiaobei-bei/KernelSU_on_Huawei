/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#include "ads_iface_filter.h"
#include "ads_iface_debug.h"
#include "ps_iface_global_def.h"

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include "securec.h"


#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
struct ads_filter_ctx_s ads_filter_ctx;

int ads_iface_dl_ipv4_icmp_echo_reply_decode(const struct icmphdr *icmph,
	struct ads_ipv4_filter_info_s *filter_info);

int ads_iface_dl_ipv4_icmp_err_pkt_decode(const struct icmphdr *icmph,
	struct ads_ipv4_filter_info_s *filter_info);

const struct ads_ipv4_icmp_decode_handler_s ads_ipv4_icmp_decode_func_tbl[] = {
	{ads_iface_dl_ipv4_icmp_echo_reply_decode,	ADS_FILTER_ORIG_DL_IPV4_ECHOREPLY}, /*  0:Echo Reply              */
	{NULL,						ADS_FILTER_ORIG_BUTT},              /*  1:Reserve                 */
	{NULL,						ADS_FILTER_ORIG_BUTT},              /*  2:Reserve                 */
	{ads_iface_dl_ipv4_icmp_err_pkt_decode,		ADS_FILTER_ORIG_DL_IPV4_ICMPERROR}, /*  3:Destination Unreachable */
	{ads_iface_dl_ipv4_icmp_err_pkt_decode,		ADS_FILTER_ORIG_DL_IPV4_ICMPERROR}, /*  4:Source Quench           */
	{ads_iface_dl_ipv4_icmp_err_pkt_decode,		ADS_FILTER_ORIG_DL_IPV4_ICMPERROR}, /*  5:Redirect (change route) */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /*  6:Reserve                 */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /*  7:Reserve                 */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /*  8:Echo Request            */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /*  9:Reserve                 */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /* 10:Reserve                 */
	{ads_iface_dl_ipv4_icmp_err_pkt_decode,		ADS_FILTER_ORIG_DL_IPV4_ICMPERROR}, /* 11:Time Exceeded           */
	{ads_iface_dl_ipv4_icmp_err_pkt_decode,		ADS_FILTER_ORIG_DL_IPV4_ICMPERROR}, /* 12:Parameter Problem       */
	{NULL,						ADS_FILTER_ORIG_BUTT, 		},  /* 13:Timestamp Request       */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /* 14:Timestamp Reply         */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /* 15:Information Request     */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /* 16:Information Reply       */
	{NULL,						ADS_FILTER_ORIG_BUTT,		},  /* 17:Address Mask Request    */
	{NULL,						ADS_FILTER_ORIG_BUTT,		}   /* 18:Address Mask Reply      */
};

/*
 *                MF     Offset
 * Non-frag       0       0
 * First-frag     1       0
 * Middle-frag    1       >0
 * Last-frag      0       >0
 *
 * ads_iface_is_first_ipv4_fragment - Is first fragment?
 * @iph: ip header
 *
 * Return true: First fragment; false: Non first fragment.
 */
STATIC bool ads_iface_is_first_ipv4_fragment(const struct iphdr *iph)
{
	return ((iph->frag_off & htons(IP_OFFSET)) == 0 && (iph->frag_off & htons(IP_MF)) != 0);
}

/*
 * ads_iface_is_ipv4_first_fragment_or_normal_pkt
 * @iph: ip header
 *
 * Return true or false.
 */
STATIC bool ads_iface_is_ipv4_first_fragment_or_normal_pkt(const struct iphdr *iph)
{
	return (iph->frag_off & htons(IP_OFFSET)) == 0;
}

/*
 * ads_iface_is_filter_aged - Is filter info aged ?.
 * @curr_time: current time;
 *
 * Return true: aged; false: Not aged.
 */
STATIC bool ads_iface_is_filter_aged(uint64_t time_cnt)
{
	struct ads_filter_ctx_s *filter_ctx = ADS_FILTER_CTX();

	if (filter_ctx->aging_time != 0)
		return time_after_eq(jiffies, (unsigned long)(filter_ctx->aging_time + time_cnt));

	return false;
}

/*
 * ads_iface_ul_ipv4_data_decode - decode ipv4 data.
 * @skb: sk buffer
 * @filter_info: filter info to save.
 *
 * Return 0: decode success; -1: failed.
 */
int ads_iface_ul_ipv4_data_decode(const struct sk_buff *skb,
	struct ads_ipv4_filter_info_s *filter_info)
{
	struct iphdr   *iph   = NULL;
	struct tcphdr  *tcph  = NULL;
	struct udphdr  *udph  = NULL;
	struct icmphdr *icmph = NULL;
	int ip_len = 0;

	iph = (struct iphdr *)skb->data;

	if (!ads_iface_is_ipv4_first_fragment_or_normal_pkt(iph))
		return -1;

	filter_info->time_cnt = jiffies;
	filter_info->ip_info.src_addr = iph->saddr;
	filter_info->ip_info.dst_addr = iph->daddr;
	filter_info->ip_info.protocol = iph->protocol;

	ip_len = iph->ihl * 4;

	switch (iph->protocol) {
	case IPPROTO_ICMP:
		icmph = (struct icmphdr *)((unsigned char *)iph + ip_len);
		if (icmph->type != ICMP_ECHO)
			return -1;

		filter_info->pkt_type = ADS_FILTER_PKT_TYPE_ICMP;
		filter_info->filter.icmp_info.id = icmph->un.echo.id;
		filter_info->filter.icmp_info.sequence = icmph->un.echo.sequence;
		return 0;
	case IPPROTO_TCP:
		tcph = (struct tcphdr *)((unsigned char *)iph + ip_len);
		filter_info->pkt_type = ADS_FILTER_PKT_TYPE_TCP;
		filter_info->filter.tcp_info.src_port = tcph->source;
		filter_info->filter.tcp_info.dst_port = tcph->dest;
		return 0;
	case IPPROTO_UDP:
		udph = (struct udphdr *)((unsigned char *)iph + ip_len);
		filter_info->pkt_type = ADS_FILTER_PKT_TYPE_UDP;
		filter_info->filter.udp_info.src_port = udph->source;
		filter_info->filter.udp_info.dst_port = udph->dest;
		return 0;
	default:
		break;
	}

	return -1;
}

/*
 * ads_iface_ul_ipv6_filter_info_save - save iface ipv6 saddr.
 * @iface_id: iface id.
 * @skb: sk buffer
 *
 */
void ads_iface_ul_ipv6_filter_info_save(uint8_t iface_id, const struct sk_buff *skb)
{
	struct ads_filter_ctx_s *filter_ctx = NULL;
	struct ipv6hdr  *ipv6hdr = NULL;
	struct in6_addr *ipv6_saddr = NULL;
	int ret;

	filter_ctx = ADS_FILTER_CTX();
	ipv6hdr = (struct ipv6hdr *)skb->data;
	ipv6_saddr = &filter_ctx->ipv6_saddr[iface_id];
	ret = memcpy_s(ipv6_saddr, sizeof(struct in6_addr),
		&ipv6hdr->saddr, sizeof(struct in6_addr));
	ADS_MEMCPY_RET_CHECK(ret, sizeof(struct in6_addr), sizeof(struct in6_addr));
}

/*
 * ads_iface_match_ipv4_filter_info - match ipv4 filter info.
 * @pkt_filter:  filter info parsed from paket.
 * @list_filter: filter info searched from list.
 *
 * Return true: Matched; false: Not matched.
 */
bool ads_iface_match_ipv4_filter_info(struct ads_ipv4_filter_info_s *pkt_filter,
	struct ads_ipv4_filter_info_s *list_filter)
{
	if (!ADS_IS_FILTER_IPHDR_MATCH(&pkt_filter->ip_info, &list_filter->ip_info))
		return false;

	switch (pkt_filter->pkt_type) {
	case ADS_FILTER_PKT_TYPE_TCP:
		if (ADS_IS_FILTER_TCP_INFO_MATCH(&pkt_filter->filter.tcp_info,
		    &list_filter->filter.tcp_info))
			return true;
		break;
	case ADS_FILTER_PKT_TYPE_UDP:
		if (ADS_IS_FILTER_UDP_INFO_MATCH(&pkt_filter->filter.udp_info,
		    &list_filter->filter.udp_info))
			return true;
		break;
	case ADS_FILTER_PKT_TYPE_ICMP:
		if (ADS_IS_FILTER_ICMP_INFO_MATCH(&pkt_filter->filter.icmp_info,
		    &list_filter->filter.icmp_info))
			return true;
		break;
	case ADS_FILTER_PKT_TYPE_FRAGMENT:
		if (ADS_IS_FILTER_FRAG_INFO_MATCH(&pkt_filter->filter.frag_info,
		    &list_filter->filter.frag_info))
			return true;
		break;
	default:
		break;
	}

	return false;
}

/*
 * ads_iface_ipv4_filter_match - match ipv4 filter info.
 * @filter_info: filter info parsed from paket.
 * @iface_id: iface matched by the filter info.
 *
 * Return true: Matched; false: Not matched.
 */
bool ads_iface_ipv4_filter_match(struct ads_ipv4_filter_info_s *filter_info, uint8_t *iface_id)
{
	struct ads_ipv4_filter_node_s *filter_node = NULL;
	struct ads_filter_ctx_s       *filter_ctx  = NULL;
	struct list_head *filter_list = NULL;
	struct list_head *pos         = NULL;
	struct list_head *temp        = NULL;
	uint8_t filter_idx;
	bool ret = false;

	*iface_id = PS_IFACE_ID_BUTT;
	filter_ctx = ADS_FILTER_CTX();
	filter_idx = ADS_GET_FILTER_INDEX(filter_info);
	filter_list = &filter_ctx->ipv4_list[filter_idx];

	list_for_each_safe(pos, temp, filter_list) {
		filter_node = list_entry(pos, struct ads_ipv4_filter_node_s, list);

		if (ads_iface_is_filter_aged(filter_node->filter_info.time_cnt)) {
			list_del(&filter_node->list);
			kfree(filter_node);
			filter_node = NULL;
			continue;
		}

		if ((filter_info->pkt_type != filter_node->filter_info.pkt_type) ||(ret == true))
			continue;

		ret = ads_iface_match_ipv4_filter_info(filter_info, &filter_node->filter_info);
		if (ret == true) {
			filter_node->filter_info.time_cnt = jiffies;
			*iface_id = filter_node->iface_id;
			if (filter_node->filter_info.pkt_type == ADS_FILTER_PKT_TYPE_ICMP) {
				list_del(&filter_node->list);
				kfree(filter_node);
				filter_node = NULL;
			}
		}
	}

	return ret;
}

/*
 * ads_iface_add_ipv4_filter - Add ipv4 filter info.
 * @iface_id: iface id associated with the filter info.
 * @filter_info: filter info parsed from ipv4 packet.
 *
 */
void ads_iface_add_ipv4_filter(uint8_t iface_id, struct ads_ipv4_filter_info_s *filter_info)
{
	struct ads_ipv4_filter_node_s *filter_node = NULL;
	struct ads_filter_ctx_s       *filter_ctx  = NULL;
	struct list_head              *filter_list = NULL;
	uint8_t filter_idx;
	int ret;

	filter_node = (struct ads_ipv4_filter_node_s *)kmalloc(sizeof(struct ads_ipv4_filter_node_s), GFP_ATOMIC);
	if (filter_node == NULL) {
		return;
	}

	(void)memset_s(filter_node, sizeof(struct ads_ipv4_filter_node_s),
		0, sizeof(struct ads_ipv4_filter_node_s));

	filter_node->iface_id = iface_id;
	ret = memcpy_s(&filter_node->filter_info, sizeof(struct ads_ipv4_filter_info_s),
		filter_info, sizeof(struct ads_ipv4_filter_info_s));
	ADS_MEMCPY_RET_CHECK(ret, sizeof(struct ads_ipv4_filter_info_s),
		sizeof(struct ads_ipv4_filter_info_s));

	filter_ctx = ADS_FILTER_CTX();
	filter_idx = ADS_GET_FILTER_INDEX(filter_info);
	filter_list = &filter_ctx->ipv4_list[filter_idx];

	list_add_tail(&filter_node->list, filter_list);
}

/*
 * ads_iface_dl_ipv4_ip_info_decode - decode ip infos from ipv4 packet.
 * @iph: ip header.
 * @filter_info: filter to save the ip infos.
 */
void ads_iface_dl_ipv4_ip_info_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	filter_info->ip_info.src_addr = iph->daddr;
	filter_info->ip_info.dst_addr = iph->saddr;
	filter_info->ip_info.protocol = iph->protocol;
}

/*
 * ads_iface_dl_ipv4_icmp_echo_reply_decode - decode icmp echo reply packet.
 * @icmph: icmp header.
 * @filter_info: filter to save the icmp infos.
 */
int ads_iface_dl_ipv4_icmp_echo_reply_decode(const struct icmphdr *icmph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	filter_info->pkt_type = ADS_FILTER_PKT_TYPE_ICMP;
	filter_info->time_cnt = jiffies;
	filter_info->filter.icmp_info.sequence = icmph->un.echo.sequence;
	filter_info->filter.icmp_info.id = icmph->un.echo.id;
	return 0;
}

/*
 * ads_iface_dl_ipv4_icmp_err_pkt_decode - decode icmp err packet.
 * @icmph: icmp header.
 * @filter_info: filter to save the packet infos.
 */
int ads_iface_dl_ipv4_icmp_err_pkt_decode(const struct icmphdr *icmph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;
	struct iphdr  *icmp_iph = NULL;
	int ret = 0;

	/* 获取icmp报文中所带的原始数据包的IP首部 */
	icmp_iph = (struct iphdr *)((char *)icmph + sizeof(struct icmphdr));

	filter_info->time_cnt = jiffies;
	filter_info->ip_info.src_addr = icmp_iph->saddr;
	filter_info->ip_info.dst_addr = icmp_iph->daddr;
	filter_info->ip_info.protocol = icmp_iph->protocol;

	switch (icmp_iph->protocol) {
	case IPPROTO_TCP:
		filter_info->pkt_type = ADS_FILTER_PKT_TYPE_TCP;
		tcph = (struct tcphdr *)((char *)icmp_iph + icmp_iph->ihl * 4);
		filter_info->filter.tcp_info.src_port = tcph->source;
		filter_info->filter.tcp_info.dst_port = tcph->dest;
		break;
	case IPPROTO_UDP:
		filter_info->pkt_type = ADS_FILTER_PKT_TYPE_UDP;
		udph = (struct udphdr *)((char *)icmp_iph + icmp_iph->ihl * 4);
		filter_info->filter.udp_info.src_port = udph->source;
		filter_info->filter.udp_info.dst_port = udph->dest;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/*
 * ads_iface_dl_ipv4_icmp_decode - decode icmpv4 packet.
 * @iph: ip header.
 * @filter_info: filter to save the icmp infos.
 * @pkt_type
 */
int ads_iface_dl_ipv4_icmp_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info, uint8_t *pkt_type)
{
	const struct ads_ipv4_icmp_decode_handler_s *icmp_handler = NULL;
	struct icmphdr *icmph = NULL;
	ads_ipv4_icmp_decode_func_t icmp_func = NULL;

	icmph = (struct icmphdr *)((char *)iph + iph->ihl * 4);

	if (icmph->type < ADS_FILTER_GET_ICMP_DECODE_FUNC_TBL_SIZE()) {
		icmp_handler = ADS_FILTER_GET_ICMP_DECODE_FUNC_TBL_ADDR(icmph->type);
		icmp_func = icmp_handler->func;
		if (icmp_func != NULL) {
			*pkt_type = icmp_handler->pkt_type;
			return icmp_func(icmph, filter_info);
		}
	}
	return -1;
}

/*
 * ads_iface_dl_ipv4_tcp_decode - decode ipv4 tcp packet.
 * @iph: ip header.
 * @filter_info: filter to save the tcp infos.
 */
int ads_iface_dl_ipv4_tcp_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	struct tcphdr *tcph = (struct tcphdr *)((char *)iph + iph->ihl * 4);

	filter_info->pkt_type = ADS_FILTER_PKT_TYPE_TCP;
	filter_info->time_cnt = jiffies;
	filter_info->filter.tcp_info.src_port = tcph->dest;
	filter_info->filter.tcp_info.dst_port = tcph->source;
	return 0;
}

/*
 * ads_iface_dl_ipv4_udp_decode - decode ipv4 udp packet.
 * @iph: ip header.
 * @filter_info: filter to save the udp infos.
 */
int ads_iface_dl_ipv4_udp_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	struct udphdr *udph = (struct udphdr *)((char *)iph + iph->ihl * 4);

	filter_info->pkt_type = ADS_FILTER_PKT_TYPE_UDP;
	filter_info->time_cnt = jiffies;
	filter_info->filter.udp_info.src_port = udph->dest;
	filter_info->filter.udp_info.dst_port = udph->source;
	return 0;
}

/*
 * ads_iface_dl_ipv4_non_first_frag_decode - decode non first fragment.
 * @iph: ip header.
 * @filter_info: filter to save the frag infos.
 */
void ads_iface_dl_ipv4_non_first_frag_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info)
{
	ads_iface_dl_ipv4_ip_info_decode(iph, filter_info);
	filter_info->pkt_type = ADS_FILTER_PKT_TYPE_FRAGMENT;
	filter_info->time_cnt = jiffies;
	filter_info->filter.frag_info.ip_id = iph->id;
}

/*
 * ads_iface_dl_ipv4_frag_decode - decode ipv4 fragment.
 * @iph: ip header.
 * @filter_info: filter info parsed from ipv4 fragment.
 * @pkt_type: is the first frag or not.
 */
int ads_iface_dl_ipv4_frag_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info, uint8_t *pkt_type)
{
	int ret = 0;

	if (ads_iface_is_first_ipv4_fragment(iph)) {
		switch (iph->protocol) {
		case IPPROTO_TCP:
			*pkt_type = ADS_FILTER_ORIG_DL_IPV4_FIRST_FRAG;
			ads_iface_dl_ipv4_tcp_decode(iph, filter_info);
			break;
		case IPPROTO_UDP:
			*pkt_type = ADS_FILTER_ORIG_DL_IPV4_FIRST_FRAG;
			ads_iface_dl_ipv4_udp_decode(iph, filter_info);
			break;
		default:
			ret = -1;
			break;
		}
	} else {
		*pkt_type = ADS_FILTER_ORIG_DL_IPV4_NON_FIRST_FRAG;
		ads_iface_dl_ipv4_non_first_frag_decode(iph, filter_info);
	}

	return ret;
}

/*
 * ads_iface_dl_ipv4_decode - decode ipv4 packet.
 * @iph: ip header.
 * @filter_info: filter to save the ipv4 infos.
 * @pkt_type
 */
int ads_iface_dl_ipv4_decode(const struct iphdr *iph,
	struct ads_ipv4_filter_info_s *filter_info, uint8_t *pkt_type)
{
	ads_iface_dl_ipv4_ip_info_decode(iph, filter_info);

	if (ip_is_fragment(iph)) {
		return ads_iface_dl_ipv4_frag_decode(iph, filter_info, pkt_type);
	}

	switch (iph->protocol) {
	case IPPROTO_ICMP:
		return ads_iface_dl_ipv4_icmp_decode(iph, filter_info, pkt_type);
	case IPPROTO_TCP:
		*pkt_type = ADS_FILTER_ORIG_DL_IPV4_TCP;
		return ads_iface_dl_ipv4_tcp_decode(iph, filter_info);
	case IPPROTO_UDP:
		*pkt_type = ADS_FILTER_ORIG_DL_IPV4_UDP;
		return ads_iface_dl_ipv4_udp_decode(iph, filter_info);
	default:
		return -1;
	}
}

/*
 * ads_iface_filter_dl_ipv4_pkt - filter ipv4 packet.
 * @skb: sk buffer
 *
 */
uint8_t ads_iface_filter_dl_ipv4_pkt(const struct sk_buff *skb)
{
	const struct iphdr *iph = NULL;
	struct ads_ipv4_filter_info_s filter_info;
	uint8_t iface_id = PS_IFACE_ID_BUTT;
	uint8_t pkt_type;

	iph = (struct iphdr *)skb->data;
	(void)memset_s(&filter_info, sizeof(filter_info), 0, sizeof(filter_info));

	if (ads_iface_dl_ipv4_decode(iph, &filter_info, &pkt_type) != 0)
		return PS_IFACE_ID_BUTT;

	if (ads_iface_ipv4_filter_match(&filter_info, &iface_id) == true) {
		if (pkt_type == ADS_FILTER_ORIG_DL_IPV4_FIRST_FRAG) {
			(void)memset_s(&filter_info, sizeof(filter_info), 0, sizeof(filter_info));
			ads_iface_dl_ipv4_non_first_frag_decode(iph, &filter_info);
			ads_iface_add_ipv4_filter(iface_id, &filter_info);
		}
	}

	return iface_id;
}

/*
 * ads_iface_filter_dl_ipv6_pkt - filter ipv6 packet.
 * @skb: sk buffer
 *
 */
uint8_t ads_iface_filter_dl_ipv6_pkt(struct ads_iface_priv_s *priv,
	const struct sk_buff *skb)
{
	struct ads_filter_ctx_s *filter_ctx = NULL;
	struct ipv6hdr  *ipv6_hdr = NULL;
	struct in6_addr *ipv6_saddr = NULL;

	filter_ctx = ADS_FILTER_CTX();
	ipv6_hdr = (struct ipv6hdr *)skb->data;
	ipv6_saddr = &filter_ctx->ipv6_saddr[priv->iface_id];

	if (ADS_FILTER_IS_IPV6_ADDR_MATCH(ipv6_saddr, &ipv6_hdr->daddr))
		return priv->iface_id;
	else
		return priv->shared_iface_id;
}

/*
 * ads_iface_filter_lists_init.
 * @filter_ctx: filter_ctx
 *
 */
void ads_iface_filter_lists_init(struct ads_filter_ctx_s *filter_ctx)
{
	struct list_head *head = NULL;
	uint32_t idx;

	for (idx = 0; idx < ADS_FILTER_LIST_MAX_NUM; idx++) {
		head = &filter_ctx->ipv4_list[idx];
		INIT_LIST_HEAD(head);
	}
}

/*
 * ads_iface_filter_list_reset.
 * @filter_ctx: filter_ctx
 *
 */
void ads_iface_filter_list_reset(struct ads_filter_ctx_s *filter_ctx)
{
	struct list_head *head = NULL;
	struct list_head *pos  = NULL;
	struct list_head *temp = NULL;
	struct ads_ipv4_filter_node_s *filter_node = NULL;
	uint32_t idx;

	for (idx = 0; idx < ADS_FILTER_LIST_MAX_NUM; idx++) {
		head = &filter_ctx->ipv4_list[idx];
		list_for_each_safe(pos, temp, head) {
			filter_node = list_entry(pos, struct ads_ipv4_filter_node_s, list);
			list_del(&filter_node->list);
			kfree(filter_node);
		}
	}
}

/*
 * ads_filter_ctx_init.
 *
 */
 void ads_iface_filter_ctx_init(void)
{
	struct ads_filter_ctx_s *filter_ctx = ADS_FILTER_CTX();

	(void)memset_s(filter_ctx, sizeof(struct ads_filter_ctx_s),
		0, sizeof(struct ads_filter_ctx_s));

	filter_ctx->aging_time = ADS_FILTER_DEFAULT_AGING_TIMELEN * ADS_FILTER_SECOND_TO_JIFFIES;
	ads_iface_filter_lists_init(filter_ctx);
}

/*
 * ads_filter_ctx_reset.
 *
 */
 void ads_iface_filter_ctx_reset(void)
{
	struct ads_filter_ctx_s *filter_ctx = ADS_FILTER_CTX();

	ads_iface_filter_list_reset(filter_ctx);

	(void)memset_s(filter_ctx->ipv6_saddr, sizeof(filter_ctx->ipv6_saddr), 0,
		sizeof(filter_ctx->ipv6_saddr));
}

/*
 * ads_iface_ul_data_filter_info_save - save filter info for uplink data.
 * iface_id: iface id.
 * @skb: sk buffer
 */
void ads_iface_ul_data_filter_info_save(uint8_t iface_id,
	const struct sk_buff *skb)
{
	struct iphdr *iphdr = NULL;
	struct ads_ipv4_filter_info_s filter_info;
	uint8_t filter_iface_id;

	iphdr = (struct iphdr *)skb->data;
	(void)memset_s(&filter_info, sizeof(filter_info), 0, sizeof(filter_info));

	switch (iphdr->version) {
	case 4:
		if (ads_iface_ul_ipv4_data_decode(skb, &filter_info) == 0) {
			if (ads_iface_ipv4_filter_match(&filter_info, &filter_iface_id) != true)
				ads_iface_add_ipv4_filter(iface_id, &filter_info);
		}
		break;
	case 6:
		ads_iface_ul_ipv6_filter_info_save(iface_id, skb);
		break;
	default:
		break;
	}
}

/*
 * ads_iface_filter_dl_data - filter dl packets.
 * @priv: iface private info.
 * @skb: sk buffer
 */
uint8_t ads_iface_filter_dl_data(struct ads_iface_priv_s *priv,
	const struct sk_buff *skb)
{
	struct iphdr *iphdr = NULL;
	uint8_t iface_id = PS_IFACE_ID_BUTT;

	iphdr = (struct iphdr *)skb->data;
	switch (iphdr->version) {
	case 4:
		iface_id = ads_iface_filter_dl_ipv4_pkt(skb);
		break;
	case 6:
		iface_id = ads_iface_filter_dl_ipv6_pkt(priv, skb);
		break;
	default:
		break;
	}

	return iface_id;
}

/*
 * ads_iface_shared_iface_select --Find the right iface for sending dl data.
 * @skb: sk buffer
 *
 * Return the priv of the right iface.
 */
struct ads_iface_priv_s *ads_iface_shared_iface_select(
		struct ads_iface_priv_s *priv, struct sk_buff *skb)
{
	struct ads_iface_priv_s    *new_priv  = NULL;
	struct ads_iface_context_s *iface_ctx = ADS_IFACE_CTX();
	size_t cache_size;
	uint8_t iface_id;

	if (unlikely(priv->shared_iface_id != PS_IFACE_ID_BUTT)) {
		cache_size = skb->len < 64 ? skb->len : 64;
		dma_unmap_single(iface_ctx->dev, virt_to_phys(skb->data),
			 cache_size, DMA_FROM_DEVICE);

		iface_id = ads_iface_filter_dl_data(priv, skb);
		new_priv = ads_iface_priv(iface_id);
		if (new_priv != NULL)
			priv = new_priv;
	}

	return priv;
}

#endif


