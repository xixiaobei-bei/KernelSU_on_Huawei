/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "osl_malloc.h"
#include "bsp_dump.h"
#include "bsp_ipc.h"
#include "bsp_memmap.h"
#include "bsp_wdt.h"
#include "bsp_icc.h"
#include "bsp_onoff.h"
#include "bsp_nvim.h"
#include "bsp_softtimer.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_dump_mem.h"
#include "bsp_dump.h"
#include "bsp_coresight.h"
#include "bsp_reset.h"
#include "nv_stru_drv.h"
#include "mdrv_om.h"
#include <gunas_errno.h>
#include "bsp_adump.h"
#include "bsp_wdt.h"
#include "dump_config.h"
#include "dump_baseinfo.h"
#include "dump_apr.h"
#include "dump_area.h"
#include "dump_exc_handle.h"
#include "dump_logs.h"
#include "dump_sec_mem.h"
#include "dump_debug.h"
#include "nrrdr_agent.h"
#include "nrrdr_logs.h"


#undef	THIS_MODU
#define THIS_MODU mod_dump

dump_queue_t* g_dump_debug_queue = NULL;
spinlock_t  g_debug_lock;

void* dump_queue_init(dump_queue_t *Q, u32 elementNum)
{
    dump_queue_t* wBuff = Q;

    Q->magic  = DUMP_QUEUE_MAGIC_NUM;
    Q->maxNum = elementNum;
    Q->front = 0;
    Q->rear = 0;
    Q->num = 0;
    Q->data = (u32*)(uintptr_t)((uintptr_t)Q+sizeof(dump_queue_t));

    return wBuff;
}
/*****************************************************************************
* 函 数 名  : dump_queue_in
* 功能描述  : 入队
*
* 输入参数  :

* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年4月21日10:47:55   lixiaofan  creat
*
*****************************************************************************/
__inline__ s32 dump_queue_in(dump_queue_t *Q, u32 element)
{
    if (Q->num == Q->maxNum)
    {
        return -1;
    }

    Q->data[Q->rear] = element;
    Q->rear = (Q->rear+1) % Q->maxNum;
    Q->num++;

    return 0;
}

/*lint -save -e730 -e718 -e746*/
__inline__ s32 dump_queue_loop_in(dump_queue_t *Q, u32 element)
{
    if (unlikely(Q->num < Q->maxNum))
    {
        return dump_queue_in(Q, element);
    }
    else
    {
        Q->data[Q->rear] = element;
        Q->rear = (Q->rear+1) % Q->maxNum;
        Q->front = (Q->front+1) % Q->maxNum;
    }

    return 0;

}

void dump_debug_init(void)
{
    u8* addr = NULL;
    addr = bsp_dump_register_field(DUMP_MODEMAP_DEBUG,"mdmap_debug",NULL,NULL,DUMP_MAMP_DEGBU_SIZE,0);
    if(addr == NULL)
    {
        dump_error("dump debug init fail\n");
        return;
    }
    g_dump_debug_queue = dump_queue_init((dump_queue_t*)((uintptr_t)(void*)(addr)),(DUMP_MAMP_DEGBU_SIZE-sizeof(dump_queue_t))>>2);
    if(g_dump_debug_queue == NULL)
    {
        dump_error("dump debug init fail\n");
        return;
    }
    spin_lock_init(&g_debug_lock);
}

void dump_debug_record(u32 step,u32 data)
{
    unsigned long flags;
    if(g_dump_debug_queue != NULL)
    {
        spin_lock_irqsave(&g_debug_lock,flags);
        dump_queue_loop_in(g_dump_debug_queue,step);
        dump_queue_loop_in(g_dump_debug_queue,data);
        spin_unlock_irqrestore(&g_debug_lock,flags);
    }
}

