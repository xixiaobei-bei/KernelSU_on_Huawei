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

#include "at_custom_mm_rslt_proc.h"
#include "at_cdma_event_report.h"
#include "at_cdma_rslt_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_custom_event_report.h"
#include "at_custom_ims_rslt_proc.h"
#include "at_custom_nrmm_rslt_proc.h"
#include "at_custom_taf_rslt_proc.h"
#include "at_data_mm_rslt_proc.h"
#include "at_device_taf_rslt_proc.h"
#include "at_event_report.h"
#include "at_general_event_report.h"
#include "at_general_mm_rslt_proc.h"
#include "at_phone_event_report.h"
#include "at_phone_ims_rslt_proc.h"
#include "at_phone_mm_rslt_proc.h"
#include "at_phone_taf_rslt_proc.h"
#include "at_sim_event_report.h"
#include "at_sim_pam_rslt_proc.h"
#include "at_sms_taf_rslt_proc.h"
#include "at_voice_event_report.h"
#include "at_voice_ims_rslt_proc.h"
#include "at_voice_mm_rslt_proc.h"
#include "at_voice_taf_rslt_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PROC_MMA_MAIN_C

/*
 * 功能描述: 通用的设置命令回复处理函数
 */
STATIC VOS_UINT32 AT_RcvMmaCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CommonSetCnf   *cnfMsg = (TAF_MMA_CommonSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (cnfMsg->errorCause == TAF_ERR_NO_ERROR) {
        return AT_OK;
    } else {
        return At_ChgTafErrorCode(indexNum, cnfMsg->errorCause);
    }
}

STATIC VOS_UINT32 AT_RcvMmaType2CommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CommonSetCnf   *cnfMsg = (TAF_MMA_CommonSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (cnfMsg->errorCause == TAF_ERR_NO_ERROR) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

STATIC VOS_UINT32 AT_RcvMmaCommonAbortCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = 0;
    return AT_ABORT;
}

