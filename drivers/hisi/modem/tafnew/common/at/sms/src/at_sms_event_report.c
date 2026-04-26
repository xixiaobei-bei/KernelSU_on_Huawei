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
#include "at_sms_event_report.h"
#include "securec.h"
#include "at_check_func.h"
#include "ppp_interface.h"
#include "taf_mmi_str_parse.h"
#include "app_vc_api.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "taf_std_lib.h"
#include "mn_comm_api.h"
#include "at_event_report.h"
#include "at_sms_comm.h"
#include "at_msg_print.h"
#include "at_common.h"

#include "at_mdrv_interface.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_EVENT_REPORT_C

#define AT_FOUR_BYTES_TO_BITS_LENS 32
#define AT_SEND_DATA_BUFF_OFFSET_1 1
#define AT_MSG_TP_MTI_DELIVER 0x00        /* SMS-DELIVER       */
#define AT_MSG_TP_MTI_DELIVER_REPORT 0x00 /* SMS-DELIVER-REPORT */
#define AT_MSG_TP_MTI_STATUS_REPORT 0x02  /* SMS-STATUS-REPORT */
#define AT_MSG_TP_MTI_COMMAND 0x02        /* SMS-COMMAND       */
#define AT_MSG_TP_MTI_SUBMIT 0x01         /* SMS-SUBMIT        */
#define AT_MSG_TP_MTI_SUBMIT_REPORT 0x01  /* SMS-SUBMIT-REPORT */
#define AT_MSG_TP_MTI_RESERVE 0x03        /* RESERVE           */

static const AT_SMS_ErrorCodeMap g_atSmsErrorCodeMap[] = {
    { TAF_MSG_ERROR_RP_CAUSE_UNASSIGNED_UNALLOCATED_NUMBER, AT_CMS_UNASSIGNED_UNALLOCATED_NUMBER },
    { TAF_MSG_ERROR_RP_CAUSE_OPERATOR_DETERMINED_BARRING, AT_CMS_OPERATOR_DETERMINED_BARRING },
    { TAF_MSG_ERROR_RP_CAUSE_CALL_BARRED, AT_CMS_CALL_BARRED },
    { TAF_MSG_ERROR_RP_CAUSE_SHORT_MESSAGE_TRANSFER_REJECTED, AT_CMS_SHORT_MESSAGE_TRANSFER_REJECTED },
    { TAF_MSG_ERROR_RP_CAUSE_DESTINATION_OUT_OF_ORDER, AT_CMS_DESTINATION_OUT_OF_SERVICE },
    { TAF_MSG_ERROR_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER, AT_CMS_UNIDENTIFIED_SUBSCRIBER },
    { TAF_MSG_ERROR_RP_CAUSE_FACILITY_REJECTED, AT_CMS_FACILITY_REJECTED },
    { TAF_MSG_ERROR_RP_CAUSE_UNKNOWN_SUBSCRIBER, AT_CMS_UNKNOWN_SUBSCRIBER },
    { TAF_MSG_ERROR_RP_CAUSE_NETWORK_OUT_OF_ORDER, AT_CMS_NETWORK_OUT_OF_ORDER },
    { TAF_MSG_ERROR_RP_CAUSE_TEMPORARY_FAILURE, AT_CMS_TEMPORARY_FAILURE },
    { TAF_MSG_ERROR_RP_CAUSE_CONGESTION, AT_CMS_CONGESTION },
    { TAF_MSG_ERROR_RP_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED, AT_CMS_RESOURCES_UNAVAILABLE_UNSPECIFIED },
    { TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED, AT_CMS_REQUESTED_FACILITY_NOT_SUBSCRIBED },
    { TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED, AT_CMS_REQUESTED_FACILITY_NOT_IMPLEMENTED },
    { TAF_MSG_ERROR_RP_CAUSE_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE, AT_CMS_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE },
    { TAF_MSG_ERROR_RP_CAUSE_INVALID_MANDATORY_INFORMATION, AT_CMS_INVALID_MANDATORY_INFORMATION },
    { TAF_MSG_ERROR_RP_CAUSE_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED, AT_CMS_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED },
    { TAF_MSG_ERROR_RP_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE, AT_CMS_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE },
    { TAF_MSG_ERROR_RP_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED, AT_CMS_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED },
    { TAF_MSG_ERROR_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED, AT_CMS_PROTOCOL_ERROR_UNSPECIFIED },
    { TAF_MSG_ERROR_RP_CAUSE_INTERWORKING_UNSPECIFIED, AT_CMS_INTERWORKING_UNSPECIFIED },
    { TAF_MSG_ERROR_TP_FCS_TELEMATIC_INTERWORKING_NOT_SUPPORTED, AT_CMS_TELEMATIC_INTERWORKING_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED, AT_CMS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_CANNOT_REPLACE_SHORT_MESSAGE, AT_CMS_CANNOT_REPLACE_SHORT_MESSAGE },
    { TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPPID_ERROR, AT_CMS_UNSPECIFIED_TPPID_ERROR },
    { TAF_MSG_ERROR_TP_FCS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED, AT_CMS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_MESSAGE_CLASS_NOT_SUPPORTED, AT_CMS_MESSAGE_CLASS_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPDCS_ERROR, AT_CMS_UNSPECIFIED_TPDCS_ERROR },
    { TAF_MSG_ERROR_TP_FCS_COMMAND_CANNOT_BE_ACTIONED, AT_CMS_COMMAND_CANNOT_BE_ACTIONED },
    { TAF_MSG_ERROR_TP_FCS_COMMAND_UNSUPPORTED, AT_CMS_COMMAND_UNSUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPCOMMAND_ERROR, AT_CMS_UNSPECIFIED_TPCOMMAND_ERROR },
    { TAF_MSG_ERROR_TP_FCS_TPDU_NOT_SUPPORTED, AT_CMS_TPDU_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_SC_BUSY, AT_CMS_SC_BUSY },
    { TAF_MSG_ERROR_TP_FCS_NO_SC_SUBSCRIPTION, AT_CMS_NO_SC_SUBSCRIPTION },
    { TAF_MSG_ERROR_TP_FCS_SC_SYSTEM_FAILURE, AT_CMS_SC_SYSTEM_FAILURE },
    { TAF_MSG_ERROR_TP_FCS_INVALID_SME_ADDRESS, AT_CMS_INVALID_SME_ADDRESS },
    { TAF_MSG_ERROR_TP_FCS_DESTINATION_SME_BARRED, AT_CMS_DESTINATION_SME_BARRED },
    { TAF_MSG_ERROR_TP_FCS_SM_REJECTEDDUPLICATE_SM, AT_CMS_SM_REJECTEDDUPLICATE_SM },
    { TAF_MSG_ERROR_TP_FCS_TPVPF_NOT_SUPPORTED, AT_CMS_TPVPF_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_TPVP_NOT_SUPPORTED, AT_CMS_TPVP_NOT_SUPPORTED },
    { TAF_MSG_ERROR_TP_FCS_SIM_SMS_STORAGE_FULL, AT_CMS_SIM_SMS_STORAGE_FULL },
    { TAF_MSG_ERROR_TP_FCS_NO_SMS_STORAGE_CAPABILITY_IN_SIM, AT_CMS_NO_SMS_STORAGE_CAPABILITY_IN_SIM },
    { TAF_MSG_ERROR_TP_FCS_ERROR_IN_MS, AT_CMS_ERROR_IN_MS },
    { TAF_MSG_ERROR_TP_FCS_MEMORY_CAPACITY_EXCEEDED, AT_CMS_MEMORY_CAPACITY_EXCEEDED },
    { TAF_MSG_ERROR_TP_FCS_SIM_APPLICATION_TOOLKIT_BUSY, AT_CMS_SIM_APPLICATION_TOOLKIT_BUSY },
    { TAF_MSG_ERROR_TP_FCS_SIM_DATA_DOWNLOAD_ERROR, AT_CMS_SIM_DATA_DOWNLOAD_ERROR },
    { TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_ERROR_CAUSE, AT_CMS_UNSPECIFIED_ERROR_CAUSE },
    { TAF_MSG_ERROR_STATE_NOT_COMPATIBLE, AT_CMS_ME_FAILURE },
    { TAF_MSG_ERROR_NO_SERVICE, AT_CMS_NO_NETWORK_SERVICE },
    { TAF_MSG_ERROR_TC1M_TIMEOUT, AT_CMS_NETWORK_TIMEOUT },
    { TAF_MSG_ERROR_TR1M_TIMEOUT, AT_CMS_NETWORK_TIMEOUT },
    { TAF_MSG_ERROR_TR2M_TIMEOUT, AT_CMS_NO_CNMA_ACKNOWLEDGEMENT_EXPECTED },
};

AT_SMS_RSP_PROC_FUN g_atSmsMsgProcTable[MN_MSG_EVT_MAX] = {
    At_SendSmRspProc,           /* MN_MSG_EVT_SUBMIT_RPT */
    At_SetCnmaRspProc,          /* MN_MSG_EVT_MSG_SENT */
    At_SmsRspNop,               /* MN_MSG_EVT_MSG_STORED */
    At_SmsDeliverProc,          /* MN_MSG_EVT_DELIVER */
    At_SmsDeliverErrProc,       /* MN_MSG_EVT_DELIVER_ERR */
    At_SmsStorageListProc,      /* MN_MSG_EVT_SM_STORAGE_LIST */ /* 区分主动上报和响应消息的处理 */
    At_SmsRspNop,               /* MN_MSG_EVT_STORAGE_FULL */
    At_SmsStorageExceedProc,    /* MN_MSG_EVT_STORAGE_EXCEED */
    At_ReadRspProc,             /* MN_MSG_EVT_READ */
    At_ListRspProc,             /* MN_MSG_EVT_LIST */
    At_WriteSmRspProc,          /* MN_MSG_EVT_WRITE */
    At_DeleteRspProc,           /* MN_MSG_EVT_DELETE */
    At_DeleteTestRspProc,       /* MN_MSG_EVT_DELETE_TEST */
    At_SmsModSmStatusRspProc,   /* MN_MSG_EVT_MODIFY_STATUS */
    At_SetCscaCsmpRspProc,      /* MN_MSG_EVT_WRITE_SRV_PARM */
    AT_QryCscaRspProc,          /* MN_MSG_EVT_READ_SRV_PARM */
    At_SmsSrvParmChangeProc,    /* MN_MSG_EVT_SRV_PARM_CHANGED */
    At_SmsRspNop,               /* MN_MSG_EVT_DELETE_SRV_PARM */
    At_SmsRspNop,               /* MN_MSG_EVT_READ_STARPT */
    At_SmsRspNop,               /* MN_MSG_EVT_DELETE_STARPT */
    AT_SetMemStatusRspProc,     /* MN_MSG_EVT_SET_MEMSTATUS */
    At_SmsRspNop,               /* MN_MSG_EVT_MEMSTATUS_CHANGED */
    At_SmsRspNop,               /* MN_MSG_EVT_MATCH_MO_STARPT_INFO */
    At_SetRcvPathRspProc,       /* MN_MSG_EVT_SET_RCVMSG_PATH */
    At_SmsRspNop,               /* MN_MSG_EVT_GET_RCVMSG_PATH */
    At_SmsRcvMsgPathChangeProc, /* MN_MSG_EVT_RCVMSG_PATH_CHANGED */
    At_SmsInitSmspResultProc,   /* MN_MSG_EVT_INIT_SMSP_RESULT */
    At_SmsInitResultProc,       /* MN_MSG_EVT_INIT_RESULT */
    At_SetCmmsRspProc,          /* MN_MSG_EVT_SET_LINK_CTRL_PARAM */
    At_GetCmmsRspProc,          /* MN_MSG_EVT_GET_LINK_CTRL_PARAM */
    At_SmsStubRspProc,          /* MN_MSG_EVT_STUB_RESULT */
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    At_SmsDeliverCbmProc,      /* MN_MSG_EVT_DELIVER_CBM */
    At_GetCbActiveMidsRspProc, /* MN_MSG_EVT_GET_CBTYPE */
    AT_ChangeCbMidsRsp,        /* MN_MSG_EVT_ADD_CBMIDS */
    AT_ChangeCbMidsRsp,        /* MN_MSG_EVT_DELETE_CBMIDS */
    AT_ChangeCbMidsRsp,        /* MN_MSG_EVT_DELETE_ALL_CBMIDS */

#if (FEATURE_ETWS == FEATURE_ON)
    At_ProcDeliverEtwsPrimNotify, /* MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY */
#else
    At_SmsRspNop, /* MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY */
#endif

#else
    At_SmsRspNop, /* MN_MSG_EVT_DELIVER_CBM */
    At_SmsRspNop, /* MN_MSG_EVT_GET_CBTYPE */
    At_SmsRspNop, /* MN_MSG_EVT_ADD_CBMIDS */
    At_SmsRspNop, /* MN_MSG_EVT_DELETE_CBMIDS */
    At_SmsRspNop  /* MN_MSG_EVT_DELETE_ALL_CBMIDS */
    At_SmsRspNop, /* MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY */
#endif
    At_SetCgsmsRspProc, /* MN_MSG_EVT_SET_SEND_DOMAIN_PARAM */
    At_GetCgsmsRspProc, /* MN_MSG_EVT_GET_SEND_DOMAIN_PARAM */
};

