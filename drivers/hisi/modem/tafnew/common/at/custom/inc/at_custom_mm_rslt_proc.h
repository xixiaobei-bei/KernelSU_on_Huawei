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
#ifndef _AT_CUSTOM_MM_RSLT_PROC_H_
#define _AT_CUSTOM_MM_RSLT_PROC_H_

#include "vos.h"
#include "taf_app_mma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
VOS_UINT32 AT_QryParaHomePlmnProc(VOS_UINT8 indexNum, VOS_UINT8 opId, const VOS_UINT8 *para);
VOS_UINT32 AT_RcvMmaApHplmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID   AT_ConvertImsiDigit2String(VOS_UINT8 *imsi, VOS_UINT8 *imsiString, VOS_UINT32 strBufLen);
VOS_UINT32 AT_RcvMmaHplmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaDplmnSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaDplmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaBorderInfoSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaBorderInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvFratIgnitionQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaEmRssiRptQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaPsSceneQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvSmsAntiAttackCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafMmaAutoAttachSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaAutoAttachQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaImsSwitchSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaImsSwitchQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaRcsSwitchQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_RcvMmaPwrOnAndRegTimeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_CSG == FEATURE_ON)
VOS_UINT32 AT_RcvMmaCsgListSearchCnfProc(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaQryCampCsgIdInfoCnfProc(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMmaCmmSetCmdRsp(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaUlFreqRptQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_RcvMmaEonsInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaHcsqInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaNetSelOptSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT16 AT_FormatHcsqQryResult(TafMmaHcsqQryResponsePara *hcsqResponsePara);
VOS_VOID AT_FormatHcsqResult(VOS_UINT16 *length, TAF_PH_Rssi *signalInfo, VOS_UINT32 rpt);
VOS_VOID AT_QryParaRspCellRoamProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para);
#endif
VOS_UINT32 AT_RcvMmaCarOosGearsQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
