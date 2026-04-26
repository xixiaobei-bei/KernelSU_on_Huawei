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
#include "at_mta_interface.h"
#include "at_cmd_msg_proc.h"
#include "at_device_cmd.h"
#include "at_mt_msg_proc.h"
#include "at_phone_as_rslt_proc.h"
#include "at_phone_taf_rslt_proc.h"

#include "at_ltev_taf_rslt_proc.h"

#include "at_general_taf_rslt_proc.h"
#include "at_general_pam_rslt_proc.h"
#include "at_general_lmm_rslt_proc.h"
#include "at_general_event_report.h"

#include "at_device_phy_rslt_proc.h"
#include "at_device_taf_rslt_proc.h"
#include "at_device_as_rslt_proc.h"
#include "at_device_drv_rslt_proc.h"

#include "at_test_as_rslt_proc.h"
#include "at_test_phy_rslt_proc.h"

#include "at_ss_taf_rslt_proc.h"
#include "at_sms_taf_rslt_proc.h"

#include "at_safety_pam_rslt_proc.h"
#include "at_cdma_event_report.h"
#include "at_cdma_rslt_proc.h"

#include "at_custom_phy_rslt_proc.h"
#include "at_custom_taf_rslt_proc.h"
#include "at_custom_l2_rslt_proc.h"
#include "at_custom_as_rslt_proc.h"
#include "at_custom_lnas_rslt_proc.h"
#include "at_custom_event_report.h"
#include "at_custom_nrmm_rslt_proc.h"
#include "at_custom_mm_rslt_proc.h"
#include "at_custom_comm_rslt_proc.h"
#include "at_custom_drv_rslt_proc.h"
#include "at_custom_rcm_rslt_proc.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "at_custom_lrrc_rslt_proc.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PROC_MTA_MAIN_C

/*
 * 功能描述: 通用的设置命令回复处理函数
 */
STATIC VOS_UINT32 AT_RcvMtaCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_CommonSetCnf *event  = (MTA_AT_CommonSetCnf *)(mtaMsg->content);

    g_atSendDataBuff.bufLen = 0;

    return AT_ConvertMtaResult(event->result);
}

STATIC AT_MsgProcEntry g_atProcMtaMsgTbl[] = {
    /* 消息ID, AT模块命令状态, 消息处理函数 */
#if (FEATURE_AGPS == FEATURE_ON && FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    { ID_MTA_AT_CGPSCLOCK_SET_CNF,              AT_CMD_CGPSCLOCK_SET,           AT_RcvMtaCgpsClockSetCnf },
#endif

#if (FEATURE_DSDS == FEATURE_ON)
    { ID_MTA_AT_RRC_PROTECT_PS_CNF,             AT_CMD_PSPROTECTMODE_SET,       AT_RcvMtaCommonSetCnf },
#endif

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)
    { ID_MTA_AT_SET_M2M_FREQLOCK_CNF,           AT_CMD_M2M_FREQLOCK_SET,        AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_QRY_M2M_FREQLOCK_CNF,           AT_CMD_M2M_FREQLOCK_QRY,        AT_RcvMtaQryM2MFreqLockCnf },
#endif

#if (FEATURE_DCXO_HI1102_SAMPLE_SHARE == FEATURE_ON)
    { ID_MTA_AT_DCXO_SAMPLE_QRY_CNF,            AT_CMD_DCXOQRY_QRY,             AT_RcvMtaRcmDcxoSampleQryCnf },
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

    { ID_AT_MTA_SET_LTESARSTUB_CNF,             AT_CMD_LTESARSTUB_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LOW_PWR_MODE_SET_CNF,           AT_CMD_LOWPWRMODE_SET,          AT_RcvMtaCommonSetCnf },
#endif

#if ((FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON))
    { ID_MTA_AT_HSRCELLINFO_IND,                AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaHsrcellInfoInd },
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { ID_MTA_AT_EVDO_REVA_RLINK_INFO_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaEvdoRevARLinkInfoInd },
    { ID_MTA_AT_EVDO_SIG_EXEVENT_IND,           AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaEvdoSigExEventInd },
    { ID_MTA_AT_CDMA_MODEM_CAP_SET_CNF,         AT_CMD_CDMAMODEMSWITCH_SET,     AT_RcvCdmaModemSetCnf },
    { ID_MTA_AT_CDMA_CAP_RESUME_SET_CNF,        AT_CMD_CDMACAPRESUME_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_MEID_SET_CNF,                   AT_CMD_MEID_SET,                AT_RcvMtaMeidSetCnf },
    { ID_MTA_AT_MEID_QRY_CNF,                   AT_CMD_MEID_QRY,                AT_RcvMtaMeidQryCnf },
    { ID_MTA_AT_EVDO_SYS_EVENT_CNF,             AT_CMD_EVDO_SYS_EVENT_SET,      AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_EVDO_SIG_MASK_CNF,              AT_CMD_EVDO_SIG_MASK_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NO_CARD_MODE_SET_CNF,           AT_CMD_NOCARDMODE_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NO_CARD_MODE_QRY_CNF,           AT_CMD_NOCARDMODE_QRY,          AT_RcvMtaNoCardModeQryCnf },
