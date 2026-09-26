/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include "at_sms_taf_rslt_proc.h"
#include "securec.h"
#include "at_check_func.h"
#include "ppp_interface.h"
#include "taf_mmi_str_parse.h"
#include "app_vc_api.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "taf_std_lib.h"
#include "mn_comm_api.h"
#include "at_event_report.h"
#include "at_sms_comm.h"
#include "at_msg_print.h"

#include "at_common.h"

#include "at_mdrv_interface.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_TAF_RSLT_PROC_C
#define AT_TPDU_TYPE_MAX_LEN 4
#define AT_SMS_INPUT_TYPE 1
#define AT_WAIT_SMS_INPUT_TYPE 2

MN_MSG_RawTsData   g_atMsgRawTsData[AT_MSG_MAX_MSG_SEGMENT_NUM];
MN_MSG_SendParm    g_atMsgDirectParm;
MN_MSG_WriteParm   g_atMsgWriteParm;

/*
 * MN_CALLBACK_CMD_CNF消息cpms命令处理
 */
VOS_UINT32 AT_ProcMnCallBackCmdCnfCpmsCmd(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    TAF_UINT32      result = AT_FAILURE;

    result = AT_CMS_UNKNOWN_ERROR;
    AT_StopTimerCmdReady(indexNum);
    return result;
}

/*
 * MN_CALLBACK_CMD_CNF消息csca read命令处理
 */
VOS_UINT32 AT_ProcMnCallBackCmdCnfCscaRead(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    TAF_UINT32      result = AT_FAILURE;

    result = At_ChgMnErrCodeToAt(indexNum, errorCode);

    AT_StopTimerCmdReady(indexNum);
    return result;
}

TAF_UINT32 At_CmdCmgdMsgProc(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
    TAF_UINT32 result          = AT_FAILURE;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    if ((errorCode == MN_ERR_CLASS_SMS_EMPTY_REC) && (*systemAppConfig != SYSTEM_APP_ANDROID)) {
        result = AT_OK;
        AT_StopTimerCmdReady(indexNum);
    } else {
        result = At_ChgMnErrCodeToAt(indexNum, errorCode); /* 发生错误 */
        AT_StopTimerCmdReady(indexNum);
    }

    return result;
}

MN_MSG_SendParm* At_GetDirectSendMsgMem(TAF_VOID)
{
    (VOS_VOID)memset_s(&g_atMsgDirectParm, sizeof(g_atMsgDirectParm), 0x00, sizeof(g_atMsgDirectParm));
    return &g_atMsgDirectParm;
}

MN_MSG_RawTsData* At_GetLongMsgSegMem(TAF_VOID)
{
    (VOS_VOID)memset_s(g_atMsgRawTsData, sizeof(g_atMsgRawTsData), 0x00, (sizeof(g_atMsgRawTsData)));
    return g_atMsgRawTsData;
}

MN_MSG_WriteParm* At_GetWriteMsgMem(TAF_VOID)
{
    (VOS_VOID)memset_s(&g_atMsgWriteParm, sizeof(g_atMsgWriteParm), 0x00, sizeof(g_atMsgWriteParm));
    return &g_atMsgWriteParm;
}

/*
 * 功能描述: 短信发送结果码处理
 */
LOCAL TAF_UINT32 At_ProcSmsMsgOrCmdRslt(TAF_UINT8 indexNum, TAF_UINT32 ret)
{
    TAF_UINT32 rslt;
    if (ret == MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].atSmsData.msgSentSmNum = 1;
        rslt = AT_WAIT_ASYNC_RETURN;
    } else {
        AT_ERR_LOG("At_ProcSmsMsgOrCmdRslt: Fail to send sms message or command.");
        if (ret == MN_ERR_CLASS_SMS_NOAVAILDOMAIN) {
            rslt = AT_CMS_UNKNOWN_ERROR;
        } else {
            rslt = AT_ERROR;
        }
    }

    return rslt;
}

TAF_UINT32 At_GetScaFromInputStr(const TAF_UINT8 *addr, MN_MSG_BcdAddr *bcdAddr, TAF_UINT32 *len)
{
    TAF_UINT32       ret;
    MN_MSG_AsciiAddr asciiAddr;

    ret = MN_MSG_DecodeAddress(addr, TAF_TRUE, &asciiAddr, len);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    bcdAddr->addrType = 0x80;
    bcdAddr->addrType |= asciiAddr.numPlan;
    bcdAddr->addrType |= ((asciiAddr.numType << 4) & 0x70);

    ret = AT_AsciiNumberToBcd((TAF_CHAR *)asciiAddr.asciiNum, bcdAddr->bcdNum, &bcdAddr->bcdLen);

    return ret;
}

/*
 * 功能描述: 处理短信过程的sca信息
 */
