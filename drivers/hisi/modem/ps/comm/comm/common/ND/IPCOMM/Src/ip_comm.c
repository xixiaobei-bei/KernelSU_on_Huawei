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

#include "ip_comm.h"
#include "securec.h"
#include "mdrv.h"
#if (FEATURE_ON == FEATURE_LTE)
#include "msp_diag.h"
#endif
#include "ps_tag.h"
#include "ps_iface_global_def.h"
#include "ip_ipm_global.h"


#define THIS_MODU ps_nd
#define THIS_FILE_ID PS_FILE_ID_IPCOMM_C /* 待修改 */

/*
 * 2 Declare the Global Variable
 */

VOS_UINT8 g_ndAllNodesMulticaseAddr[IP_IPV6_ADDR_LEN] = {
    0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
VOS_UINT8 g_ndAllRoutersMulticaseAddr[IP_IPV6_ADDR_LEN] = {
    0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};

VOS_UINT8 g_ndAllNodesMacAddr[IP_MAC_ADDR_LEN]   = { 0x33, 0x33, 0x00, 0x00, 0x00, 0x01 };
VOS_UINT8 g_ndAllRoutersMacAddr[IP_MAC_ADDR_LEN] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0x02 };


/*
 * 功能描述: 构造link-local地址
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 *  2.日    期: 2013年1月10日
 *    修改内容: DSDA输入参数修改。EpsId为高2bit为ModemID，低6bit为EpsbId。
 */
VOS_UINT32 ND_CheckEpsIdValid(VOS_UINT8 exEpsId)
{
    VOS_UINT8  epsId;
    VOS_UINT16 modemId;

    modemId = ND_GET_MODEMID_FROM_EXEPSID(exEpsId);
    if (modemId >= MODEM_ID_BUTT) {
        return PS_FAIL;
    }

    epsId = ND_GET_EPSID_FROM_EXEPSID(exEpsId);
    if ((epsId < ND_MIN_EPSB_ID) || (epsId > ND_MAX_EPSB_ID)) {
        return IP_FAIL;
    }

    return IP_SUCC;
}

/*
 * 功能描述: 构造link-local地址
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
VOS_VOID IP_BuildLinkLocalAddr(VOS_UINT8 *linkLocalAddr, const VOS_UINT8 *ifId)
{
    errno_t rlt;
    /* 构造link-local address */
    IP_SetUint16Data(linkLocalAddr, IP_IPV6_LINK_LOCAL_PREFIX);
    rlt = memcpy_s(&linkLocalAddr[ND_IPV6_IF_OFFSET], ND_IPV6_IF_LEN, ifId, ND_IPV6_IF_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
}

