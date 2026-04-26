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

/*
 * spe_port.c - spe port management
 */
#include <linux/netdevice.h>
#include <securec.h>

#include <bsp_aximem.h>
#include <bsp_maa.h>

#include "espe.h"
#include "espe_port.h"
#include "espe_desc.h"
#include "espe_dbg.h"
#include "espe_interrupt.h"


int espe_port_exist(struct spe *spe, int portno)
{
    int isgrp;

    if (unlikely(portno >= SPE_PORT_MAX)) {
        SPE_ERR("invalid portno(%d)!\n", portno);
        return 0;
    }

    isgrp = (portno >= SPE_PORT_NUM) ? 1 : 0;

    return test_bit(portno, &spe->portmap[isgrp]);
}

int espe_port_set_mac(unsigned int portno, const unsigned char *mac)
{
    struct spe *spe = &g_espe;
    unsigned int mac_hi;
    unsigned int mac_lo;

    if (!SPE_NORM_PORT(portno)) {
        SPE_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if (!espe_port_exist(spe, portno)) {
        SPE_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if (mac == NULL) {
        SPE_ERR("mac is null ptr.\n");
        return -EINVAL;
    }

    mac_lo = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    mac_hi = (mac[4]) | (mac[5] << 8);

    spe_writel(spe->regs, SPE_PORTX_MAC_ADDR_L(portno), mac_lo);
    spe_writel(spe->regs, SPE_PORTX_MAC_ADDR_H(portno), mac_hi);

    return 0;
}

void espe_set_ipv6_addr(unsigned int portno, const unsigned int *ip_addr)
{
    struct spe *spe = &g_espe;

    spe_writel(spe->regs, SPE_PORTX_IPV6_ADDR_1ST(portno), ip_addr[0]);
    spe_writel(spe->regs, SPE_PORTX_IPV6_ADDR_2ND(portno), ip_addr[1]);
    spe_writel(spe->regs, SPE_PORTX_IPV6_ADDR_3TH(portno), ip_addr[2]);
    spe_writel(spe->regs, SPE_PORTX_IPV6_ADDR_4TH(portno), ip_addr[3]);
}

void epse_set_ipv4_addr(unsigned int portno, unsigned int ip_addr)
{
    struct spe *spe = &g_espe;
    spe_writel(spe->regs, SPE_PORTX_IPV4_ADDR(portno), ip_addr);
}

void espe_ackpkt_identify(struct spe *spe, unsigned int portno)
{
    spe_writel_relaxed(spe->regs, SPE_ACK_IDEN_PORT, portno);

    return;
}

int espe_ack_para_set(struct spe *spe)
{
    struct spe_port_ctrl *ipf_port_ctrl = NULL;

    if (spe->ipfport.portno >= SPE_PORT_NUM) {
        SPE_ERR("spe ipf port not alloced, ack port can not be alloc\n");
        return -EINVAL;
    }
    ipf_port_ctrl = &spe->ports[spe->ipfport.portno].ctrl;

    if (ipf_port_ctrl->portno != spe->ipfport.portno) {
        SPE_ERR("spe ipf port num err, ack port can not be alloc\n");
        return -EINVAL;
    }

    if (ipf_port_ctrl->property.bits.spe_port_enc_type != SPE_ENC_IPF) {
        SPE_ERR("spe ipf port num err, ack port can not be alloc\n");
        return -EINVAL;
    }

    if (ipf_port_ctrl->property.bits.spe_port_en != spe->ipfport.portno) {
        SPE_ERR("spe ipf port not enable, ack port may not work\n");
    }

    spe_writel_relaxed(spe->regs, SPE_ACK_IDEN_PORT, spe->ipfport.portno);

    spe_writel_relaxed(spe->regs, SPE_ACK_IDEN_EN, 0x1);
    return 0;
}

int espe_port_wait_td_complete(unsigned int portno, unsigned int timeout)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctx *port_ctx = &spe->ports[portno];
    unsigned td_rptr;
    unsigned int port_idle;

    do {
        /* judge spe_idle */
        port_idle = spe_readl(spe->regs, SPE_IDLE);

        /* make sure td queue is empty */
        td_rptr = spe_readl(spe->regs, SPE_TDQX_RPTR(portno));
        if ((port_idle & (1 << portno)) && (port_ctx->ctrl.td_busy == td_rptr)) {
            break;
        }

        if (spe->spe_version == ESPE_VER_V200) {
            espe_process_td_desc(portno);
        }
        if (spe->spe_version == ESPE_VER_V300 && portno != spe->ipfport.portno) {
            espe_process_td_desc(portno);  // spev300, td fail free mem in cpu rd desc
        }

        if (timeout-- <= 0) {
            if (port_idle & (1 << portno)) {
                port_ctx->stat.free_td_idle++;
                SPE_ERR("spe port %u tdq wait idle timed out\n", portno);
            } else {
                port_ctx->stat.free_tdq++;
                SPE_ERR("spe port %u tdq rptr not eq wptr timed out\n", portno);
            }
            port_ctx->stat.free_busy++;

            return -EBUSY;
        }
        udelay(1);
    } while (1);

    return 0;
}

int espe_port_wait_rd_complete(unsigned int portno, unsigned int timeout)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctx *port_ctx = &spe->ports[portno];
    unsigned rd_rptr;
    unsigned rd_wptr;
    unsigned int port_idle;

    do {
        /* judge spe_idle */
        port_idle = spe_readl(spe->regs, SPE_IDLE);

        /* make sure rd queue is empty */
        rd_rptr = spe_readl(spe->regs, SPE_RDQX_RPTR(portno));
        rd_wptr = spe_readl(spe->regs, SPE_RDQX_WPTR(portno));
        if ((port_idle & (1 << portno)) && (rd_rptr == rd_wptr)) {
            break;
        }

        if (portno != spe->ipfport.portno) {
            espe_process_rd_desc(portno);
        }

        if (timeout-- <= 0) {
            if (port_idle & (1 << portno)) {
                port_ctx->stat.free_rd_idle++;
                SPE_ERR("spe port %u rdq wait idle timed out\n", portno);
            } else {
                port_ctx->stat.free_rdq++;
                SPE_ERR("spe port %u rdq rptr not eq wptr timed out\n", portno);
            }

            return -EBUSY;
        }
        udelay(1);
    } while (1);

    return 0;
}