LOCAL TAF_UINT32 At_ProcScaInfo(TAF_UINT8 indexNum, TAF_UINT32 len, TAF_UINT32 *uAddrlLen, TAF_UINT8 *data,
    MN_MSG_SendParm *sendDirectParm)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;
    TAF_UINT32 ret;
    errno_t memResult;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (len > g_atClientTab[indexNum].atSmsData.pduLen) {
        ret = At_GetScaFromInputStr(data, &sendDirectParm->msgInfo.scAddr, uAddrlLen);
        if (ret != MN_ERR_NO_ERROR) {
            AT_NORM_LOG("At_ProcScaInfo: invalid tpdu mode.");
            return AT_CMS_INVALID_PDU_MODE_PARAMETER;
        }

        if (len != (g_atClientTab[indexNum].atSmsData.pduLen + (*uAddrlLen))) {
            AT_NORM_LOG("At_ProcScaInfo: invalid tpdu data length.");
            return AT_CMS_INVALID_PDU_MODE_PARAMETER;
        }
    }

    if (sendDirectParm->msgInfo.scAddr.bcdLen == 0) {
        if (smsCtx->cscaCsmpInfo.parmInUsim.scAddr.bcdLen == 0) {
            AT_NORM_LOG("At_ProcScaInfo: without sca.");
            return AT_CMS_SMSC_ADDRESS_UNKNOWN;
        }
        memResult = memcpy_s(&sendDirectParm->msgInfo.scAddr, sizeof(sendDirectParm->msgInfo.scAddr),
                             &(smsCtx->cscaCsmpInfo.parmInUsim.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(sendDirectParm->msgInfo.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SendPduMsgOrCmd(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT32 len)
{
    TAF_UINT32         ret;
    MN_MSG_SendParm   *sendDirectParm = VOS_NULL_PTR;
    MN_OPERATION_ID_T  opId           = At_GetOpId();
    MN_MSG_TsDataInfo *tsDataInfo     = VOS_NULL_PTR;
    MN_MSG_RawTsData  *rawData        = VOS_NULL_PTR;
    TAF_UINT32         uAddrlLen     = 0;
    errno_t            memResult;

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGS_PDU_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGC_PDU_SET)) {
        AT_WARN_LOG("At_SendPduMsgOrCmd: invalid command operation.");
        return AT_ERROR;
    }

    /* Refer to protocol 31102 4.2.25 */
    if (g_atClientTab[indexNum].atSmsData.pduLen > len) {
        AT_NORM_LOG("At_SendPduMsgOrCmd: invalid tpdu data length.");
        return AT_CMS_INVALID_PDU_MODE_PARAMETER;
    }

    sendDirectParm = At_GetDirectSendMsgMem();
    sendDirectParm->memStore   = MN_MSG_MEM_STORE_NONE;
    sendDirectParm->clientType = MN_MSG_CLIENT_NORMAL;

    /* sca */
    ret = At_ProcScaInfo(indexNum, len, &uAddrlLen, data, sendDirectParm);
    if (ret != AT_SUCCESS) {
        return ret;
    }

    sendDirectParm->msgInfo.tsRawData.len = g_atClientTab[indexNum].atSmsData.pduLen;
    if (g_atClientTab[indexNum].atSmsData.pduLen > 0) {
        memResult = memcpy_s(sendDirectParm->msgInfo.tsRawData.data, sizeof(sendDirectParm->msgInfo.tsRawData.data),
            &data[uAddrlLen], g_atClientTab[indexNum].atSmsData.pduLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(sendDirectParm->msgInfo.tsRawData.data), g_atClientTab[indexNum].atSmsData.pduLen);
    }

    sendDirectParm->msgInfo.tsRawData.tpduType = MN_MSG_TPDU_COMMAND;
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGS_PDU_SET) {
        sendDirectParm->msgInfo.tsRawData.tpduType = MN_MSG_TPDU_SUBMIT;
    }

    /* 增加PDU码有效性检查，删除冗余字段 */
    tsDataInfo = At_GetMsgMem();
    ret        = MN_MSG_Decode(&sendDirectParm->msgInfo.tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("At_SendPduMsgOrCmd: fail to decode.");
        return AT_CMS_INVALID_PDU_MODE_PARAMETER;
    }

    rawData = At_GetLongMsgSegMem();
    ret     = MN_MSG_Encode(tsDataInfo, rawData);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("At_SendPduMsgOrCmd: fail to encode.");
    } else {
        if (rawData->len != sendDirectParm->msgInfo.tsRawData.len) {
            AT_NORM_LOG1("At_SendPduMsgOrCmd: pstSendParm->stMsgInfo.stTsRawData.ulLen not match:",
                         sendDirectParm->msgInfo.tsRawData.len);
            AT_NORM_LOG1("At_SendPduMsgOrCmd: pstRawData->ulLen not match:", rawData->len);
            sendDirectParm->msgInfo.tsRawData.len = rawData->len;
        }
    }

    g_atClientTab[indexNum].opId = opId;
    ret                          = MN_MSG_Send(g_atClientTab[indexNum].clientId, opId, sendDirectParm);

    return At_ProcSmsMsgOrCmdRslt(indexNum, ret);
}

