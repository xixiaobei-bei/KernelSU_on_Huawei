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

#ifndef __ADS_IFACE_I_H__
#define __ADS_IFACE_I_H__

#include <linux/types.h>
#include <linux/skbuff.h>
#include "mdrv_wan.h"
#include "ps_iface_global_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define ADS_ADDR_FAMILY_VALID(addr)	((addr) < ADS_ADDR_BUTT)

#define ADS_MNTN_REC_TX_PKT_MAX_NUM	1
#define ADS_MNTN_REC_RX_PKT_MAX_NUM	256

/*
 * enum ads_addr_type_e - address type
 */
enum ads_addr_type_e {
	ADS_IPV4_ADDR = 0,
	ADS_IPV6_ADDR,
	ADS_RAW_ADDR,
	ADS_ETH_ADDR,
	ADS_ADDR_BUTT
};

/*
 * enum ads_ps_iface_trans_mode_type_e - ps iface transfer mode
 */
enum ads_trans_mode_type_e {
	ADS_TRANS_MODE_IP = 0,
	ADS_TRANS_MODE_RAW,
	ADS_TRANS_MODE_ETH,
	ADS_TRANS_MODE_BUTT
};

struct ads_iface_config_s {
	uint8_t addr_family;
	uint8_t pdu_session_id;
	uint8_t fc_head;
	uint8_t modem_id;
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
	uint8_t shared_iface_id;
	uint8_t rsv[3];
#endif
};

struct ads_data_stats_s {
	uint32_t rx_total_packets;
	uint32_t tx_total_packets;

	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_dropped;
	uint32_t tx_dropped;

	/* detailed rx_errors: */
	uint32_t rx_state_errors;
	uint32_t rx_carrier_errors;

	/* detailed tx_errors */
	uint32_t tx_length_errors;
	uint32_t tx_state_errors;
};

struct ads_espe_stats_s {
	uint32_t rx_total_packets;
	uint32_t tx_total_packets;

	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_dropped;
	uint32_t tx_dropped;

	/* detailed rx_errors: */
	uint32_t rx_filter_errors;
	uint32_t rx_iface_errors;
	uint32_t rx_discard_errors;

	/* detailed tx_errors */
	uint32_t tx_wan_errors;
};

struct ads_dsflow_stats_s {
	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_bytes;
	uint32_t tx_bytes;
};

union ads_ipf_info_u {
	struct {
		unsigned int bid_qosid          :8;
		unsigned int pf_type            :2;
		unsigned int ff_type            :1;
		unsigned int version_err        :1;
		unsigned int head_len_err       :1;
		unsigned int bd_pkt_noeq        :1;
		unsigned int pkt_parse_err      :1;
		unsigned int bd_cd_noeq         :1;
		unsigned int pkt_len_err        :1;
		unsigned int modem_id           :2;
		unsigned int reserved           :1;
		unsigned int dl_special_flag    :1;
		unsigned int no_tcp_udp         :1;
		unsigned int ip_type            :1;
		unsigned int to_acpu_flag       :1;
		unsigned int pdu_session_id     :8;
	} rx_ipf_result_s;

	struct {
		unsigned int pdu_session_id:8;
		unsigned int fc_head:4;
		unsigned int modem_id:2;
		unsigned int higi_pri_flag:1;
		unsigned int parse_en:1;
		unsigned int reserve:16;
	} tx_ipf_info_s;

	unsigned int u32;
};

struct ads_ip_pkt_info_s {
	uint8_t  ip_ver;                                /* IP 协议版本 */
	uint8_t  l4_proto;                              /* 协议类型ICMP,TCP,UDP */
	uint8_t  icmp_type;                             /* ICMP报文类型*/
	uint8_t  rsv;

	uint16_t data_len;                              /* IP 数据包长度*/
	uint16_t ip4_id;                                /* IPv4 IDENTIFY字段 */
	uint16_t src_port;                              /* IP 源端口号*/
	uint16_t dst_port;                              /* IP 目的端口号 */

	uint32_t l4_id;                                 /* 层4的IDENTIFY,ICMP IDENTIFY+SN,TCP SEQ */
	uint32_t tcp_ack_seq;                           /* TCP ACK SEQ */
	union ads_ipf_info_u ipf_info;
	uint32_t param0;                                /* 自定义参数 */
	uint32_t param1;                                /* 自定义参数 */
	uint32_t param2;                                /* 自定义参数 */
};

struct ads_tx_ip_pkt_rec_s {
	uint8_t                  ver;
	uint8_t                  rsv[3];
	uint32_t                 rpt_num;
	struct ads_ip_pkt_info_s ip_pkt_rec_info[ADS_MNTN_REC_TX_PKT_MAX_NUM];
};

