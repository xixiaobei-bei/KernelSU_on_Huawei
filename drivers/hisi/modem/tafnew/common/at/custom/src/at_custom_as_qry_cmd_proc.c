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
#include "at_custom_as_qry_cmd_proc.h"
#include "securec.h"
#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "at_mdrv_interface.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_AS_QRY_CMD_PROC_C

VOS_UINT32 AT_QrySubClfsParamPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询云通信版本请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_SUBCLFSPARAM_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QrySubClfsParamPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SUBCLFSPARAM_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryNrCaCellPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询NRCACELL信息 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_NR_CA_CELL_INFO_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNrCaCellPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRCACELL_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryNrCaCellRptCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NR_CA_CELL_INFO_RPT_CFG_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);
    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNrCaCellRptCfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRCACELLRPTCFG_QRY;
    return AT_WAIT_ASYNC_RETURN;
}
#endif
#endif

VOS_UINT32 AT_QryRsrp(VOS_UINT8 indexNum)
{
    VOS_UINT32          ret;
    AT_MTA_RsInfoQryReq rsInfoQryReq;

    rsInfoQryReq.rsInfoType = AT_MTA_RSRP_TYPE;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_RS_INFO_QRY_REQ,
                                 (VOS_UINT8 *)&rsInfoQryReq, (VOS_UINT32)sizeof(AT_MTA_RsInfoQryReq), I0_UEPS_PID_MTA);
    if (ret != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RSRP_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryRsrq(VOS_UINT8 indexNum)
{
    VOS_UINT32          ret;
    AT_MTA_RsInfoQryReq rsInfoQryReq;

    rsInfoQryReq.rsInfoType = AT_MTA_RSRQ_TYPE;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_RS_INFO_QRY_REQ,
                                 (VOS_UINT8 *)&rsInfoQryReq, (VOS_UINT32)sizeof(AT_MTA_RsInfoQryReq), I0_UEPS_PID_MTA);
    if (ret != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RSRQ_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_QryNrSsbIdPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NR_SSB_ID_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryNrSsbIdPara: WARNING: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NR_SSB_ID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 AT_QryNCellMonitorPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 给MTA发送^NCELLMONITOR查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NCELL_MONITOR_QRY_REQ, VOS_NULL_PTR,
                                    0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NCELL_MONITOR_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestNCellMonitorPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryAnQuery(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送消息 ID_AT_MTA_WRR_ANQUERY_QRY_REQ 给 MTA 处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_ANQUERY_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ANQUERY_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCaScellInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32            ret;
    L4A_READ_ScellInfoReq sellInfoReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&sellInfoReq, sizeof(sellInfoReq), 0x00, sizeof(sellInfoReq));
    sellInfoReq.ctrl.clientId = g_atClientTab[indexNum].clientId;
    sellInfoReq.ctrl.opId = 0;
    sellInfoReq.ctrl.pid  = WUEPS_PID_AT;

    ret = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_LCASCELLINFO_QRY_REQ,
                           (VOS_UINT8 *)(&sellInfoReq), sizeof(L4A_READ_ScellInfoReq));

    if (ret == ERR_MSP_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CASCELLINFO_READ;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_QryRRCVersion(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送消息ID_AT_MTA_WRR_RRC_VERSION_QRY_REQ给AT代理处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_RRC_VERSION_QRY_REQ, VOS_NULL_PTR,
                                 0, I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RRC_VERSION_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif


