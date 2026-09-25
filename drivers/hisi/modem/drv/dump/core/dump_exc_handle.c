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
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <asm/string.h>
#include "drv_comm.h"
#include "osl_types.h"
#include "osl_thread.h"
#include "osl_sem.h"
#include "mdrv_errno.h"
#include "bsp_dump.h"
#include "bsp_dump_mem.h"
#include "bsp_slice.h"
#include "bsp_reset.h"
#include "bsp_coresight.h"
#include "bsp_wdt.h"
#include "bsp_noc.h"
#include "gunas_errno.h"
#include "dump_config.h"
#include "dump_baseinfo.h"
#include "dump_cp_agent.h"
#include "dump_area.h"
#include "dump_cp_wdt.h"
#include "dump_logs.h"
#include "dump_area.h"
#include "dump_exc_handle.h"
#include "dump_cp_core.h"
#include "dump_mdmap_core.h"
#include "nrrdr_core.h"
#include "dump_logs.h"
#include "dump_core.h"
#include "bsp_cold_patch.h"
#include "dump_debug.h"

#undef	THIS_MODU
#define THIS_MODU mod_dump


#define DUMP_NR_EXCINFO_SIZE_48	(48)
rdr_exc_info_s                  g_rdr_exc_info[EXC_INFO_BUTT];

dump_exception_ctrl_s           g_exception_ctrl;

dump_exception_info_s           g_curr_excption[EXC_INFO_BUTT];

DUMP_CP_RESET_CTRL              g_dump_mdm_reset_record;

#define RDR_MODID_TO_EXCNAME(x)     x,#x
DUMP_MOD_ID g_dump_cp_mod_id[] ={
    {RDR_MODEM_CP_DRV_MOD_ID_START      ,RDR_MODEM_CP_DRV_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_DRV_MOD_ID)},
    {RDR_MODEM_CP_OSA_MOD_ID_START      ,RDR_MODEM_CP_OSA_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_OSA_MOD_ID)},
    {RDR_MODEM_CP_OAM_MOD_ID_START      ,RDR_MODEM_CP_OAM_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_OAM_MOD_ID)},
    {RDR_MODEM_CP_GUL2_MOD_ID_START     ,RDR_MODEM_CP_GUL2_MOD_ID_END   ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_GUL2_MOD_ID)},
    {RDR_MODEM_CP_CTTF_MOD_ID_START     ,RDR_MODEM_CP_CTTF_MOD_ID_END   ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_CTTF_MOD_ID)},
    {RDR_MODEM_CP_GUWAS_MOD_ID_START    ,RDR_MODEM_CP_GUWAS_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_GUWAS_MOD_ID)},
    {RDR_MODEM_CP_CAS_MOD_ID_START      ,RDR_MODEM_CP_CAS_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_CAS_MOD_ID)},
    {RDR_MODEM_CP_CPROC_MOD_ID_START    ,RDR_MODEM_CP_CPROC_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_CPROC_MOD_ID)},
    {RDR_MODEM_CP_GUGAS_MOD_ID_START    ,RDR_MODEM_CP_GUGAS_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_GUGAS_MOD_ID)},
    {RDR_MODEM_CP_GUCNAS_MOD_ID_START   ,RDR_MODEM_CP_GUCNAS_MOD_ID_END ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_GUCNAS_MOD_ID)},
    {RDR_MODEM_CP_GUDSP_MOD_ID_START    ,RDR_MODEM_CP_GUDSP_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_GUDSP_MOD_ID)},
    {RDR_MODEM_CP_EASYRF_MOD_ID_START   ,RDR_MODEM_CP_EASYRF_MOD_ID_END ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_EASYRF_MOD_ID)},
    {RDR_MODEM_CP_MSP_MOD_ID_START      ,RDR_MODEM_CP_MSP_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_LMSP_MOD_ID)},
    {RDR_MODEM_CP_LPS_MOD_ID_START      ,RDR_MODEM_CP_LPS_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_LPS_MOD_ID)},
    {RDR_MODEM_CP_TLDSP_MOD_ID_START    ,RDR_MODEM_CP_TLDSP_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_TLDSP_MOD_ID)},
    {RDR_MODEM_CP_NRDSP_MOD_ID_START    ,RDR_MODEM_CP_NRDSP_MOD_ID_END  ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_NRDSP_MOD_ID)},
    {RDR_MODEM_CP_CPHY_MOD_ID_START     ,RDR_MODEM_CP_CPHY_MOD_ID_END   ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_CPHY_MOD_ID)},
    {RDR_MODEM_CP_IMS_MOD_ID_START      ,RDR_MODEM_CP_IMS_MOD_ID_END    ,RDR_MODID_TO_EXCNAME(RDR_MODEM_CP_IMS_MOD_ID)},
};

struct rdr_exception_info_s g_modem_exc_info[] = {
#ifdef  BSP_CONFIG_PHONE_TYPE

