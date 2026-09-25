/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "TafLogPrivacyMatch.h"
#include "TafAppXsmsInterface.h"
#include "TafSsaApi.h"
#include "AtMtaInterface.h"
#include "TafDrvAgent.h"
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
#include "AtXpdsInterface.h"
#endif
#include "MnMsgApi.h"
#include "TafCcmApi.h"
#include "RnicCdsInterface.h"
#include "TafPsApi.h"
#include "TafPsTypeDef.h"
#include "TafIfaceApi.h"
#include "dsm_rnic_pif.h"
#include "dsm_ndis_pif.h"
#if (FEATURE_ON == FEATURE_IMS)
#include "AtImsaInterface.h"
#endif
#include "securec.h"


#define    THIS_FILE_ID        PS_FILE_ID_TAF_LOG_PRIVACY_MATCH_C


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_UINT32 TAF_AppMnCallBackCsCallIsNeedLogPrivacy(
    MN_CALL_EVENT_ENUM_U32              enEvtId
)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_FALSE;

    switch(enEvtId)
    {
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

            ulResult = VOS_TRUE;

            break;

        default:
            break;
    }

    return ulResult;
}


VOS_UINT32 TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(
    TAF_SSA_EVT_ID_ENUM_UINT32          enEvtId
)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_FALSE;

    switch(enEvtId)
    {
        case ID_TAF_SSA_LCS_MOLR_NTF:

            ulResult = VOS_TRUE;

            break;

        default:
            break;
    }

    return ulResult;
}


VOS_VOID* TAF_PrivacyMatchAppMnCallBackCsCall(
    MsgBlock                           *pstMsg
)
{
    MN_AT_IND_EVT_STRU                 *pstAtIndEvt        = VOS_NULL_PTR;
    MN_AT_IND_EVT_STRU                 *pstPrivacyAtIndEvt = VOS_NULL_PTR;
    MN_CALL_EVENT_ENUM_U32              enEvtId;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulLen;
    errno_t                             lMemResult;

    pstAtIndEvt = (MN_AT_IND_EVT_STRU *)pstMsg;

    /* 获取当前的event类型，并判断该event是否需要脱敏 */
    enEvtId = MN_CALL_EVT_BUTT;
    lMemResult = memcpy_s(&enEvtId,  sizeof(enEvtId), pstAtIndEvt->aucContent, sizeof(enEvtId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(enEvtId), sizeof(enEvtId));

    if (VOS_FALSE == TAF_AppMnCallBackCsCallIsNeedLogPrivacy(enEvtId))
    {
        return (VOS_VOID *)pstMsg;
    }

    ulLength = pstAtIndEvt->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请消息 */
    pstPrivacyAtIndEvt = (MN_AT_IND_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                            DYNAMIC_MEM_PT,
                                                            ulLength);

    if (VOS_NULL_PTR == pstPrivacyAtIndEvt)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyAtIndEvt,
                          ulLength,
                          pstAtIndEvt,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* event占用了aucContent前四个字节，消息内容需要需要偏移4个字节 */
    ulLen = pstAtIndEvt->ulLength + VOS_MSG_HEAD_LENGTH - sizeof(MN_AT_IND_EVT_STRU);

    lMemResult = memset_s((VOS_VOID *)(pstPrivacyAtIndEvt->aucContent + sizeof(enEvtId)),
                          ulLen,
                          0,
                          ulLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLen, ulLen);

    return (VOS_VOID *)pstPrivacyAtIndEvt;
}



VOS_VOID* AT_PrivacyMatchRegisterSsMsg(
    MsgBlock                           *pstMsg
)
{
    MN_APP_REQ_MSG_STRU                *pstRegisterSs = VOS_NULL_PTR;
    TAF_SS_REGISTERSS_REQ_STRU         *pstSsRegReq   = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstRegisterSs = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                        DYNAMIC_MEM_PT,
                                                        ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstRegisterSs)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstRegisterSs,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsRegReq = (TAF_SS_REGISTERSS_REQ_STRU *)(pstRegisterSs->aucContent);

    memset_s(pstSsRegReq->aucFwdToNum,
             sizeof(pstSsRegReq->aucFwdToNum),
             0,
             sizeof(pstSsRegReq->aucFwdToNum));

    memset_s(pstSsRegReq->aucFwdToSubAddr,
             sizeof(pstSsRegReq->aucFwdToSubAddr),
             0,
             sizeof(pstSsRegReq->aucFwdToSubAddr));

    return (VOS_VOID *)pstRegisterSs;
}


VOS_VOID* AT_PrivacyMatchProcessUssMsg(
    MsgBlock                           *pstMsg
)
{
    MN_APP_REQ_MSG_STRU                *pstProcessUss = VOS_NULL_PTR;
    TAF_SS_PROCESS_USS_REQ_STRU        *pstSsReq      = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstProcessUss = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                        DYNAMIC_MEM_PT,
                                                        ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstProcessUss)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstProcessUss,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_PROCESS_USS_REQ_STRU *)pstProcessUss->aucContent;

    memset_s(&(pstSsReq->UssdStr),
             sizeof(TAF_SS_USSD_STRING_STRU),
             0,
             sizeof(TAF_SS_USSD_STRING_STRU));

    memset_s(pstSsReq->aucMsisdn,
             TAF_SS_MAX_MSISDN_LEN + 1,
             0,
             TAF_SS_MAX_MSISDN_LEN + 1);

    return (VOS_VOID *)pstProcessUss;
}


VOS_VOID* AT_PrivacyMatchInterRogateMsg(
    MsgBlock                           *pstMsg
)
{

    MN_APP_REQ_MSG_STRU                *pstInterRogate = VOS_NULL_PTR;
    TAF_SS_INTERROGATESS_REQ_STRU      *pstSsReq       = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstInterRogate = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                        DYNAMIC_MEM_PT,
                                                        ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstInterRogate)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstInterRogate,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_INTERROGATESS_REQ_STRU *)pstInterRogate->aucContent;

    memset_s(pstSsReq->aucPassword,
             sizeof(pstSsReq->aucPassword),
             0,
             TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)pstInterRogate;
}


VOS_VOID* AT_PrivacyMatchErasessMsg(
    MsgBlock                           *pstMsg
)
{

    MN_APP_REQ_MSG_STRU                *pstErasess      = VOS_NULL_PTR;
    TAF_SS_ERASESS_REQ_STRU            *pstSsReq        = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstErasess = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                      DYNAMIC_MEM_PT,
                                                      ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstErasess)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstErasess,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_ERASESS_REQ_STRU *)pstErasess->aucContent;

    memset_s(pstSsReq->aucPassword,
             sizeof(pstSsReq->aucPassword),
             0,
             TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)pstErasess;
}


VOS_VOID* AT_PrivacyMatchActivatessMsg(
    MsgBlock                           *pstMsg
)
{

    MN_APP_REQ_MSG_STRU                *pstActivatess   = VOS_NULL_PTR;
    TAF_SS_ACTIVATESS_REQ_STRU         *pstSsReq        = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstActivatess = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                      DYNAMIC_MEM_PT,
                                                      ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstActivatess)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstActivatess,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_ACTIVATESS_REQ_STRU *)pstActivatess->aucContent;

    memset_s(pstSsReq->aucPassword,
             sizeof(pstSsReq->aucPassword),
             0,
             TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)pstActivatess;
}


VOS_VOID* AT_PrivacyMatchDeactivatessMsg(
    MsgBlock                           *pstMsg
)
{

    MN_APP_REQ_MSG_STRU                *pstDeactivatess   = VOS_NULL_PTR;
    TAF_SS_DEACTIVATESS_REQ_STRU       *pstSsReq          = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstDeactivatess = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                      DYNAMIC_MEM_PT,
                                                      ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstDeactivatess)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstDeactivatess,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_DEACTIVATESS_REQ_STRU *)pstDeactivatess->aucContent;

    memset_s(pstSsReq->aucPassword,
                  sizeof(pstSsReq->aucPassword),
                  0,
                  TAF_SS_MAX_PASSWORD_LEN);

    return (VOS_VOID *)pstDeactivatess;
}


VOS_VOID* AT_PrivacyMatchRegPwdMsg(
    MsgBlock                           *pstMsg
)
{

    MN_APP_REQ_MSG_STRU                *pstRegpwdss   = VOS_NULL_PTR;
    TAF_SS_REGPWD_REQ_STRU             *pstSsReq      = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstRegpwdss = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                      DYNAMIC_MEM_PT,
                                                      ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstRegpwdss)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstRegpwdss,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_REGPWD_REQ_STRU *)pstRegpwdss->aucContent;

    memset_s(pstSsReq->aucOldPwdStr,
             sizeof(pstSsReq->aucOldPwdStr),
             0,
             TAF_SS_MAX_PASSWORD_LEN + 1);

    memset_s(pstSsReq->aucNewPwdStr,
             sizeof(pstSsReq->aucNewPwdStr),
             0,
             TAF_SS_MAX_PASSWORD_LEN + 1);

    memset_s(pstSsReq->aucNewPwdStrCnf,
             sizeof(pstSsReq->aucNewPwdStrCnf),
             0,
             TAF_SS_MAX_PASSWORD_LEN + 1);

    return (VOS_VOID *)pstRegpwdss;
}


VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(
    MsgBlock                           *pstMsg
)
{
    TAF_SSA_EVT_STRU                   *pstSsaEvt        = VOS_NULL_PTR;
    TAF_SSA_EVT_STRU                   *pstPrivacySsaEvt = VOS_NULL_PTR;
    TAF_SSA_LCS_MOLR_NTF_STRU          *pstLcsMolrNtf    = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    pstSsaEvt = (TAF_SSA_EVT_STRU *)pstMsg;

    /* 计算消息长度 */
    ulLength  = pstSsaEvt->stHeader.ulLength + VOS_MSG_HEAD_LENGTH;

    /* 根据当前的SsEvent判断是否需要脱敏 */
    if (VOS_FALSE == TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(pstSsaEvt->enEvtId))
    {
        return (VOS_VOID *)pstMsg;
    }

    /* 申请内存，后续统一由底层释放 */
    pstPrivacySsaEvt = (TAF_SSA_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                        DYNAMIC_MEM_PT,
                                                        ulLength);


    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstPrivacySsaEvt)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacySsaEvt,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    if (ID_TAF_SSA_LCS_MOLR_NTF == pstSsaEvt->enEvtId)
    {
        pstLcsMolrNtf = (TAF_SSA_LCS_MOLR_NTF_STRU *)pstPrivacySsaEvt->aucContent;

        /* 将敏感信息设置为全0 */
        memset_s(pstLcsMolrNtf->acLocationStr,
                 sizeof(pstLcsMolrNtf->acLocationStr),
                 0,
                 sizeof(pstLcsMolrNtf->acLocationStr));
    }

    return (VOS_VOID *)pstPrivacySsaEvt;
}


VOS_VOID* TAF_PrivacyMatchMnCallBackSsAtIndEvt(
    MsgBlock                           *pstMsg
)
{
    MN_AT_IND_EVT_STRU                 *pstAtIndEvt           = VOS_NULL_PTR;
    MN_AT_IND_EVT_STRU                 *pstPrivacyAtIndEvt    = VOS_NULL_PTR;
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pSsCallIndependentEvt = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;
    errno_t                             lMemResult;

    pstAtIndEvt = (MN_AT_IND_EVT_STRU *)pstMsg;
    ulLength    = pstAtIndEvt->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstPrivacyAtIndEvt = (MN_AT_IND_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                            DYNAMIC_MEM_PT,
                                                            ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstPrivacyAtIndEvt)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyAtIndEvt,
                          ulLength,
                          pstAtIndEvt,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pSsCallIndependentEvt = (TAF_SS_CALL_INDEPENDENT_EVENT_STRU *)(pstPrivacyAtIndEvt->aucContent);

    memset_s(&(pSsCallIndependentEvt->FwdInfo),
             sizeof(TAF_SS_FORWARDINGINFO_STRU),
             0,
             sizeof(TAF_SS_FORWARDINGINFO_STRU));

    memset_s(&(pSsCallIndependentEvt->FwdFeaturelist),
             sizeof(TAF_SS_FWDFEATURELIST_STRU),
             0,
             sizeof(TAF_SS_FWDFEATURELIST_STRU));

    memset_s(pSsCallIndependentEvt->aucPassWord,
             TAF_SS_MAX_PASSWORD_LEN,
             0,
             TAF_SS_MAX_PASSWORD_LEN);

    memset_s(&(pSsCallIndependentEvt->UssdString),
             sizeof(TAF_SS_USSD_STRING_STRU),
             0,
             sizeof(TAF_SS_USSD_STRING_STRU));

    memset_s(pSsCallIndependentEvt->aucMsisdn,
             TAF_SS_MAX_MSISDN_LEN + 1,
             0,
             TAF_SS_MAX_MSISDN_LEN + 1);

    for (i = 0; i < TAF_SS_MAX_NUM_OF_CCBS_FEATURE; i++)
    {
        memset_s(pSsCallIndependentEvt->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].aucBSubscriberNum,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1,
                 0,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1);

        memset_s(pSsCallIndependentEvt->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].aucBSubscriberSubAddr,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_SUBADDRESS + 1,
                 0,
                 TAF_SS_MAX_NUM_OF_BSUBSCRIBER_SUBADDRESS + 1);
    }

    return (VOS_VOID *)pstPrivacyAtIndEvt;
}


