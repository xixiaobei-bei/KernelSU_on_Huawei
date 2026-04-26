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

#ifndef _NAS_COMM_PACKET_SER_H
#define _NAS_COMM_PACKET_SER_H



#include "vos.h"

/*
 * 1.1 Cplusplus Announce
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * #pragma pack(*)    设置字节对齐方式
 */
#pragma pack(push, 4)

#define PS_IPV4_ADDR_LEN (4)
#define PS_IPV6_ADDR_LEN (16)
#define PS_MAX_APN_LEN (100)
#define PS_MAX_PCSCF_ADDR_NUM (3)

#define PS_MAX_ALLOWED_S_NSSAI_NUM (8)
#define PS_NSSAI_UPDATE_MAX_NUM PS_MAX_ALLOWED_S_NSSAI_NUM

#define PS_MAX_REJECT_S_NSSAI_NUM (8)
#define PS_MAX_CFG_S_NSSAI_NUM (16)
#define PS_MAC_ADDR_LEN (6)

/* 目前与AP约定的prefer nssai设置的最大规格 */
#define PS_MAX_PREFER_S_NSSAI_NUM 8

#define PS_MAX_LADN_LIST_NUM (8)
#define PS_MAX_NR_TA_NUM (16) /* TA 列表最大个数 */

#define PS_PDU_SESSION_ID_MIN (5)
#define PS_PDU_SESSION_ID_MAX (15)
#define PS_EPS_BEARER_ID_MIN 5 /* 协议上规定EPS BEARER最小可以使用的的ID */
/* 目前有效的PDU SESSION ID范围是1-15, 总共16个, 但APS目前支持的最小PDU SESSION ID依旧为5个, 后续统一此处可以改成 (PS_PDU_SESSION_ID_MAX -
 * PS_PDU_SESSION_ID_MIN + 1) */
#define PS_EPS_BEARER_ID_MAX 15
#define PS_PDU_SESSION_ID_MAX_NUM (16)

#define PS_MAX_QOS_FLOW_NUM (8)
/* 暂定每个PDU Session中最多有PS_MAX_QOS_RULE_NUM个QoS rule */
#define PS_MAX_QOS_RULE_NUM (16)
/*
 * 24501 Page-265:
 * If the selected PDU session type is "IPv4", "IPv6", "IPv4v6" or "Ethernet" and if the PDU SESSION
 * ESTABLISHMENT REQUEST message includes a Maximum number of supported packet filters IE, the SMF shall
 * consider this number as the maximum number of packet filters that can be supported by the UE for this
 * PDU session. Otherwise the SMF considers that the UE supports 16 packet filters for this PDU session.
 * 目前UE侧单个PDU最大只支持16个PF
 */
#define PS_MAX_PF_NUM_IN_PDU 16
#define PS_MAX_PF_NUM_IN_TFT (16)
#define PS_MAX_MAP_QOS_FLOW_NUM (8)
#define PS_MAX_MAP_QOS_RULE_NUM (16)
#define PS_MAX_EPS_BEARER_NUM_IN_PDU (8)

#define PS_ONE_KBPS_UNIT_WITH_KBPS (1)
#define PS_ONE_MBPS_UNIT_WITH_KBPS (1000UL)
#define PS_ONE_GBPS_UNIT_WITH_KBPS (1000000UL)
#define PS_ONE_TBPS_UNIT_WITH_KBPS (1000000000UL)
#define PS_UE_SUPPORT_MAX_RATE (4000000000UL) /* 暂定UE最大支持速率4Tbps */
#define PS_SIM_FORMAT_PLMN_LEN (3)            /* Sim卡格式的Plmn长度 */

#define PS_QFI_UNASSIGNED_VALUE (0)
#define PS_QRI_UNASSIGNED_VALUE 0

#define PS_INVALID_PDU_SESSION_ID (0)
#define PS_INVALID_PS_CALL_ID (0xff)

#define PS_MAX_USERNAME_LEN 100
#define PS_MAX_PASSWORD_LEN 100


enum PS_SSC_Mode {
    PS_SSC_MODE_1 = 0x01,
    PS_SSC_MODE_2 = 0x02,
    PS_SSC_MODE_3 = 0x03,
    PS_SSC_MODE_BUTT
};
typedef VOS_UINT8 PS_SSC_ModeUint8;


enum PS_IP_AddrType {
    PS_IP_ADDR_TYPE_IPV4   = 0x01,
    PS_IP_ADDR_TYPE_IPV6   = 0x02,
    PS_IP_ADDR_TYPE_IPV4V6 = 0x03,
    PS_IP_ADDR_TYPE_BUTT
};
typedef VOS_UINT8 PS_IP_AddrTypeUint8;


enum PS_PDU_SessionType {
    PS_PDU_SESSION_TYPE_IPV4     = 0x01,
    PS_PDU_SESSION_TYPE_IPV6     = 0x02,
    PS_PDU_SESSION_TYPE_IPV4V6   = 0x03,
    PS_PDU_SESSION_TYPE_UNSTRUCT = 0x04, /* 此枚举值仅用在5G下 */
    PS_PDU_SESSION_TYPE_ETHERNET = 0x05, /* 此枚举值仅用在5G下 */
    PS_PDU_SESSION_TYPE_BUTT
};
typedef VOS_UINT8 PS_PDU_SessionTypeUint8;


enum PS_INTEGRITY_ProtectionMaxDataRate {
    PS_INTEGRITY_PROTECTION_MAX_DATA_RATE_64K  = 0x00,
    PS_INTEGRITY_PROTECTION_MAX_DATA_RATE_FULL = 0xFF,
};
typedef VOS_UINT8 PS_INTEGRITY_ProtectionMaxDataRateUint8;


enum PS_PREF_AccessType {
    PS_PREF_ACCESS_TYPE_3GPP     = 1,
    PS_PREF_ACCESS_TYPE_NON_3GPP = 2,

    PS_PREF_ACCESS_TYPE_BUTT
};
typedef VOS_UINT8 PS_PREF_AccessTypeUint8;


enum PS_PDU_EmcInd {
    PS_PDU_NOT_FOR_EMC = 0x00,
    PS_PDU_FOR_EMC     = 0x01,

    PS_PDU_EMC_IND_BUTT = 0xFF
};
typedef VOS_UINT8 PS_PDU_EmcIndUint8;


enum PS_PCSCF_Discovery {
    PS_PCSCF_DISCOVERY_NOT_INFLUENCED  = 0x00,
    PS_PCSCF_DISCOVERY_THROUGH_NAS_SIG = 0x01,
    PS_PCSCF_DISCOVERY_THROUGH_DHCP    = 0x02,

    PS_PCSCF_DISCOVERY_BUTT = 0xFF
};
typedef VOS_UINT8 PS_PCSCF_DiscoveryUint8;


enum PS_IMS_CnSigFlag {
    PS_NOT_FOR_IMS_CN_SIG_ONLY = 0x00,
    PS_FOR_IM_CN_SIG_ONLY      = 0x01,

    PS_IM_CN_SIG_FLAG_BUTT = 0xFF
};
typedef VOS_UINT8 PS_IMS_CnSigFlagUint8;


enum PS_QOS_RuleType {
    PS_QOS_RULE_TYPE_DEDICATED = 0x00,
    PS_QOS_RULE_TYPE_DEFAULT   = 0x01,
    PS_QOS_RULE_TYPE_BUTT
};
typedef VOS_UINT8 PS_QOS_RuleTypeUint8;


enum PS_DEB_Type {
    PS_DEB_TYPE_DEDICATED = 0x00,
    PS_DEB_TYPE_DEFAULT   = 0x01,
    PS_DEB_TYPE_BUTT
};
typedef VOS_UINT8 PS_DEB_TypeUint8;


