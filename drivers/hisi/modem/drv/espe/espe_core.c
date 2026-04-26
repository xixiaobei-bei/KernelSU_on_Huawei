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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <net/netfilter/nf_conntrack.h>
#include <bsp_cpufreq.h>
#include <mdrv_pm.h>
#include "bsp_pmctrl.h"
#include "espe.h"
#include "espe_core.h"
#include "espe_desc.h"
#include "espe_ip_entry.h"
#include "espe_mac_entry.h"
#include "espe_port.h"
#include "espe_dbg.h"
#include "espe_modem_reset.h"
#include "espe_interrupt.h"
#include "func/espe_xfrm.h"
#include "func/espe_direct_fw.h"


struct spe g_espe;

#define SPE_FRE_STATE_NUM 10
#define SPE_DDR_MHZ 1000
#define SPE_DFS_1K 1024

void spe_dfs_time_set(int time)
{
    struct spe *spe = &g_espe;
    spe->spe_dfs_time_interval = time;
    SPE_ERR("time interval : %d \n", spe->spe_dfs_time_interval);
}

void spe_dfs_time_get(void)
{
    struct spe *spe = &g_espe;
    SPE_ERR("time interval : %d \n", spe->spe_dfs_time_interval);
}

void spe_dfs_start_limit_set(int value_kbps)
{
    struct spe *spe = &g_espe;
    spe->start_byte_limit = value_kbps;
}

void spe_dfs_start_limit_get(void)
{
    struct spe *spe = &g_espe;
    SPE_ERR("start_byte_limit : %d (kbps)\n", spe->start_byte_limit);
}

void spe_dfs_grade_set(int x, int val1, int val2, int val3)
{
    struct spe *spe = &g_espe;
    spe->freq_grade_array[x].freq_min = val2;
    spe->freq_grade_array[x].freq_max = val3;
}

void spe_dfs_grade_get(void)
{
    struct spe *spe = &g_espe;
    int i = 0;
    for (i = 0; i < DFS_MAX_LEVEL; i++) {
        SPE_ERR("%d  %d \n ", spe->freq_grade_array[i].freq_min, spe->freq_grade_array[i].freq_max);
    }
}

static inline int spe_calc_freq_grade(struct spe *spe, unsigned int byte)
{
    int i;

    for (i = spe->spe_fre_grade_max; i > spe->last_dfs_grade; i--) {
        if (byte > spe->freq_grade_array[i].freq_max * SPE_DFS_1K) {
            return i;
        }
    }
    return i;
}

unsigned int spe_byte_to_grade(unsigned int byte)
{
    struct spe *spe = &g_espe;
    unsigned int grade = spe->last_dfs_grade;
    int i = 0;

    if (unlikely(spe->last_dfs_grade >= DFS_MAX_LEVEL)) {
        grade = spe->spe_fre_grade_max;
        return grade;
    }

    if (byte >= spe->freq_grade_array[spe->last_dfs_grade].freq_max * SPE_DFS_1K) {
        if (spe->last_byte < spe->start_byte_limit) {
            grade = spe->spe_fre_grade_max;
        } else {
            grade = spe_calc_freq_grade(spe, byte);
        }
    } else if (spe->last_dfs_grade == 0) {
        grade = spe->last_dfs_grade;
    } else {
        for (i = 0; i < spe->last_dfs_grade; i++) {
            if (byte < spe->freq_grade_array[i].freq_min * SPE_DFS_1K) {
                grade = i;
                break;
            }
        }
    }
    return grade;
}

void spe_set_peri_volt(struct spe *spe, enum peri_volt_adjust volt)
{
    SPE_INFO("spe set peri volt do not exec. \n");
    return;
}

int spe_pmctrl_creat(struct spe *spe)
{
    SPE_INFO("spe pmctrl creat do not exec. \n");
    return 0;
}

