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

#ifndef __TAFLOGPRIVACYMATCH_H__
#define __TAFLOGPRIVACYMATCH_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include "vos.h"
#include "PsTypeDef.h"
#include "TafSsaApi.h"
#include "AtInternalMsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum TAF_LOG_PRIVACY_AT_CMD_ENUM
{
    TAF_LOG_PRIVACY_AT_CMD_BUTT                     = 0,

    TAF_LOG_PRIVACY_AT_CMD_CIMI                     = 1,
    TAF_LOG_PRIVACY_AT_CMD_CGSN                     = 2,
    TAF_LOG_PRIVACY_AT_CMD_MSID                     = 3,
    TAF_LOG_PRIVACY_AT_CMD_HPLMN                    = 4,
};
typedef VOS_UINT32 TAF_LOG_PRIVACY_AT_CMD_ENUM_UINT32;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef VOS_VOID* (*pTafLogPrivacyMatchFunc)(
    MsgBlock                           *pstMsg
);


typedef struct
{
    VOS_UINT32                          ulMsgName;                          /* 消息类型 */
    pTafLogPrivacyMatchFunc             pFuncPrivacyMatch;                  /* 消息对应的过滤函数 */
}TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU;


typedef struct
{   VOS_UINT32                          ulReceiverPid;                      /* 消息接收PID */
    VOS_UINT32                          ulTblSize;                          /* 消息过滤函数匹配处理表大小 */
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU *pstLogPrivacyMatchMsgTbl;           /* 消息过滤函数匹配处理表 */
}TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU;


typedef struct
{
    VOS_UINT32                           ulModem0Pid;
    VOS_UINT32                           ulModem1Pid;
    VOS_UINT32                           ulModem2Pid;
}TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU;


typedef struct
{
    VOS_CHAR                           *pcOriginalAtCmd;
    VOS_CHAR                           *pcPrivacyAtCmd;
}AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU;

typedef AT_MSG_STRU* (*AT_PRIVACY_FILTER_AT_PROC_FUNC)(
    AT_MSG_STRU                        *pstAtMsg
);


typedef struct
{
    VOS_UINT32                          ulAtCmdType;
    AT_PRIVACY_FILTER_AT_PROC_FUNC      pcPrivacyAtCmd;
}AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_UINT32 TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(
    TAF_SSA_EVT_ID_ENUM_UINT32          enEvtId
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* AT_PrivacyMatchCallAppSendCustomDialReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchAppMsgTypeSendReq(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID*  AT_PrivacyMatchCposSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID*  AT_PrivacyMatchSimLockWriteExSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID*  AT_PrivacyMatchMeidSetReq(
    MsgBlock                           *pstMsg
);
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* AT_PrivacyMatchAppMsgTypeWriteReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchAppMsgTypeDeleteReq(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID* TAF_PrivacyMatchAppMnCallBackCsCall(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_IMS)
VOS_VOID* AT_PrivacyMatchCallAppEconfDialReq(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID* AT_PrivacyMatchRegisterSsMsg(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchProcessUssMsg(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchInterRogateMsg(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchErasessMsg(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchCallAppOrigReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppSupsCmdReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppStartDtmfReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppStopDtmfReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppCustomEccNumReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchMnCallBackSsAtIndEvt(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchAppMnCallBackSs(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_CALL_PrivacyMatchAppCnapQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_CALL_PrivacyMatchAppCnapInfoInd(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(
    MsgBlock                           *pstMsg
);

VOS_VOID*  TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppSendFlashReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppSendBurstReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCallAppSendContReq(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_AGPS)
VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsEphInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtApReverseDataInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsPrmInfoRsp(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(
    MsgBlock                           *pstMsg
);
#endif


#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)
VOS_VOID* TAF_PrivacyMatchCallAppEncryptVoiceReq(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)
VOS_VOID* TAF_PrivacyMatchCallAppSetEcKmcReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XCALL_PrivacyMatchAppGetEcRandomCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XCALL_PrivacyMatchAppGetEcKmcCnf(
    MsgBlock                           *pstMsg
);
#endif
#endif
#endif

VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtMeidQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtCgsnQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchAtCallBackQryProc(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtUsimStatusInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtHomePlmnQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf(
    MsgBlock                           *pstMsg
);


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TafLogPrivacyMatch.h */