struct ads_rx_ip_pkt_rec_s {
	uint8_t                  ver;
	uint8_t                  rsv[3];
	uint32_t                 rpt_num;
	struct ads_ip_pkt_info_s ip_pkt_rec_info[ADS_MNTN_REC_RX_PKT_MAX_NUM];
};

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)

struct ads_ltev_ul_info_s {
	uint16_t version;                       /* 版本号 */
	uint8_t  src_addr[3];                   /* 源MAC地址 */
	uint8_t  dst_addr[3];                   /* 目的MAC地址 */
	uint8_t  priority;                      /* 优先级(1~8) */
	uint8_t  pdu_type;                      /* 上层数据包类型，IP(0)或Non-IP(1) */
	uint16_t pdb;                           /* 最大延时预算, 0表示上层未携带 */
	uint16_t traffic_period;                /* 报文发送周期 */
	uint16_t data_len;                      /* 数据长度 */
	unsigned int frag_info;                 /* 分片信息 */
};

struct ads_ltev_dl_info_s {
	uint16_t version;                       /* 版本号 */
	uint8_t  src_addr[3];                   /* 源MAC地址 */
	uint8_t  dst_addr[3];                   /* 目的MAC地址 */
	uint32_t max_data_rate;                 /* CCSA范围 0-1585200, 0xFFFFFFFF */
	uint8_t  priority;                      /* 优先级(1~8) */
	uint8_t  cbr;                           /* 信道忙率 */
	uint16_t frame_num;                     /* 接收空口帧号 */
	uint8_t  subfrm_num;                    /* 接收空口子帧号 */
	uint8_t  rsv;                           /* 接入层头部预留字段 */
	uint16_t datalen;                       /* 数据长度 */
	uint32_t frag_info;                     /* 分片信息 */
};

struct ads_tx_ltev_pkt_rec_s {
	uint8_t                   ver;
	uint8_t                   rsv[3];
	uint32_t                  rpt_num;
	struct ads_ltev_ul_info_s ltev_ul_rec_info[ADS_MNTN_REC_TX_PKT_MAX_NUM];
};

struct ads_rx_ltev_pkt_rec_s {
	uint8_t                    ver;
	uint8_t                    rsv[3];
	uint32_t                   rpt_num;
	struct ads_ltev_dl_info_s  ltev_dl_rec_info[ADS_MNTN_REC_RX_PKT_MAX_NUM];
};
#endif

struct ads_iface_hook_ops_s {
	int (*special_data_hook)(struct sk_buff *skb);
	void (*report_rx_pkt)(struct ads_rx_ip_pkt_rec_s *ads_dl_ipk_rec);
	void (*report_tx_pkt)(struct ads_tx_ip_pkt_rec_s *ads_ul_ipk_rec);
#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
	void (*report_ltev_rx_pkt)(struct ads_rx_ltev_pkt_rec_s *ads_dl_ltev_rec);
	void (*report_ltev_tx_pkt)(struct ads_tx_ltev_pkt_rec_s *ads_ul_ltev_rec);
#endif
};

/*lint -esym(752,ads_iface_create)*/
int ads_iface_create(void);
/*lint -esym(752,ads_iface_bring_up)*/
int ads_iface_bring_up(uint8_t iface_id, struct ads_iface_config_s *config);
/*lint -esym(752,ads_iface_bring_down)*/
int ads_iface_bring_down(uint8_t iface_id, struct ads_iface_config_s *config);
/*lint -esym(752,ads_iface_enable_wakelock)*/
int ads_iface_enable_wakelock(uint32_t timeout);
/*lint -esym(752,ads_iface_register_hook)*/
int ads_iface_register_hook(struct ads_iface_hook_ops_s *hook_ops);

/*lint -esym(752,ads_iface_get_data_stats)*/
struct ads_data_stats_s *ads_iface_get_data_stats(uint8_t iface_id);
/*lint -esym(752,ads_iface_get_espe_stats)*/
struct ads_espe_stats_s *ads_iface_get_espe_stats(void);
/*lint -esym(752,ads_iface_get_dsflow_stats)*/
struct ads_dsflow_stats_s *ads_iface_get_dsflow_stats(void);
/*lint -esym(752,ads_iface_clear_dsflow_stats)*/
void ads_iface_clear_dsflow_stats(void);
void ads_iface_dpl_set_needed_flg(bool needed_flg);
uint8_t ads_iface_get_iface_id(struct rx_cb_map_s *map);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_IFACE_I_H__ */
