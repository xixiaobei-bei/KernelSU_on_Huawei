/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
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

#ifndef THROT_AT_INTERFACE_H
#define THROT_AT_INTERFACE_H

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * #pragma pack(*)    设置字节对齐方式
 */
#pragma pack(push, 4)

#define    MAX_CABTRDP_PARA_NUM       15
#define    MAX_CSBTRDP_PARA_NUM       15
#define    MAX_CSDBTRDP_PARA_NUM      12
#define    NAS_THROT_AT_MAX_APN_LEN   99
#define    AT_THROT_MSG_ID_BASE       0
#define    THROT_AT_MSG_ID_HEADER     1000
#define    NAS_THROT_MAX_APN_LEN      99

/*
 * 结构说明: THROT和AT的之间的消息
 */
enum THROT_AT_MsgType {

    /* AT->THROT Cmd:CABTSRI Timeout:T3396 */
    ID_AT_THROT_CABTSRI_CONFIGURATION_SET_REQ       = AT_THROT_MSG_ID_BASE + 0x0001,/* _H2ASN_MsgChoice AT_THROT_CabtsriConfigSetReq */
    ID_AT_THROT_CABTSRI_CONFIGURATION_QUERY_REQ     = AT_THROT_MSG_ID_BASE + 0x0002,/* _H2ASN_MsgChoice AT_THROT_CabtsriQueryReq */
    ID_AT_THROT_CABTRDP_QUERY_REQ                   = AT_THROT_MSG_ID_BASE + 0x0003,/* _H2ASN_MsgChoice AT_THROT_CabtrdpQueryReq */

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /* AT->THROT Cmd:CSBTSRI Timeout:T3585 */
    /* _MSGPARSE_Interception AT_THROT_CsbtsriConfigSetReq */
    ID_AT_THROT_CSBTSRI_CONFIGURATION_SET_REQ       = AT_THROT_MSG_ID_BASE + 0x0004,
    /* _MSGPARSE_Interception AT_THROT_CsbtsriQueryReq */
    ID_AT_THROT_CSBTSRI_CONFIGURATION_QUERY_REQ     = AT_THROT_MSG_ID_BASE + 0x0005,
    /* _MSGPARSE_Interception AT_THROT_CsbtrdpQueryReq */
    ID_AT_THROT_CSBTRDP_QUERY_REQ                   = AT_THROT_MSG_ID_BASE + 0x0006,

    /* AT->THROT Cmd:CSDBTSRI Timeout:T3584 */
     /* _MSGPARSE_Interception AT_THROT_CsdbtsriConfigSetReq */
    ID_AT_THROT_CSDBTSRI_CONFIGURATION_SET_REQ      = AT_THROT_MSG_ID_BASE + 0x0007,
    /* _MSGPARSE_Interception AT_THROT_CsdbtsriQueryReq */
    ID_AT_THROT_CSDBTSRI_CONFIGURATION_QUERY_REQ    = AT_THROT_MSG_ID_BASE + 0x0008,
    /* _MSGPARSE_Interception AT_THROT_CsdbtrdpQueryReq */
    ID_AT_THROT_CSDBTRDP_QUERY_REQ                  = AT_THROT_MSG_ID_BASE + 0x0009,
#endif
    /* THROT->AT Cmd:CABTSRI Timeout:T3396*/
    ID_THROT_AT_CABTSRI_CONFIGURATION_SET_CNF       = THROT_AT_MSG_ID_HEADER + 0x0001,/* _H2ASN_MsgChoice THROT_AT_CabtsriConfigSetCnf 2_0 */
    ID_THROT_AT_CABTSRI_CONFIGURATION_QUERY_CNF     = THROT_AT_MSG_ID_HEADER + 0x0002,/* _H2ASN_MsgChoice THROT_AT_CabtsriQueryCnf 2_0 */
    ID_THROT_AT_CABTRDP_QUERY_CNF                   = THROT_AT_MSG_ID_HEADER + 0x0003,/* _H2ASN_MsgChoice THROT_AT_CabtrdpQueryCnf 2_0 */
    ID_THROT_AT_SINGLE_APN_BACKOFF_TIMER_PARA_NTF   = THROT_AT_MSG_ID_HEADER + 0x0004,/* _H2ASN_MsgChoice THROT_AT_SingleApnBackoffParaNtf 2_0 */

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /* THROT->AT CSBTSRI Timeout:T3585*/
    ID_THROT_AT_CSBTSRI_CONFIGURATION_SET_CNF       = THROT_AT_MSG_ID_HEADER + 0x0005,/* _H2ASN_MsgChoice THROT_AT_CsbtsriConfigSetCnf */
    ID_THROT_AT_CSBTSRI_CONFIGURATION_QUERY_CNF     = THROT_AT_MSG_ID_HEADER + 0x0006,/* _H2ASN_MsgChoice THROT_AT_CsbtsriQueryCnf */
    ID_THROT_AT_CSBTRDP_QUERY_CNF                   = THROT_AT_MSG_ID_HEADER + 0x0007,/* _H2ASN_MsgChoice THROT_AT_CsbtrdpQueryCnf */
    ID_THROT_AT_SINGLE_SNSSAI_BACKOFF_TIMER_PARA_NTF   = THROT_AT_MSG_ID_HEADER + 0x0008,/* _H2ASN_MsgChoice THROT_AT_SingleSnssaiBackoffParaNtf */

