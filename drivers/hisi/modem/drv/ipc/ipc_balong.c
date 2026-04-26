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
/*lint --e{537,607,701,713,718,732,746}*/
#ifdef __KERNEL__
#include <linux/version.h>
#include <linux/hwspinlock.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <soc_interrupts_app.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "securec.h"

#define DRIVER_NAME "ipc_device"
#endif

#include <osl_bio.h>
#include <osl_types.h>
#include <osl_module.h>
#include <bsp_memmap.h>
#include <bsp_ipc.h>
#include <bsp_slice.h>
#include <bsp_reset.h>
#include "ipc_balong.h"
#include <bsp_pm_om.h>

#undef THIS_MODU
#define THIS_MODU mod_ipc


spinlock_t g_ipcm_lock[IPC_SEM_BUTTOM];

/*lint --e{129, 63, 64, 409, 49, 52, 502}*/
static struct ipc_control g_ipc_ctrl = { 0 };
static struct ipc_debug_s g_ipc_debug = { 0 };

static u32 g_ipc_ccore_int_reset_flag = 0;
static u8 g_modem_reset_flag = 0;
static u32 g_ipc_resume_int_stat = 0;
static phys_addr_t g_ipc_base_addr_phy = 0;
/*lint -save -e550*/
static s32 bsp_ipc_int_enable_noirq(ipc_int_lev_e ullvl)
{
    u32 int_mask;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    /* 写中断屏蔽寄存器 */
    int_mask = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    int_mask |= (u32)1 << (u32)ullvl;
    writel(int_mask, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    return MDRV_OK;
}

s32 bsp_ipc_int_enable(ipc_int_lev_e ullvl)
{
    unsigned long flags = 0;
    s32 ret;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    /* 写中断屏蔽寄存器 */
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    ret = bsp_ipc_int_enable_noirq(ullvl);
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    return ret;
}

static s32 bsp_ipc_int_disable_noirq(ipc_int_lev_e ullvl)
{
    u32 int_mask;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    /* 写中断屏蔽寄存器 */
    int_mask = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    int_mask = int_mask & (~((u32)1 << (u32)ullvl));
    writel(int_mask, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    return MDRV_OK;
}

s32 bsp_ipc_int_disable(ipc_int_lev_e ullvl)
{
    unsigned long flags = 0;
    s32 ret;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    /* 写中断屏蔽寄存器 */
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    ret = bsp_ipc_int_disable_noirq(ullvl);
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    return ret;
}

s32 bsp_ipc_int_connect(ipc_int_lev_e ullvl, voidfuncptr routine, u32 parameter)
{
    unsigned long flags = 0;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    g_ipc_ctrl.ipc_int_table[ullvl].routine = routine;
    g_ipc_ctrl.ipc_int_table[ullvl].arg = parameter;
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    return MDRV_OK;
}

s32 bsp_ipc_int_disconnect(ipc_int_lev_e ullvl)
{
    unsigned long flags = 0;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    g_ipc_ctrl.ipc_int_table[ullvl].routine = NULL;
    g_ipc_ctrl.ipc_int_table[ullvl].arg = 0;
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    return MDRV_OK;
}

void bsp_ipc_int_mask_status_dump(void)
{
    u32 u32_int_stat1 = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    u32 u32_int_stat2 = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(g_ipc_ctrl.core_num)));
    ipc_print_error("bsp_ipc_int_mask = 0x%x\n", u32_int_stat1);
    ipc_print_error("bsp_ipc_int_stat = 0x%x\n", u32_int_stat2);
}

