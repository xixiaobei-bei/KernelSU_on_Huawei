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
#include "at_custom_taf_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "at_input_proc.h"

#include "taf_ps_api.h"
#include "at_data_proc.h"
#include "at_lte_common.h"
#include "at_mdrv_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_TAF_QRY_CMD_PROC_C

#define AT_RPT_CFG_BIT_32_INDEX_0 0
#define AT_RPT_CFG_BIT_32_INDEX_1 1
#define AT_RPT_CFG_BIT_32_INDEX_2 2
#define AT_QRY_AUTHORITY_VER_LEN 5
#define AT_NV_IMEI_LEN 15

#define AT_SOCP_DST_CHAN_DELAY 1
#define AT_SOCP_DST_CHAN_DTS 2

#define AT_RSRP_CFG_SS_VALUE_LEN 4
#define AT_RSRP_CFG_SS_VALUE_INDEX_3 3
#define AT_RSCP_CFG_SS_VALUE_LEN 4
#define AT_RSCP_CFG_SS_VALUE_INDEX_3 3

#define AT_ECIO_CFG_SS_VALUE_LEN 4
#define AT_ECIO_CFG_SS_VALUE_INDEX_3 3

/* 记录闪电卡版本从上电到拨号成功启动时间，单位秒 */
VOS_UINT32 g_lcStartTime = 0;

VOS_UINT32 AT_QryApRptPortSelectPara(VOS_UINT8 indexNum)
{
    AT_PortRptCfg                 rptCfg;
    AT_ClientConfiguration       *clientCfg = VOS_NULL_PTR;
    const AT_ClientCfgMapTab     *cfgMapTbl = VOS_NULL_PTR;
    VOS_UINT8                     i;

    (VOS_VOID)memset_s(&rptCfg, sizeof(rptCfg), 0x00, sizeof(rptCfg));

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    for (i = 0; i < AT_GET_CLIENT_CFG_TAB_LEN(); i++) {
        cfgMapTbl = AT_GetClientCfgMapTbl(i);
        clientCfg = AT_GetClientConfig(cfgMapTbl->clientId);

        if (clientCfg->reportFlg == VOS_TRUE) {
            rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_0] |= cfgMapTbl->rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_0];
            rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_1] |= cfgMapTbl->rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_1];
            rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_2] |= cfgMapTbl->rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_2];
        }
    }

    /* 返回查询结果 */
    g_atSendDataBuff.bufLen =
        (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %08X, %08X, %08X", g_parseContext[indexNum].cmdElement->cmdName,
            rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_0], rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_1],
            rptCfg.rptCfgBit32[AT_RPT_CFG_BIT_32_INDEX_2]);

    return AT_OK;
}

