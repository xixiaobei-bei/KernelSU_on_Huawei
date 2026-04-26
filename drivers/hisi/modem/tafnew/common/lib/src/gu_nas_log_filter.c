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

#include "gu_nas_log_filter.h"
#include "mn_msg_api.h"

#include "taf_tafm_remote.h"
#include "at_mta_interface.h"
#include "taf_msg_ts.h"
#include "ps_log_filter_interface.h"
#include "at_imsa_interface.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "msg_imsa_interface.h"
#include "nas_rrc_interface.h"
#include "usimm_ps_interface.h"
#include "taf_spm_snd_internal_msg.h"
#include "mm_ss_interface.h"
#include "nas_mml_ctx.h"
#include "nas_mml_lib.h"
#include "taf_comm.h"
#include "nas_multi_instance_api.h"
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#include "at_internal_msg.h"
#include "AtParse.h"
#include "at_ctx.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#include "taf_v2x_api.h"
#endif
#endif
#include "securec.h"
#include "taf_msg_chk_api.h"


#define THIS_FILE_ID PS_FILE_ID_GU_NAS_LOG_FILTER_C

#define AT_CMD_LEN_7 7
#define AT_CMD_LEN_8 8
#define AT_CMD_LEN_13 13

typedef VOS_VOID* (*pGuNasMsgFilterProcFunc)(PS_MsgHeader *pstMsg);



typedef struct {
    VOS_UINT32              ulSenderPid;
    VOS_UINT32              ulReceiverPid;
    pGuNasMsgFilterProcFunc pFuncFilterProc;
} GUNAS_MSG_FILTER_PROC_TBL_FUNC;

static const GUNAS_MSG_FILTER_PROC_TBL_FUNC g_guNasMsgFilterProcFuncTbl[] = {
#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
    { WUEPS_PID_AT, WUEPS_PID_AT, GUNAS_FilterAtToAtMsg },
#endif

    { WUEPS_PID_AT, I0_WUEPS_PID_MMA, GUNAS_FilterAtToMmaMsg },
    { WUEPS_PID_AT, I0_UEPS_PID_MTA, GUNAS_FilterAtToMtaMsg },
    { WUEPS_PID_AT, I0_MAPS_PIH_PID, GUNAS_FilterAtToPihMsg },

#if (1 < MULTI_MODEM_NUMBER)
    { WUEPS_PID_AT, I1_WUEPS_PID_MMA, GUNAS_FilterAtToMmaMsg },
    { WUEPS_PID_AT, I1_UEPS_PID_MTA, GUNAS_FilterAtToMtaMsg },
    { WUEPS_PID_AT, I1_MAPS_PIH_PID, GUNAS_FilterAtToPihMsg },

#if (2 < MULTI_MODEM_NUMBER)
    { WUEPS_PID_AT, I2_WUEPS_PID_MMA, GUNAS_FilterAtToMmaMsg },
    { WUEPS_PID_AT, I2_UEPS_PID_MTA, GUNAS_FilterAtToMtaMsg },
    { WUEPS_PID_AT, I2_MAPS_PIH_PID, GUNAS_FilterAtToPihMsg },
#endif
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    { WUEPS_PID_WRR, WUEPS_PID_GMM, GUNAS_FilterWrrToGmmMmMsg },
    { WUEPS_PID_WRR, WUEPS_PID_MM, GUNAS_FilterWrrToGmmMmMsg },
    { UEPS_PID_GAS, WUEPS_PID_MM, GUNAS_FilterGasToMmMsg },

    { TPS_PID_RRC, WUEPS_PID_GMM, GUNAS_FilterWrrToGmmMmMsg },
    { TPS_PID_RRC, WUEPS_PID_MM, GUNAS_FilterWrrToGmmMmMsg },

    { WUEPS_PID_MMA, WUEPS_PID_USIM, GUNAS_FilterMmaToUsimMsg },
    { WUEPS_PID_TAF, WUEPS_PID_TAF, GUNAS_FilterTafToTafMsg },
    { WUEPS_PID_SS, WUEPS_PID_MM, GUNAS_FilterSsToMmMsg },

    { WUEPS_PID_MM, WUEPS_PID_WRR, GUNAS_FilterGmmMmToRrcMsg },
    { WUEPS_PID_MM, TPS_PID_RRC, GUNAS_FilterGmmMmToRrcMsg },
    { WUEPS_PID_MM, UEPS_PID_GAS, GUNAS_FilterGmmMmToRrcMsg },
    { WUEPS_PID_GMM, WUEPS_PID_WRR, GUNAS_FilterGmmMmToRrcMsg },
    { WUEPS_PID_GMM, TPS_PID_RRC, GUNAS_FilterGmmMmToRrcMsg },

#endif
};

