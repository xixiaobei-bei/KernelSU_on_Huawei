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
#ifndef _AT_CUSTOM_COMM_H_
#define _AT_CUSTOM_COMM_H_

#include "vos.h"
#include "at_mta_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_NETMON_PLMN_STRING_MAX_LENGTH 10
#define AT_NETMON_GSM_RX_QUALITY_INVALID_VALUE 99
#define AT_NETMON_GSM_RSSI_INVALID_VALUE (-500)
#define AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE 0
#define AT_NETMON_UTRAN_FDD_RSSI_INVALID_VALUE 0
#define AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE 0

#define AT_PSEUD_BTS_PARAM_TIMES 2  /* 查询伪基站拦截次数 */

/* LOG3.5的端口定义: USB */
#define AT_LOG_PORT_USB 0

/* LOG3.5的端口定义: VCOM */
#define AT_LOG_PORT_VCOM 1
#if (FEATURE_MBB_CUST == FEATURE_ON)
#define AT_LOG_PORT_WIFI 2
#endif

#define AT_BODY_SAR_GBAND_GPRS_850_MASK 0x00000001
#define AT_BODY_SAR_GBAND_GPRS_900_MASK 0x00000002
#define AT_BODY_SAR_GBAND_GPRS_1800_MASK 0x00000004
#define AT_BODY_SAR_GBAND_GPRS_1900_MASK 0x00000008
#define AT_BODY_SAR_GBAND_EDGE_850_MASK 0x00010000
#define AT_BODY_SAR_GBAND_EDGE_900_MASK 0x00020000
#define AT_BODY_SAR_GBAND_EDGE_1800_MASK 0x00040000
#define AT_BODY_SAR_GBAND_EDGE_1900_MASK 0x00080000

#define HFREQ_INFO_RAT_LTE 6
#define HFREQ_INFO_RAT_NR 7

typedef struct {
    AT_WCDMA_BAND_SET_UN unWcdmaBand; /* 记录W频段信息 */
    VOS_UINT32           rsv;
    AT_GSM_BAND_SET_UN   unGsmBand; /* 记录G频段信息 */
} AT_NvWgRfMainBand;

VOS_UINT32 AT_GetGsmBandCapa(VOS_UINT32 *gBand);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID   AT_ConvertNwDnnToString(VOS_UINT8 *inputDnn, VOS_UINT8 inputLen, VOS_UINT8 *outputDnn, VOS_UINT8 outputLen);
VOS_UINT16 AT_PrintLadnDnn(VOS_UINT8 dnnNum, VOS_UINT8 dnnList[][PS_MAX_APN_LEN], VOS_UINT16 length);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
