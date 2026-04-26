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
#ifndef _AT_CUSTOM_EVENT_REPORT_H_
#define _AT_CUSTOM_EVENT_REPORT_H_

#include "vos.h"
#include "taf_app_mma.h"
#ifdef MODEM_FUSION_VERSION
#include "observer.h"
#else
#include "observer_at.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 atLwclashInd(struct MsgCB *msgBlock);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaSibFineTimeNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaLppFineTimeNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaLrrcUeCapNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrrcUeCapNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_LADN == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLadnInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
#endif

VOS_UINT32 AT_RcvMtaAfcClkUnlockCauseInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaBestFreqInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaCposrInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTempprtStatusInd(struct MsgCB *msg);
VOS_UINT32 AT_ProcTempprtEventInd(TEMP_PROTECT_EventAtInd *msg);
VOS_UINT32 AT_RcvMtaRefclkfreqInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID At_RcvMmaPsInitResultIndProc(TAF_UINT8 indexNum, TAF_PHONE_EventInfo *event);
VOS_UINT32 AT_RcvMtaEpduDataInd(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaEccStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaImpuInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaCallAltSrvInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaRatHandoverInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvSrvStatusUpdateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_IsImsRegErrRptParaValid(struct MsgCB *msg);
VOS_UINT32 AT_RcvImsaRegErrRptInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMtaTempProtectInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaJamDetectInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaPhyCommAckInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaPlmnSelectInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvCssMccNotify(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvMtaCrrconnStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMtaRlQualityInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMtaVideoDiagInfoRpt(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvCssBlockCellMccNotify(struct MsgCB *msg);
VOS_UINT32 AT_RcvMmaElevatorStateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaPseudBtsIdentInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaPseudBtsIdentInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaUlFreqChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_DSDS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaDsdsStateNotify(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
VOS_UINT32 AT_RcvThortSingleApnBackoffTimeParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
VOS_UINT32 AT_RcvThortSingleSnssaiBackoffTimerParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvThortSingleSnssaiDnnBackoffTimerParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 At_PlmnDetectIndProc(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaRrcStatInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLteCaCellExInfoNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrCaCellInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrNwCapInfoReportInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMtaHfreqinfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaFastReturn5gEndcInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMtaNvRefreshNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if ((FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON))
VOS_UINT32 AT_RcvMtaHsrcellInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif /* (FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON) */

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaPendingNssaiInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMmaMtReattachInd(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLteCategoryInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

extern VOS_UINT32 AT_RcvMmaSrchedPlmnInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLendcInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaSib16TimeUpdateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaAccessStratumRelInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvMtaXpassInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaPsTransferInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaMipiInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNwSliceAuthCmdInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNwSliceAuthRsltInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_RcvMtaNCellMonitorInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvAtMmaUsimStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMtaXcposrRptInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvCssCloudDataReport(struct MsgCB *msg);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_VOID AT_RptHcsqInfo(VOS_UINT8 index, TAF_MMA_RssiInfoInd *rssiInfo);
VOS_UINT32 AT_RcvMmaNwNameChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaNetSelOptInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