TAF_VOID At_SmsRspNop(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_INFO_LOG("At_SmsRspNop: no operation need for the event type ");
}

VOS_UINT32 AT_GetSmsRpReportCause(TAF_MSG_ErrorUint32 msgCause)
{
    VOS_UINT32 i;
    VOS_UINT32 mapLength;

    /* 27005 3.2.5 0...127 3GPP TS 24.011 [6] clause E.2 values */
    /* 27005 3.2.5 128...255 3GPP TS 23.040 [3] clause 9.2.3.22 values.  */
    mapLength = sizeof(g_atSmsErrorCodeMap) / sizeof(g_atSmsErrorCodeMap[0]);

    for (i = 0; i < mapLength; i++) {
        if (g_atSmsErrorCodeMap[i].msgErrorCode == msgCause) {
            return g_atSmsErrorCodeMap[i].atErrorCode;
        }
    }

    return AT_CMS_UNKNOWN_ERROR;
}

TAF_VOID At_MsgResultCodeFormat(TAF_UINT8 indexNum, TAF_UINT16 length)
{
    errno_t memResult;
    if (g_atVType == AT_V_ENTIRE_TYPE) {
        memResult = memcpy_s((TAF_CHAR *)g_atSndCrLfAddr,
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_OFFSET_1,
                             (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_OFFSET_1,
                            AT_CRLF_STR_LEN);
        At_SendResultData(indexNum, g_atSndCrLfAddr, length + AT_CRLF_STR_LEN);
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
}

TAF_VOID At_SendSmRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT32 ret    = AT_OK;
    TAF_UINT16 length = 0;

    AT_INFO_LOG("At_SendSmRspProc: step into function.");

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SendSmRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    /* 状态不匹配: 当前没有等待发送结果的AT命令，丢弃该结果事件上报 */
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGS_TEXT_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGS_PDU_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGC_TEXT_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGC_PDU_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMSS_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMST_SET)) {
        return;
    }

    g_atSendDataBuff.bufLen = 0;

    if (event->u.submitRptInfo.errorCode != TAF_MSG_ERROR_NO_ERROR) {
        AT_NORM_LOG("At_SendSmRspProc: pstEvent->u.stSubmitRptInfo.enRptStatus is not ok.");

        ret = AT_GetSmsRpReportCause(event->u.submitRptInfo.errorCode);
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, ret);
        return;
    }

    if (g_atClientTab[indexNum].atSmsData.msgSentSmNum < 1) {
        AT_WARN_LOG("At_SendSmRspProc: the number of sent message is zero.");
        return;
    }
    g_atClientTab[indexNum].atSmsData.msgSentSmNum--;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d", event->u.submitRptInfo.mr);

    if (g_atClientTab[indexNum].atSmsData.msgSentSmNum == 0) {
        AT_StopTimerCmdReady(indexNum);
        g_atSendDataBuff.bufLen = length;
        At_FormatResultData(indexNum, ret);
    } else {
        At_MsgResultCodeFormat(indexNum, length);
    }
}

TAF_VOID At_SetCnmaRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetCnmaRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CNMA_TEXT_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CNMA_PDU_SET)) {
        AT_StopTimerCmdReady(indexNum);
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_OK);
    }
}

TAF_VOID At_SetCscaCsmpRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    errno_t         memResult;
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetCscaCsmpRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (event->u.srvParmInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.srvParmInfo.failCause);
    } else {
        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CSMP_SET) {
            memResult = memcpy_s(&(smsCtx->cscaCsmpInfo.vp), sizeof(smsCtx->cscaCsmpInfo.vp),
                                 &(smsCtx->cscaCsmpInfo.tmpVp), sizeof(smsCtx->cscaCsmpInfo.tmpVp));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cscaCsmpInfo.vp), sizeof(smsCtx->cscaCsmpInfo.tmpVp));
            smsCtx->cscaCsmpInfo.fo     = smsCtx->cscaCsmpInfo.tmpFo;
            smsCtx->cscaCsmpInfo.foUsed = TAF_TRUE;
        }
        memResult = memcpy_s(&(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(smsCtx->cscaCsmpInfo.parmInUsim),
                             &event->u.srvParmInfo.srvParm, sizeof(event->u.srvParmInfo.srvParm));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cscaCsmpInfo.parmInUsim),
                            sizeof(event->u.srvParmInfo.srvParm));
        ret = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_VOID At_SmsDeliverErrProc(VOS_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if ((smsCtx->localStoreFlg == VOS_TRUE) && (event->u.deliverErrInfo.errorCode == TAF_MSG_ERROR_TR2M_TIMEOUT)) {
        smsCtx->cnmiType.cnmiMtType = AT_CNMI_MT_NO_SEND_TYPE;
        smsCtx->cnmiType.cnmiDsType = AT_CNMI_DS_NO_SEND_TYPE;
        AT_WARN_LOG("At_SmsDeliverErrProc: CnmiMtType and CnmiDsType changed!");
    }

    /* 短信接收流程因为写操作失败不会上报事件给AT，且该事件有ERROR LOG记录不需要上报给应用处理 */
}

/*
 * 功能描述: 短信存储读取cpms的处理
 */
LOCAL VOS_VOID AT_ProcSmsGetCpmsCmd(TAF_UINT8 indexNum)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CPMS_READ) {
        if ((g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus == TAF_FALSE) &&
            (g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus == TAF_FALSE)) {
            AT_StopTimerCmdReady(indexNum);
            At_PrintGetCpmsRsp(indexNum);
        }
    }
}

TAF_VOID At_GetCpmsMemStatus(VOS_UINT8 indexNum, MN_MSG_MemStoreUint8 memType, TAF_UINT32 *totalRec,
                             TAF_UINT32 *usedRec)
{
    MN_MSG_StorageListEvtInfo *storageList = VOS_NULL_PTR;
    AT_ModemSmsCtx            *smsCtx      = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (memType == MN_MSG_MEM_STORE_SIM) {
        storageList = &(smsCtx->cpmsInfo.usimStorage);
    } else if (memType == MN_MSG_MEM_STORE_ME) {
        storageList = &(smsCtx->cpmsInfo.nvimStorage);
    } else { /* 无存储设备 */
        *usedRec  = 0;
        *totalRec = 0;
        return;
    }

    *totalRec = storageList->totalRec;
    *usedRec  = storageList->usedRec;
}

TAF_VOID At_PrintSetCpmsRsp(TAF_UINT8 indexNum)
{
    TAF_UINT16 length = 0;
    TAF_UINT32 totalRec; /* sms capacity of NVIM or USIM */
    /* used records including all status */
    TAF_UINT32      usedRec;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* <used1>,<total1>,<used2>,<total2>,<used3>,<total3> */
    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.memReadorDelete, &totalRec, &usedRec);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,%d,", usedRec, totalRec);

    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.memSendorWrite, &totalRec, &usedRec);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,%d,", usedRec, totalRec);

    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.rcvPath.smMemStore, &totalRec, &usedRec);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,%d", usedRec, totalRec);

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);
}

TAF_UINT8* At_GetCpmsMemTypeStr(MN_MSG_MemStoreUint8 memType)
{
    TAF_UINT32 memoryType;

    if (memType == MN_MSG_MEM_STORE_SIM) {
        memoryType = AT_STRING_SM;
    } else if (memType == MN_MSG_MEM_STORE_ME) {
        memoryType = AT_STRING_ME;
    } else {
        memoryType = AT_STRING_BUTT;
    }
    return g_atStringTab[memoryType].text;
}

VOS_VOID At_PrintGetCpmsRsp(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 totalRec = 0; /* sms capacity of NVIM or USIM */
    /* used records including all status */
    VOS_UINT32      usedRec = 0;
    VOS_UINT8      *memTypeStr = VOS_NULL_PTR;
    AT_ModemSmsCtx *smsCtx     = VOS_NULL_PTR;

    smsCtx   = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* <used1>,<total1>,<used2>,<total2>,<used3>,<total3> */
    memTypeStr = At_GetCpmsMemTypeStr(smsCtx->cpmsInfo.memReadorDelete);
    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.memReadorDelete, &totalRec, &usedRec);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s,%d,%d,", memTypeStr, usedRec, totalRec);

    memTypeStr = At_GetCpmsMemTypeStr(smsCtx->cpmsInfo.memSendorWrite);
    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.memSendorWrite, &totalRec, &usedRec);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s,%d,%d,", memTypeStr, usedRec, totalRec);

    memTypeStr = At_GetCpmsMemTypeStr(smsCtx->cpmsInfo.rcvPath.smMemStore);
    At_GetCpmsMemStatus(indexNum, smsCtx->cpmsInfo.rcvPath.smMemStore, &totalRec, &usedRec);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s,%d,%d", memTypeStr, usedRec, totalRec);

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);
}

/*
 * 功能描述: 短信存储设置cpms的处理
 */
LOCAL VOS_VOID AT_ProcSmsSetCpmsCmd(TAF_UINT8 indexNum)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CPMS_SET) {
        if ((g_atClientTab[indexNum].atSmsData.waitForCpmsSetRsp == TAF_FALSE) &&
            (g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus == TAF_FALSE) &&
            (g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus == TAF_FALSE)) {
            AT_StopTimerCmdReady(indexNum);
            At_PrintSetCpmsRsp(indexNum);
        }
    }
}

