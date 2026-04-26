/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef __IPIPMGLOBAL_H__
#define __IPIPMGLOBAL_H__

/*
 * 1 Include Headfile
 */
#include "ttf_comm.h"

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
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*
 * 2 macro
 */
#define IP_NULL 0
#define IP_OK VOS_OK
#define IP_NULL_PTR VOS_NULL_PTR
#define IP_MAX_PDN_NUM 10

#define IP_DNS_MAX_NUM 4

#define IP_IPM_MTU 1500

/* 定时器最长时间，单位秒 */
#define IP_TIMER_MAX_LENGTH (64800000 / 1000)

/* IP地址长度 */
#define IP_IPV4_ADDR_LEN 4

#define IP_IPV6_ADDR_LEN 16

#define IP_UDP_FAKEHEAD_LEN 12

#define IP_UDP_HEAD_LEN 8

#define IP_UDP_DHCP_HDR_SIZE 4

#define IP_ICMPV6_HEAD_LEN 8

#define IP_ETHERNET_HEAD_LEN 14

#ifndef IP_IPV4_HEAD_LEN
#define IP_IPV4_HEAD_LEN 20 /* IPV4数据头头部长度 */
#endif

#ifndef IP_IPV6_HEAD_LEN
#define IP_IPV6_HEAD_LEN 40 /* IPV6数据头头部长度 */
#endif

/* MAC地址长度 */
#define IP_MAC_ADDR_LEN 6
#define IP_ETH_MAC_HEADER_LEN 14 /* MAC帧头长度 */
#define IP_IPV6_PAYLOAD 0xdd86   /* IPV6 payload */

/* ip包头校验成功值,16位 */
#define IP_CHECKSUM_SUCCESS 0xFFFF

/* ip包头校验的长度,按16位进行检测 */
#define IPCHECKLEN 10

#define IP_BITMOVE_24 24

#define IP_BITMOVE_16 16

#define IP_BITMOVE_8 8

#define IP_BITMOVE_4 4

#define IP_8_LOW_BYTE 0x0F

#define IP_8_HIGH_BYTE 0xF0

#define IP_32_HIGH_BYTE 0xFFFF0000

#define IP_32_LOW_BYTE 0x0000FFFF

#define IP_32_FIRST_BYTE 0xFF000000

#define IP_32_SECOND_BYTE 0x00FF0000

#define IP_32_THIRD_BYTE 0x0000FF00

#define IP_32_FOURTH_BYTE 0x000000FF

#define IP_16_HIGH_BYTE 0xFF00

#define IP_16_LOW_BYTE 0x00FF

#define IP_IPM_TTL 128

#define IP_OP_TRUE 1

#define IP_OP_FALSE 0

#define IP_DHCPV4_MSG_REPLY 2

/* 以太网硬件类型 */
#define IP_ETHERNET_HTYPE 1

/* DHCP协议option的起始偏移 */
#define IP_DHCPV4_OPTION_OFFSET 240

/* DHCP 报文OPTIONS code 长度 */
#define IP_DHCPV4_OPTION_CODE_LEN 1

/* DHCP 报文OPTIONS code + length 长度 */
#define IP_DHCPV4_OPTION_CODELEN_LEN 2

/* dhcp 报文DHCP Message Type 长度 */
#define IP_DHCPV4_OPTION_MSG_TYPE_LEN 1

/* dhcp 报文IP Address Lease Time 长度 */
#define IP_DHCPV4_OPTION_LEASE_TIME_LEN 4

/* dhcp 报文头cookie字段长度 */
#define IP_DHCPV4_HEAD_COOKIE_LEN 4

/* dhcp 报文头XID字段长度 */
#define IP_DHCPV4_HEAD_XID_LEN 4

/* dhcp 报文头secs字段长度 */
#define IP_DHCPV4_HEAD_SECS_LEN 2

/* dhcp 报文头flags字段长度 */
#define IP_DHCPV4_HEAD_FLAGS_LEN 2

#define IP_MIN(n1, n2) (((n1) > (n2)) ? (n2) : (n1))