STATIC AT_MsgProcEntry g_atProcMmaMsgTbl[] = {
    /* ！！！！注意！！！！
     * 以下为使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，
     * 主要原因：原消息处理函数中没有针对AT的cmdCurrentOpt做拦截，本次整改继承原有逻辑
     * 其他场景响应消息需要使用对应AT的cmdCurrentOpt做拦截
     *
     * ID_TAF_MMA_PHONE_MODE_SET_CNF：
     * 存在收到ID_TAF_MMA_PHONE_MODE_SET_CNF消息前，AT的cmdCurrentOpt被清除的场景
     *（关机时pdp去激活，会清除该opt）
     *
     * ID_TAF_MMA_SYS_CFG_SET_CNF：
     * 存在收到ID_TAF_MMA_SYS_CFG_SET_CNF消息前，AT的cmdCurrentOpt被清除的场景
     *
     * ID_TAF_MMA_DETACH_CNF：
     * 识别到的opt有：AT_CMD_CGATT_DETAACH_SET、AT_CMD_CGCATT_PS_DETAACH_SET、AT_CMD_CGCATT_CS_DETAACH_SET、
     * AT_CMD_CGCATT_PS_CS_DETAACH_SET和AT_CMD_COPS_SET_DEREGISTER，当前不明确是否还有其他回ID_TAF_MMA_DETACH_CNF的点，
     *
     * ID_TAF_MMA_ATTACH_CNF：
     * 识别到的opt有：AT_CMD_CGATT_ATTACH_SET、AT_CMD_CGCATT_PS_CS_ATTACH_SET、AT_CMD_CGCATT_CS_ATTACH_SET
     * 和AT_CMD_CGCATT_PS_ATTACH_SET，当前不明确是否还有其他回ID_TAF_MMA_ATTACH_CNF的点，
     *
     * ID_TAF_MMA_NET_SCAN_CNF：
     * netscan比较特殊，下发请求后，可能会被打断，打断后，AT的current opt会被清除，无法准确判断opt
     *
     * ID_TAF_MMA_CMM_SET_CNF:
     * 继承原有处理
     */
    { ID_TAF_MMA_PHONE_MODE_SET_CNF,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaPhoneModeSetCnf },
    { ID_TAF_MMA_SYS_CFG_SET_CNF,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaSysCfgSetCnf },
    { ID_TAF_MMA_DETACH_CNF,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaDetachCnf },
    { ID_TAF_MMA_ATTACH_CNF,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaAttachCnf },
    { ID_TAF_MMA_NET_SCAN_CNF,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaNetScanCnf },
    { ID_TAF_MMA_CMM_SET_CNF,                   AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCmmSetCmdRsp },

    /* 无特性宏控制的设置请求响应 */
    { ID_TAF_MMA_EOPLMN_SET_CNF,                AT_CMD_EOPLMN_SET,              AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_QUIT_CALLBACK_SET_CNF,         AT_CMD_QCCB_SET,                AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_CFPLMN_SET_CNF,                AT_CMD_CFPLMN_SET,              AT_RcvMmaCFPlmnSetCnf },
    { ID_TAF_MMA_PREF_PLMN_SET_CNF,             AT_CMD_CPOL_SET,                AT_RcvMmaPrefPlmnSetCnf },
    { ID_TAF_MMA_PLMN_SEARCH_CNF,               AT_CMD_COPS_SET_AUTOMATIC,      AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_PLMN_SEARCH_CNF,               AT_CMD_COPS_SET_MANUAL,         AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_PLMN_SEARCH_CNF,               AT_CMD_COPS_SET_MANUAL_AUTOMATIC_MANUAL, AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_PLMN_SEARCH_CNF,               AT_CMD_PLMNSRCH_SET,            AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_PREF_PLMN_TYPE_SET_CNF,        AT_CMD_MMA_SET_PREF_PLMN_TYPE,  AT_RcvMmaPrefPlmnTypeSetCnf },
    { ID_TAF_MMA_CERSSI_SET_CNF,                AT_CMD_CERSSI_SET,              AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_QUICKSTART_SET_CNF,            AT_CMD_CQST_SET,                AT_RcvTafMmaQuickStartSetCnf },
    { ID_TAF_MMA_AUTO_ATTACH_SET_CNF,           AT_CMD_CAATT_SET,               AT_RcvTafMmaAutoAttachSetCnf },
    { ID_TAF_MMA_DPLMN_SET_CNF,                 AT_CMD_DPLMNLIST_SET,           AT_RcvMmaDplmnSetCnf },
    { ID_TAF_MMA_BORDER_INFO_SET_CNF,           AT_CMD_BORDERINFO_SET,          AT_RcvMmaBorderInfoSetCnf },
    { ID_TAF_MMA_EFLOCIINFO_SET_CNF,            AT_CMD_EFLOCIINFO_SET,          AT_RcvMmaEflociInfoSetCnf },
    { ID_TAF_MMA_EFPSLOCIINFO_SET_CNF,          AT_CMD_EFPSLOCIINFO_SET,        AT_RcvMmaPsEflociInfoSetCnf },
    { ID_TAF_MMA_CIND_SET_CNF,                  AT_CMD_CIND_SET,                AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_CEMODE_SET_CNF,                AT_CMD_CEMODE_SET,              AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_EMRSSICFG_SET_CNF,             AT_CMD_EMRSSICFG_SET,           AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_EMRSSIRPT_SET_CNF,             AT_CMD_EMRSSIRPT_SET,           AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_ULFREQRPT_SET_CNF,             AT_CMD_ULFREQRPT_SET,           AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_SMS_ANTI_ATTACK_SET_CNF,       AT_CMD_SMSANTIATTACK_SET,       AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_PS_SCENE_SET_CNF,              AT_CMD_PSSCENE_SET,             AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_EXCHANGE_MODEM_INFO_CNF,       AT_CMD_EXCHANGE_MODEM_INFO_SET, AT_RcvMmaExchangeModemInfoCnf },

    /* 无特性宏控制的查询请求响应 */
    { ID_TAF_MMA_SIMLOCK_STAUS_QUERY_CNF,       AT_CMD_SIMLOCKSTATUS_READ,      AT_RcvSimLockQryRsp },
    { ID_TAF_MMA_SIMLOCK_STAUS_QUERY_CNF,       AT_CMD_CSDFLT_READ,             AT_RcvSimLockQryRsp },
    { ID_TAF_MMA_AC_INFO_QRY_CNF,               AT_CMD_ACINFO_READ,             AT_RcvMmaAcInfoQueryCnf },
    { ID_TAF_MMA_USER_SRV_STATE_QRY_CNF,        AT_CMD_USER_SRV_STATE_READ,     AT_RcvMmaUserSrvStateQryCnf },
    { ID_TAF_MMA_POWER_ON_AND_REG_TIME_QRY_CNF, AT_CMD_REGISTER_TIME_READ,      AT_RcvMmaPwrOnAndRegTimeQryCnf },
    { ID_TAF_MMA_SPN_QRY_CNF,                   AT_CMD_SPN_QUERY,               AT_RcvMmaSpnQryCnf },
    { ID_TAF_MMA_MMPLMNINFO_QRY_CNF,            AT_CMD_MMPLMNINFO_QRY,          AT_RcvMmaMMPlmnInfoQryCnf },
    { ID_TAF_MMA_LAST_CAMP_PLMN_QRY_CNF,        AT_CMD_PLMN_QRY,                AT_RcvMmaPlmnQryCnf },
    { ID_TAF_MMA_LAST_CAMP_PLMN_QRY_CNF,        AT_CMD_TIMEQRY_QRY,             AT_RcvMmaPlmnQryCnf },
    { ID_TAF_MMA_EOPLMN_QRY_CNF,                AT_CMD_EOPLMN_QRY,              AT_RcvMmaEOPlmnQryCnf },
    { ID_TAF_MMA_COPN_INFO_QRY_CNF,             AT_CMD_COPN_QRY,                AT_RcvMmaCopnInfoQueryCnf },
    { ID_TAF_MMA_ATTACH_STATUS_QRY_CNF,         AT_CMD_MMA_GET_CURRENT_ATTACH_STATUS, AT_RcvMmaAttachStatusQryCnf },
    { ID_TAF_MMA_EMC_CALL_BACK_QRY_CNF,         AT_CMD_EMCCBM_QRY,              AT_RcvMmaQryEmcCallBackCnf },
    { ID_TAF_MMA_CFPLMN_QUERY_CNF,              AT_CMD_CFPLMN_READ,             AT_RcvMmaCFPlmnQueryCnf },
    { ID_TAF_MMA_PREF_PLMN_QUERY_CNF,           AT_CMD_CPOL_READ,               AT_RcvMmaPrefPlmnQueryCnf }, /* 需要重点测试 */
    { ID_TAF_MMA_LOCATION_INFO_QRY_CNF,         AT_CMD_LOCINFO_READ,            AT_RcvMmaLocInfoQueryCnf },
    { ID_TAF_MMA_CIPHER_QRY_CNF,                AT_CMD_CIPERQRY_READ,           AT_RcvMmaCipherInfoQueryCnf },
    { ID_TAF_MMA_PHONE_MODE_QRY_CNF,            AT_CMD_CFUN_READ,               AT_RcvMmaPhoneModeQryCnf },
    { ID_TAF_MMA_CERSSI_QRY_CNF,                AT_CMD_CERSSI_READ,             AT_RcvMmaCerssiInfoQueryCnf },
    { ID_TAF_MMA_CERSSI_QRY_CNF,                AT_CMD_CESQ_SET,                AT_RcvMmaCerssiInfoQueryCnf },
    { ID_TAF_MMA_ACCESS_MODE_QRY_CNF,           AT_CMD_CPAM_READ,               AT_RcvMmaAccessModeQryCnf },
    { ID_TAF_MMA_COPS_QRY_CNF,                  AT_CMD_COPS_READ,               AT_RcvMmaCopsQryCnf },
    { ID_TAF_MMA_REG_STATE_QRY_CNF,             AT_CMD_CREG_READ,               At_QryParaRspCregProc },
    { ID_TAF_MMA_REG_STATE_QRY_CNF,             AT_CMD_CGREG_READ,              At_QryParaRspCgregProc },
    { ID_TAF_MMA_REG_STATE_QRY_CNF,             AT_CMD_CEREG_READ,              AT_QryParaRspCeregProc },
    { ID_TAF_MMA_AUTO_ATTACH_QRY_CNF,           AT_CMD_CAATT_READ,              AT_RcvMmaAutoAttachQryCnf },
    { ID_TAF_MMA_SYSINFO_QRY_CNF,               AT_CMD_SYSINFO_READ,            At_QryParaRspSysinfoProc },
    { ID_TAF_MMA_SYSINFO_QRY_CNF,               AT_CMD_SYSINFOEX_READ,          AT_QryParaRspSysinfoExProc },
    { ID_TAF_MMA_HOME_PLMN_QRY_CNF,             AT_CMD_HOMEPLMN_READ,           AT_RcvMmaApHplmnQryCnf },
    { ID_TAF_MMA_HOME_PLMN_QRY_CNF,             AT_CMD_EHPLMN_LIST_QRY,         AT_RcvMmaHplmnQryCnf },
    { ID_TAF_MMA_SYSCFG_QRY_CNF,                AT_CMD_SYSCFG_READ,             AT_RcvTafMmaSyscfgQryCnf },
    { ID_TAF_MMA_SYSCFG_QRY_CNF,                AT_CMD_SYSCFGEX_READ,           AT_RcvTafMmaSyscfgQryCnf },
    { ID_TAF_MMA_SYSCFG_QRY_CNF,                AT_CMD_WS46_QRY,                AT_RcvTafMmaSyscfgQryCnf },
    { ID_TAF_MMA_CRPN_QRY_CNF,                  AT_CMD_CRPN_QUERY,              AT_RcvTafMmaCrpnQryCnf },
    { ID_TAF_MMA_QUICKSTART_QRY_CNF,            AT_CMD_CQST_READ,               AT_RcvTafMmaQuickStartQryCnf },
    { ID_TAF_MMA_CSQ_QRY_CNF,                   AT_CMD_CSQ_SET,                 AT_RcvTafMmaCsqQryCnf },
    { ID_TAF_MMA_BATTERY_CAPACITY_QRY_CNF,      AT_CMD_CBC_SET,                 AT_RcvMmaCbcQryCnf },
    { ID_TAF_MMA_HAND_SHAKE_QRY_CNF,            AT_CMD_HS_READ,                 AT_RcvMmaHsQryCnf },
    { ID_TAF_MMA_DPLMN_QRY_CNF,                 AT_CMD_DPLMNLIST_QRY,           AT_RcvMmaDplmnQryCnf },
    { ID_TAF_MMA_BORDER_INFO_QRY_CNF,           AT_CMD_BORDERINFO_QRY,          AT_RcvMmaBorderInfoQryCnf },
    { ID_TAF_MMA_EFLOCIINFO_QRY_CNF,            AT_CMD_EFLOCIINFO_QRY,          AT_RcvMmaEflociInfoQryCnf },
    { ID_TAF_MMA_EFPSLOCIINFO_QRY_CNF,          AT_CMD_EFPSLOCIINFO_QRY,        AT_RcvMmaPsEflociInfoQryCnf },
    { ID_TAF_MMA_PACSP_QRY_CNF,                 AT_CMD_PACSP_QRY,               AT_RcvMmaPacspQryCnf },
    { ID_TAF_MMA_CEMODE_QRY_CNF,                AT_CMD_CEMODE_READ,             AT_RcvMmaCemodeQryCnf },
    { ID_TAF_MMA_REJINFO_QRY_CNF,               AT_CMD_REJINFO_QRY,             AT_RcvMmaRejinfoQryCnf },
    { ID_TAF_MMA_EMRSSICFG_QRY_CNF,             AT_CMD_EMRSSICFG_QRY,           AT_RcvMmaEmRssiCfgQryCnf },
    { ID_TAF_MMA_EMRSSIRPT_QRY_CNF,             AT_CMD_EMRSSIRPT_QRY,           AT_RcvMmaEmRssiRptQryCnf },
    { ID_TAF_MMA_ULFREQRPT_QRY_CNF,             AT_CMD_ULFREQRPT_QRY,           AT_RcvMmaUlFreqRptQryCnf },
    { ID_TAF_MMA_SMS_ANTI_ATTACK_CAP_QRY_CNF,   AT_CMD_SMSANTIATTACKCAP_QRY,    AT_RcvSmsAntiAttackCapQryCnf },
    { ID_TAF_MMA_NAS_SMS_CAP_QRY_CNF,           AT_CMD_SMSNASCAP_QRY,           AT_RcvMmaSmsNasCapQryCnf },
    { ID_TAF_MMA_PS_SCENE_QRY_CNF,              AT_CMD_PSSCENE_QRY,             AT_RcvMmaPsSceneQryCnf },
    { ID_TAF_MMA_CAR_OOS_GEARS_QRY_CNF,         AT_CMD_CAROOSGEARS_QRY,         AT_RcvMmaCarOosGearsQryCnf },
    { ID_TAF_MMA_RRCSTAT_QRY_CNF,               AT_CMD_RRCSTAT_QRY,             AT_RcvMmaRrcStatQryCnf },
    { ID_TAF_MSG_MMA_EONS_UCS2_CNF,             AT_CMD_EONSUCS2_QRY,            AT_RcvMmaEonsUcs2Cnf },
    { ID_TAF_MMA_PLMN_LIST_CNF,                 AT_CMD_COPS_TEST,               At_QryParaPlmnListProc },

    /* 无特性宏控制的主动上报 */
    { ID_TAF_MMA_USIM_STATUS_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvAtMmaUsimStatusInd },
    { ID_TAF_MMA_AC_INFO_CHANGE_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaAcInfoChangeInd },
    { ID_TAF_MMA_EMC_CALL_BACK_NTF,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaEmcCallBackNtf },
    { ID_TAF_MMA_TIME_CHANGE_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaTimeChangeInd },
    { ID_TAF_MMA_MODE_CHANGE_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaModeChangeInd },
    { ID_TAF_MMA_PLMN_CHANGE_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaPlmnChangeInd },
    { ID_TAF_MMA_SRV_STATUS_IND,                AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaSrvStatusInd },
    { ID_TAF_MMA_RSSI_INFO_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaRssiInfoInd },
    { ID_TAF_MMA_REG_STATUS_IND,                AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaRegStatusInd },
    { ID_TAF_MMA_REG_REJ_INFO_IND,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaRegRejInfoInd },
    { ID_TAF_MMA_PLMN_SELECTION_INFO_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaPlmnSelectInfoInd },
    { ID_TAF_MMA_IMSI_REFRESH_IND,              AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaImsiRefreshInd },
    { ID_TAF_MMA_MT_REATTACH_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaMtReattachInd },
    { ID_TAF_MMA_EMRSSIRPT_IND,                 AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaEmRssiRptInd },
    { ID_TAF_MMA_ELEVATOR_STATE_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaElevatorStateInd },
    { ID_TAF_MMA_ULFREQ_CHANGE_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaUlFreqChangeInd },
    { ID_TAF_MMA_PLMN_DETECT_IND,               AT_CMD_OPT_NO_NEED_CHECK,       At_PlmnDetectIndProc },
    { ID_TAF_MMA_SRCHED_PLMN_INFO_IND,          AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaSrchedPlmnInfoInd },
    { ID_TAF_MMA_INIT_LOC_INFO_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaInitLocInfoInd },
    { ID_TAF_MMA_PSEUD_BTS_IDENT_IND,           AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaPseudBtsIdentInd },
    { ID_TAF_MMA_RRCSTAT_IND,                   AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaRrcStatInd },

    /* 无特性宏控制的测试和打断 */
    { ID_TAF_MMA_PREF_PLMN_TEST_CNF,            AT_CMD_CPOL_TEST,               AT_RcvMmaPrefPlmnTestCnf },
    { ID_TAF_MMA_PLMN_LIST_ABORT_CNF,           AT_CMD_COPS_ABORT_PLMN_LIST,    AT_RcvMmaCommonAbortCnf },
    { ID_TAF_MMA_SYSCFG_TEST_CNF,               AT_CMD_SYSCFG_TEST,             AT_RcvTafMmaSyscfgTestCnf },
    { ID_TAF_MMA_SYSCFG_TEST_CNF,               AT_CMD_SYSCFGEX_TEST,           AT_RcvTafMmaSyscfgTestCnf },

    /* 以下为特性宏控制部分 */
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_TAF_MMA_MT_POWER_DOWN_CNF,             AT_CMD_MMA_MT_POWER_DOWN,       AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_SIM_INSERT_CNF,                AT_CMD_SIMINSERT_SET,           AT_RcvMmaType2CommonSetCnf },
#endif

