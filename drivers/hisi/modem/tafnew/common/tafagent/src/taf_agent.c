/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#define THIS_FILE_ID PS_FILE_ID_TAF_AGENT_C

#include "taf_agent.h"
#include "taf_agent_ctx.h"
#include "taf_agent_log.h"
#include "taf_agent_interface.h"
#include "taf_agent_oam_interface.h"
#include "taf_ccm_api.h"
#include "securec.h"
#include "mn_comm_api.h"
#include "tafagent_msg_chk.h"


extern VOS_UINT32 AT_GetDestPid(MN_CLIENT_ID_T clientId, VOS_UINT32 rcvPid);

#if (VOS_WIN32 == VOS_OS_VER)
extern VOS_VOID STUB_SetTafAgentMtaWriteACoreNvCnf(TAF_WRITE_AcoreNv *writeAcoreNvPtr);
extern VOS_VOID STUB_SetTafAgentMtaProcACoreNvCnf(VOS_VOID);
extern VOS_VOID STUB_SetTafAgentMtaReadACoreNvCnf(VOS_UINT32 modemId, VOS_UINT32 nvItemId, VOS_UINT8 *dataPtr,
                                                  VOS_UINT32 length);
extern VOS_VOID STUB_SetTafAgentCcmGetCallInfoCnf(VOS_VOID);
#endif


VOS_UINT32 TAF_AGENT_PidInit(enum VOS_InitPhaseDefine phase)
{
    switch (phase) {
        case VOS_IP_INITIAL:
            return Taf_Agent_InitCtx();

        default:
            break;
    }

    return VOS_OK;
}

