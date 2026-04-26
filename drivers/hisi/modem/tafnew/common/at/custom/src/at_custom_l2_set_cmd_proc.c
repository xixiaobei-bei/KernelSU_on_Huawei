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
#include "at_custom_l2_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_L2_SET_CMD_PROC_C

#define AT_LL2COMCFG_MAX_PARA_NUM 4
#define AT_LL2COMCFG_MIN_PARA_NUM 2
#define AT_LL2COMCFG_CMDTYPE 0
#define AT_LL2COMCFG_PARA1 1
#define AT_LL2COMCFG_PARA2 2
#define AT_LL2COMCFG_PARA3 3

#define AT_NL2COMCFG_CMDTYPE 0
#define AT_NL2COMCFG_PARA1 1
#define AT_NL2COMCFG_PARA2 2
#define AT_NL2COMCFG_PARA3 3

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_CheckL2ComCfgPara(VOS_VOID)
{
    /* 参数个数最少2个，最多4个，其他范围不正确 */
    if ((g_atParaIndex < AT_LL2COMCFG_MIN_PARA_NUM) || (g_atParaIndex > AT_LL2COMCFG_MAX_PARA_NUM)) {
        AT_WARN_LOG("AT_CheckL2ComCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数设置为空，返回错误 */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_CheckL2ComCfgPara: First para length is 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetLL2ComCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_L2ComCfgSetReq setLmacComCfg;
    VOS_UINT32            result;

    (VOS_VOID)memset_s(&setLmacComCfg, sizeof(setLmacComCfg), 0x00, sizeof(setLmacComCfg));

    /* 不是设置命令返回错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetLL2ComCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    result = AT_CheckL2ComCfgPara();

    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充内容 */
    setLmacComCfg.cmdType = g_atParaList[AT_LL2COMCFG_CMDTYPE].paraValue;
    setLmacComCfg.para1   = g_atParaList[AT_LL2COMCFG_PARA1].paraValue;
    setLmacComCfg.para2   = g_atParaList[AT_LL2COMCFG_PARA2].paraValue;
    setLmacComCfg.para3   = g_atParaList[AT_LL2COMCFG_PARA3].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_LL2_COM_CFG_SET_REQ, (VOS_UINT8 *)&setLmacComCfg,
                                    sizeof(AT_MTA_L2ComCfgSetReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LL2COMCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetLL2ComCfgPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}

VOS_UINT32 AT_SetLL2ComQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_L2ComCfgQryReq l2ComQryReq;
    VOS_UINT32            result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&l2ComQryReq, sizeof(l2ComQryReq), 0x00, sizeof(l2ComQryReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetLL2ComQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetLL2ComQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数长度不能为0，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetLL2ComQryPara: FIrst para length is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    l2ComQryReq.cmdType = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_LL2_COM_CFG_QRY_REQ, (VOS_UINT8 *)&l2ComQryReq,
                                    sizeof(AT_MTA_L2ComCfgQryReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LL2COMCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetLL2ComQryPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetNL2ComCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_L2ComCfgSetReq setNmacComCfg;
    VOS_UINT32            result;

    (VOS_VOID)memset_s(&setNmacComCfg, sizeof(setNmacComCfg), 0x00, sizeof(setNmacComCfg));

    /* 不是设置命令返回错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNL2ComCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    result = AT_CheckL2ComCfgPara();

    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充内容 */
    setNmacComCfg.cmdType = g_atParaList[AT_NL2COMCFG_CMDTYPE].paraValue;
    setNmacComCfg.para1   = g_atParaList[AT_NL2COMCFG_PARA1].paraValue;
    setNmacComCfg.para2   = g_atParaList[AT_NL2COMCFG_PARA2].paraValue;
    setNmacComCfg.para3   = g_atParaList[AT_NL2COMCFG_PARA3].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NL2_COM_CFG_SET_REQ, (VOS_UINT8 *)&setNmacComCfg,
                                    sizeof(AT_MTA_L2ComCfgSetReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NL2COMCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetNL2ComCfgPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}

VOS_UINT32 AT_SetNL2ComQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_L2ComCfgQryReq l2ComQryReq;
    VOS_UINT32            result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&l2ComQryReq, sizeof(l2ComQryReq), 0x00, sizeof(l2ComQryReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNL2ComQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetNL2ComQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数长度不能为0，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetNL2ComQryPara: First para length is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    l2ComQryReq.cmdType = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NL2_COM_CFG_QRY_REQ, (VOS_UINT8 *)&l2ComQryReq,
                                    sizeof(AT_MTA_L2ComCfgQryReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NL2COMCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetNL2ComQryPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}

VOS_UINT32 AT_CheckNPdcpCfgPara(VOS_VOID)
{
    /* 参数个数最少2个，最多4个，其他范围不正确 */
    if ((g_atParaIndex < 2) || (g_atParaIndex > 4)) {
        AT_WARN_LOG("AT_CheckNPdcpCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数设置为空，返回错误 */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_CheckNPdcpCfgPara: First para length is 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetNPdcpSleepThresCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_NPdcpSleepThresCfgReq setNPdcpCfg;
    VOS_UINT32                   result;

    (VOS_VOID)memset_s(&setNPdcpCfg, sizeof(setNPdcpCfg), 0x00, sizeof(setNPdcpCfg));

    /* 不是设置命令返回错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNPdcpSleepThresCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    result = AT_CheckNPdcpCfgPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充内容 */
    setNPdcpCfg.operation  = g_atParaList[0].paraValue;
    setNPdcpCfg.sleepThres = g_atParaList[1].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NPDCP_SLEEP_THRES_CFG_REQ, (VOS_UINT8 *)&setNPdcpCfg,
                                    sizeof(AT_MTA_NPdcpSleepThresCfgReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NPDCP_SLEEPTHRES_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetNPdcpSleepThresCfgPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}
#endif

