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

#include <linux/neighbour.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/types.h>

#ifdef SPEV300_SUPPORT
#include "espe_hal_reg_v300.h"
#else
#include "espe_hal_reg_v200.h"
#endif

#define SPE_MAA

#ifndef __SPE_REG_H__
#define __SPE_REG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ESPE_VER_V200 (0x32303061)
#define ESPE_VER_V300 (0x33303061) /* add by B5010 */

#define CLKEN_SEL 0x00AC

#define ESPE_GT_CLK 0x470
#define ESPE_GT_CLK_STAT 0x478
#define ESPE_GT_CLK_FINIAL_STAT 0x47c
#define ESPE_GT_CLK_MASK 0x38000  // bit 15 16 17

#define ESPE_CLK_DIV_THR_RATE 900  // max rate while clk_div = 16
#define ESPE_CLK_DIV_FULL_MIN_LEVEL 0x0
#define ESPE_CLK_DIV_FULL_RATE_LEVEL 0x1

#ifdef CONFIG_ESPE_PHONE_SOC
#define ESPE_CLK_DIV 0x710
#define ESPE_CLK_DIV_DEFAULT 0xF00000

typedef union {
    struct {
        unsigned int reserved_div_0 : 4;
        unsigned int spe_div : 4;
        unsigned int reserved_div_1 : 8;
        unsigned int reserved_en_0 : 4;
        unsigned int spe_en : 4;
        unsigned int reserved_2 : 8;
    } bits;
    unsigned int u32;
} spe_div_ctrl_t;
#else
#define ESPE_CLK_DIV 0x18C
#define ESPE_CLK_DIV_DEFAULT 0x8F00A032
typedef union {
    struct {
        unsigned int ipf_div : 4;
        unsigned int seceng_div : 4;
        unsigned int spe_div : 4;
        unsigned int ipf_refclk_sel : 1;
        unsigned int seceng_refclk_sel : 2;
        unsigned int spe_refclk_sel : 1;
        unsigned int bitmasken : 16;
    } bits;
    unsigned int u32;
} spe_div_ctrl_t;
#endif

#define SPE_MAC_TAB_NUMBER (128)
#define SPE_MAC_TAB_WORD_NO (4)
#define SPE_IPV4_TAB_NUMBER (16)
#define SPE_IPV4_TAB_WORD_NO (6)
#define SPE_IPV6_TAB_NUMBER (8)
#define SPE_IPV6_TAB_WORD_NO (12)
#define MAX_L4_PORTNUM_CFG_NUM (16)
#define SPE_QOS_MAC_TAB_NUMBER (32) /* add by B5010 */
#define SPE_QOS_MAC_TAB_WORD_NO (3) /* add by B5010 */

#define ESPE_VER_V200 (0x32303061)
#define ESPE_VER_V300 (0x33303061)
#define SPE_REG_BASE (0xE5004000)
#define SPE_REG_SIZE (0x4000)
#define SPE_IRQ (215)

#define SPE_EN 0x0000
#define SPE_BAND 0x0008
#define SPE_BLACK_WHITE 0x000C
#define SPE_PORT_LMTTIME 0x0010
#define SPE_ETH_MINLEN 0x0014
#define SPE_ETH_MAXLEN 0x0018
#define SPE_GLB_DMAC_ADDR_L 0x0020
#define SPE_GLB_DMAC_ADDR_H 0x0024
#define SPE_GLB_SMAC_ADDR_L 0x0028
#define SPE_GLB_SMAC_ADDR_H 0x002C
#define SPE_L4_PORTNUM_CFG(x) (0x0040 + 0x4 * (x))
#define SPE_CNT_CLR 0x0080
#define SPE_VER 0x009C
#define SPE_IDLE 0x00A4

#define SPE_TIME_MAX 0xFFFF
#define SPE_IDLE_DEF_VAL 0x1FFFF

#define SPE_OST_CTRL 0x0100
#define SPE_BURST_CTRL 0x0104
#define SPE_WCH_ID 0x0108
#define SPE_RCH_ID 0x010C
#define SPE_WCH_PRI 0x0110
#define SPE_RCH_PRI 0x0114