VOS_UINT32 TAF_AGENT_IsValidMsg(MsgBlock *msg)
{
    MSG_Header *msgHeader = VOS_NULL_PTR;

    if (msg == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_IsValidMsg: Msg is invalid!");
        return VOS_ERR;
    }

    msgHeader = (MSG_Header *)msg;

    if (msgHeader->msgName == ID_TAF_CCM_QRY_CALL_INFO_CNF) {
        return VOS_OK;
    }

    if ((VOS_GET_SENDER_ID(msgHeader) == I0_WUEPS_PID_TAF) || (VOS_GET_SENDER_ID(msgHeader) == I1_WUEPS_PID_TAF) ||
        (VOS_GET_SENDER_ID(msgHeader) == I2_WUEPS_PID_TAF) || (VOS_GET_SENDER_ID(msgHeader) == I0_UEPS_PID_MTA) ||
        (VOS_GET_SENDER_ID(msgHeader) == I1_UEPS_PID_MTA) || (VOS_GET_SENDER_ID(msgHeader) == I2_UEPS_PID_MTA) ||
        (VOS_GET_SENDER_ID(msgHeader) == I0_UEPS_PID_DSM) || (VOS_GET_SENDER_ID(msgHeader) == I1_UEPS_PID_DSM) ||
        (VOS_GET_SENDER_ID(msgHeader) == I2_UEPS_PID_DSM)) {
        if ((msgHeader->msgName >= ID_TAFAGENT_PS_GET_CID_PARA_CNF) &&
            (msgHeader->msgName < ID_TAFAGENT_MSG_ID_ENUM_BUTT)) {
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_VOID TAF_AGENT_ClearAllSem(VOS_VOID)
{
    /* 如果有锁的存在 */
    if (TAF_AGENT_GET_ACPU_CNF_SEM_LOCK_FLG() == VOS_TRUE) {
        VOS_SmV(TAF_AGENT_GetTafAcpuCnfSem());
    }
}

VOS_VOID TAF_AGENT_ProcMsg(MsgBlock *msg)
{
    VOS_UINT8 *msgTemp = VOS_NULL_PTR;
    VOS_UINT32 result = VOS_OK;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_ProcMsg!");

    if (TAF_AGENT_IsValidMsg(msg) != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_IsValidMsg: Msg invalid!");
        return;
    }

    if (TAF_AGENT_ChkTafAgentFidRcvMsgLen(msg) != VOS_TRUE) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_IsValidMsg: Msg len is invalid!");
        return;
    }

    TAFAGENT_NORMAL_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: Msg Name is %x\n", ((MSG_Header *)msg)->msgName);

    if (TAF_AGENT_GET_ACPU_CNF_SEM_LOCK_FLG() == VOS_FALSE) {
        TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: SemLockedFlg is False!");
        return;
    }

    /* 判断消息是否被释放 */
    if (TAF_AGENT_GetTafAcpuCnfMsg() == VOS_NULL_PTR) {
        TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: TafAcpuCnfMsg is VOS_NULL_PTR!");

        /* 备份回复消息 */
        msgTemp = TAF_AGENT_SaveMsg((VOS_UINT8 *)msg, VOS_GET_MSG_LEN(msg));

        TAF_AGENT_SetTafAcpuCnfMsg(msgTemp);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        /* 释放信号量，使得调用API任务继续运行 */
        result = VOS_SmV(TAF_AGENT_GetTafAcpuCnfSem());

        TAFAGENT_NORMAL_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcMsg: VOS_SmV result is %x\n", result);
    }

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_ProcMsg!");
}

VOS_UINT32 TAF_AGENT_GetPdpCidPara(TAF_PDP_PrimContext *pdpPriPara, MN_CLIENT_ID_T clientId, VOS_UINT8 cid)
{
    errno_t                    memResult;
    VOS_UINT32                 result = VOS_ERR;
    TAFAGENT_PS_GetCidParaCnf *cnf = VOS_NULL_PTR;
    TAFAGENT_PS_GetCidParaReq *msg = VOS_NULL_PTR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetPdpCidPara!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Lock Mutex SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    msg = (TAFAGENT_PS_GetCidParaReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT,
                                                                    sizeof(TAFAGENT_PS_GetCidParaReq));
    if (msg == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, ACPU_PID_TAFAGENT, AT_GetDestPid(clientId, I0_UEPS_PID_DSM),
                  sizeof(TAFAGENT_PS_GetCidParaReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgId = ID_TAFAGENT_PS_GET_CID_PARA_REQ;

    msg->cid = cid;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, msg);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT,
                            "TAF_AGENT_GetPdpCidPara: Lock Binary SEM Failed! VOS_SmP result is %x\n", result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    result = VOS_ERR;
    cnf    = (TAFAGENT_PS_GetCidParaCnf *)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetPdpCidPara: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((cnf->msgId == ID_TAFAGENT_PS_GET_CID_PARA_CNF) && (cnf->ret == VOS_OK)) {
        memResult = memcpy_s((VOS_VOID *)pdpPriPara, sizeof(TAF_PDP_PrimContext), (VOS_VOID *)&(cnf->cidInfo),
                             sizeof(TAF_PDP_PrimContext));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_PDP_PrimContext), sizeof(TAF_PDP_PrimContext));
        result = VOS_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetPdpCidPara!");

    return result;
}

VOS_UINT32 TAF_AGENT_GetCallInfoReq(MN_CLIENT_ID_T clientId, VOS_UINT8 *callNum, TAFAGENT_CALL_InfoParam *callInfos)
{
    TAFAGENT_CCM_QryCallInfoCnf *cnf = VOS_NULL_PTR;
    TAF_Ctrl                     ctrl = {0};
    TAF_CALL_QryCallInfoReqPara  qryCallInfoPara = {0};
    VOS_UINT32                   result = VOS_ERR;
    errno_t                      memResult;
    ModemIdUint16                modemId;

    qryCallInfoPara.getCallInfoType = TAF_CALL_GET_CALL_INFO_TYPE_ATA;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetCallInfoReq!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT,
                            "TAF_AGENT_GetCallInfoReq: Lock Mutex SEM Failed! VOS_SmP result is %x\n", result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    if (AT_GetModemIdFromClient(clientId, &modemId) != VOS_OK) {
        return VOS_ERR;
    }

    ctrl.moduleId = ACPU_PID_TAFAGENT;
    ctrl.clientId = clientId;
    ctrl.opId     = 0;

    /* TAFAGENT向CCM发送呼叫消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &qryCallInfoPara, ID_TAF_CCM_QRY_CALL_INFO_REQ, sizeof(qryCallInfoPara),
                                   modemId);

    if (result != MN_ERR_NO_ERROR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Malloc Msg Failed!");
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT,
                            "TAF_AGENT_GetCallInfoReq: Lock Binary SEM Failed! VOS_SmP result is %x\n", result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    result = VOS_ERR;

    cnf = (TAFAGENT_CCM_QryCallInfoCnf *)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetCallInfoReq: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if (cnf->msgName == ID_TAF_CCM_QRY_CALL_INFO_CNF) {
        memResult = memcpy_s((VOS_VOID *)callInfos, (VOS_SIZE_T)(sizeof(TAFAGENT_CALL_InfoParam) * MN_CALL_MAX_NUM),
                             (VOS_VOID *)(cnf->tafAgentCallInfo.callInfos),
                             (VOS_SIZE_T)(sizeof(TAFAGENT_CALL_InfoParam) * MN_CALL_MAX_NUM));
        TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)(sizeof(TAFAGENT_CALL_InfoParam) * MN_CALL_MAX_NUM),
                            (VOS_SIZE_T)(sizeof(TAFAGENT_CALL_InfoParam) * MN_CALL_MAX_NUM));

        *callNum = cnf->tafAgentCallInfo.numOfCalls;

        result = VOS_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetCallInfoReq!");

    return result;
}

VOS_UINT32 TAF_AGENT_GetSysMode(VOS_UINT16 clientId, TAF_AGENT_SysMode *sysMode)
{
    VOS_UINT32                  result = VOS_ERR;
    TAFAGENT_MTA_GetSysmodeCnf *cnf = VOS_NULL_PTR;
    TAFAGENT_MTA_GetSysmodeReq *msg = VOS_NULL_PTR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetSysMode!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Lock Mutex SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    msg = (TAFAGENT_MTA_GetSysmodeReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT,
                                                                     sizeof(TAFAGENT_MTA_GetSysmodeReq));
    if (msg == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, ACPU_PID_TAFAGENT, AT_GetDestPid(clientId, I0_UEPS_PID_MTA),
                  sizeof(TAFAGENT_MTA_GetSysmodeReq) - VOS_MSG_HEAD_LENGTH);

    msg->msgId = ID_TAFAGENT_MTA_GET_SYSMODE_REQ;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, msg);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Lock Binary SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    result = VOS_ERR;
    cnf    = (TAFAGENT_MTA_GetSysmodeCnf *)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetSysMode: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((cnf->msgId == ID_TAFAGENT_MTA_GET_SYSMODE_CNF) && (cnf->ret == VOS_OK)) {
        sysMode->ratType    = cnf->sysMode.ratType;
        sysMode->sysSubMode = cnf->sysMode.sysSubMode;
        result              = VOS_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetSysMode!");

    return result;
}

VOS_UINT32 TAF_AGENT_GetAntState(VOS_UINT16 clientId, VOS_UINT16 *antState)
{
    VOS_UINT32                   result = VOS_ERR;
    TAFAGENT_MTA_GetAntStateCnf *cnf = VOS_NULL_PTR;
    TAFAGENT_MTA_GetAntStateReq *msg = VOS_NULL_PTR;

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_GetAntState!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Lock Mutex SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    msg = (TAFAGENT_MTA_GetAntStateReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT,
                                                                      sizeof(TAFAGENT_MTA_GetAntStateReq));
    if (msg == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, ACPU_PID_TAFAGENT, AT_GetDestPid(clientId, I0_UEPS_PID_MTA),
                  sizeof(TAFAGENT_MTA_GetAntStateReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgId = ID_TAFAGENT_MTA_GET_ANT_STATE_REQ;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, msg);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Lock Binary SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    result = VOS_ERR;
    cnf    = (TAFAGENT_MTA_GetAntStateCnf *)(TAF_AGENT_GetTafAcpuCnfMsg());

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_GetAntState: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((cnf->msgId == ID_TAFAGENT_MTA_GET_ANT_STATE_CNF) && (cnf->rslt == VOS_OK)) {
        *antState = cnf->antState;
        result    = VOS_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_GetAntState!");

    return result;
}

VOS_UINT32 TAF_AGENT_WriteACoreNv(TAF_WRITE_AcoreNv *writeAcoreNv)
{
    TAFAGENT_MTA_WriteAcoreNvReq *req = VOS_NULL_PTR;
    TAFAGENT_MTA_WriteAcoreNvCnf *cnf = VOS_NULL_PTR;
    VOS_UINT32                    result = NV_WRITE_UNABLE;
    VOS_UINT32                    msglen;
    errno_t                       memResult;

    AT_PR_LOGH("TAF_AGENT_WriteACoreNv Enter");

    if (writeAcoreNv->nvLength < 4) {
        msglen = sizeof(TAFAGENT_MTA_WriteAcoreNvReq);
    } else {
        msglen = sizeof(TAFAGENT_MTA_WriteAcoreNvReq) - 4 + writeAcoreNv->nvLength;
    }

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_WriteACoreNv!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Lock Mutex SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return NV_WRITE_UNABLE;
    }

    /* 构造消息 */
    /*lint -save -e516*/
    req = (TAFAGENT_MTA_WriteAcoreNvReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT, msglen);
    if (req == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }

    /* 填写消息头 */
#if (1 < MULTI_MODEM_NUMBER)
    if (writeAcoreNv->modemId == MODEM_ID_0) {
        TAF_CfgMsgHdr((MsgBlock *)req, ACPU_PID_TAFAGENT, I0_UEPS_PID_MTA, msglen - VOS_MSG_HEAD_LENGTH);
    } else if (writeAcoreNv->modemId == MODEM_ID_1) {
        TAF_CfgMsgHdr((MsgBlock *)req, ACPU_PID_TAFAGENT, I1_UEPS_PID_MTA, msglen - VOS_MSG_HEAD_LENGTH);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)req, ACPU_PID_TAFAGENT, I2_UEPS_PID_MTA, msglen - VOS_MSG_HEAD_LENGTH);
    }
#else
    TAF_CfgMsgHdr((MsgBlock *)req, ACPU_PID_TAFAGENT, I0_UEPS_PID_MTA, msglen - VOS_MSG_HEAD_LENGTH);
#endif

    req->msgId       = ID_TAFAGENT_MTA_WRITE_ACORE_NV_REQ;
    req->nvItemId    = writeAcoreNv->nvItemId;
    req->nvLength    = writeAcoreNv->nvLength;
    req->isNeedCheck = writeAcoreNv->isNeedCheck;
    req->offset      = writeAcoreNv->offset;

    if (writeAcoreNv->nvLength > 0) {
        memResult = memcpy_s(req->data, msglen - (sizeof(TAFAGENT_MTA_WriteAcoreNvReq) - sizeof(req->data)),
                             writeAcoreNv->data, writeAcoreNv->nvLength);
        TAF_MEM_CHK_RTN_VAL(memResult, msglen - (sizeof(TAFAGENT_MTA_WriteAcoreNvReq) - sizeof(req->data)),
                            writeAcoreNv->nvLength);
    }

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, req);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }
    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Lock Binary SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    STUB_SetTafAgentMtaWriteACoreNvCnf(writeAcoreNv);