#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
static const VOS_CHAR *g_atFileterTable[] = {
    /* USIM相关 */
    "AT+CPIN",
    "\r\n+CPIN:",
    "AT+CLCK",
    "\r\n+CLCK:",
    "AT+CPWD",
    "\r\n+CPWD:",
    "AT^CPIN",
    "\r\n^CPIN:",
    "AT^CPIN2",
    "\r\n^CPIN2:",
    "AT+CPBW",
    "\r\n+CPBW:",
    "AT+CRSM",
    "\r\n+CRSM:",
    "AT+CSIM",
    "\r\n+CSIM:",
    "AT+CCHO",
    "\r\n+CCHO:",
    "AT+CCHC",
    "AT+CNUM",
    "\r\n+CNUM:",
    "AT+CGLA",
    "\r\n+CGLA:",
    "AT+CRLA",
    "\r\n+CRLA:",
    // 删除CIMI
    "AT^CSIN",
    "\r\n^CSIN:",
    "AT^CSTR",
    "\r\n^CSTR:",
    "AT^CSEN",
    "\r\n^CSEN:",
    "\r\n^CCIN:",
    "AT^CISA",
    "\r\n^CISA:",
    "AT^UICCAUTH",
    "\r\n^UICCAUTH:",
    "AT^CURSM",
    "\r\n^CURSM:",
    "AT^CGLA",
    "\r\n^CGLA:",
    "AT^CRLA",
    "\r\n^CRLA:",
    "\r\n^USIMICCID:",

    /* 短信相关 */

    /* SIM LOCK相关 */
    "AT^SIMLOCKUNLOCK",
    "AT^CMLCK",

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
    "AT^SILENTPININFO",
#endif

    "\r\n^ESIMEID:",
    "\r\n^PKID:",
    "\r\n^CARDSTATUSIND:",
};
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_VOID* GUNAS_FilterWrrToGmmMmMsg(PS_MsgHeader *msg)
{
    if (NAS_MML_IsNeedFiltrateL3RcvMsg((RRMM_DataInd *)msg) == VOS_TRUE) {
        return VOS_NULL_PTR;
    }

    return msg;
}

VOS_VOID* GUNAS_FilterGmmMmToRrcMsg(PS_MsgHeader *msg)
{
    if (NAS_MML_IsNeedFiltrateL3ReqMsg((RRMM_DataReq *)msg) == VOS_TRUE) {
        return VOS_NULL_PTR;
    }

    return msg;
}

VOS_VOID* GUNAS_FilterGasToMmMsg(PS_MsgHeader *msg)
{
    if (NAS_MML_IsNeedFiltrateL3RcvMsg((RRMM_DataInd *)msg) == VOS_TRUE) {
        return VOS_NULL_PTR;
    }

    return msg;
}

VOS_VOID* GUNAS_FilterMmaToUsimMsg(PS_MsgHeader *msg)
{
    switch (msg->msgName) {
        case USIMM_PINHANDLE_REQ:
            MN_NORM_LOG1("GUNAS_FilterMmaToUsimMsg: TRUE msgName ", msg->msgName);
            return VOS_NULL_PTR;

        default:
            return msg;
    }
}