VOS_VOID AT_ReportSmMeFull(VOS_UINT8 indexNum, MN_MSG_MemStoreUint8 memStore)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), "^SMMEMFULL: ");

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", At_GetStrContent(At_GetSmsArea(memStore)));
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID At_SmsStorageListProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    MN_MSG_StorageListEvtInfo *storageListInfo = VOS_NULL_PTR;
    AT_ModemSmsCtx            *smsCtx          = VOS_NULL_PTR;
    errno_t                    memResult;

    MN_MSG_MtCustomizeUint8 mtCustomize;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    storageListInfo = &event->u.storageListInfo;
    if (storageListInfo->memStroe == MN_MSG_MEM_STORE_SIM) {
        memResult = memcpy_s(&(smsCtx->cpmsInfo.usimStorage), sizeof(smsCtx->cpmsInfo.usimStorage),
                             &event->u.storageListInfo, sizeof(event->u.storageListInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cpmsInfo.usimStorage), sizeof(event->u.storageListInfo));
    } else {
        memResult = memcpy_s(&(smsCtx->cpmsInfo.nvimStorage), sizeof(smsCtx->cpmsInfo.nvimStorage),
                             &event->u.storageListInfo, sizeof(event->u.storageListInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cpmsInfo.nvimStorage), sizeof(event->u.storageListInfo));
    }

    mtCustomize = smsCtx->smMeFullCustomize.mtCustomize;
    if ((mtCustomize == MN_MSG_MT_CUSTOMIZE_FT) &&
        (event->u.storageListInfo.totalRec == event->u.storageListInfo.usedRec) &&
        (event->u.storageStateInfo.memStroe == MN_MSG_MEM_STORE_SIM)) {
        AT_INFO_LOG("At_SmsStorageListProc: FT memory full.");
        AT_ReportSmMeFull(indexNum, event->u.storageStateInfo.memStroe);
    }

    if (!AT_IsBroadcastClientIndex(indexNum)) {
        /* 接收到NV的短信容量上报，修改NV的短信容量等待标志已接收到NV的短信容量 */
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
        if (storageListInfo->memStroe == MN_MSG_MEM_STORE_ME) {
            g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus = TAF_FALSE;
        }
#endif
        /* 接收到SIM的短信容量上报，修改SIM的短信容量等待标志已接收到SIM的短信容量 */
        if (storageListInfo->memStroe == MN_MSG_MEM_STORE_SIM) {
            g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus = TAF_FALSE;
        }

        /* CPMS的设置操作需要等待所有容量信息和设置响应消息后完成 */
        AT_ProcSmsSetCpmsCmd(indexNum);

        /* CPMS的读取操作需要等待所有容量信息后完成 */
        AT_ProcSmsGetCpmsCmd(indexNum);
    }
}

VOS_VOID At_SmsStorageExceedProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    /* 初始化 */

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_SmsStorageExceedProc: Get modem id fail.");
        return;
    }

    AT_ReportSmMeFull(indexNum, event->u.storageStateInfo.memStroe);
}

TAF_UINT32 At_MsgPduInd(MN_MSG_BcdAddr *scAddr, MN_MSG_RawTsData *pdu, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    /* <alpha> 不报 */

    /* <length> */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(dst + length), ",%d", pdu->len);

    /* <data> 有可能得到是UCS2，需仔细处理 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)dst + length, "%s", g_atCrLf);

    /* SCA */
    if (scAddr->bcdLen == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(dst + length), "00");
    } else {
        /* 字符串以/0结尾多一位，所以bcdLen长度+1 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(dst + length), "%X%X%X%X", (((scAddr->bcdLen + 1) & 0xf0) >> 4),
            ((scAddr->bcdLen + 1) & 0x0f), ((scAddr->addrType & 0xf0) >> 4), (scAddr->addrType & 0x0f));

        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                      scAddr->bcdNum, (TAF_UINT16)scAddr->bcdLen);
    }

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                  pdu->data, (TAF_UINT16)pdu->len);

    return length;
}

TAF_VOID At_GetMsgFoValue(MN_MSG_TsDataInfo *tsDataInfo, TAF_UINT8 *fo)
{
    TAF_UINT8 foTemp = 0;

    switch (tsDataInfo->tpduType) {
        case MN_MSG_TPDU_DELIVER:
            /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI: */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_DELIVER);
            AT_SET_MSG_TP_MMS(foTemp, tsDataInfo->u.deliver.moreMsg);
            AT_SET_MSG_TP_RP(foTemp, tsDataInfo->u.deliver.replayPath);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.deliver.userDataHeaderInd);
            AT_SET_MSG_TP_SRI(foTemp, tsDataInfo->u.deliver.staRptInd);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            /* TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_DELIVER_REPORT);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.deliverRptAck.userDataHeaderInd);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            /* TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_DELIVER_REPORT);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.deliverRptErr.userDataHeaderInd);
            break;
        case MN_MSG_TPDU_STARPT:
            /* TP MTI TP UDHI TP MMS TP SRQ */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_STATUS_REPORT);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.staRpt.userDataHeaderInd);
            AT_SET_MSG_TP_MMS(foTemp, tsDataInfo->u.staRpt.moreMsg);
            AT_SET_MSG_TP_SRQ(foTemp, tsDataInfo->u.staRpt.staRptQualCommand); /* ?? */
            break;
        case MN_MSG_TPDU_SUBMIT:
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_SUBMIT);
            AT_SET_MSG_TP_RD(foTemp, tsDataInfo->u.submit.rejectDuplicates);
            AT_SET_MSG_TP_VPF(foTemp, tsDataInfo->u.submit.validPeriod.validPeriod);
            AT_SET_MSG_TP_RP(foTemp, tsDataInfo->u.submit.replayPath);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.submit.userDataHeaderInd);
            AT_SET_MSG_TP_SRR(foTemp, tsDataInfo->u.submit.staRptReq);
            break;
        case MN_MSG_TPDU_COMMAND:
            /* TP MTI TP UDHI TP SRR */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_COMMAND);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.command.userDataHeaderInd);
            AT_SET_MSG_TP_SRR(foTemp, tsDataInfo->u.command.staRptReq);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            /* TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_SUBMIT_REPORT);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.submitRptAck.userDataHeaderInd);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            /* TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(foTemp, AT_MSG_TP_MTI_SUBMIT_REPORT);
            AT_SET_MSG_TP_UDHI(foTemp, tsDataInfo->u.submitRptErr.userDataHeaderInd);
            break;
        default:
            AT_NORM_LOG("At_GetMsgFoValue: invalid TPDU type.");
            break;
    }

    *fo = foTemp;
}

TAF_UINT16 At_PrintMsgFo(MN_MSG_TsDataInfo *tsDataInfo, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;
    TAF_UINT8  fo     = 0;

    At_GetMsgFoValue(tsDataInfo, &fo);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)dst + length, "%d", fo);

    return length;
}

/*
 * Description: 读取ASCII类型的号码
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_ReadNumTypePara(TAF_UINT8 *dst, TAF_UINT8 *src)
{
    TAF_UINT16 length = 0;

    if (g_atCscsType == AT_CSCS_UCS2_CODE) { /* +CSCS:UCS2 */
        TAF_UINT16 srcLen = (TAF_UINT16)VOS_StrLen((TAF_CHAR *)src);

        length += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length, src,
                                                    srcLen);
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%s", src);
    }
    return length;
}

TAF_UINT16 At_PrintAsciiAddr(MN_MSG_AsciiAddr *addr, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)dst + length, "\"");
    if ((addr->len > 0) && (addr->len <= MN_MAX_ASCII_ADDRESS_NUM)) {
        addr->asciiNum[addr->len] = 0;

        if (addr->numType == MN_MSG_TON_INTERNATIONAL) {
            length += (TAF_UINT16)At_ReadNumTypePara((dst + length), (TAF_UINT8 *)"+");
        }

        length += (TAF_UINT16)At_ReadNumTypePara((dst + length), addr->asciiNum);
    }
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)dst + length, "\"");

    return length;
}

VOS_UINT32 AT_BcdAddrToAscii(MN_MSG_BcdAddr *bcdAddr, MN_MSG_AsciiAddr *asciiAddr)
{
    VOS_UINT32 ret;

    if ((bcdAddr == VOS_NULL_PTR) || (asciiAddr == VOS_NULL_PTR)) {
        AT_WARN_LOG("MN_MSG_BcdAddrToAscii: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (bcdAddr->bcdLen > MN_MSG_MAX_BCD_NUM_LEN) {
        AT_WARN_LOG("AT_BcdAddrToAscii: length of BcdAddr ucBcdLen is invalid.");

        return MN_ERR_INVALIDPARM;
    }

    asciiAddr->numType = ((bcdAddr->addrType >> 4) & 0x07);
    asciiAddr->numPlan = (bcdAddr->addrType & 0x0f);
    if ((bcdAddr->bcdNum[bcdAddr->bcdLen - 1] & 0xF0) != 0xF0) {
        asciiAddr->len = (VOS_UINT32)bcdAddr->bcdLen * AT_DOUBLE_LENGTH;
    } else {
        asciiAddr->len = ((VOS_UINT32)bcdAddr->bcdLen * AT_DOUBLE_LENGTH) - 1;
    }

    if (asciiAddr->len > MN_MAX_ASCII_ADDRESS_NUM) {
        AT_WARN_LOG("MN_MSG_BcdAddrToAscii: length of number is invalid.");
        return MN_ERR_INVALIDPARM;
    }

    ret = AT_BcdNumberToAscii(bcdAddr->bcdNum, bcdAddr->bcdLen, (VOS_CHAR *)asciiAddr->asciiNum);
    if (ret == MN_ERR_NO_ERROR) {
        return MN_ERR_NO_ERROR;
    } else {
        return MN_ERR_INVALIDPARM;
    }
}

TAF_UINT16 At_PrintBcdAddr(MN_MSG_BcdAddr *bcdAddr, TAF_UINT8 *dst)
{
    TAF_UINT16       length;
    TAF_UINT32       ret;
    MN_MSG_AsciiAddr asciiAddr;

    (VOS_VOID)memset_s(&asciiAddr, sizeof(MN_MSG_AsciiAddr), 0x00, sizeof(asciiAddr));

    if (bcdAddr->bcdLen != 0) {
        ret = AT_BcdAddrToAscii(bcdAddr, &asciiAddr);
        if (ret != MN_ERR_NO_ERROR) {
            AT_WARN_LOG("At_PrintBcdAddr: Fail to convert BCD to ASCII.");
            return 0;
        }
    }

    length = At_PrintAsciiAddr(&asciiAddr, dst);
    return length;
}

TAF_UINT16 At_PrintAddrType(MN_MSG_AsciiAddr *addr, TAF_UINT8 *dst)
{
    TAF_UINT8  addrType = 0x80;
    TAF_UINT16 length = 0;

    addrType |= addr->numPlan;
    addrType |= ((addr->numType << 4) & 0x70);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(dst + length), "%d", addrType);

    return length;
}

TAF_VOID At_ForwardMsgToTeInCmt(VOS_UINT8 indexNum, TAF_UINT16 *sendLength, MN_MSG_TsDataInfo *tsDataInfo,
                                MN_MSG_EventInfo *event)
{
    TAF_UINT16      length = *sendLength;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* +CMT */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "+CMT: ");

    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) {
        /* +CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data> */
        /* <oa> */
        length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.deliver.origAddr, (g_atSndCodeAddress + length));
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

        /* <alpha> 不报 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

        /* <scts> */
        length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.deliver.timeStamp, (g_atSndCodeAddress + length));
        if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",");
            /* <tooa> */
            length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.deliver.origAddr, (g_atSndCodeAddress + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

            /* <fo> */
            length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + length));

            /* <pid> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d", tsDataInfo->u.deliver.pid);

            /* <dcs> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d,", tsDataInfo->u.deliver.dcs.rawDcsData);

            /* <sca> */
            length += (TAF_UINT16)At_PrintBcdAddr(&event->u.deliverInfo.rcvMsgInfo.scAddr,
                                                  (g_atSndCodeAddress + length));

            /* <tosca> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d", event->u.deliverInfo.rcvMsgInfo.scAddr.addrType);

            /* <length> */
            length += AT_PrintSmsLength(tsDataInfo->u.deliver.dcs.msgCoding, tsDataInfo->u.deliver.userData.len,
                                        (g_atSndCodeAddress + length));
        }

        /* <data> 有可能得到是UCS2，需仔细处理 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

        length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                 tsDataInfo->u.deliver.dcs.msgCoding, (g_atSndCodeAddress + length),
                                                 tsDataInfo->u.deliver.userData.orgData,
                                                 (TAF_UINT16)tsDataInfo->u.deliver.userData.len);
    } else {
        /* +CMT: [<alpha>],<length><CR><LF><pdu> */
        length += (TAF_UINT16)At_MsgPduInd(&event->u.deliverInfo.rcvMsgInfo.scAddr,
                                           &event->u.deliverInfo.rcvMsgInfo.tsRawData, (g_atSndCodeAddress + length));
    }
    *sendLength = length;
}

