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
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <product_config.h>
#include "osl_sem.h"
#include "osl_spinlock.h"
#include <securec.h>
#include <bsp_print.h>
#include <mdrv_print.h>


#define THIS_MODU mod_print
#define BSP_PRINT_BUF_LEN 192
#if defined(ENABLE_BUILD_PRINT)
/*
 * 功能描述: print
 * 输入参数: u32 modid: module's id, BSP_LOG_LEVEL level: print level, char *fmt: string
 */
/*lint -save -e530 -e830*/
void mdrv_print(unsigned int level, char *fmt, ...)
{
    char print_buffer[BSP_PRINT_BUF_LEN] = {
        '\0',
    };
    va_list arglist;
    struct bsp_syslevel_ctrl sys_level = bsp_print_get_level();
    if (level == MDRV_P_CLOSE)
        return;
    va_start(arglist, fmt);
    if (vsnprintf_s(print_buffer, BSP_PRINT_BUF_LEN, (BSP_PRINT_BUF_LEN - 1), fmt, arglist) < 0) {
        va_end(arglist);
        return;
    }
    print_buffer[BSP_PRINT_BUF_LEN - 1] = '\0';

    if (sys_level.con_level >= level) {
        (void)printk(KERN_ERR "%s", print_buffer);
    } else if (sys_level.logbuf_level >= level) {
        (void)printk(KERN_INFO "%s", print_buffer);
    } else {
        va_end(arglist);
        return;
    }
    bsp_print_send_hids(level, print_buffer, arglist);
    va_end(arglist);
}
#else
void mdrv_print(unsigned int level, char *fmt, ...)
{
    char print_buffer[BSP_PRINT_BUF_LEN] = {
        '\0',
    };
    va_list arglist;
    struct bsp_syslevel_ctrl sys_level = bsp_print_get_level();
    if (level == MDRV_P_CLOSE)
        return;
    va_start(arglist, fmt);
    if (vsnprintf_s(print_buffer, BSP_PRINT_BUF_LEN, (BSP_PRINT_BUF_LEN - 1), fmt, arglist) < 0) {
        return;
    }
    va_end(arglist);
    print_buffer[BSP_PRINT_BUF_LEN - 1] = '\0';

    if (sys_level.con_level >= level) {
        (void)printk(KERN_ERR "%s", print_buffer);
    } else if (sys_level.logbuf_level >= level) {
        (void)printk(KERN_INFO "%s", print_buffer);
    }
    return;
}
#endif

/*lint -restore +e530 +e830*/
EXPORT_SYMBOL(mdrv_print);