/*
 * 功能描述: 为ICMPv6消息构造IP header，不支持扩展头
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
VOS_VOID IP_BuildIPv6Header(VOS_UINT8 *ipv6, IP_Ipv6PseduoHeader *pseduoHeader,
                            const VOS_UINT8 *interfaceId, VOS_UINT32 upperLength)
{
    VOS_UINT8  linkLocalAddr[IP_IPV6_ADDR_LEN] = {0};
    VOS_UINT8 *linkLocalAddrCalc               = &linkLocalAddr[0];
    errno_t    rlt;
    /* 构造link-local address */
    IP_SetUint16Data(linkLocalAddrCalc, IP_IPV6_LINK_LOCAL_PREFIX);
    rlt = memcpy_s(&linkLocalAddr[ND_IPV6_IF_OFFSET], ND_IPV6_IF_LEN, interfaceId, ND_IPV6_IF_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* 设置IP版本号 */
    *ipv6 = IP_VERSION_6 << 4; /* 4:version */

    /* 设置Payload，没有扩展头 */
    IP_SetUint16Data(ipv6 + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET, upperLength);

    /* 设置next header */
    *(ipv6 + IP_IPV6_BASIC_HEAD_NEXT_HEAD_OFFSET) = IP_HEAD_PROTOCOL_ICMPV6;

    /* Hop Limit 255 */
    *(ipv6 + IP_IPV6_BASIC_HOP_LIMIT_OFFSET) = IP_IPV6_ND_HOP_LIMIT;

    rlt = memcpy_s(ipv6 + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN, linkLocalAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    rlt = memcpy_s(ipv6 + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN, g_ndAllRoutersMulticaseAddr,
                   IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* 为计算checksum构造pseudo-header */
    rlt = memcpy_s(pseduoHeader->ipSrc, IP_IPV6_ADDR_LEN, linkLocalAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    rlt = memcpy_s(pseduoHeader->ipDes, IP_IPV6_ADDR_LEN, g_ndAllRoutersMulticaseAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    IP_SetUint32Data((VOS_UINT8 *)&pseduoHeader->upperLength, upperLength);
    pseduoHeader->nextHead = IP_HEAD_PROTOCOL_ICMPV6;
}

/*
 * Function Name: IP_ND_FormIPv6HeaderMsg
 * Description: IPv6报头设置
 * Input: aucSrcIPAddr --- 源IP地址
 *                   aucDstIPAddr --- 目的IP地址
 *                   ulPduLen     --- 有效负载长度
 * Output: pucData --- 报文缓冲指针
 *    1.      2011-04-07  Draft Enact
 */
VOS_VOID IP_ND_FormIPv6HeaderMsg(const ND_Ipv6AddrInfo srcIPAddr, const ND_Ipv6AddrInfo dstIPAddr,
                                 VOS_UINT32 pduLen, VOS_UINT8 *data, VOS_UINT8 type)
{
    errno_t rlt;
    /* 版本 */
    *data = (VOS_UINT8)((IP_VERSION_6 << IP_MOVEMENT_4_BITS) & IP_IPDATA_HIGH_4_BIT_MASK);
    data++;

    /* 3:传输类型、流标签 */
    data += 3;

    /* 有效负载长度 */
    IP_SetUint16Data(data, (VOS_UINT16)pduLen);
    data += 2; /* 2:len */

    /* 下一报头 */
    *data = type;
    data++;

    /* 跳限制 */
    *data = IP_IPV6_ND_HOP_LIMIT;
    data++;

    /* 源地址 */
    rlt = memcpy_s(data, IP_IPV6_ADDR_LEN, srcIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    data += IP_IPV6_ADDR_LEN;

    /* 目的地址 */
    rlt = memcpy_s(data, IP_IPV6_ADDR_LEN, dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
}
/*
 * 功能描述: RA合法性检查
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
IP_BoolUint8 IP_IsValidRAPacket(const VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset)
{
    VOS_UINT32 tmp;

    IP_ASSERT_RTN(ipMsg != VOS_NULL_PTR, IP_FALSE);

    /* link-local source address */
    IP_GetUint16Data(tmp, ipMsg + IP_IPV6_SRC_ADDR_OFFSET);
    if ((tmp & IP_IPV6_LINK_LOCAL_MASK) != IP_IPV6_LINK_LOCAL_PREFIX) {
        IPND_ERROR_LOG1(ND_TASK_PID, "IP_IsValidRAPacket: Illegal source address:!", tmp);
        return IP_FALSE;
    }

    /* Hop Limit 255 */
    tmp = *(ipMsg + IP_IPV6_BASIC_HOP_LIMIT_OFFSET);
    if (tmp != IP_IPV6_ND_HOP_LIMIT) {
        IPND_ERROR_LOG1(ND_TASK_PID, "IP_IsValidRAPacket: Illegal Hop Limit:!", tmp);
        return IP_FALSE;
    }

    /* CODE 0 */
    tmp = *(ipMsg + typeOffset + IP_ICMPV6_CODE_OFFSET);
    if (tmp != IP_IPV6_ND_VALID_CODE) {
        IPND_ERROR_LOG1(ND_TASK_PID, "IP_IsValidRAPacket: Illegal ICMPv6 CODE:!", tmp);
        return IP_FALSE;
    }

    return IP_TRUE;
}

/*  ipv6 begin */
/*
 * Function Name: IP_GetIpv6UpLayerProtocol
 * Description: 获取IPv6上层协议
 * Input: pucIpMsg---------------------IPv6数据包首地址
 *                   usPayLoad--------------------IPv6数据包payload长度
 * Output: pucNextHeader----------------下一头类型指针
 *                   pulDecodedLen----------------已译码长度指针
 * Return: IP_ErrUint2
 *    1.      2011-03-25  Draft Enact
 */
STATIC IP_ErrUint2 IP_GetIpv6UpLayerProtocol(const VOS_UINT8 *ipMsg, VOS_UINT16 payLoad, VOS_UINT8 *nextHeader,
                                             VOS_UINT32 *decodedLen)
{
    const VOS_UINT8 *ipMsgTemp      = ipMsg;
    VOS_UINT8        nextHeaderCalc;
    VOS_UINT8        extHeaderLen;
    VOS_UINT32       decodeLen      = IP_NULL;

    IP_ASSERT_RTN(ipMsg != VOS_NULL_PTR, IP_FAIL);
    IP_ASSERT_RTN(nextHeader != VOS_NULL_PTR, IP_FAIL);
    IP_ASSERT_RTN(decodedLen != VOS_NULL_PTR, IP_FAIL);

    /* 获取nextheader */
    nextHeaderCalc = *(ipMsgTemp + IP_IPV6_BASIC_HEAD_NEXT_HEAD_OFFSET);

    ipMsgTemp = ipMsgTemp + IP_IPV6_HEAD_LEN;

    /* 已跳过的数据包长度小于ulIpMsgLen */
    while (decodeLen <= payLoad) {
        switch (nextHeaderCalc) {
            case IP_EXTENSION_HEADER_TYPE_FRAGMENT /* 分段头 */:
            case IP_EXTENSION_HEADER_TYPE_AH /* AH头 */:
            case IP_EXTENSION_HEADER_TYPE_ESP /* ESP头 */:
                *nextHeader = nextHeaderCalc;
                *decodedLen = decodeLen + IP_IPV6_HEAD_LEN;
                return IP_SUCC;
            case IP_EXTENSION_HEADER_TYPE_HOPBYHOP /* 逐跳选项头 */:
            case IP_EXTENSION_HEADER_TYPE_DESTINATION /* 目的地选项头 */:
            case IP_EXTENSION_HEADER_TYPE_ROUTING /* 选路头 */:
                if (decodeLen + IP_IPV6_EXT_HEAD_LEN_OFFSET >= payLoad) {
                    IPND_WARNING_LOG(ND_TASK_PID, "IP_GetIpv6UpLayerProtocol: PayLoad Lenth is Error!");
                    return IP_FAIL;
                }

                /* 当前扩展头中的下一跳 */
                nextHeaderCalc = *(ipMsgTemp + IP_IPV6_EXT_HEAD_NEXT_HEAD_OFFSET);

                /* 读取扩展头中的长度信息，跳过此扩展头 */
                extHeaderLen = *(ipMsgTemp + IP_IPV6_EXT_HEAD_LEN_OFFSET);

                ipMsgTemp = ipMsgTemp + IP_GetExtensionLen(extHeaderLen);
                decodeLen  = decodeLen + IP_GetExtensionLen(extHeaderLen);
                if (payLoad < decodeLen) {
                    IPND_WARNING_LOG(ND_TASK_PID, "IP_GetIpv6UpLayerProtocol: Lenth Error!");
                    return IP_FAIL;
                }

                break;
            default:
                *nextHeader = nextHeaderCalc;
                *decodedLen = decodeLen + IP_IPV6_HEAD_LEN;
                return IP_SUCC;
        }
    }

    return IP_FAIL;
}

/*
 * Function Name: IP_AffirmNdParam
 * Description: 确认ND报文参数的有效性
 * Input: pucIpMsg ----------------  报文数据区指针
 *                   ulTypeOffset ------------  ICMPv6报头偏移量
 * Return: IP_ErrUint2
 *    1.      2011-04-12  Draft Enact
 */
STATIC IP_ErrUint2 IP_AffirmNdParam(const VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset)
{
    VOS_UINT32 tmp;

    IP_ASSERT_RTN(ipMsg != VOS_NULL_PTR, PS_FALSE);

    /* Hop Limit 255 */
    tmp = *(ipMsg + IP_IPV6_BASIC_HOP_LIMIT_OFFSET);

    if (tmp != IP_IPV6_ND_HOP_LIMIT) {
        IPND_ERROR_LOG1(ND_TASK_PID, "IP_AffirmNdParam: Illegal Hop Limit:!", tmp);
        return IP_FAIL;
    }

    /* CODE 0 */
    tmp = *(ipMsg + typeOffset + IP_ICMPV6_CODE_OFFSET);
    if (tmp != IP_IPV6_ND_VALID_CODE) {
        IPND_INFO_LOG1(ND_TASK_PID, "IP_AffirmNdParam: Illegal ICMPv6 CODE:!", tmp);
        return IP_FAIL;
    }

    return IP_SUCC;
}

/*
 * Function Name: IP_IsIcmpv6Packet
 * Description: 判定是否为ICMPv6包
 * Input: pucIpMsg---------------------IP数据包首地址
 *                   ulIpMsgLen-------------------IP数据包长度
 * Output: pulDecodedLen----------------已译码长度指针
 * Return: IP_BoolUint8
 * History:
 *    1.      2011-03-25  Draft Enact
 */
VOS_UINT32 g_ipCommcnNd = 0;

STATIC IP_BoolUint8 IP_IsIcmpv6Packet(const VOS_UINT8 *ipMsg, VOS_UINT32 ipMsgLen, VOS_UINT32 *decodedLen)
{
    VOS_UINT8          protocol       = IP_NULL;
    VOS_UINT8          ipVersion;
    IP_ErrUint2        rslt;
    VOS_UINT32         decodedLenCalc = IP_NULL;
    VOS_UINT16         payLoad        = IP_NULL;

    IP_ASSERT_RTN(ipMsg != VOS_NULL_PTR, IP_FALSE);
    IP_ASSERT_RTN(decodedLen != VOS_NULL_PTR, IP_FALSE);

    /* 获取IP版本号 */
    if (ipMsgLen == 0) {
        IPND_INFO_LOG(ND_TASK_PID, "\nIP_IsIcmpv6Packet: MsgLen Is ZERO!");
        return IP_FALSE;
    }

    ipVersion = IP_GetIpVersion(ipMsg);
    /* 如果版本号不是IPV6，则不是ICMPv6包 */
    if (ipVersion != IP_VERSION_6) {
        IPND_INFO_LOG1(ND_TASK_PID, "IP_IsIcmpv6Packet: IpVersion:", ipVersion);
        return IP_FALSE;
    }

    /* 获取PAYLOAD */
    if (ipMsgLen <= (IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET + 1)) {
        IPND_INFO_LOG(ND_TASK_PID, "\nIP_IsIcmpv6Packet: MsgLen Is Less Than IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET + 1!");
        return IP_FALSE;
    }
    IP_GetUint16Data(payLoad, ipMsg + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);

    /* 长度合法检查 */
    if (((VOS_UINT32)payLoad + IP_IPV6_HEAD_LEN) > ipMsgLen) {
        if (g_ipCommcnNd == 1) {
            PS_PRINTF_INFO("\n***********IP_IsIcmpv6Packet: %d %d***********\n", payLoad + IP_IPV6_HEAD_LEN,
                           ipMsgLen);

            PS_PRINTF_INFO("\n**********************\n\n");
        }
        IPND_WARNING_LOG2(ND_TASK_PID, "IP_IsIcmpv6Packet: PayLoad beyond the msg::!", (payLoad + IP_IPV6_HEAD_LEN),
                          ipMsgLen);
        return IP_FALSE;
    }

    /* 获取上层协议号 */
    rslt = IP_GetIpv6UpLayerProtocol(ipMsg, payLoad, &protocol, &decodedLenCalc);
    if (rslt != IP_SUCC) {
        if (g_ipCommcnNd == 1) {
            PS_PRINTF_INFO("\n***********IP_IsIcmpv6Packet: %d %d***********\n", payLoad + IP_IPV6_HEAD_LEN,
                           ipMsgLen);

            PS_PRINTF_INFO("\n**********************\n\n");
        }
        IPND_WARNING_LOG(ND_TASK_PID, "IP_IsIcmpv6Packet: Fail to get upper-layer protocol!");
        return IP_FALSE;
    }

    /* 判断上层协议号是否为Icmpv6 */
    if (protocol != IP_HEAD_PROTOCOL_ICMPV6) {
        IPND_INFO_LOG1(ND_TASK_PID, "IP_IsIcmpv6Packet: Protocol:", protocol);
        return IP_FALSE;
    }

    *decodedLen = decodedLenCalc;
    return IP_TRUE;
}

/*
 * 功能描述: 为计算checksum构造pseudo-header
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 IP_ConstructICMPv6PseudoHeader(const VOS_UINT8 *msgData, VOS_UINT32 typeOffset,
                                                 IP_Ipv6PseduoHeader *pseduoHeader)
{
    VOS_UINT32 length;
    VOS_UINT32 lengthTmp;
    errno_t    rlt;
    IP_ASSERT_RTN(msgData != VOS_NULL_PTR, 0);
    IP_ASSERT_RTN(pseduoHeader != VOS_NULL_PTR, 0);

    rlt = memcpy_s(pseduoHeader->ipSrc, IP_IPV6_ADDR_LEN, msgData + IP_IPV6_SRC_ADDR_OFFSET,
                   IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, 0);

    rlt = memcpy_s(pseduoHeader->ipDes, IP_IPV6_ADDR_LEN, msgData + IP_IPV6_DST_ADDR_OFFSET,
                   IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, 0);

    pseduoHeader->nextHead = IP_HEAD_PROTOCOL_ICMPV6;

    /* 获取PAYLOAD */
    IP_GetUint16Data(length, msgData + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);

    lengthTmp = (typeOffset - IP_IPV6_HEAD_LEN);
    if (length > lengthTmp) {
        length -= lengthTmp;
        IP_SetUint32Data((VOS_UINT8 *)&pseduoHeader->upperLength, length);
    } else {
        length = 0;
    }

    return length;
}

/*
 * 功能描述: 计算16bit SUM
 * 修改历史:
 *  1.日    期: 2011年4月11日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 IP_Unit16Sum(const VOS_UINT8 *data, VOS_UINT32 len)
{
    VOS_UINT32  checkSum = 0;
    VOS_UINT16 *pBuffer  = VOS_NULL_PTR;

    IP_ASSERT_RTN(data != VOS_NULL_PTR, 0);
    IP_ASSERT_RTN(len != 0, 0);

    pBuffer = (VOS_UINT16 *)(VOS_VOID *)data;

    while (len > 1) {
        checkSum += *(pBuffer++);
        len -= 2; /* 2:len */
    }

    if (len != 0) {
        checkSum += *(VOS_UINT8 *)pBuffer;
    }

    return checkSum;
}

/*
 * 功能描述: 基于pseduo-header和upper-layer protocal计算checksum
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
VOS_UINT16 IPv6_Checksum(const VOS_UINT8 *pseduoHeader, const VOS_UINT8 *data, VOS_UINT32 len)
{
    VOS_UINT32 checkSum = 0;

    IP_ASSERT_RTN(pseduoHeader != VOS_NULL_PTR, 0);
    IP_ASSERT_RTN(data != VOS_NULL_PTR, 0);
    IP_ASSERT_RTN(len != 0, 0);

    /* pseduo-header求和 */
    checkSum += IP_Unit16Sum(pseduoHeader, sizeof(IP_Ipv6PseduoHeader));

    /* IPV6 Upper-layer package求和 */
    checkSum += IP_Unit16Sum(data, len);

    while (checkSum >> 16) { /* 16:len */
        checkSum = (checkSum >> 16) + (checkSum & 0xffff); /* 16:len */
    }

    return (VOS_UINT16)(~checkSum);
}

/*
 * 功能描述: 校验checksum
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
STATIC IP_ErrUint2 IP_VerifyICMPv6(const VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset)
{
    IP_Ipv6PseduoHeader pseduoHeader = {{IP_NULL}};
    VOS_UINT32          len;

    if ((ipMsg == VOS_NULL_PTR) || (typeOffset < IP_IPV6_HEAD_LEN)) {
        IPND_ERROR_LOG(ND_TASK_PID, "IP_VerifyICMPv6: Illegal parameter!");
        return IP_FAIL;
    }

    /* 判断是否需要检验，若没有携带checksum，作为非法包处理 */
    if ((*(ipMsg + typeOffset + IP_ICMPV6_CHECKSUM_OFFSET) == 0) &&
        (*(ipMsg + typeOffset + IP_ICMPV6_CHECKSUM_OFFSET + 1) == 0)) {
        IPND_WARNING_LOG(ND_TASK_PID, "IP_VerifyICMPv6: NONE CHECKSUM!");
        return IP_FAIL;
    }

    /* 构造pseduo-header */
    if ((len = IP_ConstructICMPv6PseudoHeader(ipMsg, typeOffset, &pseduoHeader)) == 0) {
        return IP_FAIL;
    }

    /* 校验ICMPv6包 */
    if (IPv6_Checksum((VOS_UINT8 *)(VOS_UINT_PTR)(&pseduoHeader), ipMsg + typeOffset, len) != 0) {
        IPND_WARNING_LOG(ND_TASK_PID, "IP_VerifyICMPv6: CHECKSUM Error!");
        return IP_FAIL;
    }

    return IP_SUCC;
}