OSL_IRQ_FUNC(irqreturn_t, ipc_int_handler, irq, dev_id)
{
    u32 i = 0;
    u32 int_stat = 0, begin = 0, end = 0;
    u32 bit_value = 0;
    int_stat = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(g_ipc_ctrl.core_num)));
    /* 清中断 */
    writel(int_stat, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_CLR(g_ipc_ctrl.core_num)));
    /* 遍历32个中断 */
    for (i = 0; i < INTSRC_NUM; i++) {
        bit_value = int_stat & IPC_BIT_MASK(i);
        /* 如果有中断,则调用对应中断处理函数 */
        if (0 != bit_value) {
            /* 调用注册的中断处理函数 */
            if (NULL != g_ipc_ctrl.ipc_int_table[i].routine) {
                begin = bsp_get_slice_value();
                g_ipc_ctrl.last_int_cb_addr = (uintptr_t)(g_ipc_ctrl.ipc_int_table[i].routine);
                g_ipc_ctrl.ipc_int_table[i].routine(g_ipc_ctrl.ipc_int_table[i].arg);
                end = bsp_get_slice_value();
                g_ipc_debug.int_time_delta[i] = get_timer_slice_delta(begin, end);
            } else {
                ipc_print_error("BSP_DRV_IpcIntHandler:No IntConnect,MDRV_ERROR!.int num =%d\n", i);
            }
            g_ipc_debug.int_handle_times[i]++;
        }
    }
    return IRQ_HANDLED;
}

s32 bsp_ipc_int_send(ipc_int_core_e en_dst_core, ipc_int_lev_e ullvl)
{
    unsigned long flags = 0;
    if (ullvl >= INTSRC_NUM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, ullvl);
        return MDRV_ERROR;
    }
    if (en_dst_core >= IPC_CORE_BUTTOM) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, en_dst_core);
        return MDRV_ERROR;
    }

    if (g_modem_reset_flag && \
        (en_dst_core == IPC_CORE_CCORE || en_dst_core == IPC_CORE_NRCCPU)) { /* 核间信息不可以交互 */
        return IPC_ERR_MODEM_RESETING;
    }
    /* 写原始中断寄存器,产生中断 */
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    writel((u32)1 << (u32)ullvl, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_RAW_INT(en_dst_core)));

    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    g_ipc_debug.recv_int_core = en_dst_core;
    g_ipc_debug.int_send_times[en_dst_core][ullvl]++;
    return MDRV_OK;
}

static void mask_int(u32 signal_num)
{
    u32 int_mask;
    unsigned long flags = 0;
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    int_mask = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(g_ipc_ctrl.core_num)));
    int_mask = int_mask & (~((u32)1 << signal_num));
    writel(int_mask, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(g_ipc_ctrl.core_num)));
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
}

s32 bsp_ipc_sem_create(u32 signal_num)
{
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    if (g_ipc_ctrl.sem_exist[signal_num] != true) { /* 避免同一个信号量在没有删除的情况下创建多次 */
        osl_sem_init(SEM_EMPTY, &(g_ipc_ctrl.sem_ipc_task[signal_num]));
        g_ipc_ctrl.sem_exist[signal_num] = true;
        return MDRV_OK;
    } else {
        return MDRV_OK;
    }
}

s32 bsp_ipc_sem_delete(u32 signal_num)
{
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    if (g_ipc_ctrl.sem_exist[signal_num] == false) {
        ipc_print_error("semphore not exists,may be deleted already.\n");
        return MDRV_ERROR;
    } else {
        if (osl_sema_delete(&(g_ipc_ctrl.sem_ipc_task[signal_num]))) {
            ipc_print_error("Delete semphore failed.\n");
            return MDRV_ERROR;
        }
        g_ipc_ctrl.sem_exist[signal_num] = false;
        return MDRV_OK;
    }
}

