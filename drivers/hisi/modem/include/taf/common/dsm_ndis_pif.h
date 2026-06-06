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

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include "vos.h"
#include "TafPsTypeDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 暂时先通过IFACE BASE进行定义，等之后AT整体下移后，再改BASE和脱敏 */
#define DSM_NDIS_MSG_ID_HEADER          (TAF_PS_IFACE_ID_BASE + 0x00F0)

#define DSM_NDIS_IPV4_ADDR_LENGTH       (4)
#define DSM_NDIS_IPV6_ADDR_LENGTH       (16)
#define DSM_NDIS_MAX_PREFIX_NUM_IN_RA   (6)
#define DSM_NDIS_IPV6_IFID_LENGTH       (8)

/*******************************************************************************
  3 枚举定义
*******************************************************************************/

/*****************************************************************************
 枚举名称: DMS_NDIS_MSG_ID_ENUM
 枚举说明: DSM与NDIS的消息定义
*****************************************************************************/
enum DMS_NDIS_MSG_ID_ENUM
{
    ID_DSM_NDIS_IFACE_UP_IND                = DSM_NDIS_MSG_ID_HEADER + 0x01,
    ID_DSM_NDIS_IFACE_DOWN_IND              = DSM_NDIS_MSG_ID_HEADER + 0x02,
    ID_DSM_NDIS_CONFIG_IPV6_DNS_IND         = DSM_NDIS_MSG_ID_HEADER + 0x03,

    ID_DSM_NDIS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 DSM_NDIS_MSG_ID_ENUM_UINT32;


/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_UINT8                           aucIpV4Addr[DSM_NDIS_IPV4_ADDR_LENGTH];
} DSM_NDIS_IPV4_ADDR_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPdnAddr         :1;
    VOS_UINT32                          bitOpDnsPrim         :1;
    VOS_UINT32                          bitOpDnsSec          :1;
    VOS_UINT32                          bitOpPcscfPrim       :1;
    VOS_UINT32                          bitOpPcscfSec        :1;
    VOS_UINT32                          bitOpSpare           :27;

    DSM_NDIS_IPV4_ADDR_STRU             stPDNAddrInfo;
    DSM_NDIS_IPV4_ADDR_STRU             stSubnetMask;                           /*子网掩码*/
    DSM_NDIS_IPV4_ADDR_STRU             stGateWayAddrInfo;                      /*网关*/
    DSM_NDIS_IPV4_ADDR_STRU             stDnsPrimAddrInfo;                      /*主DNS信息 */
    DSM_NDIS_IPV4_ADDR_STRU             stDnsSecAddrInfo;                       /*辅DNS信息 */
    DSM_NDIS_IPV4_ADDR_STRU             stPcscfPrimAddrInfo;                    /*主PCSCF信息 */
    DSM_NDIS_IPV4_ADDR_STRU             stPcscfSecAddrInfo;                     /*辅PCSCF信息 */
} DSM_NDIS_IPV4_PDN_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucSerNum;           /*服务器个数*/
    VOS_UINT8                           aucRsv[3];

    VOS_UINT8                           aucPriServer[DSM_NDIS_IPV6_ADDR_LENGTH];
    VOS_UINT8                           aucSecServer[DSM_NDIS_IPV6_ADDR_LENGTH];
} DSM_NDIS_IPV6_DNS_SER_STRU;


typedef DSM_NDIS_IPV6_DNS_SER_STRU DSM_NDIS_IPV6_PCSCF_SER_STRU;


typedef struct
{
    VOS_UINT32                          ulBitL          :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ulBitA          :1;
    VOS_UINT32                          ulBitPrefixLen  :8;
    VOS_UINT32                          ulBitRsv        :22;

    VOS_UINT32                          ulValidLifeTime;
    VOS_UINT32                          ulPreferredLifeTime;
    VOS_UINT8                           aucPrefix[DSM_NDIS_IPV6_ADDR_LENGTH];
} DSM_NDIS_IPV6_PREFIX_STRU;


typedef struct
{
    VOS_UINT32                          ulBitOpMtu              :1;
    VOS_UINT32                          ulBitRsv1               :31;

    VOS_UINT32                          ulBitCurHopLimit        :8;
    VOS_UINT32                          ulBitM                  :1;
    VOS_UINT32                          ulBitO                  :1;
    VOS_UINT32                          ulBitRsv2               :22;

    VOS_UINT8                           aucInterfaceId[DSM_NDIS_IPV6_IFID_LENGTH];
    VOS_UINT32                          ulMtu;
    VOS_UINT32                          ulPrefixNum;
    DSM_NDIS_IPV6_PREFIX_STRU           astPrefixList[DSM_NDIS_MAX_PREFIX_NUM_IN_RA];

    DSM_NDIS_IPV6_DNS_SER_STRU          stDnsSer;
    DSM_NDIS_IPV6_PCSCF_SER_STRU        stPcscfSer;
} DSM_NDIS_IPV6_PDN_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT32                          bitOpIpv4PdnInfo : 1;
    VOS_UINT32                          bitOpIpv6PdnInfo : 1;
    VOS_UINT32                          bitOpSpare       : 30;

    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;            /* 承载号 */
    VOS_UINT8                           ucIfaceId;

    DSM_NDIS_IPV4_PDN_INFO_STRU         stIpv4PdnInfo;
    DSM_NDIS_IPV6_PDN_INFO_STRU         stIpv6PdnInfo;
} DSM_NDIS_IFACE_UP_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT32                          bitOpIpv4PdnInfo : 1;
    VOS_UINT32                          bitOpIpv6PdnInfo : 1;
    VOS_UINT32                          bitOpSpare       : 30;

    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;            /* 承载号 */
    VOS_UINT8                           ucIfaceId;
} DSM_NDIS_IFACE_DOWN_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT32                          bitOpIpv6PriDns   : 1;
    VOS_UINT32                          bitOpIpv6SecDns   : 1;
    VOS_UINT32                          bitOpIpv6Spare    : 30;

    VOS_UINT8                           aucIpv6PrimDns[TAF_IPV6_ADDR_LEN];      /* 从 PDP上下文带来的IPV6主DNS长度，不包括":" */
    VOS_UINT8                           aucIpv6SecDns[TAF_IPV6_ADDR_LEN];       /* 从 PDP上下文带来的IPV6副DNS长度，不包括":" */
} DSM_NDIS_CONFIG_IPV6_DNS_IND_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/



#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __DSM_NDIS_PIF_H__ */