TAF_UINT32 At_SendPduMsgAck(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT32 len)
{
    errno_t             memResult;
    MN_MSG_SendAckParm *ackParm = VOS_NULL_PTR;
    TAF_UINT32          ret;
    MN_OPERATION_ID_T   opId       = At_GetOpId();
    MN_MSG_TsDataInfo  *tsDataInfo = VOS_NULL_PTR;

    if (len != g_atClientTab[indexNum].atSmsData.pduLen) {
        AT_NORM_LOG("At_SendPduMsgAck: the length of PDU is not consistent.");
        return AT_CMS_OPERATION_NOT_ALLOWED; /* 输入字串太长 */
    }

    /* 执行命令操作 */
    ackParm = At_GetAckMsgMem();

    /* g_atClientTab[ucIndex].AtSmsData.ucNumType为0的情况已经在命令设置时直接处理了，此处不考虑 */
    if (g_atClientTab[indexNum].atSmsData.cnmaType == 1) {
        ackParm->rpAck              = TAF_TRUE;
        ackParm->tsRawData.tpduType = MN_MSG_TPDU_DELIVER_RPT_ACK;
    } else {
        ackParm->rpAck              = TAF_FALSE;
        ackParm->rpCause            = MN_MSG_RP_CAUSE_PROTOCOL_ERR_UNSPECIFIED;
        ackParm->tsRawData.tpduType = MN_MSG_TPDU_DELIVER_RPT_ERR;
    }

    /* g_atClientTab[ucIndex].AtSmsData.ucPduLen为0的情况已经在命令设置时直接处理了，此处不考虑 */
    ackParm->tsRawData.len = (TAF_UINT32)g_atClientTab[indexNum].atSmsData.pduLen;
    if ((len > 0) && (len <= MN_MSG_MAX_LEN)) {
        memResult = memcpy_s(ackParm->tsRawData.data, sizeof(ackParm->tsRawData.data), data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ackParm->tsRawData.data), len);
    }
    tsDataInfo = At_GetMsgMem();
    ret        = MN_MSG_Decode(&ackParm->tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("At_SendPduMsgOrCmd: Fail to decode.");
        return AT_ERROR;
    }

    if ((ackParm->rpAck == TAF_FALSE) && (tsDataInfo->u.deliverRptErr.failCause == MN_MSG_TP_CAUSE_MEMORY_FULL)) {
        ackParm->rpCause = MN_MSG_RP_CAUSE_MEMORY_EXCEEDED;
    }

    g_atClientTab[indexNum].opId = opId;
    if (MN_MSG_SendAck(g_atClientTab[indexNum].clientId, opId, ackParm) == MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt          = AT_CMD_CNMA_PDU_SET;
        g_atClientTab[indexNum].atSmsData.msgSentSmNum = 1;
        ret                                            = AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        ret = AT_ERROR;
    }
    return ret;
}

/*
 * 功能描述: 获取PDU短信信息内容
 */
