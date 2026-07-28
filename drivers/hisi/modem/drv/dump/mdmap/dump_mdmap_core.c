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
* 函 数 名  : dump_show_stack
* 功能描述  : 打印调用栈，用于非arm异常
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_show_stack(u32 modid,u32 reason)
{
    if (!(AP_DUMP_REASON_ARM == reason))
    {
        show_mem(0);

        if(modid == 0x11000025 || modid == 0x1100002A)
        {
            dump_error("not current task exc\n");
            show_stack(find_task_by_vpid(reason),NULL);
        }
        else
        {
            show_stack(current, NULL);
        }
    }
}

/*****************************************************************************
* 函 数 名  : dump_save_usr_data
* 功能描述  : 保存用户数据区
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_usr_data(char *data, u32 length)
{
    u32 len =0;
    void* addr = NULL;
    dump_field_map_t* pfield = NULL;

    if ((NULL != data) && (length))
    {
        pfield = (dump_field_map_t*)bsp_dump_get_field_map(DUMP_MODEMAP_USER_DATA);
        addr = (void*)bsp_dump_get_field_addr(DUMP_MODEMAP_USER_DATA);
        len = (length > DUMP_MODEMAP_USER_DATA_SIZE) ? DUMP_MODEMAP_USER_DATA_SIZE : length;

        if(addr != NULL)
        {
            /*coverity[secure_coding]*/
            memcpy_s((void *)addr, DUMP_MODEMAP_USER_DATA_SIZE,(const void * )(uintptr_t)data, (size_t)len);
        }

        if(pfield)
        {
            pfield->length = len;
        }
    }
    dump_ok("dump save usr data finish\n");
    return;
}


/*****************************************************************************
* 函 数 名  : dump_save_mdm_log
* 功能描述  : 保存modem的log
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_save_mdm_log(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length,u32 rdr_mod_id)
{
    if(mod_id == DRV_ERRNO_DUMP_CP_WDT && arg1 == DUMP_REASON_WDT)
    {
        dump_cp_wdt_proc();
        dump_save_base_info(BSP_MODU_OTHER_CORE,arg1,arg2, data,length);
    }
    else if(mod_id == DRV_ERRNO_DLOCK && arg1 == DUMP_REASON_DLOCK)
    {
        dump_cp_dlock_proc();
        dump_save_base_info(BSP_MODU_OTHER_CORE,arg1,arg2, data,length);
    }
    else
    {
        dump_save_base_info(mod_id, arg1, arg2, data, length);
    }

    if(DUMP_PHONE == dump_get_product_type())
    {
        dump_show_stack(mod_id,arg1);
        dump_save_cp_sysctrl();
        dump_save_balong_rdr_info(rdr_mod_id);
    }

}



void dump_save_reg(void)
{
    NV_DUMP_STRU* dump_nv_cfg = dump_get_feature_cfg();
    void* dump_addr = bsp_dump_get_field_addr(DUMP_MODEMAP_REGS);
    void* reg_addr = NULL;
    u32 size = 0;
    if(dump_nv_cfg !=NULL && dump_addr!= NULL )
    {
        if(dump_nv_cfg->dump_cfg.Bits.appRegSave1)
        {
            reg_addr = ioremap(dump_nv_cfg->appRegAddr1,dump_nv_cfg->appRegSize1);
            if(reg_addr != NULL)
            {
                size = dump_nv_cfg->appRegSize1 < DUMP_MODEMAP_REG_SINGLE_SIZE ?dump_nv_cfg->appRegSize1 : DUMP_MODEMAP_REG_SINGLE_SIZE;
                dump_memcpy(dump_addr,reg_addr, size/4 );
            }
            iounmap(reg_addr);
        }

        if(dump_nv_cfg->dump_cfg.Bits.appRegSave2)
        {
            reg_addr = ioremap(dump_nv_cfg->appRegAddr2,dump_nv_cfg->appRegSize2);
            if(reg_addr != NULL)
            {
                size = dump_nv_cfg->appRegSize2 < DUMP_MODEMAP_REG_SINGLE_SIZE ?dump_nv_cfg->appRegSize2 : DUMP_MODEMAP_REG_SINGLE_SIZE;
                dump_memcpy(dump_addr+DUMP_MODEMAP_REG_SINGLE_SIZE,reg_addr, size/4 );
            }
            iounmap(reg_addr);
        }

        if(dump_nv_cfg->dump_cfg.Bits.appRegSave3)
        {
            reg_addr = ioremap(dump_nv_cfg->appRegAddr3,dump_nv_cfg->appRegSize3);
            if(reg_addr != NULL)
            {
                size = dump_nv_cfg->appRegSize3 < DUMP_MODEMAP_REG_SINGLE_SIZE ?dump_nv_cfg->appRegSize3 : DUMP_MODEMAP_REG_SINGLE_SIZE;
                dump_memcpy(dump_addr+DUMP_MODEMAP_REG_SINGLE_SIZE*2,reg_addr, size/4 );
            }
            iounmap(reg_addr);
        }
    }
}
/*****************************************************************************
* 函 数 名  : dump_self_use_field_init
* 功能描述  : 初始化modem ap需要使用filed 空间
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_self_use_field_init(void)
{
    void* addr = NULL;

    NV_DUMP_STRU* dump_nv_cfg = dump_get_feature_cfg();

    addr = bsp_dump_register_field(DUMP_MODEMAP_BASE_INFO_SMP,"MDMAP_BASE_INFO",NULL,NULL,DUMP_MODEMAP_BASE_INFO_SIZE,0);
    if(addr == NULL)
    {
        dump_error("fail to get mdmap baseinfo field addr \n");
    }

    addr = bsp_dump_register_field(DUMP_MODEMAP_USER_DATA,  "USER_DATA",  NULL,NULL,DUMP_MODEMAP_USER_DATA_SIZE,0);
    if(addr == NULL)
    {
        dump_error("fail to get mdmap user data field addr\n");
    }

    if(dump_nv_cfg !=NULL
        && (dump_nv_cfg->dump_cfg.Bits.appRegSave1
        || dump_nv_cfg->dump_cfg.Bits.appRegSave2
        || dump_nv_cfg->dump_cfg.Bits.appRegSave3))
    {
        addr = bsp_dump_register_field(DUMP_MODEMAP_REGS,  "REG_DATA",  NULL,NULL,DUMP_MODEMAP_REG_SIZE,0);
        if(addr == NULL)
        {
            dump_error("fail to get mdmap reg data field addr\n");
        }
    }
    dump_ok("mdmap use field init ok\n");

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

    bsp_coresight_disable();

    dump_save_usr_data(data, length);

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
    if(BSP_ERROR == dump_check_status())
    {
        return;
    }

    dump_set_reboot_contex(DUMP_CPU_APP, DUMP_REASON_NORMAL);


    dump_save_mdm_log(mod_id, arg1,arg2,data,length,rdr_mod_id);

    dump_mdm_error_proc(rdr_mod_id, arg1, arg2);

    return;
}
/*****************************************************************************
* 函 数 名  : dump_mdmap_init
* 功能描述  : 初始化modem ap初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

__init s32 dump_mdmap_init(void)
{
    s32 ret;
    dump_self_use_field_init();

    ret = dump_base_info_init();
    if(ret == BSP_ERROR)
    {
        return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_BASEINFO);

    dump_optional_log_init();

    (void)bsp_dump_register_hook("dump_save_reg", dump_save_reg);

    return BSP_OK;


}



