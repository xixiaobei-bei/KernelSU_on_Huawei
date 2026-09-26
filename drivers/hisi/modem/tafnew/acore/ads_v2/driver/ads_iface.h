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

#ifndef __ADS_PS_IFACE_H__
#define __ADS_PS_IFACE_H__

#include <linux/types.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include "mdrv_maa.h"
#include "mdrv_wan.h"
#include "ads_dev_i.h"
#include "ads_iface_i.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#if (defined(LLT_OS_VER))
#define STATIC
#else
#define STATIC static
#endif

#define ADS_DATA_IP_VER_4	0x40
#define ADS_DATA_IP_VER_6	0x60
#define ADS_DATA_IP_VER_MASK	0xF0

#define ADS_IFACE_CTX()		(&ads_iface_context)
#define ADS_ESPE_TD_SKB_CB(skb)	((struct wan_info_s *)&((skb)->cb[0]))
#define ADS_ESPE_RD_SKB_CB(skb)	((struct rx_cb_map_s *)&((skb)->cb[0]))

/*
 * data flags
 *
 * @ADS_DATA_FLAG_MAA: maa own memory
 * @ADS_DATA_FLAG_LTEV: lte own memory
 */
enum {
	ADS_DATA_FLAG_MAA	= BIT(0),
	ADS_DATA_FLAG_LTEV	= BIT(1),
};

union ads_espe_usrfield0_u {
	struct {
		uint32_t iface_id :6;
		uint32_t pkt_type :2;
		uint32_t reserved :24;
	};
	uint32_t userfield0;
};

#define ADS_ESPE_TD_UFLD0_SKB_CB(skb) \
	((union ads_espe_usrfield0_u *)&(ADS_ESPE_TD_SKB_CB(skb)->userfield0))

#define ADS_ESPE_RD_UFLD0_SKB_CB(skb) \
	((union ads_espe_usrfield0_u *)&(ADS_ESPE_RD_SKB_CB(skb)->userfield0))

struct ads_iface_ps_info_s;

struct ads_iface_ps_ops_s {
	bool (*check_parse_en)(struct sk_buff *skb, uint32_t flag);
	void (*fill_wan_info)(struct sk_buff *skb,
			      struct ads_iface_ps_info_s *ps_info,
			      uint32_t flag, uint32_t *maa_flag);
};

struct ads_iface_ps_info_s {
	uint8_t idx;
	uint8_t addr;
	struct ipf_info_s ipf_info;
	const struct ads_iface_ps_ops_s *ops;
};

struct ads_iface_priv_s {
	struct ads_data_stats_s data_stats;

	uint8_t iface_id;
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
	uint8_t shared_iface_id;
#endif
	bool bypass_mode;

	unsigned long addr_family_mask;
	struct ads_iface_ps_info_s ps_infos[ADS_ADDR_BUTT];

	bool rx_dma_support;
	unsigned long user_data;
	ads_iface_rx_func_t rx_func;
	ads_iface_rx_cmplt_func_t rx_cmplt_func;
};

struct ads_iface_wakelock_s {
	uint32_t timeout;
	struct wakeup_source ws;
};

struct ads_iface_context_s {
	struct device *dev;
	struct ads_espe_stats_s espe_stats;
	struct ads_dsflow_stats_s dsflow_stats;

	struct ads_iface_hook_ops_s hook_ops;
	struct ads_iface_wakelock_s lock;

	struct ads_iface_priv_s privs[PS_IFACE_ID_BUTT];
	union {
		uint64_t rx_cmplt_bitmask;
		unsigned long rx_cmplt_mask[BITS_TO_LONGS(PS_IFACE_ID_BUTT)];
	};
	struct ads_tx_ip_pkt_rec_s ads_tx_pkt_rec_info;
	struct ads_rx_ip_pkt_rec_s ads_rx_pkt_rec_info;
#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
	struct ads_rx_ltev_pkt_rec_s ads_rx_ltev_pkt_rec_info;
#endif
	uint32_t rx_discard_flag;
	uint32_t reserved;
};

extern struct ads_iface_context_s ads_iface_context;

struct ads_iface_priv_s *ads_iface_priv(uint8_t iface_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_PS_IFACE_H__ */
