/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: eipf driver
 * Author: shennaiyu
 * Create: 2020-10-12
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

/**
 *  @brief   5000 eipf acore对外头文件，5010融合架构不对外提供。
 *  @file    mdrv_eipf.h
 *  @author  zhangsanmao
 *  @version v1.0
 *  @date    2019.12.17
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.12.17|增加说明：该头文件5010融合架构不对外提供</li></ul>
 *  @since
 */

#ifndef __MDRV_EIPF_H__
#define __MDRV_EIPF_H__

enum e_ipf_mode {
    IPF_MODE_FILTER_AND_DMA = 0,
    IPF_MODE_FILTER_ONLY,
    IPF_MODE_DMA_ONLY,
};

/* attribute of basic descriptor */
union ipf_desc_attr_u {
    struct {
        unsigned int int_en : 1;
        unsigned int mode : 2;
        unsigned int cd_en : 1;
        unsigned int dl_data_sec_n : 1;
        unsigned int dl_rqi : 1;
        unsigned int reserved0 : 2;
        unsigned int fc_head : 4;
        unsigned int irq_en : 1;
        unsigned int push_en : 1;
        unsigned int dl_qos_flowid7 : 1;
        unsigned int high_pri_flag : 1;
        unsigned int pkt_len : 16;
    } bits;
    unsigned int u32;
};

/* IDs region of basic descriptor */
union ipf_desc_id_u {
    struct {
        unsigned int pdu_session_id : 8;
        unsigned int drb_id : 8;
        unsigned int modem_id : 2;
        unsigned int rat_id : 3;
        unsigned int refl_fc_head : 4;
        unsigned int dl_qos_flow_id : 7;
    } bits;
    unsigned int u32;
};

enum pf_type_e {
    PF_NO_FRAG,
    PF_FIRST_FRAG,
    PF_LAST_FRAG,
    PF_MORE_FRAG,
};
/* filter result of result descriptor */
union ipf_rd_result_u {
    struct {
        unsigned int bid_qosid : 8;
        unsigned int high_pri_result : 1;
        unsigned int ul_rdq_sel : 1;
        unsigned int ul_special_flag : 1;
        unsigned int reserved : 11;
        unsigned int pf_type : 2;
        unsigned int ff_type : 1;
        unsigned int ip_type : 1;
        unsigned int version_err : 1;
        unsigned int head_len_err : 1;
        unsigned int bd_pkt_noeq : 1;
        unsigned int pkt_parse_err : 1;
        unsigned int bd_cd_noeq : 1;
        unsigned int pkt_len_err : 1;
    } err;
    unsigned int u32;
};

/* IPF downlink chain descriptor */
struct ipf_cd_s {
    unsigned int input_pointer_l;
    unsigned int input_pointer_h : 8;
    unsigned int blk_len : 16;
    unsigned int cd_last : 1;
    unsigned int reserved : 7;
};

/* ip protocol of packet */
union ipf_pkt_protocol_u {
    struct {
        unsigned int ipv4_id : 16;
        unsigned int protocol : 8;
        unsigned int version : 4;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
};

/* IPF downlink basic descriptor */
struct ipf_dlbd_s {
    union ipf_desc_attr_u attr;
    unsigned int input_pointer_l;
    unsigned int input_pointer_h;
    union ipf_desc_id_u id;
    unsigned int reserved;
    unsigned int user_field0;
    unsigned int user_field1;
    unsigned int user_field2;
};

enum rat_id_e {
    RAT_2G,
    RAT_3G,
    RAT_4G,
    RAT_1X,
    RAT_5G,
};
/* IPF uplink result descriptor */
struct ipf_ulrd_s {
    union ipf_desc_attr_u attr;
    unsigned int input_pointer_l;
    unsigned int input_pointer_h;
    union ipf_desc_id_u id;
    unsigned int reserved;
    union ipf_rd_result_u result;
    union ipf_pkt_protocol_u protocol;
    unsigned int tcp_udp_field0;
    unsigned int tcp_udp_field1;
    unsigned int user_field0;
    unsigned int user_field1;
    unsigned int user_field2;
};

/* IPF ipv4 RNR result descriptor */
struct ipf_rnrd_s {
    unsigned int protocol : 8;
    unsigned int version : 4;
    unsigned int operation : 2;
    unsigned int type_valid : 1;
    unsigned int code_valid : 1;
    unsigned int type : 8;
    unsigned int code : 8;
    unsigned int qos_flow_id : 8;
    unsigned int traffic_class : 8;
    unsigned int pdu_session_id : 8;
    unsigned int drb_id : 8;
    union {
        struct {
            unsigned int source_addr;
            unsigned int reserved0;
            unsigned int reserved1;
            unsigned int reserved2;
            unsigned int dst_addr;
            unsigned int reserved3;
            unsigned int reserved4;
            unsigned int reserved5;
        } v4;

