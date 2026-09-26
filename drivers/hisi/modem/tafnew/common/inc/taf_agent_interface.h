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

#ifndef _TAFAGENTAPSINTERFACE_H_
#define _TAFAGENTAPSINTERFACE_H_

#include "vos.h"
#include "taf_type_def.h"
#include "at_mn_interface.h"
#include "taf_ps_api.h"
#include "mn_call_api.h"
#include "taf_app_mma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAFAGENT_MTA_WRITE_ACORE_NV_REQ_DATA_LEN 4
#define TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF_DATA_ARRAY_LEN 4
#define TAF_AGENT_MSG_DATA_MSG_BLOCK_LEN 4



enum TAFAGENT_MsgId {
    /* TAFAGENT->DSM */
    /* TAFAGENT->DSM 获取CID的参数请求消息 */
    /* _MSGPARSE_Interception TAFAGENT_PS_GetCidParaReq */
    ID_TAFAGENT_PS_GET_CID_PARA_REQ = 0x1002,
    /* TAFAGENT->MTA */
    /* _MSGPARSE_Interception TAFAGENT_MTA_GetSysmodeReq */
    ID_TAFAGENT_MTA_GET_SYSMODE_REQ = 0x1005,
    /* TAFAGENT->TAF */
    /* _MSGPARSE_Interception TAFAGENT_MTA_GetAntStateReq */
    ID_TAFAGENT_MTA_GET_ANT_STATE_REQ = 0x1006,
    /* TAFAGENT->MTA */
    /* _MSGPARSE_Interception TAFAGENT_MTA_WriteAcoreNvReq */
    ID_TAFAGENT_MTA_WRITE_ACORE_NV_REQ = 0x100C,

    /* _MSGPARSE_Interception TAFAGENT_MTA_ProcAcoreNvReq */
    ID_TAFAGENT_MTA_PROC_ACORE_NV_REQ            = 0x100D,
    /* _MSGPARSE_Interception TAFAGENT_MTA_ReadAcoreEncryptionNvReq */
    ID_TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_REQ = 0x100E,
    /* DSM->TAFAGENT 获取CID的参数回复消息 */
    /* DSM->TAFAGENT */
    /* _MSGPARSE_Interception TAFAGENT_PS_GetCidParaCnf */
    ID_TAFAGENT_PS_GET_CID_PARA_CNF = 0x2002,

    /* TAFAGENT->MTA  获取系统模式信息回复消息 */
    /* MTA->TAFAGENT */
    /* _MSGPARSE_Interception TAFAGENT_MTA_GetSysmodeCnf */
    ID_TAFAGENT_MTA_GET_SYSMODE_CNF = 0x2005,

    /* MTA->TAFAGENT  获取天线状态回复消息 */
    /* MTA->TAFAGENT */
    /* _MSGPARSE_Interception TAFAGENT_MTA_GetAntStateCnf */
    ID_TAFAGENT_MTA_GET_ANT_STATE_CNF = 0x2007,

    /* _MSGPARSE_Interception TAFAGENT_MTA_WriteAcoreNvCnf */
    ID_TAFAGENT_MTA_WRITE_ACORE_NV_CNF = 0x200D,

    /* _MSGPARSE_Interception TAFAGENT_MTA_ProcAcoreNvCnf */
    ID_TAFAGENT_MTA_PROC_ACORE_NV_CNF            = 0x200E,
    /* _MSGPARSE_Interception TAFAGENT_MTA_ReadAcoreEncryptionNvCnf */
    ID_TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF = 0x200F,

    ID_TAFAGENT_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 TAFAGENT_MsgIdUint32;


typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT8            cid; /* 要获取参数的CID */

    VOS_UINT8            reserved[3];
} TAFAGENT_PS_GetCidParaReq;


typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */
} TAFAGENT_MTA_GetSysmodeReq;


