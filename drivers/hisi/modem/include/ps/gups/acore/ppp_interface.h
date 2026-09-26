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

#ifndef PPP_INTERFACE_H
#define PPP_INTERFACE_H

#include "pppa_ppps_at_type.h"
#include "product_config.h"
#include "v_typdef.h"
#include "PsTypeDef.h"
#if (OSA_CPU_ACPU == VOS_OSA_CPU) || (VOS_WIN32 == VOS_OS_VER) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#include "imm_interface.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 系统最多需要的PPP ID的数目 */
#define PPP_MAX_ID_NUM 1

#ifndef IPV6_ADDR_LEN
#define IPV6_ADDR_LEN 16 /* IPV6地址长度 */
#endif

enum PPP_AtCtrlOperType {
    PPP_AT_CTRL_REL_PPP_REQ     = 0, /* 释放IP类型PPP链路操作 */
    PPP_AT_CTRL_REL_PPP_RAW_REQ = 1, /* 释放PPP类型PPP链路操作 */
    PPP_AT_CTRL_HDLC_DISABLE    = 2, /* HDLC硬化模块去使能操作 */
    PPPA_CCORE_RESET_IND,

    PPP_AT_CTRL_BUTT
};
typedef VOS_UINT32 PPP_AtCtrlOperTypeUint32;

#pragma pack(4)

typedef VOS_UINT16 PPP_Id;

#define AT_PPP_RELEASE_IND_MSG 0x00 /* 释放PDP连接的请求 */
#define AT_PPP_PROTOCOL_REL_IND_MSG 0x02 /* PPP链路完成释放的指示 */

typedef struct {
    MSG_Header      msgHeader;
    VOS_UINT16      clientId;
    VOS_UINT8       rsv[2];
} AT_PPP_ReleaseInd;

/* 以下结构为C核TAF使用，PPPA不使用，待TAF整改后删除 */
typedef struct {
    MSG_Header      msgHeader;
    VOS_UINT16      pppId;
    VOS_UINT8       rsv[2];
} AT_PPP_ProtocolRelInd;

typedef struct {
    VOS_UINT16 papReqLen;   /* request长度: 24.008要求在[3,253]字节 */
    VOS_UINT8  rsv[2]; /* 对齐保留 */
    VOS_UINT8* papReq;       /* request */
} PPP_AuthPapContent;

typedef struct {
    VOS_UINT16 chapChallengeLen; /* challenge长度: 24.008要求在[3,253]字节 */
    VOS_UINT16 chapResponseLen;  /* response长度: 24.008要求在[3,253]字节 */
    VOS_UINT8* chapChallenge;     /* challenge */
    VOS_UINT8* chapResponse;      /* response */
} PPP_AuthChapContent;

typedef struct {
    PPP_AuthTypeUint8       authType;
    VOS_UINT8               rsv[3];

    union {
        PPP_AuthPapContent  papContent;
        PPP_AuthChapContent chapContent;
    } AuthContent;
} PPP_ReqAuthConfig;

typedef struct {
    VOS_UINT16 ipcpLen;     /* Ipcp帧长度 */
    VOS_UINT8  rsv[2]; /* 对齐保留 */
    VOS_UINT8* ipcp;   /* Ipcp帧 */
} PPP_ReqIpcpConfig;

typedef struct {
    PPP_ReqAuthConfig reqAuthCfg;
    PPP_ReqIpcpConfig reqIpcpCfg;
} PPP_ReqConfig;

#if (VOS_OSA_CPU == OSA_CPU_ACPU) || (VOS_OS_VER == VOS_WIN32) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
/*
 * 上层注册给PPP用于下行数据发送接口。
 * userData在PPP创建时携带，PPP发送数据时回填, PPP不关注具体取
 */
typedef VOS_UINT32 (*PPP_SendDataToModem)(VOS_ULONG userData, IMM_Zc *data);

/*对应PPPA_CREAT_PPP_REQ_STRU，用于消息方式交互*/
typedef struct {
    VOS_ULONG                    userData;   /*用户数据 */
    PPP_SendDataToModem          sender;     /* 发送函数 */
    PPP_IpCapabilityUint32       capability; /* UE IP能力 */
} PPPA_CreatPppReq;

/* PPP提供给AT的接口 */
extern VOS_UINT32 PPPA_CreatePppReq(PPP_Id* pppId, PPPA_CreatPppReq* creatPppReq);
extern VOS_UINT32 PPPA_CreateRawDataPppReq(PPP_Id* pppId, VOS_ULONG userData, PPP_SendDataToModem sender);

extern VOS_UINT32 PPPA_RcvConfigInfoInd(PPP_Id pppId, PPPA_PdpActiveResult* pppIndConfigInfo);
extern VOS_VOID   PPP_RcvNdData(VOS_UINT8 ifaceId, IMM_Zc* data);
extern VOS_VOID   PPP_UpdateWinsConfig(VOS_UINT8 wins);
extern VOS_VOID   PPP_SetRawDataByPassMode(VOS_UINT32 rawDataByPassMode);

/* IP方式下提供的上行数据接收接口 */
extern VOS_UINT32 PPP_PullPacketEvent(VOS_UINT16 pppId, IMM_Zc* immZc);

/* PPP方式下提供的上行数据接收接口 */
extern VOS_UINT32 PPP_PullRawDataEvent(VOS_UINT16 pppId, IMM_Zc* immZc);
#endif

extern VOS_UINT32 PPP_RcvAtCtrlOperEvent(VOS_UINT16 pppId, PPP_AtCtrlOperTypeUint32 ctrlOperType);

extern VOS_UINT32 PPP_RegDlDataCallback(PPP_Id pppId);

#pragma pack()

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

