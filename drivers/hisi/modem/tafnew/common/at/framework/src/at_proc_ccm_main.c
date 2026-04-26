/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 */

#include "vos.h"
#include "securec.h"

#include "at_external_module_msg_proc.h"

#include "at_cdma_event_report.h"
#include "at_cdma_rslt_proc.h"
#include "at_event_report.h"
#include "at_phone_event_report.h"
#include "at_phone_taf_rslt_proc.h"
#include "at_ss_event_report.h"
#include "at_ss_taf_rslt_proc.h"
#include "at_voice_event_report.h"
#include "at_voice_taf_rslt_proc.h"
#include "at_cmd_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PROC_CCM_MAIN_C

STATIC VOS_UINT32 AT_RcvTafCcmCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_Msg *ccmMsg  = msg;
    TAF_CCM_CommonSetCnf *event = (TAF_CCM_CommonSetCnf *)(ccmMsg->content);

    g_atSendDataBuff.bufLen = 0;

    if (event->result != TAF_ERR_NO_ERROR) {
        return AT_CME_INCORRECT_PARAMETERS;
    } else {
        return AT_OK;
    }
}

STATIC VOS_UINT32 AT_RcvTafCcmCsCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_Msg *ccmMsg  = msg;
    TAF_CCM_CsCommonSetCnf *event = (TAF_CCM_CsCommonSetCnf *)(ccmMsg->content);

    g_atSendDataBuff.bufLen = 0;

    if (event->cause != TAF_CS_CAUSE_SUCCESS) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}

