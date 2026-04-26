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

#include <linux/if_vlan.h>
#include <securec.h>

#include <mdrv_wan.h>
#include <bsp_maa.h>
#include <bsp_espe.h>
#include <bsp_trans_report.h>
#include "espe_desc.h"
#include "espe_port.h"
#include "espe_dbg.h"
#include "func/espe_direct_fw.h"
#include <asm/barrier.h>


#include <linux/timer.h>


#define ESPE_RD_FLAG_MAGIC 0x53504500
#define ESPE_RD_AVG_GET_TIMES 2
#define ESPE_RD_FLAG_SPORT_MASK 0xf0
#define ESPE_RD_FLAG_MASK 0xffffff00

/* desc number occupied by hardware */
static inline int spe_busy_td_num(struct spe *spe, int portno)
{
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    ret = (ctrl->td_free >= ctrl->td_busy) ? (ctrl->td_free - ctrl->td_busy)
                                            : (ctrl->td_depth - ctrl->td_busy + ctrl->td_free);
    return ret;
}

/* desc number can be used by software */
static inline int spe_free_td_num(struct spe *spe, int portno)
{
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    /* 1 for distinguish full and empty */
    ret = ctrl->td_depth - 1 - spe_busy_td_num(spe, portno);
    return ret;
}

static void espe_td_result_record(struct espe_td_result_s *td_result, struct spe_port_stat *stat)
{
    struct desc_result_s *result = &stat->result;

    result->td_result[td_result->td_trans_result]++;            // td result bit 0-1
    result->td_ptk_drop_rsn[td_result->td_drop_rsn]++;          // td result bit 2-5
    result->td_pkt_fw_path[__fls(td_result->td_trans_path)]++;  // td result bit 6-16
    result->td_pkt_type[td_result->td_pkt_type]++;              // td result bit 17-19
    result->td_warp[td_result->td_warp]++;                      // td result bit 20-21
    result->td_unwrap[td_result->td_unwrap]++;                  // td result bit 22-25

    return;
}

void espe_rd_rewind(unsigned int portno)
{
    struct spe *spe = &g_espe;

    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    unsigned long flags;
    unsigned int rd_wptr;
    unsigned int rd_rptr;

    spin_lock_irqsave(&ctrl->rd_lock, flags);

    /* get rdq ptr, include rd and wr ptr */
    rd_wptr = spe_readl(spe->regs, SPE_RDQX_WPTR(portno));
    rd_rptr = spe_readl(spe->regs, SPE_RDQX_RPTR(portno));
    if (rd_wptr == rd_rptr) { /* no more rd, don't need rewind */
        spin_unlock_irqrestore(&ctrl->rd_lock, flags);
        return;
    }

    rd_wptr = rd_rptr;
    spe_writel(spe->regs, SPE_RDQX_WPTR(portno), rd_wptr);

    spin_unlock_irqrestore(&ctrl->rd_lock, flags);
    return;
}

static void espe_free_maa_mem(struct spe *spe, unsigned int portno, unsigned long long maa_addr)
{
    struct sk_buff *skb = NULL;
    unsigned long long maa_org_addr = 0;

    skb = bsp_maa_to_skb(maa_addr, &maa_org_addr);
    if (unlikely((skb == NULL) || (maa_org_addr == 0))) {
        SPE_ERR("portno %u td free maa 0x%llx fail \n", portno, maa_addr);
        espe_bug(spe);
        return;
    }
    bsp_maa_free(maa_org_addr);

    return;
}
static inline void espe_update_ipf_rptr(struct spe *spe, unsigned int td_rptr)
{
    spe_writel(spe->ipfport.ipf_rd_rptr_addr, 0, td_rptr);
    return;
}

static inline void espe_td_free_mem_v300(struct spe *spe, unsigned int portno, unsigned long long maa_addr,
                                         unsigned int td_cd_en, struct sk_buff *skb)
{
    if (portno == spe->cpuport.portno && td_cd_en == 1) {
        dev_kfree_skb_any(skb);
    } else if (portno != spe->usbport.portno && portno != spe->ipfport.portno) {
        espe_free_maa_mem(spe, portno, maa_addr);
    }

    return;
}

static inline void espe_td_free_mem_v200(struct spe *spe, unsigned int portno, unsigned long long maa_addr)
{
    if (portno != spe->usbport.portno) {
        espe_free_maa_mem(spe, portno, maa_addr);
    }
    return;
}

/*
 * spev200:software check td fail && push rptr to ipf
 *         if td fail, do not produce rd desc.need free mem at td desc.
 *         for ipf port, this operation is completed before pushing the pointer.
 * spev300, Because the hardware pushes the pointer to ipf, the software only handles td failures except ipf port
 *          if ipf port td fail, produce rd desc.need free mem at cpu rd desc.
 *          if other port,td fail, do not produce rd desc.need free mem at other port desc.
 */
