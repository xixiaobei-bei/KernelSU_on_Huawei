/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012-10-10
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
/*
 * 1 Include HeadFile
 */
#include "ipv4_dhcp_server.h"
#include "vos.h"
#include "mdrv.h"
#include "ps_tag.h"
#include "securec.h"
#include "ip_comm.h"
#include "ndis_interface.h"
#include "ndis_om.h"
#include "ndis_arp_proc.h"
#include "ndis_entity.h"
#include "ndis_dl_proc.h"
#include "ipv4_dhcppkt_form.h"


#define THIS_MODU ps_ndis
/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_IPV4DHCPSERVER_C
/*lint +e767*/

/* 获得IPV4实体 */
#define IPV4_DHCP_GET_NDIS_ENTITY(Index) (&g_ndisEntity[(Index)])


/* 定义组DHCP OFFER/ACK/NACK时的临时缓冲区 */
VOS_UINT8 g_dhcpBuff[ETH_MAX_FRAME_SIZE] = {0};
/* define IP Layer Identity */
VOS_UINT16 g_ipHdrIdentity = 0;
/* define DHCP Adddr Lease Time:60 * 60 * 24 * 3 */
VOS_UINT32 g_leaseTime = (60 * 60 * 24 * 3);
/* define DHCP Packet Magic Cookie; it's the IP Addr : 99.130.83.99 */

/* define MACRO to access global var */
#define IP_GET_IP_IDENTITY() (g_ipHdrIdentity++)

/*
 * 3 Function
 */
/*
 * Function Name: IP_CalcCRC16
 * Description: 计算CRC 16校验和
 * Input: VOS_UINT8 *pucData : crc src buf
 *                   VOS_UINT16 usLen: src buf len
 * Return: the crc16 result of Src buf
 *                   (PS: the result is network byte order)
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_UINT16 IP_CalcCRC16(const VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32  checkSum      = 0;
    VOS_UINT16 *dataBuffer    = VOS_NULL_PTR;

    IP_NDIS_ASSERT_RTN(data != VOS_NULL_PTR, 0);

    if (len == 0) {
        return 0;
    }

    dataBuffer = (VOS_UINT16 *)((VOS_VOID *)data);

    while (len > 1) {
        checkSum += *(dataBuffer++);
        len -= sizeof(VOS_UINT16);
    }

    if (len != 0) {
        checkSum += *(VOS_UINT8 *)dataBuffer;
    }

    checkSum = (checkSum >> 16) + (checkSum & 0xffff); /* 16:bit */
    checkSum += (checkSum >> 16); /* 16:bit */

    return (VOS_UINT16)(~checkSum);
}
STATIC VOS_VOID IPV4_DHCP_FillFakeHdr(const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 dhcpLen,
                                      VOS_UINT16 *curTotalLen, VOS_UINT32 dstIp)
{
    VOS_UINT16              udpCrcLen;
    VOS_UINT16              checkSum;
    IPV4_Udpipfakehdr      *udpIpFakeHdr = VOS_NULL_PTR;
    /* UDPIP Fake Hdr Offset, Same to IP Hdr Offset */
    udpIpFakeHdr = (IPV4_Udpipfakehdr *)(IPV4_DHCP_GET_BUFFER() + ETH_MAC_HEADER_LEN);
    /* Fill Fake Hdr */
    (VOS_VOID)memset_s(udpIpFakeHdr->rec0, sizeof(udpIpFakeHdr->rec0), 0, sizeof(udpIpFakeHdr->rec0));

    udpIpFakeHdr->srcIP.ipAddr32bit = ipV4Entity->gwIpInfo.ipAddr32bit;
    udpIpFakeHdr->dstIP.ipAddr32bit = dstIp;
    udpIpFakeHdr->protocol       = IP_PROTOCOL_UDP;
    udpIpFakeHdr->all0           = 0;
    udpIpFakeHdr->length         = IP_HTONS(dhcpLen + IP_UDP_HDR_LEN);
    /* Fill Udp Hdr */
    udpIpFakeHdr->srcPort  = UDP_DHCP_SERVICE_PORT;
    udpIpFakeHdr->dstPort  = UDP_DHCP_CLIENT_PORT;
    udpIpFakeHdr->checksum = 0;
    udpIpFakeHdr->len      = udpIpFakeHdr->length;

    (*curTotalLen) = dhcpLen + IP_UDP_HDR_LEN;

    /* Calc UDP CRC16 */
    udpCrcLen = (*curTotalLen) + IPV4_FIX_HDR_LEN;
    checkSum  = IP_CalcCRC16((VOS_UINT8 *)udpIpFakeHdr, udpCrcLen);
    if (checkSum == 0) {
        checkSum = 0xFFFF;
    }
    udpIpFakeHdr->checksum = checkSum;
}
STATIC VOS_VOID IPV4_DHCP_FillFixHdr(const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 curTotalLen, VOS_UINT32 dstIp)
{
    ETH_Ipfixhdr           *iPFixHdr     = VOS_NULL_PTR;
    /* IP */
    iPFixHdr = (ETH_Ipfixhdr *)(IPV4_DHCP_GET_BUFFER() + ETH_MAC_HEADER_LEN);
    /* Fill IP Fix Header */
    iPFixHdr->ipHdrLen       = IPV4_FIX_HDR_LEN >> 2; /* 2:len */
    iPFixHdr->ipVer          = IP_IPV4_VERSION;
    iPFixHdr->serviceType    = 0;
    iPFixHdr->totalLen       = IP_HTONS(curTotalLen + IPV4_FIX_HDR_LEN);
    iPFixHdr->identification = IP_GET_IP_IDENTITY();
    iPFixHdr->identification = IP_HTONS(iPFixHdr->identification);
    iPFixHdr->offset         = 0;
    iPFixHdr->ttl            = IPV4_HDR_TTL;
    iPFixHdr->protocol       = IP_PROTOCOL_UDP;
    iPFixHdr->checkSum       = 0;
    iPFixHdr->srcAddr        = ipV4Entity->gwIpInfo.ipAddr32bit;
    iPFixHdr->destAddr       = dstIp;

    /* IP HDR CRC16 */
    iPFixHdr->checkSum = IP_CalcCRC16((VOS_UINT8 *)iPFixHdr, IPV4_FIX_HDR_LEN);
}
/*
 * Description: 将DHCP包封装成Ethnet帧发送到Ethnet上
 */