/*
 * 功能描述: 检查是否是合法的ND package
 * 修改历史:
 *  1.日    期: 2011年4月7日
 *    修改内容: 新生成函数
 */
IP_BoolUint8 IP_IsValidNdMsg(const VOS_UINT8 *ipMsg, VOS_UINT32 ipMsgLen, VOS_UINT32 *typeOffset)
{
    VOS_UINT32 decodedLen;
    VOS_UINT8  icmpv6Type;

    IP_ASSERT_RTN(ipMsg != VOS_NULL_PTR, IP_FALSE);
    IP_ASSERT_RTN(typeOffset != VOS_NULL_PTR, IP_FALSE);

    /* 判断是否为ICMPv6包 */
    if (IP_IsIcmpv6Packet(ipMsg, ipMsgLen, &decodedLen) != IP_TRUE) {
        IPND_ERROR_LOG(ND_TASK_PID, "IP_IsValidNdMsg: Not Icmpv6 Packet");
        return IP_FALSE;
    }

    /* 长度不足以携带checksum，作为非法包处理 */
    if (decodedLen + IP_ICMPV6_CHECKSUM_OFFSET + 1 >= ipMsgLen) {
        IPND_WARNING_LOG(ND_TASK_PID, "IP_VerifyICMPv6: NONE CHECKSUM!");
        return IP_FAIL;
    }

    if (IP_VerifyICMPv6(ipMsg, decodedLen) != IP_SUCC) {
        IPND_ERROR_LOG(ND_TASK_PID, "IP_IsValidNdMsg: Verify Fail");
        return IP_FALSE;
    }

    /* 取ICMPV6消息中的TYPE字段 */
    icmpv6Type = *(ipMsg + decodedLen);
    if (icmpv6Type != IP_ICMPV6_TYPE_ECHOREQUEST) {
        if (IP_SUCC != IP_AffirmNdParam(ipMsg, decodedLen)) {
            IPND_ERROR_LOG(ND_TASK_PID, "IP_IsValidNdMsg: ND Param Affirm Fail");
            return IP_FALSE;
        }
    }

    /* TYPE是否位于[128,137] */
    if ((icmpv6Type >= IP_ICMPV6_TYPE_ECHOREQUEST) && (icmpv6Type <= IP_ICMPV6_TYPE_REDIRECT)) {
        *typeOffset = decodedLen;
        return IP_TRUE;
    }

    IPND_ERROR_LOG1(ND_TASK_PID, "IP_IsValidNdMsg: Invalid ICMPv6 Msg:!", icmpv6Type);

    return IP_FALSE;
}

