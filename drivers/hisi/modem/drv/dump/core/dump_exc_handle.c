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
#undef	THIS_MODU
#define THIS_MODU mod_dump

rdr_exc_info_s                  g_rdr_exc_info[EXC_INFO_BUTT];

dump_exception_ctrl_s           g_exception_ctrl;

dump_exception_info_s           g_curr_excption[EXC_INFO_BUTT];

DUMP_CP_RESET_CTRL              g_dump_mdm_reset_record;

DUMP_MDM_RESET_FAIL_CTRL        g_dump_mdm_reset_fail_record;


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
    {RDR_MODEM_CP_MSP_MOD_ID_START,RDR_MODEM_CP_MSP_MOD_ID_END,RDR_MODEM_CP_LMSP_MOD_ID},
    {RDR_MODEM_CP_LPS_MOD_ID_START,RDR_MODEM_CP_LPS_MOD_ID_END,RDR_MODEM_CP_LPS_MOD_ID},
    {RDR_MODEM_CP_TLDSP_MOD_ID_START,RDR_MODEM_CP_TLDSP_MOD_ID_END,RDR_MODEM_CP_TLDSP_MOD_ID},
    {RDR_MODEM_CP_CPHY_MOD_ID_START,RDR_MODEM_CP_CPHY_MOD_ID_END,RDR_MODEM_CP_CPHY_MOD_ID},
    {RDR_MODEM_CP_IMS_MOD_ID_START,RDR_MODEM_CP_IMS_MOD_ID_END,RDR_MODEM_CP_IMS_MOD_ID},
};


modem_cp_exc_desc  g_dump_cp_desc[]= {
    {RDR_MODEM_CP_DRV_MOD_ID,"modem cp drv err"},
    {RDR_MODEM_CP_OSA_MOD_ID,"modem cp osa err"},
    {RDR_MODEM_CP_OAM_MOD_ID,"modem cp oam err"},
    {RDR_MODEM_CP_GUL2_MOD_ID,"modem cp gul2 err"},
    {RDR_MODEM_CP_CTTF_MOD_ID,"modem cp cttf err"},
    {RDR_MODEM_CP_GUWAS_MOD_ID,"modem cp guwas err"},
    {RDR_MODEM_CP_CAS_MOD_ID,"modem cp cas err"},
    {RDR_MODEM_CP_CPROC_MOD_ID,"modem cp cproc err"},
    {RDR_MODEM_CP_GUGAS_MOD_ID,"modem cp gas err"},
    {RDR_MODEM_CP_GUCNAS_MOD_ID,"modem cp gucnas err"},
    {RDR_MODEM_CP_GUDSP_MOD_ID,"modem cp gudsp err"},
    {RDR_MODEM_CP_LPS_MOD_ID,"modem cp tlps err"},
    {RDR_MODEM_CP_LMSP_MOD_ID,"modem cp tlmsp err"},
    {RDR_MODEM_CP_TLDSP_MOD_ID,"modem cp tldsp err"},
    {RDR_MODEM_CP_CPHY_MOD_ID,"modem cp cphy err"},
    {RDR_MODEM_CP_IMS_MOD_ID,"modem cp ims err"},
};


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
    memset_s(rdr_info->log_path,sizeof(rdr_info->log_path),'\0',sizeof(rdr_info->log_path));
    memcpy_s(rdr_info->log_path,sizeof(rdr_info->log_path) ,logpath, strlen(logpath));
    memcpy_s(rdr_info->log_path + strlen(logpath) ,(sizeof(rdr_info->log_path)-strlen(logpath)), RDR_DUMP_FILE_CP_PATH, strlen(RDR_DUMP_FILE_CP_PATH));

    dump_ok("this exception logpath is %s\n", rdr_info->log_path);

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