LOCAL VOS_UINT32 At_GetPduMsgInfo(TAF_UINT8 indexNum, TAF_UINT32 len, TAF_UINT8 *data, MN_MSG_WriteParm *writeParm)
{
    AT_ModemSmsCtx      *smsCtx     = VOS_NULL_PTR;
    TAF_UINT32           ret;
    TAF_UINT32           addrLen = 0;
    errno_t              memResult;
    MN_MSG_TpduTypeUint8 mtiMapTpduType[AT_TPDU_TYPE_MAX_LEN] = {
        MN_MSG_TPDU_DELIVER,
        MN_MSG_TPDU_SUBMIT,
        MN_MSG_TPDU_COMMAND,
        MN_MSG_TPDU_MAX
    };
    TAF_UINT8 fo;


    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    writeParm->writeMode = MN_MSG_WRITE_MODE_INSERT;
    writeParm->memStore  = smsCtx->cpmsInfo.memSendorWrite;
    writeParm->status    = g_atClientTab[indexNum].atSmsData.smState;

    /* sca */
    if (len > g_atClientTab[indexNum].atSmsData.pduLen) {
        ret = At_GetScaFromInputStr(data, &writeParm->msgInfo.scAddr, &addrLen);
        if (ret != MN_ERR_NO_ERROR) {
            AT_NORM_LOG("At_GetPduMsgInfo: fail to get sca from user input.");
            return AT_ERROR;
        }

        if (len != (g_atClientTab[indexNum].atSmsData.pduLen + addrLen)) {
            AT_NORM_LOG("At_GetPduMsgInfo: the length of <pdu> is not consistent with <length>.");
            return AT_ERROR;
        }
    }

    if (writeParm->msgInfo.scAddr.bcdLen == 0) {
        if (smsCtx->cscaCsmpInfo.parmInUsim.scAddr.bcdLen == 0) {
            AT_NORM_LOG("At_GetPduMsgInfo: without sca.");
        } else {
            memResult = memcpy_s(&writeParm->msgInfo.scAddr, sizeof(writeParm->msgInfo.scAddr),
                &(smsCtx->cscaCsmpInfo.parmInUsim.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(writeParm->msgInfo.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
        }
    }

    writeParm->msgInfo.tsRawData.len = g_atClientTab[indexNum].atSmsData.pduLen;
    if (g_atClientTab[indexNum].atSmsData.pduLen > 0) {
        memResult = memcpy_s(writeParm->msgInfo.tsRawData.data, sizeof(writeParm->msgInfo.tsRawData.data),
                             &data[addrLen], g_atClientTab[indexNum].atSmsData.pduLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(writeParm->msgInfo.tsRawData.data),
                            g_atClientTab[indexNum].atSmsData.pduLen);
    }

    fo                                    = writeParm->msgInfo.tsRawData.data[0];
    writeParm->msgInfo.tsRawData.tpduType = mtiMapTpduType[(fo & 0x03)];

    return AT_SUCCESS;
}

VOS_UINT32 At_WritePduMsgToMem(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT32 len)
{
    MN_MSG_WriteParm    *writeParm  = VOS_NULL_PTR;
    MN_MSG_TsDataInfo   *tsDataInfo = VOS_NULL_PTR;
    MN_MSG_RawTsData    *rawData    = VOS_NULL_PTR;
    MN_OPERATION_ID_T    opId    = At_GetOpId();
    TAF_UINT32           ret;


    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGW_PDU_SET) {
        AT_WARN_LOG("At_WritePduMsgToMem: invalid command operation.");
        return AT_ERROR;
    }

    if (g_atClientTab[indexNum].atSmsData.pduLen > len) {
        AT_NORM_LOG("At_WritePduMsgToMem: invalid tpdu data length.");
        return AT_ERROR;
    }

    writeParm = At_GetWriteMsgMem();

    if (At_GetPduMsgInfo(indexNum, len, data, writeParm) == AT_ERROR) {
        AT_NORM_LOG("At_WritePduMsgToMem: fail to get pud msg info.");
        return AT_ERROR;
    }

    /* 增加PDU码有效性检查，删除冗余字段 */
    tsDataInfo = At_GetMsgMem();
    ret        = MN_MSG_Decode(&writeParm->msgInfo.tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("At_WritePduMsgToMem: Fail to decode.");
        return AT_ERROR;
    }

    rawData = At_GetLongMsgSegMem();
    ret     = MN_MSG_Encode(tsDataInfo, rawData);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("At_WritePduMsgToMem: fail to encode.");
    } else {
        if (rawData->len != writeParm->msgInfo.tsRawData.len) {
            AT_NORM_LOG1("At_WritePduMsgToMem: pstSendParm->stMsgInfo.stTsRawData.ulLen not match:",
                         writeParm->msgInfo.tsRawData.len);
            AT_NORM_LOG1("At_WritePduMsgToMem: pstRawData->ulLen not match:", rawData->len);
            writeParm->msgInfo.tsRawData.len = rawData->len;
        }
    }

    g_atClientTab[indexNum].opId = opId;
    ret                          = MN_MSG_Write(g_atClientTab[indexNum].clientId, opId, writeParm);
    if (ret == MN_ERR_NO_ERROR) {
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_GetInvalidCharLengthForSms(VOS_UINT8 *pdu, VOS_UINT32 len)
{
    VOS_UINT32 loop;
    VOS_UINT32 ret;
    VOS_UINT32 invalidCharLengthTemp = 0;
    VOS_UINT8  hex;
    VOS_UINT32 invalidCharLength;

    for (loop = 0; loop < len; loop++) {
        ret = AT_ConvertCharToHex(pdu[loop], &hex);
        if (ret != AT_FAILURE) {
            break;
        }

        invalidCharLengthTemp++;
    }

    invalidCharLength = invalidCharLengthTemp;

    return invalidCharLength;
}

VOS_UINT32 AT_ProcAbnormalPdu(VOS_UINT8 *pdu, VOS_UINT32 len)
{
    VOS_UINT32 invalidCharLength;

    if (pdu[len] == AT_ESC) {
        return AT_CMS_INVALID_PDU_MODE_PARAMETER;
    }

    /*
     * 进入At_SmsPduProc处理函数只有结束符为回车,CTRL+Z和ESC三种情况，
     * At_SmsPduProc函数的第一个CASE处理了CTRL+Z,本函数前面流程处理了ESC情况
     * 这段代码是针对字符串结束符为回车的处理
     */
    invalidCharLength = AT_GetInvalidCharLengthForSms(pdu, len);
    if (invalidCharLength == len) {
        return AT_SUCCESS;
    } else {
        return AT_CMS_INVALID_PDU_MODE_PARAMETER;
    }
}

TAF_UINT32 At_SmsPduProc(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len)
{
    VOS_UINT32 invalidCharLength;
    TAF_UINT16 tmpLen = len;
    TAF_UINT32 ret;

    tmpLen -= 1; /* 找到最后一个字符 */
    switch (data[tmpLen]) {
        /* 根据最后一个字符决定下一步操作 */
        case AT_CTRL_Z:
            /* 根据当前用户的命令索引,发送/写/PDU[短信/命令] */
            invalidCharLength = AT_GetInvalidCharLengthForSms(data, tmpLen);
            data += invalidCharLength;
            tmpLen -= (VOS_UINT16)invalidCharLength;

            if (At_UnicodePrint2Unicode(data, &tmpLen) == AT_FAILURE) {
                return AT_CMS_INVALID_PDU_MODE_PARAMETER;
            }

            switch (g_atClientTab[indexNum].cmdCurrentOpt) {
                case AT_CMD_CMGS_PDU_SET:
                case AT_CMD_CMGC_PDU_SET:
                    ret = At_SendPduMsgOrCmd(indexNum, data, tmpLen);
                    break;

                case AT_CMD_CMGW_PDU_SET:
                    ret = At_WritePduMsgToMem(indexNum, data, tmpLen);
                    break;

                case AT_CMD_CNMA_PDU_SET:
                    ret = At_SendPduMsgAck(indexNum, data, tmpLen);
                    break;

                default:
                    ret = AT_ERROR; /*  返回错误 */
            }
            break;
        default:
            ret = AT_ProcAbnormalPdu(data, tmpLen); /*  返回错误 */
            break;
    }

    return ret;
}

VOS_UINT32 At_EncodeTextMsgTpUd(VOS_UINT8 indexNum, MN_MSG_MsgCodingUint8 msgCoding, MN_MSG_UserData *userData)
{
    VOS_UINT32 ret;
    errno_t    memResult;

    if (g_atClientTab[indexNum].smsTxtLen > MN_MSG_MAX_LEN) {
        AT_NORM_LOG("At_EncodeTextMsgTpUd: invalid text message length.");
        return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
    }

    if ((g_atCscsType == AT_CSCS_IRA_CODE) && (msgCoding == MN_MSG_MSG_CODING_7_BIT)) {
        ret = TAF_STD_ConvertAsciiToDefAlpha(g_atClientTab[indexNum].atSmsData.buffer,
                                             g_atClientTab[indexNum].smsTxtLen, userData->orgData, &userData->len,
                                             MN_MSG_MAX_LEN);
        if (ret == MN_ERR_INVALIDPARM) {
            AT_NORM_LOG("At_EncodeTextMsgTpUd: TAF_STD_ConvertAsciiToDefAlpha fail.");
            return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
        }

    } else if ((g_atCscsType == AT_CSCS_GSM_7Bit_CODE) && (msgCoding == MN_MSG_MSG_CODING_8_BIT)) {
        TAF_STD_ConvertDefAlphaToAscii(g_atClientTab[indexNum].atSmsData.buffer, g_atClientTab[indexNum].smsTxtLen,
                                       userData->orgData, &userData->len);
        return AT_SUCCESS;
    } else {
        userData->len = g_atClientTab[indexNum].smsTxtLen;
        if (userData->len > 0) {
            memResult = memcpy_s(userData->orgData, MN_MSG_MAX_LEN, g_atClientTab[indexNum].atSmsData.buffer,
                                 userData->len);
            TAF_MEM_CHK_RTN_VAL(memResult, MN_MSG_MAX_LEN, userData->len);
        }
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 获得消息数据的首字节属性
 */
LOCAL VOS_UINT32 At_GetMsgFoInfo(VOS_UINT8 indexNum, MN_MSG_TsDataInfo *tsDataInfo, TAF_UINT8 *fo)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGC_TEXT_SET) {
        *fo = g_atClientTab[indexNum].atSmsData.fo;
    } else {
        /* 判断FO的有效性 */
        if (smsCtx->cscaCsmpInfo.foUsed == TAF_TRUE) {
            *fo = smsCtx->cscaCsmpInfo.fo;
        } else {
            if ((tsDataInfo->tpduType == MN_MSG_TPDU_DELIVER) || (tsDataInfo->tpduType == MN_MSG_TPDU_SUBMIT)) {
                *fo = AT_CSMP_FO_DEFAULT_VALUE1;
            } else if ((tsDataInfo->tpduType == MN_MSG_TPDU_STARPT) || (tsDataInfo->tpduType == MN_MSG_TPDU_COMMAND)) {
                *fo = AT_CSMP_FO_DEFAULT_VALUE2;
            } else {
                AT_NORM_LOG("At_GetMsgFoInfo: invalid enTpduType.");
                return VOS_FALSE;
            }
        }
    }

    return VOS_TRUE;
}

VOS_VOID At_SendMsgFoAttr(VOS_UINT8 indexNum, MN_MSG_TsDataInfo *tsDataInfo)
{
    TAF_UINT8       fo     = 0;

    if (At_GetMsgFoInfo(indexNum, tsDataInfo, &fo) == VOS_FALSE) {
        AT_NORM_LOG("At_SendMsgFoAttr: invalid enTpduType.");
        return;
    }

    switch (tsDataInfo->tpduType) {
        case MN_MSG_TPDU_COMMAND:
            /* TP MTI TP UDHI TP SRR */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.command.userDataHeaderInd, fo);
            AT_GET_MSG_TP_SRR(tsDataInfo->u.command.staRptReq, fo);
            break;

        case MN_MSG_TPDU_DELIVER:
            /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI: */
            /* decode fo:TP MTI TP MMS TP RP TP UDHI TP SRI */
            AT_GET_MSG_TP_MMS(tsDataInfo->u.deliver.moreMsg, fo);
            AT_GET_MSG_TP_RP(tsDataInfo->u.deliver.replayPath, fo);
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.deliver.userDataHeaderInd, fo);
            AT_GET_MSG_TP_SRI(tsDataInfo->u.deliver.staRptInd, fo);
            break;

        case MN_MSG_TPDU_STARPT:
            /* TP MTI ignore TP UDHI TP MMS TP SRQ */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.staRpt.userDataHeaderInd, fo);
            AT_GET_MSG_TP_MMS(tsDataInfo->u.staRpt.moreMsg, fo);
            AT_GET_MSG_TP_SRQ(tsDataInfo->u.staRpt.staRptQualCommand, fo);
            break;

        case MN_MSG_TPDU_SUBMIT:
            AT_GET_MSG_TP_RD(tsDataInfo->u.submit.rejectDuplicates, fo);
            /* TP VPF 23040 9.2.3.3 */
            AT_GET_MSG_TP_VPF(tsDataInfo->u.submit.validPeriod.validPeriod, fo);
            /* TP RP  23040 9.2.3.17 */
            AT_GET_MSG_TP_RP(tsDataInfo->u.submit.replayPath, fo);
            /* TP UDHI23040 9.2.3.23 */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.submit.userDataHeaderInd, fo);
            /* TP SRR 23040 9.2.3.5 */
            AT_GET_MSG_TP_SRR(tsDataInfo->u.submit.staRptReq, fo);
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            /* TP MTI ignore  TP-UDHI  */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.deliverRptAck.userDataHeaderInd, fo);
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            /* TP MTI ignore  TP-UDHI  */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.deliverRptErr.userDataHeaderInd, fo);
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            /* TP MTI ignore TP-UDHI  */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.submitRptAck.userDataHeaderInd, fo);
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            /* TP MTI ignore TP-UDHI  */
            AT_GET_MSG_TP_UDHI(tsDataInfo->u.submitRptErr.userDataHeaderInd, fo);
            break;

        default:
            AT_NORM_LOG("At_SendMsgFoAttr: invalid pdu type.");
            break;
    }
}

