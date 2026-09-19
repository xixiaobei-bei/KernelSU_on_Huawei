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


#ifndef __DIAG_COMMON_H__
#define __DIAG_COMMON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <product_config.h>
#include "vos.h"
#include "msp.h"
#include "msp_debug.h"
#include "diag_message.h"
#include "diag_msg_def.h"
#include "diag_cmdid_def.h"
#include "diag_service.h"
#include "diag_common_comm.h"

/*****************************************************************************
  2 macro
*****************************************************************************/
#define DIAG_SSID_CPU       DIAG_SSID_APP_CPU
#define DIAG_AGENT_PID      MSP_PID_DIAG_APP_AGENT

#define DIAG_MSG_COMMON_PROC(stDiagInfo, Cnf, pData)    \
do{                                                     \
    stDiagInfo.ulSSId       = DIAG_SSID_CPU;            \
    stDiagInfo.ulMode       = pData->stID.mode4b;                        \
    stDiagInfo.ulSubType    = pData->stID.sec5b;        \
    stDiagInfo.ulDirection  = DIAG_MT_CNF;              \
    stDiagInfo.ulModemid    = 0;                        \
    stDiagInfo.ulMsgId      = pData->ulCmdId;           \
    stDiagInfo.ulTransId    = pData->stService.ulMsgTransId;            \
    Cnf.ulAuid = ((MSP_DIAG_DATA_REQ_STRU*)(pData->aucData))->ulAuid;   \
    Cnf.ulSn = ((MSP_DIAG_DATA_REQ_STRU*)(pData->aucData))->ulSn;       \
}while(0)


#define DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret) \
do {    \
    ulLen = (sizeof(DIAG_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH) + pstDiagHead->ulMsgLen;  \
    pstInfo = (DIAG_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);  \
    if(VOS_NULL == pstInfo) \
    {   \
        ret = ERR_MSP_MALLOC_FAILUE;    \
        goto DIAG_ERROR;    \
    }   \
    pstInfo->ulReceiverPid = MSP_PID_DIAG_AGENT;    \
    pstInfo->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;    \
    pstInfo->ulMsgId       = DIAG_MSG_MSP_A_TRANS_C_REQ;    \
    ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pstDiagHead->ulMsgLen; \
    VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pstDiagHead, ulLen);   \
    ret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo); \
    if(ret) \
    {   \
        ret = ERR_MSP_DIAG_SEND_MSG_FAIL; \
        goto DIAG_ERROR;    \
    }   \
}while(0)

#if (defined(DIAG_SYSTEM_5G) && defined(CONFIG_DIAG_NRM))
#define DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret) \
    do {    \
        ulLen = (sizeof(DIAG_MSG_A_TRANS_C_STRU) - VOS_MSG_HEAD_LENGTH) + pstDiagHead->ulMsgLen;  \
        pstInfo = (DIAG_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);  \
        if(VOS_NULL == pstInfo) \
        {   \
            ret = ERR_MSP_MALLOC_FAILUE;    \
            goto DIAG_ERROR;    \
        }   \
        pstInfo->ulReceiverPid = MSP_PID_DIAG_NRM_AGENT;    \
        pstInfo->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;    \
        pstInfo->ulMsgId       = DIAG_MSG_MSP_A_TRANS_C_REQ;    \
        ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pstDiagHead->ulMsgLen; \
        VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pstDiagHead, ulLen);   \
        ret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo); \
        if(ret) \
        {   \
            ret = ERR_MSP_DIAG_SEND_MSG_FAIL; \
            goto DIAG_ERROR;    \
        }   \
    }while(0)
#else
#define DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret)
#endif

#define     DIAG_HIGH_TS_PUSH_TIMER_LEN     (10)    /* 10 min */

#define     DIAG_ERRORLOG_TIMER_NAME        (0x00001001)
#define     DIAG_CLTINFO_TIMER_NAME         (0x00002002)
#define     DIAG_HIGH_TS_PUSH_TIMER_NAME    (0x00004004)

#define     DIAG_ERRORLOG_TIMER_PARA        (0x0000EFFE)
#define     DIAG_CLTINFO_TIMER_PARA         (0x0000DFFD)
#define     DIAG_HIGH_TS_PUSH_TIMER_PARA    (0x0000BFFB)


/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/
extern VOS_VOID diag_MessageInit(VOS_VOID);

/* DIAG全局信息初始化接口 */
extern VOS_VOID diag_MspMsgInit(VOS_VOID);

extern VOS_UINT32 diag_SendMsg(VOS_UINT32 ulSenderId, VOS_UINT32 ulRecverId, VOS_UINT32 ulMsgId, VOS_UINT8* pDta, VOS_UINT32 dtaSize);

extern VOS_VOID diag_DumpDFInfo(DIAG_FRAME_INFO_STRU * pFrame);

extern VOS_BOOL diag_IsPowerOnLogOpen(VOS_VOID);

/*****************************************************************************
  9 OTHERS
*****************************************************************************/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of msp_common.h */