void dump_get_nr_excinfo(u32 modid,void* exc_sub_type, void* sub_sys, void* desc)
{
    if(modid >= RDR_MODEM_NR_MOD_ID_START && modid <= RDR_MODEM_NR_MOD_ID_END)
    {
        if(modid >= NRRDR_MODEM_NR_CCPU_START && modid <= NRRDR_MODEM_NR_CCPU_END)
        {
            (*(u32*)exc_sub_type) = DUMP_CPU_NRCCPU;
            memcpy_s(desc,48,NRCCPU_EXCEPTION,strlen(NRCCPU_EXCEPTION));
        }
        if(modid >= NRRDR_MODEM_NR_L2HAC_START && modid <= NRRDR_MODEM_NR_L2HAC_END)
        {
            (*(u32*)exc_sub_type) = DUMP_CPU_NRL2HAC;
            memcpy_s(desc,48,NRL2HAC_EXCEPTION,strlen(NRL2HAC_EXCEPTION));
        }
    }
}
void dump_clear_reset_fail_info(void)
{
    /*coverity[secure_coding]*/
    memset_s(&g_dump_mdm_reset_fail_record,sizeof(g_dump_mdm_reset_fail_record),0,sizeof(g_dump_mdm_reset_fail_record));
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
    /*coverity[secure_coding]*/
    memset_s(&g_dump_mdm_reset_record,sizeof(g_dump_mdm_reset_record),0,sizeof(g_dump_mdm_reset_record));
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

            /*coverity[secure_coding]*/
            memset_s(&g_dump_mdm_reset_record,sizeof(g_dump_mdm_reset_record),0,sizeof(g_dump_mdm_reset_record));
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

u32 dump_match_lrccpu_rdr_id(u32 mdmcp_mod_id)
{
    u32 i = 0;
    u32 rdr_id = RDR_MODEM_CP_DRV_MOD_ID_START;
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
* 函 数 名  : dump_print_mdmcp_error
* 功能描述  : 打印CP的异常类型
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2018年7月28日15:00:33      creat
*
*****************************************************************************/

void dump_print_mdmcp_error(u32 rdr_id)
{
    u32 i = 0;
    for(i = 0; i < sizeof(g_dump_cp_desc)/sizeof(g_dump_cp_desc[0]);i++)
    {
        if(rdr_id == g_dump_cp_desc[i].modem_cp_modid)
        {
            dump_ok("%s\n",g_dump_cp_desc[i].desc);
            return;
        }
    }
    dump_ok("modem cp drv err\n");
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

    /*手机版本noc错误*/
    rdr_id = dump_match_noc_rdr_id(dump_exception_info->mod_id,dump_exception_info->arg1);
    if(rdr_id== RDR_MODEM_DRV_BUTT_MOD_ID)
    {
        /*手机版本cp传过来的错误码*/
        rdr_id = dump_match_lrccpu_rdr_id(dump_exception_info->mod_id);
        dump_print_mdmcp_error(rdr_id);
    }

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
    /*NR规划的错误码是直传的以0xB开头*/
    return dump_exception_info->mod_id;
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
    if(dump_exception_info->core ==DUMP_CPU_NRCCPU)
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
                                 u32 length,u32 core,u32 reason,char* desc,
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
            memcpy_s(exception_info_s->task_name,sizeof(exception_info_s->task_name),task_name,strlen(task_name));
        }
    }

    if(desc)
    {
        memcpy_s(exception_info_s->exc_desc,sizeof(exception_info_s->exc_desc),desc,strlen(desc));
    }
    dump_ok("fill excption info done\n");

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
void dump_wait_excption_handle_done(void)
{
    s32 ret;
    dump_ok("start to wait exception handler done\n");
    ret = down_timeout(&g_exception_ctrl.sem_wait,msecs_to_jiffies(WAIT_EXCEPTION_HANDLE_TIME));
    if(ret != 0)
    {
        dump_ok("wait exception handler timeout\n");
    }
    else
    {
        dump_ok("exception handler done\n");
    }
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


    if(index >= EXC_INFO_BUTT)
    {
        return ;
    }
    /*清除上次的异常信息*/
    memset_s(&g_curr_excption[index],sizeof(g_curr_excption[index]),sizeof(g_curr_excption[index]),0);

    if(index ==  EXC_INFO_NORMAL)
    {
        /*释放信号量*/
        up(&g_exception_ctrl.sem_wait);
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
    dump_exception_info_s* next_info_s = NULL;
    dump_exception_info_s* dest_exc_info = NULL;
    s32 ret = BSP_ERROR;

    for(;;)
    {
        down(&g_exception_ctrl.sem_exception_task);

        dump_ok("enter excption handler task \n");

        spin_lock_irqsave(&g_exception_ctrl.lock, flags);
        if(unlikely(list_empty(&g_exception_ctrl.exception_list)))
        {
            spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);
            dump_error("g_exception_ctrl.exception_list is empty\n");
            continue;
        }
        list_for_each_entry_safe(curr_info_s,next_info_s,&g_exception_ctrl.exception_list,exception_list)
        {
            dump_ok("current rdr id =0x%x\n",curr_info_s->rdr_mod_id);
            dest_exc_info = dump_get_current_excpiton_info(curr_info_s->rdr_mod_id);
            if(dest_exc_info != NULL)
            {
                memcpy_s(dest_exc_info,sizeof(*dest_exc_info),curr_info_s,sizeof(*curr_info_s));
            }
            break;
        }
        list_del(&curr_info_s->exception_list);
        kfree(curr_info_s);

        spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

        if(dest_exc_info == NULL)
        {
            continue;
        }
        ret = dump_check_reset_freq(dest_exc_info->rdr_mod_id);

        if(ret == BSP_OK && BSP_OK == dump_check_need_report_excption(dest_exc_info->rdr_mod_id))
        {
            if(dest_exc_info != NULL)
            {
                dump_show_excption_info(dest_exc_info);
                /*每个节点都是添加到队尾，实际上每次取到的就是时间戳最早的*/
                rdr_system_error(dest_exc_info->rdr_mod_id,dest_exc_info->arg1,dest_exc_info->arg2);
            }
        }
        else
        {
            dump_ok("this error no need call rdr_system_error\n");
        }
        /*ret!=BSP_OK说明是进入单独复位频率控制流程里，不再重启Modem*/
        if(ret != BSP_OK)
        {
            /*当前只有noc和dmss的错误是不需要经过rdr_system_error处理的，但仍然需要保存log，需要在此统一等待*/
            dump_wait_excption_handle_done();
        }

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
    sema_init(&g_exception_ctrl.sem_wait,0);

    g_exception_ctrl.init_flag = true;

    dump_ok("exception handler init ok\n");

    return BSP_OK;

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
    memset_s(exception_info_s,sizeof(*exception_info_s),0,sizeof(*exception_info_s));
    memcpy_s(exception_info_s,sizeof(*exception_info_s),current_exception,sizeof(*current_exception));

    spin_lock_irqsave(&g_exception_ctrl.lock, flags);

    list_add_tail(&exception_info_s->exception_list,&g_exception_ctrl.exception_list);

    spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

    dump_ok("register exception ok \n");

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
u32 dump_mdm_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone)
{
    u32 ret = BSP_OK;

    bsp_modem_cold_patch_update_modem_fail_count();

    if(NULL != fndone)
        fndone(modid, coreid);
    return ret;
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
void dump_mdm_reset(u32 modid, u32 etype, u64 coreid)
{
    dump_exception_info_s* current_exception = NULL;
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

       (void)dump_register_exception(&exception_info_s);

    }

    current_exception  = dump_get_current_excpiton_info(modid);
    if(current_exception != NULL)
    {
        /*如果是c核触发异常，重新打开开门狗*/
        if(current_exception->core == DUMP_CPU_LRCCPU)
        {
            bsp_wdt_irq_enable(WDT_CCORE_ID);
        }
    }

    /*如果是单独复位结束，此次异常处理也结束了，
    没有单独复位，全系统也就复位了，不需要执行释放动作*/
    dump_excption_handle_done(modid);

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

    memset_s(&g_rdr_exc_info,sizeof(g_rdr_exc_info),0,sizeof(g_rdr_exc_info));
    soc_ops.ops_dump  = (pfn_dump)dump_mdm_callback;
    soc_ops.ops_reset = (pfn_reset)dump_mdm_reset;
    if(rdr_register_module_ops(RDR_CP, &soc_ops, &(soc_rst)) != BSP_OK)
    {
        dump_error("fail to register  rdr ops \n");
        return BSP_ERROR;
    }

    dump_ok("register modem exc info ok");
    return BSP_OK;

}


