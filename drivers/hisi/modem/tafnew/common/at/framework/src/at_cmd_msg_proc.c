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

#include "at_cmd_msg_proc.h"
#include "securec.h"
#include "si_app_pb.h"
#include "si_app_stk.h"
#include "at_msg_print.h"
#include "at_mdrv_interface.h"
#include "at_phy_interface.h"
#include "ppp_interface.h"
#include "at_data_proc.h"
#include "at_event_report.h"
#include "at_rabm_interface.h"
#include "at_rnic_interface.h"
#include "at_device_cmd.h"
#include "at_init.h"
#include "at_common.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_lte_common.h"
#endif

#include "nv_stru_sys.h"
#include "nv_stru_was.h"
#include "nv_stru_gas.h"

#include "app_vc_api.h"

#include "product_config.h"

#include "at_cmd_css_proc.h"
#include "at_acore_only_cmd.h"

#include "at_mt_msg_proc.h"
#include "at_file_handle.h"

#include "mn_comm_api.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "at_ltev_msg_proc.h"
#include "vnas_at_interface.h"
#endif
#include "at_mta_interface.h"
#include "taf_phy_pid_def.h"
#include "dms_msg_chk.h"
#include "dms_port_i.h"


#include "at_device_as_rslt_proc.h"
#include "at_device_drv_rslt_proc.h"
#include "at_device_taf_rslt_proc.h"
#include "at_device_pam_rslt_proc.h"
#include "at_device_phy_rslt_proc.h"

#include "at_voice_taf_rslt_proc.h"
#include "at_voice_event_report.h"
#include "at_voice_hifi_rslt_proc.h"
#include "at_voice_mm_rslt_proc.h"
#include "at_ss_taf_rslt_proc.h"
#include "at_ss_event_report.h"
#include "at_sms_event_report.h"
#include "at_sms_taf_rslt_proc.h"
#include "at_sim_event_report.h"
#include "at_sim_pam_rslt_proc.h"
#include "at_sat_pam_rslt_proc.h"
#include "at_test_phy_rslt_proc.h"
#include "at_custom_as_rslt_proc.h"
#include "at_phone_taf_rslt_proc.h"
#include "at_phone_mm_rslt_proc.h"
#include "at_phone_as_rslt_proc.h"
#include "at_phone_event_report.h"

#include "at_general_drv_rslt_proc.h"
#include "at_general_mm_rslt_proc.h"
#include "at_general_pam_rslt_proc.h"
#include "at_general_taf_rslt_proc.h"
#include "at_general_event_report.h"
#include "at_safety_pam_rslt_proc.h"
#include "at_custom_drv_rslt_proc.h"
#include "at_custom_hifi_rslt_proc.h"
#include "at_custom_ims_rslt_proc.h"
#include "at_custom_l2_rslt_proc.h"
#include "at_custom_lnas_rslt_proc.h"
#include "at_custom_mm_rslt_proc.h"
#include "at_custom_pam_rslt_proc.h"
#include "at_custom_phy_rslt_proc.h"
#include "at_custom_taf_rslt_proc.h"
#include "at_custom_nrmm_rslt_proc.h"
#include "at_custom_comm_rslt_proc.h"
#include "at_custom_rcm_rslt_proc.h"
#include "at_custom_comm.h"
#include "at_custom_event_report.h"
#include "at_ltev_mm_rslt_proc.h"
#include "at_ltev_taf_rslt_proc.h"
#include "at_ltev_ttf_rslt_proc.h"
#include "at_test_ttf_rslt_proc.h"
#include "at_test_as_rslt_proc.h"
#include "at_safety_mm_rslt_proc.h"
#include "at_voice_ims_rslt_proc.h"
#include "at_cdma_rslt_proc.h"
#include "at_cdma_event_report.h"
#include "at_voice_ims_rslt_proc.h"
#include "at_phone_ims_rslt_proc.h"
#include "at_custom_event_report.h"
#include "at_data_event_report.h"
#include "at_data_taf_rslt_proc.h"
#include "at_data_mm_rslt_proc.h"
#include "at_proc_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CMDMSGPROC_C

#define AT_CMD_TMP_MAX_LENGTH 20

#define AT_LTE_CATEGORY_DEFAULT_VALUE 6
#define NAS_OM_EVENTIND_DATA_LEN 4

extern VOS_UINT32    AT_ProcRabmSetFastDormParaCnf(
        RABM_AT_SetFastdormParaCnf     *pstMsg
    );
extern VOS_UINT32 AT_RcvRnicDialModeCnf(MsgBlock *pstMsg);
extern VOS_VOID AT_RcvRnicRmnetCfgCnf(MsgBlock *msg);
extern VOS_UINT32 AT_ProcRabmReleaseRrcCnf(RABM_AT_ReleaseRrcCnf *pstMsg);

/* AT 模块处理来自AT AGENT消息函数对应表 */
static const AT_PROC_MsgFromDrvAgent g_atProcMsgFromDrvAgentTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { DRV_AGENT_MSID_QRY_CNF, AT_RcvDrvAgentMsidQryCnf },

    { DRV_AGENT_HARDWARE_QRY_RSP, AT_RcvDrvAgentHardwareQryRsp },
    { DRV_AGENT_FULL_HARDWARE_QRY_RSP, AT_RcvDrvAgentFullHardwareQryRsp },
    { DRV_AGENT_SIMLOCK_SET_CNF, AT_RcvDrvAgentSetSimlockCnf },

    { DRV_AGENT_VERTIME_QRY_CNF, AT_RcvDrvAgentVertimeQryRsp },
    { DRV_AGENT_YJCX_SET_CNF, AT_RcvDrvAgentYjcxSetCnf },
    { DRV_AGENT_YJCX_QRY_CNF, AT_RcvDrvAgentYjcxQryCnf },
    { DRV_AGENT_GPIOPL_SET_CNF, AT_RcvDrvAgentSetGpioplRsp },
    { DRV_AGENT_GPIOPL_QRY_CNF, AT_RcvDrvAgentQryGpioplRsp },
    { DRV_AGENT_DATALOCK_SET_CNF, AT_RcvDrvAgentSetDatalockRsp },
    { DRV_AGENT_TBATVOLT_QRY_CNF, AT_RcvDrvAgentQryTbatvoltRsp },
    { DRV_AGENT_VERSION_QRY_CNF, AT_RcvDrvAgentQryVersionRsp },
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    { DRV_AGENT_FCHAN_SET_CNF, AT_RcvDrvAgentSetFchanRsp },
#endif
    { DRV_AGENT_PRODTYPE_QRY_CNF, AT_RcvDrvAgentQryProdtypeRsp },

    { DRV_AGENT_CPULOAD_QRY_CNF, AT_RcvDrvAgentCpuloadQryRsp },
    { DRV_AGENT_MFREELOCKSIZE_QRY_CNF, AT_RcvDrvAgentMfreelocksizeQryRsp },
    { DRV_AGENT_MEMINFO_QRY_CNF, AT_RcvDrvAgentMemInfoQryRsp },
    { DRV_AGENT_DLOADINFO_QRY_CNF, AT_RcvDrvAgentDloadInfoQryRsp },
    { DRV_AGENT_FLASHINFO_QRY_CNF, AT_RcvDrvAgentFlashInfoQryRsp },
    { DRV_AGENT_AUTHORITYID_QRY_CNF, AT_RcvDrvAgentAuthorityIdQryRsp },
    { DRV_AGENT_AUTHVER_QRY_CNF, AT_RcvDrvAgentAuthVerQryRsp },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { DRV_AGENT_GODLOAD_SET_CNF, AT_RcvDrvAgentGodloadSetRsp },
#endif
    { DRV_AGENT_PFVER_QRY_CNF, AT_RcvDrvAgentPfverQryRsp },
    { DRV_AGENT_HWNATQRY_QRY_CNF, AT_RcvDrvAgentHwnatQryRsp },
    { DRV_AGENT_APPDMVER_QRY_CNF, AT_RcvDrvAgentAppdmverQryRsp },

    { DRV_AGENT_IMSICHG_QRY_CNF, AT_RcvDrvAgentImsiChgQryRsp },
    { DRV_AGENT_INFORBU_SET_CNF, AT_RcvDrvAgentInfoRbuSetRsp },
#if (FEATURE_LTE == FEATURE_ON)
    { DRV_AGENT_INFORRS_SET_CNF, AT_RcvDrvAgentInfoRrsSetRsp },
#endif
    { DRV_AGENT_CPNN_QRY_CNF, AT_RcvDrvAgentCpnnQryRsp },
    { DRV_AGENT_CPNN_TEST_CNF, AT_RcvDrvAgentCpnnTestRsp },
    { DRV_AGENT_NVBACKUP_SET_CNF, AT_RcvDrvAgentNvBackupSetRsp },

    { DRV_AGENT_NVRESTORE_SET_CNF, AT_RcvDrvAgentSetNvRestoreCnf },
    { DRV_AGENT_NVRSTSTTS_QRY_CNF, AT_RcvDrvAgentQryNvRestoreRstCnf },
    { DRV_AGENT_NVRESTORE_MANU_DEFAULT_CNF, AT_RcvDrvAgentNvRestoreManuDefaultRsp },

    { DRV_AGENT_ADC_SET_CNF, AT_RcvDrvAgentSetAdcRsp },
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    { DRV_AGENT_TSELRF_SET_CNF, AT_RcvDrvAgentTseLrfSetRsp },
#endif
    { DRV_AGENT_HKADC_GET_CNF, AT_RcvDrvAgentHkAdcGetRsp },

    { DRV_AGENT_TBAT_QRY_CNF, AT_RcvDrvAgentQryTbatRsp },
