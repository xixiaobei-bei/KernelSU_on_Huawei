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
#include "at_voice_taf_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_voice_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_TAF_QRY_CMD_PROC_C

TAF_UINT32 At_QryS0Para(TAF_UINT8 indexNum)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%d", ccCtx->s0TimeInfo.s0TimerLen);
    return AT_OK;
}

TAF_UINT32 At_QryCmodPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ssCtx->cModType);
    return AT_OK;
}

TAF_UINT32 At_QryCbstPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen =
        (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            ssCtx->cbstDataCfg.speed, ssCtx->cbstDataCfg.name, ssCtx->cbstDataCfg.connElem);

    return AT_OK;
}

VOS_UINT32 At_TestVtsPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "+VTS: (0-9,A-D,*,#),(1-255)");
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

/* +Vts命令的打断处理函数 */
VOS_UINT32 AT_AbortVtsPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmfPara;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(TAF_Ctrl), 0x00, sizeof(TAF_Ctrl));
    (VOS_VOID)memset_s(&dtmfPara, sizeof(TAF_CALL_DtmfParam), 0x00, sizeof(TAF_CALL_DtmfParam));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    dtmfPara.key      = (VOS_CHAR)g_atParaList[0].para[0];
    dtmfPara.onLength = AT_DTMF_STOP;

    /* DTMF定制特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->dtmfCustFlg == VOS_FALSE) {
        return AT_FAILURE;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_FAILURE;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &dtmfPara, ID_TAF_CCM_STOP_DTMF_REQ, sizeof(dtmfPara), modemId) == VOS_OK) {
        return AT_ABORT;
    }

    return AT_FAILURE;
}

/* ^DTMF命令的打断处理函数 */
VOS_UINT32 AT_AbortDtmfPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmfPara;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(TAF_Ctrl), 0x00, sizeof(TAF_Ctrl));
    (VOS_VOID)memset_s(&dtmfPara, sizeof(TAF_CALL_DtmfParam), 0x00, sizeof(TAF_CALL_DtmfParam));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    dtmfPara.key      = (VOS_CHAR)g_atParaList[1].para[0];
    dtmfPara.onLength = AT_DTMF_STOP;

    /* DTMF定制特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->dtmfCustFlg == VOS_FALSE) {
        return AT_FAILURE;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_FAILURE;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &dtmfPara, ID_TAF_CCM_STOP_DTMF_REQ, sizeof(dtmfPara), modemId) == VOS_OK) {
        return AT_ABORT;
    }

    return AT_FAILURE;
}

VOS_UINT32 AT_QryCCsErrPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        AT_GetCsCallErrCause(indexNum));

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 At_QryCstaPara(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, g_atCstaNumType);

    return AT_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
TAF_UINT32 At_QryLtecsInfo(TAF_UINT8 indexNum)
{
    if (TAF_PS_GetLteCsInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTECS_READ;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryClccPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                  ret;
    TAF_Ctrl                    ctrl;
    TAF_CALL_QryCallInfoReqPara qryCallInfoPara;
    ModemIdUint16               modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&qryCallInfoPara, sizeof(qryCallInfoPara), 0x00, sizeof(qryCallInfoPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    qryCallInfoPara.getCallInfoType = TAF_CALL_GET_CALL_INFO_TYPE_CLCC;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发消息到C核获取当前所有通话信息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, &qryCallInfoPara, ID_TAF_CCM_QRY_CALL_INFO_REQ, sizeof(qryCallInfoPara),
                                modemId);

    if (ret != VOS_OK) {
        AT_WARN_LOG("AT_QryClccPara: MN_CALL_GetCallInfos fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCC_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryClccEconfInfo(VOS_UINT8 indexNum)
{
    TAF_Ctrl      ctrl;
    VOS_UINT32    rst;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_Ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送增强型多方通话查询消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_QRY_ECONF_CALLED_INFO_REQ, 0, modemId);

    if (rst != VOS_OK) {
        AT_WARN_LOG("AT_QryClccEconfInfo: TAF_CCM_CallCommonReq fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCCECONF_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryEconfErrPara(VOS_UINT8 indexNum)
{
    VOS_UINT16     i;
    VOS_UINT16     length = 0;
    AT_ModemCcCtx *ccCtx     = VOS_NULL_PTR;
    AT_EconfInfo  *econfInfo = VOS_NULL_PTR;
    VOS_UINT8      asciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];
    VOS_UINT8      numOfCalls;
    VOS_UINT8      errNum = 0;

    ccCtx      = AT_GetModemCcCtxAddrFromClientId(indexNum);
    econfInfo  = &(ccCtx->econfInfo);
    numOfCalls = econfInfo->numOfCalls;
    (VOS_VOID)memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));

    for (i = 0; ((i < numOfCalls) && (i < TAF_CALL_MAX_ECONF_CALLED_NUM)); i++) {
        /* 查询错误原因值 */
        if ((econfInfo->callInfo[i].callNumber.numLen != 0) && (econfInfo->callInfo[i].cause != TAF_CS_CAUSE_SUCCESS)) {
            /* <CR><LF> */
            if (errNum != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            AT_BcdNumberToAscii(econfInfo->callInfo[i].callNumber.bcdNum,
                AT_MIN(econfInfo->callInfo[i].callNumber.numLen, MN_CALL_MAX_CALLED_BCD_NUM_LEN),
                (VOS_CHAR *)asciiNum);

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"%s\",%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, asciiNum,
                (econfInfo->callInfo[i].callNumber.numType | AT_NUMBER_TYPE_EXT), econfInfo->callInfo[i].cause);

            errNum++;
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryCcwaiPara(VOS_UINT8 indexNum)
{
    TAF_CALL_NvimCcwaCtrlMode ccwaCtrlMode;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ccwaCtrlMode, sizeof(ccwaCtrlMode), 0x00, sizeof(ccwaCtrlMode));

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_QryCcwaiPara: Get modem id fail.");
        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_CCWA_CTRL_MODE, &ccwaCtrlMode, sizeof(TAF_CALL_NvimCcwaCtrlMode)) != NV_OK) {
        AT_ERR_LOG("AT_QryCcwaiPara: Get NV_ITEM_CCWA_CTRL_MODE id fail.");
        return AT_ERROR;
    }

    if (ccwaCtrlMode.ccwaiMode >= CCWAI_MODE_BUTT) {
        AT_ERR_LOG("AT_QryCcwaiPara: ccwaiMode error.");
        return AT_ERROR;
    }
    if (ccwaCtrlMode.ccwaCtrlMode >= CCWA_CTRL_MODE_BUTT) {
        AT_ERR_LOG("AT_QryCcwaiPara: ccwaCtrlMode error.");
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ccwaCtrlMode.ccwaiMode, ccwaCtrlMode.ccwaCtrlMode);

    return AT_OK;
}
#endif

