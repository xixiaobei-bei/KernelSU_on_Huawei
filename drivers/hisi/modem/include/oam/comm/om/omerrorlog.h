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



#ifndef __OM_ERRORLOG_H__
#define __OM_ERRORLOG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* 商用ERR log 上报相关 */
#define     ID_OM_ERR_LOG_CTRL_IND          (0x9001)
#define     ID_OM_ERR_LOG_REPORT_REQ        (0x9002)
#define     ID_OM_ERR_LOG_REPORT_CNF        (0x9003)


/* 平台检测故障主动上报消息名称 */
#define     ID_OM_FAULT_ERR_LOG_IND         (0x9009) /* haojian 提议该处以后可能不止媒体组，还有其他组，所有改名 */

/* 平台检测告警主动上报消息名称 */
#define     ID_OM_ALARM_ERR_LOG_IND         (0x900A)

/* 业务信息收集相关 */
#define     ID_OM_INFO_CLT_REPORT_REQ       (0x9010)
#define     ID_OM_INFO_CLT_REPORT_CNF       (0x9011)


/*OM内部a核和c核使用*/
#define OM_ACPU_REPORT_BLACKLIST_NAME  0x8001 
#define OM_ACPU_REPORT_PRIORITY_NAME   0x8002
#define OM_ACPU_REPORT_PERIOD_NAME     0x8003
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum OM_ERR_LOG_MSG_TYPE_ENUM
{
    OM_ERR_LOG_MSG_SET_ERR_REPORT = 0x01, /* 上层触发故障上报 */
    OM_ERR_LOG_MSG_ERR_REPORT_END = 0x02, /* 故障上报结束 */
    OM_ERR_LOG_MSG_ON_OFF         = 0x03, /* 控制上类型开关 */
    OM_ERR_LOG_MSG_ERR_REPORT     = 0x04, /* 故障上报 */
    OM_ERR_LOG_MSG_FTM_REPORT     = 0x05, /* 工程模式主动上报 */
    OM_ERR_LOG_MSG_RESULT         = 0x06, /* 上报Result */
    OM_ERR_LOG_MSG_FTM_REQ        = 0x07, /* 工程模式命令 */
    OM_ERR_LOG_MSG_FTM_CNF        = 0x08, /* 工程模式响应 */

    OM_ERR_LOG_MSG_FAULT_REPORT   = 0x09, /* 平台检测故障主动上报 */

    OM_ERR_LOG_MSG_ALARM_REPORT   = 0x0A, /* 平台检测告警主动上报 */

    OM_ERR_LOG_MSG_INFO_CLT_REQ   = 0x10, /* 信息收集命令 */
    OM_ERR_LOG_MSG_INFO_CLT_CNF   = 0x11, /* 信息收集响应 */
    OM_ERR_LOG_MSG_INFO_CLT_END   = 0x12,

    OM_ERR_LOG_MSG_SMARTCDRX_REPORT = 0x13, /* SmartCDRX维测信息主动上报 */
    OM_ERR_LOG_MSG_BLACK_LIST     = 0x14,
    OM_ERR_LOG_MSG_PRIORITY_CFG   = 0x15,
    OM_ERR_LOG_MSG_PERIOD_CFG     = 0x16,
    OM_ERR_LOG_MSG_CFG_CNF        = 0x17,
     
    OM_ERR_LOG_MSG_TYPE_BUTT
};
/* 开关变量 */
enum OM_APP_SWITCH_ENUM
{
    OM_APP_STATUS_CLOSE = 0x00,
    OM_APP_STATUS_OPEN  = 0x01,

    OM_APP_STATUS_BUTT
};
typedef VOS_UINT32    OM_ERR_LOG_MSG_TYPE_ENUM_UINT32;
typedef VOS_UINT16    OM_ERR_LOG_MSG_TYPE_ENUM_UINT16;

enum OM_ERR_LOG_MOUDLE_ID_ENUM
{
    OM_ERR_LOG_MOUDLE_ID_GUNAS  = 0x020001,   /* GU NAS */
    OM_ERR_LOG_MOUDLE_ID_GAS    = 0x020002,   /* GAS */
    OM_ERR_LOG_MOUDLE_ID_WAS    = 0x020003,   /* WAS */
    OM_ERR_LOG_MOUDLE_ID_GUL2   = 0x020004,   /* GUL2 */
    OM_ERR_LOG_MOUDLE_ID_GUPHY  = 0x020005,   /* GUPHY */
    OM_ERR_LOG_MOUDLE_ID_USIMM  = 0x020006,   /* USIMM */
    OM_ERR_LOG_MOUDLE_ID_DRV_SCI= 0x020007,   /* SCI DRV */
    OM_ERR_LOG_MOUDLE_ID_HIFI   = 0x020008,   /* HIFI */
    OM_ERR_LOG_MOUDLE_ID_RRM    = 0x020009,   /* RRM */
    OM_ERR_LOG_MOUDLE_ID_VISP   = 0x02000A,   /* VISP CHR */

    OM_ERR_LOG_MOUDLE_ID_GNAS   = 0x020011,   /* G NAS */
    OM_ERR_LOG_MOUDLE_ID_GL2    = 0x020014,   /* GL2 */
    OM_ERR_LOG_MOUDLE_ID_GPHY   = 0x020015,   /* GPHY */

    OM_ERR_LOG_MOUDLE_ID_UNAS   = 0x020021,   /* U NAS */
    OM_ERR_LOG_MOUDLE_ID_UL2    = 0x020024,   /* UL2 */
    OM_ERR_LOG_MOUDLE_ID_UPHY   = 0x020025,   /* UPHY */

    OM_ERR_LOG_MOUDLE_ID_LMM    = 0x020030,   /* LTE EMM */
    OM_ERR_LOG_MOUDLE_ID_ESM    = 0x020031,   /* LTE ESM */
    OM_ERR_LOG_MOUDLE_ID_LSMS   = 0x020032,   /* LTE SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_LPP    = 0x020033,   /* LTE LPP */
    OM_ERR_LOG_MOUDLE_ID_LRRC   = 0x020034,   /* LTE RRC */
    OM_ERR_LOG_MOUDLE_ID_LTEL2  = 0x020035,   /* LTEL2 */
    OM_ERR_LOG_MOUDLE_ID_LPHY   = 0x020036,   /* LTE PHY */

    OM_ERR_LOG_MOUDLE_ID_TDS_MM  = 0x020040,   /* TDS MM */
    OM_ERR_LOG_MOUDLE_ID_TDS_GMM = 0x020041,   /* TDS GMM */
    OM_ERR_LOG_MOUDLE_ID_TDS_CC  = 0x020042,   /* TDS CC */
    OM_ERR_LOG_MOUDLE_ID_TDS_SM  = 0x020043,   /* TDS SM */
    OM_ERR_LOG_MOUDLE_ID_TDS_SMS = 0x020044,   /* TDS SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_TDS_LPP = 0x020045,   /* TDS LPP */
    OM_ERR_LOG_MOUDLE_ID_TRRC    = 0x020046,   /* TDS RRC */
    OM_ERR_LOG_MOUDLE_ID_TDSL2   = 0x020047,   /* TDSL2 */
    OM_ERR_LOG_MOUDLE_ID_TDS_PHY = 0x020048,   /* TDS PHY */
    OM_ERR_LOG_MOUDLE_ID_LPSOM   = 0x020049,   /* LPS OM */

    /* 以下模块不具备收发消息功能，需借助COM模块中转 */
    OM_ERR_LOG_MOUDLE_ID_IMS     = 0x020050,   /* IMS */
    OM_ERR_LOG_MOUDLE_ID_IMSA    = 0x020051,   /* IMSA */

    OM_ERR_LOG_MOUDLE_ID_CNAS    = 0x020060,   /* CNAS */
    OM_ERR_LOG_MOUDLE_ID_CAS     = 0x020061,   /* CAS */
    OM_ERR_LOG_MOUDLE_ID_CL2     = 0x020062,   /* CL2 */
    OM_ERR_LOG_MOUDLE_ID_CPROC   = 0x020063,   /* CPROC */
    OM_ERR_LOG_MOUDLE_ID_CSDR    = 0x020064,   /* CSDR */
    OM_ERR_LOG_MOUDLE_ID_CHIFI   = 0x020065,   /* CHIFI */
    OM_ERR_LOG_MOUDLE_ID_PPPC    = 0x020066,   /* PPPC */
     /* 用于底软低功耗上报*/ 
    OM_ERR_LOG_MOUDLE_ID_DRV     = 0x020067,   /* DRV */
    OM_ERR_LOG_MOUDLE_ID_CHR_STA     = 0x020068, /*OM维测*/

    OM_ERR_LOG_MOUDLE_ID_BUTT
};
typedef VOS_UINT32    OM_ERR_LOG_MOUDLE_ID_ENUM_UINT32;


