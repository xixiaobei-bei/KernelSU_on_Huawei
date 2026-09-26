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

#ifndef PPPS_PPPA_INTERFACE_H
#define PPPS_PPPA_INTERFACE_H

#include "pppa_ppps_at_type.h"
#include "v_typdef.h"
#include "v_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push, 4)

enum PPPS_PPPA_MsgType {
    ID_PPPA_PPPS_CREATE_PPP_IND  = 0x0000, /* PPPA_PPPS_CREATE_PPP_STRU */
    ID_PPPA_PPPS_RELEASE_PPP_IND = 0x0001, /* PPPS_PPPA_COMM_MSG_STRU */
    ID_PPPA_PPPS_NEGO_DATA_IND   = 0x0002, /* PPPS_PPPA_NEGO_DATA_STRU */
    ID_PPPS_PPPA_NEGO_DATA_IND   = 0x0003, /* PPPS_PPPA_NEGO_DATA_STRU */
    ID_PPPS_PPPA_PDP_ACTIVE_REQ  = 0x0004, /* PPPS_PPPA_PDP_ACTIVE_REQ */
    ID_PPPA_PPPS_PDP_ACTIVE_RSP  = 0x0005, /* PPPA_PPPS_PDP_ACTIVE_RSP */
    ID_PPPS_PPPA_PPP_RELEASE_IND = 0x0006, /* PPPS_PPPA_COMM_MSG_STRU */
    ID_PPPS_PPPA_HDLC_CFG_IND    = 0x0007, /* PPPS_PPPA_HDLC_CFG_STRU */
    ID_PPPA_PPPS_IGNORE_ECHO     = 0x0008, /* PPPS_PPPA_COMM_MSG_STRU */
    ID_PPPS_PPPA_MSG_TYPE_ENUM_BUTT
};
typedef VOS_UINT16 PPPS_PPPA_MsgTypeUint16;

/* 消息中不携带任何信元才可以使用此结构 */
typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              resv;
} PPPS_PPPA_CommMsg;

typedef struct {
    VOS_BOOL   winsEnable;     /* 是否支持NBNS */
    PPP_IpCapabilityUint32 ipCapability;
} PPPS_PPPA_Config;

typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              resv;
    PPPS_PPPA_Config        config; /* A核PPP配置 */
} PPPA_PPPS_CreatePpp;

enum PPPS_PPPA_PktType {
    PPP_ECHO_REQ = 0x00,
    PPPS_PPPA_PKT_TYPE_BUTT
};
typedef VOS_UINT8 PPPS_PPPA_PktTypeUint8; 

typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              proto;
    PPPS_PPPA_PktTypeUint8  pktType; /* 报文类型 */
    VOS_UINT8               pktPri; /* 报文优先级：解决UART承载PPP拨号超规格灌包时，ECHO报文交互异常导致断链 */
    VOS_UINT16              dataLen;
    VOS_UINT8               data[0];
} PPPS_PPPA_NegoData;

typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              resv;
    PPPA_PdpActiveConfig    config; /* PDP激活配置信息 */
} PPPS_PPPA_PdpActiveReq;

typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              resv;
    PPPA_PdpActiveResult    result; /* PDP激活结果 */
} PPPA_PPPS_PdpActiveRsp;

typedef struct {
    VOS_MSG_HEADER
    PPPS_PPPA_MsgTypeUint16 msgId;
    VOS_UINT16              resv;
    VOS_UINT32              hisAcf : 1, /* 对端是否支持地址控制域压缩 */
                            hisPcf : 1, /* 对端是否支持协议域压缩 */
                            myAcf : 1,  /* 本端是否支持地址控制域压缩 */
                            myPcf : 1,  /* 本端是否支持协议域压缩 */
                            resv1 : 28;
    VOS_UINT32 hisAccm; /* 对端的同异步转换控制字 */
} PPPS_PPPA_HdlcCfg;

typedef struct {
    VOS_MSG_HEADER /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32 msgName;
    VOS_UINT32     pppPhase;
    VOS_INT32      lcpState;
    VOS_INT32      ipcpState;
    VOS_UINT16     pppId;
    VOS_UINT16     dataLen;
} PPP_FrameMntn;

/* PPP 协议包处理方向: 接收或发送 */
#define PPP_RECV_PROTO_PACKET_TYPE 4 /* PPP模块接收到协商包 */
#define PPP_SEND_PROTO_PACKET_TYPE 5 /* PPP模块发送协商包 */

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif