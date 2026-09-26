/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __IMSA_WIFI_INTERFACE2_H__
#define __IMSA_WIFI_INTERFACE2_H__



/*
 *     1.1 Cplusplus Announce
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 *     #pragma pack(*)    设置字节对齐方式
 */
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*
 *     2 macro
 */

#define IMSA_WIFI_MAX_APN_LEN 99

#define IMSA_WIFI_IPV4_ADDR_LEN 4
#define IMSA_WIFI_IPV6_ADDR_LEN 16

/* added for vowifi emc, begin in 2016-07-15 */
#define IMSA_WIFI_NONCID_SCHEME_URI_LEN 128
#define IMSA_WIFI_CIVICLOC_COUNTRY_LEN 4

#define IMSA_WIFI_CIVICLOC_STATE_LEN 48
#define IMSA_WIFI_CIVICLOC_CITY_LEN 48
#define IMSA_WIFI_CIVICLOC_ZIP_LEN 48
#define IMSA_WIFI_CIVICLOC_RADIUS_LEN 48

#define IMSA_WIFI_CIVICLOC_COUNTRY_A5_LEN 48
#define IMSA_WIFI_CIVICLOC_ROADSTREET_LEN 100
#define IMSA_WIFI_CIVICLOC_OTHER_ADDRESS_LEN 100
#define IMSA_WIFI_COORDINATE_LATITUDE_LEN 16
#define IMSA_WIFI_COORDINATE_LONGITUDE_LEN 16
#define IMSA_WIFI_COORDINATE_ALTITUDE_LEN 16
#define IMSA_WIFI_TIMESTAMP_LEN 24
/* added for vowifi emc, end in 2016-07-15 */
/* for Boston VoWIFI Phase II,begin 2017-04-08 */
#define IMSA_WIFI_EXTENTION_VER_MAXNUM 6
/* for Boston VoWIFI Phase II,end 2017-04-08 */

#define WIFI_IMSA_SSID_LEN 33
#define IMSA_WIFI_MAX_IPV4_EPDG_NUM 2
#define IMSA_WIFI_MAX_IPV6_EPDG_NUM 2
/* for vowifi P-CSCF Restoration 2018-07-25 begin */
#define WIFI_PCSCF_ADDR_MAX_NUM 4
/* for vowifi P-CSCF Restoration 2018-07-25 end */
#define IMSA_WIFI_MAX_APMAC_LEN 16
/*
 *     3 Massage Declare
 */

/*
 *     4 Enum
 */

/*
 * 枚举说明: PDP类型
 */
enum IMSA_WIFI_PdnType {
    IMSA_WIFI_PDN_TYPE_NORMAL = 0x00,
    IMSA_WIFI_PDN_TYPE_EMC = 0x01,

    IMSA_WIFI_PDN_TYPE_BUTT
};
typedef VOS_INT8 IMSA_WIFI_PdnTypeInt8;

enum WIFI_IMSA_MsgId {
    /* WIFI发给IMSA的消息 */
    ID_WIFI_IMSA_POWERON_REQ = 0x0000,            /* _H2ASN_MsgChoice WIFI_IMSA_PoweronReq */
    ID_WIFI_IMSA_POWEROFF_REQ = 0x0001,           /* _H2ASN_MsgChoice WIFI_IMSA_PoweroffReq */
    ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF = 0x0002,   /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnActivateCnf */
    ID_WIFI_IMSA_IMS_PDN_READY_IND = 0x0003,      /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnReadyInd */
    ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_CNF = 0x0004, /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnDeactivateCnf */
    ID_WIFI_IMSA_ERROR_IND = 0x0005,              /* _H2ASN_MsgChoice WIFI_IMSA_ErrorInd */
    ID_WIFI_IMSA_STATE_IND = 0x0006,              /* _H2ASN_MsgChoice WIFI_IMSA_StateInd */
    ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_IND = 0x0007, /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnDeactivateInd */
    ID_IMSA_WIFI_LOCATION_UPDATE_IND = 0x0008,    /* _H2ASN_MsgChoice WIFI_IMSA_LocationUpdateInd */

    ID_WIFI_IMSA_OPEN_VOWIFI_RSP = 0x0009, /* _H2ASN_MsgChoice WIFI_IMSA_OpenVowifiRsp */
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    ID_WIFI_IMSA_ABNORMAL_HOLD_STATE_IND = 0x000A, /*  _H2ASN_MsgChoice WIFI_IMSA_AbnormalHoldStateInd  */
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */

    ID_WIFI_IMSA_VOWIFI_SWITCH_IND = 0x000B, /* _H2ASN_MsgChoice WIFI_IMSA_VowifiSwitchInd */
    ID_WIFI_IMSA_RTP_FAIL_IND = 0x000C,      /* _H2ASN_MsgChoice WIFI_IMSA_RtpFailInd */
    /* for vowifi P-CSCF Restoration 2018-07-25 begin */
    ID_WIFI_IMSA_IMS_PDN_MODIFY_IND = 0X000D,      /* _H2ASN_MsgChoice WIFI_IMSA_IMS_MODIFY_IND_STRU */
    ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF_V1 = 0x000E, /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnActivateCnfV1 */
    /* for vowifi P-CSCF Restoration 2018-07-25 end */
    ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF_V2 = 0x000F, /* _H2ASN_MsgChoice WIFI_IMSA_ImsPdnActivateCnfV2 */

    /* IMSA发给WIFI的消息 */
    ID_IMSA_WIFI_POWERON_CNF = 0x0100,            /* _H2ASN_MsgChoice IMSA_WIFI_PoweronCnf */
    ID_IMSA_WIFI_POWEROFF_CNF = 0x0101,           /* _H2ASN_MsgChoice IMSA_WIFI_PoweroffCnf */
    ID_IMSA_WIFI_IMS_PDN_ACTIVATE_REQ = 0x0102,   /* _H2ASN_MsgChoice IMSA_WIFI_ImsPdnActivateReq */
    ID_IMSA_WIFI_IMS_PDN_DEACTIVATE_REQ = 0x0103, /* _H2ASN_MsgChoice IMSA_WIFI_ImsPdnDeactivateReq */
    ID_IMSA_WIFI_IMS_AUDIO_INFO_IND = 0x0104,     /* _H2ASN_MsgChoice IMSA_WIFI_ImsAudioInfoInd */
    ID_IMSA_WIFI_IMS_VEDIO_INFO_IND = 0x0105,     /* _H2ASN_MsgChoice IMSA_WIFI_IMS_VEDIO_INFO_IND_STRU */

    ID_IMSA_WIFI_OPEN_VOWIFI_IND = 0x0106,  /* _H2ASN_MsgChoice IMSA_WIFI_OpenVowifiInd */
    ID_IMSA_WIFI_CLOSE_VOWIFI_IND = 0x0107, /* _H2ASN_MsgChoice IMSA_WIFI_CloseVowifiInd */

    ID_IMSA_WIFI_CELLULAR_OPEN_IND = 0x0108, /* _H2ASN_MsgChoice IMSA_WIFI_CellularOpenInd */
    /* DCM Project,begin 2017-11-22 */
    ID_IMSA_WIFI_LOCATION_UPDATE_REQ = 0x0109, /* _H2ASN_MsgChoice IMSA_WIFI_LocationUpdateReq */
    /* DCM Project,end 2017-11-22 */

    ID_IMSA_WIFI_DNS_QUERY_NTF = 0x010A, /* _H2ASN_MsgChoice IMSA_WIFI_DnsQueryNtf */

    ID_IMSA_WIFI_EPDG_INFO_IND = 0x010B, /* _H2ASN_MsgChoice IMSA_WIFI_EpdgInfoInd */
    ID_IMSA_WIFI_THRESHOLD_CONF_NTF = 0x010C, /* _H2ASN_MsgChoice IMSA_WIFI_ThresholdConfNtf */
    ID_IMSA_WIFI_RTT_INFO_IND = 0x010D,
    /* NV_Refresh_After_Sim_Switch Project,begin 2017-12-27 */
    ID_IMSA_WIFI_NV_REFRESH_IND = 0x010e, /* _H2ASN_MsgChoice IMSA_WIFI_NvRefreshInd */
    /* NV_Refresh_After_Sim_Switch Project,end 2017-12-27 */
    /* for vowifi P-CSCF Restoration 2018-07-25 begin */
    ID_IMSA_WIFI_IMS_PDN_ACTIVATE_REQ_V1 = 0x010f, /* _H2ASN_MsgChoice IMSA_WIFI_ImsPdnActivateReqV1 */
    /* for vowifi P-CSCF Restoration 2018-07-25 end */
    ID_IMSA_WIFI_DPD_IND = 0x0110,
    ID_WIFI_IMSA_MSG_BUTT
};
typedef VOS_INT32 WIFI_IMSA_MsgIdInt32;

/*
 * 枚举说明: PDP类型
 */
enum IMSA_WIFI_IpType {
    IMSA_WIFI_IP_IPV4 = 0x01,
    IMSA_WIFI_IP_IPV6 = 0x02,
    IMSA_WIFI_IP_IPV4V6 = 0x03,

    IMSA_WIFI_IP_TYPE_BUTT
};
typedef VOS_INT8 IMSA_WIFI_IpTypeInt8;

/*
 * 枚举说明: PDP类型
 */
enum IMSA_WIFI_PdnReleaseType {
    IMSA_WIFI_PDN_RELEASE_TYPE_NW_CMD = 0x00,
    IMSA_WIFI_PDN_RELEASE_TYPE_LOCAL = 0x01,

    IMSA_WIFI_PDN_RELEASE_TYPE_BUTT
};
typedef VOS_INT8 IMSA_WIFI_PdnReleaseTypeInt8;

/* add it for vowifi emc, begin in 2016-07-23 */
/*
 * 枚举说明: PDP类型
 */
enum IMSA_WIFI_ServiceType {
    IMSA_WIFI_NORMAL_SERVICE_IN_NORMAL_PDN = 0x00,
    IMSA_WIFI_EMC_SERVICE_IN_NORMAL_APN,
    IMSA_WIFI_EMC_SERVICE_IN_EMERGECY_APN,

    IMSA_WIFI_SERVICE_TYPE_BUTT
};
typedef VOS_INT8 IMSA_WIFI_ServiceTypeInt8;
/* add it for vowifi emc, end in 2016-07-23 */

/*
 * 枚举说明  : WIFI质量等级枚举
 */
enum WIFI_IMSA_QualityLevel {
    WIFI_IMSA_QUALITY_LEVEL_BAD = 0x00,
    WIFI_IMSA_QUALITY_LEVEL_NORMAL = 0x01,
    WIFI_IMSA_QUALITY_LEVEL_GOOD = 0x02,

    WIFI_IMSA_QUALITY_LEVEL_BUTT
};
typedef VOS_INT8 WIFI_IMSA_QualityLevelInt8;

/* add it for wifi pdn abnormal process, begin in 2016-08-09 */
/*
 * 枚举说明  : WIFI上报的AP服务状态
 */
enum WIFI_IMSA_ServiceStatus {
    WIFI_IMSA_SERVICE_STATUS_NORMAL_SEVICE = 0x00,
    WIFI_IMSA_SERVICE_STATUS_LIMITED_SERVICE = 0x01,
    WIFI_IMSA_SERVICE_STATUS_NO_SERVICE = 0x02,

    WIFI_IMSA_SERVICE_STATUS_BUTT
};
typedef VOS_INT8 WIFI_IMSA_ServiceStatusInt8;

/* add it for wifi pdn abnormal process, end in 2016-08-09 */

/*
 * 枚举说明  : IMS PDN激活结果枚举
 */
enum WIFI_IMSA_PdnActResult {
    WIFI_IMSA_PDN_ACT_RESULT_SUCCESS = 0x00,
    WIFI_IMSA_PDN_ACT_RESULT_PARAM_ERR,        /* 指定参数错误 */
    WIFI_IMSA_PDN_ACT_RESULT_LINKLOST,         /* 链路丢失 */
    WIFI_IMSA_PDN_ACT_RESULT_TIMEOUT,          /* 建立超时 */
    WIFI_IMSA_PDN_ACT_RESULT_MAPCON_STATE_ERR, /* MAPCON状态错误 */
    WIFI_IMSA_PDN_ACT_RESULT_OTHER_FAILURE,    /* 其他错误 */
    WIFI_IMSA_PDN_ACT_RESULT_SIM_AUTH_NO_RESP = 0x06,
    WIFI_IMSA_PDN_ACT_RESULT_AUTH_NETWORK_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_PARSE_NETWORK_PAYLOAD_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_ADD_SA_AP_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_CLIENT_PAYLOAD_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_MSG_NETWORK_OVERTIME_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_ENC_ALG_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_APN_ID_ERROR,

    WIFI_IMSA_PDN_ACT_RESULT_NETWORK_AUTH_SIM_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_STATUS_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_SA_INIT_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED,
    WIFI_IMSA_PDN_ACT_RESULT_USER_UNKNOWN = 0x12,
    WIFI_IMSA_PDN_ACT_RESULT_PLMN_NOT_ALLOWED,
    WIFI_IMSA_PDN_ACT_RESULT_AUTHORIZATION_REJECTED,
    WIFI_IMSA_PDN_ACT_RESULT_RAT_TYPE_NOT_ALLOWED,

