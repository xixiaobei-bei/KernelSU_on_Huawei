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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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

#ifndef __BSP_ESPE_H__
#define __BSP_ESPE_H__

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/netfilter/nf_conntrack_common.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#include <product_config.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

#define SPE_PORT_NUM   (16)

#define ESPE_RD_UPDATEONLY BIT(0)
#define ESPE_RD_INTR_DONE BIT(1)

#define ESPE_TD_INTR_DONE BIT(0)
#define ESPE_TD_PUSH_USB BIT(1)
#define ESPE_TD_KICK_PKT BIT(2)

#define ESPE_GETHER_RD_DIV 896
#define ESPE_DEFAULT_CPU_PORT 0
#define ESPE_DEFAULT_IPF_PORT 1

#define ESPE_ACCELETABLE_PKT_FLAG 9
#define ESPE_BYPASS_ENABLE_FLAG 0x5F5F5F5F
#define VCOM_IOCTL_ARP_FLAG 0x1E1E1E1E

enum spe_enc_type {
    SPE_ENC_NONE,
    SPE_ENC_NCM_NTB16,
    SPE_ENC_NCM_NTB32,
    SPE_ENC_RNDIS,
    SPE_ENC_WIFI,
    SPE_ENC_IPF,
    SPE_ENC_CPU,
    SPE_ENC_ACK,
    SPE_ENC_DIRECT_FW_HP,
    SPE_ENC_DIRECT_FW_MP,
    SPE_ENC_DIRECT_FW_PE,
    SPE_ENC_DIRECT_FW_NCM_NTB16,
    SPE_ENC_DIRECT_FW_NCM_NTB32,
    SPE_ENC_DIRECT_FW_RNDIS,
    SPE_ENC_DIRECT_FW_ECM,
    SPE_ENC_ECM,
    SPE_ENC_BOTTOM
};

enum spe_rsv_port_id {
    SPE_ACORE_PPP_NDIS_CTRL_ID = 2,
    SPE_ACORE_LAN_CTRL_ID = 3,
    SPE_CCORE_PPP_NDIS_CTRL_ID = 12,
    SPE_CCORE_PORT_NO_RSV_MAX = 13
};

struct iport_ipf_addr {
    dma_addr_t ulrd_wptr_addr;  //  refer to IPF ULBD WPTR
    dma_addr_t dltd_rptr_vir_addr;  //  refer to IPF DLRD RPTR
    dma_addr_t dltd_rptr_phy_addr;  //  refer to IPF DLRD RPTR
};

struct iport_spe_addr {
    dma_addr_t dltd_base_addr;  //  refer to IPF DLRD BASE
    dma_addr_t ulrd_base_addr;  //  refer to IPF ULBD BASE
    void* dltd_base_addr_v;  //  refer to IPF DLRD BASE
    void* ulrd_base_addr_v;  //  refer to IPF ULBD BASE
    dma_addr_t ulrd_rptr_addr;  //  refer to IPF ULBD RPTR
    dma_addr_t dltd_wptr_addr;  //  refer to IPF DLRD WPTR
    void* ulrd_rptr_addr_v;  //  refer to IPF ULBD RPTR
    void* dltd_wptr_addr_v;  //  refer to IPF DLRD WPTR
};

struct espe_ops {
    void (* espe_finish_rd)(struct sk_buff *skb, unsigned int len, void* priv, unsigned int flag);
    void (* espe_finish_rd_maa)(unsigned long long maa_orig, unsigned int len, unsigned int l2_hdr_offset, void* priv);
    void (* espe_finish_td)(void *param, void *priv);
    void (* espe_netif_rx)(struct sk_buff *skb, void *priv);
    void (* espe_complete_rd)(void *priv);

    struct wan_dev_info_s* (*get_wan_info)(struct net_device* dev);
};

struct espe_port_comm_attr {
    enum spe_enc_type enc_type;             /* encap type */
    unsigned int td_depth;
    unsigned int rd_depth;
    unsigned int padding_enable;
    unsigned int bypassport;
    unsigned int bypassport_en;
    struct espe_ops ops;
    struct net_device *net;
    void *priv;
    unsigned int td_copy_en;               /* add by B5010 */
    unsigned int stick_en;
    unsigned int rsv_port_id;
};

struct espe_usbport_attr {
    enum spe_enc_type enc_type;             /* encap type */
    unsigned int gether_max_size;
    unsigned int gether_max_pkt_cnt;
    unsigned int gether_timeout;
    unsigned int gether_align_parameter;
    struct net_device *net;
};

struct espe_ipfport_attr {
    struct iport_ipf_addr ipf_addr;
    struct iport_spe_addr spe_addr;
    unsigned int ext_desc_en;
    void (*check_ipf_bd)(void);
};

struct espe_pkt_om_info {
    unsigned int spe_maa_addr_lower;
    unsigned int spe_maa_addr_higher;

    unsigned int spe_portno:4;    // 0-15
    unsigned int spe_dir:1;    // 0=td 1=rd;
    unsigned int spe_pkt_type:3;
    unsigned int spe_reserved:8;
    unsigned int l3_id:16;    // ip id

    unsigned int l4_seq;
    unsigned int time_stamp;
};

struct espe_task_time {
    unsigned int int_time;
    unsigned int task_begin_time;
    unsigned int task_end_time;
};

