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

#include "taf_log_privacy_match.h"
#include "taf_ssa_api.h"
#include "at_mta_interface.h"
#include "taf_drv_agent.h"
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
#include "taf_app_xsms_interface.h"
#include "at_xpds_interface.h"
#endif
#include "mn_msg_api.h"
#include "taf_ccm_api.h"
#include "rnic_cds_interface.h"
#include "taf_ps_api.h"
#include "taf_ps_type_def.h"
#include "taf_iface_api.h"
#include "dsm_rnic_pif.h"
#include "dsm_ndis_pif.h"
#if (FEATURE_IMS == FEATURE_ON)
#include "at_imsa_interface.h"
#endif
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif
#include "securec.h"

#include "mn_comm_api.h"


#define THIS_FILE_ID PS_FILE_ID_TAF_LOG_PRIVACY_MATCH_C

VOS_UINT32 TAF_AppMnCallBackCsCallIsNeedLogPrivacy(MN_CALL_EventUint32 evtId)
{
    VOS_UINT32 result = VOS_FALSE;

    switch (evtId) {
        case TAF_CALL_EVT_CALLED_NUM_INFO_IND:
        case TAF_CALL_EVT_CALLING_NUM_INFO_IND:
        case TAF_CALL_EVT_DISPLAY_INFO_IND:
        case TAF_CALL_EVT_EXT_DISPLAY_INFO_IND:
        case TAF_CALL_EVT_CONN_NUM_INFO_IND:
        case TAF_CALL_EVT_REDIR_NUM_INFO_IND:
        case TAF_CALL_EVT_CCWAC_INFO_IND:
        case TAF_CALL_EVT_RCV_CONT_DTMF_IND:
        case TAF_CALL_EVT_RCV_BURST_DTMF_IND:
        case TAF_CALL_EVT_ECONF_NOTIFY_IND:
        case TAF_CALL_EVT_CLCCECONF_INFO:
        case MN_CALL_EVT_ORIG:
        case MN_CALL_EVT_CALL_ORIG_CNF:
        case MN_CALL_EVT_INCOMING:
        case MN_CALL_EVT_CALL_PROC:
        case MN_CALL_EVT_ALERTING:
        case MN_CALL_EVT_CONNECT:
        case MN_CALL_EVT_SS_CMD_RSLT:
        case MN_CALL_EVT_RELEASED:
        case MN_CALL_EVT_ALL_RELEASED:
        case MN_CALL_EVT_CLCC_INFO:
        case MN_CALL_EVT_CLPR_SET_CNF:
        case MN_CALL_EVT_SUPS_CMD_CNF:
        case MN_CALL_EVT_SS_NOTIFY:
        case MN_CALL_EVT_ECC_NUM_IND:
        case MN_CALL_EVT_XLEMA_CNF:

            result = VOS_TRUE;

            break;

        default:
            break;
    }

    return result;
}

VOS_UINT32 TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(TAF_SSA_EvtIdUint32 evtId)
{
    VOS_UINT32 result = VOS_FALSE;

    switch (evtId) {
        case ID_TAF_SSA_LCS_MOLR_NTF:

            result = VOS_TRUE;

            break;

        default:
            break;
    }

    return result;
}