enum PS_PF_TransDirection {
    PS_PF_TRANS_DIRECTION_RESERVED    = 0x00, /* 0 - Reserved */
    PS_PF_TRANS_DIRECTION_DOWNLINK    = 0x01, /* 1 - Downlink */
    PS_PF_TRANS_DIRECTION_UPLINK      = 0x02, /* 2 - Uplink */
    PS_PF_TRANS_DIRECTION_BIDIRECTION = 0x03, /* 3 - Birectional (Up & Downlink) (default if omitted) */

    PS_PF_TRANS_DIRECTION_BUTT = 0xFF
};
typedef VOS_UINT8 PS_PF_TransDirectionUint8;

enum PS_PF_MatchAll {
    PS_PF_NOT_MATCH_ALL = 0x00,
    PS_PF_MATCH_ALL     = 0x01,

    PS_PF_MATCH_ALL_BUTT
};
typedef VOS_UINT8 PS_PF_MatchAllUint8;


enum PS_REJ_SNssaiCause {
    PS_REJ_S_NSSAI_CAUSE_NOT_AVAIL_IN_PLMN     = 0x00,
    PS_REJ_S_NSSAI_CAUSE_NOT_AVAIL_IN_REG_AREA = 0x01,
    PS_REJ_S_NSSAI_CAUSE_DUE_TO_FAILED_OR_REVOKED_NSSAA = 0x02,
    PS_REJ_S_NSSAI_CAUSE_BUTT                  = 0x03
};
typedef VOS_UINT8 PS_REJ_SNssaiCauseUint8;


enum PS_NON_SeamlessNon3GppOffloadInd {
    PS_NO_OFFLOAD_INDICATION = 0x00,
    PS_OFFLOAD_INDICATION    = 0x01,

    PS_OFFLOAD_INDICATION_BUTT
};
typedef VOS_UINT8 PS_NON_SeamlessNon3GppOffloadIndUint8;


enum PS_REFLECT_QosInd {
    PS_REFLECT_QOS_IND_NOT_SUPPORT = 0,
    PS_REFLECT_QOS_IND_SUPPORT     = 1,

    PS_REFLECT_QOS_IND_BUTT
};
typedef VOS_UINT8 PS_REFLECT_QosIndUint8;


enum PS_IPV6_MultiHomedInd {
    PS_IPV6_MULTI_HOMED_IND_NOT_SUPPORT = 0,
    PS_IPV6_MULTI_HOMED_IND_SUPPORT     = 1,

    PS_IPV6_MULTI_HOMED_IND_BUTT
};
typedef VOS_UINT8 PS_IPV6_MultiHomedIndUint8;


enum PS_PDU_SessionAlwaysOnInd {
    PS_PDU_SESSION_ALWAYS_ON_IND_NOT = 0,
    PS_PDU_SESSION_ALWAYS_ON_IND     = 1,

    PS_PDU_SESSION_ALWAYS_ON_IND_BUTT
};
typedef VOS_UINT8 PS_PDU_SessionAlwaysOnIndUint8;


enum PS_SESSION_AmbrUnitType {
    PS_SESSION_AMBR_UNIT_TYPE_NOT_USED                                = 0x00,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_1KBPS   = 0X01,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_4KBPS   = 0X02,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_16KBPS  = 0X03,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_64KBPS  = 0X04,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_256KBPS = 0X05,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_1MBPS   = 0X06,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_4MBPS   = 0X07,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_16MBPS  = 0X08,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_64MBPS  = 0X09,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_256MBPS = 0X0A,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_1GBPS   = 0X0B,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_4GBPS   = 0X0C,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_16GBPS  = 0X0D,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_64GBPS  = 0X0E,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_256GBPS = 0X0F,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_1TBPS   = 0X10,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_4TBPS   = 0X11,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_16TBPS  = 0X12,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_64TBPS  = 0X13,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_256TBPS = 0X14,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_1PBPS   = 0X15,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_4PBPS   = 0X16,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_16PBPS  = 0X17,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_64PBPS  = 0X18,
    PS_SESSION_AMBR_UNIT_TYPE_VALUE_INCREMENT_IN_MULTIPLES_OF_256PBPS = 0X19,

