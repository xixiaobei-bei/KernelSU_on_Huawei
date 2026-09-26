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

#ifndef PS_LOG_CAS_FILE_ID_DEFINE_H
#define PS_LOG_CAS_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    /* for CDMA 1X, 2013-9-3, start */
    PS_FILE_ID_CAS_1X_IDLECOMMCHCFG_C = CAS_TEAM_FILE_ID,
    PS_FILE_ID_CAS_1X_IDLEFSM_C             = CAS_TEAM_FILE_ID + 0x0001,
    PS_FILE_ID_CAS_1X_IDLEFLOW_C            = CAS_TEAM_FILE_ID + 0x0002,
    PS_FILE_ID_CAS_1X_IDLEHANDOFF_C         = CAS_TEAM_FILE_ID + 0x0003,
    PS_FILE_ID_CAS_1X_MAIN_C                = CAS_TEAM_FILE_ID + 0x0004,
    PS_FILE_ID_CAS_1X_MAINFSM_C             = CAS_TEAM_FILE_ID + 0x0005,
    PS_FILE_ID_CAS_1XCASM_ENTRY_C           = CAS_TEAM_FILE_ID + 0x0006,
    PS_FILE_ID_CAS_1XCMEAS_ENTRY_C          = CAS_TEAM_FILE_ID + 0x0007,
    PS_FILE_ID_CAS_1X_INITFSM_C             = CAS_TEAM_FILE_ID + 0x0008,
    PS_FILE_ID_CAS_1X_PRINTFSM_C            = CAS_TEAM_FILE_ID + 0x0009,
    PS_FILE_ID_CAS_1X_MSGCOMM_C             = CAS_TEAM_FILE_ID + 0x000a,
    PS_FILE_ID_CAS_1X_NASITF_C              = CAS_TEAM_FILE_ID + 0x000b,
    PS_FILE_ID_CAS_1X_INITENTRY_C           = CAS_TEAM_FILE_ID + 0x000c,
    PS_FILE_ID_CAS_1X_INITFLOW_C            = CAS_TEAM_FILE_ID + 0x000d,
    PS_FILE_ID_CAS_1X_INITRELALL_C          = CAS_TEAM_FILE_ID + 0x000e,
    PS_FILE_ID_CAS_1X_INITUIM_C             = CAS_TEAM_FILE_ID + 0x000f,
    PS_FILE_ID_CAS_1X_CSRCHENTRY_C          = CAS_TEAM_FILE_ID + 0x0010,
    PS_FILE_ID_CAS_1X_MNTN_C                = CAS_TEAM_FILE_ID + 0x0011,
    PS_FILE_ID_CAS_1X_MAINCCB_C             = CAS_TEAM_FILE_ID + 0x0012,
    PS_FILE_ID_CAS_1X_TIMER_C               = CAS_TEAM_FILE_ID + 0x0013,
    PS_FILE_ID_CAS_1X_MAINOTA_C             = CAS_TEAM_FILE_ID + 0x0014,
    PS_FILE_ID_CAS_1X_FSMCOMM_C             = CAS_TEAM_FILE_ID + 0x0015,
    PS_FILE_ID_CAS_1X_PHYITFCOMM_C          = CAS_TEAM_FILE_ID + 0x0016,
    PS_FILE_ID_CAS_1X_PHYITFCHCFG_C         = CAS_TEAM_FILE_ID + 0x0017,
    PS_FILE_ID_CAS_1X_PHYITFFSM_C           = CAS_TEAM_FILE_ID + 0x0018,
    PS_FILE_ID_CAS_1X_PHYITFSRCHCFG_C       = CAS_TEAM_FILE_ID + 0x0019,
    PS_FILE_ID_CAS_1X_SRCHCOMPROC_C         = CAS_TEAM_FILE_ID + 0x001a,
    PS_FILE_ID_CAS_1X_SRCHFSM_C             = CAS_TEAM_FILE_ID + 0x001b,
    PS_FILE_ID_CAS_1X_SRCHMSG_C             = CAS_TEAM_FILE_ID + 0x001c,
    PS_FILE_ID_CAS_1X_SRCHSYSACQ_C          = CAS_TEAM_FILE_ID + 0x001d,
    PS_FILE_ID_CAS_1X_TTFITFENTRY_C         = CAS_TEAM_FILE_ID + 0x001e,
    PS_FILE_ID_CAS_1X_TTFITFMACCFG_C        = CAS_TEAM_FILE_ID + 0x001f,
    PS_FILE_ID_CAS_1X_TTFITFLACCFG_C        = CAS_TEAM_FILE_ID + 0x0020,
    PS_FILE_ID_CAS_1X_TTFITFRLPCFG_C        = CAS_TEAM_FILE_ID + 0x0021,
    PS_FILE_ID_CAS_1X_TTFITFRRMITF_C        = CAS_TEAM_FILE_ID + 0x0022,
    PS_FILE_ID_CAS_MNTN_C                   = CAS_TEAM_FILE_ID + 0x0023,
    PS_FILE_ID_CAS_FSM_C                    = CAS_TEAM_FILE_ID + 0x0024,
    PS_FILE_ID_CAS_PRINTFSM_C               = CAS_TEAM_FILE_ID + 0x0025,
    PS_FILE_ID_CAS_FSMCOMM_C                = CAS_TEAM_FILE_ID + 0x0026,
    PS_FILE_ID_CAS_1X_OHMUPDATE_C           = CAS_TEAM_FILE_ID + 0x0027,
    PS_FILE_ID_CAS_1X_OHMSAVEIE_C           = CAS_TEAM_FILE_ID + 0x0028,
    PS_FILE_ID_CAS_1X_COMMFUN_C             = CAS_TEAM_FILE_ID + 0x0029,
    PS_FILE_ID_CAS_1X_COMMSAVEIE_C          = CAS_TEAM_FILE_ID + 0x002a,
    PS_FILE_ID_CAS_1X_CCHGLOBAL_C           = CAS_TEAM_FILE_ID + 0x002b,
    PS_FILE_ID_CAS_1X_CCHCOMM_C             = CAS_TEAM_FILE_ID + 0x002c,
    PS_FILE_ID_CAS_1X_CCHMSG_C              = CAS_TEAM_FILE_ID + 0x002d,
    PS_FILE_ID_CAS_1X_ACSFLOW_C             = CAS_TEAM_FILE_ID + 0x002e,
    PS_FILE_ID_CAS_1X_AUTHENTICATION_C      = CAS_TEAM_FILE_ID + 0x002f,
    PS_FILE_ID_CAS_1X_SSDUPDATE_C           = CAS_TEAM_FILE_ID + 0x0030,
    PS_FILE_ID_CAS_1X_ACSFSM_C              = CAS_TEAM_FILE_ID + 0x0031,
    PS_FILE_ID_CAS_1X_ACSHANDOFF_C          = CAS_TEAM_FILE_ID + 0x0032,
    PS_FILE_ID_CAS_1X_ACSREGFLOW_C          = CAS_TEAM_FILE_ID + 0x0033,
    PS_FILE_ID_CAS_1X_CCHLOST_C             = CAS_TEAM_FILE_ID + 0x0034,
    PS_FILE_ID_CAS_1X_ACSHOFLOW_C           = CAS_TEAM_FILE_ID + 0x0035,
    PS_FILE_ID_CAS_1X_MEASCFG_C             = CAS_TEAM_FILE_ID + 0x0036,
    PS_FILE_ID_CAS_1X_MEASFSM_C             = CAS_TEAM_FILE_ID + 0x0037,
    PS_FILE_ID_CAS_1X_MEASMSG_C             = CAS_TEAM_FILE_ID + 0x0038,
    PS_FILE_ID_CAS_1X_MEASCOMPROC_C         = CAS_TEAM_FILE_ID + 0x0039,
    PS_FILE_ID_CAS_1X_MEASSAVEIE_C          = CAS_TEAM_FILE_ID + 0x003a,
    PS_FILE_ID_CAS_1X_PHYITFMEASCFG_C       = CAS_TEAM_FILE_ID + 0x003b,
    PS_FILE_ID_CAS_1X_TCHFLOW_C             = CAS_TEAM_FILE_ID + 0x003c,
    PS_FILE_ID_CAS_1X_TCHFSM_C              = CAS_TEAM_FILE_ID + 0x003d,
    PS_FILE_ID_CAS_1X_TCHENTRY_C            = CAS_TEAM_FILE_ID + 0x003e,
    PS_FILE_ID_CAS_1X_TCHSNDMSG_C           = CAS_TEAM_FILE_ID + 0x003f,
    PS_FILE_ID_CAS_1X_TCHCOMM_C             = CAS_TEAM_FILE_ID + 0x0040,
    PS_FILE_ID_CAS_1X_TCHCTRLSERVICE_C      = CAS_TEAM_FILE_ID + 0x0041,
    PS_FILE_ID_CAS_1X_TCHRELEASE_C          = CAS_TEAM_FILE_ID + 0x0042,
    PS_FILE_ID_CAS_1X_TCHPWRCTRL_C          = CAS_TEAM_FILE_ID + 0x0043,
    PS_FILE_ID_CAS_1X_TCHHARDHANDOFF_C      = CAS_TEAM_FILE_ID + 0x0044,
    PS_FILE_ID_CAS_1X_TCHSOFTHANDOFF_C      = CAS_TEAM_FILE_ID + 0x0045,
    PS_FILE_ID_CAS_1X_TCHSAVEIE_C           = CAS_TEAM_FILE_ID + 0x0046,
    PS_FILE_ID_CAS_1X_TCHPARA_C             = CAS_TEAM_FILE_ID + 0x0047,
    PS_FILE_ID_CAS_1X_TCHPWRUPFUNC_C        = CAS_TEAM_FILE_ID + 0x0048,
    PS_FILE_ID_CAS_1X_PAGE_RESPONSE_C       = CAS_TEAM_FILE_ID + 0x0049,
    PS_FILE_ID_CAS_1X_ACSORIGATTEMPT_C      = CAS_TEAM_FILE_ID + 0x004a,
    PS_FILE_ID_CAS_1X_CAPABILITY_C          = CAS_TEAM_FILE_ID + 0x004b,
    PS_FILE_ID_CAS_1X_MSG_TRANSMIT_C        = CAS_TEAM_FILE_ID + 0x004c,
    PS_FILE_ID_CAS_1X_ORD_OR_MSG_RSP_C      = CAS_TEAM_FILE_ID + 0x004d,
    PS_FILE_ID_CAS_1X_SLOTTEDMODE_C         = CAS_TEAM_FILE_ID + 0x004e,
    PS_FILE_ID_CAS_1X_SRV_RED_C             = CAS_TEAM_FILE_ID + 0x004f,
    PS_FILE_ID_CAS_1X_SRDFSM_C              = CAS_TEAM_FILE_ID + 0x0050,
    PS_FILE_ID_CAS_1X_MEASMNTN_C            = CAS_TEAM_FILE_ID + 0x0051,
    PS_FILE_ID_CAS_1X_TESTDATASO_C          = CAS_TEAM_FILE_ID + 0x0052,
    PS_FILE_ID_CAS_1X_HRPDITF_C             = CAS_TEAM_FILE_ID + 0x0053,
    PS_FILE_ID_CAS_1X_USIMITFCFG_C          = CAS_TEAM_FILE_ID + 0x0054,

    PS_FILE_ID_CAS_1X_CAVE_C                = CAS_TEAM_FILE_ID + 0x0055,

    /* for CDMA 1X, 2013-9-3, end */

    /* CAS, for CDMA HRPD, 2013-9-3, begin */
    PS_FILE_ID_CAS_HRPD_MSGCOMM_C                       = CAS_TEAM_FILE_ID + 0x0100,
    PS_FILE_ID_CAS_HRPD_TIMER_C                         = CAS_TEAM_FILE_ID + 0x0101,
    PS_FILE_ID_CAS_HRPD_MNTN_C                          = CAS_TEAM_FILE_ID + 0x0102,
    PS_FILE_ID_CAS_HSCP_ENTRY_C                         = CAS_TEAM_FILE_ID + 0x0103,
    PS_FILE_ID_CAS_TIMER_C                              = CAS_TEAM_FILE_ID + 0x0104,
    PS_FILE_ID_CAS_HRPDFSM_C                            = CAS_TEAM_FILE_ID + 0x0105,
    PS_FILE_ID_CAS_HRPDHALMP_ENTRY_C                    = CAS_TEAM_FILE_ID + 0x0106,
    PS_FILE_ID_CAS_HRPDHALMP_FLOW_C                     = CAS_TEAM_FILE_ID + 0x0107,
    PS_FILE_ID_CAS_HRPD_HALMP_FLOW_EUTRAN_C             = CAS_TEAM_FILE_ID + 0x0108,
    PS_FILE_ID_CAS_HRPDHALMP_RESELTOEUTRAN_C            = CAS_TEAM_FILE_ID + 0x0109,
    PS_FILE_ID_CAS_HRPD_HALMP_EUTRAN_RESEL_TO_HRPD_C    = CAS_TEAM_FILE_ID + 0x010a,
    PS_FILE_ID_CAS_HRPDHALMP_L2CBSR_C                   = CAS_TEAM_FILE_ID + 0x010b,
    PS_FILE_ID_CAS_HSP_ENTRY_C                          = CAS_TEAM_FILE_ID + 0x010c,
    PS_FILE_ID_CAS_HRPD_MAIN_C                          = CAS_TEAM_FILE_ID + 0x010d,
    PS_FILE_ID_CAS_HRPD_MAINCCB_C                       = CAS_TEAM_FILE_ID + 0x010e,
    PS_FILE_ID_CAS_HRPD_HALMP_SYS_SYNC_C                = CAS_TEAM_FILE_ID + 0x010f,
    PS_FILE_ID_CAS_HRPD_INIT_ENTRY_C                    = CAS_TEAM_FILE_ID + 0x0110,
    PS_FILE_ID_CAS_HRPD_INIT_PILOTSEARCH_C              = CAS_TEAM_FILE_ID + 0x0111,
    PS_FILE_ID_CAS_HRPDHALMP_FSM_C                      = CAS_TEAM_FILE_ID + 0x0112,
    PS_FILE_ID_CAS_HRPDINIT_FSM_C                       = CAS_TEAM_FILE_ID + 0x0113,
    PS_FILE_ID_CAS_HRPDHRUP_ENTRY_C                     = CAS_TEAM_FILE_ID + 0x0114,
    PS_FILE_ID_CAS_HRPD_NASITF_C                        = CAS_TEAM_FILE_ID + 0x0115,
    PS_FILE_ID_CAS_HRPD_PHYITFCHCFG_C                   = CAS_TEAM_FILE_ID + 0x0116,
    PS_FILE_ID_CAS_INTRA_MSG_COMM_C                     = CAS_TEAM_FILE_ID + 0x0117,
    PS_FILE_ID_CAS_HRPD_OHMUPDATE_C                     = CAS_TEAM_FILE_ID + 0x0118,
    PS_FILE_ID_CAS_HRPD_MAINFSM_C                       = CAS_TEAM_FILE_ID + 0x0119,
    PS_FILE_ID_CAS_HRPD_TTFITF_C                        = CAS_TEAM_FILE_ID + 0x011a,
    PS_FILE_ID_CAS_HRPD_TTFITFFSM_C                     = CAS_TEAM_FILE_ID + 0x011b,
    PS_FILE_ID_CAS_HRPDIDLE_FLOW_C                      = CAS_TEAM_FILE_ID + 0x011c,
    PS_FILE_ID_CAS_HRPDIDLE_FSM_C                       = CAS_TEAM_FILE_ID + 0x011d,
    PS_FILE_ID_CAS_HRPD_HSCP_MSG_C                      = CAS_TEAM_FILE_ID + 0x011e,
    PS_FILE_ID_CAS_HRPD_HSCP_FLOW_C                     = CAS_TEAM_FILE_ID + 0x011f,
    PS_FILE_ID_CAS_HRPD_HSCP_AN_NEG_C                   = CAS_TEAM_FILE_ID + 0x0120,
    PS_FILE_ID_CAS_HRPD_HSCP_AT_NEG_C                   = CAS_TEAM_FILE_ID + 0x0121,
    PS_FILE_ID_CAS_HRPD_HSCP_COMMONPROC_C               = CAS_TEAM_FILE_ID + 0x0122,
    PS_FILE_ID_CAS_COMMFUNC_C                           = CAS_TEAM_FILE_ID + 0x0123,
    PS_FILE_ID_CAS_HRPD_HSCP_FSM_C                      = CAS_TEAM_FILE_ID + 0x0124,
    PS_FILE_ID_CAS_HRPDCONN_FLOW_C                      = CAS_TEAM_FILE_ID + 0x0125,
    PS_FILE_ID_CAS_HRPDCONN_FSM_C                       = CAS_TEAM_FILE_ID + 0x0126,
    PS_FILE_ID_CAS_HRPDHALMP_CONNSETUP_C                = CAS_TEAM_FILE_ID + 0x0127,
    PS_FILE_ID_CAS_HRPDHALMP_CONNSETUP_SEND_MSG_C       = CAS_TEAM_FILE_ID + 0x0128,
    PS_FILE_ID_CAS_HRPDRUP_ROUTE_UPDATE_C               = CAS_TEAM_FILE_ID + 0x0129,
    PS_FILE_ID_CAS_HRPDRUP_MEAS_C                       = CAS_TEAM_FILE_ID + 0x012a,
    PS_FILE_ID_CAS_HRPD_RUP_SLAVE_MEAS_C                = CAS_TEAM_FILE_ID + 0x012b,
    PS_FILE_ID_CAS_HRPDRUP_FLOW_C                       = CAS_TEAM_FILE_ID + 0x012c,
    PS_FILE_ID_CAS_HRPDRUP_FSM_C                        = CAS_TEAM_FILE_ID + 0x012d,
    PS_FILE_ID_CAS_HRPD_RUPMNTN_C                       = CAS_TEAM_FILE_ID + 0x012e,
    PS_FILE_ID_CAS_HRPD_HSCP_APPLICATION_C              = CAS_TEAM_FILE_ID + 0x012f,
    PS_FILE_ID_CAS_HRPD_HSCP_SESSION_C                  = CAS_TEAM_FILE_ID + 0x0130,
    PS_FILE_ID_CAS_HRPD_HSCP_CONNECTION_NEG_C           = CAS_TEAM_FILE_ID + 0x0131,
    PS_FILE_ID_CAS_HRPDHSCP_MAC_NEG_C                   = CAS_TEAM_FILE_ID + 0x0132,
    PS_FILE_ID_CAS_HRPDHSCP_STREAM_NEG_C                = CAS_TEAM_FILE_ID + 0x0133,
    PS_FILE_ID_CAS_HRPDHSCP_SECURITY_NEG_C              = CAS_TEAM_FILE_ID + 0x0134,
    PS_FILE_ID_CAS_HRPD_HSCP_KEYEXCHANGE_C              = CAS_TEAM_FILE_ID + 0x0135,
    PS_FILE_ID_CAS_HRPD_LTEITF_C                        = CAS_TEAM_FILE_ID + 0x0136,
    PS_FILE_ID_CAS_HRPDRUP_MEAS_EUTRAN_C                = CAS_TEAM_FILE_ID + 0x0137,
    PS_FILE_ID_CAS_HRPD_OHMUPDATEEUTRAN_C               = CAS_TEAM_FILE_ID + 0x0138,
    PS_FILE_ID_CAS_HRPD_HRPDRRM_C                       = CAS_TEAM_FILE_ID + 0x0139,
    PS_FILE_ID_CAS_HRPD_INIT_BSR_EUTRAN_C               = CAS_TEAM_FILE_ID + 0x013a,
    PS_FILE_ID_CAS_HRPD_HALMP_L2C_DEDIRECT_C            = CAS_TEAM_FILE_ID + 0x013b,
    PS_FILE_ID_CAS_HRPD_HALMP_MNTN_C                    = CAS_TEAM_FILE_ID + 0x013c,
    PS_FILE_ID_CAS_HRPD_INIT_MNTN_C                     = CAS_TEAM_FILE_ID + 0x013d,
    PS_FILE_ID_CAS_HRPDHSP_FSM_C                        = CAS_TEAM_FILE_ID + 0x013e,
    /* CAS, for CDMA HRPD, 2013-9-3, end */

    CAS_FILE_ID_BUTT                                    = 0x0fff
}CAS_FILE_ID_DEFINE_ENUM;
typedef unsigned long  CAS_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


