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
#include "at_ltev_as_rslt_proc.h"
#include "securec.h"

#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"

#include "at_ltev_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_AS_RSLT_PROC_C

#if (FEATURE_LTEV == FEATURE_ON)
/* 将VTC结果码转化为AT返回值 */
const AT_ConvertVtcResultCodeTbl g_atChgVtcResultCodeTbl[] = {
    { VTC_AT_RESULT_OK, AT_OK },
    { VTC_AT_RESULT_ERR, AT_ERROR },
    { VTC_AT_RESULT_OPERATION_NOT_ALLOW, AT_CME_OPERATION_NOT_ALLOWED },
    { VTC_AT_RESULT_TIMEOUT, AT_DEVICE_TIMEOUT_ERR },
};

/* 将VRRC结果码转化为AT返回值 */
const AT_ConvertVrrcResultCodeTbl g_atChgVrrcResultCodeTbl[] = {
    { VRRC_AT_RESULT_OK, AT_OK },
    { VRRC_AT_RESULT_ERR, AT_ERROR },
    { VRRC_AT_RESULT_NO_SYNC_SOURCE, AT_CME_NOT_FOUND_SYNC_SOURCE },
};

AT_RreturnCodeUint32 AT_ConvertResultCodeFromVtc(VTC_AT_ResultCodeUint32 vtcResultCode)
{
    AT_RreturnCodeUint32 atReturnCode;
    VOS_UINT32           count;
    VOS_UINT32           i;

    count = AT_ARRAY_SIZE(g_atChgVtcResultCodeTbl);

    for (i = 0; i < count; i++) {
        if (g_atChgVtcResultCodeTbl[i].vtcResultCode == vtcResultCode) {
            atReturnCode = g_atChgVtcResultCodeTbl[i].atReturnCode;
            return atReturnCode;
        }
    }
    return AT_ERROR;
}

AT_RreturnCodeUint32 AT_ConvertResultCodeFromVrrc(VRRC_AT_ResultCodeUint32 vrrcResultCode)
{
    AT_RreturnCodeUint32 atReturnCode;
    VOS_UINT32           count;
    VOS_UINT32           i;

    count = AT_ARRAY_SIZE(g_atChgVrrcResultCodeTbl);

    for (i = 0; i < count; i++) {
        if (g_atChgVrrcResultCodeTbl[i].vrrcResultCode == vrrcResultCode) {
            atReturnCode = g_atChgVrrcResultCodeTbl[i].atReturnCode;
            return atReturnCode;
        }
    }
    return AT_ERROR;
}

