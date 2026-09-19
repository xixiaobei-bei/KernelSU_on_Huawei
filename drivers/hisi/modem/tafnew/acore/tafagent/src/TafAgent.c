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
#define    THIS_FILE_ID        PS_FILE_ID_TAF_AGENT_C


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "TafAgent.h"
#include "TafAgentCtx.h"
#include "TafAgentLog.h"
#include "TafAgentInterface.h"
#include "TafAgentOamInterface.h"



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
extern VOS_UINT32 AT_GetDestPid(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT32                          ulRcvPid
);


/*****************************************************************************
3 函数实现
*****************************************************************************/


VOS_UINT32 TAF_AGENT_PidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    switch ( enPhase )
    {
        case VOS_IP_INITIAL:
            return Taf_Agent_InitCtx();

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 TAF_AGENT_IsValidMsg(MsgBlock* pstMsg)
{
    MSG_HEADER_STRU                    *pstMsgHeader;

    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_IsValidMsg: Msg is invalid!");
        return VOS_ERR;
    }

    pstMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    if ((I0_WUEPS_PID_TAF == pstMsgHeader->ulSenderPid)
     || (I1_WUEPS_PID_TAF == pstMsgHeader->ulSenderPid)
     || (I2_WUEPS_PID_TAF == pstMsgHeader->ulSenderPid)
     || (I0_UEPS_PID_MTA  == pstMsgHeader->ulSenderPid)
     || (I1_UEPS_PID_MTA  == pstMsgHeader->ulSenderPid)
     || (I2_UEPS_PID_MTA  == pstMsgHeader->ulSenderPid)
     || (I0_UEPS_PID_DSM  == pstMsgHeader->ulSenderPid)
     || (I1_UEPS_PID_DSM  == pstMsgHeader->ulSenderPid)
     || (I2_UEPS_PID_DSM  == pstMsgHeader->ulSenderPid))
    {

        if ((ID_TAFAGENT_PS_FIND_CID_FOR_DIAL_CNF <= pstMsgHeader->ulMsgName)
         && (ID_TAFAGENT_MSG_ID_ENUM_BUTT > pstMsgHeader->ulMsgName))
        {
            return VOS_OK;
        }
    }

   return VOS_ERR;
}


VOS_VOID TAF_AGENT_ClearAllSem(VOS_VOID)
{
    /* 如果有锁的存在 */
    if (VOS_TRUE == TAF_AGENT_GET_ACPU_CNF_SEM_LOCK_FLG())
    {
        VOS_SmV(TAF_AGENT_GetTafAcpuCnfSem());
    }

    return;
}


VOS_VOID TAF_AGENT_ProcMsg(MsgBlock* pstMsg)
{
    VOS_UINT8                          *pucMsg;
    VOS_UINT32                          ulResult;

    ulResult    = VOS_OK;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_ProcMsg!");

    if (VOS_OK == TAF_AGENT_IsValidMsg(pstMsg))
    {
        TAFAGENT_NORMAL_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: Msg Name is %x\n", ((MSG_HEADER_STRU *)pstMsg)->ulMsgName);

        if (VOS_FALSE == TAF_AGENT_GET_ACPU_CNF_SEM_LOCK_FLG())
        {
            TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: SemLockedFlg is False!");
            return;
        }

        /* 判断消息是否被释放 */
        if (VOS_NULL_PTR == TAF_AGENT_GetTafAcpuCnfMsg())
        {
            TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: TafAcpuCnfMsg is VOS_NULL_PTR!");

            /* 备份回复消息 */
            pucMsg = TAF_AGENT_SaveMsg((VOS_UINT8*)pstMsg,pstMsg->ulLength);

            TAF_AGENT_SetTafAcpuCnfMsg(pucMsg);

            TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

            /* 释放信号量，使得调用API任务继续运行 */
            ulResult = VOS_SmV(TAF_AGENT_GetTafAcpuCnfSem());

            TAFAGENT_NORMAL_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: VOS_SmV result is %x\n", ulResult);
        }
    }

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_ProcMsg!");

    return;
}