#define SPE_RDQ_FULL_INTA_STATUS 0x0200
#define SPE_RD_DONE_INTA_STATUS 0x0204
#define SPE_TD_DONE_INTA_STATUS 0x0208
#define SPE_ADQ_EMPTY_INTA_STATUS 0x020C
#define SPE_RDQ_FULL_LEVEL 0x0210
#define SPE_ADQ_EMPTY_LEVEL 0x0214
#define SPE_RDQ_FULL_INTA_MASK 0x0218
#define SPE_RD_DONE_INTA_MASK 0x021C
#define SPE_TD_DONE_INTA_MASK 0x0220
#define SPE_ADQ_EMPTY_INTA_MASK 0x0224
#define SPE_RDQ_FULL_INTC_MASK 0x0228
#define SPE_RD_DONE_INTC_MASK 0x022C
#define SPE_TD_DONE_INTC_MASK 0x0230
#define SPE_ADQ_EMPTY_INTC_MASK 0x0234
#define SPE_RDQ_FULL_INTA 0x0238
#define SPE_RD_DONE_INTA 0x023C
#define SPE_TD_DONE_INTA 0x0240
#define SPE_ADQ_EMPTY_INTA 0x0244
#define SPE_RDQ_FULL_INTC 0x0248
#define SPE_RD_DONE_INTC 0x024C
#define SPE_TD_DONE_INTC 0x0250
#define SPE_ADQ_EMPTY_INTC 0x0254
#define SPE_INTA_INTERVAL 0x0258
#define SPE_INTC_INTERVAL 0x025C
#define SPE_BUS_ERR_INTA_STATUS 0x0260
#define SPE_BUS_ERR_INTA_MASK 0x0264
#define SPE_BUS_ERR_INTC_MASK 0x0268
#define SPE_BUS_ERR_INTA 0x026C
#define SPE_BUS_ERR_INTC 0x0270
#define HASH_CACHE_VALX(x) (0x0300 + 0x4 * (x))

#define SPE_TAB_CTRL 0x0500

#define SPE_TAB_CTRL_RM_HCACHE 0x8

#define SPE_TAB_ACT_RESULT 0x0504
#define SPE_TAB_CONTENT(x) (0x0508 + 0x4 * (x))
#define SPE_HASH_BADDR_L 0x057C
#define SPE_HASH_BADDR_H 0x0580
#define SPE_HASH_ZONE 0x0584
#define SPE_HASH_RAND 0x0588
#define SPE_HASH_L3_PROTO 0x058C
#define SPE_HASH_WIDTH 0x0590
#define SPE_HASH_DEPTH 0x0594
#define SPE_AGING_TIME 0x05A0
#define SPE_TAB_CHK_MODE 0x05A4
#define SPE_TAB_CLR 0x05A8
#define SPE_HASH_VALUE 0x05B0
#define SPE_ACK_IDEN_PORT 0x05B4
#define SPE_ACK_IDEN_EN 0x05B8
#define SPE_GMAC_PADDING_IDEN_EN 0x05BC
#define SPE_HASH_CACHE_EN 0x05C0

#define SPE_PACK_MAX_TIME 0x0600
#define SPE_PACK_CTRL 0x0604
#define SPE_PACK_ADDR_CTRL 0x0608
#define SPE_PACK_REMAIN_LEN 0x060C
#define SPE_UNPACK_CTRL 0x0610
#define SPE_UNPACK_MAX_LEN 0x0614
#define SPE_PACK_REGION_DIV 0x0618

#define SPE_WRR_DEBUG 0x0700
#define SPE_UNPACK_DEBUG 0x0710
#define SPE_ENG_DEBUG 0x0720
#define SPE_PACK_DEBUG 0x0730
#define SPE_DONE_DEBUG 0x0740
#define HAC_GIF_DEBUG 0x0750
#define SPE_LP_RD_BLOCK_CNT 0x0780
#define SPE_HP_RD_BLOCK_CNT 0x0784
#define SPE_PACK_OVERTIME_CNT 0x0788
#define SPE_PACK_PUSH_CNT 0x078C
#define SPE_DEPACK_ERR_CNT 0x0790
#define SPE_LP_RD_BLOCK_CNT0 0x0794
#define SPE_HP_RD_BLOCK_CNT1 0x0798
#define SPE_ROUTE_INFO 0x079C
#define SPE_TDQ_STATUS 0x07A0
#define SPE_RDQ_STATUS 0x07A4
#define SPE_ADQ_STATUS 0x07A8

