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

#include "at_general_pam_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_PMM_SET_CMD_PROC_C

#define AT_ADCTEMP_PARA_MAX_NUM 4
#define AT_ADCTEMP_VALUE3 2
#define AT_ADCTEMP_VALUE4 3


#define AT_CGLA_PARA_NUM 3
#define AT_CGLA_LENGTH 1
#define AT_CGLA_COMMAND 2

TAF_UINT32 At_SetCimiPara(TAF_UINT8 indexNum)
{
    TAF_AT_NvimCimiPortCfg cimiPortCfg;

    cimiPortCfg.cimiPortCfg = 0;

    /* 根据NV项配置参数，对命令下发的通道进行判断 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CIMI_PORT_CFG, &cimiPortCfg, sizeof(cimiPortCfg)) != NV_OK) {
        cimiPortCfg.cimiPortCfg = 0;
    }

    /* 该通道对应的Bit位不为0，则直接返回AT_ERROR */
    if ((cimiPortCfg.cimiPortCfg & ((VOS_UINT32)1 << indexNum)) != 0) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PIH_CimiSetReq(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS)
    {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIMI_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCgsnPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* AT命令状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息 ID_AT_MTA_CGSN_QRY_REQ 给 MTA 处理， */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, ID_AT_MTA_CGSN_QRY_REQ,
                                 VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGSN_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetAdcTempPara(TAF_UINT8 indexNum)
{
    SPY_TempThresholdPara tempPara;
    VOS_UINT32            changeFlag = AT_SPY_TEMP_THRESHOLD_PARA_UNCHANGE;
    VOS_UINT32            ret;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_ADCTEMP_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    /* 全设为无效值 */
    (VOS_VOID)memset_s(&tempPara, sizeof(tempPara), 0xFF, sizeof(tempPara));

    if (g_atParaList[0].paraLen != 0) {
        tempPara.isEnable = g_atParaList[0].paraValue;
        changeFlag        = AT_SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if (g_atParaList[1].paraLen != 0) {
        tempPara.closeAdcThreshold = (VOS_INT)g_atParaList[1].paraValue;
        changeFlag                 = AT_SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if (g_atParaList[AT_ADCTEMP_VALUE3].paraLen != 0) {
        tempPara.alarmAdcThreshold  = (VOS_INT)g_atParaList[AT_ADCTEMP_VALUE3].paraValue;
        changeFlag                  = AT_SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if (g_atParaList[AT_ADCTEMP_VALUE4].paraLen != 0) {
        tempPara.resumeAdcThreshold  = (VOS_INT)g_atParaList[AT_ADCTEMP_VALUE4].paraValue;
        changeFlag                   = AT_SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if (changeFlag == AT_SPY_TEMP_THRESHOLD_PARA_CHANGE) {
        /* 调用接口设置门限值 */
        /* 发消息到C核设置 热保护温度状态  */
        ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                     DRV_AGENT_ADC_SET_REQ, (VOS_UINT8 *)&tempPara, sizeof(tempPara),
                                     I0_WUEPS_PID_DRV_AGENT);
        if (ret != TAF_SUCCESS) {
            AT_WARN_LOG("At_SetAdcTempPara: AT_FillAndSndAppReqMsg fail.");
            return AT_ERROR;
        }

        /* 设置AT模块实体的状态为等待异步返回 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ADC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_OK;
}

/*
 * Description: +CGLA=<sessionid>,<length>,<command>
 * History:
 *  1.Date: 2013-05-15
 *    Modification: Created function
 */
TAF_UINT32 At_SetCglaPara(TAF_UINT8 indexNum)
{
    SI_PIH_CglaCommand cglaCmd;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CGLA_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <length>需要为2的整数倍 */
    if ((g_atParaList[AT_CGLA_LENGTH].paraValue % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CGLA_COMMAND].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CGLA_COMMAND].para,
                              &g_atParaList[AT_CGLA_COMMAND].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetCglaPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* length字段是实际命令长度的2倍 */
    if (g_atParaList[AT_CGLA_LENGTH].paraValue != (TAF_UINT32)(g_atParaList[AT_CGLA_COMMAND].paraLen * 2)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cglaCmd.sessionID = g_atParaList[0].paraValue;
    cglaCmd.len       = g_atParaList[AT_CGLA_COMMAND].paraLen;
    cglaCmd.command   = g_atParaList[AT_CGLA_COMMAND].para;

    /* 执行命令操作 */
    if (SI_PIH_CglaSetReq(g_atClientTab[indexNum].clientId, 0, &cglaCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGLA_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

