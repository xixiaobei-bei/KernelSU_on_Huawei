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
#include "at_event_report.h"
#include "at_data_comm.h"
#include "at_data_taf_rslt_proc.h"
#include "taf_type_def.h"
#include "securec.h"

#include "ppp_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "at_oam_interface.h"
#include "mn_comm_api.h"

#include "at_input_proc.h"
#include "at_cmd_msg_proc.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "gen_msg.h"
#include "at_lte_common.h"
#endif

#include "taf_app_mma.h"

#include "app_vc_api.h"
#include "taf_app_rabm.h"

#include "product_config.h"

#include "taf_std_lib.h"

#include "at_msg_print.h"

#include "mnmsgcbencdec.h"
#include "dms_file_node_i.h"
#include "at_cmd_msg_proc.h"
#include "at_mdrv_interface.h" /* DRV_OS_STATUS_SWITCH AT_SetModemState FREE_MEM_SIZE_GET */
#include "dms_msg_chk.h"
#include "dms_port_i.h"

#include "at_voice_event_report.h"
#include "at_voice_taf_rslt_proc.h"
#include "at_voice_hifi_rslt_proc.h"
#include "at_voice_mm_rslt_proc.h"
#include "at_ss_taf_rslt_proc.h"
#include "at_sms_comm.h"
#include "at_phone_event_report.h"
#include "at_general_pam_rslt_proc.h"
#include "at_safety_pam_rslt_proc.h"
#include "at_safety_mm_rslt_proc.h"
#include "at_custom_pam_rslt_proc.h"
#include "at_custom_taf_rslt_proc.h"
#include "at_custom_event_report.h"
#include "at_sim_event_report.h"
#include "at_sim_pam_rslt_proc.h"
#include "at_cdma_rslt_proc.h"
#include "at_cdma_event_report.h"
#include "at_data_event_report.h"
#include "at_data_lnas_rslt_proc.h"
#include "at_custom_event_report.h"
#include "at_device_pam_rslt_proc.h"
#include "at_device_taf_rslt_proc.h"
#include "at_sms_taf_rslt_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_EVENTREPORT_C

#define AT_HEX_OCCUPIED_BIT_NUM 4
#define AT_BYTE_TO_BITS_LEN 8
#define AT_GET_PIH_AT_CNF_LEN_WITHOUT_UNION \
    (sizeof(MN_APP_PihAtCnf) - sizeof(((MN_APP_PihAtCnf *)16)->pihAtEvent.pihEvent))

static const TAF_PS_EvtIdUint32 g_atBroadcastPsEvtTbl[] = {
    ID_EVT_TAF_PS_REPORT_DSFLOW_IND,   ID_EVT_TAF_PS_APDSFLOW_REPORT_IND, ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND,
    ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND, ID_EVT_TAF_PS_CGMTU_VALUE_CHG_IND, ID_EVT_TAF_PS_CALL_LIMIT_PDP_ACT_IND,
    ID_EVT_TAF_PS_REPORT_VTFLOW_IND,   ID_EVT_TAF_PS_EPDG_CTRLU_NTF,

    ID_EVT_TAF_PS_UE_POLICY_RPT_IND,   ID_EVT_TAF_PS_ETHER_SESSION_CAP_IND, ID_EVT_TAF_PS_CALL_RPT_CMD_CFG_IND,
#if (FEATURE_MBB_CUST == FEATURE_ON)
    ID_EVT_TAF_PS_SUCC_ATTACH_PROFILE_INDEX_IND, ID_MSG_TAF_PS_REPORT_DSAMBR_INFO_IND,
#endif
};

static const AT_PS_EvtFuncTbl g_atPsEvtFuncTbl[] = {
    /* PS CALL */
    { ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpActivateCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpActivateCnf },
    { ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_REJ, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpActivateRej) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpActivateRej },
    { ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpManageInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpManageInd },
    { ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpActivateInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpActivateInd },
    { ID_EVT_TAF_PS_CALL_PDP_MODIFY_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpModifyCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpModifyCnf },
    { ID_EVT_TAF_PS_CALL_PDP_MODIFY_REJ, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpModifyRej) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpModifyRej },
    { ID_EVT_TAF_PS_CALL_PDP_MODIFY_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpModifyInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpModifiedInd },
    { ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpDeactivateCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpDeactivateCnf },
    { ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_IND,  (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpDeactivateInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtPdpDeactivatedInd },

    { ID_EVT_TAF_PS_CALL_END_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallEndCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtCallEndCnf },
    { ID_EVT_TAF_PS_CALL_MODIFY_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallModifyCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtCallModifyCnf },
    { ID_EVT_TAF_PS_CALL_ANSWER_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallAnswerCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtCallAnswerCnf },
    { ID_EVT_TAF_PS_CALL_HANGUP_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallHangupCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallEvtCallHangupCnf },

    /* D */
    { ID_EVT_TAF_PS_GET_D_GPRS_ACTIVE_TYPE_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDGprsActiveTypeCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetGprsActiveTypeCnf },

    /* PPP */
    { ID_EVT_TAF_PS_PPP_DIAL_ORIG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_PppDialOrigCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtPppDialOrigCnf },

    /* +CGDCONT */
    { ID_EVT_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetPrimPdpContextInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetPrimPdpContextInfoCnf },
    { ID_EVT_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetPrimPdpContextInfoCnfMsgLen, AT_RcvTafPsEvtGetPrimPdpContextInfoCnf },

    /* +CGDSCONT */
    { ID_EVT_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetSecPdpContextInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetSecPdpContextInfoCnf },
    { ID_EVT_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetSecPdpContextInfoCnfMsgLen, AT_RcvTafPsEvtGetSecPdpContextInfoCnf },

    /* +CGTFT */
    { ID_EVT_TAF_PS_SET_TFT_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetTftInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetTftInfoCnf },
    { ID_EVT_TAF_PS_GET_TFT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetTftInfoCnfMsgLen, AT_RcvTafPsEvtGetTftInfoCnf },

    /* +CGEQREQ */
    { ID_EVT_TAF_PS_SET_UMTS_QOS_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetUmtsQosInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetUmtsQosInfoCnf },
    { ID_EVT_TAF_PS_GET_UMTS_QOS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetUmtsQosInfoCnfMsgLen, AT_RcvTafPsEvtGetUmtsQosInfoCnf },

    /* +CGEQMIN */
    { ID_EVT_TAF_PS_SET_UMTS_QOS_MIN_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetUmtsQosMinInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetUmtsQosMinInfoCnf },
    { ID_EVT_TAF_PS_GET_UMTS_QOS_MIN_INFO_CNF, 0,
      AT_ChkTafPsEvtGetUmtsQosMinInfoCnfMsgLen, AT_RcvTafPsEvtGetUmtsQosMinInfoCnf },

    /* +CGEQNEG */
    { ID_EVT_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamicUmtsQosInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamicUmtsQosInfoCnf },

    /* +CGACT */
    { ID_EVT_TAF_PS_SET_PDP_CONTEXT_STATE_CNF,  (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetPdpStateCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetPdpStateCnf },
    { ID_EVT_TAF_PS_GET_PDP_CONTEXT_STATE_CNF, 0,
      AT_ChkTafPsEvtGetPdpStateCnfMsgLen, AT_RcvTafPsEvtGetPdpStateCnf },

    /* +CGPADDR */
    { ID_EVT_TAF_PS_GET_PDP_IP_ADDR_INFO_CNF, 0,
      AT_ChkTafPsEvtGetPdpIpAddrInfoCnfMsgLen, AT_RcvTafPsEvtGetPdpIpAddrInfoCnf },
    { ID_EVT_TAF_PS_GET_PDP_CONTEXT_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetPdpContextInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetPdpContextInfoCnf },

    /* +CGAUTO */
    { ID_EVT_TAF_PS_SET_ANSWER_MODE_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetAnswerModeInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetAnsModeInfoCnf },
    { ID_EVT_TAF_PS_GET_ANSWER_MODE_INFO_CNF,  (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetAnswerModeInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetAnsModeInfoCnf },

    /* +CGCONTRDP */
    { ID_EVT_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamicPrimPdpContextInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamicPrimPdpContextInfoCnf },
    /* +CGSCONTRDP */
    { ID_EVT_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamicSecPdpContextInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamicSecPdpContextInfoCnf },

    /* +CGTFTRDP */
    { ID_EVT_TAF_PS_GET_DYNAMIC_TFT_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamicTftInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamicTftInfoCnf },

    /* +CGEQOS */
    { ID_EVT_TAF_PS_SET_EPS_QOS_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetEpsQosInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetEpsQosInfoCnf },
    { ID_EVT_TAF_PS_GET_EPS_QOS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetEpsQosInfoCnfMsgLen, AT_RcvTafPsEvtGetEpsQosInfoCnf },

    /* +CGEQOSRDP */
    { ID_EVT_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamicEpsQosInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamicEpsQosInfoCnf },

    /* ^CDQF/^DSFLOWQRY */
    { ID_EVT_TAF_PS_GET_DSFLOW_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDsflowInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetDsFlowInfoCnf },

    /* ^CDCF/^DSFLOWCLR */
    { ID_EVT_TAF_PS_CLEAR_DSFLOW_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ClearDsflowCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtClearDsFlowInfoCnf },

    /* ^CDSF/^DSFLOWRPT/^FLOWRPTCTRL */
    { ID_EVT_TAF_PS_CONFIG_DSFLOW_RPT_CNF,  (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ConfigDsflowRptCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtConfigDsFlowRptCnf },

    /* ^DSFLOWRPT */
    { ID_EVT_TAF_PS_REPORT_DSFLOW_IND,  (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ReportDsflowInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtReportDsFlowInd },

    /* ^VTFLOWRPT */
    { ID_EVT_TAF_PS_REPORT_VTFLOW_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ReportVtflowInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtReportVTFlowInd },
    { ID_EVT_TAF_PS_CONFIG_VTFLOW_RPT_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ConfigVtflowRptCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtConfigVTFlowRptCnf },

    /* ^CGDNS */
    { ID_EVT_TAF_PS_SET_PDP_DNS_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetPdpDnsInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetPdpDnsInfoCnf },
    { ID_EVT_TAF_PS_GET_PDP_DNS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetPdpDnsInfoCnfMsgLen, AT_RcvTafPsEvtGetPdpDnsInfoCnf },

    /* ^AUTHDATA */
    { ID_EVT_TAF_PS_SET_AUTHDATA_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetAuthDataInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetAuthDataInfoCnf },
    { ID_EVT_TAF_PS_GET_AUTHDATA_INFO_CNF, 0,
      AT_ChkTafPsEvtGetAuthDataInfoCnfMsgLen, AT_RcvTafPsEvtGetAuthDataInfoCnf },

    { ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallPdpPdpDisconnectInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtPdpDisconnectInd },
    { ID_EVT_TAF_PS_GET_NEGOTIATION_DNS_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetNegotiationDnsCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetDynamicDnsInfoCnf },

#if (FEATURE_LTE == FEATURE_ON)
    { ID_EVT_TAF_PS_LTECS_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_LtecsCnf) - 4),
      VOS_NULL_PTR, atReadLtecsCnfProc },
    { ID_EVT_TAF_PS_CEMODE_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CemodeCnf) - 4),
      VOS_NULL_PTR, atReadCemodeCnfProc },

    { ID_EVT_TAF_PS_GET_CID_SDF_CNF, 0,
      VOS_NULL_PTR, AT_RcvTafPsEvtGetCidSdfInfoCnf },

    { ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetLteAttachInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafGetLteAttachInfoCnf },

#endif
    { ID_EVT_TAF_PS_SET_APDSFLOW_RPT_CFG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetApdsflowRptCfgCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetApDsFlowRptCfgCnf },
    { ID_EVT_TAF_PS_GET_APDSFLOW_RPT_CFG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetApdsflowRptCfgCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetApDsFlowRptCfgCnf },
    { ID_EVT_TAF_PS_APDSFLOW_REPORT_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_ApdsflowReportInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtApDsFlowReportInd },

    { ID_EVT_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetDsflowNvWriteCfgCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetDsFlowNvWriteCfgCnf },
    { ID_EVT_TAF_PS_GET_DSFLOW_NV_WRITE_CFG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDsflowNvWriteCfgCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetDsFlowNvWriteCfgCnf },

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { ID_EVT_TAF_PS_SET_CTA_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetCtaInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetPktCdataInactivityTimeLenCnf },
    { ID_EVT_TAF_PS_GET_CTA_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetCtaInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetPktCdataInactivityTimeLenCnf },


    { ID_EVT_TAF_PS_SET_CDMA_DIAL_MODE_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CdataDialModeCnf) - 4),
      VOS_NULL_PTR, At_RcvTafPsEvtSetDialModeCnf },

    { ID_EVT_TAF_PS_CGMTU_VALUE_CHG_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CgmtuValueChgInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtCgmtuValueChgInd },
#endif

    { ID_EVT_TAF_PS_SET_IMS_PDP_CFG_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetImsPdpCfgCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetImsPdpCfgCnf },

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { ID_EVT_TAF_PS_SET_1X_DORM_TIMER_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_Set1xDormTimerCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSet1xDormTimerCnf },

    { ID_EVT_TAF_PS_GET_1X_DORM_TIMER_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_Get1XDormTimerCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGet1xDormTimerCnf },
#endif

    /* 处理telcel pdp激活受限消息 */
    { ID_EVT_TAF_PS_CALL_LIMIT_PDP_ACT_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallLimitPdpActInd) - 4),
        VOS_NULL_PTR, AT_RcvTafPsCallEvtLimitPdpActInd },

    { ID_EVT_TAF_PS_SET_DATA_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SET_DATA_SWITCH_CNF_STRU) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetDataSwitchCnf },

    { ID_EVT_TAF_PS_GET_DATA_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDataSwitchCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetDataSwitchCnf },

    { ID_EVT_TAF_PS_SET_DATA_ROAM_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SET_DATA_ROAM_SWITCH_CNF_STRU) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetDataRoamSwitchCnf },

    { ID_EVT_TAF_PS_GET_DATA_ROAM_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDataRoamSwitchCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetDataRoamSwitchCnf },

    { ID_EVT_TAF_PS_EPDG_CTRLU_NTF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_EpdgCtrluNtf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtEpdgCtrluNtf },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* +C5GQOS */
    { ID_EVT_TAF_PS_SET_5G_QOS_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_Set5gQosInfoCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSet5gQosInfoCnf },
    { ID_EVT_TAF_PS_GET_5G_QOS_INFO_CNF, 0,
        AT_ChkTafPsEvtGet5gQosInfoCnfMsgLen, AT_RcvTafPsEvtGet5gQosInfoCnf },

    /* +C5GQOSRDP */
    { ID_EVT_TAF_PS_GET_DYNAMIC_5G_QOS_INFO_CNF, 0,
      AT_ChkTafPsEvtGetDynamic5gQosInfoCnfMsgLen, AT_RcvTafPsEvtGetDynamic5gQosInfoCnf },

    /* ^CPOLICYRPT */
    { ID_EVT_TAF_PS_UE_POLICY_RPT_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_UePolicyRptInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtUePolicyRptInd },

    /* ^CPOLICYRPT */
    { ID_EVT_TAF_PS_SET_UE_POLICY_RPT_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetUePolicyRptCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetUePolicyRptCnf },

    /* ^CPOLICYCODE */
    { ID_EVT_TAF_PS_GET_UE_POLICY_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetUePolicyCnf) - 4),
      AT_ChkTafPsEvtGetUePolicyCnfMsgLen, AT_RcvTafPsEvtGetUePolicyCnf },

    /* ^CSUEPOLICY */
    { ID_EVT_TAF_PS_UE_POLICY_RSP_CHECK_RSLT_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_UePolicyRspCheckRsltInd) - 4),
        VOS_NULL_PTR, AT_RcvTafPsEvtUePolicyRspCheckRsltInd },

    { ID_EVT_TAF_PS_ETHER_SESSION_CAP_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_EtherSessCapInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEtherSessCapInd },

    { ID_EVT_TAF_PS_GET_URSP_RSD_INFO_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_UrspRsdQryCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsGet5gUrspRsdInfoCnf },

    { ID_EVT_TAF_PS_SET_CGCONTEX_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetCgContExCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetCgContExCnf },
#endif

    { ID_EVT_TAF_PS_SET_ROAMING_PDP_TYPE_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetRoamingPdpTypeCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetRoamPdpTypeCnf },
    { ID_MSG_TAF_PS_GET_SINGLE_PDN_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetSinglePdnSwitchCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtGetSinglePdnSwitchCnf },
    { ID_MSG_TAF_PS_SET_SINGLE_PDN_SWITCH_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetSinglePdnSwitchCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetSinglePdnSwitchCnf },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { ID_EVT_TAF_PS_SET_CUST_ATTACH_PROFILE_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_SetCustAttachProfileInfoCnf) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtSetCustomAttachProfileCnf },
    { ID_EVT_TAF_PS_GET_CUST_ATTACH_PROFILE_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_GetCustAttachProfileInfoCnf) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtGetCustomAttachProfileCnf },
    { ID_EVT_TAF_PS_GET_CUST_ATTACH_PROFILE_COUNT_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_GetCustAttachProfileCountCnf) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtGetCustomAttachProfileCountCnf },
    { ID_EVT_TAF_PS_SUCC_ATTACH_PROFILE_INDEX_IND, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_SuccAttachProfileIndexInd) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtSuccAttachProfileIndexInd },
    { ID_EVT_TAF_PS_SET_ATTACH_PROFILE_SWITCH_STATUS_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_SetAttachProfileSwitchStatusCnf) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtSetAttachProfileSwitchStatusCnf },
    { ID_EVT_TAF_PS_GET_ATTACH_PROFILE_SWITCH_STATUS_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_GetAttachProfileSwitchStatusCnf) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtGetAttachProfileSwitchStatusCnf },
    { ID_MSG_TAF_PS_GET_DSAMBR_INFO_CNF, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_DsambrQryResponse) - 4), VOS_NULL_PTR,
        AT_RcvTafDsambrInfoQryCnf },
    { ID_MSG_TAF_PS_REPORT_DSAMBR_INFO_IND, (sizeof(TAF_PS_Evt) +
        sizeof(TAF_PS_DsambrReportInd) - 4), VOS_NULL_PTR,
        AT_RcvTafPsEvtDsambrInfoReportInd },
#endif
    { ID_EVT_TAF_PS_CALL_RPT_CMD_CFG_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_CallRptCmdCfgInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsCallRptCmdCfgInd },
    { ID_EVT_TAF_PS_SET_PS_CALL_RPT_CMD_CNF, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_SetPsCallRptCmdCnf) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtSetPsCallRptCmdCnf },

    { ID_EVT_TAF_PS_CALL_PDP_IPV6_INFO_IND, (sizeof(TAF_PS_Evt) + sizeof(TAF_PS_Ipv6InfoInd) - 4),
      VOS_NULL_PTR, AT_RcvTafPsEvtIPv6InfoInd },
};

/* 主动上报命令与控制Bit位对应表 */
/* 命令对应顺序为Bit0~Bit63 */
static const AT_RptCmdIndexUint8 g_atCurcRptCmdTable[] = {
    AT_RPT_CMD_MODE,  AT_RPT_CMD_RSSI,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_SRVST,     AT_RPT_CMD_BUTT,
    AT_RPT_CMD_SIMST, AT_RPT_CMD_TIME,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_ANLEVEL,   AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_SMMEMFULL, AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_CTZV,
    AT_RPT_CMD_CTZE,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_DSFLOWRPT, AT_RPT_CMD_BUTT,      AT_RPT_CMD_ORIG,
    AT_RPT_CMD_CONF,  AT_RPT_CMD_CONN,      AT_RPT_CMD_CEND,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_STIN,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_CERSSI,    AT_RPT_CMD_LWCLASH,
    AT_RPT_CMD_XLEMA, AT_RPT_CMD_ACINFO,    AT_RPT_CMD_PLMN,      AT_RPT_CMD_CALLSTATE, AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,  AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT
};

static const AT_RptCmdIndexUint8 g_atUnsolicitedRptCmdTable[] = {
    AT_RPT_CMD_MODE,   AT_RPT_CMD_RSSI,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_SRVST,  AT_RPT_CMD_CREG,      AT_RPT_CMD_SIMST,
    AT_RPT_CMD_TIME,   AT_RPT_CMD_BUTT,   AT_RPT_CMD_ANLEVEL, AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_CTZV,   AT_RPT_CMD_CTZE,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_DSFLOWRPT, AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_CUSD,   AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_CSSI,   AT_RPT_CMD_CSSU,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_CERSSI, AT_RPT_CMD_LWURC,  AT_RPT_CMD_BUTT,    AT_RPT_CMD_CUUS1U, AT_RPT_CMD_CUUS1I,    AT_RPT_CMD_CGREG,
    AT_RPT_CMD_CEREG,  AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT,   AT_RPT_CMD_BUTT,      AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,   AT_RPT_CMD_C5GREG, AT_RPT_CMD_BUTT,    AT_RPT_CMD_BUTT
};