typedef struct {
    VOS_MSG_HEADER                /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId;   /* _H2ASN_Skip */

    VOS_UINT32           ret;     /* 返回结果 */
    TAF_PDP_PrimContext  cidInfo; /* 查询获取指定CID的参数 */
} TAFAGENT_PS_GetCidParaCnf;



typedef struct {
    MN_CALL_ID_T       callId;      /* 所有正在通话的个数 */
    MN_CALL_TypeUint8  callType;    /* 通话类型 */
    MN_CALL_StateUint8 callState;   /* 通话状态 */
    VOS_UINT8          reserved[1]; /* 保留位 */
} TAFAGERNT_MN_CallInfo;

/*
 * 结构说明: 记录当前系统模式, 包含:
 *           (1) RAT Type 接入技术
 *           (2) Sub Mode 系统子模式
 */
typedef struct {
    TAF_SysModeUint8    ratType;    /* 接入技术   */
    TAF_SysSubmodeUint8 sysSubMode; /* 系统子模式 */
    VOS_UINT8           reserve[2];
} TAF_AGENT_SysMode;



typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT32           ret;   /* 回复结果 */
    /* 系统模式，包括接入技术与系统子模式 */
    TAF_AGENT_SysMode    sysMode;
} TAFAGENT_MTA_GetSysmodeCnf;


typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */
} TAFAGENT_MTA_GetAntStateReq;


typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT32           rslt;
    VOS_UINT16           antState; /* 天线状态 */
    VOS_UINT8            reserved1[2];

} TAFAGENT_MTA_GetAntStateCnf;


typedef struct {
    VOS_MSG_HEADER
    TAFAGENT_MsgIdUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT32           nvItemId;
    VOS_UINT32           nvLength;
    VOS_UINT32           isNeedCheck;
    VOS_UINT32           offset;
    VOS_UINT8            data[TAFAGENT_MTA_WRITE_ACORE_NV_REQ_DATA_LEN];
} TAFAGENT_MTA_WriteAcoreNvReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId;  /* _H2ASN_Skip */
    VOS_UINT32           result; /* 返回结果 */
} TAFAGENT_MTA_WriteAcoreNvCnf;


typedef struct {
    VOS_MSG_HEADER
    TAFAGENT_MsgIdUint32               msgId; /* _H2ASN_Skip */
    TAFAGENT_MTA_ProcAcoreNvTpyeUint32 procACoreNvType;
} TAFAGENT_MTA_ProcAcoreNvReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    TAFAGENT_MsgIdUint32 msgId;  /* _H2ASN_Skip */
    VOS_UINT32           result; /* 返回结果 */
} TAFAGENT_MTA_ProcAcoreNvCnf;

typedef struct {
    VOS_MSG_HEADER
    TAFAGENT_MsgIdUint32 msgId;
    VOS_UINT32           nvItemId;
} TAFAGENT_MTA_ReadAcoreEncryptionNvReq;

typedef struct {
    VOS_MSG_HEADER
    TAFAGENT_MsgIdUint32 msgId;
    VOS_UINT32           result; /* 返回结果 */
    VOS_UINT32           nvLength;
    VOS_UINT8            dataArray[TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF_DATA_ARRAY_LEN];
} TAFAGENT_MTA_ReadAcoreEncryptionNvCnf;
/*
 * H2ASN顶级消息结构定义
 */
typedef struct {
    /* _H2ASN_MsgChoice_Export TAFAGENT_MsgIdUint32 */
    TAFAGENT_MsgIdUint32 msgId;
    VOS_UINT8            msgBlock[TAF_AGENT_MSG_DATA_MSG_BLOCK_LEN];
    /*
     * _H2ASN_MsgChoice_When_Comment          TAFAGENT_MsgIdUint32
     */
} TAF_AGENT_MSG_DATA;
/* _H2ASN_Length UINT32 */

typedef struct {
    VOS_MSG_HEADER
    TAF_AGENT_MSG_DATA msgData;
} TafAgentInterface_MSG;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