STATIC VOS_VOID IPV4_DHCP_SendDhcpToEth(const IPV4_DhcpAnalyseResult *dhcpAnalyse, const NDIS_Ipv4Info *ipV4Entity,
                                        VOS_UINT16 dhcpLen, VOS_UINT8 rabId)
{
    VOS_UINT16              curTotalLen;
    VOS_UINT32              dstIp = 0xffffffff;
    errno_t                 rlt;
    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_SendDhcpToEth : Input Para Error");
        return;
    }

    if (dhcpLen <= IPV4_DHCP_OPTION_OFFSET) {
        IP_ERROR_LOG1("IPV4_DHCP_SendDhcpToEth : Input Len too Small ", dhcpLen);
        return;
    }

    /* unicast */
    if (dhcpAnalyse->castFlg == IP_CAST_TYPE_UNICAST) {
        dstIp = dhcpAnalyse->srcIPAddr.ipAddr32bit;
    }

    IPV4_DHCP_FillFakeHdr(ipV4Entity, dhcpLen, &curTotalLen, dstIp);
    IPV4_DHCP_FillFixHdr(ipV4Entity, curTotalLen, dstIp);

    curTotalLen = curTotalLen + IPV4_FIX_HDR_LEN;

    /* MAC */
    rlt = memcpy_s(IPV4_DHCP_GET_BUFFER(), ETH_MAC_HEADER_LEN, ipV4Entity->macFrmHdr, ETH_MAC_HEADER_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    /* Update Dst Mac Addr */
    if (dhcpAnalyse->castFlg == IP_CAST_TYPE_UNICAST) {
        rlt = memcpy_s(IPV4_DHCP_GET_BUFFER(), ETH_MAC_ADDR_LEN, ipV4Entity->ueMacAddr, ETH_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    } else {
        rlt = memcpy_s(IPV4_DHCP_GET_BUFFER(), ETH_MAC_ADDR_LEN, g_broadCastAddr, ETH_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    }

    curTotalLen += ETH_MAC_HEADER_LEN;

    if (Ndis_SendMacFrm(IPV4_DHCP_GET_BUFFER(), curTotalLen, rabId) != PS_SUCC) {
        IP_ERROR_LOG("IPV4_DHCP_SendDhcpToEth:Send Dhcp Packet Error");
        IPV4_DHCP_STAT_SEND_PKT_FAIL(1);
    } else {
        IPV4_DHCP_STAT_SEND_PKT_SUCC(1);
    }

    return;
}


/*
 * Function Name: IPV4_DHCP_FindIPV4Entity
 * Description: 根据DHCP Client MAC Addr查找相应的IPV4实体;
 *                   由调用者保证入参ExRabId对应一个NDIS实体
 * History:
 *    1.      2011-3-7  Draft Enact
 *    2.      2011-12-9
 *    3.      2013-1-16  DSDA
 */
STATIC NDIS_Ipv4Info *IPV4_DHCP_FindIPV4Entity(VOS_UINT8 exRabId)
{
    NDIS_Entity    *ndisEntity = VOS_NULL_PTR;
    NDIS_Ipv4Info  *ipv4Info   = VOS_NULL_PTR;

    /* 由调用者保证入参ExRabId对应一个NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }
    ipv4Info = &(ndisEntity->ipV4Info);

    return ipv4Info;
}

/*
 * Function Name: IPV4_DHCP_DiscoverMsgProc
 * Description: 处理DHCP DISCOVER消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_DiscoverMsgProc(IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 rabId)
{
    NDIS_Ipv4Info *ipV4Entity = VOS_NULL_PTR;
    VOS_UINT16     dhcpLength;

    if (dhcpAnalyse == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_DiscoverMsgProc : Input Para pstDhcpAnalyse is NULL");
        return;
    }

    /* find the dhcp sever entity by ue mac */
    ipV4Entity = IPV4_DHCP_FindIPV4Entity(rabId);
    if (ipV4Entity == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_DiscoverMsgProc : Not Available IPV4 Entity");
        IPV4_DHCP_STAT_PROC_ERR(1);
        return;
    }

    dhcpAnalyse->castFlg = IP_CAST_TYPE_BRODCAST;

    /* form dhcp offer msg */
    dhcpLength = 0;
    if (IPV4_DHCP_FormOfferMsg(dhcpAnalyse, ipV4Entity, &dhcpLength) != PS_SUCC) {
        IP_ERROR_LOG("IPV4_DHCP_DiscoverMsgProc : Form Offer Msg Fail");
        IPV4_DHCP_STAT_PROC_ERR(1);
        return;
    }

    /* send dhcp pkt to ethnet */
    IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpLength, rabId);
    IPV4_DHCP_STAT_SEND_OFFER(1);

    return;
}
/*
 * Function Name: IPV4_DHCP_SelectingRequestMsgProc
 * Description: 处理Selecting状态下的请求消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_SelectingRequestMsgProc(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                                                  NDIS_Ipv4Info *ipV4Entity, VOS_UINT8 rabId)
{
    VOS_UINT16 dhcpAckLen = 0;

    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_SelectingRequestMsgProc : Input Para is NULL");
        return;
    }

    /* 判断当前消息 */
    if ((dhcpAnalyse->serverIPAddr.ipAddr32bit == ipV4Entity->gwIpInfo.ipAddr32bit) &&
        (dhcpAnalyse->requestIPAddr.ipAddr32bit == ipV4Entity->ueIpInfo.ipAddr32bit)) {
        /* 生成DHCP ACK并发送 */
        IPV4_DHCP_FormAckMsg(dhcpAnalyse, ipV4Entity, &dhcpAckLen);
        IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpAckLen, rabId);

        IPV4_DHCP_STAT_SEND_ACK(1);

        /* 设置IP地址为已分配 */
        ipV4Entity->ipAssignStatus = IPV4_DHCP_ADDR_STATUS_ASSIGNED;
    } else {
        /* 未选择本 DHCP Server,设置IP地址未分配 */
        ipV4Entity->ipAssignStatus = IPV4_DHCP_ADDR_STATUS_FREE;
        IP_ERROR_LOG("IPV4_DHCP_SelectingRequestMsgProc : The Client Has not Select Local IP");
    }

    return;
}


