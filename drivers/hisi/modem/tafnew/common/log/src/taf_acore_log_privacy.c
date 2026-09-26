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

#include "taf_acore_log_privacy.h"
#include "taf_log_privacy_match.h"
#include "at_internal_msg.h"
#include "AtParse.h"
#include "at_ctx.h"
#include "at_mta_interface.h"
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
#include "taf_app_xsms_interface.h"
#include "at_xpds_interface.h"
#endif
#include "taf_drv_agent.h"
#include "ps_log_filter_interface.h"
#include "taf_msg_ts.h"
#include "taf_ccm_api.h"
#include "at_rnic_interface.h"
#include "at_ndis_interface.h"
#include "rnic_cds_interface.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif
#include "dsm_rnic_pif.h"
#include "dsm_ndis_pif.h"
#include "at_parse_core.h"
#include "at_cmd_proc.h"
#include "securec.h"
#include "mn_comm_api.h"
#include "taf_msg_chk_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_TAF_ACORE_LOG_PRIVACY_C

#define LOG_PRIVACY_AT_CMD_MAX_LEN (50) /* AT脱敏,命令最大长度 */

LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(VOS_UINT32 srcPid);

LOCAL VOS_UINT32 AT_PrivacyMatchIsTrustListAtCmd(AT_Msg *atMsg);
LOCAL AT_Msg* AT_PrivacyFilterCimi(AT_Msg *msg);

LOCAL AT_Msg* AT_PrivacyFilterCgsn(AT_Msg *msg);

LOCAL AT_Msg* AT_PrivacyFilterMsid(AT_Msg *msg);

LOCAL AT_Msg* AT_PrivacyFilterHplmn(AT_Msg *msg);

LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(MsgBlock *msg);

LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(MsgBlock *msg);

static const TAF_LogPrivacyMatchModemPidMapTbl g_tafPrivacyMatchModemPidMapTbl[] = {
    { I0_WUEPS_PID_TAF, I1_WUEPS_PID_TAF, I2_WUEPS_PID_TAF },
    { I0_UEPS_PID_XSMS, I1_UEPS_PID_XSMS, I2_UEPS_PID_XSMS },
    { I0_WUEPS_PID_USIM, I1_WUEPS_PID_USIM, I2_WUEPS_PID_USIM },
    { I0_MAPS_STK_PID, I1_MAPS_STK_PID, I2_MAPS_STK_PID },
    { I0_UEPS_PID_MTA, I1_UEPS_PID_MTA, I2_UEPS_PID_MTA },
    { I0_WUEPS_PID_DRV_AGENT, I1_WUEPS_PID_DRV_AGENT, I2_WUEPS_PID_DRV_AGENT },

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    { I0_UEPS_PID_XPDS, I1_UEPS_PID_XPDS, I2_UEPS_PID_XPDS },
#endif
    { I0_WUEPS_PID_MMA, I1_WUEPS_PID_MMA, I2_WUEPS_PID_MMA },
    { I0_UEPS_PID_DSM, I1_UEPS_PID_DSM, I2_UEPS_PID_DSM },

#if (FEATURE_ON == FEATURE_IMS)
    { I0_PS_PID_IMSA, I1_PS_PID_IMSA, I0_PS_PID_IMSA },
#endif

    { I0_UEPS_PID_CCM, I1_UEPS_PID_CCM, I2_UEPS_PID_CCM },
};

/* 不包含敏感信息的at内部消息Trust名单 */
static const AT_InterMsgIdUint32 g_atCmdTrustListTbl[] = {
    ID_AT_MNTN_INPUT_MSC,
    ID_AT_MNTN_START_FLOW_CTRL,
    ID_AT_MNTN_STOP_FLOW_CTRL,
    ID_AT_MNTN_REG_FC_POINT,
    ID_AT_MNTN_DEREG_FC_POINT,
    ID_AT_MNTN_PC_REPLAY_MSG,
    ID_AT_MNTN_PC_REPLAY_CLIENT_TAB,
    ID_AT_MNTN_RPT_PORT,
    ID_AT_MNTN_PS_CALL_ENTITY_RPT,
    ID_AT_COMM_CCPU_RESET_START,
    ID_AT_COMM_CCPU_RESET_END,
    ID_AT_COMM_HIFI_RESET_START,
    ID_AT_COMM_HIFI_RESET_END,
};

static const AT_LogPrivacyMatchAtCmdMapTbl g_privacyMatchAtCmdMapTbl[] = {
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* 呼叫相关AT命令 */
    { "AT^CFSH", "AT^CFSH" },
    { "AT^CBURSTDTMF", "AT^CBURSTDTMF" },
    { "AT^CCONTDTMF", "AT^CCONTDTMF" },
    { "AT^ECCALL", "AT^ECCALL" },
    { "AT^CUSTOMDIAL", "AT^CUSTOMDIAL" },
    { "AT^ECRANDOM", "AT^ECRANDOM" },
    { "AT^ECKMC", "AT^ECKMC" },
    /* 短信与写卡操作相关AT命令 */
    { "AT^CCMGS", "AT^CCMGS" },
    { "AT^CCMGW", "AT^CCMGW" },
    { "AT^CCMGD", "AT^CCMGD" },
    { "AT^MEID", "AT^MEID" },
    { "AT^CIMEI", "AT^CIMEI" },

    /* 参数中存在tmsi/ptmsi */
    { "AT^EFLOCIINFO", "AT^EFLOCIINFO" },
    { "AT^EFPSLOCIINFO", "AT^EFPSLOCIINFO" },

    /* XPDS相关 */
    { "AT^CAGPSPRMINFO", "AT^CAGPSPRMINFO" },
    { "AT^CAGPSPOSINFO", "AT^CAGPSPOSINFO" },
    { "AT^CAGPSFORWARDDATA", "AT^CAGPSFORWARDDATA" },
    { "AT^CAGPSSTART", "AT^CAGPSSTART" },
#endif

    { "AT^PHYNUM", "AT^PHYNUM" },
    { "AT^PHONEPHYNUM", "AT^PHONEPHYNUM" },
    { "AT^EPDU", "AT^EPDU" },

    /* 电话管理命令 */
    { "AT^NVM", "AT^NVM" },

    /* 电路域业务命令 */
    { "ATD", "ATD" },
    { "AT^APDS", "AT^APDS" },
    { "AT+VTS", "AT+VTS" },
    { "AT^DTMF", "AT^DTMF" },
#if (FEATURE_ON == FEATURE_ECALL)
    { "AT^ECLSTART", "AT^ECLSTART" },
#endif
#if (FEATURE_ON == FEATURE_IMS)
    { "AT^CACMIMS", "AT^CACMIMS" },
    { "AT^ECONFDIAL", "AT^ECONFDIAL" },
#endif

    /* 补充业务命令 */
    { "AT+CCFC", "AT+CCFC" },
    { "AT+CTFR", "AT+CTFR" },
    { "AT^CHLD", "AT^CHLD" },
    { "AT+CUSD", "AT+CUSD" },
    /* SMS短信业务命令 */
    { "AT+CMGS", "AT+CMGS" },
    { "AT+CMGW", "AT+CMGW" },
    { "AT+CMGC", "AT+CMGC" },
    { "AT+CMSS", "AT+CMSS" },
    { "AT^RSTRIGGER", "AT^RSTRIGGER" },

    /* 与AP对接命令 */
    { "AT+CPOS", "AT+CPOS" },
    { "AT+CPBS", "AT+CPBS" },
    { "AT+CPBR", "AT+CPBR" },
    { "AT+CPBW", "AT+CPBW" },
    { "AT+CPBF", "AT+CPBF" },
    { "AT^EFLOCIINFO", "AT^EFLOCIINFO" },
    { "AT^EFPSLOCIINFO", "AT^EFPSLOCIINFO" },
    { "AT^AUTHDATA", "AT^AUTHDATA" },
    { "AT^CGDNS", "AT^CGDNS" },
    { "AT+CGTFT", "AT+CGTFT" },
    { "AT+CGDCONT", "AT+CGDCONT" },
    { "AT^NDISDUP", "AT^NDISDUP" },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { "AT^LTEPROFILE", "AT^LTEPROFILE" },
#endif
};