/*
 * Function Name: IP_BuildIcmpv6Checksum
 * Description: 生成ICMPv6报头的校验和
 * Input: pucIpMsg ----------------  报文数据区指针
 *                   ulTypeOffset ------------  ICMPv6报头偏移量
 * Output: pucIpMsg ----------------  在数据区ICMPv6报头位置添加校验和
 * Return: IP_ErrUint2
 *    1.      2011-04-12  Draft Enact
 */
IP_ErrUint2 IP_BuildIcmpv6Checksum(VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset)
{
    IP_Ipv6PseduoHeader pseduoHeader = {{IP_NULL}};
    VOS_UINT32          len;
    VOS_UINT16          checkSum;

    if ((ipMsg == VOS_NULL_PTR) || (typeOffset < IP_IPV6_HEAD_LEN)) {
        IPND_ERROR_LOG(ND_TASK_PID, "IP_BuildICMPv6Checksum: Illegal parameter!\n");
        return IP_FAIL;
    }

    /* 清除ICMPv6校验和字段 */
    ipMsg[typeOffset + IP_ICMPV6_CHECKSUM_OFFSET]     = 0;
    ipMsg[typeOffset + IP_ICMPV6_CHECKSUM_OFFSET + 1] = 0;

    /* 构造pseduo-header */
    len = IP_ConstructICMPv6PseudoHeader(ipMsg, typeOffset, &pseduoHeader);
    if (len == 0) {
        return IP_FAIL;
    }

    /* 生成ICMPv6包校验和 */
    checkSum = IPv6_Checksum((VOS_UINT8 *)(VOS_UINT_PTR)(&pseduoHeader), ipMsg + typeOffset, len);

    *(VOS_UINT16 *)(VOS_VOID *)(&ipMsg[typeOffset + IP_ICMPV6_CHECKSUM_OFFSET]) = checkSum;

    return IP_SUCC;
}