VOS_VOID* TAF_PrivacyMatchAppMnCallBackSs(
    MsgBlock                           *pstMsg
)
{

    TAF_SSA_EVT_STRU                   *pstSsaEvt = VOS_NULL_PTR;

    /* 由于MN_CALLBACK_SS在发送时可能会通过两种不同的结构体(TAF_SSA_EVT_STRU/MN_AT_IND_EVT_STRU)进行填充，
        处理逻辑:首先将pstMsg强转成TAF_SSA_EVT_STRU类型指针，并判断ulEvtExt字段，若ulEvtExt字段为0，则按
        TAF_SSA_EVT_STRU进行解析并脱敏，否则按MN_AT_IND_EVT_STRU。
     */
    pstSsaEvt = (TAF_SSA_EVT_STRU *)pstMsg;

    /* 根据ulEvtExt字段判断该消息是否是LCS相关的上报，如果是，作单独脱敏处理 */
    if (0 == pstSsaEvt->ulEvtExt)
    {
        /* 走到此处，表示MN_CALLBACK_SS在上报时是通过TAF_SSA_EVT_STRU填充的 */

        return TAF_PrivacyMatchMnCallBackSsLcsEvt(pstMsg);
    }
    else
    {
        /* 走到此处，表示MN_CALLBACK_SS在上报时是通过MN_AT_IND_EVT_STRU填充的 */

        return TAF_PrivacyMatchMnCallBackSsAtIndEvt(pstMsg);
    }
}


VOS_VOID* TAF_PrivacyMatchAtCallBackQryProc(
    MsgBlock                           *pstMsg
)
{
    MN_AT_IND_EVT_STRU                 *pstSrcMsg    = VOS_NULL_PTR;
    VOS_UINT8                          *pucSendAtMsg = VOS_NULL_PTR;
    VOS_UINT8                          *pucMsgBuf    = VOS_NULL_PTR;
    TAF_PH_ICC_ID_STRU                 *pstIccId     = VOS_NULL_PTR;
    TAF_UINT8                           ucQryEvtId;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;
    VOS_UINT16                          usLen;
    VOS_UINT16                          usErrorCode;

    pstSrcMsg = (MN_AT_IND_EVT_STRU *)pstMsg;

    /* 取出qry evt type */
    ucQryEvtId = pstSrcMsg->aucContent[3];

    if (TAF_PH_ICC_ID != ucQryEvtId)
    {
        return (VOS_VOID *)pstMsg;
    }

    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请消息 */
    pucSendAtMsg = (VOS_UINT8 *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                             DYNAMIC_MEM_PT,
                                             ulLength);

    if (VOS_NULL_PTR == pucSendAtMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pucSendAtMsg, ulLength, pstMsg, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pucMsgBuf = pucSendAtMsg + sizeof(MN_AT_IND_EVT_STRU) - 4;

    if (TAF_PH_ICC_ID == pucMsgBuf[3])
    {
        lMemResult = memcpy_s(&usErrorCode, sizeof(usErrorCode), &pucMsgBuf[4], sizeof(usErrorCode));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(usErrorCode), sizeof(usErrorCode));
        lMemResult = memcpy_s(&usLen, sizeof(usLen), &pucMsgBuf[6], sizeof(usLen));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(usLen), sizeof(usLen));

        if ((sizeof(TAF_PH_ICC_ID_STRU) == usLen) && (TAF_ERR_NO_ERROR == usErrorCode))
        {
            pstIccId = (TAF_PH_ICC_ID_STRU *)(pucMsgBuf + 8);

            /* 将敏感信息设置为全0 */
            memset_s(pstIccId->aucIccId,
                     sizeof(pstIccId->aucIccId),
                     0,
                     sizeof(pstIccId->aucIccId));
        }
    }

    return (VOS_VOID *)pucSendAtMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID*  TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_APP_AT_CNF_STRU           *pstMatchTafXsmsAppAtCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchTafXsmsAppAtCnf  = (TAF_XSMS_APP_AT_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchTafXsmsAppAtCnf)
    {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchTafXsmsAppAtCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    memset_s(&(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
             sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
             0,
             sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent));

    return (VOS_VOID *)pstMatchTafXsmsAppAtCnf;
}


VOS_VOID*  TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_APP_AT_CNF_STRU           *pstMatchTafXsmsAppAtCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchTafXsmsAppAtCnf  = (TAF_XSMS_APP_AT_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchTafXsmsAppAtCnf)
    {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchTafXsmsAppAtCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    memset_s(&(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
             sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
             0,
             sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent));

    return (VOS_VOID *)pstMatchTafXsmsAppAtCnf;
}


VOS_VOID*  AT_PrivacyMatchAppMsgTypeSendReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_SEND_MSG_REQ_STRU         *pstMatchTafXsmsSendMsgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchTafXsmsSendMsgReq  = (TAF_XSMS_SEND_MSG_REQ_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchTafXsmsSendMsgReq)
    {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchTafXsmsSendMsgReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    memset_s(&(pstMatchTafXsmsSendMsgReq->st1XSms.stAddr),
             sizeof(TAF_XSMS_ADDR_STRU),
             0,
             sizeof(TAF_XSMS_ADDR_STRU));

    memset_s(&(pstMatchTafXsmsSendMsgReq->st1XSms.stSubAddr),
             sizeof(TAF_XSMS_SUB_ADDR_STRU),
             0,
             sizeof(TAF_XSMS_SUB_ADDR_STRU));

    memset_s(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData,
             sizeof(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData),
             0,
             sizeof(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData));

    pstMatchTafXsmsSendMsgReq->st1XSms.ulBearerDataLen = 0;

    return (VOS_VOID *)pstMatchTafXsmsSendMsgReq;
}
#endif



VOS_VOID*  AT_PrivacyMatchCposSetReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgCposSetReq = VOS_NULL_PTR;
    AT_MTA_CPOS_REQ_STRU               *pstCposReq               = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchAppMsgCposSetReq  = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                    DYNAMIC_MEM_PT,
                                                                    ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchAppMsgCposSetReq)
    {
        return VOS_NULL_PTR;
    }

    pstCposReq = (AT_MTA_CPOS_REQ_STRU *)pstMatchAppMsgCposSetReq->aucContent;

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchAppMsgCposSetReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    memset_s(pstCposReq->acXmlText,
             sizeof(pstCposReq->acXmlText),
             0x00,
             sizeof(pstCposReq->acXmlText));

    return (VOS_VOID *)pstMatchAppMsgCposSetReq;
}


VOS_VOID*  AT_PrivacyMatchSimLockWriteExSetReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                          *pstMatchAppMsgSimlockWriteExSetReq = VOS_NULL_PTR;
    DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU        *pstSimlockWriteExSetReq = VOS_NULL_PTR;
    VOS_UINT32                                    ulLength;
    errno_t                                       lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchAppMsgSimlockWriteExSetReq  = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                    DYNAMIC_MEM_PT,
                                                                    ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchAppMsgSimlockWriteExSetReq)
    {
        return VOS_NULL_PTR;
    }

    pstSimlockWriteExSetReq = (DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU *)pstMatchAppMsgSimlockWriteExSetReq->aucContent;

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchAppMsgSimlockWriteExSetReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    lMemResult = memset_s(pstSimlockWriteExSetReq,
                          (ulLength - sizeof(MN_APP_REQ_MSG_STRU) + sizeof(pstMatchAppMsgSimlockWriteExSetReq->aucContent)),
                          0x00,
                          sizeof(DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength - sizeof(MN_APP_REQ_MSG_STRU) + sizeof(pstMatchAppMsgSimlockWriteExSetReq->aucContent),
                        sizeof(DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU));

    return (VOS_VOID *)pstMatchAppMsgSimlockWriteExSetReq;
}

#if (FEATURE_ON == FEATURE_IMS)

VOS_VOID* AT_PrivacyMatchImsaImsCtrlMsg(
    MsgBlock                                               *pstMsg
)
{
    AT_IMSA_IMS_CTRL_MSG_STRU          *pstImsCtrlMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstImsCtrlMsg = (AT_IMSA_IMS_CTRL_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                              DYNAMIC_MEM_PT,
                                                              ulLength);

    if (VOS_NULL_PTR == pstImsCtrlMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstImsCtrlMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    lMemResult = memset_s(pstImsCtrlMsg->aucWifiMsg,
                          pstImsCtrlMsg->ulWifiMsgLen,
                          0,
                          pstImsCtrlMsg->ulWifiMsgLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, pstImsCtrlMsg->ulWifiMsgLen, pstImsCtrlMsg->ulWifiMsgLen);

    return (VOS_VOID *)pstImsCtrlMsg;
}


VOS_VOID* AT_PrivacyMatchImsaNickNameSetReq(
    MsgBlock                                               *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgNickNameSetReq = VOS_NULL_PTR;
    IMSA_AT_NICKNAME_INFO_STRU         *pstNickNameInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstMatchAppMsgNickNameSetReq = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    if (VOS_NULL_PTR == pstMatchAppMsgNickNameSetReq)
    {
        return VOS_NULL_PTR;
    }

    pstNickNameInfo = (IMSA_AT_NICKNAME_INFO_STRU *)pstMatchAppMsgNickNameSetReq->aucContent;

    lMemResult = memcpy_s(pstMatchAppMsgNickNameSetReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstNickNameInfo->acNickName,
             MN_CALL_DISPLAY_NAME_STRING_SZ,
             0,
             MN_CALL_DISPLAY_NAME_STRING_SZ);

    return (VOS_VOID *)pstMatchAppMsgNickNameSetReq;
}
#endif


VOS_VOID*  AT_PrivacyMatchMeidSetReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgSetReq = VOS_NULL_PTR;
    AT_MTA_MEID_SET_REQ_STRU           *pstMeidReq           = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchAppMsgSetReq  = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                DYNAMIC_MEM_PT,
                                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchAppMsgSetReq)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstMatchAppMsgSetReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMeidReq = (AT_MTA_MEID_SET_REQ_STRU *)pstMatchAppMsgSetReq->aucContent;

    /* 过滤敏感消息 */
    memset_s(pstMeidReq->aucMeid,
             sizeof(pstMeidReq->aucMeid),
             0x00,
             sizeof(pstMeidReq->aucMeid));

    return (VOS_VOID *)pstMatchAppMsgSetReq;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID*  AT_PrivacyMatchAppMsgTypeWriteReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_WRITE_MSG_REQ_STRU        *pstMatchTafXsmsWriteMsgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchTafXsmsWriteMsgReq  = (TAF_XSMS_WRITE_MSG_REQ_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                              DYNAMIC_MEM_PT,
                                                                              ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchTafXsmsWriteMsgReq)
    {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchTafXsmsWriteMsgReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    memset_s(&(pstMatchTafXsmsWriteMsgReq->st1XSms.stAddr),
             sizeof(TAF_XSMS_ADDR_STRU),
             0,
             sizeof(TAF_XSMS_ADDR_STRU));

    memset_s(&(pstMatchTafXsmsWriteMsgReq->st1XSms.stSubAddr),
             sizeof(TAF_XSMS_SUB_ADDR_STRU),
             0,
             sizeof(TAF_XSMS_SUB_ADDR_STRU));

    memset_s(pstMatchTafXsmsWriteMsgReq->st1XSms.aucBearerData,
             sizeof(pstMatchTafXsmsWriteMsgReq->st1XSms.aucBearerData),
             0,
             sizeof(pstMatchTafXsmsWriteMsgReq->st1XSms.aucBearerData));

    pstMatchTafXsmsWriteMsgReq->st1XSms.ulBearerDataLen = 0;

    return (VOS_VOID *)pstMatchTafXsmsWriteMsgReq;
}


VOS_VOID*  AT_PrivacyMatchAppMsgTypeDeleteReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_DELETE_MSG_REQ_STRU       *pstMatchTafXsmsDeleteMsgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存 */
    pstMatchTafXsmsDeleteMsgReq  = (TAF_XSMS_DELETE_MSG_REQ_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                                DYNAMIC_MEM_PT,
                                                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstMatchTafXsmsDeleteMsgReq)
    {
        return VOS_NULL_PTR;
    }

    /* 过滤敏感消息 */
    lMemResult = memcpy_s(pstMatchTafXsmsDeleteMsgReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMatchTafXsmsDeleteMsgReq->ucIndex = 0;

    return (VOS_VOID *)pstMatchTafXsmsDeleteMsgReq;
}

#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))

VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(
    MsgBlock                           *pstMsg
)
{
    AT_XPDS_GPS_POS_INFO_RSP_STRU      *pstPrivacyMatchMsg  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_GPS_POS_INFO_RSP_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchMsg,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    memset_s(&(pstPrivacyMatchMsg->stPosInfo),
             sizeof(AT_XPDS_GPS_POS_INFO_STRU),
             0,
             sizeof(AT_XPDS_GPS_POS_INFO_STRU));

    return (VOS_VOID *)pstPrivacyMatchMsg;
}


VOS_VOID* AT_PrivacyMatchCagpsPrmInfoRsp(
    MsgBlock                           *pstMsg
)
{
    AT_XPDS_GPS_PRM_INFO_RSP_STRU      *pstPrivacyMatchMsg  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    VOS_UINT32                          ulMaxPrmDataLen;
    errno_t                             lMemResult;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_GPS_PRM_INFO_RSP_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchMsg,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    ulMaxPrmDataLen = sizeof(AT_XPDS_GPS_MODEM_PRMDATA_STRU) * TAF_MSG_CDMA_MAX_SV_NUM;

    lMemResult = memset_s(pstPrivacyMatchMsg->astMseasData,
                          ulMaxPrmDataLen,
                          0,
                          sizeof(pstPrivacyMatchMsg->astMseasData));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxPrmDataLen, sizeof(pstPrivacyMatchMsg->astMseasData));

    pstPrivacyMatchMsg->ucMeasNum = 0;

    return (VOS_VOID *)pstPrivacyMatchMsg;
}


VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(
    MsgBlock                           *pstMsg
)
{
    AT_XPDS_AP_FORWARD_DATA_IND_STRU   *pstPrivacyMatchMsg  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength     = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_AP_FORWARD_DATA_IND_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchMsg,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    lMemResult = memset_s(pstPrivacyMatchMsg->aucData,
                          pstPrivacyMatchMsg->ulDataLen,
                          0,
                          pstPrivacyMatchMsg->ulDataLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, pstPrivacyMatchMsg->ulDataLen, pstPrivacyMatchMsg->ulDataLen);

    pstPrivacyMatchMsg->ulDataLen = 0;

    return (VOS_VOID *)pstPrivacyMatchMsg;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_REFLOC_INFO_CNF_STRU   *pstPrivacyMatchRefLocInfo   = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchRefLocInfo   = (XPDS_AT_GPS_REFLOC_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                   DYNAMIC_MEM_PT,
                                                                                   ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchRefLocInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchRefLocInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    /* 替换RefLoc信息敏感数据 */
    memset_s(&(pstPrivacyMatchRefLocInfo->stRefLoc),
             sizeof(XPDS_AT_GPS_REFLOC_INFO_STRU),
             0,
             sizeof(XPDS_AT_GPS_REFLOC_INFO_STRU));

    return (VOS_VOID *)pstPrivacyMatchRefLocInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_ION_INFO_IND_STRU      *pstPrivacyMatchIonInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchIonInfo = (XPDS_AT_GPS_ION_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchIonInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchIonInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    /* 清空敏感位置信息 */
    pstPrivacyMatchIonInfo->ucABParIncl     = 0;
    pstPrivacyMatchIonInfo->ucAlpha0        = 0;
    pstPrivacyMatchIonInfo->ucAlpha1        = 0;
    pstPrivacyMatchIonInfo->ucAlpha2        = 0;
    pstPrivacyMatchIonInfo->ucAlpha3        = 0;
    pstPrivacyMatchIonInfo->ucBeta0         = 0;
    pstPrivacyMatchIonInfo->ucBeta1         = 0;
    pstPrivacyMatchIonInfo->ucBeta2         = 0;
    pstPrivacyMatchIonInfo->ucBeta3         = 0;

    return (VOS_VOID *)pstPrivacyMatchIonInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsEphInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_EPH_INFO_IND_STRU      *pstPrivacyMatchEphInfo  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    VOS_UINT32                          ulMaxEphDataLen;
    errno_t                             lMemResult;

    ulMsgLength             = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchEphInfo  = (XPDS_AT_GPS_EPH_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchEphInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchEphInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    ulMaxEphDataLen = sizeof(XPDS_AT_EPH_DATA_STRU) * TAF_MSG_CDMA_MAX_EPH_PRN_NUM;

    /* 替换星历信息敏感数据 */
    lMemResult = memset_s(pstPrivacyMatchEphInfo->astEphData,
                          ulMaxEphDataLen,
                          0,
                          sizeof(pstPrivacyMatchEphInfo->astEphData));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxEphDataLen, sizeof(pstPrivacyMatchEphInfo->astEphData));

    pstPrivacyMatchEphInfo->ucSvNum = 0;

    return (VOS_VOID *)pstPrivacyMatchEphInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_ALM_INFO_IND_STRU      *pstPrivacyMatchAlmInfo  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    VOS_UINT32                          ulMaxAlmDataLen;
    errno_t                             lMemResult;

    ulMsgLength             = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchAlmInfo  = (XPDS_AT_GPS_ALM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchAlmInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchAlmInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    ulMaxAlmDataLen = sizeof(XPDS_AT_ALM_DATA_STRU) * TAF_MSG_CDMA_MAX_ALM_PRN_NUM;

    lMemResult = memset_s(pstPrivacyMatchAlmInfo->astAlmData,
                          ulMaxAlmDataLen,
                          0,
                          sizeof(pstPrivacyMatchAlmInfo->astAlmData));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxAlmDataLen, sizeof(pstPrivacyMatchAlmInfo->astAlmData));


    pstPrivacyMatchAlmInfo->ucWeekNum = 0;
    pstPrivacyMatchAlmInfo->ucToa     = 0;
    pstPrivacyMatchAlmInfo->ucSvNum   = 0;

    return (VOS_VOID *)pstPrivacyMatchAlmInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_PDE_POSI_INFO_IND_STRU *pstPrivacyMatchPosiInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchPosiInfo = (XPDS_AT_GPS_PDE_POSI_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                 DYNAMIC_MEM_PT,
                                                                                 ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchPosiInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchPosiInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    /* 清空位置信息 */
    pstPrivacyMatchPosiInfo->lClockBias         = 0;
    pstPrivacyMatchPosiInfo->ucFixType          = 0;
    pstPrivacyMatchPosiInfo->sLocUncAng         = 0;
    pstPrivacyMatchPosiInfo->sClockDrift        = 0;
    pstPrivacyMatchPosiInfo->lClockBias         = 0;
    pstPrivacyMatchPosiInfo->lLatitude          = 0;
    pstPrivacyMatchPosiInfo->lLongitude         = 0;
    pstPrivacyMatchPosiInfo->ulLocUncA          = 0;
    pstPrivacyMatchPosiInfo->ulLocUncP          = 0;
    pstPrivacyMatchPosiInfo->ulVelocityHor      = 0;
    pstPrivacyMatchPosiInfo->ulHeading          = 0;
    pstPrivacyMatchPosiInfo->lHeight            = 0;
    pstPrivacyMatchPosiInfo->lVerticalVelo      = 0;
    pstPrivacyMatchPosiInfo->ulLocUncV          = 0;


    return (VOS_VOID *)pstPrivacyMatchPosiInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_ACQ_ASSIST_DATA_IND_STRU                   *pstPrivacyMatchAssistData   = VOS_NULL_PTR;
    VOS_UINT32                                              ulMsgLength;
    VOS_UINT32                                              ulMaxAssistDataLen;
    errno_t                                                 lMemResult;

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchAssistData   = (XPDS_AT_GPS_ACQ_ASSIST_DATA_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                       DYNAMIC_MEM_PT,
                                                                                       ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchAssistData)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchAssistData,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    ulMaxAssistDataLen = sizeof(TAF_MSG_CDMA_ACQASSIST_DATA_STRU) * TAF_MSG_CDMA_MAX_SV_NUM;

    lMemResult = memset_s(pstPrivacyMatchAssistData->astAaData,
                          ulMaxAssistDataLen,
                          0,
                          sizeof(pstPrivacyMatchAssistData->astAaData));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxAssistDataLen, sizeof(pstPrivacyMatchAssistData->astAaData));

    pstPrivacyMatchAssistData->ucSvNum  = 0;
    pstPrivacyMatchAssistData->ulRefTow = 0;

    return (VOS_VOID *)pstPrivacyMatchAssistData;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtApReverseDataInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_AP_REVERSE_DATA_IND_STRU   *pstPrivacyMatchReverseData  = VOS_NULL_PTR;
    XPDS_AT_AP_REVERSE_DATA_IND_STRU   *pstMsgReverseDataInd        = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    pstMsgReverseDataInd = (XPDS_AT_AP_REVERSE_DATA_IND_STRU *)pstMsg;

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchReverseData = (XPDS_AT_AP_REVERSE_DATA_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                  DYNAMIC_MEM_PT,
                                                                                  ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchReverseData)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchReverseData,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    /* 清理用户隐私信息 */
    lMemResult = memset_s(pstPrivacyMatchReverseData->aucData,
                          pstMsgReverseDataInd->ulDataLen,
                          0,
                          pstMsgReverseDataInd->ulDataLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, pstMsgReverseDataInd->ulDataLen, pstMsgReverseDataInd->ulDataLen);

    pstPrivacyMatchReverseData->ulDataLen    = 0;
    pstPrivacyMatchReverseData->enServerMode = 0;

    return (VOS_VOID *)pstPrivacyMatchReverseData;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_UTS_GPS_POS_INFO_IND_STRU  *pstPrivacyMatchUtsPosInfo   = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;
    errno_t                             lMemResult;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchUtsPosInfo = (XPDS_AT_UTS_GPS_POS_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                  DYNAMIC_MEM_PT,
                                                                                  ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchUtsPosInfo)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPrivacyMatchUtsPosInfo,
                          ulMsgLength,
                          pstMsg,
                          ulMsgLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulMsgLength, ulMsgLength);

    memset_s((VOS_UINT8 *)&pstPrivacyMatchUtsPosInfo->stPosInfo,
             sizeof(AT_XPDS_GPS_POS_INFO_STRU),
             0,
             sizeof(AT_XPDS_GPS_POS_INFO_STRU));

    return (VOS_VOID *)pstPrivacyMatchUtsPosInfo;
}
#endif
#endif


VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg   = VOS_NULL_PTR;
    MTA_AT_CPOSR_IND_STRU              *pstCposrInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstCposrInd = (MTA_AT_CPOSR_IND_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(pstCposrInd->acXmlText,
             MTA_CPOSR_XML_MAX_LEN + 1,
             0,
             MTA_CPOSR_XML_MAX_LEN + 1);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MTA_PrivacyMatchAtMeidQryCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_MEID_QRY_CNF_STRU           *pstMeidQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMeidQryCnf = (MTA_AT_MEID_QRY_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(pstMeidQryCnf->aucEFRUIMID,
             MTA_AT_EFRUIMID_OCTET_LEN_EIGHT,
             0,
             MTA_AT_EFRUIMID_OCTET_LEN_EIGHT);

    memset_s(pstMeidQryCnf->aucMeID,
             sizeof(pstMeidQryCnf->aucMeID),
             0,
             sizeof(pstMeidQryCnf->aucMeID));

    memset_s(pstMeidQryCnf->aucPEsn,
             sizeof(pstMeidQryCnf->aucPEsn),
             0,
             sizeof(pstMeidQryCnf->aucPEsn));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MTA_PrivacyMatchAtCgsnQryCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_CGSN_QRY_CNF_STRU           *pstCgsnQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstCgsnQryCnf = (MTA_AT_CGSN_QRY_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(pstCgsnQryCnf->aucImei,
             NV_ITEM_IMEI_SIZE,
             0,
             NV_ITEM_IMEI_SIZE);

    return (VOS_VOID *)pstSndMsg;
}




VOS_VOID* TAF_MMA_PrivacyMatchAtUsimStatusInd(
    MsgBlock                           *pstMsg
)
{
    AT_MMA_USIM_STATUS_IND_STRU        *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MMA_USIM_STATUS_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                            DYNAMIC_MEM_PT,
                                                            ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg, ulLength, pstMsg, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstSndMsg->aucIMSI[4],
             NAS_MAX_IMSI_LENGTH - 4,
             0,
             NAS_MAX_IMSI_LENGTH - 4);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtHomePlmnQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_HOME_PLMN_QRY_CNF_STRU     *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_HOME_PLMN_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg, ulLength, pstMsg, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstSndMsg->stEHplmnInfo.aucImsi[4],
             NAS_MAX_IMSI_LENGTH - 4,
             0,
             NAS_MAX_IMSI_LENGTH - 4);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtLocationInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_LOCATION_INFO_QRY_CNF_STRU *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_LOCATION_INFO_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                   DYNAMIC_MEM_PT,
                                                                   ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->ulLac    = 0;
    pstSndMsg->ucRac    = 0;
    pstSndMsg->stCellId.ulCellIdHighBit = 0;
    pstSndMsg->stCellId.ulCellIdLowBit  = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtRegStatusInd(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_REG_STATUS_IND_STRU        *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_REG_STATUS_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                            DYNAMIC_MEM_PT,
                                                            ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstSndMsg->stRegStatus.CellId.astCellId,
             sizeof(pstSndMsg->stRegStatus.CellId.astCellId),
             0,
             sizeof(pstSndMsg->stRegStatus.CellId.astCellId));

    pstSndMsg->stRegStatus.ulLac = 0;
    pstSndMsg->stRegStatus.ucRac = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_SRCHED_PLMN_INFO_IND_STRU  *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_SRCHED_PLMN_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_MAX_SRCHED_LAI_NUM; i++)
    {
        pstSndMsg->astLai[i].ulLac = 0;
    }

    return (VOS_VOID *)pstSndMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_CDMA_LOCINFO_QRY_CNF_STRU  *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_CDMA_LOCINFO_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stClocinfoPara.ulBaseId = 0;

    return (VOS_VOID *)pstSndMsg;
}
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */


VOS_VOID* TAF_MMA_PrivacyMatchAtNetScanCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_NET_SCAN_CNF_STRU          *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_NET_SCAN_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                          DYNAMIC_MEM_PT,
                                                          ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_NET_SCAN_MAX_FREQ_NUM; i++)
    {
        pstSndMsg->astNetScanInfo[i].stCellId.ulCellIdHighBit = 0;
        pstSndMsg->astNetScanInfo[i].stCellId.ulCellIdLowBit  = 0;
        pstSndMsg->astNetScanInfo[i].ulLac          = 0;
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtRegStateQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_REG_STATE_QRY_CNF_STRU     *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_REG_STATE_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stRegInfo.ulLac = 0;
    pstSndMsg->stRegInfo.ucRac = 0;

    memset_s(pstSndMsg->stRegInfo.CellId.astCellId,
             sizeof(pstSndMsg->stRegInfo.CellId.astCellId),
             0,
             sizeof(pstSndMsg->stRegInfo.CellId.astCellId));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtClocInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_CLOCINFO_IND_STRU          *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_CLOCINFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                          DYNAMIC_MEM_PT,
                                                          ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stClocinfoPara.ulBaseId = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtRejInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_REJINFO_QRY_CNF_STRU       *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_REJINFO_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                             DYNAMIC_MEM_PT,
                                                             ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stPhoneRejInfo.ucRac    = 0;
    pstSndMsg->stPhoneRejInfo.ulLac    = 0;
    pstSndMsg->stPhoneRejInfo.stCellId.ulCellIdHighBit = 0;
    pstSndMsg->stPhoneRejInfo.stCellId.ulCellIdLowBit  = 0;

    return (VOS_VOID *)pstSndMsg;
}



