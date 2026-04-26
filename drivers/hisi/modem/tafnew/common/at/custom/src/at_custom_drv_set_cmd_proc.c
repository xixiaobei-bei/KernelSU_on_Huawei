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
#include "at_custom_drv_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_ctx.h"
#include "at_input_proc.h"
#include "at_mdrv_interface.h"
#include "at_taf_agent_interface.h"
#include "at_custom_comm.h"
#include "at_mta_interface.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "taf_mta_drv_agent_msg_proc.h"
#include "at_ctx.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_DRV_SET_CMD_PROC_C

#define AT_LOGPORT_PARA_MIN_NUM 0
#define AT_LOGPORT_PARA_MAX_NUM 2
#define AT_NV_CARRIER_PARA_MIN_NUM 1
#define AT_NV_CARRIER_PARA_MAX_NUM 4
#define AT_NV_CARRIER_PARA_NUM 2
#define AT_NV_CARRIER_ENABLE 0
#define AT_NV_CARRIER_NAME 1
#define AT_NV_CARRIER_EFFECTIVEWAY 2
#define AT_NV_CARRIER_NOFLUSH 3
#define AT_NVCHK_PARA_MAX_NUM 1
#define AT_NVCHK_PARA_PARA0 0       /* NVCHK的第一个参数PARA0 */
#define AT_NVCHK_CRC_MAX_NUM 3
#define NUM_OF_CHAR_IN_UINT 4
#define BIT_LEN_8_BITS 8

#if (FEATURE_MBB_CUST == FEATURE_ON)
#define AT_WAKEUP_CFG_NUM_MIN 1
#define AT_WAKEUP_CFG_NUM_MID 2
#define AT_WAKEUP_CFG_NUM_MAX 3
#endif

