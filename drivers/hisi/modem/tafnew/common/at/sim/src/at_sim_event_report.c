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

#include "at_sim_event_report.h"
#include "at_sim_comm.h"
#include "at_sms_event_report.h"
#include "securec.h"
#include "at_check_func.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "taf_std_lib.h"
#include "mn_comm_api.h"
#include "at_common.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SIM_EVENT_REPORT_C

/*
 * 功能描述: ^USIMMEX
 * 修改历史:
 *  1.日    期: 2016年2月22日
 *    修改内容: 新生成函数
 */
TAF_UINT16 At_CardErrorInfoInd(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 i;

    if (event->pihEvent.usimmErrorInd.errNum == VOS_NULL) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^USIMMEX: NULL\r\n", g_atCrLf);

        return length;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^USIMMEX: ", g_atCrLf);

    event->pihEvent.usimmErrorInd.errNum = AT_MIN(event->pihEvent.usimmErrorInd.errNum, USIMM_ERROR_INFO_MAX);
    for (i = 0; i < event->pihEvent.usimmErrorInd.errNum; i++) {
        if (i == 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "0x%X", event->pihEvent.usimmErrorInd.errInfo[i]);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",0x%X", event->pihEvent.usimmErrorInd.errInfo[i]);
        }
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    return length;
}

/*
 * 功能描述: ^USIMICCID
 * 修改历史:
 *  1.日    期: 2016年2月22日
 *    修改内容: 新生成函数
 */
TAF_UINT16 At_CardIccidInfoInd(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^USIMICCID: ", g_atCrLf);

    length += (TAF_UINT16)AT_Hex2AsciiStrLowHalfFirst(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                      (TAF_UINT8 *)event->pihEvent.iccidContent, USIMM_ICCID_FILE_LEN);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    return length;
}

TAF_UINT16 At_SimHotPlugStatusInd(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s^SIMHOTPLUG: %d%s", g_atCrLf, event->pihEvent.simHotPlugStatus,
        g_atCrLf);
    return length;
}

TAF_UINT16 At_CardStatusInd(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;
    VOS_UINT16 tmpLen;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CARDSTATUSIND: ", g_atCrLf);

    /* 卡状态输出时，跳过消息头和消息名称 */
    tmpLen = (VOS_MSG_HEAD_LENGTH + sizeof(TAF_UINT32));

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                  (((TAF_UINT8 *)&event->pihEvent.cardStatusInd) + tmpLen),
                                                  (sizeof(USIMM_CardStatusInd) - tmpLen));

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    return length;
}

TAF_UINT16 At_SWCheckStatusInd(SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^SWCHECK: %d%s", g_atCrLf, event->pihEvent.apduSWCheckResult, g_atCrLf);
    return length;
}

TAF_UINT16 At_UpdateFileAtInd(SI_PIH_EventInfo *event)
{
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^CARDFETCHIND%s", g_atCrLf, g_atCrLf);
    return length;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/*
 * 功能描述: ^SIMREFRESH
 * 修改历史:
 *  1.日    期: 2020年5月18日
 *    修改内容: 新生成函数
 */
TAF_UINT16 AT_SimRefreshInd(SI_PIH_EventInfo *event)
{
    TAF_UINT16 length;

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^SIMREFRESH: %d%s", g_atCrLf, event->pihEvent.refreshType, g_atCrLf);
    return length;
}
#endif
#endif

TAF_VOID At_PIHIndProc(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_PIHIndProc: Get modem id fail.");
        return;
    }

    switch (event->eventType) {
        case SI_PIH_EVENT_HVRDH_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^HVRDH: %d%s", g_atCrLf, event->pihEvent.hvrdhInd.reDhFlag,
                g_atCrLf);
            break;

        case SI_PIH_EVENT_TEETIMEOUT_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^TEETIMEOUT: %d%s", g_atCrLf,
                event->pihEvent.teeTimeOut.data, g_atCrLf);
            break;

        case SI_PIH_EVENT_SIM_ERROR_IND:
            length += At_CardErrorInfoInd(indexNum, event);
            break;

        case SI_PIH_EVENT_SIM_ICCID_IND:
            length += At_CardIccidInfoInd(indexNum, event);
            break;
        case SI_PIH_EVENT_SIM_HOTPLUG_IND:
            length += At_SimHotPlugStatusInd(indexNum, event);
            break;

        case SI_PIH_EVENT_SW_CHECK_IND:
            length += At_SWCheckStatusInd(event);
            break;

        case SI_PIH_EVENT_CARDSTATUS_IND:
            length += At_CardStatusInd(indexNum, event);
            break;

        case SI_PIH_EVENT_UPDATE_FILE_AT_IND:
            length += At_UpdateFileAtInd(event);
            break;

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
        case SI_PIH_EVENT_SIM_REFRESH_IND:
            length += AT_SimRefreshInd(event);
            break;