#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
    { DRV_AGENT_SPWORD_SET_CNF, AT_RcvDrvAgentSetSpwordRsp },
#endif

    { DRV_AGENT_NVBACKUPSTAT_QRY_CNF, AT_RcvDrvAgentNvBackupStatQryRsp },
    { DRV_AGENT_NANDBBC_QRY_CNF, AT_RcvDrvAgentNandBadBlockQryRsp },
    { DRV_AGENT_NANDVER_QRY_CNF, AT_RcvDrvAgentNandDevInfoQryRsp },
    { DRV_AGENT_CHIPTEMP_QRY_CNF, AT_RcvDrvAgentChipTempQryRsp },

    { DRV_AGENT_MAX_LOCK_TIMES_SET_CNF, AT_RcvDrvAgentSetMaxLockTmsRsp },

    { DRV_AGENT_AP_SIMST_SET_CNF, AT_RcvDrvAgentSetApSimstRsp },

    { DRV_AGENT_HUK_SET_CNF, AT_RcvDrvAgentHukSetCnf },
    { DRV_AGENT_FACAUTHPUBKEY_SET_CNF, AT_RcvDrvAgentFacAuthPubkeySetCnf },
    { DRV_AGENT_IDENTIFYSTART_SET_CNF, AT_RcvDrvAgentIdentifyStartSetCnf },
    { DRV_AGENT_IDENTIFYEND_SET_CNF, AT_RcvDrvAgentIdentifyEndSetCnf },
    { DRV_AGENT_SIMLOCKDATAWRITE_SET_CNF, AT_RcvDrvAgentSimlockDataWriteSetCnf },
    { DRV_AGENT_PHONESIMLOCKINFO_QRY_CNF, AT_RcvDrvAgentPhoneSimlockInfoQryCnf },
    { DRV_AGENT_PHONESIMLOCKINFO_QRY_NEW_CNF, AT_RcvDrvAgentPhoneSimlockInfoQryCnf },
    { DRV_AGENT_SIMLOCKDATAREAD_QRY_CNF, AT_RcvDrvAgentSimlockDataReadQryCnf },
    { DRV_AGENT_PHONEPHYNUM_SET_CNF, AT_RcvDrvAgentPhonePhynumSetCnf },
    { DRV_AGENT_OPWORD_SET_CNF, AT_RcvDrvAgentOpwordSetCnf },

    { DRV_AGENT_SWVER_SET_CNF, AT_RcvDrvAgentSwverSetCnf },

    { DRV_AGENT_NVMANUFACTUREEXT_SET_CNF, AT_RcvNvManufactureExtSetCnf },


    { DRV_AGENT_QRY_CCPU_MEM_INFO_CNF, AT_RcvDrvAgentQryCcpuMemInfoCnf },

    { DRV_AGENT_SIMLOCKWRITEEX_SET_CNF, AT_RcvDrvAgentSimlockWriteExSetCnf },
    { DRV_AGENT_SIMLOCKDATAREADEX_SET_CNF, AT_RcvDrvAgentSimlockDataReadExReadCnf },
    { DRV_AGENT_GETMODEMSCID_QRY_CNF, AT_RcvDrvAgentGetSimlockEncryptIdQryCnf },
    {DRV_AGENT_HKADC_QRY_CNF, AT_RcvDrvAgentQryAdcRsp},
};

static const AT_PROC_MsgFromDms g_atProcMsgFromDmsTab[] = {
    /* 消息ID */                            /* 消息处理函数 */
    { ID_DMS_PORT_SUBSCRIP_EVENT_NOTIFY,    AT_RcvDmsSubscripEventNotify },
};

const AT_PROC_EventFromDms g_atProcEventFromDmsTab[] = {
    /* event ID */                    /* 消息处理函数 */
    { ID_DMS_EVENT_LOW_WATER_MARK, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsLowWaterMarkEvent },
    { ID_DMS_EVENT_NCM_CONN_BREAK, sizeof(struct DMS_PORT_SubscripEventNotify) + sizeof(struct DMS_NcmConnBreakInfo),
      VOS_NULL_PTR, AT_RcvDmsNcmConnBreakEvent },
    { ID_DMS_EVENT_USB_DISCONNECT, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsUsbDisconnectEvent },
    { ID_DMS_EVENT_SWITCH_GW_MODE, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsSwitchGwModeEvent },
    { ID_DMS_EVENT_ESCAPE_SEQUENCE, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsEscapeSequenceEvent },
    { ID_DMS_EVENT_DTR_DEASSERT, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsDtrDeassertedEvent },
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    { ID_DMS_EVENT_UART_INIT, sizeof(struct DMS_PORT_SubscripEventNotify), VOS_NULL_PTR,
      AT_RcvDmsUartInitEvent },
#endif
};

static const AT_PROC_MsgFromVc g_atProcMsgFromVcTab[] = {
    { APP_VC_MSG_CNF_QRY_MODE, At_RcvVcMsgQryModeCnfProc },
    { APP_VC_MSG_SET_FOREGROUND_CNF, At_RcvVcMsgSetGroundCnfProc },
    { APP_VC_MSG_SET_BACKGROUND_CNF, At_RcvVcMsgSetGroundCnfProc },
    { APP_VC_MSG_FOREGROUND_RSP, At_RcvVcMsgQryGroundRspProc },
    { APP_VC_MSG_QRY_TTYMODE_CNF, At_RcvVcMsgQryTTYModeCnfProc },
    { APP_VC_MSG_SET_TTYMODE_CNF, At_RcvVcMsgSetTTYModeCnfProc },
#if (FEATURE_ECALL == FEATURE_ON)
    { APP_VC_MSG_SET_MSD_CNF, AT_RcvVcMsgSetMsdCnfProc },
    { APP_VC_MSG_QRY_MSD_CNF, AT_RcvVcMsgQryMsdCnfProc },
    { APP_VC_MSG_QRY_ECALL_CFG_CNF, AT_RcvVcMsgQryEcallCfgCnfProc },
    { APP_VC_MSG_SET_ECALL_PUSH_CNF, AT_RcvVcMsgEcallPushCnfProc},
#endif
    { APP_VC_MSG_DTMF_DECODER_IND, At_RcvVcMsgDtmfDecoderIndProc},
#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { APP_VC_MSG_I2S_TEST_CNF, At_RcvVcI2sTestCnfProc},
    { APP_VC_MSG_I2S_TEST_RSLT_IND, AT_RcvVcI2sTestRsltIndProc},
#endif
};

