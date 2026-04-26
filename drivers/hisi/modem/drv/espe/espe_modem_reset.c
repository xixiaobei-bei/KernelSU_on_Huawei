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

#include "bsp_reset.h"

#include "hal/espe_hal_desc.h"
#include "hal/espe_hal_reg.h"

#include "espe.h"
#include "espe_dbg.h"
#include "espe_modem_reset.h"
#include "espe_port.h"
#include "espe_desc.h"

#define ESPE_PHONE_CRG_BASE 0x40285000
#define ESPE_PHONE_CRG_LEN 0x1000

struct espe_queue_stash g_espe_queue;
unsigned int g_espe_reg[SPE_GENERAL_REG_NUM][STASH_REG_BUTTON] = {
    // spe globe
    { SPE_BAND, 0 },
    { SPE_BLACK_WHITE, 0 },
    { SPE_PORT_LMTTIME, 0 },
    { SPE_ETH_MINLEN, 0 },
    { SPE_ETH_MAXLEN, 0 },
    { SPE_GLB_DMAC_ADDR_L, 0 },
    { SPE_GLB_DMAC_ADDR_H, 0 },
    { SPE_GLB_SMAC_ADDR_L, 0 },
    { SPE_GLB_SMAC_ADDR_H, 0 },
    { SPE_IPF_BD_RD_IP_HEAD, 0 },

    // spe done
    { SPE_RDQ_FULL_LEVEL, 0 },
    { SPE_ADQ_EMPTY_LEVEL, 0 },
    { SPE_RDQ_FULL_INTA_MASK, 0 },
    { SPE_RD_DONE_INTA_MASK, 0 },
    { SPE_TD_DONE_INTA_MASK, 0 },
    { SPE_ADQ_EMPTY_INTA_MASK, 0 },
    { SPE_RDQ_FULL_INTC_MASK, 0 },
    { SPE_RD_DONE_INTC_MASK, 0 },
    { SPE_TD_DONE_INTC_MASK, 0 },
    { SPE_ADQ_EMPTY_INTC_MASK, 0 },
    { SPE_BUS_ERR_INTA_MASK, 0 },
    { SPE_BUS_ERR_INTC_MASK, 0 },
    { SPE_INTA_INTERVAL, 0 },
    { SPE_INTC_INTERVAL, 0 },

    // spe eng(hash)
    { SPE_HASH_BADDR_L, 0 },
    { SPE_HASH_BADDR_H, 0 },
    { SPE_HASH_ZONE, 0 },
    { SPE_HASH_RAND, 0 },
    { SPE_HASH_L3_PROTO, 0 },
    { SPE_HASH_WIDTH, 0 },
    { SPE_HASH_DEPTH, 0 },
    { SPE_AGING_TIME, 0 },
    { SPE_ACK_IDEN_PORT, 0 },
    { SPE_ACK_IDEN_EN, 0 },
    { SPE_GMAC_PADDING_IDEN_EN, 0 },
    { SPE_HASH_CACHE_EN, 0 },

    // spe pack
    { SPE_PACK_MAX_TIME, 0 },
    { SPE_PACK_CTRL, 0 },
    { SPE_PACK_ADDR_CTRL, 0 },
    { SPE_PACK_REMAIN_LEN, 0 },
    { SPE_UNPACK_CTRL, 0 },
    { SPE_UNPACK_MAX_LEN, 0 },
    { SPE_PACK_REGION_DIV, 0 },

    { SPE_IPF_RDQ_WPTR_UPDATE_ADDR_L, 0 },
    { SPE_IPF_RDQ_WPTR_UPDATE_ADDR_H, 0 },
    { IPF_SPE_BDQ_RPTR_UPDATE_ADDR_L, 0 },
    { IPF_SPE_BDQ_RPTR_UPDATE_ADDR_H, 0 },

    // spe ad
    { SPE_ADQ_BADDR_L(0), 0 },
    { SPE_ADQ_BADDR_H(0), 0 },
    { SPE_ADQ_BADDR_L(1), 0 },
    { SPE_ADQ_BADDR_H(1), 0 },
    { SPE_ADQ_SIZE(0), 0 },
    { SPE_ADQ_SIZE(1), 0 },
    { SPE_ADQ_LEN(0), 0 },
    { SPE_ADQ_LEN(1), 0 },
    { SPE_ADQ_RPTR_UPDATE_ADDR_L(0), 0 },
    { SPE_ADQ_RPTR_UPDATE_ADDR_H(0), 0 },
    { SPE_ADQ_RPTR_UPDATE_ADDR_L(1), 0 },
    { SPE_ADQ_RPTR_UPDATE_ADDR_H(1), 0 },
    { SPE_ADQ_THRESHOLD, 0 }
};