#endif
#endif

        default:
            AT_WARN_LOG("At_PIHIndProc: Abnormal EventType.");
            return;
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvMmaImsiRefreshInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_IMSI_REFRESH].text,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_VOID AT_ProcUsimInfoInd(VOS_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_ProcUsimInfoInd: Get modem id fail.");
        return;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^SIMST:%d,%d%s", g_atCrLf, event->simStatus, event->meLockStatus,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID AT_ProcSimsqInd(VOS_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    VOS_UINT8     simsqEnable;
    ModemIdUint16 modemId;

    /* 将设置参数保存到CC上下文中 */
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_LOG1("AT_ProcSimsqInd AT_GetModemIdFromClient fail", indexNum);
        return;
    }

    simsqEnable = At_GetSimsqEnable();

    At_SetSimsqStatus(modemId, event->simStatus);

    if (simsqEnable == VOS_FALSE) {
        return;
    }

    AT_ProcReportSimSqInfo(indexNum, At_GetSimsqStatus(modemId));
}

VOS_VOID At_QryCpinRspProc(VOS_UINT8 indexNum, TAF_PH_PIN_TYPE pinType, VOS_UINT16 *length)
{
    if (pinType == TAF_SIM_PIN) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PIN");
    } else if (pinType == TAF_SIM_PUK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PUK");
    } else if (pinType == TAF_PHNET_PIN) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-NET PIN");
    } else if (pinType == TAF_PHNET_PUK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-NET PUK");
    } else if (pinType == TAF_PHNETSUB_PIN) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-NETSUB PIN");
    } else if (pinType == TAF_PHNETSUB_PUK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-NETSUB PUK");
    } else if (pinType == TAF_PHSP_PIN) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-SP PIN");
    } else if (pinType == TAF_PHSP_PUK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-SP PUK");
    } else if (pinType == TAF_PHCP_PIN) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-CP PIN");
    } else if (pinType == TAF_PHCP_PUK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "PH-CP PUK");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "READY");
    }
}

TAF_UINT32 At_ProcPinQuery(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_OK;

    /* AT+CLCK */
    if (g_parseContext[indexNum].cmdElement->cmdIndex == AT_CMD_CLCK) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        if (event->pinCnf.queryResult.usimmEnableFlg == TAF_PH_USIMM_ENABLE) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "1");
        } else {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "0");
        }
    }

    /* AT^CPIN */
    else if (g_parseContext[indexNum].cmdElement->cmdIndex == AT_CMD_CPIN_2) {
        if (event->pinCnf.pinType == TAF_SIM_PIN) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            *length +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PIN,%d,%d,%d,%d,%d",
                    event->pinCnf.remainTime.pin1RemainTime, event->pinCnf.remainTime.puk1RemainTime,
                    event->pinCnf.remainTime.pin1RemainTime, event->pinCnf.remainTime.puk2RemainTime,
                    event->pinCnf.remainTime.pin2RemainTime);
        } else if (event->pinCnf.pinType == TAF_SIM_PUK) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            *length +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PUK,%d,%d,%d,%d,%d",
                    event->pinCnf.remainTime.puk1RemainTime, event->pinCnf.remainTime.puk1RemainTime,
                    event->pinCnf.remainTime.pin1RemainTime, event->pinCnf.remainTime.puk2RemainTime,
                    event->pinCnf.remainTime.pin2RemainTime);
        } else {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "READY,,%d,%d,%d,%d",
                event->pinCnf.remainTime.puk1RemainTime, event->pinCnf.remainTime.pin1RemainTime,
                event->pinCnf.remainTime.puk2RemainTime, event->pinCnf.remainTime.pin2RemainTime);
        }
    }
    /* AT+CPIN */
    else {
        At_QryCpinRspProc(indexNum, event->pinCnf.pinType, length);
    }

    return result;
}

