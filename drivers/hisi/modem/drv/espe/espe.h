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
#ifndef __ESPE_H__
#define __ESPE_H__

#include <osl_types.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/smp.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/workqueue.h>

#include <product_config.h>
#include <bsp_nvim.h>
#include <nv_stru_drv.h>
#include <acore_nv_stru_drv.h>
#include <securec.h>
#ifdef FEATURE_NVA_ON
#include <nva_stru.h>
#include <nva_id.h>
#endif
#include <bsp_nva.h>

#include <bsp_maa.h>
#include <bsp_espe.h>
#include "hal/espe_hal_reg.h"
#include "hal/espe_hal_desc.h"
#include <bsp_trans_report.h>
#ifdef CONFIG_BALONG_ESPE_FW
#include <linux/spe/espe_interface.h>
#endif
#include "bsp_dt.h"

#define SPE_PORT_MAX (32)
#define SPE_PORT_MASK (0x1f)
#define SPE_CCORE_PORT_BEGIN (11)

#define SPE_NORM_PORT(x) (((x)&SPE_PORT_MASK) < SPE_PORT_NUM)
#define SPE_BR_PORT(x) (((x)&SPE_PORT_MASK) >= SPE_PORT_NUM)

#define SPE_MAX_TD 2048
#define SPE_PORT_MAGIC 0x53504500

#define SPE_DFS_T(x) (jiffies + msecs_to_jiffies(x))

/* for ipsec */
#define SPE_DFS_INTERVAL_DEFAULT (50)
#define SPE_DFS_PKT_PER_MS (5)  // when above 80mbit/s spe ipsec pkt, set ddr freq to max
#define SPE_DFS_IPSEC_THRESHOLE (SPE_DFS_INTERVAL_DEFAULT * SPE_DFS_PKT_PER_MS)
#define SPE_DEFAULT_START_LIMIT (2048)

#define SPE_FLAG_ENABLE BIT(0)
#define SPE_FLAG_SUSPEND BIT(1)

#define SPE_US_TO_32KHZ_CNT(us) ((us)*10 / 305)     /* us -> 32KHz */
#define SPE_S_TO_TIMER_CNT(s) ((s)*1000 * 10 / 312) /* 1 cnt = 31.2 ms */

#define SPE_IP_FW_TIMEOUT (SPE_S_TO_TIMER_CNT(5))       /* 5s, unit:32.768kHz时钟周期的2^10倍（31.2ms） */
#define SPE_MAC_FW_TIMEOUT (SPE_S_TO_TIMER_CNT(100))    /* 100s, unit:32.768kHz时钟周期的2^10倍（31.2ms） */
#define SPE_NCM_WRAP_TIMEOUT (SPE_US_TO_32KHZ_CNT(256)) /* 256us, unit:32khz 时钟周期（30.5us） */
#define SPE_QOS_MAC_LMTTIME (SPE_US_TO_32KHZ_CNT(8 * 1000 * 1000)) /* 8s, unit:32khz 时钟周期（30.5us） */

#define ESPE_DUMP_SIZE (1024 * 3) /* espe Application 3k sieze */
#define SPE_ADQ_INTERVAL_DEFAULT (10)
#define BIT(nr) (1UL << (nr))

#ifdef FEATURE_NVA_ON
typedef drv_spe_feature_s spe_feature_t;
#else
typedef DRV_SPE_FEATURE spe_feature_t;
#endif

#define ESPE_RD_MAX_GET_TIMES 5
#define ESPE_CLKS_MAX_NUM 3

enum espe_cpuport_result {
    ESPE_NET_RX_SUCCESS = NET_RX_SUCCESS, /* keep 'em coming, baby */
    ESPE_NET_RX_DROP = NET_RX_DROP,       /* packet dropped */
    ESPE_NET_RX_WAN,                      /* packet to wan port */
    ESPE_NET_RX_BOTTOM
};

