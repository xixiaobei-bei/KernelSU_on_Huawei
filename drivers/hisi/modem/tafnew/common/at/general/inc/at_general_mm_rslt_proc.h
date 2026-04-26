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

#ifndef _ATGENERALMMRSLTPROC_H_
#define _ATGENERALMMRSLTPROC_H_

#include "vos.h"
#include "taf_app_mma.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_RcvMmaHsQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_ReportWs46QryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafMmaSyscfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCerssiInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaSysCfgSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaEOPlmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID AT_GetOnlyGURatOrder(TAF_MMA_MultimodeRatCfg *ratOrder);
VOS_VOID AT_ReportSysCfgExQryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum);
VOS_VOID AT_ReportSysCfgQryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum);
VOS_UINT32 AT_ProcCesqInfoSet(struct MsgCB *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_ProcCerssiInfoQuery(struct MsgCB *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_IsSupportGMode(TAF_MMA_MultimodeRatCfg *ratOrder);
VOS_UINT32 AT_IsSupportWMode(TAF_MMA_MultimodeRatCfg *ratOrder);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_ProcCserssiInfoQuery(struct MsgCB *msg, VOS_UINT8 indexNum);
VOS_VOID AT_ConvertMultiRejectSNssaiToString(VOS_UINT8 sNssaiNum, PS_REJECTED_SNssai *sNssai, VOS_CHAR *pcStrNssai,
                                             VOS_UINT32 srcNssaiLength, VOS_UINT32 *dsrLength);
#endif
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_IsSupport1XMode(TAF_MMA_MultimodeRatCfg *ratOrder);
VOS_UINT32 AT_IsSupportHrpdMode(TAF_MMA_MultimodeRatCfg *ratOrder);
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_VOID AT_SysCfgexQryRoamParaConvert(VOS_UINT8 clientId, VOS_UINT8 *outRoam);
#endif
VOS_VOID AT_ConvertSysCfgStrToAutoModeStr(VOS_UINT8 *acqOrderBegin, VOS_UINT8 *acqOrder,
                                                VOS_UINT32 acqOrderLength, VOS_UINT8 ratNum);
VOS_UINT32 atCerssiInfoCnfProc(struct MsgCB *msgBlock);
VOS_INT16 AT_ConvertCerssiRscpToCesqRscp(VOS_INT16 cerssiRscp);
VOS_INT16 AT_ConvertCerssiRsrp(VOS_INT16 cerssiRsrp);
VOS_INT16 AT_ConvertCerssiRsrq(VOS_INT16 cerssiRsrq);
VOS_INT16 AT_ConvertCerssiRsrpToCesqRsrp(VOS_INT16 cerssiRsrp);
VOS_INT16 AT_ConvertCerssiRsrqToCesqRsrq(VOS_INT16 cerssiRsrq);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