TAF_UINT32 At_ProcPin2Query(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_FAILURE;

    /* AT^CPIN2 */
    if (g_parseContext[indexNum].cmdElement->cmdIndex == AT_CMD_CPIN2) {
        if (event->pinCnf.pinType == TAF_SIM_PIN2) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            *length +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PIN2,%d,%d,%d,%d,%d",
                    event->pinCnf.remainTime.pin2RemainTime, event->pinCnf.remainTime.puk1RemainTime,
                    event->pinCnf.remainTime.pin1RemainTime, event->pinCnf.remainTime.puk2RemainTime,
                    event->pinCnf.remainTime.pin2RemainTime);
        } else if (event->pinCnf.pinType == TAF_SIM_PUK2) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            *length +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "SIM PUK2,%d,%d,%d,%d,%d",
                    event->pinCnf.remainTime.puk2RemainTime, event->pinCnf.remainTime.puk1RemainTime,
                    event->pinCnf.remainTime.pin1RemainTime, event->pinCnf.remainTime.puk2RemainTime,
                    event->pinCnf.remainTime.pin2RemainTime);
        } else {
            result = AT_CME_SIM_FAILURE;

            return result;
        }
    } else {
        result = AT_ERROR;

        return result;
    }

    result = AT_OK;

    return result;
}

TAF_UINT32 At_ProcPinResultPinOk(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, VOS_BOOL *pbNeedRptPinReady,
                                 TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_FAILURE;

    switch (event->pinCnf.cmdType) {
        case TAF_PIN_QUERY:

            result = At_ProcPinQuery(event, length, indexNum);

            break;

        case TAF_PIN2_QUERY:

            result = At_ProcPin2Query(event, length, indexNum);

            break;

        case TAF_PIN_VERIFY:
        case TAF_PIN_UNBLOCK:
            if (event->pinCnf.pinType == TAF_SIM_NON) {
                result = AT_ERROR;
            } else {
                result = AT_OK;

                *pbNeedRptPinReady = VOS_TRUE;
            }

            break;

        case TAF_PIN_CHANGE:
        case TAF_PIN_DISABLE:
        case TAF_PIN_ENABLE:

            result = AT_OK;

            break;

        default:

            return AT_RRETURN_CODE_BUTT;
    }

    return result;
}

TAF_UINT32 At_ProcPinResultNotPinOk(TAF_PHONE_EventInfo *event, VOS_BOOL *pbNeedRptNeedPuk)
{
    TAF_UINT32 result = AT_FAILURE;

    switch (event->pinCnf.opPinResult) {
        case TAF_PH_OP_PIN_NEED_PIN1:
            result = AT_CME_SIM_PIN_REQUIRED;
            break;

        case TAF_PH_OP_PIN_NEED_PUK1:
            *pbNeedRptNeedPuk = VOS_TRUE;
            result            = AT_CME_SIM_PUK_REQUIRED;
            break;

        case TAF_PH_OP_PIN_NEED_PIN2:
            result = AT_CME_SIM_PIN2_REQUIRED;
            break;

        case TAF_PH_OP_PIN_NEED_PUK2:
            result = AT_CME_SIM_PUK2_REQUIRED;
            break;

        case TAF_PH_OP_PIN_INCORRECT_PASSWORD:
            result = AT_CME_INCORRECT_PASSWORD;
            break;

        case TAF_PH_OP_PIN_OPERATION_NOT_ALLOW:
            result = AT_CME_OPERATION_NOT_ALLOWED;
            break;

        case TAF_PH_OP_PIN_SIM_FAIL:
            result = AT_CME_SIM_FAILURE;
            break;

        default:
            result = AT_CME_UNKNOWN;
            break;
    }

    return result;
}

VOS_VOID AT_PhSendPinReady(VOS_UINT16 modemID)
{
    VOS_UINT32 i;
    VOS_UINT16 length;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if ((modemID == g_atClientCtx[i].clientConfiguration.modemId) && (g_atClientTab[i].userType == AT_APP_USER)) {
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
        (VOS_CHAR *)g_atSndCodeAddress + length, "^CPINNTY:READY");
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData((VOS_UINT8)i, g_atSndCodeAddress, length);
}

