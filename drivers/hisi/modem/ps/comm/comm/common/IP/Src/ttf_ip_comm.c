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

#include "ttf_ip_comm.h"
#include "securec.h"
#include "PsTypeDef.h"
#include "ttf_comm.h"


#define THIS_FILE_ID PS_FILE_ID_TTF_IP_COMM_C

typedef struct {
    VOS_UINT16              headLen;
    VOS_UINT16              totalLen;
    IpDataProtocolEnumUint8 protocolType;
    VOS_UINT8               rsv[3];
} TtfIpHeadParseResult;

typedef VOS_UINT32 (*TtfIpCheckDataLenFun)(
    const VOS_UINT8 *data, VOS_UINT16 dataLen, const TtfIpHeadParseResult *ipHeadInfo);
typedef IpDataTypeEnumUint8 (*TtfIpGetDataType)(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo);
typedef VOS_UINT16 (*TtfIpGetTraceLen)(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo);

typedef struct {
    TtfIpCheckDataLenFun checkLenFun;
    TtfIpGetDataType     getTypeFun;
    TtfIpGetTraceLen     getTraceLenFun;
} TtfIpProtocolMethod;

typedef struct {
    VOS_UINT32          type;
    TtfIpProtocolMethod method;
} TtfIpProtocolMethodMap;

STATIC VOS_UINT16 TTF_GetTcpHeadLen(const VOS_UINT8 *ipData, VOS_UINT16 headLen)
{
    /* TCP HEAD LEN占用字节高4bit，字段含义是32位字节长度，这里需要右移4位、左移2位，合并后右移2位 */
    return (ipData[headLen + TCP_LEN_POS] & TCP_LEN_MASK) >> 2;
}