#define IP_COMPARE(ipAddr1, ipAddr2)                                                           \
    ((ipAddr1[0] == ipAddr2[0]) && (ipAddr1[1] == ipAddr2[1]) && (ipAddr1[2] == ipAddr2[2]) && \
     (ipAddr1[3] == ipAddr2[3]))

#define IP_SET_TIMER_NULL(timerSet) ((timerSet) = VOS_NULL_PTR)

#define IP_GET_MSG_ENTITY(msgEntity) ((VOS_VOID *)&((msgEntity)->ulMsgId))

#define IP_GET_MSG_LENGTH(msgEntity) ((msgEntity)->ulLength)

#define IP_GetMsgSenderPid(msgEntity) (((PS_MsgHeader *)msgEntity)->ulSenderPid)
#define IP_GetMsgLength(msgEntity) (((PS_MsgHeader *)msgEntity)->ulLength)
#define IP_GetMsgId(msgEntity) (((PS_MsgHeader *)msgEntity)->msgName)
#define IP_GetTimerPara(msgEntity) (((REL_TimerMsgBlock *)msgEntity)->para)
#define IP_GetTimerName(msgEntity) (((REL_TimerMsgBlock *)msgEntity)->name)

/* 从定时器名中获取定时器类型 */
#define IP_GetTimerType(timerName) (timerName & IP_32_LOW_BYTE)

/* 从定时器名中获取实体索引 */
#define IP_GetEntityIndex(timerName) (((timerName & IP_32_SECOND_BYTE) >> IP_BITMOVE_16))

#define IP_GetNameFromMsg(msgId, msgEntity) do { \
        if (VOS_PID_TIMER == IP_GetMsgSenderPid(msgEntity)) {            \
            msgId = IP_GetTimerName(msgEntity);                        \
            msgId = (msgId & IP_32_THIRD_BYTE) >> IP_BITMOVE_8; \
            msgId = IP_ROUTER_TIMERMSG_ET(msgId);               \
        } else {                                                    \
            msgId = IP_GetMsgId(msgEntity);                            \
            msgId = IP_ROUTER_LAYERMSG_ET(msgId);               \
        } \
} while (0)

/* 封装写ESM消息头的宏 */
#define IP_WRITE_ESM_MSG_HEAD(msgHead, esmMsgID) do { \
    (msgHead)->ulMsgId         = (esmMsgID);     \
    (msgHead)->ulSenderCpuId   = VOS_LOCAL_CPUID;  \
    (msgHead)->ulSenderPid     = PS_PID_IP;        \
    (msgHead)->ulReceiverCpuId = VOS_LOCAL_CPUID;  \
    (msgHead)->ulReceiverPid   = PS_PID_ESM; \
} while (0)

/* 封装写RABM消息头的宏 */
#define IP_WRITE_RABM_MSG_HEAD(msgHead, rabmMsgID) do { \
    (msgHead)->ulMsgId         = (rabmMsgID);      \
    (msgHead)->ulSenderCpuId   = VOS_LOCAL_CPUID;    \
    (msgHead)->ulSenderPid     = PS_PID_IP;          \
    (msgHead)->ulReceiverCpuId = VOS_LOCAL_CPUID;    \
    (msgHead)->ulReceiverPid   = PS_PID_RABM; \
} while (0)

#define IP_ADDR_IS_INVALID(ipAddr) ((ipAddr[0] == 0) && (ipAddr[1] == 0) && (ipAddr[2] == 0) && (ipAddr[3] == 0))

#define IP_ADDR_IS_IDENTICAL(ipAddr1, ipAddr2)                                                 \
    ((ipAddr1[0] == ipAddr2[0]) && (ipAddr1[1] == ipAddr2[1]) && (ipAddr1[2] == ipAddr2[2]) && \
     (ipAddr1[3] == ipAddr2[3]))

/* 获取IP版本号 */
#define IP_GetIpVersion(ipMsg) ((ipMsg)[0] >> IP_BITMOVE_4)

