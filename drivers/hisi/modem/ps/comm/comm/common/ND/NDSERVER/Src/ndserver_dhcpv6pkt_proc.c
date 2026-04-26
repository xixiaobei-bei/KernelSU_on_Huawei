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

#include "ndserver_dhcpv6pkt_proc.h"
#include "securec.h"
#include "ndis_entity.h"
#include "ip_comm.h"
#include "ip_ipm_global.h"
#include "ndis_interface.h"
#include "ip_nd_server.h"
#include "ndserver_pkt_comm.h"
#include "ndserver_om.h"
#include "ndserver_entity.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERDHCPV6PKTPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */
STATIC VOS_UINT32 IP_GetDnsOptLen(const ND_IP_Ipv6DnsSer *dnsSer, const IP_Ipv6PktDhcpOptHdr *dhcpRequestOpt,
                                  VOS_UINT16 *dnsOptLen, VOS_UINT16 *dhcpRequestDnsOptLen,
                                  VOS_UINT16 *replyUdpDataLen)
{
    VOS_UINT32                     dhcpReqOptCodeLoopCnt;

    for (dhcpReqOptCodeLoopCnt = 0; dhcpReqOptCodeLoopCnt < (dhcpRequestOpt->dhcpOptLen);
         dhcpReqOptCodeLoopCnt += 2) { /* 2:len */
        if (VOS_NTOHS(*((VOS_UINT16 *)(&(dhcpRequestOpt->dhcpOptData[dhcpReqOptCodeLoopCnt]))))
            == IP_IPV6_DHCP6_OPT_DNS_SERVERS) {
            (*dnsOptLen) = dnsSer->dnsSerNum;
            if (((*dnsOptLen) == 0) || ((*dnsOptLen) > IP_IPV6_MAX_DNS_NUM)) {
                IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_GetDnsOptLen, DNS Option is err!");
                return PS_FAIL;
            }

            (*dhcpRequestDnsOptLen) = (VOS_UINT16)((*dnsOptLen) * sizeof(NDIS_Ipv6Addr));
            (*replyUdpDataLen) += (*dhcpRequestDnsOptLen + 4); /* 4表示2个字节的option-code+2字节的option-len */
            break;
        }
    }
    return PS_SUCC;
}
STATIC VOS_VOID IP_GetSrcAddr(ND_MacAddrInfo *srcMacAddr, ND_Ipv6AddrInfo *srcIPAddr)
{
    errno_t rlt;
    rlt = memcpy_s(srcMacAddr->macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    IP_ProduceIfaceIdFromMacAddr(srcIPAddr->ipv6Addr, srcMacAddr->macAddr);
    IP_SetUint16Data(srcIPAddr->ipv6Addr, IP_IPV6_LINK_LOCAL_PREFIX);
    return;
}
STATIC VOS_VOID IP_GetDstAddr(ND_Ipv6AddrInfo *dstIPAddr, ND_MacAddrInfo *dstMacAddr, VOS_UINT32 addrIndex,
                              const VOS_UINT8 *dhcpInfoReqData)
{
    IP_NdserverTeAddrInfo         *teInfo                      = IP_NULL_PTR;
    errno_t                        rlt;
    rlt = memcpy_s(dstIPAddr->ipv6Addr, IP_IPV6_ADDR_LEN, dhcpInfoReqData + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    /* 得到目的MAC地址 */
    teInfo = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);
    rlt = memcpy_s(dstMacAddr->macAddr, IP_MAC_ADDR_LEN, teInfo->teLinkLayerAddr, IP_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    return;
}
/*lint -e778*/
/*lint -e572*/
STATIC VOS_UINT32 IP_FillDhcpServerIdOption(VOS_UINT8 *dhcpReplyDhcpHdrOffset, ND_MacAddrInfo srcMacAddr)
{
    IP_Ipv6PktDhcpDuidLLOpt        dhcpDuidLLOpt;
    errno_t                        rlt;
    (VOS_VOID)memset_s(&dhcpDuidLLOpt, sizeof(IP_Ipv6PktDhcpDuidLLOpt), 0,
                       sizeof(IP_Ipv6PktDhcpDuidLLOpt));
    dhcpDuidLLOpt.dhcpDuidType   = VOS_HTONS(IP_IPV6_DHCP_DUID_LL_OPT_TYPE);
    dhcpDuidLLOpt.dhcpDuidHWType = VOS_HTONS(IP_IPV6_HW_TYPE);
    rlt = memcpy_s(dhcpDuidLLOpt.linkLayerAddr, IP_IPV6_DHCP_OPT_LINKADDR_SIZE, srcMacAddr.macAddr,
                   IP_IPV6_DHCP_OPT_LINKADDR_SIZE);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);
    (*(VOS_UINT16 *)dhcpReplyDhcpHdrOffset) = VOS_HTONS(IP_IPV6_DHCP_OPT_SERVER_ID);
    dhcpReplyDhcpHdrOffset += sizeof(VOS_UINT16);
    (*(VOS_UINT16 *)dhcpReplyDhcpHdrOffset) = VOS_HTONS(IP_IPV6_DHCP_DUID_LL_OPT_LEN);

    dhcpReplyDhcpHdrOffset += sizeof(VOS_UINT16);
    rlt = memcpy_s(dhcpReplyDhcpHdrOffset, IP_IPV6_DHCP_DUID_LL_OPT_LEN, (VOS_UINT8 *)(&dhcpDuidLLOpt),
                   IP_IPV6_DHCP_DUID_LL_OPT_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);
    return PS_SUCC;
}
/*lint +e778*/
/*lint +e572*/
STATIC VOS_UINT32 IP_FillDhcpClientIdOption(const IP_Ipv6PktDhcpOptHdr *dhcpClientIdOpt,
                                            VOS_UINT8 *dhcpReplyDhcpHdrOffset, VOS_UINT32 *tmpDestLen)
{
    errno_t                        rlt;

    /* 4表示2个字节的option-code+2字节的option-len */
    if (((VOS_UINT32)(VOS_NTOHS(dhcpClientIdOpt->dhcpOptLen) + 4)) > (*tmpDestLen)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_FillDhcpClientIdOption, DHCP CLient ID option is err!");
        return PS_FAIL;
    }
    rlt = memcpy_s(dhcpReplyDhcpHdrOffset, (*tmpDestLen), (VOS_UINT8 *)dhcpClientIdOpt,
                   (VOS_NTOHS(dhcpClientIdOpt->dhcpOptLen) + 4)); /* 4:len */
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);
    *tmpDestLen -= TTF_MIN(*tmpDestLen, (VOS_UINT32)(VOS_NTOHS(dhcpClientIdOpt->dhcpOptLen) + 4)); /* 4:len */
    return PS_SUCC;
}
STATIC VOS_UINT32 IP_FillDhcpDnsOption(VOS_UINT8 *dhcpReplyDhcpHdrOffset, const ND_IP_Ipv6DnsSer *dnsSer,
                                       VOS_UINT16 dnsOptLen, VOS_UINT32 tmpDestLen, VOS_UINT16 dhcpRequestDnsOptLen)
{
    IP_Ipv6DhcpDnsOption           ipv6DhcpDnsOpt;
    errno_t                        rlt;
    if (dhcpRequestDnsOptLen != 0) {
        (VOS_VOID)memset_s(&ipv6DhcpDnsOpt, sizeof(IP_Ipv6DhcpDnsOption), 0, sizeof(IP_Ipv6DhcpDnsOption));
        /*lint -e778*/
        /*lint -e572*/
        ipv6DhcpDnsOpt.optionCode = VOS_HTONS(IP_IPV6_DHCP6_OPT_DNS_SERVERS);
        /*lint +e778*/
        /*lint +e572*/
        rlt = memcpy_s(&ipv6DhcpDnsOpt.ipv6DNS[0], (sizeof(NDIS_Ipv6Addr)), dnsSer->priDnsServer,
                       (sizeof(NDIS_Ipv6Addr)));
        IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);

        if (dnsOptLen == IP_IPV6_MAX_DNS_NUM) {
            rlt = memcpy_s(&ipv6DhcpDnsOpt.ipv6DNS[1], (sizeof(NDIS_Ipv6Addr)), dnsSer->secDnsServer,
                           (sizeof(NDIS_Ipv6Addr)));
            IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);
        }
        ipv6DhcpDnsOpt.optionLen = VOS_HTONS((VOS_UINT16)(IP_IPV6_ADDR_LEN * dnsOptLen));
        rlt = memcpy_s(dhcpReplyDhcpHdrOffset, tmpDestLen, (VOS_UINT8 *)&ipv6DhcpDnsOpt,
                       (4 + (dnsOptLen * sizeof(NDIS_Ipv6Addr)))); /* 4:len */
        IP_CHK_SEC_RETURN_VAL(rlt != EOK, PS_FAIL);
    }
    return PS_SUCC;
}
VOS_VOID IP_NDSERVER_FillIpUdpInfo(NDIS_Ipv6Hdr *dhcpReplyIpv6Hdr, IP_UdpHeadInfo *dhcpReplyUdpHdr,
                                   ND_Ipv6AddrInfo srcIPAddr, ND_Ipv6AddrInfo dstIPAddr, VOS_UINT16 replyUdpDataLen)
{
    /* 填充 IP 头 */
    IP_ND_FormIPv6HeaderMsg(srcIPAddr, dstIPAddr, replyUdpDataLen, (VOS_UINT8 *)(dhcpReplyIpv6Hdr),
                            IP_HEAD_PROTOCOL_UDP);

    /* 填充 UDP 头 */
    (VOS_VOID)TTF_NDIS_InputUDPHead((VOS_UINT8 *)(dhcpReplyUdpHdr), IP_IPV6_DHCP6_UE_PORT, IP_IPV6_DHCP6_PC_PORT,
                                    replyUdpDataLen);
}
VOS_UINT32 IP_FillIdInfo(ND_MacAddrInfo srcMacAddr, VOS_UINT8 **dhcpReplyDhcpHdrOffset, VOS_UINT32 *tmpDestLen,
                         IP_Ipv6PktDhcpOptHdr *dhcpClientIdOpt, VOS_UINT8 *dhcpInfoReqData)
{
    /* 填充 DHCP reply 头 */
    *(*dhcpReplyDhcpHdrOffset) = IP_IPV6_DHCP6_TYPE_REPLY;
    *dhcpReplyDhcpHdrOffset += 1;
    if (memcpy_s(*dhcpReplyDhcpHdrOffset, IP_IPV6_DHCP6_TRANS_ID_LEN,
                 ((dhcpInfoReqData + IP_IPV6_HEAD_LEN) + IP_UDP_HEAD_LEN) + 1, IP_IPV6_DHCP6_TRANS_ID_LEN) != EOK) {
        return PS_FAIL;
    }

    *dhcpReplyDhcpHdrOffset += IP_IPV6_DHCP6_TRANS_ID_LEN;

    /* 填充 DHCP ServerID option (DUID 使用第三种方式即 DUID-LL) */
    if (IP_FillDhcpServerIdOption(*dhcpReplyDhcpHdrOffset, srcMacAddr) == PS_FAIL) {
        return PS_FAIL;
    }

    *dhcpReplyDhcpHdrOffset += sizeof(VOS_UINT16) + sizeof(VOS_UINT16) + IP_IPV6_DHCP_DUID_LL_OPT_LEN;

    *tmpDestLen = IP_IPM_MTU - IP_ETHERNET_HEAD_LEN - sizeof(NDIS_Ipv6Hdr) - sizeof(IP_UdpHeadInfo) -
                 IP_IPV6_DHCP6_REPLY_HDR_LEN - IP_IPV6_DHCP_DUID_LL_OPT_LEN;

    /* 填充 DHCP CLient ID option */
    if (dhcpClientIdOpt != VOS_NULL_PTR) {
        if (IP_FillDhcpClientIdOption(dhcpClientIdOpt, *dhcpReplyDhcpHdrOffset, tmpDestLen) == PS_FAIL) {
            return PS_FAIL;
        }
        *dhcpReplyDhcpHdrOffset += (VOS_NTOHS(dhcpClientIdOpt->dhcpOptLen) + 4); /* 4:lenoffset */
    }
    return PS_SUCC;
}
VOS_VOID IP_NDSERVER_GetOption(VOS_UINT16 reqIp6PktLen, VOS_UINT8 *dhcpInfoReqData,
                               IP_Ipv6PktDhcpOptHdr **dhcpClientIdOpt,
                               IP_Ipv6PktDhcpOptHdr **dhcpRequestOpt,
                               VOS_UINT16 *replyUdpDataLen)
{
    VOS_UINT16                     reqDhcpOptLen;
    IP_Ipv6PktDhcpOptHdr          *reqDhcpOptHdr               = VOS_NULL_PTR;
    VOS_UINT16                     dhcpClientIdOptLen;
    reqDhcpOptLen   = reqIp6PktLen - ((IP_IPV6_HEAD_LEN + IP_UDP_HEAD_LEN) + IP_UDP_DHCP_HDR_SIZE);
    reqDhcpOptHdr  = (IP_Ipv6PktDhcpOptHdr *)(VOS_VOID *)(dhcpInfoReqData + IP_IPV6_HEAD_LEN +
                                                          IP_UDP_HEAD_LEN + IP_UDP_DHCP_HDR_SIZE);
    /* 获取 DHCP Client ID Option */
    (VOS_VOID)TTF_NDIS_Ipv6GetDhcpOption(reqDhcpOptHdr, reqDhcpOptLen, IP_IPV6_DHCP_OPT_CLIEND_ID,
                                         dhcpClientIdOpt, 0);
    if (*dhcpClientIdOpt != VOS_NULL_PTR) {
        dhcpClientIdOptLen = VOS_NTOHS((*dhcpClientIdOpt)->dhcpOptLen);
        *replyUdpDataLen += (dhcpClientIdOptLen + 4); /* 4表示2个字节的option-code+2字节的option-len */
    }

    /* 获取 DHCP Request Option */
    (VOS_VOID)TTF_NDIS_Ipv6GetDhcpOption(reqDhcpOptHdr, reqDhcpOptLen, IP_IPV6_DHCP_OPT_REQUEST,
                                         dhcpRequestOpt, 0);
}
VOS_VOID IP_NDSERVER_GetFillReplyInfoAddr(VOS_UINT8 **sendBuff, NDIS_Ipv6Hdr **dhcpReplyIpv6Hdr,
                                          IP_UdpHeadInfo **dhcpReplyUdpHdr, VOS_UINT8 **dhcpReplyDhcpHdrOffset)
{
    *sendBuff = IP_NDSERVER_GET_SENDMSG_BUFFER();
    (VOS_VOID)memset_s(*sendBuff, IP_IPM_MTU, IP_NULL, IP_IPM_MTU);
    /* 指向IPV6首部 */
    *sendBuff += IP_ETHERNET_HEAD_LEN;
    *dhcpReplyIpv6Hdr     = (NDIS_Ipv6Hdr *)(*sendBuff);
    *dhcpReplyUdpHdr      = (IP_UdpHeadInfo *)((*dhcpReplyIpv6Hdr) + 1);
    *dhcpReplyDhcpHdrOffset = (VOS_UINT8 *)((*dhcpReplyUdpHdr) + 1);
}
/*
 * Description: 组装并发送DHCPV6报文
 */
