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

#ifndef _APS_L4A_INTERFACE_H_
#define _APS_L4A_INTERFACE_H_

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4200) /* zero-sized array in struct/union */
#endif

#pragma pack(push, 4)

#define APS_L4A_MAX_IPV6_PREFIX_NUM 6 /* 最大IPv6地址前缀数量 */

#define APS_L4A_MAX_APN_LEN 99 /* APN最大长度 */
#define APS_L4A_MAX_IPV4_ADDR_LEN 4
#define APS_L4A_MAX_IPV6_ADDR_LEN 16

#define APS_L4A_MAX_ACCESS_NUM_LEN 32 /* L模定义, 暂不使用 */
#define APS_L4A_MAX_USERNAME_LEN 255  /* 鉴权用户名长度 */
#define APS_L4A_MAX_PASSWORD_LEN 255  /* 鉴权密码长度 */

#define APS_L4A_MAX_SDF_PF_NUM 16

#define APS_L4A_LTE_MAX_CID 31

#define APS_L4A_AUTHDATA_MAX_USERNAME_LEN 127
#define APS_L4A_AUTHDATA_MAX_PASSWORD_LEN 127

/*
 * Maximum length of challenge used in authentication. The maximum length
 * challenge name is the same as challenge.
 */
#define APS_L4A_PPP_CHAP_CHALLNGE_LEN 16

/*
 * Maximum length of challenge name used in authentication.
 */
#define APS_L4A_PPP_CHAP_CHALLNGE_NAME_LEN 16

/*
 * Maximum length of response used in authentication.
 */
#define APS_L4A_PPP_CHAP_RESPONSE_LEN 16

/*
 * Maximum length of username used in authentication. It should be equal to
 * the AUTHLEN(include one byte length) defined in ppp_interface.h
 */
#define APS_L4A_PPP_MAX_USERNAME_LEN 99

/*
 * Maximum length of password used in authentication. It should be equal to
 * the PASSWORDLEN(include one byte length) defined in ppp_interface.h
 */
#define APS_L4A_PPP_MAX_PASSWORD_LEN 99


enum APS_L4A_MsgId {
    /* TAF->MSP_L4_L4A */
    /* _MSGPARSE_Interception APS_L4A_GetLteCsReq */
    ID_APS_L4A_GET_LTE_CS_REQ = 0x000102A4,
    /* _MSGPARSE_Interception APS_L4A_GetCemodeReq */
    ID_APS_L4A_GET_CEMODE_REQ = 0x000102A6,
    /* MSP_L4_L4A->TAF */
    /* _H2ASN_MsgChoice APS_L4A_GetLteCsCnf */
    ID_L4A_APS_GET_LTE_CS_CNF = 0x1018,
    /* _H2ASN_MsgChoice APS_L4A_GetCemodeCnf */
    ID_L4A_APS_GET_CEMODE_CNF = 0x101d,

    ID_APS_L4A_MSG_BUTT
};
typedef VOS_UINT32 APS_L4A_MsgIdUint32;


enum APS_L4A_PdpType {
    APS_L4A_PDP_IPV4   = 0x01, /* IPV4类型 */
    APS_L4A_PDP_IPV6   = 0x02, /* IPV6类型 */
    APS_L4A_PDP_IPV4V6 = 0x03, /* IPV4V6类型 */
    APS_L4A_PDP_PPP    = 0x04, /* PPP类型 */

    APS_L4A_PDP_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_PdpTypeUint8;


enum APS_L4A_PdpDataComp {
    APS_L4A_PDP_DATA_COMP_OFF = 0x00, /* default if value is omitted */
    /* manufacturer preferred compression */
    APS_L4A_PDP_DATA_COMP_ON  = 0x01,
    APS_L4A_PDP_DATA_COMP_V42 = 0x02, /* V42压缩算法 */

    APS_L4A_PDP_DATA_COMP_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_PdpDataCompUint8;


enum APS_L4A_PdpHeadComp {
    APS_L4A_PDP_HEAD_COMP_OFF = 0x00, /* default if value is omitted */
    /* manufacturer preferred compression */
    APS_L4A_PDP_HEAD_COMP_ON = 0x01,
    /* RFC1144压缩算法，applicable for SNDCP only */
    APS_L4A_PDP_HEAD_COMP_RFC1144 = 0x02,
    /* RFC2507，applicable for SNDCP only */
    APS_L4A_PDP_HEAD_COMP_RFC2507 = 0x03,

