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
#include "at_voice_event_report.h"
#include "taf_type_def.h"
#include "securec.h"

#include "ppp_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "at_oam_interface.h"
#include "mn_comm_api.h"

#include "at_input_proc.h"
#include "at_cmd_msg_proc.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "gen_msg.h"
#include "at_lte_common.h"
#endif

#include "taf_app_mma.h"

#include "app_vc_api.h"
#include "taf_app_rabm.h"

#include "product_config.h"

#include "taf_std_lib.h"

#include "at_msg_print.h"

#include "mnmsgcbencdec.h"
#include "dms_file_node_i.h"
#include "at_cmd_msg_proc.h"
#include "at_mdrv_interface.h" /* DRV_OS_STATUS_SWITCH AT_SetModemState FREE_MEM_SIZE_GET */
#include "dms_msg_chk.h"
#include "dms_port_i.h"

#include "at_event_report.h"
#include "at_ss_comm.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_EVENT_REPORT_C

#define AT_OUTPUT_MAX_LENGTH 2

MODULE_EXPORTED VOS_UINT16 AT_GetRealClientId(ModemIdUint16 modemId)
{
    if (modemId == MODEM_ID_0) {
        return AT_BROADCAST_CLIENT_ID_MODEM_0;
    }

    if (modemId == MODEM_ID_1) {
        return AT_BROADCAST_CLIENT_ID_MODEM_1;
    }

    if (modemId == MODEM_ID_2) {
        return AT_BROADCAST_CLIENT_ID_MODEM_2;
    }

    AT_ERR_LOG1("AT_GetRealClientId, enModemId err", modemId);

    return MN_CLIENT_ID_BROADCAST;
}

VOS_VOID AT_ReportCCallstateResult(ModemIdUint16 modemId, VOS_UINT8 callId, VOS_UINT8 *rptCfg,
                                   AT_CS_CallStateUint8 callState, TAF_CALL_VoiceDomainUint8 voiceDomain)
{
    VOS_UINT16 length   = 0;
    VOS_UINT8  indexNum = 0;

    VOS_UINT16 clientId;

    clientId = AT_GetRealClientId(modemId);

    /* 获取client id对应的Modem Id */
    /* 通过clientid获取index */
    if (At_ClientIdToUserId(clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ReportCCallstateResult:WARNING:AT INDEX NOT FOUND!");
        return;
    }

    if (AT_CheckRptCmdStatus(rptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CALLSTATE) == VOS_TRUE) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s: %d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CCALLSTATE].text, callId, callState, voiceDomain, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID At_ChangeEcallTypeToCallType(MN_CALL_TypeUint8 ecallType, MN_CALL_TypeUint8 *callType)
{
    switch (ecallType) {
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
        case MN_CALL_TYPE_PSAP_ECALL:
            *callType = MN_CALL_TYPE_VOICE;
            break;

        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
            *callType = MN_CALL_TYPE_EMERGENCY;
            break;

        default:
            *callType = ecallType;
            break;
    }
}

LOCAL VOS_UINT32 AT_PrintCallConnectIndBroadcastConn(TAF_CCM_CallConnectInd *connectIndMsg, ModemIdUint16 modemId,
    VOS_UINT8 indexNum)
{
    TAF_UINT16              length        = 0;
    MN_CALL_TypeUint8       newCallType;

    newCallType = MN_CALL_TYPE_VOICE;
    At_ChangeEcallTypeToCallType(connectIndMsg->connectIndPara.callType, &newCallType);

    if (AT_CheckRptCmdStatus(connectIndMsg->connectIndPara.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONN) ==
        VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^CONN:%d,%d%s", g_atCrLf,
            connectIndMsg->connectIndPara.callId, newCallType, g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    AT_ReportCCallstateResult(modemId, connectIndMsg->connectIndPara.callId,
                              connectIndMsg->connectIndPara.curcRptCfg, AT_CS_CALL_STATE_CONNECT,
                              connectIndMsg->connectIndPara.voiceDomain);

    return VOS_OK;
}

LOCAL VOS_UINT32 AT_PrintCallConnectIndVoiceCallConn(TAF_CCM_CallConnectInd *connectIndMsg, ModemIdUint16 modemId,
    VOS_UINT8 indexNum)
{
    TAF_UINT16              length        = 0;
    MN_CALL_TypeUint8       newCallType;

    newCallType = MN_CALL_TYPE_VOICE;
    At_ChangeEcallTypeToCallType(connectIndMsg->connectIndPara.callType, &newCallType);

    if (AT_CheckRptCmdStatus(connectIndMsg->connectIndPara.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONN) ==
        VOS_TRUE) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "^CONN:%d", connectIndMsg->connectIndPara.callId);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", newCallType);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
    AT_ReportCCallstateResult(modemId, connectIndMsg->connectIndPara.callId,
                              connectIndMsg->connectIndPara.curcRptCfg, AT_CS_CALL_STATE_CONNECT,
                              connectIndMsg->connectIndPara.voiceDomain);

    return VOS_OK;
}

VOS_VOID AT_ReportCendResult(VOS_UINT8 indexNum, TAF_CCM_CallReleasedInd *callReleaseInd)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;
    VOS_UINT32    causeValue;


    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_ReportCendResult: Get modem id fail.");
        return;
    }

    if (AT_CheckRptCmdStatus(callReleaseInd->releaseIndPara.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CEND) ==
        VOS_TRUE) {
            /* 车载产品使用呼叫挂断来源代替NOCLI */
            if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->cendWithEndSorceFlag == VOS_TRUE) {
                causeValue = callReleaseInd->releaseIndPara.endSource;
            } else {
                causeValue = callReleaseInd->releaseIndPara.noCliCause;
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s^CEND:%d,%d,%d,%d%s", g_atCrLf,
                callReleaseInd->releaseIndPara.callId,
                callReleaseInd->releaseIndPara.preCallTime,
                causeValue, callReleaseInd->releaseIndPara.cause, g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
}

