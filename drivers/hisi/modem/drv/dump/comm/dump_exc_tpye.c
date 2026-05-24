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
#include <asm/string.h>
#include "product_config.h"
#include "drv_comm.h"
#include "osl_types.h"
#include "bsp_dump.h"
#include "bsp_dump_mem.h"
#include "bsp_slice.h"
#include "bsp_reset.h"
#include "bsp_coresight.h"
#include "bsp_wdt.h"
#include "gunas_errno.h"
#include "dump_config.h"
#include "dump_baseinfo.h"
#include "dump_cp_agent.h"
#include "dump_area.h"
#include "dump_cp_wdt.h"
#include "dump_logs.h"
#include "dump_area.h"
#include "dump_exc_type.h"
#include "dump_cp_core.h"
#undef	THIS_MODU
#define THIS_MODU mod_dump

rdr_exc_info_s     g_rdr_exc_info;
dump_reboot_contex_s            g_dump_reboot_contex = {DUMP_CPU_BUTTON,DUMP_REASON_UNDEF};

struct rdr_exception_info_s g_modem_exc_info[] = {
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
        .e_exce_type        = CP_S_MODEMNOC,
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
};


DUMP_MOD_ID g_dump_cp_mod_id[] ={
    {RDR_MODEM_CP_DRV_MOD_ID_START,RDR_MODEM_CP_DRV_MOD_ID_END,RDR_MODEM_CP_DRV_MOD_ID},
    {RDR_MODEM_CP_OSA_MOD_ID_START,RDR_MODEM_CP_OSA_MOD_ID_END,RDR_MODEM_CP_OSA_MOD_ID},
    {RDR_MODEM_CP_OAM_MOD_ID_START,RDR_MODEM_CP_OAM_MOD_ID_END,RDR_MODEM_CP_OAM_MOD_ID},
    {RDR_MODEM_CP_GUL2_MOD_ID_START,RDR_MODEM_CP_GUL2_MOD_ID_END,RDR_MODEM_CP_GUL2_MOD_ID},
    {RDR_MODEM_CP_CTTF_MOD_ID_START,RDR_MODEM_CP_CTTF_MOD_ID_END,RDR_MODEM_CP_CTTF_MOD_ID},
    {RDR_MODEM_CP_GUWAS_MOD_ID_START,RDR_MODEM_CP_GUWAS_MOD_ID_END,RDR_MODEM_CP_GUWAS_MOD_ID},
    {RDR_MODEM_CP_CAS_MOD_ID_START,RDR_MODEM_CP_CAS_MOD_ID_END,RDR_MODEM_CP_CAS_MOD_ID},
    {RDR_MODEM_CP_CPROC_MOD_ID_START,RDR_MODEM_CP_CPROC_MOD_ID_END,RDR_MODEM_CP_CPROC_MOD_ID},
    {RDR_MODEM_CP_GUGAS_MOD_ID_START,RDR_MODEM_CP_GUGAS_MOD_ID_END,RDR_MODEM_CP_GUGAS_MOD_ID},
    {RDR_MODEM_CP_GUCNAS_MOD_ID_START,RDR_MODEM_CP_GUCNAS_MOD_ID_END,RDR_MODEM_CP_GUCNAS_MOD_ID},
    {RDR_MODEM_CP_GUDSP_MOD_ID_START,RDR_MODEM_CP_GUDSP_MOD_ID_END,RDR_MODEM_CP_GUDSP_MOD_ID},
    {RDR_MODEM_CP_LPS_MOD_ID_START,RDR_MODEM_CP_LPS_MOD_ID_END,RDR_MODEM_CP_LPS_MOD_ID},
    {RDR_MODEM_CP_LMSP_MOD_ID_START,RDR_MODEM_CP_LMSP_MOD_ID_END,RDR_MODEM_CP_LMSP_MOD_ID},
    {RDR_MODEM_CP_TLDSP_MOD_ID_START,RDR_MODEM_CP_TLDSP_MOD_ID_END,RDR_MODEM_CP_TLDSP_MOD_ID},
    {RDR_MODEM_CP_CPHY_MOD_ID_START,RDR_MODEM_CP_CPHY_MOD_ID_END,RDR_MODEM_CP_CPHY_MOD_ID},
    {RDR_MODEM_CP_IMS_MOD_ID_START,RDR_MODEM_CP_IMS_MOD_ID_END,RDR_MODEM_CP_IMS_MOD_ID},
};