int bsp_ipc_sem_take(u32 signal_num, int s32_timeout)
{
    u32 int_mask, ret;
    /* 参数检查 */
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    /* 将申请的信号量对应的释放中断清零 */
    writel((u32)1 << signal_num, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_CLR(g_ipc_ctrl.core_num)));
    ret = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num)));
    if (ret == 0) {
        mask_int(signal_num);
        g_ipc_debug.sem_take_times[signal_num]++;
        g_ipc_debug.sem_id = signal_num;
        return MDRV_OK;
    } else {
        if (g_ipc_ctrl.sem_exist[signal_num] == false) {
            ipc_print_error("need call ipc_sem_create to create this sem before call ipc_sem_take!\n");
            return MDRV_ERROR;
        }
        if (s32_timeout != 0) {
            /* 使能信号量释放中断 */
            int_mask = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(g_ipc_ctrl.core_num)));
            int_mask = int_mask | ((u32)1 << signal_num);
            writel(int_mask, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(g_ipc_ctrl.core_num)));
            if (MDRV_OK != osl_sem_downtimeout(&(g_ipc_ctrl.sem_ipc_task[signal_num]), s32_timeout)) {
                mask_int(signal_num);
                ipc_print_error("semTake timeout!\n");
                g_ipc_debug.sem_take_fail_times[signal_num]++;
                return MDRV_ERROR;
            } else {
                mask_int(signal_num);
                g_ipc_debug.sem_take_times[signal_num]++;
                g_ipc_debug.sem_id = signal_num;
                return MDRV_OK;
            }
        } else {
            return MDRV_ERROR;
        }
    }
}

s32 bsp_ipc_sem_give(u32 signal_num)
{
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    g_ipc_debug.sem_give_times[signal_num]++;
    /* 向信号量请求寄存器写0 */
    writel(0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num)));
    return MDRV_OK;
}

static u32 ipc_lsb(u32 args)
{
    u32 num = 0;
    u32 impval = args;
    if (0 == args) {
        return 0;
    }
    for (;;) {
        num++;
        if (0x1 == (impval & 0x1)) {
            break;
        }
        impval = impval >> 1;
    }
    return num;
}

/*
 * 函 数 名      : ipc_sem_int_handler
 * 功能描述  : 信号量释放中断处理函数
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值      : 无
 * 修改记录  :  2013年1月9日
 */
OSL_IRQ_FUNC(irqreturn_t, ipc_sem_int_handler, irq, dev_id)
{
    u32 int_stat = 0, u32HsCtrl = 0, u32_snum = 0, i = 32;
    int_stat = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_STAT(g_ipc_ctrl.core_num)));
    u32_snum = ipc_lsb(int_stat);
    if (u32_snum != 0) {
        do {
            /* 如果有信号量释放中断，清除该中断 */
            writel((u32)1 << --u32_snum,
                (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_CLR(g_ipc_ctrl.core_num)));
            u32HsCtrl =
                readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, u32_snum)));
            if (0 == u32HsCtrl) {
                osl_sem_up(&(g_ipc_ctrl.sem_ipc_task[u32_snum]));
            } else {
                g_ipc_debug.sem_take_fail_times[u32_snum]++;
            }
            int_stat = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_STAT(g_ipc_ctrl.core_num)));
            u32_snum = ipc_lsb(int_stat);
            i--;
        } while ((u32_snum != 0) && (i > 0));
    } else {
        return IRQ_NONE;
    }
    return IRQ_HANDLED;
}

s32 bsp_ipc_spin_lock(u32 signal_num)
{
    u32 hs_ctrl = 0;
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    for (;;) {
        hs_ctrl =
            readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num)));
        if (hs_ctrl == 0) {
            g_ipc_debug.sem_take_times[signal_num]++;
            g_ipc_debug.sem_id = signal_num;
            break;
        }
    }
    return MDRV_OK;
}