PS_BOOL_ENUM_UINT8 At_CheckReportCendCallType(MN_CALL_TypeUint8 callType)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
        case MN_CALL_TYPE_PSAP_ECALL:
            return PS_TRUE;
        default:
            return PS_FALSE;
    }
}

VOS_VOID AT_CsRspEvtReleasedProc(TAF_UINT8 indexNum, TAF_CCM_CallReleasedInd *callReleaseInd)
{
    TAF_UINT32     result = AT_FAILURE;
    TAF_UINT16     length = 0;
    VOS_UINT32     timerName;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* 停止RING TE */
    AT_VoiceStopRingTe(callReleaseInd->releaseIndPara.callId);
#endif

    /* 记录cause值和文本信息 */
    AT_UpdateCallErrInfo(indexNum, callReleaseInd->releaseIndPara.cause, &(callReleaseInd->releaseIndPara.errInfoText));

    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CHUP_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_H_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CHLD_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CTFR_SET)) {
        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            timerName = ccCtx->s0TimeInfo.timerName;

            AT_StopRelTimer(timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = VOS_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        AT_ReportCendResult(indexNum, callReleaseInd);

        return;
    } else {
        /*
         * 需要增加来电类型，传真，数据，可视电话，语音呼叫
         */

        /* g_atClientTab[ucIndex].ulCause没有使用点，赋值点删除 */

        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            timerName = ccCtx->s0TimeInfo.timerName;

            AT_StopRelTimer(timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = VOS_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        /* 上报CEND，可视电话不需要上报^CEND */
        if ((At_CheckReportCendCallType(callReleaseInd->releaseIndPara.callType) == PS_TRUE) ||
            (AT_EVT_IS_PS_VIDEO_CALL(callReleaseInd->releaseIndPara.callType,
                                     callReleaseInd->releaseIndPara.voiceDomain))) {
            AT_ReportCendResult(indexNum, callReleaseInd);

            return;
        }

        result = AT_NO_CARRIER;

        if (AT_EVT_IS_VIDEO_CALL(callReleaseInd->releaseIndPara.callType)) {
            if (callReleaseInd->releaseIndPara.cause == TAF_CS_CAUSE_CC_NW_USER_ALERTING_NO_ANSWER) {
                result = AT_NO_ANSWER;
            }

            if (callReleaseInd->releaseIndPara.cause == TAF_CS_CAUSE_CC_NW_USER_BUSY) {
                result = AT_BUSY;
            }
        }

        /* AT命令触发的话，需要清除相应的状态变量 */
        if (AT_EVT_REL_IS_NEED_CLR_TIMER_STATUS_CMD(g_atClientTab[indexNum].cmdCurrentOpt)) {
            AT_StopTimerCmdReady(indexNum);
        }
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_VOID At_CsIndEvtReleaseProc(TAF_UINT8 indexNum, TAF_CCM_CallReleasedInd *callReleaseInd)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    VOS_UINT32     timerName;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    AT_UpdateCallErrInfo(indexNum, callReleaseInd->releaseIndPara.cause, &(callReleaseInd->releaseIndPara.errInfoText));

    if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
        timerName = ccCtx->s0TimeInfo.timerName;

        AT_StopRelTimer(timerName, &(ccCtx->s0TimeInfo.s0Timer));
        ccCtx->s0TimeInfo.timerStart = VOS_FALSE;
        ccCtx->s0TimeInfo.timerName  = 0;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* 停止RING TE */
    AT_VoiceStopRingTe(callReleaseInd->releaseIndPara.callId);
#endif

    AT_ReportCendResult(indexNum, callReleaseInd);
}

PS_BOOL_ENUM_UINT8 At_CheckReportConfCallType(MN_CALL_TypeUint8 callType)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
        case MN_CALL_TYPE_PSAP_ECALL:
            return PS_TRUE;
        default:
            return PS_FALSE;
    }
}