#endif

    result = NV_WRITE_UNABLE;
    cnf    = (TAFAGENT_MTA_WriteAcoreNvCnf *)TAF_AGENT_GetTafAcpuCnfMsg();

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_WriteACoreNv: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return NV_WRITE_UNABLE;
    }

    if ((cnf->msgId == ID_TAFAGENT_MTA_WRITE_ACORE_NV_CNF) && (cnf->result == VOS_OK)) {
        result = NV_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_WriteACoreNv!");

    AT_PR_LOGH("TAF_AGENT_WriteACoreNv Exit");

    return result;
}

VOS_UINT32 TAF_AGENT_ReadACoreEncryptionNv(VOS_UINT32 modemId, VOS_UINT32 nvItemId, VOS_UINT8 *dataPtr,
                                           VOS_UINT32 length)
{
    TAFAGENT_MTA_ReadAcoreEncryptionNvReq *reqPtr = VOS_NULL_PTR;
    TAFAGENT_MTA_ReadAcoreEncryptionNvCnf *cnfPtr = VOS_NULL_PTR;
    VOS_UINT32                             result = VOS_OK;
    VOS_UINT32                             ret;
    VOS_UINT32                             reqMsglen;
    errno_t                                err;
    AT_PR_LOGH("TAF_AGENT_ReadACoreEncryptionNv Enter");
    reqMsglen = sizeof(TAFAGENT_MTA_ReadAcoreEncryptionNvReq);
    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_ReadACoreEncryptionNv!");
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);

    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ReadACoreEncryptionNv:LockMutexFail! result is %x\n", result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return NV_READ_UNABLE;
    }

    reqPtr = (TAFAGENT_MTA_ReadAcoreEncryptionNvReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT, reqMsglen);
    if (reqPtr == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ReadACoreEncryptionNv: Malloc Msg Failed!");
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());
        return NV_READ_UNABLE;
    }