    APS_L4A_PDP_HEAD_COMP_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_PdpHeadCompUint8;


enum APS_L4A_TrafficClass {
    APS_L4A_TRAFFIC_CONVERSATIONAL_CLASS = 0x00, /* Conversational class */
    APS_L4A_TRAFFIC_STREAMING_CLASS      = 0x01, /* Streaming class */
    APS_L4A_TRAFFIC_BACKGROUND_CLASS     = 0x02, /* Interactive class */
    APS_L4A_TRAFFIC_SUBSCRIBED_VALUE     = 0x03, /* Background class */

    APS_L4A_TRAFFIC_CLASS_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_TrafficClassUint8;


enum APS_L4A_DeliverOrder {
    APS_L4A_DELIVER_WITHOUT_DELIVER_ORDER = 0x00, /* 0 : Without delivery order */
    APS_L4A_DELIVER_WITH_DELIVER_ORDER    = 0x01, /* 1 : With delivery order */
    APS_L4A_DELIVER_SUBSCRIBED_VAL        = 0x02, /* 2 : Subscribed value */

    APS_L4A_DELIVER_ORDER_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_DeliverOrderUint8;


enum APS_L4A_ErrSduDelivered {
    /* 0 : Erroneous SDUs are not delivered */
    APS_L4A_ERR_SDU_NOT_DELIVERED  = 0x00,
    APS_L4A_ERR_SDU_DELIVERED      = 0x01, /* 1 : Erroneous SDUs are delivered */
    APS_L4A_ERR_SDU_NOT_DETECT     = 0x02, /* 2 : No detect */
    APS_L4A_ERR_SDU_SUBSCRIBED_VAl = 0x03, /* 3 : subscribed value */

    APS_L4A_ERR_SDU_DELIVERED_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_ErrSduDeliveredUint8;


enum APS_L4A_ResidualBer {
    APS_L4A_RESIDUAL_BER_SUBSCRIBED_VAL = 0x00, /* 0 : subscribed value */
    APS_L4A_RESIDUAL_BER_5E2            = 0x01, /* 1 : 5*10-2 */
    APS_L4A_RESIDUAL_BER_1E2            = 0x02, /* 2 : 1*10-2 */
    APS_L4A_RESIDUAL_BER_5E3            = 0x03, /* 3 : 5*10-3 */
    APS_L4A_RESIDUAL_BER_4E3            = 0x04, /* 4 : 4*10-3 */
    APS_L4A_RESIDUAL_BER_1E3            = 0x05, /* 5 : 1*10-3 */
    APS_L4A_RESIDUAL_BER_1E4            = 0x06, /* 6 : 1*10-4 */
    APS_L4A_RESIDUAL_BER_1E5            = 0x07, /* 7 : 1*10-5 */
    APS_L4A_RESIDUAL_BER_1E6            = 0x08, /* 8 : 1*10-6 */
    APS_L4A_RESIDUAL_BER_6E8            = 0x09, /* 9 : 6*10-8 */

    APS_L4A_RESIDUAL_BER_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_ResidualBerUint8;


enum APS_L4A_SduErrRatio {
    APS_L4A_SDU_ERR_RATIO_SUBSCRIBED_VAL = 0x00, /* 0 : subscribed value */
    APS_L4A_SDU_ERR_RATIO_1E2            = 0x01, /* 1 : 5*10-2 */
    APS_L4A_SDU_ERR_RATIO_7E3            = 0x02, /* 2 : 1*10-2 */
    APS_L4A_SDU_ERR_RATIO_1E3            = 0x03, /* 3 : 5*10-3 */
    APS_L4A_SDU_ERR_RATIO_1E4            = 0x04, /* 4 : 4*10-3 */
    APS_L4A_SDU_ERR_RATIO_1E5            = 0x05, /* 5 : 1*10-3 */
    APS_L4A_SDU_ERR_RATIO_1E6            = 0x06, /* 6 : 1*10-4 */
    APS_L4A_SDU_ERR_RATIO_1E1            = 0x07, /* 7 : 1*10-5 */

    APS_L4A_SDU_ERR_RATIO_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_SduErrRatioUint8;


enum APS_L4A_TrafficHandlePrio {
    APS_L4A_TRAFFIC_HANDLE_PRIO_SUBSCRIBED = 0x00, /* 0 : Subscribed value */
    APS_L4A_TRAFFIC_HANDLE_PRIO_LEV1       = 0x01, /* 1 : Priority level 1 */
    APS_L4A_TRAFFIC_HANDLE_PRIO_LEV2       = 0x02, /* 2 : Priority level 2 */
    APS_L4A_TRAFFIC_HANDLE_PRIO_LEV3       = 0x03, /* 3 : Priority level 3 */

    APS_L4A_TRAFFIC_HANDLE_PRIO_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_TrafficHandlePrioUint8;


enum APS_L4A_PdpState {
    APS_L4A_PDP_STATE_INACTIVE = 0x00, /* PDP没激活 */
    APS_L4A_PDP_STATE_ACTIVE   = 0x01, /* PDP已激活 */