/* 错误码 */
enum OM_APP_SEND_RESULT_ENUM
{
    OM_APP_MSG_OK                   = 0x00,
    OM_APP_VCOM_DEV_INDEX_ERR       = 0x01,
    OM_APP_MSG_LENGTH_ERR           = 0x02,
    OM_APP_MSG_TYPE_ERR             = 0x03,
    OM_APP_SEND_MODEM_ID_ERR        = 0x04,
    OM_APP_OMACPU_ALLOC_MSG_ERR     = 0x05,
    OM_APP_OMACPU_WRITE_NV_ERR      = 0x06,
    OM_APP_OMACPU_READ_NV_ERR       = 0x07,
    OM_APP_MSG_MODULE_ID_ERR        = 0x08,
    OM_APP_SEND_FAULT_ID_ERR        = 0x09,
    OM_APP_NO_FAULT_ID_ERR          = 0x0A,
    OM_APP_REPORT_NOT_FINISH_ERR    = 0x0B,
    OM_APP_ERRLOG_SWITCH_CLOSE_ERR  = 0x0C,
    OM_APP_ERRLOG_START_TIMER_ERR   = 0x0D,
    OM_APP_OMCCPU_ALLOC_MSG_ERR     = 0x0E,
    OM_APP_PID_ALLOC_ERR            = 0x0F,
    OM_APP_GET_PID_ERR              = 0x10,
    OM_APP_FLAG_ALLOC_ERR           = 0x11,
    OM_APP_MEM_ALLOC_ERR            = 0x12,
    OM_APP_LENG_ERR                 = 0x13,
    OM_APP_PARAM_INAVALID           = 0x14,
    OM_APP_SEND_CCPU_FAIL           = 0x15,
    OM_APP_SAVE_LIST_FAIL           = 0x16,
    
    OM_APP_SEND_RESULT_BUTT
};

/* 各组件主动上报OM返回给各组件的错误码 */
enum  CHR_REPORT_RESULT_ENUM
{
    CHR_BLACK_LIST_NOT_ALLOW         =0x100,
    CHR_THRESHOLD_NOT_ALLOW          =0x101,
    CHR_MSG_NAME_ERR                 =0x102,
    CHR_INPUT_PARAM_NULL             =0x103,

    
    CHR_REPORT_RESULT_BUTT
};

enum OM_INFO_CLT_SCENE_TYPE_ENUM
{
    OM_INFO_CLT_SCENE_TYPE_FREQ_BAND                        = 0x4001,
    OM_INFO_CLT_SCENE_TYPE_NETWORK_INFO                     = 0x4002,

    OM_INFO_CLT_SCENE_TYPE_STATS_INFO                       = 0x4003,

    OM_INFO_CLT_SCENE_TYPE_BUTT
};
typedef VOS_UINT16    OM_INFO_CLT_SCENE_TYPE_ENUM_UINT16;

/* COMM提供给其他API接口组件的通知注册接口形式 */
typedef  VOS_UINT32 (*pFuncOMGetData)(OM_ERR_LOG_MOUDLE_ID_ENUM_UINT32 enProjectModule, VOS_VOID *pData, VOS_UINT32 ulLen);

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  APP和OM接口STRUCT定义
*****************************************************************************/
/* APP-->OM error log 通用消息头，OM私用，各组件不允许使用 */
typedef struct
{
    VOS_UINT32                          ulMsgType;
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulMsgLen;
}OM_ALARM_MSG_HEAD_STRU;


/* APP请求Error log上报type=0x01 App->OM */
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT32                          ulMsgModuleID;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usFaultId; /* 故障场景编号 */
}APP_OM_REQ_ERR_LOG_STRU;

/* Result To App type=0x06  OM -> App */
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT32                          ulMsgModuleID;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usRsv;
    VOS_UINT32                          ulStatus; /* result */
}OM_APP_RESULT_CNF_STRU;

/* Result To App type=0x17  OM -> App */
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT32                          ulStatus; /* result,app并不解析此状态，只检查type=0x17 */
}OM_APP_RESULT_CFG_CNF_STRU;
/* Error log 上报完成 type=0x02 OM-> APP */
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT32                          ulMsgModuleID;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usFaultID;
    VOS_UINT32                          ulReportStatus; /* result */
    VOS_UINT32                          ulTrigLowSlice; /* 记录触发上报请求时的时间戳 */
    VOS_UINT32                          ulTrigHighSlice;
}OM_APP_REPORT_STATUS_STRU;


/*APP-->OM  type=0x13  黑名单下发给 OM*/
typedef struct
{
   VOS_UINT32  ulMsgModuleID;     /*  module id */
   VOS_UINT16  usAlarmId;         /*  alarm id */
   VOS_UINT16  usAlmType;
}CHR_LIST_INFO_S;

