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

#ifndef __IPCOMM_H__
#define __IPCOMM_H__

/*
 * 1 Include Headfile
 */
#include "vos.h"
#include "ttf_comm.h"
#include "ip_ipm_global.h"
/*
 * 1.1 Cplusplus Announce
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*
 * #pragma pack(*)    设置字节对齐方式
 */
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*
 * 2 Macro
 */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#define ND_TASK_PID UEPS_PID_NDCLIENT
#elif (OSA_CPU_ACPU == VOS_OSA_CPU)
#define ND_TASK_PID UEPS_PID_NDSERVER
#endif

/* 通过ModemId和EpsId组合成exEpsId，高2bit为ModemId,第6bit为EpsId */
#define ND_FORM_EXEPSID(modemId, epsId) (VOS_UINT8)((VOS_UINT8)(((modemId) << 6) | (epsId)))

/* 从ExEpsId中获得ModemId */
#define ND_GET_MODEMID_FROM_EXEPSID(exEpsId) (VOS_UINT16)(((exEpsId)&0xC0) >> 6)

/* 从ExEpsId中获得EpsId */
#define ND_GET_EPSID_FROM_EXEPSID(exEpsId) (VOS_UINT8)((exEpsId)&0x3F)
#define IP_DHCPV4SERVER_OFF 0
#define IP_DHCPV4SERVER_ON 1

#define IP_IPV6_LINK_LOCAL_PREFIX 0xfe80
#define IP_IPV6_LINK_LOCAL_MASK 0xffc0
#define IP_IPV6_ND_HOP_LIMIT 0xff
#define IP_IPV6_ND_VALID_CODE 0

#define ND_MAX_EPSB_ID 15 /* 最大的承载号 */
#define ND_MIN_EPSB_ID 5  /* 最小的承载号 */

#define ND_IP_MAX_PREFIX_NUM_IN_RA 6
#define ND_IPV6_IF_OFFSET 8
#define ND_IPV6_IF_LEN 8
#define ND_IP_IPV6_PREFIX_LENGTH 8

#define ND_IP_IPV6_IFID_LENGTH 8
#define ND_IP_IPV6_ADDR_LENGTH 16

#define IP_MAX_PREFIX_NUM_IN_RA ND_IP_MAX_PREFIX_NUM_IN_RA

#define IP_IPV6_OP_TRUE 1
#define IP_IPV6_DHCP6_TYPE 55
#define IP_IPV6_DHCP6_PC_PORT 546
#define IP_IPV6_DHCP6_UE_PORT 547
#define IP_IPV6_DHCP6_INFOR_REQ 11
#define IP_IPV6_DHCP6_TYPE_REPLY 7
#define IP_IPV6_DHCP6_TRANS_ID_LEN 3
#define IP_IPV6_DHCP6_OPT_DNS_SERVERS 23
#define IP_IPV6_DHCP6_REPLY_HDR_LEN 8 /* DHCP REPLY 头四字节 + Option头四字节 */

#define IP_IPV6_HW_TYPE 1
#define IP_IPV6_MAX_DNS_NUM 2
#define IP_IPV6_DHCP_OPT_LINKADDR_SIZE 6
#define IP_IPV6_DHCP_OPT_CLIEND_ID 1
#define IP_IPV6_DHCP_OPT_SERVER_ID 2
#define IP_IPV6_DHCP_OPT_REQUEST 6
#define IP_IPV6_DHCP_DUID_LL_OPT_TYPE 3
#define IP_IPV6_DHCP_OPT_CLIENT_ID_LEN (10 + 4)
#define IP_IPV6_DHCP_DUID_LL_OPT_LEN 10

#define IP_IPV6_BASIC_HEAD_NEXT_HEAD_OFFSET 6
#define IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET 4
#define IP_IPV6_BASIC_HOP_LIMIT_OFFSET 7
#define IP_IPV6_SRC_ADDR_OFFSET 8
#define IP_IPV6_DST_ADDR_OFFSET 24
#define IP_IPV6_EXT_HEAD_NEXT_HEAD_OFFSET 0
#define IP_IPV6_EXT_HEAD_LEN_OFFSET 1
#define IP_IPV6_FRA_EXT_HEAD_FRGOFFSET_OFFSET 2
#define IP_IPV6_FRA_EXT_HEAD_ID_OFFSET 4
#define IP_IPV6_FRAMENT_EXT_HEAD_LEN 8
#define IP_IPV6_AH_EXT_HEAD_SPI_OFFSET 4
#define IP_IPV6_AH_EXT_HEAD_SQN_OFFSET 8
#define IP_IPV6_ESP_EXT_HEAD_SQN_OFFSET 4

