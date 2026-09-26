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

#ifndef __MTA_COMM_INTERFACE_H__
#define __MTA_COMM_INTERFACE_H__

#include "vos.h"
#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* network monitor GSM临区 上报最大个数 */
#define NETMON_MAX_GSM_NCELL_NUM 6

/* network monitor UTRAN 临区 上报最大个数 */
#define NETMON_MAX_UTRAN_NCELL_NUM 16

/* network monitor LTE 临区 上报最大个数 */
#define NETMON_MAX_LTE_NCELL_NUM 16

/* network monitor NR 临区 上报最大个数 */
#define NETMON_MAX_NR_NCELL_NUM 16

#define NETMON_MAX_NR_CC_NUM 8

#define MTA_LRRC_MSG_TYPE_BASE 0x1000 /* MTA模块与LTE接入层消息基数 */
#define MTA_COMM_MSG_TYPE_BASE 0x2000 /* MTA模块和其他模块公共消息基数 */
#define MTA_NRRC_MSG_TYPE_BASE 0x3000

#define MTA_NV_REFRESH_FAIL_PID_MAX_NUM 64 /* 最多携带64个模块NV REFRESH失败事件 */

enum MTA_RRC_GsmBand {
    MTA_RRC_GSM_BAND_850 = 0x00,
    MTA_RRC_GSM_BAND_900,
    MTA_RRC_GSM_BAND_1800,
    MTA_RRC_GSM_BAND_1900,

    MTA_RRC_GSM_BAND_BUTT
};
typedef VOS_UINT16 MTA_RRC_GsmBandUint16;


enum MTA_NetmonCellType {
    MTA_NETMON_SCELL_TYPE  = 0,
    MTA_NETMON_NCELL_TYPE  = 1,
    MTA_NETMON_SSCELL_TYPE = 2,
    MTA_NETMON_CELL_TYPE_BUTT
};

typedef VOS_UINT32 MTA_NetMonCellTypeUint32;


enum MTA_NETMON_UtranType {
    MTA_NETMON_UTRAN_FDD_TYPE = 0,
    MTA_NETMON_UTRAN_TDD_TYPE = 1,
    MTA_NETMON_UTRAN_TYPE_BUTT
};
typedef VOS_UINT32 MTA_NETMON_UtranTypeUint32;


enum MTA_NETMON_Result {
    MTA_NETMON_RESULT_NO_ERR = 0,
    MTA_NETMON_RESULT_ERR    = 1,
    MTA_NETMON_RESULT_BUTT
};
typedef VOS_UINT32 MTA_NETMON_ResultUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: 当前向接入层查询小区的模块枚举值
 */
enum MTA_NETMON_CellQryModule {
    MTA_NETMON_CELL_QRY_MODULE_AT  = 0x00, /* AT模块向接入层查询小区信息，AT^MONSC/AT^MONNC触发 */
    MTA_NETMON_CELL_QRY_MODULE_MTC = 0x01, /* MTC模块向接入层查询小区信息 */
    MTA_NRCELL_BAND_QRY_MODULE_AT  = 0x02, /* AT模块向接入层查询NR Band信息，AT^NRCELLBAND触发 */
    MTA_NETMON_CELL_QRY_MODULE_BUTT
};
typedef VOS_UINT32 MTA_NETMON_CellQryModuleUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 枚举说明: NV REFRESH的原因
 */
enum MTA_NvRefreshReason {
    MTA_NV_REFRESH_REASON_USIM_DEPENDENT = 0x00, /* 随卡匹配 */
    MTA_NV_REFRESH_REASON_BUTT
};
typedef VOS_UINT8 MTA_NvRefreshReasonUint8;

#if (FEATURE_LTEV == FEATURE_ON)

enum TAF_MTA_VModeRatType {
    TAF_MTA_VMODE_RAT_LTEV = 0x00,
    TAF_MTA_VMODE_RAT_BUTT
};
typedef VOS_UINT8 TAF_MTA_VModeRatTypeUint8;