VOS_UINT32 AT_QryCposrPara(VOS_UINT8 indexNum)
{
    AT_ModemAgpsCtx *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(indexNum);

    /* 打印+CPOSR主动上报控制当前状态 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            agpsCtx->cposrReport);

    return AT_OK;
}

VOS_UINT32 AT_QryXcposrPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 给MTA发送+xcposr查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_XCPOSR_QRY_REQ, VOS_NULL_PTR, 0,
                                    I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryXcposrPara: send Msg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_XCPOSR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestXcposrPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryXcposrRptPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 给MTA发送查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_XCPOSRRPT_QRY_REQ, VOS_NULL_PTR, 0,
                                    I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryXcposrRptPara: send Msg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_XCPOSRRPT_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestApRptPortSelectPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-FFFFFFFF), (0-FFFFFFFF), (0-FFFFFFFF)",
        g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryMipiClkValue(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* AT 给MTA 发送查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_MIPICLK_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryMipiClkValue: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MIPI_CLK_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryBestFreqPara(VOS_UINT8 indexNum)
{
    VOS_UINT32    result;
    VOS_UINT32    rslt;
    ModemIdUint16 modemId = MODEM_ID_BUTT;

    rslt    = AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId);

    /* 如果ModemID获取失败或不在Modem0，返回失败 */
    if ((modemId != MODEM_ID_0) || (rslt != VOS_OK)) {
        AT_WARN_LOG("AT_QryBestFreqPara: Not  Modem0 or fail to get current ModemID!");
        return AT_ERROR;
    }

    /* AT 给MTA发送查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_BESTFREQ_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryBestFreqPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_BESTFREQ_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryLogEnable(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_QryCtzuPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, g_ctzuFlag);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryCrrconnPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给MTA 发送查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_CRRCONN_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryCrrconnPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CRRCONN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDataSwitchStatus(VOS_UINT8 indexNum)
{
    /* 发送跨核消息 */
    if (TAF_PS_Get_DataSwitchStatus(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DATASWITCH_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDataRoamSwitchStatus(VOS_UINT8 indexNum)
{
    /* 发送跨核消息 */
    if (TAF_PS_Get_DataRoamSwitchStatus(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DATAROAMSWITCH_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryLteAttachInfoPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetLteAttchInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId),
                               g_atClientTab[indexNum].opId) != VOS_OK) {
        AT_WARN_LOG("AT_QryLteAttachInfoPara: TAF_PS_GetLteAttchInfo fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTEATTACHINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryLendcPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_LENDC_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryLendcPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LENDC_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryLadnInfoPara(VOS_UINT8 atIndex)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[atIndex].clientId, g_atClientTab[atIndex].opId,
                                    ID_AT_MTA_QRY_LADN_INFO_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryLadnInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
    g_atClientTab[atIndex].cmdCurrentOpt = AT_CMD_CLADN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_QryLcStartTimePara(VOS_UINT8 indexNum)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    if (*systemAppConfig != SYSTEM_APP_WEBUI) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, g_lcStartTime);
    return AT_OK;
}

VOS_UINT32 At_GetNvAuthorityVer(VOS_UINT32 *pdata)
{
    VOS_UINT32 rst;

    if (pdata == NULL) {
        return ((VOS_UINT32)-1);
    }

    rst = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AUTHORITY_VER, pdata, sizeof(VOS_UINT32));

    if (rst != 0) {
        *pdata = 0;
    }

    return 0;
}

VOS_UINT32 atQryAuthorityVer(VOS_UINT8 clientId)
{
    errno_t    memResult;
    VOS_UINT32 rst;
    VOS_UINT32 authorityVer = 0;
    VOS_UINT8 ver[AT_QRY_AUTHORITY_VER_LEN] = {0};
    VOS_UINT32 i = 0;

    rst = At_GetNvAuthorityVer(&authorityVer);
    if (rst != ERR_MSP_SUCCESS) {
        CmdErrProc(clientId, rst, 0, NULL);
        return AT_ERROR;
    }

    memResult = memcpy_s(ver, sizeof(ver), &authorityVer, sizeof(VOS_UINT32));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ver), sizeof(VOS_UINT32));

    for (i = 0; i < (AT_QRY_AUTHORITY_VER_LEN - 1); i++) {
        ver[i] = (ver[i] > AT_DEC_MAX_NUM ? 0 : ver[i]) + '0';
    }

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "\r%s\r", ver);
    return AT_OK;
}

VOS_UINT32 At_GetImei(VOS_CHAR szimei[], VOS_UINT32 szimeiLen)
{
    VOS_UINT32 ret;
    VOS_UINT32 subscript                   = 0;
    VOS_CHAR   checkdata                   = 0;

    ret = TAF_ACORE_NV_READ(MODEM_ID_0, 0, szimei, szimeiLen);

    if (ret != 0) {
        return ret;
    } else {
        /* 一次赋值为subscript和subscript + 1, 所以循环加2 */
        for (subscript = 0; subscript < (szimeiLen - 2); subscript += 2) {
            checkdata += (VOS_CHAR)(((szimei[subscript]) +
                ((szimei[(VOS_UINT32)(subscript + 1)] * AT_DOUBLE_LENGTH) / AT_DECIMAL_BASE_NUM)) +
                ((szimei[(VOS_UINT32)(subscript + 1)] * AT_DOUBLE_LENGTH) % AT_DECIMAL_BASE_NUM));
        }
        checkdata = (AT_DECIMAL_BASE_NUM - (checkdata % AT_DECIMAL_BASE_NUM)) % AT_DECIMAL_BASE_NUM;

        for (subscript = 0; subscript < szimeiLen; subscript++) {
            *(szimei + subscript) = *(szimei + subscript) + 0x30; /* 字符转换 */
        }

        szimei[szimeiLen - 2] = checkdata + 0x30;
        szimei[szimeiLen - 1] = 0;
    }

    return 0;
}

