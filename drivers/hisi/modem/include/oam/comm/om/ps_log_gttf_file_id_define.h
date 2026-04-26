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

#ifndef PS_LOG_GTTF_FILE_ID_DEFINE_H
#define PS_LOG_GTTF_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    PS_FILE_ID_GTTF_GRM_MNTN_C                  = GTTF_TEAM_FILE_ID,            /* GTTF_TEAM_FILE_ID = 0x1800 */
    PS_FILE_ID_GTTF_GRM_COMM_C                  = GTTF_TEAM_FILE_ID + 0x0001,
    PS_FILE_ID_GTTF_MAC_COMM_C                  = GTTF_TEAM_FILE_ID + 0x0002,
    PS_FILE_ID_GTTF_MAC_DL_PROC_C               = GTTF_TEAM_FILE_ID + 0x0003,
    PS_FILE_ID_GTTF_MAC_UL_PROC_C               = GTTF_TEAM_FILE_ID + 0x0004,
    PS_FILE_ID_GTTF_RLC_DL_CTRL_C               = GTTF_TEAM_FILE_ID + 0x0005,
    PS_FILE_ID_GTTF_RLC_DL_DATA_PROC_C          = GTTF_TEAM_FILE_ID + 0x0006,
    PS_FILE_ID_GTTF_RLC_UL_CTRL_C               = GTTF_TEAM_FILE_ID + 0x0007,
    PS_FILE_ID_GTTF_RLC_UL_DATA_PROC_C          = GTTF_TEAM_FILE_ID + 0x0008,
    PS_FILE_ID_GTTF_T4_FUNC_C                   = GTTF_TEAM_FILE_ID + 0x0009,
    PS_FILE_ID_GTTF_GTM_C                       = GTTF_TEAM_FILE_ID + 0x000A,
    PS_FILE_ID_GTTF_COMM_C                      = GTTF_TEAM_FILE_ID + 0x000B,
    PS_FILE_ID_GTTF_QUEUE_C                     = GTTF_TEAM_FILE_ID + 0x000C,
    PS_FILE_ID_LL_C                             = GTTF_TEAM_FILE_ID + 0x000D,
    PS_FILE_ID_LLABM_C                          = GTTF_TEAM_FILE_ID + 0x000E,
    PS_FILE_ID_LLADM_C                          = GTTF_TEAM_FILE_ID + 0x000F,
    PS_FILE_ID_LLCOMM_C                         = GTTF_TEAM_FILE_ID + 0x0010,
    PS_FILE_ID_LLE_C                            = GTTF_TEAM_FILE_ID + 0x0011,
    PS_FILE_ID_LLGEA_C                          = GTTF_TEAM_FILE_ID + 0x0012,
    PS_FILE_ID_LLME_C                           = GTTF_TEAM_FILE_ID + 0x0013,
    PS_FILE_ID_LLMUX_C                          = GTTF_TEAM_FILE_ID + 0x0014,
    PS_FILE_ID_LLXID_C                          = GTTF_TEAM_FILE_ID + 0x0015,
    PS_FILE_ID_MAC_STUB_C                       = GTTF_TEAM_FILE_ID + 0x0016,
    PS_FILE_ID_SNCOMM_C                         = GTTF_TEAM_FILE_ID + 0x0017,
    PS_FILE_ID_SNDATA_C                         = GTTF_TEAM_FILE_ID + 0x0018,
    PS_FILE_ID_SNNSAPI_C                        = GTTF_TEAM_FILE_ID + 0x0019,
    PS_FILE_ID_SNSAPI_C                         = GTTF_TEAM_FILE_ID + 0x001A,
    PS_FILE_ID_SNXID_C                          = GTTF_TEAM_FILE_ID + 0x001B,
    PS_FILE_ID_DL_C                             = GTTF_TEAM_FILE_ID + 0x001C,
    PS_FILE_ID_DLDATA_C                         = GTTF_TEAM_FILE_ID + 0x001D,
    PS_FILE_ID_DLPHY_C                          = GTTF_TEAM_FILE_ID + 0x001E,
    PS_FILE_ID_DLSTATECTRL_C                    = GTTF_TEAM_FILE_ID + 0x001F,
    PS_FILE_ID_FRMWKCOM_C                       = GTTF_TEAM_FILE_ID + 0x0020,
    PS_FILE_ID_GMAC_C                           = GTTF_TEAM_FILE_ID + 0x0021,
    PS_FILE_ID_GMAC_BUFFMANAGE_C                = GTTF_TEAM_FILE_ID + 0x0022,
    PS_FILE_ID_GMAC_DLTXCTRL_C                  = GTTF_TEAM_FILE_ID + 0x0023,
    PS_FILE_ID_GMAC_ENTITYMANAGE_C              = GTTF_TEAM_FILE_ID + 0x0024,
    PS_FILE_ID_GMAC_ULTXCTRL_C                  = GTTF_TEAM_FILE_ID + 0x0025,
    PS_FILE_ID_GTTF_TRACE_C                     = GTTF_TEAM_FILE_ID + 0x0026,
    PS_FILE_ID_GUTTF_SNOW_C                     = GTTF_TEAM_FILE_ID + 0x0027,
    PS_FILE_ID_SNMNTN_C                         = GTTF_TEAM_FILE_ID + 0x0028,
    PS_FILE_ID_LLMNTN_C                         = GTTF_TEAM_FILE_ID + 0x0029,
    PS_FILE_ID_GTTF_RLC_UL_PEND_BUF_C           = GTTF_TEAM_FILE_ID + 0x002A,
    PS_FILE_ID_GTTF_RLC_RRBP_BUILDER_C          = GTTF_TEAM_FILE_ID + 0x002B,
    PS_FILE_ID_GTTF_RLC_UL_ACK_PROC_C           = GTTF_TEAM_FILE_ID + 0x002C,
    PS_FILE_ID_GTTF_RLC_RCV_UL_SDU_PROC_C       = GTTF_TEAM_FILE_ID + 0x002D,
    PS_FILE_ID_LL_ERR_LOG_C                     = GTTF_TEAM_FILE_ID + 0x002E,
    PS_FILE_ID_GTTF_GRM_ERR_LOG_C               = GTTF_TEAM_FILE_ID + 0x002F,
    PS_FILE_ID_GMAC_WAIT_SEND_QUEUE_C           = GTTF_TEAM_FILE_ID + 0x0030,
    PS_FILE_ID_SNIPSCH_C                        = GTTF_TEAM_FILE_ID + 0x0031,
    PS_FILE_ID_LL_GEA_SOFTWARE_C                = GTTF_TEAM_FILE_ID + 0x0032,

    GTTF_FILE_ID_BUTT = CTTF_TEAM_FILE_ID - 1
}GTTF_FILE_ID_DEFINE_ENUM;
typedef unsigned long GTTF_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


