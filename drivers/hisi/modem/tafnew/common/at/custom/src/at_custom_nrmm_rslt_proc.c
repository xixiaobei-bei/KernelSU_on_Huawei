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
#include "at_custom_nrmm_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_NRMM_RSLT_PROC_C

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMta5gOptionQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg = VOS_NULL_PTR;
    MTA_AT_Qry5GOptionCnf *mtaCnf = VOS_NULL_PTR;
    VOS_UINT32             result;

    /* ³õÊ¼»¯ */
    rcvMsg   = (AT_MTA_Msg *)msg;
    mtaCnf   = (MTA_AT_Qry5GOptionCnf *)rcvMsg->content;
    result   = AT_OK;
    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->rslt != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                mtaCnf->nrSaSupportFlag, mtaCnf->nrDcMode, mtaCnf->en5gcAccessMode);
    }

    return result;
}

VOS_UINT32 AT_RcvMmaNrBandBlockListQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_NrBandBlockListQryCnf *rcvMsg = (TAF_MMA_NrBandBlockListQryCnf *)msg;
    VOS_UINT16                     length;
    VOS_UINT32                     bandNum;
    VOS_UINT32                     i;

    g_atSendDataBuff.bufLen = 0;

    if (rcvMsg->result != TAF_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    bandNum = (VOS_UINT32)AT_MIN(rcvMsg->bandNum, TAF_MMA_MAX_NR_BAND_BLOCKLIST_NUM);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, rcvMsg->fr2SupportSw,
        bandNum);

    for (i = 0; i < bandNum; i++) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", rcvMsg->bandList[i]);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}
#endif