    PS_SESSION_AMBR_UNIT_TYPE_ENUM_BUTT
};

typedef VOS_UINT8 PS_SESSION_AmbrUnitTypeUint8;


enum PS_NR_QosRuleOperationCode {
    PS_NR_QOS_RULE_OPERATION_CODE_RESERVED        = 0x00, /* Reserved */
    PS_NR_QOS_RULE_OPERATION_CODE_CREATE_QOS_RULE = 0x01, /* Create new Qos rule */
    PS_NR_QOS_RULE_OPERATION_CODE_DELETE_QOS_RULE = 0x02, /* Delete exiting Qos rule */
    /* Modify exiting Qos rule and add packet filters */
    PS_NR_QOS_RULE_OPERATION_CODE_MODIFY_ADD_PF = 0x03,
    /* Modify exiting Qos rule and replace all packet filters */
    PS_NR_QOS_RULE_OPERATION_CODE_MODIFY_REPLACE_ALL_PF = 0x04,
    /* Modify exiting Qos rule and delete packet filters */
    PS_NR_QOS_RULE_OPERATION_CODE_MODIFY_DELETE_PF = 0x05,
    /* Modify exiting Qos rule without modifying packet filters */
    PS_NR_QOS_RULE_OPERATION_CODE_MODIFY_WITHOUT_PF = 0x06,
    PS_NR_QOS_RULE_OPERATION_CODE_BUTT
};

typedef VOS_UINT8 PS_NR_QosRuleOperationCodeUint8;


enum PS_NR_QosFlowOperationCode {
    PS_NR_QOS_FLOW_OPERATION_CODE_RESERVED        = 0x00, /* Reserved */
    PS_NR_QOS_FLOW_OPERATION_CODE_CREATE_QOS_FLOW = 0x01, /* Create new Qos Flow */
    PS_NR_QOS_FLOW_OPERATION_CODE_DELETE_QOS_FLOW = 0x02, /* Delete exiting Qos Flow */
    PS_NR_QOS_FLOW_OPERATION_CODE_MODIFY_QOS_FLOW = 0x03, /* Modify exiting Qos Flow */
    PS_NR_QOS_FLOW_OPERATION_CODE_BUTT
};

typedef VOS_UINT8 PS_NR_QosFlowOperationCodeUint8;

typedef PS_SESSION_AmbrUnitTypeUint8 PS_NR_BitRateUnitTypeUint8;


typedef struct {
    PS_INTEGRITY_ProtectionMaxDataRateUint8 ipMaxDataRateUl; /* 上行完整性保护最大速率 */
    PS_INTEGRITY_ProtectionMaxDataRateUint8 ipMaxDataRateDl; /* 下行完整性保护最大速率 */
    VOS_UINT8                               reserved[2];
} PS_INTEGRITY_ProtectionMaxDataRatePara;


typedef struct {
    VOS_UINT8 length; /* 长度为0标示APN/DNN无效 */
    VOS_UINT8 reserved[3];
    VOS_UINT8 value[PS_MAX_APN_LEN];
} PS_APN;


typedef struct {
    VOS_UINT32 dlSessionAmbr; /* 转换后的单位是:kbps */
    VOS_UINT32 ulSessionAmbr; /* 转换后的单位是:kbps */
} PS_PDU_SessionAmbr;


typedef struct {
    VOS_UINT32 bitOpSd : 1;
    VOS_UINT32 bitOpMappedSst : 1;
    VOS_UINT32 bitOpMappedSd : 1;
    VOS_UINT32 bitOpSpare : 29;

    VOS_UINT8  ucSst;
    VOS_UINT8  ucMappedSst;
    VOS_UINT8  aucReserved[2];
    VOS_UINT32 ulSd;
    VOS_UINT32 ulMappedSd;
} PS_S_NSSAI_STRU;


typedef struct {
    PS_S_NSSAI_STRU oldNssai;
    PS_S_NSSAI_STRU newNssai;
} PS_NssaiUpdateInfo;