AT_MSG_Proc g_atMsgProc[] = {
    { WUEPS_PID_AT, At_CmdMsgDistr },
    { I0_WUEPS_PID_TAF, At_TafAndDmsMsgProc },
    { I1_WUEPS_PID_TAF, At_TafAndDmsMsgProc },
    { I2_WUEPS_PID_TAF, At_TafAndDmsMsgProc },
    { I0_UEPS_PID_DSM, At_TafAndDmsMsgProc },
    { I1_UEPS_PID_DSM, At_TafAndDmsMsgProc },
    { I2_UEPS_PID_DSM, At_TafAndDmsMsgProc },
    { PS_PID_APP_PPP, At_PppMsgProc },
    { PS_PID_APP_NDIS, AT_RcvNdisMsg },
    { I0_DSP_PID_WPHY, At_HPAMsgProc },
    { I1_DSP_PID_WPHY, At_HPAMsgProc },
    { I0_DSP_PID_GPHY, At_GHPAMsgProc },
    { I1_DSP_PID_GPHY, At_GHPAMsgProc },
    { I2_DSP_PID_GPHY, At_GHPAMsgProc },
    { I0_WUEPS_PID_MMA, AT_ProcMmaMsg },
    { I1_WUEPS_PID_MMA, AT_ProcMmaMsg },
    { I2_WUEPS_PID_MMA, AT_ProcMmaMsg },
    { I0_WUEPS_PID_DRV_AGENT, At_ProcMsgFromDrvAgent },
    { I1_WUEPS_PID_DRV_AGENT, At_ProcMsgFromDrvAgent },
    { I2_WUEPS_PID_DRV_AGENT, At_ProcMsgFromDrvAgent },
    { I0_WUEPS_PID_VC, At_ProcMsgFromVc },
    { I1_WUEPS_PID_VC, At_ProcMsgFromVc },
    { I1_WUEPS_PID_VC, At_ProcMsgFromVc },
    { ACPU_PID_RNIC, At_RcvRnicMsg },
    { I0_WUEPS_PID_CC, At_ProcMsgFromCc },
    { I1_WUEPS_PID_CC, At_ProcMsgFromCc },
    { I1_WUEPS_PID_CC, At_ProcMsgFromCc },
#if (FEATURE_LTE == FEATURE_ON)
    { I0_MSP_L4_L4A_PID, at_L4aCnfProc },
    { I1_MSP_L4_L4A_PID, at_L4aCnfProc },
    { I0_MSP_SYS_FTM_PID, At_FtmEventMsgProc },
    { I1_MSP_SYS_FTM_PID, At_FtmEventMsgProc },
#endif
    { I0_MAPS_STK_PID, At_StkMsgProc },
    { I1_MAPS_STK_PID, At_StkMsgProc },
    { I2_MAPS_STK_PID, At_StkMsgProc },
    { I0_MAPS_PB_PID, At_PbMsgProc },
    { I1_MAPS_PB_PID, At_PbMsgProc },
    { I2_MAPS_PB_PID, At_PbMsgProc },
    { I0_MAPS_PIH_PID, At_PIHMsgProc },
    { I1_MAPS_PIH_PID, At_PIHMsgProc },
    { I2_MAPS_PIH_PID, At_PIHMsgProc },
    { I0_WUEPS_PID_RABM, AT_RabmMsgProc },
    { I1_WUEPS_PID_RABM, AT_RabmMsgProc },
    { I2_WUEPS_PID_RABM, AT_RabmMsgProc },
    { WUEPS_PID_SPY, AT_SpyMsgProc },
    { I0_UEPS_PID_MTA, AT_ProcMtaMsg },
    { I1_UEPS_PID_MTA, AT_ProcMtaMsg },
    { I2_UEPS_PID_MTA, AT_ProcMtaMsg },
#if (FEATURE_IMS == FEATURE_ON)
    { I0_PS_PID_IMSA, AT_ProcImsaMsg },
    { I1_PS_PID_IMSA, AT_ProcImsaMsg },
#endif
    { PS_PID_CSS, AT_ProcCssMsg },
#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
    { I0_PS_PID_NAS_COMM, AT_ProcThrotMsg },
    { I1_PS_PID_NAS_COMM, AT_ProcThrotMsg },
    { I2_PS_PID_NAS_COMM, AT_ProcThrotMsg },
    { I0_PS_PID_NRTHROT,  AT_ProcThrotMsg },
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { I0_UEPS_PID_XSMS, AT_ProcXsmsMsg },
    { I1_UEPS_PID_XSMS, AT_ProcXsmsMsg },
    { I2_UEPS_PID_XSMS, AT_ProcXsmsMsg },
    { DSP_PID_PROCSTUB, At_CHPAMsgProc },
#endif
#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
    { I0_UEPS_PID_XPDS, AT_ProcXpdsMsg },
    { I1_UEPS_PID_XPDS, AT_ProcXpdsMsg },
    { I2_UEPS_PID_XPDS, AT_ProcXpdsMsg },
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { CCPU_PID_CBT, AT_ProcCbtMsg },
    { DSP_PID_BBA_CAL, AT_ProcBbicMsg },
    { CCPU_PID_PAM_MFG, AT_ProcUeCbtMsg },
    { I0_PHY_PID_IDLE, AT_ProcDspIdleMsg },
#endif
    { I0_UEPS_PID_CCM, AT_ProcCcmMsg },
    { I1_UEPS_PID_CCM, AT_ProcCcmMsg },
    { I2_UEPS_PID_CCM, AT_ProcCcmMsg },
    { I0_MAPS_EMAT_PID, At_EMATMsgProc },
    { I1_MAPS_EMAT_PID, At_EMATMsgProc },
    { I2_MAPS_EMAT_PID, At_EMATMsgProc },
#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { I0_PS_PID_VNAS, AT_ProcMsgFromVnas },
#endif
    { I0_PS_PID_VERRC, AT_ProcMsgFromVrrc },
    { I0_PS_PID_VMAC_UL, AT_ProcMsgFromVmac },
    { I0_PS_PID_VTC, AT_ProcMsgFromVtc },
    { I0_PS_PID_VPDCP_UL, AT_ProcMsgFromVpdcp },
#endif
    { PS_PID_DMS, AT_ProcDmsMsg },
};

/*
 * AT码流缓存数组(包括AT命令或其它数据)。
 * 设置该缓存数组的原因:底软任务在调用At_CmdStreamPreProc接口时，某些场景下(如直接使用超级终端发送AT命令),则会
 * 出现AT码流以一个字符为单位发送到AT的消息队列中，导致AT的消息队列慢，触发单板复位。
 */
AT_DataStreamBuffer g_atDataBuff[AT_MAX_CLIENT_NUM];

/* CREG/CGREG的<CI>域以4字节上报是否使能(VDF需求) */

AT_ClientManage g_atClientTab[AT_MAX_CLIENT_NUM];

TAF_UINT32 g_pbPrintTag = VOS_FALSE;

/*
 * 3 函数、变量声明
 */
VOS_UINT32 AT_PlmnTimeZoneProc(TAF_UINT32 mcc, NAS_MM_InfoInd *mmTimeInfo);


#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON && FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)

VOS_VOID AT_SetCsCallStateWakeLock(TAF_CCM_MsgTypeUint32 msgName)
{
    switch(msgName) {
        case ID_TAF_CCM_CALL_ORIG_IND:
        case ID_TAF_CCM_CALL_CONNECT_IND:
        case ID_TAF_CCM_CALL_INCOMING_IND:
        case ID_TAF_CCM_CALL_PROC_IND:
        case ID_TAF_CCM_CALL_ALERTING_IND:
        case ID_TAF_CCM_CALL_HOLD_IND:
        case ID_TAF_CCM_CALL_RETRIEVE_IND:
            AT_WakeLock(&g_callWakeLock);
            break;

        case ID_TAF_CCM_CALL_ALL_RELEASED_IND:
            AT_WakeUnLock(&g_callWakeLock);
            break;

        default:
            break;
    }
}
#endif

VOS_VOID AT_ProcDmsMsg(struct MsgCB *msg)
{
    MSG_Header      *msgTemp = VOS_NULL_PTR;
    VOS_UINT32       i;
    VOS_UINT32       msgCnt;
    VOS_UINT32       rst;

    /* 从g_atProcMsgFromDmsTab中获取消息个数 */
    msgCnt = sizeof(g_atProcMsgFromDmsTab) / sizeof(AT_PROC_MsgFromDms);
    msgTemp   = (MSG_Header *)msg;

    /* g_atProcMsgFromDmsTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromDmsTab[i].msgName == msgTemp->msgName) {
            rst = g_atProcMsgFromDmsTab[i].procMsgFunc((struct MsgCB *)msgTemp);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcDmsMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcDmsMsg: Msg Id is invalid!");
    }
}

VOS_UINT32 AT_ChkDmsSubscripEventNotify(const MSG_Header *msg)
{
    struct DMS_PORT_SubscripEventNotify *event = VOS_NULL_PTR;
    VOS_UINT32 size;
    VOS_UINT32 len;
    VOS_UINT32 i;

    len = sizeof(struct DMS_PORT_SubscripEventNotify);
    if (VOS_GET_MSG_LEN(msg) < len) {
        AT_WARN_LOG("AT_ChkDmsSubscripEventNotify: message length is invalid!");
    }

    event = (struct DMS_PORT_SubscripEventNotify *)msg;

    size = (sizeof(g_atProcEventFromDmsTab) / sizeof(g_atProcEventFromDmsTab[0]));

    for (i = 0; i < size; i++) {
        if (event->eventId == g_atProcEventFromDmsTab[i].eventId) {
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)msg, g_atProcEventFromDmsTab[i].fixdedPartLen,
                    g_atProcEventFromDmsTab[i].chkFunc) != VOS_TRUE) {
                AT_WARN_LOG("AT_ChkDmsSubscripEventNotify: message length is invalid!");

                return VOS_FALSE;
            }
            break;
        }
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_RcvDmsSubscripEventNotify(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify  *event = VOS_NULL_PTR;
    VOS_UINT32                            i;
    VOS_UINT32                            eventCnt;
    VOS_UINT32                            rst;

    /* 从g_atProcEventFromDmsTab中获取事件个数 */
    eventCnt = sizeof(g_atProcEventFromDmsTab) / sizeof(AT_PROC_EventFromDms);
    event  = (struct DMS_PORT_SubscripEventNotify *)msg;

    /* g_atProcEventFromDmsTab查表，进行事件分发 */
    for (i = 0; i < eventCnt; i++) {
        if (g_atProcEventFromDmsTab[i].eventId == event->eventId) {
            rst = g_atProcEventFromDmsTab[i].procEventFunc((struct MsgCB *)msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_RcvDmsSubscripEventNotify: event Proc Err!");
            }

            return rst;
        }
    }

    /* 没有找到匹配的消息 */
    if (eventCnt == i) {
        AT_ERR_LOG("AT_RcvDmsSubscripEventNotify: event Id is invalid!");
    }

    return VOS_ERR;
}

TAF_VOID At_EventMsgProc(MN_AT_IndEvt *msg)
{
    switch (msg->msgName) {
        case MN_CALLBACK_MSG:
            At_SmsMsgProc(msg, msg->len);
            return;

        case MN_CALLBACK_SET:
            At_SetMsgProc((TAF_SetRslt *)msg->content);
            return;

        case MN_CALLBACK_QRY:
            At_QryMsgProc((TAF_QryRslt *)msg->content);
            return;

        case MN_CALLBACK_PS_CALL:
            /* PS域事件处理 */
            AT_RcvTafPsEvt((TAF_PS_Evt *)msg);
            return;

        case MN_CALLBACK_DATA_STATUS:
            At_DataStatusMsgProc((TAF_DataStatusInd *)msg->content);
            return;

        case MN_CALLBACK_SS:
            AT_RcvTafSsaEvt((TAF_SSA_Evt *)msg);
            return;

        case MN_CALLBACK_PHONE:
            At_PhMsgProc(msg->content, msg->len);
            break;

        case MN_CALLBACK_PHONE_BOOK:
            At_TAFPbMsgProc(msg->content, msg->len);
            break;

        case MN_CALLBACK_CMD_CNF:
            At_CmdCnfMsgProc(msg->content, msg->len);
            break;

        case MN_CALLBACK_VOICE_CONTROL:
            At_VcMsgProc(msg, msg->len);
            break;

        case MN_CALLBACK_LOG_PRINT:
            AT_LogPrintMsgProc((TAF_MntnLogPrint *)msg);
            break;

        case MN_CALLBACK_IFACE:
            AT_RcvTafIfaceEvt((TAF_PS_Evt *)msg);
            break;

        default:
            AT_LOG1("At_MsgProc: invalid message name.", msg->msgName);
            return;
    }
}

