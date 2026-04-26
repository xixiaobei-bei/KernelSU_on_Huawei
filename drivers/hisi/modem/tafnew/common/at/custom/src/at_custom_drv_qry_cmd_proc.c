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
#include "at_custom_drv_qry_cmd_proc.h"
#include "securec.h"
#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "at_mdrv_interface.h"
#include "at_input_proc.h"
#include "at_custom_comm.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "taf_sdc_ctx.h"
#include "taf_mta_drv_agent_msg_proc.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_DRV_QRY_CMD_PROC_C

VOS_UINT32 At_QryUsbSwitchPara(VOS_UINT8 indexNum)
{
    VOS_UINT8  usbStatus = USB_SWITCH_OFF;
    VOS_UINT32 result;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    if (DRV_USB_PHY_SWITCH_GET(&usbStatus) != VOS_OK) {
        return AT_ERROR;
    }

    result = (usbStatus == USB_SWITCH_ON) ? AT_USB_SWITCH_SET_VBUS_VALID : AT_USB_SWITCH_SET_VBUS_INVALID;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "+USBSWITCH: %X", result);

    return AT_OK;
}

VOS_UINT32 AT_TestUsbSwitchPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1,2)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 At_QryLogPortPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 omLogPort = AT_LOG_PORT_USB;
    VOS_UINT32 atLogPort;
    VOS_UINT32 rslt;

    AT_PR_LOGI("Rcv Msg");

    rslt = AT_QueryPpmLogPort(&omLogPort);

    AT_PR_LOGI("Call interface success!");

    if (rslt != VOS_OK) {
        return AT_ERROR;
    }

    if (omLogPort == COMM_LOG_PORT_USB) {
        atLogPort = AT_LOG_PORT_USB;
    } else {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        atLogPort = (omLogPort == COMM_LOG_PORT_VCOM) ? AT_LOG_PORT_VCOM : AT_LOG_PORT_WIFI;
#else
        atLogPort = AT_LOG_PORT_VCOM;
#endif
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", atLogPort);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_QryLogCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 atLogCfg = 0;

    PS_PRINTF_INFO("at^logcfg?!\n");

    (void)AT_GetSocpCfgIndMode(&atLogCfg);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", atLogCfg);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}
/*
 * 功能描述: ^LOGCPS的查询函数
 */
VOS_UINT32 At_QryLogCpsPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 atLogCps = 0;
    PS_PRINTF_INFO("at^logcps?!\n");

    (void)AT_GetSocpCompressIndMode(&atLogCps);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", atLogCps);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryNvLoadPara(VOS_UINT8 indexNum)
{
    AT_ModemNvloadCtx  *nvloadCarrierCtx = VOS_NULL_PTR;
    errno_t             memResult;
    VOS_UINT16          length  = 0;
    ModemIdUint16       modemId = MODEM_ID_0;
    VOS_UINT8           carrierName[AT_SET_NV_CARRIER_NAME_LEN + 1];

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_QryNvLoadPara: Get modem id fail.");
        return AT_ERROR;
    }

    nvloadCarrierCtx = AT_GetModemNvloadCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(carrierName, sizeof(carrierName), 0, sizeof(carrierName));

    memResult = memcpy_s(carrierName, sizeof(carrierName), nvloadCarrierCtx->carrierName, AT_SET_NV_CARRIER_NAME_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(carrierName), AT_SET_NV_CARRIER_NAME_LEN);

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, " \"%s\",", carrierName);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, " %d", nvloadCarrierCtx->specialEffectiveFlg);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryFlashInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取FLASH信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_FLASHINFO_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryFlashInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FLASHINFO_READ;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryAppdmverPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取 PDM版本号 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_APPDMVER_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryAppdmverPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APPDMVER_READ;
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/* 功能描述 : 查询加密狗公钥ID */
VOS_UINT32 At_QryMbbDongleInfo(VOS_UINT8 indexNum)
{
    VOS_UINT32 result = AT_OK;
    VOS_UINT32 ids[NET_DOG_KEY_NUM] = {0}; /* NETDOG有3个密钥 */

    sec_get_pub_key_ids(ids, NET_DOG_KEY_NUM);

    /* 输出查询结果: 鉴权加密公钥ID值, 鉴权签名公钥ID值, 数据签名公钥ID值 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
        (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress,
        "%s: RSA-2048,0x%X,0x%X,0x%X", g_parseContext[indexNum].cmdElement->cmdName, ids[0], ids[1], ids[2]);

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_UINT32 AT_MbbQryWakeupcfgPara(TAF_UINT8 indexNum)
{
    TAF_NVIM_MbbWakeupCfg wakeupCfg = {0};

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->powerManagerSwitch == VOS_FALSE) {
        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAKEUP_CFG, &wakeupCfg, sizeof(TAF_NVIM_MbbWakeupCfg)) != NV_OK) {
        AT_WARN_LOG("AT_MbbQryWakeupcfgPara: nv read fail.");
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)g_atSndCodeAddress,
                                                (VOS_CHAR *)g_atSndCodeAddress,
                                                "%s: %u,%u,%u",
                                                g_parseContext[indexNum].cmdElement->cmdName,
                                                wakeupCfg.remoteWakeupCtrl,
                                                wakeupCfg.remoteWakeupChannel,
                                                wakeupCfg.remoteWakeupSource);
    return AT_OK;
}

VOS_UINT32 At_QryMbbNetDogIntegrity(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_INT32 integrity;

    integrity = sec_data_integrity_check();

    /* 输出查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %u", g_parseContext[indexNum].cmdElement->cmdName, integrity);

    g_atSendDataBuff.bufLen = length;
    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_QryMaxFreelockSizePara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取 MFREELOCKSIZE信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_MFREELOCKSIZE_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryMaxFreelockSizePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MFREELOCKSIZE_READ;
    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryYjcxPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送DRV_AGENT_YJCX_QRY_REQ消息给AT代理处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_YJCX_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_WUEPS_PID_DRV_AGENT);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_YJCX_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif


