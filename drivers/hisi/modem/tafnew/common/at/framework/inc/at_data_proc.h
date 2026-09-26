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

#ifndef __AT_DATA_PROC_H__
#define __AT_DATA_PROC_H__

#include "at_ctx.h"
#include "at_input_proc.h"
#include "at_ndis_interface.h"
#include "at_rnic_interface.h"
#include "ads_interface.h"
#include "at_ppp_interface.h"
#include "ppp_interface.h"
#include "ps_iface_global_def.h"
#include "taf_ps_api.h"
#include "taf_iface_api.h"
#include "at_cmd_msg_proc.h"
#include "at_internal_msg.h"
#include "dsm_ndis_pif.h"
#include "mdrv_eipf.h"
#include "at_set_para_cmd.h"
#include "at_ndserver_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

/* ASN的部分结构体未四字节对齐导致LLT工程出现RUNTIME错误, 下方的宏可以屏蔽掉一个函数的runtime问题 */
#if defined(VOS_WIN32) && (VOS_OS_VER == VOS_WIN32) && (defined(__clang__) || defined(__GNUC__))
#define ATTRIBUTE_NO_SANITIZE_RUNTIME __attribute__((no_sanitize_undefined))
#else
#define ATTRIBUTE_NO_SANITIZE_RUNTIME
#endif

/*lint -e778 */
/*lint -e572 */
#ifndef VOS_NTOHL /* 大小字节序转换 */
#if VOS_BYTE_ORDER == VOS_BIG_ENDIAN
#define VOS_NTOHL(x) (x)
#define VOS_HTONL(x) (x)
#define VOS_NTOHS(x) (x)
#define VOS_HTONS(x) (x)
#else
#define VOS_NTOHL(x) \
    ((((x) & 0x000000ffU) << 24) | (((x) & 0x0000ff00U) << 8) |  \
     (((x) & 0x00ff0000U) >> 8) | (((x) & 0xff000000U) >> 24))

#define VOS_HTONL(x) \
    ((((x) & 0x000000ffU) << 24) | (((x) & 0x0000ff00U) << 8) |  \
     (((x) & 0x00ff0000U) >> 8) | (((x) & 0xff000000U) >> 24))

#define VOS_NTOHS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#define VOS_HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#endif /* _BYTE_ORDER==_LITTLE_ENDIAN */
#endif
/*lint -e572 */
/*lint -e778 */

#define AT_MAX_IPV6_DNS_LEN 16
#define AT_IPV6_STR_HEX_RADIX 16
#define AT_IPV6_STR_DEC_RADIX 10

#define AT_MAX_IPV6_STR_DOT_NUM 3
#define AT_MAX_IPV4V6_STR_COLON_NUM 6
#define AT_MAX_IPV6_STR_COLON_NUM 7

#define AT_IPV6_STR_MAX_TOKENS 16 /* IPV6字符串格式使用的分隔符标记最大个数 */
#define AT_IPV4_STR_DELIMITER '.' /* RFC协议使用的IPV4文本表达方式使用的分隔符 */
#define AT_IPV6_STR_DELIMITER ':' /* RFC2373使用的IPV6文本表达方式使用的分隔符 */

#define AT_GetIpv6Capability() (AT_GetCommPsCtxAddr()->ipv6Capability)

#define AT_PS_GET_SHARE_PDP_FLG() (AT_GetCommPsCtxAddr()->sharePdpFlag)

#define AT_GetEthernetCap() (AT_GetCommPsCtxAddr()->etherCap)

/* 获取APP客户端ID */
#define AT_APP_GET_CLIENT_ID() (g_atClientTab[AT_CLIENT_ID_APP].clientId)

/*
 * ^NDISSTAT: <stat>[,<err_code>[,<wx_stat>[,<PDP_type]]]
 * <err_code> value defined as follows:
 * 0      - Unknown error/Unspecified error
 * others - (E)SM Cause
 *          SM Cause is defined in TS 24.008 section 10.5.6.6
 *          ESM Cause is defined in TS 24.301 section 9.9.4.4
 */
#define AT_NDISSTAT_ERR_UNKNOWN 0

/* 获取NDIS客户端ID */
#define AT_NDIS_GET_CLIENT_ID() (g_atClientTab[AT_CLIENT_ID_NDIS].clientId)

#define AT_APS_IP6_ADDR_PREFIX_BYTE_LEN 8

#define AT_MAC_LEN 6
#define AT_IPV6_ADDR_PREFIX_BYTE_LEN 8
#define AT_IPV6_ADDR_PREFIX_BIT_LEN 64

#define AT_REG_FC 1
#define AT_DEREG_FC 0

/* QOS_TRAFFIC_CLASS */
#define AT_QOS_TRAFFIC_CLASS_CONVERSATIONAL 1
#define AT_QOS_TRAFFIC_CLASS_STREAMING 2
#define AT_QOS_TRAFFIC_CLASS_INTERACTIVE 3
#define AT_QOS_TRAFFIC_CLASS_BACKGROUND 4

#define AT_IPV6_STR_RFC2373_TOKENS 8 /* 分隔符标记的最大个数 */

#define AT_IPPROTO_UDP 0x11 /* IP头部中UDP协议号 */
#define AT_IP_VERSION 4     /* IP头部中IP V4版本号 */
#define AT_IP_DEF_TTL 0xFF  /* IP头部中IP TTL缺省值 */
#define AT_IP_RAND_ID 61468 /* IP头部ID值，随机取 */
#define AT_IP_HDR_LEN 20    /* IP 头部长度 */
#define AT_UDP_HDR_LEN 8    /* UDP 头部长度 */

#define AT_TYPE_OF_SERVICE_MAX_VALUE_RANGE 255
#define AT_TYPE_OF_SERVICE_MASK_MAX_VALUE_RANGE 255
#define AT_HIGH_DEST_PORT_MAX_VALUE_RANGE 65535
#define AT_LOW_DEST_PORT_MAX_VALUE_RANGE 65535
#define AT_HIGH_SOURCE_PORT_MAX_VALUE_RANGE 65535
#define AT_LOW_SOURCE_PORT_MAX_VALUE_RANGE 65535

/* IPV6地址后8个字节全零，则认为是无效的 */
#define AT_PS_IS_IPV6_ADDR_IID_VALID(aucIpv6Addr)                                                  \
    !(((aucIpv6Addr[8]) == 0x00) && ((aucIpv6Addr[9]) == 0x00) && ((aucIpv6Addr[10]) == 0x00) &&   \
      ((aucIpv6Addr[11]) == 0x00) && ((aucIpv6Addr[12]) == 0x00) && ((aucIpv6Addr[13]) == 0x00) && \
      ((aucIpv6Addr[14]) == 0x00) && ((aucIpv6Addr[15]) == 0x00))

#define AT_PS_INVALID_RMNET_ID 0xFFFF
#define AT_PS_INVALID_IFACE_ID 0xFF

#define AT_PS_IS_RABID_VALID(rabId) (((rabId) >= AT_PS_MIN_RABID) && ((rabId) <= AT_PS_MAX_RABID))

#define AT_PS_GET_CURR_CMD_OPT(indexNum) (g_atClientTab[indexNum].cmdCurrentOpt)
#define AT_PS_GET_PPPID(indexNum) (g_atClientTab[indexNum].pppId)



#define AT_PS_RMNET_IFACE_ID_TBL_PTR() (g_atPsIfaceIdRmNetIdTab)
#define AT_PS_RMNET_IFACE_ID_TBL_SIZE() (AT_ARRAY_SIZE(g_atPsIfaceIdRmNetIdTab))



#define AT_PS_DIAL_RAT_TYPE_NO_ASTRICT 0
#define AT_PS_DIAL_RAT_TYPE_1X_OR_HRPD 24
#define AT_PS_DIAL_RAT_TYPE_LTE_OR_EHRPD 36

#define IMS_PCSCF_ADDR_PORT_MAX 65535