#if (FEATURE_CSG == FEATURE_ON)
    { ID_TAF_MMA_CSG_LIST_SEARCH_CNF,           AT_CMD_CSG_LIST_SEARCH,         AT_RcvMmaCsgListSearchCnfProc },
    { ID_TAF_MMA_CSG_LIST_ABORT_CNF,            AT_CMD_ABORT_CSG_LIST_SEARCH,   AT_RcvMmaCommonAbortCnf },
    { ID_TAF_MMA_CSG_SPEC_SEARCH_CNF,           AT_CMD_CSG_SPEC_SEARCH,         AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_QRY_CAMP_CSG_ID_INFO_CNF,      AT_CMD_CSG_ID_INFO_QRY,         AT_RcvMmaQryCampCsgIdInfoCnfProc },
#endif

#if (FEATURE_IMS == FEATURE_ON)
    { ID_TAF_MMA_IMS_SWITCH_SET_CNF,            AT_CMD_GUL_IMS_SWITCH_SET,      AT_RcvMmaImsSwitchSetCnf },
    { ID_TAF_MMA_IMS_SWITCH_SET_CNF,            AT_CMD_RCSSWITCH_SET,           AT_RcvMmaImsSwitchSetCnf },
    { ID_TAF_MMA_VOICE_DOMAIN_SET_CNF,          AT_CMD_VOICE_DOMAIN_SET,        AT_RcvMmaVoiceDomainSetCnf },
    { ID_TAF_MMA_IMS_DOMAIN_CFG_SET_CNF,        AT_CMD_IMSDOMAINCFG_SET,        AT_RcvMmaImsDomainCfgSetCnf },
    { ID_TAF_MMA_ROAM_IMS_SUPPORT_SET_CNF,      AT_CMD_ROAM_IMS_SET,            AT_RcvMmaRoamImsSupportSetCnf },
    { ID_TAF_MMA_IMS_VIDEO_CALL_CAP_SET_CNF,    AT_CMD_IMSVTCAPCFG_SET,         AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_IMS_SMS_CFG_SET_CNF,           AT_CMD_IMSSMSCFG_SET,           AT_RcvMmaCommonSetCnf },

    { ID_TAF_MMA_IMS_SWITCH_QRY_CNF,            AT_CMD_GUL_IMS_SWITCH_QRY,      AT_RcvMmaImsSwitchQryCnf },
    { ID_TAF_MMA_VOICE_DOMAIN_QRY_CNF,          AT_CMD_VOICE_DOMAIN_QRY,        AT_RcvMmaVoiceDomainQryCnf },
    { ID_TAF_MMA_IMS_DOMAIN_CFG_QRY_CNF,        AT_CMD_IMSDOMAINCFG_QRY,        AT_RcvMmaImsDomainCfgQryCnf },
    { ID_TAF_MMA_IMS_SMS_CFG_QRY_CNF,           AT_CMD_IMSSMSCFG_QRY,           AT_RcvMmaImsSmsCfgQryCnf },
    { ID_TAF_MMA_RCS_SWITCH_QRY_CNF,            AT_CMD_RCSSWITCH_QRY,           AT_RcvMmaRcsSwitchQryCnf },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_TAF_MMA_IMS_SWITCH_SET_CNF,            AT_CMD_NR_IMS_SWITCH_SET,       AT_RcvMmaImsSwitchSetCnf },
    { ID_TAF_MMA_IMS_SWITCH_QRY_CNF,            AT_CMD_NR_IMS_SWITCH_QRY,       AT_RcvMmaImsSwitchQryCnf },
