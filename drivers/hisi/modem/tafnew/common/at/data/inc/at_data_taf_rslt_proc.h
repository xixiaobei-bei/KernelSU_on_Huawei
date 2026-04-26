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

#ifndef _ATDATATAFRSLTPROC_H_
#define _ATDATATAFRSLTPROC_H_

#include "AtParse.h"
#include "at_ctx.h"
#include "si_app_pb.h"

#include "at_cmd_proc.h"
#include "at_ndis_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_RcvTafPsCallEvtCallModifyCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsCallEvtCallAnswerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsCallEvtCallHangupCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosMinInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosMinInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetPdpStateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetPdpIpAddrInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetAnsModeInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetAnsModeInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetPdpStateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtCgactQryCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtCgeqnegTestCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetDynamicDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetPdpDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtSetAuthDataInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetDsFlowInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtClearDsFlowInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtConfigDsFlowRptCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetDynamicSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetDynamicTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetDynamicPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetAuthDataInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetPdpDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtIfaceUpCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtIfaceDownCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_VOID AT_GetIpv6MaskByPrefixLength(VOS_UINT8 localIpv6Prefix, VOS_UINT8 *localIpv6Mask);
TAF_UINT32 At_SetDialGprsPara(TAF_UINT8 indexNum, TAF_UINT8 cid, TAF_GPRS_ActiveTypeUint8 activeType);
VOS_VOID AT_ProcPppDialCnf(TAF_PS_CauseUint32 cause, VOS_UINT8 indexNum);
/*
 * 功能描述: 处理PS_CALL_END_CNF事件
 */
VOS_VOID AT_ModemProcCallEndCnfEvent(TAF_PS_CauseUint32 cause, VOS_UINT8 indexNum);

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_RcvTafPsEvtGetCustomAttachProfileCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetCustomAttachProfileCountCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetAttachProfileSwitchStatusCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetAttachProfileSwitchStatusCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetCustomAttachProfileCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSuccAttachProfileIndexInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafDsambrInfoQryCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtDsambrInfoReportInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
#endif
VOS_UINT32 AT_RcvTafPsEvtGetSinglePdnSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetSinglePdnSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetRoamPdpTypeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetDsFlowNvWriteCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetDsFlowNvWriteCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetApDsFlowRptCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_UINT32 AT_RcvTafPsEvtGetApDsFlowRptCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvTafPsEvtGet5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetDynamic5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSet5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_ChkTafPsEvtGet5gQosInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamic5gQosInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_RcvTafPsEvtUePolicyRptInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtUePolicyRspCheckRsltInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEtherSessCapInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsGet5gUrspRsdInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaEmcPdnActivateCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaEmcPdnDeactivateCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_RcvTafPsEvtSetPsCallRptCmdCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_VOID AT_RcvNdisPdnInfoCfgCnf(AT_NDIS_PdnInfoCfgCnf *ndisPdnInfoCfgCnf);
VOS_VOID AT_RcvNdisPdnInfoRelCnf(AT_NDIS_PdnInfoRelCnf *ndisPdnInfoRelCnf);
/*
 * 功能描述: A核收到ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF事件的处理
 */
VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: A核收到ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_REJ事件的处理
 */
VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateRej(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyRej(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: A核收到ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_CNF事件的处理
 */
VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: A核收到ID_EVT_TAF_PS_CALL_END_CNF事件的处理
 */
VOS_UINT32 AT_RcvTafPsCallEvtCallEndCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: ID_EVT_TAF_PS_GET_D_GPRS_ACTIVE_TYPE_CNF事件处理函数, 用于PPP拨号
 */
VOS_UINT32 AT_RcvTafPsEvtGetGprsActiveTypeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: ID_EVT_TAF_PS_PPP_DIAL_ORIG_CNF事件处理函数, 用于PPP拨号
 */
VOS_UINT32 AT_RcvTafPsEvtPppDialOrigCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetDynamicUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: ID_MSG_TAF_GET_CID_SDF_CNF事件处理函数
 */
VOS_UINT32 AT_RcvTafPsEvtGetCidSdfInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 At_RcvTafPsEvtSetDialModeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_VOID AT_ModemPsRspPdpDeactEvtCnfProc(VOS_UINT8 indexNum, TAF_PS_CallPdpDeactivateCnf *event);
VOS_VOID AT_ModemPsRspPdpActEvtRejProc(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateRej *event);
/*
 * 功能描述: 添加承载IP与RABID的映射
 */
VOS_VOID AT_PS_AddIpAddrMap(VOS_UINT16 clientId, TAF_PS_CallPdpActivateCnf *event);
VOS_VOID AT_AnswerPdpActInd(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateCnf *event);
VOS_VOID AT_ModemPsRspPdpActEvtCnfProc(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateCnf *event);

VOS_UINT32 AT_ProcRabmQryFastDormParaCnf(RABM_AT_QryFastdormParaCnf *msg);
#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_ProcRabmVoicePreferStatusReport(RABM_AT_VoicepreferStatusReport *msg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