#if (FEATURE_BASTET == FEATURE_ON)
TAF_VOID At_ForwardMsgToTeInBst(TAF_UINT8 indexNum, TAF_UINT16 *sendLength, MN_MSG_TsDataInfo *tsDataInfo,
                                MN_MSG_EventInfo *event)
{
    TAF_UINT16        length = *sendLength;
    MN_MSG_BcdAddr   *scAddr = VOS_NULL_PTR;
    MN_MSG_RawTsData *pdu    = VOS_NULL_PTR;
    TAF_UINT8         blocklistFlag;

    scAddr = &event->u.deliverInfo.rcvMsgInfo.scAddr;
    pdu    = &event->u.deliverInfo.rcvMsgInfo.tsRawData;

    /* ^BST方式上报短信内容 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "^BST: ");

    /* <length> */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d", pdu->len);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    /* Block名单标识字段,在短信PDU的首部增加一个字节，写Block名单短信标识信息，默认值255 */
    blocklistFlag = 0xFF;
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), "%X%X", ((blocklistFlag & 0xf0) >> 4), (blocklistFlag & 0x0f));

    /* SCA 短信中心地址 */
    if (scAddr->bcdLen == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "00");
    } else {
        /* 字符串以/0结尾多一位，所以bcdLen长度+1 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%X%X%X%X", (((scAddr->bcdLen + 1) & 0xf0) >> 4),
            ((scAddr->bcdLen + 1) & 0x0f), ((scAddr->addrType & 0xf0) >> 4), (scAddr->addrType & 0x0f));

        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      g_atSndCodeAddress + length, scAddr->bcdNum,
                                                      (TAF_UINT16)scAddr->bcdLen);
    }

    /* 短信内容 */
    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  g_atSndCodeAddress + length, pdu->data, (TAF_UINT16)pdu->len);

    *sendLength = length;
}

TAF_VOID AT_BlockSmsReport(TAF_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo)
{
    TAF_UINT16 length;

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s", g_atCrLf);

    /* bst的方式上报 */
    At_ForwardMsgToTeInBst(indexNum, &length, tsDataInfo, event);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}
#endif

TAF_VOID AT_ForwardDeliverMsgToTe(MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo)
{
    TAF_BOOL        bCmtiInd = VOS_FALSE;
    TAF_UINT16      length;
    VOS_UINT8       indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(event->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ForwardDeliverMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    AT_LOG1("AT_ForwardDeliverMsgToTe: current mt is", smsCtx->cnmiType.cnmiMtType);

    if (smsCtx->cnmiType.cnmiMtType == AT_CNMI_MT_NO_SEND_TYPE) {
        return;
    }

    if ((event->u.deliverInfo.rcvSmAct == MN_MSG_RCVMSG_ACT_STORE) &&
        (event->u.deliverInfo.memStore != MN_MSG_MEM_STORE_NONE)) {
        if ((smsCtx->cnmiType.cnmiMtType == AT_CNMI_MT_CMTI_TYPE) ||
            (smsCtx->cnmiType.cnmiMtType == AT_CNMI_MT_CLASS3_TYPE)) {
            bCmtiInd = TAF_TRUE;
        }

        if (tsDataInfo->u.deliver.dcs.msgClass == MN_MSG_MSG_CLASS_2) {
            bCmtiInd = TAF_TRUE;
        }
    }

#if (FEATURE_BASTET == FEATURE_ON)
    if (event->u.deliverInfo.ucBlockRptFlag == VOS_TRUE) {
        AT_BlockSmsReport(indexNum, event, tsDataInfo);
        return;
    }
#endif

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s", g_atCrLf);

    /*
     * 根据MT设置和接收到事件的CLASS类型得到最终的事件上报格式:
     * 协议要求MT为3时CLASS类型获取实际MT类型, 该情况下终端不上报事件与协议不一致
     */
    if (bCmtiInd == TAF_TRUE) {
        /* +CMTI: <mem>,<index> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "+CMTI: %s,%d",
            At_GetStrContent(At_GetSmsArea(event->u.deliverInfo.memStore)), event->u.deliverInfo.inex);
    } else {
        /* CMT的方式上报 */
        At_ForwardMsgToTeInCmt(indexNum, &length, tsDataInfo, event);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 At_StaRptPduInd(MN_MSG_BcdAddr *scAddr, MN_MSG_RawTsData *pdu, VOS_UINT8 *dst)
{
    VOS_UINT16 length = 0;

    /* <length> */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(dst + length), "%d", pdu->len);

    /* <data> 有可能得到是UCS2，需仔细处理 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%s", g_atCrLf);

    /* SCA */
    if (scAddr->bcdLen == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(dst + length), "00");
    } else {
        /* 字符串以/0结尾多一位，所以bcdLen长度+1 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(dst + length), "%X%X%X%X", (((scAddr->bcdLen + 1) & 0xf0) >> 4),
            ((scAddr->bcdLen + 1) & 0x0f), ((scAddr->addrType & 0xf0) >> 4), (scAddr->addrType & 0x0f));

        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                      scAddr->bcdNum, (TAF_UINT16)scAddr->bcdLen);
    }

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                  pdu->data, (TAF_UINT16)pdu->len);

    return length;
}

TAF_VOID AT_ForwardStatusReportMsgToTe(MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo)
{
    TAF_UINT16      length;
    VOS_UINT8       indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(event->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ForwardStatusReportMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    AT_LOG1("AT_ForwardStatusReportMsgToTe: current ds is ", smsCtx->cnmiType.cnmiDsType);

    if (smsCtx->cnmiType.cnmiDsType == AT_CNMI_DS_NO_SEND_TYPE) {
        return;
    }

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s", g_atCrLf);

    if ((event->u.deliverInfo.rcvSmAct == MN_MSG_RCVMSG_ACT_STORE) &&
        (event->u.deliverInfo.memStore != MN_MSG_MEM_STORE_NONE)) {
        /* +CDSI: <mem>,<index> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "+CDSI: %s,%d",
            At_GetStrContent(At_GetSmsArea(event->u.deliverInfo.memStore)), event->u.deliverInfo.inex);
    } else {
        /* +CDS */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "+CDS: ");
        if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) {
            /* +CDS: <fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st> */
            /* <fo> */
            length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + length));

            /* <mr> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d,", tsDataInfo->u.staRpt.mr);

            /* <ra> */
            length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.staRpt.recipientAddr, (g_atSndCodeAddress + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

            /* <tora> */
            length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.staRpt.recipientAddr, (g_atSndCodeAddress + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

            /* <scts> */
            length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.timeStamp, (g_atSndCodeAddress + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

            /* <dt> */
            length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.dischargeTime, (g_atSndCodeAddress + length));

            /* <st> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), ",%d", tsDataInfo->u.staRpt.status);
        } else {
            /* +CDS: <length><CR><LF><pdu> */
            length += (VOS_UINT16)At_StaRptPduInd(&event->u.deliverInfo.rcvMsgInfo.scAddr,
                                                  &event->u.deliverInfo.rcvMsgInfo.tsRawData,
                                                  (g_atSndCodeAddress + length));
        }
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

TAF_VOID AT_ForwardPppMsgToTe(MN_MSG_EventInfo *event)
{
    TAF_UINT32         ret;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;

    tsDataInfo = At_GetMsgMem();

    ret = MN_MSG_Decode(&event->u.deliverInfo.rcvMsgInfo.tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        return;
    }

    if (tsDataInfo->tpduType == MN_MSG_TPDU_DELIVER) {
        AT_ForwardDeliverMsgToTe(event, tsDataInfo);
    } else if (tsDataInfo->tpduType == MN_MSG_TPDU_STARPT) {
        AT_ForwardStatusReportMsgToTe(event, tsDataInfo);
    } else {
        AT_WARN_LOG("AT_ForwardPppMsgToTe: invalid tpdu type.");
    }
}

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))