static const AT_CmeCallErrCodeMap g_atCmeCallErrCodeMapTbl[] = {
    { AT_CME_INCORRECT_PARAMETERS, TAF_CS_CAUSE_INVALID_PARAMETER },
    { AT_CME_SIM_FAILURE, TAF_CS_CAUSE_SIM_NOT_EXIST },
    { AT_CME_SIM_PIN_REQUIRED, TAF_CS_CAUSE_SIM_PIN_NEED },
    { AT_CME_UNKNOWN, TAF_CS_CAUSE_NO_CALL_ID },
    { AT_CME_OPERATION_NOT_ALLOWED, TAF_CS_CAUSE_NOT_ALLOW },
    { AT_CME_INCORRECT_PARAMETERS, TAF_CS_CAUSE_STATE_ERROR },
    { AT_CME_FDN_FAILED, TAF_CS_CAUSE_FDN_CHECK_FAILURE },
    { AT_CME_CALL_CONTROL_BEYOND_CAPABILITY, TAF_CS_CAUSE_CALL_CTRL_BEYOND_CAPABILITY },
    { AT_CME_CALL_CONTROL_FAILED, TAF_CS_CAUSE_CALL_CTRL_TIMEOUT },
    { AT_CME_CALL_CONTROL_FAILED, TAF_CS_CAUSE_CALL_CTRL_NOT_ALLOWED },
    { AT_CME_CALL_CONTROL_FAILED, TAF_CS_CAUSE_CALL_CTRL_INVALID_PARAMETER },
    { AT_CME_UNKNOWN, TAF_CS_CAUSE_UNKNOWN }
};

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 g_guTmodeCnf  = 0;
VOS_UINT32 g_lteTmodeCnf = 0;
#endif


static const AT_ChgTafErrCodeTbl g_atChgTafErrCodeTbl[] = {
    { TAF_ERR_GET_CSQLVL_FAIL, AT_ERROR },
    { TAF_ERR_USIM_SVR_OPLMN_LIST_INAVAILABLE, AT_ERROR },
    { TAF_ERR_TIME_OUT, AT_CME_NETWORK_TIMEOUT },
    { TAF_ERR_USIM_SIM_CARD_NOTEXIST, AT_CME_SIM_NOT_INSERTED },
    { TAF_ERR_NEED_PIN1, AT_CME_SIM_PIN_REQUIRED },
    { TAF_ERR_NEED_PUK1, AT_CME_SIM_PUK_REQUIRED },
    { TAF_ERR_SIM_FAIL, AT_CME_SIM_FAILURE },
    { TAF_ERR_PB_STORAGE_OP_FAIL, AT_CME_SIM_FAILURE },
    { TAF_ERR_UNSPECIFIED_ERROR, AT_CME_UNKNOWN },
    { TAF_ERR_PARA_ERROR, AT_CME_INCORRECT_PARAMETERS },
    { TAF_ERR_SS_NEGATIVE_PASSWORD_CHECK, AT_CME_INCORRECT_PASSWORD },
    { TAF_ERR_SIM_BUSY, AT_CME_SIM_BUSY },
    { TAF_ERR_SIM_LOCK, AT_CME_PH_SIM_PIN_REQUIRED },
    { TAF_ERR_SIM_INCORRECT_PASSWORD, AT_CME_INCORRECT_PASSWORD },
    { TAF_ERR_PB_NOT_FOUND, AT_CME_NOT_FOUND },
    { TAF_ERR_PB_DIAL_STRING_TOO_LONG, AT_CME_DIAL_STRING_TOO_LONG },
    { TAF_ERR_PB_STORAGE_FULL, AT_CME_MEMORY_FULL },
    { TAF_ERR_PB_WRONG_INDEX, AT_CME_INVALID_INDEX },
    { TAF_ERR_CMD_TYPE_ERROR, AT_CME_OPERATION_NOT_ALLOWED },
    { TAF_ERR_FILE_NOT_EXIST, AT_CME_FILE_NOT_EXISTS },
    { TAF_ERR_NO_NETWORK_SERVICE, AT_CME_NO_NETWORK_SERVICE },
    { TAF_ERR_AT_ERROR, AT_ERROR },
    { TAF_ERR_CME_OPT_NOT_SUPPORTED, AT_CME_OPERATION_NOT_SUPPORTED },
    { TAF_ERR_NET_SEL_MENU_DISABLE, AT_CME_NET_SEL_MENU_DISABLE },
    { TAF_ERR_SYSCFG_CS_IMS_SERV_EXIST, AT_CME_CS_IMS_SERV_EXIST },
    { TAF_ERR_NO_RF, AT_CME_NO_RF },
    { TAF_ERR_NEED_PUK2, AT_CME_SIM_PUK2_REQUIRED },
    { TAF_ERR_BUSY_ON_USSD, AT_CME_OPERATION_NOT_SUPPORTED },
    { TAF_ERR_BUSY_ON_SS, AT_CME_OPERATION_NOT_SUPPORTED },
    { TAF_ERR_SS_NET_TIMEOUT, AT_CME_NETWORK_TIMEOUT },
    { TAF_ERR_NO_SUCH_ELEMENT, AT_CME_NO_SUCH_ELEMENT },
    { TAF_ERR_MISSING_RESOURCE, AT_CME_MISSING_RESOURCE },
    { TAF_ERR_IMS_NOT_SUPPORT, AT_CME_IMS_NOT_SUPPORT },
    { TAF_ERR_IMS_SERVICE_EXIST, AT_CME_IMS_SERVICE_EXIST },
    { TAF_ERR_IMS_VOICE_DOMAIN_PS_ONLY, AT_CME_IMS_VOICE_DOMAIN_PS_ONLY },
    { TAF_ERR_IMS_STACK_TIMEOUT, AT_CME_IMS_STACK_TIMEOUT },
    { TAF_ERR_IMS_OPEN_LTE_NOT_SUPPORT, AT_CME_IMS_OPEN_LTE_NOT_SUPPORT },
    { TAF_ERR_1X_RAT_NOT_SUPPORTED, AT_CME_1X_RAT_NOT_SUPPORTED },
    { TAF_ERR_SILENT_AES_DEC_PIN_FAIL, AT_CME_SILENT_AES_DEC_PIN_ERROR },
    { TAF_ERR_SILENT_VERIFY_PIN_ERR, AT_CME_SILENT_VERIFY_PIN_ERROR },
    { TAF_ERR_SILENT_AES_ENC_PIN_FAIL, AT_CME_SILENT_AES_ENC_PIN_ERROR },
    { TAF_ERR_NOT_FIND_FILE, AT_CME_NOT_FIND_FILE },
    { TAF_ERR_NOT_FIND_NV, AT_CME_NOT_FIND_NV },
    { TAF_ERR_MODEM_ID_ERROR, AT_CME_MODEM_ID_ERROR },
    { TAF_ERR_NV_NOT_SUPPORT_ERR, AT_CME_NV_NOT_SUPPORT_ERR },
    { TAF_ERR_WRITE_NV_TIMEOUT, AT_CME_WRITE_NV_TimeOut },
    { TAF_ERR_NETWORK_FAILURE, AT_CME_NETWORK_FAILURE },
    { TAF_ERR_SCI_ERROR, AT_CME_SCI_ERROR },

    { TAF_ERR_EMAT_OPENCHANNEL_ERROR, AT_ERR_EMAT_OPENCHANNEL_ERROR },
    { TAF_ERR_EMAT_OPENCHANNEL_CNF_ERROR, AT_ERR_EMAT_OPENCHANNEL_CNF_ERROR },
    { TAF_ERR_EMAT_CLOSECHANNEL_ERROR, AT_ERR_EMAT_CLOSECHANNEL_ERROR },
    { TAF_ERR_EMAT_CLOSECHANNEL_CNF_ERROR, AT_ERR_EMAT_CLOSECHANNEL_CNF_ERROR },
    { TAF_ERR_EMAT_GETEID_ERROR, AT_ERR_EMAT_GETEID_ERROR },
    { TAF_ERR_EMAT_GETEID_DATA_ERROR, AT_ERR_EMAT_GETEID_DATA_ERROR },
    { TAF_ERR_EMAT_GETPKID_ERROR, AT_ERR_EMAT_GETPKID_ERROR },
    { TAF_ERR_EMAT_GETPKID_DATA_ERROR, AT_ERR_EMAT_GETPKID_DATA_ERROR },
    { TAF_ERR_EMAT_CLEANPROFILE_ERROR, AT_ERR_EMAT_CLEANPROFILE_ERROR },
    { TAF_ERR_EMAT_CLEANPROFILE_DATA_ERROR, AT_ERR_EMAT_CLEANPROFILE_DATA_ERROR },
    { TAF_ERR_EMAT_CHECKPROFILE_ERROR, AT_ERR_EMAT_CHECKPROFILE_ERROR },
    { TAF_ERR_EMAT_CHECKPROFILE_DATA_ERROR, AT_ERR_EMAT_CHECKPROFILE_DATA_ERROR },
    { TAF_ERR_EMAT_TPDU_CNF_ERROR, AT_ERR_EMAT_TPDU_CNF_ERROR },
    { TAF_ERR_EMAT_TPDU_DATASTORE_ERROR, AT_ERR_EMAT_TPDU_DATASTORE_ERROR },
    { TAF_ERR_ESIMSWITCH_SET_ERROR, AT_ERR_ESIMSWITCH_SET_ERROR },
    { TAF_ERR_ESIMSWITCH_SET_NOT_ENABLE_ERROR, AT_ERR_ESIMSWITCH_SET_NOT_ENABLE_ERROR },
    { TAF_ERR_ESIMSWITCH_QRY_ERROR, AT_ERR_ESIMSWITCH_QRY_ERROR },
    { TAF_ERR_CARRIER_MALLOC_FAIL, AT_ERR_CARRIER_MALLOC_FAIL },
    { TAF_ERR_CARRIER_READ_NV_ORIGINAL_DATA_ERR, AT_ERR_CARRIER_READ_NV_ORIGINAL_DATA_ERR },
    { TAF_ERR_CARRIER_FILE_LEN_ERROR, AT_ERR_CARRIER_FILE_LEN_ERROR },
    { TAF_ERR_CARRIER_NV_LEN_ERROR, AT_ERR_CARRIER_NV_LEN_ERROR },
    { TAF_ERR_CARRIER_WRITE_NV_FREE_FAIL, AT_ERR_CARRIER_WRITE_NV_FREE_FAIL },
    { TAF_ERR_CARRIER_NV_ERROR, AT_ERR_CARRIER_NV_ERROR },
    { TAF_ERR_CARRIER_HMAC_VERIFY_FAIL, AT_ERR_CARRIER_HMAC_VERIFY_FAIL},
    { TAF_ERR_NOT_IN_NR_NORMAL_SERVICE, AT_ERR_NOT_IN_NR_NORMAL_SERVICE},
    { TAF_ERR_NO_NEED_REPEAT_AGAINS, AT_ERR_NO_NEED_REPEAT_AGAINS}
};

