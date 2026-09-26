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

#include "maa.h"
#ifdef CONFIG_MAA_V2
#include "hi_maa_v2.h"
#else
#include "hi_maa.h"
#endif

void maa_set_debug(int enable)
{
    g_maa_ctx->dbg = enable;
}

void maa_set_intr_threhold(int threhold)
{
    g_maa_ctx->intr_threhold = threhold;
}

void maa_set_timer_out(unsigned int timeout)
{
    g_maa_ctx->timer_out = timeout;
}

void maa_list_measure(unsigned int level)
{
    struct maa_map *map = &g_maa_ctx->opipes[level].map;
    struct maa_opipe *opipes = &g_maa_ctx->opipes[level];
    struct list_head *pos = NULL;
    unsigned int cnt = 0;
    unsigned long flags;
    spin_lock_irqsave(&opipes->lock, flags);
    list_for_each(pos, &map->free_list)
    {
        cnt++;
    }
    spin_unlock_irqrestore(&opipes->lock, flags);
    bsp_err("free_list cnt %u\n", cnt);
    cnt = 0;
    spin_lock_irqsave(&opipes->lock, flags);
    list_for_each(pos, &map->busy_list)
    {
        cnt++;
    }
    spin_unlock_irqrestore(&opipes->lock, flags);
    bsp_err("busy_list cnt %u\n", cnt);
    cnt = 0;
    spin_lock_irqsave(&opipes->lock, flags);
    list_for_each(pos, &map->reclaim_list)
    {
        cnt++;
    }
    spin_unlock_irqrestore(&opipes->lock, flags);
    bsp_err("reclaim_list cnt %u\n", cnt);
}

void maa_show_opipe_status(unsigned int level)
{
    struct maa *maa = g_maa_ctx;
    unsigned int i = level;

    if (!maa_opipe_is_enable(maa, level)) {
        bsp_err("level:%u is diable\n", level);
        return;
    }

    bsp_err("map_invalid_level %u\n", maa->status.map_invalid_level);
    bsp_err("unmap_invalid_level %u\n", maa->status.unmap_invalid_level);
    bsp_err("opipe%u:skb_alloc_success  %u\n", i, maa->opipe_status[level].alloc_success);
    bsp_err("opipe%u:skb_alloc_atomic  %u\n", i, maa->opipe_status[level].alloc_atomic);
    bsp_err("opipe%u:skb_alloc_kernel  %u\n", i, maa->opipe_status[level].alloc_kernel);
    bsp_err("opipe%u:skb_own_free  %u\n", i, maa->opipe_status[level].skb_own_free);
    bsp_err("opipe%u:map_own_success  %u\n", i, maa->opipe_status[level].map_own_success);
    bsp_err("opipe%u:map_own_fail  %u\n", i, maa->opipe_status[level].map_own_fail);
    bsp_err("opipe%u:alloc_map_own_fail  %u\n", i, maa->opipe_status[level].alloc_map_own_fail);
    bsp_err("opipe%u:free_list_empty  %u\n", i, maa->opipe_status[level].free_list_empty);
    bsp_err("opipe%u:alloc_fail  %u\n", i, maa->opipe_status[level].alloc_fail);
    bsp_err("opipe%u:skb_build_fail  %u\n", i, maa->opipe_status[level].skb_build_fail);
    bsp_err("opipe%u:unmap_own  %u\n", i, maa->opipe_status[level].unmap_own);
    bsp_err("opipe%u:opipe:map_fail_skb_free  %u\n", i, maa->opipe_status[level].map_fail_skb_free);
    bsp_err("opipe%u:map_tcp_ip_ok  %u\n", i, maa->opipe_status[level].map_tcp_ip_ok);
    bsp_err("opipe%u:map_tcp_ip_fail  %u\n", i, maa->opipe_status[level].map_tcp_ip_fail);
    bsp_err("opipe%u:unmap_tcp_ip  %u\n", i, maa->opipe_status[level].unmap_tcp_ip);
    bsp_err("opipe%u:unmap_test_tcp_ip  %u\n", i, maa->opipe_status[level].unmap_test_tcp_ip);
    bsp_err("opipe%u:opipe_full  %u\n", i, maa->opipe_status[level].opipe_full);
    bsp_err("opipe%u:opipe_dn  %u\n", i, maa->opipe_status[level].opipe_dn);
    bsp_err("opipe%u:ipipe_up  %u\n", i, maa->opipe_status[level].ipipe_up);
    bsp_err("opipe%u:delay_free_cnt %u\n", i, maa->opipes[i].delay_free_cnt);
    bsp_err("opipe%u:reclaim_cnt %u\n", i, maa->opipes[i].reclaim_cnt);
    bsp_err("opipe%u:reclaim_depth %u\n", i, maa->opipes[i].reclaim_depth);
    bsp_err("opipe%u:reclaim_clear %u\n", i, maa->opipes[i].reclaim_clear);
    bsp_err("opipe%u:skb_reclaim_free %u\n", i, maa->opipe_status[level].skb_reclaim_free);
    bsp_err("opipe%u:skb_tcp_ip_free %u\n", i, maa->opipe_status[level].skb_tcp_ip_free);
    bsp_err("opipe%u:busy %u\n", i, maa->opipes[i].map.busy);
    bsp_err("opipe%u:busy_depth %u\n", i, maa->opipes[i].busy_depth);
    bsp_err("opipe%u:max_alloc_cnt %u\n", i, maa->opipes[i].max_alloc_cnt);
    bsp_err("opipe%u:min_alloc_cnt %u\n", i, maa->opipes[i].min_alloc_cnt);
    bsp_err("opipe%u:free_success %u\n", i, maa->opipe_status[level].free);
    bsp_err("refill_succ: %u\n", maa->status.refill_succ);
    bsp_err("sche_cnt: %u\n", maa->status.sche_cnt);
    bsp_err("reclaim_succ: %u\n", maa->status.reclaim_succ);
    bsp_err("tab size: %u\n", maa->opipes[level].map.tab_max_size);
    maa_list_measure(level);
}

void maa_show_ipipe_status(void)
{
    struct maa *maa = g_maa_ctx;
    unsigned int i = 0;
    struct maa_ipipe *ipipe = NULL;

    if (maa == NULL) {
        return;
    }

    for (i = 0; i < maa->max_cpu_ipipes; i++) {
        ipipe = maa->cpu_ipipes + i;
        bsp_err("ipipe%u ipipe_empty %u\n", ipipe->idx, ipipe->debug.ipipe_empty);
        bsp_err("ipipe%u alloc_success %u\n", ipipe->idx, ipipe->debug.alloc);
    }
}

void maa_show_push_cnt(void)
{
    unsigned int level;
    unsigned int idx;
    struct maa *maa = g_maa_ctx;

    if (maa == NULL) {
        return;
    }

    for (level = 0; level < MAA_OPIPE_MAX; level++) {
        bsp_err("opipe%u alloc_cnt %u\n", level, readl(maa->regs + MAA_ACORE_ALLOC_CNT(level)));
        bsp_err("opipe%u alloc_fifo_cnt %u\n", level, readl(maa->regs + MAA_ALLOC_FIFO_CNT(level)));
        bsp_err("opipe%u alloc_maa_cnt %u\n", level, readl(maa->regs + MAA_ALLOC_MAA_CNT(level)));
        bsp_err("opipe%u maa_rls_cnt %u\n", level, readl(maa->regs + MAA_RLS_CNT(level)));
        for (idx = 0; idx < MAA_MAX_IPIPE_ENDPOINT; idx++) {
            bsp_err("opipe%u to ipipe%u alloc_destn_cnt %u\n", level, level * MAA_OPIPE_MAX + idx,
                readl(maa->regs + MAA_ALLOC_DESTN_CNT(idx, level)));
            bsp_err("opipe%u to ipipe%u destn_alloc_cnt %u\n", level, level * MAA_OPIPE_MAX + idx,
                readl(maa->regs + MAA_DESTN_ALLOC_CNT(idx, level)));
        }
    }
}

void maa_help(void)
{
    bsp_err("maa_show_opipe_status \n");
    bsp_err("maa_show_ipipe_status \n");
    bsp_err("maa_show_push_cnt \n");
}

void maa_inter_set_mask(unsigned int level, unsigned int set)
{
    unsigned int mask;

    mask = readl_relaxed(g_maa_ctx->regs + MAA_OPIPE_INT_MSK);
    if (!!set) {
        mask |= (1 << level);
    } else {
        mask &= ~(1 << level);
    }
    writel_relaxed(mask, g_maa_ctx->regs + MAA_OPIPE_INT_MSK);
    return;
}

void maa_mdmreset_status(void)
{
    struct maa *maa = g_maa_ctx;
    bsp_err("timer cnt:%d\n", maa->timer_cnt);
    bsp_err("resetting:%d\n", maa->resetting);
    bsp_err("deinit_cnt:%d\n", maa->deinit_cnt);
    bsp_err("reinit_cnt:%d\n", maa->reinit_cnt);
}

