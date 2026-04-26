/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/20
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

#ifndef __IPV4DHCPSERVER_H__
#define __IPV4DHCPSERVER_H__

#include "vos.h"
#include "ps_tag.h"
#include "ndis_entity.h"
#include "ttf_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*
 * 2 宏定义
 */
#ifndef NDIS_RELEASE

#if (VOS_WIN32 == VOS_OS_VER)
#define NDIS_ASSERT PS_PRINTF_INFO("Assert, File: %s, Line:%d\n", __FILE__, __LINE__)
#else
#define NDIS_ASSERT vos_assert((VOS_UINT32)__FILE__, (VOS_INT)__LINE__)
#endif

#define IP_NDIS_ASSERT(exp) do { \
    if (!(exp)) {           \
        NDIS_ASSERT;        \
        return;             \
    } \
} while (0)

#define IP_NDIS_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {                    \
        NDIS_ASSERT;                 \
        return (ret);                \
    } \
} while (0)

#else
#define IP_NDIS_ASSERT(exp) ((VOS_VOID)0)
#define IP_NDIS_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

/* 取结构成员的偏移地址 */
#define IP_OFFSETOF(ipType, memBer) ((VOS_UINT32) & (((ipType *)0)->memBer))

/* 定义网络字节与主机字节序转换 */
#if (VOS_BYTE_ORDER == VOS_BIG_ENDIAN)

#define IP_NTOHL(x) (x)
#define IP_HTONL(x) (x)
#define IP_NTOHS(x) (x)
#define IP_HTONS(x) (x)

#else

#define IP_NTOHL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define IP_HTONL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define IP_NTOHS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#define IP_HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#endif

/* 针对当前打印进行宏定义 */
#ifndef NDIS_RELEASE
#define IP_INFO_LOG(prinString) PS_PRINTF_INFO("%s,%d,%s \r\n", __FILE__, __LINE__, prinString)
#define IP_ERROR_LOG(prinString) PS_PRINTF_INFO("%s,%d,%s \r\n", __FILE__, __LINE__, prinString)
#define IP_ERROR_LOG1(prinString, para1) PS_PRINTF_INFO("%s,%d,%s,%d \r\n", __FILE__, __LINE__, prinString, para1)
#define IP_ERROR_LOG2(prinString, para1, para2) \
    PS_PRINTF_INFO("%s,%d,%s,%d,%d \r\n", __FILE__, __LINE__, prinString, para1, para2)
#else
#define IP_INFO_LOG(prinString)
#define IP_ERROR_LOG(prinString)
#define IP_ERROR_LOG1(prinString, para1)
#define IP_ERROR_LOG2(prinString, para1, para2)
#endif

/* UDP 协议头长度 */
#define IP_UDP_HDR_LEN 8

/* IPV4 DHCP Pkt Hostname Length */
#define IPV4_DHCP_SERVER_HOST_NAME_LEN 64

/* IPV4 DHCP Pkt Filename Length */
#define IPV4_DHCP_FILE_NAME_LEN 128

/* IP Address Lease Time 单位为s  */
#define IPV4_DHCP_LEASE_TIME (g_leaseTime)

/*  Renewal (T1) Time Value */
#define IPV4_DHCP_T1 (IPV4_DHCP_LEASE_TIME / 2)

/* Rebinding (T2) Time Value */
#define IPV4_DHCP_T2 ((IPV4_DHCP_LEASE_TIME * 7) / 8)

/* DHCP协议option的起始偏移 */
#define IPV4_DHCP_OPTION_OFFSET 240

/* DHCP Option 每一项头所占的长度 */
#define IP_DHCPV4_OPTION_ITEM_HDR_LEN 2

/* dhcp 报文头cookie字段长度 */
#define IPV4_DHCP_HEAD_COOKIE_LEN 4

/* DHCP协议单元中硬件地址长度 */
#define IPV4_DHCP_HARDWARE_LEN 16

/* DHCP 报文类型 */
#define IPV4_DHCP_OPERATION_REQUEST 1
#define IPV4_DHCP_OPERATION_REPLY 2

/* 以太网硬件类型 */
#define IPV4_DHCP_ETHERNET_HTYPE 1

/* dhcp 报文IP Address Lease Time 长度 */
#define IPV4_DHCP_OPTION_LEASE_TIME_LEN 4