#define AT_BUILD_EXRABID(i, j) ((((i) << 6) & 0xC0) | ((j) & 0x3F))
#define AT_GET_RABID_FROM_EXRABID(i) ((i) & 0x3F)

#define AT_IPV6_ADDR_MASK_FORMAT_STR_LEN 256
#define AT_IPV6_ADDR_DEC_FORMAT_STR_LEN 128
#define AT_IPV6_ADDR_COLON_FORMAT_STR_LEN 40
#define AT_IPV6_ADDR_DEC_TOKEN_NUM 16
#define AT_IPV6_ADDR_HEX_TOKEN_NUM 8
#define AT_IP_STR_DOT_DELIMITER '.'
#define AT_IP_STR_COLON_DELIMITER ':'
#define AT_DIAL_RATE_DISPALY_NV_ARRAY_LEN 6

#define AT_EVENT_INFO_ARRAY_INDEX_0 0
#define AT_EVENT_INFO_ARRAY_INDEX_1 1
#define AT_EVENT_INFO_ARRAY_INDEX_2 2

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#define AT_GET_IFACE_FC_HEAD_BY_MODEMID(usModemId)    \
    (((usModemId) == MODEM_ID_0) ? EIPF_MODEM0_ULFC : \
                                   (((usModemId) == MODEM_ID_1) ? EIPF_MODEM1_ULFC : EIPF_MODEM2_ULFC))
#endif

#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
#define TAF_PS_PPP_DIAL_UP(moduleId, exClientId, opId, cid, pppReqConfigInfo) \
    TAF_IFACE_PppUp(pppReqConfigInfo, moduleId, exClientId, opId, cid)

#define TAF_PS_PPP_DIAL_DOWN(moduleId, exClientId, opId, cid) \
    TAF_IFACE_PppDown(moduleId, exClientId, opId)
#else
#define TAF_PS_PPP_DIAL_UP(moduleId, exClientId, opId, cid, pppReqConfigInfo) \
    TAF_PS_PppDialOrig(moduleId, exClientId, opId, cid, pppReqConfigInfo)

#define TAF_PS_PPP_DIAL_DOWN(moduleId, exClientId, opId, cid) \
    TAF_PS_CallEnd(moduleId, exClientId, opId, cid)
#endif

enum AT_PDP_Status {
    AT_PDP_STATUS_DEACT = 0,
    AT_PDP_STATUS_ACT   = 1,
    AT_PDP_STATUS_HOLD  = 2,
    AT_PDP_STATUS_BUTT
};
typedef VOS_UINT32 AT_PDP_StatusUint32;

/*
 * 协议表格:
 * ASN.1 描述:
 * 结构说明: IPV6 String格式枚举
 *           HEX为RFC2373要求使用':'作为分隔符
 *           DEX为RFC要求使用'.'作为分隔符
 */
enum AT_IPV6_StrType {
    AT_IPV6_STR_TYPE_HEX = 0x01,
    AT_IPV6_STR_TYPE_DEC = 0x02,

    AT_IPV6_STR_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 AT_IPV6_StrTypeUint8;

/*
 * 协议表格:
 * ASN.1 描述:
 * 结构说明: 区分是SOURCE还是LOCAL的IP ADDR
 */
enum AT_IpAddrType {
    AT_IP_ADDR_TYPE_SOURCE = 0x01,
    AT_IP_ADDR_TYPE_LOCAL  = 0x02,