int espe_recycle_port_queue_desc(struct spe *spe, unsigned int portno)
{
    int ret;

    // disable interrput interval
    spe_writel(spe->regs, SPE_INTA_INTERVAL, 0);

    ret = espe_port_wait_td_complete(portno, 1000);
    if (ret) {
        return ret;
    }

    ret = espe_port_wait_rd_complete(portno, 1000);
    if (ret) {
        return ret;
    }

    spe_writel(spe->regs, SPE_INTA_INTERVAL, spe->irq_interval);

    return 0;
}

int espe_port_check_common_attr(struct espe_port_comm_attr *attr)
{
    int ret = 0;
    if (attr == NULL) {
        SPE_ERR("input attr NULL.\n");
        BUG_ON(1);
        return -EINVAL;
    }

    if (attr->enc_type >= SPE_ENC_BOTTOM) {
        SPE_ERR("invalid enc_type(%d).\n", attr->enc_type);
        ret = -EINVAL;
        goto error;
    }

    /* every port but ipf should have td */
    if ((!attr->rd_depth) || (!attr->td_depth) || (attr->td_depth >= SPE_MAX_TD)) {
        SPE_ERR("port desc_depth err td %u rd %u.\n", attr->td_depth, attr->rd_depth);
        ret = -EINVAL;
        goto error;
    }
    return 0;

error:
    espe_get_port_type(0, attr->enc_type);
    BUG_ON(1);
    return ret;
}

int espe_port_check_attr(struct espe_port_comm_attr *attr)
{
    int ret = 0;

    if (espe_port_check_common_attr(attr)) {
        goto error;
    }

    if (attr->enc_type == SPE_ENC_CPU && attr->ops.get_wan_info == NULL) {
        SPE_ERR("wanport set get_wan_info param failed.\n");
        goto error;
    }
    if (attr->enc_type != SPE_ENC_CPU && attr->ops.get_wan_info != NULL) {
        SPE_ERR("any port other than wanport should not set get_wan_info.\n");
        goto error;
    }

    if (attr->enc_type == SPE_ENC_IPF) {
        return 0;
    }

    if ((attr->enc_type >= SPE_ENC_NCM_NTB16 && attr->enc_type <= SPE_ENC_RNDIS) || (attr->enc_type == SPE_ENC_ECM) ||
        (attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB32) || (attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB16) ||
        (attr->enc_type == SPE_ENC_DIRECT_FW_RNDIS) || (attr->enc_type == SPE_ENC_DIRECT_FW_ECM) ||
        (attr->td_copy_en == SPE_PORT_TD_COPY_ENABLE)) {
        if (!attr->ops.espe_finish_td) {     /* only usb port needs td finish */
            SPE_ERR(" port has no espe_finish_td.\n");
            goto error;
        }
    } else {
        if (!attr->net) {
            SPE_ERR("input net dev NULL.\n");
            goto error;
        }
    }

    /* every port but ipf should have finish rd */
    if (!attr->ops.espe_finish_rd && !attr->ops.espe_finish_rd_maa) {
        SPE_ERR("port has no espe_finish_rd\n");
        goto error;
    }

    return 0;

error:
    ret = -EINVAL;
    espe_get_port_type(0, attr->enc_type);
    BUG_ON(1);
    return ret;
}

void espe_set_ipf_mac(struct ethhdr *mac_hdr)
{
    struct spe *spe = &g_espe;
    unsigned char *mac;
    unsigned int mac_lo;
    unsigned int mac_hi;
    int ret;

    mac = mac_hdr->h_dest;
    mac_lo = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
    mac_hi = (mac[5] << 8) | mac[4];

    spe_writel_relaxed(spe->regs, SPE_GLB_DMAC_ADDR_L, mac_lo);
    spe_writel_relaxed(spe->regs, SPE_GLB_DMAC_ADDR_H, mac_hi);

    mac = mac_hdr->h_source;
    mac_lo = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
    mac_hi = (mac[5] << 8) | mac[4];

    spe_writel_relaxed(spe->regs, SPE_GLB_SMAC_ADDR_L, mac_lo);
    spe_writel_relaxed(spe->regs, SPE_GLB_SMAC_ADDR_H, mac_hi);

    ret = memcpy_s(&spe->ipfport.ipf_eth_head, sizeof(spe->ipfport.ipf_eth_head), mac_hdr, sizeof(*mac_hdr));
    if (ret) {
        SPE_ERR("<espe_set_ipf_mac> memcpy_s fail!\n");
    }
}

void espe_port_clean_ctx(struct spe *spe, struct spe_port_ctx *port_ctx)
{
    int ret = 0;
    struct spe_port_ctrl *port_ctrl = &port_ctx->ctrl;

    switch (port_ctrl->property.bits.spe_port_enc_type) {
        case SPE_ENC_NONE:
        case SPE_ENC_WIFI:
        case SPE_ENC_ACK:
            break;

        case SPE_ENC_NCM_NTB16:
        case SPE_ENC_NCM_NTB32:
        case SPE_ENC_RNDIS:
        case SPE_ENC_ECM:
            ret += memset_s(&spe->usbport, sizeof(spe->usbport), 0, sizeof(struct spe_usbport_ctx));
            break;

        case SPE_ENC_IPF:
            ret += memset_s(&spe->ipfport, sizeof(spe->ipfport), 0, sizeof(struct spe_ipfport_ctx));
            ret += memset_s(&spe->wanport, sizeof(spe->wanport), 0, sizeof(struct spe_wanport_ctx));
            break;

        case SPE_ENC_CPU:
            ret += memset_s(&spe->cpuport, sizeof(spe->cpuport), 0, sizeof(struct spe_cpuport_ctx));
            break;

        default:
            break;
    }

    ret += memset_s(&port_ctx->stat, sizeof(port_ctx->stat), 0, sizeof(struct spe_port_stat));
    if (ret) {
        SPE_ERR("memset_s fail!\n");
    }
}

