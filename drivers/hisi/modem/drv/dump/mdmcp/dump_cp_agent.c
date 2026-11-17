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
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <asm/string.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include "osl_types.h"
#include "bsp_sysctrl.h"
#include "bsp_slice.h"
#include "bsp_wdt.h"
#include "bsp_ipc.h"
#include "bsp_fiq.h"
#include "bsp_coresight.h"
#include "bsp_dump.h"
#include "bsp_adump.h"
#include "bsp_ddr.h"
#include "bsp_slice.h"
#include "bsp_om.h"
#include "bsp_noc.h"
#include "dump_area.h"
#include "dump_cp_agent.h"
#include "dump_cp_wdt.h"
#include "dump_config.h"
#include "dump_baseinfo.h"
#include "dump_lphy_tcm.h"
#include "dump_cphy_tcm.h"
#include "dump_sec_mem.h"
#include "dump_exc_handle.h"

#undef	THIS_MODU
#define THIS_MODU mod_dump


/*****************************************************************************
* 函 数 名  : dump_int_handle
* 功能描述  : 处理modem cp发送过来的中断
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33       creat
*
*****************************************************************************/
void dump_cp_agent_handle(u32 param)
{
    dump_exception_info_s exception_info_s = {0, DUMP_REASON_NORMAL};

    dump_ok("modem ccore enter system error! timestamp:0x%x\n", bsp_get_slice_value());

    bsp_wdt_irq_disable(WDT_CCORE_ID);
    dump_ok("stop cp wdt\n");

    dump_fill_excption_info(&exception_info_s,DRV_ERRNO_DUMP_ARM_EXC,0,0,NULL,0,
                             DUMP_CPU_LRCCPU,DUMP_REASON_NORMAL,NULL,0,0,0,NULL);

    dump_register_exception(&exception_info_s);

    return;
}
/*****************************************************************************
* 函 数 名  : dump_cp_wdt_dlock_handle
* 功能描述  : c核异常但是错误要上报到ap的异常
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33       creat
*
*****************************************************************************/

void dump_cp_wdt_dlock_handle(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{

    u32  reason = 0;
    char* desc = NULL;

    dump_exception_info_s exception_info_s = {0,};


    dump_ok("[0x%x]modem cp wdt or pdlock enter system error! \n", bsp_get_slice_value());
    dump_ok("mod_id=0x%x arg1=0x%x arg2=0x%x len=0x%x\n", mod_id, arg1, arg2, length);


    if(arg1 == DUMP_REASON_WDT)
    {
        desc = "Modem CP WDT";
    }
    else if(arg1 == DUMP_REASON_DLOCK)
    {
        desc = "Modem CP DLOCK";
    }
    reason = arg1;

    dump_fill_excption_info(&exception_info_s,
                            mod_id, arg1,arg2,NULL,0,
                            DUMP_CPU_LRCCPU, reason, desc, 0, 0, 0, NULL);

    dump_register_exception(&exception_info_s);
}
/*****************************************************************************
* 函 数 名  : dump_cp_wdt_hook
* 功能描述  : cp 看门狗回调函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33       creat
*
*****************************************************************************/
void dump_cp_wdt_hook(void)
{
    dump_cp_wdt_dlock_handle(DRV_ERRNO_DUMP_CP_WDT, DUMP_REASON_WDT, 0, 0, 0);
}

/*****************************************************************************
* 函 数 名  : dump_cp_wdt_proc
* 功能描述  : 看门狗异常处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33       creat
*
*****************************************************************************/

__init s32 dump_cp_agent_init(void)
{
    int ret ;

    ret = bsp_ipc_int_connect(IPC_ACPU_SRC_CCPU_DUMP, (voidfuncptr)dump_cp_agent_handle, 0);
    if(unlikely(BSP_OK != ret))
    {
        dump_error("fail to connect ipc int\n");
        return BSP_ERROR;
    }

    ret = bsp_ipc_int_enable(IPC_ACPU_SRC_CCPU_DUMP);
    if(unlikely(BSP_OK != ret))
    {
        dump_error("fail to enbale ipc int\n");
        return BSP_ERROR;
    }

    ret = bsp_wdt_register_hook(WDT_CCORE_ID,dump_cp_wdt_hook);

    if(unlikely(BSP_OK != ret))
    {
        dump_error("fail to register wdt hook\n");
        return BSP_ERROR;
    }

    return BSP_OK;

}

