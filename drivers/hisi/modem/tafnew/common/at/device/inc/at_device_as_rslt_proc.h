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
#ifndef _AT_DEVICE_AS_RSLT_PROC_H_
#define _AT_DEVICE_AS_RSLT_PROC_H_

#include "vos.h"
#include "taf_type_def.h"

#include "at_mta_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_FEATURE_NAME_LEN_MAX 16
#define AT_FEATURE_CONTENT_LEN_MAX 504

typedef struct {
    VOS_UINT8 featureFlag;
    VOS_UINT8 TAF_COMM_ATTRIBUTE_ALIGNED4 featureName[AT_FEATURE_NAME_LEN_MAX];
    VOS_UINT8 TAF_COMM_ATTRIBUTE_ALIGNED4 content[AT_FEATURE_CONTENT_LEN_MAX];
} AT_FEATURE_SUPPORT_ST;

VOS_UINT32 AT_GetWcdmaBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen, AT_UE_BandCapaSt *bandCapa);
VOS_UINT32 AT_GetWcdmaDivBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_GetLteFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure);
#endif
#if (FEATURE_LTEV == FEATURE_ON)
VOS_VOID AT_GetLtevFeatureInfo(AT_FEATURE_SUPPORT_ST *featureSupport, MTA_AT_SfeatureQryCnf *featureQryCnf);
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
extern VOS_UINT32 AT_GetTdsFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
extern VOS_VOID AT_GetNrFeatureInfo(MTA_NRRC_BandInfo nrBandInfo, AT_FEATURE_SUPPORT_ST *feATure);
#endif

VOS_UINT32 AT_GetGsmBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen, AT_UE_BandCapaSt *bandCapa);

VOS_UINT32 AT_RcvMtaQrySfeatureRsp(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaQryNmrCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaQryTrxTasCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvBbicCalSetTrxTasCnf(struct MsgCB *msg);
#endif


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