typedef struct {
    VOS_UINT32         num;
    PS_NssaiUpdateInfo nssaiUpdateInfo[PS_NSSAI_UPDATE_MAX_NUM];
} PS_NssaiUpdateList;


typedef struct {
    VOS_UINT32 mappedSdFlg : 1;
    VOS_UINT32 spare : 31;

    VOS_UINT8  mappedSst;
    VOS_UINT8  reserved[3];
    VOS_UINT32 mappedSd;
} PS_MappedSNssai;


typedef struct {
    VOS_UINT32 dlApnAmbr; /* 下行速率值,单位Kbps */
    VOS_UINT32 ulApnAmbr; /* 上行速率值,单位Kbps */
} PS_ApnAmbrInfo;

typedef struct {
    VOS_UINT8       snssaiNum;
    VOS_UINT8       reserved[3];
    PS_S_NSSAI_STRU snssai[PS_MAX_ALLOWED_S_NSSAI_NUM];
} PS_ALLOW_Nssai;


typedef struct {
    VOS_UINT8       snssaiNum;
    VOS_UINT8       reserved[3];
    PS_S_NSSAI_STRU snssai[PS_MAX_CFG_S_NSSAI_NUM];
} PS_CONFIGURED_Nssai;


typedef struct {
    VOS_UINT32 opSd : 1;
    VOS_UINT32 opSpare : 31;

    PS_REJ_SNssaiCauseUint8 cause;
    VOS_UINT8               sst;
    VOS_UINT8               reserved[2];
    VOS_UINT32              sd;
} PS_REJECTED_SNssai;


typedef struct {
    VOS_UINT8          snssaiNum;
    VOS_UINT8          reserved[3];
    PS_REJECTED_SNssai snssai[PS_MAX_REJECT_S_NSSAI_NUM];
} PS_REJECTED_NssaiInfo;

typedef struct {
    VOS_UINT8 data[PS_IPV4_ADDR_LEN];
} PS_Ipv4Addr;

typedef struct {
    VOS_UINT8 data[PS_IPV6_ADDR_LEN];
} PS_Ipv6Addr;

typedef struct {
    VOS_UINT8 data[PS_MAC_ADDR_LEN];
    VOS_UINT8 rsv[2];
} PS_MacAddr;

typedef struct {
    PS_Ipv4Addr addr;
    PS_Ipv4Addr mask;
} PS_Ipv4AddrMask;

typedef struct {
    PS_Ipv6Addr addr;
    VOS_UINT8 prefixLen;
    VOS_UINT8 reserved[3];
} PS_Ipv6AddrPrefix;

typedef struct {
    VOS_UINT16 lowLimit;
    VOS_UINT16 highLimit;
} PS_PortRange;

typedef struct {
    VOS_UINT8 tos;     /* TOS */
    VOS_UINT8 tosMask; /* TOS Mask */
    VOS_UINT8 rsv[2];
} PS_TypeOfSer;

/*
 * 结构说明: Ethernet PF结构
 */
typedef struct {
    VOS_UINT32 opDstMacAddr : 1;
    VOS_UINT32 opSrcMacAddr : 1;
    VOS_UINT32 opCTagVid : 1;
    VOS_UINT32 opSTagVid : 1;
    VOS_UINT32 opCTagPcpDei : 1;
    VOS_UINT32 opSTagPcpDei : 1;
    VOS_UINT32 opEtherType : 1;
    VOS_UINT32 opSpare : 25;

    VOS_UINT8  dstMacAddr[PS_MAC_ADDR_LEN];
    VOS_UINT8  srcMacAddr[PS_MAC_ADDR_LEN];
    VOS_UINT16 cTagVid;
    VOS_UINT16 sTagVid;
    VOS_UINT8  cTagPcpDei; /* 其中bit1保存dei, bit2~4保存pcp */
    VOS_UINT8  sTagPcpDei; /* 其中bit1保存dei, bit2~4保存pcp */
    VOS_UINT16 etherType;
} PS_ETH_Pf;