    WIFI_IMSA_PDN_ACT_RESULT_NETWORK_FAILURE,
    WIFI_IMSA_PDN_ACT_RESULT_NO_APN_SUBSCRIPTION,
    WIFI_IMSA_PDN_ACT_RESULT_PDN_CONNECTION_REJECTION,
    WIFI_IMSA_PDN_ACT_RESULT_CONNECTION_REACHED,
    WIFI_IMSA_PDN_ACT_RESULT_PROTECLE_ERROR,
    WIFI_IMSA_PDN_ACT_RESULT_INTERNAL_ADDRESS_ERROR,
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    WIFI_IMSA_PDN_ACT_RESULT_IPV4_ADDRESS_CONFLICT = 0x1C, /* 双vowifi下分配相同IPV4地址时错误码 */
    WIFI_IMSA_PDN_ACTIVE_RESULT_CERT_ERROR = 0x1D,         /* 证书验证错误 */
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    WIFI_IMSA_PDN_ACTIVE_RESULT_BEARS_CONCURRENT_NOT_SUPPORT = 0x1E, /* 不支持承载并发 */
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */
    WIFI_IMSA_PDN_ACTIVE_RESULT_SHUTDOWN = 0x1F,
    WIFI_IMSA_PDN_ACTIVE_RESULT_DONE_ERROR = 0x20,
    WIFI_IMSA_PDN_ACTIVE_RESULT_REDIRECT_FAILURE = 0x21,
    /* for WIFI power opt 2018-04-16 begin */
    WIFI_IMSA_PDN_ACTIVE_RESULT_INNER_ERROR_DIR_RETRY = 0x31,
    /* for WIFI power opt 2018-04-16 end */
    /* !!!错误码从这里跳到50了，增删原因值时需要同步修改函数: IMSA_CONN_InitWifiPdnRejTblList   */
    WIFI_IMSA_PDN_ACT_RESULT_ROAM_FORBID = 0x32,
    WIFI_IMSA_PDN_ACT_RESULT_HIFI_STATUS_OVERTIME = 0x33,
    WIFI_IMSA_PDN_ACT_RESULT_IKED_ABNORMAL = 0x34, /* 52, IKED异常，向MAPCON返回23原因值,转义为52 */
    WIFI_IMSA_PDN_ACT_RESULT_UNKNOWN_NETWORK_ERROR = 0x35,
    WIFI_IMSA_PDN_ACT_RESULT_SWITCH_CACHE_MAX_REACHED = 0x36,
    WIFI_IMSA_PDN_ACT_RESULT_APN_MAX_REACHED = 0x37,
    WIFI_IMSA_PDN_ACT_RESULT_REQUEST_REPEATED = 0x38,
    /* !!!增删原因值时需要同步修改函数: IMSA_CONN_InitWifiPdnRejTblList   */
    WIFI_IMSA_PDN_ACT_RESULT_BUTT
};
typedef VOS_INT8 WIFI_IMSA_PdnActResultInt8;

/*
 * 枚举说明  : IMS PDN激活结果枚举(网络原因值)
 */
enum WIFI_IMSA_PdnActNwResult {
    WIFI_IMSA_PDN_ACT_NW_RSLT_SUCCESS = 0,
    WIFI_IMSA_PDN_ACT_NW_RSLT_UNSPTED_CRITICAL_PAYLOAD = 1,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_IKE_SPI = 4,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_MAJOR_VERSION = 5,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_SYNTAX = 7,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_MESSAGE_ID = 9,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_SPI = 11,
    WIFI_IMSA_PDN_ACT_NW_RSLT_NO_PROPOSAL_CHOSEN = 14,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_KE_PAYLOAD = 17,
    WIFI_IMSA_PDN_ACT_NW_RSLT_AUTHENTICATION_FAILED = 24,

    WIFI_IMSA_PDN_ACT_NW_RSLT_SINGLE_PAIR_REQUIRED = 34,
    WIFI_IMSA_PDN_ACT_NW_RSLT_NO_ADDITIONAL_SAS = 35,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INTERNAL_ADDRESS_FAILURE = 36,
    WIFI_IMSA_PDN_ACT_NW_RSLT_FAILED_CP_REQUIRED = 37,
    WIFI_IMSA_PDN_ACT_NW_RSLT_TS_UNACCEPTABLE = 38,
    WIFI_IMSA_PDN_ACT_NW_RSLT_INVALID_SELECTORS = 39,
    WIFI_IMSA_PDN_ACT_NW_RSLT_UNACCEPTABLE_ADDRESSES = 40,   // RFC4555
    WIFI_IMSA_PDN_ACT_NW_RSLT_UNEXPECTED_NAT_DETECTED = 41,  // RFC4555
    WIFI_IMSA_PDN_ACT_NW_RSLT_USE_ASSIGNED_HOA = 42,         // RFC5026

    WIFI_IMSA_PDN_ACT_NW_RSLT_PDN_CONNECTION_REJECTION = 8192,  // R13
    WIFI_IMSA_PDN_ACT_NW_RSLT_MAX_CONNECTION_REACHED = 8193,    // R13

    WIFI_IMSA_PDN_ACT_NW_RSLT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED = 9000,  // R13
    WIFI_IMSA_PDN_ACT_NW_RSLT_USER_UNKNOWN = 9001,                        // R13
    WIFI_IMSA_PDN_ACT_NW_RSLT_NO_APN_SUBSCRIPTION = 9002,                 // R13
    WIFI_IMSA_PDN_ACT_NW_RSLT_AUTHORIZATION_REJECTED = 9003,              // R13

    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_MISSING_OR_UNKNOWN_APN = 9048,
    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_NO_SUBSCRIPTION = 9049,

    WIFI_IMSA_PDN_ACT_NW_RSLT_NETWORK_TOO_BUSY = 10000,
    WIFI_IMSA_PDN_ACT_NW_RSLT_PGW_OVERLOAD = 10003,

    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_REQUEST_REJECTED = 10048,
    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_PROTOCOL_ERROR = 10049,

    WIFI_IMSA_PDN_ACT_NW_RSLT_NETWORK_FAILURE = 10500,  // R13

    WIFI_IMSA_PDN_ACT_NW_RSLT_ROAMING_NOT_ALLOWED = 11000,
    WIFI_IMSA_PDN_ACT_NW_RSLT_RAT_DISALLOWED = 11001,
    WIFI_IMSA_PDN_ACT_NW_RSLT_PERMENENT_FAILURE = 11002,
    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_RAT_DISALLOWED = 11003,

    WIFI_IMSA_PDN_ACT_NW_RSLT_PLMN_NOT_ALLOWED = 11011,  // R13

    WIFI_IMSA_PDN_ACT_NW_RSLT_S2B_UNSUPPORTED_PDN_TYPE = 11049,

    WIFI_IMSA_PDN_ACT_NW_RSLT_BUTT = 16384,  // MAX error
};
typedef VOS_UINT16 WIFI_IMSA_PdnActNwResultUint16;

/*
 * 枚举说明  : IMS PDN激活结果枚举
 */
enum WIFI_IMSA_PdnReactAction {
    WIFI_IMSA_PDN_REACT_ACTION_NO = 0x00,     /* PDN建立异常, MAPCON没有指定后续行为 */
    WIFI_IMSA_PDN_REACT_ACTION_RETRY_NODELAY, /* PDN建立异常, MAPCON指定立刻重试 */
    WIFI_IMSA_PDN_REACT_ACTION_RETRY_DELAY, /* PDN建立异常, MAPCON指定延迟重试,如果ReactTimerLen指出时间,则IMSA使用这个时间长度 */
    WIFI_IMSA_PDN_REACT_ACTION_PERM_REJ, /* PDN建立异常, MAPCON指定永远不要尝试 */

    WIFI_IMSA_PDN_REACT_ACTION_BUTT
};
typedef VOS_INT8 WIFI_IMSA_PdnReactActionInt8;

/*
 * 枚举说明  : IMS PDN激活结果枚举
 */
enum WIFI_IMSA_ErrorCause {
    /* 指示WIFI IMS通道出错 */
    WIFI_IMSA_ERROR_CAUSE_LINK_LOST = 0x00,     /* WIFI链路丢失 */
    WIFI_IMSA_ERROR_CAUSE_SIP_PORT_FAILURE,     /* SIP PORT绑定失败 */
    WIFI_IMSA_ERROR_CAUSE_AUDIO_PORT_FAILURE,   /* MEDIA PORT绑定失败 */
    WIFI_IMSA_ERROR_CAUSE_MAPCON_RESET_FAILURE, /* MAPCON异常 */
    WIFI_IMSA_ERROR_CAUSE_RTT_PORT_FAILURE,     /* rtt port绑定失败 */

    WIFI_IMSA_ERROR_CAUSE_BUTT
};
typedef VOS_INT8 WIFI_IMSA_ErrorCauseInt8;

/*
 * 枚举说明  : IMS PDN去激活原因值枚举
 */
enum WIFI_IMSA_PdnDeactCause {
    WIFI_IMSA_PDN_DEACT_CAUSE_KEEP_LIVE = 0x01, /* 保活包没有回复 */
    WIFI_IMSA_PDN_DEACT_CAUSE_NETWORK_DELETE,   /* 网络主动释放隧道 */
    WIFI_IMSA_PDN_DEACT_CAUSE_IKED_RESET,       /* IKED异常重启 */
    WIFI_IMSA_PDN_DEACT_CAUSE_REKEY_ERROR,      /* REKEY失败 */
    WIFI_IMSA_PDN_DEACT_CAUSE_MAPCON_TEAR,      /* MAPCON自主释放隧道 */
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    WIFI_IMSA_PDN_DEACT_CAUSE_BEARS_CONCURRENT_NOT_SUPPORT, /* 不支持承载并发 */
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */
    /* for vowifi P-CSCF Restoration 2018-07-25 begin */
    WIFI_IMSA_PDN_DEACT_CAUSE_REACTIVATION_REQUESTED, /* PCSCF Restoration */
    /* for vowifi P-CSCF Restoration 2018-07-25 end */

    WIFI_IMSA_PDN_DEACT_CAUSE_BUTT
};
typedef VOS_INT8 WIFI_IMSA_PdnDeactCauseInt8;

/*
 * 枚举说明  : WIFI接入类型枚举
 */
enum WIFI_IMS_AccessType {
    WIFI_IMSA_ACCESS_TYPE_802_11 = 0,
    WIFI_IMSA_ACCESS_TYPE_802_11_a = 1,
    WIFI_IMSA_ACCESS_TYPE_802_11_b = 2,
    WIFI_IMSA_ACCESS_TYPE_802_11_g = 3,
    WIFI_IMSA_ACCESS_TYPE_802_11_n = 4,
    WIFI_IMSA_ACCESS_TYPE_802_11_ac = 5,
    WIFI_IMSA_ACCESS_TYPE_802_11_ad = 6,
    WIFI_IMSA_ACCESS_TYPE_802_11_ah = 7,

    WIFI_IMSA_ACCESS_TYPE_BUTT
};
typedef VOS_INT32 WIFI_IMS_AccessTypeInt32;

/* define power off reason, begin in 2016-09-26 */
/*
 * 枚举说明  : WIFI释放原因类型枚举
 */
enum IMSA_WIFI_PoweroffReason {
    IMSA_WIFI_POWEROFF_REASON_WIFI_POWEROFF = 0,
    IMSA_WIFI_POWEROFF_REASON_SYSTEM_DOWN = 1,
    IMSA_WIFI_POWEROFF_REASON_VOWIFI_STOP = 2,
    IMSA_WIFI_POWEROFF_REASON_ROAMING_FORB = 3,
    IMSA_WIFI_POWEROFF_REASON_SIM_DISABLE = 4,

    IMSA_WIFI_POWEROFF_REASON_BUTT
};
typedef VOS_INT8 IMSA_WIFI_PoweroffReasonInt8;
/* define power off reason, end in 2016-09-26 */
/* for vowifi emc && normal conn concurent,begin 2017-08-11 */
/*
 * 枚举说明  : 隧道保持模式状态，进入/退出
 */
enum IMSA_WIFI_TunnelHoldStatus {
    IMSA_WIFI_TUNNEL_HOLD_STATUS_EXIT = 0x00,  /* 退出隧道模式 */
    IMSA_WIFI_TUNNEL_HOLD_STATUS_ENTER = 0x01, /* 进去隧道保持模式 */
    IMSA_WIFI_TUNNEL_HOLD_STATUS_BUTT
};
typedef VOS_INT8 IMSA_WIFI_TunnelHoldStatusInt8;

/* for vowifi emc && normal conn concurent,end 2017-08-11 */

/* modify for wifi roaming, begin in 2018-03-26 */
/*
 * 枚举说明  : WIFI国际漫游状态
 */
enum IMSA_WIFI_InterRoamingStatus {
    IMSA_WIFI_INTER_ROAMING_STATUS_NOT_SUPPORT = 0x00, /* 不支持特性 */
    IMSA_WIFI_INTER_ROAMING_STATUS_FALSE = 0x01,       /* 没有国际漫游 */
    IMSA_WIFI_INTER_ROAMING_STATUS_TRUE = 0x02,        /* 国际漫游 */

    IMSA_WIFI_INTER_ROAMING_STATUS_BUTT
};
typedef VOS_INT8 IMSA_WIFI_InterRoamingStatusInt8;
/* modify for wifi raoming, end in 2018-03-26 */

/* for vowifi P-CSCF Restoration 2018-07-25 begin */
/*
 * 结构说明: WIFI PDN修改类型
 */
enum WIFI_IMSA_PdnModifyType {
    WIFI_IMSA_PDN_MODIFY_TYPE_NO_MODIFY = 0x0,
    WIFI_IMSA_PDN_MODIFY_TYPE_PCSCF_RESELECTION = 0x01,

    WIFI_IMSA_PDN_MODIFY_TYPE_BUTT
};
typedef VOS_INT8 WIFI_IMSA_PdnModifyTypeInt8;
/* for vowifi P-CSCF Restoration 2018-07-25 end */

/*
 *     5 STRUCT
 */
