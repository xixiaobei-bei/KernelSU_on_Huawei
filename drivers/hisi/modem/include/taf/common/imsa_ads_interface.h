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
#ifndef __IMSA_ADS_INTERFACE_H__
#define __IMSA_ADS_INTERFACE_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define IMSA_ADS_MSG_ID_HEADER 0
#define IMSA_ADS_MAX_IFACE_NUM 6
#define IMSA_ADS_MAX_PDU_SESSION_NUM 2

/*
 * 枚举说明: IMSA与RNIC的消息定义
 */
enum IMSA_ADS_MsgId {
    ID_IMSA_ADS_IFACE_CONFIG_IND = IMSA_ADS_MSG_ID_HEADER + 0x00,

    ID_IMSA_ADS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 IMSA_ADS_MsgIdUint32;

/*
 * 枚举说明: IFACE操作类型取值
 */
enum IMSA_ADS_IfaceOpType {
    IMSA_ADS_IFACE_OP_TYPE_DOWN = 0x00,
    IMSA_ADS_IFACE_OP_TYPE_UP   = 0x01,

    IMSA_ADS_IFACE_OP_TYPE_BUTT
};
typedef VOS_UINT8 IMSA_ADS_IfaceOpTypeUint8;

/*
 * 枚举说明: IP类型
 */
enum IMSA_ADS_IpType {
    IMSA_ADS_IP_TYPE_NON_IP = 0x00,
    IMSA_ADS_IP_TYPE_IPV4   = 0x01,
    IMSA_ADS_IP_TYPE_IPV6   = 0x02,
    IMSA_ADS_IP_TYPE_IPV4V6 = 0x03,

    IMSA_ADS_IP_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 IMSA_ADS_IpTypeUint8;

/*
 * 4 STRUCT&UNION定义
 */

/*
 * 结构说明: PDU Session信息
 */
typedef struct {
    VOS_UINT8            pduSessionId;
    IMSA_ADS_IpTypeUint8 ipType;
    VOS_UINT8            reserved[2];
} IMSA_ADS_PduSessionInfo;

/*
 * 结构说明: IMS的网卡信息
 */
typedef struct {
    VOS_UINT8                 rmNetId;
    IMSA_ADS_IfaceOpTypeUint8 opType;
    VOS_UINT16                modemId;
    VOS_UINT32                pduSessionNum;
    IMSA_ADS_PduSessionInfo   pduSession[IMSA_ADS_MAX_PDU_SESSION_NUM];
} IMSA_ADS_IfaceInfo;

/*
 * 结构说明: IMSA设置ADS网卡接口配置消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32         msgId;
    VOS_UINT32         ifaceNum;
    IMSA_ADS_IfaceInfo ifaceInfo[IMSA_ADS_MAX_IFACE_NUM];
} IMSA_ADS_IfaceConfigInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __IMSA_ADS_INTERFACE_H__ */