static const AT_PIH_RspProcFunc g_atPihRspProcFuncTbl[] = {
    { SI_PIH_EVENT_FDN_CNF, sizeof(SI_PIH_EventFdnCnf), VOS_NULL_PTR, At_ProcPihFndBndCnf },
    { SI_PIH_EVENT_BDN_CNF, sizeof(SI_PIH_EventFdnCnf), VOS_NULL_PTR, At_ProcPihFndBndCnf },
    { SI_PIH_EVENT_GENERIC_ACCESS_CNF, sizeof(SI_PIH_CsimCommandCnf), VOS_NULL_PTR, At_ProcPihGenericAccessCnf },
    { SI_PIH_EVENT_ISDB_ACCESS_CNF, sizeof(SI_PIH_IsdbAccessCommandCnf), VOS_NULL_PTR, At_ProcPihIsdbAccessCnf },
    { SI_PIH_EVENT_CCHO_SET_CNF, sizeof(SI_PIH_OpenChannelCommandCnf), VOS_NULL_PTR, At_ProcPihCchoSetCnf },
    { SI_PIH_EVENT_CCHP_SET_CNF, sizeof(SI_PIH_OpenChannelCommandCnf), VOS_NULL_PTR, At_ProcPihCchpSetCnf },
    { SI_PIH_EVENT_CCHC_SET_CNF, 0, VOS_NULL_PTR, At_ProcPihCchcSetCnf },
    { SI_PIH_EVENT_SCICFG_SET_CNF, 0, VOS_NULL_PTR, At_ProcPihSciCfgSetCnf },
    { SI_PIH_EVENT_HVSST_SET_CNF, 0, VOS_NULL_PTR, At_ProcPihHvsstSetCnf },
    { SI_PIH_EVENT_CGLA_SET_CNF, sizeof(SI_PIH_CglaCommandCnf), VOS_NULL_PTR, At_ProcPihCglaSetCnf },
    { SI_PIH_EVENT_CARD_ATR_QRY_CNF, sizeof(SI_PIH_AtrQryCnf), VOS_NULL_PTR, At_ProcPihCardAtrQryCnf },
    { SI_PIH_EVENT_HISTORY_CARD_ATR_QRY_CNF, sizeof(SI_PIH_AtrQryCnf), VOS_NULL_PTR, At_ProcPihCardHistoryAtrQryCnf },
    { SI_PIH_EVENT_SCICFG_QUERY_CNF, sizeof(SI_PIH_SciCfgQueryCnf), VOS_NULL_PTR, At_SciCfgQueryCnf },
    { SI_PIH_EVENT_HVSST_QUERY_CNF, sizeof(SI_PIH_EventHvsstQueryCnf), VOS_NULL_PTR, At_HvsstQueryCnf },
    { SI_PIH_EVENT_CARDTYPE_QUERY_CNF, sizeof(SI_PIH_EventCardTypeQueryCnf), VOS_NULL_PTR, At_ProcPihCardTypeQryCnf },
    { SI_PIH_EVENT_CARDTYPEEX_QUERY_CNF, sizeof(SI_PIH_EventCardTypeQueryCnf), VOS_NULL_PTR, At_ProcPihCardTypeExQryCnf },
    { SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF, sizeof(SI_PIH_EventCardvoltageQueryCnf), VOS_NULL_PTR, At_ProcPihCardVoltageQryCnf },
    { SI_PIH_EVENT_PRIVATECGLA_SET_CNF, sizeof(SI_PIH_CglaHandleCnf), VOS_NULL_PTR, At_ProcPihPrivateCglaSetCnf },
    { SI_PIH_EVENT_CRSM_SET_CNF, sizeof(SI_PIH_RaccessCnf), VOS_NULL_PTR, At_ProcPihCrsmSetCnf },
    { SI_PIH_EVENT_CRLA_SET_CNF, sizeof(SI_PIH_RaccessCnf), VOS_NULL_PTR, At_ProcPihCrlaSetCnf },
    { SI_PIH_EVENT_SESSION_QRY_CNF, USIMM_CARDAPP_BUTT * sizeof(VOS_UINT32), VOS_NULL_PTR, At_ProcPihSessionQryCnf },
    { SI_PIH_EVENT_CIMI_QRY_CNF, sizeof(SI_PIH_EventImsi), VOS_NULL_PTR, At_ProcPihCimiQryCnf },
    { SI_PIH_EVENT_CCIMI_QRY_CNF, sizeof(SI_PIH_EventImsi), VOS_NULL_PTR, At_ProcPihCcimiQryCnf },
#if (FEATURE_IMS == FEATURE_ON)
    { SI_PIH_EVENT_UICCAUTH_CNF, sizeof(SI_PIH_UiccAuthCnf), VOS_NULL_PTR, AT_UiccAuthCnf },
    { SI_PIH_EVENT_URSM_CNF, sizeof(SI_PIH_UiccAccessfileCnf), VOS_NULL_PTR, AT_UiccAccessFileCnf },
#endif
#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
    { SI_PIH_EVENT_SILENT_PIN_SET_CNF, sizeof(SI_PIH_CryptoPin), VOS_NULL_PTR, At_PrintSilentPinInfo },
    { SI_PIH_EVENT_SILENT_PININFO_SET_CNF, sizeof(SI_PIH_CryptoPin), VOS_NULL_PTR, At_PrintSilentPinInfo },
#endif

    { SI_PIH_EVENT_ESIMSWITCH_SET_CNF, 0, VOS_NULL_PTR, At_PrintSetEsimSwitchInfo },
    { SI_PIH_EVENT_ESIMSWITCH_QRY_CNF, sizeof(USIMM_SlotCardType), VOS_NULL_PTR, At_PrintQryEsimSwitchInfo },
    { SI_PIH_EVENT_BWT_SET_CNF, 0, VOS_NULL_PTR, At_ProcPihBwtSetCnf },
    { SI_PIH_EVENT_PRIVATECCHO_SET_CNF, sizeof(SI_PIH_OpenChannelCommandCnf), VOS_NULL_PTR, At_ProcPihPrivateCchoSetCnf },
    { SI_PIH_EVENT_PRIVATECCHP_SET_CNF, sizeof(SI_PIH_OpenChannelCommandCnf), VOS_NULL_PTR, At_ProcPihPrivateCchpSetCnf },
#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    { SI_PIH_EVENT_NOCARD_SET_CNF, sizeof(VOS_UINT32), VOS_NULL_PTR,  AT_ProcNoCardSetCnf },
#endif
#endif
    { SI_PIH_EVENT_PASSTHROUGH_SET_CNF, 0, VOS_NULL_PTR, At_SetPassThroughCnf },
    { SI_PIH_EVENT_PASSTHROUGH_QUERY_CNF, sizeof(SI_PIH_PassThroughStateUint32), VOS_NULL_PTR, At_QryPassThroughCnf },
#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
    { SI_PIH_EVENT_SINGLEMODEMDUALSLOT_SET_CNF, 0, VOS_NULL_PTR, At_SetSingleModemDualSlotCnf },
    { SI_PIH_EVENT_SINGLEMODEMDUALSLOT_QUERY_CNF, sizeof(SI_PIH_CardSlotUint32), VOS_NULL_PTR, At_QrySingleModemDualSlotCnf },
#endif
#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
    { SI_PIH_EVENT_CARD_APP_AID_SET_CNF, 0, VOS_NULL_PTR, At_ProcPihCardAppAidSetCnf },
#endif
};

