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

#ifndef __AT_NDIS_INTERFACE_H__
#define __AT_NDIS_INTERFACE_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "vos.h"
#include "mdrv.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define AT_NDIS_MSG_ID_HEADER           (0x0000)
#define NDIS_AT_MSG_ID_HEADER           (0x0020)

#define AT_NDIS_MSG_ID_HEADER_V2        (0x0100)
#define NDIS_AT_MSG_ID_HEADER_V2        (0x0120)

#ifndef IPV4_ADDR_LEN
#define IPV4_ADDR_LEN                   (4)                 /*IPV4地址长度*/
#endif

#ifndef	IPV6_ADDR_LEN
#define IPV6_ADDR_LEN                   (16)                /*IPV6地址长度*/
#endif

#define MIN_VAL_EPSID                   (5)
#define MAX_VAL_EPSID                   (15)

#define NDIS_INVALID_HANDLE             (0x7FFFFFFF)
#define NDIS_INVALID_RABID              (0x7F)
#define NDIS_INVALID_SPEPORT            (0)

#define AT_NDIS_IPV4_ADDR_LENGTH        (4)
#define AT_NDIS_IPV6_ADDR_LENGTH        (16)
#define AT_NDIS_MAX_PREFIX_NUM_IN_RA    (6)
#define AT_NDIS_IPV6_IFID_LENGTH        (8)


/*****************************************************************************
 枚举名称: AT_NDIS_MsgId
 枚举说明: AT和NDIS模块间的接口消息
*****************************************************************************/
enum AT_NDIS_MsgId
{
    ID_AT_NDIS_PDNINFO_CFG_REQ          = AT_NDIS_MSG_ID_HEADER + 0x01,
    ID_AT_NDIS_PDNINFO_CFG_CNF          = NDIS_AT_MSG_ID_HEADER + 0x01,

    ID_AT_NDIS_PDNINFO_REL_REQ          = AT_NDIS_MSG_ID_HEADER + 0x02,
    ID_AT_NDIS_PDNINFO_REL_CNF          = NDIS_AT_MSG_ID_HEADER + 0x02,

    ID_AT_NDIS_IFACE_UP_CONFIG_IND      = AT_NDIS_MSG_ID_HEADER_V2 + 0x01,
    ID_AT_NDIS_IFACE_DOWN_CONFIG_IND    = AT_NDIS_MSG_ID_HEADER_V2 + 0x02,

    ID_AT_NDIS_MSG_ID_BUTT
};
typedef VOS_UINT32 AT_NDIS_MsgIdUint32;

/*****************************************************************************
 枚举名称: AT_NDIS_Rslt
 枚举说明: 配置的结果:成功还是失败
*****************************************************************************/
enum AT_NDIS_Rslt
{
    AT_NDIS_SUCC                        = 0,
    AT_NDIS_FAIL                        = 1,

    AT_NDIS_BUTT
};
typedef VOS_UINT8 AT_NDIS_RsltUint8;

/*****************************************************************************
 枚举名称: AT_NDIS_PdnCfgCnf
 枚举说明: NDIS向AT返回PDN配置结果类型
*****************************************************************************/
enum AT_NDIS_PdnCfgCnf
{
    AT_NDIS_PDNCFG_CNF_SUCC             = 0,                /*配置成功*/
    AT_NDIS_PDNCFG_CNF_FAIL             = 1,                /*配置失败*/
    AT_NDIS_PDNCFG_CNF_IPV4ONLY_SUCC    = 2,                /*IPV4和IPV6参数同时有效时，只有IPV4参数配置成功*/
    AT_NDIS_PDNCFG_CNF_IPV6ONLY_SUCC    = 3,                /*IPV4和IPV6参数同时有效时，只有IPV6参数配置成功*/

