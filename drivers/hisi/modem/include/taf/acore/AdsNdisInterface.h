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

#ifndef __ADS_NDIS_INTERFACE_H__
#define __ADS_NDIS_INTERFACE_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "vos.h"
#include "ImmInterface.h"


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

#define ADS_NDIS_MSG_ID_HEADER          (0x0000)
#define ADS_NDIS_MSG_ID_HEADER_V2       (0x0100)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
 枚举名称: ADS_NDIS_IP_PACKET_TYPE_ENUM
 枚举说明: PACKET类型定义
*****************************************************************************/
enum ADS_NDIS_IP_PACKET_TYPE_ENUM
{
    ADS_NDIS_IP_PACKET_TYPE_DHCPV4      = 0x00,
    ADS_NDIS_IP_PACKET_TYPE_DHCPV6      = 0x01,
    ADS_NDIS_IP_PACKET_TYPE_ICMPV6      = 0x02,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FE80   = 0x03,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FF     = 0x04,

    ADS_NDIS_IP_PACKET_TYPE_BUTT
};
typedef VOS_UINT8 ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8;

/*****************************************************************************
 枚举名称: ADS_NDIS_MSG_ID_ENUM
 枚举说明: ADS和NDIS的之间的消息
*****************************************************************************/
enum ADS_NDIS_MSG_ID_ENUM
{
    ID_ADS_NDIS_DATA_IND               = ADS_NDIS_MSG_ID_HEADER + 0x00,
    ID_ADS_NDIS_DATA_IND_V2            = ADS_NDIS_MSG_ID_HEADER_V2 + 0x00,

    ID_ADS_NDIS_MSG_ID_BUTT
};
typedef VOS_UINT32 ADS_NDIS_MSG_ID_ENUM_UINT32;


/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/

/*****************************************************************************
 结构名称: ADS_NDIS_DATA_IND_STRU
 结构说明: ID_ADS_NDIS_DATA_IND 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;
    ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8  enIpPacketType;
    VOS_UINT8                           aucReserved[4];
    IMM_ZC_STRU                        *pstData;
} ADS_NDIS_DATA_IND_STRU;

/*****************************************************************************
 结构名称: ADS_NDIS_DATA_IND_V2_STRU
 结构说明: ID_ADS_NDIS_DATA_IND_V2 消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT8                           ucIfaceId;
    ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8  enIpPacketType;
    VOS_UINT8                           aucReserved[6];
    IMM_ZC_STRU                        *pstData;
} ADS_NDIS_DATA_IND_V2_STRU;



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

#endif /* __ADS_NDIS_INTERFACE_H__ */

