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
#include "at_phone_mm_qry_cmd_proc.h"
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
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_MM_QRY_CMD_PROC_C

TAF_UINT32 At_QryCfunPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QryPhoneModeReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFUN_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCopsPara(TAF_UINT8 indexNum)
{
    /* 放开CL模式下AT+COPS查询功能 */

    if (TAF_MMA_QryCopsInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_AbortCopsPara(VOS_UINT8 indexNum)
{
    /* 当前只能处理列表搜的打断, 其它命令则不进行打断操作 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_COPS_TEST) {
        /* AT向MMA发送打断列表搜的请求 */
        if (TAF_MMA_AbortPlmnListReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
            /* 停止列表搜AT的保护定时器 */
            AT_StopRelTimer(indexNum, &g_atClientTab[indexNum].hTimer);

            /* 更新当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_ABORT_PLMN_LIST;

            return AT_WAIT_ASYNC_RETURN;
        } else {
            return AT_FAILURE;
        }
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 At_TestCopsPara(TAF_UINT8 indexNum)
{
    TAF_MMA_PlmnListPara plmnListPara;

    plmnListPara.qryNum    = TAF_MMA_MAX_PLMN_NAME_LIST_NUM;
    plmnListPara.currIndex = 0;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (At_CheckCurrRatModeIsCL((VOS_UINT8)(g_atClientTab[indexNum].clientId)) == VOS_TRUE) {
        return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
    }
#endif

    if (Taf_PhonePlmnList(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &plmnListPara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_TEST;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryCgattPara(VOS_UINT8 indexNum)
{
    /* AT给MMA模块发消息，要求MMA返回CS和PS的注册状态 */
    if (TAF_MMA_AttachStatusQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                   TAF_MMA_SERVICE_DOMAIN_PS) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MMA_GET_CURRENT_ATTACH_STATUS;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_QrySysCfgPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QrySyscfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFG_READ; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QrySysCfgExPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QrySyscfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFGEX_READ; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                  /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 AT_TestSyscfg(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = TAF_MMA_TestSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFG_TEST;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_OK;
}
#endif

VOS_UINT32 AT_TestSyscfgEx(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = TAF_MMA_TestSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFGEX_TEST;

        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

TAF_UINT32 At_QryCpamPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QryAccessModeReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPAM_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_QryCgcattPara(VOS_UINT8 indexNum)
{
    /* AT给MMA模块发消息，要求MMA返回CS和PS的注册状态 */
    if (TAF_MMA_AttachStatusQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                   TAF_MMA_SERVICE_DOMAIN_CS_PS) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MMA_GET_CURRENT_ATTACH_STATUS;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

TAF_UINT32 At_QryFPlmnPara(TAF_UINT8 indexNum)
{
    if (TAF_MMA_QryFPlmnInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFPLMN_READ; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                /* 等待异步事件返回 */
    } else {
        return AT_ERROR; /* 错误返回 */
    }
}

VOS_UINT32 At_QryCplsPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            netCtx->prefPlmnType);

    return AT_OK;
}

TAF_UINT32 At_QryCpolPara(TAF_UINT8 indexNum)
{
    VOS_UINT32      ret;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    TAF_MMA_CpolInfoQueryReq cpolInfo;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /*
     * AT向MMA请求运营商信息:
     * 因为核间消息限制，不能一次获取所有运营商信息，这里定义为一次获取37条运营商信息
     * 第一条请求消息，从索引0开始要求连续的37条运营商信息
     */
    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&cpolInfo, sizeof(cpolInfo), 0x00, sizeof(cpolInfo));

    cpolInfo.prefPLMNType = netCtx->prefPlmnType;
    cpolInfo.fromIndex    = 0;

    cpolInfo.plmnNum = TAF_MMA_MAX_PLMN_NAME_LIST_NUM;

    /* 向MMA发消息请求运营商信息 */
    ret = TAF_MMA_QueryCpolReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &cpolInfo);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPOL_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCpolPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_ERROR;
    }

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    rst = TAF_MMA_TestCpolReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, netCtx->prefPlmnType);

    if (rst == VOS_TRUE) {
        /* 设置AT模块实体的状态为等待异步返回 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPOL_TEST;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_QryQuickStart(TAF_UINT8 indexNum)
{
    AT_PR_LOGI("Rcv Msg");

    if (TAF_MMA_QryQuickStartReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作模式 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CQST_READ;
        /* 等待异步事件返回 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        /* 错误返回 */
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryPlmnPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryPlmnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PLMN_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryAcInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 发消息给MMA获取当前UE的位置信息 */
    result = TAF_MMA_QryAcInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    /* 执行命令操作 */
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ACINFO_READ;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryMmPlmnInfoPara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryMMPlmnInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MMPLMNINFO_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryRejInfoPara(VOS_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_QryRejinfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_REJINFO_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestNetScanPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        "%s: (1-20),(-125--47),(0-4)",
#else
        "%s: (1-20),(-125--47),(0-2)",
#endif
        g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_AbortNetScanPara(VOS_UINT8 indexNum)
{
    /* 当前只能处理NETSCAN设置的打断, 其它命令则不进行打断操作 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_NETSCAN_SET) {
        /* AT向MMA发送打断NETSCAN的请求 */
        if (TAF_MMA_NetScanAbortReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
            /* 停止NETSCANAT的保护定时器 */
            AT_StopRelTimer(indexNum, &g_atClientTab[indexNum].hTimer);

            /* 更新当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NETSCAN_ABORT;

            return AT_WAIT_ASYNC_RETURN;
        }

        /* AT向MMA发送打断NETSCAN的请求失败，返回失败 */
        return AT_FAILURE;
    }

    /* 当前非设置命令，返回失败 */
    return AT_FAILURE;
}

VOS_UINT32 AT_QryEmRssiCfgPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_MMA_QryEmRssiCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId) ==
        VOS_TRUE) {
        /* 返回命令处理挂起状态 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EMRSSICFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_TestCesqPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-63,99),(0-7,99),(0-96,255),(0-49,255),(0-34,255),(0-97,255)",
        g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_QryEonsUcs2Para(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryEonsUcs2Req(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EONSUCS2_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryCevdpPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给MMA 发送查询请求消息 */
    rst = TAF_MMA_QryVoiceDomainReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VOICE_DOMAIN_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryCindPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* +CIND: <ind> 目前只支持signal的查询 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            netCtx->cerssiReportType);

    return AT_OK;
}

VOS_UINT32 AT_TestCindPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: \"signal\",(0-5)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

/*
 * Description: ^CRPN?
 * History:
 *  1.Date: 2009-09-01
 *    Modification: Created function
 */
TAF_UINT32 At_QryCrpnPara(TAF_UINT8 indexNum)
{
    return AT_OK;
}

VOS_UINT32 AT_QryUserSrvStatePara(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QryUserSrvStateReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_USER_SRV_STATE_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

