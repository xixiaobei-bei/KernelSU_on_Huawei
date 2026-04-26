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
#ifndef __TAFSSAAPI_H__
#define __TAFSSAAPI_H__

#include "vos.h"
#include "taf_type_def.h"
#include "PsTypeDef.h"
#include "taf_api.h"
#include "lcs_comm_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_SSA_MSG_ID_BASE 0x5000 /* ID_TAF_SSA_INTERNAL_BASE */
#define TAF_SSA_EVT_ID_BASE (TAF_SSA_MSG_ID_BASE + 0x800)

#define TAF_SSA_LCS_THIRD_PARTY_ADDR_MAX_LEN 20
#define TAF_SSA_LCS_MTLR_MAX_NUM 7

/* 封装OSA消息头 */
#if (VOS_OS_VER == VOS_WIN32)
#define TAF_SSA_CFG_MSG_HDR(pstMsg, ulRecvPid, ulMsgId) do { \
    ((MSG_Header *)(pstMsg))->ulSenderCpuId   = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(pstMsg))->ulSenderPid     = WUEPS_PID_TAF;   \
    ((MSG_Header *)(pstMsg))->ulReceiverCpuId = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(pstMsg))->ulReceiverPid   = (ulRecvPid);     \
    ((MSG_Header *)(pstMsg))->msgName         = (ulMsgId);       \
} while (0)
#else
#define TAF_SSA_CFG_MSG_HDR(pstMsg, ulRecvPid, ulMsgId) do { \
    VOS_SET_SENDER_ID(pstMsg, WUEPS_PID_TAF);       \
    VOS_SET_RECEIVER_ID(pstMsg, (ulRecvPid));       \
    ((MSG_Header *)(pstMsg))->msgName = (ulMsgId);   \
} while (0)
#endif

/* 封装OSA消息头(SSA内部消息) */
#define TAF_SSA_CFG_INTRA_MSG_HDR(pstMsg, ulMsgId) TAF_SSA_CFG_MSG_HDR(pstMsg, WUEPS_PID_TAF, ulMsgId)

/* 获取OSA消息内容 */
#define TAF_SSA_GET_MSG_ENTITY(msg) ((VOS_VOID *)&(((MSG_Header *)(msg))->msgName))

/* 获取OSA消息长度 */
#define TAF_SSA_GET_MSG_LENGTH(msg) (VOS_GET_MSG_LEN(msg))

/* 封装OSA消息初始化消息内容接口 */
#define TAF_SSA_CLR_MSG_ENTITY(pstMsg) TAF_SSA_ClearMsgEntity((MSG_Header *)pstMsg)


enum TAF_SSA_MsgId {
    /* 标准命令[0x5000, 0x50FF] */
    /* 之前老的与呼叫无关的命令需要后面补充并转移到此处 */
    /* TAF->TAF, A_TAF->TAF */
    /* +CMOLR */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MOLR_REQ_STRU */
    ID_TAF_SSA_SET_LCS_MOLR_REQ = TAF_SSA_MSG_ID_BASE + 0x0001,
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MOLR_REQ_STRU */
    ID_TAF_SSA_GET_LCS_MOLR_REQ = TAF_SSA_MSG_ID_BASE + 0x0002,

    /* +CMTLR */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MTLR_REQ_STRU */
    ID_TAF_SSA_SET_LCS_MTLR_REQ = TAF_SSA_MSG_ID_BASE + 0x0003,
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MTLR_REQ_STRU */
    ID_TAF_SSA_GET_LCS_MTLR_REQ = TAF_SSA_MSG_ID_BASE + 0x0004,

    /* +CMTLRA */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MTLRA_REQ_STRU */
    ID_TAF_SSA_SET_LCS_MTLRA_REQ = TAF_SSA_MSG_ID_BASE + 0x0005,
    /* ERR->TAF */
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MTLRA_REQ_STRU */
    ID_TAF_SSA_GET_LCS_MTLRA_REQ = TAF_SSA_MSG_ID_BASE + 0x0006,

    /* 私有命令[0x5100, 0x51FF] */