#endif

#if (FEATURE_LTEV == FEATURE_ON)
    { ID_TAF_MTA_VMODE_SET_CNF,                 AT_CMD_VMODE_SET,               AT_RcvMtaVModeSetCnf },
    { ID_TAF_MTA_VMODE_QRY_CNF,                 AT_CMD_VMODE_QUERY,             AT_RcvMtaVModeQryCnf },
#endif

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
    { ID_MTA_AT_MBMS_SERVICE_OPTION_SET_CNF,    AT_CMD_MBMS_SERVICE_OPTION_SET, AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_MBMS_SERVICE_STATE_SET_CNF,     AT_CMD_MBMS_SERVICE_STATE_SET,  AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_MBMS_PREFERENCE_SET_CNF,        AT_CMD_MBMS_PREFERENCE_SET,     AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SIB16_NETWORK_TIME_QRY_CNF, AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY, AT_RcvMtaMBMSSib16NetworkTimeQryCnf },
    { ID_MTA_AT_BSSI_SIGNAL_LEVEL_QRY_CNF, AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY,   AT_RcvMtaMBMSBssiSignalLevelQryCnf },
    { ID_MTA_AT_NETWORK_INFO_QRY_CNF,           AT_CMD_MBMS_NETWORK_INFO_QRY,   AT_RcvMtaMBMSNetworkInfoQryCnf },
    { ID_MTA_AT_EMBMS_STATUS_QRY_CNF,           AT_CMD_EMBMS_STATUS_QRY,        AT_RcvMtaMBMSModemStatusQryCnf },
    { ID_MTA_AT_MBMS_UNSOLICITED_CFG_SET_CNF, AT_CMD_MBMS_UNSOLICITED_CFG_SET,  AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_INTEREST_LIST_SET_CNF,          AT_CMD_MBMS_INTERESTLIST_SET,   AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF, AT_CMD_MBMS_AVL_SERVICE_LIST_QRY, AT_RcvMtaMBMSCmdQryCnf },
    { ID_MTA_AT_MBMS_SERVICE_EVENT_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaMBMSServiceEventInd },
#endif
    { ID_MTA_AT_LTE_CA_CELLEX_INFO_NTF,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLteCaCellExInfoNtf },
    { ID_MTA_AT_LRRC_UE_CAP_PARA_INFO_NTF,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLrrcUeCapNtf },
    { ID_MTA_AT_LTE_LOW_POWER_SET_CNF, AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET,    AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LTE_WIFI_COEX_SET_CNF,          AT_CMD_LTE_WIFI_COEX_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LTE_WIFI_COEX_QRY_CNF,          AT_CMD_LTE_WIFI_COEX_QRY,       AT_RcvMtaIsmCoexQryCnf },
    { ID_MTA_AT_LTE_CA_CFG_SET_CNF,             AT_CMD_LTE_CA_CFG_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LTE_CA_CELLEX_QRY_CNF,          AT_CMD_LTE_CA_CELLEX_QRY,       AT_RcvMtaLteCaCellExQryCnf },
    { ID_MTA_AT_LTE_CA_CELL_RPT_CFG_SET_CNF,    AT_CMD_LCACELLRPTCFG_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LTE_CA_CELL_RPT_CFG_QRY_CNF,    AT_CMD_LCACELLRPTCFG_QRY,       AT_RcvMtaCACellQryCnf },
    { ID_MTA_AT_FINE_TIME_SET_CNF,              AT_CMD_FINE_TIME_SET,           AT_RcvMtaCommonSetCnf },

    /* 使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，主要原因：ID_MTA_AT_LPP_FINE_TIME_NTF为单播消息 */
    { ID_MTA_AT_LPP_FINE_TIME_NTF,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLppFineTimeNtf },
    { ID_MTA_AT_LL2_COM_CFG_SET_CNF,            AT_CMD_LL2COMCFG_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LL2_COM_CFG_QRY_CNF,            AT_CMD_LL2COMCFG_QRY,           AT_RcvMtaLL2ComCfgQryCnf },
    { ID_MTA_AT_LRRC_UE_CAP_PARA_SET_CNF,       AT_CMD_LRRC_UE_CAP_SET,         AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_OVERHEATING_CFG_SET_CNF,        AT_CMD_OVER_HEATING_SET,        AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_OVERHEATING_CFG_QRY_CNF,        AT_CMD_OVER_HEATING_QRY,        AT_RcvMtaOverHeatingQryCnf },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    { ID_MTA_AT_POWER_DET_QRY_CNF,              AT_CMD_FPOWDET_QRY,             AT_RcvMtaPowerDetQryCnf },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_MTA_AT_NPDCP_SLEEP_TRHRES_CFG_CNF,     AT_CMD_NPDCP_SLEEPTHRES_SET,    AT_RcvMtaNPdcpSleepThresCfgCnf },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_MTA_AT_NR_FREQLOCK_SET_CNF,            AT_CMD_NRFREQLOCK_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NR_FREQLOCK_QRY_CNF,            AT_CMD_NRFREQLOCK_QRY,          AT_RcvMtaNrFreqLockQryCnf },
    { ID_MTA_AT_UART_TEST_RSLT_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaUartTestRsltInd },
    { ID_MTA_AT_UART_TEST_CNF,                  AT_CMD_UART_TEST_SET,           AT_RcvMtaCommonSetCnf },
