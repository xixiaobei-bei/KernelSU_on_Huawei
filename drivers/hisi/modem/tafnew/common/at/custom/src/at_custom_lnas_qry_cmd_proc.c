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
#include "at_custom_lnas_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"

#include "css_at_interface.h"
#include "throt_at_interface.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "at_taf_agent_interface.h"
#include "at_ctx.h"
#endif

#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_LNAS_QRY_CMD_PROC_C

VOS_UINT32 AT_QryMccFreqPara(VOS_UINT8 indexNum)
{
    AT_CSS_MccVersionInfoReq *msg = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG("AT_QryMccFreqPara: Invalid Cmd Type");

        return AT_ERROR;
    }

    /* Allocating memory for message */
    /*lint -save -e516 */
    msg = (AT_CSS_MccVersionInfoReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_MccVersionInfoReq));
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_QryMccFreqPara: Mem Alloc failed");

        return AT_ERROR;
    }

    (VOS_VOID)memset_s(((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH),
                       sizeof(AT_CSS_MccVersionInfoReq) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_CSS_MccVersionInfoReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    AT_CFG_MSG_HDR(msg, PS_PID_CSS, ID_AT_CSS_MCC_VERSION_INFO_REQ);

    msg->clientId = g_atClientTab[indexNum].clientId;

    AT_SEND_MSG(msg);

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MCCFREQ_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryBlockCellListPara(VOS_UINT8 indexNum)
{
    AT_CSS_BlockCellListQueryReq *msg = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG("AT_QryBlockCellListPara: Invalid Cmd Type");

        return AT_ERROR;
    }

    /* Allocating memory for message */
    /*lint -save -e516 */
    msg = (AT_CSS_BlockCellListQueryReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_BlockCellListQueryReq));
    /*lint -restore */

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_QryBlockCellListPara: Mem Alloc failed");

        return AT_ERROR;
    }

    (VOS_VOID)memset_s(((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH),
                       sizeof(AT_CSS_BlockCellListQueryReq) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_CSS_BlockCellListQueryReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    AT_CFG_MSG_HDR(msg, PS_PID_CSS, ID_AT_CSS_BLOCK_CELL_LIST_QUERY_REQ);

    msg->clientId = g_atClientTab[indexNum].clientId;

    AT_SEND_MSG(msg);

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_BLOCKCELLLIST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_QryIsmCoexPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询ISMCOEX列表请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_LTE_WIFI_COEX_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryIsmCoexPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTE_WIFI_COEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/* 查询当前CABTSRI(backoff定时器参数上报)功能的开关状态 */
VOS_UINT32 At_QryCabtsrPara(VOS_UINT8 indexNum)
{
    AT_THROT_CabtsriQueryReq *msg     = VOS_NULL_PTR;
    VOS_UINT32                ret;
    ModemIdUint16             modemId = MODEM_ID_0;

    ret = AT_GetModemIdFromClient(indexNum, &modemId);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_QryCabtsrPara: Get modem id fail.");
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG("At_QryCabtsrPara: Invalid Cmd Type");

        return AT_ERROR;
    }

    /* 申请内存 */
    msg = (AT_THROT_CabtsriQueryReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_THROT_CabtsriQueryReq));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("At_QryCabtsrPara: Mem Alloc failed");

        return AT_ERROR;
    }

    (VOS_VOID)memset_s(((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH), sizeof(AT_THROT_CabtsriQueryReq) - VOS_MSG_HEAD_LENGTH,
        0x00, sizeof(AT_THROT_CabtsriQueryReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    AT_CFG_MSG_HDR(msg, AT_GetDestPid((MN_CLIENT_ID_T)indexNum, I0_PS_PID_NAS_COMM),
        ID_AT_THROT_CABTSRI_CONFIGURATION_QUERY_REQ);

    msg->modemId  = modemId;
    msg->clientId = g_atClientTab[indexNum].clientId;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_QryCabtsrPara: TAF_TraceAndSendMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CABTSR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/* 查询当前CSBTSRI(T3585的back-off定时器参数上报)功能的开关状态 */
VOS_UINT32 At_QryCsbtsrPara(VOS_UINT8 indexNum)
{
    AT_THROT_CsbtsriQueryReq *msg     = VOS_NULL_PTR;
    VOS_UINT32                ret;
    ModemIdUint16             modemId = MODEM_ID_0;

    ret = AT_GetModemIdFromClient(indexNum, &modemId);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 申请内存 */
    msg = (AT_THROT_CsbtsriQueryReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_THROT_CsbtsriQueryReq));
    if (msg == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH), sizeof(AT_THROT_CsbtsriQueryReq) - VOS_MSG_HEAD_LENGTH,
        0x00, sizeof(AT_THROT_CsbtsriQueryReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    AT_CFG_MSG_HDR(msg, AT_GetDestPid((MN_CLIENT_ID_T)indexNum, I0_PS_PID_NAS_COMM),
        ID_AT_THROT_CSBTSRI_CONFIGURATION_QUERY_REQ);

    msg->modemId  = modemId;
    msg->clientId = g_atClientTab[indexNum].clientId;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_QryCsbtsrPara: Send msg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSBTSR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/* 查询当前CSDBTSRI(基于S-NSSAI和DNN的Back-off定时器参数上报)功能的开关状态 */
VOS_UINT32 At_QryCsdbtsrPara(VOS_UINT8 indexNum)
{
    AT_THROT_CsdbtsriQueryReq *msg     = VOS_NULL_PTR;
    VOS_UINT32                 ret;
    ModemIdUint16              modemId = MODEM_ID_0;

    ret = AT_GetModemIdFromClient(indexNum, &modemId);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 申请内存 */
    msg = (AT_THROT_CsdbtsriQueryReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_THROT_CsdbtsriQueryReq));
    if (msg == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH), sizeof(AT_THROT_CsdbtsriQueryReq) -
        VOS_MSG_HEAD_LENGTH, 0x00, sizeof(AT_THROT_CsdbtsriQueryReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    AT_CFG_MSG_HDR(msg, AT_GetDestPid((MN_CLIENT_ID_T)indexNum, I0_PS_PID_NAS_COMM),
        ID_AT_THROT_CSDBTSRI_CONFIGURATION_QUERY_REQ);

    msg->modemId  = modemId;
    msg->clientId = g_atClientTab[indexNum].clientId;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_QryCsdbtsrPara: Send msg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSDBTSR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: 公共接口，AT模块获取当前网络制式
 * 输入参数: index: AT通道
 * 返回结果: 当前网络制式
 */
LOCAL VOS_UINT32 AT_GetCurSysMode(VOS_UINT8 indexNum)
{
    TAF_AGENT_SysMode curSysMode;

    (VOS_VOID)memset_s(&curSysMode, sizeof(curSysMode), 0x00, sizeof(curSysMode));

    /* 调用AGENT接口获取当前的网络制式 */
    if (TAF_AGENT_GetSysMode(indexNum, &curSysMode) == VOS_OK) {
        return curSysMode.ratType;
    } else {
        return TAF_PH_INFO_NONE_RAT;
    }
}

/*
 * 功能说明: 获取AT公共的上下文结构中LTE的上下行Cat的地址
 * 输入参数: 无
 * 返回结果: 上下行Cat的地址
 */
LOCAL AT_UeCategoryInfo* AT_GetUeCommRateCategoryAddr(VOS_VOID)
{
    AT_CommCtx *commCtx = AT_GetCommCtxAddr();

    return &(commCtx->lteCategoryInfo);
}

/*
 * 功能说明: 设置LTE的上下行Cat
 * 输入参数: dlCat: 下行CAT
 *           ulCat: 上行CAT
 */
VOS_VOID AT_SetLteCategoryInfo(VOS_UINT8 dlCat, VOS_UINT8 ulCat)
{
    AT_UeCategoryInfo *lteCategoryInfo = AT_GetUeCommRateCategoryAddr();

    lteCategoryInfo->lteUeDlCategory = dlCat;
    lteCategoryInfo->lteUeUlCategory = ulCat;
}

/*
 * 功能说明: AT^LTECAT查询命令处理函数
 * 输入参数: indexNum: AT下发通道
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_QryLteCatPara(VOS_UINT8 indexNum)
{
    AT_UeCategoryInfo *lteCategoryInfo = VOS_NULL_PTR;

    /* 如果当前的网络制式不为LTE，则直接返回ERROR */
    if (AT_GetCurSysMode(indexNum) != TAF_PH_INFO_LTE_RAT) {
        return AT_ERROR;
    }
    lteCategoryInfo = AT_GetUeCommRateCategoryAddr();
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR*)g_atSndCodeAddress,
        (TAF_CHAR*)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        lteCategoryInfo->lteUeDlCategory, lteCategoryInfo->lteUeUlCategory);

    return AT_OK;
}
#endif