VOS_VOID AT_ForwardCbMsgToTe(MN_MSG_EventInfo *event)
{
    TAF_UINT32      ret;
    MN_MSG_Cbpage   cbmPageInfo;
    VOS_UINT16      length;
    VOS_UINT8       indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(event->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ForwardCbMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    AT_LOG1("AT_ForwardCbMsgToTe: current bm is ", smsCtx->cnmiType.cnmiBmType);

    /*
     * 根据BM设置和接收到事件的CLASS类型得到最终的事件上报格式:
     * 不支持协议要求BM为3时,CBM的上报
     */

    ret = MN_MSG_DecodeCbmPage(&(event->u.cbsDeliverInfo.cbRawData), &cbmPageInfo);
    if (ret != MN_ERR_NO_ERROR) {
        return;
    }

    event->u.cbsDeliverInfo.geoAreaLen = AT_MIN(event->u.cbsDeliverInfo.geoAreaLen, TAF_CBS_CMAS_GEO_AREA_MAX_LEN);

    if (event->u.cbsDeliverInfo.geoInfoRptFlag == VOS_TRUE) {
        /* ^CBMEXT: <sn>,<mid>,<dcs>,<page>,<pages><CR><LF><data><CR><LF><geoLen>,<geoPdu> (text mode enabled) */
        /* ^CBMEXT: <length><CR><LF><pdu><CR><LF><geoLen><CR><LF><geoPdu> (PDU mode enabled); or */
        length = 0;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "^CBMEXT: ");
    } else {
        /* +CBM: <sn>,<mid>,<dcs>,<page>,<pages><CR><LF><data> (text mode enabled) */
        /* +CBM: <length><CR><LF><pdu> (PDU mode enabled); or */
        length = 0;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "+CBM: ");
    }

    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d,", cbmPageInfo.sn.rawSnData);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d,", cbmPageInfo.mid);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d,", cbmPageInfo.dcs.rawDcsData);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d,", cbmPageInfo.pageIndex);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d", cbmPageInfo.pageNum);

        /* <data> 有可能得到是UCS2，需仔细处理 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

        cbmPageInfo.content.len = (VOS_UINT32)TAF_MIN(cbmPageInfo.content.len, TAF_CBA_MAX_RAW_CBDATA_LEN);

        length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
            cbmPageInfo.dcs.msgCoding, (g_atSndCodeAddress + length),
            cbmPageInfo.content.content, (TAF_UINT16)cbmPageInfo.content.len);

        if (event->u.cbsDeliverInfo.geoInfoRptFlag == VOS_TRUE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%d", event->u.cbsDeliverInfo.geoAreaLen);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                (g_atSndCodeAddress + length), event->u.cbsDeliverInfo.geoAreaStr,
                (TAF_UINT16)event->u.cbsDeliverInfo.geoAreaLen);
        }

    } else {
        /* +CBM: <length><CR><LF><pdu> (PDU mode enabled); or */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%d", event->u.cbsDeliverInfo.cbRawData.len);

        /* <data> 有可能得到是UCS2，需仔细处理 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
            (g_atSndCodeAddress + length), event->u.cbsDeliverInfo.cbRawData.data,
            (TAF_UINT16)event->u.cbsDeliverInfo.cbRawData.len);

       if (event->u.cbsDeliverInfo.geoInfoRptFlag == VOS_TRUE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%d", event->u.cbsDeliverInfo.geoAreaLen);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(g_atSndCodeAddress + length), "%s", g_atCrLf);

            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                (g_atSndCodeAddress + length), event->u.cbsDeliverInfo.geoAreaStr,
                (TAF_UINT16)event->u.cbsDeliverInfo.geoAreaLen);
        }
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    /* 此处还需多一个空行 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

#endif

TAF_VOID At_ForwardMsgToTe(MN_MSG_EventUint32 event, MN_MSG_EventInfo *eventOutPara)
{
    AT_LOG1("At_ForwardMsgToTe: current Event is ", event);

    switch (event) {
        case MN_MSG_EVT_DELIVER:
            AT_ForwardPppMsgToTe(eventOutPara);
            break;

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
        case MN_MSG_EVT_DELIVER_CBM:
            AT_ForwardCbMsgToTe(eventOutPara);
            break;
#endif
        default:
            AT_WARN_LOG("At_SendSmtInd: invalid tpdu type.");
            break;
    }
}

TAF_VOID At_HandleSmtBuffer(VOS_UINT8 indexNum, AT_CNMI_BFR_TYPE bfrType)
{
    TAF_UINT32      loop;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (bfrType == AT_CNMI_BFR_SEND_TYPE) {
        for (loop = 0; loop < AT_BUFFER_SMT_EVENT_MAX; loop++) {
            if (smsCtx->smtBuffer.used[loop] == AT_MSG_BUFFER_USED) {
                At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, &(smsCtx->smtBuffer.event[loop]));
            }
        }
    }

    (VOS_VOID)memset_s(&(smsCtx->smtBuffer), sizeof(smsCtx->smtBuffer), 0x00, sizeof(smsCtx->smtBuffer));
}

TAF_VOID At_SetRcvPathRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetRcvPathRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (event->u.rcvMsgPathInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.srvParmInfo.failCause);
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, ret);
        return;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CPMS_SET) {
        /* 保存临时数据到内存和NVIM */
        smsCtx->cpmsInfo.rcvPath.smMemStore     = event->u.rcvMsgPathInfo.smMemStore;
        smsCtx->cpmsInfo.rcvPath.staRptMemStore = event->u.rcvMsgPathInfo.staRptMemStore;
        smsCtx->cpmsInfo.memReadorDelete        = smsCtx->cpmsInfo.tmpMemReadorDelete;
        smsCtx->cpmsInfo.memSendorWrite         = smsCtx->cpmsInfo.tmpMemSendorWrite;

        g_atClientTab[indexNum].atSmsData.waitForCpmsSetRsp = TAF_FALSE;

        /* CPMS的设置操作需要等待所有容量信息和设置响应消息后完成 */
        if ((g_atClientTab[indexNum].atSmsData.waitForCpmsSetRsp == TAF_FALSE) &&
            (g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus == TAF_FALSE) &&
            (g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus == TAF_FALSE)) {
            AT_StopTimerCmdReady(indexNum);
            At_PrintSetCpmsRsp(indexNum);
        }

    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CSMS_SET) {
        /* 执行命令操作 */
        smsCtx->csmsMsgVersion                  = event->u.rcvMsgPathInfo.smsServVersion;
        smsCtx->cpmsInfo.rcvPath.smsServVersion = smsCtx->csmsMsgVersion;

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress), "%s: ",
            g_parseContext[indexNum].cmdElement->cmdName);
        At_PrintCsmsInfo(indexNum);
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_OK);
    } else { /* AT_CMD_CNMI_SET */
        AT_StopTimerCmdReady(indexNum);
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_OK);

        smsCtx->cnmiType.cnmiBfrType          = smsCtx->cnmiType.cnmiTmpBfrType;
        smsCtx->cnmiType.cnmiDsType           = smsCtx->cnmiType.cnmiTmpDsType;
        smsCtx->cnmiType.cnmiBmType           = smsCtx->cnmiType.cnmiTmpBmType;
        smsCtx->cnmiType.cnmiMtType           = smsCtx->cnmiType.cnmiTmpMtType;
        smsCtx->cnmiType.cnmiModeType         = smsCtx->cnmiType.cnmiTmpModeType;
        smsCtx->cpmsInfo.rcvPath.rcvSmAct     = event->u.rcvMsgPathInfo.rcvSmAct;
        smsCtx->cpmsInfo.rcvPath.rcvStaRptAct = event->u.rcvMsgPathInfo.rcvStaRptAct;

        if (smsCtx->cnmiType.cnmiModeType != 0) {
            At_HandleSmtBuffer(indexNum, smsCtx->cnmiType.cnmiBfrType);
        }
    }
}

VOS_VOID At_SmsInitSmspResultProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;
    errno_t         memResult;

    VOS_UINT8 defaultIndex;

    defaultIndex = event->u.initSmspResultInfo.defaultSmspIndex;

    AT_NORM_LOG1("At_SmsInitSmspResultProc: ucDefaultIndex", defaultIndex);

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (defaultIndex >= MN_MSG_MAX_USIM_EFSMSP_NUM) {
        defaultIndex = AT_CSCA_CSMP_STORAGE_INDEX;
    }

    /* 记录defaultSmspIndex, 在csca csmp 中使用 */
    smsCtx->cscaCsmpInfo.defaultSmspIndex = defaultIndex;

    memResult = memcpy_s(&(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(smsCtx->cscaCsmpInfo.parmInUsim),
                         &event->u.initSmspResultInfo.srvParm[defaultIndex], sizeof(MN_MSG_SrvParam));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(MN_MSG_SrvParam));

    memResult = memcpy_s(&(smsCtx->cpmsInfo.rcvPath), sizeof(smsCtx->cpmsInfo.rcvPath),
                         &event->u.initSmspResultInfo.rcvMsgPath, sizeof(event->u.initSmspResultInfo.rcvMsgPath));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cpmsInfo.rcvPath), sizeof(event->u.initSmspResultInfo.rcvMsgPath));

    g_class0Tailor = event->u.initSmspResultInfo.class0Tailor;
}

VOS_VOID At_SmsSrvParmChangeProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;
    errno_t         memResult;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    memResult = memcpy_s(&(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(smsCtx->cscaCsmpInfo.parmInUsim),
                         &event->u.srvParmChangeInfo.srvParm[smsCtx->cscaCsmpInfo.defaultSmspIndex],
                         sizeof(MN_MSG_SrvParam));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(MN_MSG_SrvParam));
}

VOS_VOID At_SmsRcvMsgPathChangeProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smsCtx->cpmsInfo.rcvPath.rcvSmAct       = event->u.rcvMsgPathInfo.rcvSmAct;
    smsCtx->cpmsInfo.rcvPath.smMemStore     = event->u.rcvMsgPathInfo.smMemStore;
    smsCtx->cpmsInfo.rcvPath.rcvStaRptAct   = event->u.rcvMsgPathInfo.rcvStaRptAct;
    smsCtx->cpmsInfo.rcvPath.staRptMemStore = event->u.rcvMsgPathInfo.staRptMemStore;
}

VOS_VOID AT_SetMemStatusRspProc(VOS_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    VOS_UINT32 result;

    /* 检查用户索引值 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_SetMemStatusRspProc: AT_BROADCAST_INDEX.");
        return;
    }

#if (FEATURE_MBB_CUST == FEATURE_ON)
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CSASM_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_HSMF_SET)) {
        return;
    }
#else
    /* 判断当前操作类型是否为AT_CMD_CSASM_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CSASM_SET) {
        return;
    }
#endif

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->u.memStatusInfo.success == VOS_TRUE) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);
}

VOS_UINT16 AT_PrintSmsLength(MN_MSG_MsgCodingUint8 msgCoding, VOS_UINT32 length, VOS_UINT8 *dst)
{
    VOS_UINT16 lengthTemp;
    VOS_UINT16 smContentLength;

    /* UCS2编码显示字节长度应该是UCS2字符个数，不是BYTE数，所以需要字节数除以2 */
    if (msgCoding == MN_MSG_MSG_CODING_UCS2) {
        smContentLength = (VOS_UINT16)length >> 1;
    } else {
        smContentLength = (VOS_UINT16)length;
    }

    lengthTemp = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)dst,
        ",%d", smContentLength);

    return lengthTemp;
}

LOCAL VOS_VOID AT_FormatReportCmgrTpduDeliver(AT_ModemSmsCtx *smsCtx, MN_MSG_TsDataInfo *tsDataInfo,
    MN_MSG_EventInfo *event, TAF_UINT16 *length)
{
    /* +CMGR: <stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>, <sca>,<tosca>,<length>]<CR><LF><data> */
    /* <oa> */
    *length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.deliver.origAddr, (g_atSndCodeAddress + *length));
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");
    /* <alpha> 不报 */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

    /* <scts> */
    *length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.deliver.timeStamp, (g_atSndCodeAddress + *length));

    if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");
        /* <tooa> */
        *length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.deliver.origAddr, (g_atSndCodeAddress + *length));
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

        /* <fo> */
        *length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + *length));

        /* <pid> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.deliver.pid);

        /* <dcs> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d,", tsDataInfo->u.deliver.dcs.rawDcsData);

        /* <sca> */
        *length += (TAF_UINT16)At_PrintBcdAddr(&event->u.readInfo.msgInfo.scAddr, (g_atSndCodeAddress + *length));

        /* <tosca> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", event->u.readInfo.msgInfo.scAddr.addrType);

        /* <length> */
        *length += AT_PrintSmsLength(tsDataInfo->u.deliver.dcs.msgCoding, tsDataInfo->u.deliver.userData.len,
                                    (g_atSndCodeAddress + *length));
    }
    /* <data> 有可能得到是UCS2，需仔细处理 */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), "%s", g_atCrLf);

    *length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                             tsDataInfo->u.deliver.dcs.msgCoding, (g_atSndCodeAddress + *length),
                                             tsDataInfo->u.deliver.userData.orgData,
                                             (TAF_UINT16)tsDataInfo->u.deliver.userData.len);
}

LOCAL VOS_VOID AT_FormatReportCmgrTpduSubmit(AT_ModemSmsCtx *smsCtx, MN_MSG_TsDataInfo *tsDataInfo,
    MN_MSG_EventInfo *event, TAF_UINT16 *length)
{
    /* +CMGR: <stat>,<da>,[<alpha>][,<toda>,<fo>,<pid>,<dcs>,[<vp>], <sca>,<tosca>,<length>]<CR><LF><data> */
    /* <da> */
    *length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.submit.destAddr, (g_atSndCodeAddress + *length));
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");
    /* <alpha> 不报 */

    if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

        /* <toda> */
        *length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.submit.destAddr, (g_atSndCodeAddress + *length));
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

        /* <fo> */
        *length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + *length));

        /* <pid> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.submit.pid);
        /* <dcs> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d,", tsDataInfo->u.submit.dcs.rawDcsData);
        /* <vp>,还需要仔细处理 */
        *length += At_MsgPrintVp(&tsDataInfo->u.submit.validPeriod, (g_atSndCodeAddress + *length));
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

        /* <sca> */
        *length += At_PrintBcdAddr(&event->u.readInfo.msgInfo.scAddr, (g_atSndCodeAddress + *length));

        /* <tosca> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", event->u.readInfo.msgInfo.scAddr.addrType);

        /* <length> */
        *length += AT_PrintSmsLength(tsDataInfo->u.submit.dcs.msgCoding, tsDataInfo->u.submit.userData.len,
                                    (g_atSndCodeAddress + *length));
    }

    /* <data> 有可能得到是UCS2，需仔细处理 */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), "%s", g_atCrLf);

    *length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                             tsDataInfo->u.submit.dcs.msgCoding, (g_atSndCodeAddress + *length),
                                             tsDataInfo->u.submit.userData.orgData,
                                             (TAF_UINT16)tsDataInfo->u.submit.userData.len);
}