VOS_UINT32 AT_TestRejCallPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryCsChannelInfoPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl      ctrl;
    VOS_UINT32    rst;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_QRY_CHANNEL_INFO_REQ, 0, modemId);

    if (rst != VOS_OK) {
        AT_WARN_LOG("AT_QryCsChannelInfoPara: Send Msg fail!");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSCHANNELINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryEmcCallBack(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ret = TAF_MMA_QryCurrEmcCallBackMode(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EMCCBM_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCvhuPara(VOS_UINT8 indexNum)
{
    ModemIdUint16  modemId;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    VOS_UINT16     length;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取ModemID */
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_LOG1("AT_QryCvhuPara AT_GetModemIdFromClient fail", indexNum);
        return AT_ERROR;
    }

    /* 获取CC上下文 */
    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ccCtx->cvhuMode);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_TestEclstartPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1),(0,1,2),(0,1)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}

VOS_UINT32 AT_QryEclcfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 APP_VC_MSG_QRY_ECALL_CFG_REQ, VOS_NULL, 0, I0_WUEPS_PID_VC);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryEclmsdPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, APP_VC_MSG_QRY_MSD_REQ,
                                 VOS_NULL, 0, I0_WUEPS_PID_VC);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLMSD_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestEclmsdPara(VOS_UINT8 indexNum)
{
    /*
     * ^eclmsd不支持测试命令，
     * 该函数可以在eclmsd测试命令时返回ERROR，而不是"(MSD)"
     */
    return AT_ERROR;
}

