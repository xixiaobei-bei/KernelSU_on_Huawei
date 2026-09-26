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

#include "product_config.h"
#include "vos.h"
#include "mn_error_code.h"
#include "mn_msg_api.h"
#include "at_cmd_proc.h"
#include "securec.h"
#include "mn_comm_api.h"


#define THIS_FILE_ID PS_FILE_ID_MNMSG_API_C

/* API发送的消息类型和长度用一个二维数组对应起来 */
typedef struct {
    MN_MSG_MsgtypeUint16 msgType;
    VOS_UINT16           len;
} MSG_MSGTYPE_LEN_STRU;

static const  MSG_MSGTYPE_LEN_STRU g_msgTypeLen[] = {
    { MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT, sizeof(MN_MSG_SendParm) },
    { MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM, sizeof(MN_MSG_SendFrommemParm) },
    { MN_MSG_MSGTYPE_SEND_RPRPT, sizeof(MN_MSG_SendAckParm) },
    { MN_MSG_MSGTYPE_WRITE, sizeof(MN_MSG_WriteParm) },
    { MN_MSG_MSGTYPE_READ, sizeof(MN_MSG_ReadParm) },
    { MN_MSG_MSGTYPE_LIST, sizeof(MN_MSG_ListParm) },
    { MN_MSG_MSGTYPE_DELETE, sizeof(MN_MSG_DeleteParam) },
    { MN_MSG_MSGTYPE_DELETE_TEST, sizeof(MN_MSG_ListParm) },
    { MN_MSG_MSGTYPE_WRITE_SRV_PARA, sizeof(MN_MSG_WriteSrvParameter) },
    { MN_MSG_MSGTYPE_READ_SRV_PARA, sizeof(MN_MSG_ReadCommParam) },
    { MN_MSG_MSGTYPE_DELETE_SRV_PARA, sizeof(MN_MSG_DeleteParam) },
    { MN_MSG_MSGTYPE_MODIFY_STATUS, sizeof(MN_MSG_ModifyStatusParm) },
    { MN_MSG_MSGTYPE_SET_RCVPATH, sizeof(MN_MSG_SetRcvmsgPathParm) },
    { MN_MSG_MSGTYPE_GET_RCVPATH, sizeof(MN_MSG_GetRcvmsgPathParm) },
    { MN_MSG_MSGTYPE_GET_STORAGESTATUS, sizeof(MN_MSG_GetStorageStatusParm) },
    { MN_MSG_MSGTYPE_READ_STARPT, sizeof(MN_MSG_ReadCommParam) },
    { MN_MSG_MSGTYPE_DELETE_STARPT, sizeof(MN_MSG_DeleteParam) },
    { MN_MSG_MSGTYPE_GET_STARPT_FROMMOMSG, sizeof(MN_MSG_StarptMomsgParm) },
    { MN_MSG_MSGTYPE_GET_MOMSG_FROMSTARPT, sizeof(MN_MSG_StarptMomsgParm) },
    { MN_MSG_MSGTYPE_SET_MEMSTATUS, sizeof(MN_MSG_SetMemstatusParm) },
    { MN_MSG_MSGTYPE_SET_RELAY_LINK_CTRL, sizeof(MN_MSG_LinkCtrlType) },
    { MN_MSG_MSGTYPE_GET_RELAY_LINK_CTRL, 0 },
    { MN_MSG_MSGTYPE_STUB, sizeof(MN_MSG_StubMsg) },
#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))
    { MN_MSG_MSGTYPE_GET_ALLCBMIDS, 0 },
    { MN_MSG_MSGTYPE_ADD_CBMIDS, sizeof(TAF_CBA_CbmiRangeList) },
    { MN_MSG_MSGTYPE_DELETE_CBMIDS, sizeof(TAF_CBA_CbmiRangeList) },
    { MN_MSG_MSGTYPE_EMPTY_CBMIDS, 0 },
#endif
    { MN_MSG_MSGTYPE_SET_SMS_SEND_DOMAIN, sizeof(MN_MSG_SmsSendDomainType) },
    { MN_MSG_MSGTYPE_GET_SMS_SEND_DOMAIN, 0 },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { MN_MSG_MSGTYPE_ABORT_RPDATA_DIRECT, 0 }
#endif
};