/*
 * 功能描述: 检查当前权限是否开启
 */
LOCAL VOS_BOOL AT_CheckCurrentE5Right(VOS_BOOL bIsRight)
{
    /* 当前权限未开启,直接返回  */
    if (g_atRightOpenFlg.rightOpenFlg == AT_RIGHT_OPEN_FLAG_CLOSE) {
        return VOS_TRUE;
    }

    /* 如果已经获取到权限,则直接返回 */
    if (bIsRight == VOS_TRUE) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_BOOL AT_E5CheckRight(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    static VOS_BOOL bIsRight = VOS_FALSE;
    VOS_UINT8       cmdTmp[AT_CMD_TMP_MAX_LENGTH]; /* 须大于aucQuickCmd[]的长度 */
    const VOS_UINT8 quickCmd[] = "AT^OPENPORT=";
    VOS_UINT16      quickCmdLen;
    VOS_UINT16      leftLen;
    VOS_UINT8      *pwdPos = VOS_NULL_PTR;
    VOS_CHAR        acPassword[AT_RIGHT_PWD_LEN + 1];
    VOS_UINT8      *atStart = VOS_NULL_PTR;
    errno_t         memResult;

    if (AT_CheckCurrentE5Right(bIsRight) == VOS_TRUE) {
        return VOS_TRUE;
    }

    /* SSCOM 输入的字符串前有 0x0A */
    atStart = data;
    while ((*atStart != 'a') && (*atStart != 'A')) {
        atStart++;
        if (atStart >= (data + len)) {
            /* 未找到 a 或 A break */
            break;
        }
    }
    len -= (VOS_UINT16)(atStart - data);
    data = atStart;

    /* 长度非法,直接返回无权限 */
    quickCmdLen = (VOS_UINT16)VOS_StrLen((VOS_CHAR *)quickCmd);
    if (len <= quickCmdLen) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_FALSE;
    }

    (VOS_VOID)memset_s(cmdTmp, sizeof(cmdTmp), 0x00, sizeof(cmdTmp));

    if (quickCmdLen > 0) {
        memResult = memcpy_s(cmdTmp, sizeof(cmdTmp), data, (VOS_SIZE_T)quickCmdLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cmdTmp), (VOS_SIZE_T)quickCmdLen);
    }

    /* 不是AT^OPENPORT命令直接返回无权限 */
    if (VOS_StrNiCmp((VOS_CHAR *)cmdTmp, (VOS_CHAR *)quickCmd, (VOS_SIZE_T)quickCmdLen) != 0) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_FALSE;
    }

    leftLen = len - quickCmdLen;
    pwdPos  = data + quickCmdLen;

    if (leftLen >= sizeof(cmdTmp)) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_FALSE;
    }

    (VOS_VOID)memset_s(cmdTmp, sizeof(cmdTmp), 0x00, sizeof(cmdTmp));
    memResult = memcpy_s(cmdTmp, sizeof(cmdTmp), pwdPos, leftLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cmdTmp), leftLen);

    (VOS_VOID)memset_s(acPassword, sizeof(acPassword), 0x00, sizeof(acPassword));
    memResult = memcpy_s(acPassword, sizeof(acPassword), g_atRightOpenFlg.password, sizeof(g_atRightOpenFlg.password));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acPassword), sizeof(g_atRightOpenFlg.password));

    /* 比较密码是否相同 */
    if (VOS_StrCmp(acPassword, (VOS_CHAR *)cmdTmp) == 0) {
        /* 上报OK并记录有权限 */
        At_FormatResultData(indexNum, AT_OK);
        bIsRight = VOS_TRUE;
        (VOS_VOID)memset_s(acPassword, sizeof(acPassword), 0x00, sizeof(acPassword));
        return VOS_FALSE;
    }
    (VOS_VOID)memset_s(acPassword, sizeof(acPassword), 0x00, sizeof(acPassword));

    /* 上报ERROR */
    At_FormatResultData(indexNum, AT_ERROR);
    return VOS_FALSE;
}

TAF_VOID At_PppProtocolRelIndProc(const AT_PPP_ProtocolRelInd *msg)
{
    DMS_PortIdUint16        portId;
    VOS_UINT16              pppId;
    VOS_UINT8               indexNum;
    DMS_PortModeUint8       mode;
    DMS_PortDataModeUint8   dataMode;

    pppId    = msg->pppId;
    indexNum = g_atPppIndexTab[pppId];
    portId   = g_atClientTab[indexNum].portNo;

    mode     = DMS_PORT_GetMode(portId);
    dataMode = DMS_PORT_GetDataMode(portId);

    if (AT_IsPppUser(indexNum) == VOS_TRUE) {
        if ((mode == DMS_PORT_MODE_ONLINE_DATA) && (dataMode == DMS_PORT_DATA_PPP) &&
            (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET)) {

            DMS_PORT_ResumeCmdMode(portId);

            /* 停止定时器 */
            AT_StopTimerCmdReady(indexNum);

            /* 回复NO CARRIER */
            g_atSendDataBuff.bufLen = 0;
            At_FormatResultData(indexNum, AT_NO_CARRIER);
        }
    }
}

VOS_VOID At_PppReleaseIndProc(AT_ClientIdUint16 clientId)
{
    if (clientId >= AT_MAX_CLIENT_NUM) {
        AT_WARN_LOG("At_PppReleaseIndProc:ERROR:ucIndex is abnormal!");
        return;
    }

    if (g_atClientTab[clientId].cmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET) {
        /* 将AT通道切换为命令模式 */
        DMS_PORT_ResumeCmdMode(g_atClientTab[clientId].portNo);

        /* 停止定时器 */
        AT_StopTimerCmdReady((VOS_UINT8)clientId);

        /* 回复NO CARRIER */
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData((VOS_UINT8)clientId, AT_NO_CARRIER);

        return;
    }

    if (g_atClientTab[clientId].cmdCurrentOpt == AT_CMD_PS_DATA_CALL_END_SET) {
        return;
    }

    if (TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientId), 0,
                       g_atClientTab[clientId].cid) == VOS_OK) {
        /* 开定时器 */
        if (At_StartTimer(AT_DETACT_PDP_TIME, (VOS_UINT8)clientId) != AT_SUCCESS) {
            AT_ERR_LOG("At_PppReleaseIndProc:ERROR:Start Timer");
            return;
        }

        /* 设置当前操作类型 */
        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
    }
}

/*
 * Description: AT处理PPP发送的消息
 *   History:
 *   1.Date: 2009-09-24
 *   Modification: Created function
 */
TAF_VOID At_PppMsgProc(struct MsgCB *msg)
{
    MSG_Header *msgTemp = VOS_NULL_PTR;

    msgTemp = (MSG_Header *)msg;

    switch (msgTemp->msgName) {
        case AT_PPP_RELEASE_IND_MSG:
            At_PppReleaseIndProc(((AT_PPP_ReleaseInd *)msgTemp)->clientId);
            break;

        case AT_PPP_PROTOCOL_REL_IND_MSG:
            At_PppProtocolRelIndProc((AT_PPP_ProtocolRelInd *)msgTemp);
            break;

        default:
            AT_LOG1("At_PppMsgProc: msgName is abnormal!", msgTemp->msgName);
            break;
    }
}
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

VOS_UINT32 AT_ChkHPAMsgLenFunc(struct MsgCB *msg, TAF_ChkMsgLenNameMapTbl *msgNameMapAddr,
    VOS_UINT32 mapTblSize)
{
    HPA_AT_Header *header = VOS_NULL_PTR;
    VOS_UINT32     i;

    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < sizeof(HPA_AT_Header)) {
        return VOS_FALSE;
    }

    header = (HPA_AT_Header *)msg;

    for (i = 0; i < mapTblSize; i++) {
        if (header->msgId == (VOS_UINT16)msgNameMapAddr[i].msgName) {
                return TAF_RunChkMsgLenFunc((const MSG_Header*)msg,
                    msgNameMapAddr[i].fixdedPartLen, msgNameMapAddr[i].chkFunc);
        }
    }
    return VOS_TRUE;
}
#endif

VOS_VOID At_HPAMsgProc(struct MsgCB *msg)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    HPA_AT_Header           *header = VOS_NULL_PTR;
    TAF_ChkMsgLenNameMapTbl *msgNameMapAddr = VOS_NULL_PTR;
    VOS_UINT32               mapTblSize;

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_HpaMsgProc: pstMsg is NULL");
        return;
    }
    msgNameMapAddr = AT_GetChkDspWphyMsgLenTblAddr();
    mapTblSize = AT_GetChkDspWphyMsgLenTblSize();

    if (AT_ChkHPAMsgLenFunc(msg, msgNameMapAddr, mapTblSize) == VOS_FALSE) {
        AT_WARN_LOG("At_HpaMsgProc: pstMsg msgLen Err");
        return;
    }
    header = (HPA_AT_Header *)msg;

    switch (header->msgId) {
        case ID_HPA_AT_RF_CFG_CNF:
            At_HpaRfCfgCnfProc((HPA_AT_RfCfgCnf *)msg);
            break;

        case ID_HPA_AT_RF_RX_RSSI_IND:
            At_RfRssiIndProc((HPA_AT_RfRxRssiInd *)msg);
            break;

        case ID_WPHY_AT_TX_CLT_IND:
            At_WTxCltIndProc((WPHY_AT_TxCltInd *)msg);
            break;

        case ID_AT_WPHY_RF_PLL_STATUS_CNF:
            At_RfPllStatusCnfProc((PHY_AT_RfPllStatusCnf *)msg);
            break;

        case ID_AT_PHY_POWER_DET_CNF:
            At_RfFpowdetTCnfProc((PHY_AT_PowerDetCnf *)msg);
            break;
        case ID_HPA_AT_MIPI_WR_CNF:
            At_MipiWrCnfProc((HPA_AT_MipiWrCnf *)msg);
            break;
        case ID_HPA_AT_MIPI_RD_CNF:
            At_MipiRdCnfProc((HPA_AT_MipiRdCnf *)msg);
            break;
        case ID_HPA_AT_SSI_WR_CNF:
            At_SsiWrCnfProc((HPA_AT_SsiWrCnf *)msg);
            break;

        case ID_HPA_AT_PDM_CTRL_CNF:
            At_PdmCtrlCnfProc((HPA_AT_PdmCtrlCnf *)msg);
            break;

        default:
            AT_WARN_LOG("At_HpaMsgProc: msgName is Abnormal!");
            break;
    }
#endif
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID At_CHPAMsgProc(struct MsgCB *msg)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    HPA_AT_Header *header = VOS_NULL_PTR;

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_HpaMsgProc: pstMsg is NULL");
        return;
    }

    header = (HPA_AT_Header *)msg;

    switch (header->msgId) {
        case ID_CHPA_AT_RF_CFG_CNF:
            At_CHpaRfCfgCnfProc((CHPA_AT_RfCfgCnf *)msg);
            break;

        case ID_AT_PHY_POWER_DET_CNF:
            At_RfFpowdetTCnfProc((PHY_AT_PowerDetCnf *)msg);
            break;

        default:
            AT_WARN_LOG("At_CHPAMsgProc: msgName is Abnormal!");
            break;
    }
#endif
}
#endif

VOS_VOID At_GHPAMsgProc(struct MsgCB *msg)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    HPA_AT_Header           *header = VOS_NULL_PTR;
    TAF_ChkMsgLenNameMapTbl *msgNameMapAddr = VOS_NULL_PTR;
    VOS_UINT32               mapTblSize;

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_HpaMsgProc: pstMsg is NULL");
        return;
    }
    msgNameMapAddr = AT_GetChkDspGphyMsgLenTblAddr();
    mapTblSize = AT_GetChkDspGphyMsgLenTblSize();

    if (AT_ChkHPAMsgLenFunc(msg, msgNameMapAddr, mapTblSize) == VOS_FALSE) {
        AT_WARN_LOG("At_GHpaMsgProc: pstMsg msgLen Err");
        return;
    }
    header = (HPA_AT_Header *)msg;

    switch (header->msgId) {
        case ID_GHPA_AT_RF_MSG_CNF:
            At_HpaRfCfgCnfProc((HPA_AT_RfCfgCnf *)msg);
            break;

        case ID_GHPA_AT_RF_RX_RSSI_IND:
            At_RfRssiIndProc((HPA_AT_RfRxRssiInd *)msg);
            break;

        case ID_AT_GPHY_RF_PLL_STATUS_CNF:
            At_RfPllStatusCnfProc((PHY_AT_RfPllStatusCnf *)msg);
            break;

        case ID_HPA_AT_MIPI_WR_CNF:
            At_MipiWrCnfProc((HPA_AT_MipiWrCnf *)msg);
            break;

        case ID_HPA_AT_MIPI_RD_CNF:
            At_MipiRdCnfProc((HPA_AT_MipiRdCnf *)msg);
            break;

        case ID_HPA_AT_SSI_WR_CNF:
            At_SsiWrCnfProc((HPA_AT_SsiWrCnf *)msg);
            break;


        case ID_AT_PHY_POWER_DET_CNF:
            At_RfFpowdetTCnfProc((PHY_AT_PowerDetCnf *)msg);
            break;

        default:
            AT_WARN_LOG("At_HpaMsgProc: msgName is Abnormal!");
            break;
    }
#endif
}