struct desc_result_s {
    unsigned int td_result[TD_RESULT_BOTTOM];          // td result bit 0-1
    unsigned int td_ptk_drop_rsn[TD_DROP_RSN_BOTTOM];  // td result bit 2-5
    unsigned int td_pkt_fw_path[TD_FW_BOTTOM];         // rd desc bit6-16
    unsigned int td_pkt_type[TD_PKT_TYPE_BOTTOM];      // td result bit 17-19
    unsigned int td_warp[TD_WARP_BOTTOM];              // td result bit 20-21
    unsigned int td_unwrap[TD_NCM_UNWRAP_BOTTOM];      // td result bit 22-25

    unsigned int rd_result[RD_RESULT_BOTTOM];          // rd desc bit0-1
    unsigned int rd_pkt_drop_rsn[RD_DROP_RSN_BOTTOM];  // rd desc bit2-5
    unsigned int rd_pkt_fw_path[RD_INDICATE_BOTTOM];   // rd desc bit6-16
    unsigned int rd_pkt_type[RD_PKT_TYPE_BOTTOM];      // rd desc bit17-19
    unsigned int rd_finsh_wrap_rsn[RD_WRAP_BOTTOM];    // rd desc bit20-22
    unsigned int rd_send_cpu_rsn[RD_CPU_BOTTOM];       // rd desc bit23-26
    unsigned int rd_sport_cnt[SPE_PORT_NUM];           // rd desc bit0-1
};

#define DIRECT_FW_HP_PORT_WEIGHT 40000000
#define DIRECT_FW_MP_PORT_WEIGHT 20000000
#define DIRECT_FW_PE_PORT_WEIGHT 10000000

struct espe_direct_fw_ctx {
    unsigned int enable : 1;
    unsigned int alloced : 1;
    unsigned int reserve : 30;
    unsigned int port_no;
    unsigned int port_weight;
    unsigned int stream_cnt;
    struct list_head stream_list;
};

struct spe_port_ctrl_flags {
    unsigned int enable : 1;
    unsigned int enable_after_modem_reset : 1;
    unsigned int alloced : 1;
    unsigned int rd_not_empty : 1;
    unsigned int hids_upload : 1;
    unsigned int copy_port : 1;
    unsigned int reserve : 26;
};

struct espe_port_smp_ctx {
    unsigned long status;
    unsigned int def_cpu;
    call_single_data_t csd ____cacheline_aligned_in_smp;
    struct tasklet_struct ini_bh_tasklet;
};

struct spe_port_ctrl {
    unsigned int portno;
    spe_port_prop_t property;
    struct spe_port_ctrl_flags port_flags;
    struct net_device *net;
    struct espe_ops ops;
    struct espe_direct_fw_ctx direct_fw_ctx;
    struct espe_port_smp_ctx smp;

    struct sk_buff *td_param[SPE_MAX_TD];
    void *port_priv;
    spinlock_t lock;
    unsigned int ext_desc;

    /* td */
    void *td_addr;
    unsigned int td_depth;
    unsigned int td_evt_gap;
    dma_addr_t td_dma;
    unsigned int td_free; /* to be fill by software next time */
    unsigned int td_busy; /* filled by software last time */
    spinlock_t td_lock;
    struct espe_td_desc *axi_td;
    dma_addr_t td_axi_buf_dma;
    /* rd */
    void *rd_addr;
    unsigned int rd_depth;
    unsigned int rd_evt_gap;
    dma_addr_t rd_dma;
    unsigned int rd_busy; /* filled by software last time */
    unsigned int rd_free; /* to be fill by software next time */
    spinlock_t rd_lock;
    void *rd_long_buf;
    struct espe_rd_desc *axi_rd;
    dma_addr_t rd_axi_buf_dma;

    /* rate limit */
    unsigned int udp_limit_time;
    unsigned int udp_limit_cnt;
    unsigned int rate_limit_time;
    unsigned int rate_limit_byte;

    unsigned int bypassport;
    unsigned int bypassport_en;
    unsigned int priority;

    /* add by B5010 */
    unsigned int td_copy_en;
    unsigned int stick_en;
};

