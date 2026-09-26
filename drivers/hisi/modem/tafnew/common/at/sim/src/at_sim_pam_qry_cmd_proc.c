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
#include "at_sim_pam_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_sim_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SIM_PAM_QRY_CMD_PROC_C

TAF_UINT32 At_QryCpbsPara(TAF_UINT8 indexNum)
{
    g_pbPrintTag = VOS_TRUE;

    if (SI_PB_Query(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBS_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryParaRspSimsqPara(VOS_UINT8 indexNum)
{
    ModemIdUint16 modemId;

    /* 将设置参数保存到CC上下文中 */
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_LOG1("AT_ProcSimsqInd AT_GetModemIdFromClient fail", indexNum);
        return AT_ERROR;
    }

    AT_ProcReportSimSqInfo(indexNum, At_GetSimsqStatus(modemId));

    return AT_OK;
}

TAF_UINT32 At_QryPNNPara(TAF_UINT8 indexNum)
{

    return AT_OK;
}

TAF_UINT32 At_QryCPNNPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_CPNN_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_WUEPS_PID_DRV_AGENT);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPNN_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryOPLPara(TAF_UINT8 indexNum)
{

    return AT_OK;
}

VOS_UINT32 At_QryHvsstPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_HvSstQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryPortAttribSetPara: SI_PIH_HvSstQuery fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HVSST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryCardTypePara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_CardTypeQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("SI_PIH_CardTypeQuery: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARDTYPE_QUERY;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryCardTypeExPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_CardTypeExQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("SI_PIH_CardTypeQuery: At_QryCardTypeExPara fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARDTYPEEX_QUERY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryEflociInfoPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_QryEflociInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EFLOCIINFO_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryPsEflociInfoPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_QryPsEflociInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EFPSLOCIINFO_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryCardSession(VOS_UINT8 indexNum)
{
    if (SI_PIH_CardSessionQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId) == TAF_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARDSESSION_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_WARN_LOG("At_QryCardSession: SI_PIH_CardSessionQuery fail.");

    /* 返回命令处理挂起状态 */
    return AT_ERROR;
}

VOS_UINT32 AT_TestCpbsPara(VOS_UINT8 indexNum)
{
    if (SI_PB_Query(g_atClientTab[indexNum].clientId, 1) == TAF_SUCCESS) {
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestCpbrPara(VOS_UINT8 indexNum)
{
    if (SI_PB_Query(g_atClientTab[indexNum].clientId, 0) == TAF_SUCCESS) {
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestCpnnPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_CPNN_TEST_REQ, VOS_NULL_PTR, 0,
                                 I0_WUEPS_PID_DRV_AGENT);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPNN_TEST;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestOplPara(VOS_UINT8 indexNum)
{
    /* VOS_UINT32                          ulOplExistFlg; */
    TAF_PH_QryUsimInfo usimInfo;

    (VOS_VOID)memset_s(&usimInfo, sizeof(usimInfo), 0x00, sizeof(TAF_PH_QryUsimInfo));

    /* 执行命令操作 */
    usimInfo.efId    = TAF_PH_OPL_FILE;
    usimInfo.icctype = TAF_PH_ICC_USIM;
    usimInfo.recNum  = TAF_PH_INVALID_USIM_REC;
    if (TAF_QryUsimInfo(g_atClientTab[indexNum].clientId, 0, &usimInfo) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_OPL_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestPnnPara(VOS_UINT8 indexNum)
{
    /* VOS_UINT32                          ulPnnExistFlg; */
    TAF_PH_QryUsimInfo usimInfo;

    (VOS_VOID)memset_s(&usimInfo, sizeof(usimInfo), 0x00, sizeof(TAF_PH_QryUsimInfo));

    /* 执行命令操作 */
    usimInfo.efId    = TAF_PH_PNN_FILE;
    usimInfo.icctype = TAF_PH_ICC_USIM;
    usimInfo.recNum  = TAF_PH_INVALID_USIM_REC;
    if (TAF_QryUsimInfo(g_atClientTab[indexNum].clientId, 0, &usimInfo) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PNN_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