    AT_NDIS_PDNCFG_CNF_BUUT
};
typedef VOS_UINT8 AT_NDIS_PdnCfgCnfUint8;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv4Addr
 结构说明: IPV4地址结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ipv4AddrU8[AT_NDIS_IPV4_ADDR_LENGTH];
} AT_NDIS_Ipv4Addr;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv4PdnInfo
 结构说明: IPV4 配置信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          opPdnAddr         :1;
    VOS_UINT32                          opDnsPrim         :1;
    VOS_UINT32                          opDnsSec          :1;
    VOS_UINT32                          opWinsPrim        :1;
    VOS_UINT32                          opWinsSec         :1;
    VOS_UINT32                          opPcscfPrim       :1;
    VOS_UINT32                          opPcscfSec        :1;
    VOS_UINT32                          opSpare           :25;

    AT_NDIS_Ipv4Addr              PdnAddrInfo;
    AT_NDIS_Ipv4Addr              subnetMask;         /*子网掩码*/
    AT_NDIS_Ipv4Addr              gateWayAddrInfo;    /*网关*/
    AT_NDIS_Ipv4Addr              dnsPrimAddrInfo;    /*主DNS信息 */
    AT_NDIS_Ipv4Addr              dnsSecAddrInfo;     /*辅DNS信息 */
    AT_NDIS_Ipv4Addr              winsPrimAddrInfo;   /*主DNS信息 */
    AT_NDIS_Ipv4Addr              winsSecAddrInfo;    /*辅DNS信息 */
    AT_NDIS_Ipv4Addr              pcscfPrimAddrInfo;  /*主PCSCF信息 */
    AT_NDIS_Ipv4Addr              pcscfSecAddrInfo;   /*辅PCSCF信息 */
} AT_NDIS_Ipv4PdnInfo;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv6DnsSer
 结构说明: IPV6 DNS信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           serNum;           /*服务器个数*/
    VOS_UINT8                           rsv[3];

    VOS_UINT8                           priServer[AT_NDIS_IPV6_ADDR_LENGTH];
    VOS_UINT8                           secServer[AT_NDIS_IPV6_ADDR_LENGTH];
} AT_NDIS_Ipv6DnsSer;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv6PcscfSer
 结构说明: IPV6 PCSCF信息结构
*****************************************************************************/
typedef AT_NDIS_Ipv6DnsSer AT_NDIS_Ipv6PcscfSer;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv6Prefix
 结构说明: IPV6 前缀信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ipv6LValue          :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ipv6AValue          :1;
    VOS_UINT32                          ipv6PrefixLen  :8;
    VOS_UINT32                          rsv        :22;

    VOS_UINT32                          validLifeTime;
    VOS_UINT32                          preferredLifeTime;
    VOS_UINT8                           prefixList[AT_NDIS_IPV6_ADDR_LENGTH];
} AT_NDIS_Ipv6Prefix;

/*****************************************************************************
 结构名称: AT_NDIS_Ipv6PdnInfo
 结构说明: IPV6 配置信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          OpMtu              :1;
    VOS_UINT32                          rsv1               :31;

    VOS_UINT32                          curHopLimit        :8;
    VOS_UINT32                          ipv6MValue                  :1;
    VOS_UINT32                          ipv6OValue                  :1;
    VOS_UINT32                          rsv2               :22;

    VOS_UINT8                           interfaceId[AT_NDIS_IPV6_IFID_LENGTH];
    VOS_UINT32                          Mtu;
    VOS_UINT32                          prefixNum;
    AT_NDIS_Ipv6Prefix            prefixList[AT_NDIS_MAX_PREFIX_NUM_IN_RA];

    AT_NDIS_Ipv6DnsSer           dnsSer;
    AT_NDIS_Ipv6PcscfSer         pcscfSer;
} AT_NDIS_Ipv6PdnInfo;

/*****************************************************************************
 结构名称: AT_NDIS_PdnInfoCfgReq
 结构说明: ID_AT_NDIS_PDNINFO_CFG_REQ 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    VOS_UINT32                          opIpv4PdnInfo : 1;
    VOS_UINT32                          opIpv6PdnInfo : 1;
    VOS_UINT32                          opEthPdnInfo  : 1;
    VOS_UINT32                          opSpare       : 29;

    MODEM_ID_ENUM_UINT16                modemId;
    VOS_UINT8                           rabId;            /* 承载号 */
    VOS_UINT8                           rsv[1];          /* 消息结构体对齐*/

    VOS_INT32                          handle;           /* 设备ID，标识网卡 */

    AT_NDIS_Ipv4PdnInfo          ipv4PdnInfo;
    AT_NDIS_Ipv6PdnInfo          ipv6PdnInfo;

    VOS_INT32                           iSpePort;           /*SPE端口*/
    VOS_UINT32                          ipfFlag;
} AT_NDIS_PdnInfoCfgReq;

