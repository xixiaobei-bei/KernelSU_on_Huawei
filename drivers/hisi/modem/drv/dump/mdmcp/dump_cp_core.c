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
#include "dump_exc_type.h"
#include "dump_lphy_tcm.h"
#include "dump_cphy_tcm.h"
#include "dump_easyrf_tcm.h"
#include "dump_logs.h"
#include "dump_exc_type.h"
#include "dump_cp_core.h"
#include "dump_sec_mem.h"
#include "dump_log_save.h"
#include "dump_core.h"

#undef	THIS_MODU
#define THIS_MODU mod_dump

DUMP_CP_RESET_CTRL  g_dump_cp_reset_timestamp;

void dump_reset_ctrl_int(void)
{
    /*coverity[secure_coding]*/
    memset_s(&g_dump_cp_reset_timestamp,sizeof(g_dump_cp_reset_timestamp),0,sizeof(g_dump_cp_reset_timestamp));
}

/*****************************************************************************
* 函 数 名  : dump_check_reset_timestamp
* 功能描述  : modem 频繁单独复位的特殊处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 dump_check_reset_timestamp(u32 modid)
{
    u32 diff = 0;
    NV_DUMP_STRU* cfg = NULL;
    cfg = dump_get_feature_cfg();
    if( cfg!= NULL && cfg->dump_cfg.Bits.fetal_err == 0)
    {
        return BSP_OK;
    }

    if(g_dump_cp_reset_timestamp.count % DUMP_CP_REST_TIME_COUNT == 0
        && g_dump_cp_reset_timestamp.count !=0)
    {
        diff = (g_dump_cp_reset_timestamp.reset_time[DUMP_CP_REST_TIME_COUNT -1] - g_dump_cp_reset_timestamp.reset_time[0]);
        if( diff < DUMP_CP_REST_TIME_COUNT*DUMP_CP_REST_TIME_SLICE)
        {
           dump_error("modem reset exceed default time\n ");
           return BSP_ERROR;
        }
        /*coverity[secure_coding]*/
        memset_s(&g_dump_cp_reset_timestamp,sizeof(g_dump_cp_reset_timestamp),0,sizeof(g_dump_cp_reset_timestamp));
    }
    if(modid != RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID &&  modid != RDR_MODEM_CP_RESET_USER_RESET_MOD_ID)
    {
        g_dump_cp_reset_timestamp.reset_time[g_dump_cp_reset_timestamp.count % DUMP_CP_REST_TIME_COUNT] = bsp_get_slice_value();
        g_dump_cp_reset_timestamp.count++;
    }
    return BSP_OK;

}
/*****************************************************************************
* 函 数 名  : dump_check_single_set
* 功能描述  : modem 频繁单独复位的特殊处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_check_single_reset_by_nv(void)
{

    NV_DUMP_STRU* cfg = NULL;
    cfg = dump_get_feature_cfg();
    if( cfg!= NULL && cfg->dump_cfg.Bits.sysErrReboot == 0)
    {
        dump_error("close modem sigle reset\n");
        return BSP_ERROR;
    }

    dump_ok(" modem sigle reset open\n");
    return BSP_OK;
}
/*****************************************************************************
* 函 数 名  : dump_check_single_reset_by_modid
* 功能描述  : modem 判断错误码是否需要单独复位
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
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
        dump_ok("need modem reset\n");
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
* 功能描述  : 判断是否要执行单独复位流程
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
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
        dump_set_exc_flag(false);
        return BSP_ERROR;
    }

    return BSP_OK;
}



/*****************************************************************************
* 函 数 名  : modem_error_proc
* 功能描述  : modem异常的特殊处理，主要针对dmss和noc异常
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_callback_dmss_noc_proc(u32 modid)
{


}

/*****************************************************************************
* 函 数 名  : dump_callback
* 功能描述  : modem异常的回调处理函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
extern void bsp_modem_cold_patch_update_modem_fail_count(void);
u32 dump_mdmcp_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone)
{
    bsp_modem_cold_patch_update_modem_fail_count();
    if(fndone != NULL)
    {
        fndone(modid,coreid);
    }
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_reset_fail_proc
* 功能描述  : 单独复位失败的处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_reset_fail_proc(u32 rdr_modid)
{

    rdr_system_error(rdr_modid, 0, 0);
}

/*****************************************************************************
* 函 数 名  : dump_reset_success_proc
* 功能描述  : 单独复位成功处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_reset_success_proc(void)
{
    dump_set_exc_flag(false);

}

/*****************************************************************************
* 函 数 名  : dump_reset
* 功能描述  : modem 复位处理函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_mdmcp_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    dump_ok("enter dump reset, mod id:0x%x\n", modid);

    dump_clear_cpboot_area();
    if(BSP_OK == dump_check_cp_reset(modid))
    {
        ret = bsp_cp_reset();
        if(ret == -1)
        {
            dump_reset_fail_proc(RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID);
            return;
        }


        if(!bsp_reset_is_successful(RDR_MODEM_CP_RESET_TIMEOUT))
        {
            dump_reset_fail_proc(RDR_MODEM_CP_RESET_FAIL_MOD_ID);
        }
        else
        {
            dump_reset_success_proc();
        }
    }


}
/*****************************************************************************
* 函 数 名  : bsp_dump_init
* 功能描述  : modem dump 初始化函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 __init dump_mdmcp_init(void)
{
    s32 ret ;

    ret = dump_cp_agent_init();
    if(BSP_OK != ret)
    {
        dump_error("dump cp agent init fail\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

EXPORT_SYMBOL(dump_callback_dmss_noc_proc);
EXPORT_SYMBOL_GPL(system_error);