/*
 * Function Name: IPV4_DHCP_FormNakMsg
 * Description: 生成DHCP NACK消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_FormNackMsg(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                                      const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 *len)
{
    VOS_UINT8                  *dhcpPktBuf = VOS_NULL_PTR;
    VOS_UINT32                  dhcpOffset;
    VOS_UINT16                  dhcpOptionLen;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    VOS_INT32                   rlt;
    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_FormNackMsg : Input Para NULL");
        return;
    }

    /* 将BUF 清 零 */
    (VOS_VOID)memset_s(IPV4_DHCP_GET_BUFFER(), ETH_MAX_FRAME_SIZE, 0, ETH_MAX_FRAME_SIZE);

    dhcpOffset  = ETH_MAC_HEADER_LEN + IPV4_FIX_HDR_LEN + IP_UDP_HDR_LEN;
    dhcpPktBuf = IPV4_DHCP_GET_BUFFER() + dhcpOffset;

    /* Fill Dhcp Fix Hdr,and Keep your ip address to 0 */
    IPV4_DHCP_FormDhcpHdrStru(dhcpAnalyse, dhcpPktBuf);

    /* Form NACK Option */
    dhcpOptionLen = 0;
    dhcpPktBuf += IPV4_DHCP_OPTION_OFFSET;

    /* DHCP Msg Type */
    optionItem                    = (IPV4_DhcpOptionItem *)(dhcpPktBuf);
    optionItem->optionType      = IPV4_DHCP_OPTION_MSG_TYPE;
    optionItem->optionLen       = 1;
    optionItem->optionValue[0] = IPV4_DHCP_MSG_NAK;
    dhcpOptionLen += 3; /* 3:len */
    dhcpPktBuf += 3; /* 3:buf */

    /* DHCP SERVER IP */
    optionItem               = (IPV4_DhcpOptionItem *)dhcpPktBuf;
    optionItem->optionType = IPV4_DHCP_OPTION_SERVER_IP;
    optionItem->optionLen  = IPV4_ADDR_LEN;
    rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->gwIpInfo), IPV4_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    dhcpOptionLen += IPV4_ADDR_LEN + 2; /* 2:len */
    dhcpPktBuf += IPV4_ADDR_LEN + 2; /* 2:buf */

    /*  End Option 0xff option结束标志 */
    *dhcpPktBuf = 0xFF;
    dhcpOptionLen += 1;

    /* 返回写入的长度 */
    *len = dhcpOptionLen + IPV4_DHCP_OPTION_OFFSET;

    return;
}