#define SPE_RDQ_STATUS_EMPYT 0xFFFF0000

#define SPE_TDQX_BADDR_L(x) (0x2800 + 0x40 * (x))
#define SPE_TDQX_BADDR_H(x) (0x2804 + 0x40 * (x))
#define SPE_TDQX_LEN(x) (0x2808 + 0x40 * (x))
#define SPE_TDQX_WPTR(x) (0x280C + 0x40 * (x))
#define SPE_TDQX_RPTR(x) (0x2810 + 0x40 * (x))
#define SPE_TDQX_DEPTH(x) (0x2814 + 0x40 * (x))
#define SPE_RDQX_BADDR_L(x) (0x2818 + 0x40 * (x))
#define SPE_RDQX_BADDR_H(x) (0x281C + 0x40 * (x))
#define SPE_RDQX_LEN(x) (0x2820 + 0x40 * (x))
#define SPE_RDQX_WPTR(x) (0x2824 + 0x40 * (x))
#define SPE_RDQX_RPTR(x) (0x2828 + 0x40 * (x))
#define SPE_RDQX_DEPTH(x) (0x282C + 0x40 * (x))
#define SPE_TDQX_PRI(x) (0x2830 + 0x40 * (x))

#define SPE_IPF_RDQ_WPTR_UPDATE_ADDR_L 0x2C00
#define SPE_IPF_RDQ_WPTR_UPDATE_ADDR_H 0x2C04
#define IPF_SPE_BDQ_RPTR_UPDATE_ADDR_L 0x2C08
#define IPF_SPE_BDQ_RPTR_UPDATE_ADDR_H 0x2C0C

#define SPE_PORTX_UDP_LMTNUM(x) (0x2D00 + 0x30 * (x))
#define SPE_PORTX_LMTBYTE(x) (0x2D04 + 0x30 * (x))
#define SPE_PORTX_PROPERTY(x) (0x2D08 + 0x30 * (x))
#define SPE_PORTX_MAC_ADDR_L(x) (0x2D0C + 0x30 * (x))
#define SPE_PORTX_MAC_ADDR_H(x) (0x2D10 + 0x30 * (x))
#define SPE_PORTX_IPV6_ADDR_1ST(x) (0x2D14 + 0x30 * (x))
#define SPE_PORTX_IPV6_ADDR_2ND(x) (0x2D18 + 0x30 * (x))
#define SPE_PORTX_IPV6_ADDR_3TH(x) (0x2D1C + 0x30 * (x))
#define SPE_PORTX_IPV6_ADDR_4TH(x) (0x2D20 + 0x30 * (x))
#define SPE_PORTX_IPV6_MASK(x) (0x2D24 + 0x30 * (x))
#define SPE_PORTX_IPV4_ADDR(x) (0x2D28 + 0x30 * (x))
#define SPE_PORTX_IPV4_MASK(x) (0x2D2C + 0x30 * (x))

#define SPE_ADQ_BADDR_L(x) (0x3000 + 8 * (x))
#define SPE_ADQ_BADDR_H(x) (0x3004 + 8 * (x))
#define SPE_ADQ_SIZE(x) (0x3018 + 4 * (x))
#define SPE_ADQ_WPTR(x) (0x3030 + 8 * (x))
#define SPE_ADQ_RPTR(x) (0x3034 + 8 * (x))
#define SPE_ADQ_LEN(x) (0x3048 + 4 * (x))
#define SPE_ADQ_RPTR_UPDATE_ADDR_L(x) (0x3054 + 8 * (x))
#define SPE_ADQ_RPTR_UPDATE_ADDR_H(x) (0x3058 + 8 * (x))
#define SPE_ADQ_BUF_FLUSH(x) (0x3070 + 4 * (x))
#define SPE_ADQ_THRESHOLD 0x306C