    /* THROT->AT CSDBTSRI Timeout:T3584+ backoff*/
    ID_THROT_AT_CSDBTSRI_CONFIGURATION_SET_CNF      = THROT_AT_MSG_ID_HEADER + 0x0009,/* _H2ASN_MsgChoice THROT_AT_CsdbtsriConfigSetCnf */
    ID_THROT_AT_CSDBTSRI_CONFIGURATION_QUERY_CNF    = THROT_AT_MSG_ID_HEADER + 0x000a,/* _H2ASN_MsgChoice THROT_AT_CsdbtsriQueryCnf */
    ID_THROT_AT_CSDBTRDP_QUERY_CNF                  = THROT_AT_MSG_ID_HEADER + 0x000b,/* _H2ASN_MsgChoice THROT_AT_CsdbtrdpQueryCnf */
    ID_THROT_AT_SINGLE_SNSSAI_DNN_BACKOFF_TIMER_PARA_NTF   = THROT_AT_MSG_ID_HEADER + 0x000c, /*THROT_AT_SingleDnnSnssaiBackoffParaNtf */
#endif
    ID_THROT_AT_MSG_BUTT
};
typedef VOS_UINT32 THROT_AT_MsgTypeUint32;

/*
 * 枚举说明: 操作返回值
 */
enum THROT_AT_Result {
    THROT_AT_RESULT_SUCC = 0, /* 操作成功 */
    THROT_AT_RESULT_FAIL,     /* 操作失败 */
    THROT_AT_RESULT_BUTT
};
typedef VOS_UINT32 THROT_AT_ResultUint32;

/*
 * 枚举说明: 操作返回值
 */
enum THROT_BackOffReportAtCmd {
    THROT_BACKOFF_AT_CMD_CABTSRI = 0, /* 主动上报的AT命令 */
    THROT_BACKOFF_AT_CMD_CABTRDP,     /* 查询上报的AT命令 */
    THROT_BACKOFF_AT_CMD_BUTT
};
typedef VOS_UINT32 THROT_BackOffReportAtCmdUint32;

/*
 * 枚举说明: 异系统是否可以发起session management procedure
 */
enum THROT_AT_ReAttemptRatInd {
    THROT_AT_RAT_IND_ALLOWED        = 0, /* 异系统后，允许发起session management procedure */
    THROT_AT_RAT_IND_NOT_ALLOWED,        /* 异系统后，不允许发起session management procedure */
    THROT_AT_RAT_IND_BUTT
};
typedef VOS_UINT32 THROT_AT_ReAttemptRatIndUint32;

/*
 * 枚举说明: eplmn内是否可以发起session management procedure
 */
enum THROT_AT_ReAttemptEplmnInd {
    THROT_AT_EPLMN_IND_ALLOWED      = 0, /* eplmn范围内，允许发起session management procedure */
    THROT_AT_EPLMN_IND_NOT_ALLOWED,      /* eplmn范围内，不允许发起session management procedure */
    THROT_AT_EPLMN_IND_BUTT
};
typedef VOS_UINT32 THROT_AT_ReAttemptEplmnIndUint32;

/*
 * 枚举说明:  指示session management procedure
 */
