/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include <product_config.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <bsp_dt.h>
#include "mdrv_errno.h"
#include "osl_types.h"
#include "osl_thread.h"
#include "osl_sem.h"
#include "bsp_dump.h"
#include "bsp_dump_mem.h"
#include "bsp_slice.h"
#include "bsp_reset.h"
#include "bsp_wdt.h"
#include "bsp_cold_patch.h"
#include "nv_stru_drv.h"
#include "crdr_config.h"
#include "crdr_cp_agent.h"
#include "crdr_exc_handle.h"
#include "crdr_cp_core.h"
#include "crdr_mdmap_core.h"
#include "crdr_nr_core.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

rdr_exc_info_s g_rdr_exc_info[EXC_INFO_BUTT];
dump_exception_ctrl_s g_exception_ctrl;
dump_exception_info_s g_crdr_curr_node[EXC_INFO_BUTT];
dump_product_type_e g_product_type = DUMP_PRODUCT_BUTT;
atomic_t g_crdr_exc_proc;
NV_DUMP_STRU g_crdr_cfg;

struct rdr_exception_info_s g_modem_exc_info[] = {
#ifndef BSP_CONFIG_PHONE_TYPE
    {
        .e_modid = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_AP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMAP,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "Modem",
        .e_desc = "Modem reset system",
    },
#else
    {
        .e_modid = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_AP_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMAP,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMAP",
        .e_desc = "modem ap reset system",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_AP_DRV_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_AP_DRV_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = RDR_AP | RDR_CP,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMAP,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMAP",
        .e_desc = "modem ap drv reset system",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_LPM3_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_LPM3_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMLPM3",
        .e_desc = "modem lpm3 exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_DRV_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_DRV_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp drv exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_OSA_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_PAM_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp osa exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_OAM_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_PAM_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp oam exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_GUL2_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUAS_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp gul2 exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_CTTF_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_CTTF_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp cttf exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_GUWAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUAS_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp guwas exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_CAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_CAS_CPROC_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp cas exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_CPROC_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_CAS_CPROC_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp cproc exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_GUGAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUAS_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp guas exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_GUCNAS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUCNAS_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp gucnas exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_GUDSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUDSP_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp gudsp exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_EASYRF_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_GUDSP_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp easyRF exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_LPS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_TLPS_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp tlps exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_LMSP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_LMSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_DRV_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp lmsp exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_TLDSP_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_TLDSP_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp tldsp exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_CPHY_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_CPHY_EXC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp cphy exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_IMS_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem cp ims exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_NORMALRESET,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem normal reboot",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_RESETFAIL,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem self-reset fail",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_RESETFAIL,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem reset frequently",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_WDT_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem self-reset wdog",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_RILD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_RILD_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem reset by rild",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_3RD_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_3RD_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem reset by 3rd modem",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_NOC_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem noc reset",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_NORMALRESET,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem reset stub",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_NOC_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMNOC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem noc error",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_AP_NOC_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_AP_NOC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMNOC,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem noc reset system",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = 0,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_NORMALRESET,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem user reset without log",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_DMSS_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_DMSS_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMDMSS,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem dmss error",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CP_RESET_DLOCK_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMCP",
        .e_desc = "modem reset by bus error",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_CODE_PATCH_REVERT_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_CODE_PATCH_REVERT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NOW,
        .e_notify_core_mask = 0,
        .e_reset_core_mask = RDR_AP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW,
        .e_exce_type = CP_S_MODEMAP,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "MDMAP",
        .e_desc = "modem cold patch revert",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_NR_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_NR_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "NR",
        .e_desc = "modem nr exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_NR_L2HAC_MOD_ID,
        .e_modid_end = (unsigned int)RDR_MODEM_NR_L2HAC_MOD_ID,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "NR",
        .e_desc = "modem l2hac exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },
    {
        .e_modid = (unsigned int)RDR_MODEM_NR_CCPU_WDT,
        .e_modid_end = (unsigned int)RDR_MODEM_NR_CCPU_WDT,
        .e_process_priority = RDR_WARN,
        .e_reboot_priority = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3,
        .e_reset_core_mask = RDR_CP,
        .e_from_core = RDR_CP,
        .e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW,
        .e_exce_type = CP_S_EXCEPTION,
        .e_upload_flag = (unsigned int)RDR_UPLOAD_YES,
        .e_from_module = "NR",
        .e_desc = "modem nr wdt exc",
        .e_save_log_flags = RDR_SAVE_LOGBUF,
    },

#endif
};
int crdr_read_feature_form_dts(void)
{
    return BSP_ERROR;
}
void crdr_feature_config_init(void)
{
    s32 ret;

    ret = bsp_nvm_read(NVID_DUMP, (u8 *)&g_crdr_cfg, sizeof(NV_DUMP_STRU));
    if (ret != BSP_OK) {
        /* 使用默认值 */
        g_crdr_cfg.dump_cfg.Bits.dump_switch = 0x1;
        g_crdr_cfg.dump_cfg.Bits.ARMexc = 0x1;
        g_crdr_cfg.dump_cfg.Bits.stackFlow = 0x1;
        g_crdr_cfg.dump_cfg.Bits.taskSwitch = 0x1;
        g_crdr_cfg.dump_cfg.Bits.intSwitch = 0x1;
        g_crdr_cfg.dump_cfg.Bits.intLock = 0x1;
        g_crdr_cfg.dump_cfg.Bits.appRegSave1 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.appRegSave2 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.appRegSave3 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.commRegSave1 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.commRegSave2 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.commRegSave3 = 0x0;
        g_crdr_cfg.dump_cfg.Bits.sysErrReboot = 0x1;
        g_crdr_cfg.dump_cfg.Bits.reset_log = 0x1;
        g_crdr_cfg.dump_cfg.Bits.fetal_err = 0x1;
        g_crdr_cfg.dump_cfg.Bits.dumpTextClip = 0x0;
        crdr_error("fail to read dump nv,nv id = 0x%x\n", NVID_DUMP);
    }
}

NV_DUMP_STRU *crdr_get_feature_cfg(void)
{
    return &g_crdr_cfg;
}

void crdr_set_exc_state(u32 state)
{
    atomic_set(&g_crdr_exc_proc, state);
}
u32 crdr_get_exc_state(void)
{
    if (atomic_read(&g_crdr_exc_proc)) {
        return true;
    } else {
        return false;
    }
}

dump_product_type_e crdr_get_product_type(void)
{
    device_node_s *node = NULL;
    const char *product_type = NULL;

    if (g_product_type != DUMP_PRODUCT_BUTT) {
        return g_product_type;
    }
    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,smntn_type");
    if (node == NULL) {
        crdr_error("fail to read dts node mntn type !\n");
        return DUMP_MBB;
    }

    if (bsp_dt_property_read_string(node, "product_type", &product_type)) {
        crdr_error("fail to read product_type !\n");
        return DUMP_MBB;
    }

    if (0 == strncmp(product_type, "MBB", strlen("MBB"))) {
        g_product_type = DUMP_MBB;
    } else if (0 == strncmp(product_type, "PHONE", strlen("PHONE"))) {
        g_product_type = DUMP_PHONE;
    } else {
        crdr_error("find product type error use mbb default\n");
        g_product_type = DUMP_MBB;
    }
    return g_product_type;
}

/*
 * 功能描述: 匹配ap侧触发的和cp强相关的错误
 */
u32 crdr_match_special_rdr_id(u32 modid)
{
    u32 rdr_mod_id = RDR_MODEM_AP_MOD_ID;
    if (DUMP_PHONE == crdr_get_product_type()) {
        switch (modid) {
            case DRV_ERRNO_RESET_SIM_SWITCH:
                rdr_mod_id = RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID;
                break;
            case RILD_REBOOT_MOD_ID:
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

rdr_exc_info_s *crdr_get_rdr_exc_info_by_modid(u32 modid)
{
    rdr_exc_info_s *rdr_info = NULL;

    if (modid == RDR_MODEM_NOC_MOD_ID) {
        rdr_info = &g_rdr_exc_info[EXC_INFO_NOC];
    } else if (modid == RDR_MODEM_DMSS_MOD_ID) {
        rdr_info = &g_rdr_exc_info[EXC_INFO_DMSS];
    } else {
        rdr_info = &g_rdr_exc_info[EXC_INFO_NORMAL];
    }

    return rdr_info;
}

u32 crdr_get_exc_index(u32 modid)
{
    u32 index = EXC_INFO_NORMAL;
    if (modid == RDR_MODEM_NOC_MOD_ID) {
        index = EXC_INFO_NOC;
    } else if (modid == RDR_MODEM_DMSS_MOD_ID) {
        index = EXC_INFO_DMSS;
    } else {
        index = EXC_INFO_NORMAL;
    }
    return index;
}

dump_exception_info_s *crdr_get_current_excpiton_info(u32 modid)
{
    u32 index = crdr_get_exc_index(modid);
    if (unlikely(index >= EXC_INFO_BUTT)) {
        return NULL;
    }
    return &(g_crdr_curr_node[index]);
}

int crdr_handle_task(void *data)
{
    unsigned long flags;
    dump_exception_info_s *curr_info_s = NULL;
    dump_exception_info_s *next_info_s = NULL;
    dump_exception_info_s *dest_exc_info = NULL;

    for (;;) {
        osl_sem_down(&g_exception_ctrl.sem_exception_task);

        crdr_ok("enter excption handler task \n");

        spin_lock_irqsave(&g_exception_ctrl.lock, flags);
        if (unlikely(list_empty(&g_exception_ctrl.exception_list))) {
            spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);
            crdr_error("g_exception_ctrl.exception_list is empty\n");
            continue;
        }
        list_for_each_entry_safe(curr_info_s, next_info_s, &g_exception_ctrl.exception_list, exception_list)
        {
            crdr_ok("current rdr id =0x%x\n", curr_info_s->rdr_mod_id);
            dest_exc_info = crdr_get_current_excpiton_info(curr_info_s->rdr_mod_id);
            if (dest_exc_info != NULL) {
                if (memcpy_s(dest_exc_info, sizeof(*dest_exc_info), curr_info_s, sizeof(*curr_info_s))) {
                    bsp_debug("err\n");
                }
            }
            break;
        }
        list_del(&curr_info_s->exception_list);
        kfree(curr_info_s);

        spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

        if (dest_exc_info == NULL) {
            continue;
        }

        rdr_system_error(dest_exc_info->rdr_mod_id, dest_exc_info->arg1, dest_exc_info->arg2);

        crdr_ok("exit excption handler task \n");
    }
}

__init s32 crdr_exception_handler_init(void)
{
    struct task_struct *pid = NULL;
    struct sched_param param = {
        0,
    };
    crdr_feature_config_init();
    spin_lock_init(&g_exception_ctrl.lock);

    sema_init(&g_exception_ctrl.sem_exception_task, 0);

    INIT_LIST_HEAD(&g_exception_ctrl.exception_list);

    pid = (struct task_struct *)kthread_run(crdr_handle_task, 0, "Modem_exception");
    if (IS_ERR((void *)pid)) {
        crdr_error("fail to create kthread task failed! \n");
        return BSP_ERROR;
    }
    g_exception_ctrl.exception_task_id = (uintptr_t)pid;

    param.sched_priority = CRDR_EXC_TASK_PRIO;
    if (BSP_OK != sched_setscheduler(pid, SCHED_FIFO, &param)) {
        crdr_error("fail to set scheduler failed!\n");
        return BSP_ERROR;
    }
    sema_init(&g_exception_ctrl.sem_wait, 0);

    g_exception_ctrl.init_flag = true;

    crdr_ok("exception handler init ok\n");
    crdr_set_exc_state(0);

    return BSP_OK;
}

s32 crdr_register_exception(dump_exception_info_s *current_exception)
{
    dump_exception_info_s *exception_info_s = NULL;
    unsigned long flags;

    if (unlikely(current_exception == NULL)) {
        crdr_error("param exception_info is null\n");
        return BSP_ERROR;
    }
    /* 中断上下文也可能出现异常 */
    exception_info_s = kmalloc(sizeof(dump_exception_info_s), GFP_ATOMIC);
    if (unlikely(exception_info_s == NULL)) {
        crdr_error("fail to malloc space\n");
        return BSP_ERROR;
    }
    if (memset_s(exception_info_s, sizeof(*exception_info_s), 0, sizeof(*exception_info_s))) {
        bsp_debug("err\n");
    }
    if (memcpy_s(exception_info_s, sizeof(*exception_info_s), current_exception, sizeof(*current_exception))) {
        bsp_debug("err\n");
    }

    spin_lock_irqsave(&g_exception_ctrl.lock, flags);

    list_add_tail(&exception_info_s->exception_list, &g_exception_ctrl.exception_list);

    spin_unlock_irqrestore(&g_exception_ctrl.lock, flags);

    crdr_ok("register exception ok \n");

    up(&g_exception_ctrl.sem_exception_task);

    return BSP_OK;
}

void crdr_mdm_callback(unsigned int modid, unsigned int etype, unsigned long long coreid, char *logpath,
                       pfn_cb_dump_done fndone)
{

    if (fndone != NULL) {
        fndone(modid, coreid);
    }
}
struct rdr_exception_info_s *crdr_get_exception_info_node(u32 mod_id)
{
    u32 i = 0;
    struct rdr_exception_info_s *rdr_exc_info = NULL;

    for (i = 0; i < ARRAY_SIZE(g_modem_exc_info); i++) {
        if (g_modem_exc_info[i].e_modid <= mod_id && mod_id <= g_modem_exc_info[i].e_modid_end) {
            rdr_exc_info = &g_modem_exc_info[i];
        }
    }
    return rdr_exc_info;
}

s32 crdr_check_single_reset_by_nv(void)
{
    NV_DUMP_STRU *cfg = NULL;
    cfg = crdr_get_feature_cfg();
    if (cfg != NULL && cfg->dump_cfg.Bits.sysErrReboot == 0) {
        crdr_ok("close modem sigle reset\n");
        return BSP_ERROR;
    }

    crdr_ok(" modem sigle reset open\n");

    return BSP_OK;
}

s32 crdr_check_single_reset_by_modid(u32 modid)
{
    struct rdr_exception_info_s *exception_info_s = crdr_get_exception_info_node(modid);
    if (exception_info_s != NULL) {
        if (exception_info_s->e_reset_core_mask == RDR_CP) {
            crdr_ok("go to modem reset\n");
            return BSP_OK;
        }
        if (exception_info_s->e_reset_core_mask == RDR_AP) {
            crdr_ok("need reset system: 0x%x\n", modid);
            return BSP_ERROR;
        }
    }
    crdr_error("invalid mod id: 0x%x\n", modid);
    return BSP_ERROR;
}

s32 crdr_check_cp_reset(u32 modid)
{
    if (DUMP_PHONE != crdr_get_product_type()) {
        crdr_ok("mbb not support cp_reset\n");
        return BSP_ERROR;
    }
    if (BSP_ERROR == crdr_check_single_reset_by_modid(modid)) {
        crdr_ok("modid not support cp_reset\n");
        return BSP_ERROR;
    }
    if (BSP_ERROR == crdr_check_single_reset_by_nv()) {
        crdr_ok("crdr_check_single_reset_by_nv retun not support\n");
        return BSP_ERROR;
    }

    return BSP_OK;
}

dump_reset_result_e crdr_cp_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    crdr_ok("enter dump reset, mod id:0x%x\n", modid);

    if (BSP_OK == crdr_check_cp_reset(modid)) {
        ret = bsp_cp_reset();
        if (ret == -1) {
            return RESET_NOT_SUPPORT;
        }
        if (!bsp_reset_is_successful(RDR_MODEM_CP_RESET_TIMEOUT)) {
            return RESET_NOT_SUCCES;
        }
        return RESET_SUCCES;
    }
    return RESET_NOT_SUPPORT;
}

void crdr_mdm_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    dump_exception_info_s exception_info_s = {
        0,
    };

    if (bsp_modem_is_reboot_machine()) {
        crdr_ok("modem need reboot whole system,without logs\n");
        rdr_system_error(RDR_MODEM_CODE_PATCH_REVERT_MOD_ID, 0, 0);
        return;
    }
    ret = crdr_cp_reset(modid, etype, coreid);
    /* 任何失败的场景均执行整机重启 */
    if (ret != RESET_SUCCES) {
        exception_info_s.core = DUMP_CPU_LRCCPU;
        exception_info_s.arg1 = 0;
        exception_info_s.arg2 = 0;
        if (ret == RESET_NOT_SUPPORT) {
            exception_info_s.mod_id = DRV_ERRNO_RESET_REBOOT_REQ;
            exception_info_s.rdr_mod_id = crdr_match_special_rdr_id(DRV_ERRNO_RESET_REBOOT_REQ);
        } else {
            exception_info_s.mod_id = DRV_ERRNO_RST_FAIL;
            exception_info_s.rdr_mod_id = crdr_match_special_rdr_id(DRV_ERRNO_RST_FAIL);
        }

        (void)crdr_register_exception(&exception_info_s);
    } else {
        crdr_set_exc_state(0);
    }
}

__init s32 crdr_register_modem_exc_info(void)
{
    u32 i = 0;
    struct rdr_module_ops_pub soc_ops = {
        .ops_dump = NULL,
        .ops_reset = NULL
    };
    struct rdr_register_module_result soc_rst = { 0, 0, 0 };

    for (i = 0; i < sizeof(g_modem_exc_info) / sizeof(g_modem_exc_info[0]); i++) {
        if (rdr_register_exception(&g_modem_exc_info[i]) == 0) {
            crdr_error("modid:0x%x register rdr exception failed\n", g_modem_exc_info[i].e_modid);
            return BSP_ERROR;
        }
    }

    if (memset_s(&g_rdr_exc_info, sizeof(g_rdr_exc_info), 0, sizeof(g_rdr_exc_info))) {
        bsp_debug("err\n");
    }
    soc_ops.ops_dump = (pfn_dump)crdr_mdm_callback;
    soc_ops.ops_reset = (pfn_reset)crdr_mdm_reset;

    if (rdr_register_module_ops(RDR_CP, &soc_ops, &(soc_rst)) != BSP_OK) {
        crdr_error("fail to register  rdr ops \n");
        return BSP_ERROR;
    }
    crdr_ok("register modem exc info ok");
    return BSP_OK;
}