/*
 * 协议表格:
 * ASN.1 描述:
 * 结构说明: 详见TS 24.501 section 9.10.4.9 Traffic Flow Template，已译码
 */
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

    VOS_UINT8                 packetFilterId;
    VOS_UINT8                 nwPacketFilterId;
    PS_PF_TransDirectionUint8 direction;
    VOS_UINT8                 precedence; /* packet filter evaluation precedence */

    VOS_UINT32 secuParaIndex; /* SPI */
    VOS_UINT16 singleLcPort;
    VOS_UINT16 lcPortHighLimit;
    VOS_UINT16 lcPortLowLimit;
    VOS_UINT16 singleRmtPort;
    VOS_UINT16 rmtPortHighLimit;
    VOS_UINT16 rmtPortLowLimit;
    VOS_UINT8  protocolId;    /* 协议号 */
    VOS_UINT8  typeOfService; /* TOS */

    VOS_UINT8           typeOfServiceMask; /* TOS Mask */
    PS_PF_MatchAllUint8 matchAll;

    VOS_UINT8 rmtIpv4Address[PS_IPV4_ADDR_LEN];
    /*
     * ucSourceIpAddress[0]为IP地址高字节位
     * ucSourceIpAddress[3]为低字节位
     */
    VOS_UINT8 rmtIpv4Mask[PS_IPV4_ADDR_LEN];
    /*
     * ucSourceIpMask[0]为IP地址高字节位 ,
     * ucSourceIpMask[3]为低字节位
     */
    VOS_UINT8 rmtIpv6Address[PS_IPV6_ADDR_LEN];
    /*
     * ucRmtIpv6Address[0]为IPv6接口标识高字节位
     * ucRmtIpv6Address[15]为IPv6接口标识低字节位
     */
    VOS_UINT8 rmtIpv6Mask[PS_IPV6_ADDR_LEN];
    /*
     * ucRmtIpv6Mask[0]为高字节位
     * ucRmtIpv6Mask[15]为低字节位
     */

    VOS_UINT32 flowLabelType; /* FlowLabelType */
    VOS_UINT8  localIpv4Addr[PS_IPV4_ADDR_LEN];
    VOS_UINT8  localIpv4Mask[PS_IPV4_ADDR_LEN];
    VOS_UINT8  localIpv6Addr[PS_IPV6_ADDR_LEN];
    VOS_UINT8  localIpv6Prefix;
    VOS_UINT8  reserved2[3];
    PS_ETH_Pf  ethPf;
} PS_PF;


typedef struct {
    VOS_UINT32 pfNum;                    /* pf个数 */
    PS_PF      pf[PS_MAX_PF_NUM_IN_TFT]; /* pf表 */
} PS_TFT;


typedef struct {
    /* 此PF关联的QOS RULE ID */
    VOS_UINT8  qri;
    VOS_UINT8  reserve[3];
    PS_PF      pf;
} PS_NrPf;


typedef struct {
    VOS_UINT32 pfNum;
    PS_NrPf    pfList[PS_MAX_PF_NUM_IN_PDU];
} PS_QFT;


typedef struct {
    /*
     * 0 QCI is selected by network
     * [1 - 4]value range for guranteed bit rate Traffic Flows
     * [5 - 9]value range for non-guarenteed bit rate Traffic Flows
     */
    VOS_UINT8 qci;

    VOS_UINT8 reserved[3];

    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlGBR;

    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulGBR;

    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dlMBR;

    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 ulMBR;
} PS_EPS_Qos;


typedef struct {
    VOS_UINT32 ulMBR; /* 上行最大速率值,单位KBPS */
    VOS_UINT32 dlMBR; /* 下行最大速率值,单位KBPS */
    VOS_UINT32 ulGBR; /* 上行保证速率值,单位KBPS */
    VOS_UINT32 dlGBR; /* 上行保证速率值,单位KBPS */
} PS_EPS_ExtendedQos;


typedef struct {
    VOS_UINT32 dlApnAmbr; /* 上行速率值,单位KBPS */
    VOS_UINT32 ulApnAmbr; /* 下行速率值,单位KBPS */
} PS_EPS_ApnAmbr;