/*lint -e778*/
/*lint -e572*/
STATIC VOS_UINT32 IP_NDSERVER_SendDhcp6Reply(VOS_UINT32 addrIndex, const ND_IP_Ipv6DnsSer *dnsSer,
                                             VOS_UINT8 *dhcpInfoReqData, VOS_UINT16 reqIp6PktLen)
{
    NDIS_Ipv6Hdr                  *dhcpReplyIpv6Hdr            = VOS_NULL_PTR;
    IP_UdpHeadInfo                *dhcpReplyUdpHdr             = VOS_NULL_PTR;
    IP_Ipv6PktDhcpOptHdr          *dhcpRequestOpt              = VOS_NULL_PTR;
    IP_Ipv6PktDhcpOptHdr          *dhcpClientIdOpt             = VOS_NULL_PTR;
    VOS_UINT32                     tmpDestLen;
    VOS_UINT8                     *dhcpReplyDhcpHdrOffset      = VOS_NULL_PTR; /* 移动指针 */
    VOS_UINT16                     replyUdpDataLen;
    VOS_UINT16                     dhcpReplyPktLen;
    VOS_UINT16                     dnsOptLen                   = 0;
    VOS_UINT16                     dhcpRequestDnsOptLen        = 0;
    ND_Ipv6AddrInfo                srcIPAddr;
    ND_MacAddrInfo                 srcMacAddr;
    ND_Ipv6AddrInfo                dstIPAddr;
    ND_MacAddrInfo                 dstMacAddr;
    VOS_UINT8                     *sendBuff                    = VOS_NULL_PTR;

    IP_NDSERVER_InitAddrInfo(&srcIPAddr, &srcMacAddr, &dstIPAddr, &dstMacAddr);
    replyUdpDataLen = IP_UDP_HEAD_LEN + IP_UDP_DHCP_HDR_SIZE + IP_IPV6_DHCP_OPT_CLIENT_ID_LEN;
    IP_NDSERVER_GetOption(reqIp6PktLen, dhcpInfoReqData, &dhcpClientIdOpt, &dhcpRequestOpt, &replyUdpDataLen);
    if (dhcpRequestOpt != VOS_NULL_PTR) {
        if (IP_GetDnsOptLen(dnsSer, dhcpRequestOpt, &dnsOptLen, &dhcpRequestDnsOptLen, &replyUdpDataLen) == PS_FAIL) {
            return PS_FAIL;
        }
    }
    if (replyUdpDataLen <= (IP_UDP_HEAD_LEN + IP_UDP_DHCP_HDR_SIZE + IP_IPV6_DHCP_OPT_CLIENT_ID_LEN)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_SendDhcp6Reply, No content need to reply!");
        return PS_SUCC;
    }

    /* 根据自身MAC地址生成link-local地址 */
    IP_GetSrcAddr(&srcMacAddr, &srcIPAddr);
    /* 得到目的IPV6地址 */
    IP_GetDstAddr(&dstIPAddr, &dstMacAddr, addrIndex, dhcpInfoReqData);

    IP_NDSERVER_GetFillReplyInfoAddr(&sendBuff, &dhcpReplyIpv6Hdr, &dhcpReplyUdpHdr, &dhcpReplyDhcpHdrOffset);

    IP_NDSERVER_FillIpUdpInfo(dhcpReplyIpv6Hdr, dhcpReplyUdpHdr, srcIPAddr, dstIPAddr, replyUdpDataLen);

    if (IP_FillIdInfo(srcMacAddr, &dhcpReplyDhcpHdrOffset, &tmpDestLen, dhcpClientIdOpt, dhcpInfoReqData) != PS_SUCC) {
        return PS_FAIL;
    }

    /* 填充 DHCP DNS OPTION */
    if (IP_FillDhcpDnsOption(dhcpReplyDhcpHdrOffset, dnsSer, dnsOptLen, tmpDestLen, dhcpRequestDnsOptLen) == PS_FAIL) {
        return PS_FAIL;
    }
    dhcpReplyUdpHdr->udpCheckSum = TTF_NDIS_Ipv6_CalcCheckSum((VOS_UINT8 *)dhcpReplyUdpHdr, replyUdpDataLen,
        &dhcpReplyIpv6Hdr->srcAddr, &dhcpReplyIpv6Hdr->dstAddr, IP_HEAD_PROTOCOL_UDP);
    /* 指向以太网首部 */
    sendBuff -= IP_ETHERNET_HEAD_LEN;
    IP_NDSERVER_FormEtherHeaderMsg(srcMacAddr, dstMacAddr, sendBuff);

    IP_NDSERVER_AddTransDhcpv6PktNum(addrIndex);
    IP_NDSERVER_AddTransPktTotalNum(addrIndex);

    /* 限制reply消息长度 */
    dhcpReplyPktLen = TTF_MIN(IP_IPV6_HEAD_LEN + replyUdpDataLen + IP_ETHERNET_HEAD_LEN, IP_IPM_MTU);

    /* 将DHCPV6 REPLY消息发送到PC */
    return NdSer_SendPkt(sendBuff, dhcpReplyPktLen, IP_NDSERVER_ADDRINFO_GET_EPSID(addrIndex));
}

