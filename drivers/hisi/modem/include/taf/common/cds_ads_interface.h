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

#ifndef __CDS_ADS_INTERFACE_H__
#define __CDS_ADS_INTERFACE_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#ifdef WIN32
#pragma warning(disable : 4200) /* zero-sized array in struct/union */
#endif

#define ADS_CDS_MSG_ID_HEADER (0x0000)
#define CDS_ADS_MSG_ID_HEADER (0x0010)

#define CDS_ADS_MSG_ID_HEADER_V2 (0x0100)
#define ADS_CDS_MSG_ID_HEADER_V2 (0x0120)

/*
 * 枚举说明: CDS和ADS的之间的消息
 */
enum CDS_ADS_MsgId {
    /* 0x01~0x03 消息废弃 */
    /* CDS<--->ADS */
    ID_CDS_ADS_DATA_IND = CDS_ADS_MSG_ID_HEADER + 0x04,
    ID_ADS_CDS_ERR_IND  = ADS_CDS_MSG_ID_HEADER + 0x04,

    /* CDS<--->ADS */
    ID_CDS_ADS_DATA_IND_V2 = CDS_ADS_MSG_ID_HEADER_V2 + 0x01,
    ID_ADS_CDS_DATA_IND_V2 = ADS_CDS_MSG_ID_HEADER_V2 + 0x01,

    /* ADS--->CDS */
    ID_ADS_CDS_ERR_IND_V2 = ADS_CDS_MSG_ID_HEADER_V2 + 0x11,
    ID_ADS_CDS_DHCPV6_IND = ADS_CDS_MSG_ID_HEADER_V2 + 0x12,

    ID_CDS_ADS_MSG_ID_BUTT
};
typedef VOS_UINT32 CDS_ADS_MsgIdUint32;

/*
 * 枚举说明: CDS与ADS的IP消息包类型定义
 */
enum CDS_ADS_IpPacketType {
    CDS_ADS_IP_PACKET_TYPE_DHCP_SERVERV4   = 0x00,
    CDS_ADS_IP_PACKET_TYPE_ND_SERVERDHCPV6 = 0x01,
    CDS_ADS_IP_PACKET_TYPE_ICMPV6          = 0x02,
    CDS_ADS_IP_PACKET_TYPE_LINK_FE80       = 0x03,
    CDS_ADS_IP_PACKET_TYPE_LINK_FF         = 0x04,
    CDS_ADS_IP_PACKET_TYPE_BUTT
};
typedef VOS_UINT8 CDS_ADS_IpPacketTypeUint8;

/*
 * 枚举说明: CDS与ADS下行的IPF Bearer Id定义(用于B5000之前的平台)
 *           IPF Bearer ID:
 *               0~4      保留
 *               5~15     EPS Bearer ID
 *               16       下行DHCP
 *               17       下行DHCPv6
 *               18       下行ICMP
 *               19       下行ICMPv6
 *               20       下行Link Local Address FE80
 *               21       下行Link Local Address FF
 *               22       下行MIP
 *               23       下行MIP
 *               24~62    暂时保留，可扩展
 *               63       不匹配任何Filter
 */
enum CDS_ADS_DlIpfBearerId {
    CDS_ADS_DL_IPF_BEARER_ID_RSV0          = 0,
    CDS_ADS_DL_IPF_BEARER_ID_EPSBID5       = 5,
    CDS_ADS_DL_IPF_BEARER_ID_EPSBID15      = 15,
    CDS_ADS_DL_IPF_BEARER_ID_DHCPV4        = 16,
    CDS_ADS_DL_IPF_BEARER_ID_DHCPV6        = 17,
    CDS_ADS_DL_IPF_BEARER_ID_ICMPV4        = 18,
    CDS_ADS_DL_IPF_BEARER_ID_ICMPV6        = 19,
    CDS_ADS_DL_IPF_BEARER_ID_LL_FE80       = 20,
    CDS_ADS_DL_IPF_BEARER_ID_LL_FF         = 21,
    CDS_ADS_DL_IPF_BEARER_ID_MIP_ADV       = 22,
    CDS_ADS_DL_IPF_BEARER_ID_MIP_REG_REPLY = 23,
    CDS_ADS_DL_IPF_BEARER_ID_INVALID       = 63
};
typedef VOS_UINT8 CDS_ADS_DlIpfBearerIdUint8;

/*
 * 枚举说明: ADS给CDS的数据包类型
 */
enum ADS_CDS_PktType {
    ADS_CDS_IPF_PKT_TYPE_IP  = 0x00,
    ADS_CDS_IPF_PKT_TYPE_PPP = 0x01,

    ADS_CDS_IPF_PKT_TYPE_BUTT
};
typedef VOS_UINT8 ADS_CDS_IpfPktTypeUint8;

/*
 * 枚举说明: IPF下行过滤结果中BID定义(用于B5000及之后的平台)
 *           IPF Bearer ID:
 *               0~4      保留
 *               5~15     EPS Bearer ID
 *               16       下行DHCP
 *               17       下行DHCPv6
 *               18       下行ICMP
 *               19       下行ICMPv6
 *               20       下行Link Local Address FE80
 *               21       下行Link Local Address FF
 *               22       下行MIP
 *               23       下行MIP
 *               24       下行ICMPv6 RA
 *               25~254   暂时保留，可扩展
 *               255      不匹配任何Filter
 */