VOS_VOID* RNIC_PrivacyMatchCdsImsDataReq(
    MsgBlock                           *pstMsg
)
{
    RNIC_CDS_IMS_DATA_REQ_STRU         *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (RNIC_CDS_IMS_DATA_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                           DYNAMIC_MEM_PT,
                                                           ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    lMemResult = memset_s(pstSndMsg->aucData,
                          pstSndMsg->usDataLen,
                          0,
                          pstSndMsg->usDataLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, pstSndMsg->usDataLen, pstSndMsg->usDataLen);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsCallOrigReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_CALL_ORIG_REQ_STRU          *pstCallOrigReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstCallOrigReq = (TAF_PS_CALL_ORIG_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv4Addr,
             sizeof(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    memset_s(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv6Addr,
             sizeof(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    memset_s(pstCallOrigReq->stDialParaInfo.aucPassWord,
             sizeof(pstCallOrigReq->stDialParaInfo.aucPassWord),
             0,
             TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

    memset_s(pstCallOrigReq->stDialParaInfo.aucUserName,
             sizeof(pstCallOrigReq->stDialParaInfo.aucUserName),
             0,
             TAF_MAX_AUTHDATA_USERNAME_LEN + 1);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsPppDialOrigReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_PPP_DIAL_ORIG_REQ_STRU      *pstDialOrigReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstDialOrigReq = (TAF_PS_PPP_DIAL_ORIG_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(&(pstDialOrigReq->stPppDialParaInfo.stPppReqConfigInfo.stAuth.enAuthContent),
             sizeof(pstDialOrigReq->stPppDialParaInfo.stPppReqConfigInfo.stAuth.enAuthContent),
             0,
             sizeof(pstDialOrigReq->stPppDialParaInfo.stPppReqConfigInfo.stAuth.enAuthContent));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                                *pstSndMsg        = VOS_NULL_PTR;
    TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU      *pstSetPdpCtxReq  = VOS_NULL_PTR;
    VOS_UINT32                                      ulLength;
    errno_t                                         lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstSetPdpCtxReq = (TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv4Addr,
             sizeof(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    memset_s(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv6Addr,
             sizeof(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsSetTftInfoReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg        = VOS_NULL_PTR;
    TAF_PS_SET_TFT_INFO_REQ_STRU       *pstSetTftInfoReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstSetTftInfoReq = (TAF_PS_SET_TFT_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstSetTftInfoReq->stTftInfo.aucLocalIpv4Addr,
             sizeof(pstSetTftInfoReq->stTftInfo.aucLocalIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    memset_s(pstSetTftInfoReq->stTftInfo.aucLocalIpv6Addr,
             sizeof(pstSetTftInfoReq->stTftInfo.aucLocalIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    memset_s(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv4Addr,
             sizeof(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    memset_s(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv6Addr,
             sizeof(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    pstSetTftInfoReq->stTftInfo.ucLocalIpv6Prefix = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsEpdgCtrluNtf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                        *pstSndMsg               = VOS_NULL_PTR;
    TAF_PS_EPDG_CTRLU_NTF_STRU             *pstEpdgCtrluNtf         = VOS_NULL_PTR;
    VOS_UINT32                              ulLength;
    errno_t                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstEpdgCtrluNtf = (TAF_PS_EPDG_CTRLU_NTF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    if (pstEpdgCtrluNtf->stEpdgCtrlu.bitOpActReq == VOS_TRUE)
    {
        /* IPV4 地址 */
        memset_s(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV4Addr,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV4Addr),
                 0x00,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV4Addr));

        /* IPV6 地址 */
        memset_s(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV6Addr,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV6Addr),
                 0x00,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPdpAddr.aucIpV6Addr));

        /* IPV4 PCSCF 地址 */
        memset_s(&(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPcscf),
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPcscf),
                 0x00,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stPcscf));

        /* IPV6 PCSCF 地址 */
        memset_s(&(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stIPv6Pcscf),
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stIPv6Pcscf),
                 0x00,
                 sizeof(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo.stIPv6Pcscf));
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallHandoverRstNtf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                        *pstSndMsg               = VOS_NULL_PTR;
    TAF_PS_CALL_HANDOVER_RESULT_NTF_STRU   *pstHandoverRstNtf       = VOS_NULL_PTR;
    VOS_UINT32                              ulLength;
    errno_t                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstHandoverRstNtf = (TAF_PS_CALL_HANDOVER_RESULT_NTF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pstHandoverRstNtf->stPdpInfo.aucIpv4Addr,
             sizeof(pstHandoverRstNtf->stPdpInfo.aucIpv4Addr),
             0x00,
             sizeof(pstHandoverRstNtf->stPdpInfo.aucIpv4Addr));

    /* IPV6 地址 */
    memset_s(pstHandoverRstNtf->stPdpInfo.aucIpv6Addr,
             sizeof(pstHandoverRstNtf->stPdpInfo.aucIpv6Addr),
             0x00,
             sizeof(pstHandoverRstNtf->stPdpInfo.aucIpv6Addr));

    /* IPV4 PCSCF 地址 */
    memset_s(pstHandoverRstNtf->stPdpInfo.stIpv4PcscfList.astIpv4PcscfAddrList,
             sizeof(pstHandoverRstNtf->stPdpInfo.stIpv4PcscfList.astIpv4PcscfAddrList),
             0x00,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pstHandoverRstNtf->stPdpInfo.stIpv6PcscfList.astIpv6PcscfAddrList,
             sizeof(pstHandoverRstNtf->stPdpInfo.stIpv6PcscfList.astIpv6PcscfAddrList),
             0x00,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsEpdgCtrlNtf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_EPDG_CTRL_NTF_STRU          *pstEpdgCtrlNtf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstEpdgCtrlNtf = (TAF_PS_EPDG_CTRL_NTF_STRU *)pstSndMsg->aucContent;

    if (pstEpdgCtrlNtf->stEpdgCtrl.bitOpActCnf == VOS_TRUE)
    {
        /* 将敏感信息设置为全0 */
        memset_s(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV4Addr,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV4Addr),
                 0,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV4Addr));

        memset_s(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV6Addr,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV6Addr),
                 0,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPdpAddr.aucIpV6Addr));

        memset_s(&(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPcscf),
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPcscf),
                 0,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stPcscf));

        memset_s(&(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stIPv6Pcscf),
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stIPv6Pcscf),
                 0,
                 sizeof(pstEpdgCtrlNtf->stEpdgCtrl.stActCnfInfo.stIPv6Pcscf));
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmIfaceUpReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg      = VOS_NULL_PTR;
    TAF_IFACE_UP_REQ_STRU              *pstIfaceUpReq  = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstIfaceUpReq = (TAF_IFACE_UP_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucIPv4Addr,
             sizeof(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucIPv4Addr),
             0,
             sizeof(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucIPv4Addr));

    memset_s(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucPassword,
             sizeof(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucPassword),
             0,
             TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

    memset_s(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucUsername,
             sizeof(pstIfaceUpReq->stIfaceUp.stUsrDialParam.aucUsername),
             0,
             TAF_MAX_AUTHDATA_USERNAME_LEN + 1);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                        *pstSndMsg               = VOS_NULL_PTR;
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstGetDynamicParaCnf    = VOS_NULL_PTR;
    VOS_UINT32                              ulLength;
    VOS_UINT32                              i;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstGetDynamicParaCnf = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstSndMsg->aucContent;

    for (i = 0; i <= TAF_MAX_CID_NV; i++)
    {
        /* 将敏感信息设置为全0 */
        /* IPV4 地址 */
        pstGetDynamicParaCnf->astDynamicInfo[i].stIpv4Info.stDhcpInfo.ulAddr = 0;

        /* IPV6 地址 */
        memset_s(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.aucAddr,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.aucAddr),
                 0,
                 TAF_IPV6_ADDR_LEN);

        /* IPV4 PCSCF 地址 */
        memset_s(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv4Info.stDhcpInfo.stPcscfList.aulPcscfAddrList,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv4Info.stDhcpInfo.stPcscfList.aulPcscfAddrList),
                 0,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv4Info.stDhcpInfo.stPcscfList.aulPcscfAddrList));

        /* IPV6 PCSCF 地址 */
        memset_s((pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.stPcscfList.astPcscfAddrList),
                 sizeof(TAF_IFACE_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                 0,
                 sizeof(TAF_IFACE_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

        /* IPV6 Tmp Addr */
        memset_s(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.aucTmpAddr,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.aucTmpAddr),
                 0,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stDhcpInfo.aucTmpAddr));

        /* IPV6 Lan Addr */
        memset_s(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucLanAddr,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucLanAddr),
                 0,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucLanAddr));

        /* IPV6 Prefix Addr */
        memset_s(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucPrefixAddr,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucPrefixAddr),
                 0,
                 sizeof(pstGetDynamicParaCnf->astDynamicInfo[i].stIpv6Info.stRaInfo.aucPrefixAddr));
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceRabInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                        *pstSndMsg               = VOS_NULL_PTR;
    TAF_IFACE_RAB_INFO_IND_STRU            *pstRabInfoInd           = VOS_NULL_PTR;
    VOS_UINT32                              ulLength;
    errno_t                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstSndMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstRabInfoInd = (TAF_IFACE_RAB_INFO_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    pstRabInfoInd->ulIpAddr = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchRnicIfaceCfgInd(
    MsgBlock                           *pstMsg
)
{
    DSM_RNIC_IFACE_CFG_IND_STRU        *pstIfaceCfgInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstIfaceCfgInd = (DSM_RNIC_IFACE_CFG_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                 DYNAMIC_MEM_PT,
                                                                 ulLength);

    if (pstIfaceCfgInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstIfaceCfgInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstIfaceCfgInd->ulIpv4Addr = 0;

    memset_s(pstIfaceCfgInd->aucIpv6Addr,
             sizeof(pstIfaceCfgInd->aucIpv6Addr),
             0,
             RNICITF_MAX_IPV6_ADDR_LEN);

    return (VOS_VOID *)pstIfaceCfgInd;
}


VOS_VOID* TAF_DSM_PrivacyMatchNdisIfaceUpInd(
    MsgBlock                           *pstMsg
)
{
    DSM_NDIS_IFACE_UP_IND_STRU         *pstIfaceUpInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstIfaceUpInd = (DSM_NDIS_IFACE_UP_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    if (pstIfaceUpInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstIfaceUpInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstIfaceUpInd->stIpv4PdnInfo),
             sizeof(pstIfaceUpInd->stIpv4PdnInfo),
             0,
             sizeof(DSM_NDIS_IPV4_PDN_INFO_STRU));

    memset_s(&(pstIfaceUpInd->stIpv6PdnInfo),
             sizeof(pstIfaceUpInd->stIpv6PdnInfo),
             0,
             sizeof(DSM_NDIS_IPV6_PDN_INFO_STRU));

    return (VOS_VOID *)pstIfaceUpInd;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstPdpActCnf  = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpActCnf = (TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pstPdpActCnf->stPdpAddr.aucIpv4Addr,
             sizeof(pstPdpActCnf->stPdpAddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pstPdpActCnf->stPdpAddr.aucIpv6Addr,
             sizeof(pstPdpActCnf->stPdpAddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    memset_s(pstPdpActCnf->stIpv4PcscfList.astIpv4PcscfAddrList,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pstPdpActCnf->stIpv6PcscfList.astIpv6PcscfAddrList,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* PCO */
    memset_s(pstPdpActCnf->stCustomPcoInfo.astContainerList,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
             0,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    /* EPDG 地址 */
    memset_s(pstPdpActCnf->stEpdgInfo.astIpv4EpdgList,
             sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM,
             0,
             sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM);

    memset_s(pstPdpActCnf->stEpdgInfo.astIpv6EpdgList,
             sizeof(TAF_IPV6_EPDG_STRU) * TAF_MAX_IPV6_EPDG_NUM,
             0,
             sizeof(TAF_IPV6_EPDG_STRU) * TAF_MAX_IPV6_EPDG_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_ACTIVATE_IND_STRU  *pstPdpActInd  = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpActInd = (TAF_PS_CALL_PDP_ACTIVATE_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pstPdpActInd->stPdpAddr.aucIpv4Addr,
             sizeof(pstPdpActInd->stPdpAddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pstPdpActInd->stPdpAddr.aucIpv6Addr,
             sizeof(pstPdpActInd->stPdpAddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    memset_s(pstPdpActInd->stIpv4PcscfList.astIpv4PcscfAddrList,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pstPdpActInd->stIpv6PcscfList.astIpv6PcscfAddrList,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* PCO */
    memset_s(pstPdpActInd->stCustomPcoInfo.astContainerList,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
             0,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    /* EPDG 地址 */
    memset_s(pstPdpActInd->stEpdgInfo.astIpv4EpdgList,
             sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM,
             0,
             sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM);

    memset_s(pstPdpActInd->stEpdgInfo.astIpv6EpdgList,
             sizeof(TAF_IPV6_EPDG_STRU) * TAF_MAX_IPV6_EPDG_NUM,
             0,
             sizeof(TAF_IPV6_EPDG_STRU) * TAF_MAX_IPV6_EPDG_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpManageInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg       = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_MANAGE_IND_STRU    *pstPdpManageInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpManageInd = (TAF_PS_CALL_PDP_MANAGE_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    memset_s(pstPdpManageInd->stPdpAddr.aucIpv4Addr,
             sizeof(pstPdpManageInd->stPdpAddr.aucIpv4Addr),
             0,
             TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    memset_s(pstPdpManageInd->stPdpAddr.aucIpv6Addr,
             sizeof(pstPdpManageInd->stPdpAddr.aucIpv6Addr),
             0,
             TAF_IPV6_ADDR_LEN);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg    = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstPdpModCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpModCnf = (TAF_PS_CALL_PDP_MODIFY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    memset_s(pstPdpModCnf->stIpv4PcscfList.astIpv4PcscfAddrList,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pstPdpModCnf->stIpv6PcscfList.astIpv6PcscfAddrList,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg    = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_MODIFY_IND_STRU    *pstPdpModInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpModInd = (TAF_PS_CALL_PDP_MODIFY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    memset_s(pstPdpModInd->stIpv4PcscfList.astIpv4PcscfAddrList,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    memset_s(pstPdpModInd->stIpv6PcscfList.astIpv6PcscfAddrList,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
             0,
             sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg         = VOS_NULL_PTR;
    TAF_PS_IPV6_INFO_IND_STRU          *pstPdpIpv6InfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpIpv6InfoInd = (TAF_PS_IPV6_INFO_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV6 地址 */
    memset_s(pstPdpIpv6InfoInd->stIpv6RaInfo.astPrefixList,
             sizeof(TAF_PDP_IPV6_PREFIX_STRU) * TAF_MAX_PREFIX_NUM_IN_RA,
             0,
             sizeof(TAF_PDP_IPV6_PREFIX_STRU) * TAF_MAX_PREFIX_NUM_IN_RA);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                                        *pstSndMsg        = VOS_NULL_PTR;
    TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF_STRU              *pstPdpCtxInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              i;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPdpCtxInfoCnf = (TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstPdpCtxInfoCnf->ulCidNum; i++)
    {
        /* IPV4 地址 */
        memset_s(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv4Addr,
                 sizeof(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv4Addr),
                 0,
                 TAF_IPV4_ADDR_LEN);
        /* IPV6 地址 */
        memset_s(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv6Addr,
                 sizeof(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv6Addr),
                 0,
                 TAF_IPV6_ADDR_LEN);
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsGetTftInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_GET_TFT_INFO_CNF_STRU       *pstGetTftInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstGetTftInfoCnf = (TAF_PS_GET_TFT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstGetTftInfoCnf->ulCidNum; i++)
    {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++)
        {
            /* IPV4 地址 */
            memset_s(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv4Addr,
                     sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv4Addr),
                     0,
                     TAF_IPV4_ADDR_LEN);

            memset_s(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv4Address,
                     sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv4Address),
                     0,
                     TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            memset_s(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv6Addr,
                     sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv6Addr),
                     0,
                     TAF_IPV6_ADDR_LEN);

            memset_s(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv6Address,
                     sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv6Address),
                     0,
                     TAF_IPV6_ADDR_LEN);
            pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].ucLocalIpv6Prefix = 0;
        }
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                                        *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_GET_PDP_IP_ADDR_INFO_CNF_STRU                   *pstIpAddrInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              i;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstIpAddrInfo = (TAF_PS_GET_PDP_IP_ADDR_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i <= TAF_MAX_CID; i++)
    {
        /* IPV4 地址 */
        memset_s(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv4Addr,
                 sizeof(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv4Addr),
                 0,
                 TAF_IPV4_ADDR_LEN);

        /* IPV6 地址 */
        memset_s(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv6Addr,
                 sizeof(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv6Addr),
                 0,
                 TAF_IPV6_ADDR_LEN);
    }
    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                                        *pstSndMsg  = VOS_NULL_PTR;
    TAF_PS_GET_DYNAMIC_TFT_INFO_CNF_STRU                   *pstTftInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              i;
    VOS_UINT32                                              j;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstTftInfo = (TAF_PS_GET_DYNAMIC_TFT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstTftInfo->ulCidNum; i++)
    {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++)
        {
            /* IPV4 地址 */
            memset_s(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv4Addr,
                     sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv4Addr),
                     0,
                     TAF_IPV4_ADDR_LEN);

            memset_s(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv4Addr,
                     sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv4Addr),
                     0,
                     TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            memset_s(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv6Addr,
                     sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv6Addr),
                     0,
                     TAF_IPV6_ADDR_LEN);

            memset_s(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv6Addr,
                     sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv6Addr),
                     0,
                     TAF_IPV6_ADDR_LEN);
            pstTftInfo->astPfTftInfo[i].astTftInfo[j].ucLocalIpv6Prefix = 0;
        }
    }
    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg       = VOS_NULL_PTR;
    TAF_PS_GET_AUTHDATA_INFO_CNF_STRU  *pstAuthdataInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstAuthdataInfo = (TAF_PS_GET_AUTHDATA_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstAuthdataInfo->ulCidNum; i++)
    {
         memset_s(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucPassword,
                  sizeof(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucPassword),
                  0,
                  TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

         memset_s(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucUsername,
                  sizeof(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucUsername),
                  0,
                  TAF_MAX_AUTHDATA_USERNAME_LEN + 1);
    }
    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsReportPcoInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_REPORT_PCO_INFO_IND_STRU    *pstPcoInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPcoInfoInd = (TAF_PS_REPORT_PCO_INFO_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstPcoInfoInd->stCustomPcoInfo.astContainerList,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
             0,
             sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    return (VOS_VOID *)pstSndMsg;
}



VOS_VOID* TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf(
    MsgBlock                           *pstMsg
)
{
    DRV_AGENT_MSG_STRU                 *pstSndMsg     = VOS_NULL_PTR;
    DRV_AGENT_MSID_QRY_CNF_STRU        *pstMsidQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (DRV_AGENT_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                   DYNAMIC_MEM_PT,
                                                   ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg, ulLength, pstMsg, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMsidQryCnf = (DRV_AGENT_MSID_QRY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstMsidQryCnf->aucImei,
             TAF_PH_IMEI_LEN,
             0,
             TAF_PH_IMEI_LEN);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* AT_PrivacyMatchSmsAppMsgReq(
    MsgBlock                           *pstMsg
)
{
    VOS_UINT8                          *pucAppReq       = VOS_NULL_PTR;
    MN_APP_REQ_MSG_STRU                *pstInputAppReq  = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulCopyLength;
    errno_t                             lMemResult;

    pstInputAppReq  = (MN_APP_REQ_MSG_STRU *)pstMsg;

    /* 计算消息长度和待输出脱敏后消息有效字段长度 */
    ulLength        = VOS_MSG_HEAD_LENGTH + pstInputAppReq->ulLength;
    ulCopyLength    = (sizeof(MN_APP_REQ_MSG_STRU) - sizeof(pstInputAppReq->aucContent));

    /* 消息长度小于消息结构的头部长度，认为消息异常输出原始消息，不脱敏 */
    if (ulLength < ulCopyLength)
    {
        return pstInputAppReq;
    }

    /* 申请消息长度大小的内存，用于脱敏后消息输出 */
    pucAppReq = (VOS_UINT8 *)VOS_MemAlloc(pstInputAppReq->ulSenderPid,
                                          DYNAMIC_MEM_PT,
                                          ulLength);

    if (VOS_NULL_PTR == pucAppReq)
    {
        return VOS_NULL_PTR;
    }

    /* 仅拷贝消息头部到脱敏后消息指针 */
    lMemResult = memset_s(pucAppReq, ulLength, 0, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    lMemResult = memcpy_s(pucAppReq, ulLength, pstInputAppReq, ulCopyLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulCopyLength);

    return (VOS_VOID *)pucAppReq;

}



VOS_UINT32 TAF_MSG_IsNeedPrivacyEventToApp(
    VOS_UINT32                          ulEvent
)
{
    if ((MN_MSG_EVT_READ            == ulEvent)
     || (MN_MSG_EVT_LIST            == ulEvent)
     || (MN_MSG_EVT_DELIVER         == ulEvent)
     || (MN_MSG_EVT_WRITE_SRV_PARM  == ulEvent)
     || (MN_MSG_EVT_READ_SRV_PARM   == ulEvent)
     || (MN_MSG_EVT_SUBMIT_RPT      == ulEvent))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_VOID* TAF_PrivacyMatchAtCallBackSmsProc(
    MsgBlock                           *pstMsg
)
{
    MN_MSG_EVENT_INFO_STRU             *pstEvent        = VOS_NULL_PTR;
    MN_AT_IND_EVT_STRU                 *pstSrcMsg       = VOS_NULL_PTR;
    MN_AT_IND_EVT_STRU                 *pstSendAtMsg    = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    MN_MSG_EVENT_ENUM_U32               enEvent;
    TAF_UINT32                          ulEventLen;
    errno_t                             lMemResult;

    pstSrcMsg = (MN_AT_IND_EVT_STRU *)pstMsg;

    ulEventLen = sizeof(MN_MSG_EVENT_ENUM_U32);
    lMemResult = memcpy_s(&enEvent,  sizeof(enEvent), pstSrcMsg->aucContent, ulEventLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(enEvent), ulEventLen);

    /* 不要求脱敏的事件直接返回源消息地址 */
    if (VOS_FALSE == TAF_MSG_IsNeedPrivacyEventToApp(enEvent))
    {
        return pstMsg;
    }

    ulLength     = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstSendAtMsg = (MN_AT_IND_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                      DYNAMIC_MEM_PT,
                                                      ulLength);

    if (VOS_NULL_PTR == pstSendAtMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSendAtMsg, ulLength, pstMsg, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstEvent = (MN_MSG_EVENT_INFO_STRU *)&pstSendAtMsg->aucContent[ulEventLen];

    memset_s(&pstEvent->u, sizeof(pstEvent->u), 0, sizeof(pstEvent->u));

    return (VOS_VOID *)pstSendAtMsg;
}



VOS_VOID* TAF_MTA_PrivacyMatchAtEcidSetCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_ECID_SET_CNF_STRU           *pstMtaAtEcidSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulContentLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMtaAtEcidSetCnf = (MTA_AT_ECID_SET_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0,清空字符串,不清楚log打印机制，这里把字符串全部清0 */
    if (pstMsg->ulLength > (sizeof(AT_APPCTRL_STRU) + sizeof(pstSndMsg->ulMsgId)))
    {
        ulContentLength = pstMsg->ulLength - (sizeof(AT_APPCTRL_STRU) + sizeof(pstSndMsg->ulMsgId) + sizeof(MTA_AT_RESULT_ENUM_UINT32));
        lMemResult = memset_s(pstMtaAtEcidSetCnf->aucCellInfoStr,
                              ulContentLength,
                              0x00,
                              ulContentLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulContentLength, ulContentLength);

    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoSetReq(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_EFLOCIINFO_SET_REQ_STRU    *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_EFLOCIINFO_SET_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                DYNAMIC_MEM_PT,
                                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stEflociInfo.ulTmsi = 0;
    pstSndMsg->stEflociInfo.usLac  = 0;
    pstSndMsg->stEflociInfo.ucRfu  = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_EFLOCIINFO_QRY_CNF_STRU    *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_EFLOCIINFO_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                DYNAMIC_MEM_PT,
                                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stEflociInfo.ulTmsi = 0;
    pstSndMsg->stEflociInfo.usLac  = 0;
    pstSndMsg->stEflociInfo.ucRfu  = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_EFPSLOCIINFO_SET_REQ_STRU  *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_EFPSLOCIINFO_SET_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stPsEflociInfo.ulPTmsi = 0;
    pstSndMsg->stPsEflociInfo.usLac   = 0;
    pstSndMsg->stPsEflociInfo.ucRac   = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_EFPSLOCIINFO_QRY_CNF_STRU  *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_MMA_EFPSLOCIINFO_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stPsEflociInfo.ulPTmsi = 0;
    pstSndMsg->stPsEflociInfo.usLac   = 0;
    pstSndMsg->stPsEflociInfo.ucRac   = 0;
    pstSndMsg->stPsEflociInfo.ulPTmsiSignature = 0;
    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafSetAuthDataReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg         = VOS_NULL_PTR;
    TAF_PS_SET_AUTHDATA_INFO_REQ_STRU  *pstSetAuthDataReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstSetAuthDataReq = (TAF_PS_SET_AUTHDATA_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstSetAuthDataReq->stAuthDataInfo.aucPassWord,
             sizeof(pstSetAuthDataReq->stAuthDataInfo.aucPassWord),
             0x00,
             sizeof(pstSetAuthDataReq->stAuthDataInfo.aucPassWord));

    memset_s(pstSetAuthDataReq->stAuthDataInfo.aucUserName,
             sizeof(pstSetAuthDataReq->stAuthDataInfo.aucUserName),
             0x00,
             sizeof(pstSetAuthDataReq->stAuthDataInfo.aucUserName));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg              = VOS_NULL_PTR;
    TAF_PS_SET_PDP_DNS_INFO_REQ_STRU   *pstSetSetPdpDnsInfoReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstSetSetPdpDnsInfoReq = (TAF_PS_SET_PDP_DNS_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr,
             sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr),
             0x00,
             sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr));

    memset_s(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucSecDnsAddr,
             sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucSecDnsAddr),
             0x00,
             sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucSecDnsAddr));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg              = VOS_NULL_PTR;
    TAF_PS_GET_PDP_DNS_INFO_CNF_STRU   *pstSetGetPdpDnsInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulIndex;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstSetGetPdpDnsInfoCnf = (TAF_PS_GET_PDP_DNS_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */

    for (ulIndex = 0; ulIndex < TAF_MIN(pstSetGetPdpDnsInfoCnf->ulCidNum, TAF_MAX_CID); ulIndex++)
    {
        memset_s(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr,
                 sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr),
                 0x00,
                 sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr));

        memset_s(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucSecDnsAddr,
                 sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucSecDnsAddr),
                 0x00,
                 sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucSecDnsAddr));
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                       *pstSndMsg               = VOS_NULL_PTR;
    TAF_PS_GET_NEGOTIATION_DNS_CNF_STRU   *pstGetNegotiationDnsCnf = VOS_NULL_PTR;
    VOS_UINT32                             ulLength;
    errno_t                                lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (TAF_PS_EVT_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);
    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstGetNegotiationDnsCnf = (TAF_PS_GET_NEGOTIATION_DNS_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    memset_s(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr,
             sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr),
             0x00,
             sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr));

    memset_s(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucSecDnsAddr,
             sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucSecDnsAddr),
             0x00,
             sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucSecDnsAddr));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonScellCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstMtaAtSetNetMonScellCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMtaAtSetNetMonScellCnf = (MTA_AT_NETMON_CELL_INFO_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstMtaAtSetNetMonScellCnf->u.stNCellInfo),
             sizeof(pstMtaAtSetNetMonScellCnf->u.stNCellInfo),
             0x00,
             sizeof(pstMtaAtSetNetMonScellCnf->u.stNCellInfo));

    memset_s(&(pstMtaAtSetNetMonScellCnf->u.unSCellInfo),
             sizeof(pstMtaAtSetNetMonScellCnf->u.unSCellInfo),
             0x00,
             sizeof(pstMtaAtSetNetMonScellCnf->u.unSCellInfo));

    return (VOS_VOID *)pstSndMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf(
    MsgBlock                           *msg
)
{
    AT_MTA_MSG_STRU                    *sndMsg     = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *mtaAtCellInfo = VOS_NULL_PTR;
    VOS_UINT32                          length;
    errno_t                             memResult;

    /* 计算消息长度 */
    length = msg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    sndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(msg->ulSenderPid,
                                             DYNAMIC_MEM_PT,
                                             length);

    /* 如果没有申请到内存，则返回空指针 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(sndMsg,
                         length,
                         msg,
                         length);

    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    mtaAtCellInfo = (MTA_AT_NETMON_CELL_INFO_STRU *)(sndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(&(mtaAtCellInfo->u.secSrvCellInfo),
             sizeof(mtaAtCellInfo->u.secSrvCellInfo),
             0x00,
             sizeof(mtaAtCellInfo->u.secSrvCellInfo));

    return (VOS_VOID *)sndMsg;
}
#endif


VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstMtaAtSetNetMonScellCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (AT_MTA_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                DYNAMIC_MEM_PT,
                                                ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstMtaAtSetNetMonScellCnf = (MTA_AT_NETMON_CELL_INFO_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstMtaAtSetNetMonScellCnf->u.stNCellInfo),
             sizeof(pstMtaAtSetNetMonScellCnf->u.stNCellInfo),
             0x00,
             sizeof(pstMtaAtSetNetMonScellCnf->u.stNCellInfo));

    memset_s(&(pstMtaAtSetNetMonScellCnf->u.unSCellInfo),
             sizeof(pstMtaAtSetNetMonScellCnf->u.unSCellInfo),
             0x00,
             sizeof(pstMtaAtSetNetMonScellCnf->u.unSCellInfo));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* AT_PrivacyMatchPseucellInfoSetReq(
    MsgBlock                           *pstMsg
)
{
    MN_APP_REQ_MSG_STRU                *pstSndMsg             = VOS_NULL_PTR;
    AT_MTA_PSEUCELL_INFO_SET_REQ_STRU  *pstPseucellInfoSetReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;


    /* 申请内存，后续统一由底层释放 */
    pstSndMsg = (MN_APP_REQ_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                    DYNAMIC_MEM_PT,
                                                    ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstSndMsg,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    pstPseucellInfoSetReq = (AT_MTA_PSEUCELL_INFO_SET_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    pstPseucellInfoSetReq->ulLac = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_CCM_PrivacyMatchCallOrigReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_CALL_ORIG_REQ_STRU                             *pstCallOrigReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCallOrigReq = (TAF_CCM_CALL_ORIG_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                DYNAMIC_MEM_PT,
                                                                ulLength);

    if (VOS_NULL_PTR == pstCallOrigReq)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallOrigReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    /* 清0敏感信息 */
    memset_s(&pstCallOrigReq->stOrig.stDialNumber,
             sizeof(pstCallOrigReq->stOrig.stDialNumber),
             0,
             sizeof(MN_CALL_CALLED_NUM_STRU));

    memset_s(&pstCallOrigReq->stOrig.stSubaddr,
             sizeof(pstCallOrigReq->stOrig.stSubaddr),
             0,
             sizeof(MN_CALL_SUBADDR_STRU));
    memset_s(&pstCallOrigReq->stOrig.stBc,
             sizeof(pstCallOrigReq->stOrig.stBc),
             0,
             sizeof(MN_CALL_BC_STRU));
    memset_s(&pstCallOrigReq->stOrig.stEmergencyCat,
             sizeof(pstCallOrigReq->stOrig.stEmergencyCat),
             0,
             sizeof(MN_CALL_EMERGENCY_CAT_STRU));

    pstCallOrigReq->stOrig.ucIs1xStkEmc = 0;

    return (VOS_VOID *)pstCallOrigReq;

}


VOS_VOID* TAF_CCM_PrivacyMatchCallSupsCmdReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_CALL_SUPS_CMD_REQ_STRU                         *pstCallSupsCmdReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCallSupsCmdReq = (TAF_CCM_CALL_SUPS_CMD_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    if (VOS_NULL_PTR == pstCallSupsCmdReq)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallSupsCmdReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCallSupsCmdReq->stCallMgmtCmd.stRedirectNum,
             sizeof(pstCallSupsCmdReq->stCallMgmtCmd.stRedirectNum),
             0,
             sizeof(MN_CALL_BCD_NUM_STRU));

    memset_s(&pstCallSupsCmdReq->stCallMgmtCmd.stRemoveNum,
             sizeof(pstCallSupsCmdReq->stCallMgmtCmd.stRemoveNum),
             0,
             sizeof(MN_CALL_CALLED_NUM_STRU));

    return (VOS_VOID *)pstCallSupsCmdReq;

}



VOS_VOID* TAF_CCM_PrivacyMatchCustomDialReqCmdReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_CUSTOM_DIAL_REQ_STRU                           *pstCustomDialReqCmdReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCustomDialReqCmdReq = (TAF_CCM_CUSTOM_DIAL_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                          DYNAMIC_MEM_PT,
                                                                          ulLength);

    if (pstCustomDialReqCmdReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCustomDialReqCmdReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCustomDialReqCmdReq->stCustomDialPara.stDialNumber,
             sizeof(pstCustomDialReqCmdReq->stCustomDialPara.stDialNumber),
             0,
             sizeof(MN_CALL_CALLED_NUM_STRU));

    return (VOS_VOID *)pstCustomDialReqCmdReq;

}


VOS_VOID* TAF_CCM_PrivacyMatchCcmStartDtmfReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_START_DTMF_REQ_STRU                            *pstCcmStartDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmStartDtmfReq = (TAF_CCM_START_DTMF_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                     DYNAMIC_MEM_PT,
                                                                     ulLength);

    if (pstCcmStartDtmfReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmStartDtmfReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCcmStartDtmfReq->stDtmf.cKey,
             sizeof(pstCcmStartDtmfReq->stDtmf.cKey),
             0,
             sizeof(VOS_CHAR));

    return (VOS_VOID *)pstCcmStartDtmfReq;

}


VOS_VOID* TAF_CCM_PrivacyMatchCcmStopDtmfReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_STOP_DTMF_REQ_STRU                             *pstCcmStopDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmStopDtmfReq = (TAF_CCM_STOP_DTMF_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                     DYNAMIC_MEM_PT,
                                                                     ulLength);

    if (pstCcmStopDtmfReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmStopDtmfReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCcmStopDtmfReq->stDtmf.cKey,
             sizeof(pstCcmStopDtmfReq->stDtmf.cKey),
             0,
             sizeof(VOS_CHAR));

    return (VOS_VOID *)pstCcmStopDtmfReq;
}


VOS_VOID* TAF_CCM_PrivacyMatchCcmSetUusInfoReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_SET_UUSINFO_REQ_STRU                           *pstCcmSetUusinfoReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmSetUusinfoReq = (TAF_CCM_SET_UUSINFO_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                     DYNAMIC_MEM_PT,
                                                                     ulLength);

    if (pstCcmSetUusinfoReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmSetUusinfoReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstCcmSetUusinfoReq->stUus1Info.stUus1Info,
             sizeof(pstCcmSetUusinfoReq->stUus1Info.stUus1Info),
             0,
             sizeof(MN_CALL_UUS1_INFO_STRU) * MN_CALL_MAX_UUS1_MSG_NUM);

    return (VOS_VOID *)pstCcmSetUusinfoReq;
}



VOS_VOID* TAF_CCM_PrivacyMatchCcmCustomEccNumReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_CUSTOM_ECC_NUM_REQ_STRU                        *pstCcmCustomEccNumReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmCustomEccNumReq = (TAF_CCM_CUSTOM_ECC_NUM_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmCustomEccNumReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmCustomEccNumReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCcmCustomEccNumReq->stEccNumReq.stEccNum,
             sizeof(pstCcmCustomEccNumReq->stEccNumReq.stEccNum),
             0,
             sizeof(MN_CALL_BCD_NUM_STRU));

    return (VOS_VOID *)pstCcmCustomEccNumReq;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_CCM_PrivacyMatchCcmSendFlashReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_SEND_FLASH_REQ_STRU        *pstCcmSendFlashReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmSendFlashReq = (TAF_CCM_SEND_FLASH_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmSendFlashReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmSendFlashReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstCcmSendFlashReq->stFlashPara.aucDigit,
             sizeof(pstCcmSendFlashReq->stFlashPara.aucDigit),
             0,
             sizeof(pstCcmSendFlashReq->stFlashPara.aucDigit));

    pstCcmSendFlashReq->stFlashPara.ucDigitNum = 0;

    return (VOS_VOID *)pstCcmSendFlashReq;
}



VOS_VOID* TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_SEND_BURST_DTMF_REQ_STRU                       *pstCcmSendBrustDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmSendBrustDtmfReq = (TAF_CCM_SEND_BURST_DTMF_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmSendBrustDtmfReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmSendBrustDtmfReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstCcmSendBrustDtmfReq->stBurstDTMFPara.aucDigit,
             sizeof(pstCcmSendBrustDtmfReq->stBurstDTMFPara.aucDigit),
             0,
             sizeof(pstCcmSendBrustDtmfReq->stBurstDTMFPara.aucDigit));

    pstCcmSendBrustDtmfReq->stBurstDTMFPara.ucDigitNum  = 0;
    pstCcmSendBrustDtmfReq->stBurstDTMFPara.ulOnLength  = 0;
    pstCcmSendBrustDtmfReq->stBurstDTMFPara.ulOffLength = 0;

    return (VOS_VOID *)pstCcmSendBrustDtmfReq;
}



VOS_VOID* TAF_CCM_PrivacyMatchCcmSendContDtmfReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_SEND_CONT_DTMF_REQ_STRU                        *pstCcmSendContDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmSendContDtmfReq = (TAF_CCM_SEND_CONT_DTMF_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmSendContDtmfReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmSendContDtmfReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstCcmSendContDtmfReq->stContDTMFPara.ucDigit  = 0;

    return (VOS_VOID *)pstCcmSendContDtmfReq;
}



VOS_VOID* TAF_CCM_PrivacyMatchCcmEncryptVoiceReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_ENCRYPT_VOICE_REQ_STRU                         *pstCcmEncryptVoiceReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmEncryptVoiceReq = (TAF_CCM_ENCRYPT_VOICE_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmEncryptVoiceReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmEncryptVoiceReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstCcmEncryptVoiceReq->stEncrypVoicePara.stDialNumber),
             sizeof(TAF_ECC_CALL_BCD_NUM_STRU),
             0,
             sizeof(TAF_ECC_CALL_BCD_NUM_STRU));

    return (VOS_VOID *)pstCcmEncryptVoiceReq;
}

#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)

VOS_VOID* TAF_CCM_PrivacyMatchCcmSetEcKmcReq(
    MsgBlock                                               *pstMsg
)
{
    TAF_CCM_SET_EC_KMC_REQ_STRU                            *pstCcmSetEcKmcReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstCcmSetEcKmcReq = (TAF_CCM_SET_EC_KMC_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulLength);

    if (pstCcmSetEcKmcReq == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcmSetEcKmcReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstCcmSetEcKmcReq->stKmcData),
             sizeof(MN_CALL_APP_EC_KMC_DATA_STRU),
             0,
             sizeof(MN_CALL_APP_EC_KMC_DATA_STRU));

    return (VOS_VOID *)pstCcmSetEcKmcReq;
}
#endif
#endif


VOS_VOID* TAF_PrivacyMatchAppQryEconfCalledInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_ECONF_CALLED_INFO_CNF_STRU                 *pstQryEconfCalledInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              ulLoop = 0;
    errno_t                                                 lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryEconfCalledInfoCnf = (TAF_CCM_QRY_ECONF_CALLED_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                      DYNAMIC_MEM_PT,
                                                                                      ulLength);


    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstQryEconfCalledInfoCnf)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryEconfCalledInfoCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstQryEconfCalledInfoCnf->ucNumOfCalls = TAF_MIN(pstQryEconfCalledInfoCnf->ucNumOfCalls,TAF_CALL_MAX_ECONF_CALLED_NUM);

    for(ulLoop = 0; ulLoop < pstQryEconfCalledInfoCnf->ucNumOfCalls; ulLoop++)
    {
        memset_s(&pstQryEconfCalledInfoCnf->astCallInfo[ulLoop].stCallNumber,
                 sizeof(pstQryEconfCalledInfoCnf->astCallInfo[ulLoop].stCallNumber),
                 0,
                 sizeof(pstQryEconfCalledInfoCnf->astCallInfo[ulLoop].stCallNumber));
    }


    return (VOS_VOID *)pstQryEconfCalledInfoCnf;
}



VOS_VOID* TAF_PrivacyMatchAtCallIncomingInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CALL_INCOMING_IND_STRU     *pstCallIncomingInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCallIncomingInd = (TAF_CCM_CALL_INCOMING_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCallIncomingInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallIncomingInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCallIncomingInd->stIncomingIndPara.stCallNumber,
             sizeof(pstCallIncomingInd->stIncomingIndPara.stCallNumber),
             0,
             sizeof(MN_CALL_BCD_NUM_STRU));

    memset_s(&pstCallIncomingInd->stIncomingIndPara.stSubCallNumber,
             sizeof(pstCallIncomingInd->stIncomingIndPara.stSubCallNumber),
             0,
             sizeof(MN_CALL_SUBADDR_STRU));

    memset_s(&pstCallIncomingInd->stIncomingIndPara.stNameIndicator,
             sizeof(pstCallIncomingInd->stIncomingIndPara.stNameIndicator),
             0,
             sizeof(TAF_CALL_CNAP_STRU));

    return (VOS_VOID *)pstCallIncomingInd;
}



VOS_VOID* TAF_PrivacyMatchAtCallConnectInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CALL_CONNECT_IND_STRU      *pstCallConnectInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCallConnectInd = (TAF_CCM_CALL_CONNECT_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCallConnectInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallConnectInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCallConnectInd->stConnectIndPara.stConnectNumber,
             sizeof(pstCallConnectInd->stConnectIndPara.stConnectNumber),
             0,
             sizeof(MN_CALL_BCD_NUM_STRU));

    return (VOS_VOID *)pstCallConnectInd;
}



VOS_VOID* TAF_PrivacyMatchAtQryCallInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_CALL_INFO_CNF_STRU     *pstQryCallInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryCallInfoCnf = (TAF_CCM_QRY_CALL_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstQryCallInfoCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryCallInfoCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstQryCallInfoCnf->stQryCallInfoPara.ucNumOfCalls = TAF_MIN(pstQryCallInfoCnf->stQryCallInfoPara.ucNumOfCalls,MN_CALL_MAX_NUM);

    for(ulLoop = 0; ulLoop < pstQryCallInfoCnf->stQryCallInfoPara.ucNumOfCalls; ulLoop++)
    {
        memset_s(&pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCallNumber,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCallNumber),
                 0,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCallNumber));

        memset_s(&pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCalledNumber,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCalledNumber),
                 0,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stCalledNumber));

        memset_s(&pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stConnectNumber,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stConnectNumber),
                 0,
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stConnectNumber));

        memset_s(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stDisplayName),
                 sizeof(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ulLoop].stDisplayName),
                 0,
                 sizeof(MN_CALL_DISPLAY_NAME_STRU));

    }

    pstQryCallInfoCnf->stQryCallInfoPara.ucNumOfCalls = 0;


    return (VOS_VOID *)pstQryCallInfoCnf;
}



VOS_VOID* TAF_PrivacyMatchAtCallSsInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCallSsInd = (TAF_CCM_CALL_SS_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCallSsInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallSsInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstCallSsInd->stSsIndPara.stCallNumber,
             sizeof(pstCallSsInd->stSsIndPara.stCallNumber),
             0,
             sizeof(pstCallSsInd->stSsIndPara.stCallNumber));

    memset_s(&pstCallSsInd->stSsIndPara.stCcbsFeature,
             sizeof(pstCallSsInd->stSsIndPara.stCcbsFeature),
             0,
             sizeof(TAF_SS_CCBS_FEATURE_STRU));

    memset_s(&pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator,
             sizeof(pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator),
             0,
             sizeof(MN_CALL_ECT_IND_STRU));

    return (VOS_VOID *)pstCallSsInd;
}



VOS_VOID* TAF_PrivacyMatchAtEccNumInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_ECC_NUM_IND_STRU           *pstEccNumInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstEccNumInd = (TAF_CCM_ECC_NUM_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstEccNumInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstEccNumInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstEccNumInd->stEccNumInd.ulEccNumCount = TAF_MIN(pstEccNumInd->stEccNumInd.ulEccNumCount, MN_CALL_MAX_EMC_NUM);

    for(ulLoop = 0; ulLoop < pstEccNumInd->stEccNumInd.ulEccNumCount; ulLoop++)
    {
        memset_s(pstEccNumInd->stEccNumInd.astCustomEccNumList[ulLoop].aucEccNum,
                 sizeof(pstEccNumInd->stEccNumInd.astCustomEccNumList[ulLoop].aucEccNum),
                 0,
                 sizeof(pstEccNumInd->stEccNumInd.astCustomEccNumList[ulLoop].aucEccNum));
    }

    return (VOS_VOID *)pstEccNumInd;
}



VOS_VOID* TAF_PrivacyMatchAtQryXlemaInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_XLEMA_CNF_STRU         *pstQryXlemaCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryXlemaCnf = (TAF_CCM_QRY_XLEMA_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstQryXlemaCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryXlemaCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstQryXlemaCnf->stQryXlemaPara.ulEccNumCount = TAF_MIN(pstQryXlemaCnf->stQryXlemaPara.ulEccNumCount, MN_CALL_MAX_EMC_NUM);

    for(ulLoop = 0; ulLoop < pstQryXlemaCnf->stQryXlemaPara.ulEccNumCount; ulLoop++)
    {
        memset_s(pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[ulLoop].aucEccNum,
                 sizeof(pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[ulLoop].aucEccNum),
                 0,
                 sizeof(pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[ulLoop].aucEccNum));
    }

    return (VOS_VOID *)pstQryXlemaCnf;
}



VOS_VOID* TAF_PrivacyMatchAtCnapQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CNAP_QRY_CNF_STRU          *pstCnapQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCnapQryCnf = (TAF_CCM_CNAP_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCnapQryCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCnapQryCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstCnapQryCnf->stNameIndicator),
               sizeof(pstCnapQryCnf->stNameIndicator),
               0,
               sizeof(TAF_CALL_CNAP_STRU));


    return (VOS_VOID *)pstCnapQryCnf;
}


