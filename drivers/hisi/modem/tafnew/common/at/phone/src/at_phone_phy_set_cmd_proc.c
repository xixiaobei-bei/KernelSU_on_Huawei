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

#include "at_phone_phy_set_cmd_proc.h"
#include "securec.h"
#include "taf_app_mma.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_phone_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_PHY_SET_CMD_PROC_C

#define AT_SAMPLE_MAX_PARA_NUM 4
#define AT_SAMPLE_PARA_REQTYPE 0     /* SAMPLE的第一个参数REQTYPE */
#define AT_SAMPLE_PARA_TEMPRANGE 1   /* SAMPLE的第一个参数REQTYPE */
#define AT_SAMPLE_PARA_PPMOFFSET 2   /* SAMPLE的第一个参数REQTYPE */
#define AT_SAMPLE_PARA_TIMEOFFSET 3  /* SAMPLE的第一个参数REQTYPE */
#define AT_TEMP_ZONE_DEFAULT 31 /* 温区默认值 二进制11111 */

VOS_UINT32 AT_GetValidSamplePara(AT_MTA_SetSampleReq *atCmd)
{
    VOS_INT32 temp;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数大于4个，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex > AT_SAMPLE_MAX_PARA_NUM) {
        AT_WARN_LOG("AT_GetValidNetScanPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第1个参数长度为0，返回AT_CME_INCORR ECT_PARAMETERS */
    if (g_atParaList[AT_SAMPLE_PARA_REQTYPE].paraLen == 0) {
        AT_WARN_LOG("AT_GetValidNetScanPara: para0 Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atCmd->reqType = (VOS_UINT16)g_atParaList[AT_SAMPLE_PARA_REQTYPE].paraValue;

    if (g_atParaList[AT_SAMPLE_PARA_TEMPRANGE].paraLen == 0) {
        atCmd->tempRange = AT_TEMP_ZONE_DEFAULT;
    } else {
        atCmd->tempRange = (VOS_UINT16)g_atParaList[AT_SAMPLE_PARA_TEMPRANGE].paraValue;
    }

    if (g_atParaList[AT_SAMPLE_PARA_PPMOFFSET].paraLen == 0) {
        atCmd->ppmOffset = 0;
    } else {
        temp = 0;

        if (AT_AtoInt((VOS_CHAR *)g_atParaList[AT_SAMPLE_PARA_PPMOFFSET].para,
                g_atParaList[AT_SAMPLE_PARA_PPMOFFSET].paraLen, &temp) == VOS_ERR) {
            AT_ERR_LOG("AT_GetValidNetScanPara: para2 err");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        atCmd->ppmOffset = (VOS_INT16)temp;
    }

    if (g_atParaList[AT_SAMPLE_PARA_TIMEOFFSET].paraLen == 0) {
        atCmd->timeOffset = 0;
    } else {
        temp = 0;

        if (AT_AtoInt((VOS_CHAR *)g_atParaList[AT_SAMPLE_PARA_TIMEOFFSET].para,
                g_atParaList[AT_SAMPLE_PARA_TIMEOFFSET].paraLen, &temp) == VOS_ERR) {
            AT_ERR_LOG("AT_GetValidNetScanPara: para3 err");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        atCmd->timeOffset = (VOS_INT16)temp;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetSamplePara(VOS_UINT8 indexNum)
{
    AT_MTA_SetSampleReq atCmd;
    VOS_UINT32          result;
    ModemIdUint16       modemId;
    VOS_UINT32          ret;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));

    modemId = MODEM_ID_0;
    ret     = AT_GetModemIdFromClient(indexNum, &modemId);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_SetSamplePara: Get modem id fail.");
        return AT_ERROR;
    }

    if (modemId != MODEM_ID_0) {
        AT_ERR_LOG("AT_SetSamplePara: MODEM_ID ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ret = AT_GetValidSamplePara(&atCmd);

    if (ret != AT_OK) {
        AT_ERR_LOG("AT_SetSamplePara: AT_GetValidNetScanPara Failed");
        return ret;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_SAMPLE_REQ, (VOS_UINT8 *)&atCmd,
                                    sizeof(AT_MTA_SetSampleReq), I0_UEPS_PID_MTA);

    if (result != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetSamplePara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SAMPLE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