/*
 * 修改历史:
 * 1.日    期: 2011年6月19日
 */
STATIC VOS_UINT32 TTF_NDIS_IP6_Checksum_Update(const void *pbuf, VOS_UINT32 bytes)
{
    VOS_UINT32  sum;
    VOS_UINT32  nwords;
    VOS_UINT16 *buf = VOS_NULL_PTR;

    buf   = (VOS_UINT16 *)pbuf;
    sum = 0;

    for (nwords = (VOS_UINT32)bytes >> 5; nwords > 0; nwords--) { /* 5:bit */
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
    }
    for (nwords = (VOS_UINT32)(bytes & 0x1f) >> 1; nwords > 0; nwords--) {
        sum += *buf++;
    }

    /* Add left-over byte, if any. */
    if (bytes & 0x01) {
        /*lint -e778*/
        sum += *buf & VOS_HTONS(0xff00);
        /*lint +e778*/
    }

    return sum;
}

/*
 * 修改历史:
 * 1.日    期: 2011年6月19日
 */
STATIC VOS_UINT32 TTF_NDIS_IP6_Pseudo_Header_Checksum_Update(const NDIS_Ipv6Addr *ip6SrcAddr,
                                                             const NDIS_Ipv6Addr *ip6DstAddr, VOS_UINT32 proto,
                                                             VOS_UINT16 len)
{
    VOS_UINT32 sum;

    sum = ip6SrcAddr->in6.addr16bit[0];
    sum += ip6SrcAddr->in6.addr16bit[1];
    sum += ip6SrcAddr->in6.addr16bit[2]; /* 2:addr */
    sum += ip6SrcAddr->in6.addr16bit[3]; /* 3:addr */
    sum += ip6SrcAddr->in6.addr16bit[4]; /* 4:addr */
    sum += ip6SrcAddr->in6.addr16bit[5]; /* 5:addr */
    sum += ip6SrcAddr->in6.addr16bit[6]; /* 6:addr */
    sum += ip6SrcAddr->in6.addr16bit[7]; /* 7:addr */

    sum += ip6DstAddr->in6.addr16bit[0];
    sum += ip6DstAddr->in6.addr16bit[1];
    sum += ip6DstAddr->in6.addr16bit[2]; /* 2:addr */
    sum += ip6DstAddr->in6.addr16bit[3]; /* 3:addr */
    sum += ip6DstAddr->in6.addr16bit[4]; /* 4:addr */
    sum += ip6DstAddr->in6.addr16bit[5]; /* 5:addr */
    sum += ip6DstAddr->in6.addr16bit[6]; /* 6:addr */
    sum += ip6DstAddr->in6.addr16bit[7]; /* 7:addr */

    sum += VOS_HTONS(len);
    sum += VOS_HTONS(proto);

    return sum;
}

