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


/*lint -save -e537*/
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
#include "drv_nv_def.h"
#include "mdrv_om.h"
#include <gunas_errno.h>
#include "modem_dump.h"
#include "modem_dump_apr.h"
#include "modem_dump_base.h"
#include "modem_dump_etb.h"
#include "bsp_dump_drv.h"
#include "modem_dump_field.h"
#include "bsp_ap_dump.h"
#include "modem_dump_area.h"
#include "bsp_wdt.h"
#include "modem_dump_wdt.h"
#include "modem_dump_hook.h"
/*lint -restore*/

mdm_dump_crtl_s                     g_mdm_dump_core_ctrl = {0,0};
extern dump_reboot_contex_s         g_dump_reboot_contex; 
struct rdr_exception_info_s g_phone_exc_info[] = {
    {
        .e_modid            = RDR_MODEM_AP_MOD_ID,
        .e_modid_end        = RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem ap reset system",
    },
    {
        .e_modid            = RDR_MODEM_CP_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem self-reset ipc",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem reboot without log",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_RESETFAIL,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem self-reset fail",
    },
    {
        .e_modid            = RDR_MODEM_CP_WDT_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_WDT_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem self-reset wdt",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_RILD_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem reset by rild",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_3RD_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem reset by 3rd modem",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem reset stub",
    },
    {
        .e_modid            = RDR_MODEM_NOC_MOD_ID,
        .e_modid_end        = RDR_MODEM_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP| RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMNOC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem noc error",
    },
    {
        .e_modid            = RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem user reset without log",
    },
    {
        .e_modid            = RDR_MODEM_DMSS_MOD_ID,
        .e_modid_end        = RDR_MODEM_DMSS_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMDMSS,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP",
        .e_desc             = "modem dmss error",
    },


};
struct rdr_exception_info_s g_mbb_exc_info[] =
{
    {
        .e_modid            = RDR_MODEM_AP_MOD_ID,
        .e_modid_end        = RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "MDMAP",
        .e_desc             = "modem ap reset system",
    },
    {
        .e_modid            = RDR_MODEM_CP_DRV_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_DRV_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP DRV",
        .e_desc             = "modem cp DRV reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_OSA_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_OSA_MOD_ID_END,
        .e_process_priority = RDR_WARN,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP OSA",
        .e_desc             = "modem cp OSA reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_OAM_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_OAM_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP OAM",
        .e_desc             = "modem cp OAM reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_GUL2_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_GUL2_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP GUL2",
        .e_desc             = "modem cp GUL2 reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_GUWAS_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_GUWAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP GUWAS",
        .e_desc             = "modem cp GUWAS reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_GUGAS_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_GUGAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP GUGAS",
        .e_desc             = "modem cp GUGAS reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_GUNAS_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_GUNAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP GUNAS",
        .e_desc             = "modem cp GUNAS reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_GUDSP_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_GUDSP_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP GUDSP",
        .e_desc             = "modem cp GUDSP reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_LPS_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_LPS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP LPS",
        .e_desc             = "modem cp LPS reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_LMSP_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_LMSP_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP LMSP",
        .e_desc             = "modem cp LMSP reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_TLDSP_MOD_ID_START,
        .e_modid_end        = RDR_MODEM_CP_TLDSP_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP TLDSP",
        .e_desc             = "modem cp TLDSP reset",
    },
    {
        .e_modid            = RDR_MODEM_CP_WDT_MOD_ID,
        .e_modid_end        = RDR_MODEM_CP_WDT_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "CP WDT",
        .e_desc             = "modem self-reset wdt",
    },
};

/*****************************************************************************
* 函 数 名  : bsp_om_save_reboot_log
* 功能描述  : 对外接口用于开关机接口保存开关机信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_om_save_reboot_log(const char * func_name, const void* caller)
{

    struct timex txc = {0,};
    struct rtc_time tm = {0,};
    char log_buff[200] = {0};
    char temp[30] = {0};

    do_gettimeofday(&(txc.time));

    rtc_time_to_tm(txc.time.tv_sec, &tm);

    /* coverity[overrun-buffer-val] */
    snprintf(temp, sizeof(temp), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    snprintf((char*)log_buff,sizeof(log_buff) ,"system reboot reason: NORMAL_RESET A CORE, FUNC:%s, caller:%p, TIME:%s\n", func_name, caller, temp);

    dump_append_file(OM_DUMP_PATH, OM_RESET_LOG, (void*)log_buff, strlen(log_buff), OM_RESET_LOG_MAX);

    dump_fetal("bsp_om_save_reboot_log finish\n");

}


