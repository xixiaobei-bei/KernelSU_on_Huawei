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

#include "at_cmd_proc.h"
#include "taf_type_def.h"
#include "at_data_proc.h"
#include "at_device_cmd.h"
#include "at_voice_ims_rslt_proc.h"
#include "at_phone_ims_rslt_proc.h"
#include "at_voice_event_report.h"
#include "at_custom_ims_rslt_proc.h"
#include "at_data_taf_rslt_proc.h"
#include "at_data_event_report.h"
#include "at_phone_event_report.h"
#include "at_custom_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PROC_IMSA_MAIN_C

#if (FEATURE_IMS == FEATURE_ON)

STATIC VOS_UINT32 AT_RcvImsaCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_CnfMsg   *cnfMsg = (IMSA_AT_CnfMsg *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (cnfMsg->result == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

STATIC AT_MsgProcEntry g_atProcImsaMsgTbl[] = {
    /* ！！！！注意！！！！
     * 以下为使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，
     * 主要原因：^IMSEMCSTAT命令在发ID_AT_IMSA_EMC_PDN_ACTIVATE_REQ消息后，立马上报OK
     */
    {ID_IMSA_AT_EMC_PDN_ACTIVATE_CNF,           AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaEmcPdnActivateCnf},
    {ID_IMSA_AT_EMC_PDN_DEACTIVATE_CNF,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaEmcPdnDeactivateCnf},

    /* 无特性宏控制的设置请求响应 */
    { ID_IMSA_AT_CIREG_SET_CNF,                 AT_CMD_CIREG_SET,               AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_CIREP_SET_CNF,                 AT_CMD_CIREP_SET,               AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_CALL_ENCRYPT_SET_CNF,          AT_CMD_CALLENCRYPT_SET,         AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_PCSCF_SET_CNF,                 AT_CMD_PCSCF_SET,               AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_DMDYN_SET_CNF,                 AT_CMD_DMDYN_SET,               AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_IMSTIMER_SET_CNF,              AT_CMD_DMTIMER_SET,             AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_SMSPSI_SET_CNF,                AT_CMD_IMSPSI_SET,              AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_NICKNAME_SET_CNF,              AT_CMD_NICKNAME_SET,            AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_REGERR_REPORT_SET_CNF,         AT_CMD_IMSREGERRRPT_SET,        AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_IMS_IP_CAP_SET_CNF,            AT_CMD_IMSIPCAPCFG_SET,         AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_IMS_SRV_STAT_RPT_SET_CNF,      AT_CMD_IMS_SRV_STAT_RPT_SET,    AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_EMERGENCY_AID_SET_CNF,         AT_CMD_WIEMCAID_SET,            AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_DM_RCS_CFG_SET_CNF,            AT_CMD_DMRCSCFG_SET,            AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_USER_AGENT_CFG_SET_CNF,        AT_CMD_USERAGENTCFG_SET,        AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_RTTCFG_SET_CNF,                AT_CMD_RTTCFG_SET,              AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_RTT_MODIFY_SET_CNF,            AT_CMD_RTTMODIFY_SET,           AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_TRANSPORT_TYPE_SET_CNF,        AT_CMD_SIPPORT_SET,             AT_RcvImsaCommonSetCnf },
    { ID_IMSA_AT_BATTERYINFO_SET_CNF,           AT_CMD_BATTERYINFO_SET,         AT_RcvImsaBatteryInfoSetCnf },

    /* 无特性宏控制的查询请求响应 */
    { ID_IMSA_AT_CIREG_QRY_CNF,                 AT_CMD_CIREG_QRY,               AT_RcvImsaCiregQryCnf },
    { ID_IMSA_AT_CIREP_QRY_CNF,                 AT_CMD_CIREP_QRY,               AT_RcvImsaCirepQryCnf },
    { ID_IMSA_AT_VOLTEIMPU_QRY_CNF,             AT_CMD_IMPU_SET,                AT_RcvImsaImpuSetCnf },
    { ID_IMSA_AT_IMS_REG_DOMAIN_QRY_CNF,        AT_CMD_IMSREGDOMAIN_QRY,        AT_RcvImsaImsRegDomainQryCnf },
    { ID_IMSA_AT_ROAMING_IMS_QRY_CNF,           AT_CMD_ROAM_IMS_QRY,            AT_RcvImsaRoamImsServiceQryCnf },
    { ID_IMSA_AT_PCSCF_QRY_CNF,                 AT_CMD_PCSCF_QRY,               AT_RcvImsaPcscfQryCnf },
    { ID_IMSA_AT_DMDYN_QRY_CNF,                 AT_CMD_DMDYN_QRY,               AT_RcvImsaDmDynQryCnf },
    { ID_IMSA_AT_IMSTIMER_QRY_CNF,              AT_CMD_DMTIMER_QRY,             AT_RcvImsaImsTimerQryCnf },
    { ID_IMSA_AT_SMSPSI_QRY_CNF,                AT_CMD_IMSPSI_QRY,              AT_RcvImsaImsPsiQryCnf },
    { ID_IMSA_AT_DMUSER_QRY_CNF,                AT_CMD_DMUSER_QRY,              AT_RcvImsaDmUserQryCnf },
    { ID_IMSA_AT_NICKNAME_QRY_CNF,              AT_CMD_NICKNAME_QRY,            AT_RcvImsaNickNameQryCnf },
    { ID_IMSA_AT_VOLTEIMPI_QRY_CNF,             AT_CMD_VOLTEIMPI_SET,           AT_RcvImsaVolteImpiQryCnf },
    { ID_IMSA_AT_VOLTEDOMAIN_QRY_CNF,           AT_CMD_VOLTEDOMAIN_SET,         AT_RcvImsaVolteDomainQryCnf },
    { ID_IMSA_AT_REGERR_REPORT_QRY_CNF,         AT_CMD_IMSREGERRRPT_QRY,        AT_RcvImsaRegErrRptQryCnf },
    { ID_IMSA_AT_IMS_IP_CAP_QRY_CNF,            AT_CMD_IMSIPCAPCFG_QRY,         AT_RcvImsaImsIpCapQryCnf },
    { ID_IMSA_AT_IMS_SRV_STAT_RPT_QRY_CNF,      AT_CMD_IMS_SRV_STAT_RPT_QRY,    AT_RcvImsaImsSrvStatRptQryCnf },
    { ID_IMSA_AT_IMS_SERVICE_STATUS_QRY_CNF,    AT_CMD_IMS_SERVICE_STATUS_QRY,  AT_RcvImsaImsSrvStatusQryCnf },
    { ID_IMSA_AT_TRANSPORT_TYPE_QRY_CNF,        AT_CMD_SIPPORT_QRY,             AT_RcvImsaSipPortQryCnf },

    /* 无特性宏控制的测试和打断 */

    /* 无特性宏控制的主动上报 */
    { ID_IMSA_AT_CIREGU_IND,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaCireguInd },
    { ID_IMSA_AT_CIREPH_IND,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaCirephInd },
    { ID_IMSA_AT_CIREPI_IND,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaCirepiInd },
    { ID_IMSA_AT_MT_STATES_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaMtStateInd },
    { ID_IMSA_AT_IMS_RAT_HANDOVER_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaRatHandoverInd },
    { ID_IMSA_AT_IMS_SRV_STATUS_UPDATE_IND,     AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvSrvStatusUpdateInd },
    { ID_IMSA_AT_DMCN_IND,                      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaDmcnInd },
    { ID_IMSA_AT_REGFAIL_IND,                   AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaImsRegFailInd },
    { ID_IMSA_AT_REGERR_REPORT_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaRegErrRptInd },
    { ID_IMSA_AT_EMC_PDN_ACTIVATE_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaEmcPdnActivateInd },
    { ID_IMSA_AT_EMC_PDN_DEACTIVATE_IND,        AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaEmcPdnDeactivateInd },
    { ID_IMSA_AT_CALL_ALT_SRV_IND,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaCallAltSrvInd },
    { ID_IMSA_AT_IMPU_TYPE_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaImpuInd },
    { ID_IMSA_AT_RTT_EVENT_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaRttEventInd },
    { ID_IMSA_AT_RTT_ERROR_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaRttErrorInd },
    { ID_IMSA_AT_IMS_CTRL_MSG,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaImsCtrlMsg },
    { ID_IMSA_AT_FUSIONCALL_CTRL_MSG,           AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvImsaFusionCallCtrlMsg },

    /* 以下为特性宏控制部分 */
#if (FEATURE_ECALL == FEATURE_ON)
    { ID_IMSA_AT_ECALL_ECONTENT_TYPE_SET_CNF,   AT_CMD_ECLIMSCFG_SET,           AT_RcvImsaCommonSetCnf},
    { ID_IMSA_AT_ECALL_ECONTENT_TYPE_QRY_CNF,   AT_CMD_ECLIMSCFG_QRY,           AT_RcvImsaEcallContentTypeQryCnf},
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_IMSA_AT_IMS_URSP_SET_CNF,              AT_CMD_IMSURSP_SET,             AT_RcvImsaImsUrspSetCnf},
#endif
};

STATIC CONST AT_MsgProcEntryTblInfo g_atProcImsaMsgTblInfo = {
    .tbl      = g_atProcImsaMsgTbl,
    .entryNum = AT_ARRAY_SIZE(g_atProcImsaMsgTbl),
    .entryLen = sizeof(g_atProcImsaMsgTbl[0])
};

STATIC VOS_UINT32 AT_GetImsaUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    AT_IMSA_Msg *imsaMsg = (AT_IMSA_Msg *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(imsaMsg->clientId, indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("AT_GetImsaUserId: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }
    return VOS_OK;
}

STATIC CONST AT_ModuleMsgProcInfo g_atImsaMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_IMSA,
    .tblInfo    = &g_atProcImsaMsgTblInfo,
    .GetMsgId   = VOS_NULL_PTR,
    .GetUserId  = AT_GetImsaUserId,
};

VOS_VOID AT_InitImsaMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atImsaMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitImsaMsgProcTbl: register fail");
    }
}

/*
 * 功能描述: 来自CCM模块的消息处理函数
 */
VOS_VOID AT_ProcImsaMsg(struct MsgCB *msg)
{
    VOS_UINT32 ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_IMSA);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_ProcImsaMsg: fail");
    }
}

#endif