static const AT_LogPrivacyMatchAtCmdMapTbl g_privacyMatchRptAtCmdMapTbl[] = {
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* 呼叫相关AT命令 */
    { "\r\n^CFSH", "\r\n^CFSH\r\n" },
    { "\r\n^CBURSTDTMF", "\r\n^CBURSTDTMF\r\n" },
    { "\r\n^CCONNNUM", "\r\n^CCONNNUM\r\n" },
    { "\r\n^CCALLEDNUM", "\r\n^CCALLEDNUM\r\n" },
    { "\r\n^CCALLINGNUM", "\r\n^CCALLINGNUM\r\n" },
    { "\r\n^CREDIRNUM", "\r\n^CREDIRNUM\r\n" },
    { "\r\n^CCWAC", "\r\n^CCWAC\r\n" },
    { "\r\n^CDISP", "\r\n^CDISP\r\n" },
    { "\r\n^CCONTDTMF", "\r\n^CCONTDTMF\r\n" },
    { "\r\n^ECCALL", "\r\n^ECCALL\r\n" },
    { "\r\n^ECRANDOM", "\r\n^ECRANDOM\r\n" },
    { "\r\n^ECKMC", "\r\n^ECKMC\r\n" },

    /* 短信与写卡操作相关AT命令 */
    { "\r\n^CCMT", "\r\n^CCMT\r\n" },
    { "\r\n^CCMGW", "\r\n^CCMGW\r\n" },
    { "\r\n^MEID", "\r\n^MEID\r\n" },
    { "\r\n^GETESN", "\r\n^GETESN\r\n" },
    { "\r\n^GETMEID", "\r\n^GETMEID\r\n" },

    { "\r\n^EFLOCIINFO", "\r\n^EFLOCIINFO\r\n" },
    { "\r\n^EFPSLOCIINFO", "\r\n^EFPSLOCIINFO\r\n" },
    /* XPDS相关 */
    { "\r\n^CAGPSPRMINFO", "\r\n^CAGPSPRMINFO\r\n" },
    { "\r\n^CAGPSPOSINFO", "\r\n^CAGPSPOSINFO\r\n" },
    { "\r\n^CAGPSFORWARDDATA", "\r\n^CAGPSFORWARDDATA\r\n" },
    { "\r\n^CAGPSREVERSEDATA", "\r\n^CAGPSREVERSEDATA\r\n" },
    { "\r\n^CAGPSREFLOCINFO", "\r\n^CAGPSREFLOCINFO\r\n" },
    { "\r\n^CAGPSPDEPOSINFO", "\r\n^CAGPSPDEPOSINFO\r\n" },
    { "\r\n^CAGPSACQASSISTINFO", "\r\n^CAGPSACQASSISTINFO\r\n" },
    { "\r\n^CAGPSIONINFO", "\r\n^CAGPSIONINFO\r\n" },
    { "\r\n^CAGPSEPHINFO", "\r\n^CAGPSEPHINFO\r\n" },
    { "\r\n^CAGPSALMINFO", "\r\n^CAGPSALMINFO\r\n" },
    { "\r\n^UTSGPSPOSINFO", "\r\n^UTSGPSPOSINFO\r\n" },
#endif

    { "\r\n^SCID", "\r\n^SCID\r\n" },
    { "\r\n^PHYNUM", "\r\n^PHYNUM\r\n" },
    { "\r\n^PHONEPHYNUM", "\r\n^PHONEPHYNUM\r\n" },
    { "\r\n^HVSCONT", "\r\n^HVSCONT\r\n" },
    { "\r\n^ICCID", "\r\n^ICCID\r\n" },
    { "\r\n^EPDUR", "\r\n^EPDUR\r\n" },

    /* 电话管理命令 */
    { "\r\n^XLEMA", "\r\n^XLEMA\r\n" },

#if (FEATURE_ON == FEATURE_IMS)
    /* 电路域业务命令 */
    { "\r\n^CLCCECONF", "\r\n^CLCCECONF\r\n" },
    { "\r\n^CSSU", "\r\n^CSSU\r\n" },
    { "\r\n^CSSUEX", "\r\n^CSSUEX\r\n" },
    { "\r\n^ECONFDIAL", "\r\n^ECONFDIAL\r\n" },
    { "\r\n^ECONFERR", "\r\n^ECONFERR\r\n" },
    { "\r\n^VOLTEIMPU", "\r\n^VOLTEIMPU\r\n" },
    { "\r\n^IMSMTRPT", "\r\n^IMSMTRPT\r\n" },
    { "\r\n^VOLTEIMPI", "\r\n^VOLTEIMPI\r\n" },
    { "\r\n^DMUSER", "\r\n^DMUSER\r\n" },
#endif
    { "\r\n+CLCC", "\r\n+CLCC\r\n" },
    { "\r\n^CLCC", "\r\n^CLCC\r\n" },
    { "\r\n^CLPR", "\r\n^CLPR\r\n" },
    { "\r\nRING", "\r\nRING\r\n" },
    { "\r\n+CRING", "\r\n+CRING\r\n" },

    /* 补充业务命令 */
    { "\r\n+CLIP", "\r\n+CLIP\r\n" },
    { "\r\n+CCFC", "\r\n+CCFC\r\n" },
    { "\r\n+COLP", "\r\n+COLP\r\n" },
    { "\r\n+CUSS", "\r\n+CUSS\r\n" },
#if (FEATURE_ON == FEATURE_IMS)
    { "\r\n^CUSS", "\r\n^CUSS\r\n" },
    { "\r\n^CCWA", "\r\n^CCWA\r\n" },
    { "\r\n^CSSI", "\r\n^CSSI\r\n" },
    { "\r\nIRING", "\r\nIRING\r\n" },
#endif
    { "\r\n+CSSU", "\r\n+CSSU\r\n" },
    { "\r\n+CCWA", "\r\n+CCWA\r\n" },
    { "\r\n+CNAP", "\r\n+CNAP\r\n" },
    { "\r\n^CNAP", "\r\n^CNAP\r\n" },
    { "\r\n+CMOLRN", "\r\n+CMOLRN\r\n" },
    { "\r\n+CMOLRG", "\r\n+CMOLRG\r\n" },

    /* SMS短信业务命令 */
    { "\r\n+CMT", "\r\n+CMT\r\n" },
    { "\r\n+CDS", "\r\n+CDS\r\n" },
    { "\r\n+CMGR", "\r\n+CMGR\r\n" },
    { "\r\n+CMGL", "\r\n+CMGL\r\n" },
    { "\r\n^RSTRIGGER", "\r\n^RSTRIGGER\r\n" },

    /* 与AP对接命令 */
    { "\r\n+CPOSR", "\r\n+CPOSR\r\n" },
    { "\r\n^NVRD", "\r\n^NVRD\r\n" },

    { "\r\n+CPBS", "\r\n+CPBS\r\n" },
    { "\r\n+CPBR", "\r\n+CPBR\r\n" },
    { "\r\n+CPBW", "\r\n+CPBW\r\n" },
    { "\r\n+CPBF", "\r\n+CPBF\r\n" },
    { "\r\n^EFLOCIINFO", "\r\n^EFLOCIINFO\r\n" },
    { "\r\n^EFPSLOCIINFO", "\r\n^EFPSLOCIINFO\r\n" },
    { "\r\n^LOCINFO", "\r\n^LOCINFO\r\n" },
    { "\r\n^CLOCINFO", "\r\n^CLOCINFO\r\n" },
    { "\r\n^LOCINFO", "\r\n^LOCINFO\r\n" },
    { "\r\n+CGDCONT", "\r\n+CGDCONT\r\n" },
    { "\r\n+CGPADDR", "\r\n+CGPADDR\r\n" },
    { "\r\n^CGCONTRDP", "\r\n^CGCONTRDP\r\n" },
    { "\r\n^CGTFTRDP", "\r\n^CGTFTRDP\r\n" },
    { "\r\n^AUTHDATA", "\r\n^AUTHDATA\r\n" },
    { "\r\n^CGDNS", "\r\n^CGDNS\r\n" },
    { "\r\n+CGTFT", "\r\n+CGTFT\r\n" },
    { "\r\n^DNSQUERY", "\r\n^DNSQUERY\r\n" },
    { "\r\n^SRCHEDPLMNINFO", "\r\n^SRCHEDPLMNINFO\r\n" },
    { "\r\n+CREG", "\r\n+CREG\r\n" },
    { "\r\n+CGREG", "\r\n+CGREG\r\n" },
    { "\r\n+CEREG", "\r\n+CEREG\r\n" },
    { "\r\n+ECID", "\r\n+ECID\r\n" },
    { "\r\n^REJINFO", "\r\n^REJINFO\r\n" },
    { "\r\n^NETSCAN", "\r\n^NETSCAN\r\n" },
    { "\r\n^MONSC", "\r\n^MONSC\r\n" },
    { "\r\n^MONNC", "\r\n^MONNC\r\n" },
    { "\r\n^PSEUCELL", "\r\n^PSEUCELL\r\n" },
    { "\r\n^SIMLOCKNWDATAWRITE", "\r\n^SIMLOCKNWDATAWRITE\r\n" },
    { "\r\n^IMSCTRLMSG", "\r\n^IMSCTRLMSG\r\n" },
    { "\r\n^IMSCTRLMSGU", "\r\n^IMSCTRLMSGU\r\n" },
    { "\r\n^NICKNAME", "\r\n^NICKNAME\r\n" },

#if (FEATURE_LTEV == FEATURE_ON)
    { "\r\n^VSYNCSRC", "\r\n^VSYNCSRC\r\n" },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* AT与VNAS模块间对接命令 */
    { "\r\n^SRCID", "\r\n^SRCID\r\n" },
#endif
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { "\r\n^LTEPROFILE", "\r\n^LTEPROFILE\r\n" },
    { "\r\n^GETDONGLEINFO", "\r\n^GETDONGLEINFO\r\n" },
    { "\r\n^CHECKAUTHORITY", "\r\n^CHECKAUTHORITY\r\n" },
#endif
};

static const AT_LogPrivacyMapCmdToFunc g_privacyMapCmdToFuncTbl[] = {
    { TAF_LOG_PRIVACY_AT_CMD_CIMI, AT_PrivacyFilterCimi },
    { TAF_LOG_PRIVACY_AT_CMD_CGSN, AT_PrivacyFilterCgsn },
    { TAF_LOG_PRIVACY_AT_CMD_MSID, AT_PrivacyFilterMsid },
    { TAF_LOG_PRIVACY_AT_CMD_HPLMN, AT_PrivacyFilterHplmn },
};