/*
 * Function Name: IPV4_DHCP_OtherRequestMsgProc
 * Description: 处理非Selecting状态下的请求消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_OtherRequestMsgProc(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                                              const NDIS_Ipv4Info *ipV4Entity, VOS_UINT8 rabId)
{
    VOS_UINT16 dhcpAckLen = 0;

    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_OtherRequestMsgProc : Input Para is NULL");
        return;
    }

    /* 若请求IP同UE IP相同，则回ACK;否则回复NACK */
    if (dhcpAnalyse->clientIPAddr.ipAddr32bit == ipV4Entity->ueIpInfo.ipAddr32bit) {
        IPV4_DHCP_FormAckMsg(dhcpAnalyse, ipV4Entity, &dhcpAckLen);
        IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpAckLen, rabId);
        IPV4_DHCP_STAT_SEND_ACK(1);
    } else {
        IPV4_DHCP_FormNackMsg(dhcpAnalyse, ipV4Entity, &dhcpAckLen);
        IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpAckLen, rabId);
        /* NACK ERROR LOG */
        IP_ERROR_LOG("IPV4_DHCP_OtherRequestMsgProc : The Client Renew/Rebinding Request Fail");
        IPV4_DHCP_STAT_SEND_NACK(1);
    }

    return;
}

/*
 * Function Name: IPV4_DHCP_InitRebootRequestProc
 * Description: 处理Client 在Init-Reboot状态下的请求消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_InitRebootRequestProc(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                                                NDIS_Ipv4Info *ipV4Entity, VOS_UINT8 rabId)
{
    VOS_UINT16 dhcpAckLen = 0;

    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_InitRebootRequestProc : Input Para is NULL");
        return;
    }

    /* 若请求IP同UE IP相同，则回ACK;否则回复NACK */
    if (dhcpAnalyse->requestIPAddr.ipAddr32bit == ipV4Entity->ueIpInfo.ipAddr32bit) {
        IPV4_DHCP_FormAckMsg(dhcpAnalyse, ipV4Entity, &dhcpAckLen);
        IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpAckLen, rabId);
        ipV4Entity->ipAssignStatus = IPV4_DHCP_ADDR_STATUS_ASSIGNED;
        IPV4_DHCP_STAT_SEND_ACK(1);
    } else {
        IPV4_DHCP_FormNackMsg(dhcpAnalyse, ipV4Entity, &dhcpAckLen);
        IPV4_DHCP_SendDhcpToEth(dhcpAnalyse, ipV4Entity, dhcpAckLen, rabId);
        /* NACK ERROR LOG */
        IP_ERROR_LOG("IPV4_DHCP_InitRebootRequestProc : The Init-Reboot Client Request IP Fail");
        IPV4_DHCP_STAT_SEND_NACK(1);
    }

    return;
}

