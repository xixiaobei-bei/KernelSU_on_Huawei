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

#ifndef __ADS_INTRA_MSG_H__
#define __ADS_INTRA_MSG_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

/* ADS普通道消息起始值 */
#define ID_ADS_COMM_MSG_BASE_ID (0x1000)

/*
 * 枚举说明: 定义ADS内部消息ID
 */
enum ADS_IntraMsgId {
    /* 保留消息     [0x0000, 0x0FFF] */

    /* 普通消息     [0x1000, 0x10FF] */
    ID_ADS_CCPU_RESET_START_IND = ID_ADS_COMM_MSG_BASE_ID + 0x01,
    ID_ADS_CCPU_RESET_END_IND   = ID_ADS_COMM_MSG_BASE_ID + 0x02,

    ID_ADS_PACKET_ERROR_IND = ID_ADS_COMM_MSG_BASE_ID + 0x03,

    ID_ADS_DPL_CONFIG_IND = ID_ADS_COMM_MSG_BASE_ID + 0x04,
};
typedef VOS_UINT32 ADS_IntraMsgIdUint32;

/*
 * 4 STRUCT&UNION定义
 */

/*
 * 结构说明: CCPU复位后给ADS的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;
} ADS_CcpuResetInd;

/*
 * 结构说明: ID_ADS_PACKET_ERROR_IND 消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT16 modemId;
    VOS_UINT16 bearerMask;
} ADS_PacketErrorInd;


typedef struct {
    VOS_MSG_HEADER              /* 消息头 */ /* _H2ASN_Skip */
    ADS_IntraMsgIdUint32 msgId; /* 消息ID */ /* _H2ASN_Skip */
    VOS_BOOL             dplNeededFlg;
} ADS_DplConfigInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_INTRA_MSG_H__ */
