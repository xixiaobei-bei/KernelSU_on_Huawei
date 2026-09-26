/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __TAFLOGPRIVACYMATCH_H__
#define __TAFLOGPRIVACYMATCH_H__

#include "vos.h"
#include "PsTypeDef.h"
#include "taf_ssa_api.h"
#include "at_internal_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)


enum TAF_LogPrivacyAtCmd {
    TAF_LOG_PRIVACY_AT_CMD_BUTT = 0,

    TAF_LOG_PRIVACY_AT_CMD_CIMI  = 1,
    TAF_LOG_PRIVACY_AT_CMD_CGSN  = 2,
    TAF_LOG_PRIVACY_AT_CMD_MSID  = 3,
    TAF_LOG_PRIVACY_AT_CMD_HPLMN = 4,
};
typedef VOS_UINT32 TAF_LogPrivacyAtCmdUint32;


typedef VOS_VOID* (*pTafLogPrivacyMatchFunc)(MsgBlock *pstMsg);


typedef struct {
    VOS_UINT32              msgName;          /* 消息类型 */
    pTafLogPrivacyMatchFunc funcPrivacyMatch; /* 消息对应的过滤函数 */
} TAF_LogPrivacyMsgMatchTbl;


typedef struct {
    VOS_UINT32                       receiverPid;           /* 消息接收PID */
    VOS_UINT32                       tblSize;               /* 消息过滤函数匹配处理表大小 */
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMatchMsgTbl; /* 消息过滤函数匹配处理表 */
} TAF_LogPrivacyRcvPidMatchTbl;


typedef struct {
    VOS_UINT32 modem0Pid;
    VOS_UINT32 modem1Pid;
    VOS_UINT32 modem2Pid;
} TAF_LogPrivacyMatchModemPidMapTbl;


typedef struct {
    VOS_CHAR *originalAtCmd;
    VOS_CHAR *privacyAtCmd;
} AT_LogPrivacyMatchAtCmdMapTbl;

typedef AT_Msg* (*AT_PRIVACY_FILTER_AT_PROC_FUNC)(AT_Msg *pstAtMsg);


typedef struct {
    VOS_UINT32                     atCmdType;
    AT_PRIVACY_FILTER_AT_PROC_FUNC privacyAtCmd;
} AT_LogPrivacyMapCmdToFunc;


typedef VOS_VOID (*pTafIntraMsgPrivacyMatchFunc)(MsgBlock *pstMsg);


typedef struct {
    VOS_UINT32                   msgName;                     /* 消息类型 */
    pTafIntraMsgPrivacyMatchFunc tafIntraMsgPrivacyMatchFunc; /* 消息对应的过滤函数 */
} TAF_LogPrivacyIntraMsgMatchTbl;

typedef VOS_VOID *(*TAF_LAYER_MSG_LOG_PRIVACY_PROC_FUNC)(struct MsgCB *pMsg);


typedef struct {
    VOS_UINT32                          senderPid;
    TAF_LAYER_MSG_LOG_PRIVACY_PROC_FUNC procFunc;
}TAF_ExtLayerMsgLogPrivacyProc;
VOS_UINT32 TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(TAF_SSA_EvtIdUint32 evtId);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* AT_PrivacyMatchAppMsgTypeSendReq(MsgBlock *msg);
#endif