void __espe_process_td_desc(struct spe *spe, unsigned int portno, unsigned int td_rptr)
{
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    struct spe_port_stat *stat = &spe->ports[portno].stat;
    struct espe_td_desc *td_base = (struct espe_td_desc *)ctrl->td_addr;
    struct espe_td_desc *td_cur = NULL;
    struct espe_td_result_s *td_result = NULL;
    unsigned int idx;
    unsigned long long maa_addr;
    unsigned int td_result_u32;

    /* recycle td from td_busy to tdq rptr */
    for (idx = ctrl->td_busy; idx != td_rptr;) {
        td_cur = &td_base[idx];
        td_result_u32 = td_cur->td_result;
        td_result = (struct espe_td_result_s *)&td_result_u32;

        if (spe->dbg_level & SPE_DBG_TD_RESULT) {
            espe_td_result_record(td_result, stat);
        }
        if (unlikely(td_result->td_trans_result == TD_RESULT_DISCARD ||
                     td_result->td_trans_result == TD_RESULT_WRAP_OR_LENTH_WRONG)) {
            maa_addr = (unsigned long long)td_cur->td_inaddr_lower +
                       (((unsigned long long)td_cur->td_inaddr_upper) << 32);  /* left shit 32 bit */
            espe_td_free_mem_v200(spe, portno, maa_addr);
            stat->td_desc_fail_drop++;
        } else {
            stat->td_pkt_complete += td_result->td_pkt_ext_cnt;
            stat->td_desc_complete++;
        }

        if (unlikely(ctrl->ops.espe_finish_td)) {
            ctrl->ops.espe_finish_td(ctrl->td_param[idx], ctrl->port_priv);
        }
        idx = ((++idx) < ctrl->td_depth) ? idx : 0;
    }
    ctrl->td_busy = idx;
    if (spe->spe_version == ESPE_VER_V200 && portno == spe->ipfport.portno) {
        espe_update_ipf_rptr(spe, td_rptr);
    }
    return;
}

void espe_process_td_desc(unsigned int portno)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    struct spe_port_stat *stat = &spe->ports[portno].stat;
    unsigned int td_rptr;
    unsigned long flag;

    spin_lock_irqsave(&ctrl->td_lock, flag);
    /* get tdq rptr, */
    td_rptr = spe_readl(spe->regs, SPE_TDQX_RPTR(portno));
    if (ctrl->td_busy == td_rptr) {
        spin_unlock_irqrestore(&ctrl->td_lock, flag);
        return;
    }
    stat->td_finsh_intr_complete++;
    __espe_process_td_desc(spe, portno, td_rptr);
    spin_unlock_irqrestore(&ctrl->td_lock, flag);

    return;
}

/* this fun add by B5010 */
void __wan_spe_td_cd_pkt_proc(struct sk_buff *skb, unsigned int cd_num, struct spe_port_ctrl *port_ctrl)
{
    return;
}

void wan_spe_td_cd_pkt_proc(void *param, int portno, struct wan_info_s *wan_info, struct spe_port_ctrl *ctrl)
{
#ifdef CONFIG_ESPE_PHONE_SOC
    if (portno == ESPE_DEFAULT_CPU_PORT && skb_shinfo(param)->nr_frags > 0 &&
        wan_info->userfield0 != VCOM_IOCTL_ARP_FLAG) {
        __wan_spe_td_cd_pkt_proc(param, skb_shinfo(param)->nr_frags, ctrl);
    }
#endif
}

void espe_td_config_non_cpuport_desc(struct spe *spe, struct spe_port_ctrl *port_ctrl, struct espe_td_desc *desc,
                                     void *param, unsigned int flags)
{
    struct rx_cb_map_s *cb_info = NULL;

    cb_info = (struct rx_cb_map_s *)(((struct sk_buff *)param)->cb);
    /* word 0 */
    desc->td_fc_head = 0;
    desc->td_push_en = !!(flags & ESPE_TD_PUSH_USB);
    desc->td_high_pri_flag = 1;  // for ipf; high priority packet flag

    /* word4 */
    desc->td_bypass_en = !!(port_ctrl->bypassport_en);
    desc->td_bypass_addr = port_ctrl->bypassport;

    if (spe->usbport.portno == port_ctrl->portno) {
        desc->td_usb_net_id = cb_info->packet_info.bits.net_id;
    } else if (port_ctrl->portno == SPE_ACORE_PPP_NDIS_CTRL_ID) {
        /* word7-9 */
        desc->td_user_field0 = cb_info->userfield0;
        if (cb_info->userfield0 == VCOM_IOCTL_PPP_DATA) {
            desc->td_user_field1 = cb_info->userfield1;
        }
    } else if (port_ctrl->portno == SPE_ACORE_LAN_CTRL_ID) {
        desc->td_user_field1 = cb_info->userfield1;
        desc->td_user_field2 = virt_to_phys(((struct sk_buff *)param)->dev);
    } else {
        desc->td_user_field1 = cb_info->userfield1;
    }
    return;
}