/* DHCP 报文中Your IP Addr Offset */
#define IPV4_DHCP_HEAD_YIADDR_OFFSET 16

/* dhcp msg type类型长度 */
#define IPV4_DHCP_OPTION_MSG_TYPE_LEN 1

#define IPV4_DHCP_GET_BUFFER() (g_dhcpBuff)

/*
 * 结构名: IPV4_DhcpAddrStatus
 * 协议表格: 定义IPV4 DHCP 地址的状态
 * ASN.1描述:
 * 结构说明:
 */
typedef enum {
    IPV4_DHCP_ADDR_STATUS_FREE     = 0,
    IPV4_DHCP_ADDR_STATUS_ASSIGNED = 1,
    IPV4_DHCP_ADDR_STATUS_BUTT
} IPV4_DhcpAddrStatus;

typedef VOS_UINT32 IPV4_DhcpAddrStatusUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 发送方式
 */
enum IP_CastType {
    IP_CAST_TYPE_UNICAST  = 0,
    IP_CAST_TYPE_BRODCAST = 1,
    IP_CAST_TYPE_BUTT
};
typedef VOS_UINT8 IP_CastTypeUint8;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明:  IPV4 DHCP options类型
 */
enum IPV4_DhcpOptionType {
    IPV4_DHCP_OPTION_PAD_OPTIOIN         = 0,   /* padding用于字对齐 */
    IPV4_DHCP_OPTION_SUBNET_MASK         = 1,   /* 子网掩码 */
    IPV4_DHCP_OPTION_ROUTER_IP           = 3,   /* 默认网关 */
    IPV4_DHCP_OPTION_DOMAIN_NAME_SERVER  = 6,   /* DNS */
    IPV4_DHCP_OPTION_NETBIOS_NAME_SERVER = 44,  /* WINS NAME Server */
    IPV4_DHCP_OPTION_REQUEST_IP_ADDR     = 50,  /* Request IP Address */
    IPV4_DHCP_OPTION_LEASE_TIME          = 51,  /* 租期 */
    IPV4_DHCP_OPTION_MSG_TYPE            = 53,  /* 报文类型 */
    IPV4_DHCP_OPTION_SERVER_IP           = 54,  /* Server Identifier */
    IPV4_DHCP_OPTION_REQUEST_LIST        = 55,  /* request parameter list */
    IPV4_DHCP_OPTION_T1                  = 58,  /* Rebinding (T1) Time Value */
    IPV4_DHCP_OPTION_T2                  = 59,  /* Rebinding (T2) Time Value */
    IPV4_DHCP_OPTION_RAPID_COMMIT        = 80,  /* rapid commit */
    IPV4_DHCP_OPTION_END_OPTION          = 255, /* option结束标记 */
    IPV4_DHCP_OPTION_BUTT
};

typedef VOS_UINT8 IPV4_DhcpOptionTypeUint8;
/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: IPV4 DHCP 报文消息类型，协议规定RFC2131
 */
enum IPV4_DhcpMsg {
    IPV4_DHCP_MSG_DISCOVER = 1,
    IPV4_DHCP_MSG_OFFER    = 2,
    IPV4_DHCP_MSG_REQUEST  = 3,
    IPV4_DHCP_MSG_DECLINE  = 4,
    IPV4_DHCP_MSG_ACK      = 5,
    IPV4_DHCP_MSG_NAK      = 6,
    IPV4_DHCP_MSG_RELEASE  = 7,
    IPV4_DHCP_MSG_INFORM   = 8,
    IPV4_DHCP_MSG_BUTT
};

typedef VOS_UINT32 IPV4_DhcpMsgUint32;

/*
 * 7 STRUCT定义
 */
/* 针对网络字节序，设置为1字节对齐 */
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

/*
 * 结构名: IPV4_Udpipfakehdr
 * 协议表格:
 * ASN.1描述:
 * 结构说明: 用于计算UDP CRC
 */
