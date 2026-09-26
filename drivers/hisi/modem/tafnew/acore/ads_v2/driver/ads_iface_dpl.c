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

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include "securec.h"
#include "ads_iface.h"
#include "ads_iface_dpl.h"
#include "ads_iface_debug.h"


struct static_key ads_dpl_needed __read_mostly = STATIC_KEY_INIT_FALSE;  //lint !e10 !e86

/*
 * ads_iface_dpl_set_needed_flg - Set dpl needed flag.
 * @needed_flg: needed flag
 */
void ads_iface_dpl_set_needed_flg(bool needed_flg)
{
	ADS_LOGH("dpl needed flag is %d", needed_flg);

	if (needed_flg == true)
		static_key_enable(&ads_dpl_needed);
	else
		static_key_disable(&ads_dpl_needed);
}

/*
 * ads_iface_dpl_decode_l4_pkt_info - Decode the ipv4 packet information.
 * @ip_pkt: ip data
 * @pkt_info: ip packet record
 */
STATIC void ads_iface_dpl_decode_l4_pkt_info(uint8_t *ip_pkt,
					     struct ads_ip_pkt_info_s *pkt_info)
{
	struct tcphdr  *tcp_hdr = NULL;
	struct udphdr  *udp_hdr = NULL;
	struct icmphdr *icmp_hdr = NULL;

	switch (pkt_info->l4_proto) {
	case IPPROTO_TCP:
		tcp_hdr = (struct tcphdr *)ip_pkt;
		pkt_info->src_port = ntohs(tcp_hdr->source);
		pkt_info->dst_port = ntohs(tcp_hdr->dest);
		pkt_info->l4_id = ntohl(tcp_hdr->seq);
		pkt_info->tcp_ack_seq = ntohl(tcp_hdr->ack_seq);
		break;
        case IPPROTO_UDP:
		udp_hdr = (struct udphdr *)ip_pkt;
		pkt_info->src_port = ntohs(udp_hdr->source);
		pkt_info->dst_port = ntohs(udp_hdr->dest);
		break;
        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
		icmp_hdr = (struct icmphdr *)ip_pkt;
		pkt_info->icmp_type = icmp_hdr->type;
		if (pkt_info->icmp_type == ICMP_ECHO ||
		    pkt_info->icmp_type == ICMP_ECHOREPLY ||
		    pkt_info->icmp_type == ICMPV6_ECHO_REQUEST ||
		    pkt_info->icmp_type == ICMPV6_ECHO_REPLY) {
			pkt_info->l4_id = ntohs(icmp_hdr->un.echo.id);
			pkt_info->tcp_ack_seq = ntohs(icmp_hdr->un.echo.sequence);
		}
		break;
	default:
		pkt_info->l4_id = ntohl(((uint32_t *)ip_pkt)[0]);
		pkt_info->tcp_ack_seq = ntohl(((uint32_t *)ip_pkt)[1]);
		break;
	}
}

/*
 * ads_iface_dpl_decode_ipv6_pkt_info - Decode the ipv6 packet information.
 * @ip_pkt: ip data
 * @pkt_info: ip packet record
 */
STATIC void ads_iface_dpl_decode_ipv6_pkt_info(uint8_t *ip_pkt,
					       struct ads_ip_pkt_info_s *pkt_info)
{
	struct ipv6hdr *ipv6h = (struct ipv6hdr *)ip_pkt;

	pkt_info->ip_ver = 6;
	pkt_info->l4_proto = ipv6h->nexthdr;
	pkt_info->ip4_id = 0;
	pkt_info->data_len = ntohs(ipv6h->payload_len) + sizeof(struct ipv6hdr);

	ip_pkt += sizeof(struct ipv6hdr);
	ads_iface_dpl_decode_l4_pkt_info(ip_pkt, pkt_info);
}

/*
 * ads_iface_dpl_decode_ipv4_pkt_info - Decode the ipv4 packet information.
 * @ip_pkt: ip data
 * @pkt_info: ip packet record
 */