/*lint +e778*/
/*lint +e572*/
STATIC VOS_UINT32 NdSer_DhcpJugePortMsgTypeValid(const VOS_UINT8 *data, VOS_UINT32 addrIndex)
{
    VOS_UINT16                  srcPort;
    VOS_UINT16                  dstPort;
    VOS_UINT8                   msgType;
    const VOS_UINT8                  *udpData    = VOS_NULL_PTR;

    udpData = data + IP_IPV6_HEAD_LEN;
    srcPort  = *(VOS_UINT16 *)(VOS_VOID *)udpData;
    dstPort  = *(VOS_UINT16 *)(VOS_VOID *)(udpData + 2); /* 2:从srcport偏移两个字节获取dstport */

    if ((VOS_NTOHS(srcPort) != IP_IPV6_DHCP6_PC_PORT) || (VOS_NTOHS(dstPort) != IP_IPV6_DHCP6_UE_PORT)) {
        IP_NDSERVER_AddErrDhcpv6PktNum(addrIndex);
        IPND_WARNING_LOG2(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, usSrcPort, usDstPort is err!", srcPort, dstPort);
        return PS_FAIL;
    }

    msgType = *(udpData + IP_UDP_HEAD_LEN);
    if (msgType != IP_IPV6_DHCP6_INFOR_REQ) {
        IP_NDSERVER_AddErrDhcpv6PktNum(addrIndex);
        IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, ucMsgType is err!", msgType);
        return PS_FAIL;
    }
    return PS_SUCC;
}
VOS_UINT32 NdSer_DhcpV6GetMsgInfo(const MsgBlock *msgBlock, VOS_UINT8 **data, VOS_UINT32 *dataLen, VOS_UINT8 *exRabId)
{
    ADS_NDIS_DataInd       *rcvMsg = (ADS_NDIS_DataInd *)(VOS_VOID *)msgBlock;

    *data = IMM_ZcGetDataPtr(rcvMsg->data);
    if (*data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NdSer_DhcpV6PktProc, IMM_ZcGetDataPtr fail!");
        return PS_FAIL;
    }
    *dataLen = IMM_ZcGetUsedLen(rcvMsg->data);
    *exRabId = NDIS_FORM_EXBID(rcvMsg->modemId, rcvMsg->rabId);
    return PS_SUCC;
}
/*
 * Description: 处理DHCPV6报文
 */
VOS_VOID NdSer_DhcpV6PktProc(const MsgBlock *msgBlock)
{
    VOS_UINT32                  addrIndex;
    VOS_UINT8                  *data       = VOS_NULL_PTR;
    VOS_UINT32                  dataLen;
    IP_NdserverAddrInfo        *infoAddr   = IP_NULL_PTR;
    ND_IP_Ipv6DnsSer           *dnsSer     = VOS_NULL_PTR;
    VOS_UINT8                   exRabId;

    if (NdSer_DhcpV6GetMsgInfo(msgBlock, &data, &dataLen, &exRabId) != PS_SUCC) {
        return;
    }
#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    if (ND_CheckEpsIdValid(exRabId) != IP_SUCC) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, ucRabId is Invalid!", exRabId);
        return;
    }
#endif

    addrIndex = IP_NDSERVER_GET_ADDR_INFO_INDEX(exRabId);
    if (addrIndex >= IP_NDSERVER_ADDRINFO_MAX_NUM) {
        IPND_ERROR_LOG2(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc: Invalid Input:", exRabId, addrIndex);
        return;
    }

    IP_NDSERVER_AddRcvDHCPV6PktNum(addrIndex);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);

    if (NdSer_DhcpJugePortMsgTypeValid(data, addrIndex) == PS_FAIL) {
        return;
    }

    dnsSer = &(infoAddr->ipv6NwPara.dnsSer);

    /* Nd Server没有DNS情况下收到Dhcpv6 information request,丢弃该报文 */
    if (dnsSer->dnsSerNum == 0) {
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, No DNS information exists");
        return;
    }
    if (dataLen <= ((IP_IPV6_HEAD_LEN + IP_UDP_HEAD_LEN) + IP_UDP_DHCP_HDR_SIZE)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, No Option Len!");
        return;
    }
    if (IP_NDSERVER_SendDhcp6Reply(addrIndex, dnsSer, data, (VOS_UINT16)dataLen) == PS_FAIL) {
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "NdSer_DhcpV6PktProc, send dhcp6 reply fail");
    }

    return;
}