VOS_UINT32 AT_GetIndexNumByClientId(VOS_UINT16 clientId, AT_CmdCurOptUint32 cmdOpt, VOS_UINT8 *indexNum)
{
    /* 通过clientid获取index */
    if (At_ClientIdToUserId(clientId, indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_GetIndexNumByClientId:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(*indexNum)) {
        AT_WARN_LOG("AT_GetIndexNumByClientId : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[*indexNum].cmdCurrentOpt != cmdOpt) {
        AT_WARN_LOG("AT_GetIndexNumByClientId: WARNING:Not AT_CMD_OPT IS WRONG!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetSimLockStatus(VOS_UINT8 indexNum)
{
    /* 发消息到C核获取SIMLOCK 状态信息 */
    if (Taf_ParaQuery(g_atClientTab[indexNum].clientId, 0, TAF_PH_SIMLOCK_VALUE_PARA, VOS_NULL_PTR) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_GetSimLockStatus: Taf_ParaQuery fail.");
        return VOS_ERR;
    }

    /* ^SIMLOCK=2查询UE的锁卡状态不在AT命令处理的主流程，需要本地启动保护定时器并更新端口状态 */
    if (At_StartTimer(AT_SET_PARA_TIME, indexNum) != AT_SUCCESS) {
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIMLOCKSTATUS_READ;

    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;

    return VOS_OK;
}

VOS_VOID At_ProcMsgFromDrvAgent(struct MsgCB *msg)
{
    DRV_AGENT_Msg *msgTemp = VOS_NULL_PTR;
    VOS_UINT32     i;
    VOS_UINT32     msgCnt;
    VOS_UINT32     msgId;
    VOS_UINT32     rst;

    /* 从g_drvAgentMsgProcTab中获取消息个数 */
    msgCnt  = sizeof(g_atProcMsgFromDrvAgentTab) / sizeof(AT_PROC_MsgFromDrvAgent);
    msgTemp = (DRV_AGENT_Msg *)msg;

    /* 从消息包中获取MSG ID */
    msgId = ((DRV_AGENT_Msg *)msgTemp)->msgId;

    /* g_drvAgentMsgProcTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromDrvAgentTab[i].msgType == msgId) {
            rst = g_atProcMsgFromDrvAgentTab[i].procMsgFunc((struct MsgCB *)msgTemp);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("At_ProcMsgFromDrvAgent: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("At_ProcMsgFromDrvAgent: Msg Id is invalid!");
    }
}

VOS_VOID At_ProcMsgFromVc(struct MsgCB *msg)
{
    MN_AT_IndEvt *msgTemp = VOS_NULL_PTR;
    VOS_UINT32    i;
    VOS_UINT32    msgCnt;

    msgTemp = (MN_AT_IndEvt *)msg;
    /* 从g_atProcMsgFromVcTab中获取事件个数 */
    msgCnt = sizeof(g_atProcMsgFromVcTab) / sizeof(AT_PROC_MsgFromVc);

    /* g_atProcEventFromDmsTab查表，进行事件分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromVcTab[i].msgName== msgTemp->msgName) {
            g_atProcMsgFromVcTab[i].procMsgFunc(msgTemp);
            return;
        }
    }
    AT_WARN_LOG("At_ProcMsgFromVc : Not Find Func.");
    return;
}

VOS_VOID At_RcvRnicMsg(struct MsgCB *msg)
{
    MSG_Header *msgHeader = VOS_NULL_PTR;

    msgHeader = (MSG_Header *)msg;

    switch (msgHeader->msgName) {
        /* 处理RNIC发来的拨号模式查询回复 */
        case ID_RNIC_AT_DIAL_MODE_CNF:

            AT_RcvRnicDialModeCnf((MsgBlock *)msg);
            break;

        case ID_AT_RNIC_RMNET_CFG_CNF:
            AT_RcvRnicRmnetCfgCnf((MsgBlock *)msg);
            break;

        default:
            AT_ERR_LOG("At_RcvRnicMsg: msgName err.");
            break;
    }
}

VOS_VOID At_ProcMsgFromCc(struct MsgCB *msg)
{
    MSG_Header *msgHeader = VOS_NULL_PTR;

    msgHeader = (MSG_Header *)msg;

    switch (msgHeader->msgName) {
        case AT_CC_MSG_STATE_QRY_CNF:
            At_RcvAtCcMsgStateQryCnfProc(msg);
            break;

        default:
            break;
    }
}

VOS_VOID AT_RcvNdisMsg(struct MsgCB *msg)
{
    MSG_Header *msgTemp = VOS_NULL_PTR;

    msgTemp = (MSG_Header *)msg;
    switch (msgTemp->msgName) {
        case ID_AT_NDIS_PDNINFO_CFG_CNF:
            AT_RcvNdisPdnInfoCfgCnf((AT_NDIS_PdnInfoCfgCnf *)msgTemp);
            break;

        case ID_AT_NDIS_PDNINFO_REL_CNF:
            AT_RcvNdisPdnInfoRelCnf((AT_NDIS_PdnInfoRelCnf *)msgTemp);
            break;

        default:
            AT_WARN_LOG("AT_RcvNdisMsg: Unexpected MSG is received.");
            break;
    }
}

TAF_VOID AT_RabmMsgProc(struct MsgCB *msg)
{
    MSG_Header *msgTemp = VOS_NULL_PTR;

    msgTemp = (MSG_Header *)msg;
    switch (msgTemp->msgName) {
        case ID_RABM_AT_SET_FASTDORM_PARA_CNF:
            AT_ProcRabmSetFastDormParaCnf((RABM_AT_SetFastdormParaCnf *)msgTemp);
            break;

        case ID_RABM_AT_QRY_FASTDORM_PARA_CNF:
            AT_ProcRabmQryFastDormParaCnf((RABM_AT_QryFastdormParaCnf *)msgTemp);
            break;

        case ID_RABM_AT_SET_RELEASE_RRC_CNF:
            AT_ProcRabmReleaseRrcCnf((RABM_AT_ReleaseRrcCnf *)msgTemp);
            break;

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
        case ID_RABM_AT_SET_VOICEPREFER_PARA_CNF:
            AT_ProcRabmSetVoicePreferParaCnf((RABM_AT_SetVoicepreferParaCnf *)msgTemp);
            break;

        case ID_RABM_AT_QRY_VOICEPREFER_PARA_CNF:
            AT_ProcRabmQryVoicePreferEnableParaCnf((RABM_AT_QryVoicepreferParaCnf *)msgTemp);
            break;

        case ID_RABM_AT_VOICEPREFER_STATUS_REPORT:
            AT_ProcRabmVoicePreferStatusReport((RABM_AT_VoicepreferStatusReport *)msgTemp);
            break;
#endif
        default:
            AT_WARN_LOG("At_RabmMsgProc:WARNING:Wrong Msg!");
            break;
    }
}

VOS_UINT32 At_DsmMsgProc(const MSG_Header *msg)
{
    /* NDIS网卡相关的交互先通过AT转，之后AT整体下移之后由DSM直接与NDIS进行交互 */
    switch (msg->msgName) {
        case ID_DSM_NDIS_IFACE_UP_IND:
            AT_PS_ProcNdisIfaceUpCfg((DSM_NDIS_IfaceUpInd *)msg);
            break;

        case ID_DSM_NDIS_IFACE_DOWN_IND:
            AT_PS_ProcNdisIfaceDownCfg((DSM_NDIS_IfaceDownInd *)msg);
            break;

        case ID_DSM_NDIS_CONFIG_IPV6_DNS_IND:
            AT_PS_ProcNdisConfigIpv6Dns((DSM_NDIS_ConfigIpv6DnsInd *)msg);
            break;

        default:
            return AT_ERROR;
    }

    return AT_OK;
}

VOS_VOID At_TafAndDmsMsgProc(struct MsgCB *msg)
{
    /* NDIS网卡相关的交互先通过AT转，之后AT整体下移之后由DSM直接与NDIS进行交互 */
    if (At_DsmMsgProc((MSG_Header *)msg) == AT_OK) {
        return;
    }

    At_EventMsgProc((MN_AT_IndEvt *)msg);
}

AT_MSG_PROC_FUNC At_GetMsgProcFunBySndPid(VOS_UINT32 sndPid)
{
    VOS_UINT32 indexNum;
    VOS_UINT32 sndPidNo;

    sndPidNo = sizeof(g_atMsgProc) / sizeof(g_atMsgProc[0]);

    for (indexNum = 0; indexNum < sndPidNo; indexNum++) {
        if (sndPid == g_atMsgProc[indexNum].sndPid) {
            return g_atMsgProc[indexNum].procMsgFunc;
        }
    }

    return VOS_NULL_PTR;
}

TAF_VOID At_MsgProc(MsgBlock *msg)
{
    AT_MSG_PROC_FUNC procFun;
    VOS_UINT32       sendPid;
    VOS_UINT32       msgName;
    VOS_UINT32       sliceStart;
    VOS_UINT32       sliceEnd;
    VOS_UINT32       type;

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("At_MsgProc: msg is null.");
        return;
    }

    sliceStart = VOS_GetSlice();
    sendPid    = VOS_GET_SENDER_ID(msg);

    if (DMS_ChkDmsFidRcvMsgLen(msg) != VOS_TRUE) {
        AT_ERR_LOG("At_MsgProc: message length is invalid!");
        return;
    }
    msgName    = ((MSG_Header *)msg)->msgName;

    /* 超时处理 */
    if (sendPid == VOS_PID_TIMER) {
        At_TimeOutProc((REL_TimerMsgBlock *)msg);
        sliceEnd = VOS_GetSlice();
        AT_RecordAtMsgInfo(sendPid, msgName, sliceStart, sliceEnd);

        return;
    }

    if (sendPid == WUEPS_PID_AT) {
        type    = ((AT_Msg *)msg)->type;
        msgName = (type << 16) | msgName;
    }

    if ((sendPid == I0_WUEPS_PID_TAF) || (sendPid == I1_WUEPS_PID_TAF) || (sendPid == I2_WUEPS_PID_TAF) ||
        (sendPid == I0_UEPS_PID_DSM) || (sendPid == I1_UEPS_PID_DSM) || (sendPid == I2_UEPS_PID_DSM)) {
        msgName = ((MN_AT_IndEvt *)msg)->msgName;
    }

    procFun = At_GetMsgProcFunBySndPid(sendPid);

    if (procFun == VOS_NULL_PTR) {
        AT_LOG1("At_MsgProc other PID msg", sendPid);
    } else {
        procFun((struct MsgCB *)msg);
    }

    sliceEnd = VOS_GetSlice();
    AT_RecordAtMsgInfo(sendPid, msgName, sliceStart, sliceEnd);
}

VOS_VOID AT_EventReport(VOS_UINT32 pid, NAS_OM_EventIdUint16 eventId, VOS_UINT8 *para, VOS_UINT32 len)
{
    mdrv_diag_event_ind_s diagEvent;
    NAS_OM_EventInd      *atEvent = VOS_NULL_PTR;
    VOS_UINT8            *data    = para;
    VOS_UINT32            atEventMsgLen;
    VOS_UINT32            rslt;
    errno_t               memResult;

    /* 入参检查 */
    if ((data == VOS_NULL_PTR) && (len != 0)) {
        /* 错误打印 */
        TAF_LOG(pid, VOS_NULL, PS_LOG_LEVEL_WARNING, "NAS_EventReport:pPara is NULL.");
        return;
    }

    /* 申请内存 */
    if (len <= NAS_OM_EVENTIND_DATA_LEN) {
        atEventMsgLen = sizeof(NAS_OM_EventInd);
        atEvent       = (NAS_OM_EventInd *)PS_MEM_ALLOC(pid, atEventMsgLen);
    } else {
        atEventMsgLen = sizeof(NAS_OM_EventInd) + len - NAS_OM_EVENTIND_DATA_LEN;
        atEvent       = (NAS_OM_EventInd *)PS_MEM_ALLOC(pid, atEventMsgLen);
    }

    if (atEvent == VOS_NULL_PTR) {
        TAF_LOG(pid, VOS_NULL, PS_LOG_LEVEL_WARNING, "NAS_EventReport:Alloc mem fail.");
        return;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(atEvent, atEventMsgLen, 0x00, atEventMsgLen);

    /* 填写事件消息内容 */
    atEvent->eventId = eventId;
    atEvent->reserve = 0;
    if ((data != VOS_NULL_PTR) && (len > 0)) {
        memResult = memcpy_s(atEvent->data, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    /* 填写发给DIAG的结构体 */
    diagEvent.ulLength  = sizeof(NAS_OM_EventInd) - NAS_OM_EVENTIND_DATA_LEN + len;
    diagEvent.ulPid     = pid;
    diagEvent.ulEventId = (VOS_UINT32)eventId;
    diagEvent.ulModule  = MDRV_DIAG_GEN_MODULE(MODEM_ID_0, DIAG_MODE_UMTS);
    diagEvent.pData     = atEvent;

    rslt = mdrv_diag_event_report(&diagEvent);
    if (rslt != VOS_OK) {
        AT_WARN_LOG("AT_EventReport:OM_AcpuEvent Fail.");
    }

    /*lint -save -e516 */
    PS_MEM_FREE(pid, atEvent);
    /*lint -restore */
}

#if (FEATURE_AGPS == FEATURE_ON && FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_UINT32 AT_RcvMtaCgpsClockSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    VOS_BOOL             bEnable;
    ModemIdUint16        modemId;
    MTA_AT_CgpsclockCnf *setCnf = VOS_NULL_PTR;

    setCnf = (MTA_AT_CgpsclockCnf *)mtaMsg->content;

    /* 初始化消息变量 */
    bEnable  = 0;
    modemId  = MODEM_ID_0;

    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (setCnf->result != MTA_AT_RESULT_NO_ERROR) {
        return AT_ERROR;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        modemId = MODEM_ID_0;
    }

    bEnable = AT_GetCgpsCLockEnableFlgByModemId(modemId);

    if (AT_SetCgpsClock(bEnable, setCnf->modemId, setCnf->ratMode, indexNum) != VOS_OK) {
        AT_ERR_LOG("AT_RcvMtaCgpsClockSetCnf: AT_CfgGpsRefClk() failed!");
        return AT_ERROR;
    }

    return AT_OK;
}
#endif

VOS_VOID AT_ReportResetCmd(AT_ResetReportCauseUint32 cause)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_RESET].text, cause,
        g_atCrLf);

    /* 需要上报给上层的双RIL */
    At_SendResultData(AT_CLIENT_ID_APP, g_atSndCodeAddress, length);
    At_SendResultData(AT_CLIENT_ID_PCUI, g_atSndCodeAddress, length);

#if (MULTI_MODEM_NUMBER > 1)
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    At_SendResultData(AT_CLIENT_ID_APP5, g_atSndCodeAddress, length);
#endif
    At_SendResultData(AT_CLIENT_ID_CTRL, g_atSndCodeAddress, length);
#if (MULTI_MODEM_NUMBER > 2)
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    At_SendResultData(AT_CLIENT_ID_APP20, g_atSndCodeAddress, length);
#endif
    At_SendResultData(AT_CLIENT_ID_PCUI2, g_atSndCodeAddress, length);
#endif
#endif
}

VOS_VOID AT_StopAllTimer(VOS_VOID)
{
    VOS_UINT8        modemIndex;
    VOS_UINT32       clientIndex;
    VOS_UINT32       timerName;
    AT_ModemCcCtx   *ccCtx         = VOS_NULL_PTR;
    AT_ParseContext *parseContext  = VOS_NULL_PTR;
    AT_ClientManage *clientContext = VOS_NULL_PTR;

    for (modemIndex = 0; modemIndex < MODEM_ID_BUTT; modemIndex++) {
        ccCtx = AT_GetModemCcCtxAddrFromClientId(modemIndex);

        /* 停止S0定时器 */
        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            timerName = ccCtx->s0TimeInfo.timerName;

            AT_StopRelTimer(timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = VOS_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }
    }

    for (clientIndex = 0; clientIndex < AT_MAX_CLIENT_NUM; clientIndex++) {
        parseContext = &(g_parseContext[clientIndex]);
        AT_StopRelTimer(clientIndex, &parseContext->timer);

        clientContext = &(g_atClientTab[clientIndex]);
        AT_StopRelTimer(clientIndex, &clientContext->hTimer);
    }
}

VOS_VOID AT_ResetParseCtx(VOS_VOID)
{
    VOS_UINT8        clientIndex;
    AT_ParseContext *parseContext = VOS_NULL_PTR;

    for (clientIndex = 0; clientIndex < AT_MAX_CLIENT_NUM; clientIndex++) {
        parseContext = &(g_parseContext[clientIndex]);

        parseContext->clientStatus = AT_FW_CLIENT_STATUS_READY;

        /* 清空所有的缓存的AT命令 */
        AT_ClearBlockCmdInfo(clientIndex);

        /* 重置AT组合命令解析的信息 */
        At_ResetCombinParseInfo(clientIndex);

        parseContext->mode       = AT_NORMAL_MODE;
        parseContext->dataLen    = 0;
        parseContext->cmdLineLen = 0;

        (VOS_VOID)memset_s(parseContext->dataBuff, sizeof(parseContext->dataBuff), 0x00, sizeof(parseContext->dataBuff));

        if (parseContext->cmdLine != NULL) {
            AT_FREE(parseContext->cmdLine);
            parseContext->cmdLine = VOS_NULL_PTR;
            parseContext->cmdLine = NULL;
        }
    }
}

/*
 * 功能描述: 重置CLIENT PCUI TAB
 */
VOS_VOID AT_ResetClientPcuiTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_PCUI].clientId = AT_CLIENT_ID_PCUI;
    g_atClientTab[AT_CLIENT_ID_PCUI].portNo   = DMS_PORT_PCUI;
    g_atClientTab[AT_CLIENT_ID_PCUI].userType = AT_USBCOM_USER;
    g_atClientTab[AT_CLIENT_ID_PCUI].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT CTRL TAB
 */
VOS_VOID AT_ResetClientCtrlTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_CTRL].clientId = AT_CLIENT_ID_CTRL;
    g_atClientTab[AT_CLIENT_ID_CTRL].portNo   = DMS_PORT_CTRL;
    g_atClientTab[AT_CLIENT_ID_CTRL].userType = AT_CTR_USER;
    g_atClientTab[AT_CLIENT_ID_CTRL].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT PCUI2 TAB
 */
VOS_VOID AT_ResetClientPcui2Tab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_PCUI2].clientId = AT_CLIENT_ID_PCUI2;
    g_atClientTab[AT_CLIENT_ID_PCUI2].portNo   = DMS_PORT_PCUI2;
    g_atClientTab[AT_CLIENT_ID_PCUI2].userType = AT_PCUI2_USER;
    g_atClientTab[AT_CLIENT_ID_PCUI2].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT MODEM TAB
 */