VOS_VOID* TAF_PrivacyMatchAtCnapInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CNAP_INFO_IND_STRU         *pstCnapQryInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCnapQryInd = (TAF_CCM_CNAP_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCnapQryInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCnapQryInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstCnapQryInd->stNameIndicator),
             sizeof(pstCnapQryInd->stNameIndicator),
             0,
             sizeof(TAF_CALL_CNAP_STRU));


    return (VOS_VOID *)pstCnapQryInd;
}



VOS_VOID* TAF_PrivacyMatchAtQryUus1InfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_UUS1_INFO_CNF_STRU     *pstQryUss1InfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryUss1InfoCnf = (TAF_CCM_QRY_UUS1_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstQryUss1InfoCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryUss1InfoCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstQryUss1InfoCnf->stQryUss1Info.stUus1Info,
             sizeof(pstQryUss1InfoCnf->stQryUss1Info.stUus1Info),
             0,
             sizeof(MN_CALL_UUS1_INFO_STRU) * MN_CALL_MAX_UUS1_MSG_NUM);


    return (VOS_VOID *)pstQryUss1InfoCnf;
}


VOS_VOID* TAF_PrivacyMatchAtUus1InfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_UUS1_INFO_IND_STRU         *pstUss1InfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstUss1InfoInd = (TAF_CCM_UUS1_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstUss1InfoInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstUss1InfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstUss1InfoInd->stUus1InfoIndPara.stUusInfo),
             sizeof(pstUss1InfoInd->stUus1InfoIndPara.stUusInfo),
             0,
             sizeof(MN_CALL_UUS1_INFO_STRU));


    return (VOS_VOID *)pstUss1InfoInd;
}

