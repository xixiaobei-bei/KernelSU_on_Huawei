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

#ifndef _TAF_CCM_API_H_
#define _TAF_CCM_API_H_

#include "taf_type_def.h"
#include "mn_call_api.h"
#include "taf_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4200) /* zero-sized array in struct/union */
#endif

#pragma pack(push, 4)

#define ID_TAF_CCM_MSG_TYPE_BEGIN 0

/*
 * 结构说明: CCM消息接口枚举
 */
enum TAF_CCM_MsgType {
    /* CCM->AT */
    /* _MSGPARSE_Interception TAF_CCM_QryChannelInfoCnf */
    ID_TAF_CCM_QRY_CHANNEL_INFO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x01,
    /* 信道信息上报 */
    /* _MSGPARSE_Interception TAF_CCM_CallChannelInfoInd */
    ID_TAF_CCM_CALL_CHANNEL_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x03,
    /* _MSGPARSE_Interception TAF_CCM_CallModifyCnf */
    ID_TAF_CCM_CALL_MODIFY_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x05,
    /* _MSGPARSE_Interception TAF_CCM_CallAnswerRemoteModifyCnf */
    ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x07,
    /* _MSGPARSE_Interception TAF_CCM_CallModifyStatusInd */
    ID_TAF_CCM_CALL_MODIFY_STATUS_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x09,
    /* _MSGPARSE_Interception TAF_CCM_QryEconfCalledInfoCnf */
    ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0B,
    /* _MSGPARSE_Interception TAF_CCM_CallOrigCnf */    
    ID_TAF_CCM_CALL_ORIG_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0F,
    /* _MSGPARSE_Interception TAF_CCM_CallSupsCmdCnf */
    ID_TAF_CCM_CALL_SUPS_CMD_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x11,
    /* Call Related Command is Completed */
    /* _MSGPARSE_Interception TAF_CCM_CallSupsCmdRslt */
    ID_TAF_CCM_CALL_SUPS_CMD_RSLT_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x13,
    /* originate a MO Call */
    /* _MSGPARSE_Interception TAF_CCM_CallOrigInd */
    ID_TAF_CCM_CALL_ORIG_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x15,
    /* Call Connect */
    /* _MSGPARSE_Interception TAF_CCM_CallConnectInd */
    ID_TAF_CCM_CALL_CONNECT_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x17,
    /* incoming call */
    /* _MSGPARSE_Interception TAF_CCM_CallIncomingInd */
    ID_TAF_CCM_CALL_INCOMING_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x19,
    /* Call Released */
    /* _MSGPARSE_Interception TAF_CCM_CallReleasedInd */
    ID_TAF_CCM_CALL_RELEASED_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1B,
    /* Call All Released */
    /* _MSGPARSE_Interception TAF_CCM_CallAllReleasedInd */
    ID_TAF_CCM_CALL_ALL_RELEASED_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1D,
    /* 当前所有呼叫的信息 */
    /* _MSGPARSE_Interception TAF_CCM_QryCallInfoCnf */
    ID_TAF_CCM_QRY_CALL_INFO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1F,
    /* _MSGPARSE_Interception TAF_CCM_QryClprCnf */
    ID_TAF_CCM_QRY_CLPR_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x21,
    /* _MSGPARSE_Interception TAF_CCM_QryXlemaCnf */
    ID_TAF_CCM_QRY_XLEMA_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x23,
    /* _MSGPARSE_Interception TAF_CCM_SetCssnCnf */
    ID_TAF_CCM_SET_CSSN_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x27,
    /* Start DTMF tempooralily response */
    /* _MSGPARSE_Interception TAF_CCM_StartDtmfCnf */
    ID_TAF_CCM_START_DTMF_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x29,
    /* _MSGPARSE_Interception TAF_CCM_StartDtmfRslt */
    ID_TAF_CCM_START_DTMF_RSLT = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2B,
    /* Stop DTMF tempooralily response */
    /* _MSGPARSE_Interception TAF_CCM_StopDtmfCnf */
    ID_TAF_CCM_STOP_DTMF_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2D,
    /* _MSGPARSE_Interception TAF_CCM_StopDtmfRslt */
    ID_TAF_CCM_STOP_DTMF_RSLT = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2F,
    /* Call is Proceeding */
    /* _MSGPARSE_Interception TAF_CCM_CallProcInd */
    ID_TAF_CCM_CALL_PROC_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x31,
    /* Alerting,MO Call */
    /* _MSGPARSE_Interception TAF_CCM_CallAlertingInd */
    ID_TAF_CCM_CALL_ALERTING_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x33,
    /* Call Hold 呼叫保持 */
    /* _MSGPARSE_Interception TAF_CCM_CallHoldInd */
    ID_TAF_CCM_CALL_HOLD_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x35,
    /* Call Retrieve 呼叫恢复 */
    /* _MSGPARSE_Interception TAF_CCM_CallRetrieveInd */
    ID_TAF_CCM_CALL_RETRIEVE_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x37,
    /* SS Notify */
    /* _MSGPARSE_Interception TAF_CCM_CallSsInd */
    ID_TAF_CCM_CALL_SS_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x39,
    /* _MSGPARSE_Interception TAF_CCM_EccNumInd */
    ID_TAF_CCM_ECC_NUM_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3B,
    /* 通话时长 */
    /* _MSGPARSE_Interception TAF_CCM_GetCdurCnf */
    ID_TAF_CCM_GET_CDUR_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3D,
    /* 设置用户服务信令1信息 */
    /* _MSGPARSE_Interception TAF_CCM_SetUus1InfoCnf */
    ID_TAF_CCM_SET_UUS1_INFO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3F,
    /* 查询用户服务信令1信息 */
    /* _MSGPARSE_Interception TAF_CCM_QryUus1InfoCnf */
    ID_TAF_CCM_QRY_UUS1_INFO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x41,
    /* UUS1信息上报 */
    /* _MSGPARSE_Interception TAF_CCM_Uus1InfoInd */
    ID_TAF_CCM_UUS1_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x43,
    /* 设置当前线路号 */
    /* _MSGPARSE_Interception TAF_CCM_SetAlsCnf */
    ID_TAF_CCM_SET_ALS_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x45,
    /* 查询当前线路号 */
    /* _MSGPARSE_Interception TAF_CCM_QryAlsCnf */
    ID_TAF_CCM_QRY_ALS_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x47,
    /* _MSGPARSE_Interception TAF_CCM_CcwaiSetCnf */
    ID_TAF_CCM_CCWAI_SET_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x49,
    /* _MSGPARSE_Interception TAF_CCM_CnapInfoInd */
    ID_TAF_CCM_CNAP_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4B,
    /* _MSGPARSE_Interception TAF_CCM_CnapQryCnf */
    ID_TAF_CCM_CNAP_QRY_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4D,
    /* _MSGPARSE_Interception TAF_CCM_EconfDialCnf */
    ID_TAF_CCM_ECONF_DIAL_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4F,
    /* imsa通知SPM模块增强型多方通话参与者的状态发送变化  */
    /* _MSGPARSE_Interception TAF_CCM_EconfNotifyInd */
    ID_TAF_CCM_ECONF_NOTIFY_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x51,
    /* _MSGPARSE_Interception TAF_CCM_SendFlashCnf */
    ID_TAF_CCM_SEND_FLASH_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x53,
    /* _MSGPARSE_Interception TAF_CCM_SendBurstDtmfCnf */
    ID_TAF_CCM_SEND_BURST_DTMF_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x55,
    /* _MSGPARSE_Interception TAF_CCM_BurstDtmfInd */
    ID_TAF_CCM_BURST_DTMF_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x59,
    /* _MSGPARSE_Interception TAF_CCM_SendContDtmfCnf */
    ID_TAF_CCM_SEND_CONT_DTMF_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x5B,
    /* _MSGPARSE_Interception TAF_CCM_ContDtmfInd */
    ID_TAF_CCM_CONT_DTMF_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x5F,
    /* _MSGPARSE_Interception TAF_CCM_CcwacInfoInd */
    ID_TAF_CCM_CCWAC_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x61,
    /* _MSGPARSE_Interception TAF_CCM_CalledNumInfoInd */
    ID_TAF_CCM_CALLED_NUM_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x63,
    /* _MSGPARSE_Interception TAF_CCM_CallingNumInfoInd */
    ID_TAF_CCM_CALLING_NUM_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x65,
    /* _MSGPARSE_Interception TAF_CCM_DisplayInfoInd */
    ID_TAF_CCM_DISPLAY_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x67,
    /* _MSGPARSE_Interception TAF_CCM_ExtDisplayInfoInd */
    ID_TAF_CCM_EXT_DISPLAY_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x69,
    /* _MSGPARSE_Interception TAF_CCM_ConnNumInfoInd */
    ID_TAF_CCM_CONN_NUM_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6B,
    /* _MSGPARSE_Interception TAF_CCM_RedirNumInfoInd */
    ID_TAF_CCM_REDIR_NUM_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6D,
    /* _MSGPARSE_Interception TAF_CCM_SignalInfoInd */
    ID_TAF_CCM_SIGNAL_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6F,
    /* _MSGPARSE_Interception TAF_CCM_LineCtrlInfoInd */
    ID_TAF_CCM_LINE_CTRL_INFO_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x71,
    /* _MSGPARSE_Interception TAF_CCM_WaitingInd */
    ID_TAF_CCM_CALL_WAITING_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x73,
    /* _MSGPARSE_Interception TAF_CCM_EncryptVoiceCnf */
    ID_TAF_CCM_ENCRYPT_VOICE_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x75,
    /* _MSGPARSE_Interception TAF_CCM_EncryptVoiceInd */
    ID_TAF_CCM_ENCRYPT_VOICE_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x77,
    /* _MSGPARSE_Interception TAF_CCM_EcRemoteCtrlInd */
    ID_TAF_CCM_EC_REMOTE_CTRL_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x79,
    /* _MSGPARSE_Interception TAF_CCM_RemoteCtrlAnswerCnf */
    ID_TAF_CCM_REMOTE_CTRL_ANSWER_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7B,
    /* _MSGPARSE_Interception TAF_CCM_EccSrvCapCfgCnf */
    ID_TAF_CCM_ECC_SRV_CAP_CFG_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7D,
    /* _MSGPARSE_Interception TAF_CCM_EccSrvCapQryCnf */
    ID_TAF_CCM_ECC_SRV_CAP_QRY_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7F,
    /* _MSGPARSE_Interception TAF_CCM_SetEcTestModeCnf */
    ID_TAF_CCM_SET_EC_TEST_MODE_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x81,
    /* _MSGPARSE_Interception TAF_CCM_GetEcTestModeCnf */
    ID_TAF_CCM_GET_EC_TEST_MODE_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x83,
    /* _MSGPARSE_Interception TAF_CCM_GetEcRandomCnf */
    ID_TAF_CCM_GET_EC_RANDOM_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x85,
    /* _MSGPARSE_Interception TAF_CCM_GetEcKmcCnf */
    ID_TAF_CCM_GET_EC_KMC_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x87,
    /* _MSGPARSE_Interception TAF_CCM_SetEcKmcCnf */
    ID_TAF_CCM_SET_EC_KMC_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x89,
    /* _MSGPARSE_Interception TAF_CCM_EncryptedVoiceDataInd */
    ID_TAF_CCM_ENCRYPTED_VOICE_DATA_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8B,
    /* _MSGPARSE_Interception TAF_CCM_PrivacyModeSetCnf */
    ID_TAF_CCM_PRIVACY_MODE_SET_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8D,
    /* _MSGPARSE_Interception TAF_CCM_PrivacyModeQryCnf */
    ID_TAF_CCM_PRIVACY_MODE_QRY_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8F,
    /* _MSGPARSE_Interception TAF_CCM_PrivacyModeInd */
    ID_TAF_CCM_PRIVACY_MODE_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x91,