#endif
#endif

#if (FEATURE_DSDS == FEATURE_ON)
    { ID_TAF_MMA_DSDS_STATE_SET_CNF,            AT_CMD_DSDS_STATE_SET,          AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_DSDS_STATE_NOTIFY,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaDsdsStateNotify },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_TAF_MMA_CERSSI_QRY_CNF,                AT_CMD_CSERSSI_READ,            AT_RcvMmaCerssiInfoQueryCnf },
    { ID_TAF_MMA_NR_BAND_BLOCK_LIST_SET_CNF,    AT_CMD_NR_BAND_BLOCKLIST_SET,   AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_NR_BAND_BLOCK_LIST_QRY_CNF,    AT_CMD_NR_BAND_BLOCKLIST_QRY,   AT_RcvMmaNrBandBlockListQryCnf },
    { ID_TAF_MMA_REG_STATE_QRY_CNF,             AT_CMD_C5GREG_READ,             AT_QryParaRspC5gregProc },
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    /* ！！！！注意！！！！
     * 以下为使用AT_CMD_OPT_NO_NEED_CHECK做opt校验的响应消息，
     * 主要原因：原消息处理函数中没有针对AT的cmdCurrentOpt做拦截，本次整改继承原有逻辑
     * 其他场景响应消息需要使用对应AT的cmdCurrentOpt做拦截
     */
    { ID_TAF_MMA_CDMACSQ_SET_CNF,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCdmaCsqSetCnf },
    { ID_MMA_TAF_HDR_CSQ_SET_CNF,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaHdrCsqSetCnf },

    { ID_TAF_MMA_1XCHAN_SET_CNF,                AT_CMD_1XCHAN_SET,              AT_RcvMmaType2CommonSetCnf },
    { ID_TAF_MMA_CL_DBDOMAIN_STATUSE_SET_CNF,   AT_CMD_CLDBDOMAINSTATUS_SET,    AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_CTCC_OOS_COUNT_SET_CNF,        AT_CMD_CTOOSCOUNT_SET,          AT_RcvMmaCommonSetCnf },
    { ID_TAF_MMA_CDMA_FREQ_LOCK_SET_CNF,        AT_CMD_CFREQLOCK_SET,           AT_RcvMmaCFreqLockSetCnf },
    { ID_TAF_MMA_CTCC_ROAMING_NW_INFO_RTP_CFG_SET_CNF,   AT_CMD_CTROAMINFO_SET, AT_RcvMmaCommonSetCnf },

    { ID_TAF_MMA_1XCHAN_QUERY_CNF,              AT_CMD_1XCHAN_QRY,              AT_RcvMma1xChanQueryCnf },
    { ID_TAF_MMA_CVER_QUERY_CNF,                AT_CMD_CVER_QRY,                AT_RcvMmaCVerQueryCnf },
    { ID_TAF_MMA_GETSTA_QUERY_CNF,              AT_CMD_GETSTA_QRY,              AT_RcvMmaStateQueryCnf },
    { ID_TAF_MMA_CHIGHVER_QUERY_CNF,            AT_CMD_CHIGHVER_QRY,            AT_RcvMmaCHverQueryCnf },
    { ID_TAF_MMA_CDMA_LOCINFO_QRY_CNF,          AT_CMD_CLOCINFO_QRY,            AT_RcvMmaCLocInfoQueryCnf },
    /* 移出CDMA编译开关 */
    { ID_TAF_MMA_CURR_SID_NID_QRY_CNF,          AT_CMD_CSNID_QRY,               AT_RcvMmaQryCurrSidNidCnf },
    { ID_TAF_MMA_CTCC_ROAMING_NW_INFO_QRY_CNF,  AT_CMD_CTROAMINFO_QRY,          AT_RcvMmaCtRoamInfoCnf },
    { ID_TAF_MMA_PRLID_QRY_CNF,                 AT_CMD_PRLID_QRY,               AT_RcvMmaPrlIdQryCnf },
    { ID_TAF_MMA_RAT_COMBINED_MODE_QRY_CNF,     AT_CMD_RATCOMBINEDMODE_QRY,     AT_RcvMmaRatCombinedModeQryCnf },
    { ID_MMA_TAF_HDR_CSQ_QRY_SETTING_CNF,       AT_CMD_HDR_CSQ_QRY,             AT_RcvMmaHdrCsqQryCnf },
    { ID_TAF_MMA_CDMA_FREQ_LOCK_QRY_CNF,        AT_CMD_CFREQLOCK_QRY,           AT_RcvMmaCFreqLockQueryCnf },
    { ID_TAF_MMA_CDMACSQ_QRY_CNF,               AT_CMD_CDMACSQ_QRY,             AT_RcvMmaCdmaCsqQryCnf },

    { ID_TAF_MMA_CLMODE_IND,                    AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCLModInd },
    { ID_TAF_MMA_CL_DBDOMAIN_STATUSE_INFO_IND,  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaClDbdomainStatusReportInd },
    { ID_TAF_MMA_CSID_IND,                      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCSidInd },
    { ID_TAF_MMA_CLOCINFO_IND,                  AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaClocinfoInd },
    { ID_TAF_MMA_ROAMING_MODE_SWITCH_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaRoamingModeSwitchInd },
    { ID_TAF_MMA_COMBINED_MODE_SWITCH_IND,      AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCombinedModeSwitchInd },
    { ID_TAF_MMA_ICC_APP_TYPE_SWITCH_IND,       AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaIccAppTypeSwitchInd },
    { ID_TAF_MMA_CTIME_IND,                     AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCTimeInd },
    { ID_TAF_MMA_CDMACSQ_IND,                   AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCdmaCsqInd },
    { ID_MMA_TAF_HDR_CSQ_VALUE_IND,             AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaHdrCsqInd },
    { ID_TAF_MMA_CTCC_ROAMING_NW_INFO_REPORT_IND, AT_CMD_OPT_NO_NEED_CHECK,     AT_RcvMmaCtRoamingInfoChgInd },

    /* 移出CDMA编译开关 */
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_TAF_MMA_CSIDLIST_SET_CNF,              AT_CMD_CSIDLIST_SET,            AT_RcvMmaType2CommonSetCnf },
#endif

