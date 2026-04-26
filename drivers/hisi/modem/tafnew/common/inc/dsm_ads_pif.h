/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __DSM_ADS_PIF_H__
#define __DSM_ADS_PIF_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define DSM_ADS_MSG_ID_HEADER 0x0000
#define DSM_ADS_MAX_PDU_SESSION_NUM 1

/*
 * 枚举说明: DSM与ADS的消息定义
 */
enum DSM_ADS_MsgId {
    ID_DSM_ADS_IFACE_CONFIG_IND         = DSM_ADS_MSG_ID_HEADER + 0x01,
    ID_DSM_ADS_ESPE_TRANSFER_CONFIG_IND = DSM_ADS_MSG_ID_HEADER + 0x02,

    ID_DSM_ADS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 DSM_ADS_MsgIdUint32;

/*
 * 枚举说明: IFACE操作类型取值
 */
enum DSM_ADS_IfaceOpType {
    DSM_ADS_IFACE_OP_TYPE_DOWN = 0x00,
    DSM_ADS_IFACE_OP_TYPE_UP   = 0x01,

    DSM_ADS_IFACE_OP_TYPE_BUTT
};
typedef VOS_UINT8 DSM_ADS_IfaceOpTypeUint8;

/*
 * 枚举说明: session类型
 */
enum DSM_ADS_SessionType {
    DSM_ADS_SESSION_TYPE_RAW      = 0x00,       /* 透传或者直传类型 */
    DSM_ADS_SESSION_TYPE_IPV4     = 0x01,
    DSM_ADS_SESSION_TYPE_IPV6     = 0x02,
    DSM_ADS_SESSION_TYPE_IPV4V6   = 0x03,
    DSM_ADS_SESSION_TYPE_ETHERNET = 0x04,

    DSM_ADS_SESSION_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 DSM_ADS_SessionTypeUint8;

/*
 * 枚举说明: ESPE快转控制类型
 */
enum DSM_ADS_EspeTransferOpType {
    DSM_ADS_ESPE_TRANSFER_OP_TYPE_DISABLE  = 0x00,
    DSM_ADS_ESPE_TRANSFER_OP_TYPE_ENABLE   = 0x01,

    DSM_ADS_ESPE_TRANSFER_OP_TYPE_BUTT
};
typedef VOS_UINT8 DSM_ADS_EspeTransferOpTypeUint8;

/*
 * 结构说明: PDU Session信息
 */
typedef struct {
    VOS_UINT8                pduSessionId;
    DSM_ADS_SessionTypeUint8 sessType;
    VOS_UINT8                reserved[2];
} DSM_ADS_PduSessionInfo;

/*
 * 结构说明: IFACE信息
 */
typedef struct {
    VOS_UINT8                ifaceId;
    VOS_UINT8                sharedIfaceId;
    DSM_ADS_IfaceOpTypeUint8 opType;
    VOS_UINT8                rsv1;
    VOS_UINT16               modemId;
    VOS_UINT16               rsv2;
    DSM_ADS_PduSessionInfo   pduSessionInfo[DSM_ADS_MAX_PDU_SESSION_NUM];
} DSM_ADS_IfaceInfo;

/*
 * 结构说明: DSM设置ADS网卡接口配置消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32        msgId;
    DSM_ADS_IfaceInfo ifaceInfo;
} DSM_ADS_IfaceConfigInd;

/*
 * 结构说明: DSM通知ADS配置ESPE快转特性
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                      msgId;

    DSM_ADS_EspeTransferOpTypeUint8 opType;
    VOS_UINT8                       rsv[3];
} DSM_ADS_EspeTransferConfigInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __DSM_ADS_PIF_H__ */