enum TAF_MTA_VModePwModeType {
    TAF_MTA_VMODE_POWEROFF = 0x00,
    TAF_MTA_VMODE_POWERON  = 0x01,
    TAF_MTA_VMODE_PWMODE_BUTT
};
typedef VOS_UINT8 TAF_MTA_VModePwModeTypeUint8;


enum TAF_MTA_ResultCode {
    TAF_MTA_OK                     = 0, /* 成功 */
    TAF_MTA_ERR_INPUT_NULL_PTR     = 1, /* 空指针 */
    TAF_MTA_ERR_ALLOC_FAIL         = 2, /* 申请内存失败 */
    TAF_MTA_ERR_SEND_FAIL          = 3, /* 发送消息失败 */
    TAF_MTA_ERR_TIMER_STATUS_WRONG = 4, /* 定时器状态错误 */
    TAF_MTA_ERR_PARA_INCORRECT     = 5, /* 参数错误 */
    TAF_MTA_ERR_TIMER_EXPIRED      = 6, /* 定时器超时 */
    TAF_MTA_ERR_UNKNOWN            = 7, /* 未知错误 */
};
#endif
enum MTA_COMM_MsgType {
    /* MTA->CC, MTA->RABM, MTA->VC, MTA->TAF, MTA->DSM, MTA->CCM, MTA->MMA, MTA->IMSA, MTA->MSCC, MTA->CSS,
       MTA->ERRC, MTA->LAPDM, MTA->RLC, MTA->LL, MTA->ERRC, MTA->LMM */
    /* _MSGPARSE_Interception MTA_NV_RefreshInd */
    ID_MTA_NV_REFRESH_IND = MTA_COMM_MSG_TYPE_BASE + 0x01,
    /* CC->MTA, RABM->MTA, VC->MTA, TAF->MTA, DSM->MTA, CCM->MTA */
    /* _MSGPARSE_Interception MTA_NV_RefreshRsp */
    ID_MTA_NV_REFRESH_RSP = MTA_COMM_MSG_TYPE_BASE + 0x02,
    /* A_TAF->MTA */
#if (FEATURE_LTEV == FEATURE_ON)
    /* _MSGPARSE_Interception TAF_MTA_VMODE_SetReq */
    ID_TAF_MTA_VMODE_SET_REQ = MTA_COMM_MSG_TYPE_BASE + 0x03, /* VMODE开关机设置请求 */
    /* MTA->AT */
    /* _MSGPARSE_Interception TAF_MTA_VMODE_SetCnf */
    ID_TAF_MTA_VMODE_SET_CNF = MTA_COMM_MSG_TYPE_BASE + 0x04, /* VMODE开关机设置回复 */
    /* A_TAF->MTA */
    /* _MSGPARSE_Interception TAF_MTA_VMODE_QryReq */
    ID_TAF_MTA_VMODE_QRY_REQ = MTA_COMM_MSG_TYPE_BASE + 0x05, /* VMODE开关机查询请求 */
    /* MTA->AT */
    /* _MSGPARSE_Interception TAF_MTA_VMODE_QryCnf */
    ID_TAF_MTA_VMODE_QRY_CNF = MTA_COMM_MSG_TYPE_BASE + 0x06, /* VMODE开关机查询回复 */
#endif
    /* MTA->MMC, MTA->LRRC,, MTA->NRRC, */
    /* MTA_UeAiModeStatusNtf */
    /* _MSGPARSE_Interception MTA_UeAiModeStatusNtf */
    ID_MTA_UE_AI_MODE_STATUS_NTF = MTA_COMM_MSG_TYPE_BASE + 0x07,
    ID_MTA_COMM_MSG_TYPE_BUTT

};
typedef VOS_UINT32 MTA_COMM_MsgTypeUint32;

