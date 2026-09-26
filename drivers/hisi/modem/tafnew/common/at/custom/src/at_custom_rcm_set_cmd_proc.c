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
#include "at_custom_rcm_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_RCM_SET_CMD_PROC_C

#if (FEATURE_DCXO_HI1102_SAMPLE_SHARE == FEATURE_ON)
VOS_UINT32 AT_SetDcxoQryPara(VOS_UINT8 indexNum)
{
    VOS_UINT32             rst;
    AT_MTA_DcxoSampeQryReq dcxoSampleQry;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数个数不为1或者字符串长度不为1 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&dcxoSampleQry, sizeof(dcxoSampleQry), 0, sizeof(dcxoSampleQry));
    dcxoSampleQry.dcxoSampleType = g_atParaList[0].paraValue;

    /* 发送跨核消息到C核, 查询DCXO_SAMPLE */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_DCXO_SAMPLE_QRY_REQ, (VOS_UINT8 *)&dcxoSampleQry,
                                 (VOS_SIZE_T)sizeof(AT_MTA_DcxoSampeQryReq), I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryRficDieIDPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DCXOQRY_QRY;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}
#endif