void spe_set_freq_volt(struct spe *spe, int cur_dfs_grade)
{
    enum peri_volt_adjust peri_volt = PERI_VOLT_NO_CHANGE;
    spe_div_ctrl_t spe_div;

    if (spe->spe_dfs_div_enable) {
        // B5000不支持spe调频调压, B5010支持spe调频调压; phone形态支持spe调频,不支持spe调压
        if (spe->div_profile[cur_dfs_grade] + 1 == spe->clk_div) {
            SPE_INFO("spe div no change. \n");
            return;
        } else if (spe->div_profile[cur_dfs_grade] + 1 < spe->clk_div) {  // spe 频率只有2档，0档位2分频，其他都为1分频
            peri_volt = PERI_VOLT_UP;                                     // 升压升频
        } else {
            peri_volt = PERI_VOLT_DOWN;  // 降压降频
        }

        if (peri_volt == PERI_VOLT_UP) {
            spe_set_peri_volt(spe, PERI_VOLT_UP);
        }

        spe_div.u32 = ESPE_CLK_DIV_DEFAULT;
        spe_div.bits.spe_div = spe->div_profile[cur_dfs_grade];
        writel(spe_div.u32, spe->peri.crg_addr + ESPE_CLK_DIV);
        spe->clk_div = spe->div_profile[cur_dfs_grade] + 1;
        spe->dfs_div_cnt++;

        if (peri_volt == PERI_VOLT_DOWN) {
            spe_set_peri_volt(spe, PERI_VOLT_DOWN);
        }
    }
    return;
}
static inline void spe_set_ddr_freq(struct spe *spe, int cur_dfs_grade)
{
#ifndef CONFIG_ESPE_PHONE_SOC
    unsigned int ddr_freq;
    ddr_freq = SPE_DDR_MHZ * spe->ddr_profile[spe->cur_dfs_grade];
    bsp_cpufreq_qos_update(DFS_QOS_ID_DDR_MINFREQ_E, spe->dfs_reg_ddr_id, ddr_freq);
    if (bsp_acpufreq_vote_updata(CPUFREQ_VOTE_ID_SPE, spe->cpu_profile[spe->cur_dfs_grade])) {
        WARN_ON(1);
    }
#endif
    return;
}

static void spe_dfs_ctrl_timer(unsigned long arg)
{
    struct spe *spe = (struct spe *)(uintptr_t)arg;
    unsigned long td_bytes = 0;
    unsigned long rd_bytes = 0;
    unsigned int dfs_td_count = 0;
    unsigned int dfs_rd_count = 0;
    unsigned int i = 0;

    spe->dfs_timer_cnt++;

    if (unlikely(!spe->spe_dfs_enable)) {
        SPE_ERR("espe dfs disabled ! \n");
        return;
    }

    for (i = 0; i < SPE_PORT_NUM; i++) {
        dfs_td_count += spe->ports[i].stat.td_pkt_complete;
        dfs_rd_count += spe->ports[i].stat.rd_finsh_pkt_num;
    }

    spe->dfs_td_count = dfs_td_count;
    spe->dfs_rd_count = dfs_rd_count;

    if (spe->dfs_td_count >= spe->dfs_td_count_last) {
        td_bytes = (spe->dfs_td_count - spe->dfs_td_count_last) * SPE_AD1_PKT_LEN;
    } else {
        td_bytes = (0xffffffff - spe->dfs_td_count_last + spe->dfs_td_count) * SPE_AD1_PKT_LEN;
    }

    if (spe->dfs_rd_count >= spe->dfs_rd_count_last) {
        rd_bytes = (spe->dfs_rd_count - spe->dfs_rd_count_last) * SPE_AD1_PKT_LEN;
    } else {
        rd_bytes = (0xffffffff - spe->dfs_rd_count_last + spe->dfs_rd_count) * SPE_AD1_PKT_LEN;
    }

    if (td_bytes > rd_bytes) {
        spe->total_byte = td_bytes;
    } else {
        spe->total_byte = rd_bytes;
    }

    spe->byte = (spe->total_byte / 1024) * 8 * (1000 / spe->spe_dfs_time_interval); /* kbps, 1024,8,1000 unit trans */
    spe->dfs_td_count_last = spe->dfs_td_count;
    spe->dfs_rd_count_last = spe->dfs_rd_count;
    spe->cur_dfs_grade = spe_byte_to_grade(spe->byte);

    if (spe->cur_dfs_grade != spe->last_dfs_grade) {
        spe->dfs_cnt++;
        spe_set_freq_volt(spe, spe->cur_dfs_grade);  // spe调频调压
        spe_set_ddr_freq(spe, spe->cur_dfs_grade);   // spe投票ddr频率
    }

    spe->last_dfs_grade = spe->cur_dfs_grade;
    spe->last_byte = spe->byte;
    spe->xfrm_dfs_cnt = 0;
    mod_timer(&spe->spe_dfs_timer, SPE_DFS_T(spe->spe_dfs_time_interval));
}

