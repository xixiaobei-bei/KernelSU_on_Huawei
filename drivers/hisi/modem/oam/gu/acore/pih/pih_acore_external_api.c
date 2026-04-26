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

/*
 * 版权所有（c）华为技术有限公司 2015-2019
 * 功能描述: 该文件定义了PIH模块A核的接口
 * 生成日期: 2018年3月23日
 */

#include "pih_external_api.h"
#include "si_pb.h"
#include "si_pih.h"
#include "si_typedef.h"
#include "product_config.h"
#include "at_oam_interface.h"
#include "om_private.h"
#include "pam_tag.h"

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) && (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF))
#include "pam_app_om.h"
#else
#include "pam_om.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_PIH_API_C
#define THIS_MODU mod_pam_pih

#define PIH_GEN_LOG_MODULE(Level) (MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, Level))

#define PIH_KEY_INFO_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define PIH_KEY_NORMAL_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define PIH_KEY_WARNING_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string)
#define PIH_KEY_ERROR_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string)

#define PIH_KEY_INFO1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define PIH_KEY_NORMAL1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define PIH_KEY_WARNING1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string "%d", para1)
#define PIH_KEY_ERROR1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string "%d", para1)

#define PIH_INFO_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define PIH_NORMAL_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define PIH_WARNING_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string)
#define PIH_ERROR_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string)

#define PIH_INFO1_LOG(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)

#define PIH_NORMAL1_LOG(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)

#define PIH_WARNING1_LOG(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string "%d", para1)

#define PIH_ERROR1_LOG(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(PIH_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_PIH_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string "%d", para1)

#define SI_PIH_APDU_HDR_LEN (USIMM_APDU_HEADLEN)

/*
 * 功能描述: 获取PIH的PID
 * 修改历史:
 * 1. 日    期: 2018年03月13日
 *    修改内容: Create
 */
VOS_UINT32 SI_PIH_GetReceiverPid(MN_CLIENT_ID_T clientId, VOS_UINT32 *receiverPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    ModemIdUint16 modemId;
    SI_PIH_CardSlotUint32 slotId;

    /* 调用接口获取Modem ID */
    if (AT_GetModemIdFromClient(clientId, &modemId) != VOS_OK) {
        return VOS_ERR;
    }

    slotId = SI_GetSlotIdByModemId(modemId);
    if (slotId >= SI_PIH_CARD_SLOT_BUTT) {
        return VOS_ERR;
    }

    if (slotId == SI_PIH_CARD_SLOT_0) {
        *receiverPid = I0_MAPS_PIH_PID;
    } else if (slotId == SI_PIH_CARD_SLOT_1) {
        *receiverPid = I1_MAPS_PIH_PID;
    } else {
        *receiverPid = I2_MAPS_PIH_PID;
    }
#else
    *receiverPid = I0_MAPS_PIH_PID;
#endif
    return VOS_OK;
}

/*
 * 功能描述: FDN激活
 * 修改历史:
 * 1. 日    期: 2008年10月18日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_FdnEnable(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const VOS_UINT8 *pin2, VOS_UINT32 len)
{
    SI_PIH_FdnEnableReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    if (pin2 == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_FdnEnable:PIN NULL.");
        return TAF_FAILURE;
    }

    if ((len > SI_PIH_PIN_CODE_LEN) || (len == 0)) {
        PIH_ERROR_LOG("SI_PIH_FdnEnable:PIN's len Error.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_FdnEnable:Get receiverPid Error.");
        return TAF_FAILURE;
    }

#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    if (g_pbInitState.pbInitStep != PB_INIT_FINISHED) {
        PIH_ERROR_LOG("SI_PIH_FdnEnable:PB is Busy.");
        return TAF_FAILURE;
    }
#endif

    msg = (SI_PIH_FdnEnableReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_FdnEnableReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_FdnEnable:WARNING AllocMsg FAILED.");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_FDN_ENABLE_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_FDN_CNF;

    (VOS_VOID)memset_s(msg->pin2, SI_PIH_PIN_CODE_LEN, 0xff, SI_PIH_PIN_CODE_LEN);

    ret = memcpy_s(msg->pin2, SI_PIH_PIN_CODE_LEN, pin2, len);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_FdnEnable:WARNING SendMsg FAILED.");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: FDN去激活
 * 修改历史:
 * 1. 日    期: 2008年10月18日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_FdnDisable(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const VOS_UINT8 *pin2, VOS_UINT32 len)
{
    SI_PIH_FdnDisableReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    if (pin2 == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_FdnDisable:PIN NULL.");
        return TAF_FAILURE;
    }

    if ((len > SI_PIH_PIN_CODE_LEN) || (len == 0)) {
        PIH_ERROR_LOG("SI_PIH_FdnDisable:PIN's len Error.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_FdnDisable:Get receiverPid Error.");
        return TAF_FAILURE;
    }

#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    if (g_pbInitState.pbInitStep != PB_INIT_FINISHED) {
        PIH_ERROR_LOG("SI_PIH_FdnEnable:PB is Busy.");
        return TAF_FAILURE;
    }
#endif

    msg = (SI_PIH_FdnDisableReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_FdnDisableReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_FdnDisable:WARNING AllocMsg FAILED");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_FDN_DISALBE_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_FDN_CNF;

    (VOS_VOID)memset_s(msg->pin2, SI_PIH_PIN_CODE_LEN, 0xff, SI_PIH_PIN_CODE_LEN);

    ret = memcpy_s(msg->pin2, SI_PIH_PIN_CODE_LEN, pin2, len);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_FdnDisable:WARNING SendMsg FAILED");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: FDN状态查询
 * 修改历史:
 * 1. 日    期: 2008年10月20日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_FdnBdnQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PIH_QueryTypeUint32 queryType)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_FdnBdnQuery:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_FdnDisable:WARNING AllocMsg FAILED");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;

    if (queryType == SI_PIH_FDN_QUERY) {
        msg->eventType = SI_PIH_EVENT_FDN_CNF;
        msg->msgName = SI_PIH_FDN_QUERY_REQ;
    } else {
        msg->eventType = SI_PIH_EVENT_BDN_CNF;
        msg->msgName = SI_PIH_BDN_QUERY_REQ;
    }

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_FdnDisable:WARNING SendMsg FAILED");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: FDN去激活
 * 修改历史:
 * 1. 日    期: 2008年10月18日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_GenericAccessReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CsimCommand *data)
{
    SI_PIH_GaccessReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_GenericAccessReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    if (data == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_GenericAccessReq:Get NULL PTR.");
        return TAF_FAILURE;
    }

    if ((data->len == 0) || (data->len > SI_APDU_MAX_LEN)) {
        PIH_ERROR_LOG("SI_PIH_GenericAccessReq: Data Len is error");
        return TAF_FAILURE;
    }

    if ((data->len > SI_PIH_APDU_HDR_LEN) && (data->command[4] != data->len - SI_PIH_APDU_HDR_LEN)) {
        PIH_ERROR_LOG("SI_PIH_GenericAccessReq: Data Len is Not Eq P3");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_GaccessReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_GaccessReq) - VOS_MSG_HEAD_LENGTH + data->len);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_GenericAccessReq: AllocMsg FAILED");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_GACCESS_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_GENERIC_ACCESS_CNF;
    msg->dataLen = data->len;

    ret = memcpy_s(msg->data, data->len, data->command, data->len);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_FdnDisable:WARNING SendMsg FAILED");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: ISDB透传APDU接口函数
 * 修改历史:
 * 1. 日    期: 2012年08月28日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_IsdbAccessReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_IsdbAccessCommand *data)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 打开逻辑通道请求
 * 修改历史:
 * 1. 日    期: 2013年05月15日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CchoSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CchoCommand *cchoCmd)
{
    SI_PIH_CchoSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    /* 参数检测 */
    if (cchoCmd == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CchoSetReq: NULL PTR.");

        return TAF_FAILURE;
    }

    if ((cchoCmd->aidLen == 0) || (cchoCmd->aidLen > USIMM_AID_LEN_MAX)) {
        PIH_ERROR_LOG("SI_PIH_CchoSetReq: AID length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CchoSetReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CchoSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)(sizeof(SI_PIH_CchoSetReqMsg) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CchoSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_CCHO_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_CCHO_SET_CNF;
    msg->aidLen = cchoCmd->aidLen;

    ret = memcpy_s(msg->adfName, sizeof(msg->adfName), cchoCmd->adfName, cchoCmd->aidLen);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CchoSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 打开逻辑通道请求
 * 修改历史:
 * 1. 日    期: 2016年09月26日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CchpSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CchpCommand *cchpCmd)
{
    SI_PIH_CchpSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    /* 参数检测 */
    if (cchpCmd == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CchpSetReq: NULL_PTR.");

        return TAF_FAILURE;
    }

    if ((cchpCmd->aidLen == 0) || (cchpCmd->aidLen > USIMM_AID_LEN_MAX)) {
        PIH_ERROR_LOG("SI_PIH_CchpSetReq: AID length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CchpSetReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CchpSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)(sizeof(SI_PIH_CchpSetReqMsg) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CchpSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_CCHP_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_CCHP_SET_CNF;
    msg->apdup2 = cchpCmd->apdup2;
    msg->aidLen = cchpCmd->aidLen;

    ret = memcpy_s(msg->adfName, sizeof(msg->adfName), cchpCmd->adfName, cchpCmd->aidLen);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CchpSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 私有打开逻辑通道请求
 * 修改历史:
 * 1. 日    期: 2018年04月09日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_PrivateCchoSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CchoCommand *cchoCmd)
{
    SI_PIH_CchoSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    /* 参数检测 */
    if (cchoCmd == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchoSetReq: NULL_PTR.");

        return TAF_FAILURE;
    }

    if (cchoCmd->aidLen > USIMM_AID_LEN_MAX) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchoSetReq: AID length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchoSetReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CchoSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)(sizeof(SI_PIH_CchoSetReqMsg) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_PrivateCchoSetReq: VOS_AllocMsg FAILED.");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_PRIVATECCHO_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_PRIVATECCHO_SET_CNF;
    msg->aidLen = cchoCmd->aidLen;

    if (cchoCmd->aidLen != 0) {
        ret = memcpy_s(msg->adfName, sizeof(msg->adfName), cchoCmd->adfName, cchoCmd->aidLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_PrivateCchoSetReq: VOS_SendMsg FAILED.");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 私有打开逻辑通道请求
 * 修改历史:
 * 1. 日    期: 2018年04月09日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_PrivateCchpSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CchpCommand *cchpCmd)
{
    SI_PIH_CchpSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    /* 参数检测 */
    if (cchpCmd == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchpSetReq: NULL_PTR.");

        return TAF_FAILURE;
    }

    if (cchpCmd->aidLen > USIMM_AID_LEN_MAX) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchpSetReq: AID length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_PrivateCchpSetReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CchpSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)(sizeof(SI_PIH_CchpSetReqMsg) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_PrivateCchpSetReq: VOS_AllocMsg FAILED.");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_PRIVATECCHP_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_PRIVATECCHP_SET_CNF;
    msg->apdup2 = cchpCmd->apdup2;
    msg->aidLen = cchpCmd->aidLen;

    if (cchpCmd->aidLen != 0) {
        ret = memcpy_s(msg->adfName, sizeof(msg->adfName), cchpCmd->adfName, cchpCmd->aidLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_PrivateCchpSetReq: VOS_SendMsg FAILED.");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 关闭逻辑通道请求
 * 修改历史:
 * 1. 日    期: 2013年05月15日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CchcSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT32 sessionID)
{
    SI_PIH_CchcSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CchcSetReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CchcSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_CchcSetReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CchcSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_CCHC_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_CCHC_SET_CNF;
    msg->sessionId = sessionID;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CchcSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 透传逻辑通道APDU接口函数
 * 修改历史:
 * 1. 日    期: 2013年05月15日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CglaSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PIH_CglaCommand *data)
{
    SI_PIH_CglaReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    /* 参数检测 */
    if (data == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CglaSetReq: NULL_PTR.");

        return TAF_FAILURE;
    }

    if ((data->len > (SI_APDU_MAX_LEN + 1)) || (data->len == 0)) {
        PIH_ERROR_LOG("SI_PIH_CglaSetReq:Command length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CglaSetReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CglaReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_CglaReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CglaSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_CGLA_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_CGLA_SET_CNF;
    msg->sessionId = data->sessionID;
    msg->dataLen = data->len;

    ret = memcpy_s(msg->data, sizeof(msg->data), data->command, data->len);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CglaSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 获取卡的ATR内容
 */
VOS_UINT32 SI_PIH_GetCardATRProc(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
   SI_PIH_ReqUint32 reqName, SI_PIH_EventUint32 eventType)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_GetCardATRProc:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_GetCardATRProc: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->msgName = reqName;
    msg->client = clientId;
    msg->opId = opId;
    msg->eventType = eventType;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_GetCardATRProc:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 获取卡的ATR内容
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_GetCardATRReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return SI_PIH_GetCardATRProc(clientId, opId, SI_PIH_CARD_ATR_QRY_REQ, SI_PIH_EVENT_CARD_ATR_QRY_CNF);
}

/*
 * 功能描述: 获取卡的历史ATR内容
*/
MODULE_EXPORTED VOS_UINT32 SI_PIH_GetCardHistoryATRReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return SI_PIH_GetCardATRProc(clientId, opId, SI_PIH_CARD_HISTORY_ATR_QRY_REQ, SI_PIH_EVENT_HISTORY_CARD_ATR_QRY_CNF);
}

/*
 * 功能描述: FDN去激活
 * 修改历史:
 * 1. 日    期: 2013年03月18日
 *    修改内容: Create
 * 2. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，该接口不受宏控制
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_HvSstSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_HvsstSet *hvSStSet)
{
    SI_PIH_HvsstReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    /* 参数检测 */
    if (hvSStSet == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_HvSstSet: NULL_PTR.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_HvSstSet:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_HvsstReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_HvsstReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_HvSstSet:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.msgName = SI_PIH_HVSST_SET_REQ;
    msg->msgHeader.eventType = SI_PIH_EVENT_HVSST_SET_CNF;

    (VOS_VOID)memcpy_s(&msg->hvSSTData, sizeof(SI_PIH_HvsstSet), hvSStSet, sizeof(SI_PIH_HvsstSet));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_HvSstSet:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: HVSST命令查询函数
 * 修改历史:
 * 1. 日    期: 2013年3月18日
 *    修改内容: Create
 * 2. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，该接口不受宏控制
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_HvSstQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_HvSstQuery:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_HvSstQuery:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_HVSST_QUERY_REQ;
    msg->eventType = SI_PIH_EVENT_HVSST_QUERY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_HvSstQuery:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: passthrough上下电(U)SIM卡
 * 修改历史:
 * 1. 日    期: 2020年04月10日
 *    修改内容: 创建
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_PassThroughSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    SI_PIH_PassThroughStateUint32 state)
{
    SI_PIH_PassThroughReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_PassThroughSet: Get receiverPid Error");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_PassThroughReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        sizeof(SI_PIH_PassThroughReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_PassThroughSet: WARNING AllocMsg failed");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.msgName = SI_PIH_PASSTHROUGH_SET_REQ;
    msg->msgHeader.eventType = SI_PIH_EVENT_PASSTHROUGH_SET_CNF;
    msg->state = state;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_PassThroughSet: WARNING SendMsg failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: passthrough上下电状态查询
 * 修改历史:
 * 1. 日    期: 2020年04月10日
 *    修改内容: 创建
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_PassThroughQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_PassThroughQuery: Get receiverPid Error");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_PassThroughQuery: WARNING AllocMsg failed");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_PASSTHROUGH_QUERY_REQ;
    msg->eventType = SI_PIH_EVENT_PASSTHROUGH_QUERY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_PassThroughQuery: WARNING SendMsg failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
/*
 * 功能描述: 单MODEM多卡槽场景，设置卡槽与USIM任务对应关系
 * 修改历史:
 * 1. 日    期: 2020年05月22日
 *    修改内容: 创建
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_SingleModemDualSlotSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    SI_PIH_CardSlotUint32 slotId)
{
    SI_PIH_SingleModemDualSlotReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SingleModemDualSlotSet: Get receiverPid Error");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_SingleModemDualSlotReqMsg *)VOS_AllocMsg(WUEPS_PID_AT,
        sizeof(SI_PIH_SingleModemDualSlotReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_SingleModemDualSlotSet: WARNING AllocMsg failed");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.msgName = SI_PIH_SINGLEMODEMDUALSLOT_SET_REQ;
    msg->msgHeader.eventType = SI_PIH_EVENT_SINGLEMODEMDUALSLOT_SET_CNF;
    msg->slotId = slotId;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_SingleModemDualSlotSet: WARNING SendMsg failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 单MODEM多卡槽场景，查询卡槽与USIM任务对应关系
 * 修改历史:
 * 1. 日    期: 2020年05月22日
 *    修改内容: 创建
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_SingleModemDualSlotQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SingleModemDualSlotQuery: Get receiverPid Error");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_SingleModemDualSlotQuery: WARNING AllocMsg failed");
        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_SINGLEMODEMDUALSLOT_QUERY_REQ;
    msg->eventType = SI_PIH_EVENT_SINGLEMODEMDUALSLOT_QUERY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_SingleModemDualSlotQuery: WARNING SendMsg failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
#endif

/*
 * 功能描述: SCICFG Set
 * 修改历史:
 * 1. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_SciCfgSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PIH_CardSlotUint32 card0Slot,
    SI_PIH_CardSlotUint32 card1Slot, SI_PIH_CardSlotUint32 card2Slot)
{
    SI_PIH_ScicfgSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SciCfgSet:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    msg = (SI_PIH_ScicfgSetReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_ScicfgSetReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_SciCfgSet:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.msgName = SI_PIH_SCICFG_SET_REQ;
    msg->msgHeader.eventType = SI_PIH_EVENT_SCICFG_SET_CNF;

    msg->card0Slot = card0Slot;
    msg->card1Slot = card1Slot;
    msg->card2Slot = card2Slot;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_SciCfgSet:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: SCICFG Query
 * 修改历史:
 * 1. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，新增
 */

MODULE_EXPORTED VOS_UINT32 SI_PIH_SciCfgQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SciCfgQuery:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_SciCfgQuery:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_SCICFG_QUERY_REQ;
    msg->eventType = SI_PIH_EVENT_SCICFG_QUERY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_SciCfgQuery:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: BWT Set
 * 修改历史:
 * 1. 日    期: 2019年3月28日
 *    修改内容: 发电PIN问题上层需要动态配置60时长
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_BwtSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT16 protectTime)
{
    SI_PIH_BwtSetReq *bwtSetMsg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_BwtSet:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    bwtSetMsg = (SI_PIH_BwtSetReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_BwtSetReq) - VOS_MSG_HEAD_LENGTH);
    if (bwtSetMsg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_BwtSet:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(bwtSetMsg->msgHeader), receiverPid);
    bwtSetMsg->msgHeader.client = clientId;
    bwtSetMsg->msgHeader.opId = opId;
    bwtSetMsg->msgHeader.msgName = SI_PIH_BWT_SET_REQ;
    bwtSetMsg->msgHeader.eventType = SI_PIH_EVENT_BWT_SET_CNF;

    bwtSetMsg->protectTime = protectTime;

    PAM_OM_ReportLayerMsg(bwtSetMsg);
    if (VOS_SendMsg(WUEPS_PID_AT, bwtSetMsg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_BwtSet:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
/*
 * 功能描述: CARDAPPAID Set
 * 修改历史:
 * 1. 日    期: 2020年7月13日
 *    修改内容:
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CardAppAidSet(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    const SI_PIH_CardInitAppAidCfg *cardAppAidPara)
{
    SI_PIH_CardAppAidSetReq *cardAppAidSetMsg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (cardAppAidPara == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CardAppAidSet: NULL PTR.");
        return TAF_FAILURE;
    }

    if ((cardAppAidPara->aidLen == 0) || (cardAppAidPara->aidLen > USIMM_AID_LEN_MAX)) {
        PIH_ERROR_LOG("SI_PIH_CardAppAidSet: AID length is incorrect.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardAppAidSet:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    cardAppAidSetMsg = (SI_PIH_CardAppAidSetReq *)VOS_AllocMsg(WUEPS_PID_AT,
        sizeof(SI_PIH_CardAppAidSetReq) - VOS_MSG_HEAD_LENGTH);
    if (cardAppAidSetMsg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CardAppAidSet:WARNING AllocMsg FAILED");
        return TAF_FAILURE;
    }

    /* 填充消息 */
    VOS_SET_RECEIVER_ID(&(cardAppAidSetMsg->msgHeader), receiverPid);
    cardAppAidSetMsg->msgHeader.client = clientId;
    cardAppAidSetMsg->msgHeader.opId = opId;
    cardAppAidSetMsg->msgHeader.msgName = SI_PIH_CARD_APP_AID_SET_REQ;
    cardAppAidSetMsg->msgHeader.eventType = SI_PIH_EVENT_CARD_APP_AID_SET_CNF;
    cardAppAidSetMsg->enable = cardAppAidPara->enable;
    cardAppAidSetMsg->appType = cardAppAidPara->appType;
    cardAppAidSetMsg->aidLen = cardAppAidPara->aidLen;
    if (memcpy_s(cardAppAidSetMsg->aid, sizeof(cardAppAidSetMsg->aid), cardAppAidPara->aid,
        cardAppAidPara->aidLen) != EOK) {
        PIH_WARNING_LOG("SI_PIH_CardAppAidSet:WARNING memcpy_s error");
        (VOS_VOID)VOS_FreeMsg(WUEPS_PID_AT, cardAppAidSetMsg);
        return TAF_FAILURE;
    }

    PAM_OM_ReportLayerMsg(cardAppAidSetMsg);
    if (VOS_SendMsg(WUEPS_PID_AT, cardAppAidSetMsg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CardAppAidSet:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
#endif

/*
 * 功能描述: HVTEE Set
 * 修改历史:
 * 1. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，新增
 */
VOS_VOID SI_PIH_AcpuInit(VOS_VOID)
{
#if (FEATURE_ON == FEATURE_VSIM)
#ifdef CONFIG_TZDRIVER
    VOS_UINT8 uuid[] = {
        0x47, 0x91, 0xe8, 0xab, 0x61, 0xcd, 0x3f, 0xf4, 0x71, 0xc4, 0x1a, 0x31, 0x7e, 0x40, 0x53, 0x12
    };

    if (TC_NS_RegisterServiceCallbackFunc((VOS_CHAR *)uuid, SI_PIH_TEETimeOutCB, VOS_NULL_PTR) != VOS_OK) {
        mdrv_err("<SI_PIH_AcpuInit> Reg TEE Timeout CB FUN Fail\n");
    }

    mdrv_debug("<SI_PIH_AcpuInit> Reg TEE Timeout CB FUN Ok\n");
#endif /* CONFIG_TZDRIVER */
#endif /* (FEATURE_ON == FEATURE_VSIM) */

    return;
}

#if (FEATURE_VSIM == FEATURE_ON)
/*
 * 修改历史:
 * 1. 日    期: 2016年6月6日
 * 修改内容: 新生成函数
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_GetSecIccVsimVer(VOS_VOID)
{
    return SI_PIH_SEC_ICC_VSIM_VER;
}

#ifdef CONFIG_TZDRIVER
/*
 * 功能描述: HVTEE Set
 * 修改历史:
 * 1. 日    期: 2014年10月9日
 *    修改内容: 根据青松产品要求，新增
 */
VOS_VOID SI_PIH_TEETimeOutCB(VOS_VOID *timerDataCb)
{
    TEEC_TIMER_Property *timerData = VOS_NULL_PTR;
    MN_APP_PihAtCnf *msg = VOS_NULL_PTR;
    VOS_UINT32 result;

    if (timerDataCb == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_TEETimeOutCB: para error!");

        return;
    }

    msg = (MN_APP_PihAtCnf *)VOS_AllocMsg(MAPS_PIH_PID, sizeof(MN_APP_PihAtCnf) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_TEETimeOutCB: Alloc Msg Failed!");

        return;
    }

    timerData = (TEEC_TIMER_Property *)timerDataCb;

    msg->pihAtEvent.eventType = SI_PIH_EVENT_TEETIMEOUT_IND;

    msg->pihAtEvent.pihError = TAF_ERR_NO_ERROR;

    msg->pihAtEvent.pihEvent.teeTimeOut.data = timerData->timeType;

    VOS_SET_RECEIVER_ID(msg, WUEPS_PID_AT);

    msg->msgId = PIH_AT_EVENT_CNF;

    msg->pihAtEvent.clientId = (MN_CLIENT_ALL & AT_BROADCAST_CLIENT_ID_MODEM_0);

    PAM_OM_ReportLayerMsg(msg);
    result = VOS_SendMsg(MAPS_PIH_PID, msg);

    PAM_PRINT_NORMALFUN_RESULT(result);

    return;
}
#endif /* CONFIG_TZDRIVER */

/*
 * 功能描述: 内容查询命令
 * 修改历史:
 * 1. 日    期: 2013年3月26日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_HvCheckCardQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return TAF_SUCCESS;
}

#endif /* end of (FEATURE_VSIM == FEATURE_ON) */

#if (FEATURE_ON == FEATURE_IMS)
/*
 * 功能描述: GBA鉴权请求函数
 * 修改历史:
 * 1. 日    期: 2014年3月21日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_UiccAuthReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_UiccAuth *data)
{
    SI_PIH_UiccauthReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_UiccAuthReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    if (data == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_UiccAuthReq:Para Check Error");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_UiccauthReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_UiccauthReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_UiccAuthReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_UICCAUTH_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_UICCAUTH_CNF;

    (VOS_VOID)memcpy_s(&msg->authData, sizeof(SI_PIH_UiccAuth), data, sizeof(SI_PIH_UiccAuth));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_UiccAuthReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 修改历史:
 * 1. 日    期: 2014年3月21日
 * 修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_AccessUICCFileReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_AccessFile *data)
{
    SI_PIH_AccessfileReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_AccessUICCFileReq:Get receiverPid Error.");

        return TAF_FAILURE;
    }

    if (data == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_AccessUICCFileReq:Para Check Error");

        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_AccessfileReq *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)(sizeof(SI_PIH_AccessfileReq) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_AccessUICCFileReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_URSM_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_URSM_CNF;

    (VOS_VOID)memcpy_s(&msg->cmdData, sizeof(SI_PIH_AccessFile), data, sizeof(SI_PIH_AccessFile));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_AccessUICCFileReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
#endif /* (FEATURE_ON == FEATURE_IMS) */

/*
 * 修改历史:
 * 1. 日    期: 2014年06月04日
 * 修改内容: Create
 */
VOS_UINT32 SI_PIH_CardTypeQueryProc(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT32 msgName,
    SI_PIH_EventUint32 eventType)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardTypeQuery: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, (VOS_UINT32)sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CardTypeQuery: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = msgName;
    msg->eventType = eventType;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CardTypeQuery: WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 查询CardTypeEx状态处理
 * 修改历史:
 * 1. 日    期: 2017年8月2日
 *    修改内容: AT^CardTypeEx新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CardTypeExQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return SI_PIH_CardTypeQueryProc(clientId, opId, SI_PIH_CARDTYPEEX_QUERY_REQ, SI_PIH_EVENT_CARDTYPEEX_QUERY_CNF);
}

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
/*
 * 功能描述: 获取加密PIN信息
 * 修改历史:
 * 1. 日    期: 2017年8月2日
 *    修改内容: AT^SilentPinInfo新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_GetSilentPinInfoReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const VOS_UINT8 *pin,
    VOS_UINT32 len)
{
    SI_PIH_SilentPininfoReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;
    VOS_UINT32 result;

    if ((pin == VOS_NULL_PTR) || (len > USIMM_PINNUMBER_LEN) || (len == 0)) {
        PIH_ERROR_LOG("SI_PIH_GetSilentPinInfoReq:PIN's len Error.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_GetSilentPinInfoReq: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_SilentPininfoReq *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)sizeof(SI_PIH_SilentPininfoReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_GetSilentPinInfoReq: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_SILENT_PININFO_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_SILENT_PININFO_SET_CNF;
    msg->dataLen = USIMM_PINNUMBER_LEN;

    (VOS_VOID)memset_s(msg->data, USIMM_PINNUMBER_LEN, 0xff, USIMM_PINNUMBER_LEN);

    ret = memcpy_s(msg->data, USIMM_PINNUMBER_LEN, pin, USIMM_PINNUMBER_LEN);

    PAM_PRINT_SECFUN_RESULT(ret);

    if (VOS_ReserveMsg(WUEPS_PID_AT, (MsgBlock *)msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_GetSilentPinInfoReq:reserve Msg FAILED");

        /* 清空跨核消息中含有的敏感信息 */
        (VOS_VOID)memset_s(msg->data, USIMM_PINNUMBER_LEN, 0XFF, USIMM_PINNUMBER_LEN);

        result = VOS_FreeMsg(WUEPS_PID_AT, msg);

        PAM_PRINT_NORMALFUN_RESULT(result);

        return TAF_FAILURE;
    }

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_GetSilentPinInfoReq:WARNING SendMsg FAILED");

        /* 清空跨核消息中含有的敏感信息 */
        (VOS_VOID)memset_s(msg->data, USIMM_PINNUMBER_LEN, 0XFF, USIMM_PINNUMBER_LEN);

        result = VOS_FreeReservedMsg(WUEPS_PID_AT, msg);

        PAM_PRINT_NORMALFUN_RESULT(result);

        return TAF_FAILURE;
    }

    /* 清空跨核消息中含有的敏感信息 */
    (VOS_VOID)memset_s(msg->data, USIMM_PINNUMBER_LEN, 0XFF, USIMM_PINNUMBER_LEN);

    result = VOS_FreeReservedMsg(WUEPS_PID_AT, msg);

    PAM_PRINT_NORMALFUN_RESULT(result);

    return TAF_SUCCESS;
}

/*
 * 功能描述: 通过加密PIN信息解锁PIN
 * 修改历史:
 * 1. 日    期: 2017年8月2日
 *    修改内容: AT^SilentPin新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_SetSilentPinReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CryptoPin *cryptoPin)
{
    SI_PIH_SilentPinReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (cryptoPin == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_SetSilentPinReq: pstCryptoPin is null.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SetSilentPinReq: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_SilentPinReq *)VOS_AllocMsg(WUEPS_PID_AT,
        (VOS_UINT32)sizeof(SI_PIH_SilentPinReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_SetSilentPinReq: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_SILENT_PIN_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_SILENT_PIN_SET_CNF;

    (VOS_VOID)memcpy_s(&(msg->cryptoPin), sizeof(SI_PIH_CryptoPin), cryptoPin, sizeof(SI_PIH_CryptoPin));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_SetSilentPinReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
#endif

/*
 * 功能描述: 设置卡槽对应电路状态
 * 修改历史:
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_EsimSwitchSetFunc(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    const SI_PIH_EsimSwitchSet *esimSwitchSet)
{
    SI_PIH_EsimswitchReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (esimSwitchSet == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_EsimSwitchSetFunc:Parameter is Wrong");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_EsimSwitchSetFunc:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_EsimswitchReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_EsimswitchReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_EsimSwitchSetFunc:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.msgName = SI_PIH_ESIMSWITCH_SET_REQ;
    msg->msgHeader.eventType = SI_PIH_EVENT_ESIMSWITCH_SET_CNF;

    msg->esimSwitchData.slot = esimSwitchSet->slot;
    msg->esimSwitchData.cardType = esimSwitchSet->cardType;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_EsimSwitchSetFunc:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 查询卡槽对应电路状态
 * 修改历史:
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_EsimSwitchQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_EsimSwitchQuery:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_EsimSwitchQuery:WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_ESIMSWITCH_QRY_REQ;
    msg->eventType = SI_PIH_EVENT_ESIMSWITCH_QRY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_EsimSwitchQuery:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 查询CardType状态处理
 * 修改历史:
 * 1. 日    期: 2017年8月2日
 *    修改内容: AT^CardTypeEx修改
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CardTypeQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return SI_PIH_CardTypeQueryProc(clientId, opId, SI_PIH_CARDTYPE_QUERY_REQ, SI_PIH_EVENT_CARDTYPE_QUERY_CNF);
}

/*
 * 功能描述: 获取voltage信息
 * 修改历史:
 * 1. 日    期: 2017年01月18日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_CardVoltageQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardVoltageQuery: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, (VOS_UINT32)(sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CardVoltageQuery: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_CARDVOLTAGE_QUERY_REQ;
    msg->eventType = SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CardVoltageQuery: WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 透传逻辑通道APDU接口函数
 * 修改历史:
 * 1. 日    期: 2017年02月21日
 *    修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_PIH_PrivateCglaSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_CglaCommand *data)
{
    SI_PIH_CglaReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;
    errno_t ret;

    if (data == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_PrivateCglaSetReq:PIN NULL.");
        return TAF_FAILURE;
    }

    if ((data->len > (SI_APDU_MAX_LEN + 1)) || (data->len == 0)) {
        PIH_ERROR_LOG("SI_PIH_CglaSetReq:Command length is incorrect.");

        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CglaHandleReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    msg = (SI_PIH_CglaReq *)VOS_AllocMsg(WUEPS_PID_AT, (VOS_UINT32)(sizeof(SI_PIH_CglaReq) - VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CglaHandleReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(msg->msgHeader), receiverPid);
    msg->msgHeader.msgName = SI_PIH_PRIVATECGLA_SET_REQ;
    msg->msgHeader.client = clientId;
    msg->msgHeader.opId = opId;
    msg->msgHeader.eventType = SI_PIH_EVENT_PRIVATECGLA_SET_CNF;
    msg->sessionId = data->sessionID;
    msg->dataLen = data->len;

    ret = memcpy_s(msg->data, sizeof(msg->data), data->command, data->len);

    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CglaHandleReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

MODULE_EXPORTED VOS_UINT32 SI_PIH_CrsmSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_Crsm *crsmPara)
{
    SI_PIH_CrsmSetReqMsg *crsmMsg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (crsmPara == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CrsmSetReq: pstCrsmPara is null.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CrsmSetReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    crsmMsg = (SI_PIH_CrsmSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_CrsmSetReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (crsmMsg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CrsmSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(crsmMsg->msgHeader), receiverPid);
    crsmMsg->msgHeader.msgName = SI_PIH_CRSM_SET_REQ;
    crsmMsg->msgHeader.client = clientId;
    crsmMsg->msgHeader.opId = opId;
    crsmMsg->msgHeader.eventType = SI_PIH_EVENT_CRSM_SET_CNF;

    (VOS_VOID)memcpy_s(&(crsmMsg->msgContent), sizeof(SI_PIH_Crsm), crsmPara, sizeof(SI_PIH_Crsm));

    PAM_OM_ReportLayerMsg(crsmMsg);
    if (VOS_SendMsg(WUEPS_PID_AT, crsmMsg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CrsmSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

MODULE_EXPORTED VOS_UINT32 SI_PIH_CrlaSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_PIH_Crla *crlaPara)
{
    SI_PIH_CrlaSetReqMsg *crlaMsg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (crlaPara == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_CrlaSetReq: pstCrlaPara is null.");
        return TAF_FAILURE;
    }

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CrlaSetReq:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    /* 分配消息内存 */
    crlaMsg = (SI_PIH_CrlaSetReqMsg *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_CrlaSetReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (crlaMsg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CrlaSetReq: AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(&(crlaMsg->msgHeader), receiverPid);
    crlaMsg->msgHeader.msgName = SI_PIH_CRLA_SET_REQ;
    crlaMsg->msgHeader.client = clientId;
    crlaMsg->msgHeader.opId = opId;
    crlaMsg->msgHeader.eventType = SI_PIH_EVENT_CRLA_SET_CNF;

    (VOS_VOID)memcpy_s(&(crlaMsg->msgContent), sizeof(SI_PIH_Crla), crlaPara, sizeof(SI_PIH_Crla));

    PAM_OM_ReportLayerMsg(crlaMsg);
    if (VOS_SendMsg(WUEPS_PID_AT, crlaMsg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CchcSetReq:WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

MODULE_EXPORTED VOS_UINT32 SI_PIH_CardSessionQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardSessionQuery: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CardSessionQuery: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_SESSION_QRY_REQ;
    msg->eventType = SI_PIH_EVENT_SESSION_QRY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CardSessionQuery: WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

MODULE_EXPORTED VOS_UINT32 SI_PIH_CimiSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardSessionQuery: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CimiSetReq: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_CIMI_QRY_REQ;
    msg->eventType = SI_PIH_EVENT_CIMI_QRY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CimiSetReq: WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

MODULE_EXPORTED VOS_UINT32 SI_PIH_CCimiSetReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    SI_PIH_MsgHeader *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_CardSessionQuery: Get receiverPid Error.");
        return TAF_FAILURE;
    }

    msg = (SI_PIH_MsgHeader *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PIH_MsgHeader) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PIH_WARNING_LOG("SI_PIH_CardSessionQuery: WARNING AllocMsg FAILED");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->client = clientId;
    msg->opId = opId;
    msg->msgName = SI_PIH_CCIMI_QRY_REQ;
    msg->eventType = SI_PIH_EVENT_CCIMI_QRY_CNF;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PIH_WARNING_LOG("SI_PIH_CardSessionQuery: WARNING SendMsg FAILED");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/*
 * 功能描述: 向PIH发NOCARD设置请求
 * 修改历史:
 * 1. 日    期: 2020年05月21日
 *    修改内容: Create
 */
VOS_UINT32 SI_PIH_SendNoCardReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T operationId, VOS_UINT32 setValue)
{
    SI_PIH_SetNoCardReq *noCardMsg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PIH_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SendNoCardReq: SI_PIH_GetReceiverPid Error");
        return TAF_FAILURE;
    }

    /* 申请消息内存 */
    noCardMsg = (SI_PIH_SetNoCardReq *)VOS_AllocMsg(WUEPS_PID_AT, (sizeof(SI_PIH_SetNoCardReq) - VOS_MSG_HEAD_LENGTH));
    if (noCardMsg == VOS_NULL_PTR) {
        PIH_ERROR_LOG("SI_PIH_SendNoCardReq: Alloc Memory Fail");
        return TAF_FAILURE;
    }

    /* 填写消息 */
    VOS_SET_RECEIVER_ID(noCardMsg, receiverPid);
    noCardMsg->msgHeader.client = clientId;
    noCardMsg->msgHeader.opId = operationId;
    noCardMsg->msgHeader.msgName = SI_PIH_SET_NOCARD_REQ;
    noCardMsg->msgHeader.eventType = SI_PIH_EVENT_NOCARD_SET_CNF;
    noCardMsg->noCardMode = setValue;

    PAM_OM_ReportLayerMsg(noCardMsg);
    if (VOS_SendMsg(WUEPS_PID_AT, noCardMsg) != VOS_OK) {
        PIH_ERROR_LOG("SI_PIH_SendNoCardReq: Send Msg Fail");
        return TAF_FAILURE;
    }
    return TAF_SUCCESS;
}
#endif
#endif