void espe_td_config_cpuport_desc(struct spe *spe, struct espe_td_desc *desc, void *param)
{
    struct wan_info_s *wan_info = NULL;

    wan_info = (struct wan_info_s *)(((struct sk_buff *)param)->cb);

    /* word0 */
    desc->td_fc_head = wan_info->info.fc_head;
    desc->td_high_pri_flag = wan_info->info.higi_pri_flag;
    // ltv copy fun, set td mode by userfield1
    if (((TD_MODE_INDEX(wan_info->userfield1) & 0x3) != 0) && ((TD_MODE_INDEX(wan_info->userfield1) & 0x3) != 1)) {
        desc->td_mode = TD_MODE_INDEX(wan_info->userfield1) & 0x3;
    }

    /* word3 */
    desc->td_pdu_ssid = wan_info->info.pdu_session_id;
    desc->td_modem_id = wan_info->info.modem_id;
    desc->td_iptype = 0;

    /* word4 */
    desc->td_bypass_en = 1;
    desc->td_bypass_addr = spe->ipfport.portno;
    desc->td_host_ana = wan_info->info.parse_en;

    /* word7-9 */
    desc->td_user_field0 = wan_info->userfield0;
    desc->td_user_field1 = wan_info->userfield1;
    desc->td_user_field2 = wan_info->userfield2;

    if (wan_info->userfield0 == VCOM_IOCTL_ARP_FLAG) {
        desc->td_bypass_addr = SPE_CCORE_PPP_NDIS_CTRL_ID;
    }
    return;
}
void espe_td_config_one(struct spe_port_ctrl *port_ctrl, unsigned long long maa_l2_addr, unsigned int len, void *param,
                        unsigned int flags)
{
    struct spe *spe = &g_espe;
    struct espe_td_desc *cur_desc = (struct espe_td_desc *)port_ctrl->td_addr;
    struct espe_td_desc *desc = port_ctrl->axi_td;
    int ret;

    cur_desc = cur_desc + port_ctrl->td_free;

    ret = memset_s(desc, sizeof(*desc), 0, sizeof(struct espe_td_desc));
    if (unlikely(ret)) {
        SPE_ERR("memset_s fail.\n");
    }

    // word 0
    desc->td_irq_en = 1;
#ifdef IPF_UL_DMA
    desc->td_mode = 0;
#else
    desc->td_mode = 1;
#endif
    desc->td_pkt_len = len;
    desc->td_int_en = 1;
    desc->td_push_en = flags & ESPE_TD_KICK_PKT;

/* add by B5010 */
    /* word1-2 packet input header, suppose to point at ip header */
    desc->td_inaddr_lower = lower_32_bits(maa_l2_addr);
    desc->td_inaddr_upper = upper_32_bits(maa_l2_addr);

    /* word4 */
    desc->td_usb_net_id = 0;

    if (spe->cpuport.portno != port_ctrl->portno) {
        espe_td_config_non_cpuport_desc(spe, port_ctrl, desc, param, flags);
    } else {
        espe_td_config_cpuport_desc(spe, desc, param);
    }

    ret = memcpy_s(cur_desc, sizeof(*cur_desc), desc, sizeof(struct espe_td_desc));
    if (unlikely(ret)) {
        SPE_ERR("memcpy_s fail.\n");
    }
}

static int espe_td_param_check(struct spe *spe, int portno, unsigned long long maa_l2_addr)
{
    if (unlikely(portno >= SPE_PORT_NUM || !test_bit(portno, &spe->portmap[SPE_PORTS_NOMARL]))) {
        SPE_ERR_ONCE("port %d not exist!\n", portno);
        espe_bug(spe);
        return -EFAULT;
    }

    if (unlikely(!maa_l2_addr)) {
        spe->ports[portno].stat.td_dma_null++;
        SPE_ERR_ONCE("port %d maa_l2_addr zero null!\n", portno);
        espe_bug(spe);
        return -EFAULT;
    }

    return 0;
}

// last pkt must push! add check;
int bsp_espe_config_td(int portno, unsigned long long maa_l2_addr, unsigned int len, void *param, unsigned int td_flags)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctrl *ctrl = NULL;
    struct spe_port_stat *stat = NULL;
    struct wan_info_s *wan_info = NULL;
    struct sk_buff *skb = NULL;
    unsigned long flags;

    if (espe_td_param_check(spe, portno, maa_l2_addr) != 0) {
        return -EFAULT;
    }

    ctrl = &spe->ports[portno].ctrl;
    stat = &spe->ports[portno].stat;

    if (unlikely(!ctrl->port_flags.enable)) {
        stat->td_port_disabled++;
        SPE_ERR_ONCE("td config fail port %d still disabled! \n", portno);
        return -EIO;
    }

    if (unlikely(spe->flags & SPE_FLAG_SUSPEND)) {
        SPE_ERR_ONCE("td config port %d when spe suspend! \n", portno);
        return -EIO;
    }

    spin_lock_irqsave(&ctrl->td_lock, flags);

    if (unlikely(spe_free_td_num(spe, portno) == 0)) {
        spin_unlock_irqrestore(&ctrl->td_lock, flags);
        stat->td_full++;
        return -EBUSY;
    }
#ifndef CONFIG_ESPE_PHONE_SOC
    if (spe->spe_version == ESPE_VER_V200 || spe->spe_version == ESPE_VER_V300) {
        espe_update_adq_wptr(spe);
    }
#endif
    ctrl->td_param[ctrl->td_free] = param;
    espe_td_config_one(ctrl, maa_l2_addr, len, param, td_flags);

    skb = (struct sk_buff *)param;
    wan_info = (struct wan_info_s *)skb->cb;
    wan_spe_td_cd_pkt_proc(param, portno, wan_info, ctrl);

    ctrl->td_free = (++ctrl->td_free < ctrl->td_depth) ? ctrl->td_free : 0;

    if (unlikely(td_flags & ESPE_TD_KICK_PKT)) {
        stat->td_kick++;
        spe_writel(spe->regs, SPE_TDQX_WPTR(portno), ctrl->td_free);
    }

    stat->td_config++;
    stat->td_config_bytes += len;

    spin_unlock_irqrestore(&ctrl->td_lock, flags);

    return 0;
}

