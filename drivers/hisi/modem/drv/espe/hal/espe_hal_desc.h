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

#ifndef __SPE_HAL_DESC_H__
#define __SPE_HAL_DESC_H__

#include <linux/types.h>

#ifdef SPEV300_SUPPORT
#include "espe_hal_desc_v300.h"
#else
#include "espe_hal_desc_v200.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ESPE_DESC_IP_PKT_WORD_NUM 16
#define ESPE_DESC_CD_NUM_MAX 1024

// td result bit 0-1
enum spe_td_result {
    TD_RESULT_UPDATA_ONLY = 0,
    TD_RESULT_DISCARD,
    TD_RESULT_NORMAL,
    TD_RESULT_WRAP_OR_LENTH_WRONG,
    TD_RESULT_BOTTOM
};

/* td result bit 6-16 */
enum spe_td_fw {
    TD_FW_MAC_FILTER,     // BIT6
    TD_FW_ETH_TYPE,       // BIT7
    TD_FW_1ST_MACFW,      // BIT8
    TD_FW_IP_HDR_CHECK,   // BIT9
    TD_FW_IP_FILTER,      // BIT10
    TD_FW_TCP_UDP_CHECK,  // BIT11
    TD_FW_HASH_CALC,      // BIT12
    TD_FW_HASH_CACHE,     // BIT13
    TD_FW_HASH_DDR,       // BIT14
    TD_FW_2ND_MACFW,      // BIT15
    TD_FW_COMPLETE,       // BIT16
    TD_FW_BOTTOM
};

// td result bit 17-19
enum spe_td_pkt_type {
    TD_PKT_IPV4_TCP = 0,
    TD_PKT_IPV4_UDP,
    TD_PKT_IPV4_NON_UDP_TCP,
    TD_PKT_IPV6_TCP,
    TD_PKT_IPV6_UDP,
    TD_PKT_IPV6_NON_UDP_TCP,
    TD_PKT_NON_UDP_TCP,
    TD_PKT_TRANS_BEFORE_IP_CHECK,
    TD_PKT_TYPE_BOTTOM
};

// td result bit 20-21
enum spe_td_warp {
    TD_WARP_SUCCESS,
    TD_WARP_PART_DISCARD,
    TD_WARP_ALL_DISCARD,
    TD_WARP_BOTTOM
};

enum spe_td_ncm_unwrap {
    TD_NCM_WRAP_SUCCESS,
    TD_NCM_PORT_DISABLE,
    TD_NCM_TD_LENGTH_TOO_LONG,
    TD_NCM_NTH_FLAG_WRONG,
    TD_NCM_NTH_LENGTH_WRONG,
    TD_NCM_BLOCK_LENGTH_WRONG,
    TD_NCM_NDP_FLAG_WRONG,
    TD_NCM_NDP_LENGTH_WRONG,
    TD_NCM_DATAGRAM_WRONG,
    TD_NCM_ETH_LENGTH_WRONG,
    TD_NCM_TOTAL_DATA_LEN_TOO_LONG,
    TD_NCM_ETH_PKT_TO_LAGE,
    TD_NCM_UNWRAP_BOTTOM
};

// td result bit 22-25
enum spe_td_rndis_unwrap {
    TD_RNDIS_WRAP_SUCCESS,
    TD_RNDIS_PORT_DISABLE,
    TD_RNDIS_TD_LENGTH_TOO_LONG,
    TD_RNDIS_FIRST_MSG_TYPE,
    TD_RNDIS_MSG_LENGTH_OVERSIZE,
    TD_RNDIS_ETH_OVER_PKT_LEN,
    TD_RNDIS_ETH_LENGTH_WRONG,
    TD_RNDIS_WRAP_BOTTOM
};

// td result bit 22-25
enum spe_td_normal_result {
    TD_FINISH_SUCCESS,
    TD_PORT_DISABLE,
    TD_PKT_LEN_ABNORMITY,
    TD_NORMAL_BOTTOM
};