static int espe_port_wait_idle(struct spe *spe, unsigned int portno)
{
    int ret;
    unsigned int evt_rd_full_mask;
    unsigned int evt_rd_done_mask;
    unsigned int evt_td_done_mask;

    /* wait for port not busy. after port idle,td/rd queue is empty. */
    ret = espe_port_wait_td_complete(portno, 1000);
    if (ret) {
        return ret;
    }

    ret = espe_port_wait_rd_complete(portno, 1000);
    if (ret) {
        return ret;
    }

    /* rewind the rd desc */
    espe_rd_rewind(portno);

    evt_rd_full_mask = spe_readl_relaxed(spe->regs, SPE_RDQ_FULL_INTA_MASK) & ~(1 << portno);
    evt_rd_done_mask = spe_readl_relaxed(spe->regs, SPE_RD_DONE_INTA_MASK) & ~(1 << portno);
    evt_td_done_mask = spe_readl(spe->regs, SPE_TD_DONE_INTA_MASK) & ~(1 << portno);

    spe_writel_relaxed(spe->regs, SPE_RDQ_FULL_INTA_MASK, evt_rd_full_mask);
    spe_writel_relaxed(spe->regs, SPE_RD_DONE_INTA_MASK, evt_rd_done_mask);
    spe_writel_relaxed(spe->regs, SPE_TD_DONE_INTA_MASK, evt_td_done_mask);
    spe_writel(spe->regs, SPE_BUS_ERR_INTA_MASK, SPE_EVENT_DISABLE);
    espe_intr_set_interval(SPE_INTR_DEF_INTEVAL);
    return 0;
}

static void espe_release_port_res(struct spe *spe, struct spe_port_ctrl *port_ctrl)
{
    if (port_ctrl->rd_long_buf != NULL) {
        kfree(port_ctrl->rd_long_buf);
        port_ctrl->rd_long_buf = NULL;
    }

    /* free td queue */
    if (port_ctrl->td_addr != NULL) {
        dma_free_coherent(spe->dev, port_ctrl->td_depth * sizeof(struct espe_td_desc), (void *)port_ctrl->td_addr,
                          port_ctrl->td_dma);
        port_ctrl->td_addr = NULL;
        port_ctrl->td_dma = 0;
    }

    if (port_ctrl->rd_addr != NULL) {
        dma_free_coherent(spe->dev, port_ctrl->rd_depth * sizeof(struct espe_rd_desc), (void *)port_ctrl->rd_addr,
                          port_ctrl->rd_dma);
        port_ctrl->rd_addr = NULL;
        port_ctrl->rd_dma = 0;
    }
    return;
}


int espe_port_free(int port_num)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctx *port_ctx = NULL;
    struct spe_port_ctrl *port_ctrl = NULL;
    int ret;
    unsigned long flags;
    unsigned int portno;

    if (port_num < 0) {
        SPE_ERR("port number %d err !\n", port_num);
        return -EINVAL;
    } else {
        portno = (unsigned int)port_num;
    }
    if (unlikely(spe->flags & SPE_FLAG_SUSPEND)) {
        SPE_ERR("port free during spe suspend! \n");
        return -EIO;
    }

    if ((portno >= SPE_PORT_NUM) || (!((1 << portno) & (spe->portmap[SPE_PORTS_NOMARL])))) {
        SPE_ERR("port number %d err !\n", portno);
        return -ENOENT;
    }

    port_ctx = &spe->ports[portno];
    port_ctrl = &port_ctx->ctrl;

    /* port must be disabled before port free */
    spin_lock_irqsave(&port_ctrl->lock, flags);
    if (port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);

        port_ctx->stat.free_while_enabled++;
        SPE_ERR("port %u free while enabled!\n", portno);
        return -EIO;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    ret = espe_port_wait_idle(spe, portno);
    if (ret) {
        return ret;
    }

    espe_release_port_res(spe, port_ctrl);
    espe_port_clean_ctx(spe, port_ctx);

    clear_bit(portno, &spe->portmap[SPE_PORTS_NOMARL]);

    if (memset_s(&spe->ports[portno].ctrl, sizeof(spe->ports[portno].ctrl), 0, sizeof(struct spe_port_ctrl))) {
        SPE_ERR("port free struct not set to 0\n");
    }

    return 0;
}

int espe_alloc_br_port(void)
{
    unsigned int portno;
    struct spe *spe = &g_espe;

    portno = find_first_zero_bit(&spe->portmap[SPE_PORTS_BR], SPE_PORT_NUM);

    SPE_INFO("alloc a bridge chan.\n");

    return portno + SPE_PORT_NUM;
}
int espe_free_br_port(unsigned portno)
{
    struct spe *spe = &g_espe;
    portno -= SPE_PORT_NUM;

    if (!(portno & (spe->portmap[SPE_PORTS_BR]))) {
        SPE_ERR("port %d not allocated!\n", portno);
        return -ENOENT;
    }
    return 0;
}

int espe_get_free_portno(struct espe_port_comm_attr *attr)
{
    unsigned int portno;
    struct spe *spe = &g_espe;

    if (attr->enc_type == SPE_ENC_IPF) {
        portno = ESPE_DEFAULT_IPF_PORT;
    } else if (attr->enc_type == SPE_ENC_CPU) {
        portno = ESPE_DEFAULT_CPU_PORT;
    } else if (attr->rsv_port_id == SPE_ACORE_PPP_NDIS_CTRL_ID) {
        portno = SPE_ACORE_PPP_NDIS_CTRL_ID;
    } else if (attr->rsv_port_id == SPE_ACORE_LAN_CTRL_ID) {
        portno = SPE_ACORE_LAN_CTRL_ID;
    } else {
        portno = find_first_zero_bit(&spe->portmap[SPE_PORTS_NOMARL], SPE_CCORE_PORT_BEGIN);
    }
    set_bit(portno, &spe->portmap[SPE_PORTS_NOMARL]);

    return portno;
}

int espe_alloc_axi_desc_buffer(struct spe_port_ctrl *port_ctrl)
{
    // alloc td desc
    port_ctrl->td_axi_buf_dma = 0;
    port_ctrl->axi_td = kzalloc(sizeof(struct espe_td_desc), GFP_ATOMIC);
    if (port_ctrl->axi_td == NULL) {
        return -ENOMEM;
    }

    return 0;
}