static void espe_rd_record_result(struct espe_rd_desc *cur_desc, struct spe_port_stat *stat)
{
    struct desc_result_s *result = &stat->result;

    result->rd_result[cur_desc->rd_trans_result]++;             // rd desc bit0-1
    result->rd_pkt_drop_rsn[cur_desc->rd_drop_rsn]++;           // rd desc bit2-5
    result->rd_pkt_fw_path[__fls(cur_desc->rd_trans_path)]++;   // rd desc bit6-15
    result->rd_pkt_type[cur_desc->rd_pkt_type]++;               // rd desc bit17-19
    result->rd_finsh_wrap_rsn[cur_desc->rd_finish_warp_res]++;  // rd desc bit20-22
    result->rd_send_cpu_rsn[cur_desc->rd_tocpu_res]++;          // rd desc bit23-26

    stat->result.rd_sport_cnt[cur_desc->rd_sport]++;
    stat->rd_finished_bytes += cur_desc->rd_pkt_len;

    return;
}

static inline void espe_rd_transmit_desc(struct spe_port_ctx *port_ctx, struct sk_buff *skb)
{
    port_ctx->ctrl.ops.espe_finish_rd(skb, skb->len, port_ctx->ctrl.port_priv, ESPE_RD_FLAG_MAGIC);
    port_ctx->stat.rd_sended++;

    return;
}

static inline void espe_rd_set_skb(struct sk_buff *skb, struct espe_rd_desc *desc, unsigned long long maa_l2_addr,
                                   unsigned long long org_maa_addr)
{
    struct rx_cb_map_s *rx_cb = NULL;

    rx_cb = (struct rx_cb_map_s *)(&skb->cb);
    rx_cb->packet_info.u32 = 0;

    switch (desc->rd_pkt_type) {
        case (RD_PKT_IPV4_TCP):
            rx_cb->packet_info.bits.ip_proto = AF_INET;
            rx_cb->packet_info.bits.l4_proto = (unsigned char)IPPROTO_TCP;
            rx_cb->packet_info.bits.is_accable = ESPE_ACCELETABLE_PKT_FLAG;
            break;
        case (RD_PKT_IPV4_UDP):
            rx_cb->packet_info.bits.ip_proto = AF_INET;
            rx_cb->packet_info.bits.l4_proto = (unsigned char)IPPROTO_UDP;
            rx_cb->packet_info.bits.is_accable = ESPE_ACCELETABLE_PKT_FLAG;

            break;
        case (RD_PKT_IPV6_TCP):
            rx_cb->packet_info.bits.ip_proto = AF_INET6;
            rx_cb->packet_info.bits.l4_proto = (unsigned char)IPPROTO_TCP;
            rx_cb->packet_info.bits.is_accable = ESPE_ACCELETABLE_PKT_FLAG;
            break;
        case (RD_PKT_IPV6_UDP):
            rx_cb->packet_info.bits.ip_proto = AF_INET6;
            rx_cb->packet_info.bits.l4_proto = (unsigned char)IPPROTO_UDP;
            rx_cb->packet_info.bits.is_accable = ESPE_ACCELETABLE_PKT_FLAG;
            break;
        default:
            // because of the condition to get into this branch, default can not be reached.
            break;
    }

    rx_cb->packet_info.bits.net_id = desc->rd_ips_id;
    rx_cb->ipf_result.u32 = desc->rd_ipfres_stmid;
    rx_cb->userfield0 = desc->rd_user_field0;
    rx_cb->userfield1 = desc->rd_user_field1;
    rx_cb->userfield2 = desc->rd_user_field2;
    rx_cb->maa_org = org_maa_addr;
    rx_cb->maa_l2 = maa_l2_addr;
    rx_cb->packet_info.bits.l2_hdr_offeset = desc->rd_l2_hdr_offset;
}


/* start of new interface */
static struct sk_buff *espe_resolve_ipf_skb(struct spe *spe, struct spe_port_ctx *port_ctx, struct espe_rd_desc *desc,
                                            unsigned long long maa_l2_addr)
{
    struct sk_buff *skb = NULL;
    unsigned long long org_maa_addr;

    skb = bsp_maa_unmap(maa_l2_addr, &org_maa_addr);
    if (unlikely((skb == NULL) || (org_maa_addr < maa_l2_addr))) {
        port_ctx->stat.maa_to_skb_fail++;
        SPE_ERR_ONCE("%u time maa to skb fail, sport %u dport %u, maa 0x%llx skb %lx \n",
                     port_ctx->stat.maa_to_skb_fail, desc->rd_sport, desc->rd_dport, maa_l2_addr, (uintptr_t)skb);
        espe_bug(spe);
        return NULL;
    }

    if (unlikely((org_maa_addr - maa_l2_addr) != desc->rd_l2_hdr_offset)) {
        port_ctx->stat.skb_err++;
        SPE_ERR("%u time skb err, sport %u org_maa %lx l2_maa %lx hdr_offset %u skb %lx \n", port_ctx->stat.skb_err,
                desc->rd_sport, org_maa_addr, maa_l2_addr, desc->rd_l2_hdr_offset, (uintptr_t)skb);
        espe_bug(spe);
        return NULL;
    }

    // in phone mode mac hdr must move to higher addr than maa hdr
    skb_push(skb, desc->rd_l2_hdr_offset);
    skb_put(skb, desc->rd_pkt_len - desc->rd_l2_hdr_offset);

    espe_rd_set_skb(skb, desc, maa_l2_addr, org_maa_addr);

    return skb;
}

