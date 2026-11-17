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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "TafAcoreLogPrivacy.h"
#include "TafLogPrivacyMatch.h"
#include "AtInternalMsg.h"
#include "AtParse.h"
#include "AtCtx.h"
#include "TafAppXsmsInterface.h"
#include "AtMtaInterface.h"
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
#include "AtXpdsInterface.h"
#endif
#include "TafAppCall.h"
#include "TafDrvAgent.h"
#include "PsLogFilterInterface.h"
#include "MnMsgTs.h"
#include "TafCcmApi.h"
#include "AtRnicInterface.h"
#include "AtNdisInterface.h"
#include "RnicCdsInterface.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_TAF_ACORE_LOG_PRIVACY_C

#ifndef AT_SMS_MODE
#define    AT_SMS_MODE         (1)
#endif

#define    AT_XML_MODE         (2)

#define LOG_PRIVACY_AT_CMD_MAX_LEN         (50)                                 /* AT脱敏,命令最大长度 */

LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(
    VOS_UINT32                          ulSrcPid
);

LOCAL VOS_UINT32 AT_PrivacyMatchIsWhiteListAtCmd(
    AT_MSG_STRU                        *pstAtMsg
);
LOCAL AT_MSG_STRU* AT_PrivacyFilterCimi(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterCgsn(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterMsid(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterHplmn(
    AT_MSG_STRU                        *pstMsg
);

LOCAL VOS_VOID* AT_PrivacyMatchRnicPdnInfoCfgInd(
    MsgBlock                                               *pstMsg
);

LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(
    MsgBlock                                               *pstMsg
);

LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(
    MsgBlock                                               *pstMsg
);

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/

TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU                g_astTafPrivacyMatchModemPidMapTbl[] =
{
    {I0_WUEPS_PID_TAF,  I1_WUEPS_PID_TAF,   I2_WUEPS_PID_TAF},
    {I0_UEPS_PID_XSMS,  I1_UEPS_PID_XSMS,   I2_UEPS_PID_XSMS},
    {I0_WUEPS_PID_USIM, I1_WUEPS_PID_USIM,  I2_WUEPS_PID_USIM},
    {I0_MAPS_STK_PID,   I1_MAPS_STK_PID,    I2_MAPS_STK_PID},
    {I0_UEPS_PID_MTA,   I1_UEPS_PID_MTA,    I2_UEPS_PID_MTA},
    {I0_WUEPS_PID_DRV_AGENT, I1_WUEPS_PID_DRV_AGENT, I2_WUEPS_PID_DRV_AGENT},

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    {I0_UEPS_PID_XPDS,  I1_UEPS_PID_XPDS,   I2_UEPS_PID_XPDS},
#endif
    {I0_WUEPS_PID_MMA,  I1_WUEPS_PID_MMA,   I2_WUEPS_PID_MMA},
    {I0_UEPS_PID_DSM,   I1_UEPS_PID_DSM,    I2_UEPS_PID_DSM},

#if (FEATURE_ON == FEATURE_IMS)
    {I0_PS_PID_IMSA,    I1_PS_PID_IMSA,     I0_PS_PID_IMSA},
#endif
};

/* 不包含敏感信息的at内部消息白名单 */
AT_INTER_MSG_ID_ENUM_UINT32                                 g_aenAtCmdWhiteListTbl[] =
{
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
    ID_AT_NCM_CONN_STATUS_CMD,
    ID_AT_WATER_LOW_CMD,
    ID_AT_SWITCH_CMD_MODE,
};

AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU                    g_astPrivacyMatchAtCmdMapTbl[] =
{
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* 呼叫相关AT命令 */
    {"AT^CFSH"                  ,   "AT^CFSH"},
    {"AT^CBURSTDTMF"            ,   "AT^CBURSTDTMF"},
    {"AT^CCONTDTMF"             ,   "AT^CCONTDTMF"},
    {"AT^ECCALL"                ,   "AT^ECCALL"},
    {"AT^CUSTOMDIAL"            ,   "AT^CUSTOMDIAL"},
    {"AT^ECRANDOM"              ,   "AT^ECRANDOM"},
    {"AT^ECKMC"                 ,   "AT^ECKMC"},
    {"\r\n^CFSH"                ,   "\r\n^CFSH\r\n"},
    {"\r\n^CBURSTDTMF"          ,   "\r\n^CBURSTDTMF\r\n"},
    {"\r\n^CCONNNUM"            ,   "\r\n^CCONNNUM\r\n"},
    {"\r\n^CCALLEDNUM"          ,   "\r\n^CCALLEDNUM\r\n"},
    {"\r\n^CCALLINGNUM"         ,   "\r\n^CCALLINGNUM\r\n"},
    {"\r\n^CREDIRNUM"           ,   "\r\n^CREDIRNUM\r\n"},
    {"\r\n^CCWAC"               ,   "\r\n^CCWAC\r\n"},
    {"\r\n^CDISP"               ,   "\r\n^CDISP\r\n"},
    {"\r\n^CCONTDTMF"           ,   "\r\n^CCONTDTMF\r\n"},
    {"\r\n^ECCALL"              ,   "\r\n^ECCALL\r\n"},
    {"\r\n^ECRANDOM"            ,   "\r\n^ECRANDOM\r\n"},
    {"\r\n^ECKMC"               ,   "\r\n^ECKMC\r\n"},

    /* 短信与写卡操作相关AT命令 */
    {"AT^CCMGS"                 ,   "AT^CCMGS"},
    {"AT^CCMGW"                 ,   "AT^CCMGW"},
    {"AT^CCMGD"                 ,   "AT^CCMGD"},
    {"\r\n^CCMT"                ,   "\r\n^CCMT\r\n"},
    {"\r\n^CCMGW"               ,   "\r\n^CCMGW\r\n"},
    {"AT^MEID"                  ,   "AT^MEID"},
    {"\r\n^MEID"                ,   "\r\n^MEID\r\n"},
    {"\r\n^GETESN"              ,   "\r\n^GETESN\r\n"},
    {"\r\n^GETMEID"             ,   "\r\n^GETMEID\r\n"},
    {"AT^CIMEI"                 ,   "AT^CIMEI"},

    /* XPDS相关 */
    {"AT^CAGPSPRMINFO"          ,   "AT^CAGPSPRMINFO"},
    {"AT^CAGPSPOSINFO"          ,   "AT^CAGPSPOSINFO"},
    {"AT^CAGPSFORWARDDATA"      ,   "AT^CAGPSFORWARDDATA"},
    {"AT^CAGPSSTART"            ,   "AT^CAGPSSTART"},
    {"\r\n^CAGPSPRMINFO"        ,   "\r\n^CAGPSPRMINFO\r\n"},
    {"\r\n^CAGPSPOSINFO"        ,   "\r\n^CAGPSPOSINFO\r\n"},
    {"\r\n^CAGPSFORWARDDATA"    ,   "\r\n^CAGPSFORWARDDATA\r\n"},
    {"\r\n^CAGPSREVERSEDATA"    ,   "\r\n^CAGPSREVERSEDATA\r\n"},
    {"\r\n^CAGPSREFLOCINFO"     ,   "\r\n^CAGPSREFLOCINFO\r\n"},
    {"\r\n^CAGPSPDEPOSINFO"     ,   "\r\n^CAGPSPDEPOSINFO\r\n"},
    {"\r\n^CAGPSACQASSISTINFO"  ,   "\r\n^CAGPSACQASSISTINFO\r\n"},
    {"\r\n^CAGPSIONINFO"        ,   "\r\n^CAGPSIONINFO\r\n"},
    {"\r\n^CAGPSEPHINFO"        ,   "\r\n^CAGPSEPHINFO\r\n"},
    {"\r\n^CAGPSALMINFO"        ,   "\r\n^CAGPSALMINFO\r\n"},
    {"\r\n^UTSGPSPOSINFO"       ,   "\r\n^UTSGPSPOSINFO\r\n"},
#endif

    /* 参数中存在tmsi/ptmsi */
    { "AT^EFLOCIINFO", "AT^EFLOCIINFO" },
    { "AT^EFPSLOCIINFO", "AT^EFPSLOCIINFO" },
    { "\r\n^EFLOCIINFO", "\r\n^EFLOCIINFO\r\n" },
    { "\r\n^EFPSLOCIINFO", "\r\n^EFPSLOCIINFO\r\n" },

    {"\r\n^SCID"                ,   "\r\n^SCID\r\n"},
    {"\r\n^PHYNUM"              ,   "\r\n^PHYNUM\r\n"},
    {"\r\n^PHONEPHYNUM"         ,   "\r\n^PHONEPHYNUM\r\n"},
    {"AT^PHYNUM"                ,   "AT^PHYNUM"},
    {"AT^PHONEPHYNUM"           ,   "AT^PHONEPHYNUM"},
    {"\r\n^HVSCONT"             ,   "\r\n^HVSCONT\r\n"},
    {"\r\n^ICCID"               ,   "\r\n^ICCID\r\n"},
    {"AT^EPDU"                  ,   "AT^EPDU"},
    {"\r\n^EPDUR"               ,   "\r\n^EPDUR\r\n"},

    /* 电话管理命令 */
    {"AT^NVM"                   ,   "AT^NVM"},
    {"\r\n^XLEMA"               ,   "\r\n^XLEMA\r\n"},

    /* 电路域业务命令 */
    {"ATD"                      ,   "ATD"},
    {"AT^APDS"                  ,   "AT^APDS"},
    {"AT+VTS"                   ,   "AT+VTS"},
    {"AT^DTMF"                  ,   "AT^DTMF"},
#if (FEATURE_ON == FEATURE_ECALL)
    {"AT^ECLSTART"              ,   "AT^ECLSTART"},
#endif
#if (FEATURE_ON == FEATURE_IMS)
    {"AT^CACMIMS"               ,   "AT^CACMIMS"},
    {"AT^ECONFDIAL"             ,   "AT^ECONFDIAL"},
    {"\r\n^CLCCECONF"           ,   "\r\n^CLCCECONF\r\n"},
    {"\r\n^CSSU"                ,   "\r\n^CSSU\r\n"},
    {"\r\n^CSSUEX"              ,   "\r\n^CSSUEX\r\n"},
    {"\r\n^ECONFDIAL"           ,   "\r\n^ECONFDIAL\r\n"},
    {"\r\n^ECONFERR"            ,   "\r\n^ECONFERR\r\n"},
    {"\r\n^VOLTEIMPU"           ,   "\r\n^VOLTEIMPU\r\n"},
    {"\r\n^IMSMTRPT"            ,   "\r\n^IMSMTRPT\r\n"},
    {"\r\n^VOLTEIMPI"           ,   "\r\n^VOLTEIMPI\r\n"},
    {"\r\n^DMUSER"              ,   "\r\n^DMUSER\r\n"},
#endif
    {"\r\n+CLCC"                ,   "\r\n+CLCC\r\n"},
    {"\r\n^CLCC"                ,   "\r\n^CLCC\r\n"},
    {"\r\n^CLPR"                ,   "\r\n^CLPR\r\n"},
    {"\r\nRING"                 ,   "\r\nRING\r\n"},
    {"\r\n+CRING"               ,   "\r\n+CRING\r\n"},

    /* 补充业务命令 */
    {"AT+CCFC"                  ,   "AT+CCFC"},
    {"AT+CTFR"                  ,   "AT+CTFR"},
    {"AT^CHLD"                  ,   "AT^CHLD"},
    {"AT+CUSD"                  ,   "AT+CUSD"},
    {"\r\n+CLIP"                ,   "\r\n+CLIP\r\n"},
    {"\r\n+CCFC"                ,   "\r\n+CCFC\r\n"},
    {"\r\n+COLP"                ,   "\r\n+COLP\r\n"},
    {"\r\n+CUSS"                ,   "\r\n+CUSS\r\n"},
#if (FEATURE_ON == FEATURE_IMS)
    {"\r\n^CUSS"                ,   "\r\n^CUSS\r\n"},
    {"\r\n^CCWA"                ,   "\r\n^CCWA\r\n"},
    {"\r\n^CSSI"                ,   "\r\n^CSSI\r\n"},
    {"\r\nIRING"                ,   "\r\nIRING\r\n"},
#endif
    {"\r\n+CSSU"                ,   "\r\n+CSSU\r\n"},
    {"\r\n+CCWA"                ,   "\r\n+CCWA\r\n"},
    {"\r\n+CNAP"                ,   "\r\n+CNAP\r\n"},
    {"\r\n^CNAP"                ,   "\r\n^CNAP\r\n"},
    {"\r\n+CMOLRN"              ,   "\r\n+CMOLRN\r\n"},
    {"\r\n+CMOLRG"              ,   "\r\n+CMOLRG\r\n"},

    /* SMS短信业务命令 */
    {"AT+CMGS"                  ,   "AT+CMGS"},
    {"AT+CMGW"                  ,   "AT+CMGW"},
    {"AT+CMGC"                  ,   "AT+CMGC"},
    {"AT+CMSS"                  ,   "AT+CMSS"},
    {"AT^RSTRIGGER"             ,   "AT^RSTRIGGER"},
    {"\r\n+CMT"                 ,   "\r\n+CMT\r\n"},
    {"\r\n+CDS"                 ,   "\r\n+CDS\r\n"},
    {"\r\n+CMGR"                ,   "\r\n+CMGR\r\n"},
    {"\r\n+CMGL"                ,   "\r\n+CMGL\r\n"},
    {"\r\n^RSTRIGGER"           ,   "\r\n^RSTRIGGER\r\n"},

    /* 与AP对接命令 */
    {"AT+CPOS"                  ,   "AT+CPOS"},
    {"\r\n+CPOSR"               ,   "\r\n+CPOSR\r\n"},
    {"\r\n^DIALOGNTF"           ,   "\r\n^DIALOGNTF\r\n"},
    {"\r\n^NVRD"           ,        "\r\n^NVRD\r\n"},

    {"AT+CPBS"                  ,   "AT+CPBS"},
    {"AT+CPBR"                  ,   "AT+CPBR"},
    {"AT+CPBW"                  ,   "AT+CPBW"},
    {"AT+CPBF"                  ,   "AT+CPBF"},
    {"AT^EFLOCIINFO"            ,   "AT^EFLOCIINFO"},
    {"AT^EFPSLOCIINFO"          ,   "AT^EFPSLOCIINFO"},
    {"AT^CGDCONT"               ,   "AT^CGDCONT"},
    {"AT^AUTHDATA"              ,   "AT^AUTHDATA"},
    {"AT^CGDNS"                 ,   "AT^CGDNS"},
    {"AT+CGTFT"                 ,   "AT+CGTFT"},
    {"\r\n+CPBS"                ,   "\r\n+CPBS\r\n"},
    {"\r\n+CPBR"                ,   "\r\n+CPBR\r\n"},
    {"\r\n+CPBW"                ,   "\r\n+CPBW\r\n"},
    {"\r\n+CPBF"                ,   "\r\n+CPBF\r\n"},
    {"\r\n^EFLOCIINFO"          ,   "\r\n^EFLOCIINFO\r\n"},
    {"\r\n^EFPSLOCIINFO"        ,   "\r\n^EFPSLOCIINFO\r\n"},
    {"\r\n^LOCINFO"             ,   "\r\n^LOCINFO\r\n"},
    {"\r\n^CLOCINFO"            ,   "\r\n^CLOCINFO\r\n"},
    {"\r\n^LOCINFO"             ,   "\r\n^LOCINFO\r\n"},
    {"\r\n+CGDCONT"             ,   "\r\n+CGDCONT\r\n"},
    {"\r\n+CGPADDR"             ,   "\r\n+CGPADDR\r\n"},
    {"\r\n^CGCONTRDP"           ,   "\r\n^CGCONTRDP\r\n"},
    {"\r\n^CGTFTRDP"            ,   "\r\n^CGTFTRDP\r\n"},
    {"\r\n^AUTHDATA"            ,   "\r\n^AUTHDATA\r\n"},
    {"\r\n^CGDNS"               ,   "\r\n^CGDNS\r\n"},
    {"\r\n+CGTFT"               ,   "\r\n+CGTFT\r\n"},
    {"\r\n^DNSQUERY"            ,   "\r\n^DNSQUERY\r\n"},
    {"\r\n^SRCHEDPLMNINFO"      ,   "\r\n^SRCHEDPLMNINFO\r\n"},
    {"\r\n+CREG"                ,   "\r\n+CREG\r\n"},
    {"\r\n+CGREG"               ,   "\r\n+CGREG\r\n"},
    {"\r\n+CEREG"               ,   "\r\n+CEREG\r\n"},
    {"\r\n+ECID"                ,   "\r\n+ECID\r\n"},
    {"\r\n^REJINFO"             ,   "\r\n^REJINFO\r\n"},
    {"\r\n^NETSCAN"             ,   "\r\n^NETSCAN\r\n"},
    {"\r\n^MONSC"               ,   "\r\n^MONSC\r\n"},
    {"\r\n^MONNC"               ,   "\r\n^MONNC\r\n"},
    {"\r\n^PSEUCELL"            ,   "\r\n^PSEUCELL\r\n"},
    {"\r\n^SIMLOCKNWDATAWRITE"  ,   "\r\n^SIMLOCKNWDATAWRITE\r\n"},
    {"\r\n^IMSCTRLMSG"          ,   "\r\n^IMSCTRLMSG\r\n"},
    {"\r\n^IMSCTRLMSGU"         ,   "\r\n^IMSCTRLMSGU\r\n"},
    {"\r\n^NICKNAME"            ,   "\r\n^NICKNAME\r\n"},
};
AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU                         g_astPrivacyMapCmdToFuncTbl[] =
{
    {TAF_LOG_PRIVACY_AT_CMD_CIMI                ,           AT_PrivacyFilterCimi},
    {TAF_LOG_PRIVACY_AT_CMD_CGSN                ,           AT_PrivacyFilterCgsn},
    {TAF_LOG_PRIVACY_AT_CMD_MSID                ,           AT_PrivacyFilterMsid},
    {TAF_LOG_PRIVACY_AT_CMD_HPLMN               ,           AT_PrivacyFilterHplmn},
};

/**********************************************************************************************/
/***************************** WUEPS_PID_AT发送消息脱敏函数处理表 *****************************/
/* AT发送给GUC TAF模块消息脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToTafMsgListTbl[] =
{
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {TAF_CALL_APP_SEND_FLASH_REQ,                           AT_PrivacyMatchCallAppSendFlashReq},
    {TAF_CALL_APP_SEND_BURST_DTMF_REQ,                      AT_PrivacyMatchCallAppSendBurstReq},
    {TAF_CALL_APP_SEND_CONT_DTMF_REQ,                       AT_PrivacyMatchCallAppSendContReq},
    {TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ,                     AT_PrivacyMatchCallAppSendCustomDialReq},
#endif

#if (FEATURE_ON == FEATURE_IMS)
    {TAF_CALL_APP_ECONF_DIAL_REQ,                           AT_PrivacyMatchCallAppEconfDialReq},
#endif
    {TAF_MSG_REGISTERSS_MSG,                                AT_PrivacyMatchRegisterSsMsg},
    {TAF_MSG_PROCESS_USS_MSG,                               AT_PrivacyMatchProcessUssMsg},
    {TAF_MSG_INTERROGATESS_MSG,                             AT_PrivacyMatchInterRogateMsg},
    {TAF_MSG_ERASESS_MSG,                                   AT_PrivacyMatchErasessMsg},
    {TAF_MSG_ACTIVATESS_MSG,                                AT_PrivacyMatchActivatessMsg},
    {TAF_MSG_DEACTIVATESS_MSG,                              AT_PrivacyMatchDeactivatessMsg},
    {TAF_MSG_REGPWD_MSG,                                    AT_PrivacyMatchRegPwdMsg},

    {MN_CALL_APP_ORIG_REQ,                                  AT_PrivacyMatchCallAppOrigReq},
    {MN_CALL_APP_SUPS_CMD_REQ,                              AT_PrivacyMatchCallAppSupsCmdReq},
    {MN_CALL_APP_START_DTMF_REQ,                            AT_PrivacyMatchCallAppStartDtmfReq},
    {MN_CALL_APP_STOP_DTMF_REQ,                             AT_PrivacyMatchCallAppStopDtmfReq},
    {MN_CALL_APP_CUSTOM_ECC_NUM_REQ,                        AT_PrivacyMatchCallAppCustomEccNumReq},

    {MN_CALL_APP_SET_UUSINFO_REQ,                           AT_PrivacyMatchCallAppSetUusinfoReq},

    {MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT,                     AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM,                    AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_WRITE,                                  AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_SEND_RPRPT,                             AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_WRITE_SRV_PARA,                         AT_PrivacyMatchSmsAppMsgReq},

};

/* AT发给XSMS模块消息的脱敏处理函数表 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToXsmsMsgListTbl[] =
{
    {TAF_XSMS_APP_MSG_TYPE_SEND_REQ,                        AT_PrivacyMatchAppMsgTypeSendReq},
    {TAF_XSMS_APP_MSG_TYPE_WRITE_REQ,                       AT_PrivacyMatchAppMsgTypeWriteReq},
    {TAF_XSMS_APP_MSG_TYPE_DELETE_REQ,                      AT_PrivacyMatchAppMsgTypeDeleteReq},
};
#endif

/* AT发给MTA模块消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToMtaMsgListTbl[] =
{
    {ID_AT_MTA_CPOS_SET_REQ,                                AT_PrivacyMatchCposSetReq},
    {ID_AT_MTA_MEID_SET_REQ,                                AT_PrivacyMatchMeidSetReq},
    {ID_AT_MTA_PSEUCELL_INFO_SET_REQ,                       AT_PrivacyMatchPseucellInfoSetReq},
};

/* AT发给DRV_AGENT模块消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl[] =
{
    {DRV_AGENT_SIMLOCKWRITEEX_SET_REQ,                      AT_PrivacyMatchSimLockWriteExSetReq},
};

/* AT发给RNIC模块消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToRnicMsgListTbl[] =
{
    {ID_AT_RNIC_PDN_INFO_CFG_IND,                           AT_PrivacyMatchRnicPdnInfoCfgInd},
};

/* AT发给NDIS模块消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToNdisMsgListTbl[] =
{
    {ID_AT_NDIS_PDNINFO_CFG_REQ,                            AT_PrivacyMatchNdisPdnInfoCfgReq},
    {ID_AT_NDIS_IFACE_UP_CONFIG_IND,                        AT_PrivacyMatchNdisIfaceUpConfigReq},
};

/* AT发给IMSA模块消息的脱敏处理函数表 */
#if (FEATURE_ON == FEATURE_IMS)
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToImsaMsgListTbl[] =
{
    {ID_AT_IMSA_IMS_CTRL_MSG,                               AT_PrivacyMatchImsaImsCtrlMsg},
    {ID_AT_IMSA_NICKNAME_SET_REQ,                           AT_PrivacyMatchImsaNickNameSetReq},
};
#endif

/**********************************************************************************************/
/***************************** WUEPS_PID_TAF发送消息脱敏函数处理表 *****************************/
/* TAF(WUEPS_PID_TAF)发给AT消息的脱敏处理函数表, A核C核使用同一份代码， */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToAtMsgListTbl[] =
{
#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA)                                      \
  && (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)                       \
  && (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE))
    {ID_TAF_CALL_APP_GET_EC_RANDOM_CNF,                     TAF_XCALL_PrivacyMatchAppGetEcRandomCnf},
    {ID_TAF_CALL_APP_GET_EC_KMC_CNF,                        TAF_XCALL_PrivacyMatchAppGetEcKmcCnf},
#endif

    /* GUC A核C核都有调用，放最外层处理 */
    {MN_CALLBACK_CS_CALL,                                   TAF_PrivacyMatchAppMnCallBackCsCall},
    {MN_CALLBACK_SS,                                        TAF_PrivacyMatchAppMnCallBackSs},
    {ID_TAF_CALL_APP_CNAP_QRY_CNF,                          TAF_CALL_PrivacyMatchAppCnapQryCnf},
    {ID_TAF_CALL_APP_CNAP_INFO_IND,                         TAF_CALL_PrivacyMatchAppCnapInfoInd},

    { MN_CALLBACK_QRY,                                      TAF_PrivacyMatchAtCallBackQryProc},

    { MN_CALLBACK_MSG,                                      TAF_PrivacyMatchAtCallBackSmsProc},
};

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* AT发送给XPDS模块消息脱敏处理函数表 */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToXpdsMsgListTbl[] =
{
    {ID_AT_XPDS_GPS_POS_INFO_RSP,                           AT_PrivacyMatchCagpsPosInfoRsp},
    {ID_AT_XPDS_GPS_PRM_INFO_RSP,                           AT_PrivacyMatchCagpsPrmInfoRsp},
    {ID_AT_XPDS_AP_FORWARD_DATA_IND,                        AT_PrivacyMatchCagpsApForwardDataInd},

};
#endif
#endif

/**********************************************************************************************/
/***************************** UEPS_PID_XSMS发送消息脱敏函数处理表 ****************************/
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* XSMS发给AT消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl[] =
{
    {TAF_XSMS_APP_MSG_TYPE_RCV_IND,                         TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd},
    {TAF_XSMS_APP_MSG_TYPE_WRITE_CNF,                       TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf},
};

/**********************************************************************************************/
/***************************** UEPS_PID_XPDS发送消息脱敏函数处理表 *****************************/
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
/* XPDS发给AT模块消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_XPDS_AT_GPS_REFLOC_INFO_CNF,                        TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf},
    {ID_XPDS_AT_GPS_ION_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsIonInfoInd},
    {ID_XPDS_AT_GPS_EPH_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsEphInfoInd},
    {ID_XPDS_AT_GPS_ALM_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd},
    {ID_XPDS_AT_GPS_PDE_POSI_INFO_IND,                      TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd},
    {ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND,                    TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd},
    {ID_XPDS_AT_AP_REVERSE_DATA_IND,                        TAF_XPDS_PrivacyMatchAtApReverseDataInd},
    {ID_XPDS_AT_UTS_GPS_POS_INFO_IND,                       TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd},
};

#endif
#endif

/* TAF发给TAF消息的脱敏处理函数表 */
/* (由于hi6932无x模，导致该数组定义大小为0，会有pclint告警，gu添加处理后，删除该cdma宏) */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToTafMsgListTbl[] =
{
    /*实际发送PID是TAF*/
    {ID_TAF_CALL_APP_ENCRYPT_VOICE_REQ,                     TAF_PrivacyMatchCallAppEncryptVoiceReq},
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)
    /*实际发送PID是TAF*/
    {ID_TAF_CALL_APP_SET_EC_KMC_REQ,                        TAF_PrivacyMatchCallAppSetEcKmcReq},
#endif
};
#endif
#endif

/* TAF发给MMA消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToMmaMsgListTbl[] =
{
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
    {ID_TAF_MMA_REG_REQ,                                    TAF_MSG_PrivacyMatchMmaRegReqProc},
#endif
    {ID_TAF_MMA_EFLOCIINFO_SET_REQ,                         TAF_MMA_PrivacyMatchAtEfClocInfoSetReq},
    {ID_TAF_MMA_EFPSLOCIINFO_SET_REQ,                       TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq},
};

/* RNIC发给CDS消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astRnicAcorePrivacyMatchToCdsMsgListTbl[] =
{
    {ID_RNIC_CDS_IMS_DATA_REQ,                              RNIC_PrivacyMatchCdsImsDataReq},
};

/* TAF发给AT消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl[] =
{
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF,                   TAF_DSM_PrivacyMatchPsCallPdpActCnf},
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_IND,                   TAF_DSM_PrivacyMatchPsCallPdpActInd},
    {ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND,                     TAF_DSM_PrivacyMatchPsCallPdpManageInd},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_CNF,                     TAF_DSM_PrivacyMatchPsCallPdpModCnf},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_IND,                     TAF_DSM_PrivacyMatchPsCallPdpModInd},
    {ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_CNF,                 TAF_DSM_PrivacyMatchPsCallPdpDeactCnf},
    {ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_IND,                 TAF_DSM_PrivacyMatchPsCallPdpDeactInd},
    {ID_EVT_TAF_PS_CALL_PDP_IPV6_INFO_IND,                  TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd},
    {ID_EVT_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF,           TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf},
    {ID_EVT_TAF_PS_GET_TFT_INFO_CNF,                        TAF_DSM_PrivacyMatchPsGetTftInfoCnf},
    {ID_EVT_TAF_PS_GET_PDP_IP_ADDR_INFO_CNF,                TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf},
    {ID_EVT_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF,   TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf},
    {ID_EVT_TAF_PS_GET_DYNAMIC_TFT_INFO_CNF,                TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf},
    {ID_EVT_TAF_PS_GET_AUTHDATA_INFO_CNF,                   TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf},
    {ID_EVT_TAF_PS_REPORT_PCO_INFO_IND,                     TAF_DSM_PrivacyMatchPsReportPcoInfoInd},
    {ID_EVT_TAF_PS_GET_PDP_DNS_INFO_CNF,                    TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf},
    {ID_EVT_TAF_PS_GET_NEGOTIATION_DNS_CNF,                 TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf},
};

/* MTA发给AT消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                      g_astTafMtaAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_MTA_AT_CPOSR_IND,                                   TAF_MTA_PrivacyMatchCposrInd},
    {ID_MTA_AT_MEID_QRY_CNF,                                TAF_MTA_PrivacyMatchAtMeidQryCnf},
    {ID_MTA_AT_CGSN_QRY_CNF,                                TAF_MTA_PrivacyMatchAtCgsnQryCnf},

    {ID_MTA_AT_ECID_SET_CNF,                                TAF_MTA_PrivacyMatchAtEcidSetCnf},

    {ID_MTA_AT_SET_NETMON_SCELL_CNF,                        TAF_MTA_PrivacyMatchAtSetNetMonScellCnf},
    {ID_MTA_AT_SET_NETMON_NCELL_CNF,                        TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf},
};

/* MMA发给AT消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafMmaAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_TAF_MMA_USIM_STATUS_IND,                            TAF_MMA_PrivacyMatchAtUsimStatusInd},
    {ID_TAF_MMA_HOME_PLMN_QRY_CNF,                          TAF_MMA_PrivacyMatchAtHomePlmnQryCnf},
    {ID_TAF_MMA_LOCATION_INFO_QRY_CNF,                      TAF_MMA_PrivacyMatchAtLocationInfoQryCnf},
    {ID_TAF_MMA_REG_STATUS_IND,                             TAF_MMA_PrivacyMatchAtRegStatusInd},
    {ID_TAF_MMA_SRCHED_PLMN_INFO_IND,                       TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd},
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {ID_TAF_MMA_CDMA_LOCINFO_QRY_CNF,                       TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf},
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */
    {ID_TAF_MMA_NET_SCAN_CNF,                               TAF_MMA_PrivacyMatchAtNetScanCnf},
    {ID_TAF_MMA_REG_STATE_QRY_CNF,                          TAF_MMA_PrivacyMatchAtRegStateQryCnf},
    {ID_TAF_MMA_CLOCINFO_IND,                               TAF_MMA_PrivacyMatchAtClocInfoInd},
    {ID_TAF_MMA_REJINFO_QRY_CNF,                            TAF_MMA_PrivacyMatchAtRejInfoQryCnf},
    {ID_TAF_MMA_EFLOCIINFO_QRY_CNF,                         TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf},
    {ID_TAF_MMA_EFPSLOCIINFO_QRY_CNF,                       TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf},
};


