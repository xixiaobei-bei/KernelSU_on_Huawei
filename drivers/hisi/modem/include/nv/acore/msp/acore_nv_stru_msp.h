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


#ifndef __MSP_NV_DEF_ACORE_H__
#define __MSP_NV_DEF_ACORE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "vos.h"
#include "acore_nv_id_msp.h"

/* 53252 */
typedef struct
{
    VOS_INT16 ssLevel[4];
    VOS_INT16 ssValue[4];
}NV_RSRP_CFG_STRU;

/* 53253 */
typedef struct
{
    VOS_INT16 ssLevel[4];
    VOS_INT16 ssValue[4];
}NV_ECIO_CFG_STRU;

/* 53254 */
typedef struct
{
    VOS_INT16 ssLevel[4];
    VOS_INT16 ssValue[4];
}NV_RSCP_CFG_STRU;

/* 53257 */
typedef struct
{
    VOS_UINT32  AuthType;             /*AuthorityType*/
}NV_MSP_AUTH_TYPE_STRU;

/* 53258 */
typedef struct
{
    VOS_UINT32  AuthVer;             /*AuthorityVer*/
}NV_MSP_AUTH_VER_STRU;


typedef struct _FAULTID_MAP
{
    VOS_UINT16 usFaultId;
    VOS_UINT8  aucReserved0[2];
    VOS_UINT32 ulNvId;
    VOS_UINT8  aucReserved1[4];              //±£¡ÙŒª

}FAULTID_MAP;

typedef struct _CHR_CONFIG_CTRL
{
    VOS_UINT32   ulFaultIdNum;
    FAULTID_MAP  aucFaulIdMap[24];
    
}NV_ID_CHR_CONFIG_CTRL_INFO_STRU;


typedef struct _CHR_C_CONFIG_CTRL
{
    VOS_UINT32   ulFaultIdNum;
    FAULTID_MAP  aucFaulIdMap[24];
    
}NV_ID_CHR_C_CONFIG_CTRL_INFO_STRU;


typedef struct _CHR_ALARM
{
    VOS_UINT32 ulPid;
    VOS_UINT32 ulAlarmId;
    
}CHR_ALARM;

typedef struct _CHR_FAULTID_CONFIG
{
    VOS_UINT32 ulFaultId;
    VOS_UINT32 ulAlarmNum;
    CHR_ALARM  aucAlarmMap[96];
    
}NV_ID_CHR_FAULTID_CONFIG_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