/*
 * 功能描述: 短信COMMAND类型处理
 */
LOCAL TAF_UINT32 At_ProcTpduCommandType(TAF_UINT8 indexNum, MN_MSG_TsDataInfo *tsDataInfo)
{
    errno_t memResult;

    tsDataInfo->tpduType = MN_MSG_TPDU_COMMAND;
    /* Fo */
    At_SendMsgFoAttr(indexNum, tsDataInfo);
    /* Mr填0,由MN修改 */
    /* PID */
    /* PID */
    tsDataInfo->u.command.pid = g_atClientTab[indexNum].atSmsData.pid;

    /* TP-CT */
    tsDataInfo->u.command.cmdType = g_atClientTab[indexNum].atSmsData.commandType;
    /* TP-MN */ /* 此处去掉了绝对编号类型TAF_SMS_CMD_MSG_NUM_ABSOLUTE,需确认 */
    tsDataInfo->u.command.msgNumber = g_atClientTab[indexNum].atSmsData.messageNumber;

    /* Da */
    memResult = memcpy_s(&tsDataInfo->u.command.destAddr, sizeof(tsDataInfo->u.command.destAddr),
        &g_atClientTab[indexNum].atSmsData.asciiAddr, sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.command.destAddr),
                        sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));
    /* CDL CD */
    if (g_atClientTab[indexNum].smsTxtLen > MN_MSG_MAX_COMMAND_DATA_LEN) {
        AT_NORM_LOG("At_ProcTpduCommandType: invalid text command length.");
        return AT_ERROR;
    }

    /* 这里使用了g_atClientTab[ucIndex].usSmsTxtLen作为命令长度，与原程序不同，需确认 */
    tsDataInfo->u.command.commandDataLen = (TAF_UINT8)g_atClientTab[indexNum].smsTxtLen;
    if (g_atClientTab[indexNum].smsTxtLen > 0) {
        memResult = memcpy_s(tsDataInfo->u.command.cmdData, MN_MSG_MAX_COMMAND_DATA_LEN,
                             g_atClientTab[indexNum].atSmsData.buffer, g_atClientTab[indexNum].smsTxtLen);
        TAF_MEM_CHK_RTN_VAL(memResult, MN_MSG_MAX_COMMAND_DATA_LEN, g_atClientTab[indexNum].smsTxtLen);
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SendTextMsgOrCmd(TAF_UINT8 indexNum)
{
    errno_t            memResult;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;
    TAF_UINT32         ret;
    MN_MSG_SendParm   *sendDirectParm = VOS_NULL_PTR;
    MN_OPERATION_ID_T  opId           = At_GetOpId();
    AT_ModemSmsCtx    *smsCtx         = VOS_NULL_PTR;

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGS_TEXT_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGC_TEXT_SET)) {
        AT_WARN_LOG("At_SendTextMsgOrCmd: invalid command operation.");
        return AT_ERROR;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    tsDataInfo                 = At_GetMsgMem();
    sendDirectParm             = At_GetDirectSendMsgMem();
    sendDirectParm->memStore   = MN_MSG_MEM_STORE_NONE;
    sendDirectParm->clientType = MN_MSG_CLIENT_NORMAL;

    /* sc */
    if (smsCtx->cscaCsmpInfo.parmInUsim.scAddr.bcdLen == 0) {
        AT_NORM_LOG("At_SendTextMsgOrCmd: no SCA.");
        return AT_CMS_SMSC_ADDRESS_UNKNOWN;
    }
    memResult = memcpy_s(&sendDirectParm->msgInfo.scAddr, sizeof(sendDirectParm->msgInfo.scAddr),
        &(smsCtx->cscaCsmpInfo.parmInUsim.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(sendDirectParm->msgInfo.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGS_TEXT_SET) {
        tsDataInfo->tpduType = MN_MSG_TPDU_SUBMIT;

        /* Fo */
        At_SendMsgFoAttr(indexNum, tsDataInfo);

        /* Mr填0,由MN修改 */

        /* Da */
        memResult = memcpy_s(&tsDataInfo->u.submit.destAddr, sizeof(tsDataInfo->u.submit.destAddr),
            &g_atClientTab[indexNum].atSmsData.asciiAddr, sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.submit.destAddr), sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));

        /* PID */
        tsDataInfo->u.submit.pid = smsCtx->cscaCsmpInfo.parmInUsim.pid;

        /* DCS */
        ret = MN_MSG_DecodeDcs(smsCtx->cscaCsmpInfo.parmInUsim.dcs, &tsDataInfo->u.submit.dcs);
        if (ret != MN_ERR_NO_ERROR) {
            AT_NORM_LOG("At_SendTextMsgOrCmd: Fail to decode DCS.");
            return AT_ERROR;
        }

        /* VP */
        memResult = memcpy_s(&tsDataInfo->u.submit.validPeriod, sizeof(tsDataInfo->u.submit.validPeriod),
                             &(smsCtx->cscaCsmpInfo.vp), sizeof(smsCtx->cscaCsmpInfo.vp));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.submit.validPeriod), sizeof(smsCtx->cscaCsmpInfo.vp));

        /* UDL UD */
        ret = At_EncodeTextMsgTpUd(indexNum, tsDataInfo->u.submit.dcs.msgCoding, &tsDataInfo->u.submit.userData);
        if (ret != AT_SUCCESS) {
            AT_WARN_LOG("At_WriteTextMsgToMem: Fail to get User Data.");
            return ret;
        }
    } else {
        if (At_ProcTpduCommandType(indexNum, tsDataInfo) != AT_SUCCESS) {
            return AT_ERROR;
        }
    }

    ret = MN_MSG_Encode(tsDataInfo, &sendDirectParm->msgInfo.tsRawData);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_SendTextMsgOrCmd: Fail to encode sms message or command.");
        return At_ChgMnErrCodeToAt(indexNum, ret);
    }

    g_atClientTab[indexNum].opId = opId;
    ret = MN_MSG_Send(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, sendDirectParm);

    return At_ProcSmsMsgOrCmdRslt(indexNum, ret);
}