/* TAF发给DSM消息的脱敏处理函数表 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToDsmMsgListTbl[] =
{
    {ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ,                   TAF_DSM_PrivacyMatchTafSetAuthDataReq},
    {ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ,                    TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq},
	{ID_MSG_TAF_PS_CALL_ORIG_REQ,                           TAF_PrivacyMatchDsmPsCallOrigReq},
    {ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ,                       TAF_PrivacyMatchDsmPsPppDialOrigReq},
    {ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ,           TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq},
    {ID_MSG_TAF_PS_SET_TFT_INFO_REQ,                        TAF_PrivacyMatchDsmPsSetTftInfoReq},

};

TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl[] =
{
    {DRV_AGENT_MSID_QRY_CNF,                                TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf},
};

/**********************************************************************************************/
/*************************************** PID映射处理表 ****************************************/
/* WUEPS_PID_AT发送给不同pid的消息对应的脱敏处理表 */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astAtAcorePrivacyMatchRcvPidListTbl[] =
{
    /* AT发送给XPDS的消息过滤 */
#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    {UEPS_PID_XPDS,      sizeof(g_astAtAcorePrivacyMatchToXpdsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),        g_astAtAcorePrivacyMatchToXpdsMsgListTbl},
#endif

    /* AT发送给TAF的消息过滤 */
    {WUEPS_PID_TAF,     sizeof(g_astAtAcorePrivacyMatchToTafMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToTafMsgListTbl},
    /* AT发送给XSMS的消息 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {UEPS_PID_XSMS,     sizeof(g_astAtAcorePrivacyMatchToXsmsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astAtAcorePrivacyMatchToXsmsMsgListTbl},
#endif
    /* AT发送给MTA的消息 */
    {UEPS_PID_MTA,      sizeof(g_astAtAcorePrivacyMatchToMtaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToMtaMsgListTbl},

    {ACPU_PID_RNIC,      sizeof(g_astAtAcorePrivacyMatchToRnicMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),        g_astAtAcorePrivacyMatchToRnicMsgListTbl},

    {PS_PID_APP_NDIS,    sizeof(g_astAtAcorePrivacyMatchToNdisMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),        g_astAtAcorePrivacyMatchToNdisMsgListTbl},

    {WUEPS_PID_DRV_AGENT,      sizeof(g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl},

#if (FEATURE_ON == FEATURE_IMS)
    {PS_PID_IMSA,      sizeof(g_astAtAcorePrivacyMatchToImsaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToImsaMsgListTbl},
#endif

};

/* TAF(WUEPS_PID_TAF)发送给不同pid的消息对应的脱敏处理表 */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astTafAcorePrivacyMatchRcvPidListTbl[] =
{

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)
    /* TAF发给TAF的消息 */
    {WUEPS_PID_TAF,     sizeof(g_astTafAcorePrivacyMatchToTafMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astTafAcorePrivacyMatchToTafMsgListTbl},
#endif
#endif
    /* GUC A核C核都有调用，放最外层处理 */
    {WUEPS_PID_AT,      sizeof(g_astTafAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astTafAcorePrivacyMatchToAtMsgListTbl},

    {WUEPS_PID_MMA,     sizeof(g_astTafAcorePrivacyMatchToMmaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astTafAcorePrivacyMatchToMmaMsgListTbl},

    {UEPS_PID_DSM,      sizeof(g_astTafAcorePrivacyMatchToDsmMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astTafAcorePrivacyMatchToDsmMsgListTbl},
};

/* UEPS_PID_XSMS发送给不同pid的消息对应的脱敏处理表 */
#if(FEATURE_ON == FEATURE_UE_MODE_CDMA)
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astXsmsAcorePrivacyMatchRcvPidListTbl[] =
{
    /* XSMS发送给AT的消息过滤 */
    {WUEPS_PID_AT,      sizeof(g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),      g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl},
};

/* UEPS_PID_XPDS发送给不同pid的消息对应的脱敏处理表 */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astXpdsAcorePrivacyMatchRcvPidListTbl[] =
{
    /* XPDS发送给AT的消息过滤 */
    {WUEPS_PID_AT,      sizeof(g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),      g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl},
};
#endif
#endif

/* MTA发送给不同PID的消息对应的脱敏处理表 */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafMtaAcorePrivacyMatchRcvPidListTbl[] =
{
    /* MTA发给AT的消息 */
    {WUEPS_PID_AT,     sizeof(g_astTafMtaAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafMtaAcorePrivacyMatchToAtMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafMmaAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,     sizeof(g_astTafMmaAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafMmaAcorePrivacyMatchToAtMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,     sizeof(g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl},
};

/* RNIC发送给不同pid的消息对应的脱敏处理表 */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astRnicAcorePrivacyMatchRcvPidListTbl[] =
{
    {UEPS_PID_CDS,     sizeof(g_astRnicAcorePrivacyMatchToCdsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astRnicAcorePrivacyMatchToCdsMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astTafDsmAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,       sizeof(g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),     g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl},
};


LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(
    VOS_UINT32                          ulSrcPid
)
{
    VOS_UINT32                          ulLoop;

    for (ulLoop = 0; ulLoop < sizeof(g_astTafPrivacyMatchModemPidMapTbl)/sizeof(TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU); ulLoop++)
    {
        if ( (ulSrcPid == g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem1Pid)
          || (ulSrcPid == g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem2Pid) )
        {
            return g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem0Pid;
        }
    }

    return ulSrcPid;
}

LOCAL VOS_VOID* AT_PrivacyMatchRnicPdnInfoCfgInd(
    MsgBlock                                               *pstMsg
)
{
    AT_RNIC_PDN_INFO_CFG_IND_STRU      *pstPdnInfoCfgInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstPdnInfoCfgInd = (AT_RNIC_PDN_INFO_CFG_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                     DYNAMIC_MEM_PT,
                                                                     ulLength);

    if (VOS_NULL_PTR == pstPdnInfoCfgInd)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPdnInfoCfgInd,
                  sizeof(AT_RNIC_PDN_INFO_CFG_IND_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    pstPdnInfoCfgInd->ulIpv4Addr = 0;

    TAF_MEM_SET_S(pstPdnInfoCfgInd->aucIpv6Addr,
                  sizeof(pstPdnInfoCfgInd->aucIpv6Addr),
                  0,
                  RNICITF_MAX_IPV6_ADDR_LEN);

    return (VOS_VOID *)pstPdnInfoCfgInd;
}


LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(
    MsgBlock                                               *pstMsg
)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU       *pstPdnInfoCfgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstPdnInfoCfgReq = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                    DYNAMIC_MEM_PT,
                                                                    ulLength);

    if (VOS_NULL_PTR == pstPdnInfoCfgReq)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstPdnInfoCfgReq,
                  sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstPdnInfoCfgReq->stIpv4PdnInfo),
                  sizeof(pstPdnInfoCfgReq->stIpv4PdnInfo),
                  0,
                  sizeof(AT_NDIS_IPV4_PDN_INFO_STRU));

    TAF_MEM_SET_S(&(pstPdnInfoCfgReq->stIpv6PdnInfo),
                  sizeof(pstPdnInfoCfgReq->stIpv6PdnInfo),
                  0,
                  sizeof(AT_NDIS_IPV6_PDN_INFO_STRU));

    return (VOS_VOID *)pstPdnInfoCfgReq;
}


LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(
    MsgBlock                                               *pstMsg
)
{
    AT_NDIS_IFACE_UP_CONFIG_IND_STRU   *pstIfaceUpConfigInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* 分配消息,申请内存后续统一由底层释放 */
    pstIfaceUpConfigInd = (AT_NDIS_IFACE_UP_CONFIG_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulLength);

    if (VOS_NULL_PTR == pstIfaceUpConfigInd)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pstIfaceUpConfigInd,
                  sizeof(AT_NDIS_IFACE_UP_CONFIG_IND_STRU),
                  pstMsg,
                  ulLength);

    /* 将敏感信息设置为全0 */
    TAF_MEM_SET_S(&(pstIfaceUpConfigInd->stIpv4PdnInfo),
                  sizeof(pstIfaceUpConfigInd->stIpv4PdnInfo),
                  0,
                  sizeof(AT_NDIS_IPV4_PDN_INFO_STRU));

    TAF_MEM_SET_S(&(pstIfaceUpConfigInd->stIpv6PdnInfo),
                  sizeof(pstIfaceUpConfigInd->stIpv6PdnInfo),
                  0,
                  sizeof(AT_NDIS_IPV6_PDN_INFO_STRU));

    return (VOS_VOID *)pstIfaceUpConfigInd;
}


LOCAL VOS_UINT32 AT_PrivacyMatchIsWhiteListAtCmd(
    AT_MSG_STRU                        *pstAtMsg
)
{
    VOS_UINT32                          ulLoop;

    for (ulLoop = 0; ulLoop < sizeof(g_aenAtCmdWhiteListTbl)/sizeof(AT_INTER_MSG_ID_ENUM_UINT32); ulLoop++)
    {
        if (g_aenAtCmdWhiteListTbl[ulLoop] == pstAtMsg->enMsgId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_VOID* AT_PrivacyMatchAtCmd(
    MsgBlock                           *pstMsg
)
{
    VOS_UINT8                          *pucAtCmdData            = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usTempAtCmdLen;
    VOS_UINT16                          usPrivacyAtMsgLen;
    VOS_UINT16                          usPrivacyAtCmdLen;
    VOS_UINT16                          usAtMsgHeaderLen;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    /* 白名单中的消息不脱敏, 直接返回原消息, 不在白名单中的信息再进行脱敏检查 */
    if (VOS_TRUE == AT_PrivacyMatchIsWhiteListAtCmd(pstAtMsg))
    {
        return (VOS_VOID *)pstMsg;
    }
    else
    {
        if (pstAtMsg->ucIndex < AT_MAX_CLIENT_NUM)
        {
            if (AT_XML_MODE == g_stParseContext[pstAtMsg->ucIndex].ucMode)
            {
                /* xml模式直接进行过滤 */
                MN_NORM_LOG1("AT_PrivacyMatchAtCmd: TRUE,XML MODE, ulMsgName ", pstAtMsg->enMsgId);
                return VOS_NULL_PTR;
            }

            if (AT_SMS_MODE == g_stParseContext[pstAtMsg->ucIndex].ucMode)
            {
               /* 短信模式直接进行过滤 */
               MN_NORM_LOG1("AT_PrivacyMatchAtCmd: TRUE,SMS MODE ulMsgName ", pstAtMsg->enMsgId);
               return VOS_NULL_PTR;
            }
        }

        for (ulLoop = 0; ulLoop < (sizeof(g_astPrivacyMapCmdToFuncTbl) / sizeof(AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU)); ulLoop++)
        {
            if (pstAtMsg->ucFilterAtType == g_astPrivacyMapCmdToFuncTbl[ulLoop].ulAtCmdType)
            {
                pstPrivacyMatchAtMsg = g_astPrivacyMapCmdToFuncTbl[ulLoop].pcPrivacyAtCmd(pstAtMsg);

                if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
                {
                    break;
                }
                else
                {
                    return (VOS_VOID*) pstPrivacyMatchAtMsg;
                }
            }
        }

        /* 申请足够大的内存, 临时存放AT Cmd, 用于判断是否需要过滤, 使用后释放 */
        usTempAtCmdLen  = (pstAtMsg->usLen > LOG_PRIVACY_AT_CMD_MAX_LEN) ? pstAtMsg->usLen : LOG_PRIVACY_AT_CMD_MAX_LEN;
        pucAtCmdData    = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, usTempAtCmdLen);

        if (VOS_NULL_PTR == pucAtCmdData)
        {
            return VOS_NULL_PTR;
        }

        TAF_MEM_SET_S(pucAtCmdData, usTempAtCmdLen, 0x00, usTempAtCmdLen);
        TAF_MEM_CPY_S(pucAtCmdData, pstAtMsg->usLen, pstAtMsg->aucValue, pstAtMsg->usLen);

        (VOS_VOID)At_UpString(pucAtCmdData, pstAtMsg->usLen);

        for (ulLoop = 0; ulLoop < (sizeof(g_astPrivacyMatchAtCmdMapTbl)/sizeof(AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU)); ulLoop++)
        {
            if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcOriginalAtCmd, pucAtCmdData, VOS_StrLen(g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcOriginalAtCmd)))
            {
                usPrivacyAtCmdLen       = (VOS_UINT16)(VOS_StrLen(g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcPrivacyAtCmd));

                /* 消息结构体长度 + at命令字符串长度 - 消息结构体中aucValue数组长度 */
                usPrivacyAtMsgLen       = sizeof(AT_MSG_STRU) + usPrivacyAtCmdLen - 4;
                /* A核不调用多实例接口申请内存 */
                pstPrivacyMatchAtMsg    = (AT_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                                      DYNAMIC_MEM_PT,
                                                                      usPrivacyAtMsgLen);

                if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
                {
                    /*lint -save -e830*/
                    PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);
                    /*lint -restore*/

                    return VOS_NULL_PTR;
                }

                usAtMsgHeaderLen  = sizeof(AT_MSG_STRU) - 4;

                /* 拷贝原始消息头部 */
                TAF_MEM_CPY_S(pstPrivacyMatchAtMsg,
                              usAtMsgHeaderLen,
                              pstAtMsg,
                              usAtMsgHeaderLen);

                /* 设置新的at cmd字符串长度 */
                pstPrivacyMatchAtMsg->ulLength = usPrivacyAtMsgLen - VOS_MSG_HEAD_LENGTH;
                pstPrivacyMatchAtMsg->usLen    = usPrivacyAtCmdLen;

                /* 拷贝脱敏后at命令字符串 */
                TAF_MEM_CPY_S(pstPrivacyMatchAtMsg->aucValue,
                              usPrivacyAtCmdLen,
                              g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcPrivacyAtCmd,
                              usPrivacyAtCmdLen);

                PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

                return (VOS_VOID *)pstPrivacyMatchAtMsg;
            }
        }

        /* 未匹配, 不包含敏感信息, 返回原始消息 */
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return (VOS_VOID *)pstMsg;
    }
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterCnfCommProc(
    AT_MSG_STRU                        *pstMsg,
    VOS_UINT32                          ulStartIndex,
    VOS_CHAR                            ucEndChar
)
{
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT8                          *pucPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT16                          usPrivacyAtMsgLen;
    VOS_UINT32                          ulIndex;
    VOS_UINT8                          *pucValue = VOS_NULL_PTR;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    ulIndex         = 0;

    /* 判断是查询请求还是回复结果,如果是查询请求则不需要脱敏 */
    if ('\r' != pstAtMsg->aucValue[0])
    {
        return VOS_NULL_PTR;
    }

    usPrivacyAtMsgLen    = sizeof(AT_MSG_STRU) + pstAtMsg->usLen - 4;

    /* A核不调用多实例接口申请内存 */
    pucPrivacyMatchAtMsg = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT,
                                         DYNAMIC_MEM_PT,
                                         usPrivacyAtMsgLen);

    if (VOS_NULL_PTR == pucPrivacyMatchAtMsg)
    {
        return VOS_NULL_PTR;
    }

    pstPrivacyMatchAtMsg = (AT_MSG_STRU *)pucPrivacyMatchAtMsg;

    /* 拷贝原始消息 */
    TAF_MEM_CPY_S(pstPrivacyMatchAtMsg,
                  usPrivacyAtMsgLen,
                  pstAtMsg,
                  usPrivacyAtMsgLen);

    pucValue = pucPrivacyMatchAtMsg + sizeof(AT_MSG_STRU) - 4;

    if (AT_CMD_MAX_LEN < pstPrivacyMatchAtMsg->usLen)
    {
        pstPrivacyMatchAtMsg->usLen = AT_CMD_MAX_LEN;
    }

    for (ulIndex = ulStartIndex; ulIndex < pstPrivacyMatchAtMsg->usLen; ulIndex++)
    {
        if (ucEndChar == pucValue[ulIndex])
        {
            break;
        }

        pucValue[ulIndex] = '*';
    }

    return pstPrivacyMatchAtMsg;
}


LOCAL AT_MSG_STRU* AT_PrivacyFilterCimi(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    从第7位开始替换为*,第0位到第6位不需要替换
    0:\r     1:\n     2-4:MCC      5-6:MNC
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 7, '\r');
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterCgsn(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    从第2位开始替换为*,第0位和第1位不需要替换
    0:\r     1:\n
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 2, '\r');
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterMsid(
    AT_MSG_STRU                        *pstMsg
)
{
    VOS_UINT8                          *pucAtCmdData            = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT16                          usPrivacyAtMsgLen;
    VOS_CHAR                           *pcAtCmd         = "ATI";
    VOS_CHAR                           *pcFilterField   = "IMEI: ";
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulIndex2;
    VOS_UINT16                          usFilterFieldLen;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    /* 申请内存, 临时存放AT Cmd, 用于判断是否需要过滤, 使用后释放 */
    pucAtCmdData    = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, pstAtMsg->usLen);

    if (VOS_NULL_PTR == pucAtCmdData)
    {
        return VOS_NULL_PTR;
    }

    TAF_MEM_CPY_S(pucAtCmdData, pstAtMsg->usLen, pstAtMsg->aucValue, pstAtMsg->usLen);

    (VOS_VOID)At_UpString(pucAtCmdData, pstAtMsg->usLen);

    /* 判断是ATI的查询请求还是回复结果,如果是查询请求则不需要脱敏 */
    if ( VOS_OK == PS_MEM_CMP((VOS_UINT8 *)pcAtCmd, pucAtCmdData, VOS_StrLen(pcAtCmd)))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return VOS_NULL_PTR;
    }

    usPrivacyAtMsgLen    = sizeof(AT_MSG_STRU) + pstAtMsg->usLen - 4;

    /* A核不调用多实例接口申请内存 */
    pstPrivacyMatchAtMsg = (AT_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                       DYNAMIC_MEM_PT,
                                                       usPrivacyAtMsgLen);
    if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return VOS_NULL_PTR;
    }

    /* 拷贝原始消息 */
    TAF_MEM_CPY_S(pstPrivacyMatchAtMsg,
                  usPrivacyAtMsgLen,
                  pstAtMsg,
                  usPrivacyAtMsgLen);

    /****************************************
    只替换IMEI字段后的信息
    ****************************************/
    usFilterFieldLen = (VOS_UINT16)VOS_StrLen(pcFilterField);

    if (usFilterFieldLen > pstPrivacyMatchAtMsg->usLen)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);
        /*lint -save -e516*/
        PS_MEM_FREE(WUEPS_PID_AT, pstPrivacyMatchAtMsg);
        /*lint -restore*/

        return VOS_NULL_PTR;
    }

    if (AT_CMD_MAX_LEN < pstPrivacyMatchAtMsg->usLen)
    {
        pstPrivacyMatchAtMsg->usLen = AT_CMD_MAX_LEN;
    }

    for (ulIndex = 0; ulIndex < (VOS_UINT32)(pstPrivacyMatchAtMsg->usLen - usFilterFieldLen); ulIndex++)
    {
        /* 找出IMEI字段 */
        if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)pcFilterField, &(pstPrivacyMatchAtMsg->aucValue[ulIndex]), usFilterFieldLen))
        {
            /* 将IMEI具体值替换成* */
            for (ulIndex2 = (ulIndex + usFilterFieldLen); ulIndex2 < pstPrivacyMatchAtMsg->usLen; ulIndex2++)
            {
                if ('\r' == pstPrivacyMatchAtMsg->aucValue[ulIndex2])
                {
                    break;
                }

                pstPrivacyMatchAtMsg->aucValue[ulIndex2] = '*';
            }

            break;
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

    return pstPrivacyMatchAtMsg;
}