    {
        .e_modid            = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMAP",
        .e_desc             = "modem ap reset system",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_AP_DRV_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_AP_DRV_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP ,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMAP",
        .e_desc             = "modem ap drv reset system",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_DRV_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp drv exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_PAM_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp osa exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_PAM_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp oam exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUAS_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp gul2 exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_CTTF_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp cttf exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUAS_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp guwas exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_CAS_CPROC_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp cas exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_CAS_CPROC_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp cproc exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUAS_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp guas exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUCNAS_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp gucnas exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUDSP_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp gudsp exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_GUDSP_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp easyRF exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_TLPS_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp tlps exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_LMSP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_LMSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_DRV_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp lmsp exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_TLDSP_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp tldsp exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_CPHY_EXC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp cphy exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem cp ims exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem normal reboot",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_RESETFAIL,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem self-reset fail",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_RESETFAIL,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset frequently",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem self-reset wdt",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_RILD_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset by rild",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_3RD_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset by 3rd modem",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_NOC_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_LPM3 ,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem noc reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset stub",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_NOC_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP| RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMNOC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem noc error",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_AP_NOC_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_AP_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP| RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMNOC,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem noc reset system",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_NORMALRESET,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem user reset without log",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_DMSS_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_DMSS_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMDMSS,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem dmss error",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP |RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset by bus error",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CODE_PATCH_REVERT_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CODE_PATCH_REVERT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMAP",
        .e_desc             = "modem cold patch revert",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_NR_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_NR_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "NR",
        .e_desc             = "modem nr exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_NR_L2HAC_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_NR_L2HAC_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "NR",
        .e_desc             = "modem l2hac exc",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_NR_CCPU_WDT,
        .e_modid_end        = (unsigned int)RDR_MODEM_NR_CCPU_WDT,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_CP| RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask  = RDR_CP,
        .e_from_core        = RDR_CP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "NR",
        .e_desc             = "modem nr wdt exc",
    }, 
#else
    {
        .e_modid            = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LR | RDR_SYS_LPM3 ,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_AP,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type        = CP_S_MODEMAP,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMAP",
        .e_desc             = "LRSS MDMAP reset system",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP DRV reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID_END,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP OSA reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP OAM reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP GUL2 reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP CTTF reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP GUWAS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP CAS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP CPROC reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP GUGAS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP GUCNAS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP GUDSP reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP easyRF reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP LPS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP TLDSP reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP CPHY reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID_START,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID_END,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "LRSS MDMCP IMS reset",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_LR | RDR_LPM3,
        .e_reset_core_mask  = RDR_SYS_LR,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem self-reset wdt",
    },
    {
        .e_modid            = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_modid_end        = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_SYS_AP |RDR_SYS_LR | RDR_SYS_LPM3,
        .e_reset_core_mask  = RDR_SYS_AP,
        .e_from_core        = RDR_SYS_LR,
        .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type        = CP_S_EXCEPTION,
        .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module      = "MDMCP",
        .e_desc             = "modem reset by bus error",
    },
/*以下为NR新增的异常类型定义*/
    {
    .e_modid            = (unsigned int)NRRDR_MODEM_NR_CCPU_START,
    .e_modid_end        = (unsigned int)NRRDR_MODEM_NR_CCPU_END,
    .e_process_priority = RDR_WARN,
    .e_reboot_priority  = RDR_REBOOT_WAIT,
    .e_notify_core_mask = RDR_SYS_AP |RDR_SYS_LR | RDR_SYS_LPM3,
    .e_reset_core_mask  = RDR_SYS_AP,
    .e_from_core        = RDR_SYS_NR,
    .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
    .e_exce_type        = CP_S_EXCEPTION,
    .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
    .excinfo_callbak    = (rdr_exc_info_callback)dump_get_nr_excinfo,
    .e_from_module      = "NRCCPU",
    .e_desc             = "nrccpu exception",
    },
    {
    .e_modid            = (unsigned int)NRRDR_MODEM_NR_L2HAC_START,
    .e_modid_end        = (unsigned int)NRRDR_MODEM_NR_L2HAC_END,
    .e_process_priority = RDR_WARN,
    .e_reboot_priority  = RDR_REBOOT_WAIT,
    .e_notify_core_mask = RDR_SYS_AP |RDR_SYS_LR | RDR_SYS_LPM3,
    .e_reset_core_mask  = RDR_SYS_AP,
    .e_from_core        = RDR_SYS_NR,
    .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
    .e_exce_type        = CP_S_EXCEPTION,
    .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
    .excinfo_callbak    = (rdr_exc_info_callback)dump_get_nr_excinfo,
    .e_from_module      = "NRL2HAC",
    .e_desc             = "l2hac exception",
    },
    {
    .e_modid            = (unsigned int)NRRDR_MODEM_NR_CCPU_WDT,
    .e_modid_end        = (unsigned int)NRRDR_MODEM_NR_CCPU_WDT,
    .e_process_priority = RDR_WARN,
    .e_reboot_priority  = RDR_REBOOT_WAIT,
    .e_notify_core_mask = RDR_SYS_AP |RDR_SYS_LR | RDR_SYS_LPM3,
    .e_reset_core_mask  = RDR_SYS_AP,
    .e_from_core        = RDR_SYS_NR,
    .e_reentrant        = (unsigned int)RDR_REENTRANT_ALLOW,
    .e_exce_type        = CP_S_EXCEPTION,
    .e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
    .excinfo_callbak    = (rdr_exc_info_callback)dump_get_nr_excinfo,
    .e_from_module      = "NRCCPU",
    .e_desc             = "nrccpu wdt",
    },
#endif

};

/*****************************************************************************
* 函 数 名  : dump_get_rdr_exc_info_by_modid
* 功能描述  : 根据modid获取rdr的异常变量地址
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33   lixiaofan  creat
*
*****************************************************************************/

rdr_exc_info_s* dump_get_rdr_exc_info_by_modid(u32 modid)
{
    rdr_exc_info_s* rdr_info = NULL;

    if(modid == RDR_MODEM_NOC_MOD_ID )
    {
        rdr_info = &g_rdr_exc_info[EXC_INFO_NOC];
    }
    else if(modid == RDR_MODEM_DMSS_MOD_ID)
    {
        rdr_info = &g_rdr_exc_info[EXC_INFO_DMSS];
    }
    else
    {
        rdr_info = &g_rdr_exc_info[EXC_INFO_NORMAL];
    }

    return rdr_info;
}
/*****************************************************************************
* 函 数 名  : dump_get_rdr_exc_info_by_index
* 功能描述  : 根据index获取rdr的异常变量地址
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33   lixiaofan  creat
*
*****************************************************************************/

rdr_exc_info_s* dump_get_rdr_exc_info_by_index(u32 index)
{
    rdr_exc_info_s* rdr_info = NULL;
    if(unlikely(index >= EXC_INFO_BUTT))
    {
        return NULL;
    }

    rdr_info = &g_rdr_exc_info[index];

    return rdr_info;
}
/*****************************************************************************
* 函 数 名  : dump_get_exc_index
* 功能描述  : 根据modid获取rdr的索引
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33   lixiaofan  creat
*
*****************************************************************************/

