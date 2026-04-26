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
#include "at_device_pam_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "at_device_comm.h"
#include "at_device_cmd.h"
#include "at_mdrv_interface.h"
#include "at_input_proc.h"
#include "at_mt_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PAM_QRY_CMD_PROC_C

#if (FEATURE_PHONE_SC == FEATURE_ON)
VOS_UINT32 AT_QryPhoneSimlockInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    AT_PR_LOGI("Rcv Msg");

    /* 发送跨核消息到C核, 查询锁网锁卡信息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_PHONESIMLOCKINFO_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryPhoneSimlockInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PHONESIMLOCKINFO_READ;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QrySimlockDataReadPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    AT_PR_LOGI("Rcv Msg");

    /* 发送跨核消息到C核, 查询锁网锁卡信息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_SIMLOCKDATAREAD_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QrySimlockDataReadPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIMLOCKDATAREAD_READ;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_QryEsimSwitch(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_PIH_EsimSwitchQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryEsimSwitch: Qry Esim Switch fail.");

        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMSWITCH_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryEsimCheckProfile(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_EMAT_EsimCheckProfile(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryEsimCheckProfile: Set Esim Switch fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMCHECK_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryEsimEid(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_EMAT_EsimEidQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryEsimEid: Get eSIM EID fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMEID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryEsimPKID(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_EMAT_EsimPKIDQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryEsimPKID: Get eSIM PKID fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMPKID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_PHONE_SC == FEATURE_ON)
/*
 * 功能描述: ^GETMODEMSCID查询命令处理函数
 */
VOS_UINT32 AT_QryModemScIdPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    AT_PR_LOGI("Rcv Msg");

    /* 发送跨核消息到C核, 查询锁网锁卡信息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_GETMODEMSCID_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryModemScIdPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GETMODEMSCID_READ;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryRficDieIDExPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  rst;

    atMtInfoCtx = AT_GetMtInfoCtx();

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 通知CCPU_PID_PAM_MFG查询RFIC IDE ID */
    rst = At_SndUeCbtRfIcIdExQryReq();
    if (rst != AT_SUCCESS) {
        AT_WARN_LOG("AT_QryRficDieIDExPara: Snd fail.");
        return AT_ERROR;
    }

    atMtInfoCtx->atInfo.indexNum          = indexNum;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RFIC_DIE_ID_EX_QRY;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
#else
VOS_UINT32 At_SndUeCbtRfIcIdExQryReq(VOS_VOID)
{
    AT_UECBT_DieIdQueryReq *qryReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    qryReq = (AT_UECBT_DieIdQueryReq *)PS_ALLOC_MSG(WUEPS_PID_AT,
        sizeof(AT_UECBT_DieIdQueryReq) - VOS_MSG_HEAD_LENGTH);
    if (qryReq == VOS_NULL_PTR) {
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(qryReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(qryReq, CCPU_PID_PAM_MFG, ID_AT_UECBT_RFIC_DIE_IE_QUERY_REQ);

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, qryReq) != VOS_OK) {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndUeCbtRfIcIdExQryReq Exit");
    return AT_SUCCESS;
}
#endif
VOS_UINT32 AT_TestSimlockUnlockPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (\"NET\",\"NETSUB\",\"SP\")",
        g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