/*
 * 修改历史:
 * 1.日    期: 2011年6月19日
 */
STATIC VOS_UINT16 TTF_NDIS_IP6_Checksum_Finish(VOS_UINT32 sum)
{
    /* Fold 32-bit sum to 16 bits. */
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16); /* 16:bit */
    }

    return (VOS_UINT16) ~(VOS_UINT16)sum;
}

/*
 * 功能描述: 计算IP报文CheckSum
 * 修改历史:
 *  1.日    期: 2011年5月24日
 *    修改内容: 新生成函数
 */
VOS_UINT16 TTF_NDIS_Ipv6_CalcCheckSum(VOS_UINT8 *pData, VOS_UINT16 len, const NDIS_Ipv6Addr *ip6SrcAddr,
                                      const NDIS_Ipv6Addr *ip6DstAddr, VOS_UINT32 proto)
{
    VOS_UINT32 checkSum;
    VOS_UINT32 pseudoHdrChksum;
    VOS_UINT32 finalChksum;
    VOS_UINT16 stdIntelChksum;

    checkSum        = TTF_NDIS_IP6_Checksum_Update(pData, len);
    pseudoHdrChksum = TTF_NDIS_IP6_Pseudo_Header_Checksum_Update(ip6SrcAddr, ip6DstAddr, proto, len);
    finalChksum     = checkSum + pseudoHdrChksum;
    stdIntelChksum  = TTF_NDIS_IP6_Checksum_Finish(finalChksum);

    return stdIntelChksum;
}

