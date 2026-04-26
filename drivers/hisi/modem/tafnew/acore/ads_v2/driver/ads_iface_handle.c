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
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/static_key.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#if (defined(CONFIG_HUAWEI_BASTET) || defined(CONFIG_HW_DPIMARK_MODULE))
#include <net/inet_sock.h>
#include <linux/version.h>
#endif
#include "securec.h"
#include "ads_iface_handle.h"
#include "ads_iface_dpl.h"
#include "ads_iface_debug.h"
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
#include "ads_iface_filter.h"
#endif


/*
 * ads_iface_kfree_skb - Free skb base on flag..
 * @skb: sk buffer
 * @flag: maa own flag
 */
void ads_iface_kfree_skb(struct sk_buff *skb, uint32_t flag)
{
	if (flag & ADS_DATA_FLAG_MAA)
		mdrv_maa_skb_free(skb); /* 由操作系统接管 */
	else
		dev_kfree_skb_any(skb);
}

/*
 * ads_iface_check_ipv4_parse_en
 * @skb: sk buffer
 *
 * Return true: ipv4 packet can be parsed by ESPE.
 */
 /*lint -save -e801*/
STATIC bool __ads_iface_check_ipv4_parse_en(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;

	if (unlikely(!pskb_may_pull(skb, sizeof(*iph))))
		goto not_match;

	iph = (struct iphdr *)skb->data;
	if (*(uint8_t *)iph != 0x45)
		goto not_match;

	if (ip_is_fragment(iph) || iph->protocol != IPPROTO_TCP)
		goto not_match;

	if (unlikely(sizeof(*iph) + sizeof(*tcph) > skb->len))
		goto not_match;

	tcph = (struct tcphdr *)(skb->data + sizeof(*iph));
	if (tcp_flag_word(tcph) & TCP_FLAG_SYN)
		goto not_match;

	return true;

not_match:
	return false;
}
/*lint -restore +e801*/

/*
 * ads_iface_check_ipv6_parse_en
 * @skb: sk buffer
 *
 * Return true: ipv6 packet can be parsed by ESPE.
 */
 /*lint -save -e801*/
STATIC bool __ads_iface_check_ipv6_parse_en(struct sk_buff *skb)
{
	struct ipv6hdr *ipv6h = NULL;
	struct tcphdr *tcph = NULL;
	uint32_t pkt_len;

	if (unlikely(!pskb_may_pull(skb, sizeof(*ipv6h))))
		goto not_match;

	ipv6h = (struct ipv6hdr *)skb->data;
	if (ipv6h->nexthdr != IPPROTO_TCP)
		goto not_match;

	pkt_len = ntohs(ipv6h->payload_len);
	if (unlikely(pkt_len < sizeof(*tcph) ||
		     pkt_len + sizeof(*ipv6h) > skb->len))
		goto not_match;

	tcph = (struct tcphdr *)(skb->data + sizeof(*ipv6h));
	if (tcp_flag_word(tcph) & TCP_FLAG_SYN)
		goto not_match;

	return true;

not_match:
	return false;
}
/*lint -restore +e801*/

STATIC bool __ads_iface_check_maa_parse_en(struct sk_buff *skb)
{
	if (skb->len >= ADS_ESPE_PARSE_MIN_IP_LEN)
		return true;
	else
		return false;
}

bool ads_iface_check_ipv4_parse_en(struct sk_buff *skb, uint32_t flag)
{
	if (flag & ADS_DATA_FLAG_MAA)
		return __ads_iface_check_maa_parse_en(skb);
	else
		return __ads_iface_check_ipv4_parse_en(skb);
}

bool ads_iface_check_ipv6_parse_en(struct sk_buff *skb, uint32_t flag)
{
	if (flag & ADS_DATA_FLAG_MAA)
		return __ads_iface_check_maa_parse_en(skb);
	else
		return __ads_iface_check_ipv6_parse_en(skb);
}

/*
 * ads_iface_check_eth_parse_en
 * @skb: sk buffer
 *
 * Return true: eth packet can be parsed by ESPE.
 */
 /*lint -save -e801*/