    APS_L4A_PDP_STATE_BUTT = 0xFF
};
typedef VOS_UINT32 APS_L4A_PdpStateUint32;


enum APS_L4A_TftFilter {
    /* 0 - Pre-Release 7 TFT filter (see 3GPP TS 24.008 [8], table 10.5.162) */
    APS_L4A_TFT_FILTER_PRE_REL7 = 0x00,
    APS_L4A_TFT_FILTER_UPLINK   = 0x01, /* 1 - Uplink */
    APS_L4A_TFT_FILTER_DOWNLINK = 0x02, /* 2 - Downlink */
    /* 3 - Birectional (Up & Downlink) (default if omitted) */
    APS_L4A_TFT_FILTER_BIDIRECTION = 0x03,

    APS_L4A_TFT_FILTER_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_TftFilterUint8;


enum APS_L4A_Ipv4AddrAllocType {
    APS_L4A_IPV4_ADDR_ALLOC_TYPE_NAS_SIGNALING = 0x00,
    APS_L4A_IPV4_ADDR_ALLOC_TYPE_DHCP          = 0x01,

    APS_L4A_IPV4_ADDR_ALLOC_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_Ipv4AddrAllocTypeUint8;


enum APS_L4A_BearerType {
    APS_L4A_BEARER_TYPE_DEFAULT   = 0x00, /* 缺省承载 */
    APS_L4A_BEARER_TYPE_DEDICATED = 0x01, /* 专有承载 */

    APS_L4A_BEARER_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_BearerTypeUint8;


enum APS_L4A_GwAuthType {
    APS_L4A_GW_AUTH_TYPE_NONE = 0x00,
    APS_L4A_GW_AUTH_TYPE_PAP  = 0x01,
    APS_L4A_GW_AUTH_TYPE_CHAP = 0x02,

    APS_L4A_GW_AUTH_TYPE_BUTT = 0xFF
};
typedef VOS_UINT32 APS_L4A_GwAuthTypeUint32;


enum APS_L4A_BearerManageType {
    APS_L4A_BEARER_MANAGE_TYPE_ACCEPT = 0x00, /* 接受 */
    APS_L4A_BEARER_MANAGE_TYPE_REJ    = 0x01, /* 拒绝 */

    APS_L4A_BEARER_MANANGE_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_BearerManageTypeUint8;


enum APS_L4A_BearerManageMode {
    APS_L4A_BEARER_MANAGE_MODE_AUTO   = 0x00, /* 自动 */
    APS_L4A_BEARER_MANAGE_MODE_MANUAL = 0x01, /* 手动 */

    APS_L4A_BEARER_MANAGE_MODE_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_BearerManageModeUint8;


enum APS_L4A_AuthType {
    APS_L4A_AUTH_TYPE_NONE = 0x00,
    /* _H2ASN_IeChoice_Export APS_L4A_PAP_PARAM_STRU */
    APS_L4A_AUTH_TYPE_PAP = 0x01,
    /* _H2ASN_IeChoice_Export APS_L4A_CHAP_PARAM_STRU */
    APS_L4A_AUTH_TYPE_CHAP = 0x02,
    APS_L4A_AUTH_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 APS_L4A_AuthTypeUint8;


enum APS_L4A_EmcInd {
    APS_L4A_PDP_NOT_FOR_EMC = 0x00,
    APS_L4A_PDP_FOR_EMC     = 0x01,

    APS_L4A_EMC_IND_BUTT
};
typedef VOS_UINT8 APS_L4A_EmcIndUint8;


enum APS_L4A_PcscfDiscovery {
    APS_L4A_PCSCF_DISCOVERY_NOT_INFLUENCED  = 0x00,
    APS_L4A_PCSCF_DISCOVERY_THROUGH_NAS_SIG = 0x01,
    APS_L4A_PCSCF_DISCOVERY_THROUGH_DHCP    = 0x02,

    APS_L4A_PCSCF_DISCOVERY_BUTT
};
typedef VOS_UINT8 APS_L4A_PcscfDiscoveryUint8;


enum APS_L4A_ImsCnSigFlag {
    APS_L4A_PDP_NOT_FOR_IMS_CN_SIG_ONLY = 0x00,
    APS_L4A_PDP_FOR_IMS_CN_SIG_ONLY     = 0x01,

    APS_L4A_IMS_CN_SIG_FLAG_BUTT
};
typedef VOS_UINT8 APS_L4A_ImsCnSigFlagUint8;


enum APS_L4A_PdnRequestType {
    /* initial request: when the MS is establishing connectivity to an additional PDN for the first time */
    APS_L4A_PDN_REQUEST_TYPE_INITIAL = 0x1,

