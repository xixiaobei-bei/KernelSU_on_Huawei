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
#include <linux/wakelock.h>
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
#include "dump_logs.h"
#include "dump_cp_agent.h"
#include "dump_apr.h"
#include "dump_area.h"
#include "dump_exc_handle.h"
#include "dump_lphy_tcm.h"
#include "dump_cphy_tcm.h"
#include "dump_easyrf_tcm.h"
#include "dump_logs.h"
#include "dump_cp_core.h"
#include "dump_mdmap_core.h"
#include "dump_cp_logs.h"
#include "dump_core.h"
#include "dump_sec_mem.h"


#undef	THIS_MODU
#define THIS_MODU mod_dump

/*****************************************************************************
* 函 数 名  : dump_mdmcp_init
* 功能描述  : mdmcp 功能初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33     creat
*
*****************************************************************************/

__init s32 dump_mdmcp_init(void)
{
    s32 ret = BSP_OK;

    ret = dump_cp_agent_init();
    if(BSP_OK != ret)
    {
        dump_error("fail to init mdmcp \n");
        return BSP_ERROR;
    }

    dump_ok("dump mdmcp init ok\n");

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : dump_check_single_reset_by_nv
* 功能描述  : nv是否配置了单独复位的功能
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33     creat
*
*****************************************************************************/

s32 dump_check_single_reset_by_nv(void)
{

    NV_DUMP_STRU* cfg = NULL;
    cfg = dump_get_feature_cfg();
    if( cfg!= NULL && cfg->dump_cfg.Bits.sysErrReboot == 0)
    {
        dump_ok("close modem sigle reset\n");
        return BSP_ERROR;
    }

    dump_ok(" modem sigle reset open\n");

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_check_single_reset_by_modid
* 功能描述  : 当前的错误id是否支持单独复位
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33     creat
*
*****************************************************************************/

s32 dump_check_single_reset_by_modid(u32 modid)
{

    if ((RDR_MODEM_AP_MOD_ID == modid)
        || (RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID == modid))
    {
        dump_ok("need reset system: 0x%x\n", modid);
        return BSP_ERROR;
    }
    else if((( modid >= RDR_MODEM_CP_DRV_MOD_ID)
            &&( modid <= RDR_MODEM_CP_IMS_MOD_ID))
            || (RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID == modid)
            || (RDR_MODEM_CP_RESET_RILD_MOD_ID == modid)
            || (RDR_MODEM_CP_RESET_3RD_MOD_ID == modid)
            || (RDR_MODEM_CP_RESET_USER_RESET_MOD_ID == modid)
            || (RDR_MODEM_CP_RESET_DLOCK_MOD_ID == modid)
            || (RDR_MODEM_CP_WDT_MOD_ID == modid)
            || (RDR_MODEM_CP_NOC_MOD_ID == modid)
            || (RDR_MODEM_AP_DRV_MOD_ID == modid))
    {
        dump_ok("go to modem reset\n");
        return BSP_OK;
    }
    else
    {
        dump_error("invalid mod id: 0x%x\n", modid);
        return BSP_ERROR;
    }

}
/*****************************************************************************
* 函 数 名  : dump_check_cp_reset
* 功能描述  : 确认当期是否可以进行单独复位
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33     creat
*
*****************************************************************************/


s32 dump_check_cp_reset(u32 modid)
{
    if(DUMP_PHONE != dump_get_product_type())
    {
        dump_ok("mbb not support cp_reset\n");
        return BSP_ERROR;
    }
    if(BSP_ERROR == dump_check_single_reset_by_modid(modid))
    {
        dump_ok("modid not support cp_reset\n");
        return BSP_ERROR;
    }
    if(BSP_ERROR == dump_check_single_reset_by_nv())
    {
        dump_ok("dump_check_single_reset_by_nv retun not support\n");
        return BSP_ERROR;
    }

    return BSP_OK;
}
/*****************************************************************************
* 函 数 名  : dump_mdmcp_reset
* 功能描述  : 执行cp的单独复位
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33     creat
*
*****************************************************************************/

RESET_RESULT dump_mdmcp_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    dump_ok("enter dump reset, mod id:0x%x\n", modid);
    if(BSP_OK == dump_check_cp_reset(modid))
    {
        ret = bsp_cp_reset();
        if(ret == -1)
        {
            return RESET_NOT_SUPPORT;
        }
        if(!bsp_reset_is_successful(RDR_MODEM_CP_RESET_TIMEOUT))
        {
            return RESET_NOT_SUCCES;
        }
        return RESET_SUCCES;
    }
    return RESET_NOT_SUPPORT;
}