VOS_UINT32 AT_RcvTafCcmCallProcInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallProcInd *callProcInd = VOS_NULL_PTR;
    ModemIdUint16        modemId = MODEM_ID_0;
    VOS_UINT32           rslt;
    VOS_UINT32           checkRptCmdStatusResult;
    TAF_UINT16           length = 0;

    callProcInd = (TAF_CCM_CallProcInd *)msg;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_CsRspEvtCallProcProc: Get modem id fail.");
        return VOS_ERR;
    }

    /* CS可视电话里面，这里不能上报^CONF ，因此只有普通语音和紧急呼叫的情况下，才上报^CONF */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->confDelayReportFlag == VOS_TRUE) {
        /* 车载产品通过^CONF上报来打开HIFI，如果在PROC阶段上报，会有一段时间无声，所以这里不上报 */
        checkRptCmdStatusResult = VOS_FALSE;
    } else {
        checkRptCmdStatusResult = AT_CheckRptCmdStatus(callProcInd->procIndPata.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC,
                                                   AT_RPT_CMD_CONF);
    }

    if (((At_CheckReportConfCallType(callProcInd->procIndPata.callType) == PS_TRUE) ||
         (AT_EVT_IS_PS_VIDEO_CALL(callProcInd->procIndPata.callType, callProcInd->procIndPata.voiceDomain))) &&
        (checkRptCmdStatusResult == VOS_TRUE) && (!AT_IsBroadcastClientIndex(indexNum))) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "^CONF:%d", callProcInd->procIndPata.callId);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    AT_ReportCCallstateResult(modemId, callProcInd->procIndPata.callId, callProcInd->procIndPata.curcRptCfg,
                              AT_CS_CALL_STATE_CALL_PROC, callProcInd->procIndPata.voiceDomain);

    return VOS_OK;
}

/*
 * 功能描述: 上报ring
 */
LOCAL VOS_VOID AT_ReportCrcRingInfo(AT_CRC_TYPE crcType, TAF_CCM_CallIncomingInd *incomingInd, TAF_UINT16 *length)
{
     /* 命令与协议不符 */
    if (crcType == AT_CRC_ENABLE_TYPE) {
    /* +CRC -- +CRING: <type> */
#if (FEATURE_IMS == FEATURE_ON)
        if (incomingInd->incomingIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%sIRING%s", g_atCrLf, g_atCrLf);
        } else
#endif
        {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s+CRING: ", g_atCrLf);

            *length += (TAF_UINT16)At_CcClass2Print(incomingInd->incomingIndPara.callType,
                                                    g_atSndCodeAddress + *length);
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
        }
    } else {
#if (FEATURE_IMS == FEATURE_ON)
        if (incomingInd->incomingIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%sIRING%s", g_atCrLf, g_atCrLf);
        } else
#endif
        {
            /* +CRC -- RING */
            if (g_atVType == AT_V_ENTIRE_TYPE) {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%sRING%s", g_atCrLf, g_atCrLf);
            } else {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "2\r");
            }
        }
    }
}

AT_CliValidityUint8 AT_ConvertCLIValidity(TAF_CCM_CallIncomingInd *incomingInd)
{
    AT_CliValidityUint8 cliVality;
    /* 24008 10.5.4.30 */
    /*
     * Cause of No CLI information element provides the mobile station
     * the detailed reason why Calling party BCD nuber is not notified.
     */

    /* 号码长度不为0，则认为显示号码有效 */
    if (incomingInd->incomingIndPara.callNumber.numLen != 0) {
        return AT_CLI_VALIDITY_VALID;
    }

    switch (incomingInd->incomingIndPara.noCliCause) {
        case MN_CALL_NO_CLI_USR_REJ:
        case MN_CALL_NO_CLI_INTERACT:
        case MN_CALL_NO_CLI_PAYPHONE:
            cliVality = (AT_CliValidityUint8)incomingInd->incomingIndPara.noCliCause;
            break;

        /* 原因值不存在,号码合法或未提供 */
        case MN_CALL_NO_CLI_BUTT:
        case MN_CALL_NO_CLI_UNAVAL:
        default:
            cliVality = AT_CLI_VALIDITY_UNAVAL;
            break;
    }

    return cliVality;
}

/*
 * 功能描述: 上报clip相关信息
 */
LOCAL VOS_VOID AT_ReportClipInfo(AT_CLIP_TYPE clipType, TAF_CCM_CallIncomingInd *incomingInd, TAF_UINT16 *length)
{
    TAF_UINT8      asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];
    TAF_UINT8      cLIValid;

    if (clipType == AT_CLIP_ENABLE_TYPE) {
        /*
         * +CLIP: <number>,<type>
         * 其它部分[,<subaddr>,<satype>[,[<alpha>][,<CLI validity>]]]不用上报
         */
        AT_ERR_LOG("At_CsIncomingEvtOfIncomeStateIndProc:+CLIP:");
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s+CLIP: ", g_atCrLf);

        cLIValid = AT_ConvertCLIValidity(incomingInd);

        if (incomingInd->incomingIndPara.callNumber.numLen != 0) {
            AT_BcdNumberToAscii(incomingInd->incomingIndPara.callNumber.bcdNum,
                AT_MIN(incomingInd->incomingIndPara.callNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN),
                (VOS_CHAR *)asciiNum);

            /* 来电号码类型为国际号码，需要在号码前加+,否则会回拨失败 */
            if (((incomingInd->incomingIndPara.callNumber.numType >> 4) & 0x07) == MN_MSG_TON_INTERNATIONAL) {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
#if (FEATURE_MBB_CUST == FEATURE_ON)
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "\"+%s\",%d,,,,%d", asciiNum,
#else
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "\"+%s\",%d,\"\",,\"\",%d", asciiNum,
#endif
                    (incomingInd->incomingIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT), cLIValid);
            } else {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
#if (FEATURE_MBB_CUST == FEATURE_ON)
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "\"%s\",%d,,,,%d", asciiNum,
#else
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "\"%s\",%d,\"\",,\"\",%d", asciiNum,
#endif
                    (incomingInd->incomingIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT), cLIValid);
            }
        } else {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
#if (FEATURE_MBB_CUST == FEATURE_ON)
                (TAF_CHAR *)g_atSndCodeAddress + *length, "\"\",%d,,,,%d", AT_NUMBER_TYPE_EXT, cLIValid);