STATIC AT_MsgProcEntry g_atProcCcmMsgTbl[] = {
    /* ！！！！注意！！！！
     * 以下为使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，
     * 主要原因：原消息处理函数中没有针对AT的cmdCurrentOpt做拦截，本次整改继承原有逻辑
     * 其他场景响应消息需要使用对应AT的cmdCurrentOpt做拦截
     *
     * ID_TAF_CCM_CALL_ORIG_CNF：
     * 当前有AT_CMD_ECLSTART_SET AT_CMD_D_CS_VOICE_CALL_SET AT_CMD_D_CS_DATA_CALL_SET AT_CMD_APDS_SET
     * AT_CMD_CACMIMS_SET AT_CMD_CUSTOMDIAL_SET 6个状态会处理该回复消息，
     *
     * ID_TAF_CCM_CALL_SUPS_CMD_CNF：
     * 当前有AT_CMD_H_SET AT_CMD_CHUP_SET AT_CMD_REJCALL_SET AT_CMD_A_SET AT_CMD_CHLD_SET AT_CMD_CHLD_EX_SET
     * AT_CMD_CTFR_SET AT_CMD_ECLSTOP_SET 8种状态处理该消息，此外自动应答时也会回复该消息，但AT不处理
     */
    { ID_TAF_CCM_CALL_ORIG_CNF,             AT_CMD_OPT_NO_NEED_CHECK,       At_RcvTafCcmCallOrigCnf },
    { ID_TAF_CCM_CALL_SUPS_CMD_CNF,         AT_CMD_OPT_NO_NEED_CHECK,       At_RcvTafCcmCallSupsCmdCnf },
    { ID_TAF_CCM_SET_ALS_CNF,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmSetAlsCnf },
    { ID_TAF_CCM_SET_UUS1_INFO_CNF,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmSetUus1InfoCnf },
    { ID_TAF_CCM_QRY_CALL_INFO_CNF,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmQryCallInfoCnf },
    { ID_TAF_CCM_QRY_UUS1_INFO_CNF,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmQryUus1InfoCnf },
    { ID_TAF_CCM_QRY_ALS_CNF,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmQryAlsCnf },

    /* 无特性宏控制的设置请求响应 */
    { ID_TAF_CCM_SET_CSSN_CNF,              AT_CMD_CSSN_SET,                AT_RcvTafCcmCommonSetCnf },
    { ID_TAF_CCM_START_DTMF_CNF,            AT_CMD_DTMF_SET,                AT_RcvTafCcmCsCommonSetCnf },
    { ID_TAF_CCM_START_DTMF_CNF,            AT_CMD_VTS_SET,                 AT_RcvTafCcmCsCommonSetCnf },
    { ID_TAF_CCM_STOP_DTMF_CNF,             AT_CMD_DTMF_SET,                AT_RcvTafCcmCsCommonSetCnf },
    { ID_TAF_CCM_STOP_DTMF_CNF,             AT_CMD_VTS_SET,                 AT_RcvTafCcmCsCommonSetCnf },
    { ID_TAF_CCM_CUSTOM_ECC_NUM_CNF,        AT_CMD_NVM_SET,                 AT_RcvTafCcmCommonSetCnf },

    /* 无特性宏控制的查询请求响应 */
    { ID_TAF_CCM_QRY_CHANNEL_INFO_CNF,      AT_CMD_CSCHANNELINFO_QRY,       AT_RcvTafCcmCSChannelInfoQryCnf },
    { ID_TAF_CCM_QRY_XLEMA_CNF,             AT_CMD_XLEMA_QRY,               AT_RcvTafCcmQryXlemaCnf },
    { ID_TAF_CCM_QRY_CLPR_CNF,              AT_CMD_CLPR_SET,                AT_RcvTafCcmQryClprCnf },
    { ID_TAF_CCM_QRY_CLPR_CNF,              AT_CMD_CCLPR_SET,               AT_RcvTafCcmQryClprCnf },
    { ID_TAF_CCM_GET_CDUR_CNF,              AT_CMD_CDUR_READ,               AT_RcvTafCcmGetCdurCnf },
    { ID_TAF_CCM_CNAP_QRY_CNF,              AT_CMD_CNAPEX_QRY,              AT_RcvTafCcmCnapQryCnf },
    { ID_TAF_CCM_CALL_SUPS_CMD_RSLT_IND,    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallSupsCmdRsltInd },

    /* 无特性宏控制的测试和打断 */

    /* 无特性宏控制的主动上报 */
    { ID_TAF_CCM_CALL_CHANNEL_INFO_IND,     AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmChannelInfoInd },
    { ID_TAF_CCM_ECC_NUM_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmEccNumInd },
    { ID_TAF_CCM_CALL_ORIG_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallOrigInd },
    { ID_TAF_CCM_CALL_CONNECT_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallConnectInd },
    { ID_TAF_CCM_CALL_INCOMING_IND,         AT_CMD_OPT_NO_NEED_CHECK,       At_RcvTafCcmCallIncomingInd },
    { ID_TAF_CCM_CALL_RELEASED_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallReleaseInd },
    { ID_TAF_CCM_CALL_ALL_RELEASED_IND,     AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallAllReleasedInd },
    { ID_TAF_CCM_CALL_PROC_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallProcInd },
    { ID_TAF_CCM_CALL_ALERTING_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallAlertingInd },
    { ID_TAF_CCM_CALL_HOLD_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallHoldInd },
    { ID_TAF_CCM_CALL_RETRIEVE_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallRetrieveInd },
    { ID_TAF_CCM_CALL_SS_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallSsInd },
    { ID_TAF_CCM_UUS1_INFO_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmUus1InfoInd },
    { ID_TAF_CCM_CNAP_INFO_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCnapInfoInd },

    /* 特性宏控制部分 */
#if (FEATURE_IMS == FEATURE_ON)
    { ID_TAF_CCM_ECONF_DIAL_CNF,            AT_CMD_CACMIMS_SET,             AT_RcvTafCcmEconfDialCnf },
    { ID_TAF_CCM_ECONF_DIAL_CNF,            AT_CMD_ECONF_DIAL_SET,          AT_RcvTafCcmEconfDialCnf },
    { ID_TAF_CCM_CALL_MODIFY_CNF,           AT_CMD_CALL_MODIFY_INIT_SET,    At_RcvTafCcmCallModifyCnf },
    { ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF, AT_CMD_CALL_MODIFY_ANS_SET, At_RcvTafCcmCallAnswerRemoteModifyCnf },
    { ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF, AT_CMD_CLCCECONF_QRY,           AT_RcvTafCcmQryEconfCalledInfoCnf },
    { ID_TAF_CCM_CCWAI_SET_CNF,             AT_CMD_CCWAI_SET,               AT_RcvTafCcmCsCommonSetCnf },
    { ID_TAF_CCM_CANCEL_ADD_VIDEO_CNF,      AT_CMD_IMSVIDEOCALLCANCEL_SET,  AT_RcvTafCcmCsCommonSetCnf },

    { ID_TAF_CCM_ECONF_NOTIFY_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmEconfNotifyInd },
    { ID_TAF_CCM_CALL_MODIFY_STATUS_IND,    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallModifyStatusInd },
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { ID_TAF_CCM_SEND_FLASH_CNF,            AT_CMD_CFSH_SET,                AT_RcvTafCcmSndFlashRslt },
    { ID_TAF_CCM_SEND_BURST_DTMF_CNF,       AT_CMD_CBURSTDTMF_SET,          AT_RcvTafCcmSndBurstDTMFCnf },
    { ID_TAF_CCM_SEND_CONT_DTMF_CNF,        AT_CMD_CCONTDTMF_SET,           AT_RcvTafCcmSndContinuousDTMFCnf },
    { ID_TAF_CCM_PRIVACY_MODE_SET_CNF,      AT_CMD_CPMP_SET,                AT_RcvTafCcmPrivacyModeSetCnf },
    { ID_TAF_CCM_PRIVACY_MODE_QRY_CNF,      AT_CMD_CPMP_QRY,                AT_RcvTafCcmPrivacyModeQryCnf },

    { ID_TAF_CCM_BURST_DTMF_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmRcvBurstDtmfInd },
    { ID_TAF_CCM_CONT_DTMF_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmRcvContinuousDtmfInd },
    { ID_TAF_CCM_CCWAC_INFO_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCCWACInd },
    { ID_TAF_CCM_CALLED_NUM_INFO_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCalledNumInfoInd },
    { ID_TAF_CCM_CALLING_NUM_INFO_IND,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmCallingNumInfoInd },
    { ID_TAF_CCM_DISPLAY_INFO_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmDispInfoInd },
    { ID_TAF_CCM_EXT_DISPLAY_INFO_IND,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmExtDispInfoInd },
    { ID_TAF_CCM_CONN_NUM_INFO_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmConnNumInfoInd },
    { ID_TAF_CCM_REDIR_NUM_INFO_IND,        AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmRedirNumInfoInd },
    { ID_TAF_CCM_SIGNAL_INFO_IND,           AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmSignalInfoInd },
    { ID_TAF_CCM_LINE_CTRL_INFO_IND,        AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmLineCtrlInfoInd },
    { ID_TAF_CCM_PRIVACY_MODE_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmPrivacyModeInd },


#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
    { ID_TAF_CCM_ENCRYPT_VOICE_CNF,         AT_CMD_ECCALL_SET,              AT_RcvTafCcmEncryptCallCnf },
    { ID_TAF_CCM_REMOTE_CTRL_ANSWER_CNF,    AT_CMD_ECCTRL_SET,              AT_RcvTafCcmRemoteCtrlAnsCnf },
    { ID_TAF_CCM_ECC_SRV_CAP_CFG_CNF,       AT_CMD_ECCAP_SET,               AT_RcvTafCcmEccCapSetCnf },
    { ID_TAF_CCM_ECC_SRV_CAP_QRY_CNF,       AT_CMD_ECCAP_QRY,               AT_RcvTafCcmEccCapQryCnf },

    { ID_TAF_CCM_ENCRYPT_VOICE_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmEncryptCallInd },
    { ID_TAF_CCM_EC_REMOTE_CTRL_IND,        AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmEccRemoteCtrlInd },

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
    { ID_TAF_CCM_SET_EC_TEST_MODE_CNF,      AT_CMD_ECCTEST_SET,             AT_RcvTafCcmSetEccTestModeCnf },
    { ID_TAF_CCM_GET_EC_TEST_MODE_CNF,      AT_CMD_ECCTEST_QRY,             AT_RcvTafCcmQryEccTestModeCnf },
    { ID_TAF_CCM_GET_EC_RANDOM_CNF,         AT_CMD_ECRANDOM_QRY,            AT_RcvTafCcmQryEccRandomCnf },
    { ID_TAF_CCM_GET_EC_KMC_CNF,            AT_CMD_ECKMC_QRY,               AT_RcvTafCcmQryEccKmcCnf },
    { ID_TAF_CCM_SET_EC_KMC_CNF,            AT_CMD_ECKMC_SET,               AT_RcvTafCcmSetEccKmcCnf },

    { ID_TAF_CCM_ENCRYPTED_VOICE_DATA_IND,  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvTafCcmEncryptedVoiceDataInd },
#endif
#endif
#endif
};

