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


#ifndef __DIAG_SERVICE_H__
#define __DIAG_SERVICE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <vos.h>
#include <mdrv.h>
#include <msp.h>
#include "msp_service.h"
#include "diag_service_comm.h"

#pragma pack(push)//保存对齐状态
#pragma pack(4)//强制4字节对齐

/*****************************************************************************
  2 macro
*****************************************************************************/
#ifdef DIAG_SYSTEM_5G
#define SOCP_CODER_SRC_PS_IND   SOCP_CODER_SRC_ACPU_IND
#define SOCP_CODER_SRC_CNF      SOCP_CODER_SRC_ACPU_CNF
#else
#define SOCP_CODER_SRC_PS_IND   SOCP_CODER_SRC_LOM_IND1
#define SOCP_CODER_SRC_CNF      SOCP_CODER_SRC_LOM_CNF1
#endif

/*****************************************************************************
  3 Enum
*****************************************************************************/
/*****************************************************************************
  4 struct
*****************************************************************************/

typedef struct
{
    SOCP_CODER_SRC_ENUM_U32         ulIndChannelID;    /* 编码源通道ID，固定配置 */
    SOCP_CODER_SRC_ENUM_U32         ulCnfChannelID;    /* 编码源通道ID，固定配置 */

}DIAG_SRV_CTRL;

typedef DIAG_SERVICE_HEAD_STRU MSP_SERVICE_HEAD_STRU;

typedef unsigned int (*DIAG_SERVICE_FUNC)(DIAG_FRAME_INFO_STRU *pData);
/*****************************************************************************
  6 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  7 Fuction Extern
*****************************************************************************/
VOS_VOID diag_SvcFillHeader(DIAG_SRV_HEADER_STRU *pstSrvHeader);
VOS_VOID diag_PktTimeoutClear(VOS_VOID);
VOS_VOID diag_SrvcCreatePkt(DIAG_FRAME_INFO_STRU *pFrame);
DIAG_FRAME_INFO_STRU * diag_SrvcSavePkt(DIAG_FRAME_INFO_STRU *pFrame, VOS_UINT32 ulDataLen);
VOS_VOID diag_SrvcDestroyPkt(DIAG_FRAME_INFO_STRU *pFrame);
VOS_UINT32 diag_ServiceProc(VOS_VOID *pData, VOS_UINT32 ulDatalen);
VOS_VOID diag_ServiceProcReg(DIAG_SERVICE_FUNC pServiceFn);
VOS_VOID diag_ServiceInit(VOS_VOID);
VOS_VOID diag_SrvcPackWrite(SOCP_BUFFER_RW_STRU *pRWBuffer, const VOS_VOID * pPayload, VOS_UINT32 u32DataLen );
VOS_UINT32 diag_SrvcPackIndSend(DIAG_MSG_REPORT_HEAD_STRU *pData);
VOS_UINT32 diag_ServicePackData(DIAG_MSG_REPORT_HEAD_STRU *pData);
VOS_UINT32 diag_SrvcPackCnfSend(DIAG_MSG_REPORT_HEAD_STRU *pData);
VOS_UINT32 diag_ServicePackCnfData(DIAG_MSG_REPORT_HEAD_STRU *pData);
VOS_UINT32 diag_ServicePacketResetData(DIAG_MSG_REPORT_HEAD_STRU *pData);
VOS_VOID DIAG_DebugDFR(VOS_VOID);


#pragma pack(pop) //恢复对齐方式

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