typedef struct
{
    OM_ALARM_MSG_HEAD_STRU        stOmHeader;
    CHR_LIST_INFO_S               stBlackList[0];
} OM_APP_BLACK_LIST_STRU;


/*APP-->OM  type=0x14  优先级配置下发给 OM*/

typedef struct
{
    VOS_UINT32  ulModuleId;      /*  module id */
    VOS_UINT16  usAlarmId;       /*  alarm id */
    VOS_UINT16  priority;        /*  优先级 */
    VOS_UINT16  usAlmType;       /*  alarmtype */
    VOS_UINT16  usResved;       
}CHR_PRIORITY_INFO_S;


typedef struct
{
    OM_ALARM_MSG_HEAD_STRU        stOmHeader;
    VOS_UINT8                     ucPacketSN;            /* sn号，标识为第几包数据， 从0开始，最后一包数据值定义为0xFF */
    VOS_UINT8                     ucCount;	             /* 当前包中AlarmID配置个数 */
    VOS_UINT16                    usResved;           /* 预留 */
    CHR_PRIORITY_INFO_S           alarmMap[0];
} OM_APP_PRIORITY_CFG_STRU;

/*APP-->OM  type=0x15  周期配置下发给 OM*/
typedef struct
{
    VOS_UINT32    ulModuleId;     /* module id */
    VOS_UINT32    ulCount;         /*周期内1优先级允许上报次数*/
}CHR_PERIOD_CFG_STRU;

typedef struct{
    OM_ALARM_MSG_HEAD_STRU        stOmHeader;
    VOS_UINT8                     ucMsgSN;       /* sn号，标识为第几包数据， 从0开始，最后一包数据值定义为0xFF */
    VOS_UINT8                     ucCount;       /*  当前包中AlarmID配置个数*/
    VOS_UINT8                     ucperiod;      /* 上报周期，以第一包为准，单位小时 */
    VOS_UINT8                     ucResved;      /*  预留 */
    CHR_PERIOD_CFG_STRU           alarmMap[0];
} OM_APP_PERIOD_CFG_STRU;

/*****************************************************************************
  OM和各组件接口STRUCT定义
*****************************************************************************/

/* 下发给各组件的商用ERR log上报请求 OM->各组 */
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usFaultID;
    VOS_UINT16                          usAlarmID;
    VOS_UINT16                          usReserved;
    VOS_UINT32                          ulMsgSN;

}OM_ERR_LOG_REPORT_REQ_STRU;

/*OM和IMS通信协议头*/
typedef struct
{
    VOS_UINT32                          ulModuleId;
    VOS_UINT16                          usFaultId;   
    VOS_UINT16                          usAlarmId;
    VOS_UINT16                          usAlmType;   
    VOS_UINT16                          usReserved;
    VOS_UINT32                          ulMsgType;
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulRptLen;
}OM_IMS_MSG_HEAD_STRU;

/* 各组件-->OM的通信协议头 */
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    VOS_UINT32                          ulModuleId;
    VOS_UINT16                          usFaultId;   /* 主动上报时，GUTL不关注 ,x模的需要填写*/
    VOS_UINT16                          usAlarmId;
    VOS_UINT16                          usAlmType;   /* 被动上报时，不关注 */
    VOS_UINT16                          usReserved;
}OM_RCV_DATA_HEADER_STRU;
/* 各组件查询故障上报CNF & 故障主动上报内容 各组件 -> OM*/
typedef struct
{
    OM_RCV_DATA_HEADER_STRU             stChrOmHeader;
    VOS_UINT32                          ulMsgType;
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulRptLen;
    VOS_UINT8                           aucContent[4]; /* 上报的内容 */
}OM_FAULT_ERR_LOG_IND_STRU;


/* Error Log Payload header, OM不关注 */
typedef struct
{
    VOS_UINT32                          ulMsgModuleId;
    VOS_UINT16                          usModemId;
    VOS_UINT16                          usAlmId;
    VOS_UINT16                          usAlmLevel;
    VOS_UINT16                          usAlmType;
    VOS_UINT32                          usAlmLowSlice;
    VOS_UINT32                          usAlmHighSlice;
    VOS_UINT32                          ulAlmLength;
}OM_ERR_LOG_HEADER_STRU;


/*返回给各组件的黑名单优先级列表*/
typedef struct
{
    VOS_UINT16  usAlmType;
    VOS_UINT16  usAlarmId;

}CHR_GET_BLACKLIST_STRU;



