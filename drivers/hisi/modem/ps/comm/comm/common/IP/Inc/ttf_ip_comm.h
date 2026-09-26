/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
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

#ifndef __TTFIPCOMM_H__
#define __TTFIPCOMM_H__

#include "vos.h"
#include "ttf_comm.h"
#include "ttf_link_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* IP头按照协议解析出来的格式 */
enum IpDataProtocalEnum {
    IP_DATA_PROTOCOL_ICMPV4        = 0x01, /* ICMPV4协议标识的值 */
    IP_DATA_PROTOCOL_IGMP          = 0x02, /* IGMP协议标识的值 */
    IP_DATA_PROTOCOL_IPINIP        = 0x04, /* IP in IP (encapsulation)协议标识的值 */
    IP_DATA_PROTOCOL_TCP           = 0x06, /* TCP协议标识的值 */
    IP_DATA_PROTOCOL_UDP           = 0x11, /* UDP协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_ROUTE    = 0x2b, /* Routing Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_FRAGMENT = 0x2c, /* Fragment Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_ESP           = 0x32, /* Encapsulating Security Payload协议标识的值 */
    IP_DATA_PROTOCOL_AH            = 0x33, /* Authentication Header协议标识的值 */
    IP_DATA_PROTOCOL_ICMPV6        = 0x3a, /* ICMPV6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_NONEXT   = 0x3b, /* No Next Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_OPTS     = 0x3c, /* Destination Options for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_IPIP     = 0x5e, /* IP-within-IP Encapsulation Protocol协议标识的值 */
    IP_DATA_PROTOCOL_IPCOMP        = 0x6c, /* IP Payload Compression Protocol协议标识的值 */

    IP_DATA_PROTOCOL_BUTT
};
typedef VOS_UINT8 IpDataProtocolEnumUint8;

/* IP数据承载协议 */
enum IpDataTypeEnum {
    IP_DATA_TYPE_NULL           = 0,  /* NULL */
    IP_DATA_TYPE_ICMP           = 1,  /* ICMP */
    IP_DATA_TYPE_IGMP           = 2,  /* IGMP */
    IP_DATA_TYPE_TCP            = 3,  /* TCP */
    IP_DATA_TYPE_TCP_SYN        = 4,  /* TCP ACK */
    IP_DATA_TYPE_TCP_ACK        = 5,  /* TCP ACK */
    IP_DATA_TYPE_FTP_SIGNALLING = 6,  /* FTP SIGNALLING */
    IP_DATA_TYPE_UDP            = 7,  /* UDP */
    IP_DATA_TYPE_UDP_DNS        = 8,  /* UDP DNS */
    IP_DATA_TYPE_UDP_MIP        = 9,  /* UDP MIP AGENT */
    IP_DATA_TYPE_USER_HIGH      = 10, /* user high data */

    IP_DATA_TYPE_BUTT
};
typedef VOS_UINT8 IpDataTypeEnumUint8;

#define TTF_MASK_IPV4_ADDR(ipv4Addr) ((ipv4Addr)&0xFFFFFF00)

#define TTF_IP_MAX_OPT 40
#define TTF_TCP_MAX_OPT 40
#define TTF_TCP_HEAD_NORMAL_LEN 20
#define TTF_UDP_LEN_POS 4
#define TTF_IP_FRAG_POS_IN_HEAD 0x3

#define TTF_IP_HEAD_TO_REAL_LEN 0x2
#define TTF_IP_FRAG_TO_REAL_LEN 0x3

/* IP flags. */
#define TTF_IP_CE 0x8000     /* Flag: "Congestion"       */
#define TTF_IP_DF 0x4000     /* Flag: "Don't Fragment"   */
#define TTF_IP_MF 0x2000     /* Flag: "More Fragments"   */
#define TTF_IP_OFFSET 0x1FFF /* "Fragment Offset" part   */

#define TTF_TCP_HEAD_TO_REAL_LEN 0x2

#define TTF_IP_CONGEST_MASK 0x8000
#define TTF_IP_DF_MASK 0x4000
#define TTF_IP_MF_MASK 0x2000
#define TTF_IP_FRAGMENT_OFFSET_MASK 0x1FFF

#ifndef ntohs
#define ntohs(x) ((((VOS_UINT16)(x)&0x00ff) << 8) | (((VOS_UINT16)(x)&0xff00) >> 8))
#endif

typedef struct {
    VOS_UINT8  ihl : 4,
               ver : 4;
    VOS_UINT8  tos;
    VOS_UINT16 tot_len;
    VOS_UINT16 id;
    VOS_UINT16 frag_off;
    VOS_UINT8  ttl;
    VOS_UINT8  protocol;
    VOS_UINT16 csum;
    VOS_UINT32 saddr;
    VOS_UINT32 daddr;
} TtfIpHdr;

typedef struct {
    VOS_UINT16 sPort;
    VOS_UINT16 dPort;
    VOS_UINT32 seq;
    VOS_UINT32 ack_seq;
    VOS_UINT16 doff : 4,
               res : 4,
               cwr : 1,
               ece : 1,
               urg : 1,
               ack : 1,
               psh : 1,
               rst : 1,
               syn : 1,
               fin : 1;
    VOS_UINT16 windowsize;
    VOS_UINT16 csum;
    VOS_UINT16 urg_ptr;
} TtfTcpHdr;

typedef struct {
    VOS_UINT16 sPort;
    VOS_UINT16 dPort;
    VOS_UINT16 len;
    VOS_UINT16 csum;
} TtfUdpHdr;

#pragma pack(pop)

/* 用于解析IP包协议类型; data 入参有效性由调用者保证，为IP数据包 */
IpDataTypeEnumUint8 TTF_ParseIpDataType(VOS_UINT32 pid, const VOS_UINT8 *data, VOS_UINT16 dataLen,
    TTF_PsDataPriorityUint8 dataPriority);

/*
 * 用来获得可维可测勾取IP报的安全长度，以免泄露用户隐私
 *  *data 入参有效性由调用者保证，为IP数据包
 */
VOS_UINT16 TTF_GetIpDataTraceLen(VOS_UINT32 pid, const VOS_UINT8 *data, VOS_UINT16 sduLen);

/*
 * TTF勾取非敏感的IP地址 (因为IPV4在某些国际是否是个人数据还存在争议,
 * 因此modem建议掩掉IP地址的后几位, IPv4掩掉后8bit, IPV6掩掉后88bit)
 * ipData指向ip协议起始的位置，必须是连续的内存; dataLen为缓冲区实际长度，函数会对dataLen进行校验
 */
VOS_VOID TTF_MaskIpAddrTraces(VOS_UINT32 pid, VOS_UINT8 *ipData, VOS_UINT16 dataLen);

/*
 * 将IPV4地址的敏感信息脱敏处理，IPv4掩掉后8bit。
 * 外部输入保证ipAddr指向空间满足IPV4 4字节存储大小
 */
VOS_VOID TTF_FilterIpv4AddrSensitiveInfo(VOS_UINT8 *ipAddr);

/*
 * 将IPV6地址的敏感信息脱敏处理，IPV6掩掉后88bit。
 * 外部输入保证ipAddr指向空间满足IPV6 16字节存储大小
 */
VOS_VOID TTF_FilterIpv6AddrSensitiveInfo(VOS_UINT8 *ipAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