#if (1 < MULTI_MODEM_NUMBER)
    if (modemId == MODEM_ID_0) {
        TAF_CfgMsgHdr((MsgBlock *)reqPtr, ACPU_PID_TAFAGENT, I0_UEPS_PID_MTA, reqMsglen - VOS_MSG_HEAD_LENGTH);
    } else if (modemId == MODEM_ID_1) {
        TAF_CfgMsgHdr((MsgBlock *)reqPtr, ACPU_PID_TAFAGENT, I1_UEPS_PID_MTA, reqMsglen - VOS_MSG_HEAD_LENGTH);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)reqPtr, ACPU_PID_TAFAGENT, I2_UEPS_PID_MTA, reqMsglen - VOS_MSG_HEAD_LENGTH);
    }
#else
    TAF_CfgMsgHdr((MsgBlock *)reqPtr, ACPU_PID_TAFAGENT, I0_UEPS_PID_MTA, reqMsglen - VOS_MSG_HEAD_LENGTH);
#endif
    reqPtr->msgId    = ID_TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_REQ;
    reqPtr->nvItemId = nvItemId;
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);
    TAF_AGENT_ClearMsg();
    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, reqPtr);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ReadACoreEncryptionNv: Send Msg Failed!");
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());
        return NV_READ_UNABLE;
    }
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ReadACoreEncryptionNv:LockBinaryFail");
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());
        return NV_READ_UNABLE;
    }