#if (FEATURE_ON == FEATURE_ECALL)

VOS_VOID* TAF_PrivacyMatchAtQryEcallInfoCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_ECALL_INFO_CNF_STRU    *pstQryEcallInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryEcallInfoCnf = (TAF_CCM_QRY_ECALL_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstQryEcallInfoCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryEcallInfoCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    for (ulLoop = 0; ulLoop < MN_CALL_MAX_NUM; ulLoop++)
    {
        memset_s(&(pstQryEcallInfoCnf->stEcallInfo.astEcallInfos[ulLoop].stDialNumber),
                 sizeof(pstQryEcallInfoCnf->stEcallInfo.astEcallInfos[ulLoop].stDialNumber),
                 0,
                 sizeof(MN_CALL_CALLED_NUM_STRU));
    }

    return (VOS_VOID *)pstQryEcallInfoCnf;
}
#endif


VOS_VOID* TAF_PrivacyMatchAtQryClprCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_QRY_CLPR_CNF_STRU          *pstQryClprCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstQryClprCnf = (TAF_CCM_QRY_CLPR_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstQryClprCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstQryClprCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstQryClprCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectNum),
             sizeof(pstQryClprCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectNum),
             0,
             sizeof(MN_CALL_BCD_NUM_STRU));

    memset_s(&(pstQryClprCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr),
             sizeof(pstQryClprCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr),
             0,
             sizeof(MN_CALL_SUBADDR_STRU));

    return (VOS_VOID *)pstQryClprCnf;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_PrivacyMatchAtCalledNumInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CALLED_NUM_INFO_IND_STRU   *pstCalledNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCalledNumInfoInd = (TAF_CCM_CALLED_NUM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCalledNumInfoInd)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCalledNumInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstCalledNumInfoInd->stCalledNumInfoPara.aucDigit,
             sizeof(pstCalledNumInfoInd->stCalledNumInfoPara.aucDigit),
             0,
             sizeof(pstCalledNumInfoInd->stCalledNumInfoPara.aucDigit));

    pstCalledNumInfoInd->stCalledNumInfoPara.enNumType         = 0;
    pstCalledNumInfoInd->stCalledNumInfoPara.enNumPlan         = 0;
    pstCalledNumInfoInd->stCalledNumInfoPara.ucDigitNum        = 0;

    return (VOS_VOID *)pstCalledNumInfoInd;
}