/*****************************************************************************
  OM和IMS代理的接口STRUCT定义
*****************************************************************************/
/* OM --> IMS, REQ ， 讨论使用OM->各组件的结构体替代，归一化处理*/
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT32                          ulMsgModuleID;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usFaultId; /* 故障场景编号 */
    VOS_UINT16                          usAlarmid; 
    VOS_UINT16                          usReserved; 
    VOS_UINT32                          ulMsgSN; 
}APP_OM_REQ_ERR_LOG_CCPU_STRU;


/* 结构说明  : OM商用Errlog运行上下文 */
typedef struct
{
    OM_ERR_LOG_MOUDLE_ID_ENUM_UINT32    enProjectModule;   /* 设备号 */
    pFuncOMGetData                      pSendUlAtFunc;     /* 各组件注册 接收数据函数 */
}OM_REGISTER_PROJECT_CTX_STRU;



typedef struct
{
    VOS_UINT8           ucFuncType;
    VOS_UINT8           ucCpuId;
    VOS_UINT16          usLength;
    VOS_UINT32          ulSn;           /*Sequence Number for Trace, Event, OTA msg.*/
    VOS_UINT32          ulTimeStamp;    /*CPU time coming from ARM.*/
    VOS_UINT16          usPrimId;
    VOS_UINT16          usToolId;
    VOS_UINT32          ulDateType;
    VOS_UINT8           aucValue[4];
}OM_ERRLOG_TRANS_MSG_STRU;


/*****************************************************************************
  CLT_INFO云端频点，接口STRUCT定义
*****************************************************************************/

/* APP请求Info Clt上报type=0x10 App->OM */
typedef struct
{
    OM_ALARM_MSG_HEAD_STRU              stOmHeader;
    VOS_UINT16                          usModemID;
    VOS_UINT16                          usInfoID;
    VOS_UINT32                          ulMsgLength;
    VOS_UINT8                           aucMsgContent[4];
}APP_OM_INFO_CLT_REQ_STRU;

/* 各组件上报的信息收集 各组 -> OM*/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    VOS_UINT32                          ulMsgType;
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulRptLen;      
    VOS_UINT8                           aucContent[4]; 
} OM_INFO_CLT_REPORT_CNF_STRU;

/* Info Clt Payload header */
typedef struct
{
    VOS_UINT32                          ulMsgModuleId;
    VOS_UINT16                          usModemId;
    VOS_UINT16                          usSceneType;
    VOS_UINT32                          ulMsgLength;
}OM_INFO_CLT_HEADER_STRU;

typedef struct
{
    VOS_UINT32                        ulMcc;
    VOS_UINT32                        ulMnc;
}OM_INFO_CLT_PLMN_ID_STRU;

/* 下发给各组件的信息收集上报请求 OM->各组 */
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    VOS_UINT16                          usModemID;
    OM_INFO_CLT_SCENE_TYPE_ENUM_UINT16  enSceneType;
    VOS_UINT32                          ulMsgLen;
    VOS_UINT8                           aucContent[4];
}OM_INFO_CLT_REPORT_REQ_STRU;

/* Modem errlog report, modem chr to ap interface define begin */
typedef struct
{
   VOS_UINT8                           aucProductName[20];
   VOS_UINT16                          usSubEventID;
   VOS_UINT8                           ucSubEventCause;
   VOS_UINT8                           ucLogVersion;
   VOS_BOOL                            enModemHidsLog;/* 是否抓取HiDS Log */
}MODEM_ERR_LOG_HEADER_STRU;
/* Modem errlog report, modem chr to ap interface define end */
/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

extern VOS_UINT32 OM_RegisterGetData(OM_ERR_LOG_MOUDLE_ID_ENUM_UINT32 enProjectModule, pFuncOMGetData pFuncProjectCallback);


extern VOS_UINT32 OM_GetData(OM_ERR_LOG_MOUDLE_ID_ENUM_UINT32 enProjectModule, VOS_VOID *pData, VOS_UINT32 ulLen);

extern VOS_UINT32 Chr_GetBlackList (VOS_UINT32 ulMsgModuleID,  CHR_GET_BLACKLIST_STRU * pstBlackList, VOS_UINT32 ulMaxLen);

extern VOS_UINT32 Chr_MsgReport(OM_FAULT_ERR_LOG_IND_STRU * pstMsg);

extern VOS_UINT32 Chr_CheckBlackList (VOS_UINT32 ulMsgModuleID, VOS_UINT16 usAlarmId, VOS_UINT16 usAlmType);


#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of omerrorlog.h */