#else
                (TAF_CHAR *)g_atSndCodeAddress + *length, "\"\",,\"\",,\"\",%d", cLIValid);
#endif
        }
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
    }
}

TAF_VOID At_CsIncomingEvtOfIncomeStateIndProc(TAF_UINT8 indexNum, TAF_CCM_CallIncomingInd *incomingInd)
{
    TAF_UINT16     length = 0;
    TAF_UINT32     timerName;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);
    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    AT_ReportCrcRingInfo(ssCtx->crcType, incomingInd, &length);

    AT_ReportClipInfo(ssCtx->clipType, incomingInd, &length);

    if (ssCtx->salsType == AT_SALS_ENABLE_TYPE) {
        /* 上报是线路1还是线路2的来电 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s^ALS: ", g_atCrLf);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d", incomingInd->incomingIndPara.alsLineNo);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    /* 上报+CNAP命令 */
    AT_ReportCnapInfo(indexNum, &(incomingInd->incomingIndPara.nameIndicator));

    /* 只有呼叫类型为voice和PSAP ECALL时才支持自动接听功能，其他场景暂时不支持自动接听 */
    if (((incomingInd->incomingIndPara.callType == MN_CALL_TYPE_VOICE) ||
        (incomingInd->incomingIndPara.callType == MN_CALL_TYPE_PSAP_ECALL)) &&
        (ccCtx->s0TimeInfo.s0TimerLen != 0)) {
        /* 如果自动接听功能没启动，收到RING事件后启动 */
        if (ccCtx->s0TimeInfo.timerStart != TAF_TRUE) {
            timerName = AT_S0_TIMER;
            timerName |= AT_INTERNAL_PROCESS_TYPE;
            timerName |= (indexNum << 12);

            /* 乘1000是为了将秒转为毫秒 */
            AT_StartRelTimer(&(ccCtx->s0TimeInfo.s0Timer), (ccCtx->s0TimeInfo.s0TimerLen) * 1000, timerName,
                             incomingInd->incomingIndPara.callId, VOS_RELTIMER_NOLOOP);
            ccCtx->s0TimeInfo.timerStart = TAF_TRUE;
            ccCtx->s0TimeInfo.timerName  = timerName;
            AT_PR_LOGH("At_CsIncomingEvtOfIncomeStateIndProc: S0TimerLen = %d", ccCtx->s0TimeInfo.s0TimerLen);
        }
    }
}

LOCAL VOS_VOID At_PrintAsciiNumByIncomingOfWaitState(TAF_UINT8 indexNum, TAF_CCM_CallIncomingInd *incomingInd,
    TAF_UINT16 *length)
{
    TAF_UINT8 asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1] = {0};

    /*
     * 空口存到TAF的Cause of No CLI与AT输出的CLI Validity有差异
     * 27007 7.12
     * When CLI is not available, <number> shall be an empty string ("")
     * and <type> value will not be significant. TA may return the recommended
     * value 128 for <type>.
     */
    AT_BcdNumberToAscii(incomingInd->incomingIndPara.callNumber.bcdNum,
        AT_MIN(incomingInd->incomingIndPara.callNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN),
        (VOS_CHAR *)asciiNum);
    /* 如果是国际号码，在号码前带+号。通过右移4位，和0x07按位与得到enNumType的bit5-6-7，获取号码类型 */
    if ((AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->ccwaWithInternationNumType == VOS_TRUE) &&
        ((incomingInd->incomingIndPara.callNumber.numType >> 4) & 0x07) == MN_MSG_TON_INTERNATIONAL) {
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"+%s\",%d", asciiNum,
            (incomingInd->incomingIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT));
    } else {
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"%s\",%d", asciiNum,
            (incomingInd->incomingIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT));
    }

}