static void espe_rd_process_one_desc(struct spe *spe, struct spe_port_ctx *port_ctx, void *long_buf)
{
    struct espe_rd_desc *desc = (struct espe_rd_desc *)long_buf;
    struct spe_cpuport_ctx *cpuport = &spe->cpuport;
    struct sk_buff *skb = NULL;

    unsigned long long maa_l2_addr;
    unsigned long long rdout32low;
    unsigned long long rdout32upper;

    rdout32low = (unsigned long long)desc->rd_outaddr_lower;
    rdout32upper = (unsigned long long)desc->rd_outaddr_upper;
    maa_l2_addr = rdout32low + (rdout32upper << 32) - desc->rd_l2_hdr_offset; /* left shit 32 bit */

    // this is for dfs, DO NOT Remove
    port_ctx->stat.rd_finsh_pkt_num += desc->rd_pktnum;

    if (unlikely(maa_l2_addr == 0)) {
        spe->ports[cpuport->portno].stat.rd_maa_zero++;
        spe->ports[cpuport->portno].stat.rd_droped++;
        return;
    }

    if (unlikely(spe->drop_stub)) {
        spe->cpuport.cpu_rd_udp_drop++;
        spe->ports[spe->cpuport.portno].stat.rd_droped++;
        espe_free_maa_mem(spe, spe->cpuport.portno, maa_l2_addr);
        return;
    }


    if (spe->dbg_level & SPE_DBG_RD_RESULT) {
        espe_rd_record_result(desc, &port_ctx->stat);
    }

    skb = espe_resolve_ipf_skb(spe, port_ctx, desc, maa_l2_addr);
    if (unlikely(skb == NULL)) {
        spe->ports[cpuport->portno].stat.rd_droped++;
        return;
    }

    if (unlikely(spe->wakeup_flag == TRUE)) {
        SPE_ERR("first dl rd pkt after wakeup ! \n");
        espe_print_pkt_info(skb->data + desc->rd_l2_hdr_offset);
        spe->wakeup_flag = FALSE;
    }

    espe_rd_transmit_desc(&spe->ports[cpuport->portno], skb);
    return;
}
void __espe_process_rd_desc(struct spe *spe, struct spe_port_ctx *port_ctx, unsigned int rd_rptr, unsigned int rd_wptr)
{
    struct espe_rd_desc *rd_base = (struct espe_rd_desc *)port_ctx->ctrl.rd_addr;
    struct espe_rd_desc *long_buf = NULL;
    unsigned int rd_num = 0;
    int i = 0;
    int ret = 0;

    while (rd_rptr != rd_wptr) {
        if (rd_wptr > rd_rptr) {
            rd_num = rd_wptr - rd_rptr;
        } else {
            rd_num = port_ctx->ctrl.rd_depth - rd_rptr;
        }
        long_buf = (struct espe_rd_desc *)port_ctx->ctrl.rd_long_buf;
        ret = memcpy_s(long_buf, port_ctx->ctrl.rd_depth * sizeof(struct espe_rd_desc), &rd_base[rd_rptr],
                       rd_num * sizeof(struct espe_rd_desc));
        if (unlikely(ret)) {
            port_ctx->stat.rewind_th_cpy_fail++;
        }
        rd_rptr += rd_num;
        if (rd_rptr == port_ctx->ctrl.rd_depth) {
            rd_rptr = 0;
        }
        port_ctx->ctrl.rd_free = rd_rptr;
        spe_writel(spe->regs, SPE_RDQX_RPTR(port_ctx->ctrl.portno), rd_rptr);
        for (i = 0; i < rd_num; i++) {
            port_ctx->stat.rd_finished++;
            espe_rd_process_one_desc(spe, port_ctx, long_buf++);
        }
        port_ctx->ctrl.rd_busy = rd_wptr;
    }

    return;
}

void espe_process_rd_desc(unsigned int portno)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctx *port_ctx = &spe->ports[portno];
    unsigned int rd_rptr;
    unsigned int rd_wptr;
    int get_rd_cnt = 0;
    unsigned long flags = 0;

    port_ctx->stat.rd_finsh_intr_complete++;
    while (get_rd_cnt < spe->rd_loop_cnt) {
        spin_lock_irqsave(&port_ctx->ctrl.rd_lock, flags);
        rd_rptr = port_ctx->ctrl.rd_free;
        rd_wptr = spe_readl(spe->regs, SPE_RDQX_WPTR(portno));  // must be spe_readl, not spe_readl_relaxed
        if (rd_rptr == rd_wptr) {
            port_ctx->ctrl.port_flags.rd_not_empty = 0;
            port_ctx->stat.rd_finsh_intr_complete++;
            spin_unlock_irqrestore(&port_ctx->ctrl.rd_lock, flags);
            break;
        }
        __espe_process_rd_desc(spe, port_ctx, rd_rptr, rd_wptr);
        get_rd_cnt++;
        spin_unlock_irqrestore(&port_ctx->ctrl.rd_lock, flags);
    }

    if ((get_rd_cnt != 0) && port_ctx->ctrl.ops.espe_complete_rd) {
        port_ctx->ctrl.ops.espe_complete_rd(port_ctx->ctrl.port_priv);
        port_ctx->stat.rd_finsh_intr_complete_called++;
    }

    return;
}

