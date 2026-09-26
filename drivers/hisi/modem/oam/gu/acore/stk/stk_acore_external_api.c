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
 * 版权所有（c）华为技术有限公司 2008-2019
 * 功能描述: 该文件定义了STK模块的对外接口
 * 日    期: 2008年12月28日
 */

#include "si_app_stk.h"
#include "si_stk.h"
#include "product_config.h"
#include "mn_client.h"
#include "si_acore_api.h"
#include "nv_id_pam.h"

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) && (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF))
#include "pam_app_om.h"
#else
#include "pam_om.h"
#endif

#include "msp_diag_comm.h"

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_STK_API_C
#define THIS_MODU mod_pam_stk

#pragma pack(push, 4)

#define STK_GEN_LOG_MODULE(Level) (MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, Level))

#define STK_KEY_INFO_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define STK_KEY_NORMAL_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define STK_KEY_WARNING_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string)
#define STK_KEY_ERROR_LOG(string)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string)

#define STK_KEY_INFO_LOG1(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define STK_KEY_NORMAL_LOG1(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define STK_KEY_WARNING_LOG1(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string "%d", para1)
#define STK_KEY_ERROR_LOG1(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string "%d", para1)

#define STK_INFO_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define STK_NORMAL_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string)
#define STK_WARNING_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string)
#define STK_ERROR_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string)

#define STK_INFO_LOG1(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define STK_NORMAL_LOG1(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "NORMAL:" string "%d", para1)
#define STK_WARNING_LOG1(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string "%d", para1)
#define STK_ERROR_LOG1(string, para1)                                                                               \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "ERROR:" string "%d", para1)

#define STK_WARNING_NOSLOTID_LOG(string)                                                                              \
    (VOS_VOID)mdrv_diag_log_report(STK_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), I0_MAPS_STK_PID, VOS_NULL_PTR, __LINE__, \
        "WARNING:" string)

/*
 * 功能描述: FDN激活
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
VOS_UINT32 SI_STK_GetReceiverPid(MN_CLIENT_ID_T clientId, VOS_UINT32 *receiverPid)
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
        *receiverPid = I0_MAPS_STK_PID;
    } else if (slotId == SI_PIH_CARD_SLOT_1) {
        *receiverPid = I1_MAPS_STK_PID;
    } else {
        *receiverPid = I2_MAPS_STK_PID;
    }
#else
    *receiverPid = I0_MAPS_STK_PID;
#endif
    return VOS_OK;
}

/*
 * 功能描述: 获取STK功能的主菜单
 * 修改历史:
 * 1.日    期: 2009年7月6日
 *   修改内容: Create
 */