TAF_VOID At_CsIncomingEvtOfWaitStateIndProc(TAF_UINT8 indexNum, TAF_CCM_CallIncomingInd *incomingInd)
{
    TAF_UINT16 length = 0;
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    AT_CliValidityUint8 cliValidity = AT_ConvertCLIValidity(incomingInd);

    ssCtx  = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (ssCtx->ccwaType == AT_CCWA_ENABLE_TYPE) { /* 命令与协议不符 */
#if (FEATURE_IMS == FEATURE_ON)
        if ((incomingInd->incomingIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) &&
            (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->ccwaWithInternationNumType != VOS_TRUE)) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^CCWA: ", g_atCrLf);
        } else
#endif
        {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s+CCWA: ", g_atCrLf);
        }

        if (cliValidity == AT_CLI_VALIDITY_VALID) {
            At_PrintAsciiNumByIncomingOfWaitState(indexNum, incomingInd, &length);
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",%d", AT_NUMBER_TYPE_EXT);
        }

        if ((incomingInd->incomingIndPara.callType == MN_CALL_TYPE_VOICE) ||
            (incomingInd->incomingIndPara.callType == MN_CALL_TYPE_PSAP_ECALL)) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",1");
        } else if (incomingInd->incomingIndPara.callType == MN_CALL_TYPE_VIDEO) {
            if (incomingInd->incomingIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",2");
            } else {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",32");
            }
        } else if (incomingInd->incomingIndPara.callType == MN_CALL_TYPE_FAX) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",4");
        } else if (incomingInd->incomingIndPara.callType == MN_CALL_TYPE_CS_DATA) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",2");
        } else {
        }

        /* <alpha>,<CLI_validity> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",,%d", cliValidity);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        if (ssCtx->salsType == AT_SALS_ENABLE_TYPE) {
            /* 上报是线路1还是线路2的来电 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^ALS: ", g_atCrLf);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d", incomingInd->incomingIndPara.alsLineNo);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

PS_BOOL_ENUM_UINT8 At_CheckUartRingTeCallType(MN_CALL_TypeUint8 callType)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
        case MN_CALL_TYPE_PSAP_ECALL:
            return PS_TRUE;
        default:
            return PS_FALSE;
    }
}

VOS_UINT32 At_RcvTafCcmCallIncomingInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallIncomingInd *incomingInd = VOS_NULL_PTR;
    AT_ModemCcCtx           *ccCtx       = VOS_NULL_PTR;
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rslt;

    incomingInd = (TAF_CCM_CallIncomingInd *)msg;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_CsRspEvtConnectProc: Get modem id fail.");
        return VOS_ERR;
    }

    /*
     * 需要增加来电类型，传真，数据，可视电话，语音呼叫
     */
    /*
     * +CRC -- +CRING: <type> || RING
     * +CLIP: <number>,<type>[,<subaddr>,<satype>[,[<alpha>][,<CLI validity>]]]
     */
    if (incomingInd->incomingIndPara.callState == MN_CALL_S_INCOMING) {
        At_CsIncomingEvtOfIncomeStateIndProc(indexNum, incomingInd);
        AT_ReportCCallstateResult(modemId, incomingInd->incomingIndPara.callId, incomingInd->incomingIndPara.curcRptCfg,
                                  AT_CS_CALL_STATE_INCOMMING, incomingInd->incomingIndPara.voiceDomain);
    } else if (incomingInd->incomingIndPara.callState == MN_CALL_S_WAITING) {
        At_CsIncomingEvtOfWaitStateIndProc(indexNum, incomingInd);
        AT_ReportCCallstateResult(modemId, incomingInd->incomingIndPara.callId, incomingInd->incomingIndPara.curcRptCfg,
                                  AT_CS_CALL_STATE_WAITING, incomingInd->incomingIndPara.voiceDomain);
    } else {
        ccCtx->curIsExistCallFlag = VOS_TRUE;
        return VOS_OK;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (At_CheckUartRingTeCallType(incomingInd->incomingIndPara.callType) == PS_TRUE) {
        /* 通过UART端口的RING脚输出波形通知TE */
        AT_VoiceStartRingTe(incomingInd->incomingIndPara.callId);
    }
#endif
    ccCtx->curIsExistCallFlag = VOS_TRUE;

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallReleaseInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallReleasedInd *callReleaseInd = VOS_NULL_PTR;
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rslt;

    callReleaseInd = (TAF_CCM_CallReleasedInd *)msg;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_CsIndEvtReleaseProc(indexNum, callReleaseInd);
    } else {
        AT_LOG1("AT_RcvTafCcmCallReleaseInd ucIndex", indexNum);
        AT_LOG1("AT_RcvTafCcmCallReleaseInd[ucIndex].CmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

        AT_CsRspEvtReleasedProc(indexNum, callReleaseInd);
    }

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafCcmCallReleaseInd: Get modem id fail.");
        return VOS_ERR;
    }

    AT_ReportCCallstateResult(modemId, callReleaseInd->releaseIndPara.callId, callReleaseInd->releaseIndPara.curcRptCfg,
                              AT_CS_CALL_STATE_RELEASED, callReleaseInd->releaseIndPara.voiceDomain);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallAlertingInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallAlertingInd *callAlertingInd = VOS_NULL_PTR;
    VOS_UINT32               rslt;
    ModemIdUint16            modemId;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    VOS_UINT32               isAllowConfReport;
    VOS_UINT16               length = 0;
#endif

    callAlertingInd = (TAF_CCM_CallAlertingInd *)msg;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafCcmCallAlertingInd: Get modem id fail.");
        return VOS_ERR;
    }

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 车载产品定制在对端振铃时主动上报^CONF */
    isAllowConfReport = AT_CheckRptCmdStatus(callAlertingInd->alertingIndPara.curcRptCfg,
        AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONF);
    if ((isAllowConfReport == VOS_TRUE) &&
        (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->confDelayReportFlag == VOS_TRUE)) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CONF:%d%s", g_atCrLf,
            callAlertingInd->alertingIndPara.callId, g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
#endif

    AT_ReportCCallstateResult(modemId, callAlertingInd->alertingIndPara.callId,
                              callAlertingInd->alertingIndPara.curcRptCfg, AT_CS_CALL_STATE_ALERTING,
                              callAlertingInd->alertingIndPara.voiceDomain);

    return VOS_OK;
}