#if (FEATURE_IMS == FEATURE_ON)
    { ID_TAF_MMA_CLIMS_CFG_INFO_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaCLImsCfgInfoInd },
#endif
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { ID_TAF_MMA_EONS_CHANGE_IND,               AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaNwNameChangeInd},
    { ID_TAF_MMA_EONS_QRY_CNF,                  AT_CMD_EONS_SET,                AT_RcvMmaEonsInfoQryCnf},
    { ID_TAF_MMA_HCSQ_QRY_CNF,                  AT_CMD_HCSQ_QRY,                AT_RcvMmaHcsqInfoQryCnf },
    { ID_TAF_MMA_NETSELOPT_SET_CNF,             AT_CMD_NETSELOPT_SET,           AT_RcvMmaNetSelOptSetCnf },
    { ID_TAF_MMA_NETSELOPT_INFO_IND,            AT_CMD_OPT_NO_NEED_CHECK,       AT_RcvMmaNetSelOptInfoInd },
#endif
};

STATIC CONST AT_MsgProcEntryTblInfo g_atProcMmaMsgTblInfo = {
    .tbl      = g_atProcMmaMsgTbl,
    .entryNum = AT_ARRAY_SIZE(g_atProcMmaMsgTbl),
    .entryLen = sizeof(g_atProcMmaMsgTbl[0])
};

STATIC VOS_UINT32 AT_GetMmaUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    TAF_MMA_Msg *mmaMsg = (TAF_MMA_Msg *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(mmaMsg->ctrl.clientId, indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("AT_GetMmaUserId: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }
    return VOS_OK;
}

STATIC CONST AT_ModuleMsgProcInfo g_atMmaMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_MMA,
    .tblInfo    = &g_atProcMmaMsgTblInfo,
    .GetMsgId   = VOS_NULL_PTR,
    .GetUserId  = AT_GetMmaUserId,
};

VOS_VOID AT_InitMmaMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atMmaMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitMmaMsgProcTbl: register fail");
    }
}

/*
 * 功能描述: 来自MMA模块的消息处理函数
 */
VOS_VOID AT_ProcMmaMsg(struct MsgCB *msg)
{
    VOS_UINT32 ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_MMA);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_ProcMmaMsg: fail");
    }
}

