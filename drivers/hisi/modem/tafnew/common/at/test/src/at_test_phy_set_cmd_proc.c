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
#include "at_test_phy_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_PHY_SET_CMD_PROC_C

#define AT_TXTASTEST_PARA_NUM 5
#define AT_TXTASTEST_RAT_MODE 0
#define AT_TXTASTEST_TXSAR_FORCED_SWITCH 1
#define AT_TXTASTEST_DL_OPTIMAL_TX 2
#define AT_TXTASTEST_TXSAR_SWITCH 3
#define AT_TXTASTEST_PARA_RESERVED 4
#define AT_LTESARSTUB_PARA_NUM 1

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetTasTestCfg(VOS_UINT8 indexNum)
{
    AT_MTA_TasTestCfg tasParamReq;
    VOS_UINT32        result;

    /* 局部变量初始化 */
    memset_s(&tasParamReq, (VOS_SIZE_T)sizeof(tasParamReq), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_TasTestCfg));

    /* 参数检查 */
    if ((g_atParaIndex != AT_TXTASTEST_PARA_NUM) || (g_atParaList[AT_TXTASTEST_RAT_MODE].paraLen == 0) ||
        (g_atParaList[AT_TXTASTEST_TXSAR_FORCED_SWITCH].paraLen == 0) ||
        (g_atParaList[AT_TXTASTEST_DL_OPTIMAL_TX].paraLen == 0) ||
        (g_atParaList[AT_TXTASTEST_TXSAR_SWITCH].paraLen == 0) ||
        (g_atParaList[AT_TXTASTEST_PARA_RESERVED].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

#if (FEATURE_UE_MODE_CDMA != FEATURE_ON)
    if ((g_atParaList[AT_TXTASTEST_RAT_MODE].paraValue == MTA_AT_RATMODE_1X) ||
        (g_atParaList[AT_TXTASTEST_RAT_MODE].paraValue == MTA_AT_RATMODE_HRPD)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    /* 填写消息参数 */
    tasParamReq.ratMode = g_atParaList[AT_TXTASTEST_RAT_MODE].paraValue;
    tasParamReq.para0   = g_atParaList[AT_TXTASTEST_TXSAR_FORCED_SWITCH].paraValue;
    tasParamReq.para1   = g_atParaList[AT_TXTASTEST_DL_OPTIMAL_TX].paraValue;
    tasParamReq.para2   = g_atParaList[AT_TXTASTEST_TXSAR_SWITCH].paraValue;
    tasParamReq.para3   = g_atParaList[AT_TXTASTEST_PARA_RESERVED].paraValue;

    /* 发送消息给MTA */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_TAS_CFG_REQ, (VOS_UINT8 *)&tasParamReq, (VOS_SIZE_T)sizeof(tasParamReq),
                                    I0_UEPS_PID_MTA);
    /* 发送失败 */
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetTasTestCfg: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TAS_TEST_SET;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryTasTestCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32        result;
    AT_MTA_TasTestQry atMtaTasTestQry;

    /* 参数检查 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

#if (FEATURE_UE_MODE_CDMA != FEATURE_ON)
    if ((g_atParaList[0].paraValue == MTA_AT_RATMODE_1X) ||
        (g_atParaList[0].paraValue == MTA_AT_RATMODE_HRPD)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    /* 填写消息参数 */
    atMtaTasTestQry.ratMode = g_atParaList[0].paraValue;

    /* 发送消息给MTA */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_TAS_QRY_REQ, (VOS_UINT8 *)&atMtaTasTestQry,
                                    (VOS_SIZE_T)sizeof(atMtaTasTestQry), I0_UEPS_PID_MTA);
    /* 发送失败 */
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryTasTestCfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TAS_TEST_QRY;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetLteSarStubPara(VOS_UINT8 indexNum)
{
    AT_MTA_LteSarStubSetReq atCmd;
    VOS_UINT32              rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 参数个数检查 */
    if (g_atParaIndex != AT_LTESARSTUB_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0, sizeof(atCmd));
    atCmd.switchFlag = (VOS_UINT8)g_atParaList[0].paraValue;
    /* 下发参数更新请求消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_LTESARSTUB_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_LteSarStubSetReq), I0_UEPS_PID_MTA);
    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTESARSTUB_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

#endif