LOCAL MN_MSG_TsDataInfo g_msgDataInfo;

VOS_UINT32 MSG_SendAppReq(MN_MSG_MsgtypeUint16 msgType, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                          const VOS_VOID *sendParm)
{
    MN_APP_ReqMsg *appReq = VOS_NULL_PTR;
    VOS_UINT32     len;
    VOS_UINT32     ret;
    VOS_UINT32     sendLen;
    VOS_UINT32     loop;
    VOS_UINT32     maxLoop;
    errno_t        memResult;

    /* 消息类型有效性判断 */
    if ((msgType >= MN_MSG_MSGTYPE_MAX) || (msgType <= MN_APP_MSG_CLASS_MSG)) {
        AT_WARN_LOG("MSG_SendAppReq:Invalid enMsgType. ");
        return MN_ERR_INVALIDPARM;
    }

    /* 获取消息信息字段长度 */
    maxLoop = sizeof(g_msgTypeLen) / sizeof(MSG_MSGTYPE_LEN_STRU);
    for (loop = 0; loop < maxLoop; loop++) {
        if (msgType == g_msgTypeLen[loop].msgType) {
            break;
        }
    }

    if (loop == maxLoop) {
        return MN_ERR_CLASS_SMS_INTERNAL;
    }

    len = g_msgTypeLen[loop].len;

    sendLen = (len + sizeof(MN_APP_ReqMsg)) - (sizeof(VOS_UINT8) * 4);

    /* 填写并发送消息 */
    appReq = (MN_APP_ReqMsg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sendLen);

    if (appReq == VOS_NULL_PTR) {
        AT_WARN_LOG("MSG_SendAppReq:Alloc Msg Failed");
        return MN_ERR_NOMEM;
    }

    TAF_CfgMsgHdr((MsgBlock *)appReq, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_TAF),
                  sendLen - VOS_MSG_HEAD_LENGTH);
    appReq->msgName     = msgType;
    appReq->reserve1[0] = 0;
    appReq->reserve1[1] = 0;
    appReq->clientId    = clientId;
    appReq->opId        = opId;
    appReq->reserve2[0] = 0;

    if ((len > 0) && (sendParm != VOS_NULL_PTR)) {
        memResult = memcpy_s(appReq->content, len, (VOS_UINT8 *)sendParm, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }
    ret = TAF_TraceAndSendMsg(WUEPS_PID_AT, appReq);
    if (ret != VOS_OK) {
        AT_WARN_LOG("MSG_SendAppReq: PS_SEND_MSG fails");
        return MN_ERR_SEND_MSG_ERROR;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_ReqStub(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_StubMsg *stubParam)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (stubParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_ReqStub: pstSetParam is Null ");
        return MN_ERR_NULLPTR;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_STUB, clientId, opId, (VOS_VOID *)stubParam);

    return ret;
}

VOS_UINT32 MN_MSG_SetLinkCtrl(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_LinkCtrlType *setParam)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (setParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SetLinkCtrl:pstSetParam is Null ");
        return MN_ERR_NULLPTR;
    }

    if (setParam->relayLinkCtrl >= MN_MSG_LINK_CTRL_BUTT) {
        AT_WARN_LOG("MN_MSG_SetLinkCtrl:Invalid value");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SET_RELAY_LINK_CTRL, clientId, opId, (VOS_VOID *)setParam);

    return ret;
}

VOS_UINT32 MN_MSG_SetSmsSendDomain(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                   const MN_MSG_SmsSendDomainType *setParam)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (setParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SetSmsSendDomain: pstSetParam is Null ");
        return MN_ERR_NULLPTR;
    }

    if (setParam->smsSendDomain >= MN_MSG_SEND_DOMAIN_MAX) {
        AT_WARN_LOG("MN_MSG_SetSmsSendDomain: Invalid value");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SET_SMS_SEND_DOMAIN, clientId, opId, (VOS_VOID *)setParam);

    return ret;
}

VOS_UINT32 MN_MSG_GetLinkCtrl(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    VOS_UINT32 ret;

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_GET_RELAY_LINK_CTRL, clientId, opId, VOS_NULL_PTR);

    return ret;
}

VOS_UINT32 MN_MSG_GetSendDomain(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    VOS_UINT32 ret;

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_GET_SMS_SEND_DOMAIN, clientId, opId, VOS_NULL_PTR);

    return ret;
}