/* ========以下是接入层与MTA之间的消息结构体======== */
/* *************************network monitor查询临区信息结构体部分************************************************ */
/* MTA发给接入层的消息，通用，不区分2G/3G/服务小区/邻区 */
typedef struct {
    VOS_MSG_HEADER                      /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32               msgName;   /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NetMonCellTypeUint32 celltype;  /* 0:查询服务小区，1:查询临区 */
    VOS_UINT32               qryModule; /* 当前查询的模块信息 */
} MTA_RRC_NetmonCellQryReq;

/* GSM 临区结构 */
typedef struct {
    VOS_UINT32            opBsic : 1;
    VOS_UINT32            opCellId : 1;
    VOS_UINT32            opLac : 1;
    VOS_UINT32            opSpare : 29;
    VOS_UINT32            cellId; /* 小区ID */
    VOS_UINT16            lac;    /* 位置区码 */
    VOS_UINT16            afrcn;  /* 频点 */
    VOS_INT16             rssi;   /* 频点的RSSI */
    MTA_RRC_GsmBandUint16 band;   /* band 0-3 */
    VOS_UINT8             bsic;   /* 小区基站码 */
    VOS_UINT8             reserved[3];
} MTA_NetmonGsmNcellInfo;

/* LTE 临区结构,暂时定义的数据结构，根据需要进行调整 */
typedef struct {
    VOS_UINT32 pid;   /* 物理小区ID */
    VOS_UINT32 arfcn; /* 频点 */
    VOS_INT16  rsrp;  /* RSRP参考信号接收功率 */
    VOS_INT16  rsrq;  /* RSRQ参考信号接收质量 */
    VOS_INT16  rssi;  /* Receiving signal strength in dbm */
    VOS_UINT16 bandInd; /* 频带指示 */
} MTA_NETMON_LteNcellInfo;

/* WCDMA 临区结构 */
typedef struct {
    VOS_UINT16 arfcn; /* 频点 */
    VOS_UINT16 psc;   /* 主扰码 */
    VOS_INT16  ecn0;  /* ECN0 */
    VOS_INT16  rscp;  /* RSCP */
} MTA_NETMON_UtranNcellInfoFdd;

/* TD_SCDMA 临区结构,暂时定义结构，根据需要继续调整 */
typedef struct {
    VOS_UINT16 arfcn;  /* 频点 */
    VOS_UINT16 sc;     /* 扰码 */
    VOS_UINT16 syncId; /* 下行导频码 */
    VOS_INT16  rscp;   /* RSCP */
} MTA_NETMON_UtranNcellInfoTdd;

/* NR 邻区结构 */
typedef struct {
    VOS_UINT32 pid;   /* 物理小区ID */
    VOS_UINT32 arfcn; /* 频点 */
    VOS_INT16  rsrp;  /* RSRP参考信号接收功率 */
    VOS_INT16  rsrq;  /* RSRQ参考信号接收质量 */
    VOS_INT16  sinr;
    VOS_UINT16 bandInd; /* 频带指示 */
} MTA_NETMON_NrNcellInfo;

/* 临区数据结构 */
typedef struct {
    VOS_UINT8 gsmNCellCnt;   /* GSM 临区个数 */
    VOS_UINT8 utranNCellCnt; /* WCDMA 临区个数 */
    VOS_UINT8 lteNCellCnt;   /* LTE 临区个数 */
    VOS_UINT8 nrNCellCnt;    /* NR 临区个数 */
    /* GSM 临区数据结构 */
    MTA_NetmonGsmNcellInfo     gsmNCellInfo[NETMON_MAX_GSM_NCELL_NUM];
    MTA_NETMON_UtranTypeUint32 cellMeasTypeChoice; /* NETMON频率信息类型:FDD,TDD */
    union {
        /* FDD临区数据结构 */
        MTA_NETMON_UtranNcellInfoFdd fddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM];
        /*  TDD临区数据结构 */
        MTA_NETMON_UtranNcellInfoTdd tddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM];
    } u;
    /* LTE 临区数据结构 */
    MTA_NETMON_LteNcellInfo lteNCellInfo[NETMON_MAX_LTE_NCELL_NUM];
    /* NR 临区数据结构 */
    MTA_NETMON_NrNcellInfo nrNCellInfo[NETMON_MAX_NR_NCELL_NUM];
} RRC_MTA_NetmonNcellInfo;