bool ads_iface_check_eth_parse_en(struct sk_buff *skb, uint32_t flag)
{
	/*
	 * VLAN packet, valid len is more than 58,
	 * other eth packet, valid len is more than 54.
	 */
	if (skb->protocol == htons(ETH_P_8021Q)) {
		if (skb->len >= ADS_PARSE_MIN_VLAN_LEN)
			return true;
	} else {
		if (skb->len >= ADS_PARSE_MIN_ETH_LEN)
			return true;
	}

	return false;
}
/*lint -restore +e801*/

/*
 * ads_iface_get_skb_ps_info - Get ps info by cust protocol.
 * @skb: sk buffer
 *
 * Return pointer to ps information structure.
 */
STATIC struct ads_iface_ps_info_s *ads_iface_get_skb_ps_info(
	struct ads_iface_priv_s *priv, struct sk_buff *skb)
{
	struct ads_iface_ps_info_s *ps_info = NULL;

	if (test_bit(ADS_IPV4_ADDR, &priv->addr_family_mask) ||
	    test_bit(ADS_IPV6_ADDR, &priv->addr_family_mask)) {
		ps_info = skb->protocol == htons(ETH_P_IP) ? /*lint !e778*/
			  &priv->ps_infos[ADS_IPV4_ADDR] :
			  &priv->ps_infos[ADS_IPV6_ADDR];
	} else if (test_bit(ADS_ETH_ADDR, &priv->addr_family_mask)) {
		ps_info = &priv->ps_infos[ADS_ETH_ADDR];
	} else {
		ps_info = &priv->ps_infos[ADS_RAW_ADDR];
	}

	return ps_info;
}

/*
 * ads_iface_set_skb_protocol - Set protocol of sk buffer.
 * @priv: iface private info
 * @skb: sk buffer
 *
 * Return pointer to ps information structure.
 */
STATIC struct ads_iface_ps_info_s *ads_iface_set_skb_protocol(
	struct ads_iface_priv_s *priv, struct sk_buff *skb)
{
	struct rx_cb_map_s *map = ADS_ESPE_RD_SKB_CB(skb);
	struct ads_iface_ps_info_s *ps_info = NULL;

	if (test_bit(ADS_IPV4_ADDR, &priv->addr_family_mask) ||
	    test_bit(ADS_IPV6_ADDR, &priv->addr_family_mask)) {
		if (!map->ipf_result.bits.ip_type) {
			skb->protocol = htons(ETH_P_IP);/*lint !e778*/
			ps_info = &priv->ps_infos[ADS_IPV4_ADDR];
		} else {
			skb->protocol = htons(ETH_P_IPV6);/*lint !e778*/
			ps_info = &priv->ps_infos[ADS_IPV6_ADDR];
		}
	} else if (test_bit(ADS_ETH_ADDR, &priv->addr_family_mask)) {
		skb->protocol = 0;
		ps_info = &priv->ps_infos[ADS_ETH_ADDR];
	} else {
		skb->protocol = 0;
		ps_info = &priv->ps_infos[ADS_RAW_ADDR];
	}

	return ps_info;
}

uint8_t ads_iface_get_iface_id(struct rx_cb_map_s *map)
{
	return (uint8_t)(map->userfield0 & ADS_ESPE_RD_IFACEID_MASK);
}

/*
 * ads_iface_build_user_field2 - Build userfield2 of espe BD.
 * @skb: sk buffer
 *
 * Return userfield2 value.
 */
STATIC uint32_t ads_iface_build_user_field2(struct sk_buff *skb)
{
#if (defined(CONFIG_HUAWEI_BASTET) || defined(CONFIG_HW_DPIMARK_MODULE))
	struct sock *sk = NULL;
#ifdef CONFIG_HW_DPIMARK_MODULE
	uint8_t *tmp = NULL;
#endif
	uint32_t value;

	if (unlikely(skb == NULL))
		return 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 23))
	sk = skb_to_full_sk(skb);
#else
	sk = skb->sk;