enum THROT_AT_SessionProc {
    THROT_AT_SESSION_ALL_PROCEDURE                      = 0,
    THROT_AT_SESSION_STAND_ALONE_PDN_CONN_PROC          = 1,
    THROT_AT_SESSION_BEARER_ALLOC_PROC                  = 2,
    THROT_AT_SESSION_BEARER_MODIFY_PROC                 = 3,
    THROT_AT_SESSION_PDP_ACTIVE_PROC                    = 4,
    THROT_AT_SESSION_SECOND_PDP_ACTIVE_PROC             = 5,
    THROT_AT_SESSION_PDP_MODIFY_PROC                    = 6,
    THROT_AT_SESSION_PDU_ESTABLISH_MODIFY_PROC          = 7,
    THROT_AT_SESSION_PDU_MODIFY_PROC                    = 8,
    THROT_AT_SESSION_ATTACH_AND_STAND_ALONE_PDN_PROC    = 9,
    THROT_AT_SESSION_BUTT
};
typedef VOS_UINT32 THROT_AT_SessionProcUint32;


/*
 * 枚举说明:  指示信令的优先级
 */
enum THROT_AT_Nslpi {
    THROT_AT_NSLPI_CONFIG_LOW_PRIORITY      = 0,
    THROT_AT_NSLPI_NOT_CONFIG_LOW_PRIORITY,
    THROT_AT_NSLPI_BUTT
};
typedef VOS_UINT32 THROT_AT_NslpiUint32;

/*
 * 枚举说明: backoff timer的event type
 */
enum THROT_AT_BackOffEventType {
    THROT_AT_BACKOFF_EVENT_TYPE_STARTED      = 0,
    THROT_AT_BACKOFF_EVENT_TYPE_STOPPED,
    THROT_AT_BACKOFF_EVENT_TYPE_EXPIRED,
    THROT_AT_BACKOFF_EVENT_TYPE_DEACTIVATED,
    THROT_AT_BACKOFF_EVENT_TYPE_BUTT
};
typedef VOS_UINT32 THROT_AT_BackOffEventTypeUint32;

/*
 * 结构说明: AT模块与THROT模块间的消息结构
 */
typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT8      msg[4]; /* 消息前4字节 */
} THROT_AT_Msg;

/* 结构说明: 调节算法APN参数(结构同APP_ESM_APN_INFO_STRU) */
typedef struct {
    VOS_UINT8 apnLen;
    VOS_UINT8 apnName[NAS_THROT_MAX_APN_LEN];
} NAS_THROT_ApnInfo;

typedef struct {
    VOS_UINT32 opSd : 1;
    VOS_UINT32 opMappedSst : 1;
    VOS_UINT32 opMappedSd : 1;
    VOS_UINT32 opSpare : 29;

    VOS_UINT8  sst;
    VOS_UINT8  mappedSst;
    VOS_UINT8  reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32 sd;
    VOS_UINT32 mappedSd;
} NAS_THROT_Snssai;

typedef struct {
    VOS_UINT32       isEmptyNssai;
    NAS_THROT_Snssai nssai;
} NAS_THROT_SnssaiInfo;

typedef NAS_THROT_ApnInfo NAS_THROT_DnnInfo;

/* 结构说明: backoff timer 参数 */
typedef struct {
    VOS_UINT32                          opBackOffTimerLen           : 1;
    VOS_UINT32                          opReAttemptRatIndicator     : 1;
    VOS_UINT32                          opReAttemptEplmnIndicator   : 1;
    VOS_UINT32                          opNslpi                     : 1;
    VOS_UINT32                          opProcedure                 : 1;
    VOS_UINT32                          opSpare                     : 27;

    VOS_UINT32                          isTimerDeactive;
    VOS_UINT32                          backOffTimerLen;
    THROT_AT_ReAttemptRatIndUint32      reAttemptRatInd;
    THROT_AT_ReAttemptEplmnIndUint32    reAttemptEplmnInd;
    THROT_AT_NslpiUint32                nslpi;
    THROT_AT_SessionProcUint32          sessionProcedure;
} THROT_AT_BackOffTimerPara;

/* backoff timer */
typedef struct {
    NAS_THROT_ApnInfo                    apnInfo;
    THROT_AT_BackOffTimerPara           backOffPara;
} THROT_AT_CabtrdpParaInfo;

/* 设置命令回复的通用结构体，发送方throt，接收方at */
typedef struct {
    VOS_MSG_HEADER
    THROT_AT_MsgTypeUint32 msgId;
    VOS_UINT16             clientId;
    VOS_UINT8              rsv[2];
    VOS_UINT32             result;
} THROT_AT_CnfMsg;