STATIC CONST AT_MsgProcEntryTblInfo g_atProcCcmMsgTblInfo = {
    .tbl      = g_atProcCcmMsgTbl,
    .entryNum = AT_ARRAY_SIZE(g_atProcCcmMsgTbl),
    .entryLen = sizeof(g_atProcCcmMsgTbl[0])
};

STATIC VOS_UINT32 AT_GetCcmUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    TAF_CCM_Msg *ccmMsg = (TAF_CCM_Msg *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(ccmMsg->ctrl.clientId, indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("AT_GetCcmUserId: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }
    return VOS_OK;
}

STATIC CONST AT_ModuleMsgProcInfo g_atCcmMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_CCM,
    .tblInfo    = &g_atProcCcmMsgTblInfo,
    .GetMsgId   = VOS_NULL_PTR,
    .GetUserId  = AT_GetCcmUserId,
};

VOS_VOID AT_InitCcmMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atCcmMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitCcmMsgProcTbl: register fail");
    }
}

/*
 * 功能描述: 来自CCM模块的消息处理函数
 */
VOS_VOID AT_ProcCcmMsg(struct MsgCB *msg)
{
    VOS_UINT32 ret;

#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON && FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    AT_SetCsCallStateWakeLock(((MSG_Header *)msg)->msgName);
#endif

    ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_CCM);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_ProcCcmMsg: fail");
    }
}