    /* 如下3条消息，AT并无逻辑处理，可以废弃 */
    /* Call Related Command is in progress */
    /* _MSGPARSE_Interception TAF_CCM_SS_CMD_PROGRESS_IND_STRU */
    /* ERR->ERR */
    ID_TAF_CCM_SS_CMD_PROGRESS_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x93,
    /* 可以激活CCBS */
    /* _MSGPARSE_Interception TAF_CCM_CCBS_POSSIBLE_IND_STRU */
    ID_TAF_CCM_CCBS_POSSIBLE_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x95,
    /* 超时 */
    /* _MSGPARSE_Interception TAF_CCM_CCBS_TIME_EXPIRED_IND_STRU */
    ID_TAF_CCM_CCBS_TIME_EXPIRED_IND = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x97,
    /* CCM->AT */
    /* _MSGPARSE_Interception TAF_CCM_CancelAddVideoCnf */
    ID_TAF_CCM_CANCEL_ADD_VIDEO_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x99,
    /* _MSGPARSE_Interception TAF_CCM_CustomEccNumCnf */
    ID_TAF_CCM_CUSTOM_ECC_NUM_CNF = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x9B,

    /* TAF->CCM, A_TAF->CCM */
    /* CSCHANNELINFO查询命令 */
    /* _MSGPARSE_Interception TAF_CCM_QryChannelInfoReq */
    ID_TAF_CCM_QRY_CHANNEL_INFO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x00,
    /* 变更音视频类型 */
    /* _MSGPARSE_Interception TAF_CCM_CallModifyReq */
    ID_TAF_CCM_CALL_MODIFY_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x02,
    /* 应答对方变更音视频类型 */
    /* _MSGPARSE_Interception TAF_CCM_CallAnswerRemoteModifyReq */
    ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x04,
    /* 获取与会者信息 */
    /* _MSGPARSE_Interception TAF_CCM_QryEconfCalledInfoReq */
    ID_TAF_CCM_QRY_ECONF_CALLED_INFO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x06,
    /* 发起呼叫 */
    /* _MSGPARSE_Interception TAF_CCM_CallOrigReq */
    ID_TAF_CCM_CALL_ORIG_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x08,
    /* 呼叫结束消息 */
    /* _MSGPARSE_Interception TAF_CCM_CallEndReq */
    ID_TAF_CCM_CALL_END_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0A,
    /* 呼叫相关补充业务 */
    /* _MSGPARSE_Interception TAF_CCM_CallSupsCmdReq */
    ID_TAF_CCM_CALL_SUPS_CMD_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0C,
    /* 增强型通话发起 */
    /* _MSGPARSE_Interception TAF_CCM_EconfDialReq */
    ID_TAF_CCM_ECONF_DIAL_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0E,
    /* TAF->CCM */
    /* 增强型通话发起 */
    /* _MSGPARSE_Interception TAF_CCM_CustomDialReq */
    ID_TAF_CCM_CUSTOM_DIAL_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x10,
    /* TAF->CCM, A_TAF->CCM */
    /* 获取呼叫信息 */
    /* _MSGPARSE_Interception TAF_CCM_QryCallInfoReq */
    ID_TAF_CCM_QRY_CALL_INFO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x12,
    /* 查询呼叫源号码 */
    /* _MSGPARSE_Interception TAF_CCM_QryClprReq */
    ID_TAF_CCM_QRY_CLPR_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x14,
    /* 需要放在CCM处理吗，感觉底层逻辑有点儿复杂啊 */
    /* 紧急呼号码查询 */
    /* _MSGPARSE_Interception TAF_CCM_QryXlemaReq */
    ID_TAF_CCM_QRY_XLEMA_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x16,
    /* 使能或禁止补充业务的主动上报 */
    /* _MSGPARSE_Interception TAF_CCM_SetCssnReq */
    ID_TAF_CCM_SET_CSSN_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1A,
    /* 定制紧急呼号码 */
    /* _MSGPARSE_Interception TAF_CCM_CustomEccNumReq */
    ID_TAF_CCM_CUSTOM_ECC_NUM_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1C,
    /* 开始发送DTMF */
    /* _MSGPARSE_Interception TAF_CCM_StartDtmfReq */
    ID_TAF_CCM_START_DTMF_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1E,
    /* 停止发送DTMF */
    /* _MSGPARSE_Interception TAF_CCM_StopDtmfReq */
    ID_TAF_CCM_STOP_DTMF_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x20,
    /* 获取通话时长 */
    /* _MSGPARSE_Interception TAF_CCM_GetCdurReq */
    ID_TAF_CCM_GET_CDUR_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x22,
    /* CUUS1命令，只有GU CALL模块处理 */
    /* _MSGPARSE_Interception TAF_CCM_SetUusinfoReq */
    ID_TAF_CCM_SET_UUSINFO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x24,
    /* _MSGPARSE_Interception TAF_CCM_QryUusinfoReq */
    ID_TAF_CCM_QRY_UUSINFO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x26,
    /* _MSGPARSE_Interception TAF_CCM_QryAlsReq */
    ID_TAF_CCM_QRY_ALS_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x28,
    /* 多线路选择：ALS命令 */
    /* _MSGPARSE_Interception TAF_CCM_SetAlsReq */
    ID_TAF_CCM_SET_ALS_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2A,
    /* 设置IMS呼叫等待 */
    /* _MSGPARSE_Interception TAF_CCM_CcwaiSetReq */
    ID_TAF_CCM_CCWAI_SET_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2C,
    /* _MSGPARSE_Interception TAF_CCM_CnapQryReq */
    ID_TAF_CCM_CNAP_QRY_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2E,
    /* X模消息 */
    /* _MSGPARSE_Interception TAF_CCM_SendFlashReq */
    ID_TAF_CCM_SEND_FLASH_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x30,
    /* X模消息 */
    /* _MSGPARSE_Interception TAF_CCM_SendBurstDtmfReq */
    ID_TAF_CCM_SEND_BURST_DTMF_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x32,
    /* X模消息 */
    /* _MSGPARSE_Interception TAF_CCM_SendContDtmfReq */
    ID_TAF_CCM_SEND_CONT_DTMF_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x34,
    /* _MSGPARSE_Interception TAF_CCM_EncryptVoiceReq */
    ID_TAF_CCM_ENCRYPT_VOICE_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x36,
    /* _MSGPARSE_Interception TAF_CCM_RemoteCtrlAnswerReq */
    ID_TAF_CCM_REMOTE_CTRL_ANSWER_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x38,
    /* _MSGPARSE_Interception TAF_CCM_EccSrvCapQryReq */
    ID_TAF_CCM_ECC_SRV_CAP_QRY_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3A,
    /* _MSGPARSE_Interception TAF_CCM_EccSrvCapCfgReq */
    ID_TAF_CCM_ECC_SRV_CAP_CFG_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3C,
    /* _MSGPARSE_Interception TAF_CCM_SetEcTestModeReq */
    ID_TAF_CCM_SET_EC_TEST_MODE_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3E,
    /* _MSGPARSE_Interception TAF_CCM_GetEcRandomReq */
    ID_TAF_CCM_GET_EC_RANDOM_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x40,
    /* _MSGPARSE_Interception TAF_CCM_GetEcTestModeReq */
    ID_TAF_CCM_GET_EC_TEST_MODE_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x42,
    /* _MSGPARSE_Interception TAF_CCM_GetEcKmcReq */
    ID_TAF_CCM_GET_EC_KMC_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x44,
    /* _MSGPARSE_Interception TAF_CCM_SetEcKmcReq */
    ID_TAF_CCM_SET_EC_KMC_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x46,
    /* _MSGPARSE_Interception TAF_CCM_PrivacyModeSetReq */
    ID_TAF_CCM_PRIVACY_MODE_SET_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x48,
    /* _MSGPARSE_Interception TAF_CCM_PrivacyModeQryReq */
    ID_TAF_CCM_PRIVACY_MODE_QRY_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4A,
    /* 设置eCall优选域 */
    /* _MSGPARSE_Interception TAF_CCM_SetEcallDomainModeReq */
    ID_TAF_CCM_SET_ECALL_DOMAIN_MODE_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4C,
    /* _MSGPARSE_Interception TAF_CCM_CancelAddVideoReq */
    ID_TAF_CCM_CANCEL_ADD_VIDEO_REQ = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4E,
    /* BUTT->BUTT */
    /* _MSGPARSE_Interception TAF_CCM_CancelAddVideoReq */
    ID_TAF_CCM_MSG_TYPE_BUTT = ID_TAF_CCM_MSG_TYPE_BEGIN + 0xFF,
};
typedef VOS_UINT32 TAF_CCM_MsgTypeUint32;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;     /* 消息名 */
    TAF_Ctrl   ctrl;        /* 控制信息 */
    VOS_UINT8  content[0];  //lint !e43
} TAF_CCM_Msg;