#endif
	if (sk != NULL) {
		value = 0;
#ifdef CONFIG_HUAWEI_BASTET
		value |= (sk->acc_state & 0x01);
		value |= (((uint32_t)sk->discard_duration << 8) & 0xFF00);
#endif
#ifdef CONFIG_HW_DPIMARK_MODULE
		tmp = (uint8_t *)&(sk->sk_hwdpi_mark);
		value |= tmp[0];
		value |= tmp[1];
#endif
		return value;
	}
#endif

	return 0;
}

/*
 * ads_iface_hold_wakelock - Hold wake lock for specific timeout
 * @lock: The structure of wakeup source and configuration.
 */
STATIC void ads_iface_hold_wakelock(struct ads_iface_wakelock_s *lock)
{
	if (likely(lock->timeout))
		__pm_wakeup_event(&lock->ws, lock->timeout);
}

/*
 * ads_iface_fill_ip_wan_info - Fill wan info for egress ip packet.
 * @priv: iface private info
 * @skb: sk buffer
 * @flag: data flag
 * @maa_flag: maa own flag
 */
void ads_iface_fill_ip_wan_info(struct sk_buff *skb,
				struct ads_iface_ps_info_s *ps_info,
				uint32_t flag, uint32_t *maa_flag)
{
	struct wan_info_s *wan = ADS_ESPE_TD_SKB_CB(skb);

	wan->info = ps_info->ipf_info;

	if (ps_info->ops->check_parse_en != NULL)
		wan->info.parse_en = ps_info->ops->check_parse_en(skb, flag) ? 1 : 0;

	if (!(flag & ADS_DATA_FLAG_MAA))
		wan->userfield2 = ads_iface_build_user_field2(skb);

	wan->userfield1 = 0;
	wan->userfield0 = wan->userfield2 ?
			  ps_info->idx | 0x80000000 : ps_info->idx;

	skb_push(skb, ETH_HLEN);
	skb->mac_len = ETH_HLEN;

	if (flag & ADS_DATA_FLAG_MAA)
		*maa_flag = WAN_MAA_OWN;
	else
		((struct ethhdr *)skb->data)->h_proto = skb->protocol;
}

/*
 * ads_iface_fill_raw_wan_info - Fill wan info for egress raw packet.
 * @priv: iface private info
 * @skb: sk buffer
 * @flag: data flag
 * @maa_flag: maa own flag
 */
void ads_iface_fill_raw_wan_info(struct sk_buff *skb,
				 struct ads_iface_ps_info_s *ps_info,
				 uint32_t flag, uint32_t *maa_flag)
{
	struct wan_info_s *wan = ADS_ESPE_TD_SKB_CB(skb);

	wan->info = ps_info->ipf_info;

	wan->userfield2 = 0;
	wan->userfield1 = (flag & ADS_DATA_FLAG_LTEV) ? wan->userfield1 : 0;
	wan->userfield0 = ps_info->idx;

	skb_push(skb, ETH_HLEN);
	skb->mac_len = ETH_HLEN;

	if (flag & ADS_DATA_FLAG_MAA)
		*maa_flag = WAN_MAA_OWN;
	else
		((struct ethhdr *)skb->data)->h_proto = skb->protocol;
}

/*
 * ads_iface_fill_eth_wan_info - Fill wan info for egress ethernet packet.
 * @priv: iface private info
 * @skb: sk buffer
 * @flag: data flag
 * @maa_flag: maa own flag
 */
void ads_iface_fill_eth_wan_info(struct sk_buff *skb,
				 struct ads_iface_ps_info_s *ps_info,
				 uint32_t flag, uint32_t *maa_flag)
{
	struct wan_info_s *wan = ADS_ESPE_TD_SKB_CB(skb);

	wan->info = ps_info->ipf_info;

	if (ps_info->ops->check_parse_en != NULL)
		wan->info.parse_en = ps_info->ops->check_parse_en(skb, flag) ? 1 : 0;

	wan->userfield2 = 0;
	wan->userfield1 = 0;
	wan->userfield0 = skb->protocol == htons(ETH_P_8021Q) ? /*lint !e778*/
			  ps_info->idx | ADS_ESPE_ETH_OFFSET2 :
			  ps_info->idx | ADS_ESPE_ETH_OFFSET1;

	skb->mac_len = 0;

	if (flag & ADS_DATA_FLAG_MAA)
		*maa_flag = WAN_MAA_OWN | WAN_ESPE_L2ADDR_FROM_CB;
}