VOS_UINT32 atQryAuthorityID(VOS_UINT8 clientId)
{
    VOS_UINT32 rst;
    VOS_UINT8  data[NV_ITEM_IMEI_SIZE] = {0};
    g_atSendDataBuff.bufLen            = 0;

    rst = At_GetImei((VOS_CHAR *)data, NV_ITEM_IMEI_SIZE);
    if (rst != ERR_MSP_SUCCESS) {
        CmdErrProc(clientId, rst, 0, NULL);
        return AT_ERROR;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "\r%s\r", data);

    return AT_OK;
}

VOS_UINT32 AT_QryApRptSrvUrlPara(VOS_UINT8 indexNum)
{
    VOS_UINT8              apRptSrvUrl[AT_AP_NVIM_XML_RPT_SRV_URL_LEN + 1];
    VOS_UINT32             ret;
    errno_t                memResult;
    TAF_AT_NvimApRptSrvUrl apRptSrvUrlInfo;

    (VOS_VOID)memset_s(apRptSrvUrl, sizeof(apRptSrvUrl), 0x00, sizeof(apRptSrvUrl));

    (VOS_VOID)memset_s(&apRptSrvUrlInfo, sizeof(apRptSrvUrlInfo), 0x00, sizeof(apRptSrvUrlInfo));

    /* 读NV:NV_ITEM_AP_RPT_SRV_URL */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AP_RPT_SRV_URL, &apRptSrvUrlInfo, AT_AP_NVIM_XML_RPT_SRV_URL_LEN);

    if (ret != NV_OK) {
        AT_ERR_LOG("AT_QryApRptSrvUrlPara:Read NV fail");
        return AT_ERROR;
    }
    memResult = memcpy_s(apRptSrvUrl, sizeof(apRptSrvUrl), apRptSrvUrlInfo.apRptSrvUrl, AT_AP_NVIM_XML_RPT_SRV_URL_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(apRptSrvUrl), AT_AP_NVIM_XML_RPT_SRV_URL_LEN);
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:\"%s\"", g_parseContext[indexNum].cmdElement->cmdName, apRptSrvUrl);

    return AT_OK;
}

VOS_UINT32 AT_QryApXmlInfoTypePara(VOS_UINT8 indexNum)
{
    VOS_UINT8                apXmlInfoType[AT_AP_XML_RPT_INFO_TYPE_LEN + 1];
    VOS_UINT32               ret;
    errno_t                  memResult;
    TAF_AT_NvimApXmlInfoType apXmlInfoTypeInfo;

    (VOS_VOID)memset_s(apXmlInfoType, sizeof(apXmlInfoType), 0x00, sizeof(apXmlInfoType));

    (VOS_VOID)memset_s(&apXmlInfoTypeInfo, sizeof(apXmlInfoTypeInfo), 0x00, sizeof(apXmlInfoTypeInfo));

    /* 读NV:NV_ITEM_AP_XML_INFO_TYPE */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AP_XML_INFO_TYPE, &apXmlInfoTypeInfo, AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN);

    if (ret != NV_OK) {
        AT_ERR_LOG("AT_QryApXmlInfoTypePara:Read NV fail");
        return AT_ERROR;
    }

    memResult = memcpy_s(apXmlInfoType, sizeof(apXmlInfoType), apXmlInfoTypeInfo.apXmlInfoType,
                         AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(apXmlInfoType), AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%s", g_parseContext[indexNum].cmdElement->cmdName, apXmlInfoType);

    return AT_OK;
}

VOS_UINT32 AT_QryApXmlRptFlagPara(VOS_UINT8 indexNum)
{
    VOS_UINT32             ret;
    TAF_AT_NvimApXmlRptFlg apXmlRptFlg;

    apXmlRptFlg.apXmlRptFlg = VOS_FALSE;

    /* 读NV:NV_ITEM_AP_XML_RPT_FLAG */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AP_XML_RPT_FLAG, &apXmlRptFlg, sizeof(TAF_AT_NvimApXmlRptFlg));

    if (ret != NV_OK) {
        AT_ERR_LOG("AT_QryApXmlRptFlagPara:Read NV fail");
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName,
            apXmlRptFlg.apXmlRptFlg);

    return AT_OK;
}

VOS_UINT32 AT_QryRrcStatPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryRrcStatReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RRCSTAT_QRY;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

VOS_UINT32 AT_QryHfreqInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_HFREQINFO_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryHfreqinfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型，返回命令处理挂起状态 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HFREQINFO_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryRATCombinePara(VOS_UINT8 indexNum)
{
    TAF_NV_LC_CTRL_PARA_STRU nVData;
    VOS_UINT16               length;
    VOS_UINT8                ratMode;
    ModemIdUint16            modemId;

    (VOS_VOID)memset_s(&nVData, sizeof(nVData), 0x00, sizeof(nVData));

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_QryRATCombinePara: Get modem id fail.");

        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_LC_CTRL_PARA, &nVData, sizeof(TAF_NV_LC_CTRL_PARA_STRU)) != NV_OK) {
        AT_ERR_LOG("AT_QryRATCombinePara: Get NV_ITEM_LC_CTRL_PARA id fail.");

        return AT_ERROR;
    }

    if (nVData.ucLCEnableFlg != VOS_TRUE) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    if (nVData.enRatCombined == TAF_NVIM_LC_RAT_COMBINED_CL) {
        ratMode = 0;
    } else if (nVData.enRatCombined == TAF_NVIM_LC_RAT_COMBINED_GUL) {
        ratMode = 1;
    } else {
        return AT_ERROR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ratMode);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

/* AT^TNUMCTL查询命令处理函数 */
VOS_UINT32 AT_QryTrustNumCtrlPara(VOS_UINT8 indexNum)
{
    AT_MtTrustListCtrl *mtCtrl = VOS_NULL_PTR;
    VOS_UINT8           trustBlockNumEnable = 0; /* Block名单开关（当前不支持），当前默认填0 */

    /* 获取维护特性控制开关上下文地址 */
    mtCtrl = AT_GetCommMtTrustCtrlAddr();

    /* 查询返回结果分别为：短信Trust名单开关、语音Trust名单开关、Block名单开关（当前不支持） */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        mtCtrl->mtSmsTrustListEnable, mtCtrl->mtCallTrustListEnable, trustBlockNumEnable);

    return AT_OK;
}

/* AT^TNUMCTL测试命令处理函数 */
VOS_UINT32 AT_TestTrustNumCtrlPara(VOS_UINT8 indexNum)
{
    /* Block名单功能（当前不支持） */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-1),(0-1),(0)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}

VOS_UINT32 AT_QryLogNvePara(VOS_UINT8 indexNum)
{
    VOS_UINT32                    result;
    VOS_UINT16                    length;
    VOS_UINT32                    enable = 0;
    VOS_UINT32                    timeout = 0;
    VOS_UINT32                    bufferSize = 0;
    socp_encdst_buf_log_cfg_s logCfg = {0};

    result = AT_GetSocpSdLogCfg(&logCfg);

    if (result != VOS_OK) {
        return AT_ERROR;
    }

    enable = logCfg.log_on_flag;
    /* dts方式获取的内存，不支持延时上报 */
    if ((logCfg.log_on_flag == AT_SOCP_DST_CHAN_DELAY) || (logCfg.log_on_flag == AT_SOCP_DST_CHAN_DTS)) {
        enable = AT_SOCP_DST_CHAN_DELAY;
    }
    timeout    = logCfg.cur_time_out;
    bufferSize = logCfg.buffer_size;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName, enable, timeout,
        bufferSize);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