VOS_VOID* TAF_PrivacyMatchAtCallingNumInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CALLING_NUM_INFO_IND_STRU  *pstCallingNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCallingNumInfoInd = (TAF_CCM_CALLING_NUM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCallingNumInfoInd)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCallingNumInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstCallingNumInfoInd->stCallIngNumInfoPara.enNumPlan         = 0;
    pstCallingNumInfoInd->stCallIngNumInfoPara.enNumType         = 0;
    pstCallingNumInfoInd->stCallIngNumInfoPara.ucDigitNum        = 0;
    pstCallingNumInfoInd->stCallIngNumInfoPara.ucPi              = 0;
    pstCallingNumInfoInd->stCallIngNumInfoPara.ucSi              = 0;

    memset_s(pstCallingNumInfoInd->stCallIngNumInfoPara.aucDigit,
             sizeof(pstCallingNumInfoInd->stCallIngNumInfoPara.aucDigit),
             0,
             sizeof(pstCallingNumInfoInd->stCallIngNumInfoPara.aucDigit));

    return (VOS_VOID *)pstCallingNumInfoInd;
}


VOS_VOID* TAF_PrivacyMatchAtDisplayInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_DISPLAY_INFO_IND_STRU      *pstDisplayInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstDisplayInfoInd = (TAF_CCM_DISPLAY_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstDisplayInfoInd)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstDisplayInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstDisplayInfoInd->stDisPlayInfoIndPara.ucDigitNum        = 0;

    memset_s(pstDisplayInfoInd->stDisPlayInfoIndPara.aucDigit,
             sizeof(pstDisplayInfoInd->stDisPlayInfoIndPara.aucDigit),
             0,
             sizeof(pstDisplayInfoInd->stDisPlayInfoIndPara.aucDigit));

    return (VOS_VOID *)pstDisplayInfoInd;
}



VOS_VOID* TAF_PrivacyMatchAtExtDisplayInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_EXT_DISPLAY_INFO_IND_STRU  *pstExtDisplayInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstExtDisplayInfoInd = (TAF_CCM_EXT_DISPLAY_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstExtDisplayInfoInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstExtDisplayInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstExtDisplayInfoInd->stDisPlayInfoIndPara.ucInfoRecsDataNum  = 0;
    pstExtDisplayInfoInd->stDisPlayInfoIndPara.ucDisplayType      = 0;
    pstExtDisplayInfoInd->stDisPlayInfoIndPara.ucExtDispInd       = 0;

    memset_s(pstExtDisplayInfoInd->stDisPlayInfoIndPara.aucInfoRecsData,
             sizeof(pstExtDisplayInfoInd->stDisPlayInfoIndPara.aucInfoRecsData),
             0,
             sizeof(pstExtDisplayInfoInd->stDisPlayInfoIndPara.aucInfoRecsData));

    return (VOS_VOID *)pstExtDisplayInfoInd;
}


VOS_VOID* TAF_PrivacyMatchAtConnNumInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CONN_NUM_INFO_IND_STRU     *pstConnNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstConnNumInfoInd = (TAF_CCM_CONN_NUM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstConnNumInfoInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstConnNumInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstConnNumInfoInd->stConnNumInfoIndPara.enNumType  = 0;
    pstConnNumInfoInd->stConnNumInfoIndPara.enNumPlan  = 0;
    pstConnNumInfoInd->stConnNumInfoIndPara.ucPi       = 0;
    pstConnNumInfoInd->stConnNumInfoIndPara.ucSi       = 0;
    pstConnNumInfoInd->stConnNumInfoIndPara.ucDigitNum = 0;


    memset_s(pstConnNumInfoInd->stConnNumInfoIndPara.aucDigit,
             sizeof(pstConnNumInfoInd->stConnNumInfoIndPara.aucDigit),
             0,
             sizeof(pstConnNumInfoInd->stConnNumInfoIndPara.aucDigit));

    return (VOS_VOID *)pstConnNumInfoInd;
}



VOS_VOID* TAF_PrivacyMatchAtRedirNumInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_REDIR_NUM_INFO_IND_STRU    *pstRedirNumInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstRedirNumInfoInd = (TAF_CCM_REDIR_NUM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstRedirNumInfoInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstRedirNumInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstRedirNumInfoInd->stRedirNumInfoIndPara.bitOpPi    = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.bitOpSi    = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.enNumType  = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.enNumPlan  = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.ucPi       = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.ucSi       = 0;
    pstRedirNumInfoInd->stRedirNumInfoIndPara.ucDigitNum = 0;


    memset_s(pstRedirNumInfoInd->stRedirNumInfoIndPara.aucDigitNum,
             sizeof(pstRedirNumInfoInd->stRedirNumInfoIndPara.aucDigitNum),
             0,
             sizeof(pstRedirNumInfoInd->stRedirNumInfoIndPara.aucDigitNum));

    return (VOS_VOID *)pstRedirNumInfoInd;
}



VOS_VOID* TAF_PrivacyMatchAtCcwacInfoInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CCWAC_INFO_IND_STRU        *pstCcwcInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCcwcInfoInd = (TAF_CCM_CCWAC_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstCcwcInfoInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstCcwcInfoInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstCcwcInfoInd->stCcwacInfoPara.enNumPlan         = 0;
    pstCcwcInfoInd->stCcwacInfoPara.enNumType         = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucAlertPitch      = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucDigitNum        = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucPi              = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucSi              = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucSignal          = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucSignalIsPresent = 0;
    pstCcwcInfoInd->stCcwacInfoPara.ucSignalType      = 0;

    memset_s(pstCcwcInfoInd->stCcwacInfoPara.aucDigit,
             sizeof(pstCcwcInfoInd->stCcwacInfoPara.aucDigit),
             0,
             sizeof(pstCcwcInfoInd->stCcwacInfoPara.aucDigit));

    return (VOS_VOID *)pstCcwcInfoInd;
}



VOS_VOID* TAF_PrivacyMatchAtContDtmfInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_CONT_DTMF_IND_STRU         *pstContDtmfInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstContDtmfInd = (TAF_CCM_CONT_DTMF_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstContDtmfInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstContDtmfInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstContDtmfInd->stContDtmfIndPara.enSwitch = 0;
    pstContDtmfInd->stContDtmfIndPara.ucDigit  = 0;

    return (VOS_VOID *)pstContDtmfInd;
}



VOS_VOID* TAF_PrivacyMatchAtBurstDtmfInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_BURST_DTMF_IND_STRU        *pstBrustDtmfInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstBrustDtmfInd = (TAF_CCM_BURST_DTMF_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstBrustDtmfInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstBrustDtmfInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstBrustDtmfInd->stBurstDtmfIndPara.ulOnLength  = 0;
    pstBrustDtmfInd->stBurstDtmfIndPara.ulOffLength = 0;
    pstBrustDtmfInd->stBurstDtmfIndPara.ucDigitNum  = 0;

    memset_s(pstBrustDtmfInd->stBurstDtmfIndPara.aucDigit,
             sizeof(pstBrustDtmfInd->stBurstDtmfIndPara.aucDigit),
             0,
             sizeof(pstBrustDtmfInd->stBurstDtmfIndPara.aucDigit));

    return (VOS_VOID *)pstBrustDtmfInd;
}


VOS_VOID* TAF_PrivacyMatchAtEncryptVoiceInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_ENCRYPT_VOICE_IND_STRU     *pstEncryptVoiceInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstEncryptVoiceInd = (TAF_CCM_ENCRYPT_VOICE_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstEncryptVoiceInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstEncryptVoiceInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstEncryptVoiceInd->stEncryptVoiceIndPara.stCallingNumber),
             sizeof(pstEncryptVoiceInd->stEncryptVoiceIndPara.stCallingNumber),
             0,
             sizeof(pstEncryptVoiceInd->stEncryptVoiceIndPara.stCallingNumber));

    return (VOS_VOID *)pstEncryptVoiceInd;
}


VOS_VOID* TAF_PrivacyMatchAtGetEcRandomCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_GET_EC_RANDOM_CNF_STRU     *pstGetEcRandomCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstGetEcRandomCnf = (TAF_CCM_GET_EC_RANDOM_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstGetEcRandomCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstGetEcRandomCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(pstGetEcRandomCnf->stEcRandomData.astEccRandom,
             sizeof(pstGetEcRandomCnf->stEcRandomData.astEccRandom),
             0,
             sizeof(pstGetEcRandomCnf->stEcRandomData.astEccRandom));

    return (VOS_VOID *)pstGetEcRandomCnf;
}



VOS_VOID* TAF_PrivacyMatchAtGetEcKmcCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_GET_EC_KMC_CNF_STRU        *pstGetEcKmcCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstGetEcKmcCnf = (TAF_CCM_GET_EC_KMC_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstGetEcKmcCnf == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstGetEcKmcCnf,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&(pstGetEcKmcCnf->stKmcCnfPara.stKmcData),
             sizeof(pstGetEcKmcCnf->stKmcCnfPara.stKmcData),
             0,
             sizeof(pstGetEcKmcCnf->stKmcCnfPara.stKmcData));


    return (VOS_VOID *)pstGetEcKmcCnf;
}

#endif

#if (FEATURE_ON == FEATURE_IMS)

VOS_VOID* TAF_CCM_PrivacyMatchCcmEconfDialReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_ECONF_DIAL_REQ_STRU        *pstEconfDialReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstEconfDialReq = (TAF_CCM_ECONF_DIAL_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);

    if (VOS_NULL_PTR == pstEconfDialReq)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstEconfDialReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    memset_s(&pstEconfDialReq->stEconfDialInfo.stEconfCalllist,
             sizeof(pstEconfDialReq->stEconfDialInfo.stEconfCalllist),
             0,
             sizeof(TAF_CALL_ECONF_CALL_LIST_STRU));

    return (VOS_VOID *)pstEconfDialReq;

}


VOS_VOID* TAF_PrivacyMatchAtEconfNotifyInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CCM_ECONF_NOTIFY_IND_STRU      *pstEconfNotifyInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop = 0;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstEconfNotifyInd = (TAF_CCM_ECONF_NOTIFY_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (pstEconfNotifyInd == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstEconfNotifyInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* 将敏感信息设置为全0 */
    pstEconfNotifyInd->stEconfNotifyIndPara.ucNumOfCalls = TAF_MIN(pstEconfNotifyInd->stEconfNotifyIndPara.ucNumOfCalls,TAF_CALL_MAX_ECONF_CALLED_NUM);

    for(ulLoop = 0; ulLoop < pstEconfNotifyInd->stEconfNotifyIndPara.ucNumOfCalls; ulLoop++)
    {
        memset_s(&pstEconfNotifyInd->stEconfNotifyIndPara.astCallInfo[ulLoop].stCallNumber,
                 sizeof(pstEconfNotifyInd->stEconfNotifyIndPara.astCallInfo[ulLoop].stCallNumber),
                 0,
                 sizeof(pstEconfNotifyInd->stEconfNotifyIndPara.astCallInfo[ulLoop].stCallNumber));
    }

    return (VOS_VOID *)pstEconfNotifyInd;
}

#endif