typedef struct {
    TAF_ERROR_CodeUint32 result;
} TAF_CCM_CommonSetCnf;

typedef struct {
    TAF_CS_CauseUint32 cause;
} TAF_CCM_CsCommonSetCnf;

/*
 * 枚举名: TAF_CCM_AppReqMsg
 * 结构说明: 来自APP的请求消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;     /* 消息名 */
    TAF_Ctrl   ctrl;        /* 控制信息 */
    VOS_UINT8  content[0];  //lint !e43
} TAF_CCM_AppReqMsg;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_QryChannelInfoReq;


typedef struct {
    VOS_UINT32                result;      /* 查询结果 */
    TAF_CALL_ChannelTypeUint8 channelType; /* codec type */
    TAF_CALL_VoiceDomainUint8 voiceDomain; /* VOICE DOMAIN */
    VOS_UINT8                 reserved[2];
} TAF_CCM_QryChannelInfoPara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                 msgName;
    TAF_Ctrl                   ctrl;
    TAF_CCM_QryChannelInfoPara para;
} TAF_CCM_QryChannelInfoCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    /* 是否为本地播放回铃音标识1:本地播放回铃音；0:网络放音 */
    VOS_UINT8                 isLocalAlertingFlag;
    MN_CALL_CodecTypeUint8    codecType;   /* codec type */
    TAF_CALL_VoiceDomainUint8 voiceDomain; /* VOICE DOMAIN */
    VOS_UINT8                 rsv[1];
} TAF_CCM_CallChannelInfoInd;


