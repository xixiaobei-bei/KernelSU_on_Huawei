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
#include "at_ltev_ttf_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_snd_msg.h"
#include "at_ltev_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TTF_SET_CMD_PROC_C

#define AT_PTRRPT_PARA_NUM 1

#define AT_MCGCFG_PARA_MAX_NUM       15
#define AT_MCGCFG_PARA_GROUP_ID      0 /* MCGCFG的第一个参数GROUP_ID */
#define AT_MCGCFG_PARA_SELF_RSU_ID   1 /* MCGCFG的第二个参数SELF_RSU_ID */
#define AT_MCGCFG_PARA_SELF_RSU_TYPE 2 /* MCGCFG的第三个参数SELF_RSU_TYPE */
#define AT_MCGCFG_PARA_INDOOR_RSU_ID 3 /* MCGCFG的第四个参数INDOOR_RSU_ID */
#define AT_MCGCFG_PARA_RSU_LIST_NUM  4 /* MCGCFG的第五个参数RSU_LIST_NUM */
#define AT_MCGCFG_PARA_RSU_ID_LIST   5 /* MCGCFG的第六个参数RSU_ID_LIST */

#if (FEATURE_LTEV == FEATURE_ON)

VOS_UINT32 VPDCP_SetPtrRpt(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32       rslt;
    AT_VPDCP_ReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32       length;

    /* Allocating memory for message */
    length = sizeof(AT_VPDCP_ReqMsg) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_VPDCP_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, length, 0x00, length);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = VPDCP_MSG_RSU_PTRRPT_SET_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VPDCP_UL);
    msg->content[0]    = 1;

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VPDCP_SetPtrRpt: SEND MSG FAIL");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetPtrRpt(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_PTRRPT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    rst = VPDCP_SetPtrRpt(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (rst != VOS_OK) {
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PTRRPT_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 AT_GetOptionCmdParaByParaIndex(VOS_UINT8 paraIndex)
{
    if (g_atParaList[paraIndex].paraLen == 0) {
        return 0;
    }

    return g_atParaList[paraIndex].paraValue;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SendData(VOS_UINT8 indexNum)
{
    AT_VTC_SendVMacDataPara params;
    VOS_UINT32              ret;
    VOS_UINT32              i;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex < AT_SENDDATA_PARAM_MIN_NUM) ||
        (g_atParaIndex > AT_SENDDATA_PARAM_MAX_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 必选参数检查 */
    for (i = 0; i < AT_SENDDATA_PARAM_MIN_NUM; i++) {
        if (g_atParaList[i].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    (VOS_VOID)memset_s(&params, sizeof(params), 0, sizeof(params));

    params.saRbStart        = (VOS_UINT8)g_atParaList[AT_SENDDATA_SARB_START_INDEX].paraValue;
    params.daRbNum          = (VOS_UINT8)g_atParaList[AT_SENDDATA_DARB_NUM_INDEX].paraValue;
    params.mcs              = (VOS_UINT8)g_atParaList[AT_SENDDATA_MCS_INDEX].paraValue;
    params.period           = (VOS_UINT16)g_atParaList[AT_SENDDATA_PERIOD_INDEX].paraValue;
    params.sendFrmMask      = (VOS_UINT16)g_atParaList[AT_SENDDATA_SEND_FRMMASK_INDEX].paraValue;
    params.flag             = (VOS_UINT8)g_atParaList[AT_SENDDATA_FLAG_INDEX].paraValue;
    params.resRsvPeriod     = (VOS_UINT16)AT_GetOptionCmdParaByParaIndex(AT_SENDDATA_RES_RSV_PERIOD_INDEX);
    params.frameOffset      = (VOS_UINT16)AT_GetOptionCmdParaByParaIndex(AT_SENDDATA_FRAME_OFFSET_INDEX);
    params.pktPriority      = (VOS_UINT8)AT_GetOptionCmdParaByParaIndex(AT_SENDDATA_PKT_PRIORITY_INDEX);
    params.timeGap          = (VOS_UINT8)AT_GetOptionCmdParaByParaIndex(AT_SENDDATA_TIME_GAP_INDEX);
    params.randomFlag       = (VOS_UINT8)AT_GetOptionCmdParaByParaIndex(AT_SENDDATA_RANDOM_FLAG_INDEX);

    /* 执行命令操作 */
    ret = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, AT_VTC_PC5_SEND_DATA_REQ,
                       (VOS_UINT8 *)&params, sizeof(AT_VTC_SendVMacDataPara));
    if (ret != VOS_OK) {
        AT_WARN_LOG("AT_SendData AT_ERROR\n");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SENDDATA_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif
#endif

