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
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/io.h>
#include "bsp_maa.h"
#include "maa.h"
#include <bsp_print.h>
#include "product_config.h"
#include <linux/netdevice.h>
#include <osl_types.h>
#include <securec.h>
#ifdef CONFIG_MAA_V2
#include "hi_maa_v2.h"
#else
#include "hi_maa.h"
#endif

#define THIS_MODU mod_maa

#define maa_err(fmt, ...)                             \
    do {                                              \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
    } while (0)

unsigned long long bsp_maa_skb_map(struct sk_buff *skb, unsigned int reverse)
{
    unsigned int level;
    unsigned long long reversed_addr;
    unsigned long long phy;

    struct maa *maa = g_maa_ctx;

    if (maa == NULL || skb == NULL || skb->len == 0) {
        return 0;
    }
    level = (unsigned int)maa_len_to_levels(skb->len);
    if (level == MAA_OPIPE_LEVEL_INVALID) {
        return 0;
    }

    if (maa_freelist_is_empty(g_maa_ctx, level)) {
        maa->opipe_status[level].map_tcp_ip_fail++;
        return 0;
    }
    /* maaaddr point to iphead */
    phy = dma_map_single(maa->dev, skb->data, skb->len, DMA_TO_DEVICE);

    reversed_addr = maa_map_single(maa, phy + reverse, skb, MAA_SKB_FROM_TCP_IP, level);
    if (!reversed_addr) {
        maa->opipe_status[level].map_tcp_ip_fail++;
        return 0;
    }
    maa->opipe_status[level].map_tcp_ip_ok++;

    return reversed_addr;
}

struct sk_buff *bsp_maa_skb_unmap(unsigned long long addr)
{
    struct sk_buff *skb = NULL;
    maa_buf_t buf;
    struct maa *maa = g_maa_ctx;

    if (maa == NULL || addr == 0) {
        return NULL;
    }
    buf.addr = addr;
    if (buf.bits.from == MAA_SKB_FROM_TCP_IP) {
        maa->opipe_status[buf.bits.level].unmap_test_tcp_ip++;
        skb = maa_unmap_tcpip_skb(maa, buf);
    } else {
        skb = maa_unmap_skb(maa, buf, NULL);
    }
    if (skb == NULL) {
        return NULL;
    }

    return skb;
}
struct sk_buff *bsp_maa_unmap(unsigned long long addr, unsigned long long *orig)
{
    maa_buf_t buf;
    struct maa *maa = g_maa_ctx;

    if (unlikely((maa == NULL) || (addr == 0) || (orig == NULL))) {
        return NULL;
    }
    buf.addr = addr;
    return maa_unmap_skb(maa, buf, orig);
}

inline void maa_reset_skb(struct sk_buff *skb)
{
    skb->data = skb->head + MAA_SKB_RESERVE_SIZE;
    skb->len = 0;
    skb_reset_tail_pointer(skb);
}

struct sk_buff *bsp_maa_skb_alloc(unsigned int size)
{
    struct sk_buff *skb = NULL;
    unsigned long long addr;
    unsigned long long orig = 0;
    struct maa *maa = g_maa_ctx;

    if (maa == NULL) {
        return NULL;
    }
    addr = maa_ipipe_alloc(size);
    if (!addr) {
        return 0;
    }
    skb = maa_to_skb(maa, addr, &orig);
    if (skb == NULL) {
        return 0;
    }
    maa_reset_skb(skb);
    return skb;
}

void bsp_maa_reset_skb(struct sk_buff *skb)
{
    if (skb == NULL) {
        maa_err("input skb null\n");
        return;
    }
    maa_reset_skb(skb);
}

void bsp_maa_kfree_skb(struct sk_buff *skb)
{
    struct maa_head_rom *rom = NULL;
    struct maa *maa = g_maa_ctx;

    if (maa == NULL) {
        return;
    }
    if (skb == NULL) {
        maa_err("skb is null\n");
        return;
    }
    rom = (struct maa_head_rom *)skb->head;

    if (virt_addr_valid((uintptr_t)rom) && rom->check == MAA_OWN) {
        maa_reset_skb(skb);
        maa_ipipe_free(maa, rom->maa_addr);
    } else {
        maa->status.maybe_dobulefree_or_modified++;
        maa_err("skb 0x%llx maybe doublefree or modified\n", (unsigned long long)(uintptr_t)skb);
    }
}

struct sk_buff *bsp_maa_get_skb(unsigned long long addr)
{
    struct sk_buff *skb = NULL;
    unsigned long long orig = 0;

