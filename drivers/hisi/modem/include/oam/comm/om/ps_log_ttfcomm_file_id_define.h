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

#ifndef PS_LOG_TTFCOMM_FILE_ID_DEFINE_H
#define PS_LOG_TTFCOMM_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* TTFCOMM */
typedef enum
{
    PS_FILE_ID_PPPC_SHEL_C                      = TTFCOMM_TEAM_FILE_ID,         /* TTFCOMM_TEAM_FILE_ID = 0x2000 */
    PS_FILE_ID_PPPC_ENTITY_C                    = TTFCOMM_TEAM_FILE_ID + 0x0001,
    PS_FILE_ID_PPPC_CORE_C                      = TTFCOMM_TEAM_FILE_ID + 0x0002,
    PS_FILE_ID_PPPC_CHAP_C                      = TTFCOMM_TEAM_FILE_ID + 0x0003,
    PS_FILE_ID_PPPC_PAP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0004,
    PS_FILE_ID_PPPC_CVT_C                       = TTFCOMM_TEAM_FILE_ID + 0x0005,
    PS_FILE_ID_PPPC_CAP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0006,
    PS_FILE_ID_PPPC_CTRL_C                      = TTFCOMM_TEAM_FILE_ID + 0x0007,
    PS_FILE_ID_PPPC_PA_AGENT_C                  = TTFCOMM_TEAM_FILE_ID + 0x0008,
    PS_FILE_ID_PPPC_LCP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0009,
    PS_FILE_ID_PPPC_FSM_C                       = TTFCOMM_TEAM_FILE_ID + 0x000A,
    PS_FILE_ID_PPPC_IPCP_C                      = TTFCOMM_TEAM_FILE_ID + 0x000B,
    PS_FILE_ID_PPPC_IPV6CP_C                    = TTFCOMM_TEAM_FILE_ID + 0x000C,
    PS_FILE_ID_PPPC_MNTN_CTRL_C                 = TTFCOMM_TEAM_FILE_ID + 0x000D,
    PS_FILE_ID_PPPC_ERR_LOG_C                   = TTFCOMM_TEAM_FILE_ID + 0x000E,
    PS_FILE_ID_PPPC_DIAG_C                      = TTFCOMM_TEAM_FILE_ID + 0x000F,
    PS_FILE_ID_PPPC_UTILITY_C                   = TTFCOMM_TEAM_FILE_ID + 0x0010,
    PS_FILE_ID_PPPC_1X_HRPD_PROC_C              = TTFCOMM_TEAM_FILE_ID + 0x0011,
    PS_FILE_ID_PPPC_EHPRD_PROC_C                = TTFCOMM_TEAM_FILE_ID + 0x0012,
    PS_FILE_ID_PPPC_MEM_C                       = TTFCOMM_TEAM_FILE_ID + 0x0013,
    PS_FILE_ID_PPPC_VSNCP_C                     = TTFCOMM_TEAM_FILE_ID + 0x0014,
    PS_FILE_ID_PPPC_VSNCP_DECODE_C              = TTFCOMM_TEAM_FILE_ID + 0x0015,
    PS_FILE_ID_PPPC_VSNCP_ENCODE_C              = TTFCOMM_TEAM_FILE_ID + 0x0016,
    PS_FILE_ID_PPPC_PAP_DBG_C                   = TTFCOMM_TEAM_FILE_ID + 0x0017,
    PS_FILE_ID_PPPC_CHAP_DBG_C                  = TTFCOMM_TEAM_FILE_ID + 0x0018,
    PS_FILE_ID_PPPC_PPPA_PROC_C                 = TTFCOMM_TEAM_FILE_ID + 0x0019,
    PS_FILE_ID_PPPC_UIM_C                       = TTFCOMM_TEAM_FILE_ID + 0x001A,
    PS_FILE_ID_PPPC_UTIL_SHA1_C                 = TTFCOMM_TEAM_FILE_ID + 0x001B,
    PS_FILE_ID_PPPC_UTIL_AES_C                  = TTFCOMM_TEAM_FILE_ID + 0x001C,
    PS_FILE_ID_PPPC_UTIL_SHA256_C               = TTFCOMM_TEAM_FILE_ID + 0x001D,
    PS_FILE_ID_PPPC_EAP_C                       = TTFCOMM_TEAM_FILE_ID + 0x001E,
    PS_FILE_ID_PPPC_EAP_MGR_C                   = TTFCOMM_TEAM_FILE_ID + 0x001F,
    PS_FILE_ID_PPPC_EAP_PEER_C                  = TTFCOMM_TEAM_FILE_ID + 0x0020,
    PS_FILE_ID_PPPC_MD5_C                       = TTFCOMM_TEAM_FILE_ID + 0x0021,
    PS_FILE_ID_PPPC_EAP_AKA_C                   = TTFCOMM_TEAM_FILE_ID + 0x0022,
    PS_FILE_ID_PPPC_HDLC_HARDWARE_C             = TTFCOMM_TEAM_FILE_ID + 0x0023,
    PS_FILE_ID_PPPC_IPCP_DBG_C                  = TTFCOMM_TEAM_FILE_ID + 0x0024,
    PS_FILE_ID_PPPC_EAP_DBG_C                   = TTFCOMM_TEAM_FILE_ID + 0x0025,
    PS_FILE_ID_PPPC_VSNCP_DBG_C                 = TTFCOMM_TEAM_FILE_ID + 0x0026,
    PS_FILE_ID_PPPC_IPV6CP_DBG_C                = TTFCOMM_TEAM_FILE_ID + 0x0027,
    PS_FILE_ID_PPPC_LCP_DBG_C                   = TTFCOMM_TEAM_FILE_ID + 0x0028,
    PS_FILE_ID_PPPC_DATA_COMMON_C               = TTFCOMM_TEAM_FILE_ID + 0x0029,
    PS_FILE_ID_PPPC_DATA_PROC_C                 = TTFCOMM_TEAM_FILE_ID + 0x002A,
    PS_FILE_ID_PPPC_ADAPTER_COMMON_C            = TTFCOMM_TEAM_FILE_ID + 0x002B,

    PS_FILE_ID_DHCP_PUB_C                       = TTFCOMM_TEAM_FILE_ID + 0x002C,
    PS_FILE_ID_DHCPM_COMP_C                     = TTFCOMM_TEAM_FILE_ID + 0x002D,
    PS_FILE_ID_DHCPM_DBG_C                      = TTFCOMM_TEAM_FILE_ID + 0x002E,
    PS_FILE_ID_DHCPC_C                          = TTFCOMM_TEAM_FILE_ID + 0x002F,
    PS_FILE_ID_DHCPC_BLOCK_C                    = TTFCOMM_TEAM_FILE_ID + 0x0030,
    PS_FILE_ID_DHCPC_COMP_C                     = TTFCOMM_TEAM_FILE_ID + 0x0031,
    PS_FILE_ID_DHCP_LOG_C                       = TTFCOMM_TEAM_FILE_ID + 0x0032,
    PS_FILE_ID_DHCPC_MSG_C                      = TTFCOMM_TEAM_FILE_ID + 0x0033,
    PS_FILE_ID_DHCPC_PKT_C                      = TTFCOMM_TEAM_FILE_ID + 0x0034,
    PS_FILE_ID_DHCPC_TIMER_C                    = TTFCOMM_TEAM_FILE_ID + 0x0035,
    PS_FILE_ID_DHCPC_SERVER_C                   = TTFCOMM_TEAM_FILE_ID + 0x0036,
    PS_FILE_ID_DHCPV6_PROC_C                    = TTFCOMM_TEAM_FILE_ID + 0x0037,
    PS_FILE_ID_DHCPV6C_C                        = TTFCOMM_TEAM_FILE_ID + 0x0038,
    PS_FILE_ID_DHCPV6C_FSM_C                    = TTFCOMM_TEAM_FILE_ID + 0x0039,
    PS_FILE_ID_DHCPV6C_PKT_C                    = TTFCOMM_TEAM_FILE_ID + 0x003A,
    PS_FILE_ID_DHCPV4S_COMP_C                   = TTFCOMM_TEAM_FILE_ID + 0x003B,
    PS_FILE_ID_DHCPV4S_CFG_C                    = TTFCOMM_TEAM_FILE_ID + 0x003C,
    PS_FILE_ID_DHCPV4S_DBGCMD_C                 = TTFCOMM_TEAM_FILE_ID + 0x003D,
    PS_FILE_ID_DHCPV4S_DEC_C                    = TTFCOMM_TEAM_FILE_ID + 0x003E,
    PS_FILE_ID_DHCPV4S_PUB_C                    = TTFCOMM_TEAM_FILE_ID + 0x003F,
    PS_FILE_ID_DHCPV4S_TIMER_C                  = TTFCOMM_TEAM_FILE_ID + 0x0040,
    PS_FILE_ID_DHCPV6S_COMP_C                   = TTFCOMM_TEAM_FILE_ID + 0x0041,
    PS_FILE_ID_DHCP_ADAPTER_C                   = TTFCOMM_TEAM_FILE_ID + 0x0042,

    PS_FILE_ID_HDLC_C                           = TTFCOMM_TEAM_FILE_ID + 0x0043,
    PS_FILE_ID_IPCP_C                           = TTFCOMM_TEAM_FILE_ID + 0x0044,
    PS_FILE_ID_KASUMI_C                         = TTFCOMM_TEAM_FILE_ID + 0x0045,
    PS_FILE_ID_LCP_C                            = TTFCOMM_TEAM_FILE_ID + 0x0046,
    PS_FILE_ID_LINK_C                           = TTFCOMM_TEAM_FILE_ID + 0x0047,
    PS_FILE_ID_PAP_C                            = TTFCOMM_TEAM_FILE_ID + 0x0048,
    PS_FILE_ID_PDCP_STUB_C                      = TTFCOMM_TEAM_FILE_ID + 0x0049,
    PS_FILE_ID_PPP_ATCMD_C                      = TTFCOMM_TEAM_FILE_ID + 0x004A,
    PS_FILE_ID_PPP_FSM_C                        = TTFCOMM_TEAM_FILE_ID + 0x004B,
    PS_FILE_ID_PPP_INIT_C                       = TTFCOMM_TEAM_FILE_ID + 0x004C,
    PS_FILE_ID_PPP_INPUT_C                      = TTFCOMM_TEAM_FILE_ID + 0x004D,
    PS_FILE_ID_PPP_MBUF_C                       = TTFCOMM_TEAM_FILE_ID + 0x004E,
    PS_FILE_ID_CHAP_C                           = TTFCOMM_TEAM_FILE_ID + 0x004F,
    PS_FILE_ID_PPPID_C                          = TTFCOMM_TEAM_FILE_ID + 0x0050,
    PS_FILE_ID_PPPOE_DISCOVERY_C                = TTFCOMM_TEAM_FILE_ID + 0x0051,
    PS_FILE_ID_PPPOE_ETHERNETIO_C               = TTFCOMM_TEAM_FILE_ID + 0x0052,
    PS_FILE_ID_PPPOE_MANAGER_C                  = TTFCOMM_TEAM_FILE_ID + 0x0053,
    PS_FILE_ID_PPPOE_SESSION_C                  = TTFCOMM_TEAM_FILE_ID + 0x0054,
    PS_FILE_ID_PPPOE_STUBFORWIN_C               = TTFCOMM_TEAM_FILE_ID + 0x0055,
    PS_FILE_ID_PROTO_C                          = TTFCOMM_TEAM_FILE_ID + 0x0056,
    PS_FILE_ID_PS_MEM_C                         = TTFCOMM_TEAM_FILE_ID + 0x0057,
    PS_FILE_ID_PS_MEMCHK_C                      = TTFCOMM_TEAM_FILE_ID + 0x0058,
    PS_FILE_ID_PS_BBPSTUB_C                     = TTFCOMM_TEAM_FILE_ID + 0x0059,
    PS_FILE_ID_PS_BSPSTUB_C                     = TTFCOMM_TEAM_FILE_ID + 0x005A,
    PS_FILE_ID_PS_CFG_C                         = TTFCOMM_TEAM_FILE_ID + 0x005B,
    PS_FILE_ID_PS_GTFHELP_C                     = TTFCOMM_TEAM_FILE_ID + 0x005C,
    PS_FILE_ID_HDLC_HARDWARE_SERVICE_C          = TTFCOMM_TEAM_FILE_ID + 0x005D,
    PS_FILE_ID_HDLC_SOFTWARE_C                  = TTFCOMM_TEAM_FILE_ID + 0x005E,
    PS_FILE_ID_PS_TRACE_LOG_C                   = TTFCOMM_TEAM_FILE_ID + 0x005F,
    PS_FILE_ID_TTF_TRACE_COMM_C                 = TTFCOMM_TEAM_FILE_ID + 0x0060,
    PS_FILE_ID_TTF_IP_COMM_C                    = TTFCOMM_TEAM_FILE_ID + 0x0061,
    PS_FILE_ID_RRM_MGM_C                        = TTFCOMM_TEAM_FILE_ID + 0x0062,
    PS_FILE_ID_RRM_INIT_C                       = TTFCOMM_TEAM_FILE_ID + 0x0063,
    PS_FILE_ID_RRM_CTRL_C                       = TTFCOMM_TEAM_FILE_ID + 0x0064,
    PS_FILE_ID_RRM_TIMER_C                      = TTFCOMM_TEAM_FILE_ID + 0x0065,
    PS_FILE_ID_RRM_OMMNTN_C                     = TTFCOMM_TEAM_FILE_ID + 0x0066,
    PS_FILE_ID_RRM_LOG_C                        = TTFCOMM_TEAM_FILE_ID + 0x0067,
    PS_FILE_ID_RRM_DEBUG_C                      = TTFCOMM_TEAM_FILE_ID + 0x0068,
    PS_FILE_ID_RRM_API_C                        = TTFCOMM_TEAM_FILE_ID + 0x0069,
    PS_FILE_ID_RRM_COMM_C                       = TTFCOMM_TEAM_FILE_ID + 0x006A,
    PS_FILE_ID_RRM_DSMGM_C                      = TTFCOMM_TEAM_FILE_ID + 0x006B,
    PS_FILE_ID_TTF_ERR_LOG_C                    = TTFCOMM_TEAM_FILE_ID + 0x006C,
    PS_FILE_ID_CPU_UTILIZATION_C                = TTFCOMM_TEAM_FILE_ID + 0x006D,
    PS_FILE_ID_CPU_VIEW_C                       = TTFCOMM_TEAM_FILE_ID + 0x006E,
    PS_FILE_ID_TTF_LINK_C                       = TTFCOMM_TEAM_FILE_ID + 0x006F,
    PS_FILE_ID_TTF_MEM_C                        = TTFCOMM_TEAM_FILE_ID + 0x0070,
    PS_FILE_ID_TTF_MEMCHK_C                     = TTFCOMM_TEAM_FILE_ID + 0x0071,
    PS_FILE_ID_TTF_MSGCHK_C                     = TTFCOMM_TEAM_FILE_ID + 0x0072,
    PS_FILE_ID_TTF_TRACE_C                      = TTFCOMM_TEAM_FILE_ID + 0x0073,
    PS_FILE_ID_TTF_UTIL_C                       = TTFCOMM_TEAM_FILE_ID + 0x0074,
    PS_FILE_ID_TTFCICOM_C                       = TTFCOMM_TEAM_FILE_ID + 0x0075,

    PS_FILE_ID_ACPU_VIEW_C                      = TTFCOMM_TEAM_FILE_ID + 0x0076,

    PS_FILE_ID_TTF_NODE_MEM_C                   = TTFCOMM_TEAM_FILE_ID + 0x0077,

    PS_FILE_ID_HDLC_HARDWARE_DRIVER_C           = TTFCOMM_TEAM_FILE_ID + 0x0078,
    PS_FILE_ID_HDLC_HARDWARE_MNTN_C             = TTFCOMM_TEAM_FILE_ID + 0x0079,
    PS_FILE_ID_HDLC_HARDWARE_CONF_C             = TTFCOMM_TEAM_FILE_ID + 0x007A,

    PS_FILE_ID_MIP_UTILITY_C                    = TTFCOMM_TEAM_FILE_ID + 0x007B,
    PS_FILE_ID_MIP_DIAG_C                       = TTFCOMM_TEAM_FILE_ID + 0x007C,
    PS_FILE_ID_MIP_ENTITY_C                     = TTFCOMM_TEAM_FILE_ID + 0x007D,
    PS_FILE_ID_MIP_AUTH_C                       = TTFCOMM_TEAM_FILE_ID + 0x007E,
    PS_FILE_ID_MIP_OUT_MSG_C                    = TTFCOMM_TEAM_FILE_ID + 0x007F,
    PS_FILE_ID_MIP_IN_MSG_C                     = TTFCOMM_TEAM_FILE_ID + 0x0080,
    PS_FILE_ID_MIP_REG_SM_C                     = TTFCOMM_TEAM_FILE_ID + 0x0081,
    PS_FILE_ID_MIP_SOL_SM_C                     = TTFCOMM_TEAM_FILE_ID + 0x0082,
    PS_FILE_ID_MIP_CTRL_C                       = TTFCOMM_TEAM_FILE_ID + 0x0083,
    PS_FILE_ID_PPPC_VJ_C                        = TTFCOMM_TEAM_FILE_ID + 0x0084,

    PS_FILE_ID_NODE_MEM_ALGORITHM_C             = TTFCOMM_TEAM_FILE_ID + 0x0085,
    PS_FILE_ID_NODE_MEM_MANAGER_C               = TTFCOMM_TEAM_FILE_ID + 0x0086,
    PS_FILE_ID_EFFICIENT_NODE_MEM_MANAGER_C     = TTFCOMM_TEAM_FILE_ID + 0x0087,
    PS_FILE_ID_STABLE_NODE_MEM_MANAGER_C        = TTFCOMM_TEAM_FILE_ID + 0x0088,
    PS_FILE_ID_RRM_NODE_MEM_C                   = TTFCOMM_TEAM_FILE_ID + 0x0089,
    PS_FILE_ID_NODE_MEM_MNTN_C                  = TTFCOMM_TEAM_FILE_ID + 0x008A,
    PS_FILE_ID_RRM_DSDS3_C                      = TTFCOMM_TEAM_FILE_ID + 0x008B,

    PS_FILE_ID_RRM_RFSTATUS_REG_C               = TTFCOMM_TEAM_FILE_ID + 0x008D,
    PS_FILE_ID_RRM_DSDS_STATUS_IND_C            = TTFCOMM_TEAM_FILE_ID + 0x008E,
    PS_FILE_ID_RRM_MODEM_STATUS_IND_C           = TTFCOMM_TEAM_FILE_ID + 0x008F,
    PS_FILE_ID_RRM_RES_AVAILABLE_IND_C          = TTFCOMM_TEAM_FILE_ID + 0x0090,

    PS_FILE_ID_TTF_UNIQUE_LINK_C                = TTFCOMM_TEAM_FILE_ID + 0x0091,

    PS_FILE_ID_TTF_COPY_LINK_C                  = TTFCOMM_TEAM_FILE_ID + 0x0092,

    PS_FILE_ID_CTTF_MEM_C                       = TTFCOMM_TEAM_FILE_ID + 0x0093,
    PS_FILE_ID_PS_TRACE_MSG_C                   = TTFCOMM_TEAM_FILE_ID + 0x0094,
    PS_FILE_ID_TTF_SHARED_QUEUE_C               = TTFCOMM_TEAM_FILE_ID + 0x0095,
    PS_FILE_ID_PPPA_TASK_C                      = TTFCOMM_TEAM_FILE_ID + 0x0096,
    PS_FILE_ID_PPP_DECODE_C                     = TTFCOMM_TEAM_FILE_ID + 0x0097,
    PS_FILE_ID_PPPS_SHEL_C                      = TTFCOMM_TEAM_FILE_ID + 0x0100,
    PS_FILE_ID_PPPS_ENTITY_C                    = TTFCOMM_TEAM_FILE_ID + 0x0101,
    PS_FILE_ID_PPPS_CORE_C                      = TTFCOMM_TEAM_FILE_ID + 0x0102,
    PS_FILE_ID_PPPS_CHAP_C                      = TTFCOMM_TEAM_FILE_ID + 0x0103,
    PS_FILE_ID_PPPS_PAP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0104,
    PS_FILE_ID_PPPS_CAP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0105,
    PS_FILE_ID_PPPS_CTRL_C                      = TTFCOMM_TEAM_FILE_ID + 0x0106,
    PS_FILE_ID_PPPS_LCP_C                       = TTFCOMM_TEAM_FILE_ID + 0x0107,
    PS_FILE_ID_PPPS_FSM_C                       = TTFCOMM_TEAM_FILE_ID + 0x0108,
    PS_FILE_ID_PPPS_IPCP_C                      = TTFCOMM_TEAM_FILE_ID + 0x0109,
    PS_FILE_ID_PPPS_UTILITY_C                   = TTFCOMM_TEAM_FILE_ID + 0x010A,
    PS_FILE_ID_PPPS_PPPA_PROC_C                 = TTFCOMM_TEAM_FILE_ID + 0x010B,
    PS_FILE_ID_PPPS_TASK_C                      = TTFCOMM_TEAM_FILE_ID + 0x010C,
    PS_FILE_ID_RRM_INTERFERE_BAND_QUERYER_C     = TTFCOMM_TEAM_FILE_ID + 0x010D,
    PS_FILE_ID_RRM_INTERFERE_BAND_NV_LOADER_C          = TTFCOMM_TEAM_FILE_ID + 0x010E,
    PS_FILE_ID_RRM_INTERFERE_BAND_NV_LOADER_FUSION_C   = TTFCOMM_TEAM_FILE_ID + 0x010F,
    PS_FILE_ID_RRM_DUAL_CARD_STATE_INDICATOR_C  = TTFCOMM_TEAM_FILE_ID + 0x0110,
    PS_FILE_ID_RRM_VOICE_STATE_INDICATOR_C      = TTFCOMM_TEAM_FILE_ID + 0x0111,
    PS_FILE_ID_RRM_TASK_MANAGER_C               = TTFCOMM_TEAM_FILE_ID + 0x0112,
    PS_FILE_ID_RRM_EVENT_SERVICE_C              = TTFCOMM_TEAM_FILE_ID + 0x0113,
    PS_FILE_ID_RRM_SA_RF_BAND_CONFLICT_JUDGER_C = TTFCOMM_TEAM_FILE_ID + 0x0114,
    PS_FILE_ID_PPPS_IPV6CP_C                    = TTFCOMM_TEAM_FILE_ID + 0x0115,
    PS_FILE_ID_PPPS_MNTN_C                      = TTFCOMM_TEAM_FILE_ID + 0x0116,

    PS_FILE_ID_TTFCOMM_BUTT
}TTFCOMM_FILE_ID_DEFINE_ENUM;
typedef unsigned long  TTFCOMM_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