VOS_UINT32 SI_STK_SendReqMsg(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT32 msgName, VOS_UINT32 cmdType,
    const USIMM_U8LvData *sendData)
{
    SI_STK_Req *sTKReq = VOS_NULL_PTR;
    VOS_UINT32 sendPid;
    VOS_UINT32 receiverPid;
    errno_t ret;

    if (SI_STK_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        STK_ERROR_LOG("SI_STK_SendReqMsg:Get receiverPid Error.");
        return TAF_FAILURE;
    }

    sendPid = WUEPS_PID_AT;

    sTKReq = (SI_STK_Req *)VOS_AllocMsg(sendPid, sizeof(SI_STK_Req) - VOS_MSG_HEAD_LENGTH + sendData->dataLen);
    if (sTKReq == VOS_NULL_PTR) {
        STK_ERROR_LOG("SI_STK_SendReqMsg: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    sTKReq->msgName = msgName;
    VOS_SET_RECEIVER_ID(sTKReq, receiverPid);
    sTKReq->opId = opId;
    sTKReq->clientId = clientId;
    sTKReq->satType = cmdType;
    sTKReq->datalen = sendData->dataLen;

    if (sendData->dataLen != 0) {
        ret = memcpy_s(sTKReq->data, sendData->dataLen, sendData->data, sendData->dataLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    PAM_OM_ReportLayerMsg(sTKReq);
    if (VOS_SendMsg(sendPid, sTKReq) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * 功能描述: 获取STK功能的主菜单
 * 修改历史:
 * 1.日    期: 2009年7月6日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_GetMainMenu(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    USIMM_U8LvData sendData;

    sendData.dataLen = 0;
    sendData.data = VOS_NULL_PTR;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_GETMAINMNUE, SI_STK_NOCMDDATA, &sendData);
}

/*
 * 功能描述: 获取缓存的STK主动命令
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_GetSTKCommand(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_STK_CmdTypeUint32 cmdType)
{
    USIMM_U8LvData sendData;

    sendData.dataLen = 0;
    sendData.data = VOS_NULL_PTR;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_GETCOMMAND, cmdType, &sendData);
}

/*
 * 功能描述: 查询最后一次SIM卡上报的主动命令
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_QuerySTKCommand(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_USIM == FEATURE_ON))
    return VOS_ERR;
#else
    USIMM_U8LvData sendData;

    sendData.dataLen = 0;
    sendData.data = VOS_NULL_PTR;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_QUERYCOMMAND, SI_STK_NOCMDDATA, &sendData);
#endif
}

/*
 * 功能描述: 终端给予下发数据函数
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_DataSendSimple(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_SendDataTypeUint32 sendType,
    VOS_UINT32 dataLen, const VOS_UINT8 *data)
{
    USIMM_U8LvData sendData;

    if ((dataLen != 0) && (data == VOS_NULL_PTR)) {
        STK_ERROR_LOG("SI_STK_DataSendSimple:para error.");
        return VOS_ERR;
    }

    sendData.dataLen = dataLen;
    sendData.data = (VOS_UINT8 *)data;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_SIMPLEDOWN, sendType, &sendData);
}

/*
 * 功能描述: 终端给予响应函数
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_TerminalResponse(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_STK_TerminalRsp *tRStru)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_USIM == FEATURE_ON))
    return VOS_ERR;
#else
    USIMM_U8LvData sendData;

    if (tRStru == VOS_NULL_PTR) {
        STK_ERROR_LOG("SI_STK_TerminalResponse: The input parameter is null.");

        return VOS_ERR;
    }

    sendData.dataLen = sizeof(SI_STK_TerminalRsp);
    sendData.data = (VOS_UINT8 *)tRStru;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_TRDOWN, SI_STK_NOCMDDATA, &sendData);
#endif
}

/*
 * 功能描述: 触发IMSI切换流程，沃达丰定制流程
 * 修改历史:
 * 1.日    期: 2010年02月10日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STKDualIMSIChangeReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_USIM == FEATURE_ON))
    return TAF_FAILURE;
#else
    SI_STK_Req *sTKReq = VOS_NULL_PTR;

    sTKReq = (SI_STK_Req *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_STK_Req) - VOS_MSG_HEAD_LENGTH);
    if (sTKReq == VOS_NULL_PTR) {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    sTKReq->clientId = clientId;
    sTKReq->opId = opId;
    sTKReq->msgName = SI_STK_IMSICHG;
    VOS_SET_RECEIVER_ID(sTKReq, I0_MAPS_STK_PID);

    PAM_OM_ReportLayerMsg(sTKReq);
    if (VOS_SendMsg(I0_MAPS_STK_PID, sTKReq) != VOS_OK) {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_SendMsg Return Error");
        return VOS_ERR;
    }

    return VOS_OK;
#endif
}

/*
 * 功能描述: 判断是否支持Dual IMSI切换
 * 修改历史:
 * 1.日    期: 2010年02月11日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STKIsDualImsiSupport(VOS_VOID)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_USIM == FEATURE_ON))
    return TAF_FAILURE;
#else
    VOS_UINT16 dualIMSIFlag = 0;

    if (mdrv_nv_read(NV_ITEM_NV_HUAWEI_DOUBLE_IMSI_CFG_I, &dualIMSIFlag, sizeof(VOS_UINT16)) != NV_OK) {
        STK_WARNING_LOG("STK_InitGobal: Read NV_ITEM_NV_HUAWEI_DOUBLE_IMSI_CFG_I Fail");
    }

    /* 前后两个自节均为1，Dual IMSI功能才开启，第一个字节为NV激活标志，第二个为使能位 */
    if (dualIMSIFlag == STK_NV_ENABLED) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
#endif
}

/*
 * 功能描述: STK功能ENVELOP下载函数
 * 修改历史:
 * 1.日    期: 2008年10月18日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_MenuSelectionFunc(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const SI_STK_Envelope *eNStru)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_USIM == FEATURE_ON))
    return TAF_FAILURE;
#else
    USIMM_U8LvData sendData;

    if ((eNStru == VOS_NULL_PTR) || (eNStru->envelopeType != SI_STK_ENVELOPE_MENUSEL)) {
        STK_ERROR_LOG("SI_STK_MenuSelectionFunc: The Input Data is Error");

        return VOS_ERR;
    }

    sendData.dataLen = sizeof(SI_STK_Envelope);
    sendData.data = (VOS_UINT8 *)eNStru;

    return SI_STK_SendReqMsg(clientId, opId, SI_STK_MENUSELECTION, eNStru->envelopeType, &sendData);
#endif
}

/*
 * 功能描述: 用户回复是否允许发起呼叫
 * 修改历史:
 * 1.日    期: 2012年9月14日
 *   修改内容: Create
 */
MODULE_EXPORTED VOS_UINT32 SI_STK_SetUpCallConfirm(MN_CLIENT_ID_T clientId, SI_STK_SetupCallConfirmUint32 action)
{
    USIMM_U8LvData sendData;

    /* 参数检查 */
    if (action >= SI_STK_SETUPCALL_BUTT) {
        STK_ERROR_LOG("SI_STK_SetUpCallConfirm: The Input Para is Error");

        return VOS_ERR;
    }

    sendData.dataLen = sizeof(VOS_UINT32);
    sendData.data = (VOS_UINT8 *)&action;

    return SI_STK_SendReqMsg(clientId, 0, SI_STK_SETUPCALL_CONFIRM, SI_STK_SETUPCALL, &sendData);
}

#pragma pack(pop)