/*
 * Function Name: IPV4_DHCP_RequestMsgProc
 * Description: 处理DHCP REQUEST消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_RequestMsgProc(IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 rabId)
{
    NDIS_Ipv4Info *ipV4Entity = VOS_NULL_PTR;

    if (dhcpAnalyse == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_RequestMsgProc : Input Para NULL");
        return;
    }

    ipV4Entity = IPV4_DHCP_FindIPV4Entity(rabId);
    if (ipV4Entity == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_RequestMsgProc : Not Available IPV4 Entity");
        IPV4_DHCP_STAT_PROC_ERR(1);
        return;
    }

    /*
     * Client messages from different states
     * |              |INIT-REBOOT  |SELECTING    |RENEWING     |REBINDING |
     * |broad/unicast |broadcast    |broadcast    |unicast      |broadcast |
     * |server-ip     |MUST NOT     |MUST         |MUST NOT     |MUST NOT  |
     * |requested-ip  |MUST         |MUST         |MUST NOT     |MUST NOT  |
     * |ciaddr        |zero         |zero         |IP address   |IP address|
     */
    if (dhcpAnalyse->serverIPAddr.ipAddr32bit != 0) {
        /* Selecting Request */
        dhcpAnalyse->castFlg = IP_CAST_TYPE_BRODCAST;
        IPV4_DHCP_SelectingRequestMsgProc(dhcpAnalyse, ipV4Entity, rabId);
    } else if (dhcpAnalyse->clientIPAddr.ipAddr32bit == 0) {
        /* INIT-REBOOT Request */
        dhcpAnalyse->castFlg = IP_CAST_TYPE_BRODCAST;
        IPV4_DHCP_InitRebootRequestProc(dhcpAnalyse, ipV4Entity, rabId);
    } else {
        /* renew or rebinding */
        dhcpAnalyse->castFlg = IP_CAST_TYPE_UNICAST;
        IPV4_DHCP_OtherRequestMsgProc(dhcpAnalyse, ipV4Entity, rabId);
    }

    return;
}

