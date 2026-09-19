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


/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_api.h"
#include "diag_debug.h"
#include "msp_errno.h"
#include "nv_stru_drv.h"
#include "nv_stru_lps.h"
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "soc_socp_adapter.h"
#include "diag_connect.h"
#include "diag_msgbsp.h"
#include "diag_msglrm.h"


#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGLRM_C

DIAG_MSGLRM_CTRL g_DiagLrmCtrl =
{
    DIAG_LRM_SRC_NUM,
    {SOCP_CODER_SRC_BSP_CCORE, SOCP_CODER_SRC_LOM_IND2, SOCP_CODER_SRC_LDSP1, SOCP_CODER_SRC_LDSP2, SOCP_CODER_SRC_BBP_LOG, SOCP_CODER_SRC_BBP_DS}
};

VOS_VOID diag_LRMMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;

    ulRet = diag_ConnMgrSendFuncReg(DIAG_CONN_ID_ACPU_LRM, DIAG_LRM_SRC_NUM,
                                g_DiagLrmCtrl.ulChannelId, diag_LRMSendConnMsg);
    if(ulRet)
    {
        diag_error("LRM Reg ConnMsg Fail(0x%x)\n", ulRet);
    }

    return;
}

VOS_UINT32 diag_LRMSendConnMsg(VOS_UINT8 *pData)
{
    DIAG_CONN_MSG_SEND_T *pConnMsg;
    DIAG_CONNECT_RESULT   pstResult[DIAG_LRM_SRC_NUM];
    VOS_UINT32 ulRet;
    VOS_UINT32 ulIndex;

    pConnMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    ulRet = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_AGENT, pConnMsg->ulMsgId, (VOS_UINT8 *)&(pConnMsg->ulSn), sizeof(pConnMsg->ulSn));
    if(ulRet)
    {
        diag_error("send msg to lrm fail(0x%x)\n", ulRet);
        for(ulIndex = 0; ulIndex < DIAG_LRM_SRC_NUM; ulIndex++)
        {
            pstResult[ulIndex].ulChannelId  = g_DiagLrmCtrl.ulChannelId[ulIndex];
            pstResult[ulIndex].ulResult     = ERR_MSP_DIAG_TL_SEND_MSG_FAIL;
        }
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_LRM, pConnMsg->ulSn, DIAG_LRM_SRC_NUM, pstResult);
    }
    return ERR_MSP_SUCCESS;
}

VOS_VOID diag_LRMConnCnfProc(VOS_UINT8* pMsgBlock)
{
    DIAG_DATA_MSG_STRU*     pstMsgTmp   = NULL;
    DIAG_CONNECT_RESULT*    pstResult   = NULL;
    DIAG_CONN_CNF_MSG_STRU *pstCnf      = NULL;
    VOS_UINT32 ulCnfIndex;
    VOS_UINT32 ulLocalIndex;
    VOS_UINT32 ulCount;
    VOS_UINT32 ulRet;

    pstMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;
    pstCnf = (DIAG_CONN_CNF_MSG_STRU *)pstMsgTmp->pContext;

    pstResult = pstCnf->pstResult;
    ulCount = (pstMsgTmp->ulLen - sizeof(pstCnf->ulSn))/sizeof(DIAG_CONNECT_RESULT);

    if(ulCount != DIAG_LRM_SRC_NUM)
    {
        diag_error("ChanNum is invalid, except:0x%x cnf:0x%x\n", DIAG_LRM_SRC_NUM, ulCount);
        return;
    }

    for(ulCnfIndex = 0; ulCnfIndex < ulCount; ulCnfIndex++)
    {
        for(ulLocalIndex = 0; ulLocalIndex < ulCount; ulLocalIndex++)
        {
            if(pstResult[ulCnfIndex].ulChannelId == g_DiagLrmCtrl.ulChannelId[ulCnfIndex])
            {
                break;
            }
        }
        if(ulLocalIndex == ulCount)
        {
            diag_error("ChanId(0x%x) is invalid\n", pstResult[ulCnfIndex].ulChannelId);
            return;
        }
    }

    ulRet = diag_ConnCnf(DIAG_CONN_ID_ACPU_LRM, pstCnf->ulSn, ulCount, pstResult);
    if(ulRet)
    {
        diag_error("cnf fail, ulRet:0x%x\n", ulRet);
    }
}


VOS_VOID diag_LrmAgentMsgProc(MsgBlock* pMsgBlock)
{
    DIAG_DATA_MSG_STRU* pMsgTmp;

    pMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;

    switch(pMsgTmp->ulMsgId)
    {
        case DIAG_MSG_BSP_CMD_LIST_REQ:
            diag_BspRecvCmdList(pMsgBlock);
            break;

        case ID_MSG_DIAG_CMD_CONNECT_CNF:
        case ID_MSG_DIAG_CMD_DISCONNECT_CNF:
            diag_LRMConnCnfProc((VOS_UINT8 *)pMsgTmp);
            break;

        default:
            break;
    }
}