/* ******************************************************************************************** */
/* **************************** WUEPS_PID_AT发送消息脱敏函数处理表 **************************** */
/* AT发送给GUC TAF模块消息脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToTafMsgListTbl[] = {
    { TAF_MSG_REGISTERSS_MSG, AT_PrivacyMatchRegisterSsMsg },
    { TAF_MSG_PROCESS_USS_MSG, AT_PrivacyMatchProcessUssMsg },
    { TAF_MSG_INTERROGATESS_MSG, AT_PrivacyMatchInterRogateMsg },
    { TAF_MSG_ERASESS_MSG, AT_PrivacyMatchErasessMsg },
    { TAF_MSG_ACTIVATESS_MSG, AT_PrivacyMatchActivatessMsg },
    { TAF_MSG_DEACTIVATESS_MSG, AT_PrivacyMatchDeactivatessMsg },
    { TAF_MSG_REGPWD_MSG, AT_PrivacyMatchRegPwdMsg },


    { MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT, AT_PrivacyMatchSmsAppMsgReq },
    { MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM, AT_PrivacyMatchSmsAppMsgReq },
    { MN_MSG_MSGTYPE_WRITE, AT_PrivacyMatchSmsAppMsgReq },
    { MN_MSG_MSGTYPE_SEND_RPRPT, AT_PrivacyMatchSmsAppMsgReq },
    { MN_MSG_MSGTYPE_WRITE_SRV_PARA, AT_PrivacyMatchSmsAppMsgReq },
};

/* AT发给XSMS模块消息的脱敏处理函数表 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToXsmsMsgListTbl[] = {
    { TAF_XSMS_APP_MSG_TYPE_SEND_REQ, AT_PrivacyMatchAppMsgTypeSendReq },
    { TAF_XSMS_APP_MSG_TYPE_WRITE_REQ, AT_PrivacyMatchAppMsgTypeWriteReq },
    { TAF_XSMS_APP_MSG_TYPE_DELETE_REQ, AT_PrivacyMatchAppMsgTypeDeleteReq },
};
#endif

/* AT发给MTA模块消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToMtaMsgListTbl[] = {
    { ID_AT_MTA_CPOS_SET_REQ, AT_PrivacyMatchCposSetReq },
    { ID_AT_MTA_MEID_SET_REQ, AT_PrivacyMatchMeidSetReq },
    { ID_AT_MTA_PSEUCELL_INFO_SET_REQ, AT_PrivacyMatchPseucellInfoSetReq },
};

/* AT发给DRV_AGENT模块消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToDrvAgentMsgListTbl[] = {
    { DRV_AGENT_SIMLOCKWRITEEX_SET_REQ, AT_PrivacyMatchSimLockWriteExSetReq },
};

/* AT发给NDIS模块消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToNdisMsgListTbl[] = {
    { ID_AT_NDIS_PDNINFO_CFG_REQ, AT_PrivacyMatchNdisPdnInfoCfgReq },
    { ID_AT_NDIS_IFACE_UP_CONFIG_IND, AT_PrivacyMatchNdisIfaceUpConfigReq },
};

/* AT发给IMSA模块消息的脱敏处理函数表 */
#if (FEATURE_ON == FEATURE_IMS)
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToImsaMsgListTbl[] = {
    { ID_AT_IMSA_IMS_CTRL_MSG, AT_PrivacyMatchImsaImsCtrlMsg },
    { ID_AT_IMSA_NICKNAME_SET_REQ, AT_PrivacyMatchImsaNickNameSetReq },
};
#endif

/* ******************************************************************************************** */
/* **************************** WUEPS_PID_TAF发送消息脱敏函数处理表 **************************** */
/* TAF(WUEPS_PID_TAF)发给AT消息的脱敏处理函数表, A核C核使用同一份代码， */
static const TAF_LogPrivacyMsgMatchTbl g_tafAcorePrivacyMatchToAtMsgListTbl[] = {
    /* GUC A核C核都有调用，放最外层处理 */
    { MN_CALLBACK_CS_CALL, TAF_PrivacyMatchAppMnCallBackCsCall },
    { MN_CALLBACK_SS, TAF_PrivacyMatchAppMnCallBackSs },

    { MN_CALLBACK_QRY, TAF_PrivacyMatchAtCallBackQryProc },

    { MN_CALLBACK_MSG, TAF_PrivacyMatchAtCallBackSmsProc },
};

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* AT发送给XPDS模块消息脱敏处理函数表 */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToXpdsMsgListTbl[] = {
    { ID_AT_XPDS_GPS_POS_INFO_RSP, AT_PrivacyMatchCagpsPosInfoRsp },
    { ID_AT_XPDS_GPS_PRM_INFO_RSP, AT_PrivacyMatchCagpsPrmInfoRsp },
    { ID_AT_XPDS_AP_FORWARD_DATA_IND, AT_PrivacyMatchCagpsApForwardDataInd },
};
#endif
#endif

/* AT发送给VNAS模块消息的脱敏处理函数表 */
#if ((FEATURE_LTEV == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
static const TAF_LogPrivacyMsgMatchTbl g_atAcorePrivacyMatchToVnasMsgListTbl[] = {
    { ID_AT_VNAS_LTEV_SOURCE_ID_SET_REQ, AT_PrivacyMatchLtevSourceIdSetReq },
};
#endif

/* ******************************************************************************************** */
/* **************************** UEPS_PID_XSMS发送消息脱敏函数处理表 *************************** */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* XSMS发给AT消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafXsmsAcorePrivacyMatchToAtMsgListTbl[] = {
    { TAF_XSMS_APP_MSG_TYPE_RCV_IND, TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd },
    { TAF_XSMS_APP_MSG_TYPE_WRITE_CNF, TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf },
};

/* ******************************************************************************************** */
/* **************************** UEPS_PID_XPDS发送消息脱敏函数处理表 **************************** */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
/* XPDS发给AT模块消息的脱敏处理函数表 */
TAF_LogPrivacyMsgMatchTbl g_tafXpdsAcorePrivacyMatchToAtMsgListTbl[] = {
    { ID_XPDS_AT_GPS_REFLOC_INFO_CNF, TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf },
    { ID_XPDS_AT_GPS_ION_INFO_IND, TAF_XPDS_PrivacyMatchAtGpsIonInfoInd },
    { ID_XPDS_AT_GPS_EPH_INFO_IND, TAF_XPDS_PrivacyMatchAtGpsEphInfoInd },
    { ID_XPDS_AT_GPS_ALM_INFO_IND, TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd },
    { ID_XPDS_AT_GPS_PDE_POSI_INFO_IND, TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd },
    { ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND, TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd },
    { ID_XPDS_AT_AP_REVERSE_DATA_IND, TAF_XPDS_PrivacyMatchAtApReverseDataInd },
    { ID_XPDS_AT_UTS_GPS_POS_INFO_IND, TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd },
};

#endif
#endif

/* TAF发给TAF消息的脱敏处理函数表 */
/* (由于hi6932无x模，导致该数组定义大小为0，会有pclint告警，gu添加处理后，删除该cdma宏) */