struct espe_rd_to_cpu_rsn {
    unsigned int pkt_err;
    unsigned int mac_non_stick_v4_v6;
    unsigned int first_mac_ipver_err;
    unsigned int first_mac_dmac_smac_diff;
    unsigned int first_mac_mac_trans_fail;
    unsigned int snd_mac_brg_dmac_fail;
    unsigned int l3_check_l3_protocol_fail;
    unsigned int l3_check_ipv4_hdr_fail;
    unsigned int ipv6_hop_limit_1;
    unsigned int ipv4_hdr_len_err;
    unsigned int ipv4_one_llt_left;
    unsigned int ipv4_slice_pkt;
    unsigned int l4_port_match;
    unsigned int ip_table_mismatch;
    unsigned int stick_arp_pkt;
    unsigned int fwsucc_or_discard;
    unsigned int ecm2usb_bycpu;
};

struct espe_port_om_info {
    unsigned int rd_finished;
    unsigned int rd_finished_bytes;
    unsigned int rd_finsh_pkt;
    unsigned int rd_droped;
    unsigned int rd_sended;
    unsigned int rd_full;
    unsigned int rd_larger;
    unsigned int td_config;
    unsigned int td_full;
    unsigned int td_complete;
    unsigned int td_fail_drop;
    unsigned int td_comp;
};


struct espe_cpu_port_om_info {
    unsigned int updonly_comp;
    unsigned int rd_num;
    unsigned int rd_udp_drop;
    unsigned int rd_wan;
    unsigned int rd_wan_fail;
    unsigned int rd_nic;
    unsigned int rd_nic_fail;
    unsigned int rd_netif;
    unsigned int rd_netif_succ;
    unsigned int rd_netif_fail;
};

struct espe_om_info {
    struct espe_cpu_port_om_info cpu;
    struct espe_port_om_info port[SPE_PORT_NUM];
    struct espe_rd_to_cpu_rsn to_cpu_rsn;
    struct espe_task_time task_time;
};

struct espe_direct_fw_entry_ctx {
    unsigned int devid:16;
    unsigned int skb_cb_info:1;
    unsigned int add_wan_info:1;
    unsigned int maped:1;
    unsigned int single_check:1;
    unsigned int l2_hdr_len:5;
    unsigned int net_id:2;
    unsigned int reserved:6;
};


#if (defined(CONFIG_BALONG_ESPE))
// spe_port.c
int bsp_espe_alloc_port(struct espe_port_comm_attr *attr);
int bsp_espe_free_port(int portno);
int bsp_espe_set_portmac(int portno, const char* mac_addr);
int bsp_espe_set_usbproperty(struct espe_usbport_attr *usb_attr);
int bsp_espe_set_ipfproperty(struct espe_ipfport_attr *ipf_attr);
int bsp_espe_enable_port(int portno);
int bsp_espe_disable_port(int portno);
void bsp_espe_set_ipfmac(struct ethhdr *mac_addr);
void bsp_espe_transfer_pause(void);
void bsp_espe_transfer_restart(void);

int bsp_espe_set_ipfw_entry(int portno, struct sk_buff *skb);
int bsp_espe_update_ipfw_entry(struct wan_dev_info_s* wan_info);

int bsp_espe_config_td(int portno, unsigned long long maa_l2addr,
    unsigned int len, void *param, unsigned int espe_flags);
struct sk_buff *bsp_espe_resolve_skb(unsigned long long maa_l2addr,
    unsigned int len, unsigned int flags);
int bsp_espe_update_net(unsigned int portno, struct net_device *ndev, unsigned int alloced);

#else

static inline int bsp_espe_alloc_port(struct espe_port_comm_attr *attr){return 0;}
static inline int bsp_espe_free_port(int portno){return 0;}
static inline int bsp_espe_set_portmac(int portno, const char* mac_addr){return 0;}
static inline int bsp_espe_set_usbproperty(struct espe_usbport_attr *usb_attr){return 0;}
static inline int bsp_espe_set_ipfproperty(struct espe_ipfport_attr *ipf_attr){return 0;}
static inline int bsp_espe_enable_port(int portno){return 0;}
static inline int bsp_espe_disable_port(int portno){return 0;}
static inline void bsp_espe_set_ipfmac(struct ethhdr *mac_addr){return;}
static inline void bsp_espe_transfer_pause(void){return;}
static inline void bsp_espe_transfer_restart(void){return;}


static inline int bsp_espe_set_ipfw_entry(int portno, struct sk_buff *skb){return 0;}
static inline int bsp_espe_update_ipfw_entry(struct wan_dev_info_s* wan_info){return 0;}


static inline int bsp_espe_config_td(int portno, unsigned long long maa_l2addr,
    unsigned int len, void *param, unsigned int espe_flags){return 0;}
static inline struct sk_buff *bsp_espe_resolve_skb(unsigned long long maa_l2addr,
    unsigned int len, unsigned int flags){return 0;}
static inline int bsp_espe_update_net(unsigned int portno, struct net_device *ndev){return 0;}


#endif /*CONFIG_BALONG_ESPE*/

#if defined(CONFIG_ESPE_DIRECT_FW)

void bsp_espe_add_direct_fw_entry(struct sk_buff *skb, struct espe_direct_fw_entry_ctx *ctx);
void bsp_espe_del_direct_fw_dev(unsigned int devid);

#else

static inline void bsp_espe_add_direct_fw_entry(struct sk_buff *skb,  struct espe_direct_fw_entry_ctx *ctx){return;}
static inline void bsp_espe_del_direct_fw_dev(unsigned int devid){return;}

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__SPE_INTERFACE_H__*/

