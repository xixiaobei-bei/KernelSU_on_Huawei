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
#include "at_phone_ims_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_phone_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_IMS_QRY_CMD_PROC_C

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryBatteryInfoPara(TAF_UINT8 indexNum)
{
    AT_ModemImsContext *localBatteryInfo = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    localBatteryInfo = AT_GetModemImsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);

    if (localBatteryInfo->batteryInfo.currBatteryInfo < AT_IMSA_BATTERY_STATUS_BUTT) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            localBatteryInfo->batteryInfo.currBatteryInfo);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCiregPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送消息 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_CIREG_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryCiregPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIREG_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCirepPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送消息 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_CIREP_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryCirepPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIREP_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestCiregPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-2)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryImsPcscfPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    AT_INFO_LOG("AT_QryImsPcscfPara Entered");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_PCSCF_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryImsPcscfPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PCSCF_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDmDynPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_DMDYN_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryDmdynPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMDYN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryImsTimerPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG1("AT_QryImsTimerPara: cmt type is not AT_CMD_OPT_READ_CMD", g_atParseCmd.cmdOptType);
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_IMSTIMER_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_QryImsTimerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMTIMER_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryImsSmsPsiPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_SMSPSI_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_QryImsSmsPsiPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSPSI_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDmUserPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_DMUSER_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_QryDmUserPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMUSER_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryRoamImsServicePara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_ROAMING_IMS_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryRoamImsServicePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 查询结束挂起通道 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ROAM_IMS_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestCirepPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}
#endif