VOS_VOID AT_PhSendNeedPuk(VOS_UINT16 modemID)
{
    VOS_UINT32 i;
    VOS_UINT16 length;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if ((modemID == g_atClientCtx[i].clientConfiguration.modemId) && (g_atClientTab[i].userType == AT_APP_USER)) {
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
        (VOS_CHAR *)g_atSndCodeAddress + length, "^CPINNTY:SIM PUK");

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData((VOS_UINT8)i, g_atSndCodeAddress, length);
}

TAF_UINT32 At_ProcPhoneEvtOperPinCnf(TAF_PHONE_EventInfo *event, TAF_UINT8 indexNum)
{
    TAF_UINT32 rst;
    TAF_UINT16 length = 0;
    TAF_UINT32 result = AT_FAILURE;
    VOS_BOOL   bNeedRptPinReady = VOS_FALSE;
    VOS_BOOL   bNeedRptNeedPuk = VOS_FALSE;
    ModemIdUint16 modemId = MODEM_ID_0;

    rst = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rst != VOS_OK) {
        AT_ERR_LOG1("At_ProcPhoneEvtOperPinCnf:Get ModemID From ClientID fail,ClientID:", indexNum);
        return AT_RRETURN_CODE_BUTT;
    }

    if (event->opPhoneError == 1) {                               /* MT本地错误 */
        result = At_ChgTafErrorCode(indexNum, event->phoneError); /* 发生错误 */

        if ((result == AT_CME_SIM_PUK_REQUIRED) && (event->pinCnf.cmdType == TAF_PIN_VERIFY)) {
            bNeedRptNeedPuk = VOS_TRUE;
        }
    } else {
        if (event->pinCnf.opPinResult == TAF_PH_OP_PIN_OK) {
            result = At_ProcPinResultPinOk(event, &length, &bNeedRptPinReady, indexNum);

            if (result == AT_RRETURN_CODE_BUTT) {
                AT_NORM_LOG("At_ProcPhoneEvtOperPinCnf: return AT_RRETURN_CODE_BUTT");

                return AT_RRETURN_CODE_BUTT;
            }
        } else {
            result = At_ProcPinResultNotPinOk(event, &bNeedRptNeedPuk);
        }
    }

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);

    if (bNeedRptPinReady == VOS_TRUE) {
        AT_PhSendPinReady(modemId);
    }

    if (bNeedRptNeedPuk == VOS_TRUE) {
        AT_PhSendNeedPuk(modemId);
    }

    return AT_RRETURN_CODE_BUTT;
}

VOS_VOID AT_PhSendSimLocked(VOS_VOID)
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
        (VOS_CHAR *)g_atSndCodeAddress + length, "^CARDLOCKNTY:SIM LOCKED");

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData((VOS_UINT8)i, g_atSndCodeAddress, length);
}

/*
 * 功能描述: 处理TAF_PH_EVT_USIM_MATCH_FILES_IND事件
 */
VOS_VOID AT_RcvMmaUsimMatchFilesInd(VOS_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvMmaUsimMatchFilesInd: Get modem id fail.");
        return;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^MATCHFILE: %d,%d,%d,", g_atCrLf, event->usimMatchFile.fileType,
        event->usimMatchFile.totalLen, event->usimMatchFile.curLen);

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + length, event->usimMatchFile.content,
                                                  event->usimMatchFile.curLen);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

TAF_UINT32 At_ProcPhoneEvtUsimResponse(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_OK;

    /* +CSIM:  */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (event->opUsimAccessData == 1) {
        /* <length>, */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"",
            event->usimAccessData.len * AT_ASCII_AND_HEX_CONVERSION_FACTOR);
        /* <command>, */
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                       (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                       event->usimAccessData.response, event->usimAccessData.len);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
    }

    AT_StopTimerCmdReady(indexNum);

    return result;
}

VOS_UINT32 At_ProcPhoneEvtOpPinReminCnf(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,%d,%d,%d ", event->pinRemainCnf.pIN1Remain,
        event->pinRemainCnf.pUK1Remain, event->pinRemainCnf.pIN2Remain, event->pinRemainCnf.pUK2Remain);

    AT_StopTimerCmdReady(indexNum);

    return AT_OK;
}

