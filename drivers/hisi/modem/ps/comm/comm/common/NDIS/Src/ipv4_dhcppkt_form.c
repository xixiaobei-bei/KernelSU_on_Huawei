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
/*
 * 1 Include HeadFile
 */

#include "ipv4_dhcppkt_form.h"
#include "vos.h"
#include "securec.h"
#include "ps_tag.h"
#include "ps_common_def.h"
#include "ipv4_dhcp_server.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_IPV4DHCPPKTFORM_C
/*lint +e767*/


/*
 * 2 Declare the Global Variable
 */

const VOS_UINT8 g_magicCookie[] = { 0x63, 0x82, 0x53, 0x63 };
/*
 * Function Name: IPV4_DHCP_FormDhcpHdrStru
 * Description: form dhcp packet fix header
 * Input: VOS_UINT8 *pucDhcpData : 调用者保证长度大于DHCP固定头长度
 *    1.      2011-3-7  Draft Enact
 */
VOS_VOID IPV4_DHCP_FormDhcpHdrStru(const IPV4_DhcpAnalyseResult *dhcpAnalyse, VOS_UINT8 *dhcpData)
{
    IPV4_DhcpProtocl *dhcpHdr = VOS_NULL_PTR;
    errno_t           rlt;
    if ((dhcpData == VOS_NULL_PTR) || (dhcpAnalyse == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_FormDhcpHdrStru : Input Para NULL");
        return;
    }

    dhcpHdr = (IPV4_DhcpProtocl *)((VOS_VOID *)dhcpData);

    dhcpHdr->op                        = IPV4_DHCP_OPERATION_REPLY;
    dhcpHdr->hardwareType              = IPV4_DHCP_ETHERNET_HTYPE;
    dhcpHdr->hardwareLength            = ETH_MAC_ADDR_LEN;
    dhcpHdr->hops                      = 0;
    dhcpHdr->transactionID             = dhcpAnalyse->transactionID;
    dhcpHdr->seconds                   = 0;
    dhcpHdr->flags                     = 0;
    dhcpHdr->clientIPAddr.ipAddr32bit     = 0;
    dhcpHdr->relayIPAddr.ipAddr32bit      = 0;
    dhcpHdr->nextServerIPAddr.ipAddr32bit = 0;
    rlt = memcpy_s(dhcpHdr->clientHardwardAddr, ETH_MAC_ADDR_LEN, dhcpAnalyse->hardwareAddr,
                   ETH_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* Magic Cookie */
    rlt = memcpy_s(dhcpHdr->magicCookie, IPV4_DHCP_HEAD_COOKIE_LEN, g_magicCookie, sizeof(g_magicCookie));
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    return;
}
STATIC VOS_VOID IPV4_DHCP_FormOptionMsgType(VOS_UINT8 dhcpType, VOS_UINT8 *optionAddr, VOS_UINT16 *optAddrOffset)
{
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    optionItem                  = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType      = IPV4_DHCP_OPTION_MSG_TYPE;
    optionItem->optionLen       = 1;
    optionItem->optionValue[0]  = dhcpType;
    (*optAddrOffset) += 3; /* 3:addr */
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionSubNetMask(VOS_UINT8 *optionAddr, const NDIS_Ipv4Info *ipV4Entity,
                                                 VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_SUBNET_MASK;
    optionItem->optionLen  = IPV4_ADDR_LEN;
    rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->nmIpInfo), IPV4_ADDR_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_ADDR_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionRouter(VOS_UINT8 *optionAddr, const NDIS_Ipv4Info *ipV4Entity,
                                             VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_ROUTER_IP;
    optionItem->optionLen  = IPV4_ADDR_LEN;
    rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->gwIpInfo), IPV4_ADDR_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_ADDR_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionDNS(VOS_UINT8 *optionAddr, const NDIS_Ipv4Info *ipV4Entity, VOS_UINT8 *dnsLen,
                                          VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    *dnsLen                    = 0;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_DOMAIN_NAME_SERVER;
    if (ipV4Entity->primDnsAddr.ipAddr32bit != 0) {
        rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->primDnsAddr), IPV4_ADDR_LEN);
        if (rlt != EOK) {
            return IP_FAIL;
        }
        *dnsLen += IPV4_ADDR_LEN;
    }

    if (ipV4Entity->secDnsAddr.ipAddr32bit != 0) {
        rlt = memcpy_s(optionItem->optionValue + (*dnsLen), IPV4_ADDR_LEN, &(ipV4Entity->secDnsAddr),
                       IPV4_ADDR_LEN);
        if (rlt != EOK) {
            return IP_FAIL;
        }

        *dnsLen += IPV4_ADDR_LEN;
    }
    (*optAddrOffset) += *dnsLen;
    optionItem->optionLen = *dnsLen;
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionServer(VOS_UINT8 *optionAddr, const NDIS_Ipv4Info *ipV4Entity,
                                             VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_SERVER_IP;
    optionItem->optionLen  = IPV4_ADDR_LEN;
    rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->gwIpInfo), IPV4_ADDR_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_ADDR_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionLeaseTime(VOS_UINT8 *optionAddr, VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    VOS_UINT32                  timeLen;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_LEASE_TIME;
    optionItem->optionLen  = IPV4_DHCP_OPTION_LEASE_TIME_LEN;
    /* 转换为网络字节序 */
    timeLen = IPV4_DHCP_LEASE_TIME;
    timeLen = IP_HTONL(timeLen);
    rlt = memcpy_s(optionItem->optionValue, IPV4_DHCP_OPTION_LEASE_TIME_LEN, &timeLen,
                   IPV4_DHCP_OPTION_LEASE_TIME_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_DHCP_OPTION_LEASE_TIME_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionT1(VOS_UINT8 *optionAddr, VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    VOS_UINT32                  timeLen;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_T1;
    optionItem->optionLen  = IPV4_DHCP_OPTION_LEASE_TIME_LEN;
    timeLen                   = (VOS_UINT32)IPV4_DHCP_T1;
    timeLen                   = IP_HTONL(timeLen);
    rlt = memcpy_s(optionItem->optionValue, IPV4_DHCP_OPTION_LEASE_TIME_LEN, &timeLen,
                   IPV4_DHCP_OPTION_LEASE_TIME_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_DHCP_OPTION_LEASE_TIME_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionT2(VOS_UINT8 *optionAddr, VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    VOS_UINT32                  timeLen;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_T2;
    optionItem->optionLen  = IPV4_DHCP_OPTION_LEASE_TIME_LEN;
    timeLen                   = (VOS_UINT32)IPV4_DHCP_T2;
    timeLen                   = IP_HTONL(timeLen);
    rlt = memcpy_s(optionItem->optionValue, IPV4_DHCP_OPTION_LEASE_TIME_LEN, &timeLen,
                   IPV4_DHCP_OPTION_LEASE_TIME_LEN);
    if (rlt != EOK) {
        return IP_FAIL;
    }
    (*optAddrOffset) += IPV4_DHCP_OPTION_LEASE_TIME_LEN + 2; /* 2:addr */
    return IP_SUCC;
}
STATIC VOS_UINT32 IPV4_DHCP_FormOptionNetbios(VOS_UINT8 *optionAddr, VOS_UINT8 *winsLen,
                                              const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 *optAddrOffset)
{
    errno_t                     rlt;
    IPV4_DhcpOptionItem        *optionItem = VOS_NULL_PTR;
    *winsLen                   = 0;
    optionItem               = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionAddr);
    optionItem->optionType = IPV4_DHCP_OPTION_NETBIOS_NAME_SERVER;
    if (ipV4Entity->primWinsAddr.ipAddr32bit != 0) {
        rlt = memcpy_s(optionItem->optionValue, IPV4_ADDR_LEN, &(ipV4Entity->primWinsAddr), IPV4_ADDR_LEN);
        if (rlt != EOK) {
            return IP_FAIL;
        }

        *winsLen += IPV4_ADDR_LEN;
    }

    if (ipV4Entity->secWinsAddr.ipAddr32bit != 0) {
        rlt = memcpy_s(optionItem->optionValue + (*winsLen), IPV4_ADDR_LEN, &(ipV4Entity->secWinsAddr),
                       IPV4_ADDR_LEN);
        if (rlt != EOK) {
            return IP_FAIL;
        }

        *winsLen += IPV4_ADDR_LEN;
    }
    (*optAddrOffset) += *winsLen;
    optionItem->optionLen = *winsLen;
    return IP_SUCC;
}
/*
 * Description: 生成Option选项
 */
STATIC VOS_VOID IPV4_DHCP_FormOption(const NDIS_Ipv4Info *ipV4Entity, VOS_UINT8 *option, VOS_UINT8 dhcpType,
                                     VOS_UINT16 *optionLen)
{
    VOS_UINT8                   dnsLen;
    VOS_UINT8                  *optionAddr = VOS_NULL_PTR;
    VOS_UINT8                   winsLen;
    if ((ipV4Entity == VOS_NULL_PTR) || (option == VOS_NULL_PTR) || (optionLen == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_FormOption : Input Para Is NULL");
        return;
    }

    optionAddr = option;
    /* 报文类型 */
    IPV4_DHCP_FormOptionMsgType(dhcpType, optionAddr, optionLen);

    /* 子网掩码 */
    if (IPV4_DHCP_FormOptionSubNetMask(optionAddr + (*optionLen), ipV4Entity, optionLen) == IP_FAIL) {
        return;
    }

    /* 默认网关 */
    if (IPV4_DHCP_FormOptionRouter(optionAddr + (*optionLen), ipV4Entity, optionLen) == IP_FAIL) {
        return;
    }

    /* DNS */
    if (IPV4_DHCP_FormOptionDNS(optionAddr + (*optionLen), ipV4Entity, &dnsLen, optionLen) == IP_FAIL) {
        return;
    }
    if (dnsLen != 0) {
        *optionLen += 2; /* 2:len */
    }

    /* DHCP SERVER IP */
    if (IPV4_DHCP_FormOptionServer(optionAddr + (*optionLen), ipV4Entity, optionLen) == IP_FAIL) {
        return;
    }

    /* 租期 */
    if (IPV4_DHCP_FormOptionLeaseTime(optionAddr + (*optionLen), optionLen) == IP_FAIL) {
        return;
    }

    /* Renewal (T1) Time Value */
    if (IPV4_DHCP_FormOptionT1(optionAddr + (*optionLen), optionLen) == IP_FAIL) {
        return;
    }

    /* Renewal (T2) Time Value */
    if (IPV4_DHCP_FormOptionT2(optionAddr + (*optionLen), optionLen) == IP_FAIL) {
        return;
    }

    /* WINS : NetBios Name Server */
    if (IPV4_DHCP_FormOptionNetbios(optionAddr + (*optionLen), &winsLen, ipV4Entity, optionLen) == IP_FAIL) {
        return;
    }

    if (winsLen != 0) {
        *optionLen += 2; /* 2:len */
    }
    optionAddr += *optionLen;
    /*  End Option 0xff option结束标志 */
    *optionAddr = 0xFF;
    *optionLen += 1;

    return;
}

/*
 * Function Name: IPV4_DHCP_FormOfferMsg
 * Description: 生成DHCP OFFER消息，
 * Return: 成功返回PS_SUCC;失败返回PS_FAIL
 *    1.      2011-3-7  Draft Enact
 */
VOS_UINT32 IPV4_DHCP_FormOfferMsg(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                                  const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 *len)
{
    VOS_UINT8 *dhcpPktBuf    = VOS_NULL_PTR;
    VOS_UINT32 dhcpOffset;
    VOS_UINT16 dhcpOptionLen = 0;
    VOS_INT32  rlt;
    if ((dhcpAnalyse == VOS_NULL_PTR) || (ipV4Entity == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_FormOfferMsg : Input Para NULL");
        return PS_FAIL;
    }

    /* 将BUF 清 零 */
    (VOS_VOID)memset_s(IPV4_DHCP_GET_BUFFER(), ETH_MAX_FRAME_SIZE, 0, ETH_MAX_FRAME_SIZE);

    dhcpOffset  = ETH_MAC_HEADER_LEN + IPV4_FIX_HDR_LEN + IP_UDP_HDR_LEN;
    dhcpPktBuf = IPV4_DHCP_GET_BUFFER() + dhcpOffset;

    IPV4_DHCP_FormDhcpHdrStru(dhcpAnalyse, dhcpPktBuf);

    /* 设置your ip address */
    rlt = memcpy_s(dhcpPktBuf + IPV4_DHCP_HEAD_YIADDR_OFFSET, IPV4_ADDR_LEN, ipV4Entity->ueIpInfo.ipAddr8bit,
                   IPV4_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);

    /* dhcp option 设置 */
    dhcpPktBuf += IPV4_DHCP_OPTION_OFFSET;
    IPV4_DHCP_FormOption(ipV4Entity, dhcpPktBuf, IPV4_DHCP_MSG_OFFER, &dhcpOptionLen);

    /* 返回写入的长度 */
    *len = dhcpOptionLen + IPV4_DHCP_OPTION_OFFSET;

    return PS_SUCC;
}

/*
 * Function Name: IPV4_DHCP_FormAckMsg
 * Description: 生成DHCP ACK消息
 *    1.      2011-3-7  Draft Enact
 */
VOS_VOID IPV4_DHCP_FormAckMsg(const IPV4_DhcpAnalyseResult *dhcpAnalyse,
                              const NDIS_Ipv4Info *ipV4Entity, VOS_UINT16 *len)
{
    VOS_UINT8 *dhcpPktBuf    = VOS_NULL_PTR;
    VOS_UINT32 dhcpOffset;
    VOS_UINT16 dhcpOptionLen = 0;
    VOS_INT32  rlt;
    IP_NDIS_ASSERT(dhcpAnalyse != VOS_NULL_PTR);
    IP_NDIS_ASSERT(ipV4Entity != VOS_NULL_PTR);
    IP_NDIS_ASSERT(len != VOS_NULL_PTR);

    /* 将BUF 清 零 */
    (VOS_VOID)memset_s(IPV4_DHCP_GET_BUFFER(), ETH_MAX_FRAME_SIZE, 0, ETH_MAX_FRAME_SIZE);

    dhcpOffset  = ETH_MAC_HEADER_LEN + IPV4_FIX_HDR_LEN + IP_UDP_HDR_LEN;
    dhcpPktBuf = IPV4_DHCP_GET_BUFFER() + dhcpOffset;

    /* 设置DHCP报文头中固定部分 */
    IPV4_DHCP_FormDhcpHdrStru(dhcpAnalyse, dhcpPktBuf);

    /* 设置your ip address */
    rlt = memcpy_s(dhcpPktBuf + IPV4_DHCP_HEAD_YIADDR_OFFSET, IPV4_ADDR_LEN, ipV4Entity->ueIpInfo.ipAddr8bit,
                   IPV4_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* Form Option */
    dhcpPktBuf += IPV4_DHCP_OPTION_OFFSET;
    IPV4_DHCP_FormOption(ipV4Entity, dhcpPktBuf, IPV4_DHCP_MSG_ACK, &dhcpOptionLen);

    /* 返回写入的长度 */
    *len = dhcpOptionLen + IPV4_DHCP_OPTION_OFFSET;

    return;
}




