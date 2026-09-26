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

#ifndef __NDISOM_H__
#define __NDISOM_H__
/*
 * 1 其他头文件包含
 */
#include "vos.h"

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

#define NDIS_STAT_UL_DISCARD_USBPKT(n) (g_ndisStatStru.dicardUsbFrmNum += (n))
#define NDIS_STAT_UL_RECV_USBPKT_SUCC(n) (g_ndisStatStru.recvUsbPktSuccNum += (n))
#define NDIS_STAT_UL_SEND_ADSPKT(n) (g_ndisStatStru.sendPktToAdsSucNum += (n))
#define NDIS_STAT_DL_DISCARD_ADSPKT(n) (g_ndisStatStru.dicardAdsPktNum += (n))
#define NDIS_STAT_DL_RECV_ADSPKT_SUCC(n) (g_ndisStatStru.recvAdsPktSuccNum += (n))
#define NDIS_STAT_DL_GET_IPV6MAC_FAIL(n) (g_ndisStatStru.getIpv6MacFailNum += (n))
#define NDIS_STAT_DL_PKT_DIFF_RAB_NUM(n) (g_ndisStatStru.dlPktDiffRabNum += (n))
#define NDIS_STAT_DL_ADDMACFRM_FAIL(n) (g_ndisStatStru.addMacHdrFailNum += (n))
#define NDIS_STAT_DL_SEND_USBPKT_FAIL(n) (g_ndisStatStru.dlSendPktFailNum += (n))
#define NDIS_STAT_DL_SEND_USBPKT_SUCC(n) (g_ndisStatStru.dlSendPktSuccNum += (n))

#define NDIS_STAT_UL_RECV_DHCPPKT(n) (g_ndisStatStru.recvDhcpPktNum += (n))

#define NDIS_STAT_UL_RECV_ARP_REQUEST(n) (g_ndisStatStru.recvArpReq += (n))
#define NDIS_STAT_DL_RECV_ARP_REPLY(n) (g_ndisStatStru.recvArpReply += (n))
#define NDIS_STAT_PROC_ARP_FAIL(n) (g_ndisStatStru.procArpError += (n))
#define NDIS_STAT_DL_SEND_ARP_REQUEST_SUCC(n) (g_ndisStatStru.sendArpReqSucc += (n))
#define NDIS_STAT_DL_SEND_ARP_REQUEST_FAIL(n) (g_ndisStatStru.sendArpReqFail += (n))
#define NDIS_STAT_ARPREPLY_NOTRECV(n) (g_ndisStatStru.arpReplyNotRecv += (n))
#define NDIS_STAT_DL_SEND_ARP_REPLY(n) (g_ndisStatStru.sendArpReply += (n))
#define NDIS_STAT_DL_SEND_ARPDHCPPKT_FAIL(n) (g_ndisStatStru.sendArpDhcpNDFailNum += (n))
#define NDIS_STAT_RESET_CB_BEFORE(n) (g_ndisStatStru.resetCbBefore += (n))
#define NDIS_STAT_RESET_CB_AFTER(n) (g_ndisStatStru.resetCbAfter += (n))
#define NDIS_STAT_PROC_NOTUE_ARP(n) (g_ndisStatStru.procNotUeArp += (n))
#define NDIS_STAT_PROC_FREE_ARP(n) (g_ndisStatStru.procFreeArp += (n))
#define NDIS_STAT_MAC_INVALID_PKT_NUM(n) (g_ndisStatStru.macInvalidPktNum += (n))
#define NDIS_STAT_ENQUE_PKT_NUM(n) (g_ndisStatStru.enquePktNum += (n))
#define NDIS_STAT_SEND_QUE_PKT_NUM(n) (g_ndisStatStru.sendQuePktNum += (n))

/* define dhcp server stat macro */
#define IPV4_DHCP_STAT_RECV_DHCP_PKT(n) (g_dhcpStatStru.recvDhcpPkt += (n))
#define IPV4_DHCP_STAT_RECV_DISCOVER_MSG(n) (g_dhcpStatStru.recvDiscoverMsg += (n))
#define IPV4_DHCP_STAT_RECV_REQUEST_MSG(n) (g_dhcpStatStru.recvRequestMsg += (n))
#define IPV4_DHCP_STAT_RECV_SELETING_REQ_MSG(n) (g_dhcpStatStru.recvSelectingReqMsg += (n))
#define IPV4_DHCP_STAT_RECV_OTHER_REQ_MSG(n) (g_dhcpStatStru.recvOtherReqMsg += (n))
#define IPV4_DHCP_STAT_RECV_RELEASE_MSG(n) (g_dhcpStatStru.recvReleaseMsg += (n))
#define IPV4_DHCP_STAT_RECV_OTHER_TYPE_MSG(n) (g_dhcpStatStru.recvOtherTypeMsg += (n))
#define IPV4_DHCP_STAT_SEND_OFFER(n) (g_dhcpStatStru.sendOffer += (n))
#define IPV4_DHCP_STAT_SEND_ACK(n) (g_dhcpStatStru.sendAck += (n))
#define IPV4_DHCP_STAT_SEND_NACK(n) (g_dhcpStatStru.sendNack += (n))
#define IPV4_DHCP_STAT_PROC_ERR(n) (g_dhcpStatStru.procErr += (n))
#define IPV4_DHCP_STAT_SEND_PKT_SUCC(n) (g_dhcpStatStru.sendDhcpPktSucc += (n))
#define IPV4_DHCP_STAT_SEND_PKT_FAIL(n) (g_dhcpStatStru.sendDhcpPktFail += (n))

