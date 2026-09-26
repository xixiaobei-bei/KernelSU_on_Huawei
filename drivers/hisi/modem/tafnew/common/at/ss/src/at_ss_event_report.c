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
#include "at_ss_event_report.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "taf_type_def.h"
#include "at_data_proc.h"
#include "at_device_cmd.h"
#include "at_ss_comm.h"
#include "at_msg_print.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_EVENT_REPORT_C

#define AT_CF_CAUSE_ALWAYS_RTN_NUM 0
#define AT_CF_CAUSE_POWER_OFF_RTN_NUM 2
#define AT_CF_CAUSE_NO_ANSWER_RTN_NUM 3
#define AT_CF_CAUSE_SHADOW_ZONE_RTN_NUM 4
#define AT_CF_CAUSE_DEFLECTION_480_RTN_NUM 5
#define AT_CF_CAUSE_DEFLECTION_487_RTN_NUM 6

#define AT_SS_NTFY_ON_HOLD_RTN_NUM 2
#define AT_SS_NTFY_RETRIEVED_RTN_NUM 3
#define AT_SS_NTFY_ENTER_MPTY_RTN_NUM 4
#define AT_SS_NTFY_HOLD_RELEASED_RTN_NUM 5
#define AT_SS_NTFY_EXPLICIT_CALL_S_ALERTING_RTN_NUM 7
#define AT_SS_NTFY_EXPLICIT_CALL_TRANSFER_RTN_NUM 8
#define AT_SS_NTFY_DEFLECTED_CALL_RTN_NUM 9
#define AT_SS_NTFY_INCALL_FORWARDED_RTN_NUM 10

static const AT_CALL_CuusuMsg g_cuusuMsgType[] = {
    { MN_CALL_UUS1_MSG_SETUP, AT_CUUSU_MSG_SETUP },
    { MN_CALL_UUS1_MSG_DISCONNECT, AT_CUUSU_MSG_DISCONNECT },
    { MN_CALL_UUS1_MSG_RELEASE_COMPLETE, AT_CUUSU_MSG_RELEASE_COMPLETE }
};

static const AT_CALL_CuusiMsg g_cuusiMsgType[] = {
    { MN_CALL_UUS1_MSG_ALERT, AT_CUUSI_MSG_ALERT },
    { MN_CALL_UUS1_MSG_PROGRESS, AT_CUUSI_MSG_PROGRESS },
    { MN_CALL_UUS1_MSG_CONNECT, AT_CUUSI_MSG_CONNECT },
    { MN_CALL_UUS1_MSG_RELEASE, AT_CUUSI_MSG_RELEASE }
};

VOS_UINT8 At_GetCssiForwardCauseCode(MN_CALL_CfCauseUint8 code)
{
    switch (code) {
        case MN_CALL_CF_CAUSE_ALWAYS:
            return AT_CF_CAUSE_ALWAYS_RTN_NUM;

        case MN_CALL_CF_CAUSE_BUSY:
            return 1;

        case MN_CALL_CF_CAUSE_POWER_OFF:
            return AT_CF_CAUSE_POWER_OFF_RTN_NUM;

        case MN_CALL_CF_CAUSE_NO_ANSWER:
            return AT_CF_CAUSE_NO_ANSWER_RTN_NUM;

        case MN_CALL_CF_CAUSE_SHADOW_ZONE:
            return AT_CF_CAUSE_SHADOW_ZONE_RTN_NUM;

        case MN_CALL_CF_CAUSE_DEFLECTION_480:
            return AT_CF_CAUSE_DEFLECTION_480_RTN_NUM;

        case MN_CALL_CF_CAUSE_DEFLECTION_487:
            return AT_CF_CAUSE_DEFLECTION_487_RTN_NUM;

        default:
            AT_ERR_LOG1("At_GetCssiFormardCauseCode: enCode is fail, enCode is ", code);
            return 0xFF;
    }
}