VOS_UINT32 AT_RcvTestModeActiveStateSetCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_MsgSetCnf    *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CATM_SET) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_MsgSetCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    At_FormatResultData(indexNum, atReturnCode);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTestModeActiveStateQryCnf(VTC_AT_MsgCnf *msg)
{
    VTC_AT_TestModeActiveStateQryCnf *para = VOS_NULL_PTR;
    AT_RreturnCodeUint32              atReturnCode;
    VOS_UINT8                         indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CATM_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_TestModeActiveStateQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }
    if (para->para.state == AT_VTC_TEST_MODE_STATE_DEACTIVATED) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CATM: %d", para->para.state);
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CATM: %d,%d", para->para.state, para->para.type);
    }
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTestModeModifyL2IdSetCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_MsgSetCnf *para = VOS_NULL_PTR;
    VOS_UINT8 indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CV2XL2ID_SET) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para = (VTC_AT_MsgSetCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    At_FormatResultData(indexNum, atReturnCode);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTestModeModifyL2IdQryCnf(VTC_AT_MsgCnf *msg)
{
    VTC_AT_TestModeModifyL2IdQryCnf *para = VOS_NULL_PTR;
    AT_RreturnCodeUint32 atReturnCode;
    VOS_UINT8 indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CV2XL2ID_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_TestModeModifyL2IdQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^CV2XL2ID: %d,%d", para->para.srcId, para->para.dstId);

    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTestModeECloseStateSetCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_MsgSetCnf    *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CCUTLE_SET) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_MsgSetCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    At_FormatResultData(indexNum, atReturnCode);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTestModeECloseStateQryCnf(VTC_AT_MsgCnf *msg)
{
    VTC_AT_TestModeECloseStateQryCnf *para = VOS_NULL_PTR;
    AT_RreturnCodeUint32              atReturnCode;
    VOS_UINT8                         indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CCUTLE_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_TestModeECloseStateQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }
    /* 退出测试模式E状态 */
    if (para->para.status == AT_VTC_TEST_MODE_STATUS_OPEN) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CCUTLE: %d", para->para.status);
    } else {
        /* 数据接收状态 */
        if (para->para.direction == AT_VTC_COMMUNICATION_DIRECTION_RECEIVE) {
            if (para->para.monitorList[AT_CCUTLE_MONITOR_LIST_CONTEXT_MAX_NUM] != '\0') {
                At_FormatResultData(indexNum, AT_ERROR);
                return VOS_OK;
            }
            g_atSendDataBuff.bufLen =
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress, "+CCUTLE: %d,%d,%d,%d,\"%s\"", para->para.status,
                    para->para.direction, para->para.format, para->para.length, para->para.monitorList);
        } else {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "+CCUTLE: %d,%d", para->para.status, para->para.direction);
        }
    }
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSidelinkPacketCounterQryCnf(VTC_AT_MsgCnf *msg)
{
    VTC_AT_SidelinkPacketCounterQryCnf *para = VOS_NULL_PTR;
    AT_RreturnCodeUint32                atReturnCode;
    VOS_UINT8                           indexNum = 0;

    VOS_CHAR numOfPscchTransportBlocksbin[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1] = {0}; /* 2进制PSCCH块数 */
    VOS_CHAR numOfStchPdcpSduPacketsbin[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1] = {0};   /* 2进制PDCP SDU数 */
    VOS_CHAR numOfPsschTransportBlocksbin[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1] = {0}; /* 2进制PSSCH块数 */

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CUSPCREQ_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_SidelinkPacketCounterQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }

    /* 每种传输块或者数据包的LayerID下的数量都是用8位表示，这里检查字符串结尾，避免打印异常 */
    if ((para->para.numOfPscchTransportBlocks[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM] != '\0') ||
        (para->para.numOfStchPdcpSduPackets[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM] != '\0') ||
        (para->para.numOfPsschTransportBlocks[AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM] != '\0')) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    /* 数据包的数量由十六进制字符串转成二进制字符串 */
    (VOS_VOID)AT_HexStr2BinStr(para->para.numOfPscchTransportBlocks, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM + 1,
                               numOfPscchTransportBlocksbin, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1);
    (VOS_VOID)AT_HexStr2BinStr(para->para.numOfStchPdcpSduPackets, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM + 1,
                               numOfStchPdcpSduPacketsbin, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1);
    (VOS_VOID)AT_HexStr2BinStr(para->para.numOfPsschTransportBlocks, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM + 1,
                               numOfPsschTransportBlocksbin, AT_CCUTLE_MONITOR_LIST_PACKETS_MAX_NUM_BIN + 1);
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CUSPCREQ: %d,%d,%d,\"%s\",%d,%d,%d,\"%s\",%d,%d,%d,\"%s\"",
            AT_PSCCH_TRANSPORT_BLOCKS, AT_VTC_PACKET_COUNTER_FORMAT_BINARY, para->para.lenOfPscchTransportBlocks,
            numOfPscchTransportBlocksbin, AT_STCH_PDCP_SDU_PACKETS, AT_VTC_PACKET_COUNTER_FORMAT_BINARY,
            para->para.lenOfStchPdcpSduPackets, numOfStchPdcpSduPacketsbin, AT_PSSCH_TRANSPORT_BLOCKS,
            AT_VTC_PACKET_COUNTER_FORMAT_BINARY, para->para.lenOfPsschTransportBlocks,
            numOfPsschTransportBlocksbin);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvResetUtcTimeSetCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_MsgSetCnf    *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CUTCR_SET) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_MsgSetCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    At_FormatResultData(indexNum, atReturnCode);
    return VOS_OK;
}