LOCAL VOS_VOID AT_FormatReportCmgrTpduCommand(AT_ModemSmsCtx *smsCtx, MN_MSG_TsDataInfo *tsDataInfo,
    MN_MSG_EventInfo *event, TAF_UINT16 *length)
{
    /* +CMGR: <stat>,<fo>,<ct>[,<pid>,[<mn>],[<da>],[<toda>],<length><CR><LF><cdata>] */
    /* <fo> */
    *length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + *length));
    /* <ct> */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.command.cmdType);

    if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
        /* <pid> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.command.pid);

        /* <mn>,还需要仔细处理 */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d,", tsDataInfo->u.command.msgNumber);

        /* <da> */
        *length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.command.destAddr, (g_atSndCodeAddress + *length));
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

        /* <toda> */
        *length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.command.destAddr, (g_atSndCodeAddress + *length));

        /* <length>为0 */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.command.commandDataLen);

        /* <data> 有可能得到是UCS2，需仔细处理 */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + *length), "%s", g_atCrLf);

        *length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                 MN_MSG_MSG_CODING_8_BIT, (g_atSndCodeAddress + *length),
                                                 tsDataInfo->u.command.cmdData,
                                                 tsDataInfo->u.command.commandDataLen);
    }
}

LOCAL VOS_VOID AT_FormatReportCmgrTpduStarpt(MN_MSG_TsDataInfo *tsDataInfo, MN_MSG_EventInfo *event,
    TAF_UINT16 *length)
{
    /*
     * +CMGR: <stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     */
    /* <fo> */
    *length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (g_atSndCodeAddress + *length));

    /* <mr> */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d,", tsDataInfo->u.staRpt.mr);

    /* <ra> */
    *length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.staRpt.recipientAddr, (g_atSndCodeAddress + *length));
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

    /* <tora> */
    *length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.staRpt.recipientAddr, (g_atSndCodeAddress + *length));
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

    /* <scts> */
    *length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.timeStamp, (g_atSndCodeAddress + *length));
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",");

    /* <dt> */
    *length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.dischargeTime, (g_atSndCodeAddress + *length));

    /* <st> */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + *length), ",%d", tsDataInfo->u.staRpt.status);
}

TAF_UINT32 At_SmsPrintState(AT_CmgfMsgFormatUint8 smsFormat, MN_MSG_StatusTypeUint8 status, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    if (smsFormat == AT_CMGF_MSG_FORMAT_TEXT) { /* TEXT */
        switch (status) {
            case MN_MSG_STATUS_MT_NOT_READ:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_REC_UNREAD_TEXT].text);
                break;

            case MN_MSG_STATUS_MT_READ:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_REC_READ_TEXT].text);
                break;

            case MN_MSG_STATUS_MO_NOT_SENT:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_STO_UNSENT_TEXT].text);
                break;

            case MN_MSG_STATUS_MO_SENT:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_STO_SENT_TEXT].text);
                break;

            case MN_MSG_STATUS_NONE:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_ALL_TEXT].text);
                break;

            default:
                return 0;
        }
    } else { /* PDU */
        switch (status) {
            case MN_MSG_STATUS_MT_NOT_READ:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_REC_UNREAD_PDU].text);
                break;

            case MN_MSG_STATUS_MT_READ:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_REC_READ_PDU].text);
                break;

            case MN_MSG_STATUS_MO_NOT_SENT:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_STO_UNSENT_PDU].text);
                break;

            case MN_MSG_STATUS_MO_SENT:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_STO_SENT_PDU].text);
                break;

            case MN_MSG_STATUS_NONE:
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)dst + length, "%s", g_atStringTab[AT_STRING_ALL_PDU].text);
                break;

            default:
                return 0;
        }
    }

    return length;
}

TAF_VOID At_ReadRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT16         length     = 0;
    TAF_UINT32         ret        = AT_OK;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;
    AT_ModemSmsCtx    *smsCtx     = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ReadRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = 0;

    if (event->u.readInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.readInfo.failCause);
        At_FormatResultData(indexNum, ret);
        return;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (TAF_UINT16)At_SmsPrintState(smsCtx->cmgfMsgFormat, event->u.readInfo.status,
                                           (g_atSndCodeAddress + length));

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

    tsDataInfo = At_GetMsgMem();
    ret        = MN_MSG_Decode(&event->u.readInfo.msgInfo.tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        ret = At_ChgMnErrCodeToAt(indexNum, ret);
        At_FormatResultData(indexNum, ret);
        return;
    }

    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_PDU) { /* PDU */
        /* +CMGR: <stat>,[<alpha>],<length><CR><LF><pdu> */
        length += (TAF_UINT16)At_MsgPduInd(&event->u.readInfo.msgInfo.scAddr, &event->u.readInfo.msgInfo.tsRawData,
                                           (g_atSndCodeAddress + length));

        g_atSendDataBuff.bufLen = length;
        At_FormatResultData(indexNum, AT_OK);
        return;
    }

    switch (event->u.readInfo.msgInfo.tsRawData.tpduType) {
        case MN_MSG_TPDU_DELIVER:
            AT_FormatReportCmgrTpduDeliver(smsCtx, tsDataInfo, event, &length);
            break;
        case MN_MSG_TPDU_SUBMIT:
            AT_FormatReportCmgrTpduSubmit(smsCtx, tsDataInfo, event, &length);
            break;
        case MN_MSG_TPDU_COMMAND:
            AT_FormatReportCmgrTpduCommand(smsCtx, tsDataInfo, event, &length);
            break;
        case MN_MSG_TPDU_STARPT:
            AT_FormatReportCmgrTpduStarpt(tsDataInfo, event, &length);
            break;
        default:
            break;
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);
}

LOCAL VOS_VOID AT_ExcuteListMsgInfo(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    MN_MSG_ListParm listParm = {0};
    errno_t            memResult;

    /* 初始化 */
    memResult = memcpy_s(&listParm, sizeof(listParm), &(event->u.listInfo.receivedListPara), sizeof(MN_MSG_ListParm));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(listParm), sizeof(listParm));

    /* 通知SMS还需要继续显示剩下的短信 */
    listParm.isFirstTimeReq = VOS_FALSE;

    /* 执行命令操作 */
    if (MN_MSG_List(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &listParm) != MN_ERR_NO_ERROR) {
        g_atSendDataBuff.bufLen = 0;
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return;
    }
}

TAF_UINT32 At_PrintListMsg(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo, TAF_UINT8 *dst)
{
    TAF_UINT16      length = 0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    switch (tsDataInfo->tpduType) {
        case MN_MSG_TPDU_DELIVER:
            /* +CMGL: <index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>[<CR><LF> */
            /* <oa> */
            length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.deliver.origAddr, (dst + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");
            /* <alpha> 不报 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            /* <scts> */
            length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.deliver.timeStamp, (dst + length));
            if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)(dst + length), ",");

                /* <tooa> */
                length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.deliver.origAddr, (dst + length));

                /* <length> */
                length += AT_PrintSmsLength(tsDataInfo->u.deliver.dcs.msgCoding, tsDataInfo->u.deliver.userData.len,
                                            (dst + length));
            }

            /* <data> 有可能得到是UCS2，需仔细处理 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)dst, (TAF_CHAR *)(dst + length),
                "%s", g_atCrLf);

            length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN - (TAF_UINT32)(dst - g_atSndCodeAddress),
                                                     (TAF_INT8 *)dst, tsDataInfo->u.deliver.dcs.msgCoding,
                                                     (dst + length), tsDataInfo->u.deliver.userData.orgData,
                                                     (TAF_UINT16)tsDataInfo->u.deliver.userData.len);

            break;
        case MN_MSG_TPDU_SUBMIT:
            /* +CMGL: <index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>[<CR><LF> */
            /* <da> */
            length += At_PrintAsciiAddr(&tsDataInfo->u.submit.destAddr, (TAF_UINT8 *)(dst + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            /* <alpha> 不报 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            if (smsCtx->csdhType == AT_CSDH_SHOW_TYPE) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)(dst + length), ",");

                /* <toda> */
                length += At_PrintAddrType(&tsDataInfo->u.submit.destAddr, (dst + length));

                /* <length> */
                length += AT_PrintSmsLength(tsDataInfo->u.deliver.dcs.msgCoding, tsDataInfo->u.submit.userData.len,
                                            (dst + length));
            }

            /* <data> 有可能得到是UCS2，需仔细处理 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)dst, (TAF_CHAR *)(dst + length),
                "%s", g_atCrLf);

            length += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN, (TAF_INT8 *)dst,
                                                     tsDataInfo->u.submit.dcs.msgCoding,
                                                     (dst + length), tsDataInfo->u.submit.userData.orgData,
                                                     (TAF_UINT16)tsDataInfo->u.submit.userData.len);

            break;
        case MN_MSG_TPDU_COMMAND:
            /* +CMGL: <index>,<stat>,<fo>,<ct>[<CR><LF> */
            /* <fo> */
            length += At_PrintMsgFo(tsDataInfo, (dst + length));
            /* <ct> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)dst, (TAF_CHAR *)(dst + length),
                ",%d", tsDataInfo->u.command.cmdType);
            break;
        case MN_MSG_TPDU_STARPT:
            /*
             * +CMGL: <index>,<stat>,<fo>,<mr>,
             * [<ra>],[<tora>],<scts>,<dt>,<st>
             * [<CR><LF>
             */
            /* <fo> */
            length += (TAF_UINT16)At_PrintMsgFo(tsDataInfo, (dst + length));
            /* <mr> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",%d,", tsDataInfo->u.staRpt.mr);

            /* <ra> */
            length += (TAF_UINT16)At_PrintAsciiAddr(&tsDataInfo->u.staRpt.recipientAddr, (dst + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            /* <tora> */
            length += (TAF_UINT16)At_PrintAddrType(&tsDataInfo->u.staRpt.recipientAddr, (dst + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            /* <scts> */
            length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.timeStamp, (dst + length));
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",");

            /* <dt> */
            length += (TAF_UINT16)At_SmsPrintScts(&tsDataInfo->u.staRpt.dischargeTime, (dst + length));

            /* <st> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)(dst + length), ",%d", tsDataInfo->u.staRpt.status);
            break;
        default:
            break;
    }

    return length;
}

TAF_VOID At_ListRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT16         length = 0;
    TAF_UINT32         ret        = AT_OK;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;
    TAF_UINT32         loop;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ListRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (event->u.listInfo.success != TAF_TRUE) {
        AT_StopTimerCmdReady(indexNum);
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.listInfo.failCause);
        At_FormatResultData(indexNum, ret);
        return;
    }

    if (event->u.listInfo.firstListEvt == VOS_TRUE) {
        if (g_atVType == AT_V_ENTIRE_TYPE) {
            length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s", g_atCrLf);
        }
    }
    tsDataInfo = At_GetMsgMem();

    for (loop = 0; loop < AT_MIN(event->u.listInfo.reportNum, MN_MSG_MAX_REPORT_EVENT_NUM); loop++) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName,
            event->u.listInfo.smInfo[loop].index);

        length += (TAF_UINT16)At_SmsPrintState(smsCtx->cmgfMsgFormat, event->u.listInfo.smInfo[loop].status,
                                               (g_atSndCodeAddress + length));
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)(g_atSndCodeAddress + length), ",");

        ret = MN_MSG_Decode(&event->u.listInfo.smInfo[loop].msgInfo.tsRawData, tsDataInfo);
        if (ret != MN_ERR_NO_ERROR) {
            ret = At_ChgMnErrCodeToAt(indexNum, ret);
            At_FormatResultData(indexNum, ret);
            return;
        }

        if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_PDU) { /* PDU */
            /*
             * +CMGL: <index>,<stat>,[<alpha>],<length><CR><LF><pdu>
             * [<CR><LF>+CMGL:<index>,<stat>,[<alpha>],<length><CR><LF><pdu>
             * [...]]
             */
            /* ?? */
            length += (TAF_UINT16)At_MsgPduInd(&event->u.listInfo.smInfo[loop].msgInfo.scAddr,
                                               &event->u.listInfo.smInfo[loop].msgInfo.tsRawData,
                                               (g_atSndCodeAddress + length));
        } else {
            length += (TAF_UINT16)At_PrintListMsg(indexNum, event, tsDataInfo, (g_atSndCodeAddress + length));
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_BufferorSendResultData(indexNum, g_atSndCodeAddress, length);

        length = 0;
    }

    if (event->u.listInfo.lastListEvt == TAF_TRUE) {
        g_atSendDataBuff.bufLen = 0;
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_OK);
    } else {
        AT_ExcuteListMsgInfo(indexNum, event);
    }
}