static void espe_queue_registers_save(struct spe *espe_ctx)
{
    int portno;
    struct espe_queue_stash *queue_regs = &g_espe_queue;
    for (portno = 0; portno < SPE_CCORE_PORT_BEGIN; portno++) {
        // spe queue
        queue_regs->spe_tdqx_baddr_l[portno] = spe_readl(espe_ctx->regs, SPE_TDQX_BADDR_L(portno));
        queue_regs->spe_tdqx_baddr_h[portno] = spe_readl(espe_ctx->regs, SPE_TDQX_BADDR_H(portno));
        queue_regs->spe_tdqx_len[portno] = spe_readl(espe_ctx->regs, SPE_TDQX_LEN(portno));
        queue_regs->spe_rdqx_baddr_l[portno] = spe_readl(espe_ctx->regs, SPE_RDQX_BADDR_L(portno));
        queue_regs->spe_rdqx_baddr_h[portno] = spe_readl(espe_ctx->regs, SPE_RDQX_BADDR_H(portno));
        queue_regs->spe_rdqx_len[portno] = spe_readl(espe_ctx->regs, SPE_RDQX_LEN(portno));
        queue_regs->spe_tdqx_pri[portno] = spe_readl(espe_ctx->regs, SPE_TDQX_PRI(portno));

        // spe ports
        queue_regs->spe_portx_udp_lmtnum[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_UDP_LMTNUM(portno));
        queue_regs->spe_portx_lmtbyte[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_LMTBYTE(portno));
        queue_regs->spe_portx_property[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_PROPERTY(portno));
        queue_regs->spe_portx_mac_addr_l[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_MAC_ADDR_L(portno));
        queue_regs->spe_protx_mac_addr_h[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_MAC_ADDR_H(portno));
        queue_regs->spe_portx_ipv6_addr_1s[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_1ST(portno));
        queue_regs->spe_portx_ipv6_addr_2n[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_2ND(portno));
        queue_regs->spe_portx_ipv6_addr_3t[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_3TH(portno));
        queue_regs->spe_portx_ipv6_addr_4t[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_4TH(portno));
        queue_regs->spe_portx_ipv6_mask[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV6_MASK(portno));
        queue_regs->spe_portx_ipv4_addr[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV4_ADDR(portno));
        queue_regs->spe_portx_ipv4_mask[portno] = spe_readl(espe_ctx->regs, SPE_PORTX_IPV4_MASK(portno));
    }
}