VOS_UINT32 TAF_AGENT_FindCidForDial(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                          *pCid
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_PS_FIND_CID_FOR_DIAL_CNF_STRU     *pstCnf;
    TAFAGENT_PS_FIND_CID_FOR_DIAL_REQ_STRU     *pstMsg;

    ulResult                = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_FindCidForDial!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_FindCidForDial: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_FIND_CID_FOR_DIAL_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                                    sizeof(TAFAGENT_PS_FIND_CID_FOR_DIAL_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FindCidForDial: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_FIND_CID_FOR_DIAL_REQ;

     /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FindCidForDial: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_FindCidForDial: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_FIND_CID_FOR_DIAL_CNF_STRU*)TAF_AGENT_GetTafAcpuCnfMsg();

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FindCidForDial: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_PS_FIND_CID_FOR_DIAL_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRet))
    {
        *pCid       = pstCnf->ucCid;
        ulResult    = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_FindCidForDial!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_GetPdpCidPara(
    TAF_PDP_PRIM_CONTEXT_STRU          *pstPdpPriPara,
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                          ulResult;
    TAFAGENT_PS_GET_CID_PARA_CNF_STRU  *pstCnf;
    TAFAGENT_PS_GET_CID_PARA_REQ_STRU  *pstMsg;

    ulResult                = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetPdpCidPara!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_GET_CID_PARA_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                              sizeof(TAFAGENT_PS_GET_CID_PARA_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_GET_CID_PARA_REQ;

    pstMsg->ucCid                       = ucCid;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_GET_CID_PARA_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_PS_GET_CID_PARA_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRet))
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstPdpPriPara, sizeof(TAF_PDP_PRIM_CONTEXT_STRU), (VOS_VOID*)&(pstCnf->stCidInfo),sizeof(TAF_PDP_PRIM_CONTEXT_STRU));
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetPdpCidPara!");

    return ulResult;

}



VOS_UINT32 TAF_AGENT_SetPdpCidQosPara(
    MN_CLIENT_ID_T                      usClientId,
    TAF_PS_PDP_QOS_SET_PARA_STRU       *pstQosPara
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_PS_SET_CID_QOS_PARA_CNF_STRU      *pstCnf;
    TAFAGENT_PS_SET_CID_QOS_PARA_REQ_STRU      *pstMsg;

    ulResult                = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_SetPdpCidQosPara!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_SetPdpCidQosPara: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_SET_CID_QOS_PARA_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_SET_CID_QOS_PARA_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_SetPdpCidQosPara: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->ulMsgId                     = ID_TAFAGENT_PS_SET_CID_QOS_PARA_REQ;

    pstMsg->usClientId                  = usClientId;

    TAF_MEM_CPY_S((VOS_VOID*)&(pstMsg->stQosPara), sizeof(pstMsg->stQosPara), (VOS_VOID*)pstQosPara, sizeof(TAF_PS_PDP_QOS_SET_PARA_STRU));

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_SetPdpCidQosPara: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_SetPdpCidQosPara: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_SET_CID_QOS_PARA_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_SetPdpCidQosPara: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_PS_SET_CID_QOS_PARA_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRet))
    {
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_SetPdpCidQosPara!");

    return ulResult;

}


VOS_UINT32 TAF_AGENT_GetPdpCidQosPara(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                           ucCid,
    TAF_PS_PDP_QOS_QUERY_PARA_STRU     *pstQosPara
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_PS_GET_CID_QOS_PARA_CNF_STRU      *pstCnf;
    TAFAGENT_PS_GET_CID_QOS_PARA_REQ_STRU      *pstMsg;

    ulResult                = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetPdpCidQosPara!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidQosPara: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_GET_CID_QOS_PARA_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_GET_CID_QOS_PARA_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidQosPara: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->ulMsgId                     = ID_TAFAGENT_PS_GET_CID_QOS_PARA_REQ;
    pstMsg->ucCid                       = ucCid;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidQosPara: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidQosPara: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_GET_CID_QOS_PARA_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidQosPara: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_PS_GET_CID_QOS_PARA_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRet))
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstQosPara, sizeof(TAF_PS_PDP_QOS_QUERY_PARA_STRU), (VOS_VOID*)&(pstCnf->stQosPara), sizeof(TAF_PS_PDP_QOS_QUERY_PARA_STRU));
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetPdpCidQosPara!");

    return ulResult;

}