/* TAF发给MMA消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafAcorePrivacyMatchToMmaMsgListTbl[] = {
    { ID_TAF_MMA_EFLOCIINFO_SET_REQ, TAF_MMA_PrivacyMatchAtEfClocInfoSetReq },
    { ID_TAF_MMA_EFPSLOCIINFO_SET_REQ, TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq },
};

static const TAF_LogPrivacyMsgMatchTbl g_tafAcorePrivacyMatchToCcmMsgListTbl[] = {
    { ID_TAF_CCM_CALL_ORIG_REQ, TAF_CCM_PrivacyMatchCallOrigReq },
    { ID_TAF_CCM_CALL_SUPS_CMD_REQ, TAF_CCM_PrivacyMatchCallSupsCmdReq },
    { ID_TAF_CCM_CUSTOM_DIAL_REQ, TAF_CCM_PrivacyMatchCustomDialReqCmdReq },
#if (FEATURE_ON == FEATURE_IMS)
    { ID_TAF_CCM_ECONF_DIAL_REQ, TAF_CCM_PrivacyMatchCcmEconfDialReq },
#endif
    { ID_TAF_CCM_START_DTMF_REQ, TAF_CCM_PrivacyMatchCcmStartDtmfReq },
    { ID_TAF_CCM_STOP_DTMF_REQ, TAF_CCM_PrivacyMatchCcmStopDtmfReq },
    { ID_TAF_CCM_SET_UUSINFO_REQ, TAF_CCM_PrivacyMatchCcmSetUusInfoReq },
    { ID_TAF_CCM_CUSTOM_ECC_NUM_REQ, TAF_CCM_PrivacyMatchCcmCustomEccNumReq },

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    { ID_TAF_CCM_SEND_FLASH_REQ, TAF_CCM_PrivacyMatchCcmSendFlashReq },
    { ID_TAF_CCM_SEND_BURST_DTMF_REQ, TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq },
    { ID_TAF_CCM_SEND_CONT_DTMF_REQ, TAF_CCM_PrivacyMatchCcmSendContDtmfReq },
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)
    { ID_TAF_CCM_ENCRYPT_VOICE_REQ, TAF_CCM_PrivacyMatchCcmEncryptVoiceReq },
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)
    { ID_TAF_CCM_SET_EC_KMC_REQ, TAF_CCM_PrivacyMatchCcmSetEcKmcReq },
#endif
#endif
#endif
};

/* RNIC发给CDS消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_rnicAcorePrivacyMatchToCdsMsgListTbl[] = {
    { ID_RNIC_CDS_IMS_DATA_REQ, RNIC_PrivacyMatchCdsImsDataReq },
};

/* TAF发给AT的PS事件消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafDsmAcorePrivacyMatchPsEvtMsgListTbl[] = {
    { ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF, TAF_DSM_PrivacyMatchPsCallPdpActCnf },
    { ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_IND, TAF_DSM_PrivacyMatchPsCallPdpActInd },
    { ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND, TAF_DSM_PrivacyMatchPsCallPdpManageInd },
    { ID_EVT_TAF_PS_CALL_PDP_MODIFY_CNF, TAF_DSM_PrivacyMatchPsCallPdpModCnf },
    { ID_EVT_TAF_PS_CALL_PDP_MODIFY_IND, TAF_DSM_PrivacyMatchPsCallPdpModInd },
    { ID_EVT_TAF_PS_CALL_PDP_IPV6_INFO_IND, TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd },
    { ID_EVT_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF, TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf },
    { ID_EVT_TAF_PS_GET_TFT_INFO_CNF, TAF_DSM_PrivacyMatchPsGetTftInfoCnf },
    { ID_EVT_TAF_PS_GET_PDP_IP_ADDR_INFO_CNF, TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf },
    { ID_EVT_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF, TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf },
    { ID_EVT_TAF_PS_GET_DYNAMIC_TFT_INFO_CNF, TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf },
    { ID_EVT_TAF_PS_GET_AUTHDATA_INFO_CNF, TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf },
    { ID_EVT_TAF_PS_GET_PDP_DNS_INFO_CNF, TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf },
    { ID_EVT_TAF_PS_GET_NEGOTIATION_DNS_CNF, TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf },
    { ID_EVT_TAF_PS_EPDG_CTRLU_NTF, TAF_DSM_PrivacyMatchPsEpdgCtrluNtf },
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    { ID_EVT_TAF_PS_IP_CHANGE_IND, TAF_DSM_PrivacyMatchTafPsIpChangeInd },
#endif
    { ID_EVT_TAF_PS_GET_CUST_ATTACH_PROFILE_CNF, TAF_DSM_PrivacyMatchPsGetCustAttachProfileCnf },
};

/* TAF发给AT的IFACE事件消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafDsmAcorePrivacyMatchIfaceEvtMsgListTbl[] = {
    { ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF, TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf },
    { ID_EVT_TAF_IFACE_RAB_INFO_IND, TAF_DSM_PrivacyMatchTafIfaceRabInfoInd },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_EVT_TAF_IFACE_IP_CHANGE_IND, TAF_DSM_PrivacyMatchTafIfaceIpChangeInd },
#endif
    { ID_DSM_NDIS_IFACE_UP_IND, TAF_DSM_PrivacyMatchNdisIfaceUpInd },
};

/* MTA发给AT消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafMtaAcorePrivacyMatchToAtMsgListTbl[] = {
    { ID_MTA_AT_CPOSR_IND, TAF_MTA_PrivacyMatchCposrInd },
    { ID_MTA_AT_MEID_QRY_CNF, TAF_MTA_PrivacyMatchAtMeidQryCnf },
    { ID_MTA_AT_CGSN_QRY_CNF, TAF_MTA_PrivacyMatchAtCgsnQryCnf },

    { ID_MTA_AT_ECID_SET_CNF, TAF_MTA_PrivacyMatchAtEcidSetCnf },

    { ID_MTA_AT_SET_NETMON_SCELL_CNF, TAF_MTA_PrivacyMatchAtSetNetMonScellCnf },
    { ID_MTA_AT_SET_NETMON_NCELL_CNF, TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf },
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    { ID_MTA_AT_SET_NETMON_SSCELL_CNF, TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf },
#endif
};

/* MMA发给AT消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafMmaAcorePrivacyMatchToAtMsgListTbl[] = {
    { ID_TAF_MMA_USIM_STATUS_IND, TAF_MMA_PrivacyMatchAtUsimStatusInd },
    { ID_TAF_MMA_HOME_PLMN_QRY_CNF, TAF_MMA_PrivacyMatchAtHomePlmnQryCnf },
    { ID_TAF_MMA_LOCATION_INFO_QRY_CNF, TAF_MMA_PrivacyMatchAtLocationInfoQryCnf },
    { ID_TAF_MMA_REG_STATUS_IND, TAF_MMA_PrivacyMatchAtRegStatusInd },
    { ID_TAF_MMA_SRCHED_PLMN_INFO_IND, TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd },
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    { ID_TAF_MMA_CDMA_LOCINFO_QRY_CNF, TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf },
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */
    { ID_TAF_MMA_NET_SCAN_CNF, TAF_MMA_PrivacyMatchAtNetScanCnf },
    { ID_TAF_MMA_REG_STATE_QRY_CNF, TAF_MMA_PrivacyMatchAtRegStateQryCnf },
    { ID_TAF_MMA_CLOCINFO_IND, TAF_MMA_PrivacyMatchAtClocInfoInd },
    { ID_TAF_MMA_REJINFO_QRY_CNF, TAF_MMA_PrivacyMatchAtRejInfoQryCnf },
    { ID_TAF_MMA_EFLOCIINFO_QRY_CNF, TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf },
    { ID_TAF_MMA_EFPSLOCIINFO_QRY_CNF, TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { ID_TAF_MMA_SUBNET_ADAPTER_INFO_IND, TAF_MMA_PrivacySubnetAdapterInfoInd },
#endif
};

/* TAF发给DSM消息的脱敏处理函数表 */
static const TAF_LogPrivacyMsgMatchTbl g_tafAcorePrivacyMatchToDsmMsgListTbl[] = {
    { ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ, TAF_DSM_PrivacyMatchTafSetAuthDataReq },
    { ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ, TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq },
    { ID_MSG_TAF_PS_CALL_ORIG_REQ, TAF_PrivacyMatchDsmPsCallOrigReq },
    { ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ, TAF_PrivacyMatchDsmPsPppDialOrigReq },
    { ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ, TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq },
    { ID_MSG_TAF_PS_SET_TFT_INFO_REQ, TAF_PrivacyMatchDsmPsSetTftInfoReq },
    { ID_MSG_TAF_PS_EPDG_CTRL_NTF, TAF_PrivacyMatchDsmPsEpdgCtrlNtf },
    { ID_MSG_TAF_IFACE_UP_REQ, TAF_PrivacyMatchDsmIfaceUpReq },
    { ID_MSG_TAF_PS_SET_CUST_ATTACH_PROFILE_REQ, TAF_PrivacyMatchDsmPsSetCustAttachProfileReq },
};

static const TAF_LogPrivacyMsgMatchTbl g_tafDrvAgentAcorePrivacyMatchToAtMsgListTbl[] = {
    { DRV_AGENT_MSID_QRY_CNF, TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf },
};

static const TAF_LogPrivacyMsgMatchTbl g_tafDsmAcorePrivacyMatchToRnicMsgListTbl[] = {
    { ID_DSM_RNIC_IFACE_CFG_IND, TAF_DSM_PrivacyMatchRnicIfaceCfgInd },
};

static const TAF_LogPrivacyMsgMatchTbl g_tafCcmAcorePrivacyMatchToAtMsgListTbl[] = {
#if (FEATURE_ON == FEATURE_IMS)
    { ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF, TAF_PrivacyMatchAppQryEconfCalledInfoCnf },
    { ID_TAF_CCM_ECONF_NOTIFY_IND, TAF_PrivacyMatchAtEconfNotifyInd },
#endif

    { ID_TAF_CCM_CALL_INCOMING_IND, TAF_PrivacyMatchAtCallIncomingInd },
    { ID_TAF_CCM_CALL_CONNECT_IND, TAF_PrivacyMatchAtCallConnectInd },
    { ID_TAF_CCM_QRY_CALL_INFO_CNF, TAF_PrivacyMatchAtQryCallInfoInd },
    { ID_TAF_CCM_CALL_SS_IND, TAF_PrivacyMatchAtCallSsInd },
    { ID_TAF_CCM_ECC_NUM_IND, TAF_PrivacyMatchAtEccNumInd },
    { ID_TAF_CCM_QRY_XLEMA_CNF, TAF_PrivacyMatchAtQryXlemaInd },
    { ID_TAF_CCM_CNAP_QRY_CNF, TAF_PrivacyMatchAtCnapQryCnf },
    { ID_TAF_CCM_CNAP_INFO_IND, TAF_PrivacyMatchAtCnapInfoInd },
    { ID_TAF_CCM_QRY_UUS1_INFO_CNF, TAF_PrivacyMatchAtQryUus1InfoCnf },
    { ID_TAF_CCM_UUS1_INFO_IND, TAF_PrivacyMatchAtUus1InfoInd },
    { ID_TAF_CCM_QRY_CLPR_CNF, TAF_PrivacyMatchAtQryClprCnf },


#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    { ID_TAF_CCM_CALLED_NUM_INFO_IND, TAF_PrivacyMatchAtCalledNumInfoInd },
    { ID_TAF_CCM_CALLING_NUM_INFO_IND, TAF_PrivacyMatchAtCallingNumInfoInd },
    { ID_TAF_CCM_DISPLAY_INFO_IND, TAF_PrivacyMatchAtDisplayInfoInd },
    { ID_TAF_CCM_EXT_DISPLAY_INFO_IND, TAF_PrivacyMatchAtExtDisplayInfoInd },
    { ID_TAF_CCM_CONN_NUM_INFO_IND, TAF_PrivacyMatchAtConnNumInfoInd },
    { ID_TAF_CCM_REDIR_NUM_INFO_IND, TAF_PrivacyMatchAtRedirNumInfoInd },
    { ID_TAF_CCM_CCWAC_INFO_IND, TAF_PrivacyMatchAtCcwacInfoInfoInd },
    { ID_TAF_CCM_CONT_DTMF_IND, TAF_PrivacyMatchAtContDtmfInd },
    { ID_TAF_CCM_BURST_DTMF_IND, TAF_PrivacyMatchAtBurstDtmfInd },
    { ID_TAF_CCM_ENCRYPT_VOICE_IND, TAF_PrivacyMatchAtEncryptVoiceInd },
    { ID_TAF_CCM_GET_EC_RANDOM_CNF, TAF_PrivacyMatchAtGetEcRandomCnf },
    { ID_TAF_CCM_GET_EC_KMC_CNF, TAF_PrivacyMatchAtGetEcKmcCnf },
#endif
};