    struct maa *maa = g_maa_ctx;
    if (maa == NULL || addr == 0) {
        return NULL;
    }

    skb = maa_to_skb(maa, addr, &orig);

    return skb;
}

unsigned long long bsp_maa_get_maa(struct sk_buff *skb)
{
    struct maa_head_rom *rom = NULL;

    if (skb == NULL) {
        maa_err("input skb is null\n");
        return 0;
    }

    rom = (struct maa_head_rom *)skb->head;

    if (virt_addr_valid((uintptr_t)rom) && rom->check == MAA_OWN) {
        return rom->maa_addr;
    } else {
        if (virt_addr_valid((uintptr_t)rom)) {
            maa_err("rom->check 0x%x, rom->maa_addr 0x%llx\n", rom->check, rom->maa_addr);
        }
        maa_err("skb 0x%llx maybe modified or doublefree\n", (unsigned long long)(uintptr_t)skb);
        return 0;
    }
}

unsigned int bsp_maa_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth, unsigned int ipipe)
{
    if (adqbase == 0 || write == 0 || depth == 0) {
        maa_err("adqbase=%llx, write=%x, depth=%d\n", adqbase, write, depth);
        return 0;
    }

    return maa_set_adqbase(adqbase, write, depth, ipipe);
}

struct sk_buff *bsp_maa_to_skb(unsigned long long changed, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    struct maa *maa = g_maa_ctx;

    if (maa == NULL || orig == NULL) {
        return NULL;
    }

    *orig = 0;
    skb = maa_to_skb(maa, changed, orig);

    return skb;
}
unsigned long long bsp_maa_alloc(unsigned int size)
{
    unsigned long long addr;

    addr = maa_ipipe_alloc(size);
    if (!addr) {
        return 0;
    }

    return addr;
}

struct sk_buff *bsp_maa_alloc_skb(unsigned int size, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    unsigned long long addr;
    struct maa *maa = g_maa_ctx;
    if (orig == NULL || maa == NULL) {
        return NULL;
    }
    *orig = 0;
    addr = maa_ipipe_alloc(size);
    if (!addr) {
        return 0;
    }
    *orig = addr;
    skb = _maa_to_skb(maa, addr);
    if (skb == NULL) {
        return 0;
    }

    maa_reset_skb(skb);
    return skb;
}

void bsp_maa_free(unsigned long long changed)
{
    struct sk_buff *skb = NULL;
    maa_buf_t buf;
    struct maa *maa = g_maa_ctx;
    if (maa == NULL) {
        return;
    }
    buf.addr = changed;

    if (buf.bits.from == MAA_SKB_FROM_TCP_IP) {
        skb = maa_unmap_tcpip_skb(maa, buf);
        if (skb != NULL) {
            dev_kfree_skb_any(skb);
        }
    } else {
        return maa_ipipe_free(maa, changed);
    }
}
unsigned long long bsp_maa_to_phy(unsigned long long addr)
{
    maa_buf_t buf;

    buf.addr = addr;

    return buf.bits.phyaddr;
}

unsigned int bsp_maa_get_addr_type(unsigned long long addr)
{
    maa_buf_t buf;
    buf.addr = addr;
    if (buf.bits.from) {
        return MAA_SKB_FROM_OWN;
    } else {
        return MAA_SKB_FROM_TCP_IP;
    }
}
void mdrv_maa_skb_free(struct sk_buff *skb)
{
    return bsp_maa_kfree_skb(skb);
}

EXPORT_SYMBOL(bsp_maa_skb_alloc);
EXPORT_SYMBOL(bsp_maa_skb_unmap);
EXPORT_SYMBOL(bsp_maa_kfree_skb);
EXPORT_SYMBOL(bsp_maa_skb_map);
EXPORT_SYMBOL(bsp_maa_alloc);
EXPORT_SYMBOL(bsp_maa_free);
EXPORT_SYMBOL(bsp_maa_get_addr_type);
EXPORT_SYMBOL(bsp_maa_get_maa);
EXPORT_SYMBOL(bsp_maa_get_skb);
EXPORT_SYMBOL(bsp_maa_to_phy);
EXPORT_SYMBOL(bsp_maa_unmap);
EXPORT_SYMBOL(bsp_maa_alloc_skb);
EXPORT_SYMBOL(bsp_maa_reset_skb);
EXPORT_SYMBOL(bsp_maa_to_skb);
EXPORT_SYMBOL(mdrv_maa_skb_free);