typedef struct {
    MN_CALL_ID_T      callId;         /* Call ID */
    MN_CALL_TypeUint8 currCallType;   /* 当前呼叫类型 */
    MN_CALL_TypeUint8 expectCallType; /* 期望呼叫类型 */
    VOS_UINT8         reserved;
} TAF_CCM_CallModifyPara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32             msgName;
    TAF_Ctrl               ctrl;
    TAF_CCM_CallModifyPara para;
} TAF_CCM_CallModifyReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    MN_CALL_ID_T       callId; /* Call ID */
    VOS_UINT8          reserved[3];
    TAF_CS_CauseUint32 cause; /* 错误码 */
} TAF_CCM_CallModifyCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                msgName;
    TAF_Ctrl                  ctrl;
    MN_CALL_ID_T              callId;         /* Call ID */
    MN_CALL_ModifyStatusUint8 modifyStatus;   /* 当前的MODIFY的过程状态 */
    TAF_CALL_VoiceDomainUint8 voiceDomain;    /* VOICE DOMAIN，这里始终是IMS域 */
    MN_CALL_TypeUint8         currCallType;   /* 当前呼叫类型 */
    MN_CALL_TypeUint8         expectCallType; /* 期望呼叫类型 */
    /* 远程用户发起的modify原因，仅在MODIFY_PROC_IND时才使用 */
    TAF_CALL_ModifyReasonUint8 modifyReason;
    VOS_UINT8                  reserved[2];
    /* 错误码 */
    TAF_CS_CauseUint32 cause; /* 错误码，仅在MODIFY_PROC_END有异常时才使用 */
} TAF_CCM_CallModifyStatusInd;