/*
 * 结构说明  : IP地址的结构体
 */
typedef struct {
    IMSA_WIFI_IpTypeInt8 ipType;
    VOS_INT8 reserved[3];
    VOS_INT8 ipv4Addr[IMSA_WIFI_IPV4_ADDR_LEN];
    VOS_INT8 ipv6Addr[IMSA_WIFI_IPV6_ADDR_LEN];
} IMSA_WIFI_IpAddress;

/*
 * 结构说明  : media信息的结构体
 */
typedef struct {
    VOS_INT32 channelId;
    VOS_INT32 localRTPPort;
    VOS_INT32 localRTCPPort;
    VOS_INT32 remoteRTPPort; /* 0表示尚未获得 */
    VOS_INT32 remoteRTCPPort;
    IMSA_WIFI_IpAddress remoteAddr;
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    IMSA_WIFI_IpAddress localAddr;
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */
} IMSA_WIFI_MediaInfo;

/*
 * 结构说明: APN结构
 */
typedef struct {
    VOS_UINT8 length;
    VOS_UINT8 value[IMSA_WIFI_MAX_APN_LEN];
} IMSA_WIFI_PdpApn;

/*
 * 结构说明: IPv4 DNS地址结构体
 */
typedef struct {
    VOS_INT8 opPriDns;
    VOS_INT8 opSecDns;
    VOS_INT8 opSpare[2];

    VOS_UINT8 priDns[IMSA_WIFI_IPV4_ADDR_LEN];
    VOS_UINT8 secDns[IMSA_WIFI_IPV4_ADDR_LEN];
} IMSA_WIFI_PdpIpv4Dns;

/*
 * 结构说明: IPv4 P-CSCF地址结构体
 */
typedef struct {
    VOS_INT8 opPrimPcscfAddr;
    VOS_INT8 opSecPcscfAddr;
    VOS_INT8 opThiPcscfAddr;
    VOS_INT8 opSpare;

    VOS_UINT8 primPcscfAddr[IMSA_WIFI_IPV4_ADDR_LEN];
    VOS_UINT8 secPcscfAddr[IMSA_WIFI_IPV4_ADDR_LEN];
    VOS_UINT8 thiPcscfAddr[IMSA_WIFI_IPV4_ADDR_LEN];
} IMSA_WIFI_PdpIpv4Pcscf;

/*
 * 结构说明   : IPv6 DNS地址结构体
 */
typedef struct {
    VOS_INT8 opPriDns;
    VOS_INT8 opSecDns;
    VOS_INT8 opSpare[2];

    VOS_UINT8 priDns[IMSA_WIFI_IPV6_ADDR_LEN];
    VOS_UINT8 secDns[IMSA_WIFI_IPV6_ADDR_LEN];
} IMSA_WIFI_PdpIpv6Dns;

/*
 * 结构说明: IPv6 P-CSCF地址结构体
 */
typedef struct {
    VOS_INT8 opPrimPcscfAddr;
    VOS_INT8 opSecPcscfAddr;
    VOS_INT8 opThiPcscfAddr;
    VOS_INT8 opSpare;

    VOS_UINT8 primPcscfAddr[IMSA_WIFI_IPV6_ADDR_LEN];
    VOS_UINT8 secPcscfAddr[IMSA_WIFI_IPV6_ADDR_LEN];
    VOS_UINT8 thiPcscfAddr[IMSA_WIFI_IPV6_ADDR_LEN];
} IMSA_WIFI_PdpIpv6Pcscf;

/*
 * 结构说明: WIFI AP MAC结构体
 */
typedef struct {
    WIFI_IMS_AccessTypeInt32 accessType;
    VOS_CHAR apMac[IMSA_WIFI_MAX_APMAC_LEN];
} IMSA_WIFI_ApMac;

/*
 * 结构说明  : WIFI发给IMSA的开机的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT8 functionVersion; /* 基础版本号，表示AP支持该版本号对应特性以及以下版本号对应特性 */

    /* 扩展版本号主要解决版本跳跃问题:

      问题描述:一个vowifi特性在一个分支A上AP和modem维护的版本号为13，这个特性后面要合入到B分支上，
             而B分支上modem的版本号为10(AP对应的仍是版本号是13)，那么该特性合入到B分支上后,若modem直接给AP侧带
             funcionVersion为13，AP侧会把13和以下的特性全部打开，而modem侧并没有包含11,12对应的版本号的特性，所以
             很容易引起异常

      解决方案:接口中增加扩展版本号，用于表示合入相对于基线版本号有跳跃的vowifi特性对应的版本号。
             例如上面问题，modem给AP报的cFunctionVersion仍是10，会在扩展版本号列表中将13这个版本号带给AP,AP接收到以后
             需要打开10和10以下版本号对应的特性和版本号为13的特性


      接口约束:
        1.下面两个字段为预留字段，目前AP不需要给modem侧带扩展版本号，(即cExtentionNum和cExtentionVersion数组都应置0)
           2017-04-14

    */
    VOS_INT8 extentionNum;                                     /* 有效扩展版本号个数 */
    VOS_INT8 extentionVersion[IMSA_WIFI_EXTENTION_VER_MAXNUM]; /* 扩展版本号，用于表示合入相对于基线版本号有跳跃的vowifi特性对应的版本号 */
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
    VOS_INT8 voWiFiSwitch; /* 1: 代表VoWiFi打开；0:代表VoWiFi关闭  */
    VOS_INT8 rsv[3];
} WIFI_IMSA_PoweronReq;
/*
 * 结构说明  : WIFI发给IMSA的VOWIFI开关Switch状态的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 voWiFiSwitch;      /* 1: 代表VoWiFi打开；0:代表VoWiFi关闭  */
    VOS_INT8 rsv[3];
} WIFI_IMSA_VowifiSwitchInd;

/*
 * 结构说明  : IMSA发给WIFI通知重新发起DNS查询的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 rsv[4];
} IMSA_WIFI_DnsQueryNtf;

/*
 * 结构说明  : IMSA发给WIFI通知重新发起DNS查询的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT16 thresholdWiFiOut; /* 从WiFi域切走的门限 */
    VOS_INT16 thresholdWiFiIn;  /* 切入WiFi域的门限 */
    VOS_INT8 imsaThrshFlag;     /* 1:指示使用IMSA配置门限, 0:代表使用MAPCON内部门限 */
    VOS_INT8 rsv[3];
} IMSA_WIFI_ThresholdConfNtf;