STATIC void ads_iface_dpl_decode_ipv4_pkt_info(uint8_t *ip_pkt,
					       struct ads_ip_pkt_info_s *pkt_info)
{
	struct iphdr *iph = (struct iphdr *)ip_pkt;

	pkt_info->ip_ver = 4;
	pkt_info->l4_proto = iph->protocol;
	pkt_info->ip4_id = ntohs(iph->id);
	pkt_info->data_len = ntohs(iph->tot_len);

	ip_pkt += sizeof(struct iphdr);
	ads_iface_dpl_decode_l4_pkt_info(ip_pkt, pkt_info);
}

/*
 * ads_iface_dpl_decode_ip_pkt_info - Decode the ip packet information.
 * @data: point to ip header
 * @pkt_info: ip packet record
 */
STATIC void ads_iface_dpl_decode_ip_pkt_info(uint8_t *data,
					     struct ads_ip_pkt_info_s *pkt_info)
{
	switch (data[0] & ADS_DATA_IP_VER_MASK) {
	case ADS_DATA_IP_VER_4:
		ads_iface_dpl_decode_ipv4_pkt_info(data, pkt_info);
		break;
	case ADS_DATA_IP_VER_6:
		ads_iface_dpl_decode_ipv6_pkt_info(data, pkt_info);
		break;
	default:
		pkt_info->ip_ver = data[0];
		break;
	}
}

/*
 * __ads_iface_dpl_report_rx_pkt_rec - Report record of rx packet.
 * iface_ctx: iface context
 */
STATIC void __ads_iface_dpl_report_rx_pkt_rec(struct ads_iface_context_s *iface_ctx)
{
	if (iface_ctx->hook_ops.report_rx_pkt)
		iface_ctx->hook_ops.report_rx_pkt(&iface_ctx->ads_rx_pkt_rec_info);

	iface_ctx->ads_rx_pkt_rec_info.rpt_num = 0;
}

/*
 * __ads_iface_dpl_report_tx_pkt_rec - Report record of tx packet.
 * @iface_ctx: iface context
 * @rec_info: tx ip packet record
 */
STATIC void __ads_iface_dpl_report_tx_pkt_rec(struct ads_iface_context_s *iface_ctx,
					      struct ads_tx_ip_pkt_rec_s *rec_info)
{
	if (iface_ctx->hook_ops.report_tx_pkt)
		iface_ctx->hook_ops.report_tx_pkt(rec_info);

	rec_info->rpt_num = 0;
}

/*
 * ads_iface_dpl_rx_ip_pkt_info_rec - Record the ip pkt information of rx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 */
STATIC void ads_iface_dpl_rx_ip_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
					     struct sk_buff *skb)
{
	struct rx_cb_map_s *map = ADS_ESPE_RD_SKB_CB(skb);
	struct ads_rx_ip_pkt_rec_s *rec_stru = &iface_ctx->ads_rx_pkt_rec_info;
	struct ads_ip_pkt_info_s   *pkt_info = NULL;

	pkt_info = &(rec_stru->ip_pkt_rec_info[rec_stru->rpt_num]);
	pkt_info->ipf_info.u32 = map->ipf_result.u32;
	pkt_info->param0 = map->userfield0;
	pkt_info->param1 = map->userfield1;
	pkt_info->param2 = map->userfield2;

	dma_unmap_single(iface_ctx->dev, virt_to_phys(skb->data),
			 skb->len < 64 ? skb->len : 64, DMA_FROM_DEVICE);
	ads_iface_dpl_decode_ip_pkt_info(skb->data, pkt_info);

	rec_stru->rpt_num++;
	if (rec_stru->rpt_num >= ADS_MNTN_REC_RX_PKT_MAX_NUM)
		__ads_iface_dpl_report_rx_pkt_rec(iface_ctx);
}

/*
 * ads_iface_dpl_tx_ip_pkt_info_rec - Record the ip pkt information of tx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 */