/*
 * 功能描述: 将Submit类型Dcs原始数据解码为23038协议规定的数据
 */
LOCAL TAF_UINT32 AT_ProcSubmitTextMsgDecodeDcs(TAF_UINT8 indexNum, MN_MSG_SrvParam parmInUsim, MN_MSG_TsDataInfo *tsDataInfo)
{
    MN_MSG_DcsCode *dcs = VOS_NULL_PTR;
    errno_t memResult;
    TAF_UINT32 ret;

    tsDataInfo->tpduType = MN_MSG_TPDU_SUBMIT;

    /* Fo */
    At_SendMsgFoAttr(indexNum, tsDataInfo);

    /* Mr填0,由MN修改 */
    tsDataInfo->u.submit.mr = 0xff;

    /* Da */
    memResult = memcpy_s(&tsDataInfo->u.submit.destAddr, sizeof(tsDataInfo->u.submit.destAddr),
        &g_atClientTab[indexNum].atSmsData.asciiAddr, sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.submit.destAddr),
        sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));

    /* PID */
    tsDataInfo->u.submit.pid = parmInUsim.pid;

    /* DCS */
    dcs = &tsDataInfo->u.submit.dcs;
    ret = MN_MSG_DecodeDcs(parmInUsim.dcs, dcs);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("AT_ProcTextMsgDecodeDcs: Fail to decode DCS.");
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 将Deliver类型Dcs原始数据解码为23038协议规定的数据
 */