typedef struct {
    MN_CALL_ID_T      callId;         /* Call ID */
    MN_CALL_TypeUint8 currCallType;   /* 当前呼叫类型 */
    MN_CALL_TypeUint8 expectCallType; /* 期望呼叫类型 */
    VOS_UINT8         reserved;
} TAF_CCM_CallAnswerRemoteModifyPara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                         msgName;
    TAF_Ctrl                           ctrl;
    TAF_CCM_CallAnswerRemoteModifyPara para;
} TAF_CCM_CallAnswerRemoteModifyReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    MN_CALL_ID_T       callId; /* Call ID */
    VOS_UINT8          reserved[3];
    TAF_CS_CauseUint32 cause; /* 错误码 */
} TAF_CCM_CallAnswerRemoteModifyCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_QryEconfCalledInfoReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32              msgName;
    TAF_Ctrl                ctrl;
    VOS_UINT8               numOfMaxCalls;
    VOS_UINT8               numOfCalls; /* 所有正在通话的个数 */
    VOS_UINT8               reserved[2];
    TAF_CALL_EconfInfoParam callInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];
} TAF_CCM_QryEconfCalledInfoCnf;


typedef struct {
    MN_CALL_ID_T      callId;         /* Call ID */
    MN_CALL_TypeUint8 currCallType;   /* 当前呼叫类型 */
    MN_CALL_TypeUint8 expectCallType; /* 期望呼叫类型 */
    VOS_UINT8         reserved;
} TAF_CCM_RemoteCtrlAnswerPara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CCM_RemoteCtrlAnswerPara para;
} TAF_CCM_CallAnswerRemoteReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    MN_CALL_ID_T       callId; /* Call ID */
    VOS_UINT8          reserved[3];
    TAF_CS_CauseUint32 cause; /* 错误码 */
} TAF_CCM_CallAnswerRemoteCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32        msgName;
    TAF_Ctrl          ctrl;
    MN_CALL_OrigParam orig;
} TAF_CCM_CallOrigReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32        msgName;
    TAF_Ctrl          ctrl;
    MN_CALL_SupsParam callMgmtCmd;
} TAF_CCM_CallSupsCmdReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_SupsCmdPara supsCmdPara;
} TAF_CCM_CallSupsCmdCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32             msgName;
    TAF_Ctrl               ctrl;
    TAF_CALL_EconfDialInfo econfDialInfo;
} TAF_CCM_EconfDialReq;