VOS_UINT8 At_GetSsCode(MN_CALL_SsNotifyCodeUint8 code, MN_CALL_StateUint8 callState)
{
    switch (code) {
        case MN_CALL_SS_NTFY_FORWORDED_CALL:
            return 0;

        case MN_CALL_SS_NTFY_MT_CUG_INFO:
            return 1;

        case MN_CALL_SS_NTFY_ON_HOLD:
            return AT_SS_NTFY_ON_HOLD_RTN_NUM;

        case MN_CALL_SS_NTFY_RETRIEVED:
            return AT_SS_NTFY_RETRIEVED_RTN_NUM;

        case MN_CALL_SS_NTFY_ENTER_MPTY:
            return AT_SS_NTFY_ENTER_MPTY_RTN_NUM;

        case MN_CALL_SS_NTFY_HOLD_RELEASED:
            return AT_SS_NTFY_HOLD_RELEASED_RTN_NUM;

        case MN_CALL_SS_NTFY_DEFLECTED_CALL:
            return AT_SS_NTFY_DEFLECTED_CALL_RTN_NUM;

        case MN_CALL_SS_NTFY_INCALL_FORWARDED:
            return AT_SS_NTFY_INCALL_FORWARDED_RTN_NUM;

        case MN_CALL_SS_NTFY_EXPLICIT_CALL_TRANSFER:
            if (callState == MN_CALL_S_ALERTING) {
                return AT_SS_NTFY_EXPLICIT_CALL_S_ALERTING_RTN_NUM;
            }
            return AT_SS_NTFY_EXPLICIT_CALL_TRANSFER_RTN_NUM;

        case MN_CALL_SS_NTFY_CCBS_BE_RECALLED:
            return 0x16;

        default:
            return 0xFF;
    }
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID At_ProcCsEvtCssuexNotifiy_Ims(TAF_CCM_CallSsInd *callSsInd, VOS_UINT8 code, VOS_UINT16 *length)
{
    VOS_CHAR asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(asciiNum, (VOS_UINT32)sizeof(asciiNum), 0, (VOS_UINT32)sizeof(asciiNum));

    /* ^CSSUEX: <code2>,[<index>],<callId>[,<number>,<type>[,<forward_cause>]] */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s^CSSUEX: ", g_atCrLf);

    /* <code2>, */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", code);

    /* [index], */
    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MT_CUG_INFO) {
        /* <index> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", callSsInd->ssIndPara.ssNotify.cugIndex);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }

    /* <callId> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", callSsInd->ssIndPara.callId);

    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MT_CUG_INFO) {
        if (callSsInd->ssIndPara.callNumber.numLen != 0) {
            AT_BcdNumberToAscii(callSsInd->ssIndPara.callNumber.bcdNum,
                AT_MIN(callSsInd->ssIndPara.callNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN), asciiNum);

            /* ,<number> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", asciiNum);

            /* ,<type> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                (callSsInd->ssIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT));
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID At_ProcCsEvtCssuNotifiy_Ims(TAF_CCM_CallSsInd *callSsInd, VOS_UINT8 code, VOS_UINT16 *length)
{
    VOS_CHAR asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(asciiNum, (VOS_UINT32)sizeof(asciiNum), 0, (VOS_UINT32)sizeof(asciiNum));

    /* ^CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s^CSSU: ", g_atCrLf);

    /* <code2> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", code);

    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MT_CUG_INFO) {
        /* ,<index> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ssNotify.cugIndex);

        if (callSsInd->ssIndPara.callNumber.numLen != 0) {
            AT_BcdNumberToAscii(callSsInd->ssIndPara.callNumber.bcdNum,
                AT_MIN(callSsInd->ssIndPara.callNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN), asciiNum);

            /* ,<number> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", asciiNum);

            /* ,<type> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                (callSsInd->ssIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT));
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);

    At_ProcCsEvtCssuexNotifiy_Ims(callSsInd, code, length);
}

VOS_VOID At_ProcCsEvtCssiNotifiy_Ims(TAF_CCM_CallSsInd *callSsInd, VOS_UINT16 *length)
{
    VOS_CHAR  asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];
    VOS_UINT8 forwardCauseCode;

    memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));
    forwardCauseCode = 0xFF;

    /* ^CSSI: <code1>,<index>,<callId>[,<number>,<type>[,<forward_cause>]] */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s^CSSI: ", g_atCrLf);

    /* <code1>, */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", callSsInd->ssIndPara.ssNotify.code);

    /* [index], */
    if ((callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MO_CUG_INFO) ||
        (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_BE_FORWORDED)) {
        /* ,<index> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", callSsInd->ssIndPara.ssNotify.cugIndex);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }

    /* <callId> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", callSsInd->ssIndPara.callId);

    if ((callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MO_CUG_INFO) ||
        (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_BE_FORWORDED)) {
        /* 被连号码显示 */
        if (callSsInd->ssIndPara.connectNumber.numLen != 0) {
            (VOS_VOID)AT_BcdNumberToAscii(callSsInd->ssIndPara.connectNumber.bcdNum,
                AT_MIN(callSsInd->ssIndPara.connectNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN), asciiNum);

            /* ,<number> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", asciiNum);

            /* ,<type> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                (callSsInd->ssIndPara.connectNumber.numType | AT_NUMBER_TYPE_EXT));

            /* [,<forward_cause>] */
            forwardCauseCode = At_GetCssiForwardCauseCode(callSsInd->ssIndPara.callForwardCause);

            if (forwardCauseCode != 0xFF) {
                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", forwardCauseCode);
            }
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID At_ProcCsEvtImsHoldToneNotifiy_Ims(TAF_CCM_CallSsInd *callSsInd, VOS_UINT16 *length)
{
    /* ^IMSHOLDTONE: <hold_tone> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s: ", g_atCrLf, g_atStringTab[AT_STRING_IMS_HOLD_TONE].text);

    /* <hold_tone> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", callSsInd->ssIndPara.holdToneType);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_UINT32 AT_RcvTafCcmCallModifyStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallModifyStatusInd *statusInd = (TAF_CCM_CallModifyStatusInd *)msg;
    VOS_UINT16                   length = 0;

    if (statusInd->modifyStatus == MN_CALL_MODIFY_REMOTE_USER_REQUIRE_TO_MODIFY) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CALL_MODIFY_IND].text, statusInd->callId, statusInd->currCallType,
            statusInd->voiceDomain, statusInd->expectCallType, statusInd->voiceDomain, statusInd->modifyReason,
            g_atCrLf);
    } else if (statusInd->modifyStatus == MN_CALL_MODIFY_PROC_BEGIN) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CALL_MODIFY_BEG].text, statusInd->callId, statusInd->voiceDomain, g_atCrLf);
    } else if (statusInd->modifyStatus == MN_CALL_MODIFY_PROC_END) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CALL_MODIFY_END].text, statusInd->callId, statusInd->voiceDomain, statusInd->cause,
            g_atCrLf);
    } else {
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif

VOS_VOID At_ProcCsEvtCssuNotifiy_NonIms(TAF_CCM_CallSsInd *callSsInd, VOS_UINT8 code, VOS_UINT16 *length)
{
    VOS_CHAR asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(asciiNum, (VOS_UINT32)sizeof(asciiNum), 0, (VOS_UINT32)sizeof(asciiNum));

    /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s+CSSU: ", g_atCrLf);

    /* <code2> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", code);

    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MT_CUG_INFO) {
        /* ,<index> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ssNotify.cugIndex);

        if (callSsInd->ssIndPara.callNumber.numLen != 0) {
            AT_BcdNumberToAscii(callSsInd->ssIndPara.callNumber.bcdNum,
                AT_MIN(callSsInd->ssIndPara.callNumber.numLen, MN_CALL_MAX_BCD_NUM_LEN), asciiNum);

            /* ,<number> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", asciiNum);

            /* ,<type> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                (callSsInd->ssIndPara.callNumber.numType | AT_NUMBER_TYPE_EXT));
        }
    }

    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_EXPLICIT_CALL_TRANSFER) {
        /* ,<index> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ssNotify.cugIndex);

        if (callSsInd->ssIndPara.ssNotify.ectIndicator.rdn.presentationAllowedAddr.numLen != 0) {
            AT_BcdNumberToAscii(callSsInd->ssIndPara.ssNotify.ectIndicator.rdn.presentationAllowedAddr.bcdNum,
                AT_MIN(callSsInd->ssIndPara.ssNotify.ectIndicator.rdn.presentationAllowedAddr.numLen,
                MN_CALL_MAX_BCD_NUM_LEN), asciiNum);

            /* ,<number> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", asciiNum);

            /* ,<type> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                (callSsInd->ssIndPara.ssNotify.ectIndicator.rdn.presentationAllowedAddr.numType | AT_NUMBER_TYPE_EXT));
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID At_ProcCsEvtCssiNotifiy_NonIms(TAF_CCM_CallSsInd *callSsInd, VOS_UINT16 *length)
{
    /* +CSSI: <code1>[,<index>] */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s+CSSI: ", g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", callSsInd->ssIndPara.ssNotify.code);

    if (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_MO_CUG_INFO) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ssNotify.cugIndex);
    }

    if ((callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_CCBS_RECALL) &&
        (callSsInd->ssIndPara.ccbsFeature.opCcbsIndex == MN_CALL_OPTION_EXIST)) {
        *length += (VOS_UINT16)At_CcClass2Print(callSsInd->ssIndPara.callType, g_atSndCodeAddress + *length);

        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);

        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ccbsFeature.ccbsIndex);

        if (callSsInd->ssIndPara.ccbsFeature.opBSubscriberNum == MN_CALL_OPTION_EXIST) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", callSsInd->ssIndPara.ccbsFeature.bsubscriberNum);
        }

        if (callSsInd->ssIndPara.ccbsFeature.opNumType == MN_CALL_OPTION_EXIST) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", callSsInd->ssIndPara.ccbsFeature.numType);
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID At_ProcCsRspEvtCssuNotifiy(VOS_UINT8 indexNum, TAF_CCM_CallSsInd *callSsInd, VOS_UINT16 *length)
{
    VOS_UINT8  code;
    VOS_UINT16 lengthTemp;
    VOS_UINT32 cssuRptStatus;

    lengthTemp = *length;

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        return;
    }

    cssuRptStatus = AT_CheckRptCmdStatus(callSsInd->ssIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                         AT_RPT_CMD_CSSU);

    /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    if (((cssuRptStatus == VOS_TRUE) && (callSsInd->ssIndPara.ssNotify.code > MN_CALL_SS_NTFY_BE_DEFLECTED) &&
         (callSsInd->ssIndPara.ssNotify.code != MN_CALL_SS_NTFY_CCBS_RECALL)) &&
        ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_D_CS_VOICE_CALL_SET) &&
         (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_D_CS_DATA_CALL_SET) &&
         (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_APDS_SET))) {
        code = At_GetSsCode(callSsInd->ssIndPara.ssNotify.code, callSsInd->ssIndPara.callState);

        if (code == 0xFF) {
            AT_ERR_LOG("At_ProcCsRspEvtCssuNotifiy: code error.");
            return;
        }

#if (FEATURE_IMS == FEATURE_ON)
        if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            At_ProcCsEvtCssuNotifiy_Ims(callSsInd, code, &lengthTemp);
        } else
