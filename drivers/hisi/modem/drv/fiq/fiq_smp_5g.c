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
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <bsp_fiq.h>
#include <bsp_sysctrl.h>
#include <bsp_print.h>
#include <securec.h>
#include <osl_types.h>

#undef THIS_MODU
#define THIS_MODU mod_fiq
struct per_fiq {
    void *sysctrl_fiq_enable_reg;
    unsigned int sysctrl_fiq_enable_bit;
    void *sysctrl_fiq_status_reg;
    unsigned int sysctrl_fiq_status_bit;
    void *smem_fiq_status_addr;
    void *smem_fiq_clear_addr;
    void *smem_send_cnt_addr;
    void *smem_recive_cnt_addr;
    void *smem_cp_respond_time_addr;
};

struct fiq_ctrl {
    unsigned int fiq_init;
    unsigned int chip_type;
    void *smem_base_addr;
    void *sysctrl_base_addr;
    struct per_fiq per_cpu_fiq[NRCCPU_CORE_NUM];
};

static struct fiq_ctrl g_fiq_ctrl_5g;

static void update_fiq_status(fiq_num fiq_num_t)
{
    u32 regval;
    u32 index_fiq;

    for (index_fiq = 0; index_fiq < (u32)NRCCPU_CORE_NUM; index_fiq++) {
        /* 更新FIQ的状态 */
        regval = (u32)readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr);
        regval |= ((u32)0x1 << (u32)fiq_num_t);
        writel(regval, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr);

        /* 更新中断处理次数 */
        regval = (u32)readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_send_cnt_addr);
        regval += 1;
        writel(regval, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_send_cnt_addr);
    }
}

/*
 * 接口注意事项:
 * 对于多核FIQ控制寄存器不是一个地址时，无法做到同时发送，此函数需要分别配置寄存器
 */
static void send_fiq(void)
{
    u32 regval;
    u32 index_fiq;
    u32 bit_tmp = 0;

    for (index_fiq = 0; index_fiq < (u32)NRCCPU_CORE_NUM; index_fiq++) {
        bit_tmp |= 0x1 << g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_bit;
    }
    if (g_fiq_ctrl_5g.chip_type) { /* mbb */
        writel((u32)bit_tmp, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[0].sysctrl_fiq_enable_reg);
    } else { /* PHONE */
        regval = (u32)readl((volatile const void *)(g_fiq_ctrl_5g.per_cpu_fiq[0].sysctrl_fiq_enable_reg));
        regval &= (~((u32)bit_tmp));
        regval |= (bit_tmp << 16);
        writel(regval, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[0].sysctrl_fiq_enable_reg);
    }
    bsp_err("fiq send timestamp = 0x%x\n", bsp_get_slice_value());
}
static int check_fiq_receive(void)
{
    u32 index_fiq;
    u32 result = 1;
    udelay(2000);
    bsp_err("check this time fiq status\n");

    for (index_fiq = 0; index_fiq < (u32)NRCCPU_CORE_NUM; index_fiq++) {
        /* check R8 recieve FIQ */
        bsp_err("[fiq%d]fiq send cnt = %d,fiq receive cnt = %d\n", index_fiq,
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_send_cnt_addr),
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_recive_cnt_addr));
        bsp_err("[fiq%d]smem clear value = 0x%x,smem status value = 0x%x\n", index_fiq,
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_clear_addr),
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr));
        if (readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_clear_addr) ==
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr)) {
            result = 0;
        } else {
            bsp_err("[fiq%d]modem cpu%d fiq no handle\n", index_fiq, index_fiq);
        }
    }
    if (result) {
        return BSP_ERROR;
    } else {
        return BSP_OK;
    }
}

static void check_fiq_send(void)
{
    u32 index_fiq;
    bsp_err("[fiq]check lost time fiq status\n");
    for (index_fiq = 0; index_fiq < (u32)NRCCPU_CORE_NUM; index_fiq++) {
        bsp_err("[fiq%d]fiq status regval = 0x%x[bit%d]\n", index_fiq,
            readl((volatile const void *)(g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_status_reg)),
            g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_status_bit);
        bsp_err("[fiq%d]fiq send cnt = %d,fiq receive cnt = %d\n", index_fiq,
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_send_cnt_addr),
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_recive_cnt_addr));
        bsp_err("[fiq%d]smem clear value = 0x%x,smem status value = 0x%x\n", index_fiq,
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_clear_addr),
            readl((volatile const void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr));

        writel(0x0, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_clear_addr);
        writel(0x0, (volatile void *)g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr);
    }
}
int bsp_send_nr_fiq(fiq_num fiq_num_t)
{
    if (!g_fiq_ctrl_5g.fiq_init) {
        bsp_err("fiq no init,send_nr_fiq too early\n");
        return BSP_ERROR;
    }
    if ((fiq_num_t < 0) || (fiq_num_t >= FIQ_MAX)) {
        bsp_err("fiq_num = %d error\n", fiq_num_t);
        return BSP_ERROR;
    }
    /* 检查上一次发送的情况 */
    check_fiq_send();

    /* 更新此次发送的状态 */
    update_fiq_status(fiq_num_t);

    /* 发送FIQ */
    send_fiq();

    /* 检查此次接收情况 */
    return check_fiq_receive();
}