typedef struct {
    MN_CALL_ID_T     callId;
    VOS_UINT8        reservd[3];
    MN_CALL_EndParam endParam; /* End Cause */
} TAF_CCM_CallEndInfo;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    VOS_UINT32          msgName; /* _H2ASN_Skip */
    TAF_Ctrl            ctrl;
    TAF_CCM_CallEndInfo endInfo;
} TAF_CCM_CallEndReq;


typedef struct {
    VOS_MSG_HEADER                   /* _H2ASN_Skip */
    VOS_UINT32              msgName; /* _H2ASN_Skip */
    TAF_Ctrl                ctrl;
    TAF_CALL_CustomDialPara customDialPara;
} TAF_CCM_CustomDialReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_QryCallInfoReqPara qryCallInfoPara;
} TAF_CCM_QryCallInfoReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_QryCallInfoCnfPara qryCallInfoPara;
} TAF_CCM_QryCallInfoCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                    msgName;
    TAF_Ctrl                      ctrl;
    TAFAGENT_CALL_QryCallInfoPara tafAgentCallInfo;
} TAFAGENT_CCM_QryCallInfoCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_QryClprPara qryClprPara;
} TAF_CCM_QryClprReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    MN_CALL_ClprGetCnf clprCnf;
} TAF_CCM_QryClprCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_QryXlemaReq;

typedef MN_CALL_EccNumInfo TAF_CALL_QRY_XLEMA_PARA_STRU;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CALL_QRY_XLEMA_PARA_STRU qryXlemaPara;
} TAF_CCM_QryXlemaCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                      msgName;
    TAF_Ctrl                        ctrl;
    TAF_CALL_EcallDomainPreferUint8 ecallDomainMode;
    VOS_UINT8                       reserved[3];
} TAF_CCM_SetEcallDomainModeReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_ERROR_CodeUint32 ret; /* 回复结果 */
} TAF_CCM_SetEcallDomainModeCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                 msgName;
    TAF_Ctrl                   ctrl;
    MN_CALL_AppCustomEccNumReq eccNumReq;
} TAF_CCM_CustomEccNumReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    MN_CALL_SetCssnReq cssnReq;
} TAF_CCM_SetCssnReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_ERROR_CodeUint32 ret; /* 回复结果 */
} TAF_CCM_CommSetCnf;

typedef TAF_CCM_CommSetCnf TAF_CCM_SetCssnCnf;

typedef TAF_CCM_CommSetCnf TAF_CCM_CustomEccNumCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_OrigCnfPara origCnfPara;
} TAF_CCM_CallOrigCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_SupsCmdRsltPara supsCmdRsltPara;
} TAF_CCM_CallSupsCmdRslt;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_OrigIndPara origIndPara;
} TAF_CCM_CallOrigInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_ProcIndPara procIndPata;
} TAF_CCM_CallProcInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_AlertingIndPara alertingIndPara;
} TAF_CCM_CallAlertingInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32              msgName;
    TAF_Ctrl                ctrl;
    TAF_CALL_ConnectIndPara connectIndPara;
} TAF_CCM_CallConnectInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_ReleasedIndPara releaseIndPara;
} TAF_CCM_CallReleasedInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_IncomingIndPara incomingIndPara;
} TAF_CCM_CallIncomingInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_CallAllReleasedInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmf;
} TAF_CCM_StartDtmfReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32             msgName;
    TAF_Ctrl               ctrl;
    TAF_CALL_StartDtmfPara startDtmfPara;
} TAF_CCM_StartDtmfCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                 msgName;
    TAF_Ctrl                   ctrl;
    TAF_CALL_StartDtmfRsltPara startDtmfRslt;
} TAF_CCM_StartDtmfRslt;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmf;
} TAF_CCM_StopDtmfReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32            msgName;
    TAF_Ctrl              ctrl;
    TAF_CALL_StopDtmfPara stopDtmfPara;
} TAF_CCM_StopDtmfCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                msgName;
    TAF_Ctrl                  ctrl;
    TAF_CALL_StopDtmfRsltPara stopDtmfRsltPara;
} TAF_CCM_StopDtmfRslt;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_HoldIndPara holdIndPara;
} TAF_CCM_CallHoldInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_RetrieveInd retrieveInd;
} TAF_CCM_CallRetrieveInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    TAF_CALL_SsIndPara ssIndPara;
} TAF_CCM_CallSsInd;