#define IP_ICMPV6_TYPE_OFFSET 0
#define IP_ICMPV6_CODE_OFFSET 1
#define IP_ICMPV6_CHECKSUM_OFFSET 2
#define IP_ICMPV6_HOP_LIMIT_OFFSET 4

#define IP_ICMPV6_INIT_CHECKSUM 0

#define IP_PACKAGE_HEADER_RESERVE 2

#define IP_ICMPV6_RS_MAX_LENGTH 200

#define IP_ND_STAT_INC(x) ((x) = ((x) + (1)))

#define IP_ICMPV6_NA_FLAG_OFFSET 4

#define IP_ICMPV6_TARGET_ADDR_OFFSET 8

#define IP_ICMPV6_RS_HEADER_LEN 8
#define IP_ICMPV6_RA_HEADER_LEN 16
#define IP_ICMPV6_NS_HEADER_LEN 24
#define IP_ICMPV6_NA_HEADER_LEN 24

#define IP_ND_OPT_UNIT_LEN 8

/* 内存操作 */
#define IP_MEM_CMP(destBuffer, srcBuffer, bufferLen) VOS_MemCmp(destBuffer, srcBuffer, bufferLen)

#define IP_CHK_SEC_RETURN_NO_VAL(x) do {\
    if (x) {                        \
        return;                     \
    } \
} while (0)
#define IP_CHK_SEC_RETURN_VAL(x, y) do {\
    if (x) {                        \
        return (y);                 \
    } \
} while (0)

#define IP_CHK_SEC_CONTINUE(x) \
    if (x) {                   \
        continue;              \
    }

/* replace by safe function end */
#define IP_GetNdOptionLen(ndOptionLen) ((ndOptionLen) * IP_ND_OPT_UNIT_LEN)

#define IP_GetFragOffset(frgOffset, inputData) ((frgOffset) = (((inputData) & 0xfff8) >> 3))
#define IP_GetFragM(frgM, inputData) ((frgM) = ((inputData) & 0x0001))

#define IP_GetExtensionLen(extHeaderLen) (((extHeaderLen) + 1) * 8)

#define IP_IPV6_64BITPREFIX_EQUAL_ZERO(ipv6Addr)                                               \
    (((*(ipv6Addr)) == 0) && ((*(ipv6Addr + 1)) == 0) && ((*(ipv6Addr + 2)) == 0) &&     \
     ((*(ipv6Addr + 3)) == 0) && ((*(ipv6Addr + 4)) == 0) && ((*(ipv6Addr + 5)) == 0) && \
     ((*(ipv6Addr + 6)) == 0) && ((*(ipv6Addr + 7)) == 0))

#define IP_IPV6_IS_LINKLOCAL_ADDR(ipv6Addr) (((*(ipv6Addr)) == 0xfe) && (((*(ipv6Addr + 1)) & 0xc0) == 0x80))

#define IP_IPV6_IS_MULTICAST_ADDR(ipv6Addr) (0xff == (*(ipv6Addr)))

#define IP_IPV6_EQUAL_ALL_ZERO(ipv6Addr)                                                          \
    (((*(ipv6Addr)) == 0) && ((*(ipv6Addr + 1)) == 0) && ((*(ipv6Addr + 2)) == 0) &&        \
     ((*(ipv6Addr + 3)) == 0) && ((*(ipv6Addr + 4)) == 0) && ((*(ipv6Addr + 5)) == 0) &&    \
     ((*(ipv6Addr + 6)) == 0) && ((*(ipv6Addr + 7)) == 0) && ((*(ipv6Addr + 8)) == 0) &&    \
     ((*(ipv6Addr + 9)) == 0) && ((*(ipv6Addr + 10)) == 0) && ((*(ipv6Addr + 11)) == 0) &&  \
     ((*(ipv6Addr + 12)) == 0) && ((*(ipv6Addr + 13)) == 0) && ((*(ipv6Addr + 14)) == 0) && \
     ((*(ipv6Addr + 15)) == 0))