LOCAL TAF_UINT32 AT_ProcDeliverTextMsgDecodeDcs(TAF_UINT8 indexNum, MN_MSG_SrvParam parmInUsim, MN_MSG_TsDataInfo *tsDataInfo)
{
    MN_MSG_DcsCode *dcs = VOS_NULL_PTR;
    errno_t memResult;
    TAF_UINT32 ret;

    tsDataInfo->tpduType = MN_MSG_TPDU_DELIVER;

    /* Fo */
    At_SendMsgFoAttr(indexNum, tsDataInfo);

    /* Mr填0,由MN修改 */

    /* Da */
    memResult = memcpy_s(&tsDataInfo->u.deliver.origAddr, sizeof(tsDataInfo->u.submit.destAddr),
        &g_atClientTab[indexNum].atSmsData.asciiAddr, sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.submit.destAddr), sizeof(g_atClientTab[indexNum].atSmsData.asciiAddr));

    /* PID */
    tsDataInfo->u.deliver.pid = parmInUsim.pid;

    /* DCS */
    dcs = &tsDataInfo->u.deliver.dcs;
    ret = MN_MSG_DecodeDcs(parmInUsim.dcs, dcs);
    if (ret != MN_ERR_NO_ERROR) {
        AT_NORM_LOG("AT_ProcDeliverTextMsgDecodeDcs: Fail to decode DCS.");
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_WriteTextMsgToMem(TAF_UINT8 indexNum)
{
    errno_t            memResult;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;
    TAF_UINT32         ret;
    MN_MSG_WriteParm  *writeParm = VOS_NULL_PTR;
    AT_ModemSmsCtx    *smsCtx    = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGW_TEXT_SET) {
        AT_WARN_LOG("At_WriteTextMsgToMem: invalid command operation.");
        return AT_ERROR;
    }

    tsDataInfo           = At_GetMsgMem();
    writeParm            = At_GetWriteMsgMem();
    writeParm->writeMode = MN_MSG_WRITE_MODE_INSERT;
    writeParm->memStore  = smsCtx->cpmsInfo.memSendorWrite;
    writeParm->status    = g_atClientTab[indexNum].atSmsData.smState;

    if (smsCtx->cscaCsmpInfo.parmInUsim.scAddr.bcdLen == 0) {
        AT_NORM_LOG("At_WriteTextMsgToMem: no SCA.");
        return AT_ERROR;
    }
    memResult = memcpy_s(&writeParm->msgInfo.scAddr, sizeof(writeParm->msgInfo.scAddr),
                         &(smsCtx->cscaCsmpInfo.parmInUsim.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(writeParm->msgInfo.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr));

    if ((g_atClientTab[indexNum].atSmsData.smState == MN_MSG_STATUS_MO_NOT_SENT) ||
        (g_atClientTab[indexNum].atSmsData.smState == MN_MSG_STATUS_MO_SENT)) {
        /* 将Submit类型Dcs原始数据解码为23038协议规定的数据 */
        if (AT_ProcSubmitTextMsgDecodeDcs(indexNum, smsCtx->cscaCsmpInfo.parmInUsim, tsDataInfo) == AT_ERROR) {
            return AT_ERROR;
        }

        /* VP */
        memResult = memcpy_s(&tsDataInfo->u.submit.validPeriod, sizeof(tsDataInfo->u.submit.validPeriod),
            &(smsCtx->cscaCsmpInfo.vp), sizeof(smsCtx->cscaCsmpInfo.vp));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsDataInfo->u.submit.validPeriod), sizeof(smsCtx->cscaCsmpInfo.vp));

        /* UDL UD */
        ret = At_EncodeTextMsgTpUd(indexNum, tsDataInfo->u.submit.dcs.msgCoding, &tsDataInfo->u.submit.userData);
        if (ret != AT_SUCCESS) {
            AT_WARN_LOG("At_WriteTextMsgToMem: Fail to get User Data.");
            return ret;
        }
    } else {
        /* 将Deliver类型Dcs原始数据解码为23038协议规定的数据 */
        if (AT_ProcDeliverTextMsgDecodeDcs(indexNum, smsCtx->cscaCsmpInfo.parmInUsim, tsDataInfo) == AT_ERROR) {
            return AT_ERROR;
        }

        /* SCTS?? */

        /* UDL UD */
        ret = At_EncodeTextMsgTpUd(indexNum, tsDataInfo->u.deliver.dcs.msgCoding, &tsDataInfo->u.deliver.userData);

        if (ret != AT_SUCCESS) {
            AT_WARN_LOG("At_WriteTextMsgToMem: Fail to get User Data.");
            return ret;
        }
    }

    ret = MN_MSG_Encode(tsDataInfo, &writeParm->msgInfo.tsRawData);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_WriteTextMsgToMem: Fail to encode sms message or command.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].opId = At_GetOpId();
    ret = MN_MSG_Write(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, writeParm);
    if (ret != MN_ERR_NO_ERROR) {
        ret = AT_ERROR;
    } else {
        ret = AT_WAIT_ASYNC_RETURN;
    }
    return ret;
}