STATIC void ads_iface_dpl_tx_ip_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
					     struct sk_buff *skb)
{
	struct wan_info_s *wan = ADS_ESPE_TD_SKB_CB(skb);
	struct ads_ip_pkt_info_s *pkt_info = NULL;
	struct ads_tx_ip_pkt_rec_s rec_stru;
	int ret;

	memset_s(&rec_stru, sizeof(rec_stru), 0, sizeof(rec_stru));

	pkt_info = &(rec_stru.ip_pkt_rec_info[rec_stru.rpt_num]);
	ret = memcpy_s(&pkt_info->ipf_info, sizeof(struct ipf_info_s),
		       &wan->info, sizeof(struct ipf_info_s));
	ADS_MEMCPY_RET_CHECK(ret, sizeof(struct ipf_info_s), sizeof(struct ipf_info_s));
	pkt_info->param0 = wan->userfield0;
	pkt_info->param1 = (skb->sk != NULL) ? (unsigned int)sk_wmem_alloc_get(skb->sk) : 0;
	pkt_info->param2 = wan->userfield2;

	ads_iface_dpl_decode_ip_pkt_info(skb->data + ETH_HLEN, pkt_info);

	rec_stru.rpt_num++;
	if (rec_stru.rpt_num >= ADS_MNTN_REC_TX_PKT_MAX_NUM)
		__ads_iface_dpl_report_tx_pkt_rec(iface_ctx, &rec_stru);
}

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
/*
 * __ads_iface_dpl_report_rx_ltev_pkt_rec - Report record of ltev rx packet.
 * iface_ctx: iface context
 */
STATIC void __ads_iface_dpl_report_rx_ltev_pkt_rec(struct ads_iface_context_s *iface_ctx)
{
	if (iface_ctx->hook_ops.report_ltev_rx_pkt)
		iface_ctx->hook_ops.report_ltev_rx_pkt(&iface_ctx->ads_rx_ltev_pkt_rec_info);

	iface_ctx->ads_rx_ltev_pkt_rec_info.rpt_num = 0;
}

/*
 * __ads_iface_dpl_report_tx_ltev_pkt_rec - Report record of ltev tx packet.
 * iface_ctx: iface context
 */
STATIC void __ads_iface_dpl_report_tx_ltev_pkt_rec(struct ads_iface_context_s *iface_ctx,
						   struct ads_tx_ltev_pkt_rec_s *rec_info)
{
	if (iface_ctx->hook_ops.report_ltev_tx_pkt)
		iface_ctx->hook_ops.report_ltev_tx_pkt(rec_info);

	rec_info->rpt_num = 0;
}

/*
 * ads_iface_dpl_rx_ltev_pkt_info_rec - Record the ltev pkt information of rx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 */
STATIC void ads_iface_dpl_rx_ltev_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
					       struct sk_buff *skb)
{
	struct rx_cb_map_s *map = ADS_ESPE_RD_SKB_CB(skb);
	struct ads_ltev_dl_info_s *pkt_info = NULL;
	struct ads_rx_ltev_pkt_rec_s *rec_stru = &iface_ctx->ads_rx_ltev_pkt_rec_info;
	size_t info_size = sizeof(struct ads_ltev_dl_info_s) - sizeof(unsigned int);
	size_t cache_size;

	cache_size = skb->len < 64 ? skb->len : 64;
	dma_unmap_single(iface_ctx->dev, virt_to_phys(skb->data),
			 cache_size, DMA_FROM_DEVICE);

	pkt_info = &(rec_stru->ltev_dl_rec_info[rec_stru->rpt_num]);

	if (memcpy_s(pkt_info, sizeof(struct ads_ltev_dl_info_s), skb->data,
		(skb->len < info_size) ? skb->len : info_size) != EOK) {
		ADS_LOGE("memcpy failed.");
		return;
	}

	pkt_info->frag_info = map->userfield1;
	rec_stru->rpt_num++;
	if (rec_stru->rpt_num >= ADS_MNTN_REC_RX_PKT_MAX_NUM)
		__ads_iface_dpl_report_rx_ltev_pkt_rec(iface_ctx);
}

