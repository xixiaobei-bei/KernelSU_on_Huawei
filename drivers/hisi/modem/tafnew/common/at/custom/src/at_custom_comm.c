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
#include "at_custom_comm.h"
#include "securec.h"
#include "nv_stru_msp_interface.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_COMM_C

VOS_UINT32 AT_GetGsmBandCapa(VOS_UINT32 *gBand)
{
    AT_NvWgRfMainBand wGBand;

    (VOS_VOID)memset_s(&wGBand, sizeof(wGBand), 0x00, sizeof(wGBand));
    *gBand = 0;

    /* 从NV项中读取单板支持的频段 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_WG_RF_MAIN_BAND, &wGBand, sizeof(wGBand)) != NV_OK) {
        AT_ERR_LOG("AT_GetGsmBandCapa: Read NV fail!");
        return VOS_ERR;
    }

    if (wGBand.unGsmBand.bitBand.bandGsm850 == VOS_TRUE) {
        *gBand |= AT_BODY_SAR_GBAND_GPRS_850_MASK;
        *gBand |= AT_BODY_SAR_GBAND_EDGE_850_MASK;
    }

    if ((wGBand.unGsmBand.bitBand.bandGsmP900 == VOS_TRUE) || (wGBand.unGsmBand.bitBand.bandGsmR900 == VOS_TRUE) ||
        (wGBand.unGsmBand.bitBand.bandGsmE900 == VOS_TRUE)) {
        *gBand |= AT_BODY_SAR_GBAND_GPRS_900_MASK;
        *gBand |= AT_BODY_SAR_GBAND_EDGE_900_MASK;
    }

    if (wGBand.unGsmBand.bitBand.bandGsm1800 == VOS_TRUE) {
        *gBand |= AT_BODY_SAR_GBAND_GPRS_1800_MASK;
        *gBand |= AT_BODY_SAR_GBAND_EDGE_1800_MASK;
    }

    if (wGBand.unGsmBand.bitBand.bandGsm1900 == VOS_TRUE) {
        *gBand |= AT_BODY_SAR_GBAND_GPRS_1900_MASK;
        *gBand |= AT_BODY_SAR_GBAND_EDGE_1900_MASK;
    }

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_ConvertNwDnnToString(VOS_UINT8 *inputDnn, VOS_UINT8 inputLen, VOS_UINT8 *outputDnn, VOS_UINT8 outputLen)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    VOS_UINT8 labelLen;

    if ((inputLen < PS_MAX_APN_LEN) || (outputLen < PS_MAX_APN_LEN)) {
        return;
    }

    labelLen = inputDnn[1]; /* 第1个label的长度 */
    for (i = 1; (i < inputDnn[0]) && (i < PS_MAX_APN_LEN); i++) {
        /* 从第1个有效字符开始检查 */
        if (j < labelLen) {
            /* 拷贝labelLen个字符 */
            outputDnn[i] = inputDnn[i + 1];
            j++; /* 有效字符数增1 */
        } else {
            outputDnn[i] = '.';             /* 拷贝一个Label完毕后，追加一个'.'号 */
            j            = 0;               /* 开始下一个Label的长度累计 */
            labelLen     = inputDnn[i + 1]; /* 取下一个label的长度 */
        }
    }
    outputDnn[0] = (VOS_UINT8)(i - 1); /* 字符串的长度 */
}

VOS_UINT16 AT_PrintLadnDnn(VOS_UINT8 dnnNum, VOS_UINT8 dnnList[][PS_MAX_APN_LEN], VOS_UINT16 length)
{
    VOS_UINT32 i;
    VOS_UINT16 len;

    len = length;

    if (dnnNum == 0) {
        /* ,<ladn_dnn_num> */
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%d,", dnnNum);
        /* ,<dnn_list> */
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "\"\"");
    } else {
        /* ,<ladn_dnn_num> */
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%d,", dnnNum);
        /* ,<dnn_list> */
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "\"");

        for (i = 0; i < dnnNum; i++) {
            if (i != dnnNum - 1) {
                len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + len, "%s;", dnnList[i]);
            } else if (i == dnnNum - 1) {
                len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + len, "%s\"", dnnList[i]);
            }
        }
    }
    return len;
}
#endif

