/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
#ifndef __SPE_WAN_H__
#define __SPE_WAN_H__
#include <linux/skbuff.h>
#include <linux/if_ether.h>

#define WAN_MAA_OWN 1U

/* IPF info for SPE BD  */
struct ipf_info_s
{    
    unsigned int pdu_session_id:8;
    unsigned int fc_head:4;
    unsigned int modem_id:2;
    unsigned int higi_pri_flag:1;
    unsigned int parse_en:1;
    unsigned int reserve:16;
};

/* map of skb->CB in wan_tx */
struct wan_info_s
{
    struct ipf_info_s info;
    unsigned int userfield0;
    unsigned int userfield1;
    unsigned int userfield2;
};

union ipf_result_u
{
    struct
    {
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
    } bits;
    unsigned int u32;
};

union packet_info_u
{
    struct
    {
        unsigned int unmapped        :8;
        unsigned int l4_proto           :8;
        unsigned int ip_proto           :4;
        unsigned int l2_hdr_offeset  :5;
        unsigned int reserved          :7;
    } bits;
    unsigned int u32;
};

/* map of skb->CB in wan_rx */
struct rx_cb_map_s
{
    union ipf_result_u ipf_result;
    unsigned int userfield0;
    unsigned int userfield1;
    unsigned int userfield2;
    union packet_info_u packet_info;
    unsigned long long maa_org;
    unsigned long long maa_l2;
};

struct wan_dev_info_s
{
    struct net_device* dev;
    struct wan_info_s v4;
    struct wan_info_s v6;
};

enum wan_waterlevel_e
{
    HIGH_ALERT,
    RESUME_OK,
};

/* wan callbacks */
struct wan_callback_s
{
    int (*wan_rx)(struct sk_buff* skb);
    int (*lev_report)(enum wan_waterlevel_e);
    void (*rx_complete)(void);
};

static inline unsigned int wan_get_devid(struct sk_buff *skb) {
    struct rx_cb_map_s *cb_map =  (struct rx_cb_map_s *)&skb->cb;
    return cb_map->userfield0 & 0xffff;
}

int mdrv_wan_tx(struct sk_buff* skb, unsigned int maa_flag);
int mdrv_wan_callback_register(struct wan_callback_s* cb);
int mdrv_wan_dev_info_register(struct wan_dev_info_s* info);

void mdrv_wan_del_direct_fw_dev(unsigned int devid);
void mdrv_wan_set_ipfmac(struct ethhdr *mac_addr);
#endif