#define IP_ProduceSolicitedNodeMulticastIPAddr(dstIpv6Addr, srcIpv6Addr) do { \
    (*(dstIpv6Addr))      = (0xff);                                     \
    (*(dstIpv6Addr + 1))  = (0x02);                                     \
    (*(dstIpv6Addr + 11)) = (0x01);                                     \
    (*(dstIpv6Addr + 12)) = (0xff);                                     \
    (*(dstIpv6Addr + 13)) = (*(srcIpv6Addr + 13));                   \
    (*(dstIpv6Addr + 14)) = (*(srcIpv6Addr + 14));                   \
    (*(dstIpv6Addr + 15)) = (*(srcIpv6Addr + 15));                   \
} while (0)

#define IP_ProduceSolicitedNodeMulticastMacAddr(dstMacAddr, srcIpv6Addr) do { \
    (*(dstMacAddr))     = (0x33);                                       \
    (*(dstMacAddr + 1)) = (0x33);                                       \
    (*(dstMacAddr + 2)) = (0xff);                                       \
    (*(dstMacAddr + 3)) = (*(srcIpv6Addr + 13));                     \
    (*(dstMacAddr + 4)) = (*(srcIpv6Addr + 14));                     \
    (*(dstMacAddr + 5)) = (*(srcIpv6Addr + 15));                     \
} while (0)

#define IP_ProduceIfaceIdFromMacAddr(ipv6Addr, macAddr) do { \
    (*(ipv6Addr + 8))  = ((*(macAddr)) ^ 0x2);      \
    (*(ipv6Addr + 9))  = (*(macAddr + 1));          \
    (*(ipv6Addr + 10)) = (*(macAddr + 2));          \
    (*(ipv6Addr + 11)) = (0xff);                       \
    (*(ipv6Addr + 12)) = (0xfe);                       \
    (*(ipv6Addr + 13)) = (*(macAddr + 3));          \
    (*(ipv6Addr + 14)) = (*(macAddr + 4));          \
    (*(ipv6Addr + 15)) = (*(macAddr + 5));          \
} while (0)

#define IP_GetUint16Data(short16Data, firstByte) do { \
    (short16Data) = (*(firstByte)) << IP_BITMOVE_8; \
    (short16Data) |= (*((firstByte) + 1));          \
} while (0)
#define IP_GetUint32Data(long32Data, firstByte) do { \
    (long32Data) = (VOS_UINT32)(*(firstByte)) << IP_BITMOVE_24;        \
    (long32Data) |= (VOS_UINT32)(*((firstByte) + 1)) << IP_BITMOVE_16; \
    (long32Data) |= (VOS_UINT32)(*((firstByte) + 2)) << IP_BITMOVE_8;  \
    (long32Data) |= (VOS_UINT32)(*((firstByte) + 3));                  \
} while (0)

#define IP_SetUint16Data(firstByte, short16Data) do { \
    (*(firstByte))       = ((short16Data) >> IP_BITMOVE_8) & 0xff; \
    (*((firstByte) + 1)) = (short16Data)&0xff;                     \
} while (0)

#define IP_SetUint32Data(firstByte, long32Data) do { \
    (*(firstByte))       = ((long32Data) >> IP_BITMOVE_24) & 0xff; \
    (*((firstByte) + 1)) = ((long32Data) >> IP_BITMOVE_16) & 0xff; \
    (*((firstByte) + 2)) = ((long32Data) >> IP_BITMOVE_8) & 0xff;  \
    (*((firstByte) + 3)) = (long32Data) & 0xff;                      \
} while (0)

/* 封装写IP消息头的宏 */
#if (VOS_OS_VER != VOS_WIN32)
#define ND_IP_WRITE_MSG_HEAD(writeMsg, rabmMsgID, sendPid, receivePid) do { \
    (writeMsg)->msgId         = (rabmMsgID);                             \
    (writeMsg)->ulSenderPid     = sendPid;                        \
    (writeMsg)->ulReceiverPid   = receivePid; \
} while (0)
#else
#define ND_IP_WRITE_MSG_HEAD(writeMsg, rabmMsgID, sendPid, receivePid) do { \
    (writeMsg)->msgId         = (rabmMsgID);                             \
    (writeMsg)->ulSenderCpuId   = VOS_LOCAL_CPUID;               \
    (writeMsg)->ulSenderPid     = sendPid;                            \
    (writeMsg)->ulReceiverCpuId = VOS_LOCAL_CPUID;                    \
    (writeMsg)->ulReceiverPid   = receivePid; \
} while (0)
#endif