typedef struct {
    VOS_UINT32 extDLApnAmbr; /* 扩展上行速率值,单位KBPS */
    VOS_UINT32 extULApnAmbr; /* 扩展下行速率值,单位KBPS */
} PS_EPS_ExtendedApnAmbr;


typedef struct {
    /* IPV4:1, IPV6:2, IPV4V6:3 */
    PS_IP_AddrTypeUint8 ipType;
    VOS_UINT8           reserved[3];
    VOS_UINT8           ipv4Addr[PS_IPV4_ADDR_LEN];
    VOS_UINT8           ipv6Addr[PS_IPV6_ADDR_LEN];
} PS_IP_Addr;


typedef struct {
    VOS_UINT8 pcscfAddr[PS_IPV4_ADDR_LEN];
} PS_IPV4_Pcscf;


typedef struct {
    /* IPV4的P-CSCF地址个数，有效范围[0,3] */
    VOS_UINT8 ipv4PcscfAddrNum;
    VOS_UINT8 rsv[3]; /* 保留 */

    PS_IPV4_Pcscf ipv4PcscfAddrList[PS_MAX_PCSCF_ADDR_NUM];
} PS_IPV4_PcscfList;


typedef struct {
    VOS_UINT8 pcscfAddr[PS_IPV6_ADDR_LEN];
} PS_IPV6_Pcscf;


typedef struct {
    /* IPV6的P-CSCF地址个数，有效范围[0,3] */
    VOS_UINT8 ipv6PcscfAddrNum;
    VOS_UINT8 rsv[3]; /* 保留 */

    PS_IPV6_Pcscf ipv6PcscfAddrList[PS_MAX_PCSCF_ADDR_NUM];
} PS_IPV6_PcscfList;


typedef struct {
    PS_NR_BitRateUnitTypeUint8 unit; /* 单位 */
    VOS_UINT8                  rsv;
    VOS_UINT16                 value; /* 数值 */
} PS_NR_BitRate;


typedef struct {
    VOS_UINT32 opAveragWindow : 1;
    VOS_UINT32 opSpare : 31;

    VOS_UINT8  qi5;
    VOS_UINT8  rsv;
    VOS_UINT16 averagWindow; /* Averaging Window */
    VOS_UINT32 ulMaxRate;    /* MFBR uplink,无效值:0 */
    VOS_UINT32 dlMaxRate;    /* MFBR downlink,无效值:0  */
    VOS_UINT32 ulGMaxRate;   /* GFBR uplink,无效值:0  */
    VOS_UINT32 dlGMaxRate;   /* GFBR downlink,无效值:0  */
} PS_NR_Qos;


typedef struct {
    VOS_UINT8            qri;
    VOS_UINT8            qfi;
    VOS_UINT8            qosRulePrecedence;
    PS_QOS_RuleTypeUint8 qosRuleType; /* DQR:0 DEDICATED;1 DEFAULT */
} PS_NR_QosRuleInfo;


typedef struct {
    VOS_UINT8         qosRuleNum;
    VOS_UINT8         resv[3];
    PS_NR_QosRuleInfo qosRule[PS_MAX_QOS_RULE_NUM];
    PS_QFT            qft;
} PS_NR_QosRuleList;


typedef struct {
    VOS_UINT32 opEpsbId : 1;
    VOS_UINT32 opSpare : 31;

    VOS_UINT8 qfi;
    /* 如果没有Epsbid,表示此 QoS Flow没有映射的EPS BEARER */
    VOS_UINT8 epsbId;
    VOS_UINT8 psCallId; /* 0xFF为无效值 */
    VOS_UINT8 cid;
    PS_NR_Qos nrQos;

} PS_NR_QosFlowInfo;


typedef struct {
    VOS_UINT8         qosFlowNum;
    VOS_UINT8         resv[3];
    PS_NR_QosFlowInfo qosFlow[PS_MAX_QOS_FLOW_NUM];
} PS_NR_QosFlowList;