    /*
     * Handover: when the connectivity to a PDN is established upon handover from a non-3GPP access network
     * and the MS was connected to that PDN before the handover to the 3GPP access network
     */
    APS_L4A_PDN_REQUEST_TYPE_HANDOVER = 0x2,

    /* Unused. If received, the network shall interpret this as "initial request". */
    APS_L4A_PDN_REQUEST_TYPE_UNUSED = 0x3,

    /* emergency: the network shall use the APN or the GGSN/PDN GW configured for emergency bearer services. */
    APS_L4A_PDN_REQUEST_TYPE_EMERGENCY = 0x4,

    APS_L4A_PDN_REQUEST_TYPE_BUTT
};

typedef VOS_UINT8 APS_L4A_PdnRequestTypeUint8;


typedef struct {
    /* 针对AT或者DIAG的多应用 L4A发消息给该PID */
    VOS_UINT32 pid;
    VOS_UINT32 clientId; /* 多AT窗口 */
    /* 操作字，针对多次请求，每次请求加一,让用户结合 */
    VOS_UINT32 opId;
} APS_L4A_Appctrl;


typedef struct {
    APS_L4A_GwAuthTypeUint32 gwAuthType;
    VOS_UINT8                accNumLen;
    VOS_UINT8                rsv1[3];
    VOS_UINT8                accessNum[APS_L4A_MAX_ACCESS_NUM_LEN]; /* 此参数保留，暂时不使用 */
    VOS_UINT8                userNameLen;
    VOS_UINT8                rsv2[3];
    VOS_UINT8                userName[APS_L4A_MAX_USERNAME_LEN];
    VOS_UINT8                pwdLen;
    VOS_UINT8                rsv3[1];
    VOS_UINT8                pwd[APS_L4A_MAX_PASSWORD_LEN];
} APS_L4A_GwAuthInfo;


typedef struct {
    VOS_UINT8 apnLen;
    VOS_UINT8 apnName[APS_L4A_MAX_APN_LEN];
} APS_L4A_ApnInfo;


typedef struct {
    APS_L4A_BearerManageModeUint8 mode;        /* 承载管理模式 */
    APS_L4A_BearerManageTypeUint8 type;        /* 承载管理类型 */
    VOS_UINT8                     reserved[2]; /* 四字节对齐，保留 */
} APS_L4A_PdpManageInfo;


typedef struct {
    /*
     * 0 QCI is selected by network
     * [1 - 4]value range for guranteed bit rate Traffic Flows
     * [5 - 9]value range for non-guarenteed bit rate Traffic Flows
     */
    VOS_UINT8 qci;

    VOS_UINT8 reserved[3];

    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlGbr;

    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulGbr;

    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlMbr;

    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulMbr;

} APS_L4A_EpsQos;


typedef struct {
    VOS_UINT32 opRmtIpv4AddrAndMask : 1;
    VOS_UINT32 opRmtIpv6AddrAndMask : 1;
    VOS_UINT32 opProtocolId : 1;
    VOS_UINT32 opSingleLocalPort : 1;
    VOS_UINT32 opLocalPortRange : 1;
    VOS_UINT32 opSingleRemotePort : 1;
    VOS_UINT32 opRemotePortRange : 1;
    VOS_UINT32 opSecuParaIndex : 1;
    VOS_UINT32 opTypeOfService : 1;
    VOS_UINT32 opFlowLabelType : 1;
    VOS_UINT32 opLocalIpv4AddrAndMask : 1;
    VOS_UINT32 opLocalIpv6AddrAndMask : 1;
    VOS_UINT32 opSpare : 20;

    VOS_UINT8 packetFilterId;
    VOS_UINT8 nwPacketFilterId;
    VOS_UINT8 direction;
    VOS_UINT8 precedence; /* packet filter evaluation precedence */

    VOS_UINT32 secuParaIndex; /* SPI */
    VOS_UINT16 singleLcPort;
    VOS_UINT16 lcPortHighLimit;
    VOS_UINT16 lcPortLowLimit;
    VOS_UINT16 singleRmtPort;
    VOS_UINT16 rmtPortHighLimit;
    VOS_UINT16 rmtPortLowLimit;
    VOS_UINT8  protocolId;        /* 协议号 */
    VOS_UINT8  typeOfService;     /* TOS */
    VOS_UINT8  typeOfServiceMask; /* TOS Mask */
    VOS_UINT8  reserved[1];

    VOS_UINT8 rmtIpv4AddreTAFss[APS_L4A_MAX_IPV4_ADDR_LEN];
    /*
     * ucSourceIpAddress[0]为IP地址高字节位
     * ucSourceIpAddress[3]为低字节位
     */
    VOS_UINT8 rmtIpv4Mask[APS_L4A_MAX_IPV4_ADDR_LEN];
    /*
     * ucSourceIpMask[0]为IP地址高字节位 ,
     * ucSourceIpMask[3]为低字节位
     */
    VOS_UINT8 rmtIpv6Address[APS_L4A_MAX_IPV6_ADDR_LEN];
    /*
     * ucRmtIpv6Address[0]为IPv6接口标识高字节位
     * ucRmtIpv6Address[7]为IPv6接口标识低字节位
     */
    VOS_UINT8 rmtIpv6Mask[APS_L4A_MAX_IPV6_ADDR_LEN];
    /*
     * ucRmtIpv6Mask[0]为高字节位
     * ucRmtIpv6Mask[7]为低字节位
     */

    VOS_UINT32 flowLabelType; /* FlowLabelType */

    VOS_UINT8 localIpv4Addr[APS_L4A_MAX_IPV4_ADDR_LEN];
    VOS_UINT8 localIpv4Mask[APS_L4A_MAX_IPV4_ADDR_LEN];
    VOS_UINT8 localIpv6Addr[APS_L4A_MAX_IPV6_ADDR_LEN];
    VOS_UINT8 localIpv6Prefix;
    VOS_UINT8 reserved2[3];

} APS_L4A_PdpTft;


typedef struct {
    VOS_UINT8 prefixLen;
    VOS_UINT8 reserved[3];

    VOS_UINT8 prefix[APS_L4A_MAX_IPV6_ADDR_LEN];

} APS_L4A_PdpIpv6Prefix;


typedef struct {
    APS_L4A_PdpTypeUint8 pdpType;
    VOS_UINT8            reserved[3];
    VOS_UINT8            ipv4Addr[APS_L4A_MAX_IPV4_ADDR_LEN];
    VOS_UINT8            ipv6Addr[APS_L4A_MAX_IPV6_ADDR_LEN];

} APS_L4A_PdpAddr;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;   /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码     */

} APS_L4A_SetCgdcontCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;   /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码     */

} APS_L4A_SetCgdscontCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;   /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码     */

} APS_L4A_SetCgtftCnf;


typedef struct {
    VOS_UINT32 opBearerId : 1;      /* BearerId */
    VOS_UINT32 opApn : 1;           /* apn */
    VOS_UINT32 opIpAddr : 1;        /* aucIpaddr */
    VOS_UINT32 opSubMask : 1;       /* aucSubnetMask */
    VOS_UINT32 opGwAddr : 1;        /* aucGWAddr */
    VOS_UINT32 opDnsPrimAddr : 1;   /* aucDNSPrimAddr */
    VOS_UINT32 opDnsSecAddr : 1;    /* aucDNSSecAddr */
    VOS_UINT32 opPcscfPrimAddr : 1; /* aucPCSCFPrimAddr */
    VOS_UINT32 opPcscfSecAddr : 1;  /* aucPCSCFSecAddr */
    VOS_UINT32 opImCnSignalFlg : 1; /* enImCnSignalFlg */
    VOS_UINT32 opPadding : 22;

    VOS_UINT8 primayCid; /* default EPS bearer context */
    /* a numeric parameter which identifies the bearer */
    VOS_UINT8 bearerId;

    /* the IM CN subsystem-related signalling flag */
    APS_L4A_ImsCnSigFlagUint8 enImCnSignalFlg;

    VOS_UINT8 reserved[1];

    /* Access Pointer Name */
    VOS_UINT8 apn[APS_L4A_MAX_APN_LEN + 1];

    /* the IP Address of the MT */
    APS_L4A_PdpAddr pdpAddr;

    /* the subnet mask for the IP Address of the MT */
    APS_L4A_PdpAddr subnetMask;

    /* the Gateway Address of the MT */
    APS_L4A_PdpAddr gwAddr;

    /* the IP Address of the primary DNS Server */
    APS_L4A_PdpAddr dnsPrimAddr;

    /* the IP address of the secondary DNS Server */
    APS_L4A_PdpAddr dnsSecAddr;

    /* the IP Address of the primary P-CSCF Server */
    APS_L4A_PdpAddr pcscfPrimAddr;

    /* the IP Address of the secondary P-CSCF Server */
    APS_L4A_PdpAddr pcscfSecAddr;

} APS_L4A_PrimPdpDynamicExt;


typedef struct {
    VOS_MSG_HEADER                     /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32       enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl           ctrl;
    VOS_UINT32                errorCode;       /* 错误码 */
    VOS_UINT32                cidNum;          /* CID数量 */
    APS_L4A_PrimPdpDynamicExt primParaInfo[0]; //lint !e43 /* 缺省承载动态参数 */

} APS_L4A_SetCgcontrdpCnf;


typedef struct {
    VOS_UINT8 cid;

    /* default EPS bearer context */
    VOS_UINT8 primaryCid;

    /* a numeric parameter which identifies the bearer */
    VOS_UINT8 bearerId;

    VOS_UINT8 reserved[1];

} APS_L4A_SecPdpDynamicExt;


typedef struct {
    VOS_MSG_HEADER                    /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32      enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl          ctrl;
    VOS_UINT32               errorCode;      /* 错误码   */
    VOS_UINT32               cidNum;         /* CID数量  */
    APS_L4A_SecPdpDynamicExt secParaInfo[0]; //lint !e43 /* 专用承载动态参数 */

} APS_L4A_SetCgscontrdpCnf;


typedef struct {
    VOS_UINT32 opPktFilterId : 1;   /* packetFilterId */
    VOS_UINT32 opPrecedence : 1;    /* precedence */
    VOS_UINT32 opSrcIp : 1;         /* aucSourceIpAddr,Mask */
    VOS_UINT32 opProtocolId : 1;    /* protocolId */
    VOS_UINT32 opDestPortRange : 1; /* destination port range */
    VOS_UINT32 opSrcPortRange : 1;  /* source port range */
    VOS_UINT32 opSpi : 1;           /* ipsec security parameter index */
    VOS_UINT32 opTosMask : 1;       /* type of service */
    VOS_UINT32 opFlowLable : 1;     /* flowLable */
    VOS_UINT32 opDirection : 1;     /* Direction */
    VOS_UINT32 opNwPktFilterId : 1; /* NWPacketFltId */
    VOS_UINT32 opLocalIpv4AddrAndMask : 1;
    VOS_UINT32 opLocalIpv6AddrAndMask : 1;
    VOS_UINT32 opSpare : 19;

    /* Packet Filter Id,value range from 1 to 16 */
    VOS_UINT8 packetFilterId;
    /* evaluation precedence index,The value range is from 0 to 255 */
    VOS_UINT8 precedence;
    /* protocol number,value range from 0 to 255 */
    VOS_UINT8       protocolId;
    VOS_UINT8       reserved1;
    APS_L4A_PdpAddr sourceIpaddr;
    APS_L4A_PdpAddr sourceIpMask;
    VOS_UINT16      lowDestPort;    /* value range from 0 to 65535 */
    VOS_UINT16      highDestPort;   /* value range from 0 to 65535 */
    VOS_UINT16      lowSourcePort;  /* value range from 0 to 65535 */
    VOS_UINT16      highSourcePort; /* value range from 0 to 65535 */
    VOS_UINT32             secuParaIndex;
    VOS_UINT8              typeOfService;     /* value range from 0 to 255 */
    VOS_UINT8              typeOfServiceMask; /* value range from 0 to 255 */
    APS_L4A_TftFilterUint8 tftFilterDirection;
    VOS_UINT8              nwPktFilterId; /* value range from 1 to 16 */
    /* value range is from 00000 to FFFFF */
    VOS_UINT32 flowLable;
    VOS_UINT8  localIpv4Addr[APS_L4A_MAX_IPV4_ADDR_LEN];
    VOS_UINT8  localIpv4Mask[APS_L4A_MAX_IPV4_ADDR_LEN];
    VOS_UINT8  localIpv6Addr[APS_L4A_MAX_IPV6_ADDR_LEN];
    VOS_UINT8  localIpv6Prefix;
    VOS_UINT8  reserved2[3];
} APS_L4A_PdpTftExt;


typedef struct {
    VOS_UINT32        cid;
    VOS_UINT32        pfNum;
    APS_L4A_PdpTftExt tftInfo[APS_L4A_MAX_SDF_PF_NUM];

} APS_L4A_SdfPfTft;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode;    /* 错误码      */
    VOS_UINT32          cidNum;       /* CID数量     */
    APS_L4A_SdfPfTft    pfTftInfo[0]; //lint !e43 /* 动态TFT参数 */

} APS_L4A_SetCgtftrdpCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;   /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码 */

} APS_L4A_SetCgeqosCnf;


typedef struct {
    VOS_UINT32 opQci : 1;   /* QCI */
    VOS_UINT32 opDlGbr : 1; /* DLGBR */
    VOS_UINT32 opUlGbr : 1; /* ULGBR */
    VOS_UINT32 opDlMbr : 1; /* DLMBR */
    VOS_UINT32 opUlMbr : 1; /* ULMBR */
    VOS_UINT32 opSpare : 27;

    VOS_UINT8 cid;

    /*
     * 0 QCI is selected by network
     * [1 - 4]value range for guranteed bit rate Traffic Flows
     * [5 - 9]value range for non-guarenteed bit rate Traffic Flows
     */
    VOS_UINT8 qci;

    VOS_UINT8 reserved[2];

    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlGbr;

    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulGbr;

    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlMbr;

    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulMbr;

} APS_L4A_EpsQosExt;


typedef struct {
    VOS_MSG_HEADER                     /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;       /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode;     /* 错误码      */
    VOS_UINT32          cidNum;        /* CID数量     */
    APS_L4A_EpsQosExt   epsQosInfo[0]; //lint !e43 /* 动态EPS QOS */

} APS_L4A_SetCgeqosrdpCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId;   /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码 */

} APS_L4A_SetAuthdataCnf;


typedef struct {
    VOS_UINT32 opPdpType : 1;        /* PdpType */
    VOS_UINT32 opApn : 1;            /* apn */
    VOS_UINT32 opPdpAddr : 1;        /* aucPdpAddr */
    VOS_UINT32 opPdpDcomp : 1;       /* PdpDcomp */
    VOS_UINT32 opPdpHcomp : 1;       /* PdpHcomp */
    VOS_UINT32 opIpv4AddrAlloc : 1;  /* IPV4ADDRALLOC */
    VOS_UINT32 opEmergency : 1;      /* Emergency Indication */
    VOS_UINT32 opPcscfDiscovery : 1; /* P-CSCF discovery */
    VOS_UINT32 opImCnSignalFlg : 1;  /* IM CN Signalling Flag */
    VOS_UINT32 opPadding : 23;

    VOS_UINT8                cid;
    VOS_UINT8                apn[APS_L4A_MAX_APN_LEN + 1];
    VOS_UINT8                reserved1[3];
    APS_L4A_PdpTypeUint8     pdpType;
    VOS_UINT8                reserved2[3];
    APS_L4A_PdpAddr          pdpAddr;
    APS_L4A_PdpDataCompUint8 pdpDataComp;
    APS_L4A_PdpHeadCompUint8 pdpHeadComp;
    VOS_UINT8                ipv4AddrAlloc; /*
                                             * 0 - IPv4 Address Allocation through NAS Signaling (default if omitted)
                                             * 1 - IPv4 Address Allocated through DHCP
                                             */
    APS_L4A_EmcIndUint8 emergency;          /*
                                             * 0 - PDP context is not for emergency bearer services
                                             * 1 - PDP context is for emergency bearer services
                                             */
    /* 0 - Preference of P-CSCF address discovery not influenced by +CGDCONT */
    APS_L4A_PcscfDiscoveryUint8 pcscfDiscovery;
    /* 1 - Preference of P-CSCF address discovery through NAS Signalling */
    /* 2 - Preference of P-CSCF address discovery through DHCP */

    /* 0 - UE indicates that the PDP context is not for IM CN subsystem-related signalling only */
    APS_L4A_ImsCnSigFlagUint8 imCnSignalFlg;
    /* 1 - UE indicates that the PDP context is for IM CN subsystem-related signalling only */

    VOS_UINT8 reserved3[2];
} APS_L4A_PrimPdpExt;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    APS_L4A_PrimPdpExt  cgdcontInfo;

} APS_L4A_SetCgdcontReq;


typedef struct {
    VOS_UINT32 opPrimaryCid : 1; /* primaryCid */
    VOS_UINT32 opPdpDcomp : 1;   /* PdpDcomp */
    VOS_UINT32 opPdpHcomp : 1;   /* PdpHcomp */
    VOS_UINT32 opImCnSignalFlg : 1;
    VOS_UINT32 opPadding : 28;

    VOS_UINT8                secondaryCid; /* dedicated EPS Bearer context */
    VOS_UINT8                primaryCid;   /* Default EPS Bearer context */
    APS_L4A_PdpDataCompUint8 pdpDataComp;
    APS_L4A_PdpHeadCompUint8 pdpHeadComp;
    /* 0 - UE indicates that the PDP context is not for IM CN subsystem-related signalling only */
    /* 1 - UE indicates that the PDP context is for IM CN subsystem-related signalling only */
    APS_L4A_ImsCnSigFlagUint8 imCnSignalFlg;
    VOS_UINT8                 reserved[3];

} APS_L4A_SecPdpExt;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    APS_L4A_SecPdpExt   cgdscontInfo;

} APS_L4A_SetCgdscontReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          cid;
    APS_L4A_PdpTftExt   tftInfo;

} APS_L4A_SetCgtftReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          cid[APS_L4A_LTE_MAX_CID + 1]; /* cid:1~L4A_LTE_MAX_CID */

} APS_L4A_SetCgpaddrReq;


typedef struct {
    VOS_UINT32 opQci : 1;   /* ValQCI */
    VOS_UINT32 opDlGbr : 1; /* ValDLGBR */
    VOS_UINT32 opUlGbr : 1; /* ValULGBR */
    VOS_UINT32 opDlMbr : 1; /* ValDLMBR */
    VOS_UINT32 opUlMbr : 1; /* ValULMBR */
    VOS_UINT32 opPadding : 27;

    VOS_UINT8 cid;
    VOS_UINT8 valQci; /* 0 QCI is selected by network
                          [1 - 4]value range for guranteed bit rate Traffic Flows
                          [5 - 9]value range for non-guarenteed bit rate Traffic Flows */
    VOS_UINT8 reserved[2];
    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 valDlGbr;
    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 valUlGbr;
    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 valDlMbr;
    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 valUlMbr;

} APS_L4A_PsCgeqos;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    APS_L4A_PsCgeqos    cgeqosInfo;

} APS_L4A_SetCgeqosReq;


typedef struct {
    VOS_UINT8  cid;      /* 1~16 */
    VOS_UINT8  defined;  /* 0:undefined,1:defined */
    VOS_UINT16 authType; /* 0,1,2,0xFF */
    VOS_UINT32 plmn;     /* 16进制存储 */
    VOS_UINT8  pwdLen;
    VOS_UINT8  reserved1[3];
    VOS_UINT8  password[APS_L4A_AUTHDATA_MAX_PASSWORD_LEN + 1];
    VOS_UINT8  userNameLen;
    VOS_UINT8  reserved2[3];
    VOS_UINT8  userName[APS_L4A_AUTHDATA_MAX_USERNAME_LEN + 1];

} APS_L4A_PdpAuthdata;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    APS_L4A_PdpAuthdata authData;

} APS_L4A_SetAuthdataReq;


typedef struct {
    VOS_UINT32 opPdnType : 1;
    VOS_UINT32 opApn : 1;
    VOS_UINT32 opSdfQos : 1;
    VOS_UINT32 opLinkdCId : 1;
    VOS_UINT32 opIpv4AddrAllocType : 1;
    VOS_UINT32 opGwAuthInfo : 1;
    VOS_UINT32 opEmergencyInd : 1;
    VOS_UINT32 opPcscfDiscovery : 1;
    VOS_UINT32 opImsCnSignalFlag : 1;
    VOS_UINT32 opImsSuppFlag : 1;
    VOS_UINT32 opRoamPdnTypeFlag : 1;
    VOS_UINT32 opSpare : 21;

    APS_L4A_PdpTypeUint8           pdnType;
    APS_L4A_PdpTypeUint8           roamPdnType;
    APS_L4A_Ipv4AddrAllocTypeUint8 ipv4AddrAllocType;
    APS_L4A_BearerTypeUint8        bearerCntxtType; /* SM上下文对应承载的类型 */

    APS_L4A_EmcIndUint8         emergencyInd;
    APS_L4A_PcscfDiscoveryUint8 pcscfDiscovery;
    APS_L4A_ImsCnSigFlagUint8   imsCnSignalFlag;
    VOS_UINT8 imsSuppFlag;

    VOS_UINT32 cid;      /* 承载上下文ID */
    VOS_UINT32 linkdCid; /* 关联CID */

    VOS_UINT32         pfNum;
    APS_L4A_EpsQos     sdfQosInfo;
    APS_L4A_ApnInfo    apnInfo;
    APS_L4A_GwAuthInfo gwAuthInfo;
    APS_L4A_PdpTft     cntxtTftInfo[APS_L4A_MAX_SDF_PF_NUM];

} APS_L4A_SdfPara;



typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;

} APS_L4A_GetCemodeReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode;

    VOS_UINT32          currentUeMode;      /* 当前UE模式 */
    VOS_UINT32          supportModeCnt;     /* UE能够支持的模式的个数 */
    VOS_UINT32          supportModeList[4]; /* UE能够支持的模式 */
} APS_L4A_GetCemodeCnf;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;

} APS_L4A_GetLteCsReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 enMsgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          errorCode; /* 错误码     */

    VOS_UINT8           smsOverSgs;
    VOS_UINT8           smsOverIms;
    VOS_UINT8           csfb;
    VOS_UINT8           srvcc;
    VOS_UINT8           voLga;
    VOS_UINT8           rsv[3];
} APS_L4A_GetLteCsCnf;

/* 删除TAF_APS_GetCidImsCfgFlag函数声明，相关逻辑挪到DSM处理 */

extern VOS_UINT32 TAF_APS_GetPdpManageInfo(ModemIdUint16 modemId, APS_L4A_PdpManageInfo *pdpManageInfo);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