#define ND_WRITE_NDCLIENT_INFO(ndClientEntity, rs, upperLength) do { \
    ndClientEntity->msgType                = IP_ICMPV6_TYPE_RS; \
    ndClientEntity->ipSndNwMsg.sendMsg     = rs; \
    ndClientEntity->ipSndNwMsg.sendMsgSize = IP_IPV6_HEAD_LEN + upperLength; \
    ndClientEntity->raInfo.apsTrigerRS     = IP_TRUE; \
} while (0)

#ifndef IP_RELEASE
#if (VOS_OS_VER != VOS_WIN32)
#define IP_ASSERT(exp) do {  \
    if (!(exp)) {                                       \
        vos_assert((VOS_UINT32)THIS_FILE_ID, __LINE__); \
        return;                                         \
    } \
} while (0)

#define IP_ASSERT_RTN(exp, ret) do {                        \
    if (!(exp)) {                                       \
        vos_assert((VOS_UINT32)THIS_FILE_ID, __LINE__); \
        return (ret);                                   \
    } \
} while (0)
#else

#define IP_ASSERT(exp) do {                                                    \
    if (!(exp)) {                                                          \
        PS_PRINTF_INFO("Assert, File: %s, Line:%d\n", __FILE__, __LINE__); \
        return;                                                            \
    } \
} while (0)

#define IP_ASSERT_RTN(exp, ret) do {                                           \
    if (!(exp)) {                                                          \
        PS_PRINTF_INFO("Assert, File: %s, Line:%d\n", __FILE__, __LINE__); \
        return (ret);                                                      \
    } \
} while (0)

#endif

#else
#define IP_ASSERT(exp) ((VOS_VOID)0)
#define IP_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#if VOS_BYTE_ORDER == VOS_BIG_ENDIAN
#define VOS_NTOHL(x) (x)
#define VOS_HTONL(x) (x)
#define VOS_NTOHS(x) (x)
#define VOS_HTONS(x) (x)
#else
#define VOS_NTOHL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define VOS_HTONL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define VOS_NTOHS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#define VOS_HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#endif /* _BYTE_ORDER==_LITTLE_ENDIAN */

#define IPND_LOG(modulePID, subMod, level, prinString)
#define IPND_LOG1(modulePID, subMod, level, prinString, para1)
#define IPND_LOG2(modulePID, subMod, level, prinString, para1, para2)
#define IPND_LOG3(modulePID, subMod, level, prinString, para1, para2, para3)
#define IPND_LOG4(modulePID, subMod, level, prinString, para1, para2, para3, para4)

#define IPND_INFO_LOG(moduleId, prinString)
#define IPND_INFO_LOG1(moduleId, prinString, para1)
#define IPND_INFO_LOG2(moduleId, prinString, para1, para2)
#define IPND_INFO_LOG3(moduleId, prinString, para1, para2, para3)
#define IPND_INFO_LOG4(moduleId, prinString, para1, para2, para3, para4)

#define IPND_WARNING_LOG(moduleId, prinString)
#define IPND_WARNING_LOG1(moduleId, prinString, para1)
#define IPND_WARNING_LOG2(moduleId, prinString, para1, para2)
#define IPND_WARNING_LOG3(moduleId, prinString, para1, para2, para3)
#define IPND_WARNING_LOG4(moduleId, prinString, para1, para2, para3, para4)

#define IPND_ERROR_LOG(moduleId, prinString)
#define IPND_ERROR_LOG1(moduleId, prinString, para1)
#define IPND_ERROR_LOG2(moduleId, prinString, para1, para2)
#define IPND_ERROR_LOG3(moduleId, prinString, para1, para2, para3)
#define IPND_ERROR_LOG4(moduleId, prinString, para1, para2, para3, para4)

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON) && (VOS_OS_VER != VOS_WIN32)
#define IPCOM_PRINT_INFO(enFunction, enProcFlag) TLPS_PRINT2LAYER_INFO((enFunction), (enProcFlag))
#define IPCOM_PRINT_INFO1(enFunction, enProcFlag, ulPara1) \
    TLPS_PRINT2LAYER_INFO1((enFunction), (enProcFlag), (VOS_INT32)(ulPara1))
#define IPCOM_PRINT_INFO2(enFunction, enProcFlag, ulPara1, ulPara2) \
    TLPS_PRINT2LAYER_INFO2((enFunction), (enProcFlag), (VOS_INT32)(ulPara1), (VOS_INT32)(ulPara2))