/*
 * 结构说明:AP通过此消息给THROT, 用于设置CABTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    MODEM_ID_ENUM_UINT16        modemId;
    VOS_UINT16                  clientId;
    VOS_UINT32                  isEnableCabtsri;    /* VOS_TRUE: 打开CABTSRI功能; VOS_FALSE: 关闭CABTSRI功能 */
} AT_THROT_CabtsriConfigSetReq;

/*
 * 结构说明:通知AT 设置CABTSRI的结果
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];     /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  result;     /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CabtsriConfigSetCnf;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询当前CABTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;
} AT_THROT_CabtsriQueryReq;

/*
 * 结构说明:通知AT查询CABTSRI使能状态的结果
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];             /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  isEnableCabtsri;    /* VOS_TRUE: 打开CABTSRI功能; VOS_FALSE: 关闭CABTSRI功能 */
    VOS_UINT32                  result;             /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CabtsriQueryCnf;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定APN上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                      /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;  /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;

    VOS_UINT32                  opApn : 1;
    VOS_UINT32                  opSpare : 31;

    NAS_THROT_ApnInfo            apnInfo;

} AT_THROT_CabtrdpQueryReq;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定APN上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                          result;     /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */

    VOS_UINT32                          backOffParaNum;
    THROT_AT_CabtrdpParaInfo            backOffParaList[MAX_CABTRDP_PARA_NUM];
} THROT_AT_CabtrdpQueryCnf;

/*
 * 结构说明:THROT通过此消息, 上报backoff timer的para信息
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */

    THROT_AT_BackOffEventTypeUint32     eventType;
    NAS_THROT_ApnInfo                    apnInfo;
    THROT_AT_BackOffTimerPara           backOffPara;

} THROT_AT_SingleApnBackoffParaNtf;

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/*
 * 枚举说明:  指示受到CSBTRDP / CSDBTRDP backoff定时器影响的session management procedure
 */
enum THROT_AT_SnssaiSessionProc {
    /* backoff定时器适用所有流程 */
    THROT_AT_SNSSAI_SESSION_ALL_PROCEDURE       = 0,

    /* backoff定时器适用PDU_ESTABLISH流程 */
    THROT_AT_SNSSAI_SESSION_PDU_ESTABLISH_PROC  = 1,

    /* backoff定时器适用PDU_MODIFY流程 */
    THROT_AT_SNSSAI_SESSION_PDU_MODIFY_PROC     = 2,
    THROT_AT_SNSSAI_SESSION_BUTT
};
typedef VOS_UINT32 THROT_AT_SnssaiSessionProcUint32;


/* 结构说明: S-NSSAI Based  backoff timer 参数 */
typedef struct {
    VOS_UINT32                          opBackOffTimerLen           : 1;
    VOS_UINT32                          opReAttemptRatIndicator     : 1;
    VOS_UINT32                          opReAttemptEplmnIndicator   : 1;
    VOS_UINT32                          opProcedure                 : 1;
    VOS_UINT32                          opSpare                     : 28;

    VOS_UINT32                          isTimerDeactive;
    VOS_UINT32                          backOffTimerLen;
    THROT_AT_ReAttemptRatIndUint32      reAttemptRatInd;
    THROT_AT_ReAttemptEplmnIndUint32    reAttemptEplmnInd;
    THROT_AT_SnssaiSessionProcUint32   sessionProcedure;
} THROT_AT_SnssaiBasedBackOffTimerPara;

typedef struct {
    NAS_THROT_ApnInfo                    apnInfo;
    NAS_THROT_SnssaiInfo                 snssaiInfo;
    THROT_AT_SnssaiBasedBackOffTimerPara backOffPara;
} THROT_AT_CsdbtrdpBackOffTimerPara;

typedef struct {
    NAS_THROT_SnssaiInfo                 snssaiInfo;
    THROT_AT_SnssaiBasedBackOffTimerPara backOffPara;
} THROT_AT_CsbtrdpBackOffTimerPara;