static const AT_MnCallBackCmdCnfCmdOptEntity g_atMnCallBackCmdCnfOptFuncTbl[] = {
    { AT_CMD_CDUR_READ,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CCWA命令相关 */
    { AT_CMD_CCWA_DISABLE,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCWA_ENABLE,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCWA_QUERY,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CCFC命令 */
    { AT_CMD_CCFC_DISABLE,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCFC_ENABLE,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCFC_QUERY,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCFC_REGISTRATION, AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CCFC_ERASURE,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CUSD相关命令 */
    { AT_CMD_CUSD_REQ,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CLCK相关命令 */
    { AT_CMD_CLCK_UNLOCK,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CLCK_LOCK,  AT_ProcMnCallBackCmdCnfCnapQry},
    { AT_CMD_CLCK_QUERY,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CLOP命令 */
    { AT_CMD_COLP_READ,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CLIR命令 */
    { AT_CMD_CLIR_READ,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CLIP命令 */
    { AT_CMD_CLIP_READ,  AT_ProcMnCallBackCmdCnfCnapQry},
    /* CPWD命令 */
    {AT_CMD_CPWD_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_CNAP_QRY, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_CSCA_READ, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CPMS_SET, AT_ProcMnCallBackCmdCnfCpmsCmd},
    {AT_CMD_CPMS_READ, AT_ProcMnCallBackCmdCnfCpmsCmd},
    {AT_CMD_D_CS_VOICE_CALL_SET, AT_ProcMnCallBackCmdCnfApdsCmd},
    {AT_CMD_APDS_SET, AT_ProcMnCallBackCmdCnfApdsCmd},
    {AT_CMD_D_CS_DATA_CALL_SET, AT_ProcMnCallBackCmdCnfCsDataCall},
    {AT_CMD_CHLD_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_CHUP_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_A_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_CHLD_EX_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_H_SET, AT_ProcMnCallBackCmdCnfCnapQry},
    {AT_CMD_CMGR_SET, At_CmdCmgdMsgProc},
    {AT_CMD_CMGD_SET, At_CmdCmgdMsgProc},
    {AT_CMD_CSMS_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMMS_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CGSMS_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CGSMS_READ, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMMS_READ, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CSMP_READ, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGS_TEXT_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGS_PDU_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGC_TEXT_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGC_PDU_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMSS_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMST_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CNMA_TEXT_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CNMA_PDU_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGW_PDU_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGW_TEXT_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGL_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CMGD_TEST, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CSMP_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CSCA_SET, AT_ProcMnCallBackCmdCnfCscaRead},
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    {AT_CMD_CSCB_SET, AT_ProcMnCallBackCmdCnfCscaRead},
    {AT_CMD_CSCB_READ, AT_ProcMnCallBackCmdCnfCscaRead},
#endif
};

static const AT_VC_EventProcFunc g_atVcEventProcFuncTbl[] = {
    {APP_VC_EVT_SET_VOLUME,  At_ProcVcSetVolumeEvent},
    {APP_VC_EVT_SET_VOICE_MODE, At_ProcVcSetVoiceMode},
    {APP_VC_EVT_GET_VOLUME, At_ProcVcGetVolumeEvent},
    {APP_VC_EVT_SET_MUTE_STATUS, At_ProcVcSetMuteStatusEvent},
    {APP_VC_EVT_GET_MUTE_STATUS, At_ProcVcGetMuteStatusEvent},
#if (FEATURE_ECALL == FEATURE_ON)
    {APP_VC_EVT_ECALL_TRANS_STATUS, At_ProcVcReportEcallStateEvent},
    {APP_VC_EVT_SET_ECALL_CFG, At_ProcVcSetEcallCfgEvent},
    {APP_VC_EVT_ECALL_ABORT_CNF, AT_ProcVcEcallAbortCnf},
    {APP_VC_EVT_ECALL_TRANS_ALACK, AT_ProcVcReportEcallAlackEvent},
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
    {APP_VC_EVT_SET_SMUT_STATUS, At_ProcVcSetSpeakerMuteStatusEvent},
    {APP_VC_EVT_GET_SMUT_STATUS, At_ProcVcGetSpeakerMuteStatusEvent},
    {APP_VC_EVT_SET_CMIC_STATUS, At_ProcVcSetCmicStatusEvent},
    {APP_VC_EVT_GET_CMIC_STATUS, At_ProcVcGetCmicVolumeEvent},
    {APP_VC_EVT_SET_ECHOEX, AT_ProcVcSetEchoSuppressionEvent},
    {APP_VC_EVT_GET_ECHOEX, AT_ProcVcGetEchoSuppressionEvent},
    {APP_VC_EVT_SET_PCMFR_STATUS, AT_ProcVcSetPcmFrStatusEvent},
    {APP_VC_EVT_GET_PCMFR_STATUS, AT_ProcVcGetPcmFrStatusEvent},
    {APP_VC_EVT_SET_ECHOSWITCH, AT_ProcVcSetEchoSwitchStatusEvent},
    {APP_VC_EVT_GET_ECHOSWITCH, AT_ProcVcGetEchoSwitchStatusEvent},
#endif
};
/*
 * 5 函数、变量声明
 */

VOS_UINT32 AT_CheckRptCmdStatus(VOS_UINT8 *rptCfg, AT_CmdRptCtrlTypeUint8 rptCtrlType, AT_RptCmdIndexUint8 rptCmdIndex)
{
    const AT_RptCmdIndexUint8 *rptCmdTblPtr = VOS_NULL_PTR;
    VOS_UINT32           rptCmdTblSize;
    VOS_UINT32           tableIndex;
    VOS_UINT32           offset;
    VOS_UINT8            bit;

    /* 主动上报命令索引错误，默认主动上报 */
    if (rptCmdIndex >= AT_RPT_CMD_BUTT) {
        return VOS_TRUE;
    }

    /* 主动上报受控类型填写错误，默认主动上报 */
    if (rptCtrlType == AT_CMD_RPT_CTRL_BUTT) {
        return VOS_TRUE;
    }

    if (rptCtrlType == AT_CMD_RPT_CTRL_BY_CURC) {
        rptCmdTblPtr  = AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_PTR();
        rptCmdTblSize = AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_SIZE();
    } else {
        rptCmdTblPtr  = AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_PTR();
        rptCmdTblSize = AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_SIZE();
    }

    for (tableIndex = 0; tableIndex < rptCmdTblSize; tableIndex++) {
        if (rptCmdIndex == rptCmdTblPtr[tableIndex]) {
            break;
        }
    }

    /* 与全局变量中的Bit位对比 */
    if (rptCmdTblSize != tableIndex) {
        /* 由于用户设置的字节序与Bit映射表序相反, 首先反转Bit位 */
        offset = AT_CURC_RPT_CFG_MAX_SIZE - ((VOS_UINT32)tableIndex / AT_BYTE_TO_BITS_LEN) - 1;
        bit    = (VOS_UINT8)(tableIndex % AT_BYTE_TO_BITS_LEN);

        return (VOS_UINT32)((rptCfg[offset] >> bit) & 0x1);
    }

    return VOS_TRUE;
}

TAF_UINT32 At_ChgTafErrorCode(TAF_UINT8 indexNum, TAF_ERROR_CodeUint32 tafErrorCode)
{
    TAF_UINT32 rtn = 0;
    VOS_UINT32 msgCnt;
    VOS_UINT32 i;
    VOS_UINT32 flag = 0;

    /* 获取消息个数 */
    msgCnt = sizeof(g_atChgTafErrCodeTbl) / sizeof(AT_ChgTafErrCodeTbl);

    for (i = 0; i < msgCnt; i++) {
        if (g_atChgTafErrCodeTbl[i].tafErrCode == tafErrorCode) {
            rtn  = g_atChgTafErrCodeTbl[i].atReturnCode;
            flag = flag + 1;
            break;
        }
    }

    if (flag == 0) {
        if (AT_IsBroadcastClientIndex(indexNum)) {
            rtn = AT_CME_UNKNOWN;
        } else if (g_parseContext[indexNum].cmdElement == VOS_NULL_PTR) {
            rtn = AT_CME_UNKNOWN;
        } else if ((g_parseContext[indexNum].cmdElement->cmdIndex > AT_CMD_SMS_BEGAIN) &&
                   (g_parseContext[indexNum].cmdElement->cmdIndex < AT_CMD_SMS_END)) {
            rtn = AT_CMS_UNKNOWN_ERROR;
        } else {
            rtn = AT_CME_UNKNOWN;
        }
    }

    return rtn;
}

/*
 * Description: 把CCA返回的CLASS以字符串方式输出，注意，不完整
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CcClass2Print(MN_CALL_TypeUint8 callType, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_PSAP_ECALL:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst, "VOICE");
            break;

        case MN_CALL_TYPE_FAX:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst, "FAX");
            break;

        case MN_CALL_TYPE_VIDEO:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst, "SYNC");
            break;

        case MN_CALL_TYPE_CS_DATA:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst, "DATA");
            break;

        default:
            AT_WARN_LOG("At_CcClass2Print CallType ERROR");
            break;
    }

    return length;
}

/* PC工程中AT从A核移到C核, At_sprintf有重复定义,故在此处添加条件编译宏 */

TAF_UINT32 At_HexAlpha2AsciiString(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT8 *src,
                                   TAF_UINT16 srcLen)
{
    TAF_UINT16 len    = 0;
    TAF_UINT16 chkLen = 0;
    TAF_UINT8 *write  = dst;
    TAF_UINT8 *read   = src;
    TAF_UINT8  high   = 0;
    TAF_UINT8  low    = 0;

    if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + (AT_ASCII_AND_HEX_CONVERSION_FACTOR * srcLen)) >= maxLength) {
        AT_ERR_LOG("At_HexAlpha2AsciiString too long");
        return 0;
    }

    if (srcLen != 0) {
        /* 扫完整个字串 */
        while (chkLen++ < srcLen) {
            high = 0x0F & (*read >> 4);
            low  = 0x0F & *read;

            len += 2; /* 记录长度,高位和低位长度为2 */

            if (high <= 0x09) { /* 0-9 */
                *write++ = high + 0x30;
            } else if (high >= 0x0A) { /* A-F */
                *write++ = high + 0x37;
            } else {
            }

            if (low <= 0x09) { /* 0-9 */
                *write++ = low + 0x30;
            } else if (low >= 0x0A) { /* A-F */
                *write++ = low + 0x37;
            } else {
            }

            /* 下一个字符 */
            read++;
        }
    }
    return len;
}

VOS_UINT32 AT_Hex2AsciiStrLowHalfFirst(VOS_UINT32 maxLength, VOS_INT8 *pcHeadaddr, VOS_UINT8 *dst, VOS_UINT8 *src,
                                       VOS_UINT16 srcLen)
{
    VOS_UINT16 len = 0;
    VOS_UINT16 chkLen = 0;
    VOS_UINT8 *pcWrite = VOS_NULL_PTR;
    VOS_UINT8 *pcRead  = VOS_NULL_PTR;
    VOS_UINT8  high;
    VOS_UINT8  low;

    pcWrite = dst;
    pcRead  = src;

    if (((VOS_UINT32)(dst - (VOS_UINT8 *)pcHeadaddr) + (AT_ASCII_AND_HEX_CONVERSION_FACTOR * srcLen)) >= maxLength) {
        AT_ERR_LOG("AT_Hex2AsciiStrLowHalfFirst too long");
        return 0;
    }

    if (srcLen != 0) {
        /* 扫完整个字串 */
        while (chkLen++ < srcLen) {
            high = 0x0F & (*pcRead >> 4);
            low  = 0x0F & *pcRead;

            len += 2; /* 记录长度,高位和低位长度为2 */

            /* 先转换低半字节 */
            if (low <= 0x09) { /* 0-9 */
                *pcWrite++ = low + 0x30;
            } else if (low >= 0x0A) { /* A-F */
                *pcWrite++ = low + 0x37;
            } else {
            }

            /* 再转换高半字节 */
            if (high <= 0x09) { /* 0-9 */
                *pcWrite++ = high + 0x30;
            } else if (high >= 0x0A) { /* A-F */
                *pcWrite++ = high + 0x37;
            } else {
            }

            /* 下一个字符 */
            pcRead++;
        }
    }

    return len;
}

TAF_VOID At_PhIndProc(TAF_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    switch (event->phoneEvent) {
        case TAF_PH_EVT_USIM_INFO_IND:
            AT_ProcUsimInfoInd(indexNum, event);
            return;

        case TAF_PH_EVT_SIMSQ_IND:
            AT_ProcSimsqInd(indexNum, event);
            return;

        case TAF_MMA_EVT_PS_INIT_RESULT_IND:
            At_RcvMmaPsInitResultIndProc(indexNum, event);
            return;

        case TAF_PH_EVT_OPER_MODE_IND:
            AT_NORM_LOG("At_PhIndProc TAF_PH_EVT_OPER_MODE_IND Do nothing");
            return;

        case MN_PH_EVT_SIMLOCKED_IND:
            AT_PhSendSimLocked();
            break;

        case MN_PH_EVT_ROAMING_IND:
            AT_PhSendRoaming(event->roamStatus);
            break;

        case TAF_PH_EVT_NSM_STATUS_IND:
            AT_RcvMmaNsmStatusInd(indexNum, event);
            break;

        case TAF_PH_EVT_USIM_MATCH_FILES_IND:
            AT_RcvMmaUsimMatchFilesInd(indexNum, event);
            break;
        default:
            AT_WARN_LOG("At_PhIndProc Other PhoneEvent");
            return;
    }
}

LOCAL VOS_UINT32 At_ProcPhoneEvtPlmnListAbortCnf(TAF_UINT8 indexNum)
{
    /*
     * 容错处理, 当前不在列表搜ABORT过程中则不上报ABORT.
     * 如AT的ABORT保护定时器已超时, 之后再收到MMA的ABORT_CNF则不上报ABORT
     */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_COPS_ABORT_PLMN_LIST) {
        AT_WARN_LOG("At_PhRspProc  NOT ABORT PLMN LIST. ");

        return AT_RRETURN_CODE_BUTT;
    }
    AT_StopTimerCmdReady(indexNum);

    return AT_ABORT;
}
TAF_VOID At_PhRspProc(TAF_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    TAF_UINT32    result = AT_FAILURE;
    TAF_UINT16    length = 0;

    switch (event->phoneEvent) {
        case TAF_PH_EVT_ERR:

            result = At_ChgTafErrorCode(indexNum, event->phoneError); /* 发生错误 */

            AT_StopTimerCmdReady(indexNum);

            break;

        case TAF_PH_EVT_PLMN_LIST_REJ:

            result = At_ProcPhoneEvtPlmnListRej(event, indexNum);

            break;

        case TAF_PH_EVT_OP_PIN_CNF:

            result = At_ProcPhoneEvtOperPinCnf(event, indexNum);

            if (result == AT_RRETURN_CODE_BUTT) {
                return;
            }

            break;

        case TAF_PH_EVT_OPER_MODE_CNF:

            result = At_ProcPhoneEvtOperModeCnf(event, &length, indexNum);

            if (result == AT_RRETURN_CODE_BUTT) {
                return;
            }

            break;

        case TAF_PH_EVT_USIM_RESPONSE:

            result = At_ProcPhoneEvtUsimResponse(event, &length, indexNum);

            break;

        case TAF_PH_EVT_RESTRICTED_ACCESS_CNF:

            result = At_ProcPhoneEvtRestrictedAccessCnf(event, &length, indexNum);

            break;

        case TAF_PH_EVT_OP_PINREMAIN_CNF:

            result = At_ProcPhoneEvtOpPinReminCnf(event, &length, indexNum);

            break;

        case TAF_PH_EVT_ME_PERSONALISATION_CNF:

            result = At_ProcPhoneEvtMePersonalizationCnf(event, &length, indexNum);

            break;

        case TAF_PH_EVT_SETUP_SYSTEM_INFO_RSP:

            AT_NORM_LOG("At_PhRspProc EVT SETUP SYSTEM INFO RSP,Do nothing.");

            return;

        case TAF_PH_EVT_PLMN_LIST_ABORT_CNF:

            result = At_ProcPhoneEvtPlmnListAbortCnf(indexNum);
            if (result == AT_RRETURN_CODE_BUTT) {
                return;
            }
            break;

        default:

            AT_WARN_LOG("At_PhRspProc Other PhoneEvent");

            return;
    }

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);
}

TAF_VOID At_PhEventProc(TAF_UINT8 *data, TAF_UINT16 len)
{
    TAF_PHONE_EventInfo *event    = VOS_NULL_PTR;
    TAF_UINT8            indexNum = 0;

    event = (TAF_PHONE_EventInfo *)data;

    AT_LOG1("At_PhMsgProc pEvent->ClientId", event->clientId);
    AT_LOG1("At_PhMsgProc PhoneEvent", event->phoneEvent);
    AT_LOG1("At_PhMsgProc PhoneError", event->phoneError);

    if (At_ClientIdToUserId(event->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_PhRspProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_PhIndProc(indexNum, event);
    } else {
        AT_LOG1("At_PhMsgProc ucIndex", indexNum);
        AT_LOG1("g_atClientTab[ucIndex].CmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

        At_PhRspProc(indexNum, event);
    }
}

TAF_VOID At_PhMsgProc(TAF_UINT8 *data, TAF_UINT16 len)
{
    At_PhEventProc(data, len);
}

TAF_UINT32 At_Unicode2UnicodePrint(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT8 *src,
                                   TAF_UINT16 srcLen)
{
    TAF_UINT16 len    = 0;
    TAF_UINT16 chkLen = 0;
    TAF_UINT8  high1  = 0;
    TAF_UINT8  high2  = 0;
    TAF_UINT8  low1   = 0;
    TAF_UINT8  low2   = 0;
    TAF_UINT8 *write  = dst;
    TAF_UINT8 *read   = src;

    if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + (AT_ASCII_AND_HEX_CONVERSION_FACTOR * srcLen)) >= maxLength) {
        AT_ERR_LOG("At_Unicode2UnicodePrint too long");
        return 0;
    }

    /* 扫完整个字串 */
    while (chkLen < srcLen) {
        /* 第一个字节 */
        high1 = 0x0F & (*read >> 4);
        high2 = 0x0F & *read;

        if (high1 <= 0x09) { /* 0-9 */
            *write++ = high1 + 0x30;
        } else if (high1 >= 0x0A) { /* A-F */
            *write++ = high1 + 0x37;
        } else {
        }

        if (high2 <= 0x09) { /* 0-9 */
            *write++ = high2 + 0x30;
        } else if (high2 >= 0x0A) { /* A-F */
            *write++ = high2 + 0x37;
        } else {
        }

        /* 下一个字符 */
        chkLen++;
        read++;

        /* 第二个字节 */
        low1 = 0x0F & (*read >> 4);
        low2 = 0x0F & *read;

        if (low1 <= 0x09) { /* 0-9 */
            *write++ = low1 + 0x30;
        } else if (low1 >= 0x0A) { /* A-F */
            *write++ = low1 + 0x37;
        } else {
        }

        if (low2 <= 0x09) { /* 0-9 */
            *write++ = low2 + 0x30;
        } else if (low2 >= 0x0A) { /* A-F */
            *write++ = low2 + 0x37;
        } else {
        }

        /* 下一个字符 */
        chkLen++;
        read++;

        len += 4; /* 记录长度，包含两个高位两个低位 */
    }

    return len;
}
/*
 * Description: Unicode到Unicode打印转换
 * History:
 *  1.Date: 2009-08-19
 *    Modification: Created function
 */
TAF_UINT32 At_HexString2AsciiNumPrint(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT8 *src,
                                      TAF_UINT16 srcLen)
{
    TAF_UINT16 len    = 0;
    TAF_UINT16 chkLen = 0;
    TAF_UINT8  high1  = 0;
    TAF_UINT8  high2  = 0;
    TAF_UINT8 *write  = dst;
    TAF_UINT8 *read   = src;

    if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + (AT_ASCII_AND_HEX_CONVERSION_FACTOR * srcLen)) >= maxLength) {
        AT_ERR_LOG("At_Unicode2UnicodePrint too long");
        return 0;
    }

    /* 扫完整个字串 */
    while (chkLen < srcLen) {
        /* 第一个字节 */
        high1 = 0x0F & (*read >> 4);
        high2 = 0x0F & *read;

        if (high1 <= 0x09) { /* 0-9 */
            *write++ = high1 + 0x30;
        } else if (high1 >= 0x0A) { /* A-F */
            *write++ = high1 + 0x37;
        } else {
        }

        if (high2 <= 0x09) { /* 0-9 */
            *write++ = high2 + 0x30;
        } else if (high2 >= 0x0A) { /* A-F */
            *write++ = high2 + 0x37;
        } else {
        }

        /* 下一个字符 */
        chkLen++;
        read++;
        len += 2; /* 记录长度,高位和低位长度为2 */
    }

    return len;
}

TAF_UINT32 At_Ascii2UnicodePrint(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT8 *src,
                                 TAF_UINT16 srcLen)
{
    TAF_UINT16 len    = 0;
    TAF_UINT16 chkLen = 0;
    TAF_UINT8 *write  = dst;
    TAF_UINT8 *read   = src;
    TAF_UINT8  high   = 0;
    TAF_UINT8  low    = 0;

    /*
     * 16进制转ascii所以乘2,同时在函数内部会通过 *write++ = '0';*write++ = '0';
     * 代码给每个字符前的1、2位加上00，所以相当于其长度有扩大了2倍，所以最后是乘以4
     */
    if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + (4 * srcLen)) >= maxLength) {
        AT_ERR_LOG("At_Ascii2UnicodePrint too long");
        return 0;
    }

    /* 扫完整个字串 */
    while (chkLen++ < srcLen) {
        *write++ = '0';
        *write++ = '0';
        high     = 0x0F & (*read >> 4);
        low      = 0x0F & *read;

        /* 2个'0',一个高位，一个低位，长度为4 */
        len += 4; /* 记录长度 */

        if (high <= 0x09) { /* 0-9 */
            *write++ = high + 0x30;
        } else if (high >= 0x0A) { /* A-F */
            *write++ = high + 0x37;
        } else {
        }

        if (low <= 0x09) { /* 0-9 */
            *write++ = low + 0x30;
        } else if (low >= 0x0A) { /* A-F */
            *write++ = low + 0x37;
        } else {
        }

        /* 下一个字符 */
        read++;
    }

    return len;
}

