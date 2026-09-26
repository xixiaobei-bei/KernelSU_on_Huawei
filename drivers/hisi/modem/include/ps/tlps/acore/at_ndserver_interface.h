/*
* Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef __AT_NDSERVER_INTERFACE_H__
#define __AT_NDSERVER_INTERFACE_H__

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

#define AT_NDSERVER_MSG_ID_HEADER           0x0000

#define AT_NDSERVER_IPV6_ADDR_LENGTH        16
#define AT_NDSERVER_MAX_PREFIX_NUM_IN_RA    6
#define AT_NDSERVER_IPV6_IFID_LENGTH        8

/*****************************************************************************
 枚举名称: AT_NDSERVE_MsgId
 枚举说明: AT和NDSERVE模块间的接口消息
*****************************************************************************/
enum AT_NDSERVER_MsgId
{
    ID_AT_NDSERVER_PPP_IPV6_CFG_IND        = AT_NDSERVER_MSG_ID_HEADER + 0x01,
    ID_AT_NDSERVER_PPP_IPV6_REL_IND        = AT_NDSERVER_MSG_ID_HEADER + 0x02,

    ID_AT_NDSERVER_MSG_ID_BUTT
};
typedef VOS_UINT32 AT_NDSERVER_MsgIdUint32;


/*****************************************************************************
 结构名称: AT_NDSERVER_Ipv6DnsSer
 结构说明: IPV6 DNS信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           serNum;           /*服务器个数*/
    VOS_UINT8                           rsv[3];

    VOS_UINT8                           priServer[AT_NDSERVER_IPV6_ADDR_LENGTH];
    VOS_UINT8                           secServer[AT_NDSERVER_IPV6_ADDR_LENGTH];
} AT_NDSERVER_Ipv6DnsSer;

/*****************************************************************************
 结构名称: AT_NDSERVER_Ipv6PcscfSer
 结构说明: IPV6 PCSCF信息结构
*****************************************************************************/
typedef AT_NDSERVER_Ipv6DnsSer AT_NDSERVER_Ipv6PcscfSer;

/*****************************************************************************
 结构名称: AT_NDSERVER_Ipv6Prefix
 结构说明: IPV6 前缀信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ipv6LValue      :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ipv6AValue      :1;
    VOS_UINT32                          ipv6PrefixLen   :8;
    VOS_UINT32                          rsv             :22;

    VOS_UINT32                          validLifeTime;
    VOS_UINT32                          preferredLifeTime;
    VOS_UINT8                           prefix[AT_NDSERVER_IPV6_ADDR_LENGTH];
} AT_NDSERVER_Ipv6Prefix;

/*****************************************************************************
 结构名称: AT_NDSERVER_Ipv6PdnInfo
 结构说明: IPV6 配置信息结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                      OpMtu              :1;
    VOS_UINT32                      rsv1               :31;

    VOS_UINT32                      curHopLimit        :8;
    VOS_UINT32                      ipv6MValue         :1;
    VOS_UINT32                      ipv6OValue         :1;
    VOS_UINT32                      rsv2               :22;

    VOS_UINT8                       interfaceId[AT_NDSERVER_IPV6_IFID_LENGTH];
    VOS_UINT32                      Mtu;
    VOS_UINT32                      prefixNum;
    AT_NDSERVER_Ipv6Prefix          prefixList[AT_NDSERVER_MAX_PREFIX_NUM_IN_RA];

    AT_NDSERVER_Ipv6DnsSer          dnsSer;
    AT_NDSERVER_Ipv6PcscfSer        pcscfSer;
} AT_NDSERVER_Ipv6PdnInfo;

/*****************************************************************************
 结构名称: AT_NDSERVER_PPPIpv6CfgInd
 结构说明: ID_AT_NDSERVER_PPP_IPV6_CFG_IND 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    VOS_UINT8                           ifaceId;
    VOS_UINT8                           modemId;
    VOS_UINT8                           rsv[2];
    AT_NDSERVER_Ipv6PdnInfo             ipv6Info;
} AT_NDSERVER_PPPIpv6CfgInd;

/*****************************************************************************
 结构名称: AT_NDSERVER_PPPIpv6RelInd
 结构说明: ID_AT_NDSERVER_PPP_IPV6_REL_IND 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          msgId;

    VOS_UINT8                           ifaceId;
    VOS_UINT8                           rsv[3];
} AT_NDSERVER_PPPIpv6RelInd;


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

#endif /* __AT_NDSERVER_INTERFACE_H__ */