        /* IPF ipv4 RNR result descriptor */
        struct {
            unsigned int source_addr0;
            unsigned int source_addr1;
            unsigned int source_addr2;
            unsigned int source_addr3;
            unsigned int dst_addr0;
            unsigned int dst_addr1;
            unsigned int dst_addr2;
            unsigned int dst_addr3;
        } v6;
    } addr;
    unsigned int source_port : 16;
    unsigned int dst_port : 16;
    unsigned int filter_id : 28;
    unsigned int spi_valid : 1;
    unsigned int reserved6 : 3;

    unsigned int spi;
    unsigned int flow_label : 20;
    unsigned int reserved7 : 12;
};

struct ipf_qos_flow_id_to_drb_map_s {
    unsigned int qos_flow_id : 16;
    unsigned int pdu_session_id : 8;
    unsigned int drb_id : 8;
};

struct ipf_sessionid_to_drb_map_s {
    unsigned char drb_id;
    unsigned char pdu_session_id;
};

struct ipf_rabid_to_drb_map_s {
    unsigned char bead_id : 6;
    unsigned char reserved : 2;
    unsigned char drb_id;
};

struct ipf_filter_map_s {
    unsigned int fltn_local_addr0;
    unsigned int fltn_local_addr1;
    unsigned int fltn_local_addr2;
    unsigned int fltn_local_addr3;
    unsigned int fltn_remote_addr0;
    unsigned int fltn_remote_addr1;
    unsigned int fltn_remote_addr2;
    unsigned int fltn_remote_addr3;
    unsigned int fltn_remote_addr0_mask;
    unsigned int fltn_remote_addr1_mask;
    unsigned int fltn_remote_addr2_mask;
    unsigned int fltn_remote_addr3_mask;

    union {
        struct {
            unsigned int fltn_local_port_low : 16;  /* [15..0]  */
            unsigned int fltn_local_port_high : 16; /* [31..16]  */
        } bits;
        unsigned int u32;
    } fltn_local_port;

    union {
        struct {
            unsigned int fltn_remote_port_low : 16;  /* [15..0]  */
            unsigned int fltn_remote_port_high : 16; /* [31..16]  */
        } bits;
        unsigned int u32;
    } fltn_remote_port;

    union {
        struct {
            unsigned int fltn_traffic_class : 8;      /* [7..0]  */
            unsigned int fltn_traffic_class_mask : 8; /* [15..8]  */
            unsigned int fltn_filter_id0 : 16;        /* [31..16]  */
        } bits;
        unsigned int u32;
    } fltn_traffic_class;

    union {
        struct {
            unsigned int fltn_local_addr_mask : 7;     /* [6..0]  */
            unsigned int fltn_ul_rdq_sel : 1;          /* [7]  */
            unsigned int fltn_pdu_session_id_mask : 8; /* [15..8]  */
            unsigned int fltn_rat_id : 3;              /* [18..16]  */
            unsigned int fltn_match_pri : 1;           /* [19]  */
            unsigned int reserved_0 : 12;              /* [31..20]  */
        } bits;
        unsigned int u32;
    } fltn_ladd_mask;

    union {
        struct {
            unsigned int fltn_next_header : 8; /* [7..0]  */
            unsigned int reserved_0 : 24;      /* [31..8]  */
        } bits;
        unsigned int u32;
    } fltn_next_header;

    union {
        struct {
            unsigned int fltn_flow_lable : 20; /* [19..0]  */
            unsigned int fltn_filter_id1 : 12; /* [31..20]  */
        } bits;
        unsigned int u32;
    } fltn_flow_label;

