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
 */

#ifndef _BSP_MAA_H_
#define _BSP_MAA_H_
#include <product_config.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MAA_SKB_FROM_TCP_IP 0
#define MAA_SKB_FROM_OWN 1

#ifdef CONFIG_MAA_V2
enum maa_ipipe_idx {
    MAA_IPIPE_FOR_SPE_512 = 0,
    MAA_IPIPE_FOR_IPF_512 = 1,
    MAA_IPIPE_FOR_2_3_4G_512 = 2,
    MAA_IPIPE_FOR_ACPU_512 = 3,
    MAA_IPIPE_FOR_ECIPHER_512 = 4,
    MAA_IPIPE_FOR_5G_512 = 5,
    MAA_IPIPE_FOR_SPE_2K = 11,
    MAA_IPIPE_FOR_IPF_2K = 12,
    MAA_IPIPE_FOR_2_3_4G_2K = 8,
    MAA_IPIPE_FOR_ACPU_2K = 9,
    MAA_IPIPE_FOR_ECIPHER_2K = 10,
    MAA_IPIPE_FOR_5G_2K = 13,
    MAA_IPIPE_FOR_2_3_4G_16K = 24,
    MAA_IPIPE_FOR_5G_16K = 25,
    MAA_IPIPE_FOR_ECIPHER_16K = 26,
    MAA_IPIPE_FOR_SPE_MODEM_2K = 45,
    MAA_IPIPE_FOR_ACPU_FREE = 55
};
#else
enum maa_ipipe_idx {
    MAA_IPIPE_FOR_SPE_512 = 0,
    MAA_IPIPE_FOR_IPF_512 = 1,
    MAA_IPIPE_FOR_2_3_4G_512 = 2,
    MAA_IPIPE_FOR_ACPU_512 = 3,
    MAA_IPIPE_FOR_ECIPHER_512 = 4,
    MAA_IPIPE_FOR_5G_512 = 5,
    MAA_IPIPE_FOR_SPE_2K = 6,
    MAA_IPIPE_FOR_IPF_2K = 7,
    MAA_IPIPE_FOR_2_3_4G_2K = 8,
    MAA_IPIPE_FOR_ACPU_2K = 9,
    MAA_IPIPE_FOR_ECIPHER_2K = 10,
    MAA_IPIPE_FOR_5G_2K = 11,
    MAA_IPIPE_FOR_2_3_4G_16K = 20,
    MAA_IPIPE_FOR_5G_16K = 21,
    MAA_IPIPE_FOR_ECIPHER_16K = 22,
    MAA_IPIPE_FOR_ACPU_FREE = 33
};
#endif

enum maa_opipe_level_size {
    MAA_OPIPE_LEVEL_0_SIZE = 128U,  /* 128B, reverse 128B */
    MAA_OPIPE_LEVEL_1_SIZE = 1560U, /* 1536B reverse 128B */
    MAA_OPIPE_LEVEL_2_SIZE = 4096U, /* 4KB no reverse */
    MAA_OPIPE_LEVEL_3_SIZE = 9216U, /* 9KB reverse 128B */
    MAA_OPIPE_LEVEL_4_SIZE = 1560U, /* C mem */
    MAA_OPIPE_LEVEL_5_SIZE = 128U,  /* C mem */
    MAA_OPIPE_LEVEL_INVALID_SIZE = 0xdeadbeefU
};

#define MAA_OPIPE_LEVEL_0_BUFF_SIZE 1024U
#define MAA_OPIPE_LEVEL_1_BUFF_SIZE 2048U

enum maa_opipe_level_idx {
    MAA_OPIPE_LEVEL_0 = 0U,
    MAA_OPIPE_LEVEL_1 = 1U,
    MAA_OPIPE_LEVEL_2 = 2U,
    MAA_OPIPE_LEVEL_3 = 3U,
    MAA_OPIPE_LEVEL_4 = 4U,
    MAA_OPIPE_LEVEL_5 = 5U,
    MAA_OPIPE_LEVEL_INVALID
};
#define MAA_MAX_LEVEL 7

struct maa_opipe_debug_info {
    unsigned int skb_alloc_success;
    unsigned int skb_own_free;
    unsigned int map_own_success;
    unsigned int map_own_fail;
    unsigned int alloc_map_own_fail;
    unsigned int free_list_empty;
    unsigned int skb_alloc_fail;
    unsigned int unmap_own;
    unsigned int map_fail_skb_free;
    unsigned int map_tcp_ip_ok;
    unsigned int map_tcp_ip_fail;
    unsigned int unmap_tcp_ip;
    unsigned int unmap_test_tcp_ip;
    unsigned int delay_free_cnt;
    unsigned int reclaim_cnt;
    unsigned int max_alloc_cnt;
    unsigned int min_alloc_cnt;
    unsigned int free;
    unsigned int wptr;
    unsigned int rptr;
    unsigned int skb_reclaim_free;
    unsigned int reclaim_depth;
};

