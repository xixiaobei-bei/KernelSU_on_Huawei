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
#include <bsp_cold_patch.h>
#include "osl_types.h"
#include "osl_thread.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_dump_mem.h"
#include "bsp_coresight.h"
#include "mdrv_om.h"
#include "nv_stru_drv.h"
#include "bsp_nvim.h"
#include "bsp_dump.h"
#include "bsp_slice.h"
#include "dump_config.h"
#include "dump_logs.h"
#include "dump_apr.h"
#include "dump_exc_handle.h"
#include "dump_cp_agent.h"
#include "dump_area.h"
#include "dump_cp_core.h"
#include "dump_mdmap_core.h"
#include "nrrdr_agent.h"
#include "dump_cp_logs.h"
#include "dump_core.h"
#include "dump_config.h"
#include "nrrdr_core.h"

#undef	THIS_MODU
#define THIS_MODU mod_dump

modem_dump_ctrl_s       g_dump_ctrl[EXC_INFO_BUTT];
u32 g_dump_init_phase = DUMP_INIT_FLAG_CONFIG;
/*****************************************************************************
* 函 数 名  : dump_get_init_phase
* 功能描述  : 获取当前的初始化阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u32 dump_get_init_phase(void)
{
    return g_dump_init_phase;
}

/*****************************************************************************
* 函 数 名  : dump_set_init_phase
* 功能描述  : 设置当前的初始化阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_set_init_phase(u32 phase)
{
    g_dump_init_phase = phase;
}
/*****************************************************************************
* 函 数 名  : dump_save_balong_rdr_info
* 功能描述  : 在手机平台上更新rdr的global 头
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
* 这里为了hids工具显示，做了特殊处理，填充在rdr的ecore与注册给rdr的不一致
*****************************************************************************/

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
 __init s32 bsp_dump_init(void)
{
    s32 ret ;

    dump_file_cfg_init();

    ret = dump_exception_handler_init();
    if(ret == BSP_ERROR)
    {
        dump_error("fail to init exception handler\n");
        return BSP_ERROR;
    }

    ret = dump_register_modem_exc_info();
    if(ret == BSP_ERROR)
    {
        return BSP_ERROR;
    }

    ret = dump_mdmcp_init();
    if(BSP_OK != ret)
    {
         return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_MDMCP);

    ret = dump_nrrdr_init();
    if(BSP_OK != ret)
    {
         return BSP_ERROR;
    }

    dump_ok("bsp_dump_init ok");
    return BSP_OK;
}


#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_dump_init);
#endif