VOS_VOID* GUNAS_FilterTafToTafMsg(PS_MsgHeader *msg)
{
    switch (msg->msgName) {
        case TAF_SPM_INTERNAL_SERVICE_CTRL_RESULT_IND:
            MN_NORM_LOG1("GUNAS_FilterTafToTafMsg: TRUE msgName ", msg->msgName);
            return VOS_NULL_PTR;

        default:
            return msg;
    }
}

VOS_VOID* GUNAS_FilterSsToMmMsg(PS_MsgHeader *msg)
{
    MM_SS_DataReq *dataReq = VOS_NULL_PTR;

    if (msg->msgName != MMSS_DATA_REQ) {
        return msg;
    }

    dataReq = (MM_SS_DataReq *)msg;

    if (NAS_MML_IsSsFacilityGetPassWdMsg(dataReq->SsMsg.ssMsg, dataReq->SsMsg.ssMsgSize) == VOS_TRUE) {
        return VOS_NULL_PTR;
    }

    return msg;
}

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)

VOS_UINT32 GUNAS_ATCmdFilter(VOS_UINT8 *aTData, VOS_UINT16 len)
{
    errno_t    memResult;
    VOS_UINT32 i;
    VOS_UINT8 *tempData = VOS_NULL_PTR;

    tempData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, len);
    if (tempData == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    if (len > 0) {
        memResult = memcpy_s(tempData, len, aTData, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    (VOS_VOID)At_UpString(tempData, len);

    for (i = 0; i < (sizeof(g_atFileterTable) / sizeof(g_atFileterTable[0])); i++) {
        if (PS_MEM_CMP((VOS_UINT8 *)g_atFileterTable[i], tempData, VOS_StrLen(g_atFileterTable[i])) == VOS_OK) {
            PS_MEM_FREE(WUEPS_PID_AT, tempData);
            return VOS_TRUE;
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, tempData);
    return VOS_FALSE;
}

VOS_VOID* GUNAS_FilterAtToAtMsg(PS_MsgHeader *msg)
{
    VOS_UINT8 *tmpAdr = VOS_NULL_PTR;
    AT_Msg    *atMsg  = VOS_NULL_PTR;
    VOS_UINT16 len = 0;

    atMsg = (AT_Msg *)msg;

    switch (atMsg->msgId) {
        case ID_AT_MNTN_INPUT_MSC:
        case ID_AT_MNTN_OUTPUT_MSC:
        case ID_AT_MNTN_START_FLOW_CTRL:
        case ID_AT_MNTN_STOP_FLOW_CTRL:
        case ID_AT_MNTN_REG_FC_POINT:
        case ID_AT_MNTN_DEREG_FC_POINT:
        case ID_AT_MNTN_PC_REPLAY_MSG:
        case ID_AT_MNTN_PC_REPLAY_CLIENT_TAB:
        case ID_AT_MNTN_RPT_PORT:
        case ID_AT_MNTN_PS_CALL_ENTITY_RPT:
        case ID_AT_COMM_CCPU_RESET_START:
        case ID_AT_COMM_CCPU_RESET_END:
        case ID_AT_COMM_HIFI_RESET_START:
        case ID_AT_COMM_HIFI_RESET_END:
            return msg;

        default: {
            tmpAdr = (VOS_UINT8 *)((VOS_UINT8 *)atMsg + sizeof(AT_Msg) - sizeof(atMsg->value));

            len = atMsg->len;

            if (GUNAS_ATCmdFilter(tmpAdr, len) == VOS_TRUE) {
                MN_NORM_LOG1("GUNAS_FilterAtToAtMsg: TRUE msgName ", atMsg->msgId);
                return VOS_NULL_PTR;
            }

            return msg;
        }
    }
}
#endif

VOS_VOID* GUNAS_FilterAtToMmaMsg(PS_MsgHeader *msg)
{
    switch (msg->msgName) {
        /* 补充业务相关的信息 */
        case TAF_MSG_MMA_OP_PIN_REQ:
        case TAF_MSG_MMA_SET_PIN:
        case TAF_MSG_MMA_ME_PERSONAL_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToMmaMsg: TRUE msgName ", msg->msgName);
            return VOS_NULL_PTR;

        default:
            return msg;
    }
}

VOS_VOID* GUNAS_FilterAtToMtaMsg(PS_MsgHeader *msg)
{
    switch (msg->msgName) {
        /* 补充业务相关的信息 */
        case ID_AT_MTA_SIMLOCKUNLOCK_SET_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToMtaMsg: TRUE msgName ", msg->msgName);
            return VOS_NULL_PTR;

        default:
            return msg;
    }
}

VOS_VOID* GUNAS_FilterAtToPihMsg(PS_MsgHeader *msg)
{
    switch (msg->msgName) {
        /* 补充业务相关的信息 */
        /* SI_PIH_FDN_ENABLE_REQ */
        case AT_PIH_FDN_ENABLE_REQ:
        /* SI_PIH_FDN_DISALBE_REQ */
        case AT_PIH_FDN_DISALBE_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToPihMsg: TRUE msgName ", msg->msgName);
            return VOS_NULL_PTR;

        default:
            return msg;
    }
}

VOS_VOID* GUNAS_FilterLayerMsg(struct MsgCB *msg)
{
    PS_MsgHeader           *tempMsg = VOS_NULL_PTR;
    pGuNasMsgFilterProcFunc pfunFilter;
    VOS_UINT32              i;
    VOS_UINT32              senderPid;
    VOS_UINT32              receiverPid;

    tempMsg     = (PS_MsgHeader *)msg;
    pfunFilter  = VOS_NULL_PTR;
    senderPid   = VOS_GET_SENDER_ID(msg);
    receiverPid = VOS_GET_RECEIVER_ID(msg);

    /* 下移C核后，此处直接使用原C核的脱敏使能开关 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    if (AT_GetPrivacyFilterEnableFlg() == VOS_FALSE) {
        return msg;
    }
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (NAS_MML_GetPrivacyFilterFlagByModemId(NAS_MODEM_ID_0) == VOS_FALSE) {
        return msg;
    }

    senderPid   = NAS_MULTIINSTANCE_GetModem0Pid(VOS_GET_SENDER_ID(tempMsg));
    receiverPid = NAS_MULTIINSTANCE_GetModem0Pid(VOS_GET_RECEIVER_ID(tempMsg));

#endif
    /* g_guNasMsgFilterProcFuncTbl脱敏仅包含AT到AT的消息，以及AT给C核的消息 */
    if (TAF_ChkGunasLogPrivacyMsgLen((const MsgBlock *)msg) != VOS_TRUE) {
        MN_ERR_LOG("GUNAS_FilterAtToMtaMsg: message length is invalid.");
        return VOS_NULL_PTR;
    }

    for (i = 0; i < (sizeof(g_guNasMsgFilterProcFuncTbl) / sizeof(GUNAS_MSG_FILTER_PROC_TBL_FUNC)); i++) {
        if ((senderPid == g_guNasMsgFilterProcFuncTbl[i].ulSenderPid) &&
            (receiverPid == g_guNasMsgFilterProcFuncTbl[i].ulReceiverPid)) {
            pfunFilter = g_guNasMsgFilterProcFuncTbl[i].pFuncFilterProc;
            break;
        }
    }

    if (pfunFilter != VOS_NULL_PTR) {
        return pfunFilter(tempMsg);
    }

    return msg;
}

VOS_VOID* GUNAS_OM_LayerMsgFilter(struct MsgCB *msg)
{
    /*
     * 注册函数接口调整，之前所有pid消息在一起处理过滤，先对所有pid使用当前函数注册过滤，
     * 在后续脱敏项目中对当前函数进行拆分，各个pid注册自己的过滤函数
     */

    return GUNAS_FilterLayerMsg(msg);
}

/* 下移C核后，IMSA到AT的消息脱敏功能直接使用IMSA的脱敏函数，AT侧不使用 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)

VOS_VOID* NAS_OM_LogFilterImsaAtMtStatesIndMsgAcpu(VOS_VOID *msg)
{
    IMSA_AT_MtStatesInd *srcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_MtStatesInd *dstImsaAtMsg = VOS_NULL_PTR;
    errno_t              memResult;

    srcImsaAtMsg = (IMSA_AT_MtStatesInd *)msg;

    /* 申请新的内存 */
    dstImsaAtMsg = (VOS_VOID *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, sizeof(IMSA_AT_MtStatesInd));
    if (dstImsaAtMsg == VOS_NULL_PTR) {
        return msg;
    }

    memResult = memcpy_s(dstImsaAtMsg, sizeof(IMSA_AT_MtStatesInd), srcImsaAtMsg, sizeof(IMSA_AT_MtStatesInd));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(IMSA_AT_MtStatesInd), sizeof(IMSA_AT_MtStatesInd));
    (VOS_VOID)memset_s(dstImsaAtMsg->asciiCallNum, sizeof(dstImsaAtMsg->asciiCallNum), 0xFF, sizeof(dstImsaAtMsg->asciiCallNum));

    return dstImsaAtMsg;
}

VOS_VOID* NAS_OM_LogFilterImsaAtVolteImpuCnfMsgAcpu(VOS_VOID *msg)
{
    IMSA_AT_VolteimpuQryCnf *srcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_VolteimpuQryCnf *dstImsaAtMsg = VOS_NULL_PTR;
    errno_t                  memResult;

    srcImsaAtMsg = (IMSA_AT_VolteimpuQryCnf *)msg;

    /* 申请新的内存 */
    dstImsaAtMsg = (VOS_VOID *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, sizeof(IMSA_AT_VolteimpuQryCnf));
    if (dstImsaAtMsg == VOS_NULL_PTR) {
        return msg;
    }

    memResult = memcpy_s(dstImsaAtMsg, sizeof(IMSA_AT_VolteimpuQryCnf), srcImsaAtMsg, sizeof(IMSA_AT_VolteimpuQryCnf));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(IMSA_AT_VolteimpuQryCnf), sizeof(IMSA_AT_VolteimpuQryCnf));
    (VOS_VOID)memset_s(dstImsaAtMsg->impu, sizeof(dstImsaAtMsg->impu), 0XFF, sizeof(dstImsaAtMsg->impu));

    return dstImsaAtMsg;
}