/* ********************network monitor查询GSM 小区信息部分*********************************************** */
typedef struct {
    VOS_MSG_HEADER          /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32     msgName; /* 消息名称 */ /* _H2ASN_Skip */
} MTA_GRR_NetmonTaQryReq;


typedef struct {
    VOS_MSG_HEADER                       /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32              msgName;     /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NETMON_ResultUint32 result;      /* 返回结果 */
    VOS_UINT16              ta;          /* 返回的TA值 */
    VOS_UINT8               reserved[2]; /* 四字节对齐的保留位 */
} GRR_MTA_NetmonTaQryCnf;


typedef struct {
    VOS_UINT32            mcc;    /* 移动国家码 */
    VOS_UINT32            mnc;    /* 移动网络码 */
    VOS_UINT32            cellId; /* 小区ID */
    VOS_UINT16            lac;    /* 位置区码 */
    VOS_UINT16            arfcn;  /* 绝对频点号 */
    VOS_INT16             rssi;   /* Receiving signal strength in dbm */
    MTA_RRC_GsmBandUint16 band;   /* GSM频段(0-3) */
    VOS_UINT8             bsic;   /* 小区基站码 */
    /* IDLE态下或者PS数传态下无效,专用态下填充信道质量值，范围[0,7] ,无效值99 */
    VOS_UINT8 rxQuality;
    VOS_UINT8 reserved[2]; /* 四字节对齐的保留位 */
} GRR_MTA_NetmonScellInfo;


typedef struct {
    VOS_MSG_HEADER                         /* _H2ASN_Skip */
    VOS_UINT32               msgName;      /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NETMON_ResultUint32  result;
    MTA_NetMonCellTypeUint32 celltype;     /* 0:查询服务小区，1:查询邻区 */
    VOS_UINT32               qryModule;    /* 当前查询的模块信息 */
    union {
        GRR_MTA_NetmonScellInfo scellinfo; /* GSM下的服务小区信息 */
        RRC_MTA_NetmonNcellInfo ncellinfo; /* GSM下的邻区信息 */
    } u;
} GRR_MTA_NetmonCellQryCnf;

/* *****************************network monitor查询UTRAN 小区信息部分************************************************/
/* FDD 服务小区信息结构 */
typedef struct {
    VOS_UINT32 opDrx : 1;
    VOS_UINT32 opUra : 1;
    VOS_UINT32 opSpare : 30;
    VOS_UINT32 drx;  /* Discontinuous reception cycle length */
    VOS_INT16  ecn0; /* ECN0 */
    VOS_INT16  rssi; /* Receiving signal strength in dbm */
    VOS_INT16  rscp; /* Received Signal Code Power in dBm，接收信号码功率 */
    VOS_UINT16 psc;  /* 主扰码 */
    VOS_UINT16 ura;  /* UTRAN Registration Area Identity */
    VOS_UINT8  reserved[2];

} MTA_NETMON_UtranScellInfoFdd;

typedef struct {
    VOS_UINT32 drx;    /* Discontinuous reception cycle length */
    VOS_UINT16 sc;     /* 扰码 */
    VOS_UINT16 syncID; /* 下行导频码 */
    VOS_UINT16 rac;    /* RAC */
    VOS_INT16  rscp;   /* RSCP */
} MTA_NETMON_UtranScellInfoTdd;