/* 获取IPV4包头部长度 */
#define IP_GetIpv4HeaderLen(ipMsg) (((ipMsg)[0] & IP_8_LOW_BYTE) * 4)

/* 获取协议号 */
#define IP_GetProtocol(ipMsg) ((ipMsg)[IP_HEAD_PROTOCOL_OFF])

/* 获取IPV4数据包长度 */
#define IP_GetIpv4PacketLen(ipMsg, ipv4PacketLen) do { \
    ipv4PacketLen = (VOS_UINT16)(*((ipMsg) + IP_HEAD_IP_LEN)) << IP_BITMOVE_8;       \
    ipv4PacketLen = ipv4PacketLen | (VOS_UINT16)(*((ipMsg) + IP_HEAD_IP_LEN + 1)); \
} while (0)

/* 获取UDP的目的端口 */
#define IP_GetUdpDesPort(udpMsg, desPort) do { \
    desPort = (VOS_UINT16)(*((udpMsg) + 2)) << IP_BITMOVE_8; \
    desPort = desPort | (VOS_UINT16)(*((udpMsg) + 3));     \
} while (0)

#define IP_FormTimerType(ipModule, timerIndex) \
    (((ipModule & IP_32_FOURTH_BYTE) << IP_BITMOVE_8) | (timerIndex & IP_32_FOURTH_BYTE))

#define IP_MOVEMENT_4_BITS (4)
#define IP_IPDATA_HIGH_4_BIT_MASK (0xF0)
#define IP_IPDATA_LOW_4_BIT_MASK (0x0F)

/*
 * 4 Enum
 */
/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:  IP包字段偏移
 */
enum IP_HeadOffset {
    IP_HEAD_VERSION_OFF   = 0, /* 版本偏移 */
    IP_HEAD_IP_LEN        = 2, /* 长度偏移 */
    IP_HEAD_PROTOCOL_OFF  = 9, /* 协议偏移 */
    IP_HEAD_CHECK_SUM_OFF = 10,
    IP_HEAD_SRC_IP_OFF    = 12, /* 源IP偏移 */
    IP_HEAD_DESC_IP_OFF   = 16, /* 目的IP偏移 */

    IP_HEAD_BUTT
};
typedef VOS_UINT16 IP_HeadOffsetUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:  运输层端口枚举
 */
enum IP_PortType {
    IP_PORT_DHCPV4_SERVER = 67,
    IP_PORT_DHCPV4_CLIENT = 68,

    IP_PORT_BUTT
};
typedef VOS_UINT16 IP_PortTypeUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: IP版本
 */
enum IP_VersionType {
    IP_VERSION_4 = 4,
    IP_VERSION_6 = 6,

    IP_VERSION_BUTT
};
typedef VOS_UINT8 IP_VersionTypeUint8;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 发送方式
 */
enum IP_HeadProtocol {
    IP_HEAD_PROTOCOL_ICMPV4 = 1,
    IP_HEAD_PROTOCOL_TDP    = 6,
    IP_HEAD_PROTOCOL_UDP    = 17,
    IP_HEAD_PROTOCOL_ICMPV6 = 58,

    IP_HEAD_PROTOCOL_BUTT
};
typedef VOS_UINT32 IP_HeadProtocolUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:  DHCPV4 options类型
 */
enum IP_DhcpV4OptionType {
    IP_DHCPV4_OPTION_SUBNET_MASK        = 1,  /* 子网掩码 */
    IP_DHCPV4_OPTION_ROUTER_IP          = 3,  /* 默认网关 */
    IP_DHCPV4_OPTION_DOMAIN_NAME_SERVER = 6,  /* DNS */
    IP_DHCPV4_OPTION_REQUEST_IP_ADDR    = 50, /* Request IP Address */
    IP_DHCPV4_OPTION_LEASE_TIME         = 51, /* 租期 */
    IP_DHCPV4_OPTION_MSG_TYPE           = 53, /* 报文类型 */
    IP_DHCPV4_OPTION_SERVER_IP          = 54, /* Server Identifier */
    IP_DHCPV4_OPTION_REQUEST_LIST       = 55, /* request parameter list */
    IP_DHCPV4_OPTION_T1                 = 58, /* Rebinding (T1) Time Value */
    IP_DHCPV4_OPTION_T2                 = 59, /* Rebinding (T2) Time Value */
    IP_DHCPV4_OPTION_RAPID_COMMIT       = 80, /* rapid commit */