#endif
        {
            At_ProcCsEvtCssuNotifiy_NonIms(callSsInd, code, &lengthTemp);
        }
    }

    *length = lengthTemp;
}

VOS_VOID At_ProcCsRspEvtCssiNotifiy(VOS_UINT8 indexNum, TAF_CCM_CallSsInd *callSsInd, VOS_UINT16 *length)
{
    VOS_UINT16 lengthTemp;
    VOS_UINT32 cssiRptStatus;

    lengthTemp = *length;

    cssiRptStatus = AT_CheckRptCmdStatus(callSsInd->ssIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                         AT_RPT_CMD_CSSI);

    if ((cssiRptStatus == VOS_TRUE) && ((callSsInd->ssIndPara.ssNotify.code <= MN_CALL_SS_NTFY_BE_DEFLECTED) ||
                                        (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_CCBS_RECALL))) {
#if (FEATURE_IMS == FEATURE_ON)
        /* ^CSSI: <code1>[,<index>[,<number>,<type>]] */
        if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            At_ProcCsEvtCssiNotifiy_Ims(callSsInd, &lengthTemp);
        } else
#endif
        /* +CSSI: <code1>[,<index>] */
        {
            At_ProcCsEvtCssiNotifiy_NonIms(callSsInd, &lengthTemp);
        }
    }

    *length = lengthTemp;
}

