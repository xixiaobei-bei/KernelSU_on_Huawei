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
#include "at_voice_comm.h"
#include "securec.h"
#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_COMM_C

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT8 AT_IsEcallType(TAFAGENT_CALL_InfoParam callInfo)
{
    /* 当前存在ECALL呼叫或紧急呼叫，但还未发起至呼叫模块，呼叫状态由CCM返回 */
    if ((callInfo.callType == MN_CALL_TYPE_EMERGENCY) ||
        (callInfo.callType == MN_CALL_TYPE_PSAP_ECALL) ||
        (callInfo.callType == MN_CALL_TYPE_MIEC) ||
        (callInfo.callType == MN_CALL_TYPE_AIEC) ||
        (callInfo.callType == MN_CALL_TYPE_TEST) ||
        (callInfo.callType == MN_CALL_TYPE_RECFGURATION)) {
        return VOS_TRUE;
    }

    /* 当前存在已经发起的ECALL呼叫，呼叫信息由IMSA/GUCALL返回 */
    if ((callInfo.serviceType == TAF_CALL_SERVICE_TYPE_PSAP_ECALL) ||
        (callInfo.serviceType == TAF_CALL_SERVICE_TYPE_MIEC) ||
        (callInfo.serviceType == TAF_CALL_SERVICE_TYPE_AIEC) ||
        (callInfo.serviceType == TAF_CALL_SERVICE_TYPE_TEST_ECALL) ||
        (callInfo.serviceType == TAF_CALL_SERVICE_TYPE_RECFGURATION_ECALL)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT8 AT_IsHaveEcallExsit(TAFAGENT_CALL_InfoParam callInfos, VOS_UINT8 checkFlag)
{
    if (AT_IsEcallType(callInfos) == VOS_TRUE) {
        if (checkFlag == VOS_TRUE) {
            if (callInfos.callState == MN_CALL_S_ACTIVE) {
                return VOS_TRUE;
            }
        } else {
            return VOS_TRUE;
        }
    }
    return VOS_FALSE;
}

VOS_UINT8 AT_HaveEcallActive(VOS_UINT8 indexNum, VOS_UINT8 checkFlag)
{
    VOS_UINT8  numOfCalls         = 0;
    VOS_UINT32 i                  = 0;
    VOS_UINT8  numOfCallsSmallNum = 0;
    VOS_UINT32 rst;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_HaveEcallActive : ucIndex.");

        return VOS_FALSE;
    }
    (VOS_VOID)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));

    /* 从同步API获取通话信息 */
    rst = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);

    if (rst != VOS_OK) {
        AT_ERR_LOG("AT_HaveEcallActive : TAF_AGENT_GetCallInfoReq.");

        return VOS_FALSE;
    }

    AT_NORM_LOG1("AT_HaveEcallActive : [ucCheckFlag]", checkFlag);

    numOfCallsSmallNum = TAF_MIN(numOfCalls, MN_CALL_MAX_NUM);
    /* 当前有ECALL 通话返回TRUE */
    for (i = 0; i < numOfCallsSmallNum; i++) {
        if (AT_IsHaveEcallExsit(callInfos[i], checkFlag) == VOS_TRUE) {
            return VOS_TRUE;
        }
    }
    return VOS_FALSE;
}

VOS_VOID AT_EcallAlackDisplay(AT_ECALL_AlackValue ecallAlackInfo, VOS_UINT16 *length)
{
    TAF_INT8        timeZone = 0;
    TIME_ZONE_Time *timeInfo = VOS_NULL_PTR;

    timeInfo = &ecallAlackInfo.ecallAlackTimeInfo.universalTimeandLocalTimeZone;
    /* YYYY */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%4d/",
        timeInfo->year + 2000); /* year */

    /* MM */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d%d/",
        timeInfo->month / AT_DECIMAL_BASE_NUM,  /* month high */
        timeInfo->month % AT_DECIMAL_BASE_NUM); /* month low */
    /* dd */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d%d,",
        timeInfo->day / AT_DECIMAL_BASE_NUM,  /* day high */
        timeInfo->day % AT_DECIMAL_BASE_NUM); /* day low */

    /* hh */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d%d:",
        timeInfo->hour / AT_DECIMAL_BASE_NUM,  /* hour high */
        timeInfo->hour % AT_DECIMAL_BASE_NUM); /* hour low */

    /* mm */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length,
        "%d%d:", timeInfo->minute / AT_DECIMAL_BASE_NUM, /* minutes high */
        timeInfo->minute % AT_DECIMAL_BASE_NUM);         /* minutes high */

    /* ss */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d%d",
        timeInfo->second / AT_DECIMAL_BASE_NUM,  /* sec high */
        timeInfo->second % AT_DECIMAL_BASE_NUM); /* sec low */

    /* 获得时区 */
    if ((ecallAlackInfo.ecallAlackTimeInfo.ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
        timeZone = ecallAlackInfo.ecallAlackTimeInfo.localTimeZone;
    } else {
        timeZone = timeInfo->timeZone;
    }

    if (timeZone == AT_INVALID_TZ_VALUE) {
        timeZone = 0;
    }

    if (timeZone != AT_INVALID_TZ_VALUE) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%02d\"",
            (timeZone < 0) ? "-" : "+", (timeZone < 0) ? (-timeZone) : timeZone);
    }

    /* AlackValue */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", ecallAlackInfo.ecallAlackValue);
}
#endif

