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

#ifndef PPPC_PPPA_INTERFACE_H
#define PPPC_PPPA_INTERFACE_H

#include "pppa_pppc_at_type.h"
#include "v_typdef.h"
#include "v_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(4)

enum PPPC_PPPA_MSG_TYPE_ENUM {
    ID_PPPA_PPPC_CREATE_PPP_IND         = 0x0000,   /* PPPA_PPPC_CREATE_PPP_STRU */
    ID_PPPA_PPPC_RELEASE_PPP_IND        = 0x0001,   /* PPPC_PPPA_COMM_MSG_STRU */
    ID_PPPA_PPPC_NEGO_DATA_IND          = 0x0002,   /* PPPC_PPPA_NEGO_DATA_STRU */
    ID_PPPC_PPPA_NEGO_DATA_IND          = 0x0003,   /* PPPC_PPPA_NEGO_DATA_STRU */
    ID_PPPC_PPPA_PDP_ACTIVE_REQ         = 0x0004,   /* PPPC_PPPA_PDP_ACTIVE_REQ */
    ID_PPPA_PPPC_PDP_ACTIVE_RSP         = 0x0005,   /* PPPA_PPPC_PDP_ACTIVE_RSP */
    ID_PPPC_PPPA_PPP_RELEASE_IND        = 0x0006,   /* PPPC_PPPA_COMM_MSG_STRU */
    ID_PPPC_PPPA_HDLC_CFG_IND           = 0x0007,   /* PPPC_PPPA_HDLC_CFG_STRU */
    ID_PPPA_PPPC_IGNORE_ECHO            = 0x0008,   /* PPPC_PPPA_COMM_MSG_STRU */
    ID_PPPC_PPPA_MSG_TYPE_ENUM_BUTT
};
typedef VOS_UINT16 PPPC_PPPA_MSG_TYPE_ENUM_UINT16;

/* 消息中不携带任何信元才可以使用此结构 */
typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          resv;
} PPPC_PPPA_COMM_MSG_STRU;

typedef struct
{
    VOS_BOOL                            chapEnable;             /* 是否使能Chap鉴权 */
    VOS_BOOL                            papEnable;              /* 是否使能Pap鉴权 */
    VOS_BOOL                            winsEnable;             /* 是否支持NBNS */
    VOS_UINT16                          echoMaxLostCnt;         /* 发送LcpEchoRequest允许丢弃的最大个数 */
    VOS_UINT16                          mru;                    /* 默认MRU大小,Range:[296,1500]*/
}PPPC_PPPA_CONFIG_STRU;


typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          resv;
    PPPC_PPPA_CONFIG_STRU               config;     /* A核PPP配置 */
} PPPA_PPPC_CREATE_PPP_STRU;

enum PPPC_PPPA_PKT_TYPE_ENUM {
    PPP_ECHO_REQ            = 0x00,
    PPPC_PPPA_PKT_TYPE_BUTT
};
typedef VOS_UINT8 PPPC_PPPA_PKT_TYPE_ENUM_UINT8;

typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          proto;
    PPPC_PPPA_PKT_TYPE_ENUM_UINT8       pktType;    /* 报文类型 */
    VOS_UINT8                           resv;
    VOS_UINT16                          dataLen;
    VOS_UINT8                           resv1[2];
    VOS_UINT8                           data[0];                          
}PPPC_PPPA_NEGO_DATA_STRU;

typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          resv;
    PPPA_PDP_ACTIVE_CONFIG_STRU         config;     /* PDP激活配置信息 */
}PPPC_PPPA_PDP_ACTIVE_REQ;

typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          resv;
    PPPA_PDP_ACTIVE_RESULT_STRU         result;     /* PDP激活结果 */
}PPPA_PPPC_PDP_ACTIVE_RSP;

typedef struct {
    VOS_MSG_HEADER
    PPPC_PPPA_MSG_TYPE_ENUM_UINT16      msgId;
    VOS_UINT16                          resv;
    VOS_UINT32                          hisAcf:1,   /* 对端是否支持地址控制域压缩 */
                                        hisPcf:1,   /* 对端是否支持协议域压缩 */
                                        myAcf:1,    /* 本端是否支持地址控制域压缩 */
                                        myPcf:1,    /* 本端是否支持协议域压缩 */
                                        resv1:28;
    VOS_UINT32                          hisAccm;    /* 对端的同异步转换控制字 */
}PPPC_PPPA_HDLC_CFG_STRU;

typedef struct
{
    VOS_MSG_HEADER        /* 消息头 */        /* _H2ASN_Skip */
    VOS_UINT32  ulMsgname;
    VOS_UINT32  ulPppPhase;
    VOS_INT32   ulLcpState;
    VOS_INT32   ulIpcpState;
    VOS_UINT16  usPppId;
    VOS_UINT16  usDataLen;
} PPP_FRAME_MNTN_INFO_STRU;

/* PPP 协议包处理方向: 接收或发送*/
#define PPP_RECV_PROTO_PACKET_TYPE    4    /*PPP模块接收到协商包*/
#define PPP_SEND_PROTO_PACKET_TYPE    5     /*PPP模块发送协商包*/

#pragma pack(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif