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
#include "at_test_taf_set_cmd_proc.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "at_ctx.h"
#include "at_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_check_func.h"
#include "at_mbb_cmd.h"
#include "at_msg_print.h"
#include "taf_std_lib.h"
#include "at_lte_common.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_TAF_SET_CMD_PROC_C
#define AT_BOOT_PARA_NUM 2
#define AT_COMM_DEBUG_DEACT_ALL 0
#define AT_COMM_DEBUG_ENABLE_DRX_REPORT 1
#define AT_COMM_DEBUG_ENABLE_INIT_USIM_RECORD 3
#define AT_COMM_DEBUG_ENABLE_USIM_DEBUG 5
#define AT_COMM_DEBUG_DISABLE_USIM_DEBUG 6
#define AT_COMM_DEBUG_ACT_USIM_CARD 7
#define AT_COMM_DEBUG_DEACT_USIM_CARD 8
#define AT_DISSD_OPEN 0
#define AT_DISSD_CLOSE 1

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_SetGcfIndPara(TAF_UINT8 indexNum)
{
    VOS_UINT8  gctTestFlag = 0;
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    gctTestFlag = (VOS_UINT8)g_atParaList[0].paraValue;
    if ((gctTestFlag != 0) && (gctTestFlag != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, DRV_AGENT_GCF_IND, &gctTestFlag,
                                 sizeof(VOS_UINT8), I0_WUEPS_PID_DRV_AGENT);

    if (rst == TAF_SUCCESS) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetBOOTPara(TAF_UINT8 indexNum)
{
    /* 参数过多 */
    if (g_atParaIndex > AT_BOOT_PARA_NUM) {
        return AT_TOO_MANY_PARA;
    }

    /* 参数过少 */
    if (g_atParaIndex < AT_BOOT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 输入参数取值检查, para2必须为 0 */
    if ((g_atParaList[1].paraLen != 1) || (g_atParaList[1].para[0] != '0')) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 假命令，直接返回OK. */
    return AT_OK;
}

LOCAL TAF_UINT32 At_ProcCommDebugParaActUsimCard(VOS_VOID)
{
    USIMM_ActiveCardReq *msg = VOS_NULL_PTR;

    msg = (USIMM_ActiveCardReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(USIMM_ActiveCardReq) - VOS_MSG_HEAD_LENGTH);

    if (msg == VOS_NULL_PTR) {
        LogPrint("At_ProcCommDebugParaActUsimCard:AllocMsg Failed.");

        return VOS_ERR;
    }

    /* 初始化消息 */
    (VOS_VOID)memset_s((VOS_CHAR *)msg + VOS_MSG_HEAD_LENGTH,
                       (VOS_SIZE_T)(sizeof(USIMM_ActiveCardReq) - VOS_MSG_HEAD_LENGTH), 0x00,
                       (VOS_SIZE_T)(sizeof(USIMM_ActiveCardReq) - VOS_MSG_HEAD_LENGTH));

    VOS_SET_RECEIVER_ID(msg, WUEPS_PID_USIM);
    msg->msgHeader.msgName = USIMM_ACTIVECARD_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        LogPrint("At_ProcCommDebugParaActUsimCard:sndmsg Failed.");
    }

    return AT_OK;
}


VOS_UINT32 AT_CheckCommDebugParaValid(VOS_VOID)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetCommDebugParaDeactAllDebug(VOS_VOID)
{
    TAF_AT_NvimCommdegbugCfg    debugFlag;

    debugFlag.commDebugFlag = VOS_FALSE;

    /* 去激活所有Debug功能 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, &(debugFlag.commDebugFlag),
                          sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
        LogPrint("AT_SetCommDebugParaDeactAllDebug:TAF_ACORE_NV_READ Debug Flag0 Fail\r\n");

        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    if (debugFlag.commDebugFlag != 0x00) {
        debugFlag.commDebugFlag = 0x00;

        if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, (VOS_UINT8 *)&(debugFlag.commDebugFlag),
                               sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
            LogPrint("AT_SetCommDebugParaDeactAllDebug:TAF_ACORE_NV_WRITE Debug Flag0 Fail\r\n");

            return AT_CME_OPERATION_NOT_ALLOWED;
        }
    }

    return AT_OK;
}

VOS_UINT32 AT_SetCommDebugParaEnableDrxReport(VOS_VOID)
{
    TAF_AT_NvimCommdegbugCfg    debugFlag;

    debugFlag.commDebugFlag = VOS_FALSE;

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, &(debugFlag.commDebugFlag),
                          sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
        LogPrint("At_SetOamDebugPara:TAF_ACORE_NV_READ Debug Flag Fail\r\n");

        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    if ((debugFlag.commDebugFlag & 0x01) != 0x01) {
        debugFlag.commDebugFlag |= 0x01;

        if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, (VOS_UINT8 *)&(debugFlag.commDebugFlag),
                               sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
            LogPrint("At_SetOamDebugPara:TAF_ACORE_NV_WRITE Debug Flag Fail\r\n");

            return AT_CME_OPERATION_NOT_ALLOWED;
        }
    }

    return AT_OK;
}

VOS_UINT32 AT_SetCommDebugParaEnableInitUsimRecord(VOS_VOID)
{
    TAF_AT_NvimCommdegbugCfg    debugFlag;

    debugFlag.commDebugFlag = VOS_FALSE;

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, &(debugFlag.commDebugFlag),
                          sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
        LogPrint("At_SetOamDebugPara:TAF_ACORE_NV_READ Debug Flag2 Fail\r\n");

        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    if ((debugFlag.commDebugFlag & 0x02) != 0x02) {
        debugFlag.commDebugFlag |= 0x02;

        if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_COMMDEGBUG_CFG, (VOS_UINT8 *)&(debugFlag.commDebugFlag),
                               sizeof(TAF_AT_NvimCommdegbugCfg)) != NV_OK) {
            LogPrint("At_SetOamDebugPara:TAF_ACORE_NV_WRITE Debug Flag2 Fail\r\n");

            return AT_CME_OPERATION_NOT_ALLOWED;
        }
    }

    return AT_OK;
}

TAF_UINT32 At_SetCommDebugPara(TAF_UINT8 indexNum)
{
    VOS_UINT32                  result;

    result = AT_CheckCommDebugParaValid();
    if (result != AT_OK) {
        return result;
    }

    switch (g_atParaList[0].paraValue) {
        case AT_COMM_DEBUG_DEACT_ALL:
            /* 去激活所有Debug功能 */
            return AT_SetCommDebugParaDeactAllDebug();
        case AT_COMM_DEBUG_ENABLE_DRX_REPORT:
            /* 使能DRX定位信息上报功能 */
            return AT_SetCommDebugParaEnableDrxReport();

        case AT_COMM_DEBUG_ENABLE_INIT_USIM_RECORD:
            /* 使能USIMM初始化信息记录功能 */
            return AT_SetCommDebugParaEnableInitUsimRecord();

        case AT_COMM_DEBUG_ENABLE_USIM_DEBUG:
            return AT_CME_OPERATION_NOT_ALLOWED;

        case AT_COMM_DEBUG_DISABLE_USIM_DEBUG:
            return AT_CME_OPERATION_NOT_ALLOWED;

        case AT_COMM_DEBUG_ACT_USIM_CARD:
            /* 拆圈 */
            return At_ProcCommDebugParaActUsimCard();


        default:
            break;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetOpenportPara(VOS_UINT8 indexNum)
{
    /*
     * 能在这里处理OPENPORT, 要么是已经有权限了再输入OPENPORT,直接返回OK
     * 要么是没开启密码保护, 也直接返回OK
     */
    return AT_OK;
}

VOS_UINT32 AT_SetDockPara(VOS_UINT8 indexNum)
{
    VOS_UINT8  clientIndex;
    VOS_UINT16 length = 0;

    if (g_atClientTab[indexNum].userType != AT_APP_USER) {
        return AT_FAILURE;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_FAILURE;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_FAILURE;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(g_atParaList[0].para + g_atParaList[0].paraLen, AT_PARA_MAX_LEN - g_atParaList[0].paraLen,
             0x00, AT_PARA_MAX_LEN - g_atParaList[0].paraLen);

    for (clientIndex = 0; clientIndex < AT_MAX_CLIENT_NUM; clientIndex++) {
        if (g_atClientTab[clientIndex].userType == AT_USBCOM_USER) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%s", g_atCrLf, g_atParaList[0].para, g_atCrLf);
            At_SendResultData(clientIndex, g_atSndCodeAddress, length);
            break;
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetDissdCmd(VOS_UINT8 indexNum)
{
    VOS_UINT32        rlst;
    AT_DynamicPidType dynamicPidType;

    (VOS_VOID)memset_s(&dynamicPidType, sizeof(dynamicPidType), 0x00, sizeof(dynamicPidType));

    /* 参数检查: 必须要第0和第1个参数有数据 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_ERROR;
    }

    /* 密码不一致则写NV项返回AT_ERROR */
    if (VOS_StrCmp((VOS_CHAR *)g_ate5DissdPwd, (VOS_CHAR *)g_atParaList[1].para) != 0) {
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));
        return AT_ERROR;
    }

    /* 读NV项NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE获取当前的端口状态 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE, &dynamicPidType, sizeof(AT_DynamicPidType)) !=
        NV_OK) {
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));
        return AT_ERROR;
    }

    /* 判断NV项是否激活 */
    if (dynamicPidType.nvStatus != VOS_TRUE) {
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_DISSD_OPEN) {
        rlst = AT_OpenSpecificPort(AT_DEV_SD);
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));

        return rlst;
    }

    /* 关闭DIAG口 */
    if (g_atParaList[0].paraValue == AT_DISSD_CLOSE) {
        rlst = AT_CloseSpecificPort(AT_DEV_SD);
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));

        return rlst;
    }

    (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x00, sizeof(g_atParaList[1].para));
    return AT_OK;
}

/*
 * Description: +TEST=[<n>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetTestPara(TAF_UINT8 indexNum)
{
    return AT_OK;
}
#endif

