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
#ifndef _AT_DEVICE_TAF_QRY_CMD_PROC_H_
#define _AT_DEVICE_TAF_QRY_CMD_PROC_H_

#include "vos.h"
#include "taf_app_mma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#define USIMM_OPEN_SPEEDENABLE 0x01000101
#define USIMM_OPEN_SPEEDDISABLE 0x00000000

#define AT_QOS_TRAFFIC_CLASS_SUBSCRIBE 0

VOS_UINT32 At_QryBsn(VOS_UINT8 indexNum);
VOS_UINT32 At_QryPlatForm(VOS_UINT8 indexNum);
VOS_UINT32 At_TestTmodePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryTmmiPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryProdNamePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEqverPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryDataLock(VOS_UINT8 indexNum);
VOS_UINT32 At_QrySimLockPlmnInfo(VOS_UINT8 indexNum);
VOS_UINT32 At_QryMaxLockTimes(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCallSrvPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMDatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryFacInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_GetImeiValue(ModemIdUint16 modemId, VOS_UINT8 imei[TAF_PH_IMEI_LEN + 1], VOS_UINT8 emeiMaxLength);
VOS_UINT32 AT_QryPhyNumPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCsVer(VOS_UINT8 indexNum);
VOS_UINT32 AT_DeciDigit2Ascii(VOS_UINT8 aucDeciDigit[], VOS_UINT32 length, VOS_UINT8 aucAscii[]);
VOS_UINT32 At_QryQosPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRsimPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiwepPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCmdlenPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryNvResumePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestWiwepPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestRsfwPara(VOS_UINT8 indexNum);
TAF_UINT32 At_QryIccidPara(TAF_UINT8 indexNum);
VOS_UINT32 atQryFLNAPara(VOS_UINT8 clientId);
#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_QryUartTest(VOS_UINT8 indexId);
VOS_UINT32 AT_QryI2sTest(VOS_UINT8 indexId);
#endif

VOS_UINT32 At_QryFpaPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryFAgcgainPara(VOS_UINT8 indexNum);
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 AT_QryFDac(VOS_UINT8 indexNum);
VOS_UINT32 At_TestFdacPara(VOS_UINT8 indexNum);
#else
#endif

#if (FEATURE_LTE == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_QrySfm(VOS_UINT8 indexNum);
#endif
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