u32 dump_get_exc_index(u32 modid)
{
    u32 index = EXC_INFO_NORMAL;
    if(modid == RDR_MODEM_NOC_MOD_ID)
    {
        index = EXC_INFO_NOC;
    }
    else if(modid == RDR_MODEM_DMSS_MOD_ID)
    {
        index = EXC_INFO_DMSS;
    }
    else
    {
        index = EXC_INFO_NORMAL;
    }
    return index;
}


/*****************************************************************************
* 函 数 名  : dump_save_rdr_callback_info
* 功能描述  : 保存rdr传递的参数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_rdr_callback_info(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone)
{
    rdr_exc_info_s* rdr_info = NULL;

    if(unlikely(logpath == NULL))
    {
        dump_error("logpath is null\n");
        return;
    }

    rdr_info =dump_get_rdr_exc_info_by_modid(modid);
    if(unlikely(rdr_info == NULL))
    {
        dump_error("rdr_info is null\n");
        return;
    }

    rdr_info->modid  = modid;
    rdr_info->coreid = coreid;
    rdr_info->dump_done = fndone;

    if(unlikely((strlen(logpath) + strlen(RDR_DUMP_FILE_CP_PATH)) >= RDR_DUMP_FILE_PATH_LEN - 1))
    {
        dump_error("log path is too long %s\n", logpath);
        return ;
    }
    if(memset_s(rdr_info->log_path,sizeof(rdr_info->log_path),'\0',sizeof(rdr_info->log_path)))
    {
        bsp_debug("err\n");
    }
    if(memcpy_s(rdr_info->log_path,sizeof(rdr_info->log_path) ,logpath, strlen(logpath)))
    {
        bsp_debug("err\n");
    }
    if(memcpy_s(rdr_info->log_path + strlen(logpath) ,(sizeof(rdr_info->log_path)-strlen(logpath)), RDR_DUMP_FILE_CP_PATH, strlen(RDR_DUMP_FILE_CP_PATH)))
    {
        bsp_debug("err\n");
    }

    dump_ok("this exception logpath is %s\n", rdr_info->log_path);
    dump_debug_record(DUMP_DEBUG_ENTER_SAVE_CALLBACK_INFO,modid);

}

char* dump_get_nr_exc_desc(u32 nrrdr_modid)
{
    char* desc = NULL;
    switch(nrrdr_modid)
    {
    case  NRRDR_EXC_CCPU_DRV_MOD_ID:
        desc= "NRCCPU_DRV_EXC";
        break;
    case  NRRDR_EXC_CCPU_OSA_MOD_ID:
    case  NRRDR_EXC_CCPU_OAM_MOD_ID:
        desc= "NRCCPU_PAM_EXC";
        break;
    case  NRRDR_EXC_CCPU_NRNAS_MOD_ID:
        desc= "NRCCPU_NAS_EXC";
        break;
    case  NRRDR_EXC_CCPU_NRDSP_MOD_ID:
    case  NRRDR_EXC_CCPU_NRPHY_MOD_ID:
        desc= "NRCCPU_PHY_EXC";
        break;
    case  NRRDR_EXC_CCPU_IMS_MOD_ID:
        desc= "NRCCPU_IMS_EXC";
        break;
    case  NRRDR_EXC_CCPU_NRPS_MOD_ID:
        desc= "NRCCPU_L2&L3_EXC";
        break;
    default:
        desc = "NRCCPU_EXC";
        break;
    }
    return desc;
}
/*****************************************************************************
* 函 数 名  : dump_get_nr_excinfo
* 功能描述  :
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_get_nr_excinfo(u32 modid,void* exc_sub_type, void* desc)
{
    char* exc_desc = NULL;
    if(exc_sub_type == NULL || desc == NULL)
    {
        dump_error("error param");
        return;
    }
    if(modid >= RDR_MODEM_NR_MOD_ID_START && modid <= RDR_MODEM_NR_MOD_ID_END)
    {
        if(modid >= NRRDR_MODEM_NR_CCPU_START && modid <= NRRDR_MODEM_NR_CCPU_END)
        {
            (*(u32*)exc_sub_type) = DUMP_CPU_NRCCPU;
            exc_desc = dump_get_nr_exc_desc(modid);
            if(exc_desc != NULL)
            {
                if(memcpy_s(desc,DUMP_NR_EXCINFO_SIZE_48,exc_desc,strlen(exc_desc)))
                {
                    dump_debug("copy error\n");
                }
            }
        }
        if(modid >= NRRDR_MODEM_NR_L2HAC_START && modid <= NRRDR_MODEM_NR_L2HAC_END)
        {
            (*(u32*)exc_sub_type) = DUMP_CPU_NRL2HAC;
            if(memcpy_s(desc,DUMP_NR_EXCINFO_SIZE_48,NRL2HAC_EXCEPTION,strlen(NRL2HAC_EXCEPTION)))
            {
                dump_debug("copy error\n");
            }
        }
    }

}

s32 dump_check_reset_fail(u32 rdr_id)
{
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_reset_ctrl_int
* 功能描述  : 初始化g_dump_cp_reset_timestamp
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_reset_ctrl_int(void)
{
    if(memset_s(&g_dump_mdm_reset_record,sizeof(g_dump_mdm_reset_record),0,sizeof(g_dump_mdm_reset_record)))
    {
        bsp_debug("err\n");
    }
}


/*****************************************************************************
* 函 数 名  : dump_check_reset_freq
* 功能描述  : modem 频繁单独复位的特殊处理
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_check_reset_freq(u32 rdr_id)
{
    u32 diff = 0;
    NV_DUMP_STRU* cfg = NULL;
    cfg = dump_get_feature_cfg();

    if(DUMP_MBB == dump_get_product_type())
    {
        return BSP_OK;
    }
    if(unlikely( cfg!= NULL && cfg->dump_cfg.Bits.fetal_err == 0))
    {
        dump_error("no need check mdm reset times\n");
        return BSP_OK;
    }
    if(BSP_OK == dump_check_single_reset_by_modid(rdr_id))
    {
        if(g_dump_mdm_reset_record.count % DUMP_CP_REST_TIME_COUNT == 0
            && g_dump_mdm_reset_record.count !=0)
        {
            diff = (g_dump_mdm_reset_record.reset_time[DUMP_CP_REST_TIME_COUNT -1] - g_dump_mdm_reset_record.reset_time[0]);
            if( diff < DUMP_CP_REST_TIME_COUNT*DUMP_CP_REST_TIME_SLICE)
            {
               dump_error("stop modem single reset\n ");
               return BSP_ERROR;
            }

            if(memset_s(&g_dump_mdm_reset_record,sizeof(g_dump_mdm_reset_record),0,sizeof(g_dump_mdm_reset_record)))
            {
                bsp_debug("err\n");
            }
        }
        if(rdr_id != RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID
           && rdr_id != RDR_MODEM_CP_RESET_USER_RESET_MOD_ID)
        {
            g_dump_mdm_reset_record.reset_time[g_dump_mdm_reset_record.count % DUMP_CP_REST_TIME_COUNT] = bsp_get_slice_value();
            g_dump_mdm_reset_record.count++;
        }
        return BSP_OK;
    }
    else
    {
        dump_ok("no need check this modid\n");
    }
    return BSP_OK;

}


/*****************************************************************************
* 函 数 名  : dump_match_lrccpu_rdr_id
* 功能描述  : 转换mdmcp与rdr之间的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

u32 dump_match_ccpu_rdr_id(u32 mdmcp_mod_id)
{
    u32 i = 0;
#ifdef  BSP_CONFIG_PHONE_TYPE
    u32 rdr_id = RDR_MODEM_CP_DRV_MOD_ID_START;
#else
    u32 rdr_id = RDR_MODEM_CP_DRV_MOD_ID;
#endif
    for(i = 0; i < sizeof(g_dump_cp_mod_id)/sizeof(g_dump_cp_mod_id[0]);i++)
    {
        if(mdmcp_mod_id >= g_dump_cp_mod_id[i].mdm_id_start
            && mdmcp_mod_id <= g_dump_cp_mod_id[i].mdm_id_end)
        {
            rdr_id =  g_dump_cp_mod_id[i].rdr_id;
        }
    }
    return rdr_id;
}

/*****************************************************************************
* 函 数 名  : RDR_MODEM_DRV_BUTT_MOD_ID
* 功能描述  : 匹配noc的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_match_noc_rdr_id(u32 modid, u32 arg)
{
    u32 rdr_id = RDR_MODEM_DRV_BUTT_MOD_ID;

    if((modid == DRV_ERRNO_MODEM_NOC)
     ||(modid == NOC_RESET_GUC_MODID)
     ||(modid == NOC_RESET_NXP_MODID)
     ||(modid == NOC_RESET_BBP_DMA0_MODID)
     ||(modid == NOC_RESET_BBP_DMA1_MODID)
     ||(modid == NOC_RESET_HARQ_MODID)
     ||(modid == NOC_RESET_CPHY_MODID)
     ||(modid == NOC_RESET_GUL2_MODID))
    {
        if(arg == NOC_AP_RESET)
        {
            rdr_id =  RDR_MODEM_AP_NOC_MOD_ID;
        }
        else if(arg == NOC_CP_RESET)
        {
            rdr_id = RDR_MODEM_CP_NOC_MOD_ID;
        }
    }
    return rdr_id;
}

/*****************************************************************************
* 函 数 名  : dump_print_mdm_error
* 功能描述  : 打印CP的异常类型
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_print_mdm_error(u32 rdr_id)
{
    u32 i = 0;
    for(i = 0; i < sizeof(g_dump_cp_mod_id)/sizeof(g_dump_cp_mod_id[0]);i++)
    {
        if(rdr_id == g_dump_cp_mod_id[i].rdr_id)
        {
            dump_ok("exc_id:%s\n",g_dump_cp_mod_id[i].exc_desc);
            return;
        }
    }
    dump_ok("exc_id: RDR_MODEM_DRV_MOD_ID\n");
}

/*****************************************************************************
* 函 数 名  : dump_match_mdmcp_rdr_id
* 功能描述  : 匹配mdmcp的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_match_mdmcp_rdr_id(dump_exception_info_s* dump_exception_info)
{
    u32 rdr_id = RDR_MODEM_DRV_BUTT_MOD_ID;
    /*wdt和dlock错误*/

    if((DRV_ERRNO_DUMP_CP_WDT == dump_exception_info->mod_id))
    {

        rdr_id = RDR_MODEM_CP_WDT_MOD_ID;
        return rdr_id;
    }
    else if((DRV_ERRNO_DLOCK == dump_exception_info->mod_id))
    {
        rdr_id = RDR_MODEM_CP_RESET_DLOCK_MOD_ID;
        return rdr_id;
    }

