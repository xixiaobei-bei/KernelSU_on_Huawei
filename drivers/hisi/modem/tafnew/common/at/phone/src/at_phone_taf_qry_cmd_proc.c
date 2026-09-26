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
#include "at_phone_taf_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_phone_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_TAF_QRY_CMD_PROC_C

TAF_UINT32 At_QryCgclassPara(TAF_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "\"A\"");

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 At_QryCregPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QryRegStateReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, TAF_MMA_QRY_REG_STATUS_TYPE_CS) ==
        VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CREG_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCgregPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QryRegStateReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, TAF_MMA_QRY_REG_STATUS_TYPE_PS) ==
        VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGREG_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryTimePara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptQryReq atCmd;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_UnsolicitedRptQryReq));
    atCmd.reqType = AT_MTA_QRY_TIME_RPT_TYPE;

    /* 给MTA发送^time查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_QRY_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptQryReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryTimeQryPara(VOS_UINT8 indexNum)
{
    VOS_UINT16      length = 0;
    VOS_UINT32      nwSecond;
    AT_ModemNetCtx *netCtx   = VOS_NULL_PTR;
    NAS_MM_InfoInd *timeInfo = VOS_NULL_PTR;

    netCtx   = AT_GetModemNetCtxAddrFromModemId(MODEM_ID_0);
    timeInfo = &(netCtx->timeInfo);
    nwSecond = netCtx->nwSecond;

    /* 如果当前已经拿到C核的MM INFO直接返回结果 */
    if ((timeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        length = AT_PrintNwTimeInfo(timeInfo, (VOS_CHAR *)g_parseContext[indexNum].cmdElement->cmdName, nwSecond);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

    /* 如果没有拿到过MM INFO的时间信息则需要从C核拿PLMN */
    if (TAF_MMA_QryPlmnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TIMEQRY_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCtzrPara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptQryReq atCmd;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_UnsolicitedRptQryReq));
    atCmd.reqType = AT_MTA_QRY_CTZR_RPT_TYPE;

    /* 给MTA发送^ctzr查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_QRY_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptQryReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryXlemaPara(VOS_UINT8 indexNum)
{
    VOS_UINT32    rst;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发消息到C核查询紧急呼叫号码 */
    rst = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_QRY_XLEMA_REQ, 0, modemId);

    if (rst != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_QryXlemaPara: TAF_CCM_CallCommonReq fail.");
        return AT_ERROR;
    }

    /* 设置结束直接返回OK */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_XLEMA_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryPacspPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_QryPacspReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PACSP_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryJDCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 下发干扰检测配置查询请求消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_QRY_JAM_DETECT_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_JDCFG_READ;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryJDSwitchPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 下发JD查询请求消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_QRY_JAM_DETECT_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_JDSWITCH_READ;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 At_QryCeregPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryRegStateReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, TAF_MMA_QRY_REG_STATUS_TYPE_EPS) ==
        VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CEREG_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_QryC5gregPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryRegStateReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, TAF_MMA_QRY_REG_STATUS_TYPE_5GC) ==
        VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GREG_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryTTYModePara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给VC 发送模式查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    APP_VC_MSG_QRY_TTYMODE_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_VC);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryTTYModePara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TTYMODE_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCsdfPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        netCtx->csdfCfg.mode, netCtx->csdfCfg.auxMode);

    return AT_OK;
}

