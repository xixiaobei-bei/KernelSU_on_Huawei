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
#include "at_device_taf_rslt_proc.h"
#include "securec.h"
#include "taf_app_mma.h"
#include "at_ctx.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_event_report.h"
#include "at_device_comm.h"
#include "osm.h"
#include "at_external_module_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_TAF_RSLT_PROC_C

#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
AT_UartTestStateUint8 AT_ConvertUartTestResult(MTA_AT_UartTestRsltUint32 uartTestRslt)
{
    AT_UartTestStateUint8 returnCode;

    switch (uartTestRslt) {
        case MTA_AT_UART_TEST_RSLT_SUCCESS:
            returnCode = AT_UART_TEST_PASS;
            break;
        case MTA_AT_UART_TEST_RSLT_DATA_ERROR:
            returnCode = AT_UART_TEST_DATA_ERROR;
            break;
        case MTA_AT_UART_TEST_RSLT_ICC_NOT_OPEN:
            returnCode = AT_UART_TEST_ICC_NOT_OPEN;
            break;
        case MTA_AT_UART_TEST_RSLT_TIME_OUT:
            returnCode = AT_UART_TEST_TIMEOUT;
            break;
        default:
            returnCode = AT_UART_TEST_DATA_ERROR;
            break;
    }

    return returnCode;
}

VOS_UINT32 AT_RcvMtaUartTestRsltInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg             *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_UartTestRsltInd *uartTestRsltInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    rcvMsg          = (AT_MTA_Msg *)msg;
    uartTestRsltInd = (MTA_AT_UartTestRsltInd *)rcvMsg->content;

    AT_SetUartTestState(AT_ConvertUartTestResult(uartTestRsltInd->uartTestRslt));

    return VOS_OK;
}
#endif

#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_VOID At_RcvVcI2sTestCnfProc(MN_AT_IndEvt *event)
{
    APP_VC_I2sTestCnf *i2sTestCnf = VOS_NULL_PTR;
    VOS_UINT8          indexId = 0;
    VOS_UINT32         ret;

    /* 初始化 */

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->clientId, AT_CMD_I2S_TEST_SET, &indexId) != VOS_OK) {
        return;
    }

    i2sTestCnf = (APP_VC_I2sTestCnf *)event->content;

    if (i2sTestCnf->result == VOS_OK) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexId);
    At_FormatResultData(indexId, ret);
}

AT_I2sTestStateUint8 AT_ConvertI2sTestResult(APP_VC_I2sTestRsltUint32 i2sTestRslt)
{
    VOS_UINT8 returnCode;

    switch (i2sTestRslt) {
        case APP_VC_I2S_TEST_RSLT_SUCCESS:
            returnCode = AT_I2S_TEST_PASS;
            break;
        case APP_VC_I2S_TEST_RSLT_FAILED:
            returnCode = AT_I2S_TEST_FAILED;
            break;
        case APP_VC_I2S_TEST_RSLT_TIME_OUT:
            returnCode = AT_I2S_TEST_TIMEOUT;
            break;
        default:
            returnCode = AT_I2S_TEST_FAILED;
            break;
    }
    return returnCode;
}

VOS_VOID AT_RcvVcI2sTestRsltIndProc(MN_AT_IndEvt *msg)
{
    APP_VC_I2sTestRsltInd *i2sTestRsltInd = VOS_NULL_PTR;

    i2sTestRsltInd = (APP_VC_I2sTestRsltInd *)msg->content;

    AT_SetI2sTestState(AT_ConvertI2sTestResult(i2sTestRsltInd->result));
}
#endif

VOS_UINT32 AT_RcvMmaPhoneModeSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PhoneModeSetCnf *cnfMsg = (TAF_MMA_PhoneModeSetCnf *)msg;
    VOS_UINT32               result;
#if (FEATURE_LTE == FEATURE_ON)
    ModemIdUint16            modemId;
    VOS_UINT8                sptLteFlag;
    VOS_UINT8                sptUtralTDDFlag;
#endif

    result   = AT_FAILURE;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvMmaPhoneModeSetCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    /* V7R2 ^PSTANDBY命令复用关机处理流程 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PSTANDBY_SET) {
        AT_StopTimerCmdReady(indexNum);
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (cnfMsg->rslt == TAF_MMA_APP_OPER_RESULT_SUCCESS) {
        result = AT_OK;
    } else if (cnfMsg->errorCause == TAF_ERR_RCV_START_WHEN_1X_ACTIVE) {
        result = AT_ERR_REPEAT_START_CDMA;
    } else {
        result = At_ChgTafErrorCode(indexNum, cnfMsg->errorCause); /* 发生错误 */
    }