TAF_VOID At_WriteSmRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT32 ret    = AT_OK;
    TAF_UINT16 length = 0;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_WriteSmRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_StopTimerCmdReady(indexNum);
    if (event->u.writeInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.writeInfo.failCause);
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d", event->u.writeInfo.index);
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, ret);
}

TAF_VOID At_DeleteRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT32         ret;
    MN_MSG_DeleteParam deleteInfo;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_DeleteRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    memset_s(&deleteInfo, sizeof(deleteInfo), 0x00, sizeof(deleteInfo));

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMGD_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CBMGD_SET)) {
        return;
    }

    deleteInfo.memStore = event->u.deleteInfo.memStore;
    deleteInfo.index    = event->u.deleteInfo.index;
    if (event->u.deleteInfo.success != TAF_TRUE) {
        AT_StopTimerCmdReady(indexNum);
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.deleteInfo.failCause);
        At_FormatResultData(indexNum, ret);
        return;
    }

    if (event->u.deleteInfo.deleteType == MN_MSG_DELETE_SINGLE) {
        g_atClientTab[indexNum].atSmsData.msgDeleteTypes ^= AT_MSG_DELETE_SINGLE;
    }

    if (event->u.deleteInfo.deleteType == MN_MSG_DELETE_ALL) {
        g_atClientTab[indexNum].atSmsData.msgDeleteTypes ^= AT_MSG_DELETE_ALL;
    }

    if (event->u.deleteInfo.deleteType == MN_MSG_DELETE_READ) {
        g_atClientTab[indexNum].atSmsData.msgDeleteTypes ^= AT_MSG_DELETE_READ;
    }

    if (event->u.deleteInfo.deleteType == MN_MSG_DELETE_SENT) {
        g_atClientTab[indexNum].atSmsData.msgDeleteTypes ^= AT_MSG_DELETE_SENT;
    }

    if (event->u.deleteInfo.deleteType == MN_MSG_DELETE_NOT_SENT) {
        g_atClientTab[indexNum].atSmsData.msgDeleteTypes ^= AT_MSG_DELETE_UNSENT;
    }

    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes == 0) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_MsgDeleteCmdProc(indexNum, g_atClientTab[indexNum].opId, deleteInfo,
                            g_atClientTab[indexNum].atSmsData.msgDeleteTypes);
    }
}

VOS_UINT32 AT_GetBitMap(VOS_UINT32 *bitMap, VOS_UINT32 indexNum)
{
    VOS_UINT8  x;
    VOS_UINT32 y;
    VOS_UINT32 mask;

    y    = indexNum / AT_FOUR_BYTES_TO_BITS_LENS;
    x    = (VOS_UINT8)indexNum % AT_FOUR_BYTES_TO_BITS_LENS;
    mask = ((VOS_UINT32)1 << x);
    if ((bitMap[y] & mask) != 0) {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
}

VOS_VOID AT_SmsListIndex(VOS_UINT16 length, MN_MSG_DeleteTestEvtInfo *para, VOS_UINT16 *printOffSet)
{
    TAF_UINT32 loop;
    TAF_UINT32 msgNum = 0;

    for (loop = 0; loop < para->smCapacity; loop++) {
        if (AT_GetBitMap(para->validLocMap, loop) == TAF_TRUE) {
            msgNum++;
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d,", loop);
        }
    }

    /* 删除最后一个"," */
    if (msgNum != 0) {
        length -= 1;
    }

    *printOffSet = length;
}

TAF_VOID At_DeleteTestRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT16                length;
    MN_MSG_DeleteTestEvtInfo *para = VOS_NULL_PTR;
    VOS_UINT32                loop;
    VOS_BOOL                  bMsgExist;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_DeleteTestRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    para = (MN_MSG_DeleteTestEvtInfo *)&event->u.deleteTestInfo;

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGD_TEST) {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: (", g_parseContext[indexNum].cmdElement->cmdName);

        AT_SmsListIndex(length, para, &length);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "),(0-4)");
    } else {
        /* 判断是否有短信索引列表输出: 无短信需要输出直接返回OK */
        bMsgExist = VOS_FALSE;

        for (loop = 0; loop < MN_MSG_CMGD_PARA_MAX_LEN; loop++) {
            if (para->validLocMap[loop] != 0) {
                bMsgExist = VOS_TRUE;
                break;
            }
        }

        if (bMsgExist == VOS_TRUE) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

            AT_SmsListIndex(length, para, &length);
        } else {
            length = 0;
        }
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, AT_OK);
}

TAF_VOID At_SetCmmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_RreturnCodeUint32 result = AT_CMS_UNKNOWN_ERROR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetCmmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_StopTimerCmdReady(indexNum);

    if (event->u.linkCtrlInfo.errorCode == MN_ERR_NO_ERROR) {
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_GetCmmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_RreturnCodeUint32 result = AT_CMS_UNKNOWN_ERROR;
    /* event report:MN_MSG_EVT_SET_COMM_PARAM */
    MN_MSG_LinkCtrlEvtInfo *linkCtrlInfo = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_GetCmmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    g_atSendDataBuff.bufLen = 0;
    linkCtrlInfo            = &event->u.linkCtrlInfo;
    if (linkCtrlInfo->errorCode == MN_ERR_NO_ERROR) {
        result = AT_OK;
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                linkCtrlInfo->linkCtrl);
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_SetCgsmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_RreturnCodeUint32 result = AT_CMS_UNKNOWN_ERROR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetCgsmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_StopTimerCmdReady(indexNum);

    if (event->u.sendDomainInfo.errorCode == MN_ERR_NO_ERROR) {
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_GetCgsmsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_RreturnCodeUint32 result = AT_CMS_UNKNOWN_ERROR;
    /* event report:MN_MSG_EVT_SET_COMM_PARAM */
    MN_MSG_SendDomainEvtInfo *sendDomainInfo = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_GetCgsmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    g_atSendDataBuff.bufLen = 0;
    sendDomainInfo          = &event->u.sendDomainInfo;

    if (sendDomainInfo->errorCode == MN_ERR_NO_ERROR) {
        result = AT_OK;
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                sendDomainInfo->sendDomain);
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_QryCscaRspProc(VOS_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    errno_t         memResult;
    VOS_UINT16      length;
    VOS_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_QryCscaRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* AT模块在等待CSCA查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CSCA_READ) {
        return;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->u.srvParmInfo.success == VOS_TRUE) {
        /* 更新短信中心号码到AT模块，解决MSG模块初始化完成事件上报时AT模块未启动问题 */
        memResult = memcpy_s(&(smsCtx->cscaCsmpInfo.parmInUsim.scAddr), sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr),
                             &event->u.srvParmInfo.srvParm.scAddr, sizeof(event->u.srvParmInfo.srvParm.scAddr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cscaCsmpInfo.parmInUsim.scAddr),
                            sizeof(event->u.srvParmInfo.srvParm.scAddr));

        /* 设置错误码为AT_OK           构造结构为+CSCA: <sca>,<toda>格式的短信 */
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* 短信中心号码存在指示为存在且短信中心号码长度不为0 */
        if (((event->u.srvParmInfo.srvParm.parmInd & MN_MSG_SRV_PARM_MASK_SC_ADDR) == 0) &&
            (event->u.srvParmInfo.srvParm.scAddr.bcdLen != 0)) {
            /* 将SCA地址由BCD码转换为ASCII码 */
            length += At_PrintBcdAddr(&event->u.srvParmInfo.srvParm.scAddr, (g_atSndCodeAddress + length));

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", event->u.srvParmInfo.srvParm.scAddr.addrType);
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",128");
        }

        g_atSendDataBuff.bufLen = length;
        ret                     = AT_OK;
    } else {
        /* 根据pstEvent->u.stSrvParmInfo.ulFailCause调用At_ChgMnErrCodeToAt转换出AT模块的错误码 */
        g_atSendDataBuff.bufLen = 0;
        ret                     = At_ChgMnErrCodeToAt(indexNum, event->u.srvParmInfo.failCause);
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
}

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))

VOS_VOID At_SmsDeliverCbmProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if ((smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_SEND_OR_DISCARD_TYPE) ||
        (smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_SEND_OR_BUFFER_TYPE)) {
        At_ForwardMsgToTe(MN_MSG_EVT_DELIVER_CBM, event);
        return;
    }

    /* 目前CBS消息不缓存 */

    if (smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_EMBED_AND_SEND_TYPE) {
        /* 目前不支持 */
    }
}

VOS_UINT32 AT_CbPrintRange(VOS_UINT16 length, TAF_CBA_CbmiRangeList *cbMidr)
{
    TAF_UINT32 loop;
    TAF_UINT16 addLen = length;

    cbMidr->cbmirNum = AT_MIN(cbMidr->cbmirNum, TAF_CBA_MAX_CBMID_RANGE_NUM);
    for (loop = 0; loop < cbMidr->cbmirNum; loop++) {
        if (cbMidr->cbmiRangeInfo[loop].msgIdFrom == cbMidr->cbmiRangeInfo[loop].msgIdTo) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d", cbMidr->cbmiRangeInfo[loop].msgIdFrom);
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d-%d", cbMidr->cbmiRangeInfo[loop].msgIdFrom,
                cbMidr->cbmiRangeInfo[loop].msgIdTo);
        }

        if (loop != (cbMidr->cbmirNum - 1)) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",");
        }
    }

    addLen = length - addLen;

    return addLen;
}

VOS_VOID At_GetCbActiveMidsRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT16      length;
    TAF_UINT16      addLength;
    VOS_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_GetCbActiveMidsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 停止定时器 */
    AT_StopTimerCmdReady(indexNum);

    if (event->u.cbsCbMids.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.cbsCbMids.failCause);
        At_FormatResultData(indexNum, ret);
        return;
    }

    length = 0;

    /* 保持的永远是激活列表,所以固定填写0 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s:0,", g_parseContext[indexNum].cmdElement->cmdName);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "\"");

    /* 输出消息的MID */
    addLength = (VOS_UINT16)AT_CbPrintRange(length, &(event->u.cbsCbMids.cbMidr));

    length += addLength;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "\",\"");

    /* 输出的语言的MID */
    addLength = (VOS_UINT16)AT_CbPrintRange(length, &(smsCtx->cbsDcssInfo));

    length += addLength;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "\"");

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);
}

