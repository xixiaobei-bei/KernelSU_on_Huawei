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
#include "at_custom_hifi_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "app_vc_api.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_HIFI_RSLT_PROC_C

VOS_VOID At_RcvVcMsgSetGroundCnfProc(MN_AT_IndEvt *data)
{
    VOS_UINT8  indexNum = 0;
    VOS_UINT32 ret;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_CBG_SET, &indexNum) != VOS_OK) {
        return;
    }

    if (data->content[0] == VOS_OK) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_VOID At_RcvVcMsgQryGroundRspProc(MN_AT_IndEvt *data)
{
    APP_VC_QryGroungRsp *qryRslt  = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;
    VOS_UINT32           ret;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_CBG_READ, &indexNum) != VOS_OK) {
        return;
    }

    /* 初始化 */
    qryRslt = (APP_VC_QryGroungRsp *)data->content;

    if (qryRslt->qryRslt == VOS_OK) {
        /* 输出查询结果 */
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "^CBG:%d", qryRslt->ground);
        ret                     = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