struct spe_port_stat {
    struct desc_result_s result;
    unsigned int rd_config;
    unsigned int rd_finished;
    unsigned int rd_finished_bytes;
    unsigned int rd_finsh_intr_complete;
    unsigned int rd_finsh_intr_complete_called;
    unsigned int rd_finsh_pkt_num;
    unsigned int rd_droped;
    unsigned int rd_sended;
    unsigned int rd_maa_zero;
    unsigned int normal_cpy_fail;
    unsigned int rewind_th_cpy_fail;
    unsigned int rewind_bh_cpy_fail;

#ifdef CONFIG_BALONG_ESPE_XFRM
    unsigned int td_xfrm_config;
    unsigned int td_xfrm_config_fail;
    unsigned int td_xfrm_rcv;
    unsigned int td_xfrm_rcv_fail;
    unsigned int td_xfrm_free_skb;
#endif
    unsigned int td_config;
    unsigned int td_kick;
    unsigned int td_config_bytes;
    unsigned int td_port_disabled;
    unsigned int td_full;
    unsigned int td_dma_null;
    unsigned int td_finsh_intr_complete;
    unsigned int td_desc_complete;
    unsigned int td_desc_fail_drop;
    unsigned int td_pkt_complete;

    unsigned int free_while_enabled;
    unsigned int free_busy;
    unsigned int disable_timeout;
    unsigned int free_td_idle;
    unsigned int free_tdq;
    unsigned int free_rd_idle;
    unsigned int free_rdq;
    unsigned int maa_to_skb_fail;
    unsigned int skb_err;
};

struct spe_port_ctx {
    struct spe_port_ctrl ctrl;
    struct spe_port_stat stat;
};

struct spe_mac_fw_ctx {
    struct list_head pending;
    struct list_head backups;
    struct kmem_cache *slab;
    spinlock_t lock;
    unsigned short macfw_timeout;
    unsigned int macfw_add_fail;
    unsigned int macfw_del_fail;
};

struct spe_ip_fw_ctx {
    void *hbucket;
    void *hbucket_empty;
    dma_addr_t hbucket_dma;
    dma_addr_t hbucket_dma_empty;
    struct dma_pool *hslab;
    struct list_head free_list;
    struct list_head wan_entry_list;
    spinlock_t free_lock;
    unsigned int free_cnt;
    unsigned int free_threhold;
    unsigned int deadtime;
    unsigned int hlist_size;
    unsigned int hitem_width;
    unsigned int hzone;
    unsigned int hrand;
    spinlock_t lock;
    unsigned short ipfw_timeout;
    unsigned int ip_fw_not_add;
};

struct spe_cpuport_ctx {
    unsigned int portno;
    unsigned int alloced;
    unsigned int threshold;
    unsigned int cpu_pktnum_per_interval;
    unsigned int cpu_pkt_max_rate;

    unsigned int cpu_wan_drop_stub;
    unsigned int cpu_wan_drop_cnt;
    unsigned int cpu_updonly;
    unsigned int cpu_updonly_comp;
    unsigned int cpu_rd_num;
    unsigned int cpu_rd_udp_drop;
    unsigned int cpu_rd_dfw_updonly_drop;
    unsigned int cpu_rd_to_wan;
    unsigned int cpu_rd_to_wan_fail;
    unsigned int cpu_rd_to_nic;
    unsigned int cpu_rd_to_nic_fail;
    unsigned int cpu_rd_to_netif_rx;
    unsigned int cpu_rd_to_netif_rx_succ;
    unsigned int cpu_rd_to_netif_rx_fail;
    unsigned int cpu_vrp_fail;
    struct espe_cd_desc *cd_desc;
};

struct spe_ipfport_ctx {
    unsigned int portno;
    unsigned int alloced;
    unsigned int porten;
    unsigned int extend_desc;
    void *spe_push_addr;
    dma_addr_t spe_push_dma;
    void *ipf_rd_rptr_addr;
    struct ethhdr ipf_eth_head;
    struct espe_ipfport_attr attr;
    void (*check_ipf_bd)(void);
    unsigned int td_fail;
};

struct spe_usbport_ctx {
    unsigned int portno;
    unsigned int alloced;
    unsigned int bypass_mode;
    struct espe_usbport_attr attr;
};