void spe_dfs_init(struct spe *spe)
{
    spe_div_ctrl_t spe_div;
    SPE_ERR("spe->spe_dfs_enable %u \n", spe->spe_dfs_enable);

    if (spe->spe_dfs_enable) {
        spe->cur_dfs_grade = 0;
        spe->last_dfs_grade = DFS_MAX_LEVEL;
        spe->dfs_td_count = 0;
        spe->dfs_td_count_last = 0;
        spe->dfs_rd_count = 0;
        spe->dfs_rd_count_last = 0;
        spe->ddr_freq_requested = 0;
        spe->byte = 0;
        spe->last_byte = 0;
        spe->total_byte = 0;
        spe->start_byte_limit = 0;
        spe->dfs_timer_cnt = 0;

#ifdef CONFIG_ESPE_PHONE_SOC
        SPE_ERR("spe dfs at phone mode ! \n");
#else
        SPE_ERR("spe dfs at mbb mode ! \n");
        bsp_cpufreq_qos_request(DFS_QOS_ID_DDR_MINFREQ_E, SPE_DDR_MHZ * spe->ddr_profile[0], &spe->dfs_reg_ddr_id);
#endif

        if (spe->spe_dfs_div_enable) {
            (void)spe_pmctrl_creat(spe);

            spe_div.u32 = ESPE_CLK_DIV_DEFAULT;
            spe_div.bits.spe_div = spe->div_profile[ESPE_CLK_DIV_FULL_MIN_LEVEL];
            writel(spe_div.u32, spe->peri.crg_addr + ESPE_CLK_DIV);
            spe->clk_div = spe->div_profile[ESPE_CLK_DIV_FULL_MIN_LEVEL] + 1;
            SPE_ERR("spe_dfs_div_enabled ! set div to %u\n", spe->clk_div);
        }

        init_timer(&spe->spe_dfs_timer);
        spe->spe_dfs_timer.function = spe_dfs_ctrl_timer;
        spe->spe_dfs_timer.data = (unsigned long)(uintptr_t)spe;
        spe->spe_dfs_timer.expires = SPE_DFS_T(spe->spe_dfs_time_interval);
        spe->xfrm_dfs_cnt = 0;
        add_timer(&spe->spe_dfs_timer);
    }
    return;
}

void espe_dfs_get(void)
{
    struct spe *spe = &g_espe;

    SPE_ERR("Settings \n");
    SPE_ERR("spe->spe_dfs_enable %u \n", spe->spe_dfs_enable);
    SPE_ERR("spe->spe_dfs_div_enable %u \n", spe->spe_dfs_div_enable);
    SPE_ERR("spe->spe_dfs_time_interval : %d \n", spe->spe_dfs_time_interval);
    SPE_ERR("spe->spe_fre_grade_max: %d \n", spe->spe_fre_grade_max);
    SPE_ERR("spe->dfs_div_threshold_pktnum: %d \n", spe->dfs_div_threshold_pktnum);

    SPE_ERR("Debug infos \n");
    SPE_ERR("spe->cur_dfs_grade:%d \n", spe->cur_dfs_grade);
    SPE_ERR("spe->last_dfs_grade:%d \n", spe->last_dfs_grade);
    SPE_ERR("spe->dfs_td_count:%u \n", spe->dfs_td_count);
    SPE_ERR("spe->dfs_td_count_last:%u \n", spe->dfs_td_count_last);
    SPE_ERR("spe->dfs_rd_count:%u \n", spe->dfs_rd_count);
    SPE_ERR("spe->dfs_rd_count_last:%u \n", spe->dfs_rd_count_last);
    SPE_ERR("spe->total_byte:%u \n", spe->total_byte);
    SPE_ERR("spe->byte:%u \n", spe->byte);
    SPE_ERR("spe->clk_div:%d \n", spe->clk_div);
    SPE_ERR("spe->dfs_cnt:%d \n", spe->dfs_cnt);
    SPE_ERR("spe->dfs_timer_cnt:%d \n", spe->dfs_timer_cnt);
}