#if (VOS_WIN32 == VOS_OS_VER)
    STUB_SetTafAgentMtaReadACoreNvCnf(modemId, nvItemId, dataPtr, length);
#endif
    cnfPtr = (TAFAGENT_MTA_ReadAcoreEncryptionNvCnf *)TAF_AGENT_GetTafAcpuCnfMsg();
    if (cnfPtr == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ReadACoreEncryptionNv: Msg Cnf is VOS_NULL_PTR!");
        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);
        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());
        return NV_READ_UNABLE;
    }

    if ((cnfPtr->msgId == ID_TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF) && (cnfPtr->result == VOS_OK) &&
        (cnfPtr->nvLength > 0)) {
        err = memcpy_s(dataPtr, length, cnfPtr->dataArray, cnfPtr->nvLength);
        TAF_MEM_CHK_RTN_VAL(err, length, cnfPtr->nvLength);
        ret = NV_OK;
    } else {
        ret = NV_READ_UNABLE;
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());
    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_ReadACoreEncryptionNv!");
    AT_PR_LOGH("TAF_AGENT_ReadACoreEncryptionNv Exit");

    return ret;
}

VOS_UINT32 TAF_AGENT_ProcACoreNv(TAFAGENT_MTA_ProcAcoreNvTpyeUint32 procACoreNvType)
{
    TAFAGENT_MTA_ProcAcoreNvReq *req = VOS_NULL_PTR;
    TAFAGENT_MTA_ProcAcoreNvCnf *cnf = VOS_NULL_PTR;
    VOS_UINT32                   result = VOS_ERR;
    VOS_UINT32                   msglen;

    msglen = sizeof(TAFAGENT_MTA_ProcAcoreNvReq);

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "ENTER TAF_AGENT_ProcACoreNv!");

    /* 如果同步信号量已锁，挂起任务，依次进入等待队列；如果同步信号量未锁，锁信号量。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuSyncSem(), 0);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Lock Mutex SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* 构造消息 */
    /*lint -save -e516*/
    req = (TAFAGENT_MTA_ProcAcoreNvReq *)TAF_AllocMsgWithHeaderLen(ACPU_PID_TAFAGENT, msglen);
    if (req == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Malloc Msg Failed!");

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)req, ACPU_PID_TAFAGENT, I0_UEPS_PID_MTA, msglen - VOS_MSG_HEAD_LENGTH);
    req->msgId           = ID_TAFAGENT_MTA_PROC_ACORE_NV_REQ;
    req->procACoreNvType = procACoreNvType;

    /* 设置信号量 */
    TAF_AGENT_SetTafAcpuCnfMsg(VOS_NULL_PTR);

    TAF_AGENT_ClearMsg();

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_TRUE);

    /* 将请求消息发送给CCPU */
    result = TAF_TraceAndSendMsg(ACPU_PID_TAFAGENT, req);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Send Msg Failed!");

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);
        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    /* 等待回复信号量初始为锁状态，等待CCPU的回复后信号量解锁。 */
    result = VOS_SmP(TAF_AGENT_GetTafAcpuCnfSem(), PS_SYNC_CNF_TIMEOUT_LEN);
    if (result != VOS_OK) {
        TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Lock Binary SEM Failed! VOS_SmP result is %x\n",
                            result);
        TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(result);
        TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    STUB_SetTafAgentMtaProcACoreNvCnf();
