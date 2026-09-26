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

#ifndef __DSM_RNIC_PIF_H__
#define __DSM_RNIC_PIF_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define DSM_RNIC_MSG_ID_HEADER 0x0000

/* 最大IPV6地址长度，不包括":" */
#define RNICITF_MAX_IPV6_ADDR_LEN 16
#define RNIC_NV_NAPI_LB_MAX_LEVEL 16
#define RNIC_NV_NAPI_LB_MAX_CPUS 16
#define RNIC_NV_MAX_CLUSTER 3        /* CPU最大CLUSTER个数 */
#define RNIC_NV_RHC_MAX_LEVEL 16

/*
 * 枚举说明: DSM与RNIC的消息定义
 */
enum DSM_RNIC_MsgId {
    ID_DSM_RNIC_IFACE_CFG_IND    = DSM_RNIC_MSG_ID_HEADER + 0x01,
    ID_DSM_RNIC_IFACE_REL_IND    = DSM_RNIC_MSG_ID_HEADER + 0x02,
    ID_DSM_RNIC_NAPI_CFG_IND     = DSM_RNIC_MSG_ID_HEADER + 0x03,
    ID_DSM_RNIC_NAPI_LB_CFG_IND  = DSM_RNIC_MSG_ID_HEADER + 0x04,
    ID_DSM_RNIC_RHC_CFG_IND      = DSM_RNIC_MSG_ID_HEADER + 0x05,
    ID_DSM_RNIC_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 DSM_RNIC_MsgIdUint32;

/*
 * 枚举说明: DSM与RNIC的消息定义
 */
enum DSM_RNIC_SessionType {
    DSM_RNIC_SESSION_TYPE_INVALID = 0,
    DSM_RNIC_SESSION_TYPE_IPV4    = 1,
    DSM_RNIC_SESSION_TYPE_IPV6    = 2,
    DSM_RNIC_SESSION_TYPE_IPV4V6  = 3,
    DSM_RNIC_SESSION_TYPE_ETHER   = 4,

    DSM_RNIC_SESSION_TYPE_BUTT
};
typedef VOS_UINT32 DSM_RNIC_SessionTypeUint32;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    DSM_RNIC_SessionTypeUint32 sessType;

    VOS_UINT8 ifaceId; /* 全局网卡ID */
    union {
        VOS_UINT8 rabId;        /* 承载号, 范围[5, 15] */
        VOS_UINT8 pduSessionId; /* PduSession ID */
    };
    ModemIdUint16 modemId; /* Modem ID */

    VOS_UINT32 ipv4Addr; /* IPV4的IP地址，主机序 */
    /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8 ipv6Addr[RNICITF_MAX_IPV6_ADDR_LEN];
} DSM_RNIC_IfaceCfgInd;


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    DSM_RNIC_SessionTypeUint32 sessType;
    VOS_UINT8 ifaceId; /* 全局网卡ID */
    VOS_UINT8 rsv[3];
} DSM_RNIC_IfaceRelInd;


enum Napi_Weight_AdjMode {
    STATIC_MODE  = 0x00, /* 静态调整模式 */
    DYNAMIC_MODE = 0x01, /* 动态调整模式 */

    MODE_BUTT            /* 无效值 */
};
typedef VOS_UINT8 ModeUint8;


typedef struct {
    VOS_UINT32 dlPpsLevel1; /* RNIC网卡每秒下行报文数档位1 */
    VOS_UINT32 dlPpsLevel2; /* RNIC网卡每秒下行报文数档位2 */
    VOS_UINT32 dlPpsLevel3; /* RNIC网卡每秒下行报文数档位3 */
    VOS_UINT32 dlPpsLevel4; /* RNIC网卡每秒下行报文数档位4 */
    VOS_UINT32 dlPpsLevel5; /* RNIC网卡每秒下行报文数档位5 */
    VOS_UINT32 reserved0;   /* 保留位 */
    VOS_UINT32 reserved1;   /* 保留位 */
    VOS_UINT32 reserved2;   /* 保留位 */

} NAPI_DlPpsLevel;


typedef struct {
    VOS_UINT8 napiWeightLevel1;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位1 */
    VOS_UINT8 napiWeightLevel2;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位2 */
    VOS_UINT8 napiWeightLevel3;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位3 */
    VOS_UINT8 napiWeightLevel4;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位4 */
    VOS_UINT8 napiWeightLevel5;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位5 */
    VOS_UINT8 reserved0;         /* 保留位 */
    VOS_UINT8 reserved1;         /* 保留位 */
    VOS_UINT8 reserved2;         /* 保留位 */

} NAPI_WeightLevel;


