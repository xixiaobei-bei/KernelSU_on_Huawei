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

#include "at_general_mm_qry_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_MM_QRY_CMD_PROC_C

VOS_UINT32 AT_QryLocinfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 发消息给MMA获取当前UE的位置信息 */
    result = TAF_MMA_QryLocInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);

    /* 执行命令操作 */
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LOCINFO_READ;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryUserCfgOPlmnPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 发送消息 ID_AT_MTA_HANDLEDECT_QRY_REQ 给MTA处理 */
    rst = TAF_MMA_QryEOPlmnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EOPLMN_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryWs46Para(VOS_UINT8 indexNum)
{
    if (TAF_MMA_QrySyscfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_WS46_QRY; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                             /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestWs46Para(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "(12,22,25,28,29,30,31)");

    return AT_OK;
}

VOS_UINT32 At_QryCerssiPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = TAF_MMA_QryCerssiReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 执行命令操作 */
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CERSSI_READ;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryC5gPNssaiPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNssaiQryReq st5gNssaiQryReq;
    VOS_UINT32           rslt;

    /* 初始化 */
    (VOS_VOID)memset_s(&st5gNssaiQryReq, sizeof(st5gNssaiQryReq), 0x00, sizeof(AT_MTA_5GNssaiQryReq));
    st5gNssaiQryReq.opPlmn        = VOS_FALSE;
    st5gNssaiQryReq.sNssaiQryType = AT_MTA_NSSAI_QRY_PREFER_NSSAI;

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_5G_NSSAI_QRY_REQ, (VOS_UINT8 *)&st5gNssaiQryReq,
                                  (VOS_SIZE_T)sizeof(st5gNssaiQryReq), I0_UEPS_PID_MTA);

    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryC5gPNssaiPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GPNSSAI_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestC5gPNssai(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-159),", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryC5gNssaiPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNssaiQryReq st5gNssaiQryReq;
    VOS_UINT32           rslt;

    /* 初始化 */
    (VOS_VOID)memset_s(&st5gNssaiQryReq, sizeof(st5gNssaiQryReq), 0x00, sizeof(AT_MTA_5GNssaiQryReq));
    st5gNssaiQryReq.opPlmn        = VOS_FALSE;
    st5gNssaiQryReq.sNssaiQryType = AT_MTA_NSSAI_QRY_DEF_CFG_NSSAI_ONLY;

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_5G_NSSAI_QRY_REQ, (VOS_UINT8 *)&st5gNssaiQryReq,
                                  (VOS_SIZE_T)sizeof(st5gNssaiQryReq), I0_UEPS_PID_MTA);

    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryC5gNssaiPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GNSSAI_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestC5gNssai(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-319),", g_parseContext[indexNum].cmdElement->cmdName);
    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 At_TestC5gNssaiRdp(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1,2,3),", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_QryCserssiPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = TAF_MMA_QryCserssiReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 执行命令操作 */
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSERSSI_READ;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif



