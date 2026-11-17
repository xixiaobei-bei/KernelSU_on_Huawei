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

#ifndef __VOICEAGENTOM_H__
#define __VOICEAGENTOM_H__

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
#pragma pack(*)    设置字节对齐方式
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 macro
*****************************************************************************/
#define VOICE_AGENT_LOG_SRC_BUF_LEN                 0x100000
#define VOICE_AGENT_LOG_DATA_LEN                    ((4*1024) - 4) /* 一次传输4K的数据，但是有4个字节是用来标记数据长度的 */
#define VOICE_AGENT_LOG_DATA_NUM                    (16)
#define VOICE_AGENT_LOG_BUFFER_LEN                  0x1000
#define VOICE_AGENT_SOCP_HEAD_MAGIC_NUM             0x48495349
typedef struct
{
    VOS_UINT32  ulChannelId;
    VOS_UINT32  ulChannelStatus;
    VOS_UINT8   *pChannelPhyAddr;
    VOS_UINT8   *pChannelVirAddr;
}VOICE_AGENT_DIAG_CTRL_INFO;

typedef struct
{
    VOS_UINT32  ulDataSize;
    VOS_UINT8   aucData[VOICE_AGENT_LOG_DATA_LEN];
}VOICE_AGENT_DIAG_DATA_INFO;
#if (FEATURE_ON == FEATURE_EXTRA_MODEM_MODE)
extern VOICE_AGENT_DIAG_DATA_INFO               g_stDiagData[VOICE_AGENT_LOG_DATA_NUM];
extern VOS_UINT32 VOICE_AGENT_DiagPackSend(VOS_CHAR *pData, VOS_UINT32 ulDataSize);
extern VOS_INT32 VOICE_AGENT_OM_PROC(VOS_VOID);
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