#define IPCOM_PRINT_WARNING(enFunction, enProcFlag) TLPS_PRINT2LAYER_WARNING((enFunction), (enProcFlag))
#define IPCOM_PRINT_WARNING1(enFunction, enProcFlag, ulPara1) \
    TLPS_PRINT2LAYER_WARNING1((enFunction), (enProcFlag), (VOS_INT32)(ulPara1))
#define IPCOM_PRINT_WARNING2(enFunction, enProcFlag, ulPara1, ulPara2) \
    TLPS_PRINT2LAYER_WARNING2((enFunction), (enProcFlag), (VOS_INT32)(ulPara1), (VOS_INT32)(ulPara2))

#define IPCOM_PRINT_ERROR(enFunction, enProcFlag) TLPS_PRINT2LAYER_ERROR((enFunction), (enProcFlag))
#define IPCOM_PRINT_ERROR1(enFunction, enProcFlag, ulPara1) \
    TLPS_PRINT2LAYER_ERROR1((enFunction), (enProcFlag), (VOS_INT32)(ulPara1))
#define IPCOM_PRINT_ERROR2(enFunction, enProcFlag, ulPara1, ulPara2) \
    TLPS_PRINT2LAYER_ERROR2((enFunction), (enProcFlag), (VOS_INT32)(ulPara1), (VOS_INT32)(ulPara2))
#else
#define IPCOM_PRINT_INFO(enFunction, enProcFlag)
#define IPCOM_PRINT_INFO1(enFunction, enProcFlag, ulPara1)
#define IPCOM_PRINT_INFO2(enFunction, enProcFlag, ulPara1, ulPara2)

#define IPCOM_PRINT_WARNING(enFunction, enProcFlag)
#define IPCOM_PRINT_WARNING1(enFunction, enProcFlag, ulPara1)
#define IPCOM_PRINT_WARNING2(enFunction, enProcFlag, ulPara1, ulPara2)

#define IPCOM_PRINT_ERROR(enFunction, enProcFlag)
#define IPCOM_PRINT_ERROR1(enFunction, enProcFlag, ulPara1)
#define IPCOM_PRINT_ERROR2(enFunction, enProcFlag, ulPara1, ulPara2)
#endif


/*
 * 结构名: IP_Err
 * 结构说明: IP模块错误码
 */
enum IP_Err {
    IP_SUCC = 0x00000000,
    IP_FAIL,

    IP_MSG_DISCARD, /* 丢弃该消息，即该消息不被该当前状态处理 */
    IP_MSG_HANDLED, /* 该消息已被当前状态处理 */

    IP_ERR_BUTT
};
typedef VOS_UINT32 IP_ErrUint2;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: PS统一布尔类型枚举定义
 */
enum IP_Bool {
    IP_FALSE = 0,
    IP_TRUE  = 1,
    IP_BOOL_BUTT
};
typedef VOS_UINT8 IP_BoolUint8;

/*  ipv6 begin */
/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 扩展头类型枚举
 */
enum IP_IcmpV6Type {
    IP_ICMPV6_PACKET_TOO_BIG   = 2,
    IP_ICMPV6_TYPE_ECHOREQUEST = 128,
    IP_ICMPV6_TYPE_ECHOREPLY   = 129,
    IP_ICMPV6_TYPE_RS          = 133,
    IP_ICMPV6_TYPE_RA          = 134,
    IP_ICMPV6_TYPE_NS          = 135,
    IP_ICMPV6_TYPE_NA          = 136,
    IP_ICMPV6_TYPE_REDIRECT    = 137,

    IP_ICMPV6_TYPE_BUTT
};
typedef VOS_UINT32 IP_Icmpv6TypeUint32;

enum IP_Icmpv6OptType {
    IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR = 1,
    IP_ICMPV6_OPT_TGT_LINK_LAYER_ADDR = 2,
    IP_ICMPV6_OPT_PREFIX_INFO         = 3,
    IP_ICMPV6_OPT_REDIRECT_HEADER     = 4,
    IP_ICMPV6_OPT_MTU                 = 5,

    IP_ICMPV6_OPT_TYPE_BUTT
};
typedef VOS_UINT32 IP_Icmpv6OptTypeUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 扩展头类型枚举
 */