VOS_UINT32 TAF_AGENT_GetCallInfoReq(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                          *pucCallNum,
    TAFAGERNT_MN_CALL_INFO_STRU        *pstCallInfos
)
{
    TAFAGERNT_MN_GET_CALL_INFO_CNF_STRU *pstCnf;
    VOS_UINT32                          ulResult;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetCallInfoReq!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 发送异步应用请求 */
    ulResult = MN_CALL_SendAppRequest(ID_TAFAGENT_MN_GET_CALL_INFO_REQ,
                                      usClientId, 0, 0,
                                      VOS_NULL_PTR);
    if (MN_ERR_NO_ERROR != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Malloc Msg Failed!");
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }


    ulResult      = VOS_ERR;
    pstCnf      = (TAFAGERNT_MN_GET_CALL_INFO_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }


    if (ID_TAFAGENT_MN_GET_CALL_INFO_CNF == pstCnf->enMsgId)
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstCallInfos,
                   (VOS_SIZE_T)(sizeof(TAFAGERNT_MN_CALL_INFO_STRU) * MN_CALL_MAX_NUM),
                   (VOS_VOID*)(pstCnf->stCallInfo),
                   (VOS_SIZE_T)(sizeof(TAFAGERNT_MN_CALL_INFO_STRU) * MN_CALL_MAX_NUM));

        *pucCallNum = pstCnf->ucNumOfCalls;

        ulResult      = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetCallInfoReq!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_GetSysMode(
    VOS_UINT16                                 usClientId,
    TAF_AGENT_SYS_MODE_STRU                   *pstSysMode
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_MTA_GET_SYSMODE_CNF_STRU          *pstCnf = VOS_NULL_PTR;
    TAFAGENT_MTA_GET_SYSMODE_REQ_STRU          *pstMsg = VOS_NULL_PTR;

    ulResult                = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetSysMode!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_MTA_GET_SYSMODE_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_MTA_GET_SYSMODE_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_MTA);

    pstMsg->enMsgId                     = ID_TAFAGENT_MTA_GET_SYSMODE_REQ;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_MTA_GET_SYSMODE_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_MTA_GET_SYSMODE_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRet))
    {
        pstSysMode->enRatType       = pstCnf->stSysMode.enRatType;
        pstSysMode->enSysSubMode    = pstCnf->stSysMode.enSysSubMode;
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetSysMode!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_GetAntState(
    VOS_UINT16                                 usClientId,
    VOS_UINT16                                *pusAntState
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_MTA_GET_ANT_STATE_CNF_STRU        *pstCnf = VOS_NULL_PTR;
    TAFAGENT_MTA_GET_ANT_STATE_REQ_STRU        *pstMsg = VOS_NULL_PTR;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetAntState!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_MTA_GET_ANT_STATE_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_MTA_GET_ANT_STATE_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_MTA);
    pstMsg->enMsgId                     = ID_TAFAGENT_MTA_GET_ANT_STATE_REQ;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_MTA_GET_ANT_STATE_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_MTA_GET_ANT_STATE_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulRslt))
    {
        *pusAntState    = pstCnf->usAntState;
        ulResult        = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetAntState!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_GetDataSystem(
    VOS_UINT16                                 usClientId,
    VOS_UINT8                                 *pucBitDataSystem
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_PS_GET_DATA_SYSTEM_CNF_STRU       *pstCnf = VOS_NULL_PTR;
    TAFAGENT_PS_GET_DATA_SYSTEM_REQ_STRU       *pstMsg = VOS_NULL_PTR;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetDataSystem!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetDataSystem: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_GET_DATA_SYSTEM_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_GET_DATA_SYSTEM_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetDataSystem: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_GET_DATA_SYS_REQ;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetDataSystem: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetDataSystem: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_GET_DATA_SYSTEM_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetDataSystem: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (ID_TAFAGENT_PS_GET_DATA_SYSTEM_CNF == pstCnf->enMsgId)
    {
        *pucBitDataSystem       = pstCnf->ucBitDataSystem;
        ulResult                = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetDataSystem!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_RegDataSysChgNtf(
    VOS_UINT16                                 usClientId,
    VOS_UINT8                                  ucCid
)
{
    VOS_UINT32                                  ulResult;
    TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_CNF_STRU  *pstCnf = VOS_NULL_PTR;
    TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_REQ_STRU  *pstMsg = VOS_NULL_PTR;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_RegDataSysChgNtf!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_RegDataSysChgNtf: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_RegDataSysChgNtf: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_REQ;
    pstMsg->ucCid                       = ucCid;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_RegDataSysChgNtf: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_RegDataSysChgNtf: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_RegDataSysChgNtf: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (ID_TAFAGENT_PS_REG_DATA_SYS_CHG_NTF_CNF == pstCnf->enMsgId)
    {
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_RegDataSysChgNtf!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_DeRegDataSysChgNtf(
    VOS_UINT16                                 usClientId,
    VOS_UINT8                                  ucCid
)
{
    VOS_UINT32                                      ulResult;
    TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_CNF_STRU    *pstCnf = VOS_NULL_PTR;
    TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_REQ_STRU    *pstMsg = VOS_NULL_PTR;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_DeRegDataSysChgNtf!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_DeRegDataSysChgNtf: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_DeRegDataSysChgNtf: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_REQ;
    pstMsg->ucCid                       = ucCid;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_DeRegDataSysChgNtf: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_DeRegDataSysChgNtf: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_DeRegDataSysChgNtf: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (ID_TAFAGENT_PS_DEREG_DATA_SYS_CHG_NTF_CNF == pstCnf->enMsgId)
    {
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_DeRegDataSysChgNtf!");

    return ulResult;
}



VOS_UINT32 TAF_AGENT_GetWlanApnThrotAllowed(
    VOS_UINT16                                 usClientId,
    VOS_UINT8                                  ucApnLen,
    VOS_UINT8                                 *pucApn,
    VOS_UINT32                                *pulAllowedFlg
)
{
    TAFAGENT_PS_APN_THROT_ALLOWED_CNF_STRU     *pstCnf = VOS_NULL_PTR;
    TAFAGENT_PS_APN_THROT_ALLOWED_REQ_STRU     *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetWlanApnThrotAllowed!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetWlanApnThrotAllowed: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_APN_THROT_ALLOWED_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_APN_THROT_ALLOWED_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetWlanApnThrotAllowed: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_APN_THROT_ALLOWED_REQ;
    pstMsg->ucApnLen                    = TAF_MIN(ucApnLen, TAF_MAX_APN_LEN);
    TAF_MEM_CPY_S(pstMsg->aucApn, sizeof(pstMsg->aucApn), pucApn, pstMsg->ucApnLen);

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetWlanApnThrotAllowed: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetWlanApnThrotAllowed: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_APN_THROT_ALLOWED_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetWlanApnThrotAllowed: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (ID_TAFAGENT_PS_APN_THROT_ALLOWED_CNF == pstCnf->enMsgId)
    {
        *pulAllowedFlg          = pstCnf->ulAllowedFlg;
        ulResult                = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetWlanApnThrotAllowed!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_FeedbackWlanApnThrotResult(
    VOS_UINT16                                 usClientId,
    VOS_UINT8                                  ucApnLen,
    VOS_UINT8                                 *pucApn,
    VOS_UINT32                                 ulCause,
    VOS_INT32                                  lNetworkReason
)
{
    TAFAGENT_PS_APN_THROT_FEEDBACK_CNF_STRU    *pstCnf = VOS_NULL_PTR;
    TAFAGENT_PS_APN_THROT_FEEDBACK_REQ_STRU    *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;

    ulResult = VOS_ERR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_FeedbackWlanApnThrotResult!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_FeedbackWlanApnThrotResult: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAFAGENT_PS_APN_THROT_FEEDBACK_REQ_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT,
                                                                               sizeof(TAFAGENT_PS_APN_THROT_FEEDBACK_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FeedbackWlanApnThrotResult: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstMsg->ulReceiverPid               = AT_GetDestPid(usClientId, I0_UEPS_PID_DSM);
    pstMsg->enMsgId                     = ID_TAFAGENT_PS_APN_THROT_FEEDBACK_REQ;
    pstMsg->ulApnLen                    = TAF_MIN(ucApnLen, TAF_MAX_APN_LEN);
    TAF_MEM_CPY_S(pstMsg->aucApn, sizeof(pstMsg->aucApn), pucApn, pstMsg->ulApnLen);
    pstMsg->ulCause                     = ulCause;
    pstMsg->lNetworkReason              = lNetworkReason;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstMsg);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FeedbackWlanApnThrotResult: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_FeedbackWlanApnThrotResult: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    ulResult    = VOS_ERR;
    pstCnf      = (TAFAGENT_PS_APN_THROT_FEEDBACK_CNF_STRU*)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_FeedbackWlanApnThrotResult: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (ID_TAFAGENT_PS_APN_THROT_FEEDBACK_CNF == pstCnf->enMsgId)
    {
        ulResult                = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_FeedbackWlanApnThrotResult!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_WriteACoreNv(
    TAF_WRITE_ACORE_NV_STRU            *pstWriteAcoreNv
)
{
    TAFAGENT_MTA_WRITE_ACORE_NV_REQ_STRU         *pstReq = VOS_NULL_PTR;
    TAFAGENT_MTA_WRITE_ACORE_NV_CNF_STRU         *pstCnf = VOS_NULL_PTR;
    VOS_UINT32                                    ulResult;
    VOS_UINT32                                    ulMsglen;

    ulResult = NV_WRITE_UNABLE;

    if (4 > pstWriteAcoreNv->ulNvLength)
    {
        ulMsglen = sizeof(TAFAGENT_MTA_WRITE_ACORE_NV_REQ_STRU);
    }
    else
    {
        ulMsglen = sizeof(TAFAGENT_MTA_WRITE_ACORE_NV_REQ_STRU) - 4 + pstWriteAcoreNv->ulNvLength;
    }

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_WriteACoreNv!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return NV_WRITE_UNABLE;
    }

    /* 构造消息 */
    /*lint -save -e516*/
    pstReq = (TAFAGENT_MTA_WRITE_ACORE_NV_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT, ulMsglen);
    if (VOS_NULL_PTR == pstReq)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }

    /* 填写消息头 */
    if ( MODEM_ID_0 == pstWriteAcoreNv->ulModemId )
    {
        pstReq->ulReceiverPid = I0_UEPS_PID_MTA;
    }
    else if ( MODEM_ID_1 == pstWriteAcoreNv->ulModemId )
    {
        pstReq->ulReceiverPid = I1_UEPS_PID_MTA;
    }
    else
    {
        pstReq->ulReceiverPid = I2_UEPS_PID_MTA;
    }

    pstReq->enMsgId       = ID_TAFAGENT_MTA_WRITE_ACORE_NV_REQ;
    pstReq->ulNvItemId    = pstWriteAcoreNv->ulNvItemId;
    pstReq->ulNvLength    = pstWriteAcoreNv->ulNvLength;
    pstReq->ulIsNeedCheck = pstWriteAcoreNv->ulIsNeedCheck;
    pstReq->ulOffset      = pstWriteAcoreNv->ulOffset;

    TAF_MEM_CPY_S(pstReq->aucData, ulMsglen - (sizeof(TAFAGENT_MTA_WRITE_ACORE_NV_REQ_STRU) - sizeof(pstReq->aucData)),
                  pstWriteAcoreNv->pData, pstWriteAcoreNv->ulNvLength);

     /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstReq);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }
    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }


    ulResult = NV_WRITE_UNABLE;
    pstCnf   = (TAFAGENT_MTA_WRITE_ACORE_NV_CNF_STRU*)TAF_AGENT_GetTafAcpuCnfMsg();

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }

    if ((ID_TAFAGENT_MTA_WRITE_ACORE_NV_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulResult))
    {
        ulResult = NV_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_WriteACoreNv!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_ProcACoreNv(
    TAFAGENT_MTA_PROC_ACORE_NV_TPYE_ENUM_UINT32             enProcACoreNvType
)
{
    TAFAGENT_MTA_PROC_ACORE_NV_REQ_STRU          *pstReq = VOS_NULL_PTR;
    TAFAGENT_MTA_PROC_ACORE_NV_CNF_STRU          *pstCnf = VOS_NULL_PTR;
    VOS_UINT32                                    ulResult;
    VOS_UINT32                                    ulMsglen;

    ulResult = VOS_ERR;

    ulMsglen = sizeof(TAFAGENT_MTA_PROC_ACORE_NV_REQ_STRU);

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_ProcACoreNv!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。*/
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Lock Mutex SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    /*lint -save -e516*/
    pstReq = (TAFAGENT_MTA_PROC_ACORE_NV_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_TAFAGENT, ulMsglen);
    if (VOS_NULL_PTR == pstReq)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    pstReq->ulReceiverPid     = I0_UEPS_PID_MTA;
    pstReq->enMsgId           = ID_TAFAGENT_MTA_PROC_ACORE_NV_REQ;
    pstReq->enProcACoreNvType = enProcACoreNvType;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    ulResult = PS_SEND_MSG(ACPU_PID_TAFAGENT, pstReq);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    ulResult = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (VOS_OK != ulResult)
    {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Lock Binary SEM Failed! VOS_SmP result is %x\n", ulResult);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(ulResult);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }


    ulResult = VOS_ERR;
    pstCnf   = (TAFAGENT_MTA_PROC_ACORE_NV_CNF_STRU*)TAF_AGENT_GetTafAcpuCnfMsg();

    if (VOS_NULL_PTR == pstCnf)
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((ID_TAFAGENT_MTA_PROC_ACORE_NV_CNF == pstCnf->enMsgId)
     && (VOS_OK == pstCnf->ulResult))
    {
        ulResult = VOS_OK;
    }
    else
    {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_ProcACoreNv!");

    return ulResult;
}


VOS_UINT32 TAF_AGENT_FidInit (enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32                          ulRslt;

    switch(enPhase)
    {
        case VOS_IP_LOAD_CONFIG:

            /* 网卡模块注册PID */
            ulRslt = VOS_RegisterPIDInfo(ACPU_PID_TAFAGENT,
                                        (Init_Fun_Type)TAF_AGENT_PidInit,
                                        (Msg_Fun_Type)TAF_AGENT_ProcMsg);
            if( VOS_OK != ulRslt )
            {
                TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "reg ACPU_PID_TAFAGENT VOS_RegisterPIDInfo FAIL! ulRslt: %d\n", ulRslt);

                return VOS_ERR;
            }

            ulRslt = VOS_RegisterTaskPrio(ACPU_FID_TAFAGENT, TAF_AGENT_TASK_PRIORITY);
            if( VOS_OK != ulRslt )
            {
                return VOS_ERR;
            }
            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
        case VOS_IP_BUTT:
            break;

        default:
            break;
    }

    return VOS_OK;
}    /* TAF_AGENT_FidInit */