int fiq_5g_init(void)
{
    struct device_node *node = NULL;
    u32 tmp = 0;
    struct device_node temp_node;
    struct device_node *child_node = &temp_node;
    u32 index_fiq = 0;
    if (memset_s((void *)&g_fiq_ctrl_5g, sizeof(struct fiq_ctrl), 0, sizeof(struct fiq_ctrl))) {
        bsp_err("memset fail\n");
        return -1;
    }
    g_fiq_ctrl_5g.smem_base_addr = (void *)SHM_FIQ_NR_BASE_ADDR;
    if (memset_s(g_fiq_ctrl_5g.smem_base_addr, SHM_SIZE_NRCCORE_FIQ, 0, SHM_SIZE_NRCCORE_FIQ)) {
        bsp_err("memset fail\n");
        return -1;
    }
    node = of_find_compatible_node(NULL, NULL, "hisilicon,fiq_5g");
    if (node == NULL) {
        bsp_err("get hisilicon,fiq fail!\n");
        return -1;
    }
    g_fiq_ctrl_5g.sysctrl_base_addr = of_iomap(node, 0);
    if (g_fiq_ctrl_5g.sysctrl_base_addr == NULL) {
        bsp_err("of iomap fail\n");
        return -1;
    }
    if (of_property_read_u32_array(node, "chip_type", &g_fiq_ctrl_5g.chip_type, (unsigned long)1)) {
        bsp_err("hisilicon,fiq chip_type dts node not found!\n");
        return -1;
    }

    for_each_child_of_node(node, child_node)
    {
        if (of_property_read_u32_array(child_node, "index", &index_fiq, (unsigned long)1)) {
            bsp_err("[fiq%d]hisilicon,fiq index dts node not found!\n", index_fiq);
            return -1;
        }
        if (of_property_read_u32_array(child_node, "fiq_enable_offset", &tmp, (unsigned long)1)) {
            bsp_err("[fiq%d]hisilicon,fiq fiq_enable_offset dts node not found!\n", index_fiq);
            return -1;
        }
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_reg =
            (void *)((uintptr_t)g_fiq_ctrl_5g.sysctrl_base_addr + (uintptr_t)tmp);

        if (of_property_read_u32_array(child_node, "fiq_enable_bit",
            &g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_bit, (unsigned long)1)) {
            bsp_err("[fiq%d]hisilicon,fiq fiq_enable_bit dts node not found!\n", index_fiq);
            return -1;
        }

        if (of_property_read_u32_array(child_node, "fiq_status_offset", &tmp, (unsigned long)1)) {
            bsp_err("[fiq%d]hisilicon,fiq fiq_clear_reg dts node not found!\n", index_fiq);
            return -1;
        }
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_status_reg =
            (void *)((uintptr_t)g_fiq_ctrl_5g.sysctrl_base_addr + (uintptr_t)tmp);

        if (of_property_read_u32_array(child_node, "fiq_status_bit",
            &g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].sysctrl_fiq_status_bit, (unsigned long)1)) {
            bsp_err("[fiq%d]hisilicon,fiq fiq_clear_reg dts node not found!\n", index_fiq);
            return -1;
        }
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_clear_addr =
            (void *)(SHM_FIQ_NR_CLEAR_ADDR + (uintptr_t)(0x4 * index_fiq));
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_fiq_status_addr =
            (void *)(SHM_FIQ_NR_STATUS_ADDR + (uintptr_t)(0x4 * index_fiq));
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_cp_respond_time_addr =
            (void *)(SHM_FIQ_NR_TIMER_ADDR + (uintptr_t)(0x4 * index_fiq));
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_send_cnt_addr =
            (void *)(SHM_FIQ_NR_TOTAL_SEND_CNT + (uintptr_t)(0x4 * index_fiq));
        g_fiq_ctrl_5g.per_cpu_fiq[index_fiq].smem_recive_cnt_addr =
            (void *)(SHM_FIQ_NR_TOTAL_RECIVE_CNT + (uintptr_t)(0x4 * index_fiq));
    }
    writel(0xFFFFFFFF, (volatile void *)SHM_FIQ_NR_BARRIER);

    g_fiq_ctrl_5g.fiq_init = 0x1;
    bsp_err("init OK\n");
    return 0;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE

arch_initcall(fiq_5g_init);
#endif
EXPORT_SYMBOL(bsp_send_nr_fiq);