/* this function for maa hard error eviding */
void espe_update_adq_wptr(struct spe *spe)
{
    struct spe_adq_ctrl *cur_ctrl = NULL;
    unsigned int wptr = 0;
    unsigned int adq_num = 0;
    unsigned long flags = 0;

    spin_lock_irqsave(&spe->spe_ad_lock, flags);
    for (adq_num = 0; adq_num < SPE_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &spe->adqs_ctx[adq_num].ctrl;
        wptr = readl(cur_ctrl->maa_wptr_stub_addr);
        if (wptr < spe->adqs_ctx[adq_num].ctrl.adq_size) {
            spe_writel(spe->regs, SPE_ADQ_WPTR(adq_num), wptr);
        }
    }
    spin_unlock_irqrestore(&spe->spe_ad_lock, flags);

    return;
}

#ifndef CONFIG_ESPE_PHONE_SOC
static void espe_adq_ctrl_timer(unsigned long arg)
{
    struct spe *spe = (struct spe *)(uintptr_t)arg;
    unsigned long flags = 0;

    spin_lock_irqsave(&spe->reset_lock, flags);
    if (spe->modem_resetting != 1) {
        spin_unlock_irqrestore(&spe->reset_lock, flags);
        espe_update_adq_wptr(spe);
    } else {
        spin_unlock_irqrestore(&spe->reset_lock, flags);
    }

    mod_timer(&spe->spe_adq_timer, SPE_DFS_T(spe->spe_adq_time_interval));

    return;
}

void espe_adq_timer_init(struct spe *spe)
{
    init_timer(&spe->spe_adq_timer);
    spe->spe_adq_timer.function = espe_adq_ctrl_timer;
    spe->spe_adq_timer.data = (unsigned long)(uintptr_t)spe;
    spe->spe_adq_time_interval = SPE_ADQ_INTERVAL_DEFAULT;
    spe->spe_adq_timer.expires = SPE_DFS_T(spe->spe_adq_time_interval);
    add_timer(&spe->spe_adq_timer);
}
#endif

static void espe_ad_init_defcfg(struct spe *spe)
{
    spe->adqs_ctx[SPE_ADQ0].ctrl.maa_ipip_type = MAA_IPIPE_FOR_SPE_512;
    spe->adqs_ctx[SPE_ADQ1].ctrl.maa_ipip_type = MAA_IPIPE_FOR_SPE_2K;
    spe->adqs_ctx[SPE_ADQ0].ctrl.adbuf_len = SPE_AD0_PKT_LEN;
    spe->adqs_ctx[SPE_ADQ1].ctrl.adbuf_len = SPE_AD1_PKT_LEN;
    spe->adqs_ctx[SPE_ADQ0].ctrl.adq_size = SPE_ADQ_DEF_SIZE;
    spe->adqs_ctx[SPE_ADQ1].ctrl.adq_size = SPE_ADQ_DEF_SIZE;

    spe_writel_relaxed(spe->regs, SPE_ETH_MAXLEN, SPE_AD1_PKT_LEN);
    spe_writel_relaxed(spe->regs, SPE_ADQ_THRESHOLD, SPE_MIN_PKT_SIZE);
    spe_writel_relaxed(spe->regs, SPE_ADQ_LEN(SPE_ADQ0), SPE_ADQ_SHORT);
    spe_writel_relaxed(spe->regs, SPE_ADQ_LEN(SPE_ADQ1), SPE_ADQ_LONG);
    return;
}

int espe_ad_init(struct spe *spe)
{
    struct spe_adq_ctrl *cur_ctrl = NULL;
    enum spe_adq_num adq_num;
    dma_addr_t maa_rptr_reg_dma_addr;
    dma_addr_t ad_wptr_reg_addr;
    int ret = 0;

    espe_ad_init_defcfg(spe);

    for (adq_num = SPE_ADQ0; adq_num < SPE_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &spe->adqs_ctx[adq_num].ctrl;

        cur_ctrl->ad_base_addr = dma_alloc_coherent(spe->dev, cur_ctrl->adq_size * sizeof(struct espe_ad_desc),
                                                    &cur_ctrl->ad_dma_addr, GFP_KERNEL);
        if (cur_ctrl->ad_base_addr == NULL) {
            SPE_ERR("alloc adq desc pool failed.\n");
            ret = -ENOMEM;
        }

        spe_writel_relaxed(spe->regs, SPE_ADQ_BADDR_L(adq_num), lower_32_bits(cur_ctrl->ad_dma_addr));
        spe_writel_relaxed(spe->regs, SPE_ADQ_BADDR_H(adq_num), upper_32_bits(cur_ctrl->ad_dma_addr));
        spe_writel_relaxed(spe->regs, SPE_ADQ_SIZE(adq_num), SPE_ADQ_SIZE_SETTING);

        if (spe->spe_version == ESPE_VER_V200 || spe->spe_version == ESPE_VER_V300) { /* evide maa hard error */
            cur_ctrl->maa_wptr_stub_addr = dma_alloc_coherent(spe->dev, sizeof(unsigned long long),
                                                              &cur_ctrl->maa_wptr_stub_dma_addr, GFP_KERNEL);
            if (cur_ctrl->maa_wptr_stub_addr == NULL) {
                SPE_ERR("map maa stub fail.\n");
                ret = -ENOMEM;
            }

            maa_rptr_reg_dma_addr = (dma_addr_t)bsp_maa_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                    cur_ctrl->maa_wptr_stub_dma_addr, SPE_ADQ_DEF_SIZE,
                                                                    spe->adqs_ctx[adq_num].ctrl.maa_ipip_type);
        } else {
            ad_wptr_reg_addr = spe->res + SPE_ADQ_WPTR(adq_num);
            maa_rptr_reg_dma_addr = (dma_addr_t)bsp_maa_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                    lower_32_bits(ad_wptr_reg_addr), SPE_ADQ_DEF_SIZE,
                                                                    spe->adqs_ctx[adq_num].ctrl.maa_ipip_type);
        }

        spe_writel(spe->regs, SPE_ADQ_RPTR_UPDATE_ADDR_L(adq_num), lower_32_bits(maa_rptr_reg_dma_addr));
        spe_writel(spe->regs, SPE_ADQ_RPTR_UPDATE_ADDR_H(adq_num), upper_32_bits(maa_rptr_reg_dma_addr));
        spe_writel(spe->regs, SPE_ADQ_EN(adq_num), 1);
    }

    spe_writel_relaxed(spe->regs, SPE_ADQ_EMPTY_INTA_MASK, SPE_AD_INTR_EN);

    return ret;
}