VOS_VOID* TAF_PrivacyMatchAppMnCallBackCsCall(MsgBlock *msg)
{
    MN_AT_IndEvt       *atIndEvt        = VOS_NULL_PTR;
    MN_AT_IndEvt       *privacyAtIndEvt = VOS_NULL_PTR;
    MN_CALL_EventUint32 evtId;
    VOS_UINT32          length;
    VOS_UINT32          len;
    errno_t             memResult;

    atIndEvt = (MN_AT_IndEvt *)msg;

    /* 获取当前的event类型，并判断该event是否需要脱敏 */
    evtId     = MN_CALL_EVT_BUTT;
    memResult = memcpy_s(&evtId, sizeof(evtId), atIndEvt->content, sizeof(evtId));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(evtId), sizeof(evtId));

    if (TAF_AppMnCallBackCsCallIsNeedLogPrivacy(evtId) == VOS_FALSE) {
        return (VOS_VOID *)msg;
    }

    /* 申请消息 */
    privacyAtIndEvt = (MN_AT_IndEvt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (privacyAtIndEvt == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyAtIndEvt, length, atIndEvt, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* event占用了aucContent前四个字节，消息内容需要需要偏移4个字节 */
    len = VOS_GET_MSG_LEN(atIndEvt) + VOS_MSG_HEAD_LENGTH - sizeof(MN_AT_IndEvt);

    (VOS_VOID)memset_s((VOS_VOID *)(privacyAtIndEvt->content + sizeof(evtId)), len, 0, len);

    return (VOS_VOID *)privacyAtIndEvt;
}

VOS_VOID* AT_PrivacyMatchRegisterSsMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg        *registerSs = VOS_NULL_PTR;
    TAF_SS_RegisterssReq *ssRegReq   = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    registerSs = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (registerSs == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(registerSs, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssRegReq = (TAF_SS_RegisterssReq *)(registerSs->content);

    (VOS_VOID)memset_s(ssRegReq->fwdToNum, sizeof(ssRegReq->fwdToNum), 0, sizeof(ssRegReq->fwdToNum));

    (VOS_VOID)memset_s(ssRegReq->fwdToSubAddr, sizeof(ssRegReq->fwdToSubAddr), 0, sizeof(ssRegReq->fwdToSubAddr));

    return (VOS_VOID *)registerSs;
}

VOS_VOID* AT_PrivacyMatchProcessUssMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg        *processUss = VOS_NULL_PTR;
    TAF_SS_ProcessUssReq *ssReq      = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    processUss = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (processUss == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(processUss, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_ProcessUssReq *)processUss->content;

    memset_s(&(ssReq->ussdStr), sizeof(TAF_SS_UssdString), 0, sizeof(TAF_SS_UssdString));

    memset_s(ssReq->msisdn, TAF_SS_MAX_MSISDN_LEN + 1, 0, TAF_SS_MAX_MSISDN_LEN + 1);

    return (VOS_VOID *)processUss;
}

VOS_VOID* AT_PrivacyMatchInterRogateMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg           *interRogate = VOS_NULL_PTR;
    TAF_SS_InterrogatessReq *ssReq       = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    interRogate = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (interRogate == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(interRogate, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_InterrogatessReq *)interRogate->content;

    memset_s(ssReq->password, sizeof(ssReq->password), 0, TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)interRogate;
}

VOS_VOID* AT_PrivacyMatchErasessMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg     *erasess = VOS_NULL_PTR;
    TAF_SS_ErasessReq *ssReq   = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    erasess = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (erasess == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(erasess, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_ErasessReq *)erasess->content;

    memset_s(ssReq->password, sizeof(ssReq->password), 0, TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)erasess;
}

VOS_VOID* AT_PrivacyMatchActivatessMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg        *activatess = VOS_NULL_PTR;
    TAF_SS_ActivatessReq *ssReq      = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    activatess = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (activatess == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(activatess, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_ActivatessReq *)activatess->content;

    memset_s(ssReq->password, sizeof(ssReq->password), 0, TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)activatess;
}

VOS_VOID* AT_PrivacyMatchDeactivatessMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg          *deactivatess = VOS_NULL_PTR;
    TAF_SS_DeactivatessReq *ssReq        = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    deactivatess = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (deactivatess == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(deactivatess, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_DeactivatessReq *)deactivatess->content;

    memset_s(ssReq->password, sizeof(ssReq->password), 0, TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)deactivatess;
}

VOS_VOID* AT_PrivacyMatchRegPwdMsg(MsgBlock *msg)
{
    MN_APP_ReqMsg    *regpwdss = VOS_NULL_PTR;
    TAF_SS_RegpwdReq *ssReq    = VOS_NULL_PTR;
    VOS_UINT32        length;
    errno_t           memResult;

    /* 申请内存，后续统一由底层释放 */
    regpwdss = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (regpwdss == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(regpwdss, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssReq = (TAF_SS_RegpwdReq *)regpwdss->content;

    memset_s(ssReq->oldPwdStr, sizeof(ssReq->oldPwdStr), 0, TAF_SS_MAX_PASSWORD_LEN + 1);

    memset_s(ssReq->newPwdStr, sizeof(ssReq->newPwdStr), 0, TAF_SS_MAX_PASSWORD_LEN + 1);

    memset_s(ssReq->newPwdStrCnf, sizeof(ssReq->newPwdStrCnf), 0, TAF_SS_MAX_PASSWORD_LEN + 1);

    return (VOS_VOID *)regpwdss;
}

VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(MsgBlock *msg)
{
    TAF_SSA_Evt        *ssaEvt        = VOS_NULL_PTR;
    TAF_SSA_Evt        *privacySsaEvt = VOS_NULL_PTR;
    TAF_SSA_LcsMolrNtf *lcsMolrNtf    = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    ssaEvt = (TAF_SSA_Evt *)msg;

    /* 根据当前的SsEvent判断是否需要脱敏 */
    if (TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(ssaEvt->evtId) == VOS_FALSE) {
        return (VOS_VOID *)msg;
    }

    /* 申请内存，后续统一由底层释放 */
    privacySsaEvt = (TAF_SSA_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (privacySsaEvt == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacySsaEvt, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    if (ssaEvt->evtId == ID_TAF_SSA_LCS_MOLR_NTF) {
        lcsMolrNtf = (TAF_SSA_LcsMolrNtf *)privacySsaEvt->content;

        /* 将敏感信息设置为全0 */
        memset_s(lcsMolrNtf->locationStr, sizeof(lcsMolrNtf->locationStr), 0, sizeof(lcsMolrNtf->locationStr));
    }

    return (VOS_VOID *)privacySsaEvt;
}

VOS_VOID* TAF_PrivacyMatchMnCallBackSsAtIndEvt(MsgBlock *msg)
{
    MN_AT_IndEvt                *atIndEvt             = VOS_NULL_PTR;
    MN_AT_IndEvt                *privacyAtIndEvt      = VOS_NULL_PTR;
    TAF_SS_CallIndependentEvent *ssCallIndependentEvt = VOS_NULL_PTR;
    VOS_UINT32                   length;
    VOS_UINT32 i;
    errno_t memResult;

    atIndEvt = (MN_AT_IndEvt *)msg;

    /* 申请内存，后续统一由底层释放 */
    privacyAtIndEvt = (MN_AT_IndEvt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (privacyAtIndEvt == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyAtIndEvt, length, atIndEvt, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ssCallIndependentEvt = (TAF_SS_CallIndependentEvent *)(privacyAtIndEvt->content);

    memset_s(&(ssCallIndependentEvt->fwdInfo), sizeof(TAF_SS_Forwardinginfo), 0, sizeof(TAF_SS_Forwardinginfo));

    memset_s(&(ssCallIndependentEvt->fwdFeaturelist), sizeof(TAF_SS_Fwdfeaturelist), 0, sizeof(TAF_SS_Fwdfeaturelist));

    memset_s(ssCallIndependentEvt->passWord, TAF_SS_MAX_PASSWORD_LEN, 0, TAF_SS_MAX_PASSWORD_LEN);

    memset_s(&(ssCallIndependentEvt->ussdString), sizeof(TAF_SS_UssdString), 0, sizeof(TAF_SS_UssdString));

    memset_s(ssCallIndependentEvt->msisdn, TAF_SS_MAX_MSISDN_LEN + 1, 0, TAF_SS_MAX_MSISDN_LEN + 1);

    for (i = 0; i < TAF_SS_MAX_NUM_OF_CCBS_FEATURE; i++) {
        memset_s(ssCallIndependentEvt->genericServiceInfo.ccbsFeatureList.ccBsFeature[i].bsubscriberNum,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1, 0, TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1);

        memset_s(ssCallIndependentEvt->genericServiceInfo.ccbsFeatureList.ccBsFeature[i].bsubscriberSubAddr,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_SUBADDRESS + 1, 0, TAF_SS_MAX_NUM_OF_BSUBSCRIBER_SUBADDRESS + 1);
    }

    return (VOS_VOID *)privacyAtIndEvt;
}

VOS_VOID* TAF_PrivacyMatchAppMnCallBackSs(MsgBlock *msg)
{
    TAF_SSA_Evt *ssaEvt = VOS_NULL_PTR;

    /*
     * 由于MN_CALLBACK_SS在发送时可能会通过两种不同的结构体(TAF_SSA_Evt/MN_AT_IndEvt)进行填充，
     * 处理逻辑:首先将pstMsg强转成TAF_SSA_EVT_STRU类型指针，并判断ulEvtExt字段，若ulEvtExt字段为0，则按
     * TAF_SSA_EVT_STRU进行解析并脱敏，否则按MN_AT_IndEvt。
     */
    ssaEvt = (TAF_SSA_Evt *)msg;

    /* 根据ulEvtExt字段判断该消息是否是LCS相关的上报，如果是，作单独脱敏处理 */
    if (ssaEvt->evtExt == 0) {
        /* 走到此处，表示MN_CALLBACK_SS在上报时是通过TAF_SSA_EVT_STRU填充的 */

        return TAF_PrivacyMatchMnCallBackSsLcsEvt(msg);
    } else {
        /* 走到此处，表示MN_CALLBACK_SS在上报时是通过MN_AT_IndEvt填充的 */

        return TAF_PrivacyMatchMnCallBackSsAtIndEvt(msg);
    }
}

VOS_VOID* TAF_PrivacyMatchAtCallBackQryProc(MsgBlock *msg)
{
    MN_AT_IndEvt *srcMsg    = VOS_NULL_PTR;
    VOS_UINT8    *sendAtMsg = VOS_NULL_PTR;
    VOS_UINT8    *msgBuf    = VOS_NULL_PTR;
    TAF_PH_IccId *iccId     = VOS_NULL_PTR;
    TAF_UINT8     qryEvtId;
    VOS_UINT32    length;
    errno_t       memResult;
    VOS_UINT16    len;
    VOS_UINT16    errorCode;

    srcMsg = (MN_AT_IndEvt *)msg;

    /* 取出qry evt type */
    qryEvtId = srcMsg->content[3];

    if (qryEvtId != TAF_PH_ICC_ID) {
        return (VOS_VOID *)msg;
    }

    /* 申请消息 */
    sendAtMsg = (VOS_UINT8 *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (sendAtMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sendAtMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    msgBuf = sendAtMsg + sizeof(MN_AT_IndEvt) - 4;

    if (msgBuf[3] == TAF_PH_ICC_ID) {
        memResult = memcpy_s(&errorCode, sizeof(errorCode), &msgBuf[4], sizeof(errorCode));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(errorCode), sizeof(errorCode));
        memResult = memcpy_s(&len, sizeof(len), &msgBuf[6], sizeof(len));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(len), sizeof(len));

        if ((sizeof(TAF_PH_IccId) == len) && (errorCode == TAF_ERR_NO_ERROR)) {
            iccId = (TAF_PH_IccId *)(msgBuf + 8);

            /* 将敏感信息设置为全0 */
            memset_s(iccId->iccId, sizeof(iccId->iccId), 0, sizeof(iccId->iccId));
        }
    }

    return (VOS_VOID *)sendAtMsg;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(MsgBlock *msg)
{
    /* 记录申请的内存 */
    TAF_XSMS_AppAtCnf *matchTafXsmsAppAtCnf = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存 */
    matchTafXsmsAppAtCnf = (TAF_XSMS_AppAtCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (matchTafXsmsAppAtCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchTafXsmsAppAtCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memset_s(&(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent), sizeof(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent), 0,
             sizeof(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent));

    return (VOS_VOID *)matchTafXsmsAppAtCnf;
}

VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf(MsgBlock *msg)
{
    /* 记录申请的内存 */
    TAF_XSMS_AppAtCnf *matchTafXsmsAppAtCnf = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存 */
    matchTafXsmsAppAtCnf = (TAF_XSMS_AppAtCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (matchTafXsmsAppAtCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchTafXsmsAppAtCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memset_s(&(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent), sizeof(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent), 0,
             sizeof(matchTafXsmsAppAtCnf->xsmsAtEvent.xSmsEvent));

    return (VOS_VOID *)matchTafXsmsAppAtCnf;
}

VOS_VOID* AT_PrivacyMatchAppMsgTypeSendReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    TAF_XSMS_SendMsgReq *matchTafXsmsSendMsgReq = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存 */
    matchTafXsmsSendMsgReq = (TAF_XSMS_SendMsgReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (matchTafXsmsSendMsgReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchTafXsmsSendMsgReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memset_s(&(matchTafXsmsSendMsgReq->sms.addr), sizeof(TAF_XSMS_Addr), 0, sizeof(TAF_XSMS_Addr));

    memset_s(&(matchTafXsmsSendMsgReq->sms.subAddr), sizeof(TAF_XSMS_SubAddr), 0, sizeof(TAF_XSMS_SubAddr));

    memset_s(matchTafXsmsSendMsgReq->sms.bearerData, sizeof(matchTafXsmsSendMsgReq->sms.bearerData), 0,
             sizeof(matchTafXsmsSendMsgReq->sms.bearerData));

    matchTafXsmsSendMsgReq->sms.bearerDataLen = 0;

    return (VOS_VOID *)matchTafXsmsSendMsgReq;
}
#endif

VOS_VOID* AT_PrivacyMatchCposSetReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    MN_APP_ReqMsg  *matchAppMsgCposSetReq = VOS_NULL_PTR;
    AT_MTA_CposReq *cposReq               = VOS_NULL_PTR;
    VOS_UINT32      length;
    errno_t         memResult;

    /* 申请内存 */
    matchAppMsgCposSetReq = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (matchAppMsgCposSetReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    cposReq = (AT_MTA_CposReq *)matchAppMsgCposSetReq->content;

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchAppMsgCposSetReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memset_s(cposReq->xmlText, sizeof(cposReq->xmlText), 0x00, sizeof(cposReq->xmlText));

    return (VOS_VOID *)matchAppMsgCposSetReq;
}

VOS_VOID* AT_PrivacyMatchSimLockWriteExSetReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    MN_APP_ReqMsg                  *matchAppMsgSimlockWriteExSetReq = VOS_NULL_PTR;
    DRV_AGENT_SimlockwriteexSetReq *simlockWriteExSetReq            = VOS_NULL_PTR;
    VOS_UINT32                      length;
    errno_t                         memResult;

    /* 申请内存 */
    matchAppMsgSimlockWriteExSetReq = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (matchAppMsgSimlockWriteExSetReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    simlockWriteExSetReq = (DRV_AGENT_SimlockwriteexSetReq *)matchAppMsgSimlockWriteExSetReq->content;

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchAppMsgSimlockWriteExSetReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memResult = memset_s(simlockWriteExSetReq,
                         (length - sizeof(MN_APP_ReqMsg) + sizeof(matchAppMsgSimlockWriteExSetReq->content)), 0x00,
                         sizeof(DRV_AGENT_SimlockwriteexSetReq));
    TAF_MEM_CHK_RTN_VAL(memResult, length - sizeof(MN_APP_ReqMsg) + sizeof(matchAppMsgSimlockWriteExSetReq->content),
                        sizeof(DRV_AGENT_SimlockwriteexSetReq));

    return (VOS_VOID *)matchAppMsgSimlockWriteExSetReq;
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_VOID* AT_PrivacyMatchImsaImsCtrlMsg(MsgBlock *msg)
{
    AT_IMSA_ImsCtrlMsg *imsCtrlMsg = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    imsCtrlMsg = (AT_IMSA_ImsCtrlMsg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (imsCtrlMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(imsCtrlMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(imsCtrlMsg->wifiMsg, imsCtrlMsg->wifiMsgLen, 0, imsCtrlMsg->wifiMsgLen);

    return (VOS_VOID *)imsCtrlMsg;
}

VOS_VOID* AT_PrivacyMatchImsaNickNameSetReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    MN_APP_ReqMsg        *matchAppMsgNickNameSetReq = VOS_NULL_PTR;
    IMSA_AT_NicknameInfo *nickNameInfo              = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    matchAppMsgNickNameSetReq = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (matchAppMsgNickNameSetReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    nickNameInfo = (IMSA_AT_NicknameInfo *)matchAppMsgNickNameSetReq->content;

    memResult = memcpy_s(matchAppMsgNickNameSetReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(nickNameInfo->nickName, MN_CALL_DISPLAY_NAME_STRING_SZ, 0, MN_CALL_DISPLAY_NAME_STRING_SZ);

    return (VOS_VOID *)matchAppMsgNickNameSetReq;
}
#endif

VOS_VOID* AT_PrivacyMatchMeidSetReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    MN_APP_ReqMsg     *matchAppMsgSetReq = VOS_NULL_PTR;
    AT_MTA_MeidSetReq *meidReq           = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存 */
    matchAppMsgSetReq = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (matchAppMsgSetReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(matchAppMsgSetReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    meidReq = (AT_MTA_MeidSetReq *)matchAppMsgSetReq->content;

    /* 过滤敏感消息 */
    memset_s(meidReq->meid, sizeof(meidReq->meid), 0x00, sizeof(meidReq->meid));

    return (VOS_VOID *)matchAppMsgSetReq;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID* AT_PrivacyMatchAppMsgTypeWriteReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    TAF_XSMS_WriteMsgReq *matchTafXsmsWriteMsgReq = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存 */
    matchTafXsmsWriteMsgReq = (TAF_XSMS_WriteMsgReq *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (matchTafXsmsWriteMsgReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchTafXsmsWriteMsgReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    memset_s(&(matchTafXsmsWriteMsgReq->sms.addr), sizeof(TAF_XSMS_Addr), 0, sizeof(TAF_XSMS_Addr));

    memset_s(&(matchTafXsmsWriteMsgReq->sms.subAddr), sizeof(TAF_XSMS_SubAddr), 0, sizeof(TAF_XSMS_SubAddr));

    memset_s(matchTafXsmsWriteMsgReq->sms.bearerData, sizeof(matchTafXsmsWriteMsgReq->sms.bearerData), 0,
             sizeof(matchTafXsmsWriteMsgReq->sms.bearerData));

    matchTafXsmsWriteMsgReq->sms.bearerDataLen = 0;

    return (VOS_VOID *)matchTafXsmsWriteMsgReq;
}

VOS_VOID* AT_PrivacyMatchAppMsgTypeDeleteReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    TAF_XSMS_DeleteMsgReq *matchTafXsmsDeleteMsgReq = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存 */
    matchTafXsmsDeleteMsgReq = (TAF_XSMS_DeleteMsgReq *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (matchTafXsmsDeleteMsgReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchTafXsmsDeleteMsgReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    matchTafXsmsDeleteMsgReq->indexNum = 0;

    return (VOS_VOID *)matchTafXsmsDeleteMsgReq;
}

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))

VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(MsgBlock *msg)
{
    AT_XPDS_GpsPosInfoRsp *privacyMatchMsg = VOS_NULL_PTR;
    VOS_UINT32             msgLength;
    errno_t                memResult;

    privacyMatchMsg = (AT_XPDS_GpsPosInfoRsp *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &msgLength);

    if (privacyMatchMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchMsg, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    memset_s(&(privacyMatchMsg->posInfo), sizeof(AT_XPDS_GpsPosInfo), 0, sizeof(AT_XPDS_GpsPosInfo));

    return (VOS_VOID *)privacyMatchMsg;
}

VOS_VOID* AT_PrivacyMatchCagpsPrmInfoRsp(MsgBlock *msg)
{
    AT_XPDS_GpsPrmInfoRsp *privacyMatchMsg = VOS_NULL_PTR;
    VOS_UINT32             msgLength;
    VOS_UINT32             maxPrmDataLen;
    errno_t                memResult;

    privacyMatchMsg = (AT_XPDS_GpsPrmInfoRsp *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &msgLength);

    if (privacyMatchMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchMsg, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    maxPrmDataLen = sizeof(AT_XPDS_GpsModemPrmdata) * TAF_MSG_CDMA_MAX_SV_NUM;

    memResult = memset_s(privacyMatchMsg->mseasData, maxPrmDataLen, 0, sizeof(privacyMatchMsg->mseasData));
    TAF_MEM_CHK_RTN_VAL(memResult, maxPrmDataLen, sizeof(privacyMatchMsg->mseasData));

    privacyMatchMsg->measNum = 0;

    return (VOS_VOID *)privacyMatchMsg;
}

VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(MsgBlock *msg)
{
    AT_XPDS_ApForwardDataInd *privacyMatchMsg = VOS_NULL_PTR;
    VOS_UINT32                msgLength;
    errno_t                   memResult;

    privacyMatchMsg = (AT_XPDS_ApForwardDataInd *)TAF_AllocPrivacyMsg(WUEPS_PID_AT, msg, &msgLength);

    if (privacyMatchMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchMsg, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    (VOS_VOID)memset_s(privacyMatchMsg->data, privacyMatchMsg->dataLen, 0, privacyMatchMsg->dataLen);

    privacyMatchMsg->dataLen = 0;

    return (VOS_VOID *)privacyMatchMsg;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(MsgBlock *msg)
{
    XPDS_AT_GpsReflocInfoCnf *privacyMatchRefLocInfo = VOS_NULL_PTR;
    VOS_UINT32                msgLength;
    errno_t                   memResult;

    privacyMatchRefLocInfo = (XPDS_AT_GpsReflocInfoCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchRefLocInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchRefLocInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    /* 替换RefLoc信息敏感数据 */
    memset_s(&(privacyMatchRefLocInfo->refLoc), sizeof(XPDS_AT_GpsReflocInfo), 0, sizeof(XPDS_AT_GpsReflocInfo));

    return (VOS_VOID *)privacyMatchRefLocInfo;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(MsgBlock *msg)
{
    XPDS_AT_GpsIonInfoInd *privacyMatchIonInfo = VOS_NULL_PTR;
    VOS_UINT32             msgLength;
    errno_t                memResult;

    privacyMatchIonInfo = (XPDS_AT_GpsIonInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchIonInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchIonInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    /* 清空敏感位置信息 */
    privacyMatchIonInfo->abParIncl = 0;
    privacyMatchIonInfo->alpha0    = 0;
    privacyMatchIonInfo->alpha1    = 0;
    privacyMatchIonInfo->alpha2    = 0;
    privacyMatchIonInfo->alpha3    = 0;
    privacyMatchIonInfo->beta0     = 0;
    privacyMatchIonInfo->beta1     = 0;
    privacyMatchIonInfo->beta2     = 0;
    privacyMatchIonInfo->beta3     = 0;

    return (VOS_VOID *)privacyMatchIonInfo;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsEphInfoInd(MsgBlock *msg)
{
    XPDS_AT_GpsEphInfoInd *privacyMatchEphInfo = VOS_NULL_PTR;
    VOS_UINT32             msgLength;
    VOS_UINT32             maxEphDataLen;
    errno_t                memResult;

    privacyMatchEphInfo = (XPDS_AT_GpsEphInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchEphInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchEphInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    maxEphDataLen = sizeof(XPDS_AT_EphData) * TAF_MSG_CDMA_MAX_EPH_PRN_NUM;

    /* 替换星历信息敏感数据 */
    memResult = memset_s(privacyMatchEphInfo->ephData, maxEphDataLen, 0, sizeof(privacyMatchEphInfo->ephData));
    TAF_MEM_CHK_RTN_VAL(memResult, maxEphDataLen, sizeof(privacyMatchEphInfo->ephData));

    privacyMatchEphInfo->svNum = 0;

    return (VOS_VOID *)privacyMatchEphInfo;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd(MsgBlock *msg)
{
    XPDS_AT_GpsAlmInfoInd *privacyMatchAlmInfo = VOS_NULL_PTR;
    VOS_UINT32             msgLength;
    VOS_UINT32             maxAlmDataLen;
    errno_t                memResult;

    privacyMatchAlmInfo = (XPDS_AT_GpsAlmInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchAlmInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchAlmInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    maxAlmDataLen = sizeof(XPDS_AT_AlmData) * TAF_MSG_CDMA_MAX_ALM_PRN_NUM;

    memResult = memset_s(privacyMatchAlmInfo->almData, maxAlmDataLen, 0, sizeof(privacyMatchAlmInfo->almData));
    TAF_MEM_CHK_RTN_VAL(memResult, maxAlmDataLen, sizeof(privacyMatchAlmInfo->almData));


    privacyMatchAlmInfo->weekNum = 0;
    privacyMatchAlmInfo->toa     = 0;
    privacyMatchAlmInfo->svNum   = 0;

    return (VOS_VOID *)privacyMatchAlmInfo;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd(MsgBlock *msg)
{
    XPDS_AT_GpsPdePosiInfoInd *privacyMatchPosiInfo = VOS_NULL_PTR;
    VOS_UINT32                 msgLength;
    errno_t                    memResult;

    privacyMatchPosiInfo = (XPDS_AT_GpsPdePosiInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchPosiInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchPosiInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    /* 清空位置信息 */
    privacyMatchPosiInfo->clockBias    = 0;
    privacyMatchPosiInfo->fixType      = 0;
    privacyMatchPosiInfo->locUncAng    = 0;
    privacyMatchPosiInfo->clockDrift   = 0;
    privacyMatchPosiInfo->clockBias    = 0;
    privacyMatchPosiInfo->latitude     = 0;
    privacyMatchPosiInfo->longitude    = 0;
    privacyMatchPosiInfo->locUncA      = 0;
    privacyMatchPosiInfo->locUncP      = 0;
    privacyMatchPosiInfo->velocityHor  = 0;
    privacyMatchPosiInfo->heading      = 0;
    privacyMatchPosiInfo->height       = 0;
    privacyMatchPosiInfo->verticalVelo = 0;
    privacyMatchPosiInfo->locUncV      = 0;

    return (VOS_VOID *)privacyMatchPosiInfo;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd(MsgBlock *msg)
{
    XPDS_AT_GpsAcqAssistDataInd *privacyMatchAssistData = VOS_NULL_PTR;
    VOS_UINT32                   msgLength;
    VOS_UINT32                   maxAssistDataLen;
    errno_t                      memResult;

    privacyMatchAssistData = (XPDS_AT_GpsAcqAssistDataInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg),
                                                                                msg, &msgLength);

    if (privacyMatchAssistData == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchAssistData, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    maxAssistDataLen = sizeof(TAF_MSG_CdmaAcqassistData) * TAF_MSG_CDMA_MAX_SV_NUM;

    memResult = memset_s(privacyMatchAssistData->aaData, maxAssistDataLen, 0, sizeof(privacyMatchAssistData->aaData));
    TAF_MEM_CHK_RTN_VAL(memResult, maxAssistDataLen, sizeof(privacyMatchAssistData->aaData));

    privacyMatchAssistData->svNum  = 0;
    privacyMatchAssistData->refTow = 0;

    return (VOS_VOID *)privacyMatchAssistData;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtApReverseDataInd(MsgBlock *msg)
{
    XPDS_AT_ApReverseDataInd *privacyMatchReverseData = VOS_NULL_PTR;
    XPDS_AT_ApReverseDataInd *msgReverseDataInd       = VOS_NULL_PTR;
    VOS_UINT32                msgLength;
    errno_t                   memResult;

    msgReverseDataInd = (XPDS_AT_ApReverseDataInd *)msg;

    privacyMatchReverseData = (XPDS_AT_ApReverseDataInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchReverseData == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchReverseData, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    /* 清理用户隐私信息 */
    (VOS_VOID)memset_s(privacyMatchReverseData->data, msgReverseDataInd->dataLen, 0, msgReverseDataInd->dataLen);

    privacyMatchReverseData->dataLen    = 0;
    privacyMatchReverseData->serverMode = 0;

    return (VOS_VOID *)privacyMatchReverseData;
}

VOS_VOID* TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd(MsgBlock *msg)
{
    XPDS_AT_UtsGpsPosInfoInd *privacyMatchUtsPosInfo = VOS_NULL_PTR;
    VOS_UINT32                msgLength;
    errno_t                   memResult;

    privacyMatchUtsPosInfo = (XPDS_AT_UtsGpsPosInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &msgLength);

    if (privacyMatchUtsPosInfo == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(privacyMatchUtsPosInfo, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    memset_s((VOS_UINT8 *)&privacyMatchUtsPosInfo->posInfo, sizeof(AT_XPDS_GpsPosInfo), 0, sizeof(AT_XPDS_GpsPosInfo));

    return (VOS_VOID *)privacyMatchUtsPosInfo;
}
#endif
#endif

VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(MsgBlock *msg)
{
    AT_MTA_Msg      *sndMsg   = VOS_NULL_PTR;
    MTA_AT_CposrInd *cposrInd = VOS_NULL_PTR;
    VOS_UINT32       length;
    errno_t          memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    cposrInd = (MTA_AT_CposrInd *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(cposrInd->xmlText, MTA_CPOSR_XML_MAX_LEN + 1, 0, MTA_CPOSR_XML_MAX_LEN + 1);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MTA_PrivacyMatchAtMeidQryCnf(MsgBlock *msg)
{
    AT_MTA_Msg        *sndMsg     = VOS_NULL_PTR;
    MTA_AT_MeidQryCnf *meidQryCnf = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    meidQryCnf = (MTA_AT_MeidQryCnf *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(meidQryCnf->efruimid, MTA_AT_EFRUIMID_OCTET_LEN_EIGHT, 0, MTA_AT_EFRUIMID_OCTET_LEN_EIGHT);

    memset_s(meidQryCnf->meId, sizeof(meidQryCnf->meId), 0, sizeof(meidQryCnf->meId));

    memset_s(meidQryCnf->pEsn, sizeof(meidQryCnf->pEsn), 0, sizeof(meidQryCnf->pEsn));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MTA_PrivacyMatchAtCgsnQryCnf(MsgBlock *msg)
{
    AT_MTA_Msg        *sndMsg     = VOS_NULL_PTR;
    MTA_AT_CgsnQryCnf *cgsnQryCnf = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    cgsnQryCnf = (MTA_AT_CgsnQryCnf *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(cgsnQryCnf->imei, NV_ITEM_IMEI_SIZE, 0, NV_ITEM_IMEI_SIZE);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtUsimStatusInd(MsgBlock *msg)
{
    AT_MMA_UsimStatusInd *sndMsg = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MMA_UsimStatusInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&sndMsg->imsi[4], NAS_MAX_IMSI_LENGTH - 4, 0, NAS_MAX_IMSI_LENGTH - 4);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtHomePlmnQryCnf(MsgBlock *msg)
{
    TAF_MMA_HomePlmnQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_HomePlmnQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&sndMsg->eHplmnInfo.imsi[4], NAS_MAX_IMSI_LENGTH - 4, 0, NAS_MAX_IMSI_LENGTH - 4);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtLocationInfoQryCnf(MsgBlock *msg)
{
    TAF_MMA_LocationInfoQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                  length;
    errno_t                     memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_LocationInfoQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->lac                  = 0;
    sndMsg->rac                  = 0;
    sndMsg->cellId.cellIdHighBit = 0;
    sndMsg->cellId.cellIdLowBit  = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStatusInd(MsgBlock *msg)
{
    TAF_MMA_RegStatusInd *sndMsg = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_RegStatusInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(sndMsg->regStatus.cellId.cellId, sizeof(sndMsg->regStatus.cellId.cellId), 0,
             sizeof(sndMsg->regStatus.cellId.cellId));

    sndMsg->regStatus.lac = 0;
    sndMsg->regStatus.rac = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd(MsgBlock *msg)
{
    TAF_MMA_SrchedPlmnInfoInd *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                 length;
    VOS_UINT32                 i;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_SrchedPlmnInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_MAX_SRCHED_LAI_NUM; i++) {
        sndMsg->laiWithRscp[i].lac = 0;
    }

    return (VOS_VOID *)sndMsg;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID* TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf(MsgBlock *msg)
{
    TAF_MMA_CdmaLocinfoQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_CdmaLocinfoQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->clocinfoPara.baseId = 0;
    sndMsg->clocinfoPara.baseLatitude = 0;
    sndMsg->clocinfoPara.baseLongitude = 0;
    return (VOS_VOID *)sndMsg;
}
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */

VOS_VOID* TAF_MMA_PrivacyMatchAtNetScanCnf(MsgBlock *msg)
{
    TAF_MMA_NetScanCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32          length;
    VOS_UINT32          i;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_NetScanCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_NET_SCAN_MAX_FREQ_NUM; i++) {
        sndMsg->netScanInfo[i].cellId.cellIdHighBit = 0;
        sndMsg->netScanInfo[i].cellId.cellIdLowBit  = 0;
        sndMsg->netScanInfo[i].lac                  = 0;
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStateQryCnf(MsgBlock *msg)
{
    TAF_MMA_RegStateQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_RegStateQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->regInfo.lac = 0;
    sndMsg->regInfo.rac = 0;

    memset_s(sndMsg->regInfo.cellId.cellId, sizeof(sndMsg->regInfo.cellId.cellId), 0,
             sizeof(sndMsg->regInfo.cellId.cellId));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtClocInfoInd(MsgBlock *msg)
{
    TAF_MMA_ClocinfoInd *sndMsg = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_ClocinfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->clocinfoPara.baseId = 0;
    sndMsg->clocinfoPara.baseLatitude = 0;
    sndMsg->clocinfoPara.baseLongitude = 0;
    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtRejInfoQryCnf(MsgBlock *msg)
{
    TAF_MMA_RejinfoQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_RejinfoQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->phoneRejInfo.rac                  = 0;
    sndMsg->phoneRejInfo.lac                  = 0;
    sndMsg->phoneRejInfo.cellId.cellIdHighBit = 0;
    sndMsg->phoneRejInfo.cellId.cellIdLowBit  = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* RNIC_PrivacyMatchCdsImsDataReq(MsgBlock *msg)
{
    RNIC_CDS_ImsDataReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (RNIC_CDS_ImsDataReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(sndMsg->data, sndMsg->dataLen, 0, sndMsg->dataLen);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmPsCallOrigReq(MsgBlock *msg)
{
    TAF_PS_Msg         *sndMsg      = VOS_NULL_PTR;
    TAF_PS_CallOrigReq *callOrigReq = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    callOrigReq = (TAF_PS_CallOrigReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(callOrigReq->dialParaInfo.pdpAddr.ipv4Addr, sizeof(callOrigReq->dialParaInfo.pdpAddr.ipv4Addr), 0,
             TAF_IPV4_ADDR_LEN);

    memset_s(callOrigReq->dialParaInfo.pdpAddr.ipv6Addr, sizeof(callOrigReq->dialParaInfo.pdpAddr.ipv6Addr), 0,
             TAF_IPV6_ADDR_LEN);

    memset_s(callOrigReq->dialParaInfo.passWord, sizeof(callOrigReq->dialParaInfo.passWord), 0,
             TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

    memset_s(callOrigReq->dialParaInfo.userName, sizeof(callOrigReq->dialParaInfo.userName), 0,
             TAF_MAX_AUTHDATA_USERNAME_LEN + 1);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmPsPppDialOrigReq(MsgBlock *msg)
{
    TAF_PS_Msg            *sndMsg      = VOS_NULL_PTR;
    TAF_PS_PppDialOrigReq *dialOrigReq = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    dialOrigReq = (TAF_PS_PppDialOrigReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(&(dialOrigReq->pppDialParaInfo.pppReqConfigInfo.auth.authContent),
             sizeof(dialOrigReq->pppDialParaInfo.pppReqConfigInfo.auth.authContent), 0,
             sizeof(dialOrigReq->pppDialParaInfo.pppReqConfigInfo.auth.authContent));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq(MsgBlock *msg)
{
    TAF_PS_Msg                      *sndMsg       = VOS_NULL_PTR;
    TAF_PS_SetPrimPdpContextInfoReq *setPdpCtxReq = VOS_NULL_PTR;
    VOS_UINT32                       length;
    errno_t                          memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setPdpCtxReq = (TAF_PS_SetPrimPdpContextInfoReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(setPdpCtxReq->pdpContextInfo.pdpAddr.ipv4Addr, sizeof(setPdpCtxReq->pdpContextInfo.pdpAddr.ipv4Addr), 0,
             TAF_IPV4_ADDR_LEN);

    memset_s(setPdpCtxReq->pdpContextInfo.pdpAddr.ipv6Addr, sizeof(setPdpCtxReq->pdpContextInfo.pdpAddr.ipv6Addr), 0,
             TAF_IPV6_ADDR_LEN);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmPsSetTftInfoReq(MsgBlock *msg)
{
    TAF_PS_Msg           *sndMsg        = VOS_NULL_PTR;
    TAF_PS_SetTftInfoReq *setTftInfoReq = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setTftInfoReq = (TAF_PS_SetTftInfoReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(setTftInfoReq->tftInfo.localIpv4Addr, sizeof(setTftInfoReq->tftInfo.localIpv4Addr), 0, TAF_IPV4_ADDR_LEN);

    memset_s(setTftInfoReq->tftInfo.localIpv6Addr, sizeof(setTftInfoReq->tftInfo.localIpv6Addr), 0, TAF_IPV6_ADDR_LEN);

    memset_s(setTftInfoReq->tftInfo.sourceIpaddr.ipv4Addr, sizeof(setTftInfoReq->tftInfo.sourceIpaddr.ipv4Addr), 0,
             TAF_IPV4_ADDR_LEN);

    memset_s(setTftInfoReq->tftInfo.sourceIpaddr.ipv6Addr, sizeof(setTftInfoReq->tftInfo.sourceIpaddr.ipv6Addr), 0,
             TAF_IPV6_ADDR_LEN);

    setTftInfoReq->tftInfo.localIpv6Prefix = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsEpdgCtrluNtf(MsgBlock *msg)
{
    TAF_PS_Evt          *sndMsg       = VOS_NULL_PTR;
    TAF_PS_EpdgCtrluNtf *epdgCtrluNtf = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    epdgCtrluNtf = (TAF_PS_EpdgCtrluNtf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    if (epdgCtrluNtf->epdgCtrlu.opActReq == VOS_TRUE) {
        /* IPV4 地址 */
        memset_s(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv4Addr,
                 sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv4Addr), 0x00,
                 sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv4Addr));

        /* IPV6 地址 */
        memset_s(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv6Addr,
                 sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv6Addr), 0x00,
                 sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pdpAddr.ipv6Addr));

        /* IPV4 PCSCF 地址 */
        memset_s(&(epdgCtrluNtf->epdgCtrlu.actReqInfo.pcscf), sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pcscf), 0x00,
                 sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.pcscf));

        /* IPV6 PCSCF 地址 */
        memset_s(&(epdgCtrluNtf->epdgCtrlu.actReqInfo.iPv6Pcscf), sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.iPv6Pcscf),
                 0x00, sizeof(epdgCtrluNtf->epdgCtrlu.actReqInfo.iPv6Pcscf));
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallHandoverRstNtf(MsgBlock *msg)
{
    TAF_PS_Evt                   *sndMsg         = VOS_NULL_PTR;
    TAF_PS_CallHandoverResultNtf *handoverRstNtf = VOS_NULL_PTR;
    VOS_UINT32                    length;
    errno_t                       memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    handoverRstNtf = (TAF_PS_CallHandoverResultNtf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(handoverRstNtf->pdpInfo.ipv4Addr, sizeof(handoverRstNtf->pdpInfo.ipv4Addr), 0x00,
             sizeof(handoverRstNtf->pdpInfo.ipv4Addr));

    /* IPV6 地址 */
    memset_s(handoverRstNtf->pdpInfo.ipv6Addr, sizeof(handoverRstNtf->pdpInfo.ipv6Addr), 0x00,
             sizeof(handoverRstNtf->pdpInfo.ipv6Addr));

    /* IPV4 PCSCF 地址 */
    memset_s(handoverRstNtf->pdpInfo.ipv4PcscfList.ipv4PcscfAddrList,
             sizeof(handoverRstNtf->pdpInfo.ipv4PcscfList.ipv4PcscfAddrList), 0x00,
             sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(handoverRstNtf->pdpInfo.ipv6PcscfList.ipv6PcscfAddrList,
             sizeof(handoverRstNtf->pdpInfo.ipv6PcscfList.ipv6PcscfAddrList), 0x00,
             sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmPsEpdgCtrlNtf(MsgBlock *msg)
{
    TAF_PS_Msg         *sndMsg      = VOS_NULL_PTR;
    TAF_PS_EpdgCtrlNtf *epdgCtrlNtf = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    epdgCtrlNtf = (TAF_PS_EpdgCtrlNtf *)sndMsg->content;

    if (epdgCtrlNtf->epdgCtrl.opActCnf == VOS_TRUE) {
        /* 将敏感信息设置为全0 */
        memset_s(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv4Addr,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv4Addr), 0,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv4Addr));

        memset_s(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv6Addr,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv6Addr), 0,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pdpAddr.ipv6Addr));

        memset_s(&(epdgCtrlNtf->epdgCtrl.actCnfInfo.pcscf), sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pcscf), 0,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.pcscf));

        memset_s(&(epdgCtrlNtf->epdgCtrl.actCnfInfo.iPv6Pcscf), sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.iPv6Pcscf), 0,
                 sizeof(epdgCtrlNtf->epdgCtrl.actCnfInfo.iPv6Pcscf));
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_PrivacyMatchDsmIfaceUpReq(MsgBlock *msg)
{
    TAF_PS_Msg      *sndMsg     = VOS_NULL_PTR;
    TAF_IFACE_UpReq *ifaceUpReq = VOS_NULL_PTR;
    VOS_UINT32       length;
    errno_t          memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    ifaceUpReq = (TAF_IFACE_UpReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(ifaceUpReq->ifaceUp.usrDialParam.ipv4Addr, sizeof(ifaceUpReq->ifaceUp.usrDialParam.ipv4Addr), 0,
             sizeof(ifaceUpReq->ifaceUp.usrDialParam.ipv4Addr));

    memset_s(ifaceUpReq->ifaceUp.usrDialParam.password, sizeof(ifaceUpReq->ifaceUp.usrDialParam.password), 0,
             TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

    memset_s(ifaceUpReq->ifaceUp.usrDialParam.username, sizeof(ifaceUpReq->ifaceUp.usrDialParam.username), 0,
             TAF_MAX_AUTHDATA_USERNAME_LEN + 1);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf(MsgBlock *msg)
{
    TAF_PS_Evt                  *sndMsg            = VOS_NULL_PTR;
    TAF_IFACE_GetDynamicParaCnf *getDynamicParaCnf = VOS_NULL_PTR;
    VOS_UINT32                   length;
    VOS_UINT32                   i;
    errno_t                      memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    getDynamicParaCnf = (TAF_IFACE_GetDynamicParaCnf *)sndMsg->content;

    for (i = 0; i <= TAF_MAX_CID_NV; i++) {
        /* 将敏感信息设置为全0 */
        /* IPV4 地址 */
        getDynamicParaCnf->dynamicInfo[i].ipv4Info.dhcpInfo.addr = 0;

        /* IPV6 地址 */
        memset_s(getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.addr,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.addr), 0, TAF_IPV6_ADDR_LEN);

        /* IPV4 PCSCF 地址 */
        memset_s(getDynamicParaCnf->dynamicInfo[i].ipv4Info.dhcpInfo.pcscfList.pcscfAddrList,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv4Info.dhcpInfo.pcscfList.pcscfAddrList), 0,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv4Info.dhcpInfo.pcscfList.pcscfAddrList));

        /* IPV6 PCSCF 地址 */
        memset_s((getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.pcscfList.pcscfAddrList),
                 sizeof(TAF_IFACE_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
                 sizeof(TAF_IFACE_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

        /* IPV6 Tmp Addr */
        memset_s(getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.tmpAddr,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.tmpAddr), 0,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.dhcpInfo.tmpAddr));

        /* IPV6 Lan Addr */
        memset_s(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.lanAddr,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.lanAddr), 0,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.lanAddr));

        /* IPV6 Prefix Addr */
        memset_s(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.prefixAddr,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.prefixAddr), 0,
                 sizeof(getDynamicParaCnf->dynamicInfo[i].ipv6Info.raInfo.prefixAddr));
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceRabInfoInd(MsgBlock *msg)
{
    TAF_PS_Evt           *sndMsg     = VOS_NULL_PTR;
    TAF_IFACE_RabInfoInd *rabInfoInd = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    rabInfoInd = (TAF_IFACE_RabInfoInd *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    rabInfoInd->ipAddr = 0;

    return (VOS_VOID *)sndMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_VOID* TAF_DSM_PrivacyMatchTafPsIpChangeInd(MsgBlock *msg)
{
    TAF_PS_Evt         *evtMsg      = VOS_NULL_PTR;
    TAF_PS_IpChangeInd *ipChangeInd = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    evtMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (evtMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(evtMsg, length, msg, length);

    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    ipChangeInd = (TAF_PS_IpChangeInd *)evtMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    (VOS_VOID)memset_s(ipChangeInd->pdpAddr.ipv4Addr, TAF_IPV4_ADDR_LEN, 0, TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    (VOS_VOID)memset_s(ipChangeInd->pdpAddr.ipv6Addr, TAF_IPV6_ADDR_LEN, 0, TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    length = sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM;
    (VOS_VOID)memset_s(ipChangeInd->ipv4PcscfList.ipv4PcscfAddrList, length, 0, length);

    /* IPV6 PCSCF 地址 */
    length = sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM;
    (VOS_VOID)memset_s(ipChangeInd->ipv6PcscfList.ipv6PcscfAddrList, length, 0, length);

    /* IPV6 RA 地址 */
    length = sizeof(TAF_PDP_Ipv6Prefix) * TAF_MAX_PREFIX_NUM_IN_RA;
    (VOS_VOID)memset_s(ipChangeInd->ipv6RaInfo.prefixList, length, 0, length);

    return (VOS_VOID *)evtMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceIpChangeInd(MsgBlock *msg)
{
    TAF_PS_Evt            *evtMsg     = VOS_NULL_PTR;
    TAF_IFACE_IpChangeInd *iphangeInd = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存，后续统一由底层释放 */
    evtMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (evtMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(evtMsg, length, msg, length);

    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    iphangeInd = (TAF_IFACE_IpChangeInd *)evtMsg->content;

    /* 将敏感信息设置为全0 */
    iphangeInd->ipv4Addr = 0;

    (VOS_VOID)memset_s(iphangeInd->ipv6Addr, sizeof(iphangeInd->ipv6Addr), 0x00, sizeof(iphangeInd->ipv6Addr));

    return (VOS_VOID *)evtMsg;
}
#endif

VOS_VOID* TAF_DSM_PrivacyMatchRnicIfaceCfgInd(MsgBlock *msg)
{
    DSM_RNIC_IfaceCfgInd *ifaceCfgInd = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ifaceCfgInd = (DSM_RNIC_IfaceCfgInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ifaceCfgInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ifaceCfgInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ifaceCfgInd->ipv4Addr = 0;

    memset_s(ifaceCfgInd->ipv6Addr, sizeof(ifaceCfgInd->ipv6Addr), 0, RNICITF_MAX_IPV6_ADDR_LEN);

    return (VOS_VOID *)ifaceCfgInd;
}

VOS_VOID* TAF_DSM_PrivacyMatchNdisIfaceUpInd(MsgBlock *msg)
{
    DSM_NDIS_IfaceUpInd *ifaceUpInd = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ifaceUpInd = (DSM_NDIS_IfaceUpInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ifaceUpInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ifaceUpInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(ifaceUpInd->ipv4PdnInfo), sizeof(ifaceUpInd->ipv4PdnInfo), 0, sizeof(DSM_NDIS_Ipv4PdnInfo));

    memset_s(&(ifaceUpInd->ipv6PdnInfo), sizeof(ifaceUpInd->ipv6PdnInfo), 0, sizeof(DSM_NDIS_Ipv6PdnInfo));

    return (VOS_VOID *)ifaceUpInd;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActCnf(MsgBlock *msg)
{
    TAF_PS_Evt                *sndMsg    = VOS_NULL_PTR;
    TAF_PS_CallPdpActivateCnf *pdpActCnf = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpActCnf = (TAF_PS_CallPdpActivateCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pdpActCnf->pdpAddr.ipv4Addr, sizeof(pdpActCnf->pdpAddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pdpActCnf->pdpAddr.ipv6Addr, sizeof(pdpActCnf->pdpAddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    memset_s(pdpActCnf->ipv4PcscfList.ipv4PcscfAddrList, sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pdpActCnf->ipv6PcscfList.ipv6PcscfAddrList, sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);


    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActInd(MsgBlock *msg)
{
    TAF_PS_Evt                *sndMsg    = VOS_NULL_PTR;
    TAF_PS_CallPdpActivateInd *pdpActInd = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpActInd = (TAF_PS_CallPdpActivateInd *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pdpActInd->pdpAddr.ipv4Addr, sizeof(pdpActInd->pdpAddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pdpActInd->pdpAddr.ipv6Addr, sizeof(pdpActInd->pdpAddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    memset_s(pdpActInd->ipv4PcscfList.ipv4PcscfAddrList, sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pdpActInd->ipv6PcscfList.ipv6PcscfAddrList, sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpManageInd(MsgBlock *msg)
{
    TAF_PS_Evt              *sndMsg       = VOS_NULL_PTR;
    TAF_PS_CallPdpManageInd *pdpManageInd = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpManageInd = (TAF_PS_CallPdpManageInd *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pdpManageInd->pdpAddr.ipv4Addr, sizeof(pdpManageInd->pdpAddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pdpManageInd->pdpAddr.ipv6Addr, sizeof(pdpManageInd->pdpAddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModCnf(MsgBlock *msg)
{
    TAF_PS_Evt              *sndMsg    = VOS_NULL_PTR;
    TAF_PS_CallPdpModifyCnf *pdpModCnf = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpModCnf = (TAF_PS_CallPdpModifyCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    memset_s(pdpModCnf->ipv4PcscfList.ipv4PcscfAddrList, sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pdpModCnf->ipv6PcscfList.ipv6PcscfAddrList, sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModInd(MsgBlock *msg)
{
    TAF_PS_Evt              *sndMsg    = VOS_NULL_PTR;
    TAF_PS_CallPdpModifyInd *pdpModInd = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpModInd = (TAF_PS_CallPdpModifyCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    memset_s(pdpModInd->ipv4PcscfList.ipv4PcscfAddrList, sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv4Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pdpModInd->ipv6PcscfList.ipv6PcscfAddrList, sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM, 0,
             sizeof(TAF_PDP_Ipv6Pcscf) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd(MsgBlock *msg)
{
    TAF_PS_Evt         *sndMsg         = VOS_NULL_PTR;
    TAF_PS_Ipv6InfoInd *pdpIpv6InfoInd = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpIpv6InfoInd = (TAF_PS_Ipv6InfoInd *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    /* IPV6 地址 */
    memset_s(pdpIpv6InfoInd->ipv6RaInfo.prefixList, sizeof(TAF_PDP_Ipv6Prefix) * TAF_MAX_PREFIX_NUM_IN_RA, 0,
             sizeof(TAF_PDP_Ipv6Prefix) * TAF_MAX_PREFIX_NUM_IN_RA);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt                      *sndMsg        = VOS_NULL_PTR;
    TAF_PS_GetPrimPdpContextInfoCnf *pdpCtxInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                       length;
    VOS_UINT32                       i;
    errno_t                          memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pdpCtxInfoCnf = (TAF_PS_GetPrimPdpContextInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pdpCtxInfoCnf->cidNum; i++) {
        /* IPV4 地址 */
        memset_s(pdpCtxInfoCnf->pdpContextQueryInfo[i].priPdpInfo.pdpAddr.ipv4Addr,
                 sizeof(pdpCtxInfoCnf->pdpContextQueryInfo[i].priPdpInfo.pdpAddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);
        /* IPV6 地址 */
        memset_s(pdpCtxInfoCnf->pdpContextQueryInfo[i].priPdpInfo.pdpAddr.ipv6Addr,
                 sizeof(pdpCtxInfoCnf->pdpContextQueryInfo[i].priPdpInfo.pdpAddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetTftInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt           *sndMsg        = VOS_NULL_PTR;
    TAF_PS_GetTftInfoCnf *getTftInfoCnf = VOS_NULL_PTR;
    VOS_UINT32            length;
    VOS_UINT32            i;
    VOS_UINT32            j;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    getTftInfoCnf = (TAF_PS_GetTftInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < getTftInfoCnf->cidNum; i++) {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++) {
            /* IPV4 地址 */
            memset_s(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].localIpv4Addr,
                     sizeof(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].localIpv4Addr), 0, TAF_IPV4_ADDR_LEN);

            memset_s(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].rmtIpv4Address,
                     sizeof(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].rmtIpv4Address), 0, TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            memset_s(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].localIpv6Addr,
                     sizeof(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].localIpv6Addr), 0, TAF_IPV6_ADDR_LEN);

            memset_s(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].rmtIpv6Address,
                     sizeof(getTftInfoCnf->tftQueryInfo[i].pfInfo[j].rmtIpv6Address), 0, TAF_IPV6_ADDR_LEN);
            getTftInfoCnf->tftQueryInfo[i].pfInfo[j].localIpv6Prefix = 0;
        }
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt                 *sndMsg     = VOS_NULL_PTR;
    TAF_PS_GetPdpIpAddrInfoCnf *ipAddrInfo = VOS_NULL_PTR;
    VOS_UINT32                  length;
    VOS_UINT32                  i;
    errno_t                     memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    ipAddrInfo = (TAF_PS_GetPdpIpAddrInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    for (i = 0; i <= TAF_MAX_CID; i++) {
        /* IPV4 地址 */
        memset_s(ipAddrInfo->pdpAddrQueryInfo[i].pdpAddr.ipv4Addr,
                 sizeof(ipAddrInfo->pdpAddrQueryInfo[i].pdpAddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);

        /* IPV6 地址 */
        memset_s(ipAddrInfo->pdpAddrQueryInfo[i].pdpAddr.ipv6Addr,
                 sizeof(ipAddrInfo->pdpAddrQueryInfo[i].pdpAddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);
    }
    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt                  *sndMsg  = VOS_NULL_PTR;
    TAF_PS_GetDynamicTftInfoCnf *tftInfo = VOS_NULL_PTR;
    VOS_UINT32                   length;
    VOS_UINT32                   i;
    VOS_UINT32                   j;
    errno_t                      memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    tftInfo = (TAF_PS_GetDynamicTftInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < tftInfo->cidNum; i++) {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++) {
            /* IPV4 地址 */
            memset_s(tftInfo->pfTftInfo[i].tftInfo[j].localIpv4Addr,
                     sizeof(tftInfo->pfTftInfo[i].tftInfo[j].localIpv4Addr), 0, TAF_IPV4_ADDR_LEN);

            memset_s(tftInfo->pfTftInfo[i].tftInfo[j].sourceIpaddr.ipv4Addr,
                     sizeof(tftInfo->pfTftInfo[i].tftInfo[j].sourceIpaddr.ipv4Addr), 0, TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            memset_s(tftInfo->pfTftInfo[i].tftInfo[j].localIpv6Addr,
                     sizeof(tftInfo->pfTftInfo[i].tftInfo[j].localIpv6Addr), 0, TAF_IPV6_ADDR_LEN);

            memset_s(tftInfo->pfTftInfo[i].tftInfo[j].sourceIpaddr.ipv6Addr,
                     sizeof(tftInfo->pfTftInfo[i].tftInfo[j].sourceIpaddr.ipv6Addr), 0, TAF_IPV6_ADDR_LEN);
            tftInfo->pfTftInfo[i].tftInfo[j].localIpv6Prefix = 0;
        }
    }
    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt                *sndMsg       = VOS_NULL_PTR;
    TAF_PS_GetAuthdataInfoCnf *authdataInfo = VOS_NULL_PTR;
    VOS_UINT32                 length;
    VOS_UINT32                 i;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    authdataInfo = (TAF_PS_GetAuthdataInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < authdataInfo->cidNum; i++) {
        memset_s(authdataInfo->authDataQueryInfo[i].authDataInfo.password,
                 sizeof(authdataInfo->authDataQueryInfo[i].authDataInfo.password), 0,
                 TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

        memset_s(authdataInfo->authDataQueryInfo[i].authDataInfo.username,
                 sizeof(authdataInfo->authDataQueryInfo[i].authDataInfo.username), 0,
                 TAF_MAX_AUTHDATA_USERNAME_LEN + 1);
    }
    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf(MsgBlock *msg)
{
    DRV_AGENT_Msg        *sndMsg     = VOS_NULL_PTR;
    DRV_AGENT_MsidQryCnf *msidQryCnf = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (DRV_AGENT_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    msidQryCnf = (DRV_AGENT_MsidQryCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(msidQryCnf->imei, TAF_PH_IMEI_LEN, 0, TAF_PH_IMEI_LEN);

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* AT_PrivacyMatchSmsAppMsgReq(MsgBlock *msg)
{
    VOS_UINT8     *appReq      = VOS_NULL_PTR;
    MN_APP_ReqMsg *inputAppReq = VOS_NULL_PTR;
    VOS_UINT32     length;
    VOS_UINT32     copyLength;
    errno_t        memResult;

    inputAppReq = (MN_APP_ReqMsg *)msg;

    /* 计算消息长度和待输出脱敏后消息有效字段长度 */
    length     = VOS_MSG_HEAD_LENGTH + VOS_GET_MSG_LEN(inputAppReq);
    copyLength = (sizeof(MN_APP_ReqMsg) - sizeof(inputAppReq->content));

    /* 消息长度小于消息结构的头部长度，认为消息异常输出原始消息，不脱敏 */
    if (length < copyLength) {
        return inputAppReq;
    }

    /* 申请消息长度大小的内存，用于脱敏后消息输出 */
    appReq = (VOS_UINT8 *)VOS_MemAlloc(VOS_GET_SENDER_ID(inputAppReq), DYNAMIC_MEM_PT, length);

    if (appReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 仅拷贝消息头部到脱敏后消息指针 */
    (VOS_VOID)memset_s(appReq, length, 0, length);

    memResult = memcpy_s(appReq, length, inputAppReq, copyLength);
    TAF_MEM_CHK_RTN_VAL(memResult, length, copyLength);

    return (VOS_VOID *)appReq;
}

VOS_UINT32 TAF_MSG_IsNeedPrivacyEventToApp(VOS_UINT32 event)
{
    if ((event == MN_MSG_EVT_READ) || (event == MN_MSG_EVT_LIST) || (event == MN_MSG_EVT_DELIVER) ||
        (event == MN_MSG_EVT_WRITE_SRV_PARM) || (event == MN_MSG_EVT_READ_SRV_PARM) ||
        (event == MN_MSG_EVT_SUBMIT_RPT)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID* TAF_PrivacyMatchAtCallBackSmsProc(MsgBlock *msg)
{
    MN_MSG_EventInfo  *event     = VOS_NULL_PTR;
    MN_AT_IndEvt      *srcMsg    = VOS_NULL_PTR;
    MN_AT_IndEvt      *sendAtMsg = VOS_NULL_PTR;
    VOS_UINT32         length;
    MN_MSG_EventUint32 eventValue;
    TAF_UINT32         eventLen;
    errno_t            memResult;

    srcMsg = (MN_AT_IndEvt *)msg;

    eventLen  = sizeof(MN_MSG_EventUint32);
    memResult = memcpy_s(&eventValue, sizeof(eventValue), srcMsg->content, eventLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(eventValue), eventLen);

    /* 不要求脱敏的事件直接返回源消息地址 */
    if (TAF_MSG_IsNeedPrivacyEventToApp(eventValue) == VOS_FALSE) {
        return msg;
    }

    sendAtMsg = (MN_AT_IndEvt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (sendAtMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sendAtMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    event = (MN_MSG_EventInfo *)&sendAtMsg->content[eventLen];

    memset_s(&event->u, sizeof(event->u), 0, sizeof(event->u));

    return (VOS_VOID *)sendAtMsg;
}

VOS_VOID* TAF_MTA_PrivacyMatchAtEcidSetCnf(MsgBlock *msg)
{
    AT_MTA_Msg        *sndMsg          = VOS_NULL_PTR;
    MTA_AT_EcidSetCnf *mtaAtEcidSetCnf = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT32         contentLength;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    mtaAtEcidSetCnf = (MTA_AT_EcidSetCnf *)(sndMsg->content);

    /* 将敏感信息设置为全0,清空字符串,不清楚log打印机制，这里把字符串全部清0 */
    if (VOS_GET_MSG_LEN(msg) > (sizeof(AT_APPCTRL) + sizeof(sndMsg->msgId))) {
        contentLength = VOS_GET_MSG_LEN(msg) -
                        (sizeof(AT_APPCTRL) + sizeof(sndMsg->msgId) + sizeof(MTA_AT_ResultUint32));
        (VOS_VOID)memset_s(mtaAtEcidSetCnf->cellInfoStr, contentLength, 0x00, contentLength);
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoSetReq(MsgBlock *msg)
{
    TAF_MMA_EflociinfoSetReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_EflociinfoSetReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->eflociInfo.tmsi = 0;
    sndMsg->eflociInfo.lac  = 0;
    sndMsg->eflociInfo.rfu  = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf(MsgBlock *msg)
{
    TAF_MMA_EflociinfoQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_EflociinfoQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->eflociInfo.tmsi = 0;
    sndMsg->eflociInfo.lac  = 0;
    sndMsg->eflociInfo.rfu  = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq(MsgBlock *msg)
{
    TAF_MMA_EfpslociinfoSetReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                  length;
    errno_t                     memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_EfpslociinfoSetReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->psEflociInfo.pTmsi = 0;
    sndMsg->psEflociInfo.lac   = 0;
    sndMsg->psEflociInfo.rac   = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf(MsgBlock *msg)
{
    TAF_MMA_EfpslociinfoQryCnf *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                  length;
    errno_t                     memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_EfpslociinfoQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sndMsg->psEflociInfo.pTmsi = 0;
    sndMsg->psEflociInfo.lac   = 0;
    sndMsg->psEflociInfo.rac   = 0;
    sndMsg->psEflociInfo.pTmsiSignature = 0;
    return (VOS_VOID *)sndMsg;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)

VOS_VOID* TAF_MMA_PrivacySubnetAdapterInfoInd(MsgBlock *msg)
{
    TAF_MMA_SubnetAdapterInfoInd *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                    length;
    errno_t                       memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_MMA_SubnetAdapterInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(sndMsg->cardInfo.imsi, sizeof(sndMsg->cardInfo.imsi),
        0x00, sizeof(sndMsg->cardInfo.imsi));

    (VOS_VOID)memset_s(sndMsg->cardInfo.iccid, sizeof(sndMsg->cardInfo.iccid),
        0x00, sizeof(sndMsg->cardInfo.iccid));

    return (VOS_VOID *)sndMsg;
}
#endif

VOS_VOID* TAF_DSM_PrivacyMatchTafSetAuthDataReq(MsgBlock *msg)
{
    TAF_PS_Msg                *sndMsg         = VOS_NULL_PTR;
    TAF_PS_SetAuthdataInfoReq *setAuthDataReq = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setAuthDataReq = (TAF_PS_SetAuthdataInfoReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(setAuthDataReq->authDataInfo.passWord, sizeof(setAuthDataReq->authDataInfo.passWord), 0x00,
             sizeof(setAuthDataReq->authDataInfo.passWord));

    memset_s(setAuthDataReq->authDataInfo.userName, sizeof(setAuthDataReq->authDataInfo.userName), 0x00,
             sizeof(setAuthDataReq->authDataInfo.userName));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq(MsgBlock *msg)
{
    TAF_PS_Msg              *sndMsg              = VOS_NULL_PTR;
    TAF_PS_SetPdpDnsInfoReq *setSetPdpDnsInfoReq = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setSetPdpDnsInfoReq = (TAF_PS_SetPdpDnsInfoReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(setSetPdpDnsInfoReq->pdpDnsInfo.primDnsAddr, sizeof(setSetPdpDnsInfoReq->pdpDnsInfo.primDnsAddr), 0x00,
             sizeof(setSetPdpDnsInfoReq->pdpDnsInfo.primDnsAddr));

    memset_s(setSetPdpDnsInfoReq->pdpDnsInfo.secDnsAddr, sizeof(setSetPdpDnsInfoReq->pdpDnsInfo.secDnsAddr), 0x00,
             sizeof(setSetPdpDnsInfoReq->pdpDnsInfo.secDnsAddr));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf(MsgBlock *msg)
{
    TAF_PS_Evt              *sndMsg              = VOS_NULL_PTR;
    TAF_PS_GetPdpDnsInfoCnf *setGetPdpDnsInfoCnf = VOS_NULL_PTR;
    VOS_UINT32               length;
    VOS_UINT32               indexNum;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setGetPdpDnsInfoCnf = (TAF_PS_GetPdpDnsInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */

    for (indexNum = 0; indexNum < TAF_MIN(setGetPdpDnsInfoCnf->cidNum, TAF_MAX_CID); indexNum++) {
        memset_s(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.primDnsAddr,
                 sizeof(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.primDnsAddr), 0x00,
                 sizeof(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.primDnsAddr));

        memset_s(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.secDnsAddr,
                 sizeof(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.secDnsAddr), 0x00,
                 sizeof(setGetPdpDnsInfoCnf->pdpDnsQueryInfo[indexNum].dnsInfo.secDnsAddr));
    }

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf(MsgBlock *msg)
{
    TAF_PS_Evt                  *sndMsg               = VOS_NULL_PTR;
    TAF_PS_GetNegotiationDnsCnf *getNegotiationDnsCnf = VOS_NULL_PTR;
    VOS_UINT32                   length;
    errno_t                      memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    getNegotiationDnsCnf = (TAF_PS_GetNegotiationDnsCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    memset_s(getNegotiationDnsCnf->negotiationDns.dnsInfo.primDnsAddr,
             sizeof(getNegotiationDnsCnf->negotiationDns.dnsInfo.primDnsAddr), 0x00,
             sizeof(getNegotiationDnsCnf->negotiationDns.dnsInfo.primDnsAddr));

    memset_s(getNegotiationDnsCnf->negotiationDns.dnsInfo.secDnsAddr,
             sizeof(getNegotiationDnsCnf->negotiationDns.dnsInfo.secDnsAddr), 0x00,
             sizeof(getNegotiationDnsCnf->negotiationDns.dnsInfo.secDnsAddr));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonScellCnf(MsgBlock *msg)
{
    AT_MTA_Msg            *sndMsg                 = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *mtaAtSetNetMonScellCnf = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    mtaAtSetNetMonScellCnf = (MTA_AT_NetmonCellInfo *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(&(mtaAtSetNetMonScellCnf->u.nCellInfo), sizeof(mtaAtSetNetMonScellCnf->u.nCellInfo), 0x00,
             sizeof(mtaAtSetNetMonScellCnf->u.nCellInfo));

    memset_s(&(mtaAtSetNetMonScellCnf->u.unSCellInfo), sizeof(mtaAtSetNetMonScellCnf->u.unSCellInfo), 0x00,
             sizeof(mtaAtSetNetMonScellCnf->u.unSCellInfo));

    return (VOS_VOID *)sndMsg;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf(MsgBlock *msg)
{
    AT_MTA_Msg            *sndMsg        = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *mtaAtCellInfo = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 计算消息长度 */
    length = VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);

    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    mtaAtCellInfo = (MTA_AT_NetmonCellInfo *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(&(mtaAtCellInfo->u.secSrvCellInfo), sizeof(mtaAtCellInfo->u.secSrvCellInfo), 0x00,
             sizeof(mtaAtCellInfo->u.secSrvCellInfo));

    return (VOS_VOID *)sndMsg;
}
#endif

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf(MsgBlock *msg)
{
    AT_MTA_Msg            *sndMsg                 = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *mtaAtSetNetMonScellCnf = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    mtaAtSetNetMonScellCnf = (MTA_AT_NetmonCellInfo *)(sndMsg->content);

    /* 将敏感信息设置为全0 */
    memset_s(&(mtaAtSetNetMonScellCnf->u.nCellInfo), sizeof(mtaAtSetNetMonScellCnf->u.nCellInfo), 0x00,
             sizeof(mtaAtSetNetMonScellCnf->u.nCellInfo));

    memset_s(&(mtaAtSetNetMonScellCnf->u.unSCellInfo), sizeof(mtaAtSetNetMonScellCnf->u.unSCellInfo), 0x00,
             sizeof(mtaAtSetNetMonScellCnf->u.unSCellInfo));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* AT_PrivacyMatchPseucellInfoSetReq(MsgBlock *msg)
{
    MN_APP_ReqMsg             *sndMsg             = VOS_NULL_PTR;
    AT_MTA_PseucellInfoSetReq *pseucellInfoSetReq = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    pseucellInfoSetReq = (AT_MTA_PseucellInfoSetReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    pseucellInfoSetReq->lac = 0;

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_CCM_PrivacyMatchCallOrigReq(MsgBlock *msg)
{
    TAF_CCM_CallOrigReq *callOrigReq = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    callOrigReq = (TAF_CCM_CallOrigReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (callOrigReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callOrigReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    /* 清0敏感信息 */
    memset_s(&callOrigReq->orig.dialNumber, sizeof(callOrigReq->orig.dialNumber), 0, sizeof(MN_CALL_CalledNum));

    memset_s(&callOrigReq->orig.subaddr, sizeof(callOrigReq->orig.subaddr), 0, sizeof(MN_CALL_Subaddr));
    memset_s(&callOrigReq->orig.bc, sizeof(callOrigReq->orig.bc), 0, sizeof(MN_CALL_Bc));
    memset_s(&callOrigReq->orig.emergencyCat, sizeof(callOrigReq->orig.emergencyCat), 0, sizeof(MN_CALL_EmergencyCat));

    callOrigReq->orig.is1xStkEmc = 0;

    return (VOS_VOID *)callOrigReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCallSupsCmdReq(MsgBlock *msg)
{
    TAF_CCM_CallSupsCmdReq *callSupsCmdReq = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    callSupsCmdReq = (TAF_CCM_CallSupsCmdReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (callSupsCmdReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callSupsCmdReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&callSupsCmdReq->callMgmtCmd.redirectNum, sizeof(callSupsCmdReq->callMgmtCmd.redirectNum), 0,
             sizeof(MN_CALL_BcdNum));

    memset_s(&callSupsCmdReq->callMgmtCmd.removeNum, sizeof(callSupsCmdReq->callMgmtCmd.removeNum), 0,
             sizeof(MN_CALL_CalledNum));

    return (VOS_VOID *)callSupsCmdReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCustomDialReqCmdReq(MsgBlock *msg)
{
    TAF_CCM_CustomDialReq *customDialReqCmdReq = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    customDialReqCmdReq = (TAF_CCM_CustomDialReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (customDialReqCmdReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(customDialReqCmdReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&customDialReqCmdReq->customDialPara.dialNumber, sizeof(customDialReqCmdReq->customDialPara.dialNumber), 0,
             sizeof(MN_CALL_CalledNum));

    return (VOS_VOID *)customDialReqCmdReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmStartDtmfReq(MsgBlock *msg)
{
    TAF_CCM_StartDtmfReq *ccmStartDtmfReq = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmStartDtmfReq = (TAF_CCM_StartDtmfReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmStartDtmfReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmStartDtmfReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&ccmStartDtmfReq->dtmf.key, sizeof(ccmStartDtmfReq->dtmf.key), 0, sizeof(VOS_CHAR));

    return (VOS_VOID *)ccmStartDtmfReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmStopDtmfReq(MsgBlock *msg)
{
    TAF_CCM_StopDtmfReq *ccmStopDtmfReq = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmStopDtmfReq = (TAF_CCM_StopDtmfReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmStopDtmfReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmStopDtmfReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&ccmStopDtmfReq->dtmf.key, sizeof(ccmStopDtmfReq->dtmf.key), 0, sizeof(VOS_CHAR));

    return (VOS_VOID *)ccmStopDtmfReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmSetUusInfoReq(MsgBlock *msg)
{
    TAF_CCM_SetUusinfoReq *ccmSetUusinfoReq = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmSetUusinfoReq = (TAF_CCM_SetUusinfoReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmSetUusinfoReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmSetUusinfoReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(ccmSetUusinfoReq->uus1Info.uus1Info, sizeof(ccmSetUusinfoReq->uus1Info.uus1Info), 0,
             sizeof(MN_CALL_Uus1Info) * MN_CALL_MAX_UUS1_MSG_NUM);

    return (VOS_VOID *)ccmSetUusinfoReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmCustomEccNumReq(MsgBlock *msg)
{
    TAF_CCM_CustomEccNumReq *ccmCustomEccNumReq = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmCustomEccNumReq = (TAF_CCM_CustomEccNumReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmCustomEccNumReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmCustomEccNumReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&ccmCustomEccNumReq->eccNumReq.eccNum, sizeof(ccmCustomEccNumReq->eccNumReq.eccNum), 0,
             sizeof(MN_CALL_BcdNum));

    return (VOS_VOID *)ccmCustomEccNumReq;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID* TAF_CCM_PrivacyMatchCcmSendFlashReq(MsgBlock *msg)
{
    TAF_CCM_SendFlashReq *ccmSendFlashReq = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmSendFlashReq = (TAF_CCM_SendFlashReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmSendFlashReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmSendFlashReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(ccmSendFlashReq->flashPara.digit, sizeof(ccmSendFlashReq->flashPara.digit), 0,
             sizeof(ccmSendFlashReq->flashPara.digit));

    ccmSendFlashReq->flashPara.digitNum = 0;

    return (VOS_VOID *)ccmSendFlashReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq(MsgBlock *msg)
{
    TAF_CCM_SendBurstDtmfReq *ccmSendBrustDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmSendBrustDtmfReq = (TAF_CCM_SendBurstDtmfReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmSendBrustDtmfReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmSendBrustDtmfReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(ccmSendBrustDtmfReq->burstDTMFPara.digit, sizeof(ccmSendBrustDtmfReq->burstDTMFPara.digit), 0,
             sizeof(ccmSendBrustDtmfReq->burstDTMFPara.digit));

    ccmSendBrustDtmfReq->burstDTMFPara.digitNum  = 0;
    ccmSendBrustDtmfReq->burstDTMFPara.onLength  = 0;
    ccmSendBrustDtmfReq->burstDTMFPara.offLength = 0;

    return (VOS_VOID *)ccmSendBrustDtmfReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmSendContDtmfReq(MsgBlock *msg)
{
    TAF_CCM_SendContDtmfReq *ccmSendContDtmfReq = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmSendContDtmfReq = (TAF_CCM_SendContDtmfReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmSendContDtmfReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmSendContDtmfReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ccmSendContDtmfReq->contDTMFPara.digit = 0;

    return (VOS_VOID *)ccmSendContDtmfReq;
}

VOS_VOID* TAF_CCM_PrivacyMatchCcmEncryptVoiceReq(MsgBlock *msg)
{
    TAF_CCM_EncryptVoiceReq *ccmEncryptVoiceReq = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmEncryptVoiceReq = (TAF_CCM_EncryptVoiceReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmEncryptVoiceReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmEncryptVoiceReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(ccmEncryptVoiceReq->encrypVoicePara.dialNumber), sizeof(TAF_ECC_CallBcdNum), 0,
             sizeof(TAF_ECC_CallBcdNum));

    return (VOS_VOID *)ccmEncryptVoiceReq;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)

VOS_VOID* TAF_CCM_PrivacyMatchCcmSetEcKmcReq(MsgBlock *msg)
{
    TAF_CCM_SetEcKmcReq *ccmSetEcKmcReq = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ccmSetEcKmcReq = (TAF_CCM_SetEcKmcReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ccmSetEcKmcReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccmSetEcKmcReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(ccmSetEcKmcReq->kmcData), sizeof(MN_CALL_AppEcKmcData), 0, sizeof(MN_CALL_AppEcKmcData));

    return (VOS_VOID *)ccmSetEcKmcReq;
}
#endif
#endif

VOS_VOID* TAF_PrivacyMatchAppQryEconfCalledInfoCnf(MsgBlock *msg)
{
    TAF_CCM_QryEconfCalledInfoCnf *qryEconfCalledInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                     length;
    VOS_UINT32                     loop = 0;
    errno_t                        memResult;

    /* 申请内存，后续统一由底层释放 */
    qryEconfCalledInfoCnf = (TAF_CCM_QryEconfCalledInfoCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (qryEconfCalledInfoCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(qryEconfCalledInfoCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    qryEconfCalledInfoCnf->numOfCalls = TAF_MIN(qryEconfCalledInfoCnf->numOfCalls, TAF_CALL_MAX_ECONF_CALLED_NUM);

    for (loop = 0; loop < qryEconfCalledInfoCnf->numOfCalls; loop++) {
        memset_s(&qryEconfCalledInfoCnf->callInfo[loop].callNumber,
                 sizeof(qryEconfCalledInfoCnf->callInfo[loop].callNumber), 0,
                 sizeof(qryEconfCalledInfoCnf->callInfo[loop].callNumber));
    }

    return (VOS_VOID *)qryEconfCalledInfoCnf;
}

VOS_VOID* TAF_PrivacyMatchAtCallIncomingInd(MsgBlock *msg)
{
    TAF_CCM_CallIncomingInd *callIncomingInd = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    callIncomingInd = (TAF_CCM_CallIncomingInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (callIncomingInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callIncomingInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&callIncomingInd->incomingIndPara.callNumber, sizeof(callIncomingInd->incomingIndPara.callNumber), 0,
             sizeof(MN_CALL_BcdNum));

    memset_s(&callIncomingInd->incomingIndPara.subCallNumber, sizeof(callIncomingInd->incomingIndPara.subCallNumber), 0,
             sizeof(MN_CALL_Subaddr));

    memset_s(&callIncomingInd->incomingIndPara.nameIndicator, sizeof(callIncomingInd->incomingIndPara.nameIndicator), 0,
             sizeof(TAF_CALL_Cnap));

    return (VOS_VOID *)callIncomingInd;
}

VOS_VOID* TAF_PrivacyMatchAtCallConnectInd(MsgBlock *msg)
{
    TAF_CCM_CallConnectInd *callConnectInd = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    callConnectInd = (TAF_CCM_CallConnectInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (callConnectInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callConnectInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&callConnectInd->connectIndPara.connectNumber, sizeof(callConnectInd->connectIndPara.connectNumber), 0,
             sizeof(MN_CALL_BcdNum));

    return (VOS_VOID *)callConnectInd;
}

VOS_VOID* TAF_PrivacyMatchAtQryCallInfoInd(MsgBlock *msg)
{
    TAF_CCM_QryCallInfoCnf *qryCallInfoCnf = VOS_NULL_PTR;
    VOS_UINT32              loop;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    qryCallInfoCnf = (TAF_CCM_QryCallInfoCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (qryCallInfoCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(qryCallInfoCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    qryCallInfoCnf->qryCallInfoPara.numOfCalls = TAF_MIN(qryCallInfoCnf->qryCallInfoPara.numOfCalls, MN_CALL_MAX_NUM);

    for (loop = 0; loop < qryCallInfoCnf->qryCallInfoPara.numOfCalls; loop++) {
        memset_s(&qryCallInfoCnf->qryCallInfoPara.callInfos[loop].callNumber,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].callNumber), 0,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].callNumber));

        memset_s(&qryCallInfoCnf->qryCallInfoPara.callInfos[loop].calledNumber,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].calledNumber), 0,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].calledNumber));

        memset_s(&qryCallInfoCnf->qryCallInfoPara.callInfos[loop].connectNumber,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].connectNumber), 0,
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].connectNumber));

        memset_s(&(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].displayName),
                 sizeof(qryCallInfoCnf->qryCallInfoPara.callInfos[loop].displayName), 0, sizeof(MN_CALL_DisplayName));
    }

    qryCallInfoCnf->qryCallInfoPara.numOfCalls = 0;

    return (VOS_VOID *)qryCallInfoCnf;
}

VOS_VOID* TAF_PrivacyMatchAtCallSsInd(MsgBlock *msg)
{
    TAF_CCM_CallSsInd *callSsInd = VOS_NULL_PTR;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    callSsInd = (TAF_CCM_CallSsInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (callSsInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callSsInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&callSsInd->ssIndPara.callNumber, sizeof(callSsInd->ssIndPara.callNumber), 0,
             sizeof(callSsInd->ssIndPara.callNumber));

    memset_s(&callSsInd->ssIndPara.ccbsFeature, sizeof(callSsInd->ssIndPara.ccbsFeature), 0,
             sizeof(TAF_SS_CcbsFeature));

    memset_s(&callSsInd->ssIndPara.ssNotify.ectIndicator, sizeof(callSsInd->ssIndPara.ssNotify.ectIndicator), 0,
             sizeof(MN_CALL_EctInd));

    return (VOS_VOID *)callSsInd;
}

VOS_VOID* TAF_PrivacyMatchAtEccNumInd(MsgBlock *msg)
{
    TAF_CCM_EccNumInd *eccNumInd = VOS_NULL_PTR;
    VOS_UINT32         loop;
    VOS_UINT32         length;
    errno_t            memResult;

    /* 申请内存，后续统一由底层释放 */
    eccNumInd = (TAF_CCM_EccNumInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (eccNumInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(eccNumInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    eccNumInd->eccNumInd.eccNumCount = TAF_MIN(eccNumInd->eccNumInd.eccNumCount, MN_CALL_MAX_EMC_NUM);

    for (loop = 0; loop < eccNumInd->eccNumInd.eccNumCount; loop++) {
        memset_s(eccNumInd->eccNumInd.customEccNumList[loop].eccNum,
                 sizeof(eccNumInd->eccNumInd.customEccNumList[loop].eccNum), 0,
                 sizeof(eccNumInd->eccNumInd.customEccNumList[loop].eccNum));
    }

    return (VOS_VOID *)eccNumInd;
}

VOS_VOID* TAF_PrivacyMatchAtQryXlemaInd(MsgBlock *msg)
{
    TAF_CCM_QryXlemaCnf *qryXlemaCnf = VOS_NULL_PTR;
    VOS_UINT32           loop;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    qryXlemaCnf = (TAF_CCM_QryXlemaCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (qryXlemaCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(qryXlemaCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    qryXlemaCnf->qryXlemaPara.eccNumCount = TAF_MIN(qryXlemaCnf->qryXlemaPara.eccNumCount, MN_CALL_MAX_EMC_NUM);

    for (loop = 0; loop < qryXlemaCnf->qryXlemaPara.eccNumCount; loop++) {
        memset_s(qryXlemaCnf->qryXlemaPara.customEccNumList[loop].eccNum,
                 sizeof(qryXlemaCnf->qryXlemaPara.customEccNumList[loop].eccNum), 0,
                 sizeof(qryXlemaCnf->qryXlemaPara.customEccNumList[loop].eccNum));
    }

    return (VOS_VOID *)qryXlemaCnf;
}

VOS_VOID* TAF_PrivacyMatchAtCnapQryCnf(MsgBlock *msg)
{
    TAF_CCM_CnapQryCnf *cnapQryCnf = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    cnapQryCnf = (TAF_CCM_CnapQryCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (cnapQryCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(cnapQryCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(cnapQryCnf->nameIndicator), sizeof(cnapQryCnf->nameIndicator), 0, sizeof(TAF_CALL_Cnap));

    return (VOS_VOID *)cnapQryCnf;
}

VOS_VOID* TAF_PrivacyMatchAtCnapInfoInd(MsgBlock *msg)
{
    TAF_CCM_CnapInfoInd *cnapQryInd = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    cnapQryInd = (TAF_CCM_CnapInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (cnapQryInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(cnapQryInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(cnapQryInd->nameIndicator), sizeof(cnapQryInd->nameIndicator), 0, sizeof(TAF_CALL_Cnap));

    return (VOS_VOID *)cnapQryInd;
}

VOS_VOID* TAF_PrivacyMatchAtQryUus1InfoCnf(MsgBlock *msg)
{
    TAF_CCM_QryUus1InfoCnf *qryUss1InfoCnf = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    qryUss1InfoCnf = (TAF_CCM_QryUus1InfoCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (qryUss1InfoCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(qryUss1InfoCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(qryUss1InfoCnf->qryUss1Info.uus1Info, sizeof(qryUss1InfoCnf->qryUss1Info.uus1Info), 0,
             sizeof(MN_CALL_Uus1Info) * MN_CALL_MAX_UUS1_MSG_NUM);

    return (VOS_VOID *)qryUss1InfoCnf;
}

VOS_VOID* TAF_PrivacyMatchAtUus1InfoInd(MsgBlock *msg)
{
    TAF_CCM_Uus1InfoInd *uss1InfoInd = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    uss1InfoInd = (TAF_CCM_Uus1InfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (uss1InfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(uss1InfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(uss1InfoInd->uus1InfoIndPara.uusInfo), sizeof(uss1InfoInd->uus1InfoIndPara.uusInfo), 0,
             sizeof(MN_CALL_Uus1Info));

    return (VOS_VOID *)uss1InfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtQryClprCnf(MsgBlock *msg)
{
    TAF_CCM_QryClprCnf *qryClprCnf = VOS_NULL_PTR;
    VOS_UINT32          length;
    errno_t             memResult;

    /* 申请内存，后续统一由底层释放 */
    qryClprCnf = (TAF_CCM_QryClprCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (qryClprCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(qryClprCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(qryClprCnf->clprCnf.clprInfo.redirectInfo.redirectNum),
             sizeof(qryClprCnf->clprCnf.clprInfo.redirectInfo.redirectNum), 0, sizeof(MN_CALL_BcdNum));

    memset_s(&(qryClprCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr),
             sizeof(qryClprCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr), 0, sizeof(MN_CALL_Subaddr));

    return (VOS_VOID *)qryClprCnf;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID* TAF_PrivacyMatchAtCalledNumInfoInd(MsgBlock *msg)
{
    TAF_CCM_CalledNumInfoInd *calledNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 申请内存，后续统一由底层释放 */
    calledNumInfoInd = (TAF_CCM_CalledNumInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (calledNumInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(calledNumInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(calledNumInfoInd->calledNumInfoPara.digit, sizeof(calledNumInfoInd->calledNumInfoPara.digit), 0,
             sizeof(calledNumInfoInd->calledNumInfoPara.digit));

    calledNumInfoInd->calledNumInfoPara.numType  = 0;
    calledNumInfoInd->calledNumInfoPara.numPlan  = 0;
    calledNumInfoInd->calledNumInfoPara.digitNum = 0;

    return (VOS_VOID *)calledNumInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtCallingNumInfoInd(MsgBlock *msg)
{
    TAF_CCM_CallingNumInfoInd *callingNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    callingNumInfoInd = (TAF_CCM_CallingNumInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (callingNumInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(callingNumInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    callingNumInfoInd->callIngNumInfoPara.numPlan  = 0;
    callingNumInfoInd->callIngNumInfoPara.numType  = 0;
    callingNumInfoInd->callIngNumInfoPara.digitNum = 0;
    callingNumInfoInd->callIngNumInfoPara.pi       = 0;
    callingNumInfoInd->callIngNumInfoPara.si       = 0;

    memset_s(callingNumInfoInd->callIngNumInfoPara.digit, sizeof(callingNumInfoInd->callIngNumInfoPara.digit), 0,
             sizeof(callingNumInfoInd->callIngNumInfoPara.digit));

    return (VOS_VOID *)callingNumInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtDisplayInfoInd(MsgBlock *msg)
{
    TAF_CCM_DisplayInfoInd *displayInfoInd = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    displayInfoInd = (TAF_CCM_DisplayInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (displayInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(displayInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    displayInfoInd->disPlayInfoIndPara.digitNum = 0;

    memset_s(displayInfoInd->disPlayInfoIndPara.digit, sizeof(displayInfoInd->disPlayInfoIndPara.digit), 0,
             sizeof(displayInfoInd->disPlayInfoIndPara.digit));

    return (VOS_VOID *)displayInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtExtDisplayInfoInd(MsgBlock *msg)
{
    TAF_CCM_ExtDisplayInfoInd *extDisplayInfoInd = VOS_NULL_PTR;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请内存，后续统一由底层释放 */
    extDisplayInfoInd = (TAF_CCM_ExtDisplayInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (extDisplayInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(extDisplayInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    extDisplayInfoInd->disPlayInfoIndPara.infoRecsDataNum = 0;
    extDisplayInfoInd->disPlayInfoIndPara.displayType     = 0;
    extDisplayInfoInd->disPlayInfoIndPara.extDispInd      = 0;

    memset_s(extDisplayInfoInd->disPlayInfoIndPara.infoRecsData,
             sizeof(extDisplayInfoInd->disPlayInfoIndPara.infoRecsData), 0,
             sizeof(extDisplayInfoInd->disPlayInfoIndPara.infoRecsData));

    return (VOS_VOID *)extDisplayInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtConnNumInfoInd(MsgBlock *msg)
{
    TAF_CCM_ConnNumInfoInd *connNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    connNumInfoInd = (TAF_CCM_ConnNumInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (connNumInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(connNumInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    connNumInfoInd->connNumInfoIndPara.numType  = 0;
    connNumInfoInd->connNumInfoIndPara.numPlan  = 0;
    connNumInfoInd->connNumInfoIndPara.pi       = 0;
    connNumInfoInd->connNumInfoIndPara.si       = 0;
    connNumInfoInd->connNumInfoIndPara.digitNum = 0;

    memset_s(connNumInfoInd->connNumInfoIndPara.digit, sizeof(connNumInfoInd->connNumInfoIndPara.digit), 0,
             sizeof(connNumInfoInd->connNumInfoIndPara.digit));

    return (VOS_VOID *)connNumInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtRedirNumInfoInd(MsgBlock *msg)
{
    TAF_CCM_RedirNumInfoInd *redirNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    redirNumInfoInd = (TAF_CCM_RedirNumInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (redirNumInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(redirNumInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    redirNumInfoInd->redirNumInfoIndPara.opPi     = 0;
    redirNumInfoInd->redirNumInfoIndPara.opSi     = 0;
    redirNumInfoInd->redirNumInfoIndPara.numType  = 0;
    redirNumInfoInd->redirNumInfoIndPara.numPlan  = 0;
    redirNumInfoInd->redirNumInfoIndPara.pi       = 0;
    redirNumInfoInd->redirNumInfoIndPara.si       = 0;
    redirNumInfoInd->redirNumInfoIndPara.digitNum = 0;

    memset_s(redirNumInfoInd->redirNumInfoIndPara.digitNumArray,
             sizeof(redirNumInfoInd->redirNumInfoIndPara.digitNumArray), 0,
             sizeof(redirNumInfoInd->redirNumInfoIndPara.digitNumArray));

    return (VOS_VOID *)redirNumInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtCcwacInfoInfoInd(MsgBlock *msg)
{
    TAF_CCM_CcwacInfoInd *ccwcInfoInd = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    ccwcInfoInd = (TAF_CCM_CcwacInfoInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (ccwcInfoInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ccwcInfoInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    ccwcInfoInd->ccwacInfoPara.numPlan         = 0;
    ccwcInfoInd->ccwacInfoPara.numType         = 0;
    ccwcInfoInd->ccwacInfoPara.alertPitch      = 0;
    ccwcInfoInd->ccwacInfoPara.digitNum        = 0;
    ccwcInfoInd->ccwacInfoPara.pi              = 0;
    ccwcInfoInd->ccwacInfoPara.si              = 0;
    ccwcInfoInd->ccwacInfoPara.signal          = 0;
    ccwcInfoInd->ccwacInfoPara.signalIsPresent = 0;
    ccwcInfoInd->ccwacInfoPara.signalType      = 0;

    memset_s(ccwcInfoInd->ccwacInfoPara.digit, sizeof(ccwcInfoInd->ccwacInfoPara.digit), 0,
             sizeof(ccwcInfoInd->ccwacInfoPara.digit));

    return (VOS_VOID *)ccwcInfoInd;
}

VOS_VOID* TAF_PrivacyMatchAtContDtmfInd(MsgBlock *msg)
{
    TAF_CCM_ContDtmfInd *contDtmfInd = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    contDtmfInd = (TAF_CCM_ContDtmfInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (contDtmfInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(contDtmfInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    contDtmfInd->contDtmfIndPara.dtmfSwitch = 0;
    contDtmfInd->contDtmfIndPara.digit      = 0;

    return (VOS_VOID *)contDtmfInd;
}

VOS_VOID* TAF_PrivacyMatchAtBurstDtmfInd(MsgBlock *msg)
{
    TAF_CCM_BurstDtmfInd *brustDtmfInd = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 申请内存，后续统一由底层释放 */
    brustDtmfInd = (TAF_CCM_BurstDtmfInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (brustDtmfInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(brustDtmfInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    brustDtmfInd->burstDtmfIndPara.onLength  = 0;
    brustDtmfInd->burstDtmfIndPara.offLength = 0;
    brustDtmfInd->burstDtmfIndPara.digitNum  = 0;

    memset_s(brustDtmfInd->burstDtmfIndPara.digit, sizeof(brustDtmfInd->burstDtmfIndPara.digit), 0,
             sizeof(brustDtmfInd->burstDtmfIndPara.digit));

    return (VOS_VOID *)brustDtmfInd;
}

VOS_VOID* TAF_PrivacyMatchAtEncryptVoiceInd(MsgBlock *msg)
{
    TAF_CCM_EncryptVoiceInd *encryptVoiceInd = VOS_NULL_PTR;
    VOS_UINT32               length;
    errno_t                  memResult;

    /* 申请内存，后续统一由底层释放 */
    encryptVoiceInd = (TAF_CCM_EncryptVoiceInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (encryptVoiceInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(encryptVoiceInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(encryptVoiceInd->encryptVoiceIndPara.callingNumber),
             sizeof(encryptVoiceInd->encryptVoiceIndPara.callingNumber), 0,
             sizeof(encryptVoiceInd->encryptVoiceIndPara.callingNumber));

    return (VOS_VOID *)encryptVoiceInd;
}

VOS_VOID* TAF_PrivacyMatchAtGetEcRandomCnf(MsgBlock *msg)
{
    TAF_CCM_GetEcRandomCnf *getEcRandomCnf = VOS_NULL_PTR;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    getEcRandomCnf = (TAF_CCM_GetEcRandomCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (getEcRandomCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(getEcRandomCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(getEcRandomCnf->ecRandomData.eccRandom, sizeof(getEcRandomCnf->ecRandomData.eccRandom), 0,
             sizeof(getEcRandomCnf->ecRandomData.eccRandom));

    return (VOS_VOID *)getEcRandomCnf;
}

VOS_VOID* TAF_PrivacyMatchAtGetEcKmcCnf(MsgBlock *msg)
{
    TAF_CCM_GetEcKmcCnf *getEcKmcCnf = VOS_NULL_PTR;
    VOS_UINT32           length;
    errno_t              memResult;

    /* 申请内存，后续统一由底层释放 */
    getEcKmcCnf = (TAF_CCM_GetEcKmcCnf *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (getEcKmcCnf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(getEcKmcCnf, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&(getEcKmcCnf->kmcCnfPara.kmcData), sizeof(getEcKmcCnf->kmcCnfPara.kmcData), 0,
             sizeof(getEcKmcCnf->kmcCnfPara.kmcData));

    return (VOS_VOID *)getEcKmcCnf;
}

#endif

#if (FEATURE_IMS == FEATURE_ON)

VOS_VOID* TAF_CCM_PrivacyMatchCcmEconfDialReq(MsgBlock *msg)
{
    TAF_CCM_EconfDialReq *econfDialReq = VOS_NULL_PTR;
    VOS_UINT32            length;
    errno_t               memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    econfDialReq = (TAF_CCM_EconfDialReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (econfDialReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(econfDialReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    memset_s(&econfDialReq->econfDialInfo.econfCalllist, sizeof(econfDialReq->econfDialInfo.econfCalllist), 0,
             sizeof(TAF_CALL_EconfCallList));

    return (VOS_VOID *)econfDialReq;
}

VOS_VOID* TAF_PrivacyMatchAtEconfNotifyInd(MsgBlock *msg)
{
    TAF_CCM_EconfNotifyInd *econfNotifyInd = VOS_NULL_PTR;
    VOS_UINT32              loop           = 0;
    VOS_UINT32              length;
    errno_t                 memResult;

    /* 申请内存，后续统一由底层释放 */
    econfNotifyInd = (TAF_CCM_EconfNotifyInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (econfNotifyInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(econfNotifyInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    econfNotifyInd->econfNotifyIndPara.numOfCalls = TAF_MIN(econfNotifyInd->econfNotifyIndPara.numOfCalls,
                                                            TAF_CALL_MAX_ECONF_CALLED_NUM);

    for (loop = 0; loop < econfNotifyInd->econfNotifyIndPara.numOfCalls; loop++) {
        memset_s(&econfNotifyInd->econfNotifyIndPara.callInfo[loop].callNumber,
                 sizeof(econfNotifyInd->econfNotifyIndPara.callInfo[loop].callNumber), 0,
                 sizeof(econfNotifyInd->econfNotifyIndPara.callInfo[loop].callNumber));
    }

    return (VOS_VOID *)econfNotifyInd;
}

#endif

#if ((FEATURE_LTEV == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_VOID* AT_PrivacyMatchLtevSourceIdSetReq(MsgBlock *msg)
{
    /* 记录申请的内存 */
    MN_APP_ReqMsg           *matchLtevSourceIdSetReq = VOS_NULL_PTR;
    VNAS_LtevSourceIdSetReq *sourceIdSetReq = VOS_NULL_PTR;
    VOS_UINT32               length = 0;
    errno_t                  memResult;

    /* 申请内存 */
    matchLtevSourceIdSetReq = (MN_APP_ReqMsg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);
    /* 如果没有申请到内存，则返回空指针 */
    if (matchLtevSourceIdSetReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    sourceIdSetReq = (VNAS_LtevSourceIdSetReq *)matchLtevSourceIdSetReq->content;

    /* 过滤敏感消息 */
    memResult = memcpy_s(matchLtevSourceIdSetReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    sourceIdSetReq->sourceId = 0;

    return (VOS_VOID *)matchLtevSourceIdSetReq;
}
#endif

VOS_VOID* TAF_PrivacyMatchDsmPsSetCustAttachProfileReq(MsgBlock *msg)
{
    TAF_PS_Msg                         *sndMsg               = VOS_NULL_PTR;
    TAF_PS_SetCustAttachProfileInfoReq *setCustAttProInfoReq = VOS_NULL_PTR;
    VOS_UINT32                          length;
    errno_t                             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Msg *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    setCustAttProInfoReq = (TAF_PS_SetCustAttachProfileInfoReq *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.userName,
        sizeof(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.userName), 0x00,
        sizeof(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.userName));

    (VOS_VOID)memset_s(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.pwd,
        sizeof(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.pwd), 0x00,
        sizeof(setCustAttProInfoReq->custAttachApn.nvAttachProfileInfo.pwd));

    return (VOS_VOID *)sndMsg;
}

VOS_VOID* TAF_DSM_PrivacyMatchPsGetCustAttachProfileCnf(MsgBlock *msg)
{
    TAF_PS_Evt                         *sndMsg               = VOS_NULL_PTR;
    TAF_PS_GetCustAttachProfileInfoCnf *getCustAttProInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          length;
    errno_t                             memResult;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (TAF_PS_Evt *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    getCustAttProInfoCnf = (TAF_PS_GetCustAttachProfileInfoCnf *)sndMsg->content;

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.userName,
        sizeof(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.userName), 0x00,
        sizeof(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.userName));

    (VOS_VOID)memset_s(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.pwd,
        sizeof(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.pwd), 0x00,
        sizeof(getCustAttProInfoCnf->custAttachApn.nvAttachProfileInfo.pwd));

    return (VOS_VOID *)sndMsg;
}