/* UTRAN 服务小区结构 */
typedef struct {
    VOS_UINT32                 opCellID : 1;
    VOS_UINT32                 opLAC : 1;
    VOS_UINT32                 opSpare : 30;
    VOS_UINT32                 mcc;                /* 移动国家码 */
    VOS_UINT32                 mnc;                /* 移动网络码 */
    VOS_UINT32                 cellID;             /* 小区ID */
    VOS_UINT16                 arfcn;              /* 频点 */
    VOS_UINT16                 lac;                /* 位置区码 */
    MTA_NETMON_UtranTypeUint32 cellMeasTypeChoice; /* NETMON频率信息类型:FDD,TDD */
    union {
        MTA_NETMON_UtranScellInfoFdd cellMeasResultsFDD; /* FDD */
        MTA_NETMON_UtranScellInfoTdd cellMeasResultsTDD; /*  TDD */
    } u;
} RRC_MTA_NetmonUtranScellInfo;


typedef struct {
    VOS_MSG_HEADER                      /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32               msgName;   /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NETMON_ResultUint32  result;
    MTA_NetMonCellTypeUint32 celltype;  /* 0:查询服务小区，1:查询临区 */
    VOS_UINT32               qryModule; /* 当前查询的模块信息 */
    union {
        RRC_MTA_NetmonUtranScellInfo scellinfo; /* UTRAN下的服务小区信息 */
        RRC_MTA_NetmonNcellInfo      ncellinfo; /* UTRAN下的邻区信息 */
    } u;
} RRC_MTA_NetmonCellInfoQryCnf;

/* LTE 服务小区结构 */
typedef struct {
    VOS_UINT32 mcc;    /* 移动国家码 */
    VOS_UINT32 mnc;    /* 移动网络码 */
    VOS_UINT32 cellID; /* 小区ID */
    VOS_UINT32 pid;    /* 物理小区ID */
    VOS_UINT32 arfcn;  /* 频点 */
    VOS_UINT16 tac;
    VOS_INT16  rsrp;
    VOS_INT16  rsrq;
    VOS_INT16  rssi;
} MTA_NETMON_EutranScellInfo;


typedef struct {
    VOS_MSG_HEADER                      /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32               msgName;   /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NetMonCellTypeUint32 celltype;  /* 0：查询服务小区，1：查询临区 */
    VOS_UINT32               qryModule; /* 当前查询的模块信息 */
} MTA_LRRC_NetmonCellQryReq;


typedef struct {
    VOS_MSG_HEADER                      /* 消息头 */ /* _H2ASN_Skip */
    VOS_UINT32               msgName;   /* 消息名称 */ /* _H2ASN_Skip */
    MTA_NETMON_ResultUint32  result;
    MTA_NetMonCellTypeUint32 celltype;  /* 0:查询服务小区，1:查询临区 */
    VOS_UINT32               qryModule; /* 当前查询的模块信息 */
    union {
        MTA_NETMON_EutranScellInfo scellinfo; /* LTE下的服务小区信息 */
        RRC_MTA_NetmonNcellInfo    ncellinfo; /* LTE下的邻区信息 */
    } u;
} LRRC_MTA_NetmonCellInfoQryCnf;


typedef struct {
    VOS_MSG_HEADER                    /* _H2ASN_Skip */
    VOS_UINT32               msgName; /* _H2ASN_Skip */
    MTA_NvRefreshReasonUint8 reason;  /* NV REFRESH原因 */
    VOS_UINT8                rsv[3];
} MTA_NV_RefreshInd;


typedef struct {
    VOS_MSG_HEADER          /* _H2ASN_Skip */
    VOS_UINT32     msgName; /* _H2ASN_Skip */
    VOS_UINT32     rslt;    /* VOS_OK:成功，其他:失败 */
} MTA_NV_RefreshRsp;

typedef struct {
    VOS_UINT8      pidNum;                                  /* NV REFRESH失败事件PID个数 */
    VOS_UINT8      rsv[3];                                  /* 保留位 */
    VOS_UINT32     pid[MTA_NV_REFRESH_FAIL_PID_MAX_NUM]; /* NV REFRESH失败事件PID */
} MTA_NV_RefreshPidInfo;

