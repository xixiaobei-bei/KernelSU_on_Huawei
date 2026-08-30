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
#include <product_config.h>
#include <linux/wakelock.h>
#include <mdrv.h>
#include <osl_types.h>
#include "msp_service.h"
#include "bsp_diag.h"

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

#define DIAG_SRV_SET_MODEM_ID(pstFrame, modemid)    ((pstFrame)->stService.mdmid3b = (modemid))
#define DIAG_SRV_SET_TRANS_ID(pstFrame, transid)    ((pstFrame)->stService.MsgTransId = (transid))
#define DIAG_SRV_SET_MSG_LEN(pstFrame, msglen)      ((pstFrame)->u32MsgLen = (msglen))
#define DIAG_SRV_SET_COMMAND_ID(pstFrame, pri, mode, sec, cmdid)\
do{\
    (pstFrame)->stID.pri4b  = pri;\
    (pstFrame)->stID.mode4b = mode;\
    (pstFrame)->stID.sec5b  = sec;\
    (pstFrame)->stID.cmdid19b = cmdid;\
}while(0);
#define DIAG_SRV_SET_COMMAND_ID_EX(pstFrame, cmdid) ((pstFrame)->u32CmdId = (cmdid))
#ifdef DIAG_SYSTEM_5G
#define SERVICE_HEAD_SID(pData)   ((u32)(((diag_service_head_stru *)pData)->sid4b))
#else
#define SERVICE_HEAD_SID(pData)         ((u32)(((diag_service_head_stru *)pData)->sid8b))
#endif

#define SERVICE_MSG_PROC            (0x1000)

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
    struct wake_lock                stWakelock;
}DIAG_SRV_CTRL;

/*****************************************************************************
  6 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  7 Fuction Extern
*****************************************************************************/
void diag_SvcFillHeader(DIAG_SRV_HEADER_STRU *pstSrvHeader);
void diag_PktTimeoutClear(void);
void diag_SrvcCreatePkt(diag_frame_head_stru *pFrame);
diag_frame_head_stru * diag_SrvcSavePkt(diag_frame_head_stru *pFrame, u32 ulDataLen);
void diag_SrvcDestroyPkt(diag_frame_head_stru *pFrame);
u32 diag_ServiceProc(void *pData, u32 ulDatalen);
void diag_ServiceProcReg(DRV_DIAG_SERVICE_FUNC pServiceFn);
void diag_ServiceInit(void);
void diag_SrvcPackWrite(SOCP_BUFFER_RW_STRU *pRWBuffer, const void * pPayload, u32 u32DataLen );
u32 diag_SrvcPackIndSend(DIAG_MSG_REPORT_HEAD_STRU *pData);
u32 diag_ServicePackData(DIAG_MSG_REPORT_HEAD_STRU *pData);
u32 diag_SrvcPackCnfSend(DIAG_MSG_REPORT_HEAD_STRU *pData);
u32 diag_ServicePackCnfData(DIAG_MSG_REPORT_HEAD_STRU *pData);
void DIAG_DebugDFR(void);
u32 diag_ServicePacketResetData(DIAG_MSG_REPORT_HEAD_STRU *pData);
u32 diag_GetSrvData(diag_frame_head_stru *pHeader, u32 ulDatalen, diag_frame_head_stru **pProcHead);


#pragma pack(pop) //恢复对齐方式

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
