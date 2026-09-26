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
#ifndef _BALONG_MAA_H_
#define _BALONG_MAA_H_

#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <bsp_trans_report.h>
#include <bsp_maa.h>
#include <bsp_print.h>

#define MAA_LEVELS_MAX_BIT 18
#define MAA_LEVELS_MAX_SIZE (1 << MAA_LEVELS_MAX_BIT)
#define MAA_FILL_MAX_IN_INT 8
#define MAA_ACPU_MID 0x41
#define MAA_NRCPU_MID 0x28
#define MAA_LRCPU_MID 0x3f
#define MAA_OPIPE_DN_BIT(X) BIT(X)
#define MAA_SKB_FROM_TCP_IP 0
#define MAA_SKB_FROM_NIC 2
#define MAA_SKB_FROM_OWN 1
#define MAA_CP_MEM 1
#define MAA_AP_MEM 0

#define MAA_OWN 0x4D41414D
#define MAA_FREE 0x41414D55

#define MAA_SKB_RESERVE_SIZE SKB_DATA_ALIGN(64)
#define MAA_SKB_RESERVE_MASK (~(MAA_SKB_RESERVE_SIZE - 1))
#define MAA_PHY_ADDR_BITS 40
#define MAA_PHY_ADDR_MASK ((1ULL << MAA_PHY_ADDR_BITS) - 1)
#define MAA_TEST_RELEASE_OPIPE5 BIT(4)
#define MAA_DEBUG_MSG BIT(5)
#define MAA_DEBUG_INTR_REFILL BIT(6)
#define MAA_DEBUG_ALLOC_FREE_TIMESTAMP BIT(7)
#define MAA_DEBUG_BUGON BIT(8)
#define MAA_DEBUG_SYSTEM_ERR BIT(9)
#define MAA_DEBUG_TIMER_EN BIT(10)
#define MAA_DEBUG_RELEASE_IRQ_EN BIT(11)
#define MAX_PERF_DEBUG_CNT (1 << 10)
#define MAX_PERF_DEBUG_SIZE 2
#define MAX_PERF_DEBUG_TIME_IDX 1
#define MAX_PERF_DEBUG_CNT_IDX 0
#define MAA_DUMP_REG_NUM 2048
#define MAA_DUMP_SIZE (MAA_DUMP_REG_NUM * sizeof(unsigned int))
#define MAA_BACK_REGS_NUM 10
#define MAA_IPIPE_USING_CONT 21
#define MAA_RELEASE_THRSH_LP 128

#define MAA_FLS_SIZE 32

#define MAA_ALLOC_FIFO_SIZE 16

#define MAA_OPIPE_FREE_LEVELS 1
#define MAA_OPIPE_ALLOC_LEVELS 7
#define MAA_OPIPE_MAX (MAA_OPIPE_ALLOC_LEVELS + MAA_OPIPE_FREE_LEVELS)

#ifdef CONFIG_MAA_V2
#define MAA_MAX_IPIPE_ENDPOINT (8)
#else
#define MAA_MAX_IPIPE_ENDPOINT (6)
#endif
#define MAA_IPIPE_ALLOC_MAX_COUNT (MAA_OPIPE_ALLOC_LEVELS * MAA_MAX_IPIPE_ENDPOINT)
#define MAA_OPIPE_STATUS_INIT 0x5a5a5a5a
#define MAA_INIT_OK (0x4D41415F)
#define MAA_ADDR_SIZE sizeof(maa_buf_t)
#define MAA_TABEL_START 0xF
#define MAA_BURST_LEN_BIT 4
#define MAA_BURST_LEN (1 << MAA_BURST_LEN_BIT)
#define MAA_OPIPE_USED_MAGIC 0xcccccccc

#define THIS_MODU mod_maa
#define maa_err(fmt, ...)                             \
    do {                                              \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
    } while (0)

enum maa_list_e {
    MAA_LOCALLIST,
    MAA_BUSYLIST,
    MAA_RECLAIMLIST,
};