LOCAL VOS_VOID AT_PrintCallConnectIndMoCallColp(TAF_CCM_CallConnectInd *connectIndMsg, ModemIdUint16 modemId,
    VOS_UINT8 indexNum)
{
    AT_ModemSsCtx          *ssCtx = VOS_NULL_PTR;
    VOS_UINT8               bcdNumLen;
    TAF_UINT16              length        = 0;
    TAF_UINT8               asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1] = {0};

    ssCtx = AT_GetModemSsCtxAddrFromModemId(modemId);

    if (connectIndMsg->connectIndPara.callDir == MN_CALL_DIR_MO) {
        if (ssCtx->colpType == AT_COLP_ENABLE_TYPE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s+COLP: ", g_atCrLf);
            if (connectIndMsg->connectIndPara.connectNumber.numLen != 0) {
                bcdNumLen = TAF_MIN(connectIndMsg->connectIndPara.connectNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN);
                AT_BcdNumberToAscii(connectIndMsg->connectIndPara.connectNumber.bcdNum,
                                    bcdNumLen, (VOS_CHAR *)asciiNum);
                length +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",%d,\"\",,\"\"", asciiNum,
                        (connectIndMsg->connectIndPara.connectNumber.numType | AT_NUMBER_TYPE_EXT));
            } else {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",,\"\",,\"\"");
            }
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            At_SendResultData(indexNum, g_atSndCodeAddress, length);
        }
    }
}

VOS_UINT32 AT_RcvTafCcmCallConnectInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallConnectInd *connectIndMsg = VOS_NULL_PTR;
    TAF_UINT32              result        = AT_FAILURE;
    TAF_UINT16              length        = 0;
    ModemIdUint16           modemId;
    VOS_UINT32              rslt;

    modemId = MODEM_ID_0;

    connectIndMsg = (TAF_CCM_CallConnectInd *)msg;

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* 停止RING TE */
    AT_VoiceStopRingTe(connectIndMsg->connectIndPara.callId);
#endif

    /* CS呼叫成功, 清除CS域错误码和文本信息 */
    AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_SUCCESS, &(connectIndMsg->connectIndPara.errInfoText));

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_CsRspEvtConnectProc: Get modem id fail.");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        return AT_PrintCallConnectIndBroadcastConn(connectIndMsg, modemId, indexNum);
    }

    /* 需要判断来电类型，如VOICE或者DATA */
    AT_PrintCallConnectIndMoCallColp(connectIndMsg, modemId, indexNum);

    /* Video下，通过At_FormatResultData来上报CONNECT */
    if (AT_EVT_IS_VIDEO_CALL(connectIndMsg->connectIndPara.callType)) {
        /* IMS Video不给上层报CONNECT，上报^CONN */
        if (connectIndMsg->connectIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            if (AT_CheckRptCmdStatus(connectIndMsg->connectIndPara.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC,
                                     AT_RPT_CMD_CONN) == VOS_TRUE) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s^CONN:%d,%d%s", g_atCrLf,
                    connectIndMsg->connectIndPara.callId, connectIndMsg->connectIndPara.callType, g_atCrLf);

                At_SendResultData(indexNum, g_atSndCodeAddress, length);
            }

            AT_ReportCCallstateResult(modemId, connectIndMsg->connectIndPara.callId,
                                      connectIndMsg->connectIndPara.curcRptCfg, AT_CS_CALL_STATE_CONNECT,
                                      connectIndMsg->connectIndPara.voiceDomain);

            return VOS_OK;
        }

        g_atClientTab[indexNum].csRabId = connectIndMsg->connectIndPara.rabId;
        result                          = AT_CONNECT;
    } else {
        return AT_PrintCallConnectIndVoiceCallConn(connectIndMsg, modemId, indexNum);
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    AT_ReportCCallstateResult(modemId, connectIndMsg->connectIndPara.callId, connectIndMsg->connectIndPara.curcRptCfg,
                              AT_CS_CALL_STATE_CONNECT, connectIndMsg->connectIndPara.voiceDomain);

    return VOS_OK;
}

PS_BOOL_ENUM_UINT8 At_CheckReportOrigCallType(MN_CALL_TypeUint8 callType)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
            return PS_TRUE;

        default:
            return PS_FALSE;
    }
}