    AT_IP_ADDR_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 AT_IpAddrTypeUint8;

/*
 * 协议表格:
 * ASN.1 描述:
 * 枚举说明: HiLink模式
 */
enum AT_HilinkMode {
    AT_HILINK_NORMAL_MODE,
    AT_HILINK_GATEWAY_MODE,
    AT_HILINK_MODE_BUTT
};
typedef VOS_UINT8 AT_HilinkModeUint8;

/* APP拨号状态 */
/*
 * 协议表格:
 * ASN.1 描述:
 * 枚举说明: APP拨号状态
 */
enum AT_APP_ConnState {
    AT_APP_DIALING,  /* 0: indicates is connecting */
    AT_APP_DIALED,   /* 1: indicates connected */
    AT_APP_UNDIALED, /* 2: indicates disconnected */
    AT_APP_BUTT
};
typedef VOS_UINT32 AT_APP_ConnStateUint32;

/*
 * PPP拨号后的速率气泡显示，分两种情况处理:
 * 1.2G模式下，则根据当前驻留的小区模式是GSM/GPRS/EDGE来决定速率气泡的显示,对应如下:
 *     GSM          - 9600
 *     GPRS         - 85600
 *     GPRS Class33 - 107800
 *     EDGE         - 236800
 *     EDGE Class33 - 296000
 *     默认
 * 2.3g模式下，则根据RRC版本和HSDPA的category信息来决定速率气泡的显示，对应如下:
 *     RRC版本为R99   - 384000
 *     RRC版本为非R99 - 判断HSDPA的category信息
 *                   6  - 3600000
 *                   8  - 7200000
 *                   9  - 10200000
 *                   10 - 14400000
 *                   12 - 1800000
 *                   14 - 21600000
 *                   18 - 28800000
 *                   24 - 43200000
 *                   26 - 57600000
 *                   28 - 86400000
 *     有扩展的category的话，默认 - 21600000
 *     无扩展的category的话，默认 - 7200000
 */
enum PPP_RateDisplay {
    PPP_RATE_DISPLAY_GSM = 0,
    PPP_RATE_DISPLAY_GPRS,
    PPP_RATE_DISPLAY_GPRS_CALSS33,
    PPP_RATE_DISPLAY_EDGE,
    PPP_RATE_DISPLAY_EDGE_CALSS33,

    PPP_RATE_DISPLAY_R99,
    PPP_RATE_DISPLAY_DPA_CATEGORY_6,
    PPP_RATE_DISPLAY_DPA_CATEGORY_8,
    PPP_RATE_DISPLAY_DPA_CATEGORY_9,
    PPP_RATE_DISPLAY_DPA_CATEGORY_10,
    PPP_RATE_DISPLAY_DPA_CATEGORY_12,
    PPP_RATE_DISPLAY_DPA_CATEGORY_14,
    PPP_RATE_DISPLAY_DPA_CATEGORY_18,
    PPP_RATE_DISPLAY_DPA_CATEGORY_24,
    PPP_RATE_DISPLAY_DPA_CATEGORY_26,
    PPP_RATE_DISPLAY_DPA_CATEGORY_28,

