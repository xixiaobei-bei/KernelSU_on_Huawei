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
#include "TafAppCall.h"
#include "TafAppXsmsInterface.h"
#include "TafSsaApi.h"
#include "AtMtaInterface.h"
#include "TafDrvAgent.h"
#include "AtXpdsInterface.h"
#include "MnMsgApi.h"
#include "TafCcmApi.h"
#include "RnicCdsInterface.h"
#include "TafPsApi.h"
#include "TafPsTypeDef.h"
#include "AtImsaInterface.h"


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

    pstAtIndEvt = (MN_AT_IND_EVT_STRU *)pstMsg;

    /* 获取当前的event类型，并判断该event是否需要脱敏 */
    enEvtId = MN_CALL_EVT_BUTT;
    TAF_MEM_CPY_S(&enEvtId,  sizeof(enEvtId), pstAtIndEvt->aucContent, sizeof(enEvtId));

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

    TAF_MEM_CPY_S(pstPrivacyAtIndEvt,
                  ulLength,
                  pstAtIndEvt,
                  ulLength);

    /* event占用了aucContent前四个字节，消息内容需要需要偏移4个字节 */
    ulLen = pstAtIndEvt->ulLength + VOS_MSG_HEAD_LENGTH - sizeof(MN_AT_IND_EVT_STRU);

    TAF_MEM_SET_S((VOS_VOID *)(pstPrivacyAtIndEvt->aucContent + sizeof(enEvtId)),
                  ulLen,
                  0,
                  ulLen);

    return (VOS_VOID *)pstPrivacyAtIndEvt;
}


VOS_VOID* AT_PrivacyMatchCallAppEconfDialReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstEconfDialReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstEconfDialReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstEconfDialReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstEconfDialReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstEconfDialReq->unParm.stEconfDial.stEconfCalllist),
                  sizeof(TAF_CALL_ECONF_CALL_LIST_STRU),
                  0,
                  sizeof(TAF_CALL_ECONF_CALL_LIST_STRU));

    return (VOS_VOID *)pstEconfDialReq;
}


VOS_VOID* AT_PrivacyMatchRegisterSsMsg(
    MsgBlock                           *pstMsg
)
{
    MN_APP_REQ_MSG_STRU                *pstRegisterSs = VOS_NULL_PTR;
    TAF_SS_REGISTERSS_REQ_STRU         *pstSsRegReq   = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstRegisterSs,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsRegReq = (TAF_SS_REGISTERSS_REQ_STRU *)(pstRegisterSs->aucContent);

    TAF_MEM_SET_S(pstSsRegReq->aucFwdToNum,
                  sizeof(pstSsRegReq->aucFwdToNum),
                  0,
                  sizeof(pstSsRegReq->aucFwdToNum));

    TAF_MEM_SET_S(pstSsRegReq->aucFwdToSubAddr,
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

    TAF_MEM_CPY_S(pstProcessUss,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_PROCESS_USS_REQ_STRU *)pstProcessUss->aucContent;

    TAF_MEM_SET_S(&(pstSsReq->UssdStr),
                  sizeof(TAF_SS_USSD_STRING_STRU),
                  0,
                  sizeof(TAF_SS_USSD_STRING_STRU));

    TAF_MEM_SET_S(pstSsReq->aucMsisdn,
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

    TAF_MEM_CPY_S(pstInterRogate,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_INTERROGATESS_REQ_STRU *)pstInterRogate->aucContent;

    TAF_MEM_SET_S(pstSsReq->aucPassword,
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

    TAF_MEM_CPY_S(pstErasess,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_ERASESS_REQ_STRU *)pstErasess->aucContent;

    TAF_MEM_SET_S(pstSsReq->aucPassword,
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

    TAF_MEM_CPY_S(pstActivatess,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_ACTIVATESS_REQ_STRU *)pstActivatess->aucContent;

    TAF_MEM_SET_S(pstSsReq->aucPassword,
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

    TAF_MEM_CPY_S(pstDeactivatess,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_DEACTIVATESS_REQ_STRU *)pstDeactivatess->aucContent;

    TAF_MEM_SET_S(pstSsReq->aucPassword,
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

    TAF_MEM_CPY_S(pstRegpwdss,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSsReq = (TAF_SS_REGPWD_REQ_STRU *)pstRegpwdss->aucContent;

    TAF_MEM_SET_S(pstSsReq->aucOldPwdStr,
                  sizeof(pstSsReq->aucOldPwdStr),
                  0,
                  TAF_SS_MAX_PASSWORD_LEN + 1);

    TAF_MEM_SET_S(pstSsReq->aucNewPwdStr,
                  sizeof(pstSsReq->aucNewPwdStr),
                  0,
                  TAF_SS_MAX_PASSWORD_LEN + 1);

    TAF_MEM_SET_S(pstSsReq->aucNewPwdStrCnf,
                  sizeof(pstSsReq->aucNewPwdStrCnf),
                  0,
                  TAF_SS_MAX_PASSWORD_LEN + 1);

    return (VOS_VOID *)pstRegpwdss;
}

VOS_VOID* AT_PrivacyMatchCallAppOrigReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstCallOrigReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCallOrigReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                              DYNAMIC_MEM_PT,
                                                              ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCallOrigReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstCallOrigReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstCallOrigReq->unParm.stOrig.stDialNumber),
                  sizeof(MN_CALL_CALLED_NUM_STRU),
                  0,
                  sizeof(MN_CALL_CALLED_NUM_STRU));

    TAF_MEM_SET_S(&(pstCallOrigReq->unParm.stOrig.stSubaddr),
                  sizeof(MN_CALL_SUBADDR_STRU),
                  0,
                  sizeof(MN_CALL_SUBADDR_STRU));

    return (VOS_VOID *)pstCallOrigReq;
}


VOS_VOID* AT_PrivacyMatchCallAppSupsCmdReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstSupsCmdReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstSupsCmdReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                             DYNAMIC_MEM_PT,
                                                             ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstSupsCmdReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstSupsCmdReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstSupsCmdReq->unParm.stCallMgmtCmd.stRedirectNum),
                  sizeof(MN_CALL_BCD_NUM_STRU),
                  0,
                  sizeof(MN_CALL_BCD_NUM_STRU));

    TAF_MEM_SET_S(&(pstSupsCmdReq->unParm.stCallMgmtCmd.stRemoveNum),
                  sizeof(MN_CALL_CALLED_NUM_STRU),
                  0,
                  sizeof(MN_CALL_CALLED_NUM_STRU));

    return (VOS_VOID *)pstSupsCmdReq;
}


VOS_VOID* AT_PrivacyMatchCallAppStartDtmfReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstStartDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstStartDtmfReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                               DYNAMIC_MEM_PT,
                                                               ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstStartDtmfReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstStartDtmfReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstStartDtmfReq->unParm.stDtmf.cKey),
                  sizeof(VOS_CHAR),
                  0,
                  sizeof(VOS_CHAR));

    return (VOS_VOID *)pstStartDtmfReq;
}