    ID_TAF_SSA_MSG_ID_BUTT
};
typedef VOS_UINT32 TAF_SSA_MsgIdUint32;


enum TAF_SSA_EvtId {
    /* 标准命令[0x5800, 0x58FF] */
    /* 之前老的与呼叫无关的命令需要后面补充并转移到此处 */

    /* +CMOLR */
    /* TAF->TAF, A_TAF->TAF */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MOLR_CNF_STRU */
    ID_TAF_SSA_SET_LCS_MOLR_CNF = TAF_SSA_EVT_ID_BASE + 0x0001,
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MOLR_CNF_STRU */
    ID_TAF_SSA_GET_LCS_MOLR_CNF = TAF_SSA_EVT_ID_BASE + 0x0002,

    /* +CMOLRG/+CMOLRN/+CMOLRE */
    /* _MSGPARSE_Interception TAF_ELF_SSA_LCS_MOLR_NTF_STRU */
    ID_TAF_SSA_LCS_MOLR_NTF = TAF_SSA_EVT_ID_BASE + 0x0003,

    /* +CMTLR */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MTLR_CNF_STRU */
    ID_TAF_SSA_SET_LCS_MTLR_CNF = TAF_SSA_EVT_ID_BASE + 0x0004,
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MTLR_CNF_STRU */
    ID_TAF_SSA_GET_LCS_MTLR_CNF = TAF_SSA_EVT_ID_BASE + 0x0005,
    /* _MSGPARSE_Interception TAF_ELF_SSA_LCS_MTLR_NTF_STRU */
    ID_TAF_SSA_LCS_MTLR_NTF = TAF_SSA_EVT_ID_BASE + 0x0006,

    /* +CMTLRA */
    /* _MSGPARSE_Interception TAF_ELF_SSA_SET_LCS_MTLRA_CNF_STRU */
    ID_TAF_SSA_SET_LCS_MTLRA_CNF = TAF_SSA_EVT_ID_BASE + 0x0007,
    /* _MSGPARSE_Interception TAF_ELF_SSA_GET_LCS_MTLRA_CNF_STRU */
    ID_TAF_SSA_GET_LCS_MTLRA_CNF = TAF_SSA_EVT_ID_BASE + 0x0008,

    /* 私有命令[0x5900, 0x59FF] */

    ID_TAF_SSA_EVT_ID_BUTT
};
typedef VOS_UINT32 TAF_SSA_EvtIdUint32;


enum TAF_SSA_LcsMolrEnableType {
    TAF_SSA_LCS_MOLR_ENABLE_TYPE_DISABLE = 0,
    TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA,
    TAF_SSA_LCS_MOLR_ENABLE_TYPE_GAD,
    TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA_GAD,

    TAF_SSA_LCS_MOLR_ENABLE_TYPE_BUTT
};
typedef VOS_UINT8 TAF_SSA_LcsMolrEnableTypeUint8;


enum TAF_SSA_LcsMolrRpt {
    TAF_SSA_LCS_MOLR_RPT_NMEA = 0,
    TAF_SSA_LCS_MOLR_RPT_GAD,

    TAF_SSA_LCS_MOLR_RPT_BUTT
};
typedef VOS_UINT8 TAF_SSA_LcsMolrRptUint8;


enum TAF_SSA_LcsMtlrSubscribe {
    TAF_SSA_LCS_MTLR_SUBSCRIBE_OFF = 0,
    TAF_SSA_LCS_MTLR_SUBSCRIBE_CONTRL,
    TAF_SSA_LCS_MTLR_SUBSCRIBE_SUPL,
    TAF_SSA_LCS_MTLR_SUBSCRIBE_CONTRL_SUPL,

    TAF_SSA_LCS_MTLR_SUBSCRIBE_BUTT
};
typedef VOS_UINT8 TAF_SSA_LcsMtlrSubscribeUint8;


typedef struct {
    VOS_UINT8 gpgsa : 1;
    VOS_UINT8 gpgga : 1;
    VOS_UINT8 gpgsv : 1;
    VOS_UINT8 gprmc : 1;
    VOS_UINT8 gpvtg : 1;
    VOS_UINT8 gpgll : 1;
    VOS_UINT8 spare : 2;
} TAF_SSA_LcsNmeaRep;