void espe_ad_exit(struct spe *spe)
{
    struct spe_adq_ctrl *cur_ctrl = NULL;
    enum spe_adq_num adq_num;

    for (adq_num = SPE_ADQ0; adq_num < SPE_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &spe->adqs_ctx[adq_num].ctrl;
        if (cur_ctrl->ad_base_addr != NULL) {
            dma_free_coherent(spe->dev, cur_ctrl->adq_size * sizeof(struct espe_ad_desc), cur_ctrl->ad_base_addr,
                              cur_ctrl->ad_dma_addr);
            cur_ctrl->ad_base_addr = NULL;
        }
        if (spe->spe_version == ESPE_VER_V200 || spe->spe_version == ESPE_VER_V300) {
            if (cur_ctrl->maa_wptr_stub_addr != NULL) {
                dma_free_coherent(spe->dev, sizeof(unsigned long long), cur_ctrl->maa_wptr_stub_addr,
                                  cur_ctrl->maa_wptr_stub_dma_addr);
                cur_ctrl->maa_wptr_stub_addr = NULL;
            }
            del_timer(&spe->spe_adq_timer);
        }
    }

    return;
}

#ifndef SPE_MAA
int espe_ad_config(unsigned int portn, dma_addr_t dma, unsigned int len)
{
    struct spe *spe_ctx = &g_espe;
    enum spe_adq_num adq_num;
    struct spe_adq_ctrl *cur_ctrl;
    unsigned int wptr, rptr;
    struct espe_ad_desc *base_desc = NULL;
    unsigned long long len_ll = (unsigned long long)len;

    if (len > SPE_AD1_PKT_LEN) {
        adq_num = SPE_ADQ0;
    } else {
        adq_num = SPE_ADQ1;
    }
    wptr = spe_readl(spe_ctx->regs, SPE_ADQ_WPTR(adq_num));
    rptr = spe_readl(spe_ctx->regs, SPE_ADQ_RPTR(adq_num));
    cur_ctrl = &spe_ctx->adqs_ctx[adq_num].ctrl;
    if ((wptr + 1) % SPE_ADQ_DEF_SIZE != rptr) {
        base_desc = (struct espe_ad_desc *)cur_ctrl->ad_base_addr;
        base_desc[wptr].ad_addr = ((len_ll << 40) | (dma & 0xfffffffff)); /* 40 bit is maa  valid add */
        wptr = (wptr + 1) % SPE_ADQ_DEF_SIZE;
        spe_writel(spe_ctx->regs, SPE_ADQ_WPTR(adq_num), wptr);
    } else {
        return -1;
    }
    return 0;
}

int spe_ad_empty(struct spe *spe, enum spe_adq_num adq_num)
{
    unsigned int buff_len;
    unsigned int wptr;
    unsigned int rptr;
    int ret = 0;

    wptr = spe_readl(spe->regs, SPE_ADQ_WPTR(adq_num));
    rptr = spe_readl(spe->regs, SPE_ADQ_RPTR(adq_num));
    SPE_ERR("spe ad empty at wptr %u rptr %u \n", wptr, rptr);

    if (SPE_ADQ0 == adq_num) {
        buff_len = SPE_AD0_PKT_LEN;
    } else if (SPE_ADQ1 == adq_num) {
        buff_len = SPE_AD1_PKT_LEN;
    } else {
        return -1;
    }
    return ret;
}