typedef struct {
    /* Fake Hdr */
    VOS_UINT8 rec0[8];         /* 8:全部为0不影响CRC计算，为了同IP头大小保持相同 */
    IPV4_AddrItem srcIP;    /* 源IP地址 */
    IPV4_AddrItem dstIP;    /* 目的IP地址 */
    VOS_UINT8         all0;     /* 全部为0 */
    VOS_UINT8         protocol; /* IP首部的协议字段 */
    VOS_UINT16        length;   /* 整个UDP长度，不包括伪首部长度 */

    /* Udp Hdr */
    VOS_UINT16 srcPort;  /* source port */
    VOS_UINT16 dstPort;  /* dest port */
    VOS_UINT16 len;      /* udp length */
    VOS_UINT16 checksum; /* udp check sum */
} IPV4_Udpipfakehdr;

/*
 * 结构名: IPV4_DHCPSERVER_PROTOCL_STRU
 * 协议表格:
 * ASN.1描述:
 * 结构说明: DHCPV4协议结构体
 */
typedef struct {
    VOS_UINT8 op;                     /* 报文类型，1：客户端请求报文，2：服务器响应报文 */
    VOS_UINT8         hardwareType;   /* 硬件类型，1：以太网 */
    VOS_UINT8         hardwareLength; /* 硬件地址长度 */
    VOS_UINT8         hops;           /* 经过的中继数目 */
    VOS_UINT32        transactionID;  /* 用户选取的随机数，使服务器的回复与用户的报文相关联 */
    VOS_UINT16        seconds;        /* 客户端在开始DHCP请求后的逝去的秒数 */
    VOS_UINT16        flags; /* 只有标志字段的最高位才有意义，其余的位均被置为0，最高位0：服务器以单播形式发送响应报文，最高位1：服务器以广播形式发送响应报文 */
    IPV4_AddrItem clientIPAddr;                                    /* 客户端的IP地址 */
    IPV4_AddrItem yourIPAddr;                                      /* 服务器分配给客户端的IP地址 */
    IPV4_AddrItem nextServerIPAddr;                                /* 服务器的IP地址 */
    IPV4_AddrItem relayIPAddr;                                     /* 中继的IP地址 */
    VOS_UINT8         clientHardwardAddr[IPV4_DHCP_HARDWARE_LEN];     /* 客户端的MAC地址 */
    VOS_UINT8         serverHostName[IPV4_DHCP_SERVER_HOST_NAME_LEN]; /* 服务器名字 */
    VOS_UINT8         fileName[IPV4_DHCP_FILE_NAME_LEN];              /* 客户端的启动配置文件名 */
    VOS_UINT8         magicCookie[IPV4_DHCP_HEAD_COOKIE_LEN];         /* MAGIC COOKIE */
} IPV4_DhcpProtocl;

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif
/*
 * 结构名: IPV4_DhcpAnalyseResult
 * 协议表格:
 * ASN.1描述:
 * 结构说明: 解析DHCP包结果
 */
typedef struct {
    IPV4_AddrItem         srcIPAddr;
    IPV4_AddrItem         dstIpAddr;
    VOS_UINT32                transactionID;
    IPV4_AddrItem         clientIPAddr;
    IPV4_AddrItem         yourIPAddr;
    IPV4_DhcpMsgUint32    msgType;
    IPV4_AddrItem         requestIPAddr;
    IPV4_AddrItem         serverIPAddr;

    VOS_UINT8 hardwareLen;
    VOS_UINT8 rec[3]; /* 3:rsv */

    VOS_UINT16 castFlg;
    VOS_UINT8  hardwareAddr[ETH_MAC_ADDR_LEN];
} IPV4_DhcpAnalyseResult;

/*
 * 结构名: IPV4_DhcpOptionItem
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV4 DHCP Option结构体
 */
typedef struct {
    VOS_UINT8 optionType;
    VOS_UINT8 optionLen;
    VOS_UINT8 optionValue[254]; /* 254:optionValue */
} IPV4_DhcpOptionItem;

/*
 * 4 全局变量声明
 */
extern VOS_UINT32        g_leaseTime;
extern VOS_UINT8         g_dhcpBuff[];


/*
 * 10 函数声明
 */
VOS_VOID  IPV4_DHCP_ProcDhcpPkt(VOS_UINT8 *ipPkt, VOS_UINT8 rabId, VOS_UINT32 pktMemLen);
VOS_VOID  IPV4_DHCP_ProcDhcpMsg(IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 rabId);

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

#endif /* end of ipv4_dhcp_server.h */