TAF_UINT16 At_PrintReportData(TAF_UINT32 maxLength, TAF_INT8 *headaddr, MN_MSG_MsgCodingUint8 msgCoding, TAF_UINT8 *dst,
                              TAF_UINT8 *src, TAF_UINT16 srcLen)
{
    TAF_UINT16 length = 0;
    TAF_UINT32 printStrLen = 0;
    TAF_UINT32 maxMemLength;
    VOS_UINT32 ret;
    errno_t    memResult;
    VOS_UINT8  printUserData[MN_MSG_MAX_8_BIT_LEN];

    /* DCS: UCS2 */
    if (msgCoding == MN_MSG_MSG_CODING_UCS2) {
        length += (TAF_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length, src,
                                                      srcLen);
    }
    /* DCS: 8BIT */
    else if (msgCoding == MN_MSG_MSG_CODING_8_BIT) { /* DATA:8BIT */
        if (g_atCscsType == AT_CSCS_GSM_7Bit_CODE) {
            memset_s(printUserData, sizeof(printUserData), 0, MN_MSG_MAX_8_BIT_LEN);

            srcLen      = TAF_MIN(srcLen, MN_MSG_MAX_8_BIT_LEN);
            printStrLen = 0;
            ret         = TAF_STD_ConvertAsciiToDefAlpha(src, srcLen, printUserData, &printStrLen, srcLen);
            if (ret != MN_ERR_NO_ERROR) {
                AT_NORM_LOG("At_PrintReportData : TAF_STD_ConvertAsciiToDefAlpha fail. ");
                return 0;
            }

            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                          printUserData, (VOS_UINT16)printStrLen);
        } else {
            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                          src, srcLen);
        }
    }
    /* DCS: 7BIT */
    else {
        if (g_atCscsType == AT_CSCS_UCS2_CODE) { /* +CSCS:UCS2 */
            length += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress, dst + length,
                                                        src, srcLen);
        } else {
            if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + srcLen) >= maxLength) {
                AT_ERR_LOG("At_PrintReportData too long");
                return 0;
            }

            if ((g_atCscsType == AT_CSCS_IRA_CODE) && (msgCoding == MN_MSG_MSG_CODING_7_BIT)) {
                TAF_STD_ConvertDefAlphaToAscii(src, srcLen, (dst + length), &printStrLen);
                length += (TAF_UINT16)printStrLen;
            } else {
                maxMemLength = maxLength - (TAF_UINT32)(dst - (TAF_UINT8 *)headaddr);
                if (srcLen > 0) {
                    memResult = memcpy_s((dst + length), maxMemLength, src, srcLen);
                    TAF_MEM_CHK_RTN_VAL(memResult, maxMemLength, srcLen);
                }
                length += srcLen;
            }
        }
    }

    return length;
}