/* struct of espe_td_desc->stream_id_result as spe result  */
struct espe_td_result_s {
    unsigned int td_trans_result : 2;
    unsigned int td_drop_rsn : 4;
    unsigned int td_trans_path : 11;
    unsigned int td_pkt_type : 3;     // message type
    unsigned int td_warp : 2;         // drop ncm like packet reason
    unsigned int td_unwrap : 4;       // packet analytic error type
    unsigned int td_pkt_ext_cnt : 6;  // ncm and other packet extract count
};

/* struct of espe_td_desc->stream_id_result as spe result  */
struct espe_td_ipsec_info_s {
    unsigned int dport_num : 4;
    unsigned int update_pkt : 1;
    unsigned int reserve : 27;
};

struct espe_ext_td_desc {
    struct espe_td_desc std_td_desc;
    unsigned int td_ip_pkt_hdr[ESPE_DESC_IP_PKT_WORD_NUM];
};

// rd bit 0-1
enum spe_rd_result {
    RD_RESULT_UPDATA_ONLY = 0,
    RD_RESULT_DISCARD,
    RD_RESULT_SUCCESS,
    RD_RESULT_WRAP_OR_LENGTH_WRONG,
    RD_RESULT_BOTTOM
};

// rd bit 2-5
enum spe_rd_pkt_drop_rsn {
    RD_DROP_RSN_UNDISCARD = 0,
    RD_DROP_RSN_RD_POINT_NULL = 15,
    RD_DROP_RSN_BOTTOM
};

// rd bit 6-16
enum spe_rd_fw {
    RD_MAC_FILTER,        // BIT6
    RD_ETH_TYPE,          // BIT7
    RD_1ST_MAC_FW,        // BIT8
    RD_IP_HDR_CHECK,      // BIT9
    RD_IP_FILTER,         // BIT10
    RD_TCP_UDP_CHECK,     // BIT11
    RD_HASH_CHECK,        // BIT12
    RD_HASH_CACHE_CHECK,  // BIT13
    RD_HASH_DDR_CHECK,    // BIT14
    RD_2ND_MAC_CHECK,     // BIT15
    RD_TRANS_COMPLETE,    // BIT16
    RD_INDICATE_BOTTOM
};

// rd bit 17-19
enum spe_rd_pkt_type {
    RD_PKT_IPV4_TCP = 0,
    RD_PKT_IPV4_UDP,
    RD_PKT_IPV4_NON_UDP_TCP,
    RD_PKT_IPV6_TCP,
    RD_PKT_IPV6_UDP,
    RD_PKT_IPV6_NON_UDP_TCP,
    RD_PKT_NON_UDP_TCP,
    RD_PKT_TRANS_BEFORE_IP_CHECK,
    RD_PKT_TYPE_BOTTOM
};

// rd bit 20-22
enum spe_rd_finsh_wrap_rsn {
    RD_WRAP_WRONG_FORMAT,
    RD_WRAP_PUSH_EN,
    RD_WRAP_NUM_OVERSIZE,
    RD_WRAP_LENGTH_OVERSIZE,
    RD_WRAP_TIMEOUT,
    RD_WRAP_PORT_DIS,
    RD_WRAP_MUTLI_NCM,
    RD_WRAP_BOTTOM
};

/* struct of espe_td_desc->rd_stemid_info as rd info  */
struct espe_rd_ipsec_info_s {
    unsigned int sport_num : 4;
    unsigned int dport_num : 4;
    unsigned int usb_sg_cnt : 8;
    unsigned int eth_pkt_type : 16;
};

/* struct of espe_rd_desc->rd_result as spe transfer result  */
struct espe_rd_result_s {
    unsigned int rd_trans_result : 2;
    unsigned int rd_drop_rsn : 4;
    unsigned int rd_trans_path : 11;
    unsigned int rd_mess_indicat : 3;  // message type
    unsigned int rd_sg_comp_rsn : 3;
    unsigned int rd_tocpu_rsn : 4;
    unsigned int rd_reserve : 4;      // packet analytic error type
    unsigned int rd_updata_only : 1;  // updata only packet flag
};

struct espe_ext_rd_desc {
    struct espe_rd_desc std_rd_desc;
    unsigned int rd_ip_pkt_hdr[ESPE_DESC_IP_PKT_WORD_NUM];
};

struct espe_ad_desc {
    unsigned long long ad_addr;  // lower 32bit for input packet phy or maa addr;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SPE_HAL_DESC_H__ */