VOS_VOID AT_ResetClientModemTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_MODEM].clientId = AT_CLIENT_ID_MODEM;
    g_atClientTab[AT_CLIENT_ID_MODEM].portNo   = DMS_PORT_MODEM;
    g_atClientTab[AT_CLIENT_ID_MODEM].userType = AT_MODEM_USER;
    g_atClientTab[AT_CLIENT_ID_MODEM].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT NDIS TAB
 */
VOS_VOID AT_ResetClientNdisTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_NDIS].clientId = AT_CLIENT_ID_NDIS;
    g_atClientTab[AT_CLIENT_ID_NDIS].portNo   = DMS_PORT_NCM_CTRL;
    g_atClientTab[AT_CLIENT_ID_NDIS].userType = AT_NDIS_USER;
    g_atClientTab[AT_CLIENT_ID_NDIS].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT UART TAB
 */
VOS_VOID AT_ResetClientUartTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_UART].clientId = AT_CLIENT_ID_UART;
    g_atClientTab[AT_CLIENT_ID_UART].portNo   = DMS_PORT_UART;
    g_atClientTab[AT_CLIENT_ID_UART].userType = AT_UART_USER;
    g_atClientTab[AT_CLIENT_ID_UART].used     = AT_CLIENT_USED;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)
/*
 * 功能描述: 重置CLIENT HSUART TAB
 */
VOS_VOID AT_ResetClientHsUartTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_HSUART].clientId = AT_CLIENT_ID_HSUART;
    g_atClientTab[AT_CLIENT_ID_HSUART].portNo   = DMS_PORT_HSUART;
    g_atClientTab[AT_CLIENT_ID_HSUART].userType = AT_HSUART_USER;
    g_atClientTab[AT_CLIENT_ID_HSUART].used     = AT_CLIENT_USED;
}
#endif

/*
 * 功能描述: 重置CLIENT SOCK TAB
 */
VOS_VOID AT_ResetClientSockTab(VOS_VOID)
{
    g_atClientTab[AT_CLIENT_ID_SOCK].clientId = AT_CLIENT_ID_SOCK;
    g_atClientTab[AT_CLIENT_ID_SOCK].portNo   = DMS_PORT_SOCK;
    g_atClientTab[AT_CLIENT_ID_SOCK].userType = AT_SOCK_USER;
    g_atClientTab[AT_CLIENT_ID_SOCK].used     = AT_CLIENT_USED;
}

/*
 * 功能描述: 重置CLIENT APP TAB
 */
VOS_VOID AT_ResetClientAppTab(VOS_VOID)
{
    VOS_UINT32 loop;
    VOS_UINT32 indexNum;

    for (loop = 0; loop < DMS_APP_PORT_SIZE; loop++) {
        indexNum                         = AT_CLIENT_ID_APP + loop;
        g_atClientTab[indexNum].clientId = AT_CLIENT_ID_APP + (VOS_UINT16)loop;
        g_atClientTab[indexNum].portNo   = DMS_PORT_APP + (VOS_UINT16)loop;
        g_atClientTab[indexNum].userType = AT_APP_USER;
        g_atClientTab[indexNum].used     = AT_CLIENT_USED;
    }
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)
/*
 * 功能描述: 重置CLIENT CMUX TAB
 */
VOS_VOID AT_ResetClientCmuxTab(VOS_VOID)
{
    VOS_UINT32 loop;
    VOS_UINT32 indexNum;

    for (loop = 0; loop < MAX_CMUX_PORT_NUM; loop++) {
        indexNum                         = AT_CLIENT_ID_CMUXAT + loop;
        g_atClientTab[indexNum].clientId = AT_CLIENT_ID_CMUXAT + loop;
        g_atClientTab[indexNum].portNo   = DMS_PORT_CMUXAT + loop;
        g_atClientTab[indexNum].userType = AT_CMUXAT_USER + loop;
        g_atClientTab[indexNum].used     = AT_CLIENT_USED;
    }
}
#endif