VOS_VOID* AT_PrivacyMatchCposSetReq(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchSimLockWriteExSetReq(MsgBlock *msg);

#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID* AT_PrivacyMatchImsaImsCtrlMsg(MsgBlock *msg);
VOS_VOID* AT_PrivacyMatchImsaNickNameSetReq(MsgBlock *msg);
#endif

VOS_VOID* AT_PrivacyMatchMeidSetReq(MsgBlock *msg);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* AT_PrivacyMatchAppMsgTypeWriteReq(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchAppMsgTypeDeleteReq(MsgBlock *msg);
#endif

VOS_VOID* TAF_PrivacyMatchAppMnCallBackCsCall(MsgBlock *msg);
VOS_VOID* AT_PrivacyMatchRegisterSsMsg(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchProcessUssMsg(MsgBlock *msg);
VOS_VOID* AT_PrivacyMatchInterRogateMsg(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchErasessMsg(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchActivatessMsg(MsgBlock *msg);
VOS_VOID* AT_PrivacyMatchDeactivatessMsg(MsgBlock *msg);
VOS_VOID* AT_PrivacyMatchRegPwdMsg(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchMnCallBackSsAtIndEvt(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchAppMnCallBackSs(MsgBlock *msg);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(MsgBlock *msg);

VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf(MsgBlock *msg);

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsEphInfoInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtApReverseDataInd(MsgBlock *msg);

VOS_VOID* TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchCagpsPrmInfoRsp(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(MsgBlock *msg);
#endif
#endif

VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(MsgBlock *msg);

VOS_VOID* TAF_MTA_PrivacyMatchAtMeidQryCnf(MsgBlock *msg);

VOS_VOID* TAF_MTA_PrivacyMatchAtCgsnQryCnf(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchAtCallBackQryProc(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtUsimStatusInd(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtHomePlmnQryCnf(MsgBlock *msg);

VOS_VOID* TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf(MsgBlock *msg);

VOS_VOID* AT_PrivacyMatchSmsAppMsgReq(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchAtCallBackSmsProc(MsgBlock *msg);

VOS_VOID* TAF_MTA_PrivacyMatchAtEcidSetCnf(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoSetReq(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf(MsgBlock *msg);

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_VOID* TAF_MMA_PrivacySubnetAdapterInfoInd(MsgBlock *msg);
#endif
VOS_VOID* TAF_CCM_PrivacyMatchCallOrigReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCallSupsCmdReq(MsgBlock *msg);

VOS_VOID* TAF_CCM_PrivacyMatchCustomDialReqCmdReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmStartDtmfReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmStopDtmfReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSetUusInfoReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmCustomEccNumReq(MsgBlock *msg);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendFlashReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendContDtmfReq(MsgBlock *msg);
VOS_VOID* TAF_CCM_PrivacyMatchCcmEncryptVoiceReq(MsgBlock *msg);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_VOID* TAF_CCM_PrivacyMatchCcmSetEcKmcReq(MsgBlock *msg);
#endif
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID* TAF_PrivacyMatchAppQryEconfCalledInfoCnf(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtEconfNotifyInd(MsgBlock *msg);
#endif

VOS_VOID* TAF_PrivacyMatchAtCallIncomingInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCallConnectInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtQryCallInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCallSsInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtEccNumInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtQryXlemaInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCnapQryCnf(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCnapInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtQryUus1InfoCnf(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtUus1InfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtQryClprCnf(MsgBlock *msg);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* TAF_PrivacyMatchAtCalledNumInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCallingNumInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtDisplayInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtExtDisplayInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtConnNumInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtRedirNumInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtCcwacInfoInfoInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtContDtmfInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtBurstDtmfInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtEncryptVoiceInd(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtGetEcRandomCnf(MsgBlock *msg);
VOS_VOID* TAF_PrivacyMatchAtGetEcKmcCnf(MsgBlock *msg);
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID* TAF_CCM_PrivacyMatchCcmEconfDialReq(MsgBlock *msg);
#endif
VOS_VOID* TAF_DSM_PrivacyMatchTafSetAuthDataReq(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf(MsgBlock *msg);

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonScellCnf(MsgBlock *msg);

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf(MsgBlock *msg);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf(MsgBlock *msg);
#endif

VOS_VOID* AT_PrivacyMatchPseucellInfoSetReq(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtLocationInfoQryCnf(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStatusInd(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd(MsgBlock *msg);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID* TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf(MsgBlock *msg);
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */

VOS_VOID* TAF_MMA_PrivacyMatchAtNetScanCnf(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStateQryCnf(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtClocInfoInd(MsgBlock *msg);

VOS_VOID* TAF_MMA_PrivacyMatchAtRejInfoQryCnf(MsgBlock *msg);

VOS_VOID* RNIC_PrivacyMatchCdsImsDataReq(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpManageInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetTftInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmPsCallOrigReq(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmPsPppDialOrigReq(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmPsSetTftInfoReq(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsEpdgCtrluNtf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallHandoverRstNtf(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmPsEpdgCtrlNtf(MsgBlock *msg);

VOS_VOID* TAF_PrivacyMatchDsmIfaceUpReq(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceRabInfoInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchRnicIfaceCfgInd(MsgBlock *msg);

#if (FEATURE_ON == FEATURE_UE_MODE_NR)
VOS_VOID* TAF_DSM_PrivacyMatchTafPsIpChangeInd(MsgBlock *msg);

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceIpChangeInd(MsgBlock *msg);
#endif

VOS_VOID* TAF_DSM_PrivacyMatchNdisIfaceUpInd(MsgBlock *msg);

#if ((FEATURE_LTEV == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_VOID* AT_PrivacyMatchLtevSourceIdSetReq(MsgBlock *msg);
#endif

VOS_VOID* TAF_PrivacyMatchDsmPsSetCustAttachProfileReq(MsgBlock *msg);
VOS_VOID* TAF_DSM_PrivacyMatchPsGetCustAttachProfileCnf(MsgBlock *msg);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of taf_log_privacy_match.h */
