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
#ifndef _AT_CUSTOM_AS_RSLT_PROC_H_
#define _AT_CUSTOM_AS_RSLT_PROC_H_

#include "vos.h"
#include "at_mta_interface.h"
#include "at_ctx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
VOS_UINT32 AT_RcvMtaEcidSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_VOID   AT_NetMonFmtPlmnId(VOS_UINT32 mcc, VOS_UINT32 mnc, VOS_CHAR *pstrPlmn, VOS_UINT16 *length);
VOS_VOID   AT_NetMonFmtGsmSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length);
VOS_VOID   AT_NetMonFmtUtranFddSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length);
VOS_VOID   AT_NetMonFmtGsmNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen);
VOS_VOID   AT_NetMonFmtUtranFddNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen);

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
VOS_VOID AT_NetMonFmtUtranTddSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length);
VOS_VOID AT_NetMonFmtUtranTddNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen);
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_VOID AT_NetMonFmtEutranSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length);
VOS_VOID AT_NetMonFmtEutranNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_NrCellBandFmtDataSa(MTA_AT_NetmonCellInfo *cellInfo, VOS_UINT16 *outLen);
VOS_VOID AT_NrCellBandFmtDataNsa(MTA_AT_NetmonCellInfo *cellInfo, VOS_UINT16 *outLen);
VOS_VOID AT_NetMonFmtNrSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length);
VOS_VOID AT_NetMonFmtNrSSCellData(MTA_AT_NetmonCellInfo *cellInfo, VOS_UINT16 *outLen);
VOS_VOID AT_NetMonFmtNrNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen);
#endif

VOS_UINT32 AT_RcvMtaSetNetMonSCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaSetNetMonNCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaSetNetMonSSCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaSetNrCellBandCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaErrcCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaEccCfgCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaRsInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID AT_FormatRsrp(VOS_UINT8 indexNum, MTA_AT_RsInfoQryCnf *rsInfoQryCnf);
VOS_VOID AT_FormatRsrq(VOS_UINT8 indexNum, MTA_AT_RsInfoQryCnf *rsInfoQryCnf);
#endif

VOS_UINT32 AT_RcvMtaPseudBtsSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_UINT32 AT_RcvMtaSubClfSparamQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaOverHeatingQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
extern VOS_UINT32 AT_RcvMtaNrrcCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrCaCellInfoRptCfgSetCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvMtaNrNwCapInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrNwCapInfoRptCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrCaCellInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrCaCellInfoRptCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrPowerSavingQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNrSsbIdQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID AT_FmtNrSsbIdData(MTA_AT_NrSsbIdQryCnf *ssbIdInfo, VOS_UINT16 *outLen);
#endif

VOS_UINT32 AT_RcvMtaQryNCellMonitorCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaAnqueryQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 atScellInfoQryCnfProc(struct MsgCB *msgBlock);
VOS_UINT32 AT_RcvMtaWrrRrcVersionSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaWrrRrcVersionQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_VOID AT_QryParaAnQueryProc(VOS_UINT8 indexNum, VOS_UINT8 opId, const VOS_UINT8 *para);
AT_CmdAntennaLevelUint8 AT_CalculateAntennaLevel(VOS_INT16 rscp, VOS_INT16 ecio);
VOS_VOID                AT_GetSmoothAntennaLevel(VOS_UINT8 indexNum, AT_CmdAntennaLevelUint8 level);
VOS_UINT32 atSetAnlevelCnfSameProc(struct MsgCB *msgBlock);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