typedef union {
    struct {
        unsigned long long phyaddr : 40;
        unsigned long long idx : 18;
        unsigned long long crc : 1;
        unsigned long long from : 2;
        unsigned long long level : 3;
    } bits;
    unsigned long long addr;
} maa_buf_t;

struct maa_opipe_para {
    unsigned int size[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int base[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int depth[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int dthr[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int gap[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int disable_irq[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int from[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int tab_size[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int opipeidx[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int alloc[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int cpu[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int cnt;
    unsigned int dthr_config;
};

struct maa_ipipe_para {
    unsigned int size[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int phy[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int depth[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int ipipeidx[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int alloc[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int cpu[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int cnt;
};

struct maa_para_s {
    struct maa_opipe_para oparamter;
    struct maa_ipipe_para iparamter;
    unsigned int ddr_buf_depth[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int opipes_used[MAA_OPIPE_ALLOC_LEVELS];
    unsigned int opipe_cnt;
    unsigned int max_opipes;
    unsigned int opipe_for_free;
    unsigned int opipe_free_level;
    unsigned int opipe_use_aximem;
    unsigned int release_depth;
    unsigned int protect_thresh;
    unsigned int mem_line;
    unsigned int release_in_task;
    unsigned int reserve_mem_size;
    unsigned int fullfill_idx;
    unsigned int fullfill_count;
};

struct maa_local_fifo {
    unsigned long long addr[2 * MAA_BURST_LEN]; // buffer size is 2 MAA_BURST_LEN
    unsigned int idx;
    unsigned int empty;
};

struct maa_fifo {
    unsigned long long base;
    unsigned long long phy;
    unsigned int write;
    unsigned int read;
    unsigned int depth;
    unsigned int uthrl;
    unsigned int dthrl;
    unsigned int free;
    unsigned size;
    unsigned long long update_write_addr;
    unsigned long long update_write_virt;
    unsigned long long update_read_addr;
};

struct maa_ipipe_alloc_debug {
    unsigned int alloc;
    unsigned int size_level_err;
    unsigned int ipipe_empty;
};

struct maa_free_debug {
    unsigned int free[MAA_OPIPE_MAX];
    unsigned int free_fail;
};

struct maa_ipipe {
    struct maa_fifo fifo;
    struct maa_ipipe_alloc_debug debug;
    unsigned long long wptr_update_addr;
    unsigned int level;
    unsigned int idx;
    unsigned int user;
    struct maa_local_fifo local_fifo;
    spinlock_t lock;
};

struct maa_perf_log {
    unsigned int fifo[MAX_PERF_DEBUG_SIZE][MAX_PERF_DEBUG_CNT];
    unsigned int idx;
};

struct maa_perf_debug {
    struct maa_perf_log intr_perf_debug[MAA_OPIPE_ALLOC_LEVELS];
    struct maa_perf_log hf_perf_debug[MAA_OPIPE_ALLOC_LEVELS];
    struct maa_perf_log release_perf_debug;
};

struct maa_head_rom {
    unsigned int check;
    unsigned long long maa_addr;
};

struct maa_tab {
    struct list_head list;
    unsigned int magic;
    unsigned int idx;
    unsigned long long maa_addr;
    void *manager;
};

struct maa_map {
    struct list_head free_list;
    struct list_head busy_list;
    struct list_head reclaim_list;

    struct maa_tab *tab;
    struct maa_tab *ext_tab;
    unsigned int tab_max_size;
    unsigned int free;
    unsigned int busy;
    unsigned int reclaim;
    unsigned int curr;
    unsigned int total;
    unsigned int empty;
    unsigned int cache;
};
struct maa_debug {
    unsigned int free_fail;
    unsigned int unmap_invalid_level;
    unsigned int map_invalid_level;
    unsigned int unmap_invalid_idx;
    unsigned int map_sec_levels;
    unsigned int safe_check_err;
    unsigned int rls_full;
    unsigned int maybe_dobulefree_or_modified;
    unsigned int sche_cnt;
    unsigned int refill_succ;
    unsigned int reclaim_succ;
    unsigned int work_resetting;
    unsigned int work_start;
    unsigned int work_end;
    unsigned int mem_protect;
    unsigned int irq_cnt;
    unsigned int int_ts;
    unsigned int max_sche_time;
    unsigned int min_sche_time;
    unsigned int reclye_to_maa;
    unsigned int fill_cnt[MAA_OPIPE_MAX];
};

struct maa_opipe_debug {
    unsigned int alloc_success;
    unsigned int skb_build_fail;
    unsigned int alloc_fail;
    unsigned int alloc_atomic;
    unsigned int alloc_kernel;
    unsigned int skb_own_free;
    unsigned int skb_tcp_ip_free;
    unsigned int skb_reclaim_free;
    unsigned int map_fail_skb_free;
    unsigned int free;
    unsigned int max_fill_cnt;
    unsigned int to_skb_fail;

    unsigned int free_list_empty;
    unsigned int map_own_success;
    unsigned int map_own_fail;
    unsigned int alloc_map_own_fail;
    unsigned int unmap_own;
    unsigned int unmap_cache_flush;
    unsigned int unmap_own_fail;
    unsigned int map_tcp_ip_ok;
    unsigned int unmap_test_tcp_ip;
    unsigned int map_tcp_ip_fail;
    unsigned int unmap_tcp_ip;
    unsigned int opipe_full;
    unsigned int opipe_dn;
    unsigned int opipe_up;
    unsigned int ipipe_up;
};

struct maa_opipe {
    spinlock_t lock;
    spinlock_t refill_lock;
    struct maa_map map;
    struct maa_fifo fifo;
    struct sk_buff_head free_queue;
    unsigned int skb_alloc_gate;
    unsigned int delay_free_cnt;
    unsigned int reclaim_cnt;
    unsigned int reclaim_depth;
    unsigned int last_delay_free_cnt;
    unsigned int reclaim_clear;
    unsigned int equal_times;
    unsigned int busy_depth;
    unsigned int maybe_modified;
    unsigned int current_cnt;
    unsigned int max_alloc_cnt;
    unsigned int min_alloc_cnt;
    u64 addr_fifo[MAA_ALLOC_FIFO_SIZE];
    u32 status;
    unsigned int level;
    unsigned int from;
    unsigned int used;
};

struct maa_release_poll {
    struct maa_fifo fifo;
    struct workqueue_struct *workqueue;
    struct work_struct release_work;
    struct tasklet_struct tasklet;
    unsigned int doing;
    maa_buf_t local_release[2 * MAA_BURST_LEN]; // buffer size is 2 MAA_BURST_LEN
    unsigned int max_free_cnt;
    unsigned int min_free_cnt;
    unsigned int mcp_fail;
};

struct maa_para {
    unsigned int size;
    unsigned int base;
    unsigned int depth;
    unsigned int ipipeidx;
    unsigned int alloc;
    unsigned int cpu;
};

struct maa_back_regs_cfg {
    char *start;
    char *end;
    unsigned int num;
    unsigned int back_addr;
};

struct maa_reserve_mem {
    spinlock_t lock;
    unsigned int start;
    unsigned int cur;
    unsigned int end;
    unsigned int size;
};

struct maa_allc_para {
    char *name;
    unsigned int ipipe_id;
    unsigned int idx;
    unsigned int level;
    unsigned int launch;
};

struct maa {
    struct maa_para_s *para;
    spinlock_t lock;
    spinlock_t irq_lock;
    spinlock_t reset_lock;
    void __iomem *regs;
    unsigned int *dump_base;
    unsigned int back_regs_num;
    resource_size_t phy_regs;
    struct tasklet_struct tasklet;
    unsigned int version;
    struct device *dev;
    unsigned int int_mask;
    unsigned long irq_flags;
    int irq;
    int ipipe_irq;
    struct clk *clk;
    struct maa_debug status;
    struct maa_opipe_debug opipe_status[MAA_OPIPE_MAX];
    struct maa_back_regs_cfg back[MAA_BACK_REGS_NUM];
    struct wakeup_source wake_lock;
    struct workqueue_struct *workqueue;
    unsigned int sche_cnt;
    unsigned int fifo_empty;
    unsigned int need_schedule;
    struct timer_list timer;
    unsigned long timeout_jiffies;
    struct work_struct fill_work;
    unsigned int irq_status;
    struct maa_opipe opipes[MAA_OPIPE_MAX];
    struct maa_ipipe *peri_ipipes;
    struct maa_ipipe *cpu_ipipes;
    struct maa_ipipe *free_ipipe;
    struct maa_ipipe maa_lp_ipipe;
    unsigned int max_peri_ipipes;
    unsigned int max_cpu_ipipes;
    unsigned int max_opipes;
    unsigned long long use;
    unsigned int cpu_using_ipipe[MAA_OPIPE_MAX];
    unsigned int peri_using_ipipe[MAA_OPIPE_MAX];
    struct maa_allc_para *allc_para;
    unsigned int allc_para_size;
    struct maa_release_poll release;
    struct trans_report_info report_info;
    struct maa_debug_info hds_debug_info;
    unsigned int dbg;
    unsigned int intr_threhold;
    unsigned int timer_out;
    unsigned int *ipipe_write_ptr_virt;
    unsigned long long ipipe_write_ptr_phy;
    unsigned int fifo_timer_en;
    struct maa_reserve_mem reserve_mem;
    unsigned int opipe_for_free;
    unsigned int opipe_free_level;
    unsigned int opipe_use_aximem;
    unsigned long totalram;
    unsigned long freeram;
    unsigned int mem_line;
    unsigned int resetting;
    unsigned int timer_cnt;
    unsigned int deinit_cnt;
    unsigned int reinit_cnt;
    unsigned int protect_thresh;
    unsigned int release_in_task;
    unsigned int fullfill_idx;
    unsigned int fullfill_count;
};
extern struct maa *g_maa_ctx;

unsigned long long maa_map_single(struct maa *maa, unsigned long long phy, void *manger, int from, unsigned int level);

struct sk_buff *maa_unmap_skb(struct maa *maa, maa_buf_t buf, unsigned long long *orig);
struct sk_buff *maa_unmap_tcpip_skb(struct maa *maa, maa_buf_t buf);

void maa_ipipe_free(struct maa *maa, unsigned long long ptr);

unsigned long long maa_ipipe_alloc(unsigned int size);

int maa_freelist_is_empty(struct maa *maa, unsigned int level);
unsigned int maa_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth, unsigned int ipipe);
struct sk_buff *maa_to_skb(struct maa *maa, unsigned long long addr, unsigned long long *orig);
struct sk_buff *_maa_to_skb(struct maa *maa, unsigned long long addr);
static inline int maa_len_to_levels(unsigned int size)
{
    if (!size || size > MAA_OPIPE_LEVEL_1_SIZE) {
        return MAA_OPIPE_LEVEL_INVALID;
    }
    return MAA_OPIPE_LEVEL_1;
}

int maa_suspend(struct device *dev);
int maa_resume(struct device *dev);
int maa_prepare(struct device *dev);
void maa_complete(struct device *dev);
int maa_core_probe(struct maa *maa);
void maa_refill_bigpool_work(struct work_struct *work);
void maa_bigpool_timer(unsigned long data);
irqreturn_t maa_interrupt(int irq, void *dev_id);

static inline int maa_opipe_is_enable(struct maa *maa, unsigned int level)
{
    return (level <= maa->max_opipes) && (maa->opipes[level].used == MAA_OPIPE_USED_MAGIC);
}

#endif