VOS_UINT32 MN_MSG_Send(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_SendParm *sendParm)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (sendParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_Send:pstSendDirectParm is Null ");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if (sendParm->memStore > MN_MSG_MEM_STORE_ME)
#else
    if (sendParm->memStore > MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_Send:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    /* 如果带有SCADDR,24011协议规定,BCD编码最少长度是2,最大长度是11 */
    if (sendParm->msgInfo.scAddr.bcdLen > 0) {
        if ((sendParm->msgInfo.scAddr.bcdLen < MN_MSG_MIN_BCD_NUM_LEN) ||
            (sendParm->msgInfo.scAddr.bcdLen > MN_MSG_MAX_BCD_NUM_LEN)) {
            AT_WARN_LOG("MN_MSG_Send:Invalid SC Address");
            return MN_ERR_CLASS_SMS_INVALID_SCADDR;
        }
    }

    /* 判断输入TPDU格式的合法性,即解码是否成功 */
    if ((sendParm->msgInfo.tsRawData.tpduType != MN_MSG_TPDU_COMMAND) &&
        (sendParm->msgInfo.tsRawData.tpduType != MN_MSG_TPDU_SUBMIT)) {
        AT_WARN_LOG("MN_MSG_Send:Invalid Tpdu Type");
        return MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
    }

    (VOS_VOID)memset_s(&g_msgDataInfo, sizeof(g_msgDataInfo), 0x00, sizeof(g_msgDataInfo));
    ret = MN_MSG_Decode(&sendParm->msgInfo.tsRawData, &g_msgDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("MN_MSG_Send:Invalid Tpdu");
        return MN_ERR_CLASS_SMS_INVALID_TPDU;
    }

    if (sendParm->msgInfo.tsRawData.tpduType == MN_MSG_TPDU_SUBMIT) {
        if (g_msgDataInfo.tpduType != MN_MSG_TPDU_SUBMIT) {
            AT_WARN_LOG("MN_MSG_Send:Conflict TPDU type");
            return MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
        }

        if (g_msgDataInfo.u.submit.destAddr.len == 0) {
            AT_WARN_LOG("MN_MSG_Send:No DA");
            return MN_ERR_CLASS_SMS_INVALID_DESTADDR;
        }
    } else {
        if (g_msgDataInfo.tpduType != MN_MSG_TPDU_COMMAND) {
            AT_WARN_LOG("MN_MSG_Send:Conflict TPDU type");
            return MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
        }
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT, clientId, opId, (VOS_VOID *)sendParm);

    return ret;
}

VOS_UINT32 MN_MSG_SendFromMem(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                              const MN_MSG_SendFrommemParm *sendFromMemParm)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (sendFromMemParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SendFromMem:pstSendFromMemParm is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((sendFromMemParm->memStore != MN_MSG_MEM_STORE_SIM) && (sendFromMemParm->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (sendFromMemParm->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_SendFromMem:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if ((sendFromMemParm->destAddr.bcdLen > 0) && ((sendFromMemParm->destAddr.bcdLen < MN_MSG_MIN_BCD_NUM_LEN) ||
                                                   (sendFromMemParm->destAddr.bcdLen > MN_MSG_MAX_BCD_NUM_LEN))) {
        AT_WARN_LOG("MN_MSG_SendFromMem:Invalid Dest Address");
        return MN_ERR_CLASS_SMS_INVALID_DESTADDR;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM, clientId, opId, (VOS_VOID *)sendFromMemParm);

    return ret;
}

VOS_UINT32 MN_MSG_SendAck(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_SendAckParm *ackParm)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (ackParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SendAck:pstAckParm is Null");
        return MN_ERR_NULLPTR;
    }
    /* 必须是Deliver Ack 或Deliver Err */
    if ((ackParm->tsRawData.tpduType != MN_MSG_TPDU_DELIVER_RPT_ACK) &&
        (ackParm->tsRawData.tpduType != MN_MSG_TPDU_DELIVER_RPT_ERR)) {
        AT_WARN_LOG("MN_MSG_SendAck:Invalid Tpdu Type");
        return MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
    }
    /* 判断输入TPDU格式的合法性,即解码是否成功 */
    (VOS_VOID)memset_s(&g_msgDataInfo, sizeof(g_msgDataInfo), 0x00, sizeof(g_msgDataInfo));
    ret = MN_MSG_Decode(&ackParm->tsRawData, &g_msgDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("MN_MSG_SendAck:Invalid Tpdu");
        return MN_ERR_CLASS_SMS_INVALID_TPDU;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SEND_RPRPT, clientId, opId, (VOS_VOID *)ackParm);

    return ret;
}

VOS_UINT32 MN_MSG_Write(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_WriteParm *writeParm)
{
    VOS_UINT32 ret;

    /* 判断输入参数的合法性 */
    if (writeParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_Write:pstWriteParm is Null");
        return MN_ERR_NULLPTR;
    }

    if ((writeParm->writeMode != MN_MSG_WRITE_MODE_INSERT) && (writeParm->writeMode != MN_MSG_WRITE_MODE_REPLACE)) {
        AT_WARN_LOG("MN_MSG_Write:Invalid enWriteMode");
        return MN_ERR_INVALIDPARM;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((writeParm->memStore != MN_MSG_MEM_STORE_SIM) && (writeParm->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (writeParm->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_Write:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (writeParm->status >= MN_MSG_STATUS_NONE) {
        AT_WARN_LOG("MN_MSG_Write:Invalid enStatus");
        return MN_ERR_INVALIDPARM;
    }

    /* 如果有SC ADDR,判断SC ADDR是否合理 */
    if (writeParm->msgInfo.scAddr.bcdLen > 0) {
        if ((writeParm->msgInfo.scAddr.bcdLen < MN_MSG_MIN_BCD_NUM_LEN) ||
            (writeParm->msgInfo.scAddr.bcdLen > MN_MSG_MAX_BCD_NUM_LEN)) {
            AT_WARN_LOG("MN_MSG_Send:Invalid SC Address");
            return MN_ERR_CLASS_SMS_INVALID_SCADDR;
        }
    }

    /* 判断输入TPDU格式的合法性,即解码是否成功 */
    (VOS_VOID)memset_s(&g_msgDataInfo, sizeof(g_msgDataInfo), 0x00, sizeof(g_msgDataInfo));
    ret = MN_MSG_Decode(&writeParm->msgInfo.tsRawData, &g_msgDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("MN_MSG_Write:Invalid Tpdu");
        return MN_ERR_CLASS_SMS_INVALID_TPDU;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_WRITE, clientId, opId, (VOS_VOID *)writeParm);

    return ret;
}

VOS_UINT32 MN_MSG_Read(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ReadParm *readParm)
{
    VOS_UINT32 ret;

    if (readParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_Read:pstReadParm is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((readParm->memStore != MN_MSG_MEM_STORE_SIM) && (readParm->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (readParm->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_Read:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_READ, clientId, opId, (VOS_VOID *)readParm);

    return ret;
}

VOS_UINT32 MN_MSG_List(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ListParm *listParm)
{
    VOS_UINT32 ret;

    if (listParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_List:pstListParm is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((listParm->memStore != MN_MSG_MEM_STORE_SIM) && (listParm->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (listParm->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_List:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (listParm->status > MN_MSG_STATUS_NONE) {
        AT_WARN_LOG("MN_MSG_List:Invalid enStatus");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_LIST, clientId, opId, (VOS_VOID *)listParm);

    return ret;
}

VOS_UINT32 MN_MSG_Delete(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParam)
{
    VOS_UINT32 ret;

    if (deleteParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_Delete:pstDelete is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((deleteParam->memStore != MN_MSG_MEM_STORE_SIM) && (deleteParam->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (deleteParam->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_Delete:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (deleteParam->deleteType > MN_MSG_DELETE_ALL) {
        AT_WARN_LOG("MN_MSG_Delete:Invalid enDeleteType");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_DELETE, clientId, opId, (VOS_VOID *)deleteParam);

    return ret;
}

VOS_UINT32 MN_MSG_Delete_Test(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ListParm *listPara)
{
    VOS_UINT32 ret;

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((listPara->memStore != MN_MSG_MEM_STORE_SIM) && (listPara->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (listPara->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_Delete:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (listPara->status >= MN_MSG_STATUS_MAX) {
        AT_WARN_LOG("MN_MSG_Delete:Invalid enStatus");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_DELETE_TEST, clientId, opId, (VOS_VOID *)listPara);

    return ret;
}

VOS_UINT32 MN_MSG_DeleteStaRpt(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParam)
{
    VOS_UINT32 ret;

    if (deleteParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_DeleteStaRpt:pstDeleteParam is Null");
        return MN_ERR_NULLPTR;
    }

    if ((deleteParam->memStore != MN_MSG_MEM_STORE_SIM) && (deleteParam->memStore != MN_MSG_MEM_STORE_ME)) {
        AT_WARN_LOG("MN_MSG_DeleteStaRpt:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (deleteParam->memStore != MN_MSG_MEM_STORE_SIM) {
        AT_WARN_LOG("MN_MSG_DeleteStaRpt:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if ((deleteParam->deleteType != MN_MSG_DELETE_SINGLE) && (deleteParam->deleteType != MN_MSG_DELETE_ALL)) {
        AT_WARN_LOG("MN_MSG_DeleteStaRpt:Invalid enDeleteType");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_DELETE_STARPT, clientId, opId, (VOS_VOID *)deleteParam);

    return ret;
}

VOS_UINT32 MN_MSG_WriteSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_WriteSrvParameter *srvParam)
{
    VOS_UINT32 ret;

    if (srvParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_WriteSrvParam:pstSetSrvParm is Null");
        return MN_ERR_NULLPTR;
    }

    if ((srvParam->writeMode != MN_MSG_WRITE_MODE_INSERT) && (srvParam->writeMode != MN_MSG_WRITE_MODE_REPLACE)) {
        AT_WARN_LOG("MN_MSG_WriteSrvParam:Invalid enWriteMode");
        return MN_ERR_INVALIDPARM;
    }

    if ((srvParam->memStore != MN_MSG_MEM_STORE_SIM) && (srvParam->memStore != MN_MSG_MEM_STORE_ME)) {
        AT_WARN_LOG("MN_MSG_WriteSrvParam:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if ((srvParam->memStore == MN_MSG_MEM_STORE_ME) && (srvParam->index != 0)) {
        AT_WARN_LOG("MN_MSG_WriteSrvParam:Only one record supported in NV.");
        return MN_ERR_INVALIDPARM;
    }

    /* 如果设置的值没有一个有效,则返回 */
    if ((srvParam->srvParm.parmInd & MN_MSG_SRV_PARM_TOTALABSENT) == MN_MSG_SRV_PARM_TOTALABSENT) {
        AT_WARN_LOG("MN_MSG_WriteSrvParam:Invalid Parm,No Valid Data");
        return MN_ERR_INVALIDPARM;
    }

    /* 判断SC Addr和Dest Addr的有效性 */
    if (((srvParam->srvParm.parmInd & MN_MSG_SRV_PARM_MASK_SC_ADDR) >> 1) == MN_MSG_SRV_PARM_PRESENT) {
        if (srvParam->srvParm.scAddr.bcdLen > MN_MSG_MAX_BCD_NUM_LEN) {
            AT_WARN_LOG("MN_MSG_WriteSrvParam:Invalid SC Address");
            return MN_ERR_CLASS_SMS_INVALID_SCADDR;
        }
    }

    if ((srvParam->srvParm.parmInd & MN_MSG_SRV_PARM_MASK_DEST_ADDR) == MN_MSG_SRV_PARM_PRESENT) {
        if (srvParam->srvParm.destAddr.bcdLen > MN_MSG_MAX_BCD_NUM_LEN) {
            AT_WARN_LOG("MN_MSG_WriteSrvParam:Invalid Dest Address");
            return MN_ERR_CLASS_SMS_INVALID_DESTADDR;
        }
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_WRITE_SRV_PARA, clientId, opId, (VOS_VOID *)srvParam);

    return ret;
}

VOS_UINT32 MN_MSG_ReadSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ReadCommParam *readParam)
{
    VOS_UINT32 ret;

    if (readParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_ReadSrvParam:pstReadParam is Null");
        return MN_ERR_NULLPTR;
    }

    if ((readParam->memStore != MN_MSG_MEM_STORE_SIM) && (readParam->memStore != MN_MSG_MEM_STORE_ME)) {
        AT_WARN_LOG("MN_MSG_ReadSrvParam:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if ((readParam->memStore == MN_MSG_MEM_STORE_ME) && (readParam->index != 0)) {
        AT_WARN_LOG("MN_MSG_ReadSrvParam: Only one record supported in NV.");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_READ_SRV_PARA, clientId, opId, (VOS_VOID *)readParam);

    return ret;
}

VOS_UINT32 MN_MSG_DeleteSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParam)
{
    VOS_UINT32 ret;

    if (deleteParam == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_DeleteSrvParam:pstDeleteParam is Null");
        return MN_ERR_NULLPTR;
    }

    if ((deleteParam->memStore != MN_MSG_MEM_STORE_SIM) && (deleteParam->memStore != MN_MSG_MEM_STORE_ME)) {
        AT_WARN_LOG("MN_MSG_DeleteSrvParam:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if ((deleteParam->deleteType != MN_MSG_DELETE_SINGLE) && (deleteParam->deleteType != MN_MSG_DELETE_ALL)) {
        AT_WARN_LOG("MN_MSG_DeleteSrvParam:Invalid enDeleteType");
        return MN_ERR_INVALIDPARM;
    }

    if (deleteParam->memStore == MN_MSG_MEM_STORE_ME) {
        if ((deleteParam->deleteType != MN_MSG_DELETE_SINGLE) && (deleteParam->index != 0)) {
            AT_WARN_LOG("MN_MSG_WriteSrvParam:Only one record supported in NV.");
            return MN_ERR_INVALIDPARM;
        }
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_DELETE_SRV_PARA, clientId, opId, (VOS_VOID *)deleteParam);

    return ret;
}

VOS_UINT32 MN_MSG_ModifyStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                               const MN_MSG_ModifyStatusParm *modifyParm)
{
    VOS_UINT32 ret;

    if (modifyParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_ModifyStatus:pstModifyParm is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((modifyParm->memStore != MN_MSG_MEM_STORE_SIM) && (modifyParm->memStore != MN_MSG_MEM_STORE_ME))
#else
    if (modifyParm->memStore != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_ModifyStatus:Invalid enMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (modifyParm->status > MN_MSG_STATUS_MO_SENT) {
        AT_WARN_LOG("MN_MSG_ModifyStatus:Invalid enStatus");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_MODIFY_STATUS, clientId, opId, (VOS_VOID *)modifyParm);

    return ret;
}

VOS_UINT32 MN_MSG_SetRcvMsgPath(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_SetRcvmsgPathParm *setRcvPathParm)
{
    VOS_UINT32 ret;

    if (setRcvPathParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SetRcvMsgPath:pstSetRcvPathParm is Null");
        return MN_ERR_NULLPTR;
    }

    if ((setRcvPathParm->rcvSmAct > MN_MSG_RCVMSG_ACT_TRANSFER_AND_ACK) ||
        (setRcvPathParm->rcvStaRptAct > MN_MSG_RCVMSG_ACT_TRANSFER_AND_ACK)) {
        AT_WARN_LOG("MN_MSG_SetRcvMsgPath:Invalid enRcvSmAct");
        return MN_ERR_INVALIDPARM;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((setRcvPathParm->smMemStore > MN_MSG_MEM_STORE_ME) || (setRcvPathParm->staRptMemStore > MN_MSG_MEM_STORE_ME))
#else
    if ((setRcvPathParm->smMemStore > MN_MSG_MEM_STORE_SIM) || (setRcvPathParm->staRptMemStore > MN_MSG_MEM_STORE_SIM))
#endif
    {
        AT_WARN_LOG("MN_MSG_SetRcvMsgPath:Invalid enSmMemStore");
        return MN_ERR_INVALIDPARM;
    }

    if (setRcvPathParm->rcvSmAct == MN_MSG_RCVMSG_ACT_STORE) {
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
        if ((setRcvPathParm->smMemStore != MN_MSG_MEM_STORE_ME) && (setRcvPathParm->smMemStore != MN_MSG_MEM_STORE_SIM))
#else
        if (setRcvPathParm->smMemStore != MN_MSG_MEM_STORE_SIM)
#endif
        {
            AT_WARN_LOG("MN_MSG_SetRcvMsgPath:Invalid Parm");
            return MN_ERR_INVALIDPARM;
        }
    }

    if (setRcvPathParm->rcvStaRptAct == MN_MSG_RCVMSG_ACT_STORE) {
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
        if ((setRcvPathParm->staRptMemStore != MN_MSG_MEM_STORE_ME) &&
            (setRcvPathParm->staRptMemStore != MN_MSG_MEM_STORE_SIM))
#else
        if (setRcvPathParm->staRptMemStore != MN_MSG_MEM_STORE_SIM)
#endif
        {
            AT_WARN_LOG("MN_MSG_SetRcvMsgPath:Invalid Parm");
            return MN_ERR_INVALIDPARM;
        }
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SET_RCVPATH, clientId, opId, (VOS_VOID *)setRcvPathParm);

    return ret;
}

VOS_UINT32 MN_MSG_GetRcvMsgPath(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_GetRcvmsgPathParm *getRcvPathParm)
{
    VOS_UINT32 ret;

    if (getRcvPathParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_GetRcvMsgPath:pstGetRcvPathParm is Null");
        return MN_ERR_NULLPTR;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_GET_RCVPATH, clientId, opId, (VOS_VOID *)getRcvPathParm);

    return ret;
}

VOS_UINT32 MN_MSG_GetStorageStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                   const MN_MSG_GetStorageStatusParm *storageParm)
{
    VOS_UINT32 ret;

    if (storageParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_GetStorageStatus:pstMemParm is Null");
        return MN_ERR_NULLPTR;
    }

#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((storageParm->mem1Store != MN_MSG_MEM_STORE_SIM) && (storageParm->mem1Store != MN_MSG_MEM_STORE_ME) &&
        (storageParm->mem1Store != MN_MSG_MEM_STORE_NONE))
#else
    if (storageParm->mem1Store != MN_MSG_MEM_STORE_SIM)
#endif
    {
        AT_WARN_LOG("MN_MSG_GetStorageStatus:Invalid enMemStore:mem1");
        return MN_ERR_INVALIDPARM;
    }

    if (storageParm->mem1Store == storageParm->mem2Store) {
        AT_WARN_LOG("MN_MSG_GetStorageStatus:Invalid enMemStore:mem1 == mem2");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_GET_STORAGESTATUS, clientId, opId, (VOS_VOID *)storageParm);

    return ret;
}

VOS_UINT32 MN_MSG_SetMemStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                               const MN_MSG_SetMemstatusParm *memStatusParm)
{
    VOS_UINT32 ret;

    if (memStatusParm == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_SetMemStatus:pMemStatusParm is Null");
        return MN_ERR_NULLPTR;
    }

    if ((memStatusParm->memFlag != MN_MSG_MEM_FULL_SET) && (memStatusParm->memFlag != MN_MSG_MEM_FULL_UNSET)) {
        AT_WARN_LOG("MN_MSG_SetMemStatus:Invalid enMemFlag");
        return MN_ERR_INVALIDPARM;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_SET_MEMSTATUS, clientId, opId, (VOS_VOID *)memStatusParm);

    return ret;
}

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))

VOS_UINT32 MN_MSG_GetAllCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    VOS_UINT32 ret;

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_GET_ALLCBMIDS, clientId, opId, VOS_NULL_PTR);

    return ret;
}

VOS_UINT32 MN_MSG_AddCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const TAF_CBA_CbmiRangeList *cbmirList)
{
    VOS_UINT32 ret;

    if (cbmirList == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_DeleteCbMsg: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_ADD_CBMIDS, clientId, opId, (VOS_VOID *)cbmirList);

    return ret;
}

VOS_UINT32 MN_MSG_DelCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const TAF_CBA_CbmiRangeList *cbmirList)
{
    VOS_UINT32 ret;

    if (cbmirList == VOS_NULL_PTR) {
        AT_WARN_LOG("MN_MSG_DeleteCbMsg: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_DELETE_CBMIDS, clientId, opId, (VOS_VOID *)cbmirList);

    return ret;
}

VOS_UINT32 MN_MSG_EmptyCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    VOS_UINT32 ret;

    ret = MSG_SendAppReq(MN_MSG_MSGTYPE_EMPTY_CBMIDS, clientId, opId, VOS_NULL_PTR);

    return ret;
}

#endif