STATIC VOS_UINT32 TTF_CheckTcpDataLen(const VOS_UINT8 *data, VOS_UINT16 dataLen, const TtfIpHeadParseResult *ipHeadInfo)
{
    VOS_UINT16 tcpHeadLen;

    if (dataLen < (ipHeadInfo->headLen + TTF_TCP_HEAD_NORMAL_LEN)) {
        /* 数据包长度，不够容纳完整的TCP头， */
        return VOS_ERR;
    }

    tcpHeadLen = TTF_GetTcpHeadLen(data, ipHeadInfo->headLen);
    if (dataLen < (tcpHeadLen + ipHeadInfo->headLen)) {
        /* 数据包长度，不够容纳完整的TCP头， */
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 TTF_CheckUdpDataLen(const VOS_UINT8 *data, VOS_UINT16 dataLen, const TtfIpHeadParseResult *ipHeadInfo)
{
    VOS_UINT16 udpLen;

    if (dataLen < (ipHeadInfo->headLen + UDP_HEAD_LEN)) {
        /* 数据包长度，不够容纳完整的UDP头 */
        return VOS_ERR;
    }

    /* 获取 UDP的总长度 */
    udpLen = IP_GET_VAL_NTOH_U16(data, (ipHeadInfo->headLen + TTF_UDP_LEN_POS));
    if (udpLen < UDP_HEAD_LEN) {
        /* UdpLen 字段非法 */
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 TTF_CheckIcmpDataLen(
    const VOS_UINT8 *data, VOS_UINT16 dataLen, const TtfIpHeadParseResult *ipHeadInfo)
{
    if (dataLen < (ipHeadInfo->headLen + ICMP_HEADER_LEN)) {
        /* 数据包长度，不够容纳完整的ICMPV4头 */
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 TTF_CheckDataLenDefault(
    const VOS_UINT8 *data, VOS_UINT16 dataLen, const TtfIpHeadParseResult *ipHeadInfo)
{
    return VOS_OK;
}

/* 解析TCP包，获取TCP 类型; data 入参有效性由调用者保证，为IP数据包 */
STATIC IpDataTypeEnumUint8 TTF_GetTcpType(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    VOS_UINT32       tcpFlag;
    VOS_UINT16       port;
    const VOS_UINT16 tcpHeadLen = TTF_GetTcpHeadLen(data, ipHeadInfo->headLen);

    /* SDU数据长度等于IP包头长度和TCP包头部长度之和，并且TCP包FLAG标志中含有ACK */
    if (ipHeadInfo->totalLen == (tcpHeadLen + ipHeadInfo->headLen)) {
        tcpFlag = data[ipHeadInfo->headLen + TCP_FLAG_POS] & TCP_FLAG_MASK;

        if ((TCP_SYN_MASK & tcpFlag) == TCP_SYN_MASK) {
            return IP_DATA_TYPE_TCP_SYN;
        }

        if ((TCP_ACK_MASK & tcpFlag) == TCP_ACK_MASK) {
            return IP_DATA_TYPE_TCP_ACK;
        }
    } else {
        port = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen + TCP_DST_PORT_POS);
        if (port == FTP_DEF_SERVER_SIGNALLING_PORT) {
            return IP_DATA_TYPE_FTP_SIGNALLING;
        }
    }

    return IP_DATA_TYPE_TCP;
}

/* 解析UDP包，获取UDP 类型; data 入参有效性由调用者保证，为IP数据包 */
STATIC IpDataTypeEnumUint8 TTF_GetUdpType(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    const VOS_UINT32 portOffset = ipHeadInfo->headLen + UDP_DST_PORT_POS;
    const VOS_UINT16 port       = IP_GET_VAL_NTOH_U16(data, portOffset);

    if (port == DNS_DEF_SERVER_PORT) {
        return IP_DATA_TYPE_UDP_DNS;
    }

    if (port == MIP_AGENT_PORT) {
        return IP_DATA_TYPE_UDP_MIP;
    }

    return IP_DATA_TYPE_UDP;
}

/* 解析ICMPV4包，获取ICMPV4 类型. data 入参有效性由调用者保证，为IP数据包首地址 */
STATIC IpDataTypeEnumUint8 TTF_GetIcmpV4Type(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    const VOS_UINT16 fragmentOffset = IP_GET_VAL_NTOH_U16(data, IPV4_HEAD_FRAGMENT_OFFSET_POS) &
                                      IPV4_HEAD_FRAGMENT_OFFSET_MASK;

    /* 分段 */
    if (fragmentOffset != 0) {
        return IP_DATA_TYPE_NULL;
    }

    if ((data[ipHeadInfo->headLen] == ICMP_TYPE_REQUEST) || (data[ipHeadInfo->headLen] == ICMP_TYPE_REPLY)) {
        return IP_DATA_TYPE_ICMP;
    }

    return IP_DATA_TYPE_NULL;
}

/* 解析ICMPV6包，获取ICMPV6类型； data 入参有效性由调用者保证，为IP数据包首地址 */
STATIC IpDataTypeEnumUint8 TTF_GetIcmpV6Type(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    const VOS_UINT8 icmpType = data[ipHeadInfo->headLen];

    /* 获取ICMPV6报文的类型 */
    if ((icmpType == ICMPV6_TYPE_REQUEST) || (icmpType == ICMPV6_TYPE_REPLY)) {
        return IP_DATA_TYPE_ICMP;
    }

    return IP_DATA_TYPE_NULL;
}

STATIC IpDataTypeEnumUint8 TTF_GetDataTypeDefault(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    return IP_DATA_TYPE_NULL;
}

/* 解析TCP包，获取TCP 可trace长度; data 入参有效性由调用者保证，为IP数据包 */
STATIC VOS_UINT16 TTF_GetTcpTraceLen(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    VOS_UINT16       traceLen, sourcePort, destPort;
    const VOS_UINT16 tcpHeadLen = TTF_GetTcpHeadLen(data, ipHeadInfo->headLen);

    /* SDU数据长度等于IP包头长度和TCP包头部长度之和，并且TCP包FLAG标志中含有ACK */
    if (ipHeadInfo->totalLen == (tcpHeadLen + ipHeadInfo->headLen)) {
        traceLen = ipHeadInfo->totalLen;
    } else {
        sourcePort = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen);
        destPort   = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen + TCP_DST_PORT_POS);

        /* FTP命令全部勾取，其它勾TCP头 */
        if ((sourcePort == FTP_DEF_SERVER_SIGNALLING_PORT) ||
            (destPort == FTP_DEF_SERVER_SIGNALLING_PORT)) {
            traceLen = ipHeadInfo->totalLen;
        } else {
            traceLen = ipHeadInfo->headLen + tcpHeadLen;
        }
    }

    return traceLen;
}

/* 解析UDP包，获取UDP 可trace的长度; data 入参有效性由调用者保证，为IP数据包 */
STATIC VOS_UINT16 TTF_GetUdpTraceLen(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    VOS_UINT16 sourcePort = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen);
    VOS_UINT16 destPort   = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen + UDP_DST_PORT_POS);
    VOS_UINT16 traceLen; 

    /* DNS全部勾取，其它勾UDP头 */
    if ((sourcePort == DNS_DEF_SERVER_PORT) || (destPort == DNS_DEF_SERVER_PORT)) {
        traceLen = ipHeadInfo->totalLen;
    } else {
        traceLen = ipHeadInfo->headLen + UDP_HEAD_LEN;
    }

    return traceLen;
}

STATIC VOS_UINT16 TTF_GetIcmpTraceLen(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    return ipHeadInfo->headLen + ICMP_HEADER_LEN;
}

STATIC VOS_UINT16 TTF_GetTraceLenDefault(const VOS_UINT8 *data, const TtfIpHeadParseResult *ipHeadInfo)
{
    return ipHeadInfo->headLen;
}

#define DECLEAR_METHOD(chkLen, getType, getTraceLen)                                \
    {                                                                               \
        .checkLenFun = chkLen, .getTypeFun = getType, .getTraceLenFun = getTraceLen \
    }

STATIC TtfIpProtocolMethodMap g_protocolMethodMap[] = {
    { .type = IP_DATA_PROTOCOL_TCP, .method = DECLEAR_METHOD(TTF_CheckTcpDataLen, TTF_GetTcpType, TTF_GetTcpTraceLen) },
    { .type = IP_DATA_PROTOCOL_UDP, .method = DECLEAR_METHOD(TTF_CheckUdpDataLen, TTF_GetUdpType, TTF_GetUdpTraceLen) },
    { .type = IP_DATA_PROTOCOL_ICMPV4, .method = DECLEAR_METHOD(TTF_CheckIcmpDataLen, TTF_GetIcmpV4Type, TTF_GetIcmpTraceLen) },
    { .type = IP_DATA_PROTOCOL_ICMPV6, .method = DECLEAR_METHOD(TTF_CheckIcmpDataLen, TTF_GetIcmpV6Type, TTF_GetIcmpTraceLen) },
};

STATIC TtfIpProtocolMethod g_protocolMethodNullObj = DECLEAR_METHOD(
    TTF_CheckDataLenDefault, TTF_GetDataTypeDefault, TTF_GetTraceLenDefault);

STATIC TtfIpProtocolMethod* TTF_GetProtocolMethod(IpDataProtocolEnumUint8 protocolType)
{
    VOS_UINT32 loop;

    for (loop = 0; loop < TTF_GET_ARRAYSIZE(g_protocolMethodMap); loop++) {
        if (protocolType == g_protocolMethodMap[loop].type) {
            return &(g_protocolMethodMap[loop].method);
        }
    }

    return &g_protocolMethodNullObj;
}

STATIC VOS_UINT32 TTF_PraseIpHead(
    VOS_UINT32 pid, const VOS_UINT8 *data, VOS_UINT16 dataLen, TtfIpHeadParseResult *result)
{
    VOS_UINT32 ipVersion;

    if (dataLen <= IPV4_HEAD_NORMAL_LEN) {
        TTF_LOG(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType IPHeadLen is exception.");
        return VOS_ERR;
    }

    ipVersion = data[0] & IP_VER_MASK;
    if (ipVersion == IPV4_VER_VAL) {
        result->headLen      = IP_HEADER_LEN_2_BYTE_NUM(data[0] & IP_HEADER_LEN_MASK);
        result->totalLen     = IP_GET_VAL_NTOH_U16(data, IP_IPV4_DATA_LEN_POS);
        result->protocolType = data[PROTOCOL_POS];
    } else if (ipVersion == IPV6_VER_VAL) {
        result->headLen      = IPV6_HEAD_NORMAL_LEN;
        result->totalLen     = IP_GET_VAL_NTOH_U16(data, IP_IPV6_DATA_LEN_POS) + IPV6_HEAD_NORMAL_LEN;
        result->protocolType = data[PROTOCOL_POS_V6];
    } else {
        TTF_LOG(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType Protocol is Null.");
        return VOS_ERR;
    }

    if ((dataLen < result->totalLen) || (result->totalLen < result->headLen)) {
        TTF_LOG3(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType data len invalid.", dataLen,
            result->totalLen, result->headLen);
        return VOS_ERR;
    }

    return VOS_OK;
}

MODULE_EXPORTED IpDataTypeEnumUint8 TTF_ParseIpDataType(VOS_UINT32 pid, const VOS_UINT8 *data,
    VOS_UINT16 dataLen, TTF_PsDataPriorityUint8 dataPriority)
{
    TtfIpHeadParseResult ipHeadInfo = {0};
    TtfIpProtocolMethod *method     = VOS_NULL_PTR;

    if (TTF_PraseIpHead(pid, data, dataLen, &ipHeadInfo) != VOS_OK) {
        return IP_DATA_TYPE_BUTT;
    }

#if (FEATURE_BASTET == FEATURE_ON)
    if (dataPriority == TTF_PS_DATA_PRIORITY_HIGH) {
        TTF_LOG(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType user high priority data.");
        return IP_DATA_TYPE_USER_HIGH;
    }
#endif

    method = TTF_GetProtocolMethod(ipHeadInfo.protocolType);

    /* 安全检查: 检查数据包大小是否能够容纳对应协议包头，不能容纳的异常包，就不用继续解析了 */
    if (method->checkLenFun(data, dataLen, &ipHeadInfo) != VOS_OK) {
        TTF_LOG2(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "datalen<1> ProtocalType<2> is exception.", dataLen,
            ipHeadInfo.protocolType);
        return IP_DATA_TYPE_BUTT;
    }

    return method->getTypeFun(data, &ipHeadInfo);
}

MODULE_EXPORTED VOS_UINT16 TTF_GetIpDataTraceLen(VOS_UINT32 pid, const VOS_UINT8 *data, VOS_UINT16 dataLen)
{
    TtfIpHeadParseResult ipHeadInfo = {0};
    TtfIpProtocolMethod *method     = VOS_NULL_PTR;

    if (TTF_PraseIpHead(pid, data, dataLen, &ipHeadInfo) != VOS_OK) {
        return 0;
    }

    method = TTF_GetProtocolMethod(ipHeadInfo.protocolType);

    /* 安全检查: 检查数据包大小是否能够容纳对应协议包头，不能容纳的异常包，就不用继续解析了 */
    if (method->checkLenFun(data, dataLen, &ipHeadInfo) != VOS_OK) {
        TTF_LOG2(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "datalen<1> ProtocalType<2> is exception.", dataLen,
            ipHeadInfo.protocolType);
        return 0;
    }

    return method->getTraceLenFun(data, &ipHeadInfo);
}

#define TTF_IPV4_MASK_IP_ADDR_SENSITIVE_POS 3       /* IPV4 地址脱敏的位置 */
#define TTF_IPV4_MASK_IP_ADDR_SENSITIVE_BYTE_NUM 1  /* IPV4 地址脱敏的字节数 */
#define TTF_IPV6_MASK_IP_ADDR_SENSITIVE_POS 5       /* IPV6 地址脱敏的位置 */
#define TTF_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM 11 /* IPV6 地址脱敏的字节数 */

MODULE_EXPORTED VOS_VOID TTF_FilterIpv4AddrSensitiveInfo(VOS_UINT8 *ipAddr)
{
    *(ipAddr + TTF_IPV4_MASK_IP_ADDR_SENSITIVE_POS) = 0;
}

MODULE_EXPORTED VOS_VOID TTF_FilterIpv6AddrSensitiveInfo(VOS_UINT8 *ipAddr)
{
    VOS_UINT8 *ipSensitiveAddr = ipAddr + TTF_IPV6_MASK_IP_ADDR_SENSITIVE_POS;
    (VOS_VOID)memset_s(
        ipSensitiveAddr, TTF_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM, 0, TTF_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM);
}

MODULE_EXPORTED VOS_VOID TTF_MaskIpAddrTraces(VOS_UINT32 pid, VOS_UINT8 *ipData, VOS_UINT16 dataLen)
{
    const VOS_UINT32 ipVersion = ipData[0] & IP_VER_MASK;

    if (ipVersion == IPV4_VER_VAL) {
        if (dataLen < IPV4_HEAD_NORMAL_LEN) {
            TTF_LOG1(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_MaskIpAddrTraces IPHeadLen is exception.", dataLen);
            return;
        }

        TTF_FilterIpv4AddrSensitiveInfo(ipData + IPV4_SRC_IP_ADDR_OFFSET_POS);
        TTF_FilterIpv4AddrSensitiveInfo(ipData + IPV4_DST_IP_ADDR_OFFSET_POS);
    } else if (ipVersion == IPV6_VER_VAL) {
        if (dataLen < IPV6_HEAD_NORMAL_LEN) {
            TTF_LOG1(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_MaskIpAddrTraces IPHeadLen is exception.", dataLen);
            return;
        }

        TTF_FilterIpv6AddrSensitiveInfo(ipData + IPV6_SRC_IP_ADDR_OFFSET_POS);
        TTF_FilterIpv6AddrSensitiveInfo(ipData + IPV6_DST_IP_ADDR_OFFSET_POS);
    } else {
        TTF_LOG(pid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_MaskIpAddrTraces Protocol is Null.");
    }
}