enum CDS_ADS_DlIpfBid {
    CDS_ADS_DL_IPF_BID_RSV0          = 0,
    CDS_ADS_DL_IPF_BID_EPSBID5       = 5,
    CDS_ADS_DL_IPF_BID_EPSBID15      = 15,
    CDS_ADS_DL_IPF_BID_DHCPV4        = 16,
    CDS_ADS_DL_IPF_BID_DHCPV6        = 17,
    CDS_ADS_DL_IPF_BID_ICMPV4        = 18,
    CDS_ADS_DL_IPF_BID_ICMPV6        = 19,
    CDS_ADS_DL_IPF_BID_LL_FE80       = 20,
    CDS_ADS_DL_IPF_BID_LL_FF         = 21,
    CDS_ADS_DL_IPF_BID_MIP_ADV       = 22,
    CDS_ADS_DL_IPF_BID_MIP_REG_REPLY = 23,
    CDS_ADS_DL_IPF_BID_ICMPV6_RA     = 24,
    CDS_ADS_DL_IPF_BID_INVALID       = 255
};
typedef VOS_UINT8 CDS_ADS_DlIpfBidUint8;

/*
 * 枚举说明: IPF上行过滤结果中BID内容定义(用于B5000及之后的平台)
 *           IPF Bearer ID:
 *               0~4      保留
 *               5~15     EPS Bearer ID
 *               16       上行DHCP
 *               17       上行DHCPv6
 *               18       上行ICMP
 *               19       上行ICMPv6
 *               20       上行Link Local Address FE80
 *               21       上行Link Local Address FF
 *               22~254   暂时保留，可扩展
 *               255      不匹配任何Filter
 */
enum CDS_ADS_UlIpfBid {
    CDS_ADS_UL_IPF_BID_RSV0     = 0,
    CDS_ADS_UL_IPF_BID_EPSBID5  = 5,
    CDS_ADS_UL_IPF_BID_EPSBID15 = 15,
    CDS_ADS_UL_IPF_BID_DHCPV4   = 16,
    CDS_ADS_UL_IPF_BID_DHCPV6   = 17,
    CDS_ADS_UL_IPF_BID_ICMPV4   = 18,
    CDS_ADS_UL_IPF_BID_ICMPV6   = 19,
    CDS_ADS_UL_IPF_BID_LL_FE80  = 20,
    CDS_ADS_UL_IPF_BID_LL_FF    = 21,
    CDS_ADS_UL_IPF_BID_INVALID  = 255
};
typedef VOS_UINT8 CDS_ADS_UlIpfBidUint8;

/*
 * 结构说明: ID_CDS_ADS_DATA_IND 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId;

    ModemIdUint16             modemId;
    VOS_UINT8                 rabId;
    CDS_ADS_IpPacketTypeUint8 ipPacketType;
    VOS_UINT16                len;
    VOS_UINT8                 rsv[2];
    VOS_UINT8                 data[4]; /* 修改数组大小时请同步知会修改TAF_DEFAULT_CONTENT_LEN宏 */
} CDS_ADS_DataInd;

/* 结构说明: ID_ADS_CDS_DHCPV6_IND 消息结构 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId; /* 消息ID */
    ModemIdUint16 modemId; /* modem ID */
    VOS_UINT16 pdLen; /* PD的长度, 单位: bit */
    VOS_UINT8 pdAddrData[16]; /* PD前缀地址 */
    VOS_UINT8 ifaceId;
    VOS_UINT8 pduSessionId;
    VOS_UINT8 rsv[2];
} ADS_CDS_Dhcpv6Ind;

/*
 * 结构说明: ID_ADS_CDS_ERR_IND 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId;

    ModemIdUint16 modemId;
    VOS_UINT8     rabId;
    VOS_UINT8     errRate;
    VOS_UINT8     rsv[4];
} ADS_CDS_ErrInd;

/*
 * 结构说明: ID_CDS_ADS_DATA_IND_V2 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId;

    VOS_UINT8             ifaceId;
    CDS_ADS_UlIpfBidUint8 bid;
    VOS_UINT16            len;
    VOS_UINT8             data[0]; /*lint !e43*/
} CDS_ADS_DataIndV2;

/*
 * 结构说明: ID_ADS_CDS_DATA_IND_V2 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId;

    VOS_UINT8             ifaceId;
    CDS_ADS_DlIpfBidUint8 bid;
    ModemIdUint16         modemId;
    VOS_UINT8             pduSessionId;
    VOS_UINT8             rsv[1];
    VOS_UINT16            len;
    VOS_UINT8             data[0]; /*lint !e43*/
} ADS_CDS_DataIndV2;

/*
 * 结构说明: ID_ADS_CDS_ERR_IND_V2 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    CDS_ADS_MsgIdUint32 msgId;

    VOS_UINT8     ifaceId;
    VOS_UINT8     rsv1[1];
    ModemIdUint16 modemId;
    VOS_UINT8     pduSessionId;
    VOS_UINT8     errRate; /* 错包率，扩展备用，目前不处理 */
    VOS_UINT8     rsv2[2];
} ADS_CDS_ErrIndV2;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __CDS_ADS_INTERFACE_H__ */
