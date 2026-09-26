/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "taf_type_def.h"
#include "taf_msg_chk_api.h"
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
#include "at_cmd_msg_proc.h"
#include "at_ndis_interface.h"
#include "dms_task.h"
#include "dms_port.h"
#include "at_event_report.h"
#include "ppp_interface.h"
#include "si_app_pb.h"
#endif
#include "at_rnic_interface.h"
#include "at_internal_msg.h"
#include "taf_api.h"
#include "mn_msg_api.h"
#include "taf_ssa_api.h"
#include "taf_app_mma.h"
#include "app_vc_api.h"
#include "at_mn_interface.h"
#include "taf_drv_agent.h"
#include "dsm_ndis_pif.h"
#include "taf_ps_api.h"
#include "gen_msg.h"
#include "msp_at.h"
#include "at_rabm_interface.h"
#include "at_mta_interface.h"
#include "at_imsa_interface.h"
#include "css_at_interface.h"
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
#include "nascbtinterface.h"
#endif
#include "taf_ccm_api.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#include "taf_v2x_api.h"
#endif
#include "at_oam_interface.h"
#include "at_phy_interface.h"
#include "at_mt_interface.h"
#include "throt_at_interface.h"


#define THIS_FILE_ID PS_FILE_ID_DMS_MSG_CHK_C

#define AT_GET_SI_PB_EVENT_LEN_WITHOUT_UNION (sizeof(SI_PB_EventInfo) - sizeof(((SI_PB_EventInfo*)16)->pbEvent))
#define AT_GET_PB_AT_CNF_LEN_WITHOUT_UNION (sizeof(MN_APP_PbAtCnf) - sizeof(((MN_APP_PbAtCnf *)16)->pbAtEvent.pbEvent))

STATIC VOS_UINT32 AT_ChkCssCloudDataReportMsgLen(const MSG_Header *msgHeader);
#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkThrotSingleApnBackoffTimeParaNtfMsgLen(const MSG_Header *msgHeader);
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkThrotSingleSnssaiBackoffParaNtfMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkThrotSingleSnssaiDnnBackoffParaNtfMsgLen(const MSG_Header *msgHeader);
#endif

STATIC VOS_UINT32 AT_ChkImsaFusioncallCtrlMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkMtaQryNmrMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkMtaLrrcUeCapParaInfoNtfMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkMtaEcidSetCnfMsgLen(const MSG_Header *msgHeader);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkMtaNrrcUeCapParaInfoNtfMsgLen(const MSG_Header *msgHeader);
#endif
#if (FEATURE_ON== FEATURE_UE_MODE_CDMA)
STATIC VOS_UINT32 AT_ChkMtaEvdoRevARLinkMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkMtaEvdoSigExEventMsgLen(const MSG_Header *msgHeader);
#endif
STATIC VOS_UINT32 AT_ChkDrvAgentSimlockDataReadExReadCnfMsgLen(const MSG_Header *msg);
STATIC VOS_UINT32 AT_ChkFtmSetLtCommCmdCnfMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkFtmRdLtCommCmdCnfMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 AT_ChkMnCallBackQryMsgLen(const MSG_Header *msg);
STATIC VOS_UINT32 AT_ChkMnCallBackSsMsgLen(const MSG_Header *msg);
#if (VOS_OS_VER == VOS_WIN32)
STATIC VOS_UINT32 AT_ChkMnCallBackPsCallMsgLen(const MSG_Header *msg);
#endif
STATIC VOS_UINT32 AT_ChkMnCallBackPBMsgLen(const MSG_Header *msg);
STATIC VOS_UINT32 AT_ChkDrvAgentMemInfoQryRspMsgLen(const MSG_Header *msg);
STATIC VOS_UINT32 AT_ChkDrvAgentNandBadBlockQryRspMsgLen(const MSG_Header *msg);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkTafMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkTafMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDsmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkDsmMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDrvAgentMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkDrvAgentMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVcMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVcMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCcMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkCcMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMspL4aMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkMspL4aMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMspSysFtmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkMspSysFtmMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMapsStkMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkMapsStkMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkRabmMsgMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkRabmMsgMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkSpyMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkSpyMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMtaMsgLenMapTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkMtaMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkImsaMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkImsaMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCssMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkCssMsgLenTblSize(VOS_VOID);
#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkThrotMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkThrotMsgLenTblSize(VOS_VOID);
#endif
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
STATIC VOS_UINT32 AT_ChkUeCbtRfIcIdExQryCnfMsgLen(const MSG_Header *msgHeader);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkBbicMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkBbicMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkPamMfgMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkPamMfgMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDspIdleMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkDspIdleMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCbtMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkCbtMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCcmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkCcmMsgLenTblSize(VOS_VOID);
#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVnasMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVnasMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVrrcMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVrrcMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVmacUlMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVmacUlMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVtcMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVtcMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVpdcpUlMsgTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkVpdcpUlMsgTblSize(VOS_VOID);
#endif

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
STATIC VOS_UINT32 AT_ChkDmsIntraPortEventIndMsgLen(const MSG_Header *msg);
STATIC TAF_ChkMsgLenNameMapTbl* DMS_GetChkDmsMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 DMS_GetChkDmsMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkAppPppMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkAppPppMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDmsMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkDmsMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkAppNdisMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkAppNdisMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkEmatMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkEmatMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkRnicMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 AT_GetChkRnicMsgLenTblSize(VOS_VOID);

