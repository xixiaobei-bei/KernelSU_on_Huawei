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

#ifndef __ADS_PS_IFACE_HANDLE_H__
#define __ADS_PS_IFACE_HANDLE_H__

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "ads_iface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * bit mask of ipf_result_u.err
 * 0000 0000 0000 0001
 * 1111 1000 0000 0000
 */
#define ADS_IFACE_IPF_FILTER_ERR_MASK	0x0001F800

/* bit mask for maa own skb */
#define ADS_IFACE_SKB_MAA_OWN_MASK	0x00000001

#define ADS_ESPE_ETH_OFFSET1		0x40
#define ADS_ESPE_ETH_OFFSET2		0x80
#define ADS_ESPE_RD_IFACEID_MASK	0x0000003F
#define ADS_ESPE_RD_PKTTYPE_MASK	0x000000C0

#define ADS_ESPE_PARSE_MIN_IP_LEN	40

#define ADS_PARSE_MIN_ETH_LEN	54
#define ADS_PARSE_MIN_VLAN_LEN	58

bool ads_iface_check_ipv4_parse_en(struct sk_buff *skb, uint32_t flag);
bool ads_iface_check_ipv6_parse_en(struct sk_buff *skb, uint32_t flag);
bool ads_iface_check_eth_parse_en(struct sk_buff *skb, uint32_t flag);
void ads_iface_fill_ip_wan_info(struct sk_buff *skb,
				struct ads_iface_ps_info_s *ps_info,
				uint32_t flag, uint32_t *maa_flag);
void ads_iface_fill_raw_wan_info(struct sk_buff *skb,
				 struct ads_iface_ps_info_s *ps_info,
				 uint32_t flag, uint32_t *maa_flag);
void ads_iface_fill_eth_wan_info(struct sk_buff *skb,
				 struct ads_iface_ps_info_s *ps_info,
				 uint32_t flag, uint32_t *maa_flag);

void ads_iface_kfree_skb(struct sk_buff *skb, uint32_t flag);

int ads_iface_data_egress(struct ads_iface_priv_s *priv,
			  struct sk_buff *skb, uint32_t flag);

enum wan_rx_handle_rslt ads_iface_espe_rx_handle(struct sk_buff *skb);
void ads_iface_espe_rx_complete(void);
int ads_iface_espe_lev_report(enum wan_waterlevel_e level);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_PS_IFACE_HANDLE_H__ */