int espe_alloc_desc_queue(struct spe_port_ctrl *port_ctrl, struct espe_port_comm_attr *attr)
{
    struct spe *spe = &g_espe;
    unsigned int td_desc_size;
    unsigned int rd_desc_size;
    int ret;

    if (port_ctrl->ext_desc) {
        td_desc_size = sizeof(struct espe_ext_td_desc) * port_ctrl->td_depth;
        rd_desc_size = sizeof(struct espe_ext_rd_desc) * port_ctrl->rd_depth;
    } else {
        td_desc_size = sizeof(struct espe_td_desc) * port_ctrl->td_depth;
        rd_desc_size = sizeof(struct espe_rd_desc) * port_ctrl->rd_depth;
    }

    port_ctrl->td_addr = dma_alloc_coherent(spe->dev, td_desc_size, &port_ctrl->td_dma, GFP_KERNEL);
    if (port_ctrl->td_addr == NULL) {
        SPE_ERR("port %d alloc td pool failed.\n", port_ctrl->portno);
        goto alloc_desc_fail;
    }

    port_ctrl->rd_addr = dma_alloc_coherent(spe->dev, rd_desc_size, &port_ctrl->rd_dma, GFP_KERNEL);
    if (port_ctrl->rd_addr == NULL) {
        SPE_ERR("port %d alloc rd pool failed.\n", port_ctrl->portno);
        goto alloc_desc_fail;
    }

    port_ctrl->rd_long_buf = kzalloc(rd_desc_size, GFP_KERNEL);
    if (port_ctrl->rd_long_buf == NULL) {
        SPE_ERR("port %d alloc rd_long_buf failed.\n", port_ctrl->portno);
        goto alloc_desc_fail;
    }

    ret = espe_alloc_axi_desc_buffer(port_ctrl);
    if (ret) {
        goto alloc_desc_fail;
    }
    return 0;

alloc_desc_fail:

    if (port_ctrl->td_addr != NULL) {
        dma_free_coherent(spe->dev, td_desc_size, port_ctrl->td_addr, port_ctrl->td_dma);
        port_ctrl->td_addr = NULL;
        port_ctrl->td_dma = 0;
    }

    if (port_ctrl->rd_addr != NULL) {
        dma_free_coherent(spe->dev, rd_desc_size, port_ctrl->rd_addr, port_ctrl->rd_dma);
        port_ctrl->rd_addr = NULL;
        port_ctrl->rd_dma = 0;
    }

    if (port_ctrl->rd_long_buf != NULL) {
        kfree(port_ctrl->rd_long_buf);
        port_ctrl->rd_long_buf = NULL;
    }

    return -ENOMEM;
}
static void espe_port_property_init(struct spe_port_ctrl *port_ctrl, struct espe_port_comm_attr *attr)
{
    /* copy param from attr */
    port_ctrl->property.bits.spe_port_attach_brg = SPE_BRG_NORMAL;
    if (attr->enc_type == SPE_ENC_ECM) {
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NONE;
    } else {
        port_ctrl->property.bits.spe_port_enc_type = attr->enc_type;
    }
    port_ctrl->property.bits.spe_port_pad_en = attr->padding_enable;

    // direct fw
    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_HP) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_HP_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NONE;
    }

    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_MP) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_MP_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NONE;
    }

    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_PE) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_PE_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NONE;
    }
    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_NCM_NTB32) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_HP_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NCM_NTB32;
    }
    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_NCM_NTB16) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_PE_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NCM_NTB16;
    }
    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_RNDIS) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_HP_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_RNDIS;
    }
    if (port_ctrl->property.bits.spe_port_enc_type == SPE_ENC_DIRECT_FW_ECM) {
        port_ctrl->direct_fw_ctx.port_weight = DIRECT_FW_HP_PORT_WEIGHT;
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_ECM;
    }
    return;
}
void espe_portctrl_init(struct spe *spe, struct spe_port_ctrl *port_ctrl, struct espe_port_comm_attr *attr)
{
    espe_port_property_init(port_ctrl, attr);

    port_ctrl->td_depth = attr->td_depth;
    port_ctrl->rd_depth = attr->rd_depth;
    port_ctrl->ops.espe_finish_rd = attr->ops.espe_finish_rd;
    port_ctrl->ops.espe_finish_rd_maa = attr->ops.espe_finish_rd_maa;
    port_ctrl->ops.espe_complete_rd = attr->ops.espe_complete_rd;
    port_ctrl->ops.espe_finish_td = attr->ops.espe_finish_td;
    port_ctrl->ops.espe_netif_rx = attr->ops.espe_netif_rx;
    port_ctrl->port_priv = attr->priv;

    if (attr->net) {
        dev_hold(attr->net);
        port_ctrl->net = attr->net;
        dev_put(attr->net);
    } else {
        port_ctrl->net = NULL;
    }

    /* set some default value   */
    port_ctrl->priority = SPE_DEF_PORT_PRI;

    /* rate limit */
    port_ctrl->udp_limit_time = PORT_UDP_LIMIT_TIME_DEF;
    port_ctrl->udp_limit_cnt = PORT_UDP_LIMIT_CNT_DEF;
    port_ctrl->rate_limit_time = PORT_RATE_LIMIT_TIME_DEF;
    port_ctrl->rate_limit_byte = PORT_RATE_LIMIT_BYTE_DEF;

    port_ctrl->td_evt_gap = PORT_TD_EVT_GAP_DEF;
    port_ctrl->rd_evt_gap = PORT_RD_EVT_GAP_DEF;

    // bypass
    port_ctrl->bypassport = attr->bypassport;
    port_ctrl->bypassport_en = attr->bypassport_en;

    if (spe->spe_version == ESPE_VER_V300) {
        port_ctrl->td_copy_en = attr->td_copy_en;
        port_ctrl->stick_en = attr->stick_en;
    }
    return;
}