void espe_transfer_ctx_init(struct spe *spe)
{
    int i;
    spe->flags = 0;
    spe->mask_flags = 0;

    spe->min_pkt_len = SPE_MIN_PKT_SIZE;
    spe->cpuport.portno = SPE_PORT_MAGIC;
    spe->ipfport.portno = SPE_PORT_MAGIC;
    spe->wanport.portno = SPE_PORT_MAGIC;
    spe->usbport.portno = SPE_PORT_MAGIC;

    spe->cpuport.cpu_pkt_max_rate = SPE_CPU_PORT_DEF_MAX_RATE;
    spe->irq_interval = SPE_INTR_DEF_INTEVAL;
    spe->cpuport.cpu_pktnum_per_interval = SPE_PER_INTERVAL_CALC(spe->cpuport.cpu_pkt_max_rate);
    spe->ipfw.ipfw_timeout = SPE_IP_FW_TIMEOUT;
    spe->macfw.macfw_timeout = SPE_MAC_FW_TIMEOUT;
    spe->usbport.attr.gether_timeout = SPE_NCM_WRAP_TIMEOUT;

    /* set timeout */
    spe_writel(spe->regs, SPE_PACK_MAX_TIME, spe->usbport.attr.gether_timeout);

    spe_writel(spe->regs, SPE_ETH_MINLEN, spe->min_pkt_len);

    if (spe->ipfport.extend_desc) {
        spe_writel(spe->regs, SPE_IPF_BD_RD_IP_HEAD, 0);
    } else {
        spe_writel(spe->regs, SPE_IPF_BD_RD_IP_HEAD, 1);
    }

    for (i = 0; i < SPE_PORT_NUM; i++) {
        /* set spe limit rate to the max */
        spe_writel(spe->regs, SPE_PORTX_UDP_LMTNUM(i), SPE_PORT_UDP_LIM_MAX);
        spe_writel(spe->regs, SPE_PORTX_LMTBYTE(i), SPE_PORT_BYTE_LIM_MAX);
    }
}

void spe_clk_gate_switch(unsigned int enable)
{
    struct spe *spe = &g_espe;
    unsigned int clken_sel = 0;

    if (enable) {
        clken_sel = 0x3f;
    }
    spe_writel(spe->regs, CLKEN_SEL, clken_sel);
    return;
}

void __spe_core_init(struct spe *spe)
{
    spe_black_white_t list;

    espe_transfer_ctx_init(spe);

    /* set ip fw hash bucket base and depth ,width unit is word */
    espe_ip_fw_htab_set();

    /* set ip fw mac fw agint timer */
    espe_aging_timer_set();

    // spe init blck white list
    list.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);
    list.bits.spe_mac_black_white = 1;
    list.bits.spe_ip_black_white = 1;
    spe_writel(spe->regs, SPE_BLACK_WHITE, list.u32);

    espe_direct_fw_init(spe);
    espe_reset_modem_init();
    spin_lock_init(&spe->reset_lock);

    espe_ackpkt_identify(spe, ESPE_DEFAULT_IPF_PORT);

    espe_om_dump_init(spe);

    if (spe->clock_gate_en) {
        spe_clk_gate_switch(true);
    }

    spe->wakeup_flag = FALSE;

#ifndef CONFIG_ESPE_PHONE_SOC
    if (spe->spe_version == ESPE_VER_V200 || spe->spe_version == ESPE_VER_V300) {
        espe_adq_timer_init(spe);
        spin_lock_init(&spe->spe_ad_lock);
    }
#endif
    return;
}

int spe_core_init(struct spe *spe)
{
    int ret;

    espe_port_init(spe);

    ret = espe_ip_entry_table_init(spe);
    if (ret) {
        SPE_ERR("spe_entry_table_init failed\n");
        goto error;
    }

    ret = espe_mac_entry_table_init(spe);
    if (ret) {
        SPE_ERR("spe_entry_table_init failed\n");
        goto error;
    }
#ifdef SPE_MAA
    ret = espe_ad_init(spe);
    if (ret) {
        SPE_ERR("espe_ad_init failed\n");
        goto error;
    }
#else
    ret = espe_ad_no_maa_init(spe);
    if (ret) {
        SPE_ERR("espe_ad_no_maa_init failed\n");
        goto error;
    }
#endif
    ret = espe_interrput_init(spe);
    if (ret) {
        SPE_ERR("failed to request irq #%d --> %d\n", spe->irq, ret);
        goto error;
    }
    __spe_core_init(spe);

    return 0;

error:
    espe_ad_exit(spe);
    espe_ip_entry_table_exit(spe);
    espe_mac_entry_table_exit(spe);

    return ret;
}

