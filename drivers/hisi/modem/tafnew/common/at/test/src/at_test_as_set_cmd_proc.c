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

#include "at_test_as_set_cmd_proc.h"
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
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_AS_SET_CMD_PROC_C
#define AT_CWAS_PARA_MIN_NUM 1
#define AT_CWAS_PARA_MAX_NUM 4
#define AT_CGAS_PARA_MIN_NUM 1
#define AT_CGAS_PARA_MAX_NUM 11
#define AT_NRFREQLOCK_PARA_NUM_BAND 3
#define AT_NRFREQLOCK_PARA_NUM_FREQ 4
#define AT_NRFREQLOCK_PARA_NUM_CELL 5
#define AT_NRFREQLOCK_BAND 2
#define AT_NRFREQLOCK_NRARFCN 3
#define AT_NRFREQLOCK_PHYCELL_ID 4

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

/* For WAS AutoTest */
TAF_UINT32 At_SetCwasPara(TAF_UINT8 indexNum)
{
    AT_MTA_WrrAutotestQryPara atCmd;
    VOS_UINT32                rst;

    /* 参数过多 */
    if ((g_atParaIndex > AT_CWAS_PARA_MAX_NUM) || (g_atParaIndex < AT_CWAS_PARA_MIN_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_WrrAutotestQryPara));
    atCmd.cmd = (TAF_UINT8)g_atParaList[0].paraValue;

    for (atCmd.paraNum = 0; atCmd.paraNum < (g_atParaIndex - 1); atCmd.paraNum++) {
        atCmd.para[atCmd.paraNum] = g_atParaList[atCmd.paraNum + 1].paraValue;
    }

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_AUTOTEST_QRY_REQ,
                                 (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_WrrAutotestQryPara), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CWAS_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCGasPara(TAF_UINT8 indexNum)
{
    GAS_AT_CmdPara atCmd;
    VOS_UINT32     rst;

    /* 参数过多 */
    if ((g_atParaIndex > AT_CGAS_PARA_MAX_NUM) || (g_atParaIndex < AT_CGAS_PARA_MIN_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度等于0 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(GAS_AT_CmdPara), 0x00, sizeof(GAS_AT_CmdPara));

    /* 获取命令参数 */
    atCmd.cmd = (TAF_UINT8)g_atParaList[0].paraValue;
    for (atCmd.paraNum = 0; atCmd.paraNum < (g_atParaIndex - 1); atCmd.paraNum++) {
        atCmd.para[atCmd.paraNum] = g_atParaList[atCmd.paraNum + 1].paraValue;
    }

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_GAS_AUTOTEST_QRY_REQ,
                                 (VOS_UINT8 *)&atCmd, sizeof(GAS_AT_CmdPara), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGAS_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_GetMeanRpt(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数过多 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息ID_AT_MTA_WRR_MEANRPT_QRY_REQ给AT AGENT处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_MEANRPT_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MEANRPT_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
/* For Was AutoTest */

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

TAF_UINT32 At_SetNrFreqLockPara(TAF_UINT8 indexNum)
{
    AT_MTA_SetNrfreqlockReq nrFreqLock;
    VOS_UINT32              rst;

    (VOS_VOID)memset_s(&nrFreqLock, sizeof(nrFreqLock), 0x00, sizeof(nrFreqLock));

    rst = AT_GetNrFreqLockPara(indexNum, &nrFreqLock);

    if (rst != AT_SUCCESS) {
        return rst;
    }

    /* 发送消息 ID_AT_MTA_NR_FREQLOCK_SET_REQ 给C核 AT 代理处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NR_FREQLOCK_SET_REQ,
                                 (VOS_UINT8 *)&nrFreqLock, sizeof(AT_MTA_SetNrfreqlockReq), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRFREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_GetNrFreqLockPara(VOS_UINT8 clientId, AT_MTA_SetNrfreqlockReq *nrFreqLockInfo)
{
    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("GetNrFreqLockPara: At Cmd Opt Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    nrFreqLockInfo->freqType = (VOS_UINT8)g_atParaList[0].paraValue;

    switch (nrFreqLockInfo->freqType) {
        case MTA_AT_FREQLOCK_TYPE_LOCK_NONE:
            if (g_atParaList[0].paraLen != 1) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_FREQ:
            if (g_atParaIndex != AT_NRFREQLOCK_PARA_NUM_FREQ) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            nrFreqLockInfo->freqType = (VOS_UINT8)g_atParaList[0].paraValue;
            nrFreqLockInfo->scsType  = (VOS_UINT8)g_atParaList[1].paraValue;
            nrFreqLockInfo->band     = (VOS_UINT16)g_atParaList[AT_NRFREQLOCK_BAND].paraValue;
            nrFreqLockInfo->nrArfcn  = (VOS_UINT32)g_atParaList[AT_NRFREQLOCK_NRARFCN].paraValue;

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_CELL:
            if (g_atParaIndex != AT_NRFREQLOCK_PARA_NUM_CELL) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            nrFreqLockInfo->freqType  = (VOS_UINT8)g_atParaList[0].paraValue;
            nrFreqLockInfo->scsType   = (VOS_UINT8)g_atParaList[1].paraValue;
            nrFreqLockInfo->band      = (VOS_UINT16)g_atParaList[AT_NRFREQLOCK_BAND].paraValue;
            nrFreqLockInfo->nrArfcn   = (VOS_UINT32)g_atParaList[AT_NRFREQLOCK_NRARFCN].paraValue;
            nrFreqLockInfo->phyCellId = (VOS_UINT16)g_atParaList[AT_NRFREQLOCK_PHYCELL_ID].paraValue;

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_BAND:
            if (g_atParaIndex != AT_NRFREQLOCK_PARA_NUM_BAND) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            nrFreqLockInfo->freqType = (VOS_UINT8)g_atParaList[0].paraValue;
            nrFreqLockInfo->scsType  = (VOS_UINT8)g_atParaList[1].paraValue;
            nrFreqLockInfo->band     = (VOS_UINT16)g_atParaList[AT_NRFREQLOCK_BAND].paraValue;
            break;
        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}
#endif

#endif