/*****************************************************************************
* 函 数 名  : dump_save_rdr_global_info
* 功能描述  : 在手机平台上更新rdr的global 头
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_rdr_global_info(u32 mod_id)
{
    struct timex txc = {0,};
    struct rtc_time tm = {0,};
    char temp[30] = {0};

    struct dump_global_base_info_s* global_base_info;
    struct rdr_exception_info_s* rdr_exc_info = NULL;
    u32 i = 0;

    do_gettimeofday(&(txc.time));
    rtc_time_to_tm(txc.time.tv_sec, &tm);
    snprintf(temp, sizeof(temp), "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    global_base_info = (struct dump_global_base_info_s*)dump_get_global_baseinfo();
    if(global_base_info == NULL)
    {
        dump_fetal("get global_base_info error\n");
        return;
    }

    for(i = 0; i < (sizeof(g_phone_exc_info)/sizeof(g_phone_exc_info[0]));i++)
    {
       if(g_phone_exc_info[i].e_modid == mod_id)
       {
            rdr_exc_info = &g_phone_exc_info[i];
       }
    }
    if(rdr_exc_info == NULL)
    {
        dump_fetal("find rdr exc info error\n");
        return;
    }
    global_base_info->modid = rdr_exc_info->e_modid;
    global_base_info->arg1 = 0;
    global_base_info->arg2 = 0;
    memcpy(global_base_info->e_module, rdr_exc_info->e_from_module,16);
    global_base_info->e_type = rdr_exc_info->e_exce_type;

    /*这里为了hids工具显示，做了特殊处理，填充在rdr的ecore与注册给rdr的不一致*/
    if(g_dump_reboot_contex.reboot_core  == DUMP_CPU_COMM)
    {
        global_base_info->e_core = RDR_CP;
    }
    else if(g_dump_reboot_contex.reboot_core  == DUMP_CPU_APP)
    {
        global_base_info->e_core = RDR_AP;
    }
    global_base_info->start_flag = DUMP_START_EXCH;
    global_base_info->savefile_flag = DUMP_SAVE_FILE_END;

    memcpy(global_base_info->e_desc, rdr_exc_info->e_desc, (unsigned int)(strlen(rdr_exc_info->e_desc) < 48 ? strlen(rdr_exc_info->e_desc): 48) );
    memcpy(global_base_info->datetime,temp,24);

    dump_fetal("update modem rdr global info finish\n");

}
/*****************************************************************************
* 函 数 名  : dump_int_handle
* 功能描述  : 处理modem cp发送过来的中断
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_int_handle(s32 param)
{
    dump_base_info_t* modem_cp_base_info = NULL;
    u32 rdr_mod_id = 0;

    dump_fetal("[0x%x]================ modem ccore enter system error! ================\n", bsp_get_slice_value());
	
    if(g_mdm_dump_core_ctrl.exc_flag)
    {
        dump_fetal("exc has happen,do not deal new exc\n");
        return;
    }
    else
    {
        g_mdm_dump_core_ctrl.exc_flag = true;
    }

    g_dump_reboot_contex.reboot_core  = DUMP_CPU_COMM;

    bsp_wdt_irq_disable(WDT_CCORE_ID);
    dump_fetal("stop cp wdt finish\n");

    bsp_coresight_disable();

    if (DUMP_INIT_FLAG != g_mdm_dump_core_ctrl.init_flag)
    {
        dump_fetal("modem dump not init direct reboot\n");

        dump_wait_for_reboot();

        return;
    }

    modem_cp_base_info = (dump_base_info_t*)bsp_dump_get_field_addr(DUMP_CP_BASE_INFO);

    if(modem_cp_base_info != NULL)
    {
        if(modem_cp_base_info->modId != DRV_ERRNO_DUMP_ARM_EXC)
        {
            g_dump_reboot_contex.reboot_reason = DUMP_REASON_NORMAL;

        }
        else
        {
            g_dump_reboot_contex.reboot_reason = DUMP_REASON_ARM;
        }
    }

    dump_fetal("exc core is = 0x%x,exc reason is 0x%x\n",g_dump_reboot_contex.reboot_core,g_dump_reboot_contex.reboot_reason);

    if(DUMP_MBB == dump_get_product_type())
    {
        if(modem_cp_base_info != NULL)
        {
            rdr_mod_id = modem_cp_base_info->modId & 0xF0000000;
            dump_fetal("modem cp exc mod_id is 0x%x\n",rdr_mod_id);
        }
    }
    else
    {
        rdr_mod_id = RDR_MODEM_CP_MOD_ID;
    }

    dump_save_base_info(BSP_MODU_OTHER_CORE,0,0,0,0);

    if(DUMP_PHONE == dump_get_product_type())
    {
        dump_save_modem_sysctrl();

        dump_save_rdr_global_info(rdr_mod_id);
    }

    rdr_system_error(rdr_mod_id, 0, 0);

    return;
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
void modem_error_proc(u32 modid)
{
    if (DUMP_INIT_FLAG != g_mdm_dump_core_ctrl.init_flag)
    {
        dump_fetal("modem dump not init direct reboot\n");
        return;
    }

    g_dump_reboot_contex.reboot_core = DUMP_CPU_APP;

    g_dump_reboot_contex.reboot_reason = (modid == RDR_MODEM_NOC_MOD_ID)? DUMP_REASON_NOC : DUMP_REASON_DMSS;

    g_mdm_dump_core_ctrl.exc_flag = true;

    dump_save_exc_task();

    bsp_coresight_disable();

    dump_save_base_info(modid,0,0,0,0);

    if(DUMP_PHONE== dump_get_product_type())
    {
        dump_save_modem_sysctrl();
        dump_save_rdr_global_info(modid);
    }

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
u32 dump_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone)
{
    modem_exc_info_s* exc_info = NULL;

    if(modid == RDR_MODEM_NOC_MOD_ID || modid == RDR_MODEM_DMSS_MOD_ID  )
    {
        dump_fetal("[0x%x]================ modem noc or dmss error happened ! ================\n", bsp_get_slice_value());
        modem_error_proc(modid);
    }
    else
    {
        dump_fetal("enter dump callback, mod id:0x%x\n", modid);
    }

    if(bsp_reset_ccore_is_reboot() == 0)
    {
        dump_notify_target(modid);
    }
    else
    {
        dump_fetal("modem is reseting now,do not notify\n");
    }

    exc_info = (modem_exc_info_s*)dump_get_rdr_exc_info();

    if(exc_info == NULL)
    {
        return (u32)BSP_ERROR;
    }
    exc_info->modid  = modid;
    exc_info->coreid = coreid;
    exc_info->dump_done = fndone;

    if((strlen(logpath) + strlen(RDR_DUMP_FILE_CP_PATH)) >= RDR_DUMP_FILE_PATH_LEN - 1)
    {
        dump_fetal("log path is too long %s\n", logpath);
        return (u32)BSP_ERROR;
    }
    memset(exc_info->log_path,'\0',RDR_DUMP_FILE_PATH_LEN);
    memcpy(exc_info->log_path, logpath, strlen(logpath));
    memcpy(exc_info->log_path + strlen(logpath) , RDR_DUMP_FILE_CP_PATH, strlen(RDR_DUMP_FILE_CP_PATH));

    dump_fetal("log path is %s\n", exc_info->log_path);

    bsp_dump_save_self_addr();

    bsp_dump_hook_callback();

    dump_save_and_reboot();

    return BSP_OK;
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
void dump_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret = -1;
    u32 fail_id =  0;
    dump_fetal("enter dump reset, mod id:0x%x\n", modid);

    if(DUMP_PHONE == dump_get_product_type())
    {
        if ((RDR_MODEM_AP_MOD_ID == modid)
            || (RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID == modid))
        {
            return;
        }
        else if((RDR_MODEM_CP_MOD_ID == modid)
                || (RDR_MODEM_CP_WDT_MOD_ID == modid)
                || (RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID == modid)
                || (RDR_MODEM_CP_RESET_RILD_MOD_ID == modid)
                || (RDR_MODEM_CP_RESET_3RD_MOD_ID == modid
                || (RDR_MODEM_CP_RESET_USER_RESET_MOD_ID== modid)))
        {
            ret = bsp_cp_reset();
            if(ret == -1)
            {
                dump_fetal("bsp_cp_reset is stub,reset ap\n");

                g_dump_reboot_contex.reboot_core = DUMP_CPU_APP;
                g_dump_reboot_contex.reboot_reason= DUMP_REASON_RST_NOT_SUPPORT;

                dump_save_momdem_reset_info(RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,"MDM_RST_OFF");

                dump_save_rdr_global_info(RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID);

                rdr_system_error(RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID, 0, 0);

                return;
            }

            if (!bsp_reset_is_successful(RDR_MODEM_CP_RESET_TIMEOUT))
            {
                g_dump_reboot_contex.reboot_core = DUMP_CPU_APP;
                g_dump_reboot_contex.reboot_reason= DUMP_REASON_RST_FAIL;

                dump_fetal("modem reset fail, notify rdr\n");

                fail_id =  (u32)modem_reset_fail_id_get();

                dump_save_momdem_reset_info((u32)fail_id,"MDM_RST_FAIL");

                dump_save_rdr_global_info(RDR_MODEM_CP_RESET_FAIL_MOD_ID);

                rdr_system_error(RDR_MODEM_CP_RESET_FAIL_MOD_ID, 0, 0);
            }
            else
            {
                g_mdm_dump_core_ctrl.exc_flag = false;

                if(g_dump_reboot_contex.reboot_core  == DUMP_CPU_COMM)
                {
                    bsp_wdt_irq_enable(WDT_CCORE_ID);
                    dump_fetal("modem reset success enable cp wdt\n");

                }

                dump_base_info_init();

                g_dump_reboot_contex.reboot_core = DUMP_CPU_BUTTON;
                g_dump_reboot_contex.reboot_reason= DUMP_REASON_UNDEF;
            }
        }
        else
        {
            dump_fetal("invalid mod id: 0x%x\n", modid);
        }
    }

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
    u32 rdr_mod_id = RDR_MODEM_AP_MOD_ID;

    dump_fetal("[0x%x]================ modem acore enter system error! ================\n", bsp_get_slice_value());
    dump_fetal("mod_id=0x%x arg1=0x%x arg2=0x%x data=0x%p len=0x%x\n", mod_id, arg1, arg2, data, length);

    g_dump_reboot_contex.reboot_core  = DUMP_CPU_APP;
    g_dump_reboot_contex.reboot_reason= DUMP_REASON_NORMAL;

    rdr_mod_id = dump_match_rdr_mod_id(mod_id);

    dump_fetal("rdr mod id is 0x%x\n", rdr_mod_id);

    if(rdr_mod_id == RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID ||rdr_mod_id ==  RDR_MODEM_CP_RESET_USER_RESET_MOD_ID)
    {
        dump_fetal("rdr mod id no need to save log,enter rdr_system_error directly\n");

        rdr_system_error(rdr_mod_id, arg1, arg2);

        return;
    }

    if(g_mdm_dump_core_ctrl.exc_flag)
    {
        dump_fetal("exception has happened can not deal new exception\n");

        return;
    }
    else
    {
        g_mdm_dump_core_ctrl.exc_flag = true;
    }

    if (DUMP_INIT_FLAG != g_mdm_dump_core_ctrl.init_flag)
    {
        dump_fetal("modem dump has not init\n");
        return;
    }

    bsp_coresight_disable();

    dump_save_exc_task();

    if(DUMP_MBB == dump_get_product_type())
    {
    }
    else
    {
        dump_show_stack();
    }

    if(mod_id == DRV_ERRNO_DUMP_CP_WDT && arg1 == DUMP_REASON_WDT)
    {

        dump_fetal(" wdt error save ccore etb \n");

        dump_cs_etb(WDT_CCORE_ID);
        dump_save_cp_base_info(mod_id, arg1, arg2, data, length);
        g_dump_reboot_contex.reboot_core  = DUMP_CPU_COMM;
        g_dump_reboot_contex.reboot_reason= DUMP_REASON_WDT;

    }
    else
    {
        dump_save_base_info(mod_id, arg1, arg2, data, length);
    }

    dump_save_usr_data(data, length);

    if(DUMP_PHONE == dump_get_product_type())
    {
        dump_save_modem_sysctrl();

        dump_save_rdr_global_info(rdr_mod_id);
    }

    rdr_system_error(rdr_mod_id, arg1, arg2);

    return;
}

/*****************************************************************************
* 函 数 名  : dump_init_phase1
* 功能描述  : modem dump初始化第一阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_init_phase1(void)
{
    s32 ret = BSP_ERROR;

    dump_product_type_init();

    ret = dump_config_init();
    if(BSP_OK != ret)
    {
        dump_error("dump_init_phase1[%d]: dump_config_init failed! ret = %d\n", __LINE__, ret);
    }

    (void)bsp_om_debug_init();

    dump_memmap_modem_ddr();

    ret = bsp_ipc_int_connect(IPC_ACPU_SRC_CCPU_DUMP, (voidfuncptr)dump_int_handle, 0);
    if(BSP_OK != ret)
    {
        dump_error("dump_init_phase1[%d]: bsp_ipc_int_connect failed! ret = %d\n", __LINE__, ret);
        return BSP_ERR_DUMP_INIT_FAILED;
    }

    ret = bsp_ipc_int_enable(IPC_ACPU_SRC_CCPU_DUMP);
    if(BSP_OK != ret)
    {
        dump_error("dump_init_phase1[%d]: bsp_ipc_int_enable failed! ret = %d\n", __LINE__, ret);
        return BSP_ERR_DUMP_INIT_FAILED;
    }
    g_mdm_dump_core_ctrl.init_flag = DUMP_INIT_FLAG_PHASE1;

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_init_phase2
* 功能描述  : modem dump初始化第二阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_init_phase2(void)
{
    struct rdr_module_ops_pub   soc_ops = {NULL,NULL};
    modem_exc_info_s* exc_info = NULL;
    int i = 0;
    int ret = BSP_ERROR;

    exc_info = (modem_exc_info_s*)dump_get_rdr_exc_info();

    if(exc_info != NULL)
    {
        exc_info->dump_done = NULL;
    }

    ret = dump_coresight_memmap();
    if(ret == BSP_ERROR)
    {
        dump_fetal("coresight map fail\n");
    }

    ret = dump_base_info_init();
    if(ret == BSP_ERROR)
    {
        dump_fetal("dump_base_info_init fail\n");
        return BSP_ERROR;
    }


    ret = dump_register_hook();
    if(ret == BSP_ERROR)
    {
        dump_fetal("dump_register_hook fail\n");
    }

    ret = bsp_dump_save_task_init();
    if(ret == BSP_ERROR)
    {
        dump_fetal("dump_register_hook fail\n");
        return BSP_ERROR;
    }


    if(DUMP_PHONE == dump_get_product_type())
    {
        for(i=0; i<sizeof(g_phone_exc_info)/sizeof(struct rdr_exception_info_s); i++)
        {

            if(rdr_register_exception(&g_phone_exc_info[i]) == 0)
            {
                dump_fetal("dump init: rdr_register_exception 0x%x fail\n", g_phone_exc_info[i].e_modid);
                return BSP_ERROR;
            }
        }
    }
    else
    {
        for(i=0; i<sizeof(g_mbb_exc_info)/sizeof(struct rdr_exception_info_s); i++)
        {
            if(rdr_register_exception(&g_mbb_exc_info[i]) == 0)
            {
                dump_fetal("dump init: rdr_register_exception 0x%x fail\n", g_mbb_exc_info[i].e_modid);
                return BSP_ERROR;
            }
        }
    }


    soc_ops.ops_dump  = (pfn_dump)dump_callback;
    soc_ops.ops_reset = (pfn_reset)dump_reset;
    
    if(rdr_register_module_ops(RDR_CP, &soc_ops, &(exc_info->soc_rst)) != BSP_OK)
    {
        dump_fetal("dump init: rdr_register_soc_ops fail\n");
        return BSP_ERROR;
    }

    dump_fetal("modem dump init success!\n");

    return BSP_OK;
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
s32 __init bsp_dump_init(void)
{
    s32 ret = BSP_ERROR;

    g_mdm_dump_core_ctrl.exc_flag = false;

    ret  = dump_init_phase1();
    if(BSP_OK != ret)
    {
        return BSP_ERROR;
    }

    ret  =dump_init_phase2();
    if(BSP_OK != ret)
    {
        return BSP_ERROR;
    }

    if(dump_get_product_type() == DUMP_PHONE)
    {
        ret = bsp_apr_init();

        if(ret)
        {
            dump_fetal("dump init apr info fail\n");
        }
    }

    g_mdm_dump_core_ctrl.init_flag = DUMP_INIT_FLAG;

    return BSP_OK;
}

EXPORT_SYMBOL_GPL(system_error);
module_init(bsp_dump_init);


