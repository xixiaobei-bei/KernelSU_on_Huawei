/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

/*
 * 1 头文件包含
 */
#include <product_config.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include "osl_sem.h"
#include "osl_spinlock.h"
#include "osl_bio.h"
#include <bsp_slice.h>
#include "bsp_dump.h"
#include "bsp_print.h"
#include "bsp_socp.h"
#include "bsp_ipc.h"
#include "bsp_hardtimer.h"
#include <bsp_modem_log.h>
#include <securec.h>
#include <mntn_interface.h>
#include "bsp_om_log.h"
#include "bsp_rfile.h"
#include "bsp_pcie.h"

#define OLD_MATCH ((u32)1)
#define NEW_MATCH ((u32)0)

#define THIS_MODU mod_print
u32 g_print_close = 0;
struct bsp_syslevel_ctrl g_print_sys_level = { BSP_P_ERR, BSP_P_INFO };
struct new_om_log g_om_log;
struct delayed_work g_flush_modem_log_work;
bsp_log_string_pull_func g_print_to_hids_func = NULL;
struct bsp_print_tag g_print_tag[MODU_MAX] = {
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
};
static DEFINE_SPINLOCK(g_om_log_spinlock);
#define __LOG_BUF_LEN (1 << 17)
static char g_logbuffer[__LOG_BUF_LEN] = {0};
static u32 g_logbuf_index = 0;
#define LOG_BUF_MASK (__LOG_BUF_LEN - 1)
#define LOG_BUF(idx) (g_logbuffer[(idx) & (LOG_BUF_MASK)])
#define LOG_BUF_TIMESLICE_LEN 16
#define HIDSLEVEL2MODULE(hids_level) ((0xf << 16) | ((hids_level) << 12))
#define LOG_MOD_DELAY_30000 30000
#define LOG_MOD_DELAY_5000 5000
#define LOG_MOD_DELAY_10000 10000
void bsp_log2buf(const char *buf, u32 len)
{
    unsigned long flags;
    u32 i = 0;
    char timeslice[LOG_BUF_TIMESLICE_LEN] = {0};
    u32 length;
    length = snprintf_s(timeslice, sizeof(timeslice), sizeof(timeslice) - 1, "[%-8x]", bsp_get_slice_value());
    if (length < 0)
        printk(KERN_ERR "snprintf_s err!\n");

    spin_lock_irqsave(&g_om_log_spinlock, flags);
    length = strlen(timeslice);

    /* 记录时间戳 */
    for (i = 0; i < length; i++) {
        LOG_BUF(g_logbuf_index) = *(timeslice + i);
        g_logbuf_index++;
    }
    /* 记录buf数据 */
    length = len;
    for (i = 0; i < length; i++) {
        LOG_BUF(g_logbuf_index) = *(buf + i);
        g_logbuf_index++;
    }
    spin_unlock_irqrestore(&g_om_log_spinlock, flags);
    return;
}

void bsp_store_log_buf(char *buf, u32 len)
{
    u32 i = 0;
    u32 tempindex = 0;
    u32 length = 0;
    unsigned long flags;
    if ((buf == NULL) || (!len))
        return;

    if (len > __LOG_BUF_LEN)
        length = __LOG_BUF_LEN;
    else
        length = len;

    spin_lock_irqsave(&g_om_log_spinlock, flags);
    if (g_logbuf_index < (__LOG_BUF_LEN - 1)) {
        tempindex = 0;
    } else {
        tempindex = (g_logbuf_index + __LOG_BUF_LEN - length) & LOG_BUF_MASK;
    }
    for (i = 0; i < length; i++) {
        *(buf + i) = LOG_BUF(tempindex);
        tempindex = (tempindex + 1) & LOG_BUF_MASK;
    }
    spin_unlock_irqrestore(&g_om_log_spinlock, flags);
    return;
}

void bsp_log_string_pull_hook(bsp_log_string_pull_func hook)
{
    if (hook != NULL) {
        g_print_to_hids_func = hook;
    }
    return;
}

u32 bsp_print_level_to_hids(u32 level)
{
    switch (level) {
        case BSP_P_FATAL:
            return BSP_HIDS_LEVEL_ERROR;
        case BSP_P_ERR:
            return BSP_HIDS_LEVEL_WARNING;
        case BSP_P_WRN:
            return BSP_HIDS_LEVEL_NOTICE;
        default:
            return BSP_HIDS_LEVEL_INFO;
    }
}
#if (defined(ENABLE_BUILD_PRINT) || defined(ENABLE_BUILD_OM))
/*
 * 功能描述: 查询模块当前设置的打印级别
 * 输入参数: sel:sel = 0 close print; sel = 1 open print
 * 输出参数: 无
 * 返 回 值: 打印级别
 */
void bsp_print_control(u32 sel)
{
    g_print_close = sel;
    return;
}
/*
 * 功能描述: 系统级别设置
 * 输入参数: u32 console：控制台打印级别；u32 logbuf：default for acore
 * 输出参数: BSP_OK/BSP_ERROR
 */
s32 logs(u32 console, u32 logbuf)
{
    if (console >= BSP_LEVEL_SUM || logbuf >= BSP_LEVEL_SUM)
        return BSP_ERROR;
    g_print_sys_level.logbuf_level = logbuf;
    g_print_sys_level.con_level = console;
    return BSP_OK;
}
/*
 * 功能描述: set all modules' level
 * 输入参数: u32 level: mod_level
 * 输出参数: NA
 */
void set_all_module(u32 level)
{
    int i = 0;
    if (level >= BSP_LEVEL_SUM)
        return;
    for (i = 0; i < MODU_MAX; i++)
        g_print_tag[i].modlevel = level;

    return;
}
/*
 * 功能描述: set module's level according to modid
 * 输入参数: u32 modid: module's id, u32 level: mod_level
 * 输出参数: BSP_OK/BSP_ERROR
 * 返 回 值: 成功/失败
 */
s32 logm(u32 modid, u32 level)
{
    if (modid >= MODU_MAX) {
        bsp_err("modid is error!\n");
        return BSP_ERROR;
    }
    if (level >= BSP_LEVEL_SUM) {
        bsp_err("level can't over 5!\n");
        return BSP_ERROR;
    }
    if (mod_all == modid) {
        set_all_module(level);
    } else
        g_print_tag[modid].modlevel = level;
    return BSP_OK;
}
/*
 * 功能描述: inquire module's level according to modid
 * 输入参数: u32 modid: module's id
 * 输出参数: NA
 */
void logc(u32 modid)
{
    if (modid >= MODU_MAX) {
        bsp_err("modid is error!\n");
        return;
    }
    bsp_err("con_level:%d logbuf_level:%d mod_level:%d\n\n", g_print_sys_level.con_level,
        g_print_sys_level.logbuf_level, g_print_tag[modid].modlevel);
    return;
}
/*
 * 功能描述: print
 * 输入参数: u32 modid: module's id, BSP_LOG_LEVEL level: print level, char *fmt: string
 */
void bsp_print(module_tag_e modid, enum bsp_log_level level, char *fmt, ...)
{
    char print_buffer[BSP_PRINT_BUF_LEN] = {
        '\0',
    };
    va_list arglist;
    u32 module_id, hids_level;
    if (modid >= MODU_MAX || level == BSP_PRINT_OFF) {
        return;
    }

    if (g_print_tag[modid].modlevel < level) { // 传入级别低于模块默认级别，返回
        return;
    }

    va_start(arglist, fmt);
    if (vsnprintf_s(print_buffer, BSP_PRINT_BUF_LEN, (BSP_PRINT_BUF_LEN - 1), fmt, arglist) < 0) {
        va_end(arglist);
        return;
    }
    print_buffer[BSP_PRINT_BUF_LEN - 1] = '\0';

    (void)bsp_log2buf(print_buffer, strlen(print_buffer) + 1);

    if (g_print_sys_level.con_level >= level) {
        (void)printk(KERN_ERR "%s", print_buffer);
    } else if (g_print_sys_level.logbuf_level >= level) {
        (void)printk(KERN_INFO "%s", print_buffer);
    } else {
        va_end(arglist);
        return;
    }
    if (g_print_to_hids_func != NULL) {
        hids_level = bsp_print_level_to_hids(level);
        module_id = HIDSLEVEL2MODULE(hids_level);
        g_print_to_hids_func(module_id, DIAG_DRV_HDS_PID, print_buffer, arglist);
    }
    va_end(arglist);
}
void bsp_print_send_hids(u32 level, const char *print_buf, va_list arglist)
{
    u32 module_id, hids_level;
    if (g_print_to_hids_func != NULL) {
        hids_level = bsp_print_level_to_hids(level);
        module_id = HIDSLEVEL2MODULE(hids_level);
        g_print_to_hids_func(module_id, DIAG_DRV_HDS_PID, print_buf, arglist);
    }
}
EXPORT_SYMBOL_GPL(bsp_print);
EXPORT_SYMBOL_GPL(logs);
EXPORT_SYMBOL_GPL(logm);
EXPORT_SYMBOL_GPL(logc);
EXPORT_SYMBOL_GPL(bsp_print_control);

#endif
u32 bsp_print_get_len_limit(void)
{
    return BSP_PRINT_LEN_LIMIT;
}
EXPORT_SYMBOL_GPL(bsp_print_get_len_limit);
struct bsp_syslevel_ctrl bsp_print_get_level(void)
{
    return g_print_sys_level;
}