STATIC VOS_VOID* NAS_OM_LogFilterImsaAtDmuserQryCnfMsgAcpu(VOS_VOID *msg)
{
    IMSA_AT_DmuserQryCnf *srcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_DmuserQryCnf *dstImsaAtMsg = VOS_NULL_PTR;
    errno_t               memResult;

    srcImsaAtMsg = (IMSA_AT_DmuserQryCnf *)msg;

    /* 申请新的内存 */
    dstImsaAtMsg = (VOS_VOID *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, sizeof(IMSA_AT_DmuserQryCnf));
    if (dstImsaAtMsg == VOS_NULL_PTR) {
        return msg;
    }

    (VOS_VOID)memset_s(dstImsaAtMsg, sizeof(IMSA_AT_DmuserQryCnf), 0, sizeof(IMSA_AT_DmuserQryCnf));

    memResult = memcpy_s(dstImsaAtMsg, sizeof(IMSA_AT_DmuserQryCnf), srcImsaAtMsg, sizeof(IMSA_AT_DmuserQryCnf));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(IMSA_AT_DmuserQryCnf), sizeof(IMSA_AT_DmuserQryCnf));
    /* 过滤IMPI */
    (VOS_VOID)memset_s(dstImsaAtMsg->dmUser.impi, sizeof(srcImsaAtMsg->dmUser.impi), 0, sizeof(srcImsaAtMsg->dmUser.impi));
    /* 过滤IMPU */
    (VOS_VOID)memset_s(dstImsaAtMsg->dmUser.impu, sizeof(srcImsaAtMsg->dmUser.impu), 0, sizeof(srcImsaAtMsg->dmUser.impu));

    return dstImsaAtMsg;
}

