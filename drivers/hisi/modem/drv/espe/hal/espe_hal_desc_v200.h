/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __SPE_HAL_DESC_V200_H__
#define __SPE_HAL_DESC_V200_H__

#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* td result bit 2-5 */
enum spe_td_ptk_drop_rsn {
    TD_DROP_RSN_TRANS_SUCC = 0,
    TD_DROP_RSN_UNWRAP_ERR,
    TD_DROP_RSN_LENGTH_ERR,
    TD_DROP_RSN_SPORT_DISABLE,
    TD_DROP_RSN_MAC_FILT,
    TD_DROP_RSN_RESERVE0,
    TD_DROP_RSN_TTL_ZERO,
    TD_DROP_RSN_IP_FILT,
    TD_DROP_RSN_UDP_RATE_LIMIT,
    TD_DROP_RSN_MAC_FW_ENTRY_ERR,
    TD_DROP_RSN_DPORT_DISABLE,
    TD_DROP_RSN_RESERVE1,
    TD_DROP_RSN_RESERVE2,
    TD_DROP_RSN_V6_HOP_LIMIT_ZERO,
    TD_DROP_RSN_RD_PTR_NULL,
    TD_DROP_RSN_BOTTOM
};

struct espe_td_desc {
    /* word0 */
    unsigned int td_int_en : 1;  // for ipf: ipf bd finish with/without interrupt
    unsigned int td_mode : 2;    // for ipf, suppose to fix at 1; 0:copy and filter, 1:filter only, 2:copy only
    unsigned int td_reserve0 : 5;
    unsigned int td_fc_head : 4;  // for ipf, filter chain head.
    unsigned int td_irq_en : 1;   // for spe; spe td finish with&without interrupt.
    unsigned int td_push_en : 1;  // for both; send packet now while set 1
    unsigned int td_reserve1 : 1;
    unsigned int td_high_pri_flag : 1;  // for ipf; high priority packet flag
    unsigned int td_pkt_len : 16;       // for spe; packet len

    /* word1-2 packet input header, suppose to point at ip header */
    unsigned int td_inaddr_lower;  // lower 32bit for input packet phy or maa addr;
    unsigned int td_inaddr_upper;  // lower 32bit for input packet phy or maa addr;

    /* word3 */
    unsigned int td_pdu_ssid : 8;  // for ipf; pdu ssid(0-7bit)
    unsigned int td_reserve2 : 8;
    unsigned int td_modem_id : 2;  // for ipf; modem_id(16-17bit)
    unsigned int td_reserve3 : 11;
    unsigned int td_iptype : 3;  // for ipf; iptype(29-31bit)

    /* word4 */
    unsigned int td_bypass_en : 1;
    unsigned int td_bypass_addr : 4;
    unsigned int td_host_ana : 1;      // phone mode packet analysis enable,
    unsigned int td_drop_ack_ind : 1;  // ACK TD effect 0:normal,1:drop pkt,
    unsigned int td_usb_net_id : 8;    // LAN netcard id, set by CPU TD&USB TD,
    unsigned int td_reserve4 : 17;     // reserve

    /* word5 */
    unsigned int td_result;  // stream id spe reseult .function unknow;

    /* word6 */
    unsigned int td_info;

    /* word7 */
    unsigned int td_user_field0;

    /* word8-9 */
    unsigned int td_user_field1;
    unsigned int td_user_field2;
};

// rd bit 23-27
enum spe_rd_send_cpu_rsn {
    RD_CPU_PKT_ERR = 0,  // DL OVER LARGE PKT, DL NON-IP PKT, DL SPECIAL
    RD_CPU_MAC_NON_STICK_V4_V6,
    RD_CPU_1STMAC_IPVER_ERR,
    RD_CPU_1STMAC_DMAC_SMAC_DIFF,
    RD_CPU_1STMAC_MAC_TRANS_FAIL,
    RD_CPU_2SNDMAC_BRG_DMAC_FAIL,
    RD_CPU_L3_CHECK_L3_PROTOCOL_FAIL,
    RD_CPU_L3_CHECK_IPV4_HDR_FAIL,
    RD_CPU_IPV6_HOP_LIMIT_1,
    RD_CPU_IPV4_HDR_LEN_ERR,
    RD_CPU_IPV4_ONE_TTL_LEFT,
    RD_CPU_IPV4_SLICE_PKT,
    RD_CPU_L4_PORT_MATCH,
    RD_CPU_IP_TABLE_MISMATCH,
    RD_CPU_RESERVE,
    RD_CPU_FWSUCC_OR_DISCARD,
    RD_CPU_BOTTOM
};

struct espe_rd_desc {
    /* word0 */
    unsigned int rd_int_en : 1;  // for ipf spe: ipf bd finish with/without interrupt
    unsigned int rd_mode : 2;    // for ipf, suppose to fix at 1; 0:copy and filter, 1:filter only, 2:copy only
    unsigned int rd_reserve0 : 5;
    unsigned int rd_fc_head : 4;  // for ipf, filter chain head.
    unsigned int rd_reserve1 : 3;
    unsigned int rd_high_pri_flag : 1;  // for ipf; high priority packet flag
    unsigned int rd_pkt_len : 16;       // for    spe; packet len

    /* word1-2 */
    unsigned int rd_outaddr_lower;
    unsigned int rd_outaddr_upper;

    /* word3 */
    unsigned int rd_pdu_ssid : 8;  // for ipf; pdu ssid(0-7bit)
    unsigned int rd_ips_id : 8;    // for ipf; mutli-NCM ips id; function know
    unsigned int rd_modem_id : 2;  // for ipf; modem_id(16-17bit)
    unsigned int rd_reserve3 : 6;
    unsigned int rd_l2_hdr_offset : 5;  // offset of mac header to ip header
    unsigned int rd_iptype : 3;         // for ipf; iptype(29-31bit)

    /* word4 spe transfer result */
    unsigned int rd_trans_result : 2;
    unsigned int rd_drop_rsn : 4;
    unsigned int rd_trans_path : 10;
    unsigned int rd_trans_path_finish : 1;
    unsigned int rd_pkt_type : 3;
    unsigned int rd_finish_warp_res : 3;
    unsigned int rd_tocpu_res : 4;
    unsigned int rd_reserve4 : 4;
    unsigned int rd_updata_only : 1;

    /* word5 */
    unsigned int rd_sport : 5;
    unsigned int rd_dport : 5;
    unsigned int rd_pktnum : 6;
    unsigned int rd_ethtype : 16;

    /* word6 */
    unsigned int rd_ipfres_stmid;  // ipf result, stream id

    /* word7 */
    unsigned int rd_user_field0;
    /* word8-9 */
    unsigned int rd_user_field1;
    unsigned int rd_user_field2;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SPE_HAL_DESC_V200_H__ */