    IP_DHCPV4_OPTION_BUTT
};
typedef VOS_UINT8 IP_DhcpV4OptionTypeUint8;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:   DHCPV4 报文头偏移
 */
enum IP_DhcpV4HeadOffset {
    IP_DHCPV4_HEAD_OP_OFF     = 0,
    IP_DHCPV4_HEAD_HTYPE_OFF  = 1,
    IP_DHCPV4_HEAD_HLEN_OFF   = 2,
    IP_DHCPV4_HEAD_HOPS_OFF   = 3,
    IP_DHCPV4_HEAD_XID_OFF    = 4,
    IP_DHCPV4_HEAD_SECS_OFF   = 8,
    IP_DHCPV4_HEAD_FLAGS_OFF  = 10,
    IP_DHCPV4_HEAD_CIADDR_OFF = 12,
    IP_DHCPV4_HEAD_YIADDR_OFF = 16,
    IP_DHCPV4_HEAD_SIADDR_OFF = 20,
    IP_DHCPV4_HEAD_GIADDR_OFF = 24,
    IP_DHCPV4_HEAD_CHADDR_OFF = 28,
    IP_DHCPV4_HEAD_COOKIE_OFF = 236,

    IP_DHCPV4_HEAD_BUTT
};
typedef VOS_UINT16 IP_DhcpV4HeadOffsetUint16;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:   DHCPV4 报文类型
 */
enum IP_DhcpV4Msg {
    IP_DHCPV4_MSG_DISCOVER = 1,
    IP_DHCPV4_MSG_OFFER    = 2,
    IP_DHCPV4_MSG_REQUEST  = 3,
    IP_DHCPV4_MSG_DECLINE  = 4,
    IP_DHCPV4_MSG_ACK      = 5,
    IP_DHCPV4_MSG_NAK      = 6,
    IP_DHCPV4_MSG_RELEASE  = 7,
    IP_DHCPV4_MSG_INFORM   = 8,

    IP_DHCPV4_MSG_BUTT
};
typedef VOS_UINT32 IP_Dhcpv4MsgUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 以太网协议payload枚举
 */
enum IP_GmacPayloadType {
    IP_GMAC_PAYLOAD_TYPE_ARP  = 0x0806,
    IP_GMAC_PAYLOAD_TYPE_IPV4 = 0x0800,
    IP_GMAC_PAYLOAD_TYPE_IPV6 = 0X86dd,

    IP_GMAC_PAYLOAD_TYPE_BUTT
};
typedef VOS_UINT32 IP_GmacPayloadTypeUint32;

/*
 * 5 STRUCT
 */
/*
 * 结构名: IP_Ipv4Head
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV4头结构
 */
typedef struct {
    VOS_UINT8  ipv4Version : 4;             /* 版本 */
    VOS_UINT8  headLength : 4;          /* 首部长度，单位为4字节 */
    VOS_UINT8  priorityValue : 3;            /* 优先级 */
    VOS_UINT8  ipv4Dvalue : 1;                   /* 表示要求有更低的时延 */
    VOS_UINT8  ipv4TValue : 1;                   /* 表示要求有更高的吞吐量 */
    VOS_UINT8  ipv4Rvalue : 1;                   /* 表示有更高的可靠性 */
    VOS_UINT8  ipv4CValue : 1;                   /* 表示要求选择代价更小的路由 */
    VOS_UINT8  reserved : 1;            /* 目前尚未使用 */
    VOS_UINT16 totalLength;                   /* 总长度，单位:字节 */
    VOS_UINT16 identification;           /* 标识 */
    VOS_UINT16 mfFlag : 1;              /* MF=1表示有分片 */
    VOS_UINT16 dfFlag : 1;              /* DF=0表示允许分片 */
    VOS_UINT16 reservedFlag : 1;        /* 目前未使用 */
    VOS_UINT16 flagmentExcursion : 13;  /* 片偏移 */
    VOS_UINT8  ttlValue;                      /* 生存时间 */
    VOS_UINT8  protocolValue;                 /* 协议 */
    VOS_UINT16 fcsValue;                      /* 首部检验和 */
    VOS_UINT8  ipSrc[IP_IPV4_ADDR_LEN]; /* 源IP地址 */
    VOS_UINT8  ipDes[IP_IPV4_ADDR_LEN]; /* 目的IP地址 */
} IP_Ipv4Head;                        /* IP数据报头部结构体 */