VOS_VOID AT_ResetClientTab(VOS_VOID)
{
    VOS_UINT32 clientIndex;

    for (clientIndex = 0; clientIndex < AT_MAX_CLIENT_NUM; clientIndex++) {
        /* 清空对应表项 */
        (VOS_VOID)memset_s(&g_atClientTab[clientIndex], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    }

    /* USB PCUI */
    AT_ResetClientPcuiTab();

    /* USB Control */
    AT_ResetClientCtrlTab();

    /* USB PCUI2 */
    AT_ResetClientPcui2Tab();

    /* USB MODEM */
    AT_ResetClientModemTab();

    /* NDIS MODEM */
    AT_ResetClientNdisTab();

    /* USB UART */
    AT_ResetClientUartTab();

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* HSUART */
    AT_ResetClientHsUartTab();
#endif

    /* SOCK */
    AT_ResetClientSockTab();

    /* APP */
    AT_ResetClientAppTab();

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    /* CMUX */
    AT_ResetClientCmuxTab();
#endif
}

VOS_VOID AT_ResetOtherCtx(VOS_VOID)
{
    /* <CR> */
    g_atS3 = 13;

    g_atCrLf[0] = g_atS3;

    /* <LF> */
    g_atS4 = 10;

    g_atCrLf[1] = g_atS4;

    /* <DEL> */
    g_atS5 = 8;

    /* 指示命令返回码类型 */
    g_atVType = AT_V_ENTIRE_TYPE;

    /* 指示TE编码类型 */
    g_atCscsType = AT_CSCS_IRA_CODE;

    g_atEType = AT_E_ECHO_CMD;

    /*
     * CONNECT <text> result code is given upon entering online data state.
     * Dial tone and busy detection are disabled.
     */
    g_atXType = 0;

    /* 默认LTE上下行category都为6 */
    g_atDlRateCategory.lteUeDlCategory = AT_LTE_CATEGORY_DEFAULT_VALUE;
    g_atDlRateCategory.lteUeUlCategory = AT_LTE_CATEGORY_DEFAULT_VALUE;
}

VOS_VOID AT_ResetMscLevel(DMS_PortIdUint16 portId)
{
    /* 与AT变量保持一致,拉低所有管脚信号 */
    DMS_PORT_DeassertDcd(portId);
    DMS_PORT_DeassertDsr(portId);
    DMS_PORT_DeassertCts(portId);
    DMS_PORT_DeassertRi(portId);
}

#if (FEATURE_ON == FEATURE_AT_HSUART)

VOS_VOID AT_InitUartCtrlInfo(VOS_VOID)
{
    AT_UartCtx *uartCtx = VOS_NULL_PTR;

    uartCtx = AT_GetUartCtxAddr();

    /* 初始化FLOW CTRL默认值 */
    uartCtx->flowCtrl.dceByDte = AT_UART_DEFAULT_FC_DCE_BY_DTE;
    uartCtx->flowCtrl.dteByDce = AT_UART_DEFAULT_FC_DTE_BY_DCE;

    /* 初始化UART LINE CTRL默认值 */
    DMS_PORT_SetDcdMode(DMS_PORT_UART_DEFAULT_DCD_MODE);
    DMS_PORT_SetDtrMode(DMS_PORT_UART_DEFAULT_DTR_MODE);
    DMS_PORT_SetDsrMode(DMS_PORT_UART_DEFAULT_DSR_MODE);
}

VOS_VOID AT_ResetHsuartPppInfo(VOS_UINT8 indexNum)
{
    DMS_PortIdUint16    portId;

    portId   = AT_GetDmsPortIdByClientId(indexNum);

    /* 切换为命令模式 */
    DMS_PORT_ResumeCmdMode(portId);

    /* 清除缓存 */
    DMS_PORT_FlushTxData(portId);

    /* 恢复为低水线 */
    DMS_PORT_SetUartWaterMarkFlg(VOS_FALSE);

    /* 重置客户端信息 */
    AT_HSUART_InitLink(indexNum);

    /* 重置流控、管脚配置模式 */
    AT_InitUartCtrlInfo();

    /* 重置管脚信号 */
    AT_ResetMscLevel(portId);

    /* 重置波特率 帧格式配置 */
    AT_HSUART_ConfigDefaultPara();
}
#endif

VOS_VOID AT_ResetModemPppInfo(VOS_UINT8 indexNum)
{
    DMS_PortIdUint16    portId;

    portId   = AT_GetDmsPortIdByClientId(indexNum);

    /* 切换为命令模式 */
    DMS_PORT_ResumeCmdMode(portId);

    /* 与AT变量保持一致,拉低所有管脚信号 */
    AT_ResetMscLevel(portId);
}

VOS_VOID AT_ResetPppDailInfo(VOS_VOID)
{
#if (FEATURE_ON == FEATURE_AT_HSUART)
    AT_ResetHsuartPppInfo(AT_CLIENT_ID_HSUART);
#endif
    AT_ResetModemPppInfo(AT_CLIENT_ID_MODEM);
}

VOS_UINT32 AT_RcvCcpuResetStartInd(struct MsgCB *msg)
{
    AT_PR_LOGI("enter %u", VOS_GetSlice());

#if (FEATURE_CCORE_RESET_RPT_BY_FILE_NODE != FEATURE_ON)
    /* 上报^RESET:0命令 */
    AT_ReportResetCmd(AT_RESET_REPORT_CAUSE_CCPU_START);
#endif

    /* 停止所有启动的内部定时器 */
    AT_StopAllTimer();

    /* 初始化上下文信息 */
    AT_InitCtx();

    AT_PR_LOGI("nv write begin %u", VOS_GetSlice());

    AT_PR_LOGI("nv write end %u", VOS_GetSlice());

    /* 读取NV项 */
    AT_ReadNV();

    /* 装备初始化 */
    AT_InitDeviceCmd();

    /* STK初始化 */
    AT_InitStk();

    AT_PR_LOGI("nv read end %u", VOS_GetSlice());

    /* AT模块参数的初始化 */
    AT_InitPara();

    /* 重置客户端解析信息 */
    AT_ResetParseCtx();

    /* 重置用户信息 */
    AT_ResetClientTab();

    /* 重置其他散落的全局变量 */
    AT_ResetOtherCtx();

    /* 重置PPP拨号相关信息 */
    AT_ResetPppDailInfo();

    AT_PR_LOGI("set modem state %u", VOS_GetSlice());

    /* C核单独复位后设置设备节点，当前未启动 */
    AT_SetModemState(MODEM_ID_0, PS_FALSE);

    AT_PR_LOGI("leave %u", VOS_GetSlice());

    /* 释放信号量，使得调用API任务继续运行 */
    VOS_SmV(AT_GetResetSem());

    return VOS_OK;
}

VOS_UINT32 AT_RcvCcpuResetEndInd(struct MsgCB *msg)
{
    AT_PR_LOGI("enter %u", VOS_GetSlice());

    /* 设置复位完成的标志 */
    AT_SetResetFlag(VOS_FALSE);

#if (FEATURE_CCORE_RESET_RPT_BY_FILE_NODE != FEATURE_ON)
    /* 上报^RESET:1命令 */
    AT_ReportResetCmd(AT_RESET_REPORT_CAUSE_CCPU_END);
#endif

    AT_PR_LOGI("leave %u", VOS_GetSlice());

    return VOS_OK;
}

VOS_UINT32 AT_RcvHifiResetBeginInd(struct MsgCB *msg)
{
    APP_VC_MsgUint16          msgName = APP_VC_MSG_HIFI_RESET_BEGIN_NTF;
    APP_VC_HifiResetTypeUint8 hifiResetType = APP_VC_HIFI_RESET_FROM_CP;
    AT_PR_LOGI("%u", VOS_GetSlice());

    /* 上报^RESET:2命令 */
    AT_ReportResetCmd(AT_RESET_REPORT_CAUSE_HIFI_BEGIN);

    AT_DBG_SAVE_HIFI_RESET_NUM(1);

    APP_VC_SendHifiResetNtf(I0_WUEPS_PID_VC, hifiResetType, msgName);

#if (MULTI_MODEM_NUMBER == 2)
    APP_VC_SendHifiResetNtf(I1_WUEPS_PID_VC, hifiResetType, msgName);
#endif

#if (MULTI_MODEM_NUMBER == 3)
    APP_VC_SendHifiResetNtf(I1_WUEPS_PID_VC, hifiResetType, msgName);
    APP_VC_SendHifiResetNtf(I2_WUEPS_PID_VC, hifiResetType, msgName);
#endif

    return VOS_OK;
}

VOS_UINT32 AT_RcvHifiResetEndInd(struct MsgCB *msg)
{
    APP_VC_MsgUint16          msgName = APP_VC_MSG_HIFI_RESET_END_NTF;
    APP_VC_HifiResetTypeUint8 hifiResetType = APP_VC_HIFI_RESET_FROM_CP;
    AT_PR_LOGI("%u", VOS_GetSlice());

    /* 上报^RESET:3命令 */
    AT_ReportResetCmd(AT_RESET_REPORT_CAUSE_HIFI_END);
    APP_VC_SendHifiResetNtf(I0_WUEPS_PID_VC, hifiResetType, msgName);
#if (MULTI_MODEM_NUMBER == 2)
    APP_VC_SendHifiResetNtf(I1_WUEPS_PID_VC, hifiResetType, msgName);
#endif
#if (MULTI_MODEM_NUMBER == 3)
    APP_VC_SendHifiResetNtf(I1_WUEPS_PID_VC, hifiResetType, msgName);
    APP_VC_SendHifiResetNtf(I2_WUEPS_PID_VC, hifiResetType, msgName);
#endif

    return VOS_OK;
}

VOS_VOID AT_SpyMsgProc(struct MsgCB *msg)
{
    TEMP_PROTECT_EventAtInd *dataMsg = (TEMP_PROTECT_EventAtInd *)msg;

    switch (dataMsg->msgId) {
#if (FEATURE_LTE == FEATURE_ON)
        case ID_TEMPPRT_AT_EVENT_IND:
            AT_ProcTempprtEventInd(dataMsg);
            break;
#endif

        case ID_TEMPPRT_STATUS_AT_EVENT_IND:
            AT_RcvTempprtStatusInd((struct MsgCB *)dataMsg);
            break;

        default:
            AT_WARN_LOG("AT_SpyMsgProc:WARNING:Wrong Msg!\n");
            break;
    }
}

VOS_UINT32 AT_RcvMmaCbcQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_BatteryCapacityQryCnf *cbcCnf = (TAF_MMA_BatteryCapacityQryCnf *)msg;
    TAF_UINT16                     length   = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        cbcCnf->batteryStatus.batteryPowerStatus);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", cbcCnf->batteryStatus.batteryRemains);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