typedef struct {
    NAPI_DlPpsLevel  dlPktNumPerSecLevel; /* RNIC网卡每秒下行报文数档位 */
    NAPI_WeightLevel napiWeightLevel;     /* RNIC网卡对应每秒下行报文数NAPI Weight值档位 */

} NAPI_WeightDynamicAdjCfg;


typedef struct {
    VOS_UINT8                 napiEnable;
    ModeUint8                 napiWeightAdjMode;
    VOS_UINT8                 napiPollWeight;
    VOS_UINT8                 groEnable;
    VOS_UINT16                napiPollQueMaxLen;       /* RNIC Poll队列支持的最大长度 */
    VOS_UINT8                 reserved1;               /* 保留位 */
    VOS_UINT8                 reserved2;               /* 保留位 */
    NAPI_WeightDynamicAdjCfg  napiWeightDynamicAdjCfg; /* Napi Weight动态调整配置 */
} Rnic_NapiCfg;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgId;

    Rnic_NapiCfg        napiCfg;
} DSM_RNIC_NapiCfgInd;


typedef struct {
    VOS_UINT32 pps;                                   /* RNIC网卡下行每秒收包个数档位值 */
    VOS_UINT8  cpusWeight[RNIC_NV_NAPI_LB_MAX_CPUS];  /* 参与NAPI负载均衡的CPU Weight值档位 */

} NAPI_LbLevelCfg;


typedef struct {
    VOS_UINT8            napiLbEnable;                                 /* NAPI负载均衡功能开关 */
    VOS_UINT8            napiLbValidLevel;                             /* Napi负载均衡有效档位 */
    VOS_UINT16           napiLbCpumask;                                /* 参与Napi负载均衡的CPU掩码 */
    VOS_UINT8            reserved0;                                    /* 保留位 */
    VOS_UINT8            reserved1;                                    /* 保留位 */
    VOS_UINT8            reserved2;                                    /* 保留位 */
    VOS_UINT8            reserved3;                                    /* 保留位 */
    VOS_UINT32           reserved4;                                    /* 保留位 */
    VOS_UINT32           reserved5;                                    /* 保留位 */
    NAPI_LbLevelCfg      napiLbLevelCfg[RNIC_NV_NAPI_LB_MAX_LEVEL];    /* Napi负载均衡档位配置 */
} Rnic_NapiLbCfg;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgId;

    Rnic_NapiLbCfg      napiLbCfg;
} DSM_RNIC_NapiLbCfgInd;


typedef struct {
    VOS_UINT32 totalPps;                       /* RNIC网卡下行每秒收包个数档位值 */
    VOS_UINT32 nonTcpPps;                      /* RNIC网卡下行每秒收非tcp包个数档位值 */
    VOS_UINT32 backlogQueLimitLen;             /* CPU backlog que限长 */
    VOS_UINT8  congestionCtrl;                 /* 拥塞控制开关 */
    VOS_UINT8  rpsBitMask;                     /* RPS CPU掩码 */
    VOS_UINT8  reserved1;                      /* 保留位 */
    VOS_UINT8  isolationDisable;               /* 关闭isolation标识 */
    VOS_INT32  ddrBd;                          /* DDR带宽档位 */
    VOS_UINT32 freqReq[RNIC_NV_MAX_CLUSTER];   /* 请求调整CLUSTER主频值 */

} RNIC_RhcLevel2;


typedef struct {
    VOS_UINT8        rhcFeature;                    /* 接收高速控制特性开关 */
    VOS_UINT8        qccEnable;                     /* NAPI队列拥塞控制功能开关 */
    VOS_UINT8        napiWtExpEnable;               /* NAPI队列拥塞控制Weight扩张功能开关 */
    VOS_UINT8        ddrReqEnable;                  /* 根据NAPI队列拥塞程度给DDR投票功能开关 */
    VOS_UINT8        qccTimeout;                    /* NAPI队列拥塞控制持续时长,单位:100ms */
    VOS_UINT8        napiPollMax;                   /* NAPI poll最大连续执行次数 */
    VOS_UINT8        rpsMaskEnable;                 /* RPS Mask动态更新功能开关 */
    VOS_UINT8        reserved0;                     /* 保留位 */
    VOS_INT32        ddrMidBd;                      /* 中档位DDR带宽 */
    VOS_INT32        ddrHighBd;                     /* 高档位DDR带宽 */
    RNIC_RhcLevel2 levelCfg[RNIC_NV_RHC_MAX_LEVEL]; /* 接收高速控制档位配置 */

} Rnic_RhcCfg;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32     msgId;

    Rnic_RhcCfg    rhcCfg;
} DSM_RNIC_RhcCfgInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __DSM_RNIC_PIF_H__ */
