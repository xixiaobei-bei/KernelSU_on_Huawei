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
#ifndef __ATVOICETAFRSLTPROC_H__
#define __ATVOICETAFRSLTPROC_H__

#include "si_app_emat.h"
#include "taf_ps_api.h"
#include "si_app_stk.h"
#include "mn_call_api.h"
#include "AtParse.h"
#include "at_ctx.h"
#include "si_app_pb.h"

#include "at_cmd_proc.h"

#include "taf_ccm_api.h"
#include "taf_call_comm.h"
#include "at_set_para_cmd.h"
#include "taf_msg_chk_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_VOID At_ReportClccDisplayName(MN_CALL_DisplayName *displayName, VOS_UINT16 *length);
VOS_VOID At_ProcQryClccResult(VOS_UINT8 numOfCalls, TAF_CCM_QryCallInfoCnf *qryCallInfoCnf, VOS_UINT8 indexNum);
VOS_UINT32 At_RcvTafCcmCallSupsCmdCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallSupsCmdRsltInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmGetCdurCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmQryClprCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 At_IsCmdCurrentOptSendedOrigReq(AT_CmdCurOptUint32 cmdCurrentOpt);
PS_BOOL_ENUM_UINT8 At_CheckOrigCnfCallType(MN_CALL_TypeUint8 callType, VOS_UINT8 indexNum);
VOS_UINT32 At_RcvTafCcmCallOrigCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEconfDialCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmQryEconfCalledInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 At_ProcQryClccEconfResult(TAF_CCM_QryEconfCalledInfoCnf *callInfos, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvTafCcmCSChannelInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 At_ProcVcSetEcallCfgEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo);
VOS_VOID AT_RcvVcMsgQryEcallCfgCnfProc(MN_AT_IndEvt *data);
VOS_VOID AT_RcvVcMsgSetMsdCnfProc(MN_AT_IndEvt *data);
VOS_VOID AT_RcvVcMsgQryMsdCnfProc(MN_AT_IndEvt *data);
VOS_UINT32 AT_ProcVcEcallAbortCnf(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo);
#endif
VOS_UINT32 AT_RcvMmaQryEmcCallBackCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 atReadLtecsCnfProc(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
#endif
PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdSuccess(VOS_UINT8 ataReportOkAsyncFlag, TAF_UINT8 indexNum);
PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdOthers(TAF_UINT8 indexNum);
VOS_UINT32 AT_ProcMnCallBackCmdCnfApdsCmd(TAF_UINT8 indexNum, TAF_UINT32 errorCode);
VOS_UINT32 AT_ProcMnCallBackCmdCnfCsDataCall(TAF_UINT8 indexNum, TAF_UINT32 errorCode);
VOS_VOID AT_FillClccRptPara(VOS_UINT8 numType, VOS_UINT16 *length, const VOS_UINT8 *asciiNum);
#if (FEATURE_ECALL == FEATURE_ON)
VOS_VOID AT_RcvVcMsgEcallPushCnfProc(MN_AT_IndEvt *data);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