/*
 * ads_iface_dpl_tx_ltev_pkt_info_rec - Record the ltev pkt information of tx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 */
STATIC void ads_iface_dpl_tx_ltev_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
					       struct sk_buff *skb)
{
	struct wan_info_s *wan = ADS_ESPE_TD_SKB_CB(skb);
	struct ads_ltev_ul_info_s *pkt_info = NULL;
	struct ads_tx_ltev_pkt_rec_s rec_stru;
	size_t info_size = sizeof(struct ads_ltev_ul_info_s) - sizeof(unsigned int);
	uint8_t *data = skb->data + ETH_HLEN;
	uint32_t len = skb->len - ETH_HLEN;

	memset_s(&rec_stru, sizeof(rec_stru), 0, sizeof(rec_stru));
	pkt_info = &(rec_stru.ltev_ul_rec_info[rec_stru.rpt_num]);

	if (memcpy_s(pkt_info, sizeof(struct ads_ltev_ul_info_s), data,
		     (len < info_size) ? len : info_size) != EOK) {
		ADS_LOGE("memcpy failed.");
		return;
	}

	pkt_info->frag_info = wan->userfield1;
	rec_stru.rpt_num++;
	if (rec_stru.rpt_num >= ADS_MNTN_REC_TX_PKT_MAX_NUM)
		__ads_iface_dpl_report_tx_ltev_pkt_rec(iface_ctx, &rec_stru);
}
#endif

/*
 * ads_iface_dpl_rx_pkt_info_rec - Record packet information of rx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 * @flag: maa own flag
 */
void ads_iface_dpl_rx_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
				   struct sk_buff *skb, uint32_t flag)
{
	union ads_espe_usrfield0_u *usrfield0 = ADS_ESPE_RD_UFLD0_SKB_CB(skb);

	if (!static_key_false(&ads_dpl_needed))
		return;

	if (unlikely(flag & ADS_DATA_FLAG_MAA))
		return;

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
	if (unlikely(usrfield0->iface_id == PS_IFACE_ID_LTEV)) {
		ads_iface_dpl_rx_ltev_pkt_info_rec(iface_ctx, skb);
		return;
	}
#endif

	switch (usrfield0->pkt_type) {
	case 0:
		ads_iface_dpl_rx_ip_pkt_info_rec(iface_ctx, skb);
		break;
	default:
		break;
	}
}

/*
 * ads_iface_dpl_rx_pkt_info_rec - Record packet information of tx path.
 * iface_ctx: iface context
 * @skb: sk buffer
 * @flag: maa own flag
 */
void ads_iface_dpl_tx_pkt_info_rec(struct ads_iface_context_s *iface_ctx,
				   struct sk_buff *skb, uint32_t flag)
{
	union ads_espe_usrfield0_u *usrfield0 = ADS_ESPE_TD_UFLD0_SKB_CB(skb);

	if (!static_key_false(&ads_dpl_needed))
		return;

	if (unlikely(flag & ADS_DATA_FLAG_MAA))
		return;

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
	if (unlikely(usrfield0->iface_id == PS_IFACE_ID_LTEV)) {
		ads_iface_dpl_tx_ltev_pkt_info_rec(iface_ctx, skb);
		return;
	}
#endif

	switch (usrfield0->pkt_type) {
	case 0:
		ads_iface_dpl_tx_ip_pkt_info_rec(iface_ctx, skb);
		break;
	default:
		break;
	}
}

/*
 * ads_iface_dpl_report_rx_pkt_rec - Report record of rx packet.
 * iface_ctx: iface context
 */
void ads_iface_dpl_report_rx_pkt_rec(struct ads_iface_context_s *iface_ctx)
{
	if (!static_key_false(&ads_dpl_needed))
		return;

	__ads_iface_dpl_report_rx_pkt_rec(iface_ctx);
#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
	__ads_iface_dpl_report_rx_ltev_pkt_rec(iface_ctx);
#endif
}