VOS_UINT32 AT_IsClientBlock(VOS_VOID)
{
    VOS_UINT32          atStatus;
    VOS_UINT32          atMode;
    AT_PortBuffCfgUint8 smsBuffCfg;

    smsBuffCfg = AT_GetPortBuffCfg();
    if (smsBuffCfg == AT_PORT_BUFF_DISABLE) {
        return VOS_FALSE;
    }

    atStatus = AT_IsAnyParseClientPend();
    atMode   = AT_IsAllClientDataMode();

    /* 若当前有一个通道处于 pend状态，则需要缓存短信 */
    if (atStatus == VOS_TRUE) {
        return VOS_TRUE;
    }

    /* 若当前所有通道都处于data模式，则缓存短信 */
    if (atMode == VOS_TRUE) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID At_VcEventProc(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvt, APP_VC_EventUint32 event)
{
    AT_VC_EventMatchFunc      atVcEventMatchFunc = VOS_NULL_PTR;
    VOS_UINT32                arraySize;
    VOS_UINT32                i;

    arraySize = AT_ARRAY_SIZE(g_atVcEventProcFuncTbl);

    for (i = 0; i < arraySize; i++) {
        if (g_atVcEventProcFuncTbl[i].event == event) {
            atVcEventMatchFunc = g_atVcEventProcFuncTbl[i].atVcEventMatchFunc;
            break;
        }
    }

    if (atVcEventMatchFunc != VOS_NULL_PTR) {
        atVcEventMatchFunc(indexNum, vcEvt);
    }

    return;
}

TAF_VOID At_VcMsgProc(MN_AT_IndEvt *data, TAF_UINT16 len)
{
    APP_VC_EventInfo  *event = VOS_NULL_PTR;
    APP_VC_EventUint32 eventTemp = APP_VC_EVT_BUTT;
    TAF_UINT8          indexNum = 0;
    TAF_UINT32         eventLen;
    errno_t            memResult;

    AT_INFO_LOG("At_VcMsgProc: Step into function.");
    AT_LOG1("At_VcMsgProc: pstData->clientId,", data->clientId);

    eventLen  = sizeof(APP_VC_EventUint32);
    memResult = memcpy_s(&eventTemp, sizeof(eventTemp), data->content, eventLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(eventTemp), eventLen);
    event = (APP_VC_EventInfo *)&data->content[eventLen];

    if (At_ClientIdToUserId(data->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_VcMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (!AT_IsBroadcastClientIndex(indexNum)) {
        AT_LOG1("At_VcMsgProc: ucIndex", indexNum);
        if (indexNum >= AT_MAX_CLIENT_NUM) {
            AT_WARN_LOG("At_VcMsgProc: invalid CLIENT ID or indexNum.");
            return;
        }

        if (event->opId != g_atClientTab[indexNum].opId) {
            AT_LOG1("At_VcMsgProc: pstEvent->opId,", event->opId);
            AT_LOG1("At_VcMsgProc: g_atClientTab[ucIndex].opId,", g_atClientTab[indexNum].opId);
            AT_NORM_LOG("At_VcMsgProc: invalid operation id.");
            return;
        }

        AT_LOG1("g_atClientTab[ucIndex].CmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);
    }

    if (eventTemp >= APP_VC_EVT_BUTT) {
        AT_WARN_LOG("At_SmsRspProc: invalid event type.");
        return;
    }

    AT_LOG1("At_VcMsgProc enEvent", eventTemp);
    At_VcEventProc(indexNum, event, eventTemp);
}

TAF_VOID At_SetParaRspProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_PARA_SET_RESULT result, TAF_PARA_TYPE paraType)
{
    AT_RreturnCodeUint32 ret    = AT_FAILURE;
    TAF_UINT16           length = 0;

    /* 如果是PS域的复合命令 */
    if (g_atClientTab[indexNum].asyRtnNum > 0) {
        g_atClientTab[indexNum].asyRtnNum--; /* 命令个数减1 */
        if (result == TAF_PARA_OK) {
            if (g_atClientTab[indexNum].asyRtnNum != 0) {
                return; /* 如果OK并且还有其它命令 */
            }
        } else {
            g_atClientTab[indexNum].asyRtnNum = 0; /* 如果ERROR则不再上报其它命令结果 */
        }
    }

    AT_StopTimerCmdReady(indexNum);

    switch (result) {
        case TAF_PARA_OK:
            ret = AT_OK;
            break;

        case TAF_PARA_SIM_IS_BUSY:
            if (g_parseContext[indexNum].cmdElement->cmdIndex > AT_CMD_SMS_BEGAIN) {
                ret = AT_CMS_U_SIM_BUSY;
            } else {
                ret = AT_CME_SIM_BUSY;
            }
            break;

        default:
            if (g_parseContext[indexNum].cmdElement->cmdIndex > AT_CMD_SMS_BEGAIN) {
                ret = AT_CMS_UNKNOWN_ERROR;
            } else {
                ret = AT_CME_UNKNOWN;
            }
            break;
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, ret);
}

TAF_VOID At_SetMsgProc(TAF_SetRslt *setRslt)
{
    TAF_UINT8 indexNum;

    AT_LOG1("At_SetMsgProc ClientId", setRslt->clientId);
    AT_LOG1("At_SetMsgProc Result", setRslt->rslt);
    AT_LOG1("At_SetMsgProc ParaType", setRslt->paraType);

    if (setRslt->clientId == AT_BUTT_CLIENT_ID) {
        AT_WARN_LOG("At_SetMsgProc Error ucIndex");
        return;
    }

    indexNum = 0;
    if (At_ClientIdToUserId(setRslt->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_SetMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_SetMsgProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_SetMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    At_SetParaRspProc(indexNum, setRslt->opId, setRslt->rslt, setRslt->paraType);
}

TAF_UINT32 At_PIHNotBroadIndProc(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_PIHNotBroadIndProc: Get modem id fail.");
        return VOS_ERR;
    }

    switch (event->eventType) {
        case SI_PIH_EVENT_PRIVATECGLA_SET_IND:

            /* ^CGLA查询请求下发后会有多条IND上报，通过当前通道上报不需要广播 */
            length += At_PrintPrivateCglaResult(indexNum, event);
            break;

        default:
            AT_WARN_LOG("At_PIHNotBroadIndProc: Abnormal EventType.");
            return VOS_ERR;
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

TAF_VOID At_PIHRspProc(TAF_UINT8 indexNum, SI_PIH_EventInfo *event)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT16 length = 0;
    TAF_UINT32 tmp;
    TAF_UINT32 i;

    if (event->pihError != TAF_ERR_NO_ERROR) { /* 错误 */
        AT_StopTimerCmdReady(indexNum);

        result = At_ChgTafErrorCode(indexNum, (TAF_UINT16)(event->pihError)); /* 发生错误 */

        g_atSendDataBuff.bufLen = length;

        At_FormatResultData(indexNum, result);

        return;
    }

    tmp = (sizeof(g_atPihRspProcFuncTbl) / sizeof(g_atPihRspProcFuncTbl[0]));

    for (i = 0; i < tmp; i++) {
        /* 找到处理函数，进行输出相关处理 */
        if (event->eventType == g_atPihRspProcFuncTbl[i].eventType) {
            result = g_atPihRspProcFuncTbl[i].atPihRspProcFunc(indexNum, event, &length);

            if (result == AT_ERROR) {
                AT_WARN_LOG("At_PIHRspProc : pAtPihRspProcFunc is return error!");
                return;
            }

            break;
        }
    }

    /* 没找到处理函数，直接返回 */
    if (i == tmp) {
        AT_WARN_LOG("At_PIHRspProc : no find AT Proc Func!");
        return;
    }

    result = AT_OK;

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);

    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;
}

VOS_UINT32 AT_ChkPIHMsgLen(struct MsgCB *msg)
{
    MN_APP_PihAtCnf  *msgTemp  = VOS_NULL_PTR;
    SI_PIH_EventInfo *event = VOS_NULL_PTR;
    TAF_UINT32        size;
    TAF_UINT32        i;

    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < AT_GET_PIH_AT_CNF_LEN_WITHOUT_UNION) {
        AT_WARN_LOG("AT_ChkPIHMsgLen: message length is invalid!");
    }
    msgTemp = (MN_APP_PihAtCnf *)msg;
    event = &msgTemp->pihAtEvent;

    size = (sizeof(g_atPihRspProcFuncTbl) / sizeof(g_atPihRspProcFuncTbl[0]));

    for (i = 0; i < size; i++) {
        if (event->eventType == g_atPihRspProcFuncTbl[i].eventType) {
#if (VOS_OS_VER == VOS_WIN32)
            if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) == sizeof(MN_APP_PihAtCnf)) {
                return VOS_TRUE;
            }
#endif
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)msg,
                    (AT_GET_PIH_AT_CNF_LEN_WITHOUT_UNION + g_atPihRspProcFuncTbl[i].fixdedPartLen),
                    g_atPihRspProcFuncTbl[i].chkFunc) != VOS_TRUE) {
                AT_WARN_LOG("AT_ChkPIHMsgLen: message length is invalid!");
                return VOS_FALSE;
            }
            break;
        }
    }
    return VOS_TRUE;
}