#if (FEATURE_LTEV == FEATURE_ON)
/*
 * 当前TAF_MTA_Ctrl结构只有VMODE_SetReq 和VMODE_QryReq使用，后续准备删除
 */
typedef struct {
    VOS_UINT16 clientId; /* moduleId是AT的，填入AT对应的clientId；非AT的，填0 */
    VOS_UINT8  opId;
    VOS_UINT8  reserved;

    /* AT处理MTA消息时，先获取clientid，此处把moduleId挪到下方 */
    VOS_UINT32 moduleId; /* 填入PID */
} TAF_MTA_Ctrl;

typedef struct {
    VOS_MSG_HEADER                        /* _H2ASN_Skip */
    VOS_UINT32                   msgName; /* _H2ASN_Skip */
    TAF_MTA_Ctrl                 ctrl;    /* 控制端口信息 */
    TAF_MTA_VModeRatTypeUint8    rat;     /* 暂时只支持LTE制式 */
    TAF_MTA_VModePwModeTypeUint8 mode;    /* 0表示关机，1表示开机 */
    VOS_UINT8                    reserved[2];
} TAF_MTA_VMODE_SetReq;


typedef struct {
    VOS_MSG_HEADER          /* _H2ASN_Skip */
    VOS_UINT32     msgName; /* _H2ASN_Skip */
    TAF_MTA_Ctrl   ctrl;    /* 控制端口信息 */
    VOS_UINT32     result;  /* TAF_MTA_OK:成功，其他:失败 */
} TAF_MTA_VMODE_SetCnf;


typedef struct {
    VOS_MSG_HEADER          /* _H2ASN_Skip */
    VOS_UINT32     msgName; /* _H2ASN_Skip */
    TAF_MTA_Ctrl   ctrl;    /* 控制端口信息 */
} TAF_MTA_VMODE_QryReq;


typedef struct {
    VOS_MSG_HEADER                        /* _H2ASN_Skip */
    VOS_UINT32                   msgName; /* _H2ASN_Skip */
    TAF_MTA_Ctrl                 ctrl;    /* 控制端口信息 */
    VOS_UINT32                   result;  /* TAF_MTA_OK:成功，其他:失败 */
    TAF_MTA_VModePwModeTypeUint8 mode;    /* 0表示关机，1表示开机 */
    VOS_UINT8                    reserved[3];
} TAF_MTA_VMODE_QryCnf;
#endif


typedef struct {

    VOS_UINT16                originalId;
    VOS_UINT16                terminalId;
    VOS_UINT32                timeStamp;
    VOS_UINT32                sn;
} MTA_OmHeader;


typedef struct {
    VOS_UINT32                moudleId;
    union {
        AT_APPCTRL            appHeader;
        MTA_OmHeader          omHeader;
    } u;

} MTA_AtOmCommHeader;


enum MTA_UeAiModeRegType {
    MTA_UE_AI_MODE_REG_TYPE_MOTION = 0x00, /* 运动状态注册 */
    MTA_UE_AI_MODE_REG_TYPE_SCENE  = 0x01, /* 场景状态注册 */
    MTA_UE_AI_MODE_REG_TYPE_BUTT
};
typedef VOS_UINT32 MTA_UeAiModeRegTypeUint32;