LOCAL AT_MSG_STRU* AT_PrivacyFilterHplmn(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    从第15位开始替换为*,第0位到第14位不需要替换
    0:\r     1:\n     2-9:^HPLMN:(此处还有一个空格)
    10-12:MCC   13-14:MNC
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 15, ',');
}


VOS_VOID* AT_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstAtMsgPrivacyMatchMsg  = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;
    ulRcvPid        = pstRcvMsgHeader->ulReceiverPid;

    if (WUEPS_PID_AT == pstRcvMsgHeader->ulReceiverPid)
    {
        pstAtMsgPrivacyMatchMsg = AT_PrivacyMatchAtCmd((MsgBlock *)pstMsg);

        return (VOS_VOID *)pstAtMsgPrivacyMatchMsg;
    }

    /* A核单编译, 需要将I1/I2的PID转换为I0的PID */
    ulRcvPid        = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    ulTblSize       = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (ulLoop = 0; ulLoop < (sizeof(g_astAtAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (ulRcvPid == g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

    /* 查找入参消息对应的脱敏函数 */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstAtMsgPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstAtMsgPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl    = VOS_NULL_PTR;
    VOS_VOID                                               *pstTafLogPrivacyMatchMsg    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader             = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* A核单编译, 需要将I1/I2的PID转换为I0的PID */
    ulRcvPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* 根据rcv pid查找pid映射表 */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (ulRcvPid == g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 若根据rcv pid找到pid映射表，继续根据匹配表查找处理函数 */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

        /* 根据msg name查找过滤函数映射表 */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstTafLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstTafLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_XSMS_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstXsmsPrivacyMatchMsg      = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl       = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstMsgHeader                = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulMsgName;

    pstMsgHeader    = (MSG_HEADER_STRU *)pstMsg;
    ulTblSize       = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (ulIndex = 0; ulIndex < (sizeof(g_astXsmsAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulIndex++)
    {
        if (pstMsgHeader->ulReceiverPid == g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].ulTblSize;

            break;
        }
    }

    /* 查找入参消息对应的脱敏函数 */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        ulMsgName = (VOS_UINT16)(pstMsgHeader->ulMsgName);

        for (ulIndex = 0; ulIndex < ulTblSize; ulIndex++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulIndex].ulMsgName)
            {
                pstXsmsPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulIndex].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstXsmsPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}
#endif

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))

VOS_VOID* TAF_XPDS_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstXpdsPrivacyMatchMsg    = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl     = VOS_NULL_PTR;
    PS_MSG_HEADER_STRU                                     *pstMsgHeader              = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;

    pstMsgHeader    = (PS_MSG_HEADER_STRU *)pstMsg;
    ulTblSize       = 0;

    /* 查找入参消息对应的rcvpid表 */
    for (ulLoop = 0; ulLoop < (sizeof(g_astXpdsAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (pstMsgHeader->ulReceiverPid == g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 查找入参消息对应的脱敏函数 */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        ulMsgName = pstMsgHeader->ulMsgName;

        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstXpdsPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstXpdsPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}
#endif


VOS_VOID* TAF_MTA_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                                               *pstMtaLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafMtaAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* 根据ulMsgName查找过滤函数映射表 */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstMtaLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstMtaLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_MMA_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                                               *pstMmaLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafMmaAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* 根据ulMsgName查找过滤函数映射表 */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstMmaLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstMmaLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl       = VOS_NULL_PTR;
    VOS_VOID                                               *pstDrvAgentLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader                = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* 根据ulReceiverPid查找PID映射表 */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 若根据ulReceiverPid找到PID映射表，继续查找匹配表 */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* 根据ulMsgName查找过滤函数映射表 */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstDrvAgentLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstDrvAgentLogPrivacyMatchMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID TAF_OM_LayerMsgLogPrivacyMatchRegAcore(VOS_VOID)
{
     /*  1、AT在PID init时先读取NV, 然后再注册脱敏回调函数, 所以此处直接使用NV值,
       *     log脱敏NV打开, 则注册回调函数, 否则不注册
       *  2、A核单编译, 只能访问I0接口, 所以I1/I2的消息同样注册I0的过滤接口, 在过滤函数内增加PID转换
       *  3、产品线确认NV配置以MODEM_0为准
       */

    if (VOS_TRUE == AT_GetPrivacyFilterEnableFlg())
    {
        PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT,     AT_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(ACPU_PID_RNIC,    RNIC_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (MULTI_MODEM_NUMBER >= 2)
        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (3 == MULTI_MODEM_NUMBER)

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif
#endif
#endif
    }

    return;
}


VOS_VOID* RNIC_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstPrivacyMsg            = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulTblSize       = 0;

    ulRcvPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* 查找入参消息对应的rcvpid表 */
    for (ulLoop = 0; ulLoop < (sizeof(g_astRnicAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (ulRcvPid == g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

    /* 查找入参消息对应的脱敏函数 */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstPrivacyMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstPrivacyMsg;
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_DSM_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl    = VOS_NULL_PTR;
    VOS_VOID                                               *pstTafLogPrivacyMatchMsg    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader             = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulReceiverPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafDsmAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    ulReceiverPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* 根据rcv pid查找pid映射表 */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (ulReceiverPid == g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* 若根据rcv pid找到pid映射表，继续根据匹配表查找处理函数 */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        /* 目前只处理AT消息,后续增加其他模块时需要修改 */
        ulMsgName = ((TAF_PS_EVT_STRU *)pstRcvMsgHeader)->ulEvtId;

        /* 根据msg name查找过滤函数映射表 */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch)
                {
                    pstTafLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);
                    return pstTafLogPrivacyMatchMsg;
                }
            }
        }
    }

    /* 没找到处理函数，直接返回原消息 */
    return (VOS_VOID *)pstMsg;
}