/*
 * 结构说明  : WIFI发给IMSA通知RTP丢包启动切换的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 rsv[4];
} WIFI_IMSA_RtpFailInd;

/*
 * 结构说明  : IMSA回复WIFI的开机的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT8 functionVersion; /* 基础版本号，表示modem支持该版本号对应特性以及以下版本号对应特性 */

    /* 扩展版本号主要解决版本跳跃问题:
    问题描述:一个vowifi特性在一个分支A上AP和modem维护的版本号为13，这个特性后面要合入到B分支上，
           而B分支上modem的版本号为10(AP对应的仍是版本号是13)，那么该特性合入到B分支上后,若modem直接给AP侧带
           funcionVersion为13，AP侧会把13和以下的特性全部打开，而modem侧并没有包含11,12对应的版本号的特性，所以
           很容易引起异常

    解决方案:接口中增加扩展版本号，用于表示合入相对于基线版本号有跳跃的vowifi特性对应的版本号。
           例如上面问题，modem给AP报的cFunctionVersion仍是10，会在扩展版本号列表中将13这个版本号带给AP,AP接收到以后
           需要打开10和10以下版本号对应的特性和版本号为13的特性

    接口约束:
    1.合入存在版本跳跃的特性时才会用到扩展版本号相关字段，其他情况不应携带(即cExtentionNum和cExtentionVersion数组都应置0)
    2.合入存在版本跳跃的特性时，modem在该接口中，需要带有效扩展版本号个数cExtentionNum，并在扩展版本号数组中填充扩展版本号；并明确扩展版本号
      数组中每个元素对应的vowifi具体特性
    3.合入存在版本跳跃的特性时，modem侧通过该接口带下来的cFunctionVersion应是该特性合入前的modem版本号

    4.
       4.1.若modem回复POWER-ON-CNF消息中未携带有效扩展版本号，AP只需匹配基础版本号cFunctionVersion;
           打开cFunctionVersion版本号对应的特性和该版本号以下所有版本号对应的特性，

       4.2.若modem恢复POWER-ON-CNF消息中有携带有效扩展版本号，AP侧需要打开cFunctionVersion版本号对应的特性和该版本号以下所有版本号对应的特性，
           并需要打开扩展版本号对应的特性

    */
    VOS_INT8 extentionNum; /* 有效扩展版本号个数 */

    VOS_INT8 extentionVersion[IMSA_WIFI_EXTENTION_VER_MAXNUM]; /* 扩展版本号，用于表示合入相对于基线版本号有跳跃的vowifi特性对应的版本号 */

    /* for Boston VoWIFI Phase II,end 2017-04-08 */

} IMSA_WIFI_PoweronCnf;

/*
 * 结构说明  : WIFI发给IMSA关机的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;          /* _H2ASN_Skip */
    IMSA_WIFI_PoweroffReasonInt8 reason; /* 0:WiFi关闭；1：系统关闭；2: VoWiFi关闭；3:漫游禁止关闭 */
    VOS_INT8 reserve[3];
} WIFI_IMSA_PoweroffReq;

/*
 * 结构说明  : WIFI发给IMSA关机回复的结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 reserve[4];
} IMSA_WIFI_PoweroffCnf;

/* for vowifi P-CSCF Restoration 2018-07-25 begin */
/*
 * 结构说明: IPv4 P-CSCF地址结构体
 */
typedef struct {
    VOS_UINT8 pcscfAddr[IMSA_WIFI_IPV4_ADDR_LEN];
} WIFI_PDP_Ipv4Pcscf;
typedef struct {
    VOS_INT8 pcscfAddrNum;
    VOS_INT8 opSpare[3];

    WIFI_PDP_Ipv4Pcscf ipv4PcscfAddrList[WIFI_PCSCF_ADDR_MAX_NUM];
} IMSA_WIFI_PdpIpv4PcscfV1;
/*
 * 结构说明: IPv6 P-CSCF地址结构体
 */
typedef struct {
    VOS_UINT8 pcscfAddr[IMSA_WIFI_IPV6_ADDR_LEN];
} WIFI_PDP_Ipv6Pcscf;
typedef struct {
    VOS_INT8 pcscfAddrNum;
    VOS_INT8 opSpare[3];

    WIFI_PDP_Ipv6Pcscf ipv6PcscfAddrList[WIFI_PCSCF_ADDR_MAX_NUM];
} IMSA_WIFI_PdpIpv6PcscfV1;

/*
 * 结构说明: 激活请求结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */

    IMSA_WIFI_IpTypeInt8 ipType;
    VOS_INT8 isHandover;  /* 1：Handover，0：Initial */
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    VOS_INT8 opid;
    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_IpAddress pdpAddr;
    IMSA_WIFI_PdpIpv4Dns dns;
    IMSA_WIFI_PdpIpv4PcscfV1 pcscfV1;
    IMSA_WIFI_PdpIpv6Dns iPv6Dns;
    IMSA_WIFI_PdpIpv6PcscfV1 iPv6PcscfV1;
} IMSA_WIFI_ImsPdnActivateReqV1;
/*
 * 结构说明: WIFI PDN激活结果结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_IpTypeInt8 pdnType;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    WIFI_IMSA_PdnActResultInt8 cause;
    WIFI_IMSA_PdnReactActionInt8 retryAction;

    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_IpAddress pdpAddr;
    IMSA_WIFI_PdpIpv4Dns dns;
    IMSA_WIFI_PdpIpv4PcscfV1 pcscfV1;
    IMSA_WIFI_PdpIpv6Dns iPv6Dns;
    IMSA_WIFI_PdpIpv6PcscfV1 iPv6PcscfV1;
    IMSA_WIFI_ApMac apMac;
    VOS_INT32 pdnReActTimerLen; /* 以秒为单位的重新发起WifiPdn建立过程的定时器 */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT32 wiFiMtu; /* WIFI下带MTU */
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
    VOS_INT32 networkReason;
} WIFI_IMSA_ImsPdnActivateCnfV1;
/*
 * 结构说明: WIFI PDN激活结果结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_IpTypeInt8 pdnType;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    WIFI_IMSA_PdnActResultInt8 cause;
    WIFI_IMSA_PdnReactActionInt8 retryAction;

    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_IpAddress pdpAddr;
    IMSA_WIFI_PdpIpv4Dns dns;
    IMSA_WIFI_PdpIpv4PcscfV1 pcscfV1;
    IMSA_WIFI_PdpIpv6Dns iPv6Dns;
    IMSA_WIFI_PdpIpv6PcscfV1 iPv6PcscfV1;
    IMSA_WIFI_ApMac apMac;
    VOS_INT32 pdnReActTimerLen; /* 以秒为单位的重新发起WifiPdn建立过程的定时器 */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT32 wiFiMtu; /* WIFI下带MTU */
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
    VOS_INT32 networkReason;
    VOS_INT8  opid;
    VOS_INT8  rsv[3];
} WIFI_IMSA_ImsPdnActivateCnfV2;

/*
 * 结构说明: 激活请求结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_IpTypeInt8 pdnType;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    WIFI_IMSA_PdnModifyTypeInt8 modifyType;
    VOS_INT8 rsv;

    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_PdpIpv4PcscfV1 pcscfV1;
    IMSA_WIFI_PdpIpv6PcscfV1 iPv6PcscfV1;
    IMSA_WIFI_ApMac apMac;
} WIFI_IMSA_ImsPdnModifyInd;
/* for vowifi P-CSCF Restoration 2018-07-25 end */