static void espe_queue_registers_restore(struct spe *espe_ctx)
{
    int portno;
    struct espe_queue_stash *queue_regs = &g_espe_queue;

    for (portno = 0; portno < SPE_CCORE_PORT_BEGIN; portno++) {
        // spe queue
        spe_writel(espe_ctx->regs, SPE_TDQX_BADDR_L(portno), queue_regs->spe_tdqx_baddr_l[portno]);
        spe_writel(espe_ctx->regs, SPE_TDQX_BADDR_H(portno), queue_regs->spe_tdqx_baddr_h[portno]);
        spe_writel(espe_ctx->regs, SPE_TDQX_LEN(portno), queue_regs->spe_tdqx_len[portno]);
        spe_writel(espe_ctx->regs, SPE_RDQX_BADDR_L(portno), queue_regs->spe_rdqx_baddr_l[portno]);
        spe_writel(espe_ctx->regs, SPE_RDQX_BADDR_H(portno), queue_regs->spe_rdqx_baddr_h[portno]);
        spe_writel(espe_ctx->regs, SPE_RDQX_LEN(portno), queue_regs->spe_rdqx_len[portno]);
        spe_writel(espe_ctx->regs, SPE_TDQX_PRI(portno), queue_regs->spe_tdqx_pri[portno]);

        // spe ports
        spe_writel(espe_ctx->regs, SPE_PORTX_UDP_LMTNUM(portno), queue_regs->spe_portx_udp_lmtnum[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_LMTBYTE(portno), queue_regs->spe_portx_lmtbyte[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_PROPERTY(portno), queue_regs->spe_portx_property[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_MAC_ADDR_L(portno), queue_regs->spe_portx_mac_addr_l[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_MAC_ADDR_H(portno), queue_regs->spe_protx_mac_addr_h[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_1ST(portno), queue_regs->spe_portx_ipv6_addr_1s[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_2ND(portno), queue_regs->spe_portx_ipv6_addr_2n[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_3TH(portno), queue_regs->spe_portx_ipv6_addr_3t[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV6_ADDR_4TH(portno), queue_regs->spe_portx_ipv6_addr_4t[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV6_MASK(portno), queue_regs->spe_portx_ipv6_mask[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV4_ADDR(portno), queue_regs->spe_portx_ipv4_addr[portno]);
        spe_writel(espe_ctx->regs, SPE_PORTX_IPV4_MASK(portno), queue_regs->spe_portx_ipv4_mask[portno]);
    }
}

/*
 * save espe registers, this function suppose to be called when
 *    1.All spe port disabled in ap/cp;
 *    2.AD queue disabled and cleaned in m3;
 *    3.spe en disabled in m3
 */
static inline void espe_registers_save(struct spe *espe_ctx)
{
    int i;

    for (i = 0; i < SPE_GENERAL_REG_NUM; i++) {
        g_espe_reg[i][STASH_REG_VALUE] = spe_readl(espe_ctx->regs, g_espe_reg[i][STASH_REG_ADDR]);
    }
}

static inline void espe_registers_restore(struct spe *espe_ctx)
{
    int i;

    for (i = 0; i < SPE_GENERAL_REG_NUM; i++) {
        spe_writel(espe_ctx->regs, g_espe_reg[i][STASH_REG_ADDR], g_espe_reg[i][STASH_REG_VALUE]);
    }

    // rptr can only be write while debug mode on
    spe_writel(espe_ctx->regs, SPE_DBGEN, 1);
    spe_writel(espe_ctx->regs, SPE_DBGEN, 0);
}

static inline void espe_dis_ad(struct spe *espe_ctx)
{
    // disable
    spe_writel(espe_ctx->regs, SPE_ADQ_EN(0), 0x0);
    spe_writel(espe_ctx->regs, SPE_ADQ_EN(1), 0x0);

    /*
     * clean ad cache,
     * *for espe will pre-read ad to espe ad cache,
     * *and espe use ad in cache instead of those AD queue
     * *if we do not AD clean cache before restore registers
     * *espe will use AD in cache and then pre-read AD queue
     * *and use the same AD again, cause the same AD get used twice
     */
    spe_writel(espe_ctx->regs, SPE_ADQ_BUF_FLUSH(0), 0x1);
    spe_writel(espe_ctx->regs, SPE_ADQ_BUF_FLUSH(1), 0x1);
}

static void espe_en_ad(struct spe *espe_ctx)
{
    struct spe_adq_ctrl *cur_ctrl = NULL;
    enum spe_adq_num adq_num;
    dma_addr_t maa_rptr_reg_addr;
    dma_addr_t ad_wptr_reg_addr;

    for (adq_num = SPE_ADQ0; adq_num < SPE_ADQ_BOTTOM; adq_num++) {
        cur_ctrl = &espe_ctx->adqs_ctx[adq_num].ctrl;
        if (cur_ctrl->ad_base_addr == NULL) {
            SPE_ERR("modem reset adq desc pool NULL.\n");
            BUG_ON(1);
            return;
        }

        if (espe_ctx->spe_version == ESPE_VER_V200 || espe_ctx->spe_version == ESPE_VER_V300) {  // evide maa hard error
            maa_rptr_reg_addr = (dma_addr_t)bsp_maa_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                cur_ctrl->maa_wptr_stub_dma_addr, SPE_ADQ_DEF_SIZE,
                                                                espe_ctx->adqs_ctx[adq_num].ctrl.maa_ipip_type);
        } else {
            ad_wptr_reg_addr = espe_ctx->res + SPE_ADQ_WPTR(adq_num);
            maa_rptr_reg_addr = (dma_addr_t)bsp_maa_set_adqbase((unsigned long long)cur_ctrl->ad_dma_addr,
                                                                lower_32_bits(ad_wptr_reg_addr), SPE_ADQ_DEF_SIZE,
                                                                espe_ctx->adqs_ctx[adq_num].ctrl.maa_ipip_type);
        }
    }
}

// this may cause spe send a ad empty interrupt to ap;
// maybe we just disable ad empty interrupt as default;
static void espe_drx_restore_reg(struct spe *espe_ctx)
{
    espe_registers_restore(espe_ctx);
    espe_queue_registers_restore(espe_ctx);
}

int espe_en(struct spe *espe_ctx)
{
    spe_en_t value;

    value.u32 = spe_readl(espe_ctx->regs, SPE_EN);
    value.bits.spe_en = 1;
    spe_writel(espe_ctx->regs, SPE_EN, value.u32);

    return 0;
}

int espe_dis(struct spe *espe_ctx)
{
    spe_en_t value;

    value.u32 = spe_readl(espe_ctx->regs, SPE_EN);
    value.bits.spe_en = 0;
    spe_writel(espe_ctx->regs, SPE_EN, value.u32);

    return 0;
}

void espe_reset_modem(void)
{
    struct spe *espe_ctx = &g_espe;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    // only deal acore port
    for (portno = SPE_CCORE_PORT_BEGIN - 1; portno >= 0 && portno < SPE_CCORE_PORT_BEGIN; portno--) {
        port_ctrl = &espe_ctx->ports[portno].ctrl;
        port_ctrl->port_flags.enable_after_modem_reset = port_ctrl->port_flags.enable;
        if (port_ctrl->port_flags.enable) {
            espe_disable_port(espe_ctx, portno);
        }
    }

    espe_dis(espe_ctx);
    espe_dis_ad(espe_ctx);
    espe_queue_registers_save(espe_ctx);
    espe_registers_save(espe_ctx);

    espe_ctx->modem_reset_count++;
}

void espe_unreset_modem(void)
{
    struct spe *espe_ctx = &g_espe;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned int portno;

    // reset espe
    spe_writel(espe_ctx->peri.crg_addr, espe_ctx->peri.crg_rst_en, espe_ctx->peri.crg_rst_mask);
    mdelay(10);
    spe_writel(espe_ctx->peri.crg_addr, espe_ctx->peri.crg_rst_dis, espe_ctx->peri.crg_rst_mask);
    mdelay(10);

    // restore registers
    espe_drx_restore_reg(espe_ctx);
    espe_en_ad(espe_ctx);

    for (portno = 0; portno < SPE_CCORE_PORT_BEGIN; portno++) {
        port_ctrl = &espe_ctx->ports[portno].ctrl;
        port_ctrl->rd_free = 0;
        port_ctrl->rd_busy = 0;
        port_ctrl->td_free = 0;
        port_ctrl->td_busy = 0;
        if (port_ctrl->port_flags.enable_after_modem_reset || portno == ESPE_DEFAULT_IPF_PORT ||
            portno == ESPE_DEFAULT_CPU_PORT) {
            espe_enable_port(portno);
        }
        port_ctrl->port_flags.enable_after_modem_reset = 0;
    }

    espe_en(espe_ctx);
    espe_ctx->modem_unreset_count++;

    return;
}

int espe_reset_modem_cb(drv_reset_cb_moment_e eparam, int userdata)
{
    struct spe *espe_ctx = &g_espe;
    unsigned long flags = 0;

    SPE_ERR("espe_reset_modem_cb %d \n", eparam);

    if (eparam == MDRV_RESET_CB_BEFORE) {
        if (espe_ctx->modem_unreset_count != espe_ctx->modem_reset_count) {
            SPE_ERR("unblanced call espe modem reset!ureset time %u,reset time %u \n", espe_ctx->modem_unreset_count,
                    espe_ctx->modem_reset_count);
            espe_bug(espe_ctx);
            return 0;
        }
        spin_lock_irqsave(&espe_ctx->reset_lock, flags);
        espe_ctx->modem_resetting = 1;
        spin_unlock_irqrestore(&espe_ctx->reset_lock, flags);
        espe_reset_modem();
    } else if (eparam == MDRV_RESET_CB_AFTER) {
        if (espe_ctx->modem_unreset_count != (espe_ctx->modem_reset_count - 1)) {
            SPE_ERR("unblanced call espe modem unreset!ureset time %u,reset time %u \n", espe_ctx->modem_unreset_count,
                    espe_ctx->modem_reset_count);
            espe_bug(espe_ctx);
            return 0;
        }
        espe_unreset_modem();
        spin_lock_irqsave(&espe_ctx->reset_lock, flags);
        espe_ctx->modem_resetting = 0;
        spin_unlock_irqrestore(&espe_ctx->reset_lock, flags);
    } else {
        espe_ctx->modem_noreset_count++;
        SPE_ERR("drv_reset_cb_moment_e is neither before or after !\n");
        espe_bug(espe_ctx);

        return -EINVAL;
    }

    return 0;
}

void espe_reset_modem_init(void)
{
    int ret;
    struct spe *espe_ctx = &g_espe;

    espe_ctx->modem_reset_count = 0;
    espe_ctx->modem_unreset_count = 0;
    espe_ctx->modem_noreset_count = 0;

    ret = bsp_reset_cb_func_register("Eg_espe", espe_reset_modem_cb, 0, DRV_RESET_CB_PIOR_ESPE);
    if (ret != 0) {
        SPE_ERR("Set modem reset call back func failed\n");
    }

    return;
}