VOS_UINT32 AT_RcvVtcCbrQryCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_CbrQryCnf    *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CCBRREQ_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_CbrQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }
    if (para->para.isPscchCbrValid == VOS_TRUE) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CCBRREQ: %d,%d", para->para.psschCbr, para->para.pscchCbr);
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CCBRREQ: %d", para->para.psschCbr);
    }
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSendingDataActionSetCnf(VTC_AT_MsgCnf *msg)
{
    AT_RreturnCodeUint32 atReturnCode;
    VTC_AT_MsgSetCnf    *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CV2XDTS_SET) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_MsgSetCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    At_FormatResultData(indexNum, atReturnCode);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSendingDataActionQryCnf(VTC_AT_MsgCnf *msg)
{
    VTC_AT_SendingDataActionQryCnf *para = VOS_NULL_PTR;
    AT_RreturnCodeUint32            atReturnCode;
    VOS_UINT8                       indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CV2XDTS_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para         = (VTC_AT_SendingDataActionQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVtc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }
    if (para->para.action == AT_VTC_SENDING_DATA_ACTION_STOP) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CV2XDTS: %d", para->para.action);
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "+CV2XDTS: %d,%d,%d", para->para.action, para->para.dataSize,
            para->para.periodicity);
    }
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncSourceQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_SyncSourceStateQryCnf *para     = VOS_NULL_PTR;
    VOS_UINT8                      indexNum = 0;
    AT_RreturnCodeUint32           atReturnCode;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSYNCSRC_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para = (VRRC_AT_SyncSourceStateQryCnf *)msg->content;
    atReturnCode = AT_ConvertResultCodeFromVrrc(para->result);
    if (atReturnCode != AT_OK) {
        At_FormatResultData(indexNum, atReturnCode);
        return VOS_OK;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^VSYNCSRC: %d,%d,%d,%d,%d", para->para.type, para->para.status,
        para->para.earfcn, para->para.slssId, para->para.subSlssId);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncSourceRptSetCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_SimpleMsgCnf *para     = VOS_NULL_PTR;
    VOS_UINT8             indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSYNCSRCRPT_SET) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);
    para = (VRRC_AT_SimpleMsgCnf *)msg->content;

    if (para->result == VOS_OK) {
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }
    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncSourceRptQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_SyncSourceRptSwitchQryCnf *para     = VOS_NULL_PTR;
    VOS_UINT8                          indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSYNCSRCRPT_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para = (VRRC_AT_SyncSourceRptSwitchQryCnf *)msg->content;
    if (para->result != VOS_OK) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^VSYNCSRCRPT: %d", para->para.status);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncModeQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_SyncModeQryCnf *para     = VOS_NULL_PTR;
    VOS_UINT8               indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSYNCMODE_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para = (VRRC_AT_SyncModeQryCnf *)msg->content;
    if (para->result != VOS_OK) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^VSYNCMODE: %d", para->para.mode);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncModeSetCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_SimpleMsgCnf *para     = VOS_NULL_PTR;
    VOS_UINT8             indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSYNCMODE_SET) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);
    para = (VRRC_AT_SimpleMsgCnf *)msg->content;

    if (para->result == VOS_OK) {
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }
    return VOS_OK;
}

