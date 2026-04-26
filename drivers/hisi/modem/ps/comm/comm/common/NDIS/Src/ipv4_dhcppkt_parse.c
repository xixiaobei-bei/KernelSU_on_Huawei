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

#include "ipv4_dhcppkt_parse.h"
#include "vos.h"
#include "securec.h"
#include "ps_tag.h"
#include "ps_common_def.h"
#include "ndis_interface.h"
#include "ndis_om.h"
#include "ipv4_dhcp_server.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_IPV4DHCPPKTPARSE_C
/*lint +e767*/


/*
 * 2 Declare the Global Variable
 */

/*
 * Function Name: IPV4_DHCP_DhcpPktDhcpInfoCheck
 * Description: DHCP报文UDP信息检查
 * Input: VOS_UINT8  *pucIpPkt 指向UDP头开始的位置
 * History:
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_UINT32 IPV4_DHCP_DhcpPktDhcpInfoCheck(VOS_UINT8 *dhcp, VOS_UINT32 dhcpMemLen)
{
    IPV4_DhcpProtocl     *dhcpFixHdr  = VOS_NULL_PTR;
    IPV4_DhcpOptionItem  *optionStru  = VOS_NULL_PTR;
    VOS_UINT8            *optionStart = VOS_NULL_PTR;
    VOS_UINT8            *optionEnd   = VOS_NULL_PTR;

    /* DHCP固定头存在 */
    if (sizeof(IPV4_DhcpProtocl) > dhcpMemLen) {
        return PS_FALSE;
    }

    /* 地址长度非法 */
    dhcpFixHdr = (IPV4_DhcpProtocl *)((VOS_VOID *)dhcp);
    if (dhcpFixHdr->hardwareLength > ETH_MAC_ADDR_LEN) {
        return PS_FALSE;
    }

    /* OPTION判断 */
    optionStart = dhcp + sizeof(IPV4_DhcpProtocl);
    optionEnd   = optionStart + (dhcpMemLen - sizeof(IPV4_DhcpProtocl));
    while (optionStart < optionEnd) {
        optionStru = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionStart);
        if (optionStru->optionType == IPV4_DHCP_OPTION_PAD_OPTIOIN) {
            /* PAD Only 1 byte */
            optionStart++;
        } else if (optionStru->optionType == IPV4_DHCP_OPTION_END_OPTION) {
            break;
        } else {
            /* 判断OptionLen是否有效 */
            if ((optionStart + IP_DHCPV4_OPTION_ITEM_HDR_LEN) > optionEnd) {
                return PS_FALSE;
            }

            optionStart += optionStru->optionLen + IP_DHCPV4_OPTION_ITEM_HDR_LEN;
        }
    }

    if (optionStart > optionEnd) {
        return PS_FALSE;
    }

    return PS_TRUE;
}
STATIC VOS_VOID IPV4_DHCP_SetAnalyseByOptionType(const VOS_UINT8 *optionStart, const VOS_UINT8 *optionEnd,
                                                 IPV4_DhcpAnalyseResult *analyseRst)
{
    const IPV4_DhcpOptionItem  *optionStru  = VOS_NULL_PTR;
    VOS_INT32             rlt;
    while (optionStart < optionEnd) {
        optionStru = (IPV4_DhcpOptionItem *)((VOS_VOID *)optionStart);
        if (optionStru->optionType == IPV4_DHCP_OPTION_MSG_TYPE) {
            rlt = memcpy_s(&(analyseRst->msgType), IPV4_DHCP_OPTION_MSG_TYPE_LEN, optionStru->optionValue,
                           IPV4_DHCP_OPTION_MSG_TYPE_LEN);
            if (rlt != EOK) {
                optionStart += optionStru->optionLen + IP_DHCPV4_OPTION_ITEM_HDR_LEN;
                continue;
            }
        } else if (optionStru->optionType == IPV4_DHCP_OPTION_REQUEST_IP_ADDR) {
            rlt = memcpy_s(&(analyseRst->requestIPAddr), IPV4_ADDR_LEN, optionStru->optionValue,
                           IPV4_ADDR_LEN);
            if (rlt != EOK) {
                optionStart += optionStru->optionLen + IP_DHCPV4_OPTION_ITEM_HDR_LEN;
                continue;
            }
        } else if (optionStru->optionType == IPV4_DHCP_OPTION_SERVER_IP) {
            rlt = memcpy_s(&(analyseRst->serverIPAddr), IPV4_ADDR_LEN, optionStru->optionValue,
                           IPV4_ADDR_LEN);
            if (rlt != EOK) {
                optionStart += optionStru->optionLen + IP_DHCPV4_OPTION_ITEM_HDR_LEN;
                continue;
            }
        } else if (IPV4_DHCP_OPTION_PAD_OPTIOIN == optionStru->optionType) {
            /* PAD Only 1 byte */
            optionStart++;
            continue;
        } else if (optionStru->optionType == IPV4_DHCP_OPTION_END_OPTION) {
            break;
        } else {
        }

        optionStart += optionStru->optionLen + IP_DHCPV4_OPTION_ITEM_HDR_LEN;
    }
    return;
}