#ifdef  BSP_CONFIG_PHONE_TYPE
    /*手机版本noc错误*/
    rdr_id = dump_match_noc_rdr_id(dump_exception_info->mod_id,dump_exception_info->arg1);
    if(rdr_id== RDR_MODEM_DRV_BUTT_MOD_ID)
    {
        /*手机版本cp传过来的错误码*/
        rdr_id = dump_match_ccpu_rdr_id(dump_exception_info->mod_id);
        dump_print_mdm_error(rdr_id);
    }
#else
    /*mbb产品cp的错误码匹配*/
    rdr_id = dump_exception_info->mod_id;
#endif

    return rdr_id;


}


/*****************************************************************************
* 函 数 名  : dump_match_special_rdr_id
* 功能描述  : 匹配ap侧触发的和cp强相关的错误
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
u32 dump_match_special_rdr_id(u32 modid)
{
    u32 rdr_mod_id = RDR_MODEM_DRV_BUTT_MOD_ID;
    if(DUMP_PHONE == dump_get_product_type())
    {
        switch(modid)
        {
        case DRV_ERRNO_RESET_SIM_SWITCH:
            rdr_mod_id = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID;
            break;
        case NAS_REBOOT_MOD_ID_RILD:
            rdr_mod_id = RDR_MODEM_CP_RESET_RILD_MOD_ID;
            break;
        case DRV_ERRNO_RESET_3RD_MODEM:
            rdr_mod_id = RDR_MODEM_CP_RESET_3RD_MOD_ID;
            break;
        case DRV_ERRNO_RESET_REBOOT_REQ:
            rdr_mod_id = RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID;
            break;
        case DRV_ERROR_USER_RESET:
            rdr_mod_id = RDR_MODEM_CP_RESET_USER_RESET_MOD_ID;
            break;
        case DRV_ERRNO_RST_FAIL:
            rdr_mod_id = RDR_MODEM_CP_RESET_FAIL_MOD_ID;
            break;
        case DRV_ERRNO_NOC_PHONE:
            rdr_mod_id = RDR_MODEM_NOC_MOD_ID;
            break;
        case DRV_ERRNO_DMSS_PHONE:
            rdr_mod_id = RDR_MODEM_DMSS_MOD_ID;
            break;
        default:
            break;
        }
    }
    return rdr_mod_id;
}

/*****************************************************************************
* 函 数 名  : dump_match_mdmap_rdr_id
* 功能描述  : 匹配mdmap侧的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_match_mdmap_rdr_id(dump_exception_info_s* dump_exception_info)
{
    u32 rdr_mod_id = RDR_MODEM_DRV_BUTT_MOD_ID;
    rdr_mod_id = dump_match_special_rdr_id(dump_exception_info->mod_id);
    if( rdr_mod_id == RDR_MODEM_DRV_BUTT_MOD_ID)
    {
        /*底软的错误，并且在商用版本上都走单独复位，不允许执行整机复位*/
        if((dump_exception_info->mod_id <= (u32)RDR_MODEM_CP_DRV_MOD_ID_END)
            && EDITION_INTERNAL_BETA !=dump_get_edition_type())
        {
            rdr_mod_id = RDR_MODEM_AP_DRV_MOD_ID;
        }
        else
        {
            rdr_mod_id = RDR_MODEM_AP_MOD_ID;
        }

    }
    return rdr_mod_id;
}
/*****************************************************************************
* 函 数 名  : dump_match_mdmnr_rdr_id
* 功能描述  : 匹配nr的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
s32 dump_match_mdmnr_rdr_id(dump_exception_info_s* dump_exception_info)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    return NRRDR_MODEM_NR_CCPU_START;
#else
    return RDR_MODEM_NR_MOD_ID;
#endif
}

/*****************************************************************************
* 函 数 名  : dump_match_rdr_mod_id
* 功能描述  : 将drv的错误码转换为rdr的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
u32 dump_match_rdr_mod_id(dump_exception_info_s* dump_exception_info)
{
    u32 rdr_id = RDR_MODEM_AP_MOD_ID;
    if(unlikely(dump_exception_info == NULL))
    {
        return rdr_id;
    }
    if(dump_exception_info->core ==DUMP_CPU_NRCCPU || dump_exception_info->core == DUMP_CPU_NRL2HAC)
    {
        rdr_id =  dump_match_mdmnr_rdr_id(dump_exception_info);
    }
    else if(dump_exception_info->core == DUMP_CPU_LRCCPU)
    {
        rdr_id =  dump_match_mdmcp_rdr_id(dump_exception_info);
    }
    else if(dump_exception_info->core == DUMP_CPU_APP)
    {
        rdr_id =  dump_match_mdmap_rdr_id(dump_exception_info);
    }
    return rdr_id;
}

/*****************************************************************************
* 函 数 名  : dump_show_excption_info
* 功能描述  : 显示当前的异常信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_show_excption_info(dump_exception_info_s* exception_info_s)
{
}


/*****************************************************************************
* 函 数 名  : dump_report_exc_to_rdr
* 功能描述  : 进入到modem的异常处理流程
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
void dump_fill_excption_info(dump_exception_info_s* exception_info_s,
                                 u32 mod_id,u32 arg1,u32 arg2,char* data,
                                 u32 length,u32 core,u32 reason,const char* desc,
                                 dump_reboot_ctx_t contex,
                                 u32 task_id,u32 int_no,u8* task_name)
{
    if(unlikely(exception_info_s == NULL))
    {
        dump_error("exception_info_s is null\n");
        return;
    }
    exception_info_s->core = core;
    exception_info_s->mod_id = mod_id;
    exception_info_s->rdr_mod_id =  dump_match_rdr_mod_id(exception_info_s);
    exception_info_s->arg1 = arg1;
    exception_info_s->arg2 = arg2;
    exception_info_s->data= data;
    exception_info_s->length= length;
    exception_info_s->voice = dump_get_mdm_voice_status();
    exception_info_s->reboot_contex = contex;
    exception_info_s->reason = reason;
    if( exception_info_s->reboot_contex == DUMP_CTX_INT)
    {
        exception_info_s->int_no = int_no;
    }
    else
    {
        exception_info_s->task_id =task_id;
        if(task_name != NULL)
        {
            if(memcpy_s(exception_info_s->task_name,sizeof(exception_info_s->task_name),task_name,strlen(task_name)))
            {
                bsp_debug("err\n");
            }
        }
    }

    if(NULL != desc)
    {
        if(memcpy_s(exception_info_s->exc_desc,sizeof(exception_info_s->exc_desc),desc,strlen(desc)))
        {
            bsp_debug("err\n");
        }
    }
    dump_ok("fill excption info done\n");
    dump_debug_record(DUMP_DEBUG_FILL_EXCINFO,exception_info_s->mod_id);
    dump_debug_record(DUMP_DEBUG_FILL_EXCINFO,exception_info_s->rdr_mod_id);
    dump_debug_record(DUMP_DEBUG_FILL_EXCINFO,bsp_get_slice_value());

}

/*****************************************************************************
* 函 数 名  : dump_get_exception_info_node
* 功能描述  : 根据错误码查找对应的异常节点
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

struct rdr_exception_info_s* dump_get_exception_info_node(u32 mod_id)
{
    u32 i = 0;
    struct rdr_exception_info_s* rdr_exc_info = NULL;

    for(i = 0; i < (sizeof(g_modem_exc_info)/sizeof(g_modem_exc_info[0]));i++)
    {
#ifndef BSP_CONFIG_PHONE_TYPE
#endif
        {
            if(g_modem_exc_info[i].e_modid == mod_id)
            {
               rdr_exc_info = &g_modem_exc_info[i];
            }
        }


    }
    return rdr_exc_info;
}
/*****************************************************************************
* 函 数 名  : dump_set_reboot_contex
* 功能描述  : 设定异常原因和异常核
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_check_need_report_excption(u32 rdr_id)
{
     /*noc和dmss的错误，已经调用过了rdr_system_error，不需要再进行处理*/
     if(rdr_id == RDR_MODEM_NOC_MOD_ID || rdr_id == RDR_MODEM_DMSS_MOD_ID)
     {
         dump_ok("rdr_id = 0x%x\n",rdr_id);
         return BSP_ERROR;
     }
     /*其他所有异常都需要经过rdr*/
     return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_get_current_excpiton_info
