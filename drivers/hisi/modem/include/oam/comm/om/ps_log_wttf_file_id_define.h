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

#ifndef PS_LOG_WTTF_FILE_ID_DEFINE_H
#define PS_LOG_WTTF_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    PS_FILE_ID_WTTF_BMCENTPROC_C                = WTTF_TEAM_FILE_ID,            /* WTTF_TEAM_FILE_ID = 0x1400 */
    PS_FILE_ID_WTTF_BMCCTRL_C                   = WTTF_TEAM_FILE_ID + 0x0001,
    PS_FILE_ID_WTTF_BMCMNTN_C                   = WTTF_TEAM_FILE_ID + 0x0002,
    PS_FILE_ID_WTTF_MACCENTPROC_C               = WTTF_TEAM_FILE_ID + 0x0003,
    PS_FILE_ID_WTTF_MACCOMM_C                   = WTTF_TEAM_FILE_ID + 0x0004,
    PS_FILE_ID_WTTF_MACDENTPROC_C               = WTTF_TEAM_FILE_ID + 0x0005,
    PS_FILE_ID_WTTF_MACEENTPROC_C               = WTTF_TEAM_FILE_ID + 0x0006,
    PS_FILE_ID_WTTF_MACCTRL_C                   = WTTF_TEAM_FILE_ID + 0x0007,
    PS_FILE_ID_WTTF_MACHSENTPROC_C              = WTTF_TEAM_FILE_ID + 0x0008,
    PS_FILE_ID_WTTF_MACMNTN_C                   = WTTF_TEAM_FILE_ID + 0x0009,
    PS_FILE_ID_WTTF_MACTRAFFICMEAS_C            = WTTF_TEAM_FILE_ID + 0x000A,
    PS_FILE_ID_WTTF_PDCPDATAPROC_C              = WTTF_TEAM_FILE_ID + 0x000B,
    PS_FILE_ID_WTTF_PDCPENTPROC_C               = WTTF_TEAM_FILE_ID + 0x000C,
    PS_FILE_ID_WTTF_PDCPCTRL_C                  = WTTF_TEAM_FILE_ID + 0x000D,
    PS_FILE_ID_WTTF_RLCAMDATAPROC_C             = WTTF_TEAM_FILE_ID + 0x000E,
    PS_FILE_ID_WTTF_RLCAMENTITYMGMT_C           = WTTF_TEAM_FILE_ID + 0x000F,
    PS_FILE_ID_WTTF_RLCCIPHER_C                 = WTTF_TEAM_FILE_ID + 0x0010,
    PS_FILE_ID_WTTF_RLCCOMMON_C                 = WTTF_TEAM_FILE_ID + 0x0011,
    PS_FILE_ID_WTTF_RLCCTRL_C                   = WTTF_TEAM_FILE_ID + 0x0012,
    PS_FILE_ID_WTTF_RLCMNTN_C                   = WTTF_TEAM_FILE_ID + 0x0013,
    PS_FILE_ID_WTTF_RLCRESETPROC_C              = WTTF_TEAM_FILE_ID + 0x0014,
    PS_FILE_ID_WTTF_RLCSTATUSPROC_C             = WTTF_TEAM_FILE_ID + 0x0015,
    PS_FILE_ID_WTTF_RLCTMDATAPROC_C             = WTTF_TEAM_FILE_ID + 0x0016,
    PS_FILE_ID_WTTF_RLCTMENTITYMGMT_C           = WTTF_TEAM_FILE_ID + 0x0017,
    PS_FILE_ID_WTTF_RLCUMDATAPROC_C             = WTTF_TEAM_FILE_ID + 0x0018,
    PS_FILE_ID_WTTF_RLCUMENTITYMGMT_C           = WTTF_TEAM_FILE_ID + 0x0019,
    PS_FILE_ID_WTTF_MACEHSENTPROC_C             = WTTF_TEAM_FILE_ID + 0x001A,
    PS_FILE_ID_WTTF_MAC_AMRSWITCH_C             = WTTF_TEAM_FILE_ID + 0x001B,
    PS_FILE_ID_WTTF_MACCDLENTPROC_C             = WTTF_TEAM_FILE_ID + 0x001C,
    PS_FILE_ID_WTTF_MACDULENTPROC_C             = WTTF_TEAM_FILE_ID + 0x001D,
    PS_FILE_ID_WTTF_MACDDLENTPROC_C             = WTTF_TEAM_FILE_ID + 0x001E,
    PS_FILE_ID_WTTF_MACDLCTRL_C                 = WTTF_TEAM_FILE_ID + 0x001F,
    PS_FILE_ID_WTTF_MACULPROC_C                 = WTTF_TEAM_FILE_ID + 0x0020,
    PS_FILE_ID_WTTF_MACDLPROC_C                 = WTTF_TEAM_FILE_ID + 0x0021,
    PS_FILE_ID_WTTF_RLCAMDLDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x0022,
    PS_FILE_ID_WTTF_RLCAMDLENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x0023,
    PS_FILE_ID_WTTF_RLCAMULDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x0024,
    PS_FILE_ID_WTTF_RLCAMULENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x0025,
    PS_FILE_ID_WTTF_RLCTMDLDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x0026,
    PS_FILE_ID_WTTF_RLCTMDLENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x0027,
    PS_FILE_ID_WTTF_RLCTMULDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x0028,
    PS_FILE_ID_WTTF_RLCTMULENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x0029,
    PS_FILE_ID_WTTF_RLCUMDLDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x002A,
    PS_FILE_ID_WTTF_RLCUMDLENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x002B,
    PS_FILE_ID_WTTF_RLCUMULDATAPROC_C           = WTTF_TEAM_FILE_ID + 0x002C,
    PS_FILE_ID_WTTF_RLCUMULENTITYMGMT_C         = WTTF_TEAM_FILE_ID + 0x002D,
    PS_FILE_ID_WTTF_RLCDLCIPHER_C               = WTTF_TEAM_FILE_ID + 0x002E,
    PS_FILE_ID_WTTF_RLCULCIPHER_C               = WTTF_TEAM_FILE_ID + 0x002F,
    PS_FILE_ID_WTTF_RLCDLPROC_C                 = WTTF_TEAM_FILE_ID + 0x0030,
    PS_FILE_ID_WTTF_RLCULPROC_C                 = WTTF_TEAM_FILE_ID + 0x0031,
    PS_FILE_ID_WTTF_RLCDLCTRL_C                 = WTTF_TEAM_FILE_ID + 0x0032,
    PS_FILE_ID_WTTF_RLCULRESETPROC_C            = WTTF_TEAM_FILE_ID + 0x0033,
    PS_FILE_ID_WTTF_RLCDLRESETPROC_C            = WTTF_TEAM_FILE_ID + 0x0034,
    PS_FILE_ID_WTTF_RLCDLSTATUSPROC_C           = WTTF_TEAM_FILE_ID + 0x0035,
    PS_FILE_ID_WTTF_RLCULSTATUSPROC_C           = WTTF_TEAM_FILE_ID + 0x0036,
    PS_FILE_ID_WTTF_RLCINTERFACE_C              = WTTF_TEAM_FILE_ID + 0x0037,
    PS_FILE_ID_WTTF_MACMSGPROC_C                = WTTF_TEAM_FILE_ID + 0x0038,
    PS_FILE_ID_WTTF_RLCMSGPROC_C                = WTTF_TEAM_FILE_ID + 0x0039,
    PS_FILE_ID_WTTF_RLCAMQUEUEMGMT_C            = WTTF_TEAM_FILE_ID + 0x003A,
    PS_FILE_ID_WTTF_MACCULENTPROC_C             = WTTF_TEAM_FILE_ID + 0x003B,
    PS_FILE_ID_WTTF_MACIENTPROC_C               = WTTF_TEAM_FILE_ID + 0x003C,
    PS_FILE_ID_WTTF_TRACE_C                     = WTTF_TEAM_FILE_ID + 0x003D,
    PS_FILE_ID_WTTF_PBDTASKENTRY_C              = WTTF_TEAM_FILE_ID + 0x003E,
    PS_FILE_ID_WTTF_RMDLTASKENTRY_C             = WTTF_TEAM_FILE_ID + 0x003F,
    PS_FILE_ID_WTTF_RMULTASKENTRY_C             = WTTF_TEAM_FILE_ID + 0x0040,
    PS_FILE_ID_WTTF_NODE_MEM_C                  = WTTF_TEAM_FILE_ID + 0x0041,
    PS_FILE_ID_WTTF_PDCP_API_C                  = WTTF_TEAM_FILE_ID + 0x0042,
    PS_FILE_ID_WTTF_MAC_API_C                   = WTTF_TEAM_FILE_ID + 0x0043,
    PS_FILE_ID_WTTF_MAC_ERR_LOG_C               = WTTF_TEAM_FILE_ID + 0x0044,
    PS_FILE_ID_WTTF_RLC_ERR_LOG_C               = WTTF_TEAM_FILE_ID + 0x0045,
    PS_FILE_ID_WTTF_MAC_PHY_SHARE_ENTITY_C      = WTTF_TEAM_FILE_ID + 0x0046,
    PS_FILE_ID_WTTF_MAC_PHY_SHARE_ENTITY_FUSION_C = WTTF_TEAM_FILE_ID + 0x0047,
    PS_FILE_ID_WTTF_MAC_SLOT0_REG_C               = WTTF_TEAM_FILE_ID + 0x0048,
    PS_FILE_ID_WTTF_MAC_SLOT0_REG_FUSION_C        = WTTF_TEAM_FILE_ID + 0x0049,
    PS_FILE_ID_WTTF_MAC_BBPMST_CTRL_C             = WTTF_TEAM_FILE_ID + 0x004A,
    PS_FILE_ID_WTTF_MAC_BBPMST_CTRL_FUSION_C      = WTTF_TEAM_FILE_ID + 0x004B,

    WTTF_FILE_ID_BUTT = GTTF_TEAM_FILE_ID - 1
}WTTF_FILE_ID_DEFINE_ENUM;
typedef unsigned long WTTF_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


