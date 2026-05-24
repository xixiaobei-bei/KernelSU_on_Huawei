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
#include <linux/wakelock.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "osl_malloc.h"
#include "osl_thread.h"
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
#include "bsp_coresight.h"
#include "bsp_reset.h"
#include "nv_stru_drv.h"
#include "mdrv_om.h"
#include <gunas_errno.h>
#include "bsp_adump.h"
#include "bsp_wdt.h"
#include "dump_config.h"
#include "dump_baseinfo.h"
#include "dump_logs.h"
#include "dump_cp_agent.h"
#include "dump_apr.h"
#include "dump_exc_type.h"
#include "dump_area.h"

#undef	THIS_MODU
#define THIS_MODU mod_dump

/*****************************************************************************
* 函 数 名  : dump_mdm_error_proc
* 功能描述  : 进入到modem的异常处理流程
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_mdm_error_proc(u32 rdr_mod_id, u32 arg1, u32 arg2)
{
    if((RDR_MODEM_CP_RESET_RILD_MOD_ID == rdr_mod_id)
          || (RDR_MODEM_CP_RESET_3RD_MOD_ID == rdr_mod_id)
          || (RDR_MODEM_CP_RESET_DLOCK_MOD_ID == rdr_mod_id)
          || (RDR_MODEM_CP_WDT_MOD_ID == rdr_mod_id)
          || (RDR_MODEM_AP_DRV_MOD_ID== rdr_mod_id))
    {
        dump_ok("modem ap trig modem single reset\n");
        if(BSP_OK == dump_check_reset_timestamp(rdr_mod_id))
        {
            rdr_system_error(rdr_mod_id, arg1, arg2);
        }
        else
        {
            dump_set_exc_flag(false);
            dump_error("stop modem reset\n");
        }

    }
    else
    {
        dump_ok("modem ap trig system reset\n");
        rdr_system_error(rdr_mod_id, arg1, arg2);
    }

}

/*****************************************************************************
* 函 数 名  : dump_check_status
* 功能描述  : 判断当前dump状态是否应该继续进行
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 dump_check_status(void)
{
    if(true == dump_check_has_error())
    {
        return BSP_ERROR;
    }

    if (dump_get_init_phase() < DUMP_INIT_FLAG_APR)
    {
        dump_error("modem dump has not init\n");
        return BSP_ERROR;
    }
    return BSP_OK;

}

/*****************************************************************************
* 函 数 名  : system_error
* 功能描述  : modem 异常函数入口
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    u32 rdr_mod_id;

    dump_ok("modem acore enter system error! timestamp:0x%x\n", bsp_get_slice_value());
    dump_ok("mod_id=0x%x arg1=0x%x arg2=0x%x data=0x%pK len=0x%x\n", mod_id, arg1, arg2, data, length);

    rdr_mod_id = dump_match_rdr_mod_id(mod_id);

    dump_ok("rdr mod id is 0x%x\n", rdr_mod_id);

    if(rdr_mod_id == RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID ||rdr_mod_id ==  RDR_MODEM_CP_RESET_USER_RESET_MOD_ID)
    {
        dump_ok("rdr mod id no need to save log,enter rdr_system_error directly\n");
        rdr_system_error(rdr_mod_id, arg1, arg2);
        return;
    }
    dump_mdm_error_proc(rdr_mod_id, arg1, arg2);
    return;
}