static TAF_ChkMsgLenNameMapTbl g_atChkTafMsgLenTbl[] = {
    { MN_CALLBACK_MSG, (sizeof(MN_AT_IndEvt) + sizeof(MN_MSG_EventInfo)), VOS_NULL_PTR },
    { MN_CALLBACK_SET, (sizeof(MN_AT_IndEvt) + sizeof(TAF_SetRslt) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { MN_CALLBACK_QRY, 0, AT_ChkMnCallBackQryMsgLen },
#if (VOS_OS_VER == VOS_WIN32)
    { MN_CALLBACK_PS_CALL, 0, AT_ChkMnCallBackPsCallMsgLen },
#else
    { MN_CALLBACK_PS_CALL, (sizeof(TAF_PS_Evt) + sizeof(TAF_Ctrl) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
    { MN_CALLBACK_DATA_STATUS, (sizeof(MN_AT_IndEvt) + sizeof(TAF_DataStatusInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { MN_CALLBACK_SS, 0, AT_ChkMnCallBackSsMsgLen },
    { MN_CALLBACK_PHONE, (sizeof(MN_AT_IndEvt) + sizeof(TAF_PHONE_EventInfo) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { MN_CALLBACK_PHONE_BOOK , 0, AT_ChkMnCallBackPBMsgLen },
    { MN_CALLBACK_CMD_CNF, (sizeof(MN_AT_IndEvt) + sizeof(AT_CmdCnfEvent) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { MN_CALLBACK_VOICE_CONTROL, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_EventInfo)), VOS_NULL_PTR },
    { MN_CALLBACK_LOG_PRINT, sizeof(TAF_MntnLogPrint), VOS_NULL_PTR },
    { MN_CALLBACK_IFACE, (sizeof(TAF_PS_Evt) + sizeof(TAF_Ctrl) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkDsmMsgLenTbl[] = {
    { ID_DSM_NDIS_IFACE_UP_IND, sizeof(DSM_NDIS_IfaceUpInd), VOS_NULL_PTR },
    { ID_DSM_NDIS_IFACE_DOWN_IND, sizeof(DSM_NDIS_IfaceDownInd), VOS_NULL_PTR },
    { ID_DSM_NDIS_CONFIG_IPV6_DNS_IND, sizeof(DSM_NDIS_ConfigIpv6DnsInd), VOS_NULL_PTR },
};

/* AT接收WUEPS_PID_DRV_AGENT消息 */
static TAF_ChkMsgLenNameMapTbl g_atChkDrvAgentMsgLenTbl[] = {
    { DRV_AGENT_MSID_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_MsidQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_GPIOPL_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_GpioplSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_GPIOPL_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_GpioplQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_DATALOCK_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_DatalockSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_TBATVOLT_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_TbatvoltQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_VERTIME_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_VersionTime) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_YJCX_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_YjcxSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_YJCX_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_YjcxQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_APPDMVER_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_AppdmverQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_DLOADINFO_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_DloadinfoQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_FLASHINFO_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_FlashinfoQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_AUTHVER_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_AuthverQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_AUTHORITYID_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_AuthorityidQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { DRV_AGENT_GODLOAD_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_GodloadSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { DRV_AGENT_HWNATQRY_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_HwnatqryQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_CPULOAD_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_CpuloadQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_MFREELOCKSIZE_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_MfreelocksizeQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_MEMINFO_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_MeminfoQryRsp) - TAF_DEFAULT_CONTENT_LEN),
     AT_ChkDrvAgentMemInfoQryRspMsgLen },
    { DRV_AGENT_HARDWARE_QRY_RSP, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_HardwareQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_FULL_HARDWARE_QRY_RSP,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_FullHardwareQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_VERSION_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_VersionQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_PFVER_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_PfverQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
    { DRV_AGENT_FCHAN_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_FchanSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { DRV_AGENT_PRODTYPE_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_ProdtypeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_RXDIV_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_QryRxdivCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_SIMLOCK_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SetSimlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_IMSICHG_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_ImsichgQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_NVRESTORE_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvrestoreRst) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_INFORBU_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_InforbuSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_NVRSTSTTS_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvrestoreRst) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_CPNN_TEST_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_CpnnTestCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_NVBACKUP_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvbackupSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_CPNN_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_CpnnQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_NVRESTORE_MANU_DEFAULT_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvrestoreRst) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_ADC_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_AdcSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
    { DRV_AGENT_TSELRF_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_TselrfSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { DRV_AGENT_HKADC_GET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_HkadcGetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_TBAT_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_TbatQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON == FEATURE_SECURITY_SHELL)
    { DRV_AGENT_SPWORD_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SpwordSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { DRV_AGENT_NVBACKUPSTAT_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvbackupstatQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_NANDBBC_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NandbbcQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      AT_ChkDrvAgentNandBadBlockQryRspMsgLen },
    { DRV_AGENT_NANDVER_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NandverQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_CHIPTEMP_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_ChiptempQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_HUK_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_HukSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_FACAUTHPUBKEY_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_FacauthpubkeySetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_IDENTIFYSTART_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_IdentifystartSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_IDENTIFYEND_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_IdentifyendSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_SIMLOCKDATAWRITE_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SimlockdatawriteSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_PHONESIMLOCKINFO_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_PhonesimlockinfoQryCnfNew) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_SIMLOCKDATAREAD_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SimlockdatareadQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_PHONEPHYNUM_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_PhonephynumSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_OPWORD_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_OpwordSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if ( FEATURE_ON == FEATURE_LTE )
    { DRV_AGENT_INFORRS_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_InforrsSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { DRV_AGENT_MAX_LOCK_TIMES_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_MaxLockTmsSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_AP_SIMST_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_ApSimstSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_SWVER_SET_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SwverSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_NVMANUFACTUREEXT_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NvmanufactureextSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_QRY_CCPU_MEM_INFO_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_QryCcpuMemInfoCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_SIMLOCKWRITEEX_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SimlockwriteexSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_SIMLOCKDATAREADEX_SET_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_SimlockdatareadexReadCnf) - TAF_DEFAULT_CONTENT_LEN),
      AT_ChkDrvAgentSimlockDataReadExReadCnfMsgLen},
    { DRV_AGENT_PHONESIMLOCKINFO_QRY_NEW_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_PhonesimlockinfoQryCnfNew) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { DRV_AGENT_GETMODEMSCID_QRY_CNF,
      (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_GetSimlockEncryptIdQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { DRV_AGENT_HKADC_QRY_CNF, (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_AdcQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR }
};

static TAF_ChkMsgLenNameMapTbl g_atChkVcMsgLenTbl[] = {
    { APP_VC_MSG_CNF_QRY_MODE, ((sizeof(MN_AT_IndEvt) + sizeof(APP_VC_EventInfo)) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_SET_FOREGROUND_CNF, sizeof(MN_AT_IndEvt), VOS_NULL_PTR },
    { APP_VC_MSG_SET_BACKGROUND_CNF, sizeof(MN_AT_IndEvt), VOS_NULL_PTR },
    { APP_VC_MSG_FOREGROUND_RSP, sizeof(MN_AT_IndEvt), VOS_NULL_PTR },
    { APP_VC_MSG_SET_MSD_CNF, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_SetMsdCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_QRY_MSD_CNF, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_MsgQryMsdCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_QRY_ECALL_CFG_CNF, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_MsgQryEcallCfgCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_DTMF_DECODER_IND, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_DtmfDecoderInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_SET_TTYMODE_CNF, sizeof(MN_AT_IndEvt), VOS_NULL_PTR },
    { APP_VC_MSG_QRY_TTYMODE_CNF, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_QryTtymodeCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_SET_ECALL_PUSH_CNF, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_SetMsdCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { APP_VC_MSG_I2S_TEST_CNF, sizeof(MN_AT_IndEvt), VOS_NULL_PTR },
    { APP_VC_MSG_I2S_TEST_RSLT_IND, (sizeof(MN_AT_IndEvt) + sizeof(APP_VC_I2sTestRsltInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
};

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
static TAF_ChkMsgLenNameMapTbl g_atChkDspWphyMsgLenTbl[] = {
    { ID_HPA_AT_RF_CFG_CNF, sizeof(HPA_AT_RfCfgCnf), VOS_NULL_PTR },
    { ID_HPA_AT_RF_RX_RSSI_IND, sizeof(HPA_AT_RfRxRssiInd), VOS_NULL_PTR },
    { ID_WPHY_AT_TX_CLT_IND, sizeof(WPHY_AT_TxCltInd), VOS_NULL_PTR },
    { ID_AT_WPHY_RF_PLL_STATUS_CNF, sizeof(PHY_AT_RfPllStatusCnf), VOS_NULL_PTR },
    { ID_AT_PHY_POWER_DET_CNF, sizeof(PHY_AT_PowerDetCnf), VOS_NULL_PTR },
    { ID_HPA_AT_MIPI_WR_CNF, sizeof(HPA_AT_MipiWrCnf), VOS_NULL_PTR },
    { ID_HPA_AT_MIPI_RD_CNF, sizeof(HPA_AT_MipiRdCnf), VOS_NULL_PTR },
    { ID_HPA_AT_PDM_CTRL_CNF, sizeof(HPA_AT_PdmCtrlCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkDspGphyMsgLenTbl[] = {
    { ID_GHPA_AT_RF_MSG_CNF, sizeof(HPA_AT_RfCfgCnf), VOS_NULL_PTR },
    { ID_GHPA_AT_RF_RX_RSSI_IND, sizeof(HPA_AT_RfRxRssiInd), VOS_NULL_PTR },
    { ID_AT_GPHY_RF_PLL_STATUS_CNF, sizeof(PHY_AT_RfPllStatusCnf), VOS_NULL_PTR },
    { ID_HPA_AT_MIPI_WR_CNF, sizeof(HPA_AT_MipiWrCnf), VOS_NULL_PTR },
    { ID_HPA_AT_MIPI_RD_CNF, sizeof(HPA_AT_MipiRdCnf), VOS_NULL_PTR },
    { ID_AT_PHY_POWER_DET_CNF, sizeof(PHY_AT_PowerDetCnf), VOS_NULL_PTR },
};
#endif

TAF_ChkMsgLenNameMapTbl g_atChkCcMsgLenTbl[] = {
    { AT_CC_MSG_STATE_QRY_CNF, sizeof(AT_CC_StateQryCnfMsg), VOS_NULL_PTR },
};

/* 来源g_atL4aCnfMsgFunTable g_atL4aIndMsgFunTable */
static TAF_ChkMsgLenNameMapTbl g_atChkMspL4aMsgLenTbl[] = {
    { ID_MSG_L4A_CSQ_INFO_CNF, sizeof(L4A_CSQ_InfoCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_ANQUERY_INFO_CNF, sizeof(L4A_CSQ_INFO_IND_STRU), VOS_NULL_PTR },
    { ID_MSG_L4A_CELL_ID_CNF, sizeof(L4A_READ_CellIdCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_LWCLASHQRY_CNF, sizeof(L4A_READ_LwclashCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_RADVER_SET_CNF, sizeof(L4A_SET_RadverCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_CELL_INFO_CNF, sizeof(L4A_READ_CellInfoCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_CERSSI_INQ_CNF, sizeof(L4A_CSQ_InfoCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_LCACELLQRY_CNF, sizeof(L4A_READ_LcacellCnf), VOS_NULL_PTR },
    { ID_MSG_L4A_RSSI_IND, sizeof(L4A_CSQ_INFO_IND_STRU), VOS_NULL_PTR },
    { ID_MSG_L4A_ANLEVEL_IND, sizeof(L4A_CSQ_INFO_IND_STRU), VOS_NULL_PTR },
    { ID_MSG_L4A_CERSSI_IND, sizeof(L4A_CSQ_INFO_IND_STRU), VOS_NULL_PTR },
    { ID_MSG_L4A_LWCLASH_IND, sizeof(L4A_READ_LwclashInd), VOS_NULL_PTR },
    { ID_MSG_L4A_LCACELL_IND, sizeof(L4A_READ_LcacellInd), VOS_NULL_PTR },
    { ID_MSG_L4A_ISMCOEXSET_CNF, sizeof(L4A_ISMCOEX_Cnf), VOS_NULL_PTR },
    { ID_MSG_L4A_LCASCELLINFO_QRY_CNF, sizeof(L4A_READ_ScellInfoCnf), VOS_NULL_PTR },
};

/* 来源g_lteAtFtmCnfMsgTbl,g_lteAtFtmIndMsgTbl */
static TAF_ChkMsgLenNameMapTbl g_atChkMspSysFtmMsgLenTbl[] = {
    { ID_MSG_SYM_SET_TMODE_CNF, (sizeof(AT_FW_DataMsg) + sizeof(SYM_SET_TmodeCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_FCHAN_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetFchanCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_RD_FCHAN_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdFchanCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_TXON_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetTxonCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_RD_TXON_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdTxonCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_RXON_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetRxonCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_RD_RXON_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdRxonCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_FWAVE_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetFwaveCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_TSELRF_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetTselrfCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_AAGC_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_SetAagcCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_RD_AAGC_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdAagcCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_FRSSI_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_FRSSI_Cnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_RD_TBAT_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdTbatCnf)), VOS_NULL_PTR },
    { ID_MSG_FTM_SET_LTCOMMCMD_CNF, 0, AT_ChkFtmSetLtCommCmdCnfMsgLen },
    { ID_MSG_FTM_RD_LTCOMMCMD_CNF,  0, AT_ChkFtmRdLtCommCmdCnfMsgLen },
    { ID_MSG_FTM_RD_FPLLSTATUS_CNF, (sizeof(AT_FW_DataMsg) + sizeof(FTM_RdFpllstatusCnf)), VOS_NULL_PTR },
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    { ID_MSG_FTM_TX_CLT_INFO_IND, (sizeof(AT_FW_DataMsg) + sizeof(FTM_CltInfoInd)), VOS_NULL_PTR },
#endif
};

/* WUEPS_PID_AT接收来自MAPS_STK_PID的消息检查 */
static TAF_ChkMsgLenNameMapTbl g_atChkMapsStkMsgLenTbl[] = {
    { STK_AT_DATAPRINT_CNF, sizeof(MN_APP_StkAtDataprint), VOS_NULL_PTR },
    { STK_AT_EVENT_CNF, sizeof(MN_APP_StkAtCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkRabmMsgMsgLenTbl[] = {
    { ID_RABM_AT_SET_FASTDORM_PARA_CNF, sizeof(RABM_AT_SetFastdormParaCnf), VOS_NULL_PTR },
    { ID_RABM_AT_QRY_FASTDORM_PARA_CNF, sizeof(RABM_AT_QryFastdormParaCnf), VOS_NULL_PTR },
    { ID_RABM_AT_SET_RELEASE_RRC_CNF, sizeof(RABM_AT_ReleaseRrcCnf), VOS_NULL_PTR },
#if (FEATURE_HUAWEI_VP == FEATURE_ON)
    { ID_RABM_AT_SET_VOICEPREFER_PARA_CNF, sizeof(RABM_AT_SetVoicepreferParaCnf), VOS_NULL_PTR },
    { ID_RABM_AT_QRY_VOICEPREFER_PARA_CNF, sizeof(RABM_AT_QryVoicepreferParaCnf), VOS_NULL_PTR },
    { ID_RABM_AT_VOICEPREFER_STATUS_REPORT, sizeof(RABM_AT_VoicepreferStatusReport), VOS_NULL_PTR },
#endif
};

static TAF_ChkMsgLenNameMapTbl g_atChkSpyMsgLenTbl[] = {
#if (FEATURE_LTE == FEATURE_ON)
    { ID_TEMPPRT_AT_EVENT_IND, sizeof(TEMP_PROTECT_EventAtInd), VOS_NULL_PTR },
#endif
    { ID_TEMPPRT_STATUS_AT_EVENT_IND, sizeof(TEMP_PROTECT_EventAtInd), VOS_NULL_PTR },
};

/* AT接收MTA消息 */
static TAF_ChkMsgLenNameMapTbl g_atChkMtaMsgLenTbl[] = {
    { ID_MTA_AT_CPOS_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CposCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_CPOSR_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CposrInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_XCPOSRRPT_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_XcposrrptInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CGPSCLOCK_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CgpsclockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SIMLOCKUNLOCK_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SimlockunlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_NMR_CNF, 0, AT_ChkMtaQryNmrMsgLen },
    { ID_MTA_AT_WRR_AUTOTEST_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrAutotestQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_CELLINFO_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrCellinfoQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_MEANRPT_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrMeanrptQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_FREQLOCK_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrFreqlockSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_RRC_VERSION_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrRrcVersionSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_WRR_CELLSRH_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrCellsrhSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_FREQLOCK_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrFreqlockQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_WRR_RRC_VERSION_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrRrcVersionQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_WRR_CELLSRH_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_WrrCellsrhQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_GAS_AUTOTEST_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_GasAutotestQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_BODY_SAR_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CURC_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CurcQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_UNSOLICITED_RPT_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UnsolicitedRptSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_UNSOLICITED_RPT_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UnsolicitedRptQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_IMEI_VERIFY_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(VOS_UINT32) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CGSN_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CgsnQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NCELL_MONITOR_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NCELL_MONITOR_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NcellMonitorQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NCELL_MONITOR_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NcellMonitorInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_REFCLKFREQ_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_REFCLKFREQ_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RefclkfreqQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_REFCLKFREQ_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RefclkfreqInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
    { ID_MTA_AT_NW_SLICE_AUTH_CMD_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NwSliceAuthCmdInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NW_SLICE_AUTH_RSLT_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NwSliceAuthRsltInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
#endif
    { ID_MTA_AT_HANDLEDECT_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_HandledectSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_HANDLEDECT_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_HandledectQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PS_TRANSFER_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PsTransferInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON == FEATURE_DSDS)
    { ID_MTA_AT_RRC_PROTECT_PS_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RrcProtectPsCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_ECID_SET_CNF, 0, AT_ChkMtaEcidSetCnfMsgLen },
    { ID_MTA_AT_MIPICLK_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RfLcdMipiclkCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MIPICLK_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RfLcdMipiclkInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_DPDTTEST_FLAG_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetDpdttestFlagCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_SET_DPDT_VALUE_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetDpdtValueCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_DPDT_VALUE_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryDpdtValueCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    { ID_MTA_AT_SET_TRX_TAS_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetTrxTasCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_TRX_TAS_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryTrxTasCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_SET_JAM_DETECT_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetJamDetectCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_JAM_DETECT_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryJamDetectCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_JAM_DETECT_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_JamDetectInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },

    { ID_MTA_AT_SET_GSM_FREQLOCK_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetGsmFreqlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_GSM_FREQLOCK_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryGsmFreqlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON == FEATURE_PROBE_FREQLOCK)
    { ID_MTA_AT_SET_M2M_FREQLOCK_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetM2MFreqlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_M2M_FREQLOCK_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryM2MFreqlockCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_XPASS_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_XpassInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },

#if(FEATURE_ON== FEATURE_LTE)
#if(FEATURE_ON== FEATURE_LTE_MBMS)
    { ID_MTA_AT_MBMS_SERVICE_OPTION_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MBMS_SERVICE_STATE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MBMS_PREFERENCE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SIB16_NETWORK_TIME_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MbmsSib16NetworkTimeQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_BSSI_SIGNAL_LEVEL_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MbmsBssiSignalLevelQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NETWORK_INFO_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MbmsNetworkInfoQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_EMBMS_STATUS_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EmbmsStatusQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MBMS_UNSOLICITED_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MBMS_SERVICE_EVENT_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MbmsServiceEventInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_INTEREST_LIST_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MbmsAvlServiceListQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
    { ID_MTA_AT_LTE_LOW_POWER_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_WIFI_COEX_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_WIFI_COEX_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LteWifiCoexQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CA_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CA_CELLEX_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CaCellInfoCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CA_CELLEX_INFO_NTF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CaCellInfoNtf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CA_CELL_RPT_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CaCellSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CA_CELL_RPT_CFG_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CaCellQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_FINE_TIME_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_FineTimeSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SIB_FINE_TIME_NTF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SibFineTimeInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LPP_FINE_TIME_NTF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LppFineTimeInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LL2_COM_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_L2ComCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LL2_COM_CFG_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_L2ComCfgQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LRRC_UE_CAP_PARA_INFO_NTF, 0, AT_ChkMtaLrrcUeCapParaInfoNtfMsgLen },
    { ID_MTA_AT_LRRC_UE_CAP_PARA_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UeCapParamSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_OVERHEATING_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_OverHeatingSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_OVERHEATING_CFG_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_OverHeatingQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
#if (FEATURE_ON== FEATURE_UE_MODE_NR)
    { ID_MTA_AT_NL2_COM_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_L2ComCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NL2_COM_CFG_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_L2ComCfgQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NPDCP_SLEEP_TRHRES_CFG_CNF,
        (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NPdcpSleepThresCfgCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NRPWRCTRL_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrpwrctrlSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NRPHY_COM_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrphyComCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NRRC_UE_CAP_PARA_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UeCapParamSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NRRC_UE_CAP_PARA_INFO_NTF, 0, AT_ChkMtaNrrcUeCapParaInfoNtfMsgLen },
#endif

    { ID_MTA_AT_SET_FR_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetFrCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_SIB16_TIME_UPDATE_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_Sib16TimeUpdate) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON== FEATURE_UE_MODE_NR)
    { ID_MTA_AT_LENDC_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LendcQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LENDC_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LendcInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NRRCCAP_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrrccapCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_AUTON2LOPT_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_AutoN2lOptCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NRRCCAP_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrrccapQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NRPWRCTRL_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrpwrctrlSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NRPOWERSAVING_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrPowerSavingCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NRPOWERSAVING_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrPowerSavingQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_MTA_AT_NR_FREQLOCK_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NRRC_FREQLOCK_SET_CNF_STRU) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NR_FREQLOCK_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrrcFreqlockQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_NR_CA_CELL_INFO_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrCaCellInfoCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NR_CA_CELL_INFO_RPT_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NR_CA_CELL_INFO_RPT_CFG_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrCaCellInfoRptCfgQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NR_CA_CELL_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrCaCellInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NR_NW_CAP_INFO_REPORT_IND,
      (sizeof(AT_MTA_Msg) - TAF_DEFAULT_CONTENT_LEN + sizeof(MTA_AT_NrNwCapInfoReportInd)), VOS_NULL_PTR },
    { ID_MTA_AT_NR_NW_CAP_INFO_RPT_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) - TAF_DEFAULT_CONTENT_LEN + sizeof(MTA_AT_ResultCnf)), VOS_NULL_PTR },
    { ID_MTA_AT_NR_NW_CAP_INFO_RPT_CFG_QRY_CNF,
      (sizeof(AT_MTA_Msg) - TAF_DEFAULT_CONTENT_LEN + sizeof(MTA_AT_NrNwCapInfoRptCfgQryCnf)), VOS_NULL_PTR },
    { ID_MTA_AT_NR_NW_CAP_INFO_QRY_CNF,
      (sizeof(AT_MTA_Msg) - TAF_DEFAULT_CONTENT_LEN + sizeof(MTA_AT_NrNwCapInfoQryCnf)), VOS_NULL_PTR },
#endif
#if (FEATURE_ON== FEATURE_UE_MODE_CDMA)
    { ID_MTA_AT_MEID_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_MEID_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MeidQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_EVDO_SYS_EVENT_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_EVDO_SIG_MASK_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_EVDO_REVA_RLINK_INFO_IND, 0, AT_ChkMtaEvdoRevARLinkMsgLen },
    { ID_MTA_AT_EVDO_SIG_EXEVENT_IND, 0, AT_ChkMtaEvdoSigExEventMsgLen },
    { ID_MTA_AT_NO_CARD_MODE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NO_CARD_MODE_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NoCardModeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_TRANSMODE_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TransmodeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_UE_CENTER_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetUeCenterCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_UE_CENTER_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryUeCenterCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_NETMON_SCELL_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NetmonCellInfo) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_NETMON_NCELL_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NetmonCellInfo) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_MTA_AT_SET_NR_CELL_BAND_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NetmonCellInfo) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_NETMON_SSCELL_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NetmonCellInfo) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_NR_SSB_ID_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrSsbIdQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryAfcClkFreqXocoefCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_ANQUERY_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_AntennaInfoQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CSNR_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CsnrQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CSQLVL_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CsqlvlQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_XCPOSR_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_XCPOSR_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryXcposrCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_XCPOSRRPT_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_XCPOSRRPT_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryXcposrrptCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CLEAR_HISTORY_FREQ_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_SENSOR_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetSensorCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_SCREEN_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetScreenCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_FRAT_IGNITION_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_FratIgnitionQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_FRAT_IGNITION_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_FratIgnitionSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_SET_MODEM_TIME_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetTimeCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_RX_TEST_MODE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetRxtestmodeCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_AFC_CLK_UNLOCK_CAUSE_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_AfcClkUnlockInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_BESTFREQ_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ResultCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_BEST_FREQ_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_BestFreqCaseInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_BESTFREQ_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_BestfreqQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_RFIC_DIE_ID_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RficDieIdReqCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_RFFE_DIE_ID_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RffeDieIdReqCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PHY_COM_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PhyComCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CRRCONN_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetCrrconnCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CRRCONN_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryCrrconnCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CRRCONN_STATUS_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CrrconnStatusInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_VTRLQUALRPT_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetVtrlqualrptCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_RL_QUALITY_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RlQualityInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_VIDEO_DIAG_INFO_RPT, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_VideoDiagInfoRpt) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PMU_DIE_SN_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PmuDieSnReqCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_MODEM_CAP_UPDATE_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ModemCapUpdateCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_TAS_TEST_CFG_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TasTestCfgCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_TAS_TEST_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TasTestQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LTE_CATEGORY_INFO_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LteCategoryInfoInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_ACCESS_STRATUM_REL_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_AccessStratumRelInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_RS_INFO_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_RsInfoQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_ERRCCAP_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ErrccapCfgCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_ERRCCAP_QRY_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ErrccapQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PSEUCELL_INFO_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PseucellInfoSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#if(FEATURE_OFF== FEATURE_UE_MODE_NR)
    { ID_MTA_AT_POWER_DET_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PowerDetQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_MIPIREAD_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_MipiReadCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PHYMIPIWRITE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PhyMipiWriteCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CHRALARMRLAT_CFG_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_ChrAlarmRlatCfgSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_ECC_CFG_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SET_ECC_CFG_CNF_STRU) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_ECC_STATUS_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EccStatusInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_EPDUR_DATA_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EpduDataInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_EPDU_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EpduSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON == FEATURE_DCXO_HI1102_SAMPLE_SHARE)
    { ID_MTA_AT_DCXO_SAMPLE_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_DcxoSampeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_PHY_COMM_ACK_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PhyCommAckInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_COMM_BOOSTER_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CommBoosterSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_COMM_BOOSTER_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CommBoosterInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_COMM_BOOSTER_QUERY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CommBoosterQueryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_NVLOAD_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetNvloadCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SMS_DOMAIN_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SmsDomainSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SMS_DOMAIN_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SmsDomainQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SET_SAMPLE_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetSampleCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_GPS_LOCSET_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_GpslocsetSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_CCLK_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CclkQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_TEMP_PROTECT_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TempProtectInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_AT_MTA_GAME_MODE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_GameModeSetCfn) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON== FEATURE_UE_MODE_CDMA)
    { ID_MTA_AT_CDMA_MODEM_CAP_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CdmaModemCapSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_CDMA_CAP_RESUME_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_CdmaCapResumeSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
    { ID_MTA_AT_NV_REFRESH_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NvRefreshSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_PSEUDBTS_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PseudBtsSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SUBCLFSPARAM_SET_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SubclfsparamSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_SUBCLFSPARAM_QRY_CNF,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SubclfsparamQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#if(defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_AT_MTA_SET_LTESARSTUB_CNF, (sizeof(AT_MTA_Msg) + sizeof(AT_MTA_LteSarStubSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_PSEUD_BTS_IDENT_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PseudBtsIdentInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_FORCESYNC_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetForcesyncCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_AT_MTA_SET_LTEPWRDISS_CNF, (sizeof(AT_MTA_Msg) + sizeof(AT_MTA_LtepwrdissSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_SFEATURE_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SfeatureQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_MTA_AT_LOW_PWR_MODE_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LowPwrModeCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_TXPOWER_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TxpowerQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_TDD_SUBFRAME_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_TddSubframeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON== FEATURE_UE_MODE_NR)
    { ID_MTA_AT_NTXPOWER_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NtxpowerQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_5G_NSSAI_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_5GNssaiSetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_5G_NSSAI_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_5GNssaiQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_LADN == FEATURE_ON)
    { ID_MTA_AT_SET_LADN_RPT_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetLadnRptCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_QRY_LADN_INFO_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryLadnInfoCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_LADN_INFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LadnInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
    { ID_MTA_AT_PENDING_NSSAI_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_PendingNssaiInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_UART_TEST_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UartTestCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_UART_TEST_RSLT_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_UartTestRsltInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif

    { ID_MTA_AT_MCS_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_McsQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { ID_MTA_AT_HFREQINFO_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_HfreqinfoQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_HFREQINFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_HfreqinfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#if (FEATURE_ON== FEATURE_UE_MODE_NR)
    { ID_MTA_AT_5G_OPTION_SET_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_Set5GOptionCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_5G_OPTION_QRY_CNF, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_Qry5GOptionCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { ID_MTA_AT_FAST_RETURN_5G_ENDC_IND,
      (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_FastReturn5gEndcInd) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
#endif
#if (FEATURE_LTEV == FEATURE_ON)
    { ID_TAF_MTA_VMODE_SET_CNF, sizeof(TAF_MTA_VMODE_SetCnf), VOS_NULL_PTR },
    { ID_TAF_MTA_VMODE_QRY_CNF, sizeof(TAF_MTA_VMODE_QryCnf), VOS_NULL_PTR },
#endif
#if ((FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON))
    { ID_MTA_AT_HSRCELLINFO_IND, (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_HsrcellInfoInd) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
#endif
    { ID_MTA_AT_NV_REFRESH_NTF, 0, VOS_NULL_PTR },
    { ID_MTA_AT_QUICK_CARD_SWITCH_CNF,
     (sizeof(AT_MTA_Msg) + sizeof(MTA_AT_SetQuickCardSwitchCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkImsaMsgLenTbl[] = {
    { ID_IMSA_AT_CIREG_SET_CNF, sizeof(IMSA_AT_CiregSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREG_QRY_CNF, sizeof(IMSA_AT_CiregQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREP_SET_CNF, sizeof(IMSA_AT_CirepSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREP_QRY_CNF, sizeof(IMSA_AT_CirepQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_VOLTEIMPU_QRY_CNF, sizeof(IMSA_AT_VolteimpuQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREGU_IND, sizeof(IMSA_AT_CireguInd), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREPH_IND, sizeof(IMSA_AT_CirephInd), VOS_NULL_PTR },
    { ID_IMSA_AT_CIREPI_IND, sizeof(IMSA_AT_CirepiInd), VOS_NULL_PTR },
    { ID_IMSA_AT_MT_STATES_IND, sizeof(IMSA_AT_MtStatesInd), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_CTRL_MSG, sizeof(IMSA_AT_ImsCtrlMsg), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_REG_DOMAIN_QRY_CNF, sizeof(IMSA_AT_ImsRegDomainQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CALL_ENCRYPT_SET_CNF, sizeof(IMSA_AT_CallEncryptSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_ROAMING_IMS_QRY_CNF, sizeof(IMSA_AT_RoamingImsQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_RAT_HANDOVER_IND, sizeof(IMSA_AT_ImsRatHandoverInd), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_SRV_STATUS_UPDATE_IND, sizeof(IMSA_AT_ImsSrvStatusUpdateInd), VOS_NULL_PTR },
    { ID_IMSA_AT_PCSCF_SET_CNF, sizeof(IMSA_AT_PcscfSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_PCSCF_QRY_CNF, sizeof(IMSA_AT_PcscfQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_DMDYN_SET_CNF, sizeof(IMSA_AT_DmdynSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_DMDYN_QRY_CNF, sizeof(IMSA_AT_DmdynQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_DMCN_IND, sizeof(IMSA_AT_DmcnInd), VOS_NULL_PTR },
    { ID_IMSA_AT_IMSTIMER_QRY_CNF, sizeof(IMSA_AT_ImstimerQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMSTIMER_SET_CNF, sizeof(IMSA_AT_ImstimerSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_SMSPSI_SET_CNF, sizeof(IMSA_AT_SmspsiSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_SMSPSI_QRY_CNF, sizeof(IMSA_AT_SmspsiQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_DMUSER_QRY_CNF, sizeof(IMSA_AT_DmuserQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_NICKNAME_SET_CNF, sizeof(IMSA_AT_NicknameSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_NICKNAME_QRY_CNF, sizeof(IMSA_AT_NicknameQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_REGFAIL_IND, sizeof(IMSA_AT_RegFailInd), VOS_NULL_PTR },
    { ID_IMSA_AT_BATTERYINFO_SET_CNF, sizeof(IMSA_AT_BatteryInfoSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_VOLTEIMPI_QRY_CNF, sizeof(IMSA_AT_VolteimpiQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_VOLTEDOMAIN_QRY_CNF, sizeof(IMSA_AT_VoltedomainQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_REGERR_REPORT_SET_CNF, sizeof(IMSA_AT_RegerrReportSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_REGERR_REPORT_QRY_CNF, sizeof(IMSA_AT_RegerrReportQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_REGERR_REPORT_IND, sizeof(IMSA_AT_RegerrReportInd), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_IP_CAP_SET_CNF, sizeof(IMSA_AT_ImsIpCapSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_IP_CAP_QRY_CNF, sizeof(IMSA_AT_ImsIpCapQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_EMC_PDN_ACTIVATE_IND, sizeof(IMSA_AT_EmcPdnActivateInd), VOS_NULL_PTR },
    { ID_IMSA_AT_EMC_PDN_DEACTIVATE_IND, sizeof(IMSA_AT_EmcPdnDeactivateInd), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_SRV_STAT_RPT_SET_CNF, sizeof(IMSA_AT_ImsSrvStatReportSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_SRV_STAT_RPT_QRY_CNF, sizeof(IMSA_AT_ImsSrvStatReportQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMS_SERVICE_STATUS_QRY_CNF, sizeof(IMSA_AT_ImsServiceStatusQryCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_EMERGENCY_AID_SET_CNF, sizeof(IMSA_AT_EmergencyAidSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_CALL_ALT_SRV_IND, sizeof(IMSA_AT_CallAltSrvInd), VOS_NULL_PTR },
    { ID_IMSA_AT_DM_RCS_CFG_SET_CNF, sizeof(IMSA_AT_DmRcsCfgSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_USER_AGENT_CFG_SET_CNF, sizeof(IMSA_AT_UserAgentCfgSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_IMPU_TYPE_IND, sizeof(IMSA_AT_ImpuTypeInd), VOS_NULL_PTR },
    { ID_IMSA_AT_RTTCFG_SET_CNF, sizeof(IMSA_AT_RttcfgSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_RTT_MODIFY_SET_CNF, sizeof(IMSA_AT_RttModifySetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_RTT_EVENT_IND, sizeof(IMSA_AT_RttEventInd), VOS_NULL_PTR },
    { ID_IMSA_AT_RTT_ERROR_IND, sizeof(IMSA_AT_RttErrorInd), VOS_NULL_PTR },
    { ID_IMSA_AT_TRANSPORT_TYPE_SET_CNF, sizeof(IMSA_AT_TransportTypeSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_TRANSPORT_TYPE_QRY_CNF, sizeof(IMSA_AT_TransportTypeQryCnf), VOS_NULL_PTR },
#if (FEATURE_ECALL == FEATURE_ON)
    { ID_IMSA_AT_ECALL_ECONTENT_TYPE_SET_CNF, sizeof(IMSA_AT_EcallContentTypeSetCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_ECALL_ECONTENT_TYPE_QRY_CNF, sizeof(IMSA_AT_EcallContentTypeQryCnf), VOS_NULL_PTR },
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_IMSA_AT_IMS_URSP_SET_CNF, sizeof(IMSA_AT_ImsUrspSetCnf), VOS_NULL_PTR },
#endif
    { ID_IMSA_AT_FUSIONCALL_CTRL_MSG, 0, AT_ChkImsaFusioncallCtrlMsgLen },
    { ID_IMSA_AT_EMC_PDN_ACTIVATE_CNF, sizeof(IMSA_AT_EmcPdnActivateCnf), VOS_NULL_PTR },
    { ID_IMSA_AT_EMC_PDN_DEACTIVATE_CNF, sizeof(IMSA_AT_EmcPdnDeactivateCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkCssMsgLenTbl[] = {
    { ID_CSS_AT_MCC_INFO_SET_CNF, sizeof(CSS_AT_MccInfoSetCnf), VOS_NULL_PTR },
    { ID_CSS_AT_MCC_VERSION_INFO_CNF, sizeof(CSS_AT_MccVersionInfoCnf), VOS_NULL_PTR },
    { ID_CSS_AT_QUERY_MCC_INFO_NOTIFY, sizeof(CSS_AT_QueryMccInfoNotify), VOS_NULL_PTR },
    { ID_CSS_AT_BLOCK_CELL_LIST_SET_CNF, sizeof(CSS_AT_BlockCellListSetCnf), VOS_NULL_PTR },
    { ID_CSS_AT_BLOCK_CELL_LIST_QUERY_CNF, sizeof(CSS_AT_BlockCellListQueryCnf), VOS_NULL_PTR },
    { ID_CSS_AT_BLOCK_CELL_MCC_NOTIFY, sizeof(CSS_AT_BlockCellMccNotify), VOS_NULL_PTR },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_CSS_AT_PLMN_DEPLOYMENT_INFO_SET_CNF, sizeof(CSS_AT_PlmnDeploymentInfoSetCnf), VOS_NULL_PTR },
#endif
    { ID_CSS_AT_CLOUD_DATA_SET_CNF, sizeof(CSS_AT_CloudDataSetCnf), VOS_NULL_PTR},
    { ID_CSS_AT_CLOUD_DATA_REPORT, 0, AT_ChkCssCloudDataReportMsgLen },
};

#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
static TAF_ChkMsgLenNameMapTbl g_atChkThrotMsgLenTbl[] = {
#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    { ID_THROT_AT_CABTSRI_CONFIGURATION_SET_CNF, sizeof(THROT_AT_CabtsriConfigSetCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CABTSRI_CONFIGURATION_QUERY_CNF, sizeof(THROT_AT_CabtsriQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CABTRDP_QUERY_CNF, sizeof(THROT_AT_CabtrdpQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_SINGLE_APN_BACKOFF_TIMER_PARA_NTF, 0, AT_ChkThrotSingleApnBackoffTimeParaNtfMsgLen },
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    { ID_THROT_AT_CSBTSRI_CONFIGURATION_SET_CNF, sizeof(THROT_AT_CsbtsriConfigSetCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CSBTSRI_CONFIGURATION_QUERY_CNF, sizeof(THROT_AT_CsbtsriQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CSBTRDP_QUERY_CNF, sizeof(THROT_AT_CsbtrdpQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_SINGLE_SNSSAI_BACKOFF_TIMER_PARA_NTF, 0, AT_ChkThrotSingleSnssaiBackoffParaNtfMsgLen },
    { ID_THROT_AT_CSDBTSRI_CONFIGURATION_SET_CNF, sizeof(THROT_AT_CsdbtsriConfigSetCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CSDBTSRI_CONFIGURATION_QUERY_CNF, sizeof(THROT_AT_CsdbtsriQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_CSDBTRDP_QUERY_CNF, sizeof(THROT_AT_CsdbtrdpQueryCnf), VOS_NULL_PTR },
    { ID_THROT_AT_SINGLE_SNSSAI_DNN_BACKOFF_TIMER_PARA_NTF, 0, AT_ChkThrotSingleSnssaiDnnBackoffParaNtfMsgLen },
#endif
};
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
static TAF_ChkMsgLenNameMapTbl g_atChkCbtMsgLenTbl[] = {
    { ID_CCBT_AT_SET_WORK_MODE_CNF, sizeof(AT_CCBT_LOAD_PHY_CNF_STRU), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkBbicMsgLenTbl[] = {
    { ID_BBIC_CAL_AT_MT_TX_CNF, sizeof(BBIC_CAL_AT_MT_TRX_CNF), VOS_NULL_PTR },
    { ID_BBIC_CAL_AT_MT_RX_CNF, sizeof(BBIC_CAL_AT_MT_TRX_CNF), VOS_NULL_PTR },
    { ID_BBIC_AT_CAL_MSG_CNF, sizeof(BBIC_CAL_MSG_CNF_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_GSM_TX_PD_CNF, sizeof(BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_CAL_AT_MT_TX_PD_IND, sizeof(BBIC_CAL_AT_MT_TX_PD_IND), VOS_NULL_PTR },
    { ID_BBIC_AT_MIPI_READ_CNF, sizeof(BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_MIPI_WRITE_CNF, sizeof(BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_PLL_QRY_CNF, sizeof(BBIC_CAL_PLL_QRY_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_CAL_AT_MT_RX_RSSI_IND, sizeof(BBIC_CAL_AT_MT_RX_RSSI_IND), VOS_NULL_PTR },
    { ID_BBIC_AT_DPDT_CNF, sizeof(BBIC_CAL_DPDT_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_TEMP_QRY_CNF, sizeof(BBIC_CAL_TEMP_QRY_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_DCXO_CNF, sizeof(BBIC_CAL_DCXO_IND_STRU), VOS_NULL_PTR },
    { ID_BBIC_AT_TRX_TAS_CNF, sizeof(BBIC_CAL_SetTrxTasCnfStru), VOS_NULL_PTR }
};

static TAF_ChkMsgLenNameMapTbl g_atChkPamMfgMsgLenTbl[] = {
    { ID_UECBT_AT_RFIC_MEM_TEST_CNF, sizeof(UECBT_AT_RficMemTestCnf), VOS_NULL_PTR },
    { ID_UECBT_AT_RFIC_DIE_IE_QUERY_CNF, 0, AT_ChkUeCbtRfIcIdExQryCnfMsgLen },
};

static TAF_ChkMsgLenNameMapTbl g_atChkDspIdleMsgLenTbl[] = {
    { ID_FESS_FLPM_NAS_SERDES_AGING_CNF, sizeof(FESS_FLPM_NasAtCnf), VOS_NULL_PTR },
    { ID_FESS_FLPM_NAS_AT_NOBIZ_CNF, sizeof(FESS_FLPM_NasAtCnf), VOS_NULL_PTR },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_FESS_FLPM_NAS_SERDES_SLT_CNF, sizeof(FESS_FLPM_NasAtCnf), VOS_NULL_PTR },
#endif
};
#endif

static TAF_ChkMsgLenNameMapTbl g_atChkCcmMsgLenTbl[] = {
    { ID_TAF_CCM_QRY_CHANNEL_INFO_CNF, sizeof(TAF_CCM_QryChannelInfoCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_CHANNEL_INFO_IND, sizeof(TAF_CCM_CallChannelInfoInd), VOS_NULL_PTR },
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { ID_TAF_CCM_SEND_FLASH_CNF, sizeof(TAF_CCM_SendFlashCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_SEND_BURST_DTMF_CNF, sizeof(TAF_CCM_SendBurstDtmfCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_BURST_DTMF_IND, sizeof(TAF_CCM_BurstDtmfInd), VOS_NULL_PTR },
    { ID_TAF_CCM_SEND_CONT_DTMF_CNF, sizeof(TAF_CCM_SendContDtmfCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CONT_DTMF_IND, sizeof(TAF_CCM_ContDtmfInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CCWAC_INFO_IND, sizeof(TAF_CCM_CcwacInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALLED_NUM_INFO_IND, sizeof(TAF_CCM_CalledNumInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALLING_NUM_INFO_IND, sizeof(TAF_CCM_CallingNumInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_DISPLAY_INFO_IND, sizeof(TAF_CCM_DisplayInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_EXT_DISPLAY_INFO_IND, sizeof(TAF_CCM_ExtDisplayInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CONN_NUM_INFO_IND, sizeof(TAF_CCM_ConnNumInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_REDIR_NUM_INFO_IND, sizeof(TAF_CCM_RedirNumInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_SIGNAL_INFO_IND, sizeof(TAF_CCM_SignalInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_LINE_CTRL_INFO_IND, sizeof(TAF_CCM_LineCtrlInfoInd), VOS_NULL_PTR },
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
    { ID_TAF_CCM_ENCRYPT_VOICE_CNF, sizeof(TAF_CCM_EncryptVoiceCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ENCRYPT_VOICE_IND, sizeof(TAF_CCM_EncryptVoiceInd), VOS_NULL_PTR },
    { ID_TAF_CCM_EC_REMOTE_CTRL_IND, sizeof(TAF_CCM_EcRemoteCtrlInd), VOS_NULL_PTR },
    { ID_TAF_CCM_REMOTE_CTRL_ANSWER_CNF, sizeof(TAF_CCM_RemoteCtrlAnswerCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ECC_SRV_CAP_CFG_CNF, sizeof(TAF_CCM_EccSrvCapCfgCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ECC_SRV_CAP_QRY_CNF, sizeof(TAF_CCM_EccSrvCapQryCnf), VOS_NULL_PTR },
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
    { ID_TAF_CCM_SET_EC_TEST_MODE_CNF, sizeof(TAF_CCM_SetEcTestModeCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_GET_EC_TEST_MODE_CNF, sizeof(TAF_CCM_GetEcTestModeCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_GET_EC_RANDOM_CNF, sizeof(TAF_CCM_GetEcRandomCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_GET_EC_KMC_CNF, sizeof(TAF_CCM_GetEcKmcCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_SET_EC_KMC_CNF, sizeof(TAF_CCM_SetEcKmcCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ENCRYPTED_VOICE_DATA_IND, sizeof(TAF_CCM_EncryptedVoiceDataInd), VOS_NULL_PTR },
#endif
#endif
    { ID_TAF_CCM_PRIVACY_MODE_SET_CNF, sizeof(TAF_CCM_PrivacyModeSetCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_PRIVACY_MODE_QRY_CNF, sizeof(TAF_CCM_PrivacyModeQryCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_PRIVACY_MODE_IND, sizeof(TAF_CCM_PrivacyModeInd), VOS_NULL_PTR },
#endif
#if (FEATURE_IMS == FEATURE_ON)
    { ID_TAF_CCM_CALL_MODIFY_CNF, sizeof(TAF_CCM_CallAnswerRemoteModifyCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF, sizeof(TAF_CCM_CallAnswerRemoteModifyCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF, sizeof(TAF_CCM_QryEconfCalledInfoCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_MODIFY_STATUS_IND, sizeof(TAF_CCM_CallModifyStatusInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CCWAI_SET_CNF, sizeof(TAF_CCM_CcwaiSetCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CANCEL_ADD_VIDEO_CNF, sizeof(TAF_CCM_CancelAddVideoCnf), VOS_NULL_PTR },
#endif
    { ID_TAF_CCM_QRY_XLEMA_CNF, sizeof(TAF_CCM_QryXlemaCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_QRY_CALL_INFO_CNF, sizeof(TAF_CCM_QryCallInfoCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_SET_ALS_CNF, sizeof(TAF_CCM_SetAlsCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_SET_UUS1_INFO_CNF, sizeof(TAF_CCM_SetUus1InfoCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_QRY_UUS1_INFO_CNF, sizeof(TAF_CCM_QryUus1InfoCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_QRY_ALS_CNF, sizeof(TAF_CCM_QryAlsCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ECC_NUM_IND, sizeof(TAF_CCM_EccNumInd), VOS_NULL_PTR },
    { ID_TAF_CCM_QRY_CLPR_CNF, sizeof(TAF_CCM_QryClprCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_SET_CSSN_CNF, sizeof(TAF_CCM_SetCssnCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_ORIG_IND, sizeof(TAF_CCM_CallOrigInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_ORIG_CNF, sizeof(TAF_CCM_CallOrigCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_SUPS_CMD_CNF, sizeof(TAF_CCM_CallSupsCmdCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_SUPS_CMD_RSLT_IND, sizeof(TAF_CCM_CallSupsCmdRslt), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_CONNECT_IND, sizeof(TAF_CCM_CallConnectInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_INCOMING_IND, sizeof(TAF_CCM_CallIncomingInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_RELEASED_IND, sizeof(TAF_CCM_CallReleasedInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_ALL_RELEASED_IND, sizeof(TAF_CCM_CallAllReleasedInd), VOS_NULL_PTR },
    { ID_TAF_CCM_START_DTMF_CNF, sizeof(TAF_CCM_StartDtmfCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_STOP_DTMF_CNF, sizeof(TAF_CCM_StopDtmfCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_PROC_IND, sizeof(TAF_CCM_CallProcInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_ALERTING_IND, sizeof(TAF_CCM_CallAlertingInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_HOLD_IND, sizeof(TAF_CCM_CallHoldInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_RETRIEVE_IND, sizeof(TAF_CCM_CallRetrieveInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CALL_SS_IND, sizeof(TAF_CCM_CallSsInd), VOS_NULL_PTR },
    { ID_TAF_CCM_GET_CDUR_CNF, sizeof(TAF_CCM_GetCdurCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_UUS1_INFO_IND, sizeof(TAF_CCM_Uus1InfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CNAP_INFO_IND, sizeof(TAF_CCM_CnapInfoInd), VOS_NULL_PTR },
    { ID_TAF_CCM_CNAP_QRY_CNF, sizeof(TAF_CCM_CnapQryCnf), VOS_NULL_PTR },
#if (FEATURE_IMS == FEATURE_ON)
    { ID_TAF_CCM_ECONF_DIAL_CNF, sizeof(TAF_CCM_EconfDialCnf), VOS_NULL_PTR },
    { ID_TAF_CCM_ECONF_NOTIFY_IND, sizeof(TAF_CCM_EconfNotifyInd), VOS_NULL_PTR },
#endif
    { ID_TAF_CCM_STOP_DTMF_RSLT, sizeof(TAF_CCM_StopDtmfRslt), VOS_NULL_PTR },
    { ID_TAF_CCM_START_DTMF_RSLT, sizeof(TAF_CCM_StartDtmfRslt), VOS_NULL_PTR },
    { ID_TAF_CCM_CUSTOM_ECC_NUM_CNF, sizeof(TAF_CCM_CustomEccNumCnf), VOS_NULL_PTR },
};

#if (FEATURE_LTEV == FEATURE_ON)

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
static TAF_ChkMsgLenNameMapTbl g_atChkVnasMsgLenTbl[] = {
    { ID_VNAS_AT_LTEV_SOURCE_ID_SET_CNF, sizeof(VNAS_ReqMsg), VOS_NULL_PTR },
    { ID_VNAS_AT_LTEV_SOURCE_ID_QRY_CNF,
      (sizeof(VNAS_ReqMsg) + sizeof(VNAS_LtevSourceIdQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
};
#endif

static TAF_ChkMsgLenNameMapTbl g_atChkVrrcMsgLenTbl[] = {
    { VRRC_AT_GNSS_TEST_START_CNF, (sizeof(MN_AT_IndEvt) - 3), VOS_NULL_PTR },
    { VRRC_AT_GNSS_INFO_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_GnssInfoQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_RSU_VPHYSTAT_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_VphyStatQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_RSU_VPHYSTAT_CLR_SET_CNF,
      (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_VphyStatClrSetCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { VRRC_AT_RSU_VSNRRSRP_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_VSnrRsrpQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_GNSS_INFO_GET_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_GnssInfoGetCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_RSU_RSSI_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_RsuRssiQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_SYNC_SOURCE_QRY_CNF,
      (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SyncSourceStateQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { VRRC_AT_SYNC_SOURCE_RPT_SET_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SimpleMsgCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_SYNC_SOURCE_RPT_QRY_CNF,
      (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SyncSourceRptSwitchQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { VRRC_AT_SYNC_SOURCE_RPT_IND,
      (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SyncSourceStatePara) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { VRRC_AT_SYNC_MODE_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SyncModeQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_SYNC_MODE_SET_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SimpleMsgCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VRRC_AT_SET_TX_POWER_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VRRC_AT_SetTxPowerCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkVmacUlMsgLenTbl[] = {
    { VMAC_MSG_RSU_VRSSI_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VMAC_AT_VrssiQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VMAC_AT_RSSI_RPT_IND, (sizeof(MN_AT_IndEvt) + sizeof(VMAC_AT_Pc5RssiPara) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VMAC_AT_CBR_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VMAC_AT_CbrQryCnf) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
    { VMAC_AT_CBR_RPT_IND, (sizeof(MN_AT_IndEvt) + sizeof(VMAC_AT_CbrPara) - TAF_DEFAULT_CONTENT_LEN), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkVtcMsgLenTbl[] = {
    { VTC_AT_TEST_MODE_ACTIVE_STATE_SET_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_MsgSetCnf)), VOS_NULL_PTR },
    { VTC_AT_TEST_MODE_ACTIVE_STATE_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_TestModeActiveStateQryCnf)),
      VOS_NULL_PTR },
    { VTC_AT_TEST_LOOP_MODE_E_CLOSE_STATE_SET_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_MsgSetCnf)), VOS_NULL_PTR },
    { VTC_AT_TEST_LOOP_MODE_E_CLOSE_STATE_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_TestModeECloseStateQryCnf)),
      VOS_NULL_PTR },
    { VTC_AT_SIDELINK_PACKET_COUNTER_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_SidelinkPacketCounterQryCnf)),
      VOS_NULL_PTR },
    { VTC_AT_CBR_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_CbrQryCnf)), VOS_NULL_PTR },
    { VTC_AT_RESET_UTC_TIME_SET_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_MsgSetCnf)), VOS_NULL_PTR },
    { VTC_AT_SENDING_DATA_ACTION_SET_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_MsgSetCnf)), VOS_NULL_PTR },
    { VTC_AT_SENDING_DATA_ACTION_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_SendingDataActionQryCnf)),
      VOS_NULL_PTR },
    { VTC_AT_PC5_SEND_DATA_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_SendVMacDataCnf)), VOS_NULL_PTR },
    { VTC_AT_LAYER_TWO_ID_SET_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_MsgSetCnf)), VOS_NULL_PTR },
    { VTC_AT_LAYER_TWO_ID_QRY_CNF, (sizeof(VTC_AT_MsgCnf) + sizeof(VTC_AT_TestModeModifyL2IdQryCnf)), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkVpdcpUlMsgLenTbl[] = {
    { VPDCP_MSG_RSU_PTRRPT_QRY_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VPDCP_AT_PtrrptQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
    { VPDCP_MSG_RSU_PTRRPT_SET_CNF, (sizeof(MN_AT_IndEvt) + sizeof(VPDCP_AT_PtrrptQryCnf) - TAF_DEFAULT_CONTENT_LEN),
      VOS_NULL_PTR },
};
#endif

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
static TAF_ChkMsgLenNameMapTbl g_dmsChkDmsMsgLenTbl[] = {
    { ID_DMS_MSC_IND, sizeof(struct DMS_DMS_MscInd), VOS_NULL_PTR },
    { ID_DMS_INTRA_PORT_EVENT_IND, 0, AT_ChkDmsIntraPortEventIndMsgLen },
};

static TAF_ChkMsgLenNameMapTbl g_atChkAppPppMsgLenTbl[] = {
    { AT_PPP_RELEASE_IND_MSG, sizeof(AT_PPP_ReleaseInd), VOS_NULL_PTR },
    { AT_PPP_PROTOCOL_REL_IND_MSG, sizeof(AT_PPP_ProtocolRelInd), VOS_NULL_PTR },
};

static TAF_ChkMsgLenSndPidMapTbl g_dmsChkMsgLenTbl[] = {
    { PS_PID_DMS, DMS_GetChkDmsMsgLenTblAddr, DMS_GetChkDmsMsgLenTblSize },
};
#endif

static TAF_ChkMsgLenNameMapTbl g_atChkRnicMsgLenTbl[] = {
    { ID_RNIC_AT_DIAL_MODE_CNF, sizeof(RNIC_AT_DialModeCnf), VOS_NULL_PTR },
    { ID_AT_RNIC_RMNET_CFG_CNF, sizeof(RNIC_AT_RmnetCfgCnf), VOS_NULL_PTR },
};

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
static TAF_ChkMsgLenNameMapTbl g_atRcvAppNdisMsgChkTbl[] = {
    { ID_AT_NDIS_PDNINFO_CFG_CNF, sizeof(AT_NDIS_PdnInfoCfgCnf), VOS_NULL_PTR },
    { ID_AT_NDIS_PDNINFO_REL_CNF, sizeof(AT_NDIS_PdnInfoRelCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_atChkDmsMsgLenTbl[] = {
    { ID_DMS_PORT_SUBSCRIP_EVENT_NOTIFY, 0, AT_ChkDmsSubscripEventNotify },
};

static TAF_ChkMsgLenNameMapTbl g_atChkEmatMsgLenTbl[] = {
    { EMAT_AT_EVENT_CNF, 0, AT_ChkEmatAtEventCnfMsgLen },
};
#endif

/* MAPS_PB_PID/MAPS_PIH_PID/MAPS_EMAT_PID 直接在函数内部检查 不在入口处理 */
static TAF_ChkMsgLenSndPidMapTbl g_atChkMsgLenTbl[] = {
    { ACPU_PID_RNIC, AT_GetChkRnicMsgLenTblAddr, AT_GetChkRnicMsgLenTblSize },
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
    { PS_PID_APP_PPP, AT_GetChkAppPppMsgLenTblAddr, AT_GetChkAppPppMsgLenTblSize },
    { PS_PID_APP_NDIS, AT_GetChkAppNdisMsgLenTblAddr, AT_GetChkAppNdisMsgLenTblSize },
    { PS_PID_DMS, AT_GetChkDmsMsgLenTblAddr, AT_GetChkDmsMsgLenTblSize },
#endif
    { I0_WUEPS_PID_TAF, AT_GetChkTafMsgLenTblAddr, AT_GetChkTafMsgLenTblSize },
    { I1_WUEPS_PID_TAF, AT_GetChkTafMsgLenTblAddr, AT_GetChkTafMsgLenTblSize },
    { I2_WUEPS_PID_TAF, AT_GetChkTafMsgLenTblAddr, AT_GetChkTafMsgLenTblSize },
    { I0_UEPS_PID_DSM, AT_GetChkDsmMsgLenTblAddr, AT_GetChkDsmMsgLenTblSize },
    { I1_UEPS_PID_DSM, AT_GetChkDsmMsgLenTblAddr, AT_GetChkDsmMsgLenTblSize },
    { I2_UEPS_PID_DSM, AT_GetChkDsmMsgLenTblAddr, AT_GetChkDsmMsgLenTblSize },
    { I0_WUEPS_PID_DRV_AGENT, AT_GetChkDrvAgentMsgLenTblAddr, AT_GetChkDrvAgentMsgLenTblSize },
    { I1_WUEPS_PID_DRV_AGENT, AT_GetChkDrvAgentMsgLenTblAddr, AT_GetChkDrvAgentMsgLenTblSize },
    { I2_WUEPS_PID_DRV_AGENT, AT_GetChkDrvAgentMsgLenTblAddr, AT_GetChkDrvAgentMsgLenTblSize },
    { I0_WUEPS_PID_VC, AT_GetChkVcMsgLenTblAddr, AT_GetChkVcMsgLenTblSize },
    { I1_WUEPS_PID_VC, AT_GetChkVcMsgLenTblAddr, AT_GetChkVcMsgLenTblSize },
    { I2_WUEPS_PID_VC, AT_GetChkVcMsgLenTblAddr, AT_GetChkVcMsgLenTblSize },
    { I0_WUEPS_PID_CC, AT_GetChkCcMsgLenTblAddr, AT_GetChkCcMsgLenTblSize },
    { I1_WUEPS_PID_CC, AT_GetChkCcMsgLenTblAddr, AT_GetChkCcMsgLenTblSize },
    { I2_WUEPS_PID_CC, AT_GetChkCcMsgLenTblAddr, AT_GetChkCcMsgLenTblSize },
    { I0_MSP_L4_L4A_PID, AT_GetChkMspL4aMsgLenTblAddr, AT_GetChkMspL4aMsgLenTblSize },
    { I1_MSP_L4_L4A_PID, AT_GetChkMspL4aMsgLenTblAddr, AT_GetChkMspL4aMsgLenTblSize },
    { I0_MSP_SYS_FTM_PID, AT_GetChkMspSysFtmMsgLenTblAddr, AT_GetChkMspSysFtmMsgLenTblSize },
    { I1_MSP_SYS_FTM_PID, AT_GetChkMspSysFtmMsgLenTblAddr, AT_GetChkMspSysFtmMsgLenTblSize },
    { I0_MAPS_STK_PID, AT_GetChkMapsStkMsgLenTblAddr, AT_GetChkMapsStkMsgLenTblSize },
    { I1_MAPS_STK_PID, AT_GetChkMapsStkMsgLenTblAddr, AT_GetChkMapsStkMsgLenTblSize },
    { I2_MAPS_STK_PID, AT_GetChkMapsStkMsgLenTblAddr, AT_GetChkMapsStkMsgLenTblSize },
    { I0_WUEPS_PID_RABM, AT_GetChkRabmMsgMsgLenTblAddr, AT_GetChkRabmMsgMsgLenTblSize },
    { I1_WUEPS_PID_RABM, AT_GetChkRabmMsgMsgLenTblAddr, AT_GetChkRabmMsgMsgLenTblSize },
    { I2_WUEPS_PID_RABM, AT_GetChkRabmMsgMsgLenTblAddr, AT_GetChkRabmMsgMsgLenTblSize },
    { WUEPS_PID_SPY, AT_GetChkSpyMsgLenTblAddr, AT_GetChkSpyMsgLenTblSize },
    { I0_UEPS_PID_MTA, AT_GetChkMtaMsgLenMapTblAddr, AT_GetChkMtaMsgLenTblSize },
    { I1_UEPS_PID_MTA, AT_GetChkMtaMsgLenMapTblAddr, AT_GetChkMtaMsgLenTblSize },
    { I2_UEPS_PID_MTA, AT_GetChkMtaMsgLenMapTblAddr, AT_GetChkMtaMsgLenTblSize },
    { I0_PS_PID_IMSA, AT_GetChkImsaMsgLenTblAddr, AT_GetChkImsaMsgLenTblSize },
    { I1_PS_PID_IMSA, AT_GetChkImsaMsgLenTblAddr, AT_GetChkImsaMsgLenTblSize },
    { PS_PID_CSS, AT_GetChkCssMsgLenTblAddr, AT_GetChkCssMsgLenTblSize },
#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
    { I0_PS_PID_NAS_COMM, AT_GetChkThrotMsgLenTblAddr, AT_GetChkThrotMsgLenTblSize },
    { I1_PS_PID_NAS_COMM, AT_GetChkThrotMsgLenTblAddr, AT_GetChkThrotMsgLenTblSize },
    { I2_PS_PID_NAS_COMM, AT_GetChkThrotMsgLenTblAddr, AT_GetChkThrotMsgLenTblSize },
    { I0_PS_PID_NRTHROT, AT_GetChkThrotMsgLenTblAddr, AT_GetChkThrotMsgLenTblSize },
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { CCPU_PID_CBT, AT_GetChkCbtMsgLenTblAddr, AT_GetChkCbtMsgLenTblSize },
    { DSP_PID_BBA_CAL, AT_GetChkBbicMsgLenTblAddr, AT_GetChkBbicMsgLenTblSize },
    { CCPU_PID_PAM_MFG, AT_GetChkPamMfgMsgLenTblAddr, AT_GetChkPamMfgMsgLenTblSize },
    { I0_DSP_PID_IDLE, AT_GetChkDspIdleMsgLenTblAddr, AT_GetChkDspIdleMsgLenTblSize },
    { I1_DSP_PID_IDLE, AT_GetChkDspIdleMsgLenTblAddr, AT_GetChkDspIdleMsgLenTblSize },
    { I2_DSP_PID_IDLE, AT_GetChkDspIdleMsgLenTblAddr, AT_GetChkDspIdleMsgLenTblSize },
#endif
    { I0_UEPS_PID_CCM, AT_GetChkCcmMsgLenTblAddr, AT_GetChkCcmMsgLenTblSize },
    { I1_UEPS_PID_CCM, AT_GetChkCcmMsgLenTblAddr, AT_GetChkCcmMsgLenTblSize },
    { I2_UEPS_PID_CCM, AT_GetChkCcmMsgLenTblAddr, AT_GetChkCcmMsgLenTblSize },
#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { I0_PS_PID_VNAS, AT_GetChkVnasMsgLenTblAddr, AT_GetChkVnasMsgLenTblSize },
#endif
    { I0_PS_PID_VERRC, AT_GetChkVrrcMsgLenTblAddr, AT_GetChkVrrcMsgLenTblSize },
    { I0_PS_PID_VMAC_UL, AT_GetChkVmacUlMsgLenTblAddr, AT_GetChkVmacUlMsgLenTblSize },
    { I0_PS_PID_VTC, AT_GetChkVtcMsgLenTblAddr, AT_GetChkVtcMsgLenTblSize },
    { I0_PS_PID_VPDCP_UL, AT_GetChkVpdcpUlMsgTblAddr, AT_GetChkVpdcpUlMsgTblSize },
#endif
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
    { I0_MAPS_EMAT_PID, AT_GetChkEmatMsgLenTblAddr, AT_GetChkEmatMsgLenTblSize },
    { I1_MAPS_EMAT_PID, AT_GetChkEmatMsgLenTblAddr, AT_GetChkEmatMsgLenTblSize },
    { I2_MAPS_EMAT_PID, AT_GetChkEmatMsgLenTblAddr, AT_GetChkEmatMsgLenTblSize },
#endif
};

/* MSP_APP_DS_FID下的PID接收消息有效长度检查MAP */
static TAF_ChkMsgLenRcvPidMapTbl g_mspAppDsFidChkMsgLenTbl[] = {
    TAF_MSG_CHECK_TBL_ITEM(WUEPS_PID_AT, g_atChkMsgLenTbl),
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
    TAF_MSG_CHECK_TBL_ITEM(PS_PID_DMS, g_dmsChkMsgLenTbl),
#endif
};

static AT_PbMsgLenInfo g_atPbMsgLenTbl[] = {
    { SI_PB_EVENT_INFO_IND, sizeof(SI_PB_EventInfoInd), VOS_NULL_PTR },
    { SI_PB_EVENT_SET_CNF, sizeof(SI_PB_EventSetCnf), VOS_NULL_PTR },
    { SI_PB_EVENT_READ_CNF, sizeof(SI_PB_EventReadCnf), VOS_NULL_PTR },
    { SI_PB_EVENT_SREAD_CNF, sizeof(SI_PB_EventReadCnf), VOS_NULL_PTR },
    { SI_PB_EVENT_SEARCH_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_SADD_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_ADD_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_MODIFY_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_SMODIFY_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_DELETE_CNF, 0, VOS_NULL_PTR },
    { SI_PB_EVENT_QUERY_CNF, sizeof(SI_PB_EventQueryCnf), VOS_NULL_PTR },
};

STATIC VOS_UINT32 AT_ChkCssCloudDataReportMsgLen(const MSG_Header *msgHeader)
{
    const CSS_AT_CloudDataReport *dataReport = VOS_NULL_PTR;
    VOS_UINT32 minMsgLen;
    VOS_UINT32 expectedLen;

    minMsgLen = sizeof(CSS_AT_CloudDataReport) - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }

    dataReport  = (const CSS_AT_CloudDataReport *)msgHeader;
    expectedLen = minMsgLen + dataReport->dataLen;

    return TAF_ChkMsgLenWithExpectedLen(msgHeader, expectedLen);
}

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkThrotSingleApnBackoffTimeParaNtfMsgLen(const MSG_Header *msgHeader)
{
    VOS_UINT32 minMsgLen;

    minMsgLen = sizeof(THROT_AT_SingleApnBackoffParaNtf) - VOS_MSG_HEAD_LENGTH;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkThrotSingleSnssaiBackoffParaNtfMsgLen(const MSG_Header *msgHeader)
{
    VOS_UINT32 minMsgLen;

    minMsgLen = sizeof(THROT_AT_SingleSnssaiBackoffParaNtf) - VOS_MSG_HEAD_LENGTH;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
STATIC VOS_UINT32 AT_ChkThrotSingleSnssaiDnnBackoffParaNtfMsgLen(const MSG_Header *msgHeader)
{
    VOS_UINT32 minMsgLen;

    minMsgLen = sizeof(THROT_AT_SingleSnssaiDnnBackoffParaNtf) - VOS_MSG_HEAD_LENGTH;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

STATIC VOS_UINT32 AT_ChkImsaFusioncallCtrlMsgLen(const MSG_Header *msgHeader)
{
    const IMSA_AT_FusionCallCtrlMsg *callCtrl = VOS_NULL_PTR;
    VOS_UINT32 minMsgLen;
    VOS_UINT32 expectedLen;

    minMsgLen = sizeof(IMSA_AT_FusionCallCtrlMsg) - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }
    callCtrl = (const IMSA_AT_FusionCallCtrlMsg *)msgHeader;
    expectedLen = minMsgLen + callCtrl->msgLen;

    return TAF_ChkMsgLenWithExpectedLen(msgHeader, expectedLen);
}

STATIC VOS_UINT32 AT_ChkMtaLrrcUeCapParaInfoNtfMsgLen(const MSG_Header *msgHeader)
{
    const MTA_AT_LrrcUeCapInfoNotify *lrrcNotify = VOS_NULL_PTR;
    const AT_MTA_Msg *mtaMsg = VOS_NULL_PTR;
    VOS_UINT32 minMsgLen;
    VOS_UINT32 expectedLen;

    minMsgLen = sizeof(AT_MTA_Msg) + sizeof(MTA_AT_LrrcUeCapInfoNotify) -
        TAF_DEFAULT_CONTENT_LEN - VOS_MSG_HEAD_LENGTH;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }

    mtaMsg = (AT_MTA_Msg *)msgHeader;
    lrrcNotify = (MTA_AT_LrrcUeCapInfoNotify *)mtaMsg->content;
    expectedLen = minMsgLen + lrrcNotify->msgLen;

    return TAF_ChkMsgLenWithExpectedLen(msgHeader, expectedLen);
}

STATIC VOS_UINT32 AT_ChkMtaEcidSetCnfMsgLen(const MSG_Header *msgHeader)
{
    const MTA_AT_EcidSetCnf *secCnf = VOS_NULL_PTR;
    const AT_MTA_Msg *mtaMsg = VOS_NULL_PTR;
    VOS_UINT32 minMsgLen;
    VOS_UINT32 strMaxLen;
    VOS_UINT32 i;

    /* 有两个4字节的可变长数组, 所以需要减去两次TAF_DEFAULT_CONTENT_LEN */
    minMsgLen = sizeof(AT_MTA_Msg) - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN + sizeof(MTA_AT_EcidSetCnf) -
        TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }
    strMaxLen = VOS_GET_MSG_LEN(msgHeader) - minMsgLen;
    mtaMsg = (const AT_MTA_Msg *)msgHeader;
    secCnf = (const MTA_AT_EcidSetCnf *)mtaMsg->content;
    for (i = 0; i < strMaxLen; i++) {
        /* 正常情况下应该是最后一个字节是0, 所以从后往前遍历效率高 */
        if (secCnf->cellInfoStr[strMaxLen - i - 1] == 0) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
STATIC VOS_UINT32 AT_ChkMtaNrrcUeCapParaInfoNtfMsgLen(const MSG_Header *msgHeader)
{
    const MTA_AT_NrrcUeCapInfoNotify *nrrcNotify = VOS_NULL_PTR;
    const AT_MTA_Msg *mtaMsg = VOS_NULL_PTR;
    VOS_UINT32 minMsgLen;
    VOS_UINT32 expectedLen;

    minMsgLen = sizeof(AT_MTA_Msg) + sizeof(MTA_AT_NrrcUeCapInfoNotify) -
        TAF_DEFAULT_CONTENT_LEN - VOS_MSG_HEAD_LENGTH;
    if (VOS_GET_MSG_LEN(msgHeader) < minMsgLen) {
        return VOS_FALSE;
    }
    mtaMsg = (AT_MTA_Msg *)msgHeader;
    nrrcNotify = (MTA_AT_NrrcUeCapInfoNotify *)mtaMsg->content;
    expectedLen = minMsgLen + nrrcNotify->msgLen;

    return TAF_ChkMsgLenWithExpectedLen(msgHeader, expectedLen);
}
#endif

VOS_UINT32 AT_ChkMtaQryNmrMsgLen(const MSG_Header *msgHeader)
{
    const AT_MTA_Msg       *mtaMsg = VOS_NULL_PTR;
    const MTA_AT_QryNmrCnf *qryNmr = VOS_NULL_PTR;
    VOS_UINT32              fixLen;

    /* AT_MTA_Msg和MTA_AT_QryNmrCnf结构体中都有默认的4字节 */
    fixLen = sizeof(AT_MTA_Msg) + sizeof(MTA_AT_QryNmrCnf) - 2 * TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < fixLen) {
        return VOS_FALSE;
    }

    mtaMsg = (const AT_MTA_Msg *)msgHeader;
    qryNmr = (const MTA_AT_QryNmrCnf *)(mtaMsg->content);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < (fixLen + qryNmr->nmrLen)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#if (FEATURE_ON== FEATURE_UE_MODE_CDMA)

STATIC VOS_UINT32 AT_ChkMtaEvdoRevARLinkMsgLen(const MSG_Header *msgHeader)
{
    const AT_MTA_Msg                  *mtaMsg = VOS_NULL_PTR;
    const MTA_AT_EvdoRevaRlinkInfoInd *rlinkInfo = VOS_NULL_PTR;
    VOS_UINT32                         fixLen;

    fixLen = sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EvdoRevaRlinkInfoInd) - 2 * TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < fixLen) {
        return VOS_FALSE;
    }

    mtaMsg    = (const AT_MTA_Msg *)msgHeader;
    rlinkInfo = (const MTA_AT_EvdoRevaRlinkInfoInd *)(mtaMsg->content);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < (fixLen + rlinkInfo->paraLen)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC VOS_UINT32 AT_ChkMtaEvdoSigExEventMsgLen(const MSG_Header *msgHeader)
{
    const AT_MTA_Msg               *mtaMsg = VOS_NULL_PTR;
    const MTA_AT_EvdoSigExeventInd *sigExEvent = VOS_NULL_PTR;
    VOS_UINT32                      fixLen;

    fixLen = sizeof(AT_MTA_Msg) + sizeof(MTA_AT_EvdoSigExeventInd) - 2 * TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < fixLen) {
        return VOS_FALSE;
    }

    mtaMsg     = (const AT_MTA_Msg *)msgHeader;
    sigExEvent = (const MTA_AT_EvdoSigExeventInd *)(mtaMsg->content);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < (fixLen + sigExEvent->paraLen)) {
            return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

VOS_UINT32 AT_ChkDrvAgentSimlockDataReadExReadCnfMsgLen(const MSG_Header *msg)
{
    const DRV_AGENT_Msg                      *agentMsg   = VOS_NULL_PTR;
    const DRV_AGENT_SimlockdatareadexReadCnf *readCnfMsg = VOS_NULL_PTR;
    VOS_UINT32                                fixLen;

    fixLen = sizeof(DRV_AGENT_Msg) +
             sizeof(DRV_AGENT_SimlockdatareadexReadCnf) - VOS_MSG_HEAD_LENGTH - 2 * TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msg) < fixLen) {
        return VOS_FALSE;
    }

    agentMsg   = (const DRV_AGENT_Msg *)msg;
    readCnfMsg = (const DRV_AGENT_SimlockdatareadexReadCnf *)agentMsg->content;
    fixLen += readCnfMsg->dataLen;
    if (VOS_GET_MSG_LEN(msg) < fixLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_ChkUeCbtRfIcIdExQryCnfMsgLen(const MSG_Header *msgHeader)
{
    UECBT_AT_DieIdQueryInd *rcvMsg = (UECBT_AT_DieIdQueryInd *)msgHeader;
    VOS_UINT32     minMsgLen;

    minMsgLen = sizeof(UECBT_AT_DieIdQueryInd);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    minMsgLen += rcvMsg->chipNum;
    if (VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

VOS_UINT32 AT_ChkFtmSetLtCommCmdCnfMsgLen(const MSG_Header *msgHeader)
{
    const AT_FW_DataMsg *dataMsg = VOS_NULL_PTR;
    VOS_UINT32           minMsgLen;

    minMsgLen = sizeof(AT_FW_DataMsg);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }
    dataMsg = (const AT_FW_DataMsg *)msgHeader;

    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < (minMsgLen + dataMsg->len)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkFtmRdLtCommCmdCnfMsgLen(const MSG_Header *msgHeader)
{
    const AT_FW_DataMsg *dataMsg = VOS_NULL_PTR;
    VOS_UINT32           minMsgLen;

    if (VOS_GET_MSG_LEN(msgHeader) < (sizeof(AT_FW_DataMsg) - VOS_MSG_HEAD_LENGTH)) {
        return VOS_FALSE;
    }
    dataMsg = (const AT_FW_DataMsg *)msgHeader;

    minMsgLen = sizeof(AT_FW_DataMsg) + dataMsg->len;
    if (VOS_GET_MSG_LEN(msgHeader) < (minMsgLen - VOS_MSG_HEAD_LENGTH)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

#if (VOS_OS_VER == VOS_WIN32)
LOCAL VOS_UINT32 AT_ChkMnCallBackPsCallMsgLen(const MSG_Header *msg)
{
    /* PC阶段对MN_CALLBACK_PS_CALL事件由后续的g_atPsEvtFuncTbl检查 */
    return VOS_TRUE;
}
#endif

STATIC VOS_UINT32 AT_ChkMnCallBackQryMsgLen(const MSG_Header *msg)
{
    const MN_AT_IndEvt *msgEvent = VOS_NULL_PTR;
    const TAF_QryRslt  *event = VOS_NULL_PTR;
    VOS_UINT32          minMsgLen;

    minMsgLen = sizeof(MN_AT_IndEvt) + sizeof(TAF_QryRslt) - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msg) < minMsgLen) {
        return VOS_FALSE;
    }

    msgEvent = (const MN_AT_IndEvt *)msg;
    event = (const TAF_QryRslt *)msgEvent->content;
    if (VOS_GET_MSG_LEN(msg) < (minMsgLen + event->paraLen)) {
        return VOS_FALSE;
    }

#if (VOS_OS_VER == VOS_WIN32)
    if (event->queryType == TAF_PH_ICC_ID) {
        if (VOS_GET_MSG_LEN(msg) != minMsgLen + sizeof(TAF_PH_IccId)) {
            return VOS_FALSE;
        }
    }
#endif

    return VOS_TRUE;
}

STATIC VOS_UINT32 AT_ChkMnCallBackSsMsgLen(const MSG_Header *msg)
{
    const TAF_SSA_Evt *event = VOS_NULL_PTR;
    VOS_UINT32         totalCallMsgLen;
    VOS_UINT32         totalLcsMsgLen;

    if (VOS_GET_MSG_LEN(msg) < (sizeof(TAF_SSA_Evt) - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN)) {
        return VOS_FALSE;
    }

    event = (TAF_SSA_Evt *)msg;
    if (event->evtExt != 0) {
        totalCallMsgLen = sizeof(MN_AT_IndEvt) + sizeof(TAF_SS_CallIndependentEvent) - TAF_DEFAULT_CONTENT_LEN;
        return TAF_ChkMsgLenWithExpectedLen(msg, totalCallMsgLen - VOS_MSG_HEAD_LENGTH);
    }

    totalLcsMsgLen = sizeof(TAF_SSA_Evt) + sizeof(TAF_Ctrl) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < totalLcsMsgLen) {
        return VOS_FALSE;
    }
    return VOS_TRUE;
}

STATIC VOS_UINT32 AT_ChkMnCallBackPBMsgLen(const MSG_Header *msg)
{
    const MN_AT_IndEvt    *evtMsg = VOS_NULL_PTR;
    const SI_PB_EventInfo *event = VOS_NULL_PTR;
    VOS_UINT32             actualMsgLen;
    TAF_UINT32             size;
    TAF_UINT32             i;
#if (VOS_OS_VER == VOS_WIN32)
    VOS_UINT32             pbEventStructLen;
#endif

    actualMsgLen = sizeof(MN_AT_IndEvt) + AT_GET_SI_PB_EVENT_LEN_WITHOUT_UNION - TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH < actualMsgLen) {
        return VOS_FALSE;
    }

    evtMsg = (const MN_AT_IndEvt *)msg;
    event =  (const SI_PB_EventInfo *)evtMsg->content;

    size = (sizeof(g_atPbMsgLenTbl) / sizeof(g_atPbMsgLenTbl[0]));
    /*
     * 1、单板场景下，MN_CALLBACK_PHONE_BOOK消息的检查原则是消息申请按照union最大申请或者该消息实际大小
     * 申请，只有(VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < fixdedPartLen这种情况认为检查失败
     * 2、PC环境下，MN_CALLBACK_PHONE_BOOK消息的检查原则，消息申请的大小即不是按照union最大申请，也不是
     * 消息实际大小申请时，认为检查失败
     */
#if (VOS_OS_VER == VOS_WIN32)
    pbEventStructLen = sizeof(MN_AT_IndEvt) + sizeof(SI_PB_EventInfo) - TAF_DEFAULT_CONTENT_LEN;
    for (i = 0; i < size; i++) {
        if (event->pbEventType == g_atPbMsgLenTbl[i].eventType) {
            if (((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) != pbEventStructLen) &&
                (TAF_RunChkMsgLenFunc((const MSG_Header *)msg, (actualMsgLen + g_atPbMsgLenTbl[i].fixdedPartLen),
                g_atPbMsgLenTbl[i].chkFunc) != VOS_TRUE)) {
                return VOS_FALSE;
            }
            break;
        }
    }
#else
    for (i = 0; i < size; i++) {
        if (event->pbEventType == g_atPbMsgLenTbl[i].eventType) {
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)msg, (actualMsgLen + g_atPbMsgLenTbl[i].fixdedPartLen),
                g_atPbMsgLenTbl[i].chkFunc) != VOS_TRUE) {

                return VOS_FALSE;
            }

            break;
        }
    }
#endif
    return VOS_TRUE;
}

VOS_UINT32 AT_ChkDrvAgentMemInfoQryRspMsgLen(const MSG_Header *msg)
{
    VOS_UINT32 memBufLen;
    VOS_UINT32 actualMsgLen;

    /* DRVAGENT_RcvDrvAgentMemInfoQry函数内存申请是固定的 */
    memBufLen = AT_PID_MEM_INFO_LEN * sizeof(AT_PID_MemInfoPara);
    actualMsgLen = (sizeof(DRV_AGENT_Msg) +
                    sizeof(DRV_AGENT_MeminfoQryRsp) - TAF_DEFAULT_CONTENT_LEN) - TAF_DEFAULT_CONTENT_LEN + memBufLen;

    if (VOS_GET_MSG_LEN(msg) < (actualMsgLen - VOS_MSG_HEAD_LENGTH)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkDrvAgentNandBadBlockQryRspMsgLen(const MSG_Header *msg)
{
    const DRV_AGENT_Msg           *rcvMsg = VOS_NULL_PTR;
    const DRV_AGENT_NandbbcQryCnf *nandBadBlockQryRspMsg = VOS_NULL_PTR;
    VOS_UINT32                     minValidLen;
    VOS_UINT32                     actualLength;
    VOS_UINT32                     badBlockNum;

    /* 最小有效长度，参考DRVAGENT_RcvDrvAgentNandBadBlockReq */
    minValidLen = (sizeof(DRV_AGENT_Msg) + sizeof(DRV_AGENT_NandbbcQryCnf) - TAF_DEFAULT_CONTENT_LEN);

    if (VOS_GET_MSG_LEN(msg) >= (minValidLen - VOS_MSG_HEAD_LENGTH)) {
        rcvMsg = (const DRV_AGENT_Msg *)msg;
        nandBadBlockQryRspMsg = (const DRV_AGENT_NandbbcQryCnf *)rcvMsg->content;

        badBlockNum = TAF_MIN(nandBadBlockQryRspMsg->badBlockNum, DRV_AGENT_NAND_BADBLOCK_MAX_NUM);

        actualLength = minValidLen + badBlockNum * sizeof(VOS_UINT32);
        if (actualLength - VOS_MSG_HEAD_LENGTH > VOS_GET_MSG_LEN(msg)) {
            return VOS_FALSE;
        }

        return VOS_TRUE;
    }

    return VOS_FALSE;
}

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkEmatMsgLenTblAddr(VOS_VOID)
{
    return g_atChkEmatMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkEmatMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkEmatMsgLenTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkTafMsgLenTblAddr(VOS_VOID)
{
    return g_atChkTafMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkTafMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkTafMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDsmMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDsmMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkDsmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDsmMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDrvAgentMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDrvAgentMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkDrvAgentMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDrvAgentMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVcMsgLenTblAddr(VOS_VOID)
{
    return g_atChkVcMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVcMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVcMsgLenTbl);
}
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
TAF_ChkMsgLenNameMapTbl* AT_GetChkDspWphyMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDspWphyMsgLenTbl;
}

VOS_UINT32 AT_GetChkDspWphyMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDspWphyMsgLenTbl);
}

TAF_ChkMsgLenNameMapTbl* AT_GetChkDspGphyMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDspGphyMsgLenTbl;
}

VOS_UINT32 AT_GetChkDspGphyMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDspGphyMsgLenTbl);
}
#endif
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCcMsgLenTblAddr(VOS_VOID)
{
    return g_atChkCcMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkCcMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkCcMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMspL4aMsgLenTblAddr(VOS_VOID)
{
    return g_atChkMspL4aMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkMspL4aMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkMspL4aMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMspSysFtmMsgLenTblAddr(VOS_VOID)
{
    return g_atChkMspSysFtmMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkMspSysFtmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkMspSysFtmMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMapsStkMsgLenTblAddr(VOS_VOID)
{
    return g_atChkMapsStkMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkMapsStkMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkMapsStkMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkRabmMsgMsgLenTblAddr(VOS_VOID)
{
    return g_atChkRabmMsgMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkRabmMsgMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkRabmMsgMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkSpyMsgLenTblAddr(VOS_VOID)
{
    return g_atChkSpyMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkSpyMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkSpyMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkMtaMsgLenMapTblAddr(VOS_VOID)
{
    return g_atChkMtaMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkMtaMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkMtaMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkImsaMsgLenTblAddr(VOS_VOID)
{
    return g_atChkImsaMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkImsaMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkImsaMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCssMsgLenTblAddr(VOS_VOID)
{
    return g_atChkCssMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkCssMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkCssMsgLenTbl);
}

#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkThrotMsgLenTblAddr(VOS_VOID)
{
    return g_atChkThrotMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkThrotMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkThrotMsgLenTbl);
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCbtMsgLenTblAddr(VOS_VOID)
{
    return g_atChkCbtMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkCbtMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkCbtMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkBbicMsgLenTblAddr(VOS_VOID)
{
    return g_atChkBbicMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkBbicMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkBbicMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkPamMfgMsgLenTblAddr(VOS_VOID)
{
    return g_atChkPamMfgMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkPamMfgMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkPamMfgMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDspIdleMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDspIdleMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkDspIdleMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDspIdleMsgLenTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkCcmMsgLenTblAddr(VOS_VOID)
{
    return g_atChkCcmMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkCcmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkCcmMsgLenTbl);
}

#if (FEATURE_LTEV == FEATURE_ON)

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVnasMsgLenTblAddr(VOS_VOID)
{
    return g_atChkVnasMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVnasMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVnasMsgLenTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVrrcMsgLenTblAddr(VOS_VOID)
{
    return g_atChkVrrcMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVrrcMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVrrcMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVmacUlMsgLenTblAddr(VOS_VOID)
{
    return g_atChkVmacUlMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVmacUlMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVmacUlMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVtcMsgLenTblAddr(VOS_VOID)
{
    return g_atChkVtcMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVtcMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVtcMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkVpdcpUlMsgTblAddr(VOS_VOID)
{
    return g_atChkVpdcpUlMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkVpdcpUlMsgTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkVpdcpUlMsgLenTbl);
}
#endif

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkDmsMsgLenTblAddr(VOS_VOID)
{
    return g_atChkDmsMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkDmsMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkDmsMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* DMS_GetChkDmsMsgLenTblAddr(VOS_VOID)
{
    return g_dmsChkDmsMsgLenTbl;
}

STATIC VOS_UINT32 DMS_GetChkDmsMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_dmsChkDmsMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkAppPppMsgLenTblAddr(VOS_VOID)
{
    return g_atChkAppPppMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkAppPppMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkAppPppMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkAppNdisMsgLenTblAddr(VOS_VOID)
{
    return g_atRcvAppNdisMsgChkTbl;
}

STATIC VOS_UINT32 AT_GetChkAppNdisMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atRcvAppNdisMsgChkTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* AT_GetChkRnicMsgLenTblAddr(VOS_VOID)
{
    return g_atChkRnicMsgLenTbl;
}

STATIC VOS_UINT32 AT_GetChkRnicMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_atChkRnicMsgLenTbl);
}

VOS_UINT32 AT_IsAt2AtMsg(const MSG_Header *msgHeader)
{
    if ((VOS_GET_SENDER_ID(msgHeader) != WUEPS_PID_AT) || (VOS_GET_RECEIVER_ID(msgHeader) != WUEPS_PID_AT)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/* AT命令消息不遵循按消息名分发机制，独立函数处理 */
VOS_UINT32 AT_ChkAt2AtMsgLen(const MSG_Header *msgHeader)
{
    const AT_Msg *atMsg = VOS_NULL_PTR;
    VOS_UINT32    minMsgLen;

    /* 非AT命令消息，返回VOS_TRUE */
    if (AT_IsAt2AtMsg(msgHeader) != VOS_TRUE) {
        return VOS_TRUE;
    }

    minMsgLen = sizeof(AT_Msg) - AT_MSG_DEFAULT_VALUE_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    atMsg = (const AT_Msg *)msgHeader;
    minMsgLen += atMsg->len;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 DMS_ChkDmsFidRcvMsgLen(const MsgBlock *msg)
{
    const MSG_Header *msgHeader = VOS_NULL_PTR;

    if (VOS_GET_SENDER_ID(msg) == VOS_PID_TIMER) {
        return TAF_ChkTimerMsgLen(msg);
    }

    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < sizeof(MSG_Header)) {
        return VOS_FALSE;
    }

    msgHeader = (const MSG_Header *)msg;
    if (AT_ChkAt2AtMsgLen(msgHeader) != VOS_TRUE) {
        return VOS_FALSE;
    }

    return TAF_ChkMsgLen(msg, g_mspAppDsFidChkMsgLenTbl, TAF_GET_ARRAY_NUM(g_mspAppDsFidChkMsgLenTbl));
}

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))

VOS_UINT32 AT_ChkDmsIntraPortEventIndMsgLen(const MSG_Header *msg)
{
    const struct DMS_IntraEvent *intraEvent = VOS_NULL_PTR;

    if (VOS_GET_MSG_LEN(msg) < (sizeof(struct DMS_IntraEvent) - VOS_MSG_HEAD_LENGTH)) {
        return VOS_FALSE;
    }

    intraEvent = (const struct DMS_IntraEvent *)msg;
    if (VOS_GET_MSG_LEN(msg) < (sizeof(struct DMS_IntraEvent) + intraEvent->len - VOS_MSG_HEAD_LENGTH)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

VOS_VOID DMS_SortDmsFidChkMsgLenTbl(VOS_VOID)
{
    TAF_SortChkMsgLenTblByMsgName(g_atChkRnicMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkRnicMsgLenTbl));
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) || (defined(__PC_UT__)))
    TAF_SortChkMsgLenTblByMsgName(g_atRcvAppNdisMsgChkTbl, TAF_GET_ARRAY_NUM(g_atRcvAppNdisMsgChkTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkDmsMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkDmsMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkAppPppMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkAppPppMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_dmsChkDmsMsgLenTbl, TAF_GET_ARRAY_NUM(g_dmsChkDmsMsgLenTbl));
#endif
#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    TAF_SortChkMsgLenTblByMsgName(g_atChkVnasMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVnasMsgLenTbl));
#endif
    TAF_SortChkMsgLenTblByMsgName(g_atChkVrrcMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVrrcMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkVmacUlMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVmacUlMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkVtcMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVtcMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkVpdcpUlMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVpdcpUlMsgLenTbl));
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    TAF_SortChkMsgLenTblByMsgName(g_atChkDspIdleMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkDspIdleMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkPamMfgMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkPamMfgMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkBbicMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkBbicMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkCbtMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkCbtMsgLenTbl));
#endif
    TAF_SortChkMsgLenTblByMsgName(g_atChkCcmMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkCcmMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkCssMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkCssMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkImsaMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkImsaMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkMtaMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkMtaMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkSpyMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkSpyMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkRabmMsgMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkSpyMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkMapsStkMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkMapsStkMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkMspSysFtmMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkMspSysFtmMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkMspL4aMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkMspL4aMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkCcMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkCcMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkVcMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkVcMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkDrvAgentMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkDrvAgentMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkTafMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkTafMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_atChkDsmMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkDsmMsgLenTbl));
#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
    TAF_SortChkMsgLenTblByMsgName(g_atChkThrotMsgLenTbl, TAF_GET_ARRAY_NUM(g_atChkThrotMsgLenTbl));
#endif
}

VOS_BOOL AT_ChkPbMsgLen(const MSG_Header *msg)
{
    const MN_APP_PbAtCnf  *pbAtCnfMsg  = VOS_NULL_PTR;
    const SI_PB_EventInfo *event = VOS_NULL_PTR;
    TAF_UINT32             size;
    TAF_UINT32             i;

    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < AT_GET_PB_AT_CNF_LEN_WITHOUT_UNION) {
        return VOS_FALSE;
    }
    pbAtCnfMsg = (const MN_APP_PbAtCnf *)msg;
    event = (const SI_PB_EventInfo *)&pbAtCnfMsg->pbAtEvent;

    size = (sizeof(g_atPbMsgLenTbl) / sizeof(g_atPbMsgLenTbl[0]));

    for (i = 0; i < size; i++) {
        if (event->pbEventType == g_atPbMsgLenTbl[i].eventType) {
#if (VOS_OS_VER == VOS_WIN32)
            if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) == sizeof(MN_APP_PbAtCnf)) {
                return VOS_TRUE;
            }
#endif
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)msg,
                (AT_GET_PB_AT_CNF_LEN_WITHOUT_UNION + g_atPbMsgLenTbl[i].fixdedPartLen),
                g_atPbMsgLenTbl[i].chkFunc) != VOS_TRUE) {
                return VOS_FALSE;
            }
            break;
        }
    }
    return VOS_TRUE;
}