typedef struct {
    VOS_UINT8         mapQosFlowNum;
    VOS_UINT8         resv[3];
    PS_NR_QosFlowInfo mapQosFlow[PS_MAX_MAP_QOS_FLOW_NUM];
} PS_EPS_MapQosFlowList;


typedef struct {
    VOS_UINT8         mapQosRuleNum;
    VOS_UINT8         resv[3];
    PS_NR_QosRuleInfo mapQosRule[PS_MAX_MAP_QOS_RULE_NUM];
    PS_QFT            qft;
} PS_EPS_MapQosRuleList;


typedef struct {
    VOS_UINT32 opQos : 1;
    VOS_UINT32 opApnAmbr : 1;
    VOS_UINT32 opTft : 1;
    VOS_UINT32 opSpare : 29;

    VOS_UINT8        epsbId;
    PS_DEB_TypeUint8 bearerType; /* DEB:0 DEDICATED;1 DEFAULT */
    VOS_UINT8        resv[2];

    PS_EPS_Qos     epsQos;
    PS_EPS_ApnAmbr apnAmbr;
    PS_TFT         tft;
} PS_MAP_EpsBearerInfo;


typedef struct {
    VOS_UINT8 epsBearerNum;
    VOS_UINT8 resv[3];

    PS_MAP_EpsBearerInfo epsBearer[PS_MAX_EPS_BEARER_NUM_IN_PDU];
} PS_MAP_EpsBearerList;


typedef struct {
    VOS_UINT8 plmnId[PS_SIM_FORMAT_PLMN_LEN];
    VOS_UINT8 rsv;
} PS_SIM_FormatPlmnId;


typedef struct {
    VOS_UINT8 plmnId[3];
    VOS_UINT8 rsv;
} PS_NR_PlmnId;


typedef struct {
    VOS_UINT8 tac;
    VOS_UINT8 tacCont;
    VOS_UINT8 tacCont1;
    VOS_UINT8 rsv[1];
} PS_NR_Tac;


typedef struct {
    PS_NR_PlmnId plmnId;
    PS_NR_Tac    tac;
} PS_NR_Tai;


typedef struct {
    VOS_UINT32 taNum; /* TA的个数    */
    PS_NR_Tai  tai[PS_MAX_NR_TA_NUM];
} PS_NR_TaiList;


typedef struct {
    PS_APN        dnn;
    PS_NR_TaiList taiList;
} PS_LADN;


typedef struct {
    VOS_UINT8 ladnNum;
    VOS_UINT8 reserved[3];
    PS_LADN   ladnList[PS_MAX_LADN_LIST_NUM];
} PS_LADN_Info;


typedef struct {
    VOS_UINT32 opPrimDnsAddr : 1;
    VOS_UINT32 opSecDnsAddr : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT8 primDnsAddr[PS_IPV4_ADDR_LEN];
    VOS_UINT8 secDnsAddr[PS_IPV4_ADDR_LEN];
} PS_IPV4_Dns;


typedef struct {
    VOS_UINT32 opPrimDnsAddr : 1;
    VOS_UINT32 opSecDnsAddr : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT8 primDnsAddr[PS_IPV6_ADDR_LEN];
    VOS_UINT8 secDnsAddr[PS_IPV6_ADDR_LEN];

} PS_IPV6_Dns;

/*
 * 结构说明: 24.501协议 9.10.4.2
 */
typedef struct {
    PS_REFLECT_QosIndUint8     rqosInd;
    PS_IPV6_MultiHomedIndUint8 mh6Ind;
    VOS_UINT8                  rsv[2];
} PS_5GSM_Capability;

/*
 * 结构说明: ALLOWED SSC MODE数据结构
 */
typedef struct {
    VOS_UINT8 sscMode1Allowed; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
    VOS_UINT8 sscMode2Allowed; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
    VOS_UINT8 sscMode3Allowed; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
    VOS_UINT8 reserved[1];
} PS_ALLOWED_SscMode;


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
