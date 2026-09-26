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
#ifndef __AT_RABM_INTERFACE_H__
#define __AT_RABM_INTERFACE_H__

#include "vos.h"
#include "taf_type_def.h"
#include "PsTypeDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)


enum AT_RABM_MsgId {
    /* AT->RABM */
    /* _MSGPARSE_Interception AT_RABM_SetFastdormParaReq */
    ID_AT_RABM_SET_FASTDORM_PARA_REQ = 0,
    /* _MSGPARSE_Interception AT_RABM_QryFastdormParaReq */
    ID_AT_RABM_QRY_FASTDORM_PARA_REQ = 1,
    /* _MSGPARSE_Interception AT_RABM_ReleaseRrcReq */
    ID_AT_RABM_SET_RELEASE_RRC_REQ = 4,
    /* _MSGPARSE_Interception AT_RABM_SetVoicepreferParaReq  */
    ID_AT_RABM_SET_VOICEPREFER_PARA_REQ = 6,
    /* _MSGPARSE_Interception AT_RABM_QryVoicepreferParaReq */
    ID_AT_RABM_QRY_VOICEPREFER_PARA_REQ = 8,
    /* RABM->AT */
    /* _MSGPARSE_Interception RABM_AT_SetFastdormParaCnf */
    ID_RABM_AT_SET_FASTDORM_PARA_CNF = 2,
    /* _MSGPARSE_Interception RABM_AT_QryFastdormParaCnf */
    ID_RABM_AT_QRY_FASTDORM_PARA_CNF = 3,
    /* _MSGPARSE_Interception RABM_AT_ReleaseRrcCnf */
    ID_RABM_AT_SET_RELEASE_RRC_CNF = 5,
    /* _MSGPARSE_Interception RABM_AT_SetVoicepreferParaCnf */
    ID_RABM_AT_SET_VOICEPREFER_PARA_CNF = 7,
    /* _MSGPARSE_Interception RABM_AT_QryVoicepreferParaCnf */
    ID_RABM_AT_QRY_VOICEPREFER_PARA_CNF = 9,
    /* _MSGPARSE_Interception RABM_AT_VoicepreferStatusReport */
    ID_RABM_AT_VOICEPREFER_STATUS_REPORT = 10,

    ID_RABM_AT_MSG_ID_ENUM_BUTT

};
typedef VOS_UINT32 AT_RABM_MsgIdUint32;


enum AT_RABM_ParaSetRslt {
    AT_RABM_PARA_SET_RSLT_SUCC = 0, /* 参数设置成功 */
    AT_RABM_PARA_SET_RSLT_FAIL,     /* 参数设置失败 */
    AT_RABM_PARA_SET_RSLT_BUTT
};
typedef VOS_UINT32 AT_RABM_ParaSetRsltUint32;


enum AT_RABM_FastdormOperation {
    AT_RABM_FASTDORM_STOP_FD_ASCR,    /* 停止FD和ASCR */
    AT_RABM_FASTDORM_START_FD_ONLY,   /* 仅允许FD */
    AT_RABM_FASTDORM_START_ASCR_ONLY, /* 仅允许ASCR */
    AT_RABM_FASTDORM_START_FD_ASCR,   /* 允许FD和ASCR */
    AT_RABM_FASTDORM_START_BUTT
};
typedef VOS_UINT32 AT_RABM_FastdormOperationUint32;


typedef struct {
    AT_RABM_FastdormOperationUint32 fastDormOperationType; /* 操作类型 */
    VOS_UINT32                      timeLen;               /* 无流量时长 */
} AT_RABM_FastdormPara;


typedef struct {
    MSG_Header           msgHeader;    /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T       clientId;     /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T    opId;         /* AT发消息给RABM所带的OpId */
    VOS_UINT8            reserve1[1];  /* 保留 */
    AT_RABM_FastdormPara fastDormPara; /* FAST DORMANCY特性相关的参数 */
} AT_RABM_SetFastdormParaReq;


typedef struct {
    MSG_Header        msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T    clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8         reserve1[1]; /* 保留 */
} AT_RABM_QryFastdormParaReq;


typedef struct {
    MSG_Header msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT16 clientId;    /* AT发消息给RABM所带的ClientId */
    VOS_UINT8  opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8  reserve1[1]; /* 保留 */
} AT_RABM_ReleaseRrcReq;


typedef struct {
    MSG_Header msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT16 clientId;    /* AT发消息给RABM所带的ClientId */
    VOS_UINT8  opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8  reserve1[1]; /* 保留 */
    VOS_UINT32 rslt;        /* 设置结果 */
} RABM_AT_ReleaseRrcCnf;


typedef struct {
    MSG_Header                msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T            clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T         opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8                 reserve1[1]; /* 保留 */
    AT_RABM_ParaSetRsltUint32 rslt;        /* 设置结果 */
} RABM_AT_SetFastdormParaCnf;


typedef struct {
    MSG_Header           msgHeader;    /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T       clientId;     /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T    opId;         /* AT发消息给RABM所带的OpId */
    VOS_UINT8            rslt;         /* 结果 */
    AT_RABM_FastdormPara fastDormPara; /* 查询返回的当前参数 */
} RABM_AT_QryFastdormParaCnf;


typedef struct {
    MSG_Header        msgHeader;           /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T    clientId;            /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T opId;                /* AT发消息给RABM所带的OpId */
    VOS_UINT8         reserve1[1];         /* 保留 */
    VOS_UINT32        voicePreferApStatus; /* VoicePrefer AP status */
} AT_RABM_SetVoicepreferParaReq;

typedef struct {
    MSG_Header                msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T            clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T         opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8                 reserve1[1]; /* 保留 */
    AT_RABM_ParaSetRsltUint32 rslt;        /* 设置结果 */
} RABM_AT_SetVoicepreferParaCnf;


typedef struct {
    MSG_Header        msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T    clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8         reserve1[1]; /* 保留 */
} AT_RABM_QryVoicepreferParaReq;


typedef struct {
    MSG_Header        msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T    clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8         reserve1[1]; /* 保留 */
    VOS_UINT32        rslt;        /* 结果 */
} RABM_AT_QryVoicepreferParaCnf;


typedef struct {
    MSG_Header        msgHeader;   /* 消息头 */ /* _H2ASN_Skip */
    MN_CLIENT_ID_T    clientId;    /* AT发消息给RABM所带的ClientId */
    MN_OPERATION_ID_T opId;        /* AT发消息给RABM所带的OpId */
    VOS_UINT8         reserve1[1]; /* 保留 */
    VOS_UINT32        vpStatus;    /* modem 侧VP状态0:未激活,1:激活 */
} RABM_AT_VoicepreferStatusReport;

/*
 * H2ASN顶级消息结构定义
 */
typedef struct {
    AT_RABM_MsgIdUint32 msgID; /* _H2ASN_MsgChoice_Export AT_RABM_MsgIdUint32
                                * 为了兼容NAS的消息头定义，所以转换ASN.AT_RABM_MSG_ID_ENUM_UINT32
                                */

    VOS_UINT8 msgBlock[4];
    /*
     * _H2ASN_MsgChoice_When_Comment          AT_RABM_MsgIdUint32
     */
} AT_RABM_SndInternalData;

typedef struct {
    VOS_MSG_HEADER
    AT_RABM_SndInternalData msgData;
} AtRabmInterfacemsg_Msg;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of NasMmcSuspendProcAct.h */