TAF_VOID At_PIHMsgProc(struct MsgCB *msg)
{
    MN_APP_PihAtCnf *msgTemp  = VOS_NULL_PTR;
    TAF_UINT8        indexNum = 0;

    if (AT_ChkPIHMsgLen(msg) != VOS_TRUE) {
        AT_ERR_LOG("At_PIHMsgProc: message length is invalid!");
        return;
    }

    msgTemp = (MN_APP_PihAtCnf *)msg;

    if (msgTemp->msgId != PIH_AT_EVENT_CNF) {
        AT_ERR_LOG1("At_PIHMsgProc: The Msg Id is Wrong", msgTemp->msgId);
        return;
    }

    AT_LOG1("At_PIHMsgProc pEvent->ClientId", msgTemp->pihAtEvent.clientId);
    AT_LOG1("At_PIHMsgProc EventType", msgTemp->pihAtEvent.eventType);
    AT_LOG1("At_PIHMsgProc SIM Event Error", msgTemp->pihAtEvent.pihError);

    if (At_ClientIdToUserId(msgTemp->pihAtEvent.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("At_PIHMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_PIHIndProc(indexNum, &msgTemp->pihAtEvent);
        AT_WARN_LOG("At_PIHMsgProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_PIHMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    /* 非广播的主动上报 */
    if (At_PIHNotBroadIndProc(indexNum, &msgTemp->pihAtEvent) == VOS_OK) {
        return;
    }

    At_PIHRspProc(indexNum, &msgTemp->pihAtEvent);
}

TAF_VOID At_DataStatusMsgProc(TAF_DataStatusInd *dataStatus)
{
    AT_RreturnCodeUint32 result   = AT_FAILURE;
    TAF_UINT8            indexNum = 0;

    AT_LOG1("At_DataStatusMsgProc: ClientId", dataStatus->clientId);
    AT_LOG1("At_DataStatusMsgProc: ucDomain", dataStatus->domain);
    AT_LOG1("At_DataStatusMsgProc: ucRabId", dataStatus->rabId);
    AT_LOG1("At_DataStatusMsgProc: ucStatus", dataStatus->status);
    AT_LOG1("At_DataStatusMsgProc: ucRabId", dataStatus->cause);
    if (At_ClientIdToUserId(dataStatus->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_DataStatusMsgProc: At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_DataStatusMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_DataStatusMsgProc: ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    switch (dataStatus->status) {
        case TAF_RABM_STOP_DATA:
        case TAF_DATA_STOP:
            break;

        default:
            break;
    }

    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_ConvertCallError(TAF_CS_CauseUint32 cause)
{
    const AT_CmeCallErrCodeMap *callErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32                  callErrMapTblSize;
    VOS_UINT32                  cnt;

    callErrMapTblPtr  = AT_GET_CME_CALL_ERR_CODE_MAP_TBL_PTR();
    callErrMapTblSize = AT_GET_CME_CALL_ERR_CODE_MAP_TBL_SIZE();

    for (cnt = 0; cnt < callErrMapTblSize; cnt++) {
        if (callErrMapTblPtr[cnt].csCause == cause) {
            return callErrMapTblPtr[cnt].cmeCode;
        }
    }

    return AT_CME_UNKNOWN;
}

/*
 * MN_CALLBACK_CMD_CNF消息的分发处理
 */
LOCAL VOS_UINT32 AT_MnCallBackCmdCnfCmdOptDistr(AT_CmdCurOptUint32 cmdCurrentOpt, TAF_UINT32 errorCode, TAF_UINT8 indexNum)
{
    AT_MnCallBackCmdCnfCmdOptMatchFunc      pktTypeMatchFunc = VOS_NULL_PTR;
    TAF_UINT32                              result   = AT_FAILURE;
    VOS_UINT32                              arraySize;
    VOS_UINT32                              i;

    arraySize = AT_ARRAY_SIZE(g_atMnCallBackCmdCnfOptFuncTbl);

    for (i = 0; i < arraySize; i++) {
        if (g_atMnCallBackCmdCnfOptFuncTbl[i].atCmdType == cmdCurrentOpt) {
            pktTypeMatchFunc = g_atMnCallBackCmdCnfOptFuncTbl[i].pktTypeMatchFunc;
        }
    }

    if (pktTypeMatchFunc != VOS_NULL_PTR) {
        result = pktTypeMatchFunc(indexNum, errorCode);
    }else {
        /* 默认值不知道是不是该填这个，暂时先写这个 */
        result = AT_CME_UNKNOWN;
        AT_StopTimerCmdReady(indexNum);
    }

    return result;
}

TAF_VOID At_CmdCnfMsgProc(TAF_UINT8 *data, TAF_UINT16 len)
{
    AT_CmdCnfEvent *cmdCnf = VOS_NULL_PTR;
    MN_CLIENT_ID_T  clientId;
    TAF_UINT32      errorCode;
    TAF_UINT8       indexNum = 0;
    TAF_UINT32      result   = AT_FAILURE;
    TAF_UINT16      length   = 0;

    cmdCnf = (AT_CmdCnfEvent *)data;

    clientId  = cmdCnf->clientId;
    errorCode = cmdCnf->errorCode;

    if (At_ClientIdToUserId(clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_CmdCnfMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_CmdCnfMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    if (g_parseContext[indexNum].clientStatus == AT_FW_CLIENT_STATUS_READY) {
        AT_WARN_LOG("At_CmdCnfMsgProc : AT command entity is released.");
        return;
    }

    /*
     * call业务上报的是TAF_CS_CAUSE_SUCCESS，而短信业务上报的是MN_ERR_NO_ERROR,
     * 他们的实际值都为0
     */
    if (errorCode == MN_ERR_NO_ERROR) {
        /*
         * 因判断是否有呼叫在C核上实现，在无呼叫的情况下上报MN_ERR_NO_ERROR
         * AT命令返回结果需要为AT_OK
         */
        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_H_SET ||
            g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CHUP_SET) {
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, AT_OK);
        }

        AT_NORM_LOG("At_CmdCnfMsgProc Rsp No Err");
        return;
    }

    AT_LOG1("At_CmdCnfMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    result = AT_MnCallBackCmdCnfCmdOptDistr(g_atClientTab[indexNum].cmdCurrentOpt, errorCode, indexNum);

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_PrcoPsEvtErrCode(VOS_UINT8 indexNum, TAF_PS_CauseUint32 cuase)
{
    VOS_UINT32 result;

    /* 转换错误码格式 */
    if (cuase != TAF_PS_CAUSE_SUCCESS) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    /* 清除AT操作符, 并停止定时器 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_LogPrintMsgProc(TAF_MntnLogPrint *msg)
{
    AT_PR_LOGI("[MDOEM:%d]%s", msg->modemId, msg->log);
}

VOS_UINT32 AT_IsBroadcastPsEvt(TAF_PS_EvtIdUint32 evtId)
{
    VOS_UINT32 i;

    for (i = 0; i < AT_ARRAY_SIZE(g_atBroadcastPsEvtTbl); i++) {
        if (evtId == g_atBroadcastPsEvtTbl[i]) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_VOID AT_RcvTafPsEvt(TAF_PS_Evt *evt)
{
    VOS_UINT32     i;
    VOS_UINT32     result = VOS_OK;
    VOS_UINT8      indexNum = 0;
    MN_PS_EVT_FUNC evtFunc = VOS_NULL_PTR;
    TAF_Ctrl      *ctrl = VOS_NULL_PTR;

    /* 初始化 */
    ctrl    = (TAF_Ctrl *)(evt->content);

    if (At_ClientIdToUserId(ctrl->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvTafPsEvt: At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        /*
         * 广播IDNEX不可以作为数组下标使用，需要在事件处理函数中仔细核对，避免数组越界。
         * 目前只有流量上报/NW ACT/NW DISCONNET为广播事件，需要添加其它广播事件，请仔细核对，
         */
        if (AT_IsBroadcastPsEvt(evt->evtId) == VOS_FALSE) {
            AT_WARN_LOG("AT_RcvTafPsEvt: AT_BROADCAST_INDEX,but not Broadcast Event.");
            return;
        }
    }

    /* 在事件处理表中查找处理函数 */
    for (i = 0; i < AT_ARRAY_SIZE(g_atPsEvtFuncTbl); i++) {
        if (evt->evtId == g_atPsEvtFuncTbl[i].evtId) {
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)evt, g_atPsEvtFuncTbl[i].msgLen,
                    g_atPsEvtFuncTbl[i].chkFunc) == VOS_FALSE) {
                AT_ERR_LOG("AT_RcvTafPsEvt: Check MsgLenth Err");
                return;
            }

            /* 事件ID匹配 */
            evtFunc = g_atPsEvtFuncTbl[i].evtFunc;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    if (evtFunc != VOS_NULL_PTR) {
        result = evtFunc(indexNum, (TAF_Ctrl *)evt->content);
    } else {
        AT_ERR_LOG1("AT_RcvTafPsEvt: Unexpected event received! <EvtId>", evt->evtId);
        result = VOS_ERR;
    }

    /* 根据处理函数的返回结果, 决定是否清除AT定时器以及操作符: 该阶段不考虑 */
    if (result != VOS_OK) {
        AT_ERR_LOG1("AT_RcvTafPsEvt: Can not handle this message! <MsgId>", evt->evtId);
    }
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_ConvertMultiSNssaiToString(VOS_UINT8 sNssaiNum, const PS_S_NSSAI_STRU *sNssai, VOS_CHAR *pcStrNssai,
                                       VOS_UINT32 srcNssaiLength, VOS_UINT32 *dsrLength)
{
    VOS_UINT32 length = 0;
    VOS_UINT32 loop;
    VOS_INT32  bufLen;

    /*
     * 27007 rel15, 10.1.1章节
     * sst                                     only slice/service type (SST) is present
     * sst;mapped_sst                          SST and mapped configured SST are present
     * sst.sd                                  SST and slice differentiator (SD) are present
     * sst.sd;mapped_sst                       SST, SD and mapped configured SST are present
     * sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present
     */

    *dsrLength = 0;

    for (loop = 0; loop < sNssaiNum; loop++) {
        if (length >= AT_EVT_MULTI_S_NSSAI_LEN) {
            AT_ERR_LOG1("AT_ConvertMultiSNssaiToString:ERROR: ulLength abnormal:", length);
            *dsrLength = 0;

            return;
        }

        /* 如果有多个S-NSSAI，每个S-NSSAI通过":"分割 */
        if (loop != 0) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, ":");
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, srcNssaiLength - length, (srcNssaiLength - length) - 1);
            if (bufLen > 0) {
                length = length + (VOS_UINT32)bufLen;
            }
        }

        if ((sNssai[loop].bitOpSd == VOS_TRUE) && (sNssai[loop].bitOpMappedSst == VOS_TRUE) &&
            (sNssai[loop].bitOpMappedSd == VOS_TRUE)) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1,
                                "%02x.%06x;%02x.%06x", sNssai[loop].ucSst, sNssai[loop].ulSd, sNssai[loop].ucMappedSst,
                                sNssai[loop].ulMappedSd);
            /* sst/sd/mapped sst/mapped sd共8个字节 */
            *dsrLength += 8;
        } else if ((sNssai[loop].bitOpSd == VOS_TRUE) && (sNssai[loop].bitOpMappedSst == VOS_TRUE)) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1,
                                "%02x.%06x;%02x", sNssai[loop].ucSst, sNssai[loop].ulSd, sNssai[loop].ucMappedSst);
            /* sst/sd/mapped sst共5个字节 */
            *dsrLength += 5;
        } else if (sNssai[loop].bitOpSd == VOS_TRUE) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, "%02x.%06x",
                                sNssai[loop].ucSst, sNssai[loop].ulSd);
            /* sst/sd共4个字节 */
            *dsrLength += 4;
        } else if (sNssai[loop].bitOpMappedSst == VOS_TRUE) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, "%02x;%02x",
                                sNssai[loop].ucSst, sNssai[loop].ucMappedSst);
            /* sst/mapped sst共2个字节 */
            *dsrLength += 2;
        } else {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, "%02x",
                                sNssai[loop].ucSst);
            /* sst1个字节 */
            *dsrLength += 1;
        }
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, srcNssaiLength - length, (srcNssaiLength - length) - 1);

        if (bufLen > 0) {
            length = length + (VOS_UINT32)bufLen;
        }
    }
}
#endif

VOS_BOOL AT_PH_IsPlmnValid(TAF_PLMN_Id *plmnId)
{
    VOS_UINT32 i;

    for (i = 0; i < AT_MCC_LENGTH; i++) {
        if ((((plmnId->mcc >> (i * AT_HEX_OCCUPIED_BIT_NUM)) & 0x0F) > AT_DEC_MAX_NUM) ||
            ((((plmnId->mnc >> (i * AT_HEX_OCCUPIED_BIT_NUM)) & 0x0F) > AT_DEC_MAX_NUM) &&
             (i != AT_MNC_LENGTH_TWO_BYTES)) ||
            ((((plmnId->mnc >> (i * AT_HEX_OCCUPIED_BIT_NUM)) & 0x0F) > AT_DEC_MAX_NUM) &&
            (((plmnId->mnc >> (i * AT_HEX_OCCUPIED_BIT_NUM)) & 0x0F) != 0x0F))) {
            /* PLMN ID无效 */
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}

VOS_CHAR* AT_GetPdpTypeStr(TAF_PDP_TypeUint8 pdpType)
{
    switch (pdpType) {
        case TAF_PDP_IPV4:
            return (VOS_CHAR *)g_atStringTab[AT_STRING_IPv4].text;
        case TAF_PDP_IPV6:
            return (VOS_CHAR *)g_atStringTab[AT_STRING_IPv6].text;
        case TAF_PDP_IPV4V6:
            return (VOS_CHAR *)g_atStringTab[AT_STRING_IPv4v6].text;
        case TAF_PDP_ETHERNET:
            return (VOS_CHAR *)g_atStringTab[AT_STRING_Ethernet].text;
        default:
            AT_WARN_LOG1("AT_GetPdpTypeStr: Invalid PDP Type!", pdpType);
            break;
    }

    return (VOS_CHAR *)g_atStringTab[AT_STRING_IP].text;
}