/*
 * Description: 解析DHCP报文
 */
STATIC VOS_VOID IPV4_DHCP_AnalyseDhcpPkt(VOS_UINT8 *dhcp, VOS_UINT32 dhcpLen,
                                         IPV4_DhcpAnalyseResult *analyseRst)
{
    IPV4_DhcpProtocl     *dhcpFixHdr  = VOS_NULL_PTR;
    VOS_UINT8            *optionStart = VOS_NULL_PTR;
    VOS_UINT8            *optionEnd   = VOS_NULL_PTR;
    VOS_INT32             rlt;
    if ((dhcp == VOS_NULL_PTR) || (analyseRst == VOS_NULL_PTR)) {
        IP_ERROR_LOG("IPV4_DHCP_AnalyseDhcpPkt : Input Para is NULL");
        return;
    }

    if (dhcpLen < IPV4_DHCP_OPTION_OFFSET) {
        IP_ERROR_LOG1("IPV4_DHCP_AnalyseDhcpPkt : Dhcp Packet is small ", dhcpLen);
        return;
    }

    /* 记录DHCP固定头中的信息 */
    dhcpFixHdr                          = (IPV4_DhcpProtocl *)((VOS_VOID *)dhcp);
    analyseRst->transactionID         = dhcpFixHdr->transactionID;
    analyseRst->clientIPAddr.ipAddr32bit = dhcpFixHdr->clientIPAddr.ipAddr32bit;
    analyseRst->yourIPAddr.ipAddr32bit   = dhcpFixHdr->yourIPAddr.ipAddr32bit;
    analyseRst->hardwareLen           = ETH_MAC_ADDR_LEN;
    rlt = memcpy_s(analyseRst->hardwareAddr, ETH_MAC_ADDR_LEN, dhcpFixHdr->clientHardwardAddr,
                   ETH_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* Option Start and End Address,not include Magic Code */
    optionStart = dhcp + IPV4_DHCP_OPTION_OFFSET;
    /* Remove End */
    optionEnd = optionStart + dhcpLen - IPV4_DHCP_OPTION_OFFSET;
    IPV4_DHCP_SetAnalyseByOptionType(optionStart, optionEnd, analyseRst);
    return;
}

/*
 * Function Name: IPV4_DHCP_DhcpPktUdpInfoCheck
 * Description: DHCP报文UDP信息检查
 * Input: VOS_UINT8  *pucIpPkt 指向UDP头开始的位置
 * History: VOS_VOID
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_UINT32 IPV4_DHCP_DhcpPktUdpInfoCheck(const VOS_UINT8 *udp, VOS_UINT32 udpMemLen)
{
    const ETH_Udphdr *udpHdr = VOS_NULL_PTR;
    VOS_UINT32  udpLen;

    /* 头长度判断 */
    if (udpMemLen < IP_UDP_HDR_LEN) {
        return PS_FALSE;
    }

    /* UDP长度合法性 */
    udpHdr = (ETH_Udphdr *)((VOS_VOID *)udp);
    udpLen  = IP_NTOHS(udpHdr->len);
    if (udpLen > udpMemLen) {
        return PS_FALSE;
    }

    /* 端口号67 */
    if (udpHdr->dstPort != UDP_DHCP_SERVICE_PORT) {
        return PS_FALSE;
    }

    return PS_TRUE;
}

/*
 * Function Name: IPV4_DHCP_DhcpPktIPInfoCheck
 * Description: DHCP报文IP层信息检查
 * Input: VOS_UINT8  *pucIpPkt 指向IP头开始的位置
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_UINT32 IPV4_DHCP_DhcpPktIPInfoCheck(const VOS_UINT8 *ipPkt, VOS_UINT32 ipMemLen)
{
    const ETH_Ipfixhdr *ipHdr = VOS_NULL_PTR;
    VOS_UINT32    ipHdrLen;
    VOS_UINT32    ipLen;

    /* IP头内存存在 */
    if (sizeof(ETH_Ipfixhdr) > ipMemLen) {
        return PS_FALSE;
    }

    ipHdr   = (ETH_Ipfixhdr *)((VOS_VOID *)ipPkt);
    ipHdrLen = ipHdr->ipHdrLen << 2; /* 2:len */
    ipLen    = IP_NTOHS(ipHdr->totalLen);

    /* 非IPv4数据包 */
    if (ipHdr->ipVer != IP_IPV4_VERSION) {
        return PS_FALSE;
    }

    /* 非UDP包 */
    if (ipHdr->protocol != IP_PROTOCOL_UDP) {
        return PS_FALSE;
    }

    /* IP头中长度合法性判断 */
    if ((ipLen > ipMemLen) || (ipHdrLen >= ipMemLen) || (ipHdrLen >= ipLen)) {
        return PS_FALSE;
    }

    /* 存在UDP头 */
    if ((ipHdrLen + IP_UDP_HDR_LEN) > ipMemLen) {
        return PS_FALSE;
    }

    return PS_TRUE;
}