int bsp_espe_update_net(unsigned int portno, struct net_device *ndev, unsigned int alloced)
{
    return 0;
}
static int espe_alloc_port_desc(struct spe *spe, struct spe_port_ctrl *port_ctrl, unsigned int portno,
                                struct espe_port_comm_attr *attr)
{
    switch (attr->enc_type) {
        case SPE_ENC_IPF:
            spe->ipfport.portno = portno;
            spe->ipfport.alloced = 1;
            if (spe->cpuport.alloced) {
                spe->ports[spe->cpuport.portno].ctrl.bypassport = spe->ipfport.portno;
                spe->ports[spe->cpuport.portno].ctrl.bypassport_en = 1;
            }
            port_ctrl->ext_desc = spe->ipfport.extend_desc;
            break;
        case SPE_ENC_NONE:
        case SPE_ENC_DIRECT_FW_HP:
        case SPE_ENC_DIRECT_FW_MP:
        case SPE_ENC_DIRECT_FW_PE:
            break;
        case SPE_ENC_CPU:
            spe->wanport.portno = portno;
            spe->cpuport.portno = portno;
            spe->cpuport.alloced = 1;
            spe->wanport.get_wan_info = attr->ops.get_wan_info;
            if (spe->ipfport.alloced) {
                port_ctrl->bypassport = spe->ipfport.portno;
                port_ctrl->bypassport_en = 1;
            }
            break;
        case SPE_ENC_NCM_NTB16:
        case SPE_ENC_NCM_NTB32:
        case SPE_ENC_RNDIS:
        case SPE_ENC_ECM:
        case SPE_ENC_DIRECT_FW_NCM_NTB16:
        case SPE_ENC_DIRECT_FW_NCM_NTB32:
        case SPE_ENC_DIRECT_FW_RNDIS:
        case SPE_ENC_DIRECT_FW_ECM:
            if (spe->usbport.alloced) {
                return -EINVAL;
            }
            port_ctrl->port_flags.copy_port = 1;
            spe->usbport.portno = portno;
            spe->usbport.alloced = 1;
            break;
        default:
            return -EINVAL;
    }
    return espe_alloc_desc_queue(port_ctrl, attr);
}
static int espe_portdesc_init(struct spe *spe, unsigned int portno, struct espe_port_comm_attr *attr)
{
    struct spe_port_ctrl *port_ctrl = &spe->ports[portno].ctrl;
    int ret;

    port_ctrl->port_flags.copy_port = 0;
    port_ctrl->ext_desc = 0;

    ret = espe_alloc_port_desc(spe, port_ctrl, portno, attr);
    if (ret) {
        SPE_ERR("port %d alloc desc failed.\n", portno);
        return ret;
    }
    // td queue
    spe_writel_relaxed(spe->regs, SPE_TDQX_PRI(portno), SPE_DEF_PORT_PRI);
    spe_writel_relaxed(spe->regs, SPE_TDQX_BADDR_L(portno), lower_32_bits(port_ctrl->td_dma));
    spe_writel_relaxed(spe->regs, SPE_TDQX_BADDR_H(portno), upper_32_bits(port_ctrl->td_dma));
    spe_writel_relaxed(spe->regs, SPE_TDQX_LEN(portno), port_ctrl->td_depth);

    // rd queue
    spe_writel_relaxed(spe->regs, SPE_RDQX_BADDR_L(portno), lower_32_bits(port_ctrl->rd_dma));
    spe_writel_relaxed(spe->regs, SPE_RDQX_BADDR_H(portno), upper_32_bits(port_ctrl->rd_dma));
    spe_writel_relaxed(spe->regs, SPE_RDQX_LEN(portno), port_ctrl->rd_depth);

    return 0;
}

void espe_intr_set_interval(unsigned irq_interval)
{
    struct spe *spe = &g_espe;

    spe->irq_interval = irq_interval;
    spe_writel(spe->regs, SPE_INTA_INTERVAL, spe->irq_interval);
    spe->dfs_div_threshold_pktnum = SPE_PER_INTERVAL_CALC(ESPE_CLK_DIV_THR_RATE);
}

void espe_set_cpuport_maxrate(unsigned rate)
{
    struct spe *spe = &g_espe;

    spe->cpuport.cpu_pkt_max_rate = rate;
}

void espe_en_port_stick_mode(struct spe *spe, unsigned int bypassport)
{
    return;
}

void espe_port_set_stick_mode(struct spe *spe, struct spe_port_ctrl *port_ctrl, struct espe_port_comm_attr *attr,
                              unsigned int portno)
{
    if ((attr->enc_type == SPE_ENC_DIRECT_FW_HP) || (attr->enc_type == SPE_ENC_DIRECT_FW_MP) ||
        (attr->enc_type == SPE_ENC_DIRECT_FW_PE) || (attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB16) ||
        (attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB32) || (attr->enc_type == SPE_ENC_DIRECT_FW_RNDIS) ||
        (attr->enc_type == SPE_ENC_DIRECT_FW_ECM)) {
        port_ctrl->direct_fw_ctx.stream_cnt = 1;
        port_ctrl->direct_fw_ctx.enable = 1;
        port_ctrl->direct_fw_ctx.alloced = 1;
        port_ctrl->direct_fw_ctx.port_no = portno;
        if (spe->direct_fw.dfw_flags.enable == 0) {
            add_timer(&spe->direct_fw.direct_fw_timer);
        }
        spe->direct_fw.dfw_flags.enable = 1;
        INIT_LIST_HEAD(&port_ctrl->direct_fw_ctx.stream_list);

        spe->direct_fw.port_nums[spe->direct_fw.port_cnt] = portno;
        spe->direct_fw.port_cnt++;
    } else {
        port_ctrl->direct_fw_ctx.stream_cnt = 0;
        port_ctrl->direct_fw_ctx.enable = 0;
        port_ctrl->direct_fw_ctx.alloced = 0;
    }
        /* add by B5010 */

    return;
}
void espe_port_set_reg(struct spe *spe, struct spe_port_ctrl *port_ctrl, struct espe_port_comm_attr *attr,
                       unsigned int portno)
{
    unsigned int evt_rd_full_mask;
    unsigned int evt_rd_done_mask;
    unsigned int evt_td_done_mask;

    // when disable / enable port, rd may be not in start pos.
    // we update the free / busy slot from the hardware.
    // rd_busy must restore to rptr
    port_ctrl->td_free = spe_readl(spe->regs, SPE_TDQX_WPTR(portno));
    port_ctrl->td_busy = spe_readl(spe->regs, SPE_TDQX_RPTR(portno));
    // be care, rd_busy must restore to rptr
    port_ctrl->rd_busy = spe_readl(spe->regs, SPE_RDQX_RPTR(portno));
    port_ctrl->rd_free = port_ctrl->rd_busy;


    // set port property
    spe_writel_relaxed(spe->regs, SPE_PORTX_PROPERTY(portno), port_ctrl->property.u32);
    spe_writel_relaxed(spe->regs, SPE_TDQX_PRI(portno), port_ctrl->priority);

    if (attr->enc_type != SPE_ENC_IPF) {
        // set mac addr
        if (!((attr->enc_type >= SPE_ENC_NCM_NTB16 && attr->enc_type <= SPE_ENC_RNDIS) || attr->enc_type == SPE_ENC_ECM ||
               attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB16 || attr->enc_type == SPE_ENC_DIRECT_FW_NCM_NTB32 ||
               attr->enc_type == SPE_ENC_DIRECT_FW_RNDIS || attr->enc_type == SPE_ENC_DIRECT_FW_ECM)) {
            espe_port_set_mac(portno, port_ctrl->net->dev_addr);
        }

        evt_rd_done_mask = spe_readl(spe->regs, SPE_RD_DONE_INTA_MASK) & ~(1 << portno);
        spe_writel(spe->regs, SPE_RD_DONE_INTA_MASK, evt_rd_done_mask);
    }
    evt_rd_full_mask = spe_readl(spe->regs, SPE_RDQ_FULL_INTA_MASK) & ~(1 << portno);
    evt_td_done_mask = spe_readl(spe->regs, SPE_TD_DONE_INTA_MASK) & ~(1 << portno);

    spe_writel(spe->regs, SPE_TD_DONE_INTA_MASK, evt_td_done_mask);
    spe_writel(spe->regs, SPE_RDQ_FULL_INTA_MASK, evt_rd_full_mask);
    spe_writel(spe->regs, SPE_BUS_ERR_INTA_MASK, SPE_EVENT_DISABLE);
    espe_intr_set_interval(SPE_INTR_DEF_INTEVAL);

    spin_lock_init(&port_ctrl->td_lock);
    spin_lock_init(&port_ctrl->rd_lock);
    spin_lock_init(&port_ctrl->lock);
    port_ctrl->port_flags.alloced = 1;
    return;
}