s32 bsp_ipc_spin_lock_timeout(u32 signal_num, u32 timeoutms)
{
    u32 hs_ctrl = 0;
    u32 start_time, end_time, elapsed;
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }

    start_time = bsp_get_slice_value();
    elapsed = timeoutms * bsp_get_slice_freq() / DIV_BASE_TIME; /*lint !e647*/

    for (;;) {
        hs_ctrl =
            readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num)));
        if (hs_ctrl == 0) {
            g_ipc_debug.sem_take_times[signal_num]++;
            g_ipc_debug.sem_id = signal_num;
            break;
        }

        end_time = bsp_get_slice_value();
        if (get_timer_slice_delta(start_time, end_time) > elapsed) {
            g_ipc_debug.sem_core =
                readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(g_ipc_ctrl.core_num, signal_num)));
            return MDRV_ERROR;
        }
    }
    return MDRV_OK;
}

s32 bsp_ipc_spin_lock_timeout_irqsave(unsigned int signal_num, unsigned int timeoutms, unsigned long *flags)
{
    int ret;

    spin_lock_irqsave(&g_ipcm_lock[signal_num], *flags);
    ret = bsp_ipc_spin_lock_timeout(signal_num, timeoutms);
    if (ret == MDRV_ERROR) {
        spin_unlock_irqrestore(&g_ipcm_lock[signal_num], *flags);
        return ret;
    }
    return ret;
}

s32 bsp_ipc_spin_trylock(u32 signal_num)
{
    u32 hs_ctrl;
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    hs_ctrl = readl(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num));
    if (hs_ctrl == 0) {
        g_ipc_debug.sem_take_times[signal_num]++;
        g_ipc_debug.sem_id = signal_num;
        return MDRV_OK;
    } else {
        return MDRV_ERROR;
    }
}

s32 bsp_ipc_spin_unlock(u32 signal_num)
{
    if (signal_num >= IPC_SEM_BUTTOM_ACORE) {
        ipc_print_error("[%s]Wrong para, line:%d,para = %d\n", __FUNCTION__, __LINE__, signal_num);
        return MDRV_ERROR;
    }
    writel(0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(g_ipc_ctrl.core_num, signal_num)));
    g_ipc_debug.sem_give_times[signal_num]++;
    return MDRV_OK;
}
extern void run_icc_pm_debug_callback(void);
extern void icc_wakeup_flag_set(void);
static void get_ipc_int_stat(void)
{
    u32 temp = 0x1;
    pr_info("%s +\n", __func__);
    g_ipc_resume_int_stat =
        readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(g_ipc_ctrl.core_num)));
    if (g_ipc_resume_int_stat) {
        ipc_print_error("[C SR]g_ipc_resume_int_stat = 0x%x\n", g_ipc_resume_int_stat);
        if (g_ipc_resume_int_stat & (temp << IPC_ACPU_INT_SRC_CCPU_ICC)) {
            icc_wakeup_flag_set();
            run_icc_pm_debug_callback();
        } else if (g_ipc_resume_int_stat & (temp << IPC_ACPU_INT_SRC_CCPU_PM_OM)) {
            bsp_pm_om_wakeup_stat();
        }
    }
    pr_info("%s -\n", __func__);
    return;
}
static struct syscore_ops ipc_dpm_ops = {
    .resume = get_ipc_int_stat,
};
phys_addr_t bsp_get_ipc_base_addr(void)
{
    return g_ipc_base_addr_phy;
}

static void ipc_free_sem_taked(u32 core_id)
{
    u32 i = 0;
    u32 ret = 0;

    for (i = 0; i < SEMSRC_NUM; i++) {
        /* 判断资源锁占用，如果占用，则释放 */
        ret = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(core_id, i)));
        if (ret == ((1 << HS_STATUS_LOCK) | core_id)) {
            writel(0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(core_id, i)));
        }
    }
}