#if ((FEATURE_LTE == FEATURE_ON) || (FEATURE_UE_MODE_TDS == FEATURE_ON))
    /* 如果GU处理结果正确，则发送到TL测并等待结果 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SET_TMODE && result == AT_OK) {
        modemId  = MODEM_ID_0;
        if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
            AT_ERR_LOG1("AT_RcvMmaPhoneModeSetCnf:Get ModemID From ClientID fail,ClientID:", indexNum);

            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        sptLteFlag      = AT_IsModemSupportRat(modemId, TAF_MMA_RAT_LTE);
        sptUtralTDDFlag = AT_IsModemSupportUtralTDDRat(modemId);

        if ((sptLteFlag == VOS_TRUE) || (sptUtralTDDFlag == VOS_TRUE)) {
            AT_ProcOperModeWhenLteOn(indexNum);
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }
    }
#endif

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if (FEATURE_LTE == FEATURE_ON)

TAF_UINT32 AT_ProcOperModeWhenLteOn(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
#else
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    atDevCmdCtrl = AT_GetDevCmdCtrl();
    return atSetTmodePara(indexNum, atDevCmdCtrl->currentTMode);
#else
    atMtInfoCtx = AT_GetMtInfoCtx();
    return atSetTmodePara(indexNum, atMtInfoCtx->currentTMode);
#endif
}
#endif

TAF_UINT32 At_ProcPhoneEvtOperModeCnf(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result;
#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT8 sptLteFlag;
    VOS_UINT8 sptUtralTDDFlag;
#endif

    ModemIdUint16 modemId = MODEM_ID_0;

    result = AT_GetModemIdFromClient(indexNum, &modemId);

    if (result != VOS_OK) {
        AT_ERR_LOG1("At_ProcPhoneEvtOperModeCnf:Get ModemID From ClientID fail,ClientID:", indexNum);
        return AT_RRETURN_CODE_BUTT;
    }

    if (event->opPhoneError == 1) {                               /* MT本地错误 */
        result = At_ChgTafErrorCode(indexNum, event->phoneError); /* 发生错误 */
    } else if (event->operMode.cmdType == TAF_PH_CMD_QUERY) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d", event->operMode.phMode);
        result = AT_OK;
    } else {
        result = AT_OK;
    }

    /* V7R2 ^PSTANDBY命令复用关机处理流程 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PSTANDBY_SET) {
        AT_StopTimerCmdReady(indexNum);

        return AT_RRETURN_CODE_BUTT;
    }

#if ((FEATURE_LTE == FEATURE_ON) || (FEATURE_UE_MODE_TDS == FEATURE_ON))
    /* 如果GU处理结果正确，则发送到TL测并等待结果 */
    if (result == AT_OK) {
        sptLteFlag      = AT_IsModemSupportRat(modemId, TAF_MMA_RAT_LTE);
        sptUtralTDDFlag = AT_IsModemSupportUtralTDDRat(modemId);

        if ((sptLteFlag == VOS_TRUE) || (sptUtralTDDFlag == VOS_TRUE)) {
            if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SET_TMODE) {
                AT_ProcOperModeWhenLteOn(indexNum);

                return AT_RRETURN_CODE_BUTT;
            }
        }
    }
#endif

    AT_StopTimerCmdReady(indexNum);

    return result;
}

