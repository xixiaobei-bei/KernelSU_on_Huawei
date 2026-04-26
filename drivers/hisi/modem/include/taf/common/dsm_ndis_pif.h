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

#ifndef __DSM_NDIS_PIF_H__
#define __DSM_NDIS_PIF_H__

#include "vos.h"
#include "taf_ps_type_def.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

/* 暂时先通过IFACE BASE进行定义，等之后AT整体下移后，再改BASE和脱敏 */
#define DSM_NDIS_MSG_ID_HEADER (TAF_PS_IFACE_ID_BASE + 0x00F0)

#define DSM_NDIS_IPV4_ADDR_LENGTH 4
#define DSM_NDIS_IPV6_ADDR_LENGTH 16
#define DSM_NDIS_MAX_PREFIX_NUM_IN_RA 6
#define DSM_NDIS_IPV6_IFID_LENGTH 8

/*
 * 枚举说明: DSM与NDIS的消息定义
 */
enum DMS_NDIS_MsgId {
    /* DSM->AT */
    /* _MSGPARSE_Interception DSM_NDIS_IfaceUpInd */
    ID_DSM_NDIS_IFACE_UP_IND        = DSM_NDIS_MSG_ID_HEADER + 0x01,
    /* _MSGPARSE_Interception DSM_NDIS_IfaceDownInd */
    ID_DSM_NDIS_IFACE_DOWN_IND      = DSM_NDIS_MSG_ID_HEADER + 0x02,
    /* _MSGPARSE_Interception DSM_NDIS_ConfigIpv6DnsInd */
    ID_DSM_NDIS_CONFIG_IPV6_DNS_IND = DSM_NDIS_MSG_ID_HEADER + 0x03,

    ID_DSM_NDIS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 DSM_NDIS_MsgIdUint32;


typedef struct {
    VOS_UINT8 ipV4Addr[DSM_NDIS_IPV4_ADDR_LENGTH];
} DSM_NDIS_Ipv4Addr;


typedef struct {
    VOS_UINT32 opPdnAddr : 1;
    VOS_UINT32 opDnsPrim : 1;
    VOS_UINT32 opDnsSec : 1;
    VOS_UINT32 opPcscfPrim : 1;
    VOS_UINT32 opPcscfSec : 1;
    VOS_UINT32 opSpare : 27;

    VOS_UINT8 sessionId;
    VOS_UINT8 reserved[3];

    DSM_NDIS_Ipv4Addr pdnAddrInfo;
    DSM_NDIS_Ipv4Addr subnetMask;        /* 子网掩码 */
    DSM_NDIS_Ipv4Addr gateWayAddrInfo;   /* 网关 */
    DSM_NDIS_Ipv4Addr dnsPrimAddrInfo;   /* 主DNS信息 */
    DSM_NDIS_Ipv4Addr dnsSecAddrInfo;    /* 辅DNS信息 */
    DSM_NDIS_Ipv4Addr pcscfPrimAddrInfo; /* 主PCSCF信息 */
    DSM_NDIS_Ipv4Addr pcscfSecAddrInfo;  /* 辅PCSCF信息 */
} DSM_NDIS_Ipv4PdnInfo;


typedef struct {
    VOS_UINT8 serNum; /* 服务器个数 */
    VOS_UINT8 rsv[3];

    VOS_UINT8 priServer[DSM_NDIS_IPV6_ADDR_LENGTH];
    VOS_UINT8 secServer[DSM_NDIS_IPV6_ADDR_LENGTH];
} DSM_NDIS_Ipv6DnsSer;


typedef DSM_NDIS_Ipv6DnsSer DSM_NDIS_IPV6_PCSCF_SER_STRU;


typedef struct {
    VOS_UINT32 bitL : 1; /* 1: VALID; 0: INVALID */
    VOS_UINT32 bitA : 1;
    VOS_UINT32 bitPrefixLen : 8;
    VOS_UINT32 bitRsv : 22;

    VOS_UINT32 validLifeTime;
    VOS_UINT32 preferredLifeTime;
    VOS_UINT8  prefix[DSM_NDIS_IPV6_ADDR_LENGTH];
} DSM_NDIS_Ipv6Prefix;


typedef struct {
    VOS_UINT32 bitOpMtu : 1;
    VOS_UINT32 bitRsv1 : 31;

    VOS_UINT32 bitCurHopLimit : 8;
    VOS_UINT32 bitM : 1;
    VOS_UINT32 bitO : 1;
    VOS_UINT32 bitRsv2 : 22;

    VOS_UINT8 pduSessionId;
    VOS_UINT8 reserved[3];

    VOS_UINT8           interfaceId[DSM_NDIS_IPV6_IFID_LENGTH];
    VOS_UINT32          mtu;
    VOS_UINT32          prefixNum;
    DSM_NDIS_Ipv6Prefix prefixList[DSM_NDIS_MAX_PREFIX_NUM_IN_RA];

    DSM_NDIS_Ipv6DnsSer          dnsSer;
    DSM_NDIS_IPV6_PCSCF_SER_STRU pcscfSer;
} DSM_NDIS_Ipv6PdnInfo;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT32 opIpv4PdnInfo : 1;
    VOS_UINT32 opIpv6PdnInfo : 1;
    VOS_UINT32 opEthPdnInfo : 1;
    VOS_UINT32 opSpare : 29;

    ModemIdUint16 modemId;
    VOS_UINT8     rabId; /* 承载号 */
    VOS_UINT8     ifaceId;

    DSM_NDIS_Ipv4PdnInfo ipv4PdnInfo;
    DSM_NDIS_Ipv6PdnInfo ipv6PdnInfo;
} DSM_NDIS_IfaceUpInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT32 opIpv4PdnInfo : 1;
    VOS_UINT32 opIpv6PdnInfo : 1;
    VOS_UINT32 opSpare : 30;

    ModemIdUint16 modemId;
    VOS_UINT8     rabId; /* 承载号 */
    VOS_UINT8     ifaceId;
} DSM_NDIS_IfaceDownInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT32 opIpv6PriDns : 1;
    VOS_UINT32 opIpv6SecDns : 1;
    VOS_UINT32 opIpv6Spare : 30;

    /* 从 PDP上下文带来的IPV6主DNS长度，不包括":" */
    VOS_UINT8 ipv6PrimDns[TAF_IPV6_ADDR_LEN];
    /* 从 PDP上下文带来的IPV6副DNS长度，不包括":" */
    VOS_UINT8 ipv6SecDns[TAF_IPV6_ADDR_LEN];
    VOS_UINT8 ifaceId;
    VOS_UINT8 rsv[3];
} DSM_NDIS_ConfigIpv6DnsInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __DSM_NDIS_PIF_H__ */
