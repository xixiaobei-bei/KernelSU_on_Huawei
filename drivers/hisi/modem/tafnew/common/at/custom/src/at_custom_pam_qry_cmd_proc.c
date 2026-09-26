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
#include "at_custom_pam_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "si_app_stk.h"
#include "at_input_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PAM_QRY_CMD_PROC_C

VOS_UINT32 At_QryPassThrough(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_PassThroughQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryPassThrough: SI_PIH_PassThroughQuery fail");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PASSTHROUGH_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
/*
 * 功能描述: (AT^SINGLEMODEMDUALSLOT)状态查询
 */
VOS_UINT32 At_QrySingleModemDualSlot(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_SingleModemDualSlotQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QrySingleModemDualSlot: SI_PIH_SingleModemDualSlotQuery fail");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SINGLEMODEMDUALSLOT_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_QrySciChgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_SciCfgQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QrySciChgPara: SI_PIH_SciCfgQuery fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SCICHG_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_VSIM == FEATURE_ON)
#if (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_ON)
VOS_UINT32 At_QryIccVsimVer(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^ICCVSIMVER: %u", SI_PIH_GetSecIccVsimVer());
    return AT_OK;
}

VOS_UINT32 At_QryHvCheckCardPara(VOS_UINT8 indexNum)
{
    if (SI_PIH_HvCheckCardQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HVSCONT_READ;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}
#endif
#endif /* end of (FEATURE_VSIM == FEATURE_ON) */

TAF_UINT32 At_QryCardVoltagePara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = SI_PIH_CardVoltageQuery(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryCardVoltagePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARDVOLTAGE_QUERY;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryImsichgPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (SI_STKIsDualImsiSupport() == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_IMSICHG_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_WUEPS_PID_DRV_AGENT);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSICHG_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryModemStatusPara(VOS_UINT8 indexNum)
{
    return AT_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/*
 * 功能描述: 查询无卡模式
 * 修改历史:
 *  1.日    期: 2020年05月21日
 *    修改内容: 新生成函数
 */
VOS_UINT32 AT_QueryNoCard(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;
    ModemIdUint16 modemId = MODEM_ID_0;

    result = AT_GetModemIdFromClient(indexNum, &modemId);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_QueryNoCard:Get Modem Id fail!");
        return AT_ERROR;
    }

    /* 获取当前NOCARD的状态 */
    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        usimInfoCtx->noCardMode);
    return AT_OK;
}
#endif
#endif

VOS_UINT32 AT_TestImsichgPara(VOS_UINT8 indexNum)
{
    if (SI_STKIsDualImsiSupport() == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName,
        g_parseContext[indexNum].cmdElement->param);

    return AT_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 AT_TestApSimStPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}
#endif