VOS_UINT32 At_SetUsbSwitchPara(VOS_UINT8 indexNum)
{
    VOS_INT   iResult;
    VOS_UINT8 usbStatus;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* para too many */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usbStatus = USB_SWITCH_OFF;

    if (DRV_USB_PHY_SWITCH_GET(&usbStatus) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置Modem的USB启动 */
    if (g_atParaList[0].paraValue == AT_USB_SWITCH_SET_VBUS_VALID) {
        if (usbStatus != USB_SWITCH_ON) {
            iResult = DRV_USB_PHY_SWITCH_SET(USB_SWITCH_ON);
        } else {
            return AT_OK;
        }
    } else if (g_atParaList[0].paraValue == AT_USB_SWITCH_SET_VBUS_INVALID) {
        if (usbStatus != USB_SWITCH_OFF) {
            iResult = DRV_USB_PHY_SWITCH_SET(USB_SWITCH_OFF);
        } else {
            return AT_OK;
        }
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (iResult == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryAntState(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT16 antState;

    AT_PR_LOGI("Rcv Msg");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    antState = 0;

    ret = TAF_AGENT_GetAntState(indexNum, &antState);
    AT_PR_LOGI("Call interface success!");
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_QryAntState: TAF_AGENT_GetAntState fail");

        return AT_ERROR;
    }

    /* 根据接口约定，天线状态非0时，直接返回1 */
#ifndef FEATURE_ANTEN_DETECT
    if ((antState & AT_CMD_MAX_ANT_BIT_MASK) == 0)
#else
    if (antState == 0)
#endif
    {
        AT_NORM_LOG("AT_QryAntState: usAntState Set to 0");
        antState = 0;
    } else {
        AT_NORM_LOG("AT_QryAntState: usAntState Set to 1");
        antState = 1;
    }

    /* AT输出 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, antState);
    return AT_OK;
}

VOS_UINT32 At_SetLogPortPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt;
    VOS_UINT32 omLogPort;  // 准备切换的LogPort

    AT_PR_LOGI("Rcv Msg");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多或没有 */
    if ((g_atParaIndex > AT_LOGPORT_PARA_MAX_NUM) || (g_atParaIndex == AT_LOGPORT_PARA_MIN_NUM)) {
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_LOG_PORT_USB) {
        omLogPort = CPM_OM_PORT_TYPE_USB;
    } else {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        if (g_atParaList[0].paraValue == AT_LOG_PORT_VCOM) {
            omLogPort = CPM_OM_PORT_TYPE_VCOM;
        } else if (g_atParaList[0].paraValue == AT_LOG_PORT_WIFI) {
            omLogPort = CPM_OM_PORT_TYPE_WIFI;
        } else {
            return AT_ERROR;
        }
#else
        omLogPort = CPM_OM_PORT_TYPE_VCOM;
#endif
    }

    /* 参数只有一个，默认永久生效 */
    if (g_atParaIndex == 1) {
        g_atParaList[1].paraValue = VOS_TRUE;
    }

    /* 调用OM的接口 */
    rslt = AT_SwitchDiagLogPort(omLogPort, g_atParaList[1].paraValue);

    AT_PR_LOGI("Call interface success!");

    if (rslt == VOS_OK) {
        return AT_OK;
    } else if (rslt == ERR_MSP_AT_CHANNEL_BUSY) {
        return AT_CME_USB_TO_VCOM_IN_CONN_ERROR;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetDieSNPara(VOS_UINT8 indexNum)
{
    VOS_UINT8  dieSN[AT_DIE_SN_LEN];
    VOS_INT32  i;
    VOS_UINT16 length;

    AT_PR_LOGI("Rcv Msg");

    (VOS_VOID)memset_s(dieSN, sizeof(dieSN), 0x00, sizeof(dieSN));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        AT_WARN_LOG("At_SetDieSNPara: ucCmdOptType error!");

        return AT_ERROR;
    }

    /* 获取DIE SN */
    if (AT_GetEfuseDieid(dieSN, AT_DIE_SN_LEN) != VOS_OK) {
        AT_WARN_LOG("At_SetDieSNPara: Get Die Sn Failed!");

        return AT_ERROR;
    }

    /* 最高位,高4 BIT置0 */
    dieSN[AT_DIE_SN_LEN - 1] = (dieSN[AT_DIE_SN_LEN - 1] & 0x0F);

    /* 格式化输出查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: 0x", g_parseContext[indexNum].cmdElement->cmdName);

    for (i = (AT_DIE_SN_LEN - 1); i >= 0; i--) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", dieSN[i]);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

/*
 * 功能描述: (AT^CFG)AP设置CFG信息
 */
VOS_UINT32 AT_SetLogCfgParaEx(VOS_UINT32 paraValue)
{
    VOS_UINT32 ret;
    if (paraValue == 0) {
        ret = (VOS_UINT32)AT_SetSocpCfgIndMode(SOCP_IND_MODE_DIRECT);
        if (ret != VOS_OK) {
            PS_PRINTF_WARNING("[socp]at^logcfg=0 can not config!\n");
            return AT_DIAG_VCOM_SET_CFG_ERROR;
        }
    } else if (paraValue == 1) {
        ret = (VOS_UINT32)AT_SetSocpCfgIndMode(SOCP_IND_MODE_DELAY);
        if (ret != VOS_OK) {
            PS_PRINTF_WARNING("[socp]at^logcfg=1 can not config!\n");
            return AT_DIAG_VCOM_SET_CFG_ERROR;
        }

    } else {
        ret = (VOS_UINT32)AT_SetSocpCfgIndMode(SOCP_IND_MODE_CYCLE);
        if (ret != VOS_OK) {
            PS_PRINTF_WARNING("[socp]at^logcfg=2 can not config!\n");
            return AT_DIAG_VCOM_SET_CFG_ERROR;
        }
    }
    return AT_OK;
}

VOS_UINT32 AT_SetLogCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 logPort = CBP_OM_PORT_TYPE_BUTT;
    VOS_UINT32 rslt;
    VOS_UINT32 ret;
    /* add print by cuijunqiang, for socp timer set, 2016.01.21 */

    PS_PRINTF_INFO("at^logcfg= %d, param %d.\n", g_atParaList[0].paraValue, indexNum);

    if ((g_atParaList[0].paraValue != SOCP_IND_MODE_DIRECT) && (g_atParaList[0].paraValue != SOCP_IND_MODE_DELAY) &&
        (g_atParaList[0].paraValue != SOCP_IND_MODE_CYCLE)) {
        PS_PRINTF_WARNING("<AT_SetLogCfgPara> input param error:%d!\n", g_atParaList[0].paraValue);
        return AT_DIAG_CFG_SET_ERROR;
    }
    rslt = AT_QueryPpmLogPort(&logPort);
    if (rslt != VOS_OK) {
        PS_PRINTF_WARNING("get port fail!\n");
        return AT_DIAG_GET_PORT_ERROR;
    }
    if (logPort == CPM_OM_PORT_TYPE_USB) {
        if (g_atParaList[0].paraValue == SOCP_IND_MODE_DIRECT) {
            PS_PRINTF_WARNING("[socp]usb at^logcfg=0!\n");
            return AT_OK;
        } else {
            PS_PRINTF_WARNING("[socp]usb not support at^logcps=%d!\n", g_atParaList[0].paraValue);
            return AT_DIAG_USB_NOT_SUPPORT_CFG;
        }
    } else if (logPort == CPM_OM_PORT_TYPE_VCOM) {
        ret = AT_SetLogCfgParaEx(g_atParaList[0].paraValue);
        return ret;

    } else {
        PS_PRINTF_WARNING("at^logcfg get port error!\n");
        return AT_DIAG_GET_PORT_NOT_USB_OR_VCOM;
    }
}

/*
 * 功能描述: enLogPort-端口，ulParaValue-模式
 */
VOS_UINT32 AT_SetLogCpsParaEx(VOS_UINT32 logPort, VOS_UINT32 paraValue)
{
    VOS_UINT32 ret;
    if (logPort == CPM_OM_PORT_TYPE_USB) {
        if (paraValue == 0) {
            PS_PRINTF_WARNING("[deflate]usb at^logcps=0!\n");
            return AT_OK;
        } else {
            PS_PRINTF_WARNING("[deflate]usb not support at^logcps=%d!\n", paraValue);
            return AT_DIAG_USB_NOT_SUPPORT_CPS;
        }
    } else if (logPort == CPM_OM_PORT_TYPE_VCOM) {
        if (paraValue == 0) {
            ret = (VOS_UINT32)AT_SetSocpCpsIndMode(DEFLATE_IND_NO_COMPRESS);
            if (ret != VOS_OK) {
                PS_PRINTF_WARNING("[deflate]vcom at^logcps=0 can not config!\n");
                return AT_DIAG_VCOM_SET_CPS_ERROR;
            }
            return AT_OK;
        } else {
            ret = (VOS_UINT32)AT_SetSocpCpsIndMode(DEFLATE_IND_COMPRESS);
            if (ret != VOS_OK) {
                PS_PRINTF_WARNING("[deflate]vcom at^logcps=1 can not config!\n");
                return AT_DIAG_VCOM_SET_NOCPS_ERROR;
            }
            return AT_OK;
        }
    } else {
        PS_PRINTF_WARNING("[deflate]vcom at^logcps=1 can not config!\n");
        return AT_ERROR;
    }
}
/*
 * 功能描述: (AT^LOGCPS)设置压缩模式
 */
VOS_UINT32 AT_SetLogCpsPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt;
    VOS_UINT32 logPort = CBP_OM_PORT_TYPE_BUTT;
    VOS_UINT32 ret;

    PS_PRINTF_INFO("at^logcps=%d!\n", g_atParaList[0].paraValue);
    if ((g_atParaList[0].paraValue != DEFLATE_IND_NO_COMPRESS) && (g_atParaList[0].paraValue != DEFLATE_IND_COMPRESS)) {
        PS_PRINTF_WARNING("<AT_SetLogCpsPara> input param error:%d!\n", g_atParaList[0].paraValue);
        return AT_DIAG_CPS_SET_ERROR;
    }
    rslt = AT_QueryPpmLogPort(&logPort);
    if (rslt != VOS_OK) {
        PS_PRINTF_WARNING("[deflate]get port fail!\n");
        return AT_DIAG_GET_PORT_ERROR;
    }
    ret = AT_SetLogCpsParaEx(logPort, g_atParaList[0].paraValue);
    return ret;
}

VOS_UINT32 AT_WriteNvAndSaveNvParaProc(VOS_UINT8 indexNum) {
    AT_ModemNvloadCtx                *nvloadCarrierCtx = VOS_NULL_PTR;
    TAF_NVIM_UsimDependentResultInfo  activeNvloadInfo;
    VOS_UINT32                        nvWriteResult;
    errno_t                           memResult;
    VOS_UINT16                        length;
    ModemIdUint16                     modemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_WriteNvAndSaveNvParaProc: Get modem id fail.");
        return AT_ERROR;
    }

    nvloadCarrierCtx = AT_GetModemNvloadCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(&activeNvloadInfo, sizeof(activeNvloadInfo), 0, sizeof(activeNvloadInfo));

    length = g_atParaList[AT_NV_CARRIER_NAME].paraLen;
    if (length > AT_SET_NV_CARRIER_NAME_LEN) {
        AT_WARN_LOG("AT_WriteNvAndSaveNvParaProc: CARRIERNAME Length Error");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(length != 0) {
        memResult = memcpy_s(activeNvloadInfo.operName, sizeof(activeNvloadInfo.operName),
            g_atParaList[AT_NV_CARRIER_NAME].para, length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(activeNvloadInfo.operName), length);
    }

    if(g_atParaIndex > AT_NV_CARRIER_PARA_NUM) {
        if(g_atParaList[AT_NV_CARRIER_EFFECTIVEWAY].paraLen != 0) {
            activeNvloadInfo.effectiveWay = (VOS_UINT8)g_atParaList[AT_NV_CARRIER_EFFECTIVEWAY].paraValue;
        }
    }

    if((VOS_StrNiCmp((VOS_CHAR *)nvloadCarrierCtx->carrierName, (VOS_CHAR *)activeNvloadInfo.operName,
            AT_SET_NV_CARRIER_NAME_LEN) != 0) ||
       (nvloadCarrierCtx->specialEffectiveFlg != activeNvloadInfo.effectiveWay)) {
        /* 写NV项 */
        nvWriteResult = TAF_ACORE_NV_WRITE(modemId, NV_ITEM_USIM_DEPENDENT_RESULT_INFO, (VOS_UINT8 *)&activeNvloadInfo,
            sizeof(TAF_NVIM_UsimDependentResultInfo));
        if (nvWriteResult != NV_OK) {
            AT_ERR_LOG("AT_WriteNvAndSaveNvParaProc: NV_ITEM_USIM_DEPENDENT_RESULT_INFO NV Write Error.");
            return AT_ERROR;
        } else {
            AT_NORM_LOG("AT_WriteNvAndSaveNvParaProc: NV_ITEM_USIM_DEPENDENT_RESULT_INFO NV Write OK");

            /* 保存到全局变量中 */
            memResult = memcpy_s(nvloadCarrierCtx->carrierName, sizeof(nvloadCarrierCtx->carrierName),
                activeNvloadInfo.operName, sizeof(activeNvloadInfo.operName));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(nvloadCarrierCtx->carrierName), sizeof(activeNvloadInfo.operName));
            nvloadCarrierCtx->specialEffectiveFlg = activeNvloadInfo.effectiveWay;
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetNvLoadPara(VOS_UINT8 indexNum)
{
    AT_MTA_NvloadSetReq atCmd;
    VOS_UINT32          result;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.noFlushFlg = VOS_FALSE;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为1-4个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex < AT_NV_CARRIER_PARA_MIN_NUM || g_atParaIndex > AT_NV_CARRIER_PARA_MAX_NUM) {
        AT_WARN_LOG("AT_SetNvLoadPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度检查 */
    if (g_atParaList[AT_NV_CARRIER_ENABLE].paraLen == 0) {
        AT_WARN_LOG("AT_SetNvLoadPara: ENABLE Length Error");
        return AT_CME_INCORRECT_PARAMETERS;
    }
    atCmd.reqType = (VOS_UINT8)g_atParaList[AT_NV_CARRIER_ENABLE].paraValue;

    /* 参数个数大于1时，写NV，并将参数1，2保存到全局变量 */
    if (g_atParaIndex > AT_NV_CARRIER_PARA_MIN_NUM) {
       result = AT_WriteNvAndSaveNvParaProc(indexNum);
        if (result != AT_SUCCESS){
            return result;
        }
    }

    if (g_atParaIndex == AT_NV_CARRIER_PARA_MAX_NUM) {
        if (g_atParaList[AT_NV_CARRIER_NOFLUSH].paraLen != 0) {
            atCmd.noFlushFlg = (VOS_UINT8)g_atParaList[AT_NV_CARRIER_NOFLUSH].paraValue;
        }
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NVLOAD_SET_REQ, (VOS_UINT8 *)&atCmd,
        sizeof(AT_MTA_NvloadSetReq), I0_UEPS_PID_MTA);

    if (result != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetNvLoadPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NVLOAD_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetNVCHKPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 loopIndex;

    AT_PR_LOGI("Rcv Msg");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_NVCHK_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 未带参数判断 */
    if (g_atParaList[AT_NVCHK_PARA_PARA0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果是全部检查，则循环检查全部CRC */
    if (g_atParaList[AT_NVCHK_PARA_PARA0].paraValue == 0) {
        for (loopIndex = 0; loopIndex < AT_NVCHK_CRC_MAX_NUM; loopIndex++) {
            if (AT_CheckFactoryNv(loopIndex) != 0) {
                AT_PR_LOGI("Call interface success!");
                return AT_ERROR;
            }
        }

        return AT_OK;
    }

    /* 如果返回0，为正常，非0为异常 */
    if (AT_CheckFactoryNv(g_atParaList[AT_NVCHK_PARA_PARA0].paraValue - 1) == 0) {
        AT_PR_LOGI("Call interface success!");
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetSdrebootCmd(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查: 第一个参数用户输入密码必需存在 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_ERROR;
    }

    /* 密码不对返回AT_ERROR */
    if (VOS_StrCmp((VOS_CHAR *)g_ate5DissdPwd, (VOS_CHAR *)g_atParaList[0].para) != 0) {
        (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x00, sizeof(g_atParaList[0].para));
        return AT_ERROR;
    }

    /* 密码一致则 发消息到C核REBOOT 单板 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, DRV_AGENT_SDREBOOT_REQ,
                                 VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetSdrebootCmd: AT_FillAndSndAppReqMsg fail.");
        (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x00, sizeof(g_atParaList[0].para));
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x00, sizeof(g_atParaList[0].para));
    return AT_OK;
}

TAF_UINT32 At_SetYjcxPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_YJCX_SET_REQ, VOS_NULL_PTR, 0,
                                 I0_WUEPS_PID_DRV_AGENT);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_YJCX_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif


#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能描述: 生成指定字节长度的随机数
 */
VOS_UINT32 AT_RandomBytes(VOS_UINT8 *buf, VOS_UINT16 len)
{
    VOS_UINT32 loop;
    VOS_UINT32 interIndex;
    VOS_UINT32 bufTemp = 0;
    VOS_UINT32 loopTimes;

    loopTimes = len / NUM_OF_CHAR_IN_UINT;
    loopTimes = ((len % NUM_OF_CHAR_IN_UINT) == 0) ? loopTimes : (loopTimes + 1);

    for (loop = 0; loop < loopTimes; ++loop) {
        if (VOS_CryptRand(&bufTemp, 0xffffffff) != VOS_OK) {
            AT_WARN_LOG("AT_RandomBytes: VOS_CryptRand fail.");
            return VOS_ERR;
        }

        /* 拆分为u8的数 */
        for (interIndex = 0; (interIndex < NUM_OF_CHAR_IN_UINT) &&
            ((loop * NUM_OF_CHAR_IN_UINT + interIndex) < len); interIndex++) {
            *buf = bufTemp & 0xff;
            buf++;
            bufTemp >>= BIT_LEN_8_BITS;
        }

        bufTemp = 0;
    }

    return VOS_OK;
}

/*
 * 功能描述  : 单板会使用鉴权加密公钥对128bits的随机数进行加密，生成256字节的密文作为输出。
 *             装备工具使用此密文向服务器请求下一步鉴权完成命令时的输入数据
 */
VOS_UINT32 At_CheckMbbAuthority(VOS_UINT8 indexNum)
{
    VOS_UINT32 i;
    VOS_UINT32 result;
    VOS_UINT32 rsaLen;
    VOS_UINT16 length = 0;
    VOS_UINT8 randomData[DRV_AGENT_NETDOG_AUTH_RANDOM_LEN] = {0};
    VOS_UINT8 rsa[DRV_AGENT_RSA2048_ENCRYPT_LEN] = {0};

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取随机数 */
    result = AT_RandomBytes(randomData, DRV_AGENT_NETDOG_AUTH_RANDOM_LEN * sizeof(VOS_UINT8));
    if (result != VOS_OK) {
        g_atSendDataBuff.bufLen = length;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    /* 随机数加密动作 */
    result = sec_auth_start(rsa, &rsaLen, randomData, DRV_AGENT_NETDOG_AUTH_RANDOM_LEN);
    if (result == VOS_OK) {
        /* 输出设置结果 */
        result = AT_OK;
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
            (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress,
            "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        for (i = 0; i < DRV_AGENT_RSA2048_ENCRYPT_LEN; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", rsa[i]);
        }
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能描述 : 向设备输入鉴权签名私钥加密后的数据, 设备对输入的数据使用鉴权签名公钥解密
 *            并与鉴权开始时产生的128bit的随机数对比, 校验鉴权成功失败
 */
VOS_UINT32 At_SetMbbConfirmAuthority(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    VOS_UINT16 length;

    /* 设置参数只有一个，大于一个返回错误 */
    if (g_atParaIndex != 1) {
        return AT_TOO_MANY_PARA;
    }

    length = g_atParaList[0].paraLen;
    /* 装备下发下来的是字符串，512个字节 */
    if (length != (DRV_AGENT_RSA2048_ENCRYPT_LEN * 2)) {
        AT_WARN_LOG("At_SetMbbConfirmAuthority: input para len fail\n.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串参数转换为码流 */
    result = At_AsciiNum2HexString(g_atParaList[0].para, &length);
    if ((result != AT_SUCCESS) || (length != DRV_AGENT_RSA2048_ENCRYPT_LEN)) {
        return AT_DEVICE_OTHER_ERROR;
    }

    result = sec_auth_confirm(g_atParaList[0].para, DRV_AGENT_RSA2048_ENCRYPT_LEN);
    g_atSendDataBuff.bufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    if (result == VOS_OK) {
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

LOCAL VOS_UINT32 AT_wakeUpCfgDataUpdate(TAF_NVIM_MbbWakeupCfg *wakeUpCfg)
{
    if (wakeUpCfg == VOS_NULL_PTR) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex < AT_WAKEUP_CFG_NUM_MIN) || (g_atParaIndex > AT_WAKEUP_CFG_NUM_MAX)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == AT_WAKEUP_CFG_NUM_MIN) {
        if (g_atParaList[0].paraLen != 1) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        wakeUpCfg->remoteWakeupCtrl = (VOS_UINT8)g_atParaList[0].paraValue;
    } else if (g_atParaIndex == AT_WAKEUP_CFG_NUM_MID) {
        if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen != 1)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        wakeUpCfg->remoteWakeupCtrl = (VOS_UINT8)g_atParaList[0].paraValue;
        wakeUpCfg->remoteWakeupChannel = (VOS_UINT8)g_atParaList[1].paraValue;
    } else if (g_atParaIndex == AT_WAKEUP_CFG_NUM_MAX) {
        if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen != 1) || (g_atParaList[2].paraLen > 2)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        wakeUpCfg->remoteWakeupCtrl = (VOS_UINT8)g_atParaList[0].paraValue;
        wakeUpCfg->remoteWakeupChannel = (VOS_UINT8)g_atParaList[1].paraValue;
        wakeUpCfg->remoteWakeupSource = (VOS_UINT16)g_atParaList[2].paraValue;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_MbbSetWakeUpCfgPara(TAF_UINT8 indexNum)
{
    TAF_NVIM_MbbWakeupCfg wakeupCfg = {0};

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->powerManagerSwitch == VOS_FALSE) {
        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAKEUP_CFG, &wakeupCfg, sizeof(TAF_NVIM_MbbWakeupCfg)) != NV_OK) {
        AT_WARN_LOG("AT_MbbSetWakeUpCfgPara: nv read fail.");
        return AT_ERROR;
    }

    if (AT_wakeUpCfgDataUpdate(&wakeupCfg) != AT_SUCCESS) {
        return AT_ERROR;
    }

    if (((wakeupCfg.remoteWakeupCtrl != REMOTE_WAKEUP_ON) && (wakeupCfg.remoteWakeupCtrl != REMOTE_WAKEUP_OFF)) ||
        (wakeupCfg.remoteWakeupChannel > REMOTE_WAKEUP_CHN_MAX) ||
        (wakeupCfg.remoteWakeupSource > REMOTE_WAKEUP_SRC_MAX)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_WAKEUP_CFG, (VOS_UINT8 *)&wakeupCfg, sizeof(wakeupCfg)) != NV_OK) {
        AT_WARN_LOG("AT_MbbSetWakeUpCfgPara: Fail to write NV 2123.");
        return AT_ERROR;
    }

    At_FormatResultData(indexNum, AT_OK);

    return AT_SUCCESS;
}
#endif