* 功能描述  : 查找当前正在处理的异常节点信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

dump_exception_info_s* dump_get_current_excpiton_info(u32 modid)
{
    u32 index = dump_get_exc_index(modid);
    if(unlikely(index >= EXC_INFO_BUTT))
    {
        return NULL;
    }
    return &(g_curr_excption[index]);
}
/*****************************************************************************
* 函 数 名  : dump_wait_excption_handle_done
* 功能描述  : 等待此次异常处理完成
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
void dump_wait_excption_handle_done(dump_exception_info_s* dest_exc_info)
{
    s32 ret;
    if(dest_exc_info == NULL)
    {
        return;
    }
    dump_ok("start to wait exception handler done\n");
    ret = down_timeout(&dest_exc_info->sem_wait,msecs_to_jiffies(WAIT_EXCEPTION_HANDLE_TIME));
    if(ret != 0)
    {
        dump_ok("wait exception handler timeout\n");
    }
    else
    {
        dump_debug_record(DUMP_DEBUG_EXCWPTION_HANDLE_DONE,bsp_get_slice_value());
        dump_ok("exception handler done\n");
    }

}
/*****************************************************************************
* 函 数 名  : dump_mdm_wdt_disable
* 功能描述  : 异常处理前关闭看门狗
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_mdm_wdt_disable(void)
{
    bsp_wdt_irq_disable(WDT_CCORE_ID);
    dump_debug_record(DUMP_DEBUG_STOP_LR_WDT,bsp_get_slice_value());
    dump_ok("stop cp wdt\n");

    bsp_wdt_irq_disable(WDT_NRCCPU_ID);
    dump_debug_record(DUMP_DEBUG_STOP_NR_WDT,bsp_get_slice_value());
    dump_ok("stop nrccpu wdt finish\n");

}

/*****************************************************************************
* 函 数 名  : dump_mdm_wdt_enable
* 功能描述  : 使能看门狗中断
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_mdm_wdt_enable(void)
{
    bsp_wdt_irq_enable(WDT_CCORE_ID);
    dump_ok("enbale lr wdt\n");
    dump_debug_record(DUMP_DEBUG_ENABLE_LR_WDT,bsp_get_slice_value());

    bsp_wdt_irq_enable(WDT_NRCCPU_ID);
    dump_ok("enbale nr wdt\n");
    dump_debug_record(DUMP_DEBUG_ENABLE_NR_WDT,bsp_get_slice_value());
}

/*****************************************************************************
* 函 数 名  : dump_excption_handle_done
* 功能描述  : 异常处理完成
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
void dump_excption_handle_done(u32 modid)
{
    u32 index = dump_get_exc_index(modid);
    dump_exception_info_s curr_info_s ={0,};
    unsigned long flags;
    if(index >= EXC_INFO_BUTT)
    {
        return;
    }

    if(index == EXC_INFO_NORMAL)
    {
        dump_ok("clear last excinfo");
        if(memcpy_s(&curr_info_s,sizeof(curr_info_s),&g_curr_excption[EXC_INFO_NORMAL],sizeof(g_curr_excption[EXC_INFO_NORMAL])))
        {
            dump_debug("error\n");
        }
        spin_lock_irqsave(&g_exception_ctrl.lock, flags);
        /*清除上次的异常信息*/
        if(memset_s(&g_curr_excption[index],sizeof(g_curr_excption[index]),sizeof(g_curr_excption[index]),0))
        {
            bsp_debug("err\n");
        }
        if(curr_info_s.status == DUMP_STATUS_REGISTER)
        {
            g_curr_excption[index].status = DUMP_STATUS_NONE;
            dump_ok("alloc next excinfo enter\n");
        }
        spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

        dump_debug_record(DUMP_DEBUG_CLEAR_LAST_EXCINFO,bsp_get_slice_value());

    }

    dump_ok("dump_excption_handle_done\n");
}

