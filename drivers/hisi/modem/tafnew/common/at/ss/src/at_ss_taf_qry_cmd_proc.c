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
#include "at_ss_taf_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_ss_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_TAF_QRY_CMD_PROC_C

TAF_UINT32 At_QryClipPara(TAF_UINT8 indexNum)
{
    TAF_SS_InterrogatessReq para;

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置SsCode */
    para.ssCode = TAF_CLIP_SS_CODE;

    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLIP_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryClirPara(TAF_UINT8 indexNum)
{
    TAF_SS_InterrogatessReq para;

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置SsCode */
    para.ssCode = TAF_CLIR_SS_CODE;

    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == TAF_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLIR_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryColpPara(TAF_UINT8 indexNum)
{
    TAF_SS_InterrogatessReq para;

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置SsCode */
    para.ssCode = TAF_COLP_SS_CODE;

    para.opBsService = 0;

    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COLP_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCcugPara(TAF_UINT8 indexNum)
{
    TAF_UINT16     length = 0;
    TAF_UINT16     indexTemp;
    TAF_UINT16     info;
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    indexTemp = (TAF_UINT16)ssCtx->ccugCfg.index;

    info = (TAF_UINT16)((ssCtx->ccugCfg.suppressPrefer << 1) | ssCtx->ccugCfg.suppressOa);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        ssCtx->ccugCfg.enable);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", indexTemp);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", info);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 At_QryCssnPara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptQryReq atCmd;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_UnsolicitedRptQryReq));
    atCmd.reqType = AT_MTA_QRY_CSSN_RPT_TYPE;

    /* 给MTA发送^cssn查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_QRY_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptQryReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryCuus1Para(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_QRY_CLPR_REQ到C核 */
    rst = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_QRY_UUSINFO_REQ, 0, modemId);

    if (rst == MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APP_SET_UUSINFO_REQ;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

TAF_UINT32 At_QryCcwaPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ssCtx->ccwaType);
    return AT_OK;
}

TAF_UINT32 At_QryCusdPara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptQryReq atCmd;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_UnsolicitedRptQryReq));
    atCmd.reqType = AT_MTA_QRY_CUSD_RPT_TYPE;

    /* 给MTA发送+cusd查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_QRY_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptQryReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCnapExPara(VOS_UINT8 indexNum)
{
    VOS_UINT32    ret;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_CNAP_QRY_REQ到C核 */
    ret = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_CNAP_QRY_REQ, 0, modemId);

    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNAPEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 AT_QryCnapPara(VOS_UINT8 indexNum)
{
    TAF_SS_InterrogatessReq cnapQry;

    /* 初始化 */
    (VOS_VOID)memset_s(&cnapQry, (VOS_SIZE_T)sizeof(cnapQry), 0x00, (VOS_SIZE_T)sizeof(cnapQry));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置SsCode */
    cnapQry.ssCode = TAF_CNAP_SS_CODE;

    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &cnapQry) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNAP_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryAlsPara(TAF_UINT8 indexNum)
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

    /* 发送TAF_CS_ALS_LINE_NO_QRY消息给CCM，由CCM返回当前的线路号。 */
    rst = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_QRY_ALS_REQ, 0, modemId);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APP_QRY_ALS_REQ;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

TAF_UINT32 At_QryUssdModePara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 0：USSD非透传方案（即单板进行USSD字符编解码）1：USSD透传方案（即单板不进行USSD字符编解码，只是透传) */
    g_atSendDataBuff.bufLen =
        (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            ssCtx->ussdTransMode);

    return AT_OK;
}

VOS_UINT32 AT_QryCmolrPara(VOS_UINT8 indexNum)
{
    /* 执行查询操作 */
    TAF_SSA_GetCmolrInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMOLR_READ;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCmolrePara(VOS_UINT8 indexNum)
{
    AT_ModemAgpsCtx *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, agpsCtx->cmolreType);

    /* 命令返回OK */
    return AT_OK;
}

VOS_UINT32 AT_QryCmtlrPara(VOS_UINT8 indexNum)
{
    /* 执行查询操作 */
    TAF_SSA_GetCmtlrInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMTLR_READ;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCmtlraPara(VOS_UINT8 indexNum)
{
    /* 执行查询操作 */
    TAF_SSA_GetCmtlraInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMTLRA_READ;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能描述: +CCWA命令的打断处理
 *
 */
VOS_UINT32 AT_AbortCcwaPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    MN_CLIENT_OperationId clientOperationId;

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->ssAbortEnable == VOS_FALSE) {
        return AT_FAILURE;
    }

    clientOperationId.clientId = g_atClientTab[indexNum].clientId;
    clientOperationId.opId = 0;
    clientOperationId.reserve = 0;

    /* +CCWA开启/关闭/查询处理需支持可中断 */
    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CCWA_DISABLE) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CCWA_ENABLE) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CCWA_QUERY)) {
        result = MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ABORT_SS_MSG, VOS_NULL_PTR, 0, I0_WUEPS_PID_TAF);
        if (result == TAF_SUCCESS) {
            return AT_ABORT;
        }
    }
    return AT_FAILURE;
}

/*
 * 功能描述: +CLIP命令的打断处理
 *
 */
VOS_UINT32 AT_AbortClipPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    MN_CLIENT_OperationId clientOperationId;

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->ssAbortEnable == VOS_FALSE) {
        return AT_FAILURE;
    }

    clientOperationId.clientId = g_atClientTab[indexNum].clientId;
    clientOperationId.opId = 0;
    clientOperationId.reserve = 0;

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CLIP_READ) {
        result = MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ABORT_SS_MSG, VOS_NULL_PTR, 0, I0_WUEPS_PID_TAF);
        if (result == TAF_SUCCESS) {
            return AT_ABORT;
        }
    }
    return AT_FAILURE;
}

/*
 * 功能描述: +CLIR命令的打断处理
 *
 */
VOS_UINT32 AT_AbortClirPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    MN_CLIENT_OperationId clientOperationId;

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->ssAbortEnable == VOS_FALSE) {
        return AT_FAILURE;
    }

    clientOperationId.clientId = g_atClientTab[indexNum].clientId;
    clientOperationId.opId = 0;
    clientOperationId.reserve = 0;

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CLIR_READ) {
        result = MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ABORT_SS_MSG, VOS_NULL_PTR, 0, I0_WUEPS_PID_TAF);
        if (result == TAF_SUCCESS) {
            return AT_ABORT;
        }
    }
    return AT_FAILURE;
}
#endif

VOS_UINT32 At_TestChldPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1,1x,2,2x,3,4,5)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 At_TestCuus1Para(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "+CUUS1:(0,1),(0,1),(0-6),(0-4),(0-3)");

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCcwaPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCcfcPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-5)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCusdPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-2)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestCmolrPara(VOS_UINT8 indexNum)
{
    /* 输出测试命令结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (0-3),(0-6),(0,1),(0-127),(0,1),(0,1),(0-127),(0-4),"
        "(0,1),(1-65535),(1-65535),(1-127),(0,1),"
        /* "\"$GPGSA,$GPGGA,$GPGSV,$GPRMC,$GPVTG,$GPGLL\"," */
        "\"$GPGGA,$GPRMC,$GPGLL\","
        "\"<third-party-address>\"", /* <third-party-address>参数暂不确定格式 */
        g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_TestCmtlraPara(VOS_UINT8 indexNum)
{
    /* 输出测试命令结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