enum IP_ExtensionHeaderType {
    IP_EXTENSION_HEADER_TYPE_HOPBYHOP    = 0x0,
    IP_EXTENSION_HEADER_TYPE_ROUTING     = 0x2b,
    IP_EXTENSION_HEADER_TYPE_FRAGMENT    = 0x2c,
    IP_EXTENSION_HEADER_TYPE_ESP         = 0x32,
    IP_EXTENSION_HEADER_TYPE_AH          = 0x33,
    IP_EXTENSION_HEADER_TYPE_DESTINATION = 0x3c,

    IP_EXTENSION_HEADER_TYPE_BUTT
};
typedef VOS_UINT32 IP_ExtensionHeaderTypeUint32;


/*
 * 结构名: ND_IP_Ipv6Prefix
 * 协议表格:
 * ASN.1描述:
 * 结构说明: ND_IP_IPV6_PREFIX_STRU的数据结构
 */
typedef struct {
    VOS_UINT32 prefixLValue : 1; /* 1: 有效VALID; 0: INVALID */
    VOS_UINT32 prefixAValue : 1;
    VOS_UINT32 prefixLen : 8;
    VOS_UINT32 rsv : 22;

    VOS_UINT32 validLifeTime;
    VOS_UINT32 preferredLifeTime;
    VOS_UINT8  prefix[IP_IPV6_ADDR_LEN];
} ND_IP_Ipv6Prefix;

/*
 * 结构名: ND_IP_Ipv6DnsSer
 * 协议表格:
 * ASN.1描述:
 * 结构说明: NAS_IP_IPV6_DNS_SER_STRU的数据结构
 */
typedef struct {
    VOS_UINT8 dnsSerNum; /* DNS服务器个数 */
    VOS_UINT8 reserved[3]; /* 3:rsv */

    VOS_UINT8 priDnsServer[IP_IPV6_ADDR_LEN];
    VOS_UINT8 secDnsServer[IP_IPV6_ADDR_LEN];
} ND_IP_Ipv6DnsSer;

/*
 * 结构名: ND_IP_Ipv6SipSer
 * 协议表格:
 * ASN.1描述:
 * 结构说明: NAS_IP_IPV6_SIP_SER_STRU的数据结构
 */
typedef struct {
    VOS_UINT8 sipSerNum; /* SIP服务器个数 */
    VOS_UINT8 reserved[3]; /* 3:rsv */

    VOS_UINT8 priSipServer[IP_IPV6_ADDR_LEN];
    VOS_UINT8 secSipServer[IP_IPV6_ADDR_LEN];
} ND_IP_Ipv6SipSer;

/*
 * 结构名: ESM_IP_Ipv6NWPara
 * 协议表格:
 * ASN.1描述:
 * 结构说明: ESM_IP_IPV6_NW_PARA_STRU的数据结构
 */
typedef struct {
    VOS_UINT32 opMtu : 1;
    VOS_UINT32 rsv : 31;

    VOS_UINT8 curHopLimit;
    VOS_UINT8 reserved[3]; /* 3:rsv */

    VOS_UINT8 interfaceId[ND_IP_IPV6_IFID_LENGTH];

    VOS_UINT32             mtuValue;
    VOS_UINT32             prefixNum;
    ND_IP_Ipv6Prefix prefixList[ND_IP_MAX_PREFIX_NUM_IN_RA];

    ND_IP_Ipv6DnsSer dnsSer;
    ND_IP_Ipv6SipSer sipSer; /* SIP服务器 */
} ESM_IP_Ipv6NWPara;

typedef struct {
    VOS_UINT32 sendMsgSize; /* 消息的总长度 */
    VOS_UINT32 resv;        /* 为了同64位操作系统兼容，指针放在在8字节对齐位置 */
    VOS_UINT8 *sendMsg;    /* 消息的首地址 */
} IP_SndMsg;

typedef struct {
    VOS_UINT32 opSrcLinkLayerAddr : 1; /* 1: 有效VALID; 0: INVALID */
    VOS_UINT32 rsv : 31;

    VOS_UINT8 srcLinkLayerAddr[IP_MAC_ADDR_LEN];
    VOS_UINT8 reserve1[2]; /* 2:rsv */
} IP_ND_MsgRs;

typedef ND_IP_Ipv6Prefix IP_Ipv6PrefixSt;