void spe_add_ad(unsigned int adq_num)
{
    dma_addr_t dma;
    void *ad_buffer;
    int ret = 0;
    unsigned int wptr;
    unsigned int rptr;
    struct spe *spe = &g_espe;

    if (adq_num == 0) {
        ad_buffer = kmalloc(SPE_AD0_PKT_LEN, GFP_ATOMIC);
        if (!ad_buffer) {
            SPE_ERR("init alloc spe ad fail. \n");
            return;
        }
        dma = dma_map_single(spe->dev, ad_buffer, SPE_AD0_PKT_LEN, DMA_FROM_DEVICE);
        dma = virt_to_phys(ad_buffer);
        ret = espe_ad_config(0, dma, SPE_AD0_PKT_LEN);
        if (ret) {
            SPE_ERR("adinit config spe ad0 fail. \n");
        }
    } else if (adq_num == 1) {
        ad_buffer = kmalloc(SPE_AD1_PKT_LEN, GFP_ATOMIC);
        if (!ad_buffer) {
            SPE_ERR("init alloc spe ad fail. \n");
            return;
        }
        dma = dma_map_single(spe->dev, ad_buffer, SPE_AD1_PKT_LEN, DMA_FROM_DEVICE);
        dma = virt_to_phys(ad_buffer);
        ret = espe_ad_config(0, dma, SPE_AD1_PKT_LEN);
        if (ret) {
            SPE_ERR("adinit config spe ad0 fail. \n");
        }
    } else {
        SPE_ERR("wrong adq_num input. \n");
        return;
    }
    wptr = spe_readl(spe->regs, SPE_ADQ_WPTR(adq_num));
    rptr = spe_readl(spe->regs, SPE_ADQ_RPTR(adq_num));
    SPE_ERR("spe ad add at wptr %u rptr %u \n", wptr, rptr);
}

static void espe_ad_no_maa_init_defcfg(struct spe *spe)
{
    spe_writel_relaxed(spe->regs, SPE_ADQ_THRESHOLD, SPE_AD_PKT_LEN_THR);

    spe->adqs_ctx[SPE_ADQ0].ctrl.adbuf_len = SPE_AD0_PKT_LEN;
    spe->adqs_ctx[SPE_ADQ0].ctrl.adq_size = SPE_ADQ_DEF_SIZE;
    spe_writel_relaxed(spe->regs, SPE_ADQ_LEN(SPE_ADQ0), SPE_ADQ_SHORT);

    spe->adqs_ctx[SPE_ADQ1].ctrl.adbuf_len = SPE_AD1_PKT_LEN;
    spe->adqs_ctx[SPE_ADQ1].ctrl.adq_size = SPE_ADQ_DEF_SIZE;
    spe_writel_relaxed(spe->regs, SPE_ADQ_LEN(SPE_ADQ1), SPE_ADQ_LONG);

    return;
}

int espe_ad_no_maa_init(struct spe *spe)
{
    struct spe_adq_ctrl *cur_ctrl = NULL;
    enum spe_adq_num adq_num;
    int ret = 0;
    int i;
    dma_addr_t dma;
    void *ad_buffer = NULL;

    espe_ad_no_maa_init_defcfg(spe);

    for (adq_num = SPE_ADQ0; adq_num < SPE_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &spe->adqs_ctx[adq_num].ctrl;
        cur_ctrl->ad_base_addr = dma_alloc_coherent(spe->dev, cur_ctrl->adq_size * sizeof(struct espe_ad_desc),
                                                    &cur_ctrl->ad_dma_addr, GFP_KERNEL);
        if (!cur_ctrl->ad_base_addr) {
            SPE_ERR("alloc adq desc pool failed.\n");
            ret = -ENOMEM;
        }

        spe_writel_relaxed(spe->regs, SPE_ADQ_BADDR_H(adq_num), upper_32_bits(cur_ctrl->ad_dma_addr));
        spe_writel_relaxed(spe->regs, SPE_ADQ_BADDR_L(adq_num), lower_32_bits(cur_ctrl->ad_dma_addr));
        spe_writel_relaxed(spe->regs, SPE_ADQ_SIZE(adq_num), SPE_ADQ_SIZE_SETTING);
        spe_writel(spe->regs, SPE_ADQ_EN(adq_num), 1);
    }

    for (i = 0; i < (SPE_ADQ_DEF_SIZE - 1); i++) {
        ad_buffer = kmalloc(SPE_AD0_PKT_LEN, GFP_ATOMIC);
        if (!ad_buffer) {
            SPE_ERR("init alloc spe ad fail. \n");
            return -ENOMEM;
        }
        dma = dma_map_single(spe->dev, ad_buffer, SPE_AD0_PKT_LEN, DMA_FROM_DEVICE);
        dma = virt_to_phys(ad_buffer);
        ret = espe_ad_config(0, dma, SPE_AD0_PKT_LEN);
        if (ret) {
            SPE_ERR("adinit config spe ad0 fail. \n");
        }

        ad_buffer = kmalloc(SPE_AD1_PKT_LEN, GFP_ATOMIC);
        if (!ad_buffer) {
            SPE_ERR("init alloc spe ad fail. \n");
            return -ENOMEM;
        }
        dma = dma_map_single(spe->dev, ad_buffer, SPE_AD1_PKT_LEN, DMA_FROM_DEVICE);
        ret = espe_ad_config(0, dma, SPE_AD1_PKT_LEN);
        if (ret) {
            SPE_ERR("adinit config spe ad0 fail. \n");
        }
    }
    SPE_ERR("no_maa ad init success \n");

    spe_writel_relaxed(spe->regs, SPE_ADQ_EMPTY_INTA_MASK, SPE_AD_INTR_EN);

    return ret;
}

#endif

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(bsp_espe_config_td);