struct spe_wanport_ctx {
    unsigned int portno;
    struct list_head wan_dev_list;  // direct_fw use
    struct wan_dev_info_s *(*get_wan_info)(struct net_device *dev);
};

struct spe_adq_ctrl {
    /* ad */
    void *ad_base_addr;
    dma_addr_t ad_dma_addr;
    unsigned int adq_size;
    unsigned int adbuf_len;
    unsigned int maa_ipip_type;

    void *maa_wptr_stub_addr;
    dma_addr_t maa_wptr_stub_dma_addr;

    spinlock_t ad_lock;

    /* rd skb pool */
    struct sk_buff_head *ad_free_q;
    unsigned int ad_skb_num;
    unsigned int ad_skb_size;
    unsigned int ad_skb_used;
    unsigned int ad_skb_align;
    unsigned int ad_desc_filled;
};

struct spe_adq_stat {
    unsigned int ad_empty_cnt;
};

struct spe_adq_ctx {
    struct spe_adq_ctrl ctrl;
    struct spe_adq_stat ad_stat;
};

enum spe_adq_num {
    SPE_ADQ0,
    SPE_ADQ1,
    SPE_ADQ_BOTTOM
};

struct spe_dev_stat {
    unsigned int evt_td;
    unsigned int evt_td_complt[SPE_PORT_NUM];
    unsigned int evt_td_full[SPE_PORT_NUM];
    unsigned int evt_ad_empty[SPE_ADQ_BOTTOM];
    unsigned int evt_td_errport;
    unsigned int evt_rd;
    unsigned int evt_rd_complt[SPE_PORT_NUM];
    unsigned int evt_rd_full[SPE_PORT_NUM];
    unsigned int evt_rd_empty[SPE_PORT_NUM];
    unsigned int evt_rd_errport;
    unsigned int evt_buf_rd_err;
    unsigned int evt_unknown;
    // unsigned int wait_ready;
    unsigned int wait_idle;
    unsigned int ipfw_del;
    unsigned int ipfw_del_enter;
    unsigned int ipfw_del_leave;
    unsigned int ipfw_del_nothing_leave;
    unsigned int ipfw_add;
    unsigned int ipfw_add_enter;
    unsigned int ipfw_add_leave;
    unsigned int disable_timeout;
    unsigned int int_time;
    unsigned int wq_task_begin_time;
    unsigned int wq_task_end_time;
};

struct spe_direct_fw_flags {
    unsigned int enable : 1;
    unsigned int reserve : 31;
};

struct spe_direct_fw_ctrl {
    struct spe_direct_fw_flags dfw_flags;
    struct timer_list direct_fw_timer;
    struct list_head wan_dev_list;
    int entry_cnt;
    unsigned int port_cnt;
    unsigned int port_nums[SPE_PORT_NUM];
    unsigned char fw_port_mac[ETH_ALEN];
    unsigned int direct_fw_time_interval;  // by ms

    unsigned int entry_add;
    unsigned int entry_add_max;
    unsigned int entry_add_skb_null;
    unsigned int entry_add_l3maa_null;
    unsigned int not_tcp_udp;
    unsigned int entry_add_pkt_err_total;
    unsigned int entry_add_dfw_disabled;

    unsigned int entry_add_success;
    unsigned int ipfw_node_get_fail;
    unsigned int ip_fw_node_get_fail;
    unsigned int iph_len_err;
    unsigned int iph_frag;
    unsigned int ipproto_err;
    unsigned int add_tuple_fail;
    unsigned int no_wan_info;
    unsigned int no_packet_info;
    unsigned int entry_exist;
};

struct spe_dfs_freq {
    unsigned int freq;
    unsigned int freq_min;
    unsigned int freq_max;
};

enum spe_ports_type {
    SPE_PORTS_NOMARL,
    SPE_PORTS_BR,
    SPE_PORTS_BOTTOM
};

struct spe_pericrg {
    void __iomem *crg_addr;
    unsigned int complete_flag;
    unsigned int crg_phy_addr;
    unsigned int crg_len;
    unsigned int crg_rst_en;
    unsigned int crg_rst_dis;
    unsigned int crg_rst_stat;
    unsigned int crg_rst_mask;
};

