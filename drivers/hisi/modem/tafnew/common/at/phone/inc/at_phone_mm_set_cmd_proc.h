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
#ifndef __ATPHONEMMSETCMDPROC_H__
#define __ATPHONEMMSETCMDPROC_H__

#include "taf_type_def.h"
#include "AtParse.h"
#include "at_mta_interface.h"
#include "taf_app_mma.h"
#include "at_set_para_cmd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

typedef VOS_UINT8  AT_SyscfgexRatTypeUint8;

typedef struct {
    VOS_UINT8               ratOrderNum;                          /* syscfgex中设置的acqoder中的指示接入技术的个数 */
    AT_SyscfgexRatTypeUint8 ratOrder[TAF_PH_MAX_SUPPORT_RAT_NUM]; /* at^syscfgex中设置的acqoder接入优先级 */
    VOS_UINT8               rsv;
} AT_SyscfgexRatOrder;

TAF_UINT32 At_SetCfunPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetCopsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCgattPara(TAF_UINT8 indexNum);
VOS_UINT32 At_CheckRepeatedRatOrder(AT_SyscfgexRatOrder *syscfgExRatOrder);
VOS_UINT32 At_CheckSysCfgExRatTypeValid(VOS_UINT8 ratOrder);
VOS_UINT32 AT_ConvertSysCfgExGuBandPara(VOS_UINT8 *para, VOS_UINT16 len, TAF_UserSetPrefBand64 *prefBandPara);
VOS_UINT32 AT_ConvertSysCfgExLteBandPara(VOS_UINT8 *para, VOS_UINT16 len, TAF_USER_SetLtePrefBandInfo *prefBandPara);
VOS_UINT32 AT_SetSysCfgExPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetSystemInfoEx(TAF_UINT8 indexNum);

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetCpamPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetSysCfgPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetSystemInfo(TAF_UINT8 indexNum);
#endif
TAF_UINT32 At_SetCgcattPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetSrvstPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetFplmnPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetModePara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCplsPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_GetOperatorNameFromParam(VOS_UINT16 *operNameLen, VOS_CHAR *operName, VOS_UINT16 bufSize,
                                       VOS_UINT32 formatType);

TAF_UINT32 At_SetCpolPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetQuickStart(TAF_UINT8 indexNum);
TAF_MMA_ServiceDomainUint8 At_ConvertDetachTypeToServiceDomain(VOS_UINT32 cgcattMode);
TAF_MMA_AttachTypeUint8 At_ConvertCgcattModeToAttachType(VOS_UINT32 cgcattMode);
VOS_UINT32 AT_SetCopnPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNetScanPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEmRssiCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCesqPara(TAF_UINT8 indexNum);
AT_SYSCFG_RatPrioUint8 AT_GetGuAccessPrioNvim(ModemIdUint16 modemId);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_InputValueTransToVoiceDomain(VOS_UINT32 value, TAF_MMA_VoiceDomainUint32 *voiceDomain);
VOS_UINT32 AT_SetCevdpPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetCindPara(VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