/*
 * 结构说明:AP通过此消息给THROT, 用于设置CSBTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    MODEM_ID_ENUM_UINT16        modemId;
    VOS_UINT16                  clientId;
    VOS_UINT32                  isEnableCsbtsri;    /* VOS_TRUE: 打开CSBTSRI功能; VOS_FALSE: 关闭CSBTSRI功能 */
} AT_THROT_CsbtsriConfigSetReq;
/*
 * 结构说明:AP通过此消息给THROT, 用于设置CSBTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    MODEM_ID_ENUM_UINT16        modemId;
    VOS_UINT16                  clientId;
    VOS_UINT32                  isEnableCsdbtsri;    /* VOS_TRUE: 打开CSDBTSRI功能; VOS_FALSE: 关闭CSDBTSRI功能 */
} AT_THROT_CsdbtsriConfigSetReq;

/*
 * 结构说明:通知AT 设置CSBTSRI的结果
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];     /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  result;     /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CsbtsriConfigSetCnf;
/*
 * 结构说明:通知AT 设置CSBTSRI的结果
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];     /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  result;     /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CsdbtsriConfigSetCnf;


/*
 * 结构说明:AP通过此消息给THROT, 用于查询当前CSBTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;
} AT_THROT_CsbtsriQueryReq;
/*
 * 结构说明:AP通过此消息给THROT, 用于查询当前CSBTSRI的使能状态
 */
typedef struct {
    VOS_MSG_HEADER                          /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;      /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;
} AT_THROT_CsdbtsriQueryReq;

/*
 * 结构说明:通知AT查询CSBTSRI使能状态的结果
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];             /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  isEnableCsbtsri;    /* VOS_TRUE: 打开CSBTSRI功能; VOS_FALSE: 关闭CSBTSRI功能 */
    VOS_UINT32                  result;             /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CsbtsriQueryCnf;
/*
 * 结构说明:通知AT查询CSBTSRI使能状态的结果
 */
typedef struct {
    VOS_MSG_HEADER                                  /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;              /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    VOS_UINT8                   rsv[2];             /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                  isEnableCsdbtsri;    /* VOS_TRUE: 打开CSDBTSRI功能; VOS_FALSE: 关闭CSDBTSRI功能 */
    VOS_UINT32                  result;             /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */
} THROT_AT_CsdbtsriQueryCnf;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定S-nssai上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                      /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;  /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;

    VOS_UINT32                  opSnssai: 1;
    VOS_UINT32                  opSpare : 31;
    NAS_THROT_SnssaiInfo        snssaiInfo;

} AT_THROT_CsbtrdpQueryReq;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定S-nssai上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                      /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32      msgId;  /* _H2ASN_Skip */
    VOS_UINT16                  clientId;
    MODEM_ID_ENUM_UINT16        modemId;

    VOS_UINT32                  opApn: 1;
    VOS_UINT32                  opSnssai: 1;
    VOS_UINT32                  opSpare : 30;
    NAS_THROT_ApnInfo           apnInfo;
    NAS_THROT_SnssaiInfo        snssaiInfo;

} AT_THROT_CsdbtrdpQueryReq;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定APN上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                          result;     /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */

    VOS_UINT32                          backOffParaNum;
    THROT_AT_CsbtrdpBackOffTimerPara    backOffParaList[MAX_CSBTRDP_PARA_NUM];
} THROT_AT_CsbtrdpQueryCnf;

/*
 * 结构说明:AP通过此消息给THROT, 用于查询特定APN上的backoff timer参数
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32                          result; /* THROT_AT_RESULT_SUCC 表示成功，THROT_AT_RESULT_FAIL表示失败 */

    VOS_UINT32                          backOffParaNum;
    THROT_AT_CsdbtrdpBackOffTimerPara   backOffParaList[MAX_CSDBTRDP_PARA_NUM];
} THROT_AT_CsdbtrdpQueryCnf;

/*
 * 结构说明:THROT通过此消息, 上报backoff timer(T3585)的para信息
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */

    THROT_AT_CsbtrdpBackOffTimerPara   backOffPara;
    THROT_AT_BackOffEventTypeUint32     eventType;

} THROT_AT_SingleSnssaiBackoffParaNtf;

/*
 * 结构说明:THROT通过此消息, 上报backoff timer(T3584)的para信息
 */
typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
    THROT_AT_MsgTypeUint32              msgId;  /* _H2ASN_Skip */
    VOS_UINT16                          clientId;
    VOS_UINT8                           rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */

    THROT_AT_CsdbtrdpBackOffTimerPara   backOffPara;
    THROT_AT_BackOffEventTypeUint32     eventType;

} THROT_AT_SingleSnssaiDnnBackoffParaNtf;
#endif //#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