/*****************************************************************************
* 函 数 名  : dump_convert_id_mdmcp2rdr
* 功能描述  : 转换mdmcp与rdr之间的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

u32 dump_convert_id_mdmcp2rdr(u32 mdmcp_mod_id)
{
    u32 i = 0;
    u32 rdr_id = (u32)RDR_MODEM_CP_DRV_MOD_ID;
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
* 函 数 名  : dump_match_rdr_mod_id
* 功能描述  : 将drv的错误码转换为rdr的错误码
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u32 dump_match_rdr_mod_id(u32 drv_mod_id)
{
    u32 rdr_mod_id = RDR_MODEM_AP_MOD_ID;

    if(DUMP_PHONE == dump_get_product_type())
    {
        switch(drv_mod_id)
        {
        case DRV_ERRNO_RESET_SIM_SWITCH:
            rdr_mod_id = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID;
            return rdr_mod_id;
        case NAS_REBOOT_MOD_ID_RILD:
            rdr_mod_id = RDR_MODEM_CP_RESET_RILD_MOD_ID;
            return rdr_mod_id;
        case DRV_ERRNO_RESET_3RD_MODEM:
            rdr_mod_id = RDR_MODEM_CP_RESET_3RD_MOD_ID;
            return rdr_mod_id;
        case DRV_ERRNO_RESET_REBOOT_REQ:
            rdr_mod_id = RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID;
            return rdr_mod_id;
        case DRV_ERROR_USER_RESET:
            rdr_mod_id = RDR_MODEM_CP_RESET_USER_RESET_MOD_ID;
            return rdr_mod_id;
        case DRV_ERRNO_DLOCK:
            rdr_mod_id = RDR_MODEM_CP_RESET_DLOCK_MOD_ID;
            return rdr_mod_id;
        default:
            break;
        }
    }

    if((DRV_ERRNO_DUMP_CP_WDT == drv_mod_id))
    {
        rdr_mod_id = RDR_MODEM_CP_WDT_MOD_ID;
    }
    else if((DRV_ERRNO_DLOCK == drv_mod_id))
    {
        rdr_mod_id = RDR_MODEM_CP_RESET_DLOCK_MOD_ID;
    }
    else if((drv_mod_id <= (u32)RDR_MODEM_CP_DRV_MOD_ID_END)
        && EDITION_INTERNAL_BETA !=dump_get_edition_type())
    {
        rdr_mod_id = RDR_MODEM_AP_DRV_MOD_ID;
    }
    else
    {
        rdr_mod_id = RDR_MODEM_AP_MOD_ID;
    }

    return rdr_mod_id;

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
    return dump_mdmcp_callback(modid,etype,coreid,logpath,fndone);
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
    dump_mdmcp_reset(modid,etype,coreid);
}
/*****************************************************************************
* 函 数 名  : dump_register_modem_exc_info
* 功能描述  : modem dump初始化第一阶段
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_register_modem_exc_info(void)
{
    u32 i = 0;
    struct rdr_module_ops_pub soc_ops = {
    .ops_dump = NULL,
    .ops_reset = NULL
    };

    for(i=0; i<sizeof(g_modem_exc_info)/sizeof(struct rdr_exception_info_s); i++)
    {

        if(rdr_register_exception(&g_modem_exc_info[i]) == 0)
        {
            dump_error("fail to register exception to rdr,modid=0x%x\n", g_modem_exc_info[i].e_modid);
            return BSP_ERROR;
        }
    }


    soc_ops.ops_dump  = (pfn_dump)dump_callback;
    soc_ops.ops_reset = (pfn_reset)dump_reset;
    if(rdr_register_module_ops(RDR_CP, &soc_ops, &(g_rdr_exc_info.soc_rst)) != BSP_OK)
    {
        dump_error("fail to register ops\n");
        return BSP_ERROR;
    }
    g_rdr_exc_info.dump_done = NULL;

    return BSP_OK;

}