/*
 * 结构名: NDIS_StatInfo
 * 协议表格:
 * ASN.1描述:
 * 结构说明: NDIS统计量结构
 */
typedef struct {
    VOS_UINT32 dicardUsbFrmNum;    /* 上行丢弃的USB数据包个数 */
    VOS_UINT32 recvUsbPktSuccNum;  /* 上行成功接收到USB的包的个数 */
    VOS_UINT32 sendPktToAdsSucNum; /* 上行发送到ADS成功的个数 */
    VOS_UINT32 dicardAdsPktNum;    /* 下行被丢弃的ADS 数据包NUM */
    VOS_UINT32 recvAdsPktSuccNum;  /* 下行从ADS收到包成功个数 */
    VOS_UINT32 getIpv6MacFailNum;  /* 下行发包时获取IPV6 MAC地址失败个数 */
    VOS_UINT32 dlPktDiffRabNum;    /* 下行数据包类型和承载类型不一致个数 */
    VOS_UINT32 addMacHdrFailNum;   /* 添加MAC头失败的统计 */
    VOS_UINT32 dlSendPktFailNum;   /* 发送包失败的统计量 */
    VOS_UINT32 dlSendPktSuccNum;   /* 下行成功发送数据包至底软的统计量 */

    VOS_UINT32 recvDhcpPktNum; /* 接收到的DHCP包 */

    /* ARP 统计信息 */
    VOS_UINT32 recvArpReq;           /* 收到的ARP Request包 */
    VOS_UINT32 recvArpReply;         /* 收到的ARP Reply包 */
    VOS_UINT32 procArpError;         /* 处理ARP失败个数 */
    VOS_UINT32 procNotUeArp;         /* 不是对应UE/PC地址的ARP报文个数 */
    VOS_UINT32 procFreeArp;          /* freeARP报文个数 */
    VOS_UINT32 sendArpReqSucc;       /* 发送ARP Request成功包 */
    VOS_UINT32 sendArpReqFail;       /* 发送ARP Request失败数 */
    VOS_UINT32 arpReplyNotRecv;      /* ARP请求没有收到Rely的计数 */
    VOS_UINT32 sendArpReply;         /* 发送ARP REPLY个数 */
    VOS_UINT32 sendArpDhcpNDFailNum; /* 发送ARP或DHCP或ND包失败个数 */

    VOS_UINT32 resetCbBefore;         /* 单独复位BEFORE统计 */
    VOS_UINT32 resetCbAfter;          /* 单独复位AFTER统计 */
    VOS_UINT32 macInvalidPktNum;      /* 下行IP包发送时PC MAC地址无效的统计量 */
    VOS_UINT32 enquePktNum;           /* 下行成功缓存的IP包个数 */
    VOS_UINT32 sendQuePktNum;         /* 下行成功发送缓存的IP包个数 */
} NDIS_StatInfo;

/*
 * 结构名: IPV4_DhcpStatInfo
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV4 DHCP 统计信息
 */
typedef struct {
    VOS_UINT32 recvDhcpPkt;
    VOS_UINT32 recvDiscoverMsg;
    VOS_UINT32 recvRequestMsg;
    VOS_UINT32 recvSelectingReqMsg;
    VOS_UINT32 recvOtherReqMsg;
    VOS_UINT32 recvReleaseMsg;
    VOS_UINT32 recvOtherTypeMsg;
    VOS_UINT32 sendOffer;
    VOS_UINT32 sendAck;
    VOS_UINT32 sendNack;
    VOS_UINT32 procErr;
    VOS_UINT32 sendDhcpPktSucc;
    VOS_UINT32 sendDhcpPktFail;
} IPV4_DhcpStatInfo;

extern NDIS_StatInfo g_ndisStatStru;
extern IPV4_DhcpStatInfo g_dhcpStatStru;

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

#endif