VOS_VOID* AT_PrivacyMatchCallAppStopDtmfReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstStopDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstStopDtmfReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                              DYNAMIC_MEM_PT,
                                                              ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstStopDtmfReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstStopDtmfReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstStopDtmfReq->unParm.stDtmf.cKey),
                  sizeof(VOS_CHAR),
                  0,
                  sizeof(VOS_CHAR));

    return (VOS_VOID *)pstStopDtmfReq;
}


VOS_VOID* AT_PrivacyMatchCallAppCustomEccNumReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstCustomEccNumReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCustomEccNumReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                  DYNAMIC_MEM_PT,
                                                                  ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCustomEccNumReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstCustomEccNumReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstCustomEccNumReq->unParm.stEccNumReq.stEccNum),
                  sizeof(MN_CALL_BCD_NUM_STRU),
                  0,
                  sizeof(MN_CALL_BCD_NUM_STRU));

    return (VOS_VOID *)pstCustomEccNumReq;
}


VOS_VOID* AT_PrivacyMatchCallAppSetUusinfoReq(
    MsgBlock                           *pstMsg
)
{
    MN_CALL_APP_REQ_MSG_STRU           *pstCustomSetuusinfoReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCustomSetuusinfoReq = (MN_CALL_APP_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                      DYNAMIC_MEM_PT,
                                                                      ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCustomSetuusinfoReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstCustomSetuusinfoReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstCustomSetuusinfoReq->unParm.stUus1Info.stUus1Info,
                  sizeof(MN_CALL_UUS1_INFO_STRU) * MN_CALL_MAX_UUS1_MSG_NUM,
                  0,
                  sizeof(MN_CALL_UUS1_INFO_STRU) * MN_CALL_MAX_UUS1_MSG_NUM);

    return (VOS_VOID *)pstCustomSetuusinfoReq;
}


VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(
    MsgBlock                           *pstMsg
)
{
    TAF_SSA_EVT_STRU                   *pstSsaEvt        = VOS_NULL_PTR;
    TAF_SSA_EVT_STRU                   *pstPrivacySsaEvt = VOS_NULL_PTR;
    TAF_SSA_LCS_MOLR_NTF_STRU          *pstLcsMolrNtf    = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstPrivacySsaEvt,
                  ulLength,
                  pstMsg,
                  ulLength);

    if (ID_TAF_SSA_LCS_MOLR_NTF == pstSsaEvt->enEvtId)
    {
        pstLcsMolrNtf = (TAF_SSA_LCS_MOLR_NTF_STRU *)pstPrivacySsaEvt->aucContent;

        /* 将敏感信息设置为全0 */
        TAF_MEM_SET_S(pstLcsMolrNtf->acLocationStr,
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

    TAF_MEM_CPY_S(pstPrivacyAtIndEvt,
                  ulLength,
                  pstAtIndEvt,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pSsCallIndependentEvt = (TAF_SS_CALL_INDEPENDENT_EVENT_STRU *)(pstPrivacyAtIndEvt->aucContent);

    TAF_MEM_SET_S(&(pSsCallIndependentEvt->FwdInfo),
                  sizeof(TAF_SS_FORWARDINGINFO_STRU),
                  0,
                  sizeof(TAF_SS_FORWARDINGINFO_STRU));

    TAF_MEM_SET_S(&(pSsCallIndependentEvt->FwdFeaturelist),
                  sizeof(TAF_SS_FWDFEATURELIST_STRU),
                  0,
                  sizeof(TAF_SS_FWDFEATURELIST_STRU));

    TAF_MEM_SET_S(pSsCallIndependentEvt->aucPassWord,
                  TAF_SS_MAX_PASSWORD_LEN,
                  0,
                  TAF_SS_MAX_PASSWORD_LEN);

    TAF_MEM_SET_S(&(pSsCallIndependentEvt->UssdString),
                  sizeof(TAF_SS_USSD_STRING_STRU),
                  0,
                  sizeof(TAF_SS_USSD_STRING_STRU));

    TAF_MEM_SET_S(pSsCallIndependentEvt->aucMsisdn,
                  TAF_SS_MAX_MSISDN_LEN + 1,
                  0,
                  TAF_SS_MAX_MSISDN_LEN + 1);

    for (i = 0; i < TAF_SS_MAX_NUM_OF_CCBS_FEATURE; i++)
    {
        TAF_MEM_SET_S(pSsCallIndependentEvt->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].aucBSubscriberNum,
                      TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1,
                      0,
                      TAF_SS_MAX_NUM_OF_BSUBSCRIBER_NUMBER + 1);

        TAF_MEM_SET_S(pSsCallIndependentEvt->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].aucBSubscriberSubAddr,
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


VOS_VOID* TAF_CALL_PrivacyMatchAppCnapQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_APP_CNAP_QRY_CNF_STRU     *pstCnapQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCnapQryCnf = (TAF_CALL_APP_CNAP_QRY_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                   DYNAMIC_MEM_PT,
                                                                   ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCnapQryCnf)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstCnapQryCnf,
                  sizeof(TAF_CALL_APP_CNAP_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstCnapQryCnf->stNameIndicator),
                  sizeof(TAF_CALL_CNAP_STRU),
                  0,
                  sizeof(TAF_CALL_CNAP_STRU));

    return (VOS_VOID *)pstCnapQryCnf;
}


VOS_VOID* TAF_CALL_PrivacyMatchAppCnapInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_APP_CNAP_INFO_IND_STRU    *pstCnapInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 计算消息长度 */
    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 申请内存，后续统一由底层释放 */
    pstCnapInfoInd = (TAF_CALL_APP_CNAP_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                     DYNAMIC_MEM_PT,
                                                                     ulLength);

    /* 如果没有申请到内存，则返回空指针 */
    if (VOS_NULL_PTR == pstCnapInfoInd)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstCnapInfoInd,
                  sizeof(TAF_CALL_APP_CNAP_INFO_IND_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstCnapInfoInd->stNameIndicator),
                  sizeof(TAF_CALL_CNAP_STRU),
                  0,
                  sizeof(TAF_CALL_CNAP_STRU));

    return (VOS_VOID *)pstCnapInfoInd;
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

    TAF_MEM_CPY_S(pucSendAtMsg, ulLength, pstMsg, ulLength);

    pucMsgBuf = pucSendAtMsg + sizeof(MN_AT_IND_EVT_STRU) - 4;

    if (TAF_PH_ICC_ID == pucMsgBuf[3])
    {
        TAF_MEM_CPY_S(&usErrorCode, sizeof(usErrorCode), &pucMsgBuf[4], sizeof(usErrorCode));
        TAF_MEM_CPY_S(&usLen, sizeof(usLen), &pucMsgBuf[6], sizeof(usLen));

        if ((sizeof(TAF_PH_ICC_ID_STRU) == usLen) && (TAF_ERR_NO_ERROR == usErrorCode))
        {
            pstIccId = (TAF_PH_ICC_ID_STRU *)(pucMsgBuf + 8);

            /* 将敏感信息设置为全0 */
            TAF_MEM_SET_S(pstIccId->aucIccId,
                          sizeof(pstIccId->aucIccId),
                          0,
                          sizeof(pstIccId->aucIccId));
        }
    }

    return (VOS_VOID *)pucSendAtMsg;
}