/* ******************************************************************************************** */
/* ************************************** PID映射处理表 *************************************** */
/* WUEPS_PID_AT发送给不同pid的消息对应的脱敏处理表 */
static const TAF_LogPrivacyRcvPidMatchTbl g_atAcorePrivacyMatchRcvPidListTbl[] = {
/* AT发送给XPDS的消息过滤 */
#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    { UEPS_PID_XPDS, sizeof(g_atAcorePrivacyMatchToXpdsMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToXpdsMsgListTbl },
#endif

    /* AT发送给TAF的消息过滤 */
    { WUEPS_PID_TAF, sizeof(g_atAcorePrivacyMatchToTafMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToTafMsgListTbl },
/* AT发送给XSMS的消息 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    { UEPS_PID_XSMS, sizeof(g_atAcorePrivacyMatchToXsmsMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToXsmsMsgListTbl },
#endif
    /* AT发送给MTA的消息 */
    { UEPS_PID_MTA, sizeof(g_atAcorePrivacyMatchToMtaMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToMtaMsgListTbl },

    { PS_PID_APP_NDIS, sizeof(g_atAcorePrivacyMatchToNdisMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToNdisMsgListTbl },

    { WUEPS_PID_DRV_AGENT, sizeof(g_atAcorePrivacyMatchToDrvAgentMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToDrvAgentMsgListTbl },

#if (FEATURE_ON == FEATURE_IMS)
    { PS_PID_IMSA, sizeof(g_atAcorePrivacyMatchToImsaMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToImsaMsgListTbl },
#endif

    /* AT发送给VNAS的消息 */
#if ((FEATURE_LTEV == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { I0_PS_PID_VNAS, sizeof(g_atAcorePrivacyMatchToVnasMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_atAcorePrivacyMatchToVnasMsgListTbl },
#endif
};

/* TAF(WUEPS_PID_TAF)发送给不同pid的消息对应的脱敏处理表 */
static const TAF_LogPrivacyRcvPidMatchTbl g_tafAcorePrivacyMatchRcvPidListTbl[] = {
    /* GUC A核C核都有调用，放最外层处理 */
    { WUEPS_PID_AT, sizeof(g_tafAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafAcorePrivacyMatchToAtMsgListTbl },

    { WUEPS_PID_MMA, sizeof(g_tafAcorePrivacyMatchToMmaMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafAcorePrivacyMatchToMmaMsgListTbl },

    { UEPS_PID_DSM, sizeof(g_tafAcorePrivacyMatchToDsmMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafAcorePrivacyMatchToDsmMsgListTbl },
    { UEPS_PID_CCM, sizeof(g_tafAcorePrivacyMatchToCcmMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafAcorePrivacyMatchToCcmMsgListTbl },
};

/* UEPS_PID_XSMS发送给不同pid的消息对应的脱敏处理表 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
static const TAF_LogPrivacyRcvPidMatchTbl g_xsmsAcorePrivacyMatchRcvPidListTbl[] = {
    /* XSMS发送给AT的消息过滤 */
    { WUEPS_PID_AT, sizeof(g_tafXsmsAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafXsmsAcorePrivacyMatchToAtMsgListTbl },
};

/* UEPS_PID_XPDS发送给不同pid的消息对应的脱敏处理表 */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LogPrivacyRcvPidMatchTbl g_xpdsAcorePrivacyMatchRcvPidListTbl[] = {
    /* XPDS发送给AT的消息过滤 */
    { WUEPS_PID_AT, sizeof(g_tafXpdsAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafXpdsAcorePrivacyMatchToAtMsgListTbl },
};
#endif
#endif

/* MTA发送给不同PID的消息对应的脱敏处理表 */
static const TAF_LogPrivacyRcvPidMatchTbl g_tafMtaAcorePrivacyMatchRcvPidListTbl[] = {
    /* MTA发给AT的消息 */
    { WUEPS_PID_AT, sizeof(g_tafMtaAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafMtaAcorePrivacyMatchToAtMsgListTbl },
};

static const TAF_LogPrivacyRcvPidMatchTbl g_tafMmaAcorePrivacyMatchRcvPidListTbl[] = {
    { WUEPS_PID_AT, sizeof(g_tafMmaAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafMmaAcorePrivacyMatchToAtMsgListTbl },
};

static const TAF_LogPrivacyRcvPidMatchTbl g_tafDrvAgentAcorePrivacyMatchRcvPidListTbl[] = {
    { WUEPS_PID_AT, sizeof(g_tafDrvAgentAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafDrvAgentAcorePrivacyMatchToAtMsgListTbl },
};

/* RNIC发送给不同pid的消息对应的脱敏处理表 */
static const TAF_LogPrivacyRcvPidMatchTbl g_rnicAcorePrivacyMatchRcvPidListTbl[] = {
    { UEPS_PID_CDS, sizeof(g_rnicAcorePrivacyMatchToCdsMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_rnicAcorePrivacyMatchToCdsMsgListTbl },
};

static const TAF_LogPrivacyRcvPidMatchTbl g_tafDsmAcorePrivacyMatchRcvPidListTbl[] = {
    { WUEPS_PID_AT, sizeof(g_tafDsmAcorePrivacyMatchPsEvtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafDsmAcorePrivacyMatchPsEvtMsgListTbl },
    { WUEPS_PID_AT, sizeof(g_tafDsmAcorePrivacyMatchIfaceEvtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafDsmAcorePrivacyMatchIfaceEvtMsgListTbl },
    { ACPU_PID_RNIC, sizeof(g_tafDsmAcorePrivacyMatchToRnicMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafDsmAcorePrivacyMatchToRnicMsgListTbl },
};

static const TAF_LogPrivacyRcvPidMatchTbl g_tafCcmAcorePrivacyMatchRcvPidListTbl[] = {
    { WUEPS_PID_AT, sizeof(g_tafCcmAcorePrivacyMatchToAtMsgListTbl) / sizeof(TAF_LogPrivacyMsgMatchTbl), g_tafCcmAcorePrivacyMatchToAtMsgListTbl },
};


LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(VOS_UINT32 srcPid)
{
    VOS_UINT32 loop;

    for (loop = 0; loop < sizeof(g_tafPrivacyMatchModemPidMapTbl) / sizeof(TAF_LogPrivacyMatchModemPidMapTbl); loop++) {
        if ((srcPid == g_tafPrivacyMatchModemPidMapTbl[loop].modem1Pid) ||
            (srcPid == g_tafPrivacyMatchModemPidMapTbl[loop].modem2Pid)) {
            return g_tafPrivacyMatchModemPidMapTbl[loop].modem0Pid;
        }
    }

    return srcPid;
}

VOS_UINT32 AT_PrivacyFilterMode(AT_Msg *atMsg)
{
    if (atMsg->indexNum < AT_MAX_CLIENT_NUM) {
        if (g_parseContext[atMsg->indexNum].mode == AT_XML_MODE) {
            /* xml模式直接进行过滤 */
            MN_NORM_LOG1("AT_PrivacyFilterTypeAndMode: TRUE,XML MODE, msgName ", atMsg->msgId);
            return VOS_FALSE;
        }

        if (g_parseContext[atMsg->indexNum].mode == AT_SMS_MODE) {
            /* 短信模式直接进行过滤 */
            MN_NORM_LOG1("AT_PrivacyFilterTypeAndMode: TRUE,SMS MODE msgName ", atMsg->msgId);
            return VOS_FALSE;
        }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if ((g_parseContext[atMsg->indexNum].mode == AT_UE_POLICY_MODE) ||
            (g_parseContext[atMsg->indexNum].mode == AT_IMS_URSP_MODE)) {
            /* UE POLICY模式直接进行过滤 */
            MN_NORM_LOG1("AT_PrivacyFilterTypeAndMode: TRUE,UE POLICY MODE msgName ", atMsg->msgId);
            return VOS_FALSE;
        }
#endif
    }
    return VOS_TRUE;
}

VOS_VOID* AT_PrivacyFilterType(AT_Msg *atMsg)
{
    AT_Msg    *privacyMatchAtMsg = VOS_NULL_PTR;
    VOS_UINT32 loop;

    for (loop = 0; loop < (sizeof(g_privacyMapCmdToFuncTbl) / sizeof(AT_LogPrivacyMapCmdToFunc)); loop++) {
        if (atMsg->filterAtType == g_privacyMapCmdToFuncTbl[loop].atCmdType) {
            privacyMatchAtMsg = g_privacyMapCmdToFuncTbl[loop].privacyAtCmd(atMsg);

            if (privacyMatchAtMsg == VOS_NULL_PTR) {
                break;
            } else {
                return (VOS_VOID *)privacyMatchAtMsg;
            }
        }
    }
    return VOS_NULL_PTR;
}

VOS_VOID* AT_PrivacyFilterMatchCmdTbl(AT_Msg *atMsg, VOS_CHAR *pcPrivacyAtCmd)
{
    AT_Msg    *privacyMatchAtMsg = VOS_NULL_PTR;
    errno_t    memResult;
    VOS_UINT16 privacyAtMsgLen;
    VOS_UINT16 atMsgHeaderLen;
    VOS_UINT16 privacyAtCmdLen;

    privacyAtCmdLen = (VOS_UINT16)(VOS_StrLen(pcPrivacyAtCmd));

    if (privacyAtCmdLen > AT_MSG_DEFAULT_VALUE_LEN) {
        /* 消息结构体长度 + at命令字符串长度 - 消息结构体中aucValue数组长度 */
        privacyAtMsgLen = sizeof(AT_Msg) + privacyAtCmdLen - AT_MSG_DEFAULT_VALUE_LEN;
    } else {
        privacyAtMsgLen = sizeof(AT_Msg);
    }

    /* A核不调用多实例接口申请内存 */
    privacyMatchAtMsg = (AT_Msg *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, privacyAtMsgLen);

    if (privacyMatchAtMsg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(privacyMatchAtMsg, privacyAtMsgLen, 0, privacyAtMsgLen);

    atMsgHeaderLen = sizeof(AT_Msg) - AT_MSG_DEFAULT_VALUE_LEN;

    /* 拷贝原始消息头部 */
    memResult = memcpy_s(privacyMatchAtMsg, privacyAtMsgLen, atMsg, atMsgHeaderLen);
    TAF_MEM_CHK_RTN_VAL(memResult, privacyAtMsgLen, atMsgHeaderLen);

    /* 设置新的at cmd字符串长度 */
    VOS_SET_MSG_LEN(privacyMatchAtMsg, (privacyAtMsgLen - VOS_MSG_HEAD_LENGTH));
    privacyMatchAtMsg->len = privacyAtCmdLen;

    /* 拷贝脱敏后at命令字符串 */
    if (privacyAtCmdLen > 0) {
        memResult = memcpy_s(privacyMatchAtMsg->value, privacyAtCmdLen, pcPrivacyAtCmd, privacyAtCmdLen);
        TAF_MEM_CHK_RTN_VAL(memResult, privacyAtCmdLen, privacyAtCmdLen);
    }

    return (VOS_VOID *)privacyMatchAtMsg;
}

LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(MsgBlock *msg)
{
    AT_NDIS_PdnInfoCfgReq *pdnInfoCfgReq = VOS_NULL_PTR;
    VOS_UINT32             length;
    errno_t                memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    pdnInfoCfgReq = (AT_NDIS_PdnInfoCfgReq *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (pdnInfoCfgReq == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(pdnInfoCfgReq, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(&(pdnInfoCfgReq->ipv4PdnInfo), sizeof(pdnInfoCfgReq->ipv4PdnInfo), 0, sizeof(AT_NDIS_Ipv4PdnInfo));

    (VOS_VOID)memset_s(&(pdnInfoCfgReq->ipv6PdnInfo), sizeof(pdnInfoCfgReq->ipv6PdnInfo), 0, sizeof(AT_NDIS_Ipv6PdnInfo));

    return (VOS_VOID *)pdnInfoCfgReq;
}

LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(MsgBlock *msg)
{
    AT_NDIS_IfaceUpConfigInd *ifaceUpConfigInd = VOS_NULL_PTR;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 分配消息,申请内存后续统一由底层释放 */
    ifaceUpConfigInd = (AT_NDIS_IfaceUpConfigInd *)TAF_AllocPrivacyMsg(VOS_GET_SENDER_ID(msg), msg, &length);

    if (ifaceUpConfigInd == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(ifaceUpConfigInd, length, msg, length);
    TAF_MEM_CHK_RTN_VAL(memResult, length, length);

    /* 将敏感信息设置为全0 */
    (VOS_VOID)memset_s(&(ifaceUpConfigInd->ipv4PdnInfo), sizeof(ifaceUpConfigInd->ipv4PdnInfo), 0, sizeof(AT_NDIS_Ipv4PdnInfo));

    (VOS_VOID)memset_s(&(ifaceUpConfigInd->ipv6PdnInfo), sizeof(ifaceUpConfigInd->ipv6PdnInfo), 0, sizeof(AT_NDIS_Ipv6PdnInfo));

    return (VOS_VOID *)ifaceUpConfigInd;
}

LOCAL VOS_UINT32 AT_PrivacyMatchIsTrustListAtCmd(AT_Msg *atMsg)
{
    VOS_UINT32 loop;

    for (loop = 0; loop < sizeof(g_atCmdTrustListTbl) / sizeof(AT_InterMsgIdUint32); loop++) {
        if (g_atCmdTrustListTbl[loop] == atMsg->msgId) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_VOID* AT_PrivacyMatchAtCmd(MsgBlock *msg)
{
    VOS_UINT8 *atCmdData         = VOS_NULL_PTR;
    AT_Msg    *atMsg             = VOS_NULL_PTR;
    AT_Msg    *privacyMatchAtMsg = VOS_NULL_PTR;
    VOS_UINT32 loop;
    errno_t    memResult;
    VOS_UINT16 tempAtCmdLen;
    VOS_UINT16 setAtTmpLen;

    atMsg = (AT_Msg *)msg;

    /* Trust名单中的消息不脱敏, 直接返回原消息, 不在Trust名单中的信息再进行脱敏检查 */
    if (AT_PrivacyMatchIsTrustListAtCmd(atMsg) == VOS_TRUE) {
        return (VOS_VOID *)msg;
    } else {
        if (AT_PrivacyFilterMode(atMsg) == VOS_FALSE) {
            return VOS_NULL_PTR;
        }

        privacyMatchAtMsg = (AT_Msg *)AT_PrivacyFilterType(atMsg);

        if (privacyMatchAtMsg != VOS_NULL_PTR) {
            return (VOS_VOID *)privacyMatchAtMsg;
        }

        /* 申请足够大的内存, 临时存放AT Cmd, 用于判断是否需要过滤, 使用后释放 */
        tempAtCmdLen = (atMsg->len > LOG_PRIVACY_AT_CMD_MAX_LEN) ? atMsg->len : LOG_PRIVACY_AT_CMD_MAX_LEN;
        atCmdData    = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, tempAtCmdLen);

        if (atCmdData == VOS_NULL_PTR) {
            return VOS_NULL_PTR;
        }

        (VOS_VOID)memset_s(atCmdData, tempAtCmdLen, 0x00, tempAtCmdLen);
        if (atMsg->len > 0) {
            memResult = memcpy_s(atCmdData, tempAtCmdLen, atMsg->value, atMsg->len);
            TAF_MEM_CHK_RTN_VAL(memResult, tempAtCmdLen, atMsg->len);
        }

        (VOS_VOID)At_UpString(atCmdData, atMsg->len);

        /* 上报与回复命令的处理 */
        for (loop = 0; loop < (sizeof(g_privacyMatchRptAtCmdMapTbl) / sizeof(AT_LogPrivacyMatchAtCmdMapTbl)); loop++) {
            if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_privacyMatchRptAtCmdMapTbl[loop].originalAtCmd, atCmdData,
                                     VOS_StrLen(g_privacyMatchRptAtCmdMapTbl[loop].originalAtCmd))) {
                privacyMatchAtMsg = AT_PrivacyFilterMatchCmdTbl(atMsg, g_privacyMatchRptAtCmdMapTbl[loop].privacyAtCmd);

                PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

                return (VOS_VOID *)privacyMatchAtMsg;
            }
        }

        setAtTmpLen = atMsg->len;
        AT_DiscardInvalidChar(atCmdData, tempAtCmdLen, &setAtTmpLen);

        /* 下发命令的处理 */
        for (loop = 0; loop < (sizeof(g_privacyMatchAtCmdMapTbl) / sizeof(AT_LogPrivacyMatchAtCmdMapTbl)); loop++) {
            if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_privacyMatchAtCmdMapTbl[loop].originalAtCmd, atCmdData,
                                     VOS_StrLen(g_privacyMatchAtCmdMapTbl[loop].originalAtCmd))) {
                privacyMatchAtMsg = AT_PrivacyFilterMatchCmdTbl(atMsg, g_privacyMatchAtCmdMapTbl[loop].privacyAtCmd);

                PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

                return (VOS_VOID *)privacyMatchAtMsg;
            }
        }

        /* 未匹配, 不包含敏感信息, 返回原始消息 */
        PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

        return (VOS_VOID *)msg;
    }
}

LOCAL AT_Msg* AT_PrivacyFilterCnfCommProc(AT_Msg *msg, VOS_UINT32 startIndex, VOS_CHAR endChar)
{
    AT_Msg    *atMsg                  = VOS_NULL_PTR;
    AT_Msg    *privacyMatchAtMsg      = VOS_NULL_PTR;
    VOS_UINT8 *privacyMatchAtMsgValue = VOS_NULL_PTR;
    VOS_UINT8 *value                  = VOS_NULL_PTR;
    VOS_UINT32 indexNum;
    errno_t    memResult;
    VOS_UINT16 privacyAtMsgLen;

    atMsg = (AT_Msg *)msg;

    indexNum = 0;

    /* 判断是查询请求还是回复结果,如果是查询请求则不需要脱敏 */
    if (atMsg->value[0] != '\r') {
        return VOS_NULL_PTR;
    }

    privacyAtMsgLen = sizeof(AT_Msg) + atMsg->len - 4;

    /* A核不调用多实例接口申请内存 */
    privacyMatchAtMsgValue = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, privacyAtMsgLen);

    if (privacyMatchAtMsgValue == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    privacyMatchAtMsg = (AT_Msg *)privacyMatchAtMsgValue;

    /* 拷贝原始消息 */
    memResult = memcpy_s(privacyMatchAtMsg, privacyAtMsgLen, atMsg, privacyAtMsgLen);
    TAF_MEM_CHK_RTN_VAL(memResult, privacyAtMsgLen, privacyAtMsgLen);

    value = privacyMatchAtMsgValue + sizeof(AT_Msg) - 4;

    if (privacyMatchAtMsg->len > AT_CMD_MAX_LEN) {
        privacyMatchAtMsg->len = AT_CMD_MAX_LEN;
    }

    for (indexNum = startIndex; indexNum < privacyMatchAtMsg->len; indexNum++) {
        if (endChar == value[indexNum]) {
            break;
        }

        value[indexNum] = '*';
    }

    return privacyMatchAtMsg;
}

LOCAL AT_Msg* AT_PrivacyFilterCimi(AT_Msg *msg)
{
    /*
     * 从第7位开始替换为*,第0位到第6位不需要替换
     * 0:\r     1:\n     2-4:MCC      5-6:MNC
     */
    return AT_PrivacyFilterCnfCommProc(msg, 7, '\r');
}

LOCAL AT_Msg* AT_PrivacyFilterCgsn(AT_Msg *msg)
{
    /*
     * 从第2位开始替换为*,第0位和第1位不需要替换
     * 0:\r     1:\n
     */
    return AT_PrivacyFilterCnfCommProc(msg, 2, '\r');
}

LOCAL AT_Msg* AT_PrivacyFilterMsid(AT_Msg *msg)
{
    VOS_UINT8 *atCmdData         = VOS_NULL_PTR;
    AT_Msg    *atMsg             = VOS_NULL_PTR;
    AT_Msg    *privacyMatchAtMsg = VOS_NULL_PTR;
    VOS_UINT16 privacyAtMsgLen;
    VOS_CHAR  *pcAtCmd       = "ATI";
    VOS_CHAR  *pcFilterField = "IMEI: ";
    VOS_UINT32 indexNum, index2;
    errno_t    memResult;
    VOS_UINT16 filterFieldLen;

    atMsg = (AT_Msg *)msg;

    if (atMsg->len == 0) {
        return VOS_NULL_PTR;
    }

    /* 申请内存, 临时存放AT Cmd, 用于判断是否需要过滤, 使用后释放 */
    atCmdData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, atMsg->len);

    if (atCmdData == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    memResult = memcpy_s(atCmdData, atMsg->len, atMsg->value, atMsg->len);
    TAF_MEM_CHK_RTN_VAL(memResult, atMsg->len, atMsg->len);

    (VOS_VOID)At_UpString(atCmdData, atMsg->len);

    /* 判断是ATI的查询请求还是回复结果,如果是查询请求则不需要脱敏 */
    if (PS_MEM_CMP((VOS_UINT8 *)pcAtCmd, atCmdData, VOS_StrLen(pcAtCmd)) == VOS_OK) {
        PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

        return VOS_NULL_PTR;
    }

    privacyAtMsgLen = sizeof(AT_Msg) + atMsg->len - 4;

    /* A核不调用多实例接口申请内存 */
    privacyMatchAtMsg = (AT_Msg *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, privacyAtMsgLen);
    if (privacyMatchAtMsg == VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

        return VOS_NULL_PTR;
    }

    /* 拷贝原始消息 */
    memResult = memcpy_s(privacyMatchAtMsg, privacyAtMsgLen, atMsg, privacyAtMsgLen);
    TAF_MEM_CHK_RTN_VAL(memResult, privacyAtMsgLen, privacyAtMsgLen);

    /*
     * 只替换IMEI字段后的信息
     */
    filterFieldLen = (VOS_UINT16)VOS_StrLen(pcFilterField);

    if (filterFieldLen > privacyMatchAtMsg->len) {
        PS_MEM_FREE(WUEPS_PID_AT, atCmdData);
        /*lint -save -e516*/
        PS_MEM_FREE(WUEPS_PID_AT, privacyMatchAtMsg);
        /*lint -restore*/

        return VOS_NULL_PTR;
    }

    if (privacyMatchAtMsg->len > AT_CMD_MAX_LEN) {
        privacyMatchAtMsg->len = AT_CMD_MAX_LEN;
    }

    for (indexNum = 0; indexNum < (VOS_UINT32)(privacyMatchAtMsg->len - filterFieldLen); indexNum++) {
        /* 找出IMEI字段 */
        if (PS_MEM_CMP((VOS_UINT8 *)pcFilterField, &(privacyMatchAtMsg->value[indexNum]), filterFieldLen) == VOS_OK) {
            /* 将IMEI具体值替换成* */
            for (index2 = (indexNum + filterFieldLen); index2 < privacyMatchAtMsg->len; index2++) {
                if (privacyMatchAtMsg->value[index2] == '\r') {
                    break;
                }

                privacyMatchAtMsg->value[index2] = '*';
            }

            break;
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, atCmdData);

    return privacyMatchAtMsg;
}

LOCAL AT_Msg* AT_PrivacyFilterHplmn(AT_Msg *msg)
{
    /*
     * 从第15位开始替换为*,第0位到第14位不需要替换
     * 0:\r     1:\n     2-9:^HPLMN:(此处还有一个空格)
     * 10-12:MCC   13-14:MNC
     */
    return AT_PrivacyFilterCnfCommProc(msg, 15, ',');
}

VOS_VOID* AT_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    VOS_VOID                        *atMsgPrivacyMatchMsg = VOS_NULL_PTR;
    const TAF_LogPrivacyMsgMatchTbl *privacyMatchMsgTbl   = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader         = VOS_NULL_PTR;
    VOS_UINT32                       tblSize;
    VOS_UINT32                       loop;
    VOS_UINT32                       msgName;
    VOS_UINT32                       rcvPid;

    if (TAF_ChkAtSndLogPrivacyMsgLen((const MsgBlock *)msg) != VOS_TRUE) {
        MN_ERR_LOG("AT_AcoreMsgLogPrivacyMatchProc: message length is invalid.");
        return VOS_NULL_PTR;
    }

    rcvMsgHeader = (MSG_Header *)msg;
    rcvPid       = VOS_GET_RECEIVER_ID(rcvMsgHeader);

    if (VOS_GET_RECEIVER_ID(rcvMsgHeader) == WUEPS_PID_AT) {
        atMsgPrivacyMatchMsg = AT_PrivacyMatchAtCmd((MsgBlock *)msg);

        return (VOS_VOID *)atMsgPrivacyMatchMsg;
    }

    /* A核单编译, 需要将I1/I2的PID转换为I0的PID */
    rcvPid = TAF_LogPrivacyGetModem0Pid(VOS_GET_RECEIVER_ID(rcvMsgHeader));

    tblSize = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (loop = 0; loop < (sizeof(g_atAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl)); loop++) {
        if (rcvPid == g_atAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            privacyMatchMsgTbl = g_atAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            tblSize = g_atAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    msgName = (VOS_UINT16)(rcvMsgHeader->msgName);

    /* 查找入参消息对应的脱敏函数 */
    if (privacyMatchMsgTbl != VOS_NULL_PTR) {
        for (loop = 0; loop < tblSize; loop++) {
            if (msgName == privacyMatchMsgTbl[loop].msgName) {
                atMsgPrivacyMatchMsg = privacyMatchMsgTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return atMsgPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID* TAF_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                        *tafLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;
    VOS_UINT32                       rcvPid;

    if (TAF_ChkTafSndLogPrivacyMsgLen((const MsgBlock *)msg) != VOS_TRUE) {
        MN_ERR_LOG("AT_CcoreMsgLogPrivacyMatchProc: message length is invalid.");
        return VOS_NULL_PTR;
    }

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    /* A核单编译, 需要将I1/I2的PID转换为I0的PID */
    rcvPid = TAF_LogPrivacyGetModem0Pid(VOS_GET_RECEIVER_ID(rcvMsgHeader));

    /* 根据rcv pid查找pid映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        if (rcvPid == g_tafAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            logPrivacyMsgMatchTbl = g_tafAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            msgMatchTblSize = g_tafAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 若根据rcv pid找到pid映射表，继续根据匹配表查找处理函数 */
    if (logPrivacyMsgMatchTbl != VOS_NULL_PTR) {
        msgName = (VOS_UINT16)(rcvMsgHeader->msgName);

        /* 根据msg name查找过滤函数映射表 */
        for (loop = 0; loop < msgMatchTblSize; loop++) {
            if (msgName == logPrivacyMsgMatchTbl[loop].msgName) {
                tafLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return tafLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_XSMS_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    VOS_VOID                        *xsmsPrivacyMatchMsg = VOS_NULL_PTR;
    const TAF_LogPrivacyMsgMatchTbl *privacyMatchMsgTbl  = VOS_NULL_PTR;
    MSG_Header                      *msgHeader           = VOS_NULL_PTR;
    VOS_UINT32                      tblSize;
    VOS_UINT32                      indexNum;
    VOS_UINT32                      msgName;

    msgHeader = (MSG_Header *)msg;
    tblSize   = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (indexNum = 0; indexNum < (sizeof(g_xsmsAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl));
         indexNum++) {
        if (VOS_GET_RECEIVER_ID(msgHeader) == g_xsmsAcorePrivacyMatchRcvPidListTbl[indexNum].receiverPid) {
            privacyMatchMsgTbl = g_xsmsAcorePrivacyMatchRcvPidListTbl[indexNum].logPrivacyMatchMsgTbl;

            tblSize = g_xsmsAcorePrivacyMatchRcvPidListTbl[indexNum].tblSize;

            break;
        }
    }

    /* 查找入参消息对应的脱敏函数 */
    if (privacyMatchMsgTbl != VOS_NULL_PTR) {
        msgName = (VOS_UINT16)(msgHeader->msgName);

        for (indexNum = 0; indexNum < tblSize; indexNum++) {
            if (msgName == privacyMatchMsgTbl[indexNum].msgName) {
                xsmsPrivacyMatchMsg = privacyMatchMsgTbl[indexNum].funcPrivacyMatch((MsgBlock *)msg);

                return xsmsPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}
#endif

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))

VOS_VOID* TAF_XPDS_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    VOS_VOID                  *xpdsPrivacyMatchMsg = VOS_NULL_PTR;
    TAF_LogPrivacyMsgMatchTbl *privacyMatchMsgTbl  = VOS_NULL_PTR;
    PS_MsgHeader              *msgHeader           = VOS_NULL_PTR;
    VOS_UINT32                 tblSize;
    VOS_UINT32                 loop;
    VOS_UINT32                 msgName;

    msgHeader = (PS_MsgHeader *)msg;
    tblSize   = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (loop = 0; loop < (sizeof(g_xpdsAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl));
         loop++) {
        if (VOS_GET_RECEIVER_ID(msgHeader) == g_xpdsAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            privacyMatchMsgTbl = g_xpdsAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            tblSize = g_xpdsAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 查找入参消息对应的脱敏函数 */
    if (privacyMatchMsgTbl != VOS_NULL_PTR) {
        msgName = msgHeader->msgName;

        for (loop = 0; loop < tblSize; loop++) {
            if (msgName == privacyMatchMsgTbl[loop].msgName) {
                xpdsPrivacyMatchMsg = privacyMatchMsgTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return xpdsPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}
#endif

VOS_VOID* TAF_MTA_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                        *mtaLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafMtaAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        if (VOS_GET_RECEIVER_ID(rcvMsgHeader) == g_tafMtaAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            logPrivacyMsgMatchTbl = g_tafMtaAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            msgMatchTblSize = g_tafMtaAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (logPrivacyMsgMatchTbl != VOS_NULL_PTR) {
        msgName = (VOS_UINT16)rcvMsgHeader->msgName;

        /* 根据msgName查找过滤函数映射表 */
        for (loop = 0; loop < msgMatchTblSize; loop++) {
            if (msgName == logPrivacyMsgMatchTbl[loop].msgName) {
                mtaLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return mtaLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID* TAF_MMA_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                        *mmaLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafMmaAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        if (VOS_GET_RECEIVER_ID(rcvMsgHeader) == g_tafMmaAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            logPrivacyMsgMatchTbl = g_tafMmaAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            msgMatchTblSize = g_tafMmaAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (logPrivacyMsgMatchTbl != VOS_NULL_PTR) {
        msgName = (VOS_UINT16)rcvMsgHeader->msgName;

        /* 根据msgName查找过滤函数映射表 */
        for (loop = 0; loop < msgMatchTblSize; loop++) {
            if (msgName == logPrivacyMsgMatchTbl[loop].msgName) {
                mmaLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return mmaLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID* TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl      = VOS_NULL_PTR;
    VOS_VOID                        *drvAgentLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader               = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafDrvAgentAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        if (VOS_GET_RECEIVER_ID(rcvMsgHeader) == g_tafDrvAgentAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            logPrivacyMsgMatchTbl = g_tafDrvAgentAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            msgMatchTblSize = g_tafDrvAgentAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (logPrivacyMsgMatchTbl != VOS_NULL_PTR) {
        msgName = (VOS_UINT16)rcvMsgHeader->msgName;

        /* 根据msgName查找过滤函数映射表 */
        for (loop = 0; loop < msgMatchTblSize; loop++) {
            if (msgName == logPrivacyMsgMatchTbl[loop].msgName) {
                drvAgentLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return drvAgentLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID TAF_OM_LayerMsgLogPrivacyMatchRegAcore(VOS_VOID)
{
    /*
     * 1、AT在PID init时先读取NV, 然后再注册脱敏回调函数, 所以此处直接使用NV值,
     *    log脱敏NV打开, 则注册回调函数, 否则不注册
     * 2、A核单编译, 只能访问I0接口, 所以I1/I2的消息同样注册I0的过滤接口, 在过滤函数内增加PID转换
     * 3、产品线确认NV配置以MODEM_0为准
     */

    if (AT_GetPrivacyFilterEnableFlg() == VOS_TRUE) {
        PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, AT_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(ACPU_PID_RNIC, RNIC_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_DSM, TAF_DSM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        /* 2020-02-18 sendpid检查，脱敏函数适配 */
        PS_OM_LayerMsgReplaceCBReg(ACPU_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_CCM, TAF_CCM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_MTA, TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (MULTI_MODEM_NUMBER >= 2)
        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_CCM, TAF_CCM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_DSM, TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_MTA, TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (3 == MULTI_MODEM_NUMBER)

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_CCM, TAF_CCM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_DSM, TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_MTA, TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif
#endif
#endif
    }
}

VOS_VOID* RNIC_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    VOS_VOID                        *privacyMsg         = VOS_NULL_PTR;
    const TAF_LogPrivacyMsgMatchTbl *privacyMatchMsgTbl = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader       = VOS_NULL_PTR;
    VOS_UINT32                       tblSize;
    VOS_UINT32                       loop;
    VOS_UINT32                       msgName;
    VOS_UINT32                       rcvPid;

    rcvMsgHeader = (MSG_Header *)msg;

    tblSize = 0;

    rcvPid = TAF_LogPrivacyGetModem0Pid(VOS_GET_RECEIVER_ID(rcvMsgHeader));

    /* 查找入参消息对应的rcvpid表 */
    for (loop = 0; loop < (sizeof(g_rnicAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl));
         loop++) {
        if (rcvPid == g_rnicAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            privacyMatchMsgTbl = g_rnicAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            tblSize = g_rnicAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    msgName = (VOS_UINT16)(rcvMsgHeader->msgName);

    /* 查找入参消息对应的脱敏函数 */
    if (privacyMatchMsgTbl != VOS_NULL_PTR) {
        for (loop = 0; loop < tblSize; loop++) {
            if (msgName == privacyMatchMsgTbl[loop].msgName) {
                privacyMsg = privacyMatchMsgTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return privacyMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID* TAF_DSM_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                        *tafLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       i;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;
    VOS_UINT32                       receiverPid;

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafDsmAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    receiverPid = TAF_LogPrivacyGetModem0Pid(VOS_GET_RECEIVER_ID(rcvMsgHeader));

    /* 根据rcv pid查找pid映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        logPrivacyMsgMatchTbl = g_tafDsmAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

        if ((logPrivacyMsgMatchTbl == VOS_NULL_PTR) ||
            (g_tafDsmAcorePrivacyMatchRcvPidListTbl[loop].receiverPid != receiverPid)) {
            continue;
        }

        msgMatchTblSize = g_tafDsmAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

        msgName = rcvMsgHeader->msgName;

        if (receiverPid == WUEPS_PID_AT) {
            if ((msgName == MN_CALLBACK_PS_CALL) || (msgName == MN_CALLBACK_IFACE)) {
                msgName = ((TAF_PS_Evt *)rcvMsgHeader)->evtId;
            }
        }

        /* 根据msg name查找过滤函数映射表 */
        for (i = 0; i < msgMatchTblSize; i++) {
            if (logPrivacyMsgMatchTbl[i].msgName != msgName) {
                continue;
            }

            if (logPrivacyMsgMatchTbl[i].funcPrivacyMatch != VOS_NULL_PTR) {
                tafLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[i].funcPrivacyMatch((MsgBlock *)msg);
                return tafLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

VOS_VOID* TAF_CCM_AcoreMsgLogPrivacyMatchProc(MsgBlock *msg)
{
    const TAF_LogPrivacyMsgMatchTbl *logPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                        *tafLogPrivacyMatchMsg = VOS_NULL_PTR;
    MSG_Header                      *rcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                       loop;
    VOS_UINT32                       rcvPidMatchTblSize;
    VOS_UINT32                       msgMatchTblSize;
    VOS_UINT32                       msgName;
    VOS_UINT32                       rcvPid;

    rcvMsgHeader = (MSG_Header *)msg;

    rcvPidMatchTblSize = sizeof(g_tafCcmAcorePrivacyMatchRcvPidListTbl) / sizeof(TAF_LogPrivacyRcvPidMatchTbl);
    msgMatchTblSize    = 0;

    /* A核单编译, 需要将I1/I2的PID转换为I0的PID */
    rcvPid = TAF_LogPrivacyGetModem0Pid(VOS_GET_RECEIVER_ID(rcvMsgHeader));

    /* 根据rcv pid查找pid映射表 */
    for (loop = 0; loop < rcvPidMatchTblSize; loop++) {
        if (rcvPid == g_tafCcmAcorePrivacyMatchRcvPidListTbl[loop].receiverPid) {
            logPrivacyMsgMatchTbl = g_tafCcmAcorePrivacyMatchRcvPidListTbl[loop].logPrivacyMatchMsgTbl;

            msgMatchTblSize = g_tafCcmAcorePrivacyMatchRcvPidListTbl[loop].tblSize;

            break;
        }
    }

    /* 若根据rcv pid找到pid映射表，继续根据匹配表查找处理函数 */
    if (logPrivacyMsgMatchTbl != VOS_NULL_PTR) {
        msgName = (rcvMsgHeader->msgName);

        /* 根据msg name查找过滤函数映射表 */
        for (loop = 0; loop < msgMatchTblSize; loop++) {
            if (msgName == logPrivacyMsgMatchTbl[loop].msgName) {
                tafLogPrivacyMatchMsg = logPrivacyMsgMatchTbl[loop].funcPrivacyMatch((MsgBlock *)msg);

                return tafLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)msg;
}