int bsp_espe_alloc_port(struct espe_port_comm_attr *attr)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctx *port_ctx = NULL;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    unsigned long flag;

    spinlock_t *port_alloc_lock = &spe->port_alloc_lock;
    int ret;

    if (unlikely(spe->flags & SPE_FLAG_SUSPEND)) {
        SPE_ERR("port alloc during spe suspend! \n");
        return -EIO;
    }

    // check validity of attr param
    ret = espe_port_check_attr(attr);
    if (ret) {
        return ret;
    }

    spin_lock_irqsave(port_alloc_lock, flag);

    portno = espe_get_free_portno(attr);
    if ((portno < 0) || (portno >= SPE_PORT_NUM)) {
        spin_unlock_irqrestore(port_alloc_lock, flag);
        return -1;
    }

    spin_unlock_irqrestore(port_alloc_lock, flag);

    port_ctx = &spe->ports[portno];
    ret = memset_s(port_ctx, sizeof(*port_ctx), 0, sizeof(struct spe_port_ctx));
    if (ret) {
        SPE_ERR("memset_s fail!\n");
    }
    port_ctrl = &spe->ports[portno].ctrl;
    port_ctrl->portno = portno;

    espe_portctrl_init(spe, port_ctrl, attr);

    ret = espe_portdesc_init(spe, portno, attr);
    if (ret) {
        goto init_desc_fail;
    }

    espe_port_set_reg(spe, port_ctrl, attr, portno);
    espe_port_set_stick_mode(spe, port_ctrl, attr, portno);
    espe_get_port_type(portno, attr->enc_type);
    SPE_ERR("port alloc succ ! \n");

    return portno;

init_desc_fail:
    clear_bit(portno, &spe->portmap[SPE_PORTS_NOMARL]);
    SPE_ERR("port alloc fail! \n");
    BUG_ON(1);
    return ret;
}

int bsp_espe_free_port(int portno)
{
    return espe_port_free(portno);
}

int bsp_espe_set_portmac(int portno, const char *mac_addr)
{
    const char *mac = mac_addr;
    unsigned int port_num;
    int ret;

    if ((portno >= SPE_PORT_NUM) || (portno < 0)) {
        SPE_ERR("<bsp_espe_set_portmac>input portnum %d err \n", portno);
        return -EINVAL;
    }

    port_num = (unsigned int)portno;

    ret = espe_port_set_mac(port_num, mac);
    return ret;
}