VOS_VOID* NAS_OM_LogFilterImsaAtMsgAcpu(struct MsgCB *msg)
{
    PS_MsgHeader *tempMsg        = VOS_NULL_PTR;
    VOS_VOID     *returnTraceMsg = msg;

    tempMsg = (PS_MsgHeader *)msg;

    if (AT_GetPrivacyFilterEnableFlg() == VOS_FALSE) {
        return msg;
    }

    if (VOS_GET_RECEIVER_ID(tempMsg) != WUEPS_PID_AT) {
        return msg;
    }

    switch (tempMsg->msgName) {
        case ID_IMSA_AT_MT_STATES_IND:
            returnTraceMsg = NAS_OM_LogFilterImsaAtMtStatesIndMsgAcpu(msg);
            break;

        case ID_IMSA_AT_VOLTEIMPI_QRY_CNF:
            returnTraceMsg = VOS_NULL_PTR;
            break;

        case ID_IMSA_AT_VOLTEIMPU_QRY_CNF:
            returnTraceMsg = NAS_OM_LogFilterImsaAtVolteImpuCnfMsgAcpu(msg);
            break;
        case ID_IMSA_AT_DMUSER_QRY_CNF:
            returnTraceMsg = NAS_OM_LogFilterImsaAtDmuserQryCnfMsgAcpu(msg);
            break;
        default:
            break;
    }

    return returnTraceMsg;
}

