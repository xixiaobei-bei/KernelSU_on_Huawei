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

#ifndef PS_LOG_MSP_FILE_ID_DEFINE_H
#define PS_LOG_MSP_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
                 MSP_FILE_ID_AT_LTE_COMMON_C = MSP_TEAM_FILE_ID,
                 MSP_FILE_ID_AT_LTE_UPGRADE_PROC_C,
                 MSP_FILE_ID_AT_LTE_EVENTREPORT_C,
                 MSP_FILE_ID_AT_LTE_CT_PROC_C,
                 MSP_FILE_ID_AT_LTE_ST_PROC_C,
                 MSP_FILE_ID_AT_LTE_MS_PROC_C,
                 MSP_FILE_ID_FTM_CT_CORE_C,
                 MSP_FILE_ID_MSP_SERVICE_C,
                 MSP_FILE_ID_MSP_DEBUG_C,
                 MSP_FILE_ID_DIAG_ACORE_COMMON_C,
                 MSP_FILE_ID_DIAG_CCORE_COMMON_C,
                 MSP_FILE_ID_DIAG_NRM_COMMON_C,
                 MSP_FILE_ID_DIAG_TS_C,
                 MSP_FILE_ID_DIAG_SERVICE_C,
                 MSP_FILE_ID_DIAG_GTR_PROC_C,
                 MSP_FILE_ID_DIAG_NV_PROC_C,
                 MSP_FILE_ID_DIAG_ERRORLOG_C,
                 MSP_FILE_ID_DIAG_MESSAGE_C,
                 MSP_FILE_ID_DIAG_MSGMSP_C,
                 MSP_FILE_ID_DIAG_MSGBSP_C,
                 MSP_FILE_ID_DIAG_MSGDSP_C,
                 MSP_FILE_ID_DIAG_MSGBBP_C,
                 MSP_FILE_ID_DIAG_MSGPS_C,
                 MSP_FILE_ID_DIAG_MSGHIFI_C,
                 MSP_FILE_ID_DIAG_MSGLRM_C,
                 MSP_FILE_ID_DIAG_MSGNRM_C,
                 MSP_FILE_ID_DIAG_MSGHAC_C,
                 MSP_FILE_ID_DIAG_API_C,
                 MSP_FILE_ID_DIAG_ADP_NRM_C,
                 MSP_FILE_ID_DIAG_CFG_C,
                 MSP_FILE_ID_DIAG_DEBUG_C,
                 MSP_FILE_ID_DIAG_BUF_CTRL_C,
                 MSP_FILE_ID_DIAG_FS_PROC_C,
                 MSP_FILE_ID_DIAG_CONNECT_C,
                 MSP_FILE_ID_BBP_AGENT_C,
                 MSP_FILE_ID_L4A_PROC_C,
                 MSP_FILE_ID_L4A_FUN_C,
                 MSP_FILE_ID_DRX_API_C,
                 MSP_FILE_ID_DRX_TEST_C,
                 MSP_FILE_ID_FTM_TDS_CMD_C,
                 MSP_FILE_ID_FTM_MAILBOX_PROC_C,
                 MSP_FILE_ID_FTM_COMMON_API_C,
                 MSP_FILE_ID_FTM_COMM_CMD_C,
                 MSP_FILE_ID_FTM_BASE_CMD_C,
                 MSP_FILE_ID_FTM_LTE_NONSIGNAL_BT_C,
                 MSP_FILE_ID_FTM_LTE_QUICK_CT_C,
                 MSP_FILE_ID_AT_COMM_CMD_C,
                 MSP_FILE_ID_LTE_SLEEPFLOW_C,
                 MSP_FILE_ID_FTM_MNTN_C,
                 MSP_FILE_ID_DIAG_MSGEASYRF_C,
                 MSP_FILE_ID_DIAG_MSGHL1C_C,
                 MSP_FILE_ID_DIAG_DT_C,

                 MSP_FILE_ID_BUTT
}MSP_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