typedef struct {
    VOS_UINT32 opSrcLinkLayerAddr : 1; /* 1: 有效VALID; 0: INVALID */
    VOS_UINT32 opMtu : 1;
    VOS_UINT32 rsv1 : 30;

    VOS_UINT32 curHopLimit : 8;
    VOS_UINT32 raMValue : 1;
    VOS_UINT32 raOValue : 1;
    VOS_UINT32 rsv2 : 6;
    VOS_UINT32 routerLifeTime : 16;
    VOS_UINT32 reachableTimer;
    VOS_UINT32 reTransTime;

    VOS_UINT8           srcLinkLayerAddr[IP_MAC_ADDR_LEN];
    VOS_UINT8           reserve1[2]; /* 2:rsv */
    VOS_UINT32          mtuValue;
    VOS_UINT32          prefixNum;
    IP_Ipv6PrefixSt prefixList[IP_MAX_PREFIX_NUM_IN_RA];
} IP_ND_MsgRa;

typedef struct {
    VOS_UINT32 opSrcLinkLayerAddr : 1; /* 1: 有效VALID; 0: INVALID */
    VOS_UINT32 rsv : 31;

    VOS_UINT8 targetAddr[IP_IPV6_ADDR_LEN];
    VOS_UINT8 srcLinkLayerAddr[IP_MAC_ADDR_LEN];
    VOS_UINT8 reserve1[2]; /* 2:rsv */
} IP_ND_MsgNs;

typedef struct {
    VOS_UINT32 opTargetLinkLayerAddr : 1; /* 1: 有效VALID; 0: INVALID */
    VOS_UINT32 rsv1 : 31;

    VOS_UINT32 naRValue : 1;
    VOS_UINT32 naSValue : 1;
    VOS_UINT32 naOValue : 1;
    VOS_UINT32 rsv2 : 29;
    VOS_UINT8  targetAddr[IP_IPV6_ADDR_LEN];
    VOS_UINT8  targetLinkLayerAddr[IP_MAC_ADDR_LEN];
    VOS_UINT8  reserve1[2]; /* 2:rsv */
} IP_ND_MsgNa;

typedef union {
    IP_ND_MsgRs rsInfo;
    IP_ND_MsgRa raInfo;
    IP_ND_MsgNs nsInfo;
    IP_ND_MsgNa naInfo;
} IP_ND_MsgStU;

typedef struct {
    VOS_UINT8 ndType;
    VOS_UINT8 code;
    VOS_UINT8 reserved1[2]; /* 2:rsv */

    VOS_UINT8 srcIp[IP_IPV6_ADDR_LEN];
    VOS_UINT8 desIp[IP_IPV6_ADDR_LEN];

    IP_ND_MsgStU ndMsgStru;
} IP_ND_Msg;

/* Moved  begin */
/*
 * 结构名: IPv6_PSEDUOHEADER_STRU
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV6伪首部
 */
typedef struct {
    VOS_UINT8  ipSrc[IP_IPV6_ADDR_LEN]; /* 源IP地址 */
    VOS_UINT8  ipDes[IP_IPV6_ADDR_LEN]; /* 目的IP地址 */
    VOS_UINT32 upperLength;              /* 整个UPPER-LAYER长度，不包括伪首部长度 */
    VOS_UINT8  zeroAll[3];                  /* 3:全部为0 */
    VOS_UINT8  nextHead;
} IP_Ipv6PseduoHeader;

/*
 * 结构名: NDIS_Ipv6Hdr
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV6地址
 */
typedef struct {
    union {
        VOS_UINT8  addr8bit[16]; /* 16:bit */
        VOS_UINT16 addr16bit[8]; /* 8:bit */
        VOS_UINT32 addr32bit[4]; /* 4:bit */
    } in6;
} NDIS_Ipv6Addr;

/*
 * 结构名: NDIS_Ipv6Hdr
 * 协议表格:
 * ASN.1描述:
 * 结构说明: IPV6首部
 */
typedef struct {
    VOS_UINT32         verClassFlow; /* 4 bit version, 8 bit class, 20 bit flow. */
    VOS_UINT16         payLoadlen;   /* Payload length. */
    VOS_UINT8          nextHeader;   /* Next Header. */
    VOS_UINT8          hLim;         /* Hop limit. */
    NDIS_Ipv6Addr srcAddr;          /* IPv6 source address. */
    NDIS_Ipv6Addr dstAddr;          /* IPv6 destination address. */
} NDIS_Ipv6Hdr;

/*
 * 结构名: IP_UdpHeadInfo
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP头部结构
 */
