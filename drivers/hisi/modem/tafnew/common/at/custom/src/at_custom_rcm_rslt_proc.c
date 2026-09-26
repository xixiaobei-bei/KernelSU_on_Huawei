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
#include "at_custom_rcm_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_RCM_RSLT_PROC_C

#if (FEATURE_DCXO_HI1102_SAMPLE_SHARE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaRcmDcxoSampleQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg                = VOS_NULL_PTR;
    MTA_AT_DcxoSampeQryCnf *mtaAtDcxoSampleQryCnf = VOS_NULL_PTR;
    VOS_UINT16              dataLen;
    VOS_UINT32              result;
    VOS_UINT32              i;

    /* ³õÊ¼»¯ */
    rcvMsg                = (AT_MTA_Msg *)msg;
    mtaAtDcxoSampleQryCnf = (MTA_AT_DcxoSampeQryCnf *)rcvMsg->content;
    dataLen               = 0;
    i                     = 0;
    result                = AT_OK;

    /* ´òÓ¡^PHYCOMACK: */
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,", mtaAtDcxoSampleQryCnf->dcxoSampleType);

    if (mtaAtDcxoSampleQryCnf->result == MTA_AT_DCXO_SAMPLE_RESULT_NO_ERROR) {
        switch (mtaAtDcxoSampleQryCnf->dcxoSampleType) {
            case AT_MTA_DCXO_SAMPLE_H:
            case AT_MTA_DCXO_SAMPLE_M:
            case AT_MTA_DCXO_SAMPLE_L:
                dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,", mtaAtDcxoSampleQryCnf->result);
                for (i = 0; i < UCOM_NV_DCXO_SAMPLE_H_MAX_NUM; i++) {
                    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,",
                        mtaAtDcxoSampleQryCnf->sctionData.sample[i].swTime);

                    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,",
                        mtaAtDcxoSampleQryCnf->sctionData.sample[i].swMode);

                    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,",
                        mtaAtDcxoSampleQryCnf->sctionData.sample[i].swTemp);
                    if (i == (UCOM_NV_DCXO_SAMPLE_H_MAX_NUM - 1)) {
                        dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                            (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d",
                            mtaAtDcxoSampleQryCnf->sctionData.sample[i].swPpm);
                    } else {
                        dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                            (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,",
                            mtaAtDcxoSampleQryCnf->sctionData.sample[i].swPpm);
                    }
                }
                break;
            case AT_MTA_DCXO_SAMPLE_INITFREQ:
                dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,", mtaAtDcxoSampleQryCnf->result);
                dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d,",
                    mtaAtDcxoSampleQryCnf->sctionData.dcxoInitPpm.swInitFrequency);

                dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d",
                    mtaAtDcxoSampleQryCnf->sctionData.dcxoInitPpm.swInitTemperature);
                break;
            default:
                mtaAtDcxoSampleQryCnf->result = MTA_AT_DCXO_SAMPLE_RESULT_ERROR;
                dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d", mtaAtDcxoSampleQryCnf->result);
                break;
        }
    } else {
        dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%d", mtaAtDcxoSampleQryCnf->result);
    }

    g_atSendDataBuff.bufLen = dataLen;

    return result;
}

#endif