VOS_VOID*  TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_APP_AT_CNF_STRU           *pstMatchTafXsmsAppAtCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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
    TAF_MEM_CPY_S(pstMatchTafXsmsAppAtCnf,
                  sizeof(TAF_XSMS_APP_AT_CNF_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(&(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
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
    TAF_MEM_CPY_S(pstMatchTafXsmsAppAtCnf,
                  sizeof(TAF_XSMS_APP_AT_CNF_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(&(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
                  sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent),
                  0,
                  sizeof(pstMatchTafXsmsAppAtCnf->stXsmsAtEvent.XSmsEvent));

    return (VOS_VOID *)pstMatchTafXsmsAppAtCnf;
}


VOS_VOID* AT_PrivacyMatchCallAppSendCustomDialReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU                 *pstCustomDialReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstCustomDialReq  = (TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                               DYNAMIC_MEM_PT,
                                                                               ulLength);

    if (VOS_NULL_PTR == pstCustomDialReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S((VOS_UINT8 *)pstCustomDialReq,
                  sizeof(TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(&(pstCustomDialReq->stCustomDialPara.stDialNumber),
                  sizeof(MN_CALL_CALLED_NUM_STRU),
                  0,
                  sizeof(MN_CALL_CALLED_NUM_STRU));

    return (VOS_VOID *)pstCustomDialReq;
}


VOS_VOID*  AT_PrivacyMatchAppMsgTypeSendReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_SEND_MSG_REQ_STRU         *pstMatchTafXsmsSendMsgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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
    TAF_MEM_CPY_S(pstMatchTafXsmsSendMsgReq,
                  sizeof(TAF_XSMS_SEND_MSG_REQ_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(&(pstMatchTafXsmsSendMsgReq->st1XSms.stAddr),
                  sizeof(TAF_XSMS_ADDR_STRU),
                  0,
                  sizeof(TAF_XSMS_ADDR_STRU));

    TAF_MEM_SET_S(&(pstMatchTafXsmsSendMsgReq->st1XSms.stSubAddr),
                  sizeof(TAF_XSMS_SUB_ADDR_STRU),
                  0,
                  sizeof(TAF_XSMS_SUB_ADDR_STRU));

    TAF_MEM_SET_S(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData,
                  sizeof(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData),
                  0,
                  sizeof(pstMatchTafXsmsSendMsgReq->st1XSms.aucBearerData));

    pstMatchTafXsmsSendMsgReq->st1XSms.ulBearerDataLen = 0;

    return (VOS_VOID *)pstMatchTafXsmsSendMsgReq;
}



VOS_VOID*  AT_PrivacyMatchCposSetReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgCposSetReq = VOS_NULL_PTR;
    AT_MTA_CPOS_REQ_STRU               *pstCposReq               = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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
    TAF_MEM_CPY_S(pstMatchAppMsgCposSetReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(pstCposReq->acXmlText,
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
    DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU        *pstSimlockWriteExSetReq;
    VOS_UINT32                                    ulLength;

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
    TAF_MEM_CPY_S(pstMatchAppMsgSimlockWriteExSetReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(pstSimlockWriteExSetReq,
                  (ulLength - sizeof(MN_APP_REQ_MSG_STRU) + sizeof(pstMatchAppMsgSimlockWriteExSetReq->aucContent)),
                  0x00,
                  sizeof(DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU));

    return (VOS_VOID *)pstMatchAppMsgSimlockWriteExSetReq;
}


VOS_VOID* AT_PrivacyMatchImsaImsCtrlMsg(
    MsgBlock                                               *pstMsg
)
{
    AT_IMSA_IMS_CTRL_MSG_STRU          *pstImsCtrlMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstImsCtrlMsg = (AT_IMSA_IMS_CTRL_MSG_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                              DYNAMIC_MEM_PT,
                                                              ulLength);

    if (VOS_NULL_PTR == pstImsCtrlMsg)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstImsCtrlMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstImsCtrlMsg->aucWifiMsg,
                  pstImsCtrlMsg->ulWifiMsgLen,
                  0,
                  pstImsCtrlMsg->ulWifiMsgLen);

    return (VOS_VOID *)pstImsCtrlMsg;
}


VOS_VOID* AT_PrivacyMatchImsaNickNameSetReq(
    MsgBlock                                               *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgNickNameSetReq = VOS_NULL_PTR;
    IMSA_AT_NICKNAME_INFO_STRU         *pstNickNameInfo;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstMatchAppMsgNickNameSetReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstNickNameInfo->acNickName,
                  MN_CALL_DISPLAY_NAME_STRING_SZ,
                  0,
                  MN_CALL_DISPLAY_NAME_STRING_SZ);

    return (VOS_VOID *)pstMatchAppMsgNickNameSetReq;
}


VOS_VOID*  AT_PrivacyMatchMeidSetReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    MN_APP_REQ_MSG_STRU                *pstMatchAppMsgSetReq = VOS_NULL_PTR;
    AT_MTA_MEID_SET_REQ_STRU           *pstMeidReq           = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstMatchAppMsgSetReq,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstMeidReq = (AT_MTA_MEID_SET_REQ_STRU *)pstMatchAppMsgSetReq->aucContent;

    /* 过滤敏感消息 */
    TAF_MEM_SET_S(pstMeidReq->aucMeid,
                  sizeof(pstMeidReq->aucMeid),
                  0x00,
                  sizeof(pstMeidReq->aucMeid));

    return (VOS_VOID *)pstMatchAppMsgSetReq;
}


VOS_VOID*  AT_PrivacyMatchAppMsgTypeWriteReq(
    MsgBlock                           *pstMsg
)
{
    /* 记录申请的内存 */
    TAF_XSMS_WRITE_MSG_REQ_STRU        *pstMatchTafXsmsWriteMsgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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
    TAF_MEM_CPY_S(pstMatchTafXsmsWriteMsgReq,
                  sizeof(TAF_XSMS_WRITE_MSG_REQ_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(&(pstMatchTafXsmsWriteMsgReq->st1XSms.stAddr),
                  sizeof(TAF_XSMS_ADDR_STRU),
                  0,
                  sizeof(TAF_XSMS_ADDR_STRU));

    TAF_MEM_SET_S(&(pstMatchTafXsmsWriteMsgReq->st1XSms.stSubAddr),
                  sizeof(TAF_XSMS_SUB_ADDR_STRU),
                  0,
                  sizeof(TAF_XSMS_SUB_ADDR_STRU));

    TAF_MEM_SET_S(pstMatchTafXsmsWriteMsgReq->st1XSms.aucBearerData,
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
    TAF_MEM_CPY_S(pstMatchTafXsmsDeleteMsgReq,
                  sizeof(TAF_XSMS_DELETE_MSG_REQ_STRU),
                  pstMsg,
                  ulLength);

    pstMatchTafXsmsDeleteMsgReq->ucIndex = 0;

    return (VOS_VOID *)pstMatchTafXsmsDeleteMsgReq;
}



VOS_VOID* AT_PrivacyMatchCallAppSendFlashReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_APP_SEND_FLASH_REQ_STRU   *pstFlashReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstFlashReq  = (TAF_CALL_APP_SEND_FLASH_REQ_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                    DYNAMIC_MEM_PT,
                                                                    ulLength);

    if (VOS_NULL_PTR == pstFlashReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstFlashReq,
                  sizeof(TAF_CALL_APP_SEND_FLASH_REQ_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(pstFlashReq->stFlashPara.aucDigit,
                  sizeof(pstFlashReq->stFlashPara.aucDigit),
                  0,
                  sizeof(pstFlashReq->stFlashPara.aucDigit));

    pstFlashReq->stFlashPara.ucDigitNum = 0;

    return (VOS_VOID *)pstFlashReq;
}


VOS_VOID* AT_PrivacyMatchCallAppSendBurstReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_BURST_DTMF_REQ_MSG_STRU   *pstBurstDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstBurstDtmfReq  = (TAF_CALL_BURST_DTMF_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulLength);

    if (VOS_NULL_PTR == pstBurstDtmfReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstBurstDtmfReq,
                  sizeof(TAF_CALL_BURST_DTMF_REQ_MSG_STRU),
                  pstMsg,
                  ulLength);

    TAF_MEM_SET_S(pstBurstDtmfReq->stBurstDTMFPara.aucDigit,
                  sizeof(pstBurstDtmfReq->stBurstDTMFPara.aucDigit),
                  0,
                  sizeof(pstBurstDtmfReq->stBurstDTMFPara.aucDigit));

    pstBurstDtmfReq->stBurstDTMFPara.ucDigitNum  = 0;
    pstBurstDtmfReq->stBurstDTMFPara.ulOnLength  = 0;
    pstBurstDtmfReq->stBurstDTMFPara.ulOffLength = 0;

    return (VOS_VOID *)pstBurstDtmfReq;
}


VOS_VOID* AT_PrivacyMatchCallAppSendContReq(
    MsgBlock                           *pstMsg
)
{
    TAF_CALL_CONT_DTMF_REQ_MSG_STRU    *pstConttDtmfReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstConttDtmfReq  = (TAF_CALL_CONT_DTMF_REQ_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                       DYNAMIC_MEM_PT,
                                                                       ulLength);

    if (VOS_NULL_PTR == pstConttDtmfReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstConttDtmfReq,
                  sizeof(TAF_CALL_CONT_DTMF_REQ_MSG_STRU),
                  pstMsg,
                  ulLength);

    pstConttDtmfReq->stContDTMFPara.ucDigit  = 0;

    return (VOS_VOID *)pstConttDtmfReq;
}


VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(
    MsgBlock                           *pstMsg
)
{
    AT_XPDS_GPS_POS_INFO_RSP_STRU      *pstPrivacyMatchMsg  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_GPS_POS_INFO_RSP_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchMsg,
                  sizeof(AT_XPDS_GPS_POS_INFO_RSP_STRU),
                  pstMsg,
                  ulMsgLength);

    TAF_MEM_SET_S(&(pstPrivacyMatchMsg->stPosInfo),
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

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_GPS_PRM_INFO_RSP_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                        DYNAMIC_MEM_PT,
                                                                        ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchMsg,
                  sizeof(AT_XPDS_GPS_PRM_INFO_RSP_STRU),
                  pstMsg,
                  ulMsgLength);

    ulMaxPrmDataLen = sizeof(AT_XPDS_GPS_MODEM_PRMDATA_STRU) * TAF_MSG_CDMA_MAX_SV_NUM;

    TAF_MEM_SET_S(pstPrivacyMatchMsg->astMseasData,
                  ulMaxPrmDataLen,
                  0,
                  sizeof(pstPrivacyMatchMsg->astMseasData));

    pstPrivacyMatchMsg->ucMeasNum = 0;

    return (VOS_VOID *)pstPrivacyMatchMsg;
}


VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(
    MsgBlock                           *pstMsg
)
{
    AT_XPDS_AP_FORWARD_DATA_IND_STRU   *pstPrivacyMatchMsg  = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;

    ulMsgLength     = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchMsg  = (AT_XPDS_AP_FORWARD_DATA_IND_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchMsg)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchMsg,
                  ulMsgLength,
                  pstMsg,
                  ulMsgLength);

    TAF_MEM_SET_S(pstPrivacyMatchMsg->aucData,
                  pstPrivacyMatchMsg->ulDataLen,
                  0,
                  pstPrivacyMatchMsg->ulDataLen);

    pstPrivacyMatchMsg->ulDataLen = 0;

    return (VOS_VOID *)pstPrivacyMatchMsg;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_REFLOC_INFO_CNF_STRU   *pstPrivacyMatchRefLocInfo   = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLength;

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchRefLocInfo   = (XPDS_AT_GPS_REFLOC_INFO_CNF_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                   DYNAMIC_MEM_PT,
                                                                                   ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchRefLocInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchRefLocInfo,
                  sizeof(XPDS_AT_GPS_REFLOC_INFO_CNF_STRU),
                  pstMsg,
                  ulMsgLength);

    /* 替换RefLoc信息敏感数据 */
    TAF_MEM_SET_S(&(pstPrivacyMatchRefLocInfo->stRefLoc),
                  sizeof(XPDS_AT_GPS_REFLOC_INFO_STRU),
                  0,
                  sizeof(XPDS_AT_GPS_REFLOC_INFO_STRU));

    return (VOS_VOID *)pstPrivacyMatchRefLocInfo;
}


VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(
    MsgBlock                           *pstMsg
)
{
    XPDS_AT_GPS_ION_INFO_IND_STRU      *pstPrivacyMatchIonInfo;
    VOS_UINT32                          ulMsgLength;

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchIonInfo = (XPDS_AT_GPS_ION_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchIonInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchIonInfo,
                  sizeof(XPDS_AT_GPS_ION_INFO_IND_STRU),
                  pstMsg,
                  ulMsgLength);

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

    ulMsgLength             = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchEphInfo  = (XPDS_AT_GPS_EPH_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchEphInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchEphInfo,
                  sizeof(XPDS_AT_GPS_EPH_INFO_IND_STRU),
                  pstMsg,
                  ulMsgLength);

    ulMaxEphDataLen = sizeof(XPDS_AT_EPH_DATA_STRU) * TAF_MSG_CDMA_MAX_EPH_PRN_NUM;

    /* 替换星历信息敏感数据 */
    TAF_MEM_SET_S(pstPrivacyMatchEphInfo->astEphData,
                  ulMaxEphDataLen,
                  0,
                  sizeof(pstPrivacyMatchEphInfo->astEphData));

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

    ulMsgLength             = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchAlmInfo  = (XPDS_AT_GPS_ALM_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                            DYNAMIC_MEM_PT,
                                                                            ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchAlmInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchAlmInfo,
                  sizeof(XPDS_AT_GPS_ALM_INFO_IND_STRU),
                  pstMsg,
                  ulMsgLength);

    ulMaxAlmDataLen = sizeof(XPDS_AT_ALM_DATA_STRU) * TAF_MSG_CDMA_MAX_ALM_PRN_NUM;

    TAF_MEM_SET_S(pstPrivacyMatchAlmInfo->astAlmData,
                  ulMaxAlmDataLen,
                  0,
                  sizeof(pstPrivacyMatchAlmInfo->astAlmData));

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

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchPosiInfo = (XPDS_AT_GPS_PDE_POSI_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                 DYNAMIC_MEM_PT,
                                                                                 ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchPosiInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchPosiInfo,
                  sizeof(XPDS_AT_GPS_PDE_POSI_INFO_IND_STRU),
                  pstMsg,
                  ulMsgLength);

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

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchAssistData   = (XPDS_AT_GPS_ACQ_ASSIST_DATA_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                       DYNAMIC_MEM_PT,
                                                                                       ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchAssistData)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchAssistData,
                  sizeof(XPDS_AT_GPS_ACQ_ASSIST_DATA_IND_STRU),
                  pstMsg,
                  ulMsgLength);

    ulMaxAssistDataLen = sizeof(TAF_MSG_CDMA_ACQASSIST_DATA_STRU) * TAF_MSG_CDMA_MAX_SV_NUM;

    TAF_MEM_SET_S(pstPrivacyMatchAssistData->astAaData,
                  ulMaxAssistDataLen,
                  0,
                  sizeof(pstPrivacyMatchAssistData->astAaData));

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

    pstMsgReverseDataInd = (XPDS_AT_AP_REVERSE_DATA_IND_STRU *)pstMsg;

    ulMsgLength                 = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchReverseData = (XPDS_AT_AP_REVERSE_DATA_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                  DYNAMIC_MEM_PT,
                                                                                  ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchReverseData)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchReverseData,
                  ulMsgLength,
                  pstMsg,
                  ulMsgLength);

    /* 清理用户隐私信息 */
    TAF_MEM_SET_S(pstPrivacyMatchReverseData->aucData,
                  pstMsgReverseDataInd->ulDataLen,
                  0,
                  pstMsgReverseDataInd->ulDataLen);

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

    ulMsgLength = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    pstPrivacyMatchUtsPosInfo = (XPDS_AT_UTS_GPS_POS_INFO_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                                  DYNAMIC_MEM_PT,
                                                                                  ulMsgLength);

    if (VOS_NULL_PTR == pstPrivacyMatchUtsPosInfo)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPrivacyMatchUtsPosInfo,
                  sizeof(XPDS_AT_UTS_GPS_POS_INFO_IND_STRU),
                  pstMsg,
                  ulMsgLength);

    TAF_MEM_SET_S((VOS_UINT8 *)&pstPrivacyMatchUtsPosInfo->stPosInfo,
                   sizeof(AT_XPDS_GPS_POS_INFO_STRU),
                   0,
                   sizeof(AT_XPDS_GPS_POS_INFO_STRU));

    return (VOS_VOID *)pstPrivacyMatchUtsPosInfo;
}



VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg   = VOS_NULL_PTR;
    MTA_AT_CPOSR_IND_STRU              *pstCposrInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstCposrInd = (MTA_AT_CPOSR_IND_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstCposrInd->acXmlText,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstMeidQryCnf = (MTA_AT_MEID_QRY_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstMeidQryCnf->aucEFRUIMID,
                  MTA_AT_EFRUIMID_OCTET_LEN_EIGHT,
                  0,
                  MTA_AT_EFRUIMID_OCTET_LEN_EIGHT);

    TAF_MEM_SET_S(pstMeidQryCnf->aucMeID,
                  sizeof(pstMeidQryCnf->aucMeID),
                  0,
                  sizeof(pstMeidQryCnf->aucMeID));

    TAF_MEM_SET_S(pstMeidQryCnf->aucPEsn,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstCgsnQryCnf = (MTA_AT_CGSN_QRY_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstCgsnQryCnf->aucImei,
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

    TAF_MEM_CPY_S(pstSndMsg, ulLength, pstMsg, ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&pstSndMsg->aucIMSI[4],
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

    TAF_MEM_CPY_S(pstSndMsg, ulLength, pstMsg, ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&pstSndMsg->stEHplmnInfo.aucImsi[4],
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_LOCATION_INFO_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->ulLac    = 0;
    pstSndMsg->ucRac    = 0;
    pstSndMsg->ulCellid = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtRegStatusInd(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_REG_STATUS_IND_STRU        *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_REG_STATUS_IND_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSndMsg->stRegStatus.CellId.aulCellId,
                  sizeof(pstSndMsg->stRegStatus.CellId.aulCellId),
                  0,
                  sizeof(pstSndMsg->stRegStatus.CellId.aulCellId));

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_SRCHED_PLMN_INFO_IND_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_MAX_SRCHED_LAI_NUM; i++)
    {
        pstSndMsg->astLai[i].ulLac = 0;
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_CDMA_LOCINFO_QRY_CNF_STRU  *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_CDMA_LOCINFO_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stClocinfoPara.ulBaseId = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtNetScanCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_NET_SCAN_CNF_STRU          *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          i;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_NET_SCAN_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    for (i = 0; i < TAF_MMA_NET_SCAN_MAX_FREQ_NUM; i++)
    {
        pstSndMsg->astNetScanInfo[i].ulCellId = 0;
        pstSndMsg->astNetScanInfo[i].usLac    = 0;
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtRegStateQryCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_REG_STATE_QRY_CNF_STRU     *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_REG_STATE_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stRegInfo.ulLac = 0;
    pstSndMsg->stRegInfo.ucRac = 0;

    TAF_MEM_SET_S(pstSndMsg->stRegInfo.CellId.aulCellId,
                  sizeof(pstSndMsg->stRegInfo.CellId.aulCellId),
                  0,
                  sizeof(pstSndMsg->stRegInfo.CellId.aulCellId));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtClocInfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_CLOCINFO_IND_STRU          *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_CLOCINFO_IND_STRU),
                  pstMsg,
                  ulLength);

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_REJINFO_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stPhoneRejInfo.ucRac    = 0;
    pstSndMsg->stPhoneRejInfo.ulLac    = 0;
    pstSndMsg->stPhoneRejInfo.ulCellId = 0;

    return (VOS_VOID *)pstSndMsg;
}



VOS_VOID* RNIC_PrivacyMatchCdsImsDataReq(
    MsgBlock                           *pstMsg
)
{
    RNIC_CDS_IMS_DATA_REQ_STRU         *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSndMsg->aucData,
                  pstSndMsg->usDataLen,
                  0,
                  pstSndMsg->usDataLen);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_PrivacyMatchDsmPsCallOrigReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_CALL_ORIG_REQ_STRU          *pstCallOrigReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_ORIG_REQ_STRU) + sizeof(MSG_HEADER_STRU),
                  pstMsg,
                  ulLength);

    pstCallOrigReq = (TAF_PS_CALL_ORIG_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv4Addr,
                  sizeof(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    TAF_MEM_SET_S(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv6Addr,
                  sizeof(pstCallOrigReq->stDialParaInfo.stPdpAddr.aucIpv6Addr),
                  0,
                  TAF_IPV6_ADDR_LEN);

    TAF_MEM_SET_S(pstCallOrigReq->stDialParaInfo.aucPassWord,
                  sizeof(pstCallOrigReq->stDialParaInfo.aucPassWord),
                  0,
                  TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

    TAF_MEM_SET_S(pstCallOrigReq->stDialParaInfo.aucUserName,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_PPP_DIAL_ORIG_REQ_STRU) + sizeof(MSG_HEADER_STRU),
                  pstMsg,
                  ulLength);

    pstDialOrigReq = (TAF_PS_PPP_DIAL_ORIG_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstDialOrigReq->stPppDialParaInfo.stPppReqConfigInfo.stAuth.enAuthContent),
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU) + sizeof(MSG_HEADER_STRU),
                  pstMsg,
                  ulLength);

    pstSetPdpCtxReq = (TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv4Addr,
                  sizeof(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    TAF_MEM_SET_S(pstSetPdpCtxReq->stPdpContextInfo.stPdpAddr.aucIpv6Addr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_SET_TFT_INFO_REQ_STRU) + sizeof(MSG_HEADER_STRU),
                  pstMsg,
                  ulLength);

    pstSetTftInfoReq = (TAF_PS_SET_TFT_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSetTftInfoReq->stTftInfo.aucLocalIpv4Addr,
                  sizeof(pstSetTftInfoReq->stTftInfo.aucLocalIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    TAF_MEM_SET_S(pstSetTftInfoReq->stTftInfo.aucLocalIpv6Addr,
                  sizeof(pstSetTftInfoReq->stTftInfo.aucLocalIpv6Addr),
                  0,
                  TAF_IPV6_ADDR_LEN);

    TAF_MEM_SET_S(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv4Addr,
                  sizeof(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    TAF_MEM_SET_S(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv6Addr,
                  sizeof(pstSetTftInfoReq->stTftInfo.stSourceIpaddr.aucIpv6Addr),
                  0,
                  TAF_IPV6_ADDR_LEN);

    pstSetTftInfoReq->stTftInfo.ucLocalIpv6Prefix = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstPdpActCnf  = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpActCnf = (TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    TAF_MEM_SET_S(pstPdpActCnf->stPdpAddr.aucIpv4Addr,
                  sizeof(pstPdpActCnf->stPdpAddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    TAF_MEM_SET_S(pstPdpActCnf->stPdpAddr.aucIpv6Addr,
                  sizeof(pstPdpActCnf->stPdpAddr.aucIpv6Addr),
                  0,
                  TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpActCnf->stIpv4PcscfList.astIpv4PcscfAddrList,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpActCnf->stIpv6PcscfList.astIpv6PcscfAddrList,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* PCO */
    TAF_MEM_SET_S(pstPdpActCnf->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    /* EPDG 地址 */
    TAF_MEM_SET_S(pstPdpActCnf->stEpdgInfo.astIpv4EpdgList,
                  sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM,
                  0,
                  sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM);

    TAF_MEM_SET_S(pstPdpActCnf->stEpdgInfo.astIpv6EpdgList,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_ACTIVATE_IND_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpActInd = (TAF_PS_CALL_PDP_ACTIVATE_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    TAF_MEM_SET_S(pstPdpActInd->stPdpAddr.aucIpv4Addr,
                  sizeof(pstPdpActInd->stPdpAddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    TAF_MEM_SET_S(pstPdpActInd->stPdpAddr.aucIpv6Addr,
                  sizeof(pstPdpActInd->stPdpAddr.aucIpv6Addr),
                  0,
                  TAF_IPV6_ADDR_LEN);

    /* IPV4 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpActInd->stIpv4PcscfList.astIpv4PcscfAddrList,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpActInd->stIpv6PcscfList.astIpv6PcscfAddrList,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* PCO */
    TAF_MEM_SET_S(pstPdpActInd->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    /* EPDG 地址 */
    TAF_MEM_SET_S(pstPdpActInd->stEpdgInfo.astIpv4EpdgList,
                  sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM,
                  0,
                  sizeof(TAF_IPV4_EPDG_STRU) * TAF_MAX_IPV4_EPDG_NUM);

    TAF_MEM_SET_S(pstPdpActInd->stEpdgInfo.astIpv6EpdgList,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_MANAGE_IND_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpManageInd = (TAF_PS_CALL_PDP_MANAGE_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 地址 */
    TAF_MEM_SET_S(pstPdpManageInd->stPdpAddr.aucIpv4Addr,
                  sizeof(pstPdpManageInd->stPdpAddr.aucIpv4Addr),
                  0,
                  TAF_IPV4_ADDR_LEN);

    /* IPV6 地址 */
    TAF_MEM_SET_S(pstPdpManageInd->stPdpAddr.aucIpv6Addr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_MODIFY_CNF_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpModCnf = (TAF_PS_CALL_PDP_MODIFY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpModCnf->stIpv4PcscfList.astIpv4PcscfAddrList,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpModCnf->stIpv6PcscfList.astIpv6PcscfAddrList,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);
    /* PCO */
    TAF_MEM_SET_S(pstPdpModCnf->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg    = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_MODIFY_IND_STRU    *pstPdpModInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_MODIFY_CNF_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpModInd = (TAF_PS_CALL_PDP_MODIFY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV4 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpModInd->stIpv4PcscfList.astIpv4PcscfAddrList,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV4_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* IPV6 PCSCF 地址 */
    TAF_MEM_SET_S(pstPdpModInd->stIpv6PcscfList.astIpv6PcscfAddrList,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM,
                  0,
                  sizeof(TAF_PDP_IPV6_PCSCF_STRU) * TAF_PCSCF_ADDR_MAX_NUM);

    /* PCO */
    TAF_MEM_SET_S(pstPdpModInd->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpDeactCnf(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                                        *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU                    *pstPdpDeactCnf = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpDeactCnf = (TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* PCO */
    TAF_MEM_SET_S(pstPdpDeactCnf->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpDeactInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                                        *pstSndMsg      = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU                    *pstPdpDeactInd = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpDeactInd = (TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* PCO */
    TAF_MEM_SET_S(pstPdpDeactInd->stCustomPcoInfo.astContainerList,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM,
                  0,
                  sizeof(TAF_PS_CUSTOM_PCO_CONTAINER_STRU) * TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM);

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_EVT_STRU                    *pstSndMsg         = VOS_NULL_PTR;
    TAF_PS_IPV6_INFO_IND_STRU          *pstPdpIpv6InfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_IPV6_INFO_IND_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPdpIpv6InfoInd = (TAF_PS_IPV6_INFO_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    /* IPV6 地址 */
    TAF_MEM_SET_S(pstPdpIpv6InfoInd->stIpv6RaInfo.astPrefixList,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstPdpCtxInfoCnf = (TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstPdpCtxInfoCnf->ulCidNum; i++)
    {
        /* IPV4 地址 */
        TAF_MEM_SET_S(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv4Addr,
                      sizeof(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv4Addr),
                      0,
                      TAF_IPV4_ADDR_LEN);
        /* IPV6 地址 */
        TAF_MEM_SET_S(pstPdpCtxInfoCnf->astPdpContextQueryInfo[i].stPriPdpInfo.stPdpAddr.aucIpv6Addr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstGetTftInfoCnf = (TAF_PS_GET_TFT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstGetTftInfoCnf->ulCidNum; i++)
    {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++)
        {
            /* IPV4 地址 */
            TAF_MEM_SET_S(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv4Addr,
                          sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv4Addr),
                          0,
                          TAF_IPV4_ADDR_LEN);

            TAF_MEM_SET_S(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv4Address,
                          sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv4Address),
                          0,
                          TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            TAF_MEM_SET_S(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv6Addr,
                          sizeof(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucLocalIpv6Addr),
                          0,
                          TAF_IPV6_ADDR_LEN);

            TAF_MEM_SET_S(pstGetTftInfoCnf->astTftQueryInfo[i].astPfInfo[j].aucRmtIpv6Address,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstIpAddrInfo = (TAF_PS_GET_PDP_IP_ADDR_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i <= TAF_MAX_CID; i++)
    {
        /* IPV4 地址 */
        TAF_MEM_SET_S(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv4Addr,
                      sizeof(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv4Addr),
                      0,
                      TAF_IPV4_ADDR_LEN);

        /* IPV6 地址 */
        TAF_MEM_SET_S(pstIpAddrInfo->astPdpAddrQueryInfo[i].stPdpAddr.aucIpv6Addr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstTftInfo = (TAF_PS_GET_DYNAMIC_TFT_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstTftInfo->ulCidNum; i++)
    {
        for (j = 0; j < TAF_MAX_SDF_PF_NUM; j++)
        {
            /* IPV4 地址 */
            TAF_MEM_SET_S(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv4Addr,
                          sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv4Addr),
                          0,
                          TAF_IPV4_ADDR_LEN);

            TAF_MEM_SET_S(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv4Addr,
                          sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv4Addr),
                          0,
                          TAF_IPV4_ADDR_LEN);

            /* IPV6 地址 */
            TAF_MEM_SET_S(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv6Addr,
                          sizeof(pstTftInfo->astPfTftInfo[i].astTftInfo[j].aucLocalIpv6Addr),
                          0,
                          TAF_IPV6_ADDR_LEN);

            TAF_MEM_SET_S(pstTftInfo->astPfTftInfo[i].astTftInfo[j].stSourceIpaddr.aucIpv6Addr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstAuthdataInfo = (TAF_PS_GET_AUTHDATA_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    for (i = 0; i < pstAuthdataInfo->ulCidNum; i++)
    {
         TAF_MEM_SET_S(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucPassword,
                      sizeof(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucPassword),
                      0,
                      TAF_MAX_AUTHDATA_PASSWORD_LEN + 1);

         TAF_MEM_SET_S(pstAuthdataInfo->astAuthDataQueryInfo[i].stAuthDataInfo.aucUsername,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_PS_REPORT_PCO_INFO_IND_STRU) + sizeof(MSG_HEADER_STRU) + sizeof(VOS_UINT32),
                  pstMsg,
                  ulLength);

    pstPcoInfoInd = (TAF_PS_REPORT_PCO_INFO_IND_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstPcoInfoInd->stCustomPcoInfo.astContainerList,
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

    TAF_MEM_CPY_S(pstSndMsg, ulLength, pstMsg, ulLength);

    pstMsidQryCnf = (DRV_AGENT_MSID_QRY_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstMsidQryCnf->aucImei,
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
    TAF_MEM_SET_S(pucAppReq, ulLength, 0, ulLength);

    TAF_MEM_CPY_S(pucAppReq, ulLength, pstInputAppReq, ulCopyLength);

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

    pstSrcMsg = (MN_AT_IND_EVT_STRU *)pstMsg;

    ulEventLen = sizeof(MN_MSG_EVENT_ENUM_U32);
    TAF_MEM_CPY_S(&enEvent,  sizeof(enEvent), pstSrcMsg->aucContent, ulEventLen);

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

    TAF_MEM_CPY_S(pstSendAtMsg, ulLength, pstMsg, ulLength);

    pstEvent = (MN_MSG_EVENT_INFO_STRU *)&pstSendAtMsg->aucContent[ulEventLen];

    TAF_MEM_SET_S(&pstEvent->u, sizeof(pstEvent->u), 0, sizeof(pstEvent->u));

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstMtaAtEcidSetCnf = (MTA_AT_ECID_SET_CNF_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0,清空字符串,不清楚log打印机制，这里把字符串全部清0 */
    if (pstMsg->ulLength > (sizeof(AT_APPCTRL_STRU) + sizeof(pstSndMsg->ulMsgId)))
    {
        ulContentLength = pstMsg->ulLength - (sizeof(AT_APPCTRL_STRU) + sizeof(pstSndMsg->ulMsgId) + sizeof(MTA_AT_RESULT_ENUM_UINT32));
        TAF_MEM_SET_S(pstMtaAtEcidSetCnf->aucCellInfoStr,
                      ulContentLength,
                      0x00,
                      ulContentLength);
    }

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoSetReq(
    MsgBlock                           *pstMsg
)
{
    TAF_MMA_EFLOCIINFO_SET_REQ_STRU    *pstSndMsg     = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_EFLOCIINFO_SET_REQ_STRU),
                  pstMsg,
                  ulLength);

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_EFLOCIINFO_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_EFPSLOCIINFO_SET_REQ_STRU),
                  pstMsg,
                  ulLength);

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

    TAF_MEM_CPY_S(pstSndMsg,
                  sizeof(TAF_MMA_EFPSLOCIINFO_QRY_CNF_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstSndMsg->stPsEflociInfo.ulPTmsi = 0;
    pstSndMsg->stPsEflociInfo.usLac   = 0;
    pstSndMsg->stPsEflociInfo.ucRac   = 0;

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_DSM_PrivacyMatchTafSetAuthDataReq(
    MsgBlock                           *pstMsg
)
{
    TAF_PS_MSG_STRU                    *pstSndMsg         = VOS_NULL_PTR;
    TAF_PS_SET_AUTHDATA_INFO_REQ_STRU  *pstSetAuthDataReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstSetAuthDataReq = (TAF_PS_SET_AUTHDATA_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSetAuthDataReq->stAuthDataInfo.aucPassWord,
                  sizeof(pstSetAuthDataReq->stAuthDataInfo.aucPassWord),
                  0x00,
                  sizeof(pstSetAuthDataReq->stAuthDataInfo.aucPassWord));

    TAF_MEM_SET_S(pstSetAuthDataReq->stAuthDataInfo.aucUserName,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstSetSetPdpDnsInfoReq = (TAF_PS_SET_PDP_DNS_INFO_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr,
                  sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr),
                  0x00,
                  sizeof(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucPrimDnsAddr));

    TAF_MEM_SET_S(pstSetSetPdpDnsInfoReq->stPdpDnsInfo.aucSecDnsAddr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstSetGetPdpDnsInfoCnf = (TAF_PS_GET_PDP_DNS_INFO_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */

    for (ulIndex = 0; ulIndex < TAF_MIN(pstSetGetPdpDnsInfoCnf->ulCidNum, TAF_MAX_CID); ulIndex++)
    {
        TAF_MEM_SET_S(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr,
                      sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr),
                      0x00,
                      sizeof(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucPrimDnsAddr));

        TAF_MEM_SET_S(pstSetGetPdpDnsInfoCnf->astPdpDnsQueryInfo[ulIndex].stDnsInfo.aucSecDnsAddr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstGetNegotiationDnsCnf = (TAF_PS_GET_NEGOTIATION_DNS_CNF_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr,
                  sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr),
                  0x00,
                  sizeof(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr));

    TAF_MEM_SET_S(pstGetNegotiationDnsCnf->stNegotiationDns.stDnsInfo.aucSecDnsAddr,
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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstMtaAtSetNetMonScellCnf = (MTA_AT_NETMON_CELL_INFO_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstMtaAtSetNetMonScellCnf->stNCellInfo),
                  sizeof(pstMtaAtSetNetMonScellCnf->stNCellInfo),
                  0x00,
                  sizeof(pstMtaAtSetNetMonScellCnf->stNCellInfo));

    TAF_MEM_SET_S(&(pstMtaAtSetNetMonScellCnf->unSCellInfo),
                  sizeof(pstMtaAtSetNetMonScellCnf->unSCellInfo),
                  0x00,
                  sizeof(pstMtaAtSetNetMonScellCnf->unSCellInfo));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf(
    MsgBlock                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstSndMsg     = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstMtaAtSetNetMonScellCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstMtaAtSetNetMonScellCnf = (MTA_AT_NETMON_CELL_INFO_STRU *)(pstSndMsg->aucContent);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstMtaAtSetNetMonScellCnf->stNCellInfo),
                  sizeof(pstMtaAtSetNetMonScellCnf->stNCellInfo),
                  0x00,
                  sizeof(pstMtaAtSetNetMonScellCnf->stNCellInfo));

    TAF_MEM_SET_S(&(pstMtaAtSetNetMonScellCnf->unSCellInfo),
                  sizeof(pstMtaAtSetNetMonScellCnf->unSCellInfo),
                  0x00,
                  sizeof(pstMtaAtSetNetMonScellCnf->unSCellInfo));

    return (VOS_VOID *)pstSndMsg;
}


VOS_VOID* AT_PrivacyMatchPseucellInfoSetReq(
    MsgBlock                           *pstMsg
)
{
    MN_APP_REQ_MSG_STRU                *pstSndMsg             = VOS_NULL_PTR;
    AT_MTA_PSEUCELL_INFO_SET_REQ_STRU  *pstPseucellInfoSetReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

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

    TAF_MEM_CPY_S(pstSndMsg,
                  ulLength,
                  pstMsg,
                  ulLength);

    pstPseucellInfoSetReq = (AT_MTA_PSEUCELL_INFO_SET_REQ_STRU *)pstSndMsg->aucContent;

    /* 将敏感信息设置为全0 */
    pstPseucellInfoSetReq->ulLac = 0;

    return (VOS_VOID *)pstSndMsg;
}