#ifdef CONFIG_IPC_MSG
static struct ipc_msg g_ipc_msg = { 0 };
static int ipc_msg_init(void)
{
    struct device_node *ptr_device_node = NULL;
    const char *compatible_name = "hisilicon,ipc_v230_app";
    int ret;

    ptr_device_node = of_find_compatible_node(NULL, NULL, compatible_name); /*lint !e838*/
    if (ptr_device_node == NULL) {
        ipc_print_error("ipc msg of find fail\n");
        return BSP_ERROR;
    }

    g_ipc_msg.base_addr = of_iomap(ptr_device_node, 0);
    if (g_ipc_msg.base_addr == NULL) {
        ipc_print_error("ipc msg base fail\n");
        return BSP_ERROR;
    }

    ret = of_property_read_u32(ptr_device_node, "src_id", &g_ipc_msg.src_id); /*lint !e838*/
    if (ret) {
        ipc_print_error("ipc src_id fail\n");
        return BSP_ERROR;
    }

    ret = of_property_read_u32(ptr_device_node, "recv_mbx_id", &g_ipc_msg.recv_mbx);
    if (ret) {
        ipc_print_error("ipc recv_mbx_id fail\n");
        return BSP_ERROR;
    }

    ipc_print_error("ipc msg init success\n");
    return BSP_OK;
}
#else
static int ipc_msg_init(void)
{
    return BSP_OK;
}
#endif

static s32 bsp_ipc_probe(void)
{
    s32 ret, i; /*lint !e34*/
    u32 array_size;
    void *p_iomap_ret = NULL;
    struct device_node *node = NULL;
    int len = 0;
    int na;
    int ns;
    const __be32 *prop = NULL;
    phys_addr_t ipc_phy_base;

    if (BSP_ERROR == ipc_msg_init()) {
        ipc_print_error("ipc msg init fail\n");
        return BSP_ERROR;
    }

    node = of_find_compatible_node(NULL, NULL, "hisilicon,ipc_balong_app");
    if (node == NULL) {
        ipc_print_error("ipc of find fail\n");
        return MDRV_ERROR;
    }

    na = of_n_addr_cells(node);
    ns = of_n_size_cells(node);

    prop = of_get_property(node, "reg", &len);
    if ((prop == NULL) || len < (na + ns) * sizeof(*prop)) { /*lint !e574*/
        ipc_print_error("ipc of_get_property reg fail\n");
        return MDRV_ERROR;
    }

    ipc_phy_base = of_read_number(prop, na);
    g_ipc_base_addr_phy = ipc_phy_base;

    g_ipc_ctrl.core_num = IPC_CORE_ACORE;

    /* 内存映射，获得基址 */
    p_iomap_ret = of_iomap(node, 0);
    if (p_iomap_ret == NULL) {
        ipc_print_error("acore ipc iomap fail\n");
        return MDRV_ERROR;
    }

    for (i = 0; i < INTSRC_NUM; i++) {
        g_ipc_ctrl.sem_exist[i] = false;
    }
    array_size = sizeof(struct ipc_entry) * INTSRC_NUM;
    (void)memset_s((void *)(g_ipc_ctrl.ipc_int_table), array_size, 0x0, array_size);

    g_ipc_ctrl.ipc_base = p_iomap_ret;

    writel(0x0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    writel(0x0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(g_ipc_ctrl.core_num)));
    spin_lock_init(&g_ipc_ctrl.lock);

    /* 获取中断号 */
    g_ipc_ctrl.irq_int_no = irq_of_parse_and_map(node, 0);
    ret = request_irq(g_ipc_ctrl.irq_int_no, ipc_int_handler, IRQF_NO_SUSPEND, "ipc_irq", (void *)NULL);
    if (ret) {
        ipc_print_error("ipc int handler error,init failed\n");
        iounmap(p_iomap_ret);
        return MDRV_ERROR;
    }

    g_ipc_ctrl.irq_sem_no = irq_of_parse_and_map(node, 1);
    ret = request_irq(g_ipc_ctrl.irq_sem_no, ipc_sem_int_handler, IRQF_NO_SUSPEND, "ipc_sem", (void *)NULL);
    if (ret) {
        ipc_print_error("ipc sem handler error,init failed\n");
        iounmap(p_iomap_ret);
        return MDRV_ERROR;
    }

    register_syscore_ops(&ipc_dpm_ops);
    ipc_print_error("ipc probe success\n");
    return MDRV_OK; /*lint !e438*/
}
static s32 bsp_ipc_remove(struct platform_device *dev)
{
    free_irq(g_ipc_ctrl.irq_sem_no, NULL);
    free_irq(g_ipc_ctrl.irq_int_no, NULL);
    return MDRV_OK;
}
/*lint -restore +e550*/
void ipc_modem_reset_cb(drv_reset_cb_moment_e stage, int userdata)
{
    unsigned long flags = 0;
    spin_lock_irqsave(&g_ipc_ctrl.lock, flags);
    if (stage == MDRV_RESET_CB_BEFORE) {
        ipc_free_sem_taked((u32)IPC_CORE_CCORE);
        g_modem_reset_flag = 1;
    }
    if (stage == MDRV_RESET_RESETTING) {
        g_modem_reset_flag = 0;
    }
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flags);
    return;
}
void disable_ipc_irq(void)
{
    disable_irq(g_ipc_ctrl.irq_int_no);
    disable_irq(g_ipc_ctrl.irq_sem_no);
    return;
}
void enable_ipc_irq(void)
{
    enable_irq(g_ipc_ctrl.irq_int_no);
    enable_irq(g_ipc_ctrl.irq_sem_no);
    return;
}
void ccore_ipc_disable(void)
{
    unsigned long flag = 0;
    spin_lock_irqsave(&g_ipc_ctrl.lock, flag);
    g_ipc_ccore_int_reset_flag =
        readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));

    writel(0x0, (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));

    /* 单独复位的特殊处理，不屏蔽 */
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_CCPU_RESET_IDLE);
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_CCPU_RESET_SUCC);
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_NRCCPU_RESET_IDLE);
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_NRCCPU_RESET_SUCC);
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_MCPU);
    (void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_CCPU_ICC);

    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flag);
}
void ccore_ipc_enable(void)
{
    unsigned long flag = 0;
    spin_lock_irqsave(&g_ipc_ctrl.lock, flag);
    writel(g_ipc_ccore_int_reset_flag,
        (volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(g_ipc_ctrl.core_num)));
    spin_unlock_irqrestore(&g_ipc_ctrl.lock, flag);
}