VOS_UINT32 AT_RcvTafCcmCallOrigInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_ModemCcCtx       *ccCtx   = VOS_NULL_PTR;
    TAF_CCM_CallOrigInd *origInd = VOS_NULL_PTR;
    ModemIdUint16        modemId = MODEM_ID_0;
    VOS_UINT32           rslt;
    TAF_UINT16           length = 0;
    VOS_UINT32           checkRptCmdStatusResult;
    MN_CALL_TypeUint8    newCallType;

    origInd  = (TAF_CCM_CallOrigInd *)msg;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_CsRspEvtOrigProc: Get modem id fail.");
        return VOS_ERR;
    }

    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    /* 可视电话里面，这里不能上报^ORIG ，因此只有普通语音和紧急呼叫的情况下，才上报^ORIG */
    checkRptCmdStatusResult = AT_CheckRptCmdStatus(origInd->origIndPara.curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC,
                                                   AT_RPT_CMD_ORIG);
    newCallType             = MN_CALL_TYPE_VOICE;
    At_ChangeEcallTypeToCallType(origInd->origIndPara.callType, &newCallType);

    if (((At_CheckReportOrigCallType(newCallType) == PS_TRUE) ||
         (AT_EVT_IS_PS_VIDEO_CALL(origInd->origIndPara.callType, origInd->origIndPara.voiceDomain))) &&
        (checkRptCmdStatusResult == VOS_TRUE) && (!AT_IsBroadcastClientIndex(indexNum))) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "^ORIG:%d", origInd->origIndPara.callId);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", newCallType);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    /* 发起呼叫后收到回复的OK后，将当前是否存在呼叫标志置为TRUE */
    ccCtx->curIsExistCallFlag = VOS_TRUE;

    AT_ReportCCallstateResult(modemId, origInd->origIndPara.callId, origInd->origIndPara.curcRptCfg,
                              AT_CS_CALL_STATE_ORIG, origInd->origIndPara.voiceDomain);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallHoldInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallHoldInd *callHoldInd = VOS_NULL_PTR;
    VOS_UINT32           loop;
    VOS_UINT32           rslt;
    ModemIdUint16        modemId;

    callHoldInd = (TAF_CCM_CallHoldInd *)msg;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafCcmCallHoldInd: Get modem id fail.");
        return VOS_ERR;
    }

    callHoldInd->holdIndPara.callNum = AT_MIN(callHoldInd->holdIndPara.callNum, MN_CALL_MAX_NUM);
    for (loop = 0; loop < (callHoldInd->holdIndPara.callNum); loop++) {
        AT_ReportCCallstateResult(modemId, callHoldInd->holdIndPara.callId[loop], callHoldInd->holdIndPara.curcRptCfg,
                                  AT_CS_CALL_STATE_HOLD, callHoldInd->holdIndPara.voiceDomain);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallAllReleasedInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_ModemCcCtx *ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);
    ccCtx->curIsExistCallFlag = VOS_FALSE;

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallRetrieveInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallRetrieveInd *callRetrieveInd = VOS_NULL_PTR;
    VOS_UINT32               loop;
    VOS_UINT32               rslt;
    ModemIdUint16            modemId;

    callRetrieveInd = (TAF_CCM_CallRetrieveInd *)msg;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafCcmCallHoldInd: Get modem id fail.");
        return VOS_ERR;
    }

    callRetrieveInd->retrieveInd.callNum = AT_MIN(callRetrieveInd->retrieveInd.callNum, MN_CALL_MAX_NUM);
    for (loop = 0; loop < (callRetrieveInd->retrieveInd.callNum); loop++) {
        AT_ReportCCallstateResult(modemId, callRetrieveInd->retrieveInd.callId[loop],
                                  callRetrieveInd->retrieveInd.curcRptCfg, AT_CS_CALL_STATE_RETRIEVE,
                                  callRetrieveInd->retrieveInd.voiceDomain);
    }

    return VOS_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEconfNotifyInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EconfNotifyInd *notifyInd = (TAF_CCM_EconfNotifyInd *)msg;
    AT_ModemCcCtx          *ccCtx = AT_GetModemCcCtxAddrFromClientId(notifyInd->ctrl.clientId);
    VOS_UINT16              length = 0;
    errno_t                 memResult;

    ccCtx->econfInfo.numOfCalls = AT_MIN(notifyInd->econfNotifyIndPara.numOfCalls, TAF_CALL_MAX_ECONF_CALLED_NUM);

    if (ccCtx->econfInfo.numOfCalls > 0) {
        memResult = memcpy_s(ccCtx->econfInfo.callInfo, sizeof(ccCtx->econfInfo.callInfo),
                             notifyInd->econfNotifyIndPara.callInfo,
                             (sizeof(TAF_CALL_EconfInfoParam) * ccCtx->econfInfo.numOfCalls));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ccCtx->econfInfo.callInfo),
                            (sizeof(TAF_CALL_EconfInfoParam) * ccCtx->econfInfo.numOfCalls));
    }

    /* call_num取pstNotifyInd->ucNumOfCalls，而不是pstCcCtx->stEconfInfo.ucNumOfCalls，可以方便发现错误 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d%s", g_atCrLf, g_atStringTab[AT_STRING_ECONFSTATE].text,
        notifyInd->econfNotifyIndPara.numOfCalls, g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaMtStateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_MtStatesInd *mtStatusInd = (IMSA_AT_MtStatesInd *)msg;
    VOS_CHAR             acString[AT_IMSA_CALL_ASCII_NUM_MAX_LENGTH + 1];
    errno_t              memResult;

    (VOS_VOID)memset_s(acString, sizeof(acString), 0x00, sizeof(acString));
    memResult = memcpy_s(acString, sizeof(acString), mtStatusInd->asciiCallNum, AT_IMSA_CALL_ASCII_NUM_MAX_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acString), AT_IMSA_CALL_ASCII_NUM_MAX_LENGTH);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^IMSMTRPT: %s,%d,%d%s", g_atCrLf, acString, mtStatusInd->mtStatus,
        mtStatusInd->causeCode, g_atCrLf);
    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaRttEventInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_RttEventInd *rttEvtInd = (IMSA_AT_RttEventInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRINT_RTTEVENT].text,
        rttEvtInd->rttEvent.callId, rttEvtInd->rttEvent.event, rttEvtInd->rttEvent.evtReason, g_atCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaRttErrorInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_RttErrorInd *rttErrInd = (IMSA_AT_RttErrorInd *)msg;
    VOS_CHAR             acRttErrTxt[IMSA_AT_RTT_REASON_TEXT_MAX_LEN + 1];
    errno_t              memResult;

    (VOS_VOID)memset_s(acRttErrTxt, sizeof(acRttErrTxt), 0x00, sizeof(acRttErrTxt));
    memResult = memcpy_s(acRttErrTxt, sizeof(acRttErrTxt), rttErrInd->rttError.reasonText,
                         IMSA_AT_RTT_REASON_TEXT_MAX_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acRttErrTxt), IMSA_AT_RTT_REASON_TEXT_MAX_LEN);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d,%d,%d,\"%s\"%s", g_atCrLf, g_atStringTab[AT_STRINT_RTTERROR].text,
        rttErrInd->rttError.callId, rttErrInd->rttError.operation, rttErrInd->rttError.causeCode, acRttErrTxt,
        g_atCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#endif

#if (FEATURE_ECALL == FEATURE_ON)
VOS_VOID AT_RcvTafEcallStatusErrorInd(VOS_VOID)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ECLSTAT: 2,3%s", g_atCrLf, g_atCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, g_atSndCodeAddress, length);
}

VOS_UINT32 At_ProcVcReportEcallStateEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    if ((vcEvtInfo->ecallState == APP_VC_ECALL_MSD_TRANSMITTING_FAIL) ||
        (vcEvtInfo->ecallState == APP_VC_ECALL_PSAP_MSD_REQUIRETRANSMITTING)) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECLSTAT: %d,%d%s", g_atCrLf, vcEvtInfo->ecallState,
            vcEvtInfo->ecallDescription, g_atCrLf);
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECLSTAT: %d%s", g_atCrLf, vcEvtInfo->ecallState, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

TAF_CALL_ChannelTypeUint8 AT_ConvertCodecTypeToChannelType(VOS_UINT8              isLocalAlertingFlag,
                                                           MN_CALL_CodecTypeUint8 codecType)
{
    TAF_CALL_ChannelTypeUint8 channelType = TAF_CALL_CHANNEL_TYPE_NONE;

    if (isLocalAlertingFlag == VOS_TRUE) {
        /* 本地振铃,无带内音信息 */
        channelType = TAF_CALL_CHANNEL_TYPE_NONE;
    } else {
        /* 网络振铃，需要设置channel type */
        /* 带内音可用，默认窄带语音 */
        channelType = TAF_CALL_CHANNEL_TYPE_NARROW;

        if ((codecType == MN_CALL_CODEC_TYPE_AMRWB) || (codecType == MN_CALL_CODEC_TYPE_EVS)) {
            /* 带内音可用，宽带语音 */
            channelType = TAF_CALL_CHANNEL_TYPE_WIDE;
        }

        if (codecType == MN_CALL_CODEC_TYPE_EVS_NB) {
            channelType = TAF_CALL_CHANNEL_TYPE_EVS_NB;
        }

        if (codecType == MN_CALL_CODEC_TYPE_EVS_WB) {
            channelType = TAF_CALL_CHANNEL_TYPE_EVS_WB;
        }

        if (codecType == MN_CALL_CODEC_TYPE_EVS_SWB) {
            channelType = TAF_CALL_CHANNEL_TYPE_EVS_SWB;
        }
    }

    return channelType;
}