    PPP_RATE_DISPLAY_BUTT
};
typedef VOS_UINT32 PPP_RateDisplayUint32;

/* DHCP配置，全主机序 */
typedef struct {
    VOS_UINT32 iPAddr;     /* IP 地址，网侧分配 */
    VOS_UINT32 subNetMask; /* 子网掩码地址，根据IP地址计算 */
    VOS_UINT32 gateWay;    /* 网关地址，也是本DHCP Server的地址 */
    VOS_UINT32 primDNS;    /* 主 DNS地址，网侧分配 */
    VOS_UINT32 sndDNS;     /* 次 DNS地址，网侧分配 */
} AT_DHCP_Config;

typedef struct AT_DHCP_PARA {
    VOS_UINT16     clientID;  /* Client ID */
    VOS_UINT8      rabID;     /* Rab ID */
    VOS_UINT8      cid;       /* CID */
    VOS_UINT32     speed;     /* Um Speed */
    VOS_UINT32     dLMaxRate; /* 理论最大下行速率 */
    VOS_UINT32     uLMaxRate; /* 理论最大上行速率 */
    AT_DHCP_Config dhcpCfg;
} AT_DHCP_Para;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPv4 packet header 结构
 */
typedef struct {
#if (VOS_LITTLE_ENDIAN == VOS_BYTE_ORDER) /* 小端字节序 */
    VOS_UINT8 ucIpHdrLen : 4;             /* IP头部长度 */
    VOS_UINT8 ucIpVer : 4;                /* IP版本号 */
#elif (VOS_BYTE_ORDER == VOS_BIG_ENDIAN)  /* 大端字节序 */
    VOS_UINT8 ucIpVer : 4;    /* IP版本号 */
    VOS_UINT8 ucIpHdrLen : 4; /* IP头部长度 */
#else
#error "Please fix Macro VOS_BYTE_ORDER"                   /* VOS_BYTE_ORDER未定义 */
#endif
    VOS_UINT8  serviceType;    /* IP TOS字段 */
    VOS_UINT16 totalLen;       /* IP数据包总长度 */
    VOS_UINT16 identification; /* IP数据包ID */
    VOS_UINT16 offset;         /* IP分片偏移量 */
    VOS_UINT8  tTL;            /* IPTTL */
    VOS_UINT8  protocol;       /* IP数据载荷部分协议 */
    VOS_UINT16 checkSum;       /* IP头部校验和 */
    VOS_UINT32 srcAddr;        /* 源IP地址 */
    VOS_UINT32 dstAddr;        /* 目的IP地址 */
} AT_IPHDR;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP header 结构
 */
typedef struct {
    VOS_UINT16 srcPort;  /* 源端口 */
    VOS_UINT16 dstPort;  /* 目的端口 */
    VOS_UINT16 len;      /* UDP包长度 */
    VOS_UINT16 checksum; /* UDP校验和 */
} AT_UdpHdr;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP packet 结构
 */
typedef struct {
    AT_IPHDR   ipHdr;  /* IP头 */
    AT_UdpHdr  udpHdr; /* UDP头 */
    VOS_UINT32 body;
} AT_UdpPacketFormat;


/*
 * 结构说明: IFACEID与RMNETID映射关系的结构
 */
typedef struct {
    PS_IFACE_IdUint8       ifaceId;
    RNIC_DEV_ID_ENUM_UINT8 rmNetId;
    VOS_UINT8              reserved1;
    VOS_UINT8              reserved2;
} AT_PS_RmnetIfaceId;


typedef struct {
    VOS_UINT32     cmdCurrentOpt;
    MN_PS_EVT_FUNC evtFunc;
} AT_PS_DynamicInfoReportFuncTbl;

extern AT_HilinkModeUint8 g_hiLinkMode;

/*
 * 功能描述: 把IPV4地址由字符串格式转换成数字格式
 */
VOS_UINT32 AT_Ipv4AddrAtoi(VOS_CHAR *pcString, VOS_UINT8 *number, VOS_UINT32 numBufLen);

/*
 * 功能描述: 把IPV4地址由数字格式转换成字符串格式
 */
VOS_UINT32 AT_Ipv4AddrItoa(VOS_CHAR *pcString, VOS_UINT32 strLen, VOS_UINT8 *number);

/*
 * 功能描述: IPV4类型的地址转换为字符串类型
 */
VOS_UINT32 AT_Ipv4Addr2Str(VOS_CHAR *pcString, VOS_UINT32 strBuflen, VOS_UINT8 *number, VOS_UINT32 addrNumCnt);
/*
 * 功能描述: 根据转换基数(10或16), 将整数转换为ASCII码, 将结果输出至字符串
 */
VOS_CHAR* AT_Itoa(VOS_UINT16 value, VOS_CHAR *pcStr, VOS_UINT16 radix, VOS_UINT32 length);
/*
 * 功能描述: 字符串转整形
 */
VOS_UINT64 AT_AtoI(VOS_CHAR *string, VOS_UINT32 stringLen);

/*
 * 功能描述: 字符串转整形(可以带符号转换)
 */
VOS_INT32 AT_AtoInt(VOS_CHAR *string, VOS_UINT32 stringLen, VOS_INT32 *out);

/*
 * 功能描述: 将IPV6地址格式转换为字符串压缩格式
 */
VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(VOS_UINT8 aucAddrStr[], VOS_UINT8 aucIpAddr[], VOS_UINT8 tokensNum);

VOS_VOID AT_SendRelPppReq(VOS_UINT16 pppId, PPP_AtCtrlOperTypeUint32 operType);

VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 capability);

VOS_UINT8 AT_CalcIpv6PrefixLength(VOS_UINT8 *localIpv6Mask, VOS_UINT32 ipv6MaskLen);

VOS_UINT32 AT_Ipv6AddrAtoi(VOS_CHAR *pcString, VOS_UINT8 *number, VOS_UINT32 ipv6BufLen);

VOS_UINT32 AT_PS_ValidateDialParam(const VOS_UINT8 indexNum, const TAF_IFACE_UserTypeUint8 userType);

/*
 * 功能描述: 生成扩展的ClientId(ModemId + ClientId)
 */
VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 clientId);