#endif

    { ID_MTA_AT_NRPHY_COM_CFG_SET_CNF,          AT_CMD_NRPHY_COM_CFG_SET,       AT_RcvMtaCommonSetCnf},
    { ID_MTA_AT_NTXPOWER_QRY_CNF,               AT_CMD_NTXPOWER_QRY,            AT_RcvMtaNtxPowerQryCnf },
    { ID_MTA_AT_5G_NSSAI_SET_CNF,               AT_CMD_C5GDFTCFGNSSAI_SET,      AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_5G_NSSAI_SET_CNF,               AT_CMD_C5GPNSSAI_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_5G_NSSAI_QRY_CNF,               AT_CMD_C5GNSSAIRDP_SET,         AT_RcvMta5gNssaiQryCnf },
    { ID_MTA_AT_5G_NSSAI_QRY_CNF,               AT_CMD_C5GPNSSAI_QRY,           AT_RcvMta5gNssaiQryCnf },
    { ID_MTA_AT_5G_NSSAI_QRY_CNF,               AT_CMD_C5GNSSAI_QRY,            AT_RcvMta5gNssaiQryCnf },
#if (FEATURE_LADN == FEATURE_ON)
    { ID_MTA_AT_SET_LADN_RPT_CNF,               AT_CMD_CLADN_SET,               AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_QRY_LADN_INFO_CNF,              AT_CMD_CLADN_QRY,               AT_RcvMtaLadnInfoQryCnf },
    { ID_MTA_AT_LADN_INFO_IND,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLadnInfoInd },
#endif
    { ID_MTA_AT_FAST_RETURN_5G_ENDC_IND,        AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaFastReturn5gEndcInd },
    { ID_MTA_AT_5G_OPTION_SET_CNF,              AT_CMD_5G_OPTION_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_5G_OPTION_QRY_CNF,              AT_CMD_5G_OPTION_QRY,           AT_RcvMta5gOptionQryCnf },
    { ID_MTA_AT_SET_NETMON_SSCELL_CNF,          AT_CMD_MONSSC_SET,              AT_RcvMtaSetNetMonSSCellCnf },
    { ID_MTA_AT_NR_SSB_ID_QRY_CNF,              AT_CMD_NR_SSB_ID_QRY,           AT_RcvMtaNrSsbIdQryCnf },
    { ID_MTA_AT_SET_TRX_TAS_CNF,                AT_CMD_TRX_TAS_SET,             AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_QRY_TRX_TAS_CNF,                AT_CMD_TRX_TAS_QRY,             AT_RcvMtaQryTrxTasCnf },
    { ID_MTA_AT_NL2_COM_CFG_SET_CNF,            AT_CMD_NL2COMCFG_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NL2_COM_CFG_QRY_CNF,            AT_CMD_NL2COMCFG_QRY,           AT_RcvMtaNL2ComCfgQryCnf },
    { ID_MTA_AT_NRRC_UE_CAP_PARA_SET_CNF,       AT_CMD_NRRC_UE_CAP_SET,         AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_LENDC_QRY_CNF,                  AT_CMD_LENDC_QRY,               AT_RcvMtaLendcQryCnf },
    { ID_MTA_AT_NRRCCAP_CFG_SET_CNF,            AT_CMD_NRRCCAPCFG_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NRRCCAP_QRY_CNF,                AT_CMD_NRRCCAPQRY_SET,          AT_RcvMtaNrrcCapQryCnf },
    { ID_MTA_AT_NRPOWERSAVING_CFG_SET_CNF,      AT_CMD_NRPOWERSAVINGCFG_SET,    AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NRPOWERSAVING_QRY_CNF,          AT_CMD_NRPOWERSAVINGQRY_SET,    AT_RcvMtaNrPowerSavingQryCnf },
    { ID_MTA_AT_NRPWRCTRL_SET_CNF,              AT_CMD_NRPWRCTRL_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NR_CA_CELL_INFO_QRY_CNF,        AT_CMD_NRCACELL_QRY,            AT_RcvMtaNrCaCellInfoQryCnf },
    { ID_MTA_AT_NR_CA_CELL_INFO_RPT_CFG_SET_CNF, AT_CMD_NRCACELLRPTCFG_SET,     AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NR_CA_CELL_INFO_RPT_CFG_QRY_CNF, AT_CMD_NRCACELLRPTCFG_QRY,     AT_RcvMtaNrCaCellInfoRptCfgQryCnf },
    { ID_MTA_AT_NR_NW_CAP_INFO_RPT_CFG_SET_CNF, AT_CMD_NRNWCAPRPTCFG_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NR_NW_CAP_INFO_QRY_CNF,         AT_CMD_NRNWCAPQRY_SET,          AT_RcvMtaNrNwCapInfoQryCnf },
    { ID_MTA_AT_NR_NW_CAP_INFO_RPT_CFG_QRY_CNF, AT_CMD_NRNWCAPRPTQRY_SET,       AT_RcvMtaNrNwCapInfoRptCfgQryCnf },
    { ID_MTA_AT_PMU_DIE_SN_QRY_CNF,             AT_CMD_MULTI_PMU_DIE_ID_QRY,    AT_RcvMtaPmuDieSNQryCnf },
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
    { ID_MTA_AT_NW_SLICE_AUTH_CMD_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNwSliceAuthCmdInd },
    { ID_MTA_AT_NW_SLICE_AUTH_RSLT_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNwSliceAuthRsltInd },
    { ID_MTA_AT_PENDING_NSSAI_IND,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaPendingNssaiInd },
#endif
    { ID_MTA_AT_NRRC_UE_CAP_PARA_INFO_NTF,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNrrcUeCapNtf },
    { ID_MTA_AT_LENDC_INFO_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLendcInfoInd },
    { ID_MTA_AT_NR_CA_CELL_INFO_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNrCaCellInfoInd },
    { ID_MTA_AT_NR_NW_CAP_INFO_REPORT_IND,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNrNwCapInfoReportInd },
    { ID_MTA_AT_AUTON2LOPT_CFG_SET_CNF,         AT_CMD_AUTON2LOPTCFG_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_NR_CELL_BAND_CNF,           AT_CMD_NRCELLBAND_SET,          AT_RcvMtaSetNrCellBandCnf },