/*
 * 功能描述: Returns a specific option.
 * 修改历史:
 *  1.日    期: 2011年4月28日
 *    修改内容: 新生成函数
 */
VOS_UINT32 TTF_NDIS_Ipv6GetDhcpOption(IP_Ipv6PktDhcpOptHdr *pFirstOpt, VOS_UINT16 totOptLen,
                                      VOS_UINT16 ndOptCode, IP_Ipv6PktDhcpOptHdr **pOpt, VOS_INT32 lNumber)
{
    VOS_UINT16 optLen;

    *pOpt = pFirstOpt;

    /*
     * 避免读越界, 并保护usTotOptLen与usOptLen之间的长度关系
     * IPV6_PKT_DHCP_OPT_HDR_STRU中 OptCode与OptLen的长度
     */
    /* 4表示2个字节的option-code+2字节的option-len */
    while (totOptLen >= 4) {
        optLen = VOS_NTOHS((*pOpt)->dhcpOptLen);
        if (optLen == 0) {
            return PS_FAIL;
        }

        /* 4:剩余总长度小于当前Option长度, 退出 */
        if (totOptLen < (optLen + 4)) {
            break;
        }

        if (ndOptCode == VOS_NTOHS((*pOpt)->dhcpOptCode)) {
            if (lNumber == 0) {
                return PS_SUCC;
            }

            lNumber--;
        }

        totOptLen -= (optLen + 4); /* 4:len */
        *pOpt = (IP_Ipv6PktDhcpOptHdr *)(VOS_VOID *)(((VOS_UINT8 *)*pOpt + optLen) + 4); /* 4:len */
    }

    *pOpt = VOS_NULL_PTR;

    return PS_SUCC;
}

/*
 * 功能描述: 填充IP头部
 * 修改历史:
 *  1.日    期: 2011年4月28日
 *    修改内容: 新生成函数
 */
VOS_UINT32 TTF_NDIS_InputUDPHead(VOS_UINT8 *pBuf, VOS_UINT16 srcPort, VOS_UINT16 dstPort, VOS_UINT16 udpDataLen)
{
    IP_UdpHeadInfo *udpHdr = (IP_UdpHeadInfo *)(VOS_VOID *)pBuf;
    if (udpDataLen < sizeof(IP_UdpHeadInfo)) {
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(udpHdr, sizeof(IP_UdpHeadInfo), 0, sizeof(IP_UdpHeadInfo));

    udpHdr->srcPort = VOS_HTONS(srcPort);
    udpHdr->dstPort = VOS_HTONS(dstPort);
    udpHdr->udpLen     = VOS_HTONS(udpDataLen);
    udpHdr->udpCheckSum   = 0;

    return VOS_OK;
}