struct spe_smp_ctx {
    void (*process_desc)(void *spe, unsigned int evt_rd_done, unsigned int evt_td_done);
    unsigned int sch_fail;
    unsigned int intr_core;
    unsigned int cpuport_core;
    unsigned int def_cpuport_core;
    unsigned int hp_cpuport_core;
    unsigned int use_hp_cpu_thr;
    struct workqueue_struct *workqueue;
    struct work_struct work;
};

struct spe {
    unsigned int spe_version;
    unsigned int flags;
    unsigned int print_once_flags;
    unsigned int print_rd_flags;
    unsigned int mask_flags;  // 0: not mask,can add entry; 1: mask add entry will succ without add one to spe hardware.
    unsigned int bugon_flag;
    unsigned int spe_mode;

    struct device *dev;
    device_node_s *np;
    struct clk **clks;
    unsigned int clks_num;
    unsigned int clk_div;
    unsigned int irq;
    unsigned int irq_interval;
    dma_addr_t res;
    void __iomem *regs;
    unsigned long portmap[SPE_PORTS_BOTTOM];

    struct spe_pericrg peri;
    struct spe_port_ctx ports[SPE_PORT_NUM];
    struct spe_adq_ctx adqs_ctx[SPE_ADQ_BOTTOM];

    struct spe_cpuport_ctx cpuport;
    struct spe_ipfport_ctx ipfport;
    struct spe_wanport_ctx wanport;
    struct spe_usbport_ctx usbport;
    struct spe_smp_ctx smp;

    struct spe_ip_fw_ctx ipfw;
    struct spe_mac_fw_ctx macfw;
    struct spe_direct_fw_ctrl direct_fw;
    spinlock_t port_alloc_lock;

#define DFS_MAX_LEVEL 10
    struct spe_dfs_freq freq_grade_array[DFS_MAX_LEVEL];
    struct timer_list spe_dfs_timer;
    unsigned int spe_dfs_enable;
    unsigned int spe_dfs_div_enable;
    int cur_dfs_grade;
    int last_dfs_grade;
    unsigned int spe_fre_grade_max;
    unsigned int dfs_timer_cnt;
    unsigned int dfs_cnt;
    unsigned int dfs_div_cnt;
    unsigned int dfs_div_fail_cnt;
    unsigned int dfs_div_threshold_pktnum;
    unsigned int byte;
    unsigned int last_byte;
    unsigned int start_byte_limit;
    unsigned int total_byte;
    unsigned int dfs_td_count;
    unsigned int dfs_td_count_last;
    unsigned int dfs_rd_count;
    unsigned int dfs_rd_count_last;
    int spe_dfs_time_interval;
    int dfs_reg_acore_id;
    int dfs_reg_ddr_id;
    int ddr_freq_requested;
    unsigned int cpu_profile[DFS_MAX_LEVEL];
    unsigned int ddr_profile[DFS_MAX_LEVEL];
    unsigned int div_profile[DFS_MAX_LEVEL];
    unsigned int xfrm_dfs_cnt;
    unsigned int div_pmctrl;

    struct spe_dev_stat stat;
    struct trans_report_info report_info;
    struct espe_om_info hds_info;

    unsigned int min_pkt_len;
    unsigned int msg_level;
    unsigned int dbg_level;
    unsigned int *entry_bak;
    unsigned int porten_bak;
    unsigned int not_idle;
    unsigned int suspend_count;
    unsigned int resume_count;
    unsigned int modem_reset_count;
    unsigned int modem_unreset_count;
    unsigned int modem_noreset_count;
    unsigned int halt;
    spe_feature_t spe_feature;
    spinlock_t pm_lock;
    unsigned int spe_rd_pkt_cnt;
    bool wakeup_flag;
    unsigned int *dump_base;
    struct timer_list spe_adq_timer;
    int spe_adq_time_interval;
    spinlock_t spe_ad_lock;
    unsigned int modem_resetting;
    spinlock_t reset_lock;
    unsigned int drop_stub;
    unsigned int clock_gate_en;
    unsigned int rd_loop_cnt;
};

extern struct spe g_espe;

#endif /* __ESPE_H__ */