VOS_UINT32 AT_RcvVmacCbrQryCnf(MN_AT_IndEvt *msg)
{
    VMAC_AT_CbrQryCnf *para     = VOS_NULL_PTR;
    VOS_UINT8          indexNum = 0;

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CBR_QRY) {
        return VOS_ERR;
    }
    AT_StopTimerCmdReady(indexNum);
    para = (VMAC_AT_CbrQryCnf *)msg->content;
    if (para->result != VOS_OK) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^CBR: %d", para->para.psschCbr);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvGnssTestStartCnf(MN_AT_IndEvt *msg)
{
    VOS_UINT32 result = AT_ERROR;
    VOS_UINT8  indexNum = 0;

    /* 初始化 */

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvGnssTestStartCnf:WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvGnssTestStartCnf : AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_GNSS_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_GNSS_SET) {
        AT_WARN_LOG("AT_RcvGnssTestStartCnf : AT_CMD_GNSS_SET.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    if (msg->content[0] == VOS_OK) {
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvGnssInfoQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_GnssInfoQryCnf *gnssQryCnf = VOS_NULL_PTR;
    AT_MTA_FileRdReq        atFileRdReq;
    VOS_UINT32              result;
    VOS_UINT16              length;
    VOS_UINT8               indexNum   = 0;
    errno_t                 memResult;
    /* 初始化 */
    gnssQryCnf = (VRRC_AT_GnssInfoQryCnf *)msg->content;

    memResult = memset_s(&atFileRdReq, sizeof(atFileRdReq), 0x00, sizeof(AT_MTA_FileRdReq));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atFileRdReq), sizeof(AT_MTA_FileRdReq));

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvGnssInfoQryCnf:WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvGnssInfoQryCnf : AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_DATA_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DATA_QRY) {
        AT_WARN_LOG("AT_RcvGnssInfoQryCnf : AT_CMD_DATA_QRY.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^GNSSTEST查询命令返回 */
    if (gnssQryCnf->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvGnssInfoQryCnf : AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_INT32)(gnssQryCnf->phy1ppsStat), (VOS_INT32)(gnssQryCnf->syncStat), (VOS_INT32)(gnssQryCnf->satNum));

        g_atSendDataBuff.bufLen = length;
    }
    /* 输出结果 */
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_UINT32 AT_RcvGnssDebugInfoGetCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_GnssInfoGetCnf *gnssInfo = VOS_NULL_PTR;
    VOS_UINT32              result   = AT_OK;
    VOS_UINT16              length   = 0;
    VOS_UINT8               indexNum = 0;
    VOS_UINT32              loop     = 0;

    /* 初始化 */
    gnssInfo = (VRRC_AT_GnssInfoGetCnf *)msg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvGnssDebugInfoGetCnf:WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvGnssDebugInfoGetCnf : AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_GNSSINFO_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_GNSSINFO_QRY) {
        AT_WARN_LOG("AT_RcvGnssDebugInfoGetCnf : AT_CMD_GNSSINFO_QRY.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^GNSSINFO查询命令返回 */
    if (gnssInfo->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvGnssDebugInfoGetCnf : AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;
        if (gnssInfo->satNum == 0) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s:%d%s", g_parseContext[indexNum].cmdElement->cmdName,
                gnssInfo->satNum, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%s%s,%s%s%d%s",
                g_parseContext[indexNum].cmdElement->cmdName, g_atCrLf, gnssInfo->longitude, gnssInfo->latitude,
                g_atCrLf, gnssInfo->satNum, g_atCrLf);

            for (loop = 0; loop < gnssInfo->satNum; loop++) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "No%d:%d    %d%s", loop,
                gnssInfo->sysInfo[loop].gnssSystem, gnssInfo->sysInfo[loop].cnr, g_atCrLf);
            }
        }
        g_atSendDataBuff.bufLen = length;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvRsuVphyStatQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_VphyStatQryCnf *vphyStatCnf = VOS_NULL_PTR;
    VOS_UINT32              result;
    VOS_UINT16              length;
    VOS_UINT8               indexNum = 0;

    /* 初始化 */
    vphyStatCnf = (VRRC_AT_VphyStatQryCnf *)msg->content;
    length      = 0;
    result      = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvRsuVphyStatQryCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvRsuVphyStatQryCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_VPHYSTAT_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VPHYSTAT_QRY) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^VPHYSTAT查询命令返回 */
    if (vphyStatCnf->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvRsuVphyStatQryCnf: AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            vphyStatCnf->firstSaDecSucSum, vphyStatCnf->secSaDecSucSum, vphyStatCnf->twiSaDecSucSum,
            vphyStatCnf->uplinkPackSum, vphyStatCnf->downlinkPackSum, vphyStatCnf->phyUplinkPackSum);

        g_atSendDataBuff.bufLen = length;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvRsuVphyStatClrSetCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_VphyStatClrSetCnf *vphyStatClrSetCnf;
    VOS_UINT32                 result = AT_ERROR;
    VOS_UINT8                  indexNum = 0;

    /* 初始化消息变量 */
    vphyStatClrSetCnf = (VRRC_AT_VphyStatClrSetCnf *)msg->content;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvRsuVphyStatClrSetCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvRsuVphyStatClrSetCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_VPHYSTATCLR_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VPHYSTATCLR_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^VPHYSTATCLR底层返回值 */
    if (vphyStatClrSetCnf->result == VOS_OK) {
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvRsuVsnrRsrpQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_VSnrRsrpQryCnf *vsnrRsrpQryCnf = VOS_NULL_PTR;
    VOS_UINT32              result;
    VOS_UINT8               indexNum = 0;
    VOS_UINT8               loop;

    /* 初始化 */
    vsnrRsrpQryCnf = (VRRC_AT_VSnrRsrpQryCnf *)msg->content;
    result         = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvRsuVsnrRsrpQryCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvRsuVsnrRsrpQryCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_VSNRRSRP_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VSNRRSRP_QRY) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^VSNRRSRP查询命令返回 */
    if (vsnrRsrpQryCnf->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvRsuVsnrRsrpQryCnf: AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s:%d%s",
            g_parseContext[indexNum].cmdElement->cmdName, vsnrRsrpQryCnf->msgNum, g_atCrLf);
        vsnrRsrpQryCnf->msgNum = AT_MIN(vsnrRsrpQryCnf->msgNum, VRRC_AT_MAX_RSRP_NUM);
        for (loop = 0; loop < vsnrRsrpQryCnf->msgNum; ++loop) {
            g_atSendDataBuff.bufLen +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                    "%s: %5d      %5d      %5d      %5d      %5d      %5d      %5d",
                    g_parseContext[indexNum].cmdElement->cmdName,
                    vsnrRsrpQryCnf->saMainSNR[loop], vsnrRsrpQryCnf->saDiversitySNR[loop],
                    vsnrRsrpQryCnf->daMainSNR[loop], vsnrRsrpQryCnf->daDiversitySNR[loop],
                    vsnrRsrpQryCnf->daRsrp[loop], vsnrRsrpQryCnf->daMainMaxRsrp, vsnrRsrpQryCnf->daDiversityMaxRsrp);
            if (loop != vsnrRsrpQryCnf->msgNum - 1) {
                g_atSendDataBuff.bufLen +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s", g_atCrLf);
            }
        }
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvRsuV2xRssiQryCnf(MN_AT_IndEvt *msg)
{
    VRRC_AT_RsuRssiQryCnf *rssiQryCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT8              indexNum = 0;

    /* 初始化 */
    rssiQryCnf = (VRRC_AT_RsuRssiQryCnf *)msg->content;
    result     = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvRsuV2xRssiQryCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvRsuV2xRssiQryCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为 AT_CMD_V2XRSSI_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_V2XRSSI_QRY) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化 AT^V2XRSSI 查询命令返回 */
    if (rssiQryCnf->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvRsuV2xRssiQryCnf: AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s:%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, rssiQryCnf->daMainRSSIMax, rssiQryCnf->daDiversityRSSIMax);
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvVrrcMsgSetTxPowerCnfProc(MN_AT_IndEvt *msg)
{
    VRRC_AT_SetTxPowerCnf         *powerOutputQryCnf = VOS_NULL_PTR;
    VOS_UINT32                     result = AT_ERROR;
    VOS_UINT8                      indexNum = 0;

    /* 初始化消息变量 */
    powerOutputQryCnf = (VRRC_AT_SetTxPowerCnf *)msg->content;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvVrrcMsgSetTxPowerCnfProc: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvVrrcMsgSetTxPowerCnfProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VTXPOWER_SET) {
        AT_WARN_LOG("AT_RcvVrrcMsgSetTxPowerCnfProc : Current Option is not AT_CMD_CEMODE_READ.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* phy会将实际设置的功率返回回来，对返回的功率进行粗略范围检查 */
    if (powerOutputQryCnf->result == VOS_OK) {
        if ((powerOutputQryCnf->txPower >= AT_SET_TX_POWER_MIN) &&
            (powerOutputQryCnf->txPower <= AT_SET_TX_POWER_MAX)) {
            result = AT_OK;
        } else {
            AT_WARN_LOG1("AT_RcvVrrcMsgSetTxPowerCnfProc, WARNING, Return result %d!", result);
            result = AT_ERROR;
        }
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#endif