TAF_VOID AT_CsSsNotifyEvtIndProc(TAF_UINT8 indexNum, TAF_CCM_CallSsInd *callSsInd)
{
    VOS_UINT8  code;
    VOS_UINT16 length = 0;
    VOS_UINT32 cssiRptStatus;
    VOS_UINT32 cssuRptStatus;

    cssiRptStatus = AT_CheckRptCmdStatus(callSsInd->ssIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                         AT_RPT_CMD_CSSI);

    if ((cssiRptStatus == VOS_TRUE) && ((callSsInd->ssIndPara.ssNotify.code <= MN_CALL_SS_NTFY_BE_DEFLECTED) ||
                                        (callSsInd->ssIndPara.ssNotify.code == MN_CALL_SS_NTFY_CCBS_RECALL))) {
#if (FEATURE_IMS == FEATURE_ON)
        /* ^CSSI: <code1>[,<index>[,<number>,<type>]] */
        if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            At_ProcCsEvtCssiNotifiy_Ims(callSsInd, &length);
        } else
#endif
        /* +CSSI: <code1>[,<index>] */
        {
            At_ProcCsEvtCssiNotifiy_NonIms(callSsInd, &length);
        }
    }

    cssuRptStatus = AT_CheckRptCmdStatus(callSsInd->ssIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                         AT_RPT_CMD_CSSU);

    if ((cssuRptStatus == VOS_TRUE) && (callSsInd->ssIndPara.ssNotify.code > MN_CALL_SS_NTFY_BE_DEFLECTED) &&
        (callSsInd->ssIndPara.ssNotify.code != MN_CALL_SS_NTFY_CCBS_RECALL)) {
        code = At_GetSsCode(callSsInd->ssIndPara.ssNotify.code, callSsInd->ssIndPara.callState);

        if (code == 0xFF) {
            AT_ERR_LOG("AT_CsSsNotifyEvtIndProc: cssu code error.");
            return;
        }

#if (FEATURE_IMS == FEATURE_ON)
        if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
            /* ^CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
            At_ProcCsEvtCssuNotifiy_Ims(callSsInd, code, &length);
        } else
#endif
        {
            /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
            At_ProcCsEvtCssuNotifiy_NonIms(callSsInd, code, &length);
        }
    }

#if (FEATURE_IMS == FEATURE_ON)
    if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
        At_ProcCsEvtImsHoldToneNotifiy_Ims(callSsInd, &length);
    }
#endif

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID AT_ProcCsRspEvtSsNotify(VOS_UINT8 indexNum, TAF_CCM_CallSsInd *callSsInd)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId;
    VOS_UINT32    rslt;

    At_ProcCsRspEvtCssiNotifiy(indexNum, callSsInd, &length);

    At_ProcCsRspEvtCssuNotifiy(indexNum, callSsInd, &length);

#if (FEATURE_IMS == FEATURE_ON)
    if (callSsInd->ssIndPara.voiceDomain == TAF_CALL_VOICE_DOMAIN_IMS) {
        At_ProcCsEvtImsHoldToneNotifiy_Ims(callSsInd, &length);
    }
#endif

    if ((indexNum != AT_BROADCAST_CLIENT_INDEX_MODEM_0) && (indexNum != AT_BROADCAST_CLIENT_INDEX_MODEM_1) &&
        (indexNum != AT_BROADCAST_CLIENT_INDEX_MODEM_2)) {
        modemId = MODEM_ID_0;

        /* 获取client id对应的Modem Id */
        rslt = AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId);

        if (rslt != VOS_OK) {
            AT_WARN_LOG("AT_ProcCsRspEvtSsNotify: WARNING:MODEM ID NOT FOUND!");
            return;
        }

        /* CCALLSTATE需要广播上报，根据MODEM ID设置广播上报的Index */
        if (modemId == MODEM_ID_0) {
            indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
        } else if (modemId == MODEM_ID_1) {
            indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_1;
        } else {
            indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_2;
        }
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvTafCcmCallSsInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallSsInd *callSsInd = VOS_NULL_PTR;

    callSsInd = (TAF_CCM_CallSsInd *)msg;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_CsSsNotifyEvtIndProc(indexNum, callSsInd);
    } else {
        AT_LOG1("AT_RcvTafCcmCallSsInd ucIndex", indexNum);
        AT_LOG1("AT_RcvTafCcmCallSsInd[ucIndex].CmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

        AT_ProcCsRspEvtSsNotify(indexNum, callSsInd);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCnapInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CnapInfoInd *cnapInfoInd = VOS_NULL_PTR;

    cnapInfoInd = (TAF_CCM_CnapInfoInd *)msg;

    AT_ReportCnapInfo(indexNum, &cnapInfoInd->nameIndicator);

    return VOS_OK;
}

VOS_UINT32 AT_ConCallMsgTypeToCuusiMsgType(MN_CALL_Uus1MsgTypeUint32 msgType, AT_CuusiMsgTypeUint32 *cuusiMsgType

)
{
    VOS_UINT32 i;

    for (i = 0; i < sizeof(g_cuusiMsgType) / sizeof(AT_CALL_CuusiMsg); i++) {
        if (msgType == g_cuusiMsgType[i].callMsgType) {
            *cuusiMsgType = g_cuusiMsgType[i].cuusiMsgType;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_UINT32 AT_ConCallMsgTypeToCuusuMsgType(MN_CALL_Uus1MsgTypeUint32 msgType, AT_CuusuMsgTypeUint32 *cuusuMsgType

)
{
    VOS_UINT32 i;

    for (i = 0; i < sizeof(g_cuusuMsgType) / sizeof(AT_CALL_CuusuMsg); i++) {
        if (msgType == g_cuusuMsgType[i].callMsgType) {
            *cuusuMsgType = g_cuusuMsgType[i].cuusuMsgType;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_UINT32 AT_RcvTafCcmUus1InfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_Uus1InfoInd  *uus1InfoInd = VOS_NULL_PTR;
    VOS_UINT32            msgType;
    AT_CuusiMsgTypeUint32 cuusiMsgType;
    AT_CuusuMsgTypeUint32 cuusuMsgType;
    VOS_UINT32            ret;
    MN_CALL_DirUint8      callDir;
    VOS_UINT16            length;

    uus1InfoInd = (TAF_CCM_Uus1InfoInd *)msg;

    callDir = uus1InfoInd->uus1InfoIndPara.callDir;
    msgType = AT_CUUSI_MSG_ANY;

    ret = AT_ConCallMsgTypeToCuusiMsgType(uus1InfoInd->uus1InfoIndPara.uusInfo.msgType, &cuusiMsgType);
    if (ret == VOS_OK) {
        msgType = cuusiMsgType;
        callDir = MN_CALL_DIR_MO;
    } else {
        ret = AT_ConCallMsgTypeToCuusuMsgType(uus1InfoInd->uus1InfoIndPara.uusInfo.msgType, &cuusuMsgType);
        if (ret == VOS_OK) {
            msgType = cuusuMsgType;
            callDir = MN_CALL_DIR_MT;
        }
    }

    if (ret != VOS_OK) {
        if (uus1InfoInd->uus1InfoIndPara.callDir == MN_CALL_DIR_MO) {
            msgType = AT_CUUSI_MSG_ANY;
        } else {
            msgType = AT_CUUSU_MSG_ANY;
        }
    }

    length = 0;

    if (callDir == MN_CALL_DIR_MO) {
        /* 未激活则不进行任何处理,不能上报 */
        if (AT_CheckRptCmdStatus(uus1InfoInd->uus1InfoIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                 AT_RPT_CMD_CUUS1I) == VOS_FALSE) {
            return VOS_ERR;
        }
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s+CUUS1I:", g_atCrLf);
    } else {
        /* 未激活则不进行任何处理,不能上报 */
        if (AT_CheckRptCmdStatus(uus1InfoInd->uus1InfoIndPara.unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                 AT_RPT_CMD_CUUS1U) == VOS_FALSE) {
            return VOS_ERR;
        }
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s+CUUS1U:", g_atCrLf);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,", msgType);

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                  uus1InfoInd->uus1InfoIndPara.uusInfo.uuie,
                                                  (uus1InfoInd->uus1InfoIndPara.uusInfo.uuie[MN_CALL_LEN_POS] +
                                                   MN_CALL_UUIE_HEADER_LEN));

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_VOID AT_RcvSsaLcsMolrNtf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_LcsMolrNtf *lcsMolrNtf    = VOS_NULL_PTR;
    AT_ModemAgpsCtx    *agpsCtx       = VOS_NULL_PTR;
    VOS_CHAR           *pcLocationStr = VOS_NULL_PTR;
    VOS_UINT32          result;
    VOS_UINT32          returnCodeIndex;
    VOS_UINT16          length;
    errno_t             stringRet;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaLcsMolrNtf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    lcsMolrNtf      = (TAF_SSA_LcsMolrNtf *)event;
    agpsCtx         = AT_GetModemAgpsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);
    result          = AT_ConvertTafSsaErrorCode(indexNum, lcsMolrNtf->result);
    returnCodeIndex = 0;
    length          = 0;

    /* 判断查询操作是否成功 */
    if (result == AT_OK) {
        pcLocationStr = (VOS_CHAR *)PS_MEM_ALLOC(WUEPS_PID_AT, lcsMolrNtf->locationStrLen + 1);
        if (pcLocationStr == VOS_NULL_PTR) {
            AT_ERR_LOG("AT_RcvSsaLcsMolrNtf: Alloc Memory Failed!");
            return;
        }
        (VOS_VOID)memset_s(pcLocationStr, lcsMolrNtf->locationStrLen + 1, 0x00, lcsMolrNtf->locationStrLen + 1);

        if (lcsMolrNtf->locationStrLen > 0) {
            stringRet = strncpy_s(pcLocationStr, lcsMolrNtf->locationStrLen + 1, lcsMolrNtf->locationStr,
                                  lcsMolrNtf->locationStrLen);
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, lcsMolrNtf->locationStrLen + 1, lcsMolrNtf->locationStrLen);
        }
        pcLocationStr[lcsMolrNtf->locationStrLen] = '\0';

        if (lcsMolrNtf->rptTypeChoice == TAF_SSA_LCS_MOLR_RPT_NMEA) {
            /* 输出+CMOLRN定位信息 */
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s%s\"%s\"%s", g_atCrLf, g_atStringTab[AT_STRING_CMOLRN].text,
                pcLocationStr, g_atCrLf);
        } else {
            /* 输出+CMOLRG定位信息 */
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_CMOLRG].text,
                pcLocationStr, g_atCrLf);
        }

        PS_MEM_FREE(WUEPS_PID_AT, pcLocationStr);
    } else {
        if (AT_GetReturnCodeId(result, &returnCodeIndex) != VOS_OK) {
            AT_ERR_LOG("AT_RcvSsaLcsMolrNtf: result code indexNum is err!");
            return;
        }

        /* 输出+CMOLRE错误码 */
        if (agpsCtx->cmolreType == AT_CMOLRE_NUMERIC) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_CMOLRE].text,
                (VOS_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0], g_atCrLf);
        } else {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_CMOLRE].text,
                (VOS_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[1], g_atCrLf);
        }
    }

    /* 输出结果到AT通道 */
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID AT_RcvSsaLcsMtlrNtf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_LcsMtlrNtf *lcsMtlrNtf = VOS_NULL_PTR;
    VOS_UINT32          tmpStrLen;
    VOS_UINT32          i;
    VOS_UINT16          length;

    /* 只能为广播通道 */
    if (!AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaLcsMtlrNtf: WARNING:Not AT_BROADCAST_INDEX!");
        return;
    }

    lcsMtlrNtf = (TAF_SSA_LcsMtlrNtf *)event;
    length     = 0;

    /*
     * +CMTLR: <handle-id>,<notification-type>,<location-type>,
     * [<client-external-id>],[<client-name>][,<plane>]
     */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%d,", g_atCrLf, g_atStringTab[AT_STRING_CMTLR].text,
        lcsMtlrNtf->mtlrPara.handleId, lcsMtlrNtf->mtlrPara.ntfType, lcsMtlrNtf->mtlrPara.locationType);

    if (lcsMtlrNtf->mtlrPara.opClientExId == VOS_TRUE) {
        tmpStrLen = lcsMtlrNtf->mtlrPara.clientExId.length <= LCS_CLIENT_EXID_MAX_LEN ?
                        lcsMtlrNtf->mtlrPara.clientExId.length :
                        LCS_CLIENT_EXID_MAX_LEN;

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");

        for (i = 0; i < tmpStrLen; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", lcsMtlrNtf->mtlrPara.clientExId.value[i]);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",");

    if (lcsMtlrNtf->mtlrPara.opClientName == VOS_TRUE) {
        tmpStrLen = lcsMtlrNtf->mtlrPara.clientName.length <= LCS_CLIENT_NAME_MAX_LEN ?
                        lcsMtlrNtf->mtlrPara.clientName.length :
                        LCS_CLIENT_NAME_MAX_LEN;

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");

        for (i = 0; i < tmpStrLen; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", lcsMtlrNtf->mtlrPara.clientName.value[i]);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");
    }

    if (lcsMtlrNtf->mtlrPara.opPlane == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", lcsMtlrNtf->mtlrPara.plane);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\r\n");

    /* 输出结果到AT通道 */
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