/*****************************************************************************
 结构名称: AT_NDIS_PdnInfoCfgCnf
 结构说明: ID_AT_NDIS_PDNINFO_CFG_CNF 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    MODEM_ID_ENUM_UINT16                modemId;
    VOS_UINT8                           rabId;
    AT_NDIS_PdnCfgCnfUint8            result;
    VOS_UINT8                           rabType;
    VOS_UINT8                           rsv[3];
} AT_NDIS_PdnInfoCfgCnf;

/*****************************************************************************
 结构名称: AT_NDIS_PdnInfoRelReq
 结构说明: ID_AT_NDIS_PDNINFO_REL_REQ 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    MODEM_ID_ENUM_UINT16                modemId;
    VOS_UINT8                           rabId;          /*范围5-15*/
    VOS_UINT8                           rsv[1];
} AT_NDIS_PdnInfoRelReq;

/*****************************************************************************
 结构名称: AT_NDIS_PdnInfoRelReq
 结构说明: ID_AT_NDIS_PDNINFO_REL_CNF 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    MODEM_ID_ENUM_UINT16                modemId;
    VOS_UINT8                           rabId;      /*范围5-15*/
    AT_NDIS_RsltUint8             result;
    VOS_UINT8                           rabType;
    VOS_UINT8                           rsv[3];
} AT_NDIS_PdnInfoRelCnf;

/*****************************************************************************
 结构名称: AT_NDIS_IfaceInfo
 结构说明: IFACE详细信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ifaceId;
    VOS_UINT8                           pduSessionId;
    VOS_UINT8                           fcHead;
    VOS_UINT8                           modemId;
}AT_NDIS_IfaceInfo;

/*****************************************************************************
 结构名称: AT_NDIS_IfaceUpConfigInd
 结构说明: ID_AT_NDIS_IFACE_UP_CONFIG_IND 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    VOS_UINT32                          opIpv4PdnInfo : 1;
    VOS_UINT32                          opIpv6PdnInfo : 1;
    VOS_UINT32                          opEthPdnInfo  : 1;
    VOS_UINT32                          opSpare       : 29;

    VOS_UINT8                           iFaceId;
    VOS_UINT8                           rsv[3];

    VOS_INT32                          handle;

    AT_NDIS_Ipv4PdnInfo          ipv4PdnInfo;
    AT_NDIS_Ipv6PdnInfo          ipv6PdnInfo;

    AT_NDIS_IfaceInfo             ipv4IfaceInfo;
    AT_NDIS_IfaceInfo             ipv6IfaceInfo;
} AT_NDIS_IfaceUpConfigInd;

/*****************************************************************************
 结构名称: AT_NDIS_IfaceDownConfigInd
 结构说明: ID_AT_NDIS_IFACE_DOWN_CONFIG_IND 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    VOS_UINT32                          opIpv4PdnInfo : 1;
    VOS_UINT32                          opIpv6PdnInfo : 1;
    VOS_UINT32                          opEthPdnInfo  : 1;
    VOS_UINT32                          opSpare       : 29;

    VOS_UINT8                           iFaceId;
    VOS_UINT8                           rsv[3];
} AT_NDIS_IfaceDownConfigInd;



#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __AT_NDIS_INTERFACE_H__ */