/*
 * Function Name: IPV4_DHCP_ReleaseMsgProc
 * Description: 处理DHCP RELEASE消息
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_VOID IPV4_DHCP_ReleaseMsgProc(const IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 rabId)
{
    NDIS_Ipv4Info *ipV4Entity = VOS_NULL_PTR;

    if (dhcpAnalyse == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_ReleaseMsgProc : Input Para pstDhcpAnalyse is NULL");
        return;
    }

    ipV4Entity = IPV4_DHCP_FindIPV4Entity(rabId);
    if (ipV4Entity == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_DiscoverMsgProc : Not Available IPV4 Entity");
        IPV4_DHCP_STAT_PROC_ERR(1);
        return;
    }

    /* 修改分配状态 : 未分配 */
    ipV4Entity->ipAssignStatus = IPV4_DHCP_ADDR_STATUS_FREE;

    return;
}

/*
 * Function Name: IPV4_DHCP_ProcDhcpMsg
 * Description: 处理DHCP数据报
 *    1.      2011-3-7  Draft Enact
 */
VOS_VOID IPV4_DHCP_ProcDhcpMsg(IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 rabId)
{
    NDIS_Ipv4Info *ipV4Entity = VOS_NULL_PTR;
    VOS_UINT32     addrIndex;
    if (dhcpAnalyse == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_ProcDhcpMsg : Input Para pstDhcpAnalyse is NULL");
        return;
    }

    /* find the dhcp sever entity by ue mac */
    ipV4Entity = IPV4_DHCP_FindIPV4Entity(rabId);
    if (ipV4Entity == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_ProcDhcpMsg : Not Available IPV4 Entity");
        IPV4_DHCP_STAT_PROC_ERR(1);
        return;
    }

    /* save the ue mac addr,and update the mac hdr */
    if (Ndis_UpdateMacAddr(dhcpAnalyse->hardwareAddr, ETH_MAC_ADDR_LEN, ipV4Entity) == PS_FAIL) {
        IP_ERROR_LOG("IPV4_DHCP_ProcDhcpMsg : Ndis_UpdateMacAddr FAIL");
        return;
    }
    addrIndex = NDIS_GetEntityIndex(rabId);
    /* 发送下行IP缓存队列中的数据包 */
    NDIS_SendDlPkt(addrIndex);
    switch (dhcpAnalyse->msgType) {
        case IPV4_DHCP_MSG_DISCOVER:
            IPV4_DHCP_DiscoverMsgProc(dhcpAnalyse, rabId);
            IPV4_DHCP_STAT_RECV_DISCOVER_MSG(1);
            break;

        case IPV4_DHCP_MSG_REQUEST:
            IPV4_DHCP_RequestMsgProc(dhcpAnalyse, rabId);
            IPV4_DHCP_STAT_RECV_REQUEST_MSG(1);
            break;

        case IPV4_DHCP_MSG_RELEASE:
            IPV4_DHCP_ReleaseMsgProc(dhcpAnalyse, rabId);
            IPV4_DHCP_STAT_RECV_RELEASE_MSG(1);
            break;

        case IPV4_DHCP_MSG_DECLINE:
            /* 调用DECLINE处理函数 */
            IP_ERROR_LOG("IP_DHCPV4SERVER_ProcRabmDataInd:DECLINE Msg is received!");
            IPV4_DHCP_STAT_RECV_OTHER_TYPE_MSG(1);
            break;

        case IPV4_DHCP_MSG_INFORM:
            /* 调用INFORM处理函数 */
            IP_ERROR_LOG("IP_DHCPV4SERVER_ProcRabmDataInd:INFORM Msg is received!");
            IPV4_DHCP_STAT_RECV_OTHER_TYPE_MSG(1);
            break;

        default:
            IP_ERROR_LOG("IP_DHCPV4SERVER_ProcRabmDataInd:Illegal DHCPV4 Msg!");
            IPV4_DHCP_STAT_RECV_OTHER_TYPE_MSG(1);
            break;
    }

    return;
}