#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

STATIC VOS_VOID* NAS_OM_LogFilterVnasAtLtevSourceIdQryCnfMsgAcpu(VOS_VOID *msg)
{
    VNAS_ReqMsg             *dstVnasAtMsg = VOS_NULL_PTR;
    VNAS_LtevSourceIdQryCnf *sourceIdQryCnfMsg = VOS_NULL_PTR;
    errno_t                  memResult;
    VOS_UINT32               msgLength;

    msgLength = sizeof(VNAS_ReqMsg) + sizeof(VNAS_LtevSourceIdQryCnf) - AT_LENGTH_OF_POINTER;
    /* 申请新的内存 */
    dstVnasAtMsg = (VNAS_ReqMsg *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, msgLength);
    if (dstVnasAtMsg == VOS_NULL_PTR) {
        return msg;
    }

    (VOS_VOID)memset_s(dstVnasAtMsg, msgLength, 0, msgLength);
    sourceIdQryCnfMsg = (VNAS_LtevSourceIdQryCnf *)dstVnasAtMsg->content;

    /* 过滤敏感消息 */
    memResult = memcpy_s(dstVnasAtMsg, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);
    /* 将敏感信息设置为全0 */
    sourceIdQryCnfMsg->sourceid = 0;

    return dstVnasAtMsg;
}

VOS_VOID* NAS_OM_LogFilterVnasAtMsgAcpu(struct MsgCB *msg)
{
    PS_MsgHeader *tempMsg        = VOS_NULL_PTR;
    VOS_VOID     *returnTraceMsg = VOS_NULL_PTR;

    tempMsg = (PS_MsgHeader *)msg;

    if (AT_GetPrivacyFilterEnableFlg() == VOS_FALSE) {
        return msg;
    }

    if (VOS_GET_RECEIVER_ID(tempMsg) != WUEPS_PID_AT) {
        return msg;
    }

    switch (tempMsg->msgName) {
        case ID_VNAS_AT_LTEV_SOURCE_ID_QRY_CNF:
            returnTraceMsg = NAS_OM_LogFilterVnasAtLtevSourceIdQryCnfMsgAcpu(msg);
            break;
        default:
            break;
    }

    return returnTraceMsg;
}
#endif