/*
 * 结构说明: 激活请求结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */

    IMSA_WIFI_IpTypeInt8 ipType;
    VOS_INT8 isHandover;  /* 1：Handover，0：Initial */
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    VOS_INT8 reServe;
    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_IpAddress pdpAddr;
    IMSA_WIFI_PdpIpv4Dns dns;
    IMSA_WIFI_PdpIpv4Pcscf pcscf;
    IMSA_WIFI_PdpIpv6Dns iPv6Dns;
    IMSA_WIFI_PdpIpv6Pcscf iPv6Pcscf;
} IMSA_WIFI_ImsPdnActivateReq;

/*
 * 结构说明: WIFI PDN激活结果结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_IpTypeInt8 pdnType;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    WIFI_IMSA_PdnActResultInt8 cause;
    WIFI_IMSA_PdnReactActionInt8 retryAction;

    IMSA_WIFI_PdpApn apn;
    IMSA_WIFI_IpAddress pdpAddr;
    IMSA_WIFI_PdpIpv4Dns dns;
    IMSA_WIFI_PdpIpv4Pcscf pcscf;
    IMSA_WIFI_PdpIpv6Dns iPv6Dns;
    IMSA_WIFI_PdpIpv6Pcscf iPv6Pcscf;
    IMSA_WIFI_ApMac apMac;
    VOS_INT32 pdnReActTimerLen; /* 以秒为单位的重新发起WifiPdn建立过程的定时器 */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT32 wiFiMtu; /* WIFI下带MTU */
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
    VOS_INT32 networkReason;
} WIFI_IMSA_ImsPdnActivateCnf;

/*
 * 结构说明: WIFI链路建立成功结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    VOS_INT8 opid;
    VOS_INT8 reserve[2];
} WIFI_IMSA_ImsPdnReadyInd;

/*
 * 结构说明: WIFI PDN去激活请求结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_PdpApn apn;
    VOS_INT8 isLocal;     /* 1：本地释放，0：与网侧交互 */
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    /* for Boston VoWIFI Phase II,begin 2017-04-08 */
    VOS_INT8 isHandover; /* 指示是否是切换导致的承载释放,1，切换释放，0，普通释放 */
    VOS_INT8 opid;
    /* for Boston VoWIFI Phase II,end 2017-04-08 */
} IMSA_WIFI_ImsPdnDeactivateReq;

/*
 * 结构说明: WIFI PDN去激活结果结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    VOS_INT8 opid;
    VOS_INT8 reserve[2];
} WIFI_IMSA_ImsPdnDeactivateCnf;

/*
 * 结构说明: WIFI PDN去激活结果结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_PdpApn apn;
    VOS_INT8 serviceType;              /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    WIFI_IMSA_PdnDeactCauseInt8 cause; /* 通知当前隧道主动释放的原因 */
    VOS_INT8 reserve[2];
} WIFI_IMSA_ImsPdnDeactivateInd;

/*
 * 结构说明: WIFI状态结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    WIFI_IMSA_QualityLevelInt8 wifiQuality;
    WIFI_IMSA_ServiceStatusInt8 serviceStatus;
    VOS_INT8 reserve[2];
    IMSA_WIFI_ApMac apMac;
    VOS_CHAR ssid[WIFI_IMSA_SSID_LEN];
    VOS_CHAR reserve1[3];
} WIFI_IMSA_StateInd;

/*
 * 结构说明: WIFI错误指示结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    WIFI_IMSA_ErrorCauseInt8 cause;
    VOS_INT8 serviceType; /* 0：普通APN上的普通业务，1：普通APN上的紧急业务, 2: 紧急APN上的紧急业务 */
    VOS_INT8 reserve[2];
} WIFI_IMSA_ErrorInd;

/*
 * 结构说明: IMS audio信息结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 audioInfoCnt;
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    IMSA_WIFI_ServiceTypeInt8 serviceType; /* 0,普通APN上的普通WIFI业务；1，普通APN上的紧急WIFI业务；3，紧急APN上的WIFI紧急业务 */
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */
    VOS_INT8 reserved[2];
    IMSA_WIFI_MediaInfo mediaInfo[4];
} IMSA_WIFI_ImsAudioInfoInd;

/*
 * 结构说明: IMS RTT信息结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 rttInfoCnt;
    /* for vowifi emc && normal conn concurent,begin 2017-08-11 */
    IMSA_WIFI_ServiceTypeInt8 serviceType; /* 0,普通APN上的普通WIFI业务；1，普通APN上的紧急WIFI业务；3，紧急APN上的WIFI紧急业务 */
    /* for vowifi emc && normal conn concurent,end 2017-08-11 */
    VOS_INT8 reserved[2];
    IMSA_WIFI_MediaInfo mediaInfo[4];
} IMSA_WIFI_ImsRttInfoInd;

/*
 * 结构说明: IMS video信息结构体
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 videoInfoCnt;
    VOS_INT8 reserved[3];
    IMSA_WIFI_MediaInfo mediaInfo[2];
} IMSA_WIFI_ImsVideoInfoInd;

/*
 * 结构说明  : IMSA通知MAPCON打开VOWIFI消息结构
 * 消息约束  :
 *     1)IMSA只有在VOWIFI未开机，而IMSA认为需要打开VOWIFI时才使用此消息通知MAPCON；
 *     2)MAPCON在收到此消息时，如果当前WIFI状态一切正常，需要给IMSA下发ID_WIFI_IMSA_POWERON_REQ
 *     消息指示开机，此时无需向IMSA回复ID_WIFI_IMSA_OPEN_VOWIFI_RSP；
 *     3)如果MAPCON因某种异常原因发现当前不能打开VOWIFI，则在收到此消息后，需要给IMSA回复
 *     ID_WIFI_IMSA_OPEN_VOWIFI_RSP通知IMSA打开VOWIFI失败；
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 reserve[4];        /* 保留位 */
} IMSA_WIFI_OpenVowifiInd;

/*
 * 结构说明  : MAPCON通知IMSA打开VOWIFI结果
 * 消息约束  : 当前只有MAPCON在无法打开VOWIFI时才使用此消息回复IMSA
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT32 result;           /* 0 无异常；1 打开WIFI失败 */
} WIFI_IMSA_OpenVowifiRsp;

