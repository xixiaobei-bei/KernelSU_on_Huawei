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

#ifndef PS_LOG_CTTF_FILE_ID_DEFINE_H
#define PS_LOG_CTTF_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* CTTF */
typedef enum
{
    /* CDMA L2 , 2013-9-4 start*/
    PS_FILE_ID_CTTF_1X_FLAC_CTRL_C              = CTTF_TEAM_FILE_ID,            /* CTTF_TEAM_FILE_ID = 0x1c00 */
    PS_FILE_ID_CTTF_1X_FLAC_CFG_C               = CTTF_TEAM_FILE_ID + 0x0001,
    PS_FILE_ID_CTTF_1X_FLAC_CSCH_C              = CTTF_TEAM_FILE_ID + 0x0002,
    PS_FILE_ID_CTTF_1X_FLAC_DSCH_C              = CTTF_TEAM_FILE_ID + 0x0003,
    PS_FILE_ID_CTTF_1X_FLAC_PID_ENTRY_C         = CTTF_TEAM_FILE_ID + 0x0004,
    PS_FILE_ID_CTTF_1X_FMAC_CTRL_C              = CTTF_TEAM_FILE_ID + 0x0005,
    PS_FILE_ID_CTTF_1X_FMAC_DEMUX_ENT_PROC_C    = CTTF_TEAM_FILE_ID + 0x0006,
    PS_FILE_ID_CTTF_1X_FMAC_DEMUX_DATA_PROC_C   = CTTF_TEAM_FILE_ID + 0x0007,
    PS_FILE_ID_CTTF_1X_FRLP_CTRL_C              = CTTF_TEAM_FILE_ID + 0x0008,
    PS_FILE_ID_CTTF_1X_FRLP_DATA_BUF_C          = CTTF_TEAM_FILE_ID + 0x0009,
    PS_FILE_ID_CTTF_1X_FRLP_TYPE1_RX_C          = CTTF_TEAM_FILE_ID + 0x000A,
    PS_FILE_ID_CTTF_1X_FRLP_TYPE3_RX_C          = CTTF_TEAM_FILE_ID + 0x000B,
    PS_FILE_ID_CTTF_1X_LAC_MNTN_C               = CTTF_TEAM_FILE_ID + 0x000C,
    PS_FILE_ID_CTTF_1X_RLAC_SEND_MSG_C          = CTTF_TEAM_FILE_ID + 0x000D,
    PS_FILE_ID_CTTF_1X_FLAC_SEND_MSG_C          = CTTF_TEAM_FILE_ID + 0x000E,
    PS_FILE_ID_CTTF_1X_FLAC_ADDR_MATCH_C        = CTTF_TEAM_FILE_ID + 0x000F,
    PS_FILE_ID_CTTF_1X_RLOOPBACK_C              = CTTF_TEAM_FILE_ID + 0x0010,
    PS_FILE_ID_CTTF_1X_FLOOPBACK_C              = CTTF_TEAM_FILE_ID + 0x0011,
    PS_FILE_ID_CTTF_1X_VOICEAGENT_C             = CTTF_TEAM_FILE_ID + 0x0012,
    PS_FILE_ID_CTTF_1X_MAC_MNTN_C               = CTTF_TEAM_FILE_ID + 0x0013,
    PS_FILE_ID_CTTF_1X_RMARKOV_C                = CTTF_TEAM_FILE_ID + 0x0014,
    PS_FILE_ID_CTTF_1X_FMARKOV_C                = CTTF_TEAM_FILE_ID + 0x0015,
    PS_FILE_ID_CTTF_1X_MARKOV_UTILITY_C         = CTTF_TEAM_FILE_ID + 0x0016,
    PS_FILE_ID_CTTF_1X_RTDSO_C                  = CTTF_TEAM_FILE_ID + 0x0017,
    PS_FILE_ID_CTTF_1X_FTDSO_C                  = CTTF_TEAM_FILE_ID + 0x0018,
    PS_FILE_ID_CTTF_1X_TDSO_UTILITY_C           = CTTF_TEAM_FILE_ID + 0x0019,
    PS_FILE_ID_CTTF_1X_TESTSO_C                 = CTTF_TEAM_FILE_ID + 0x001A,
    PS_FILE_ID_CTTF_1X_RLAC_CTRL_C              = CTTF_TEAM_FILE_ID + 0x001B,
    PS_FILE_ID_CTTF_1X_RLAC_CFG_C               = CTTF_TEAM_FILE_ID + 0x001C,
    PS_FILE_ID_CTTF_1X_RLAC_CSCH_C              = CTTF_TEAM_FILE_ID + 0x001D,
    PS_FILE_ID_CTTF_1X_RLAC_DSCH_C              = CTTF_TEAM_FILE_ID + 0x001E,
    PS_FILE_ID_CTTF_1X_RLAC_PID_ENTRY_C         = CTTF_TEAM_FILE_ID + 0x001F,
    PS_FILE_ID_CTTF_1X_RLAC_ADDR_MATCH_C        = CTTF_TEAM_FILE_ID + 0x0020,
    PS_FILE_ID_CTTF_1X_RLP_MNTN_C               = CTTF_TEAM_FILE_ID + 0x0021,
    PS_FILE_ID_CTTF_1X_RLP_SECURITY_C           = CTTF_TEAM_FILE_ID + 0x0022,
    PS_FILE_ID_CTTF_1X_RLP_SHARE_BUFFER_C       = CTTF_TEAM_FILE_ID + 0x0023,
    PS_FILE_ID_CTTF_1X_RLP_BLOB_C               = CTTF_TEAM_FILE_ID + 0x0024,
    PS_FILE_ID_CTTF_1X_RMAC_CTRL_C              = CTTF_TEAM_FILE_ID + 0x0025,
    PS_FILE_ID_CTTF_1X_RMAC_MUX_ENT_PROC_C      = CTTF_TEAM_FILE_ID + 0x0026,
    PS_FILE_ID_CTTF_1X_RMAC_MUX_DATA_PROC_C     = CTTF_TEAM_FILE_ID + 0x0027,
    PS_FILE_ID_CTTF_1X_RMAC_SRBP_ENT_PROC_C     = CTTF_TEAM_FILE_ID + 0x0028,
    PS_FILE_ID_CTTF_1X_RMAC_SRBP_DATA_PROC_C    = CTTF_TEAM_FILE_ID + 0x0029,
    PS_FILE_ID_CTTF_1X_RRLP_PIDENTRY_C          = CTTF_TEAM_FILE_ID + 0x002A,
    PS_FILE_ID_CTTF_1X_FRLP_PIDENTRY_C          = CTTF_TEAM_FILE_ID + 0x002B,
    PS_FILE_ID_CTTF_1X_FRLP_SEND_RRLP_MSG_C     = CTTF_TEAM_FILE_ID + 0x002C,
    PS_FILE_ID_CTTF_1X_RRLP_SEND_CAS_MSG_C      = CTTF_TEAM_FILE_ID + 0x002D,
    PS_FILE_ID_CTTF_1X_RRLP_SEND_FRLP_MSG_C     = CTTF_TEAM_FILE_ID + 0x002E,
    PS_FILE_ID_CTTF_1X_RRLP_SEND_RRLP_MSG_C     = CTTF_TEAM_FILE_ID + 0x002F,
    PS_FILE_ID_CTTF_1X_RRLP_CTRL_C              = CTTF_TEAM_FILE_ID + 0x0030,
    PS_FILE_ID_CTTF_1X_RRLP_BO_IF_C             = CTTF_TEAM_FILE_ID + 0x0031,
    PS_FILE_ID_CTTF_1X_RRLP_CDS_TX_IF_C         = CTTF_TEAM_FILE_ID + 0x0032,
    PS_FILE_ID_CTTF_1X_RRLP_TX_C                = CTTF_TEAM_FILE_ID + 0x0033,
    PS_FILE_ID_CTTF_1X_RRLP_TYPE1_TX_C          = CTTF_TEAM_FILE_ID + 0x0034,
    PS_FILE_ID_CTTF_1X_RRLP_TYPE3_TX_C          = CTTF_TEAM_FILE_ID + 0x0035,
    PS_FILE_ID_CTTF_RLP_COMM_C                  = CTTF_TEAM_FILE_ID + 0x0036,
    PS_FILE_ID_CRC_C                            = CTTF_TEAM_FILE_ID + 0x0037,
    PS_FILE_ID_SHA1_C                           = CTTF_TEAM_FILE_ID + 0x0038,
    /*PS_FILE_ID_PS_TRACE_LOG_C,*/
    PS_FILE_ID_CTTF_COMM_C                      = CTTF_TEAM_FILE_ID + 0x0039,
    PS_FILE_ID_CTTF_TIMER_C                     = CTTF_TEAM_FILE_ID + 0x003A,
    PS_FILE_ID_CDMA_UTIL_C                      = CTTF_TEAM_FILE_ID + 0x003B,
    PS_FILE_ID_CTTF_CTRL_C                      = CTTF_TEAM_FILE_ID + 0x003C,
    PS_FILE_ID_CTTF_1X_REV_ENTRY_C              = CTTF_TEAM_FILE_ID + 0x003D,
    PS_FILE_ID_CTTF_1X_FWD_ENTRY_C              = CTTF_TEAM_FILE_ID + 0x003E,
    PS_FILE_ID_CTTF_ISR_C                       = CTTF_TEAM_FILE_ID + 0x003F,
    PS_FILE_ID_PS_QNODE_C                       = CTTF_TEAM_FILE_ID + 0x0040,
    PS_FILE_ID_CTTF_LOG_C                       = CTTF_TEAM_FILE_ID + 0x0041,
    PS_FILE_ID_CTTF_MNTN_C                      = CTTF_TEAM_FILE_ID + 0x0042,
    /*PS_FILE_ID_TTF_TRACE_COMM_C,*/
    PS_FILE_ID_CTTF_1X_FMAC_COMMCH_ENT_PROC_C   = CTTF_TEAM_FILE_ID + 0x0043,
    PS_FILE_ID_CTTF_1X_FMAC_COMMCH_DATA_PROC_C  = CTTF_TEAM_FILE_ID + 0x0044,
    PS_FILE_ID_CTTF_1X_MAC_UTILITY_C            = CTTF_TEAM_FILE_ID + 0x0045,
    PS_FILE_ID_CTTF_1X_MAC_CTRL_EXT_C           = CTTF_TEAM_FILE_ID + 0x0046,
    PS_FILE_ID_CTTF_HRPD_CCMAC_DATA_PROC_C      = CTTF_TEAM_FILE_ID + 0x0047,
    PS_FILE_ID_CTTF_HRPD_CCMAC_ENT_PROC_C       = CTTF_TEAM_FILE_ID + 0x0048,
    PS_FILE_ID_CTTF_HRPD_FMAC_CTRL_C            = CTTF_TEAM_FILE_ID + 0x0049,
    PS_FILE_ID_CTTF_HRPD_RMAC_CTRL_C            = CTTF_TEAM_FILE_ID + 0x004A,
    PS_FILE_ID_CTTF_HRPD_MAC_UTILITY_C          = CTTF_TEAM_FILE_ID + 0x004B,
    PS_FILE_ID_CTTF_HRPD_FWD_ENTRY_C            = CTTF_TEAM_FILE_ID + 0x004C,
    PS_FILE_ID_CTTF_HRPD_SIG_ENTRY_C            = CTTF_TEAM_FILE_ID + 0x004D,
    PS_FILE_ID_CTTF_HRPD_REV_ENTRY_C            = CTTF_TEAM_FILE_ID + 0x004E,
    PS_FILE_ID_CTTF_HRPD_RSIG_PID_ENTRY_C       = CTTF_TEAM_FILE_ID + 0x004F,
    PS_FILE_ID_CTTF_HRPD_FSIG_PID_ENTRY_C       = CTTF_TEAM_FILE_ID + 0x0050,
    PS_FILE_ID_CTTF_HRPD_SIG_PID_ENTRY_C        = CTTF_TEAM_FILE_ID + 0x0051,
    PS_FILE_ID_CTTF_HRPD_FSLP_PID_ENTRY_C       = CTTF_TEAM_FILE_ID + 0x0052,
    PS_FILE_ID_CTTF_HRPD_RSLP_PID_ENTRY_C       = CTTF_TEAM_FILE_ID + 0x0053,
    PS_FILE_ID_CTTF_HRPD_FTCMAC_DATA_PROC_C     = CTTF_TEAM_FILE_ID + 0x0054,
    PS_FILE_ID_CTTF_HRPD_FTCMAC_ENT_PROC_C      = CTTF_TEAM_FILE_ID + 0x0055,
    PS_FILE_ID_CTTF_HRPD_RMAC_SEND_MSG_C        = CTTF_TEAM_FILE_ID + 0x0056,
    PS_FILE_ID_CTTF_HRPD_ACMAC_ACC_PROC_C       = CTTF_TEAM_FILE_ID + 0x0057,
    PS_FILE_ID_CTTF_HRPD_ACMAC_CFG_PROC_C       = CTTF_TEAM_FILE_ID + 0x0058,
    PS_FILE_ID_CTTF_HRPD_ACMAC_CTRL_PROC_C      = CTTF_TEAM_FILE_ID + 0x0059,
    PS_FILE_ID_CTTF_HRPD_FSPS_CTRL_C            = CTTF_TEAM_FILE_ID + 0x005A,
    PS_FILE_ID_CTTF_HRPD_RSPS_CTRL_C            = CTTF_TEAM_FILE_ID + 0x005B,
    PS_FILE_ID_CTTF_HRPD_FSPS_PC_C              = CTTF_TEAM_FILE_ID + 0x005C,
    PS_FILE_ID_CTTF_HRPD_RSPS_PC_C              = CTTF_TEAM_FILE_ID + 0x005D,
    PS_FILE_ID_CTTF_HRPD_FSPS_SECURITY_C        = CTTF_TEAM_FILE_ID + 0x005E,
    PS_FILE_ID_CTTF_HRPD_RSPS_SECURITY_C        = CTTF_TEAM_FILE_ID + 0x005F,
    PS_FILE_ID_CTTF_HRPD_FSPS_STREAM_C          = CTTF_TEAM_FILE_ID + 0x0060,
    PS_FILE_ID_CTTF_HRPD_RSPS_STREAM_C          = CTTF_TEAM_FILE_ID + 0x0061,
    PS_FILE_ID_CTTF_HRPD_FSPS_PIDENTRY_C        = CTTF_TEAM_FILE_ID + 0x0062,
    PS_FILE_ID_CTTF_HRPD_RSPS_PIDENTRY_C        = CTTF_TEAM_FILE_ID + 0x0063,
    PS_FILE_ID_CTTF_HRPD_FPA_CTRL_C             = CTTF_TEAM_FILE_ID + 0x0064,
    PS_FILE_ID_CTTF_HRPD_FDOS_C                 = CTTF_TEAM_FILE_ID + 0x0065,
    PS_FILE_ID_CTTF_HRPD_RDOS_C                 = CTTF_TEAM_FILE_ID + 0x0066,
    PS_FILE_ID_CTTF_HRPD_FPA_PID_ENTRY_C        = CTTF_TEAM_FILE_ID + 0x0067,
    PS_FILE_ID_CTTF_HRPD_FRLP_C                 = CTTF_TEAM_FILE_ID + 0x0068,
    PS_FILE_ID_CTTF_HRPD_PA_MNTN_C              = CTTF_TEAM_FILE_ID + 0x0069,
    PS_FILE_ID_CTTF_HRPD_RPA_CTRL_C             = CTTF_TEAM_FILE_ID + 0x006A,
    PS_FILE_ID_CTTF_HRPD_RPA_PID_ENTRY_C        = CTTF_TEAM_FILE_ID + 0x006B,
    PS_FILE_ID_CTTF_HRPD_RPA_SEND_PPP_MSG_C     = CTTF_TEAM_FILE_ID + 0x006C,
    PS_FILE_ID_CTTF_HRPD_RPA_SEND_SNP_MSG_C     = CTTF_TEAM_FILE_ID + 0x006D,
    PS_FILE_ID_CTTF_HRPD_RPA_SEND_NAS_MSG_C     = CTTF_TEAM_FILE_ID + 0x006E,
    PS_FILE_ID_CTTF_HRPD_RPA_SEND_CAS_MSG_C     = CTTF_TEAM_FILE_ID + 0x006F,
    PS_FILE_ID_CTTF_HRPD_FPA_SEND_RPA_MSG_C     = CTTF_TEAM_FILE_ID + 0x0070,
    PS_FILE_ID_CTTF_HRPD_RPA_SEND_FPA_MSG_C     = CTTF_TEAM_FILE_ID + 0x0071,
    PS_FILE_ID_CTTF_HRPD_RRLP_C                 = CTTF_TEAM_FILE_ID + 0x0072,
    PS_FILE_ID_CTTF_HRPD_RTCMAC_CTRL_PROC_C     = CTTF_TEAM_FILE_ID + 0x0073,
    PS_FILE_ID_CTTF_HRPD_RTCMAC_DATA_PROC_C     = CTTF_TEAM_FILE_ID + 0x0074,
    PS_FILE_ID_CTTF_HRPD_RTCMAC_ENT_PROC_C      = CTTF_TEAM_FILE_ID + 0x0075,
    PS_FILE_ID_CTTF_HRPD_RTC3MAC_DATAPROC_C     = CTTF_TEAM_FILE_ID + 0x0076,
    PS_FILE_ID_CTTF_HRPD_MAC_MNTN_C             = CTTF_TEAM_FILE_ID + 0x0077,
    PS_FILE_ID_CTTF_HRPD_RTAP_C                 = CTTF_TEAM_FILE_ID + 0x0078,
    PS_FILE_ID_CTTF_HRPD_FTAP_C                 = CTTF_TEAM_FILE_ID + 0x0079,
    PS_FILE_ID_TTF_TPE_C                        = CTTF_TEAM_FILE_ID + 0x007A,
    PS_FILE_ID_CTTF_DATA_MOVE_DRIVER_C          = CTTF_TEAM_FILE_ID + 0x007B,
    PS_FILE_ID_CTTF_DATA_MOVE_SERVICE_C         = CTTF_TEAM_FILE_ID + 0x007C,
    PS_FILE_ID_CTTF_HRPD_MNTN_C                 = CTTF_TEAM_FILE_ID + 0x007D,
    PS_FILE_ID_CTTF_HRPD_RPA_FCP_CTRL_C         = CTTF_TEAM_FILE_ID + 0x007E,
    PS_FILE_ID_CTTF_HRPD_RTAP_CTRL_C            = CTTF_TEAM_FILE_ID + 0x007F,
    PS_FILE_ID_CTTF_HRPD_RDOS_CTRL_C            = CTTF_TEAM_FILE_ID + 0x0080,
    PS_FILE_ID_CTTF_HRPD_RPA_RRLP_CTRL_C        = CTTF_TEAM_FILE_ID + 0x0081,
    PS_FILE_ID_CTTF_1X_MNTN_C                   = CTTF_TEAM_FILE_ID + 0x0082,
    PS_FILE_ID_AS_COMM_DUMP_C                   = CTTF_TEAM_FILE_ID + 0x0083,
    PS_FILE_ID_AS_COMM_LOG_C                    = CTTF_TEAM_FILE_ID + 0x0084,
    PS_FILE_ID_CTTF_MNTN_COMM_C                 = CTTF_TEAM_FILE_ID + 0x0085,
    PS_FILE_ID_CTTF_1X_VOICECUST_C              = CTTF_TEAM_FILE_ID + 0x0086,
    PS_FILE_ID_MD5_ADAPTER_C                    = CTTF_TEAM_FILE_ID + 0x0087,
    PS_FILE_ID_RAND_DRBG_ADAPTER_C              = CTTF_TEAM_FILE_ID + 0x0088,
    PS_FILE_ID_CTTF_BUTT
    /* CDMA L2 , 2013-9-4 end*/
}CTTF_FILE_ID_DEFINE_ENUM;
typedef unsigned long  CTTF_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