/*
 * Function Name: IPV4_DHCP_IsDhcpPacket
 * Description: 判断是否是DHCP报文
 * Input: VOS_UINT8  *pucIpPkt 指向IP头开始的位置
 * History:
 *    1.      2011-3-7  Draft Enact
 */
STATIC VOS_UINT32 IPV4_DHCP_IsDhcpPacket(VOS_UINT8 *ipPkt, VOS_UINT32 pktMemLen)
{
    ETH_Ipfixhdr *ipHdr = VOS_NULL_PTR;
    VOS_UINT32    ipHdrLen;
    VOS_UINT8    *udp = VOS_NULL_PTR;
    VOS_UINT32    udpMemLen;
    VOS_UINT8    *dhcp = VOS_NULL_PTR;
    VOS_UINT32    dhcpMemLen;

    if (ipPkt == VOS_NULL_PTR) {
        return PS_FALSE;
    }

    /* IP层判断 */
    if (IPV4_DHCP_DhcpPktIPInfoCheck(ipPkt, pktMemLen) != PS_TRUE) {
        return PS_FALSE;
    }

    /* UDP判断 */
    ipHdr    = (ETH_Ipfixhdr *)((VOS_VOID *)ipPkt);
    ipHdrLen  = ipHdr->ipHdrLen << 2; /* 2:len */
    udp      = ipPkt + ipHdrLen;
    udpMemLen = pktMemLen - ipHdrLen;
    if (IPV4_DHCP_DhcpPktUdpInfoCheck(udp, udpMemLen) != PS_TRUE) {
        return PS_FALSE;
    }

    /* DHCP判断 */
    dhcp      = udp + IP_UDP_HDR_LEN;
    dhcpMemLen = udpMemLen - IP_UDP_HDR_LEN;
    if (IPV4_DHCP_DhcpPktDhcpInfoCheck(dhcp, dhcpMemLen) != PS_TRUE) {
        return PS_FALSE;
    }

    return PS_TRUE;
}

/*
 * Function Name: IPV4_DHCP_ProcDhcpPkt
 * Description: 处理DHCP报文
 * Input: VOS_UINT8  *pucIpPkt 指向IP头开始的位置
 * History:
 *    1.      2011-3-7  Draft Enact
 */
VOS_VOID IPV4_DHCP_ProcDhcpPkt(VOS_UINT8 *ipPkt, VOS_UINT8 rabId, VOS_UINT32 pktMemLen)
{
    IPV4_DhcpAnalyseResult dhcpRst;
    ETH_Ipfixhdr          *ipFixHdr = VOS_NULL_PTR;
    VOS_UINT32             dhcpOffset;
    VOS_UINT32             dhcpLen;
    if (ipPkt == VOS_NULL_PTR) {
        IP_ERROR_LOG("IPV4_DHCP_ProcDhcpPkt : Input Para Is NULL");
        return;
    }

    /* not dhcp packet */
    if (IPV4_DHCP_IsDhcpPacket(ipPkt, pktMemLen) != PS_TRUE) {
        return;
    }

    /* record src ip and dst ip in the ip hdr */
    (VOS_VOID)memset_s(&dhcpRst, sizeof(IPV4_DhcpAnalyseResult), 0, sizeof(IPV4_DhcpAnalyseResult));

    ipFixHdr                    = (ETH_Ipfixhdr *)((VOS_VOID *)ipPkt);
    dhcpRst.srcIPAddr.ipAddr32bit = ipFixHdr->srcAddr;
    dhcpRst.dstIpAddr.ipAddr32bit = ipFixHdr->destAddr;

    /* analyse the dhcp packet */
    dhcpOffset = (ipFixHdr->ipHdrLen << 2) + IP_UDP_HDR_LEN; /* 2:len */
    dhcpLen    = pktMemLen - dhcpOffset;

    if (dhcpLen >= ETH_MAX_FRAME_SIZE) {
        IP_ERROR_LOG1("IPV4_DHCP_ProcDhcpPkt : Dhcp Len Error,Len : ", dhcpLen);
        return;
    }

    IPV4_DHCP_AnalyseDhcpPkt(ipPkt + dhcpOffset, dhcpLen, &dhcpRst);

    /* proc analysis dhcp packet result */
    IPV4_DHCP_ProcDhcpMsg(&dhcpRst, rabId);

    IPV4_DHCP_STAT_RECV_DHCP_PKT(1);

    return;
}