typedef MN_CALL_EccNumInfo TAF_CALL_ECC_NUM_IND_PARA_STRU;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                     msgName;
    TAF_Ctrl                       ctrl;
    TAF_CALL_ECC_NUM_IND_PARA_STRU eccNumInd;
} TAF_CCM_EccNumInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32        msgName;
    TAF_Ctrl          ctrl;
    MN_CALL_Uus1Param uus1Info; /* 设置UUS1信息结构 */
} TAF_CCM_SetUusinfoReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT32 ret; /* 回复结果 */
} TAF_CCM_SetUus1InfoCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_QryUusinfoReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_QryUus1InfoPara qryUss1Info;
} TAF_CCM_QryUus1InfoCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_Uus1InfoIndPara uus1InfoIndPara;
} TAF_CCM_Uus1InfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName;
    TAF_Ctrl            ctrl;
    MN_CALL_SetAlsParam setAls; /* 设置ALS NO结构   */
} TAF_CCM_SetAlsReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT32 ret; /* 回复结果 */
} TAF_CCM_SetAlsCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_QryAlsReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName;
    TAF_Ctrl            ctrl;
    TAF_CALL_QryAlsPara qryAlsPara;
} TAF_CCM_QryAlsCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_CcwaiSetReq ccwaiSet;
} TAF_CCM_CcwaiSetReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT32 result;
} TAF_CCM_CcwaiSetCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32    msgName;
    TAF_Ctrl      ctrl;
    TAF_CALL_Cnap nameIndicator;
} TAF_CCM_CnapInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_CnapQryReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32    msgName;
    TAF_Ctrl      ctrl;
    TAF_CALL_Cnap nameIndicator;
} TAF_CCM_CnapQryCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32             msgName;
    TAF_Ctrl               ctrl;
    TAF_CALL_EconfDialPara econfDialCnf;
} TAF_CCM_EconfDialCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_EconfNotifyIndPara econfNotifyIndPara;
} TAF_CCM_EconfNotifyInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgName;
    TAF_Ctrl           ctrl;
    TAF_CALL_FlashPara flashPara;
} TAF_CCM_SendFlashReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32            msgName;
    TAF_Ctrl              ctrl;
    TAF_CALL_FlashCnfPara result;
} TAF_CCM_SendFlashCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32             msgName;
    TAF_Ctrl               ctrl;
    TAF_CALL_BurstDtmfPara burstDTMFPara;
} TAF_CCM_SendBurstDtmfReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                msgName;
    TAF_Ctrl                  ctrl;
    TAF_CALL_BurstDtmfCnfPara burstDtmfCnfPara;
} TAF_CCM_SendBurstDtmfCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT8  result;
    VOS_UINT8  reserved[3];
} TAF_CCM_SendBurstDtmfRslt;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    /* *<  refers to S.0005 3.7.3.3.2.9 */
    TAF_CALL_BurstDtmfIndPara burstDtmfIndPara;
} TAF_CCM_BurstDtmfInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32            msgName;
    TAF_Ctrl              ctrl;
    TAF_CALL_ContDtmfPara contDTMFPara;
} TAF_CCM_SendContDtmfReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_ContDtmfCnfPara contDtmfCnfPara;
} TAF_CCM_SendContDtmfCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT8  result;
    VOS_UINT8  reserved[3];
} TAF_CCM_SendContDtmfRslt;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CTRL_ContDtmfIndPara contDtmfIndPara;
} TAF_CCM_ContDtmfInd;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                msgName;
    TAF_Ctrl                  ctrl;
    TAF_CALL_CcwacInfoIndPara ccwacInfoPara;
} TAF_CCM_CcwacInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                    msgName;
    TAF_Ctrl                      ctrl;
    TAF_CALL_CalledNumInfoIndPara calledNumInfoPara;
} TAF_CCM_CalledNumInfoInd;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                     msgName;
    TAF_Ctrl                       ctrl;
    TAF_CALL_CallingNumInfoIndPara callIngNumInfoPara;
} TAF_CCM_CallingNumInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_DisplayInfoIndPara disPlayInfoIndPara;
} TAF_CCM_DisplayInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                     msgName;
    TAF_Ctrl                       ctrl;
    TAF_CALL_ExtDisplayInfoIndPara disPlayInfoIndPara;
} TAF_CCM_ExtDisplayInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_ConnNumInfoIndPara connNumInfoIndPara;
} TAF_CCM_ConnNumInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_REDIR_NumInfoIndPara redirNumInfoIndPara;
} TAF_CCM_RedirNumInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                 msgName;
    TAF_Ctrl                   ctrl;
    TAF_CALL_SignalInfoIndPara signalInfoIndPara;
} TAF_CCM_SignalInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CALL_LineCtrlInfoIndPara lineCtrlInfoIndPara;
} TAF_CCM_LineCtrlInfoInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT8  callId;
    VOS_UINT8  callWaitingInd;
    VOS_UINT8  rsved[2];
} TAF_CCM_WaitingInd;


typedef struct {
    TAF_CALL_EncryptVoiceTypeUint32 eccVoiceType;
    TAF_ECC_CallBcdNum              dialNumber; /* Call Number */
} TAF_CCM_EncryptVoicePara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CCM_EncryptVoicePara encrypVoicePara;
} TAF_CCM_EncryptVoiceReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                        msgName;
    TAF_Ctrl                          ctrl;
    TAF_CALL_EncryptVoiceStatusUint32 eccVoiceStatus;
} TAF_CCM_EncryptVoiceCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CALL_EncryptVoiceIndPara encryptVoiceIndPara;
} TAF_CCM_EncryptVoiceInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                    msgName;
    TAF_Ctrl                      ctrl;
    TAF_CALL_RemoteCtrlTypeUint32 remoteCtrlType;
} TAF_CCM_EcRemoteCtrlInd;


typedef struct {
    TAF_CALL_RemoteCtrlTypeUint32   remoteCtrlEvtType;
    TAF_CALL_RemoteCtrlResultUint32 result;
} TAF_CCM_RemoteCtrlAnswerInfo;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CCM_RemoteCtrlAnswerInfo remoteCtrlAnswerInfo;
} TAF_CCM_RemoteCtrlAnswerReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                          msgName;
    TAF_Ctrl                            ctrl;
    TAF_CALL_RemoteCtrlOperResultUint32 result; /* 短信发送结果 */
} TAF_CCM_RemoteCtrlAnswerCnf;


typedef struct {
    TAF_CALL_EccSrvCapUint32    eccSrvCap;
    TAF_CALL_EccSrvStatusUint32 eccSrvStatus;
} TAF_CCM_EccSrvCapCfgPara;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CCM_EccSrvCapCfgPara capCfgPara;
} TAF_CCM_EccSrvCapCfgReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                        msgName;
    TAF_Ctrl                          ctrl;
    TAF_CALL_EccSrvCapCfgResultUint32 result;
} TAF_CCM_EccSrvCapCfgCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_EccSrvCapQryReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CALL_EccSrvCapQryCnfPara eccSrvCapQryCnfPara;
} TAF_CCM_EccSrvCapQryCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                   msgName;
    TAF_Ctrl                     ctrl;
    TAF_CALL_SetEcTestModeUint32 eccTestModeStatus;
} TAF_CCM_SetEcTestModeReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT32 result;
} TAF_CCM_SetEcTestModeCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_GetEcTestModeReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                    msgName;
    TAF_Ctrl                      ctrl;
    TAF_CALL_GetEcTestModeCnfPara testModeCnfPara;
} TAF_CCM_GetEcTestModeCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_GetEcRandomReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_GetEcRandomCnfPara ecRandomData;
} TAF_CCM_GetEcRandomCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_GetEcKmcReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32               msgName;
    TAF_Ctrl                 ctrl;
    TAF_CALL_GetEcKmcCnfPara kmcCnfPara;
} TAF_CCM_GetEcKmcCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    MN_CALL_AppEcKmcData kmcData;
} TAF_CCM_SetEcKmcReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
    VOS_UINT32 result;
} TAF_CCM_SetEcKmcCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                         msgName;
    TAF_Ctrl                           ctrl;
    TAF_CALL_EncryptedVoiceDataIndPara encVoiceDataIndPara;
} TAF_CCM_EncryptedVoiceDataInd;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_PrivacyModeSetPara privacyMode;
} TAF_CCM_PrivacyModeSetReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                msgName;
    TAF_Ctrl                  ctrl;
    TAF_CALL_ResultTypeUint32 result;
} TAF_CCM_PrivacyModeSetCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    TAF_Ctrl   ctrl;
} TAF_CCM_PrivacyModeQryReq;


typedef struct {
    VOS_UINT8                 callId;
    TAF_CALL_PrivacyModeUint8 privacyMode;
    VOS_UINT8                 reserved[2];
} TAF_CCM_PrivacyModeInfo;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                     msgName;
    TAF_Ctrl                       ctrl;
    TAF_CALL_PrivacyModeQryCnfPara privacyModeQryCnfPara;
} TAF_CCM_PrivacyModeQryCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    TAF_Ctrl                    ctrl;
    TAF_CALL_PrivacyModeIndPara privacyModeIndPara;
} TAF_CCM_PrivacyModeInd;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_CcwaiSetReq ccwaiSet;
} TAF_CCM_SetCcwaiReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_GetCdurPara getCdurPara;
} TAF_CCM_GetCdurReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32           msgName;
    TAF_Ctrl             ctrl;
    TAF_CALL_GetCdurPara getCdurPara;
} TAF_CCM_GetCdurCnf;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32     msgName;
    TAF_Ctrl       ctrl;
    VOS_UINT32     callId;
} TAF_CCM_CancelAddVideoReq;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32     msgName;
    TAF_Ctrl       ctrl;
    VOS_UINT32     result; /* 成功返回VOS_OK，失败返回VOS_ERR */
} TAF_CCM_CancelAddVideoCnf;

VOS_UINT32 TAF_CCM_CallCommonReq(TAF_Ctrl *ctrl, const VOS_VOID *para, VOS_UINT32 msgType, VOS_UINT32 paraLen,
                                 ModemIdUint16 modemId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of taf_ccm_api.h */