typedef struct {
    VOS_UINT16 srcPort; /* 源端口 */
    VOS_UINT16 dstPort; /* 目的端口 */
    VOS_UINT16 udpLen;     /* UDP包长度 */
    VOS_UINT16 udpCheckSum;   /* UDP校验和 */
} IP_UdpHeadInfo;

/*
 * 结构名: IP_UdpHeadInfo
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP头部结构
 */
typedef struct {
    VOS_UINT16 optionCode;                            /* OPTION_DNS_SERVERS:23 */
    VOS_UINT16 optionLen;                             /* Length of the list of DNS recursive name
                            servers in octets; must be a multiple of 16 */
    NDIS_Ipv6Addr ipv6DNS[IP_IPV6_MAX_DNS_NUM]; /* DNS-recursive-name-server (IPv6 address) */
} IP_Ipv6DhcpDnsOption;

/*
 * 结构名: IP_Ipv6PktDhcpOptHdr
 * 协议表格:
 * ASN.1描述:
 * 结构说明: UDP头部结构
 */
/* DHCP Options */
typedef struct {
    VOS_UINT16 dhcpOptCode;
    VOS_UINT16 dhcpOptLen;
    VOS_UINT8  dhcpOptData[4]; /* 4:data */
} IP_Ipv6PktDhcpOptHdr;

/* DHCP DUID-LL Options */
typedef struct {
    VOS_UINT16 dhcpDuidType;
    VOS_UINT16 dhcpDuidHWType;
    VOS_UINT8  linkLayerAddr[IP_IPV6_DHCP_OPT_LINKADDR_SIZE];
} IP_Ipv6PktDhcpDuidLLOpt;

typedef struct {
    VOS_UINT8 ndOptType;
    VOS_UINT8 ndOptLen;
    VOS_UINT8 ndOptData[1];
} NDIS_PktNdOptHdr;

/*
 * 7 Extern Global Variable
 */
extern VOS_UINT8 g_ndAllNodesMulticaseAddr[];
extern VOS_UINT8 g_ndAllRoutersMulticaseAddr[];
extern VOS_UINT8 g_ndAllNodesMacAddr[];
extern VOS_UINT8 g_ndAllRoutersMacAddr[];


/* Moved  begin */
IP_BoolUint8 IP_IsValidRAPacket(const VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset);

VOS_VOID   IP_BuildIPv6Header(VOS_UINT8 *ipv6, IP_Ipv6PseduoHeader *pseduoHeader,
                              const VOS_UINT8 *interfaceId, VOS_UINT32 upperLength);
VOS_VOID IP_ND_FormIPv6HeaderMsg(const ND_Ipv6AddrInfo srcIPAddr, const ND_Ipv6AddrInfo dstIPAddr,
                                 VOS_UINT32 pduLen, VOS_UINT8 *data, VOS_UINT8 type);

VOS_UINT16 IPv6_Checksum(const VOS_UINT8 *pseduoHeader, const VOS_UINT8 *data, VOS_UINT32 len);
IP_BoolUint8 IP_IsValidNdMsg(const VOS_UINT8 *ipMsg, VOS_UINT32 ipMsgLen, VOS_UINT32 *typeOffset);

IP_ErrUint2 IP_BuildIcmpv6Checksum(VOS_UINT8 *ipMsg, VOS_UINT32 typeOffset);
/* Moved  end */
VOS_UINT32         ND_CheckEpsIdValid(VOS_UINT8 exEpsId);
VOS_UINT16         TTF_NDIS_Ipv6_CalcCheckSum(VOS_UINT8 *data, VOS_UINT16 len,
                                              const NDIS_Ipv6Addr *ip6SrcAddr,
                                              const NDIS_Ipv6Addr *ip6DstAddr, VOS_UINT32 proto);
VOS_UINT32         TTF_NDIS_Ipv6GetDhcpOption(IP_Ipv6PktDhcpOptHdr *pFirstOpt, VOS_UINT16 totOptLen,
                                              VOS_UINT16 ndOptCode, IP_Ipv6PktDhcpOptHdr **pOpt,
                                              VOS_INT32 lNumber);
VOS_UINT32         TTF_NDIS_InputUDPHead(VOS_UINT8 *pBuf, VOS_UINT16 srcPort, VOS_UINT16 dstPort,
                                         VOS_UINT16 udpDataLen);


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

#endif /* end of ip_comm.h */