#endif

    result = VOS_ERR;
    cnf    = (TAFAGENT_MTA_ProcAcoreNvCnf *)TAF_AGENT_GetTafAcpuCnfMsg();

    if (cnf == VOS_NULL_PTR) {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "TAF_AGENT_ProcACoreNv: Msg Cnf is VOS_NULL_PTR!");

        TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(1);

        TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

        VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

        return VOS_ERR;
    }

    if ((cnf->msgId == ID_TAFAGENT_MTA_PROC_ACORE_NV_CNF) && (cnf->result == VOS_OK)) {
        result = VOS_OK;
    } else {
        TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(1);
    }

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    VOS_SmV(TAF_AGENT_GetTafAcpuSyncSem());

    TAFAGENT_NORMAL_LOG(ACPU_PID_TAFAGENT, "EXIT TAF_AGENT_ProcACoreNv!");

    return result;
}

MODULE_EXPORTED VOS_UINT32 TAF_AGENT_FidInit(enum VOS_InitPhaseDefine phase)
{
    VOS_UINT32 rslt;

    switch (phase) {
        case VOS_IP_LOAD_CONFIG:

            /* 网卡模块注册PID */
            rslt = VOS_RegisterPIDInfo(ACPU_PID_TAFAGENT, (InitFunType)TAF_AGENT_PidInit,
                                       (MsgFunType)TAF_AGENT_ProcMsg);
            if (rslt != VOS_OK) {
                TAFAGENT_ERROR_LOG1(ACPU_PID_TAFAGENT, "reg ACPU_PID_TAFAGENT VOS_RegisterPIDInfo FAIL! ulRslt: %d\n",
                                    rslt);

                return VOS_ERR;
            }

            rslt = VOS_RegisterTaskPrio(ACPU_FID_TAFAGENT, TAF_AGENT_TASK_PRIORITY);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
            if (VOS_RegisterFidTaskCoreBind(ACPU_FID_TAFAGENT, VOS_CORE_MASK_CORE0) != VOS_OK) {
                return VOS_ERR;
            }
#endif

            TAF_AGENT_SortTafAgentFidChkMsgLenTbl();
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
} /* TAF_AGENT_FidInit */