/*
 * 结构说明  : IMSA通知MAPCON关闭VOWIFI消息结构
 * 消息约束  :
 *     1)IMSA只有向MAPCON发送过ID_IMSA_WIFI_OPEN_VOWIFI_IND才会向MAPCON发送此消息；
 *     2)IMSA只要向MAPCON发送过ID_IMSA_WIFI_OPEN_VOWIFI_IND，除非出现MAPCON向IMSA直接下发
 *     ID_WIFI_IMSA_POWEROFF_REQ(特殊场景，用户在打video时直接把整个WIFI关闭)，其他情况
 *     IMSA在后续一定会在某个不需要VOWIFI的时间向MAPCON发送此消息；
 *     3)IMSA向MAPCON上报此消息后，MAPCON如果同意关闭VOWIFI，则会向IMSA下发ID_WIFI_IMSA_POWEROFF_REQ，
 *     如果不同意关闭VOWIFI，则可以选择不向IMSA下发任何消息；
 *     (特殊场景，如用户打video期间又手动打开VOWIFI，此处的判定逻辑由MAPCON保证)
 *     4)如果之前IMSA向MAPCON发送过ID_IMSA_WIFI_OPEN_VOWIFI_IND，但是在收到MAPCON下发
 *     ID_WIFI_IMSA_POWERON_REQ之前，IMSA又想关闭VOWIFI，此时IMSA只要直接向MAPCON上报此消息即可，
 *     后续IMSA和MAPCON都会按收到的消息顺序进行处理；
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 reserve[4];        /* 保留位 */
} IMSA_WIFI_CloseVowifiInd;

/* 2017-08-31 begin */
/*
 * 结构说明  : IMSA通知MAPCON当前Celluar开机消息结构
 * 消息约束  :
 *     1)IMSA只有在Cellular开机才使用此消息通知MAPCON;
 *     2)MAPCON在收到此消息时，如果当前WIFI状态和隧道状态不一致,需要本地处理WiFi状态和WiFi隧道状态
 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 wiFiStatus;        /* 1: WiFi On; 0:WiFi Off */
    VOS_INT8 wiFiTunnelStatus;  /* 1: WiFi Tunnel Exist; 0:WiFi Tunnel non-exist */
    VOS_INT8 reserve[2];        /* 保留位 */
} IMSA_WIFI_CellularOpenInd;
/* 2017-08-31 end */
/* DCM Project,begin 2017-11-22 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 reserve[4];        /* 保留位 */
} IMSA_WIFI_LocationUpdateReq;
/* DCM Project,end 2017-11-22 */
/* NV_Refresh_After_Sim_Switch Project,begin 2017-12-27 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId; /* _H2ASN_Skip */
    VOS_INT8 reserve[4];        /* 保留位 */
} IMSA_WIFI_NvRefreshInd;
/* NV_Refresh_After_Sim_Switch Project,end 2017-12-27 */
typedef struct {
    VOS_UINT8 ipv4Addr[IMSA_WIFI_IPV4_ADDR_LEN];
} IMSA_WIFI_Ipv4Epdg;

typedef struct {
    VOS_UINT8 ipv6Addr[IMSA_WIFI_IPV6_ADDR_LEN];
} IMSA_WIFI_Ipv6Epdg;

/*
 * 结构说明: IMSA通知wifi网侧携带的EPDG地址信息结构体
 */
typedef struct {
    VOS_UINT16 ipv4EpdgNum;
    VOS_UINT16 ipv6EpdgNum;
    IMSA_WIFI_Ipv4Epdg ipv4EpdgList[IMSA_WIFI_MAX_IPV4_EPDG_NUM];
    IMSA_WIFI_Ipv6Epdg ipv6EpdgList[IMSA_WIFI_MAX_IPV6_EPDG_NUM];
} IMSA_WIFI_EpdgInfo;

typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_EpdgInfo epdgInfo;
} IMSA_WIFI_EpdgInfoInd;

/*
 * 结构说明: IMS位置信息信息结构体
 */

typedef struct {
    VOS_CHAR country[IMSA_WIFI_CIVICLOC_COUNTRY_LEN]; /* 国家码信息 */
    VOS_CHAR state[IMSA_WIFI_CIVICLOC_STATE_LEN];     /* 州信息 */
    VOS_CHAR city[IMSA_WIFI_CIVICLOC_CITY_LEN];       /* 市信息 */
    VOS_CHAR zip[IMSA_WIFI_CIVICLOC_ZIP_LEN];         /* 邮编信息 */
    VOS_CHAR radius[IMSA_WIFI_CIVICLOC_RADIUS_LEN];   /* radius信息 */
    VOS_CHAR countryA5[IMSA_WIFI_CIVICLOC_COUNTRY_A5_LEN];
    VOS_CHAR roadStreet[IMSA_WIFI_CIVICLOC_ROADSTREET_LEN]; /* 街道信息 */
    VOS_CHAR otherAddress[IMSA_WIFI_CIVICLOC_OTHER_ADDRESS_LEN];
} IMSA_WIFI_CivicLocInfo;

typedef struct {
    VOS_CHAR latitude[IMSA_WIFI_COORDINATE_LATITUDE_LEN];
    VOS_CHAR longitude[IMSA_WIFI_COORDINATE_LONGITUDE_LEN];
    VOS_CHAR altitude[IMSA_WIFI_COORDINATE_ALTITUDE_LEN];
} IMSA_WIFI_CoordinateLocInfo;

typedef struct {
    VOS_UINT32 msgId;
    VOS_INT8 isCivicLocInfoExist;
    VOS_INT8 isCoordinateLocInfoExist;
    /* modify for wifi roaming, begin in 2018-03-26 */
    IMSA_WIFI_InterRoamingStatusInt8 wifiInterRoamingFlag; /* 0:不支持特性；1:没有国际漫游； 2:国际漫游 */
    VOS_INT8 reserved;
    /* modify for wifi raoming, end in 2018-03-26 */
    IMSA_WIFI_CivicLocInfo civicLocInfo;
    IMSA_WIFI_CoordinateLocInfo coordinateLocInfo;
    VOS_CHAR retentionExpiry[IMSA_WIFI_TIMESTAMP_LEN];
    VOS_CHAR timeStamp[IMSA_WIFI_TIMESTAMP_LEN];
} WIFI_IMSA_LocationUpdateInd;

/*
 * 结构说明: WIFI-IMSA间的结构体
 */

typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 reserve[4];
} WIFI_IMSA_Msg;

/* for vowifi emc && normal conn concurent,begin 2017-08-11 */
/*
 * 结构说明: WIFI丢网,MAPCON通知IMSA进入或退出隧道保持消息
 */
/* 接口调整，将保留字段放到结构体int8类型后面 */
typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    IMSA_WIFI_TunnelHoldStatusInt8 abnormalState; /* 1，表示进入隧道保持模式，0，表示退出隧道模式 */
    VOS_INT8 reserve;
    VOS_INT16 abNormalTime;
} WIFI_IMSA_AbnormalHoldStateInd;
/* for vowifi emc && normal conn concurent,end 2017-08-11 */

typedef struct {
    WIFI_IMSA_MsgIdInt32 msgId;
    VOS_INT8 reserve[4];
} IMSA_WIFI_DpdInd;
/*
 *     6 UNION
 */

/*
 *     7 Extern Global Variable
 */

/*
 *     8 Fuction Extern
 */

/*
 *     9 OTHERS
 */

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __IMSA_WIFI_INTERFACE_H__