/*
 * ads_iface_data_egress - Iface data egress handle function.
 * @priv: iface private info
 * @skb: sk buffer
 * @flag: flag of maa own skb
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
int ads_iface_data_egress(struct ads_iface_priv_s *priv,
			  struct sk_buff *skb, uint32_t flag)
{
	struct ads_iface_context_s *iface_ctx = ADS_IFACE_CTX();
	struct ads_iface_ps_info_s *ps_info = NULL;
	unsigned int maa_flag = 0;

	priv->data_stats.tx_total_packets++;
	iface_ctx->dsflow_stats.tx_packets++;
	iface_ctx->dsflow_stats.tx_bytes += skb->len;

	if (unlikely(!skb->len)) {
		priv->data_stats.tx_length_errors++;
		goto out_free_drop;
	}

	ps_info = ads_iface_get_skb_ps_info(priv, skb);
	if (unlikely(!test_bit(ps_info->addr, &priv->addr_family_mask))){
		priv->data_stats.tx_state_errors++;
		goto out_free_drop;
	}

	ps_info->ops->fill_wan_info(skb, ps_info, flag, &maa_flag);
	ads_iface_dpl_tx_pkt_info_rec(iface_ctx, skb, flag);

	if (mdrv_wan_tx(skb, maa_flag)) {
		iface_ctx->espe_stats.tx_dropped++;
		iface_ctx->espe_stats.tx_wan_errors++;
	} else {
		iface_ctx->espe_stats.tx_packets++;
	}

	iface_ctx->espe_stats.tx_total_packets++;
	priv->data_stats.tx_packets++;
	return 0;

out_free_drop:
	ads_iface_kfree_skb(skb, flag);
	priv->data_stats.tx_dropped++;
	return -EFAULT;
}
/*lint -restore +e801*/

/*
 * ads_iface_data_ingress - Iface data ingress handle function.
 * @priv: iface private info
 * @skb: sk buffer
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
STATIC int ads_iface_data_ingress(struct ads_iface_priv_s *priv,
				  struct sk_buff *skb)
{
	struct ads_iface_context_s *iface_ctx = ADS_IFACE_CTX();
	struct ads_iface_ps_info_s *ps_info = NULL;
	ads_iface_rx_func_t rx_func;

	priv->data_stats.rx_total_packets++;
	iface_ctx->dsflow_stats.rx_packets++;
	iface_ctx->dsflow_stats.rx_bytes += skb->len;

	ps_info = ads_iface_set_skb_protocol(priv, skb);
	if (!test_bit(ps_info->addr, &priv->addr_family_mask)) {
		priv->data_stats.rx_state_errors++;
		goto out_free_drop;
	}

	rx_func = priv->rx_func;
	if (rx_func) {
		__set_bit(priv->iface_id, iface_ctx->rx_cmplt_mask);
		if (!priv->rx_dma_support)
			dma_unmap_single(iface_ctx->dev,
					 virt_to_phys(skb->data - ETH_HLEN),
					 skb->len + ETH_HLEN, DMA_FROM_DEVICE);
		rx_func(priv->user_data, skb);
	} else {
		priv->data_stats.rx_carrier_errors++;
		goto out_free_drop;
	}

	priv->data_stats.rx_packets++;
	return 0;

out_free_drop:
	ads_iface_kfree_skb(skb, 0);
	priv->data_stats.rx_dropped++;
	return -EFAULT;
}
/*lint -restore +e801*/

/*
 * ads_iface_data_ingress - Iface special data handle function.
 * @iface_ctx: iface context
 * @skb: sk buffer
 *
 * Return 0 on success, negative on failure.
 */