    union {
        struct {
            unsigned int fltn_type : 8;           /* [7..0]  */
            unsigned int fltn_code : 8;           /* [15..8]  */
            unsigned int fltn_qos_flow_id : 8;    /* [23..16]  */
            unsigned int fltn_pdu_session_id : 8; /* [31..24]  */
        } bits;
        unsigned int u32;
    } fltn_icmp;

    union {
        struct {
            unsigned int fltn_next_index : 10; /* [9..0]  */
            unsigned int reserved_0 : 6;       /* [15..10]  */
            unsigned int fltn_pri : 9;         /* [24..16]  */
            unsigned int reserved_1 : 7;       /* [31..25]  */
        } bits;
        unsigned int u32;
    } fltn_chain;

    unsigned int fltn_spi;

    union {
        struct {
            unsigned int fltn_en : 1;          /* [0]  */
            unsigned int fltn_type : 1;        /* [1]  */
            unsigned int reserved_0 : 2;       /* [3..2]  */
            unsigned int fltn_spi_en : 1;      /* [4]  */
            unsigned int fltn_code_en : 1;     /* [5]  */
            unsigned int fltn_type_en : 1;     /* [6]  */
            unsigned int fltn_fl_en : 1;       /* [7]  */
            unsigned int fltn_nh_en : 1;       /* [8]  */
            unsigned int fltn_tos_en : 1;      /* [9]  */
            unsigned int fltn_rport_en : 1;    /* [10]  */
            unsigned int fltn_lport_en : 1;    /* [11]  */
            unsigned int fltn_raddr_en : 1;    /* [12]  */
            unsigned int fltn_laddr_en : 1;    /* [13]  */
            unsigned int fltn_pdu_en : 1;      /* [14]  */
            unsigned int flt_special_flag : 1; /* [15]  */
            unsigned int reserved_1 : 16;      /* [31..16]  */
        } bits;
        unsigned int u32;
    } fltn_rule_ctrl;
};

enum ipf_fchead_e {
    EIPF_MODEM0_ULFC = 0,
    EIPF_MODEM1_ULFC,
    EIPF_MODEM2_ULFC,
    EIPF_MODEM0_DLFC,
    EIPF_MODEM1_DLFC,
    EIPF_MODEM2_DLFC,
    EIPF_LTEV_DLFC,
    EIPF_LTEV_ULFC,
    EIPF_ETH_ULFC,
    EIPF_ETH_DLFC,
    EIPF_FCHEAD_BOTTOM,
};

struct ipf_chn_size_info_s {
    unsigned int bd_total_size;
    unsigned int rd_total_size;
    unsigned int cd_total_size;
    unsigned int rnrd_size;
};

struct ipf_chn_base_info_s {
    void *bd_base;
    void *rd_base;
};

enum ipf_chn_e {
    CHN_LR,
    CHN_NR,
    CHN_BOTTOM,
};

struct ipf_ops_s {
    int (*rx_complete_cb)(void);
    int (*rnrd_complete_cb)(void);
};

int mdrv_ipf_accum_period_set(unsigned int period);
int mdrv_ipf_set_filters(enum ipf_fchead_e fchead, struct ipf_filter_map_s *base, unsigned int num);
int mdrv_ipf_reg_ops(struct ipf_ops_s *ops);
int mdrv_ipf_get_cd_queue(enum ipf_chn_e chn, unsigned int cn_num, unsigned int *base_h, unsigned int *base_l);
int mdrv_ipf_get_bd_queue(enum ipf_chn_e chn, unsigned int *rptr, unsigned int *wptr);
int mdrv_ipf_set_bd_queue(enum ipf_chn_e chn, unsigned int wptr);
int mdrv_ipf_open_chn(enum ipf_chn_e chn, struct ipf_chn_size_info_s *size, struct ipf_chn_base_info_s *base);
int mdrv_ipf_get_rd(enum ipf_chn_e chn, struct ipf_ulrd_s *rd, unsigned int *num);
int mdrv_ipf_get_rnrd(enum ipf_chn_e chn, struct ipf_rnrd_s *rnrd, unsigned int *num);
int mdrv_ipf_config_flowid_map(struct ipf_qos_flow_id_to_drb_map_s *map, unsigned int num);
int mdrv_ipf_config_sessionid_map(struct ipf_sessionid_to_drb_map_s *map, unsigned int num);

#endif