STATIC VOS_VOID* NAS_OM_LogFilterVrrcAtSyncSourceRptIndAcpu(VOS_VOID *msg)
{
    MN_AT_IndEvt                *dstVrrcAtMsg = VOS_NULL_PTR;
    VRRC_AT_SyncSourceStatePara *sourceStateParaMsg = VOS_NULL_PTR;
    errno_t                      memResult;
    VOS_UINT32                   msgLength;

    msgLength = sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SyncSourceStatePara) - AT_LENGTH_OF_POINTER;
    /* 申请新的内存 */
    dstVrrcAtMsg = (MN_AT_IndEvt *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, msgLength);
    if (dstVrrcAtMsg == VOS_NULL_PTR) {
        return msg;
    }

    (VOS_VOID)memset_s(dstVrrcAtMsg, msgLength, 0, msgLength);
    sourceStateParaMsg = (VRRC_AT_SyncSourceStatePara *)dstVrrcAtMsg->content;

    /* 过滤敏感消息 */
    memResult = memcpy_s(dstVrrcAtMsg, msgLength, msg, msgLength);
    TAF_MEM_CHK_RTN_VAL(memResult, msgLength, msgLength);

    /* 将敏感信息设置为全0 */
    sourceStateParaMsg->slssId = 0;
    sourceStateParaMsg->subSlssId = 0;

    return dstVrrcAtMsg;
}

VOS_VOID* NAS_OM_LogFilterVrrcAtMsgAcpu(struct MsgCB *msg)
{
    PS_MsgHeader *tempMsg        = VOS_NULL_PTR;
    VOS_VOID     *returnTraceMsg = VOS_NULL_PTR;

    tempMsg = (PS_MsgHeader *)msg;

    if (AT_GetPrivacyFilterEnableFlg() == VOS_FALSE) {
        return msg;
    }

    if (VOS_GET_RECEIVER_ID(tempMsg) != WUEPS_PID_AT) {
        return msg;
    }

    switch (tempMsg->msgName) {
        case VRRC_AT_SYNC_SOURCE_RPT_IND:
            returnTraceMsg = NAS_OM_LogFilterVrrcAtSyncSourceRptIndAcpu(msg);
            break;
        default:
            break;
    }

    return returnTraceMsg;
}
#endif
#endif

/* 下移C核后，原A核AT的脱敏函数不使用 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)

VOS_VOID GUNAS_OM_LayerMsgReplaceCBRegACore(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I0_PS_PID_IMSA, NAS_OM_LogFilterImsaAtMsgAcpu);
    PS_OM_LayerMsgReplaceCBReg(I1_PS_PID_IMSA, NAS_OM_LogFilterImsaAtMsgAcpu);

#if (FEATURE_LTEV == FEATURE_ON)
    PS_OM_LayerMsgReplaceCBReg(I0_PS_PID_VERRC, NAS_OM_LogFilterVrrcAtMsgAcpu);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    PS_OM_LayerMsgReplaceCBReg(I0_PS_PID_VNAS, NAS_OM_LogFilterVnasAtMsgAcpu);
#endif
#endif
}
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_VOID I0_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_SMS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_WRR, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_GAS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(TPS_PID_RRC, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_MMA, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_SS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_GMM, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_MM, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_PS_PID_MM, GUNAS_OM_LayerMsgFilter);
}

VOS_VOID I1_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_SMS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_WRR, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_GAS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_MMA, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_SS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_GMM, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_MM, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_PS_PID_MM, GUNAS_OM_LayerMsgFilter);
}

VOS_VOID I2_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_SMS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_GAS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_MMA, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_SS, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_GMM, GUNAS_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_MM, GUNAS_OM_LayerMsgFilter);
}
#endif