VOS_UINT32 AT_TestJDCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    /* 输出GSM的参数列表 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0),(0-70),(0-255)%s",
        g_parseContext[indexNum].cmdElement->cmdName, g_atCrLf);

    /* 输出WCDMA的参数列表 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (1),(0-70),(0-100),(0-65535),(0-100)%s",
        g_parseContext[indexNum].cmdElement->cmdName, g_atCrLf);

    /* 输出LTE的参数列表 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (2),(0-70),(0-100),(0-1000),(0-100)",
        g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryCclkPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    VOS_UINT32      rslt;
    ModemIdUint16   modemId = MODEM_ID_0;
    TIME_ZONE_Time  time;

    (VOS_VOID)memset_s(&time, sizeof(time), 0x00, sizeof(time));

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_QryCclkPara: Get modem id fail.");
        return AT_ERROR;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if ((netCtx->timeInfo.ieFlg & NAS_MM_INFO_IE_UTLTZ) != NAS_MM_INFO_IE_UTLTZ) {
        /* AT 给MTA发送查询请求消息 */
        rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_CCLK_QRY_REQ, VOS_NULL_PTR, 0,
                                      I0_UEPS_PID_MTA);

        if (rslt != TAF_SUCCESS) {
            AT_WARN_LOG("AT_QryCclkPara: send Msg fail.");
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCLK_QRY;

        return AT_WAIT_ASYNC_RETURN;
    }

    /* 获得时区 */
    time.timeZone = netCtx->timeInfo.universalTimeandLocalTimeZone.timeZone;

    /* 若已经获得 Local time zone,则时区修改为 Local time zone */
    if ((netCtx->timeInfo.ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
        time.timeZone = netCtx->timeInfo.localTimeZone;
    }

    time.year   = netCtx->timeInfo.universalTimeandLocalTimeZone.year;
    time.month  = netCtx->timeInfo.universalTimeandLocalTimeZone.month;
    time.day    = netCtx->timeInfo.universalTimeandLocalTimeZone.day;
    time.hour   = netCtx->timeInfo.universalTimeandLocalTimeZone.hour;
    time.minute = netCtx->timeInfo.universalTimeandLocalTimeZone.minute;
    time.second = netCtx->timeInfo.universalTimeandLocalTimeZone.second;

    AT_PrintCclkTime(indexNum, &time, modemId);

    return AT_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 At_MbbQryCclkPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    NAS_MM_InfoInd *timeInfo = VOS_NULL_PTR;
    VOS_UINT32 second;
    VOS_UINT16 length;

    netCtx = AT_GetModemNetCtxAddrFromModemId(MODEM_ID_0);
    timeInfo = &(netCtx->cclkTimeInfo);
    second = netCtx->cclkSecond;

    if ((timeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        length = AT_PrintMbbCclkTimeInfo(timeInfo, (VOS_CHAR*)g_parseContext[indexNum].cmdElement->cmdName, second);
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }

    return AT_ERROR;
}
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
TAF_UINT32 At_QryCmuxPara(TAF_UINT8 indexNum)
{
    cmux_info_type* cmux_info = VOS_NULL_PTR;
    VOS_UINT16      len = 0;

    /* 检查通道，只支持UART/CMUX/HSUART端口下发 */
    if ((AT_CheckCmuxUser(indexNum) != VOS_TRUE) && (AT_CheckUartUser(indexNum) != VOS_TRUE) &&
        (AT_CheckHsUartUser(indexNum) != VOS_TRUE)) {
        return AT_ERROR;
    }

    cmux_info = DMS_PORT_GetCmuxInfoType();
    len = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d",
                                g_parseContext[indexNum].cmdElement->cmdName,
                                cmux_info->operating_mode, cmux_info->subset,
                                cmux_info->port_speed, cmux_info->max_frame_size_N1,
                                cmux_info->response_timer_T1 / T1_T2_FACTOR, cmux_info->max_cmd_num_tx_times_N2,
                                cmux_info->response_timer_T2 / T1_T2_FACTOR, cmux_info->response_timer_T3,
                                cmux_info->window_size_k);

    g_atSendDataBuff.bufLen = len;

    return AT_OK;
}

VOS_UINT32 At_TestCmuxPara(VOS_UINT8 indexNum)
{
    /* 检查通道，只支持UART和HSUART端口下发 */
    if ((AT_CheckUartUser(indexNum) != VOS_TRUE) && (AT_CheckHsUartUser(indexNum) != VOS_TRUE)) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,(TAF_CHAR *)g_atSndCodeAddress,
                                (TAF_CHAR *)g_atSndCodeAddress,
                                "%s+CMUX: (0),(0),(1-6),(31-1540),(10-250),(0-10),(10-250),(1-255),(1-7)%s",
                                g_atCrLf,g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    return AT_OK;
}

#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 At_QryCpuLoadPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取 CPULOAD信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_CPULOAD_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryCpuLoadPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPULOAD_READ;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