#ifdef CONFIG_PM

static void ipc_dpm_complete(struct device *dev)
{
    if (g_ipc_resume_int_stat) {
        bsp_pm_log((unsigned int)PM_OM_AIPC, sizeof(g_ipc_resume_int_stat), (void *)&g_ipc_resume_int_stat);
        bsp_pm_om_wakeup_log();
    }

    return;
}

static s32 ipc_suspend_noirq(struct device *dev)
{
    u32 i = 0;
    u32 ret = 0;
    pr_info("%s +\n", __func__);
    for (i = 0; i < SEMSRC_NUM; i++) {
        ret = readl((const volatile void *)(g_ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(g_ipc_ctrl.core_num, i)));
        if (ret == 0x8) {
            ipc_print_error("signum id = %d is occupied\n", i);
            pr_info("%s -\n", __func__);
            return MDRV_ERROR;
        }
    }
    pr_info("%s -\n", __func__);
    return MDRV_OK;
}
static const struct dev_pm_ops balong_ipc_pm_ops = {
    .suspend_noirq = ipc_suspend_noirq,
    .complete = ipc_dpm_complete,
};

#define BALONG_DEV_PM_OPS (&balong_ipc_pm_ops)
#else
#define BALONG_DEV_PM_OPS NULL
#endif

static struct platform_driver balong_ipc_driver = {
    .remove = bsp_ipc_remove,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .pm = BALONG_DEV_PM_OPS,
    },
};