/*****************************************************************************
* 函 数 名  : dump_get_exception_info_node
* 功能描述  : 根据错误码查找对应的异常节点
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

int dump_handle_excption_task(void *data)
{
    unsigned long flags;
    dump_exception_info_s* curr_info_s = NULL;
    s32 ret = BSP_ERROR;

    for(;;)
    {
        osl_sem_down(&g_exception_ctrl.sem_exception_task);

        dump_ok("enter excption handler task \n");
        curr_info_s = &g_curr_excption[EXC_INFO_NORMAL];
        if(curr_info_s == NULL)
        {
            continue;
        }

        dump_mdm_wdt_disable();

        spin_lock_irqsave(&g_exception_ctrl.lock, flags);

        ret = dump_check_reset_freq(curr_info_s->rdr_mod_id);

        if(ret == BSP_OK && BSP_OK == dump_check_need_report_excption(curr_info_s->rdr_mod_id))
        {

            dump_show_excption_info(curr_info_s);
            /*每个节点都是添加到队尾，实际上每次取到的就是时间戳最早的*/
            rdr_system_error(curr_info_s->rdr_mod_id,curr_info_s->arg1,curr_info_s->arg2);
            dump_debug_record(DUMP_DEBUG_CALL_RDR_SYSTEM_ERROR,curr_info_s->rdr_mod_id);

        }
        else
        {
        /*当前这个rdrid是要触发modem单独复位的，但是此时modem单独复位过多，直接关闭modem*/
            if(BSP_OK == dump_check_single_reset_by_modid(curr_info_s->rdr_mod_id))
            {
#ifdef  BSP_CONFIG_PHONE_TYPE
                if(g_exception_ctrl.modem_off != DUMP_MODEM_OFF)
                {
                    bsp_modem_power_off();
                    bsp_wdt_irq_enable(WDT_CCORE_ID);
                    bsp_wdt_irq_enable(WDT_NRCCPU_ID);
                    dump_debug_record(DUMP_DEBUG_POWER_MODEM_OFF,curr_info_s->rdr_mod_id);
                    g_exception_ctrl.modem_off = DUMP_MODEM_OFF;
                    dump_ok("modem reset too many times,shut down\n");
                }
#endif
            }
        }
        spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

        dump_ok("exit excption handler task \n");

    }
}