VOS_UINT32 AT_Ipv6AddrMask2FormatString(VOS_CHAR *pcIpv6FormatStr, VOS_UINT8 aucIpv6Addr[], VOS_UINT32 addrLen,
        VOS_UINT8 aucIpv6Mask[], VOS_UINT32 maskLen);

VOS_VOID AT_PS_ReportImsCtrlMsgu(VOS_UINT8 indexNum, AT_IMS_CtrlMsgReceiveModuleUint8 module, VOS_UINT32 msgLen,
                                 VOS_UINT8 *dst);

VOS_UINT32 AT_PS_BuildIfaceCtrl(const VOS_UINT32 moduleId, const VOS_UINT16 portClientId, const VOS_UINT8 opId,
                                TAF_Ctrl *ctrl);

VOS_UINT32 AT_PS_ProcMapconMsg(const VOS_UINT16 clientId, const AT_MAPCON_CTRL_MSG_STRU *atMapConMsgPara);

VOS_VOID AT_PS_ProcNdisIfaceUpCfg(const DSM_NDIS_IfaceUpInd *rcvMsg);

VOS_VOID AT_PS_ProcNdisIfaceDownCfg(const DSM_NDIS_IfaceDownInd *rcvMsg);

VOS_VOID AT_PS_ProcNdisConfigIpv6Dns(const DSM_NDIS_ConfigIpv6DnsInd *rcvMsg);

VOS_VOID AT_RcvTafIfaceEvt(TAF_PS_Evt *evt);

VOS_VOID   AT_PS_SendNdisIPv4IfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg);
VOS_VOID   AT_PS_SendNdisEthIfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg);

VOS_VOID         AT_PS_SendNdisIPv6IfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg);
RNIC_DEV_ID_ENUM_UINT8 AT_PS_GetRmnetIdFromIfaceId(
    const PS_IFACE_IdUint8                enIfaceId
);

VOS_UINT8 AT_IsPppUser(VOS_UINT8 indexNum);


#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
VOS_VOID AT_PS_SendNdserverPppIPv6RelInd(VOS_VOID);
VOS_VOID AT_PS_SendNdserverPppIPv6CfgInd(const AT_NDSERVER_Ipv6PdnInfo *ipv6PdnInfo);
#else
static inline VOS_VOID AT_PS_SendNdserverPppIPv6RelInd(VOS_VOID)
{
}

static inline VOS_VOID AT_PS_SendNdserverPppIPv6CfgInd(const AT_NDSERVER_Ipv6PdnInfo *ipv6PdnInfo)
{
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_MacAddrAtoi(const VOS_CHAR *inputStr, VOS_UINT8 *addrValue, VOS_UINT32 numBufLen);

VOS_UINT32 AT_Ipv4AddrAndMaskAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *ipv4Addr, VOS_UINT8 *ipv4Mask);

VOS_UINT32 AT_Ipv6AddrAndPrefixLenAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *ipv6AddrValue,
    VOS_UINT8 *prefixLen);

VOS_UINT32 AT_PortRangeAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT16 *lPort, VOS_UINT16 *hPort);

VOS_UINT32 AT_TosAndMaskAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *tos, VOS_UINT8 *mask);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __AT_DATA_PROC_H__ */