static struct platform_device balong_ipc_device = {
    .name = DRIVER_NAME,
    .id = -1,
    .num_resources = 0,
};
int hi_ipc_init(void)
{
    s32 ret;
    ret = bsp_ipc_probe();
    if (ret) {
        ipc_print_error("bsp_ipc_probe is failed!\n");
        return ret;
    }

    ret = platform_device_register(&balong_ipc_device);
    if (ret) {
        ipc_print_error("Platform ipc device register is failed!\n");
        return ret;
    }
    ret = platform_driver_register(&balong_ipc_driver);
    if (ret) {
        ipc_print_error("Platform ipc deriver register is failed!\n");
        platform_device_unregister(&balong_ipc_device);
        return ret;
    }
    return ret;
}
static void bsp_ipc_exit(void)
{
    platform_driver_unregister(&balong_ipc_driver);
    platform_device_unregister(&balong_ipc_device);
}

void bsp_ipc_debug_show(void)
{
    u32 i = 0;
    ipc_print_error("\ncurrent sem ID       : \t%d\n", g_ipc_debug.sem_id);
    ipc_print_error("\ncurrent sem core     : \t%d\n", g_ipc_debug.sem_core);
    ipc_print_error("current recv int coreid: \t%d\n", g_ipc_debug.recv_int_core);
    for (i = 0; i < INTSRC_NUM; i++) {
        ipc_print_error("sem%2d take count      : \t%d\n", i, g_ipc_debug.sem_take_times[i]);
        ipc_print_error("sem%2d give count      : \t%d\n", i, g_ipc_debug.sem_give_times[i]);
        ipc_print_error("int%2d recv count      : \t%d\n", i, g_ipc_debug.int_handle_times[i]);
        ipc_print_error("int%2d cost time       : \t%d us\n", i, g_ipc_debug.int_time_delta[i] * (DIV_BASE_TIME * DIV_BASE_TIME) / HI_TCXO_CLK);
    }
}

void bsp_int_send_info(void)
{
    u32 i = 0;
    u32 j = 0;
    for (i = 0; i < IPC_CORE_BUTTOM; i++) {
        for (j = 0; j < INTSRC_NUM; j++) {
            ipc_print_error("int%2d send to core%d count: \t%d\n", j, i, g_ipc_debug.int_send_times[i][j]);
        }
    }
}
/*lint -save -e19*/
EXPORT_SYMBOL(bsp_ipc_int_enable);
EXPORT_SYMBOL(bsp_ipc_int_disable);
EXPORT_SYMBOL(bsp_ipc_int_connect);
EXPORT_SYMBOL(bsp_ipc_int_disconnect);
EXPORT_SYMBOL(bsp_ipc_int_send);
EXPORT_SYMBOL(bsp_ipc_sem_create);
EXPORT_SYMBOL(bsp_ipc_sem_take);
EXPORT_SYMBOL(bsp_ipc_sem_give);
EXPORT_SYMBOL(bsp_ipc_spin_lock);
EXPORT_SYMBOL(bsp_ipc_spin_unlock);
EXPORT_SYMBOL(bsp_ipc_debug_show);
EXPORT_SYMBOL(bsp_int_send_info);
EXPORT_SYMBOL(bsp_ipc_sem_delete);
EXPORT_SYMBOL(bsp_ipc_spin_trylock);
EXPORT_SYMBOL(enable_ipc_irq);
EXPORT_SYMBOL(disable_ipc_irq);
EXPORT_SYMBOL(ccore_ipc_enable);
EXPORT_SYMBOL(ccore_ipc_disable);
EXPORT_SYMBOL(ipc_modem_reset_cb);
EXPORT_SYMBOL(bsp_ipc_spin_lock_timeout);
EXPORT_SYMBOL(bsp_ipc_int_mask_status_dump);

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
arch_initcall(hi_ipc_init);
module_exit(bsp_ipc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Drive Group");
#endif
/*lint -restore +e19*/