/*****************************************************************************
* 函 数 名  : dump_exception_handler_init
* 功能描述  : 异常流程链表初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

__init s32 dump_exception_handler_init(void)
{
    struct task_struct * pid = NULL;
    struct sched_param   param = {0,};

    spin_lock_init(&g_exception_ctrl.lock);

    sema_init(&g_exception_ctrl.sem_exception_task, 0);

    INIT_LIST_HEAD(&g_exception_ctrl.exception_list);

    g_curr_excption[EXC_INFO_NORMAL].status = DUMP_STATUS_NONE;

    dump_reset_ctrl_int();

    pid = (struct task_struct *)kthread_run(dump_handle_excption_task, 0, "Modem_exception");
    if (IS_ERR((void*)pid))
    {
        dump_error("fail to create kthread task failed! \n", __LINE__);
        return BSP_ERROR;
    }
    g_exception_ctrl.exception_task_id = (uintptr_t)pid;

    param.sched_priority = 98;
    if (BSP_OK != sched_setscheduler(pid, SCHED_FIFO, &param))
    {
        dump_error("fail to set scheduler failed!\n", __LINE__);
        return BSP_ERROR;
    }
    g_exception_ctrl.init_flag = true;

    dump_ok("exception handler init ok\n");

    return BSP_OK;

}
bool dump_check_need_reboot_sys(u32 rdr_id)
{
    if(rdr_id == RDR_MODEM_AP_MOD_ID
        || rdr_id== RDR_MODEM_AP_MOD_ID
        || rdr_id==RDR_MODEM_NOC_MOD_ID
        || rdr_id==RDR_MODEM_DMSS_MOD_ID
        || rdr_id==RDR_MODEM_CP_RESET_FAIL_MOD_ID
        || rdr_id== RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID)
    {
        return true;
    }
    return false;
}
void dump_update_exception_info(rdr_exc_info_s *rdr_exc_info)
{
    dump_exception_info_s *exception = NULL;
    dump_base_info_t *modem_cp_base_info = NULL;
    dump_cp_reboot_contex *reboot_contex = NULL;
    void* addr1 = NULL;
    void* addr = NULL;
    if (unlikely(rdr_exc_info == NULL)) {
        return;
    }

    exception = (dump_exception_info_s *)dump_get_current_excpiton_info(rdr_exc_info->modid);
    if (unlikely(exception == NULL )) {
        return;
    }
    if(exception->reason == DUMP_REASON_WDT) {
        if(exception->core == DUMP_CPU_NRCCPU) {
            addr = bsp_dump_get_field_addr(DUMP_NRCCPU_BASE_INFO_SMP);
            addr1 = (u8 *)bsp_dump_get_field_addr(DUMP_NRCCPU_REBOOTCONTEX);
        } else if(exception->core == DUMP_CPU_LRCCPU) {
            addr = bsp_dump_get_field_addr(DUMP_LRCCPU_BASE_INFO_SMP);
            addr1 = (u8 *)bsp_dump_get_field_addr(DUMP_LRCCPU_REBOOTCONTEX);
        }
        if (unlikely(addr == NULL || addr1 == NULL)) {
            return;
        }

        modem_cp_base_info = (dump_base_info_t *)(uintptr_t)addr;
        if(modem_cp_base_info->reboot_cpu == BSP_MODU_OTHER_CORE && modem_cp_base_info->cpu_max_num > 1) {
            modem_cp_base_info->reboot_cpu = 0; /*c 核未响应默认是核0异常*/
            reboot_contex = (dump_cp_reboot_contex *)((uintptr_t)(addr1));
            if(memcpy_s(modem_cp_base_info->taskName,sizeof(modem_cp_base_info->taskName),reboot_contex->taskName,sizeof(reboot_contex->taskName))) {
                dump_debug("err\n");
            }
            modem_cp_base_info->reboot_context = reboot_contex->reboot_context;
            modem_cp_base_info->reboot_int = reboot_contex->reboot_int;
            modem_cp_base_info->reboot_task = reboot_contex->reboot_task;
        }
        if(memcpy_s(exception->task_name,sizeof(exception->task_name),modem_cp_base_info->taskName,sizeof(modem_cp_base_info->taskName))) {
            dump_debug("err\n");
        }
        exception->task_id = modem_cp_base_info->reboot_task;
        exception->int_no = modem_cp_base_info->reboot_int;
        exception->reboot_contex = modem_cp_base_info->reboot_context;


    }
}
/*****************************************************************************
* 函 数 名  : dump_register_exception
* 功能描述  : 将一个异常类型添加到队列里
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

s32 dump_register_exception(dump_exception_info_s* current_exception)
{
    dump_exception_info_s* exception_info_s = NULL;
    unsigned long flags;
    dump_exception_info_s* curr_info_s = &g_curr_excption[EXC_INFO_NORMAL];
    u32 status = 0;
    if(unlikely(current_exception == NULL))
    {
        dump_error("param exception_info is null\n");
        return BSP_ERROR;
    }
    /*中断上下文也可能出现异常*/
    exception_info_s = kmalloc(sizeof(dump_exception_info_s),GFP_ATOMIC);
    if(unlikely(exception_info_s == NULL))
    {
        dump_error("fail to malloc space\n");
        return BSP_ERROR;
    }
    if(memset_s(exception_info_s,sizeof(*exception_info_s),0,sizeof(*exception_info_s)))
    {
        bsp_debug("err\n");
    }
    if(memcpy_s(exception_info_s,sizeof(*exception_info_s),current_exception,sizeof(*current_exception)))
    {
        bsp_debug("err\n");
    }

    spin_lock_irqsave(&g_exception_ctrl.lock, flags);

    if(curr_info_s->status == DUMP_STATUS_NONE)
    {
        curr_info_s->status = DUMP_STATUS_REGISTER;
    }
    else
    {
        spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);
        dump_ok("now is handle exception exit\n");
        return BSP_ERROR;
    }
    status = curr_info_s->status;
    if(memcpy_s(curr_info_s,sizeof(dump_exception_info_s),current_exception,sizeof(dump_exception_info_s)))
    {
        dump_debug("error");
    }
    curr_info_s->status = status;
    spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

    dump_ok("register exception ok \n");
    dump_debug_record(DUMP_DEBUG_REGISTER_EXCINFO,exception_info_s->mod_id);

    up(&g_exception_ctrl.sem_exception_task);
    /*lint -save -e429*/
    return BSP_OK;
    /*lint +restore +e429*/

}