static void espe_set_usb_pack_attr(struct spe *spe, struct espe_usbport_attr *usb_attr)
{
    spe_pack_ctrl_t usb_pack_ctrl;
    spe_unpack_max_len_t usb_unpack_len;

    // set default settings
    usb_pack_ctrl.bits.spe_pack_max_pkt_cnt = SPE_DEF_USB_ENC_PKT_CNT;
    usb_pack_ctrl.bits.spe_pack_max_len = (unsigned int)SPE_DEF_USB_MAX_SIZE;
    usb_unpack_len.bits.spe_ncm_max_len = (unsigned int)SPE_DEF_USB_MAX_SIZE;
    usb_unpack_len.bits.spe_rndis_max_len = (unsigned int)SPE_DEF_USB_MAX_SIZE;

    spe_writel_relaxed(spe->regs, SPE_UNPACK_MAX_LEN, usb_unpack_len.u32);
    spe_writel_relaxed(spe->regs, SPE_PACK_CTRL, usb_pack_ctrl.u32);
    spe_writel_relaxed(spe->regs, SPE_PACK_REGION_DIV, ESPE_GETHER_RD_DIV);
    spe_writel_relaxed(spe->regs, SPE_PACK_ADDR_CTRL, SPE_DEF_PACK_ALIGN);
    spe_writel_relaxed(spe->regs, SPE_UNPACK_CTRL, SPE_DEF_USB_UNPACK_MAX_SIZE);

    if (usb_attr->gether_align_parameter != 0) {
        spe->usbport.attr.gether_align_parameter = usb_attr->gether_align_parameter;
        spe_writel_relaxed(spe->regs, SPE_PACK_ADDR_CTRL, usb_attr->gether_align_parameter);
    }

    if (usb_attr->gether_max_pkt_cnt != 0) {
        spe->usbport.attr.gether_max_pkt_cnt = usb_attr->gether_max_pkt_cnt;
        usb_pack_ctrl.u32 = spe_readl(spe->regs, SPE_PACK_CTRL);
        usb_pack_ctrl.bits.spe_pack_max_pkt_cnt = usb_attr->gether_max_pkt_cnt;
        spe_writel_relaxed(spe->regs, SPE_PACK_CTRL, usb_pack_ctrl.u32);
    }

    if (usb_attr->gether_max_size != 0) {
        spe->usbport.attr.gether_max_size = usb_attr->gether_max_size;
        usb_unpack_len.bits.spe_ncm_max_len = usb_attr->gether_max_size;
        usb_unpack_len.bits.spe_rndis_max_len = usb_attr->gether_max_size;

        usb_pack_ctrl.u32 = spe_readl(spe->regs, SPE_PACK_CTRL);
        usb_pack_ctrl.bits.spe_pack_max_len = usb_attr->gether_max_size;

        spe_writel_relaxed(spe->regs, SPE_PACK_CTRL, usb_pack_ctrl.u32);
        spe_writel_relaxed(spe->regs, SPE_UNPACK_MAX_LEN, usb_unpack_len.u32);
    }

    if (usb_attr->gether_timeout != 0) {
        spe->usbport.attr.gether_timeout = usb_attr->gether_timeout;
        spe_writel_relaxed(spe->regs, SPE_PACK_MAX_TIME, usb_attr->gether_timeout);
    }
    return;
}
static int espe_check_usbproperty_para(struct spe *spe, struct espe_usbport_attr *usb_attr)
{
    if (usb_attr == NULL) {
        SPE_ERR("in put usb param err \n");
        return -EINVAL;
    }

    if (!spe->usbport.alloced) {
        SPE_ERR("no usb port err \n");
        return -EINVAL;
    }

    if (!usb_attr->net) {
        SPE_ERR("input net dev NULL.\n");
        return -EINVAL;
    }

    if ((usb_attr->enc_type < SPE_ENC_NCM_NTB16 || usb_attr->enc_type > SPE_ENC_RNDIS) &&
        (usb_attr->enc_type != SPE_ENC_ECM)) {
            SPE_ERR("port enc type not usb! \n");
            BUG_ON(1);
            return -EINVAL;
    }

    return 0;
}

int bsp_espe_set_usbproperty(struct espe_usbport_attr *usb_attr)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    int ret;

    ret = espe_check_usbproperty_para(spe, usb_attr);
    if (ret) {
        return ret;
    }

    portno = spe->usbport.portno;
    port_ctrl = &spe->ports[portno].ctrl;

    dev_hold(usb_attr->net);
    port_ctrl->net = usb_attr->net;
    spe->usbport.attr.net = usb_attr->net;
    dev_put(usb_attr->net);

    ret = espe_port_set_mac(portno, port_ctrl->net->dev_addr);
    if (ret) {
        SPE_ERR("espe_port_set_mac fail \n");
    }

    spe->usbport.attr.enc_type = usb_attr->enc_type;
    if (usb_attr->enc_type == SPE_ENC_ECM) {
        port_ctrl->property.bits.spe_port_enc_type = SPE_ENC_NONE;
    } else {
        port_ctrl->property.bits.spe_port_enc_type = usb_attr->enc_type;
    }
    spe_writel(spe->regs, SPE_PORTX_PROPERTY(portno), port_ctrl->property.u32);

    espe_set_usb_pack_attr(spe, usb_attr);

    return 0;
}

int bsp_espe_set_ipfproperty(struct espe_ipfport_attr *attr)
{
    struct spe *spe = &g_espe;
    struct iport_spe_addr *espe_reg = NULL;
    struct iport_ipf_addr *eipf_reg = NULL;
    unsigned int portno;

    if (attr == NULL) {
        SPE_ERR("in put ipf param err \n");
        return -EINVAL;
    }

    spe->ipfport.check_ipf_bd = attr->check_ipf_bd;
    espe_reg = &attr->spe_addr;
    eipf_reg = &attr->ipf_addr;

    portno = spe->ipfport.portno;
    if (!spe->ipfport.alloced) {
        SPE_ERR("eipf ctx port NULL. \n");
        return -1;
    }

    if ((espe_reg == NULL) && (eipf_reg == NULL)) {
        SPE_ERR("eipf ctx input error. \n");
        return -EINVAL;
    }

    spe->ipfport.spe_push_addr = (char *)dma_alloc_coherent(spe->dev, sizeof(unsigned int), &spe->ipfport.spe_push_dma,
                                                            GFP_KERNEL);

    if (spe->ipfport.spe_push_addr == NULL) {
        SPE_ERR("eipf spe_push_addr alloc err. \n");
        return -1;
    }

    espe_reg->dltd_base_addr = spe->ports[portno].ctrl.td_dma;
    espe_reg->ulrd_base_addr = spe->ports[portno].ctrl.rd_dma;
    espe_reg->dltd_base_addr_v = spe->ports[portno].ctrl.td_addr;
    espe_reg->ulrd_base_addr_v = spe->ports[portno].ctrl.rd_addr;
    espe_reg->ulrd_rptr_addr = spe->res + SPE_RDQX_RPTR(portno);
    espe_reg->dltd_wptr_addr = spe->res + SPE_TDQX_WPTR(portno);
    espe_reg->ulrd_rptr_addr_v = (unsigned char *)spe->regs + SPE_RDQX_RPTR(portno);
    espe_reg->dltd_wptr_addr_v = (unsigned char *)spe->regs + SPE_TDQX_WPTR(portno);
    attr->ext_desc_en = spe->ipfport.extend_desc;

    spe_writel(spe->regs, SPE_IPF_RDQ_WPTR_UPDATE_ADDR_L, lower_32_bits(eipf_reg->ulrd_wptr_addr));
    spe_writel(spe->regs, SPE_IPF_RDQ_WPTR_UPDATE_ADDR_H, upper_32_bits(eipf_reg->ulrd_wptr_addr));

    if (spe->spe_version == ESPE_VER_V200) {
        spe->ipfport.ipf_rd_rptr_addr = (void *)(uintptr_t)(eipf_reg->dltd_rptr_vir_addr);
        spe_writel(spe->regs, IPF_SPE_BDQ_RPTR_UPDATE_ADDR_L, lower_32_bits(spe->ipfport.spe_push_dma));
        spe_writel(spe->regs, IPF_SPE_BDQ_RPTR_UPDATE_ADDR_H, upper_32_bits(spe->ipfport.spe_push_dma));
    } else {
        spe->ipfport.ipf_rd_rptr_addr = (void *)(uintptr_t)(eipf_reg->dltd_rptr_phy_addr);
        spe_writel(spe->regs, IPF_SPE_BDQ_RPTR_UPDATE_ADDR_L, (unsigned int)(uintptr_t)spe->ipfport.ipf_rd_rptr_addr);
    }

    SPE_ERR("ipf port property set complete \n");

    return 0;
}