typedef struct {
    VOS_UINT32 opMethod : 1;
    VOS_UINT32 opHorAccSet : 1;
    VOS_UINT32 opVerReq : 1;
    VOS_UINT32 opVerAccSet : 1;
    VOS_UINT32 opVelReq : 1;
    VOS_UINT32 opRepMode : 1;
    VOS_UINT32 opTimeout : 1;
    VOS_UINT32 opInterval : 1;
    VOS_UINT32 opShapeRep : 1;
    VOS_UINT32 opPlane : 1;
    VOS_UINT32 opNmeaRep : 1;
    VOS_UINT32 opThirdPartyAddr : 1;
    VOS_UINT32 opSpare : 20;

    TAF_SSA_LcsMolrEnableTypeUint8 enable;
    LCS_MolrMethodUint8            method;
    LCS_HorAccSetUint8             horAccSet;
    VOS_UINT8                      horAcc;

    LCS_VerReqUint8    verReq;
    LCS_VerAccSetUint8 verAccSet;
    VOS_UINT8          verAcc;
    LCS_VelReqUint8    velReq;

    VOS_UINT8        reserved1[3];
    LCS_RepModeUint8 repMode;
    VOS_UINT16       timeOut;
    VOS_UINT16       interval;

    VOS_UINT8          shapeRep;
    LCS_PlaneUint8     plane;
    TAF_SSA_LcsNmeaRep nmeaRep;

    VOS_CHAR thirdPartyAddr[TAF_SSA_LCS_THIRD_PARTY_ADDR_MAX_LEN + 1];
    /*
     * _H2ASN_Array2String
     */
} TAF_SSA_LcsMolrParaSet;


typedef struct {
    TAF_Ctrl               ctrl;
    TAF_SSA_LcsMolrParaSet molrPara;
} TAF_SSA_SetLcsMolrReq;


typedef struct {
    TAF_Ctrl             ctrl;
    TAF_ERROR_CodeUint32 result;
} TAF_SSA_SetLcsMolrCnf;


typedef struct {
    TAF_Ctrl  ctrl;
    VOS_UINT8 reserved1[4];
} TAF_SSA_GetLcsMolrReq;


typedef struct {
    TAF_Ctrl                       ctrl;
    TAF_ERROR_CodeUint32           result;
    LCS_MOLR_Para                  molrPara;
    TAF_SSA_LcsMolrEnableTypeUint8 enable; /* MO-LR使能状态 */
    LCS_PlaneUint8                 plane;
    TAF_SSA_LcsNmeaRep             nmeaRep;
    VOS_UINT8                      reserved[1];
} TAF_SSA_GetLcsMolrSnf;


typedef struct {
    TAF_Ctrl                ctrl;
    TAF_ERROR_CodeUint32    result;
    TAF_SSA_LcsMolrRptUint8 rptTypeChoice;
    VOS_UINT8               reserved1[1];
    VOS_UINT16              locationStrLen;
    VOS_CHAR                locationStr[8];
} TAF_SSA_LcsMolrNtf;


typedef struct {
    TAF_Ctrl                      ctrl;
    TAF_SSA_LcsMtlrSubscribeUint8 subscribe;
    VOS_UINT8                     reserved1[3];
} TAF_SSA_SetLcsMtlrReq;


typedef TAF_SSA_SetLcsMolrCnf TAF_SSA_SetLcsMtlrCnf;


typedef TAF_SSA_GetLcsMolrReq TAF_SSA_GetLcsMtlrReq;


typedef struct {
    TAF_Ctrl                      ctrl;
    TAF_ERROR_CodeUint32          result;
    TAF_SSA_LcsMtlrSubscribeUint8 subscribe;
    VOS_UINT8                     reserved1[3];
} TAF_SSA_GetLcsMtlrCnf;


typedef struct {
    TAF_Ctrl      ctrl;
    LCS_MTLR_Para mtlrPara;
} TAF_SSA_LcsMtlrNtf;