struct maa_ipipe_debug_info {
    unsigned int ch_en;
};

#define MAA_PER_IPIPE_POINT 6

struct maa_debug_info {
    unsigned int rsl_full;
    unsigned int cur_ts;
    unsigned int int_ts;
    unsigned int max_sche_time;
    unsigned int min_sche_time;
    unsigned int fill_cnt[MAA_MAX_LEVEL];
    unsigned int irq_mask;
    unsigned int irq_raw;
    unsigned int work_resetting;
    unsigned int work_start;
    unsigned int work_end;
    unsigned int refill_succ;
    unsigned int sche_cnt;
    unsigned int mem_protect;
    unsigned int map_invalid_level;
    unsigned int unmap_invalid_level;
    unsigned int timer_en;
    unsigned int fifo_timer_en;
    unsigned int rsl_rptr;
    unsigned int rsl_wptr;
    struct maa_opipe_debug_info opipe_dbg[MAA_OPIPE_LEVEL_INVALID];
    struct maa_ipipe_debug_info ipipe_dbg[MAA_OPIPE_LEVEL_INVALID * MAA_PER_IPIPE_POINT];
    unsigned int opipe_cnt[MAA_OPIPE_LEVEL_INVALID * 4]; // first 4 level need statistics
    unsigned int ipipe_cnt[MAA_PER_IPIPE_POINT * 2 * MAA_OPIPE_LEVEL_INVALID]; // first 2 level need statistics
};

#ifdef CONFIG_MAA_BALONG
unsigned int bsp_maa_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth,
    unsigned int ipipe);

unsigned long long bsp_maa_skb_map(struct sk_buff *skb, unsigned int reverse);

struct sk_buff *bsp_maa_skb_unmap(unsigned long long maadr);

struct sk_buff *bsp_maa_skb_alloc(unsigned int size);

void bsp_maa_kfree_skb(struct sk_buff *skb);

struct sk_buff *bsp_maa_get_skb(unsigned long long addr);
unsigned long long bsp_maa_get_maa(struct sk_buff *skb);
void bsp_maa_reset_skb(struct sk_buff *skb);

struct sk_buff *bsp_maa_to_skb(unsigned long long changed, unsigned long long *orig);

struct sk_buff *bsp_maa_alloc_skb(unsigned int size, unsigned long long *orig);
void bsp_maa_free(unsigned long long orig);
unsigned long long bsp_maa_alloc(unsigned int size);
unsigned long long bsp_maa_to_phy(unsigned long long addr);
unsigned int bsp_maa_get_addr_type(unsigned long long addr);
int bsp_maa_reinit(void);
struct sk_buff *bsp_maa_unmap(unsigned long long addr, unsigned long long *orig);

#else
static inline unsigned int bsp_maa_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth,
    unsigned int ipipe)
{
    return 0;
}

static inline void bsp_maa_enable_ipipe(unsigned int ipipe, int enable)
{
    return;
}

static inline unsigned long long bsp_maa_skb_map(struct sk_buff *skb, unsigned int reverse)
{
    return 0;
}

static inline struct sk_buff *bsp_maa_skb_unmap(unsigned long long maadr)
{
    return NULL;
}

static inline struct sk_buff *bsp_maa_skb_alloc(unsigned int size)
{
    return NULL;
}

static inline void bsp_maa_kfree_skb(struct sk_buff *skb)
{
    return;
}

static inline struct sk_buff *bsp_maa_get_skb(unsigned long long addr)
{
    return NULL;
}

static inline unsigned long long bsp_maa_get_maa(struct sk_buff *skb)
{
    return 0;
}

static inline void bsp_maa_reset_skb(struct sk_buff *skb)
{
    return;
}

static inline struct sk_buff *bsp_maa_to_skb(unsigned long long changed, unsigned long long *orig)
{
    return NULL;
}

static inline struct sk_buff *bsp_maa_alloc_skb(unsigned int size, unsigned long long *orig)
{
    return NULL;
}

static inline void bsp_maa_free(unsigned long long orig)
{
    return;
}

static inline unsigned long long bsp_maa_alloc(unsigned int size)
{
    return 0;
}

static inline unsigned long long bsp_maa_to_phy(unsigned long long addr)
{
    return 0;
}

static inline unsigned int bsp_maa_get_addr_type(unsigned long long addr)
{
    return 0;
}

int bsp_maa_reinit(void)
{
    return 0;
}

static inline struct sk_buff *bsp_maa_unmap(unsigned long long addr, unsigned long long *orig)
{
    return NULL;
}

#endif

#ifdef __cplusplus
}
#endif
#endif