enum MTA_MotionStatSupport {
    MTA_MOTION_STAT_SUPPORT_UNKNOWN         = 0x00000000, /* UNKNOWN状态 */
    MTA_MOTION_STAT_SUPPORT_IN_VEHICLE      = 0x00000001, /* 支持车载 */
    MTA_MOTION_STAT_SUPPORT_RIDING          = 0x00000002, /* 支持骑车 */
    MTA_MOTION_STAT_SUPPORT_WALK_SLOW       = 0x00000004, /* 支持慢走 */
    MTA_MOTION_STAT_SUPPORT_RUN_FAST        = 0x00000008, /* 支持快跑 */
    MTA_MOTION_STAT_SUPPORT_STATIONARY      = 0x00000010, /* 支持静止 */
    MTA_MOTION_STAT_SUPPORT_TILT            = 0x00000020, /* 支持倾斜 */
    MTA_MOTION_STAT_SUPPORT_END             = 0x00000040,
    MTA_MOTION_STAT_SUPPORT_BUS             = 0x00000080, /* 支持公交 */
    MTA_MOTION_STAT_SUPPORT_CAR             = 0x00000100, /* 支持小车 */
    MTA_MOTION_STAT_SUPPORT_METRO           = 0x00000200, /* 支持地铁 */
    MTA_MOTION_STAT_SUPPORT_HIGH_SPEED_RAIL = 0x00000400, /* 支持高铁 */
    MTA_MOTION_STAT_SUPPORT_AUTO            = 0x00000800, /* 支持公路交通 */
    MTA_MOTION_STAT_SUPPORT_RAIL            = 0x00001000, /* 支持铁路交通 */
    MTA_MOTION_STAT_SUPPORT_CLIMBING_MOUNT  = 0x00002000, /* 支持爬山 */
    MTA_MOTION_STAT_SUPPORT_FAST_WALK       = 0x00004000, /* 支持快走 */
    MTA_MOTION_STAT_SUPPORT_STOP_VEHICLE    = 0x00008000, /* 支持停车 */
    MTA_MOTION_STAT_SUPPORT_ELEVATOR        = 0x00010000, /* 支持电梯 */
    MTA_MOTION_STAT_SUPPORT_RELATIVE_STILL  = 0x00020000, /* 支持微动 */
    MTA_MOTION_STAT_SUPPORT_GARAGE          = 0x00040000, /* 支持车库 */
};
typedef VOS_UINT32 MTA_MotionStatSupportUint32;


enum MTA_UeSceneStat {
    MTA_UE_SCENE_STAT_NORMAL = 0x00000000,
    MTA_UE_SCENE_STAT_SLEEP  = 0x00000001, /* 进入睡眠模式 */
    MTA_UE_SCENE_STAT_BUTT
};
typedef VOS_UINT32 MTA_UeSceneStatUint32;


typedef struct {
    VOS_MSG_HEADER      /* _H2ASN_Skip */
    VOS_UINT32 msgName; /* _H2ASN_Skip */
    VOS_UINT32 motionStatus; /* 0为普通状态，非0取值参考MTA_MotionStatSupportUint32枚举值定义(按位或) */
    VOS_UINT32 sceneStatus;  /* 0为普通状态，非0取值参考MTA_UeSceneStatUint32枚举值定义(按位或) */
} MTA_UeAiModeStatusNtf;

VOS_UINT32 NAS_RegNvRefreshIndMsg(ModemIdUint16 modemId, VOS_UINT32 regPid);

/*
 * 1. MODEM_FUSION_VERSION宏开时NRRC使用注册机制，宏关时无须注册默认给NRRC发送消息
 * 2. 使用注册机制的组件需要知会TAF新增消息解析，如未知会使用注册机制TAF不处理消息解析问题
 */
VOS_UINT32 TAF_MTA_RegUeAiModeStatusNtfMsg(ModemIdUint16 modemId, VOS_UINT32 regPid,
    MTA_UeAiModeRegTypeUint32 regModeStatus);

#if (FEATURE_LTEV == FEATURE_ON)
VOS_UINT32 TAF_MTA_VModeSetReq(TAF_MTA_Ctrl *ctrl, TAF_MTA_VModeRatTypeUint8 rat, TAF_MTA_VModePwModeTypeUint8 mode);
VOS_UINT32 TAF_MTA_VModeQryReq(TAF_MTA_Ctrl *ctrl);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mta_comm_interface.h */