STATIC int ads_iface_special_data_handle(struct ads_iface_context_s *iface_ctx,
					 struct sk_buff *skb)
{
	if (iface_ctx->hook_ops.special_data_hook) {
		dma_unmap_single(iface_ctx->dev,
				 virt_to_phys(skb->data - ETH_HLEN),
				 skb->len + ETH_HLEN, DMA_FROM_DEVICE);
		iface_ctx->hook_ops.special_data_hook(skb);
		return 0;
	}

	ADS_LOGE("special_data_hook is null.");
	ads_iface_kfree_skb(skb, 0);
	return -EFAULT;
}

/*
 * ads_iface_espe_rx_complete - Rx complete handle function.
 */
void ads_iface_espe_rx_complete(void)
{
	struct ads_iface_context_s *iface_ctx = ADS_IFACE_CTX();
	struct ads_iface_priv_s *priv = NULL;
	ads_iface_rx_cmplt_func_t rx_cmplt_func = NULL;
	uint64_t bitmask;
	unsigned int bit;
	unsigned int base = ~0U;

	bitmask = iface_ctx->rx_cmplt_bitmask;
	while (bitmask) {
		bit = (unsigned int)__ffs64(bitmask) + 1;
		bitmask >>= bit;
		base += bit;
		__clear_bit((int)base, iface_ctx->rx_cmplt_mask);

		priv = ads_iface_priv((uint8_t)base);
		if (unlikely(priv == NULL))
			continue;

		rx_cmplt_func = priv->rx_cmplt_func;
		if (rx_cmplt_func)
			rx_cmplt_func(priv->user_data);
	}

	ads_iface_dpl_report_rx_pkt_rec(iface_ctx);
	ads_iface_hold_wakelock(&iface_ctx->lock);
}

/*
 * ads_iface_espe_rx_handle - Rx handle function.
 * @skb: sk buffer
 *
 * Return WAN_RX_CONSUMED if skb was consumed, WAN_RX_PASS as if no rx_handler
 * was called.
 */
/*lint -save -e801*/
enum wan_rx_handle_rslt ads_iface_espe_rx_handle(struct sk_buff *skb)
{
	struct rx_cb_map_s *map = ADS_ESPE_RD_SKB_CB(skb);
	struct ads_iface_context_s *iface_ctx = ADS_IFACE_CTX();
	struct ads_iface_priv_s *priv = NULL;
	enum wan_rx_handle_rslt ret = WAN_RX_CONSUMED;

	iface_ctx->espe_stats.rx_total_packets++;

	ads_iface_dpl_rx_pkt_info_rec(iface_ctx, skb, 0);

	if (unlikely(map->ipf_result.u32 & ADS_IFACE_IPF_FILTER_ERR_MASK))
		iface_ctx->espe_stats.rx_filter_errors++;

	if (unlikely(map->ipf_result.bits.dl_special_flag)) {
		ads_iface_special_data_handle(iface_ctx, skb);
		goto out_succ;
	}

	if (unlikely(iface_ctx->rx_discard_flag)) {
		iface_ctx->espe_stats.rx_discard_errors++;
		goto out_drop_free;
	}

	priv = ads_iface_priv((uint8_t)(map->userfield0 & ADS_ESPE_RD_IFACEID_MASK));
	if (unlikely(priv == NULL)) {
		iface_ctx->espe_stats.rx_iface_errors++;
		goto out_drop_free;
	}

#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
	priv = ads_iface_shared_iface_select(priv, skb);
#endif

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
	if (priv->bypass_mode) {
		ret = WAN_RX_PASS;
		goto out_succ;
	}
#endif

	ads_iface_data_ingress(priv, skb);

out_succ:
	iface_ctx->espe_stats.rx_packets++;
	return ret;

out_drop_free:
	ads_iface_kfree_skb(skb, 0);
	iface_ctx->espe_stats.rx_dropped++;
	return ret;
}
/*lint -restore +e801*/

/*
 * ads_iface_espe_lev_report - Water level handle function of epse bd fifo.
 * @level: water levle
 */
int ads_iface_espe_lev_report(enum wan_waterlevel_e level)
{
	return 0;
}