/*
 * 结构名: IP_UdpHead
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP头结构
 */
typedef struct {
    VOS_UINT16 portSrc; /* 源端口号 */
    VOS_UINT16 portDes; /* 目的端口号 */
    VOS_UINT16 udpLength;  /* 整个UDP长度 */
    VOS_UINT16 fcsValue;     /* 整个UDP检验和 */
} IP_UdpHead;        /* UDP数据报头结构体 */

/*
 * 结构名: IP_UdpFakeHead
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP伪首部
 */
typedef struct {
    VOS_UINT8  ipSrc[IP_IPV4_ADDR_LEN]; /* 源IP地址 */
    VOS_UINT8  ipDes[IP_IPV4_ADDR_LEN]; /* 目的IP地址 */
    VOS_UINT8  allZero;                     /* 全部为0 */
    VOS_UINT8  protocolValue;                 /* IP首部的协议字段 */
    VOS_UINT16 udpLength;                   /* 整个UDP长度，不包括伪首部长度 */
} IP_UdpFakeHead;                     /* UDP伪首部结构体 */

/*
 * 结构名: IP_TimerInfo
 * 协议表格:
 * ASN.1描述:　
 * 结构说明: 定时器结构体
 */
typedef struct {
    HTIMER     hTm;         /* 定时器指针 */
    VOS_UINT32 timerName;      /* 定时器名字 */
    VOS_UINT8  loopTimes; /* 循环启动次数 */
    VOS_UINT8  timerStatus;    /* 定时器的状态 */
    VOS_UINT8  reserved[2]; /* 2:rsv */
} IP_TimerInfo; /* 定时器结构体 */

/*
 * 结构名: IP_GmacHeader
 * 协议表格:
 * ASN.1描述:
 * 结构说明:以太网帧头结构体
 */
typedef struct {
    VOS_UINT8  macAddrDes[IP_MAC_ADDR_LEN];
    VOS_UINT8  macAddrSrc[IP_MAC_ADDR_LEN];
    VOS_UINT16 frmType;
    VOS_UINT8  reserved[2]; /* 2:rsv */
} IP_GmacHeader;

/*
 * 结构名: IP_DhcpV4Lease
 * 协议表格:
 * ASN.1描述:
 * 结构说明: DHCPv4租期结构
 */
typedef struct {
    VOS_UINT32 leaseT;
    VOS_UINT32 leaseT1;
    VOS_UINT32 leaseT2;
} IP_DhcpV4Lease;

/*****************************************************************************
 结构名    : L2_MSGTYPE_STRU_LEN
 协议表格  :
 ASN.1描述 :
 结构说明  : L2消息结构体长度
*****************************************************************************/
typedef struct {
    VOS_UINT32                      msgId;        /* 消息ID */
    VOS_UINT32                      msTypeLenth;  /* 消息类型长度 */
} ND_MsgTypeStruLen;
typedef struct {
    VOS_UINT8  ipv6Addr[IP_IPV6_ADDR_LEN];  /* 消息类型长度 */
} ND_Ipv6AddrInfo;

typedef struct {
    VOS_UINT8  macAddr[IP_MAC_ADDR_LEN];  /* 消息类型长度 */
    VOS_UINT8  resv[2]; /* 2:rsv */
} ND_MacAddrInfo;

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

#endif /* end of ip_ipm_global.h */