void bsp_espe_set_ipfmac(struct ethhdr *mac_addr)
{
    if (mac_addr == NULL) {
        return;
    }

    espe_set_ipf_mac(mac_addr);

    return;
}

int espe_enable_port(int portno)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctrl *port_ctrl = NULL;
    spe_port_prop_t property;
    unsigned long flags = 0;

    if (!espe_port_exist(spe, portno)) {
        return -ENOENT;
    }

    port_ctrl = &spe->ports[portno].ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);

    if (port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        SPE_DBG("port %d is enabled\n", portno);
        return 0;
    }

    property.u32 = spe_readl(spe->regs, SPE_PORTX_PROPERTY(portno));
    property.bits.spe_port_en = 1;
    spe->ports[portno].ctrl.property.u32 = property.u32;
    spe_writel(spe->regs, SPE_PORTX_PROPERTY(portno), property.u32);
    spe_writel_relaxed(spe->regs, SPE_TDQX_PRI(portno), SPE_DEF_PORT_PRI);

    port_ctrl->port_flags.enable = 1;

    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    return 0;
}

int bsp_espe_enable_port(int portno)
{
    SPE_INFO("port %d enable called\n", portno);
    return espe_enable_port(portno);
}

int espe_disable_port(struct spe *spe, int port_num)
{
    struct spe_port_ctrl *port_ctrl = NULL;
    spe_port_prop_t property;
    unsigned long flags = 0;
    unsigned int portno;

    SPE_TRACE("enter\n");

    if (port_num < 0) {
        SPE_ERR("spe port %d err\n", port_num);
        return -EINVAL;
    } else {
        portno = (unsigned int)port_num;
    }

    if (!espe_port_exist(spe, portno)) {
        SPE_ERR("spe port %d not exist\n", port_num);
        return -ENOENT;
    }

    port_ctrl = &spe->ports[portno].ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);
    if (!port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        SPE_DBG("port %u is disabled\n", portno);
        return 0;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    property.u32 = spe_readl(spe->regs, SPE_PORTX_PROPERTY(portno));
    property.bits.spe_port_en = 0;
    port_ctrl->property.u32 = property.u32;
    spe_writel(spe->regs, SPE_PORTX_PROPERTY(portno), property.u32);
    port_ctrl->port_flags.enable = 0;

    SPE_TRACE("leave\n");

    return 0;
}

int bsp_espe_disable_port(int port_num)
{
    struct spe *spe = &g_espe;
    int ret;

    SPE_INFO("port %d disable called\n", port_num);
    ret = espe_disable_port(spe, port_num);
    if (ret) {
        SPE_ERR("spe disable port %d fail.\n", port_num);
        return ret;
    }

    ret = espe_recycle_port_queue_desc(spe, port_num);
    if (ret) {
        SPE_ERR("spe recycle port:%d queue desc fail.\n", port_num);
        return ret;
    }

    return 0;
}


void espe_set_bypass(unsigned int port_num, unsigned int bypass_dir, unsigned int en)
{
    struct spe *spe = &g_espe;
    if (port_num >= SPE_PORT_NUM) {
        SPE_ERR("wrong spe port \n");
        return;
    }
    spe->ports[port_num].ctrl.bypassport_en = en;
    spe->ports[port_num].ctrl.bypassport = bypass_dir;
}

void spe_print_packet_stream(void)
{
    struct spe_port_ctrl *ctrl = NULL;
    struct spe_port_stat *stat = NULL;
    int i;

    for (i = 0; i < SPE_PORT_NUM; i++) {
        ctrl = &g_espe.ports[i].ctrl;
        stat = &g_espe.ports[i].stat;
        SPE_ERR("port[i]:%d ", ctrl->portno);
    }
    SPE_ERR("\ntd: \n");

    for (i = 0; i < SPE_PORT_NUM; i++) {
        stat = &g_espe.ports[i].stat;
        SPE_ERR("%d ", stat->rd_config);
    }
    SPE_ERR("\ntd complete: \n");

    for (i = 0; i < SPE_PORT_NUM; i++) {
        stat = &g_espe.ports[i].stat;
        SPE_ERR("%d ", stat->td_finsh_intr_complete);
    }
    SPE_ERR("\nrd complete: \n");

    for (i = 0; i < SPE_PORT_NUM; i++) {
        stat = &g_espe.ports[i].stat;
        SPE_ERR("%d ", stat->td_config);
    }
    SPE_ERR("\nrd: \n");

    for (i = 0; i < SPE_PORT_NUM; i++) {
        stat = &g_espe.ports[i].stat;
        SPE_ERR("%d ", stat->rd_finished);
    }
    SPE_ERR("\n");

    return;
}

void espe_port_init(struct spe *spe)
{
    /* Rsv Port for cpu/ipf/ndis ctrl */
    spe->portmap[SPE_PORTS_NOMARL] = 0x7;

    spe->rd_loop_cnt = ESPE_RD_MAX_GET_TIMES;

    spin_lock_init(&spe->port_alloc_lock);

    return;
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(bsp_espe_alloc_port);
EXPORT_SYMBOL(bsp_espe_disable_port);
EXPORT_SYMBOL(bsp_espe_enable_port);
EXPORT_SYMBOL(bsp_espe_set_usbproperty);

EXPORT_SYMBOL(espe_en_port_stick_mode);
EXPORT_SYMBOL(espe_set_bypass);