/*
 * 功能描述: 获取短信处理类型
 */
LOCAL VOS_UINT32 At_GetSmsTextProcType(TAF_UINT16 tmpLen, TAF_UINT8 *smsProcType, TAF_UINT8 *data)
{
    if (data[tmpLen] == AT_CTRL_Z) {
        *smsProcType = AT_SMS_INPUT_TYPE;

    } else if (g_atS3 == data[tmpLen]) {
        *smsProcType = AT_WAIT_SMS_INPUT_TYPE;

    } else {
        return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SmsTextProc(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len)
{
    TAF_UINT16 tmpLen      = len;
    TAF_UINT8  smsProcType = 0;
    TAF_UINT32 ret;
    errno_t    memResult;
    TAF_UINT32 cmdValidFLg;

    if (tmpLen < 1) {
        return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
    }

    if (At_GetSmsTextProcType(tmpLen - 1, &smsProcType, data)!= AT_SUCCESS) {
        return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
    }

    if (g_atCscsType != AT_CSCS_IRA_CODE) {
        tmpLen -= 1;

        if (At_UnicodePrint2Unicode(data, &tmpLen) == AT_FAILURE) {
            return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
        }

        /* UNICODE 长度判断 */
        if ((g_atClientTab[indexNum].smsTxtLen + tmpLen) > AT_UNICODE_SMS_MAX_LENGTH) {
            return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
        }
    } else {
        if (data[tmpLen - 1] == AT_CTRL_Z) {
            tmpLen -= 1;
        }

        /* ASCII 长度判断 */
        if ((g_atClientTab[indexNum].smsTxtLen + tmpLen) > AT_ASCII_SMS_MAX_LENGTH) {
            return AT_CMS_INVALID_TEXT_MODE_PARAMETER;
        }
    }

    cmdValidFLg = (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGS_TEXT_SET) ||
                  (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGW_TEXT_SET) ||
                  (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGC_TEXT_SET);

    if (cmdValidFLg == VOS_FALSE) {
        return AT_ERROR;
    }

    if (tmpLen > 0) {
        memResult = memcpy_s(&g_atClientTab[indexNum].atSmsData.buffer[g_atClientTab[indexNum].smsTxtLen],
                             AT_SMS_SEG_MAX_LENGTH - g_atClientTab[indexNum].smsTxtLen, data, tmpLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SMS_SEG_MAX_LENGTH - g_atClientTab[indexNum].smsTxtLen, tmpLen);
    }

    g_atClientTab[indexNum].smsTxtLen += tmpLen;

    /* 删除冗余else */
    if (smsProcType != 1) {
        return AT_WAIT_SMS_INPUT;
    }

    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_CMGS_TEXT_SET:
        case AT_CMD_CMGC_TEXT_SET:
            ret = At_SendTextMsgOrCmd(indexNum);
            break;

        default:
            ret = At_WriteTextMsgToMem(indexNum);
            break;
    }

    return ret;
}

TAF_UINT32 At_SmsProc(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 获取当前短信模式 */
    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) { /* TEXT */
        return At_SmsTextProc(indexNum, data, len);
    } else {
        /* 如果处理有问题，返回AT_ERROR */
        return At_SmsPduProc(indexNum, data, len); /* PDU */
    }
}

VOS_VOID AT_ProcSmsStream(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length, AT_ParseContext *clientCxt)
{
    AT_RreturnCodeUint32 result;

    result = (AT_RreturnCodeUint32)At_SmsProc(clientId, data, length);

    if ((result == AT_SUCCESS) || (result == AT_WAIT_SMS_INPUT)) {
        At_FormatResultData(clientId, result);
        return;
    }

    g_atClientTab[clientId].smsTxtLen = 0; /* 短信BUFFER清空 */

    At_SetCmdSubMode(clientId, AT_NORMAL_MODE);

    if (result == AT_WAIT_ASYNC_RETURN) {
        if (clientCxt->cmdElement != NULL) {
            if (At_StartTimer(clientCxt->cmdElement->setTimeOut, clientId) != AT_SUCCESS) {
                AT_ERR_LOG("atCmdMsgProc():ERROR:Start Timer Failed");
            }

            clientCxt->clientStatus = AT_FW_CLIENT_STATUS_PEND;
        }
        return;
    }

    At_FormatResultData(clientId, result);
}

VOS_UINT32 AT_RcvMtaSmsDomainQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg             *mtaMsg = VOS_NULL_PTR;
    MTA_AT_SmsDomainQryCnf *qryCnf = VOS_NULL_PTR;
    VOS_UINT32              result;

    /* 初始化消息变量 */
    mtaMsg = (AT_MTA_Msg *)msg;
    qryCnf = (MTA_AT_SmsDomainQryCnf *)mtaMsg->content;

    /* 判断查询操作是否成功 */
    if (qryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result = AT_OK;
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                qryCnf->smsDomain);
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}

VOS_UINT32 AT_RcvMmaSmsNasCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_SmsNasCapQryCnf *smsNasCapQryCnf = (TAF_MMA_SmsNasCapQryCnf *)msg;
    VOS_UINT32               result = 0;

    if (smsNasCapQryCnf->result == TAF_ERR_NO_ERROR) {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                smsNasCapQryCnf->smsNasCap);
        result = AT_OK;
    } else {
        result = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}