/*****************************************************************************
* 函 数 名  : dump_mdm_callback
* 功能描述  : modem异常的回调处理函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
void dump_mdm_callback(unsigned int modid, unsigned int etype, unsigned long long coreid, char* logpath, pfn_cb_dump_done fndone)
{
    u32 ret = BSP_OK;

#ifdef CONFIG_COLD_PATCH
    bsp_modem_cold_patch_update_modem_fail_count();
    dump_debug_record(DUMP_DEBUG_UPDATE_FAIL_COUNT,bsp_get_slice_value());
#endif
    dump_save_mdmboot_info();

    dump_save_last_kmsg();

    if(NULL != fndone)
    {
        fndone(modid, coreid);
        dump_debug_record(DUMP_DEBUG_NOTIFY_RDR,bsp_get_slice_value());
        dump_debug_record(DUMP_DEBUG_NOTIFY_RDR,modid);
    }
    if(ret != BSP_OK)
    {
        dump_error("callback error\n");
    }
}

/*****************************************************************************
* 函 数 名  : dump_mdm_reset
* 功能描述  : modem 复位处理函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
void dump_mdm_reset(unsigned int modid, unsigned int etype, unsigned long long coreid)
{
    s32 ret;
    char* desc = NULL;
    u32 drv_mod_id = DRV_ERRNO_RESET_REBOOT_REQ;

    dump_exception_info_s exception_info_s = {0,};

    ret = dump_mdmcp_reset(modid,etype,coreid);

    if(ret != RESET_SUCCES)
    {
       if(ret == RESET_NOT_SUPPORT)
       {
            dump_fill_excption_info(&exception_info_s,
                                    DRV_ERRNO_RESET_REBOOT_REQ,
                                    0,0,NULL,0,DUMP_CPU_APP,
                                    DUMP_REASON_RST_NOT_SUPPORT,
                                    "reset not support",
                                    DUMP_CTX_TASK,
                                    0,0,"modem_reset");
            drv_mod_id =DRV_ERRNO_RESET_REBOOT_REQ;
            desc = "MDM_RST_FREQ";
       }
       else
       {
            dump_fill_excption_info(&exception_info_s,
                                    DRV_ERRNO_RST_FAIL,0,0,NULL,0,
                                    DUMP_CPU_APP,DUMP_REASON_RST_FAIL,
                                    "reset fail",
                                    DUMP_CTX_TASK,
                                    0,0,"modem_reset");
            drv_mod_id =DRV_ERRNO_RST_FAIL;
            desc = "MDM_RST_FAIL";
       }


    }

    /*单独复位成功打开看门狗中断*/
    if(ret == RESET_SUCCES)
    {
        dump_mdm_wdt_enable();
    }

    /*如果是单独复位结束，此次异常处理也结束了，
    没有单独复位，全系统也就复位了，不需要执行释放动作*/
    dump_excption_handle_done(modid);


    /*上次异常清空后，处理单独复位没有返回OK的异常*/
    if(ret != RESET_SUCCES)
    {
        (void)dump_register_exception(&exception_info_s);
    }


}


/*****************************************************************************
* 函 数 名  : dump_register_modem_exc_info
* 功能描述  : modem dump初始化第一阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/
__init s32 dump_register_modem_exc_info(void)
{
    u32 i = 0;
    struct rdr_module_ops_pub soc_ops = {
    .ops_dump = NULL,
    .ops_reset = NULL
    };
    struct rdr_register_module_result soc_rst = {0,0,0};

    for(i=0; i<sizeof(g_modem_exc_info)/sizeof(struct rdr_exception_info_s); i++)
    {
        if(rdr_register_exception(&g_modem_exc_info[i]) == 0)
        {
            dump_error("modid:0x%x register rdr exception failed\n", g_modem_exc_info[i].e_modid);
            return BSP_ERROR;
        }
    }

    if(memset_s(&g_rdr_exc_info,sizeof(g_rdr_exc_info),0,sizeof(g_rdr_exc_info)))
    {
        bsp_debug("err\n");
    }
    soc_ops.ops_dump  = (pfn_dump)dump_mdm_callback;
    soc_ops.ops_reset = (pfn_reset)dump_mdm_reset;
#ifdef  BSP_CONFIG_PHONE_TYPE
    if(rdr_register_module_ops(RDR_CP, &soc_ops, &(soc_rst)) != BSP_OK)
    {
        dump_error("fail to register  rdr ops \n");
        return BSP_ERROR;
    }
#else
    if(rdr_register_module_ops(RDR_SYS_LR, &soc_ops, &(soc_rst)) != BSP_OK)
    {
        dump_error("fail to register  rdr ops \n");
        return BSP_ERROR;
    }
#endif

    dump_ok("register modem exc info ok");
    return BSP_OK;

}