#endif
    { ID_MTA_AT_PMU_DIE_SN_QRY_CNF,             AT_CMD_PMU_DIE_SN_QRY,          AT_RcvMtaPmuDieSNQryCnf },
    { ID_MTA_AT_CPOS_SET_CNF,                   AT_CMD_CPOS_SET,                AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_WRR_FREQLOCK_SET_CNF,           AT_CMD_FREQLOCK_SET,            AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_WRR_AUTOTEST_QRY_CNF,           AT_CMD_CWAS_QUERY,              AT_RcvMtaWrrAutotestQryCnf },
    { ID_MTA_AT_SIMLOCKUNLOCK_SET_CNF,          AT_CMD_SIMLOCKUNLOCK_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SIMLOCKUNLOCK_SET_CNF,          AT_CMD_CLCK_SIMLOCKUNLOCK,      AT_RcvMtaCommonSetCnf },

    /* 使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，主要原因：ID_MTA_AT_SIB_FINE_TIME_NTF为单播消息 */
    { ID_MTA_AT_SIB_FINE_TIME_NTF,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaSibFineTimeNtf },
    { ID_MTA_AT_QRY_NMR_CNF,                    AT_CMD_CNMR_QUERY,              AT_RcvMtaQryNmrCnf },
    { ID_MTA_AT_WRR_CELLINFO_QRY_CNF,           AT_CMD_CELLINFO_QUERY,          AT_RcvMtaWrrCellinfoQryCnf },
    { ID_MTA_AT_WRR_MEANRPT_QRY_CNF,            AT_CMD_MEANRPT_QUERY,           AT_RcvMtaWrrMeanrptQryCnf },
    { ID_MTA_AT_WRR_RRC_VERSION_SET_CNF,        AT_CMD_RRC_VERSION_SET,         AT_RcvMtaWrrRrcVersionSetCnf },
    { ID_MTA_AT_WRR_CELLSRH_SET_CNF,            AT_CMD_WAS_MNTN_SET_CELLSRH,    AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_WRR_FREQLOCK_QRY_CNF,           AT_CMD_FREQLOCK_QUERY,          AT_RcvMtaWrrFreqLockQryCnf },
    { ID_MTA_AT_WRR_RRC_VERSION_QRY_CNF,        AT_CMD_RRC_VERSION_QUERY,       AT_RcvMtaWrrRrcVersionQryCnf },
    { ID_MTA_AT_WRR_CELLSRH_QRY_CNF,            AT_CMD_WAS_MNTN_QRY_CELLSRH,    AT_RcvMtaWrrCellSrhQryCnf },

    { ID_MTA_AT_GAS_AUTOTEST_QRY_CNF,           AT_CMD_CGAS_QUERY,              AT_RcvMtaGrrAutotestQryCnf },

    { ID_MTA_AT_BODY_SAR_SET_CNF,               AT_CMD_BODYSARON_SET,           AT_RcvMtaBodySarSetCnf },

    { ID_MTA_AT_CURC_QRY_CNF,                   AT_CMD_CURC_READ,               AT_RcvMtaQryCurcCnf },
    { ID_MTA_AT_UNSOLICITED_RPT_SET_CNF,        AT_CMD_UNSOLICITED_RPT_SET,     AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_UNSOLICITED_RPT_QRY_CNF,        AT_CMD_UNSOLICITED_RPT_READ,    AT_RcvMtaQryUnsolicitedRptCnf },

    { ID_MTA_AT_IMEI_VERIFY_QRY_CNF,            AT_CMD_IMEI_VERIFY_READ,        AT_RcvMtaImeiVerifyQryCnf },
    { ID_MTA_AT_CGSN_QRY_CNF,                   AT_CMD_CGSN_READ,               AT_RcvMtaCgsnQryCnf },
    { ID_MTA_AT_NCELL_MONITOR_SET_CNF,          AT_CMD_NCELL_MONITOR_SET,       AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_NCELL_MONITOR_QRY_CNF,          AT_CMD_NCELL_MONITOR_READ,      AT_RcvMtaQryNCellMonitorCnf },

    { ID_MTA_AT_REFCLKFREQ_SET_CNF,             AT_CMD_REFCLKFREQ_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_REFCLKFREQ_QRY_CNF,             AT_CMD_REFCLKFREQ_READ,         AT_RcvMtaRefclkfreqQryCnf },
    { ID_MTA_AT_HANDLEDECT_SET_CNF,             AT_CMD_HANDLEDECT_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_HANDLEDECT_QRY_CNF,             AT_CMD_HANDLEDECT_QRY,          AT_RcvMtaHandleDectQryCnf },

    { ID_MTA_AT_ECID_SET_CNF,                   AT_CMD_ECID_SET,                AT_RcvMtaEcidSetCnf },

    { ID_MTA_AT_MIPICLK_QRY_CNF,                AT_CMD_MIPI_CLK_QRY,            AT_RcvMtaMipiInfoCnf },
    { ID_MTA_AT_SET_DPDTTEST_FLAG_CNF,          AT_CMD_DPDTTEST_SET,            AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_SET_DPDT_VALUE_CNF,             AT_CMD_DPDT_SET,                AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_DPDT_VALUE_CNF,             AT_CMD_TFDPDT_SET,              AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_QRY_DPDT_VALUE_CNF,             AT_CMD_DPDTQRY_SET,             AT_RcvMtaQryDpdtValueCnf },
    { ID_MTA_AT_QRY_DPDT_VALUE_CNF,             AT_CMD_TFDPDTQRY_SET,           AT_RcvMtaQryDpdtValueCnf },

    { ID_MTA_AT_SET_JAM_DETECT_CNF,             AT_CMD_JDSWITCH_SET,            AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_JAM_DETECT_CNF,             AT_CMD_JDCFG_SET,               AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_QRY_JAM_DETECT_CNF,             AT_CMD_JDCFG_READ,              AT_RcvMtaQryJamDetectCfgCnf },
    { ID_MTA_AT_QRY_JAM_DETECT_CNF,             AT_CMD_JDSWITCH_READ,           AT_RcvMtaQryJamDetectSwitchCnf },


    { ID_MTA_AT_SET_GSM_FREQLOCK_CNF,           AT_CMD_GSM_FREQLOCK_SET,        AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_QRY_GSM_FREQLOCK_CNF,           AT_CMD_GSM_FREQLOCK_QRY,        AT_RcvMtaGFreqLockQryCnf },

    { ID_MTA_AT_SET_FR_CNF,                     AT_CMD_FRSTATUS_SET,            AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_TRANSMODE_QRY_CNF,              AT_CMD_TRANSMODE_READ,          AT_RcvMtaTransModeQryCnf },

    { ID_MTA_AT_UE_CENTER_SET_CNF,              AT_CMD_UE_CENTER_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_UE_CENTER_QRY_CNF,              AT_CMD_UE_CENTER_QRY,           AT_RcvMtaUECenterQryCnf },
    { ID_MTA_AT_SET_NETMON_SCELL_CNF,           AT_CMD_MONSC_SET,               AT_RcvMtaSetNetMonSCellCnf },
    { ID_MTA_AT_SET_NETMON_NCELL_CNF,           AT_CMD_MONNC_SET,               AT_RcvMtaSetNetMonNCellCnf },
    { ID_MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF,    AT_CMD_AFCCLKINFO_QRY,          AT_RcvMtaAfcClkInfoCnf },
    { ID_MTA_AT_ANQUERY_QRY_CNF,                AT_CMD_ANQUERY_READ,            AT_RcvMtaAnqueryQryCnf },
    { ID_MTA_AT_CSNR_QRY_CNF,                   AT_CMD_CSNR_QUERY,              AT_RcvMtaCsnrQryCnf },
    { ID_MTA_AT_CSQLVL_QRY_CNF,                 AT_CMD_CSQLVL_SET,              AT_RcvMtaCsqlvlQryCnf },
    { ID_MTA_AT_CSQLVL_QRY_CNF,                 AT_CMD_CSQLVLEXT_SET,           AT_RcvMtaCsqlvlQryCnf },
    { ID_MTA_AT_XCPOSR_SET_CNF,                 AT_CMD_XCPOSR_SET,              AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_XCPOSR_QRY_CNF,                 AT_CMD_XCPOSR_QRY,              AT_RcvMtaQryXcposrCnf },
    { ID_MTA_AT_XCPOSRRPT_SET_CNF,              AT_CMD_XCPOSRRPT_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_XCPOSRRPT_QRY_CNF,              AT_CMD_XCPOSRRPT_QRY,           AT_RcvMtaQryXcposrRptCnf },
    { ID_MTA_AT_CLEAR_HISTORY_FREQ_CNF,         AT_CMD_CLEAR_HISTORY_FREQ,      AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_SENSOR_CNF,                 AT_CMD_SENSOR_SET,              AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_SCREEN_CNF,                 AT_CMD_SCREEN_SET,              AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_FRAT_IGNITION_QRY_CNF,          AT_CMD_FRATIGNITION_QRY,        AT_RcvFratIgnitionQryCnf },
    { ID_MTA_AT_FRAT_IGNITION_SET_CNF,          AT_CMD_FRATIGNITION_SET,        AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SET_MODEM_TIME_CNF,             AT_CMD_MODEM_TIME_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_RX_TEST_MODE_SET_CNF,           AT_CMD_RXTESTMODE_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_BESTFREQ_SET_CNF,               AT_CMD_BESTFREQ_SET,            AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_BESTFREQ_QRY_CNF,               AT_CMD_BESTFREQ_QRY,            AT_RcvMtaQryBestFreqCnf },

    { ID_MTA_AT_RFIC_DIE_ID_QRY_CNF,            AT_CMD_RFIC_DIE_ID_QRY,         AT_RcvMtaRficDieIDQryCnf },

    { ID_MTA_AT_RFFE_DIE_ID_QRY_CNF,            AT_CMD_RFFE_DIE_ID_QRY,         AT_RcvMtaRffeDieIDQryCnf },

    { ID_MTA_AT_PHY_COM_CFG_SET_CNF,            AT_CMD_PHY_COM_CFG_SET,         AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_CRRCONN_SET_CNF,                AT_CMD_CRRCONN_SET,             AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_CRRCONN_QRY_CNF,                AT_CMD_CRRCONN_QRY,             AT_RcvMtaQryCrrconnCnf },
    { ID_MTA_AT_VTRLQUALRPT_SET_CNF,            AT_CMD_VTRLQUALRPT_SET,         AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_MODEM_CAP_UPDATE_CNF,           AT_CMD_MODEM_CAP_UPDATE_SET,    AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_TAS_TEST_CFG_CNF,               AT_CMD_TAS_TEST_SET,            AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_TAS_TEST_QRY_CNF,               AT_CMD_TAS_TEST_QRY,            AT_RcvMtaTasTestQryCnf },

    { ID_MTA_AT_RS_INFO_QRY_CNF,                AT_CMD_RSRP_QRY,                AT_RcvMtaRsInfoQryCnf },
    { ID_MTA_AT_RS_INFO_QRY_CNF,                AT_CMD_RSRQ_QRY,                AT_RcvMtaRsInfoQryCnf },

    { ID_MTA_AT_ERRCCAP_CFG_SET_CNF,            AT_CMD_ERRCCAPCFG_SET,          AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_ERRCCAP_QRY_SET_CNF,            AT_CMD_ERRCCAPQRY_SET,          AT_RcvMtaErrcCapQryCnf },

    { ID_MTA_AT_PSEUCELL_INFO_SET_CNF,          AT_CMD_PSEUCELL_SET,            AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_MIPIREAD_SET_CNF,               AT_CMD_MIPIREAD_SET,            AT_RcvMtaSetMipiReadCnf },

    { ID_MTA_AT_PHYMIPIWRITE_SET_CNF,           AT_CMD_PHYMIPIWRITE_SET,        AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_CHRALARMRLAT_CFG_SET_CNF,       AT_CMD_CHRALARMRLATCFG_SET,     AT_RcvMtaChrAlarmRlatCfgSetCnf },

    { ID_MTA_AT_ECC_CFG_SET_CNF,                AT_CMD_ECCCFG_SET,              AT_RcvMtaEccCfgCnf },
    { ID_MTA_AT_EPDU_SET_CNF,                   AT_CMD_EPDU_SET,                AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_COMM_BOOSTER_SET_CNF,           AT_CMD_COMM_BOOSTER_SET,        AT_RcvMtaCommBoosterSetCnf },
    { ID_MTA_AT_COMM_BOOSTER_QUERY_CNF,         AT_CMD_COMM_BOOSTER_QRY,        AT_RcvMtaCommBoosterQueryCnf },

    { ID_MTA_AT_NVLOAD_SET_CNF,                 AT_CMD_NVLOAD_SET,              AT_RcvMtaNvLoadSetCnf },

    { ID_MTA_AT_SMS_DOMAIN_SET_CNF,             AT_CMD_SMSDOMAIN_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SMS_DOMAIN_QRY_CNF,             AT_CMD_SMSDOMAIN_QRY,           AT_RcvMtaSmsDomainQryCnf },

    { ID_MTA_AT_SET_SAMPLE_CNF,                 AT_CMD_SAMPLE_SET,              AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_GPS_LOCSET_SET_CNF,             AT_CMD_GPSLOCSET_SET,           AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_CCLK_QRY_CNF,                   AT_CMD_CCLK_QRY,                AT_RcvMtaCclkQryCnf },

    { ID_AT_MTA_GAME_MODE_SET_CNF,              AT_CMD_GAME_MODE_SET,           AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_NV_REFRESH_SET_CNF,             AT_CMD_NVREFRESH_SET,           AT_RcvMtaNvRefreshSetCnf },

    { ID_MTA_AT_PSEUDBTS_SET_CNF,               AT_CMD_PSEUDBTS_SET,            AT_RcvMtaPseudBtsSetCnf },

    { ID_MTA_AT_SUBCLFSPARAM_SET_CNF,           AT_CMD_SUBCLFSPARAM_SET,        AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_SUBCLFSPARAM_QRY_CNF,           AT_CMD_SUBCLFSPARAM_QRY,        AT_RcvMtaSubClfSparamQryCnf },

    { ID_MTA_AT_FORCESYNC_SET_CNF,              AT_CMD_FORCESYNC_SET,           AT_RcvMtaCommonSetCnf },
    { ID_AT_MTA_SET_LTEPWRDISS_CNF,             AT_CMD_LTEPWRDISS_SET,          AT_RcvMtaCommonSetCnf },

    { ID_MTA_AT_QUICK_CARD_SWITCH_CNF,          AT_CMD_QUICK_CARD_SWITCH_SET,   AT_RcvMtaCommonSetCnf },
    { ID_MTA_AT_TXPOWER_QRY_CNF,                AT_CMD_TXPOWER_QRY,             AT_RcvMtaTxPowerQryCnf },

    { ID_MTA_AT_MCS_SET_CNF,                    AT_CMD_MCS_SET,                 AT_RcvMtaMcsSetCnf },
    { ID_MTA_AT_HFREQINFO_QRY_CNF,              AT_CMD_HFREQINFO_QRY,           AT_RcvMtaHfreqinfoQryCnf },

    { ID_MTA_AT_SFEATURE_QRY_CNF,               AT_CMD_SFEATURE_QRY,            AT_RcvMtaQrySfeatureRsp },

    { ID_MTA_AT_AFC_CLK_UNLOCK_CAUSE_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaAfcClkUnlockCauseInd },
    { ID_MTA_AT_BEST_FREQ_INFO_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaBestFreqInfoInd },
    { ID_MTA_AT_CRRCONN_STATUS_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaCrrconnStatusInd },
    { ID_MTA_AT_RL_QUALITY_INFO_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaRlQualityInfoInd },
    { ID_MTA_AT_VIDEO_DIAG_INFO_RPT,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaVideoDiagInfoRpt },
    { ID_MTA_AT_ACCESS_STRATUM_REL_IND,         AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaAccessStratumRelInd },
    { ID_MTA_AT_ECC_STATUS_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaEccStatusInd },
    { ID_MTA_AT_EPDUR_DATA_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaEpduDataInd },
    { ID_MTA_AT_PHY_COMM_ACK_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaPhyCommAckInd },
    { ID_MTA_AT_COMM_BOOSTER_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaCommBoosterInd },
    { ID_MTA_AT_TEMP_PROTECT_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaTempProtectInd },
    { ID_MTA_AT_PSEUD_BTS_IDENT_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaPseudBtsIdentInd },
    { ID_MTA_AT_HFREQINFO_IND,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaHfreqinfoInd },
    { ID_MTA_AT_SIB16_TIME_UPDATE_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaSib16TimeUpdateInd },
    { ID_MTA_AT_NV_REFRESH_NTF,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNvRefreshNtf },
    { ID_MTA_AT_LTE_CATEGORY_INFO_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaLteCategoryInfoInd },
    { ID_MTA_AT_CPOSR_IND,                      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaCposrInd },
    { ID_MTA_AT_XCPOSRRPT_IND,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaXcposrRptInd },
    { ID_MTA_AT_NCELL_MONITOR_IND,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaNCellMonitorInd },
    { ID_MTA_AT_REFCLKFREQ_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaRefclkfreqInd },
    { ID_MTA_AT_PS_TRANSFER_IND,                AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaPsTransferInd },
    { ID_MTA_AT_MIPICLK_INFO_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaMipiInfoInd },
    { ID_MTA_AT_JAM_DETECT_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaJamDetectInd },
    { ID_MTA_AT_XPASS_INFO_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMtaXpassInfoInd },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { ID_MTA_AT_QRY_TDD_SUBFRAME_CNF,           AT_CMD_TDD_SUBFRAME_QRY,        AT_RcvMtaTddSubframeQryCnf },
#endif
};

STATIC CONST AT_MsgProcEntryTblInfo g_atProcMtaMsgTblInfo = {
    .tbl      = g_atProcMtaMsgTbl,
    .entryNum = AT_ARRAY_SIZE(g_atProcMtaMsgTbl),
    .entryLen = sizeof(g_atProcMtaMsgTbl[0])
};

STATIC VOS_UINT32 AT_GetMtaUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(mtaMsg->appCtrl.clientId, indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("AT_GetMtaUserId: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }
    return VOS_OK;
}

STATIC CONST AT_ModuleMsgProcInfo g_atMtaMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_MTA,
    .tblInfo    = &g_atProcMtaMsgTblInfo,
    .GetMsgId   = VOS_NULL_PTR,
    .GetUserId  = AT_GetMtaUserId,
};

VOS_VOID AT_InitMtaMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atMtaMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitMtaMsgProcTbl: register fail");
    }
}

/*
 * 功能描述: 来自MTA模块的消息处理函数
 */
VOS_VOID AT_ProcMtaMsg(struct MsgCB *msg)
{
    VOS_UINT32 ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_MTA);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_ProcMtaMsg: fail");
    }
}