VOS_UINT32 AT_RcvTafCcmChannelInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallChannelInfoInd *channelInfoInd = VOS_NULL_PTR;
    VOS_UINT16                  length = 0;
    TAF_CALL_ChannelTypeUint8   channelType;

    channelInfoInd = (TAF_CCM_CallChannelInfoInd *)msg;

    if ((channelInfoInd->codecType == MN_CALL_CODEC_TYPE_BUTT) && (channelInfoInd->isLocalAlertingFlag == VOS_FALSE)) {
        AT_WARN_LOG("AT_RcvTafCcmChannelInfoInd: WARNING: CodecType BUTT!");
        return VOS_ERR;
    }

    channelType = AT_ConvertCodecTypeToChannelType(channelInfoInd->isLocalAlertingFlag, channelInfoInd->codecType);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d,%d%s", g_atCrLf,
        g_atStringTab[AT_STRING_CS_CHANNEL_INFO].text, channelType, channelInfoInd->voiceDomain, g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaEmcCallBackNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_EmcCallBackNtf *emcCallBack = (TAF_MMA_EmcCallBackNtf *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^EMCCBM: %d%s", g_atCrLf, emcCallBack->isInCallBack, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_VOID At_RcvVcMsgDtmfDecoderIndProc(MN_AT_IndEvt *data)
{
    APP_VC_DtmfDecoderInd *dtmfInd  = VOS_NULL_PTR;
    VOS_UINT8              indexNum = 0;
    VOS_CHAR               output[AT_OUTPUT_MAX_LENGTH];

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(data->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_RcvVcMsgDtmfDecoderIndProc:WARNING:AT INDEX NOT FOUND!");
        return;
    }

    /* 初始化 */
    dtmfInd   = (APP_VC_DtmfDecoderInd *)data->content;
    output[0] = dtmfInd->ucDtmfCode;
    output[1] = '\0';

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^DDTMF: %s%s", g_atCrLf, output, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
}