VOS_UINT32 AT_RcvDrvAgentSetDatalockRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg            *rcvMsg         = VOS_NULL_PTR;
    DRV_AGENT_DatalockSetCnf *datalockSetCnf = VOS_NULL_PTR;
    VOS_UINT8                 indexNum       = 0;
    VOS_UINT32                result;

    /* 初始化 */
    rcvMsg         = (DRV_AGENT_Msg *)msg;
    datalockSetCnf = (DRV_AGENT_DatalockSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(datalockSetCnf->atAppCtrl.clientId, AT_CMD_DATALOCK_SET, &indexNum) !=
        VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (datalockSetCnf->fail != VOS_TRUE) {
        /* 设置dATalock已解锁 */
        g_atDataLocked = VOS_FALSE;
        result         = AT_OK;
    } else {
        result = AT_ERROR;
    }

    /* 调用AT_FormAtResultData发送命令结果  */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSetMaxLockTmsRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg              *rcvMsg           = VOS_NULL_PTR;
    DRV_AGENT_MaxLockTmsSetCnf *maxlockTmsSetCnf = VOS_NULL_PTR;
    VOS_UINT8                   indexNum         = 0;

    /* 初始化消息 */
    rcvMsg           = (DRV_AGENT_Msg *)msg;
    maxlockTmsSetCnf = (DRV_AGENT_MaxLockTmsSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(maxlockTmsSetCnf->atAppCtrl.clientId, AT_CMD_MAXLCKTMS_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^MAXLCKTMS设置命令返回 */
    if (maxlockTmsSetCnf->result != VOS_OK) {
        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_DEVICE_OTHER_ERROR);
    } else {
        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_OK);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSetSimlockCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_SetSimlockCnf *event  = VOS_NULL_PTR;
    VOS_UINT32               ret;
    VOS_UINT8                indexNum = 0;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_SetSimlockCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_DRV_AGENT_SIMLOCK_SET_REQ, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);

    if (event->result == DRV_AGENT_NO_ERROR) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    At_FormatResultData(indexNum, ret);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSetNvRestoreCnf(struct MsgCB *msg)
{
    VOS_UINT8               indexNum = 0;
    DRV_AGENT_Msg          *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_NvrestoreRst *event    = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvrestoreRst *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_NVRESTORE_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%d", event->result);

    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentQryNvRestoreRstCnf(struct MsgCB *msg)
{
    VOS_UINT8               indexNum = 0;
    VOS_UINT32              restoreStatus;
    DRV_AGENT_Msg          *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_NvrestoreRst *event  = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvrestoreRst *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_NVRSTSTTS_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);

    if ((event->result == AT_NV_RESTORE_RESULT_INIT) || (event->result == AT_NV_RESTORE_RUNNING)) {
        restoreStatus = AT_NV_RESTORE_FAIL;
    } else if (event->result == VOS_OK) {
        restoreStatus = AT_NV_RESTORE_SUCCESS;
    } else {
        restoreStatus = AT_NV_RESTORE_FAIL;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%d", restoreStatus);

    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_VOID AT_PhSendRestoreFactParm(VOS_VOID)
{
    VOS_UINT16 length;
    VOS_UINT32 i;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if (g_atClientTab[i].userType == AT_APP_USER) {
            break;
        }
    }

    /* 未找到E5 User,则不用上报 */
    if (i >= AT_MAX_CLIENT_NUM) {
        return;
    }

    length = 0;
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^FACTORY");
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData((VOS_UINT8)i, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvDrvAgentNvRestoreManuDefaultRsp(struct MsgCB *msg)
{
    VOS_UINT8               indexNum = 0;
    VOS_UINT32              rst;
    VOS_UINT32              result;
    DRV_AGENT_Msg          *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_NvrestoreRst *event  = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvrestoreRst *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_F_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    result = event->result;
    if (result != NV_OK) {
        rst = AT_ERROR;
    } else {
        /* E5通知APP恢复用户设置  */
        AT_PhSendRestoreFactParm();

        rst = AT_OK;
    }
    At_FormatResultData(indexNum, rst);

    return VOS_OK;
}

VOS_UINT32 AT_RcvSimLockQryRsp(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MMA_SimlockStatus *event = (AT_MMA_SimlockStatus *)msg;
    VOS_UINT32            ret;

    /* 复位AT状态 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SIMLOCKSTATUS_READ) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", "^SIMLOCK", event->simlockEnableFlg);
        ret                     = AT_OK;

    } else {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_OutputCsdfltDefault(indexNum, event->simlockEnableFlg);
    }

    return ret;
}

VOS_UINT32 AT_RcvDrvAgentFacAuthPubkeySetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                 *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_FacauthpubkeySetCnf *event  = VOS_NULL_PTR;
    VOS_UINT8                      indexNum;
    VOS_UINT32                     result;

    AT_PR_LOGI("enter");

    /* 初始化 */
    rcvMsg   = (DRV_AGENT_Msg *)msg;
    event    = (DRV_AGENT_FacauthpubkeySetCnf *)rcvMsg->content;
    indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    result   = VOS_NULL;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_FACAUTHPUBKEYEX_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentIdentifyEndSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg               *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_IdentifyendSetCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                    indexNum = 0;
    VOS_UINT32                   result;

    AT_PR_LOGI("enter");

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_IdentifyendSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_IDENTIFYEND_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSimlockDataWriteSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                    *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_SimlockdatawriteSetCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                         indexNum = 0;
    VOS_UINT32                        result;

    AT_PR_LOGI("enter");

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_SimlockdatawriteSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_SIMLOCKDATAWRITE_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentPhonePhynumSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg               *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_PhonephynumSetCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                    indexNum = 0;
    VOS_UINT32                   result;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_PhonephynumSetCnf *)rcvMsg->content;

    AT_PR_LOGI("enter");

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_PHONEPHYNUM_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentOpwordSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg          *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_OpwordSetCnf *event    = VOS_NULL_PTR;
    VOS_UINT8               indexNum = 0;
    VOS_UINT32              result;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_OpwordSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_OPWORD_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 密码比对成功，获取权限 */
        g_ate5RightFlag = AT_E5_RIGHT_FLAG_YES;
        result          = AT_OK;
    } else {
        /* 输出设置结果, 转换错误码 */
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

/*
 * 功能描述: TL C核模式设置返回处理函数
 输入参数  : ucClientId Client ID
 *           pMsgBlock  消息内容
 */
VOS_UINT32 atSetTmodeParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG           *event    = NULL;
    SYM_SET_TmodeCnf *tmodeCnf = NULL;

    /* pMsgBlock 在上层调用中已判断是否为空
 */
    event    = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    tmodeCnf = (SYM_SET_TmodeCnf *)event->param1;

    HAL_SDMLOG("\n atSetTmodeParaCnfProc, ulErrCode=0x%x\n", (VOS_INT)tmodeCnf->errCode);

    CmdErrProc(clientId, tmodeCnf->errCode, 0, NULL);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetFLNAParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG         *event = NULL;
    FTM_SetAagcCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetAagcCnf *)event->param1;

    CmdErrProc(clientId, cnf->errCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atQryFLNAParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    FTM_RdAagcCnf *cnf   = NULL;
    OS_MSG        *event = NULL;
    VOS_UINT16     length;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdAagcCnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FLNA:%d", cnf->aggcLvl);

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

