/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#ifndef __VOICEAGENTNV_H__
#define __VOICEAGENTNV_H__

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/

#include "Voice_agent_public.h"
#include "NVIM_Interface.h"

/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
#pragma pack(*)    ????????
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 macro
*****************************************************************************/
#define HIFI_MODEM_NV_NO_THIS_ID    0x100f0016

#define HIFI_MODEM_NV_BEGIN_ID          30000
#define HIFI_MODEM_NV_END_ID            30200
#define HIFI_MODEM_NV_MAX_NUM           200
#define B5000_MODEM_NV_CONTENT_MAX      (2 * 200 * 512)
#define B5000_NV_READY_MAIGC_WORD       0x66666666
#define VOICE_AGENT_GET_NV_LENGTH(id, len)  mdrv_nv_get_length(id, len)

typedef struct
{
    VOS_UINT16                          ulHifiModemNvId;
    VOS_UINT16                          ulNvDataLen;
    VOS_UINT32                          ulNvDataOffset[2];
}HIFI_MODEM_NV_STRU;

typedef struct
{
    VOS_UINT32                          ulHifiNvMagic;
    VOS_UINT32                          ulHifiNvNum;
    HIFI_MODEM_NV_STRU                  stNvID[HIFI_MODEM_NV_MAX_NUM];
    VOS_UINT8                           ulData[B5000_MODEM_NV_CONTENT_MAX];
}HIFI_MODEM_NV_DATA_STRU;
#if (FEATURE_ON == FEATURE_EXTRA_MODEM_MODE)
extern VOS_INT32 VOICE_AGENT_NV_PROC(VOS_VOID);
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif




#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif /* end of VOICEAGENTPublic.h */