#define SPE_TAB_ENTRY_INTERVAL (4)
#define SPE_MAC_ENTRY_NUM (512)    /* mac and mac filter entry total 128, every entry 4 word */
#define SPE_IPV4_ENTRY_NUM (96)    /* ipv4 filter entry total 16, every entry 6 word */
#define SPE_IPV6_ENTRY_NUM (96)    /* ipv6 filter entry total 8, every entry 12 word */
#define SPE_QOS_MAC_ENTRY_NUM (96) /* qos mac entry total 32, every entry 3 word */

#define SPE_TAB 0x800
#define SPE_MAC_TAB(x) (SPE_TAB + SPE_TAB_ENTRY_INTERVAL * (x))
#define SPE_IPV4_TAB(x) (SPE_MAC_TAB(SPE_MAC_ENTRY_NUM) + SPE_TAB_ENTRY_INTERVAL * (x))
#define SPE_IPV6_TAB(x) (SPE_IPV4_TAB(SPE_IPV4_ENTRY_NUM) + SPE_TAB_ENTRY_INTERVAL * (x))
#define SPE_QOS_MAC_TAB(x) (SPE_IPV6_TAB(SPE_IPV6_ENTRY_NUM) + SPE_TAB_ENTRY_INTERVAL * (x)) /* add by B5010 */

enum spe_tab_list_type {
    SPE_TAB_LIST_MAC_FILT = 0,
    SPE_TAB_LIST_IPV4_FILT,
    SPE_TAB_LIST_IPV6_FILT,
    SPE_TAB_LIST_BOTTOM
};

enum spe_brg_type {
    SPE_BRG_BRIF,
    SPE_BRG_NORMAL,
    SPE_BRG_MIX,
    SPE_BRG_BR,
    SPE_BRG_BOTTOM
};

typedef union {
    struct {
        unsigned int spe_en : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
} spe_en_t;

typedef union {
    struct {
        unsigned int spe_mac_black_white : 1;
        unsigned int spe_ip_black_white : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} spe_black_white_t;

typedef union {
    struct {
        unsigned int spe_port_attach_brg : 2;
        unsigned int spe_port_enc_type : 4;
        unsigned int spe_port_pad_en : 1;
        unsigned int reserved : 24;
        unsigned int spe_port_en : 1;
    } bits;
    unsigned int u32;
} spe_port_prop_t; /* HI_SPE_CH_PROPERTY_T */

typedef union {
    struct {
        unsigned int spe_dbgen : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
} spe_dbgen_t;

typedef union {
    struct {
        unsigned int spe_tab_done : 1;
        unsigned int spe_tab_success : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} spe_tab_act_result_t;

typedef union {
    struct {
        unsigned int spe_ip_aging_time : 16;
        unsigned int spe_mac_aging_time : 16;
    } bits;
    unsigned int u32;
} spe_ageing_timer_t;

typedef union {
    struct {
        unsigned int spe_pack_max_pkt_cnt : 7;
        unsigned int reserved_1 : 1;
        unsigned int spe_pack_max_len : 17;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} spe_pack_ctrl_t;
typedef union {
    struct {
        unsigned int spe_ncm_max_len : 16;
        unsigned int spe_rndis_max_len : 16;
    } bits;
    unsigned int u32;
} spe_unpack_max_len_t;

typedef union {
    struct {
        unsigned int spe_tab_clr : 5;
        unsigned int reserved : 27;
    } bits;
    unsigned int u32;
} spe_tab_clt_t;

typedef union {
    struct {
        unsigned int spe_port_udp_lmttime : 8;
        unsigned int spe_port_lmttime : 8;
        unsigned int spe_port_lmt_en : 16;
    } bits;
    unsigned int u32;
} spe_port_lmttime_t;

typedef union {
    struct {
        unsigned int spe_l4_portnum_cfg : 18;
        unsigned int reserved : 14;
    } bits;
    unsigned int u32;
} spe_l4_cfg_t;

static inline unsigned int spe_readl(const void __iomem *base, unsigned int offset)
{
    return readl(base + offset);
}

static inline unsigned int spe_readl_relaxed(const void __iomem *base, unsigned int offset)
{
    return readl_relaxed(base + offset);
}

static inline void spe_writel(void __iomem *base, unsigned int offset, unsigned int value)
{
    writel(value, base + offset);
}

static inline void spe_writel_relaxed(void __iomem *base, unsigned int offset, unsigned int value)
{
    writel_relaxed(value, base + offset);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SPE_REG_H__ */