typedef struct {
    LCS_MtlraOpUint8 allow;
    VOS_UINT8        handleId;
    VOS_UINT8        reserved1[2];
} TAF_SSA_LcsMtlraParaSet;


typedef struct {
    TAF_Ctrl                ctrl;
    TAF_SSA_LcsMtlraParaSet cmtlraPara;
} TAF_SSA_SetLcsMtlraReq;


typedef TAF_SSA_SetLcsMolrCnf TAF_SSA_SetLcsMtlraCnf;


typedef TAF_SSA_GetLcsMolrReq TAF_SSA_GetLcsMtlraReq;


typedef struct {
    TAF_Ctrl                                     ctrl;
    TAF_ERROR_CodeUint32                         result;
    VOS_UINT8                                    cnt;
    VOS_UINT8                                    reserved1[3];
    LCS_MtlraOpUint8 TAF_COMM_ATTRIBUTE_ALIGNED4 allow[TAF_SSA_LCS_MTLR_MAX_NUM];
    VOS_UINT8                                    reserved2[1];
    VOS_UINT8        TAF_COMM_ATTRIBUTE_ALIGNED4 handleId[TAF_SSA_LCS_MTLR_MAX_NUM];
    VOS_UINT8                                    reserved3[1];
} TAF_SSA_GetLcsMtlraCnf;


typedef struct {
    MSG_Header                            header; /* _H2ASN_Skip */
    VOS_UINT32                            evtExt;
    TAF_SSA_EvtIdUint32                   evtId;
    VOS_UINT8 TAF_COMM_ATTRIBUTE_ALIGNED4 content[4]; /* 修改数组大小时请同步修改TAF_DEFAULT_CONTENT_LEN宏 */
} TAF_SSA_Evt;

/*
 * H2ASN顶级消息结构定义
 */
typedef struct {
    /* _H2ASN_MsgChoice_Export TAF_SSA_MsgIdUint32 */
    TAF_SSA_MsgIdUint32                   msgId;
    VOS_UINT8 TAF_COMM_ATTRIBUTE_ALIGNED4 msgBlock[4];
    /*
     * _H2ASN_MsgChoice_When_Comment          TAF_SSA_MsgIdUint32
     */
} TAF_SSA_MsgReq;
/* _H2ASN_Length UINT32 */

typedef struct {
    VOS_MSG_HEADER
    TAF_SSA_MsgReq msgReq;
} TafSsaApi_Msg;

typedef struct {
    VOS_UINT32          msgName;
    VOS_UINT32          evtExt;
    /* _H2ASN_MsgChoice_Export TAF_SSA_EvtIdUint32 */
    TAF_SSA_EvtIdUint32 evtId;
    VOS_UINT8 TAF_COMM_ATTRIBUTE_ALIGNED4 evtBlock[4];
    /*
     * _H2ASN_MsgChoice_When_Comment          TAF_SSA_EvtIdUint32
     */
} TAF_SSA_EvtCnf;
/* _H2ASN_Length UINT32 */

typedef struct {
    VOS_MSG_HEADER
    TAF_SSA_EvtCnf evtCnf;
} TAF_SSA_EvtMsg;

extern VOS_VOID TAF_SSA_SndTafMsg(TAF_SSA_MsgIdUint32 msgId, const VOS_VOID *data, VOS_UINT32 length);
extern VOS_VOID TAF_SSA_GetCmolrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId);
extern VOS_VOID TAF_SSA_GetCmtlraInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId);
extern VOS_VOID TAF_SSA_GetCmtlrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId);
extern VOS_VOID TAF_SSA_SetCmolrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                     TAF_SSA_LcsMolrParaSet *molrPara);
extern VOS_VOID TAF_SSA_SetCmtlraInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                      TAF_SSA_LcsMtlraParaSet *cmtlraPara);
extern VOS_VOID TAF_SSA_SetCmtlrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                     TAF_SSA_LcsMtlrSubscribeUint8 subscribe);

extern VOS_VOID TAF_SSA_ClearMsgEntity(MSG_Header *msg);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of taf_ssa_api.h */