VOS_VOID AT_EcallAlAckListDisplay(VOS_VOID)
{
    VOS_UINT32          i                  = 0;
    VOS_UINT16          length             = 0;
    VOS_UINT32          readNum            = 0;
    AT_ECALL_AlackInfo *ecallAlackInfoAddr = VOS_NULL_PTR;

    ecallAlackInfoAddr = AT_EcallAlAckInfoAddr();

    /* 显示所有ALACK 值和接受时间 */
    for (i = 0; i < (VOS_UINT32)AT_MIN(ecallAlackInfoAddr->ecallAlackNum, AT_ECALL_ALACK_NUM); i++) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "^ECLLIST: ");

        readNum = (ecallAlackInfoAddr->ecallAlackBeginNum + i) % AT_ECALL_ALACK_NUM;
        AT_EcallAlackDisplay(ecallAlackInfoAddr->ecallAlackInfo[readNum], &length);

        if (i + 1 < ecallAlackInfoAddr->ecallAlackNum) {
            /* 回车换行 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_QryEclListPara(VOS_UINT8 indexNum)
{
    AT_EcallAlAckListDisplay();

    return AT_OK;
}

VOS_UINT32 AT_QryEclModePara(VOS_UINT8 indexNum)
{
    VOS_UINT16              length;
    TAF_NVIM_CustomEcallCfg ecallCfg;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&ecallCfg, sizeof(ecallCfg), 0x00, sizeof(ecallCfg));
    /* 先读取NV的值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, &ecallCfg, sizeof(ecallCfg));

    /* NV读取失败或NV未激活时，不改变模式 */
    if (result != NV_OK) {
        AT_ERR_LOG("AT_QryEclModePara: NV read error!");
    } else {
        AT_SetEclModeValue(ecallCfg.ecallForceMode);
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, AT_GetEclModeValue());
    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_TestEclModePara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName,
        g_parseContext[indexNum].cmdElement->param);
    return AT_OK;
}
#endif

/* AT^TRUSTCNUM查询命令处理函数 */
VOS_UINT32 AT_QryTrustCallNumPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_MtCallNumTrustList callTrustList;
    VOS_UINT32                  result;
    VOS_UINT16                  length = 0;
    VOS_UINT8                   count  = 0;
    VOS_UINT8                   listIndex;

    /* 从NV中读取语音Trust名单列表 */
    (VOS_VOID)memset_s(&callTrustList, sizeof(callTrustList), 0, sizeof(callTrustList));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MT_CALL_NUM_TRUST_LIST, &callTrustList, sizeof(callTrustList));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_QryTrustCallNumPara: Read NV Fail.");
        return AT_ERROR;
    }

    /* 输出有效Trust名单号码 */
    for (listIndex = 0; listIndex < TAF_TRUSTLIST_MAX_LIST_SIZE; listIndex++) {
        if (VOS_StrLen((VOS_CHAR *)callTrustList.recordList[listIndex].number) != 0) {
            if (count != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,\"%s\"",
                g_parseContext[indexNum].cmdElement->cmdName,
                callTrustList.recordList[listIndex].index, callTrustList.recordList[listIndex].number);
            count++;
        }
    }
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

/* AT^TRUSTCNUM测试命令处理函数 */
VOS_UINT32 AT_TestCallTrustNumPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-19)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_TestCpasPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: (0-5)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_TestClprPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-7)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