VOS_UINT32 AT_QryMBMSCmdPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给MTA 发送查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_MBMS_AVL_SERVICE_LIST_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryMBMSCmdPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MBMS_AVL_SERVICE_LIST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestMBMSCMDPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (\"MBMS_SERVICE_ENABLER\",\"ACTIVATE\",\"DEACTIVATE\",\"DEACTIVATE_ALL\",\"MBMS_PREFERENCE\",\"SIB16_GET_NETWORK_TIME\",\"BSSI_SIGNAL_LEVEL\",\"NETWORK_INFORMATION\",\"MODEM_STATUS\")",
        g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}
#endif
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_QryRsrpCfgPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_LteRsrpCfg rsrpCfg;
    VOS_UINT32   i = 0;

    (VOS_VOID)memset_s(&rsrpCfg, sizeof(rsrpCfg), 0x00, sizeof(rsrpCfg));

    if (g_ate5RightFlag == AT_E5_RIGHT_FLAG_YES) {
        if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_RSRP_CFG, &rsrpCfg, sizeof(rsrpCfg)) != NV_OK) {
            PS_PRINTF_WARNING("read RSRP_CFG NV fail!\n");
            return AT_ERROR;
        }

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        for (i = 0; i < AT_RSRP_CFG_SS_VALUE_LEN - 1; i++) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(%d)%d,",
                i + 1, rsrpCfg.value[i]);
        }

        g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(4)%d",
            rsrpCfg.value[AT_RSRP_CFG_SS_VALUE_INDEX_3]);
    } else {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_QryRscpCfgPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_LteRscpCfg rscpCfg;
    VOS_UINT32   i = 0;

    (VOS_VOID)memset_s(&rscpCfg, sizeof(rscpCfg), 0x00, sizeof(rscpCfg));

    if (g_ate5RightFlag == AT_E5_RIGHT_FLAG_YES) {
        if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_RSCP_CFG, &rscpCfg, sizeof(rscpCfg)) != NV_OK) {
            return AT_ERROR;
        }

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        for (i = 0; i < AT_RSCP_CFG_SS_VALUE_LEN - 1; i++) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(%d)%d,",
                i + 1, rscpCfg.value[i]);
        }

        g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(4)%d",
            rscpCfg.value[AT_RSCP_CFG_SS_VALUE_INDEX_3]);
    } else {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_QryEcioCfgPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_LteEcioCfg ecioCfg;
    VOS_UINT32   i = 0;

    (VOS_VOID)memset_s(&ecioCfg, sizeof(ecioCfg), 0x00, sizeof(ecioCfg));

    if (g_ate5RightFlag == AT_E5_RIGHT_FLAG_YES) {
        if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_ECIO_CFG, &ecioCfg, sizeof(ecioCfg)) != NV_OK) {
            PS_PRINTF_WARNING("read ECIO_CFG NV fail!\n");
            return AT_ERROR;
        }

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        for (i = 0; i < AT_ECIO_CFG_SS_VALUE_LEN - 1; i++) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(%d)%d,",
                i + 1, ecioCfg.value[i]);
        }

        g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "(4)%d",
            ecioCfg.value[AT_ECIO_CFG_SS_VALUE_INDEX_3]);
    } else {
        return AT_ERROR; /* 如果权限未打开，返回ERROR */
    }

    return AT_OK;
}
#endif

VOS_UINT32 AT_QryAuthverPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取SIMLOCK MANAGER版本信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_AUTHVER_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryAuthverPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_AUTHVER_READ;
    return AT_WAIT_ASYNC_RETURN;
}