VOS_VOID AT_ChangeCbMidsRsp(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT32 ret;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ChangeCbMidsRsp : AT_BROADCAST_INDEX.");
        return;
    }

    if (event->u.cbsChangeInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.cbsChangeInfo.failCause);
    } else {
        ret = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

#if (FEATURE_ETWS == FEATURE_ON)
VOS_VOID At_ProcDeliverEtwsPrimNotify(VOS_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_CBA_EtwsPrimNtfEvtInfo *primNtf = VOS_NULL_PTR;
    VOS_UINT16                  length;

    primNtf = &event->u.etwsPrimNtf;

    /* ^ETWSPN: <plmn id>,<warning type>,<msg id>,<sn>,<auth> [,<warning security information>] */
    /* 示例: ^ETWSPN: "46000",0180,4352,3000,1 */

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + length), "%s^ETWSPN: ", g_atCrLf);

    /*
     * <plmn id>
     * ulMcc，ulMnc的说明及示例：
     * ulMcc的低8位    （即bit0--bit7），对应 MCC digit 1;
     * ulMcc的次低8位  （即bit8--bit15），对应 MCC digit 2;
     * ulMcc的次次低8位（即bit16--bit23），对应 MCC digit 3;
     * ulMnc的低8位    （即bit0--bit7），对应 MNC digit 1;
     * ulMnc的次低8位  （即bit8--bit15），对应 MNC digit 2;
     * ulMnc的次次低8位（即bit16--bit23），对应 MNC digit 3;
     */
    if ((primNtf->plmn.mnc & 0xFF0000) == 0x0F0000) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "\"%d%d%d%d%d\",", (primNtf->plmn.mcc & 0xFF),
            (primNtf->plmn.mcc & 0xFF00) >> 8, (primNtf->plmn.mcc & 0xFF0000) >> 16, (primNtf->plmn.mnc & 0xFF),
            (primNtf->plmn.mnc & 0xFF00) >> 8);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "\"%d%d%d%d%d%d\",", (primNtf->plmn.mcc & 0xFF),
            (primNtf->plmn.mcc & 0xFF00) >> 8, (primNtf->plmn.mcc & 0xFF0000) >> 16, (primNtf->plmn.mnc & 0xFF),
            (primNtf->plmn.mnc & 0xFF00) >> 8, (primNtf->plmn.mnc & 0xFF0000) >> 16);
    }

    /* <warning type> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + length), "%04X,", primNtf->warnType);
    /* <msg id> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + length), "%04X,", primNtf->msgId);
    /* <sn> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + length), "%04X,", primNtf->sn);

    /* <auth> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + length), "%d%s", primNtf->authRslt, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}
#endif /* (FEATURE_ETWS == FEATURE_ON) */

#endif
TAF_VOID At_SmsModSmStatusRspProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    VOS_UINT32 ret;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SmsModSmStatusRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_StopTimerCmdReady(indexNum);
    if (event->u.modifyInfo.success != TAF_TRUE) {
        ret = At_ChgMnErrCodeToAt(indexNum, event->u.deleteInfo.failCause);
    } else {
        ret = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, ret);
}

TAF_VOID At_SmsInitResultProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smsCtx->cpmsInfo.usimStorage.totalRec = event->u.initResultInfo.totalSmRec;
    smsCtx->cpmsInfo.usimStorage.usedRec  = event->u.initResultInfo.usedSmRec;
}

VOS_VOID At_SmsStubRspProc(VOS_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    VOS_UINT32 ret;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SmsStubRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    /* AT模块在等待CMSTUB命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMSTUB_SET) {
        return;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->u.result.errorCode == MN_ERR_NO_ERROR) {
        ret = AT_OK;
    } else {
        ret = AT_CMS_UNKNOWN_ERROR;
    }

    /* 调用At_FormatResultData输出结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, ret);
}

LOCAL VOS_VOID AT_SmsDeliverPreProc(VOS_VOID)
{
#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_SmsStartRingTe(VOS_TRUE);
#endif

    /* 短信接收唤醒锁定时器启动 */
#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON)
    AT_SmsWakeLock(&g_smsWakeLock);
#endif
}

TAF_VOID At_BufferMsgInTa(VOS_UINT8 indexNum, MN_MSG_EventUint32 event, MN_MSG_EventInfo *eventOutPara)
{
    MN_MSG_EventInfo *eventInfo = VOS_NULL_PTR;
    TAF_UINT8        *used      = VOS_NULL_PTR;
    AT_ModemSmsCtx   *smsCtx    = VOS_NULL_PTR;
    errno_t           memResult;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smsCtx->smtBuffer.index = smsCtx->smtBuffer.index % AT_BUFFER_SMT_EVENT_MAX;
    used                    = &(smsCtx->smtBuffer.used[smsCtx->smtBuffer.index]);
    eventInfo               = &(smsCtx->smtBuffer.event[smsCtx->smtBuffer.index]);
    smsCtx->smtBuffer.index++;

    if (*used == AT_MSG_BUFFER_FREE) {
        *used = AT_MSG_BUFFER_USED;
    }

    memResult = memcpy_s(eventInfo, sizeof(MN_MSG_EventInfo), eventOutPara, sizeof(MN_MSG_EventInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_EventInfo), sizeof(MN_MSG_EventInfo));
}

TAF_VOID AT_StubSaveAutoReplyData(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo)
{
    errno_t         memResult;
    TAF_UINT8       loop;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 自动回复功能未开启直接返回; */
    if (smsCtx->smsAutoReply == 0) {
        return;
    }

    /* 接收消息不是DELIVER短信或TP-RP没有置位直接返回 */
    if ((tsDataInfo->tpduType != MN_MSG_TPDU_DELIVER) || (tsDataInfo->u.deliver.replayPath != VOS_TRUE)) {
        return;
    }

    /* 申请并保存自动回复相关参数到缓存 */
    for (loop = 0; loop < AT_SMSMT_BUFFER_MAX; loop++) {
        if (smsCtx->smsMtBuffer[loop].used == TAF_TRUE) {
            continue;
        }

        AT_StubClearSpecificAutoRelyMsg(indexNum, loop);

        /* 记录接收短信信息记录到内存，用于 GCF测试用例34。2。8 */
        smsCtx->smsMtBuffer[loop].event = (MN_MSG_EventInfo *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(MN_MSG_EventInfo));
        if (smsCtx->smsMtBuffer[loop].event == VOS_NULL_PTR) {
            AT_WARN_LOG("At_SmsDeliverProc: Fail to alloc memory.");
            return;
        }

        smsCtx->smsMtBuffer[loop].tsDataInfo = (MN_MSG_TsDataInfo *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                                                 sizeof(MN_MSG_TsDataInfo));
        if (smsCtx->smsMtBuffer[loop].tsDataInfo == VOS_NULL_PTR) {
            /*lint -save -e516 */
            PS_MEM_FREE(WUEPS_PID_AT, smsCtx->smsMtBuffer[loop].event);
            smsCtx->smsMtBuffer[loop].event = VOS_NULL_PTR;
            /*lint -restore */
            AT_WARN_LOG("At_SmsDeliverProc: Fail to alloc memory.");
            return;
        }

        memResult = memcpy_s(smsCtx->smsMtBuffer[loop].event, sizeof(MN_MSG_EventInfo), event,
                             sizeof(MN_MSG_EventInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_EventInfo), sizeof(MN_MSG_EventInfo));
        memResult = memcpy_s(smsCtx->smsMtBuffer[loop].tsDataInfo, sizeof(MN_MSG_TsDataInfo), tsDataInfo,
                             sizeof(MN_MSG_TsDataInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_TsDataInfo), sizeof(MN_MSG_TsDataInfo));

        smsCtx->smsMtBuffer[loop].used = TAF_TRUE;

        break;
    }
}

VOS_VOID At_SmsDeliverProc(TAF_UINT8 indexNum, MN_MSG_EventInfo *event)
{
    TAF_UINT16         length = 0;
    TAF_UINT32         ret;
    MN_MSG_TsDataInfo *tsDataInfo = VOS_NULL_PTR;
    VOS_UINT8          userId = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    AT_ModemSmsCtx    *smsCtx = VOS_NULL_PTR;

    /* 通过ClientId获取ucUserId */
    if (At_ClientIdToUserId(event->clientId, &userId) == AT_FAILURE) {
        AT_WARN_LOG("AT_SmsDeliverProc: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(userId);

    /*
     * 当前短信类型为CLass0且短信定制为
     * 1:H3G与意大利TIM Class 0短信需求相同，不对短信进行存储，要求将CLASS 0
     * 短信直接采用+CMT进行主动上报。不受CNMI以及CPMS设置的影响，如果后台已经
     * 打开，则后台对CLASS 0短信进行显示。
     * CLass0的短信此时不考虑MT,MODE的参数
     */

    tsDataInfo = At_GetMsgMem();
    ret        = MN_MSG_Decode(&event->u.deliverInfo.rcvMsgInfo.tsRawData, tsDataInfo);
    if (ret != MN_ERR_NO_ERROR) {
        return;
    }

    AT_StubSaveAutoReplyData(userId, event, tsDataInfo);

    AT_SmsDeliverPreProc();

    if ((tsDataInfo->tpduType == MN_MSG_TPDU_DELIVER) && (tsDataInfo->u.deliver.dcs.msgClass == MN_MSG_MSG_CLASS_0) &&
        (g_class0Tailor != MN_MSG_CLASS0_DEF)) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        if ((g_class0Tailor == MN_MSG_CLASS0_TIM) || (g_class0Tailor == MN_MSG_CLASS0_VIVO)) {
            /* +CMT格式上报 */
            At_ForwardMsgToTeInCmt(userId, &length, tsDataInfo, event);
        } else {
            /* +CMTI: <mem>,<index> */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "+CMTI: %s,%d",
                At_GetStrContent(At_GetSmsArea(event->u.deliverInfo.memStore)), event->u.deliverInfo.inex);
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_SendResultData(userId, g_atSndCodeAddress, length);

        return;
    }

    if (smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_SEND_OR_DISCARD_TYPE) {
        At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, event);
        return;
    }

    /* 当模式为0时缓存 */
    if (smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_BUFFER_TYPE) {
        At_BufferMsgInTa(indexNum, MN_MSG_EVT_DELIVER, event);
        return;
    }

    /* 当模式为2时缓存 */
    if (smsCtx->cnmiType.cnmiModeType == AT_CNMI_MODE_SEND_OR_BUFFER_TYPE) {
        /* 判断是否具备缓存的条件 */
        if (AT_IsClientBlock() == VOS_TRUE) {
            At_BufferMsgInTa(indexNum, MN_MSG_EVT_DELIVER, event);
        } else {
            At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, event);
        }
        return;
    }

    /* 目前不支持 AT_CNMI_MODE_EMBED_AND_SEND_TYPE */
}

TAF_VOID At_SmsMsgProc(MN_AT_IndEvt *data, TAF_UINT16 len)
{
    MN_MSG_EventInfo  *event = VOS_NULL_PTR;
    MN_MSG_EventUint32 eventTemp = MN_MSG_EVT_MAX;
    TAF_UINT8          indexNum = 0;
    TAF_UINT32         eventLen;
    errno_t            memResult;

    AT_INFO_LOG("At_SmsMsgProc: Step into function.");
    AT_LOG1("At_SmsMsgProc: pstData->clientId,", data->clientId);

    eventLen  = sizeof(MN_MSG_EventUint32);
    memResult = memcpy_s(&eventTemp, sizeof(eventTemp), data->content, eventLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(eventTemp), eventLen);
    event = (MN_MSG_EventInfo *)&data->content[eventLen];

    if (At_ClientIdToUserId(data->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_SmsMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (!AT_IsBroadcastClientIndex(indexNum)) {
        if (event->opId != g_atClientTab[indexNum].opId) {
            AT_LOG1("At_SmsMsgProc: pstEvent->opId,", event->opId);
            AT_LOG1("At_SmsMsgProc: g_atClientTab[ucIndex].opId,", g_atClientTab[indexNum].opId);
            AT_NORM_LOG("At_SmsMsgProc: invalid operation id.");
            return;
        }

        AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);
    }

    if (eventTemp >= MN_MSG_EVT_MAX) {
        AT_WARN_LOG("At_SmsRspProc: invalid event type.");
        return;
    }

    AT_LOG1("At_SmsMsgProc enEvent", eventTemp);
    g_atSmsMsgProcTable[eventTemp](indexNum, event);
}