void spe_core_exit(struct spe *spe)
{
    SPE_TRACE("enter\n");

    if (spe->irq) {
        free_irq(spe->irq, spe);
        spe->irq = 0;
    }

    espe_ip_entry_table_exit(spe);
    espe_mac_entry_table_exit(spe);


    SPE_TRACE("exit\n");

    return;
}

void espe_transfer_pause(void)
{
    struct spe *spe = &g_espe;
    unsigned long flags;
    unsigned int tab_clean;

    spin_lock_irqsave(&spe->macfw.lock, flags);
    spe_writel(spe->regs, SPE_HASH_DEPTH, SPE_IPFW_HTABLE_EMPTY_SIZE);
    spe_writel(spe->regs, SPE_HASH_BADDR_H, upper_32_bits(spe->ipfw.hbucket_dma_empty));
    spe_writel(spe->regs, SPE_HASH_BADDR_L, lower_32_bits(spe->ipfw.hbucket_dma_empty));
    spe_writel(spe->regs, SPE_HASH_CACHE_EN, 0);

    tab_clean = spe_readl(spe->regs, SPE_TAB_CLR);
    tab_clean |= BIT(1);
    tab_clean |= BIT(4); /* get bit 4 */
    spe_writel(spe->regs, SPE_TAB_CLR, tab_clean);

    spe->mask_flags = 1;
    spin_unlock_irqrestore(&spe->macfw.lock, flags);

    SPE_ERR("spe hardware forward pause\n");
}

void espe_transfer_restart(void)
{
    struct spe *spe = &g_espe;
    unsigned long flags;
    unsigned int tab_clean;
    spin_lock_irqsave(&spe->macfw.lock, flags);
    spe_writel(spe->regs, SPE_HASH_BADDR_H, upper_32_bits(spe->ipfw.hbucket_dma));
    spe_writel(spe->regs, SPE_HASH_BADDR_L, lower_32_bits(spe->ipfw.hbucket_dma));
    spe_writel(spe->regs, SPE_HASH_DEPTH, spe->ipfw.hlist_size);
    spe_writel(spe->regs, SPE_HASH_CACHE_EN, 1);

    tab_clean = spe_readl(spe->regs, SPE_TAB_CLR);
    tab_clean &= ~(BIT(1));
    tab_clean &= ~(BIT(4)); /* get bit 4 */
    spe_writel(spe->regs, SPE_TAB_CLR, tab_clean);

    spe->mask_flags = 0;
    {
        struct espe_mac_fw_entry_ext *pos = NULL;
        struct espe_mac_fw_entry_ext *n = NULL;

        list_for_each_entry_safe(pos, n, &spe->macfw.backups, list)
        {
            espe_mac_entry_config(SPE_TAB_CTRL_DEL_MAC_FW, (unsigned int *)&pos->ent,
                                  sizeof(pos->ent) / sizeof(unsigned int));
            espe_mac_entry_config(SPE_TAB_CTRL_ADD_MAC_FW, (unsigned int *)&pos->ent,
                                  sizeof(pos->ent) / sizeof(unsigned int));
        }
    }

    spin_unlock_irqrestore(&spe->macfw.lock, flags);
    SPE_ERR("spe hardware forward restart\n");
}

void bsp_espe_transfer_pause(void)
{
    espe_transfer_pause();
}

void bsp_espe_transfer_restart(void)
{
    espe_transfer_restart();
}

void mdrv_espe_transfer_pause(void)
{
    struct spe *spe = &g_espe;

    if (spe->spe_version == ESPE_VER_V200) {
        espe_transfer_pause();
    }
}

void mdrv_espe_transfer_restart(void)
{
    struct spe *spe = &g_espe;

    if (spe->spe_version == ESPE_VER_V200) {
        espe_transfer_restart();
    }
}

EXPORT_SYMBOL(g_espe);

MODULE_ALIAS("hisilicon network hardware accelerator driver");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(spe) driver");

