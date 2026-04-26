/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define CHR_OM_PRE_ALLOC_LEN (4)
#define CHR_OM_PRODUCT_NAME_LEN (20)


/* 商用ERR log 上报相关 */
#define     ID_OM_ERR_LOG_CTRL_IND          (0x9001)
#define     ID_OM_ERR_LOG_REPORT_REQ        (0x9002)
#define     ID_OM_ERR_LOG_REPORT_CNF        (0x9003)

/* 平台检测故障主动上报消息名称 */
#define     ID_OM_FAULT_ERR_LOG_IND         (0x9009) /* 提议该处以后可能不止媒体组，还有其他组，所有改名 */

/* 平台检测告警主动上报消息名称 */
#define     ID_OM_ALARM_ERR_LOG_IND         (0x900A)

/* 业务信息收集相关 */
#define     ID_OM_INFO_CLT_REPORT_REQ       (0x9010)
#define     ID_OM_INFO_CLT_REPORT_CNF       (0x9011)

#define     ID_OM_MTA_NV_REFRESH_RSP        (0x2002) /* 防止OM->MTA新增消息和ID_MTA_NV_REFRESH_RSP取值重复 */

/* OM内部a核和c核使用 */
#define OM_ACPU_REPORT_BLOCKLIST_NAME 0x8001
#define OM_ACPU_REPORT_PRIORITY_NAME 0x8002
#define OM_ACPU_REPORT_PERIOD_NAME 0x8003
/* 3 枚举定义 */
typedef enum {
    OM_ERR_LOG_MSG_SET_ERR_REPORT = 0x01, /* 上层触发故障上报 */
    OM_ERR_LOG_MSG_ERR_REPORT_END = 0x02, /* 故障上报结束 */
    OM_ERR_LOG_MSG_ON_OFF = 0x03,         /* 控制上类型开关 */
    OM_ERR_LOG_MSG_ERR_REPORT = 0x04,     /* 故障上报 */
    OM_ERR_LOG_MSG_FTM_REPORT = 0x05,     /* 工程模式主动上报 */
    OM_ERR_LOG_MSG_RESULT = 0x06,         /* 上报Result */
    OM_ERR_LOG_MSG_FTM_REQ = 0x07,        /* 工程模式命令 */
    OM_ERR_LOG_MSG_FTM_CNF = 0x08,        /* 工程模式响应 */

    OM_ERR_LOG_MSG_FAULT_REPORT = 0x09, /* 平台检测故障主动上报 */

    OM_ERR_LOG_MSG_ALARM_REPORT = 0x0A, /* 平台检测告警主动上报 */

    OM_ERR_LOG_MSG_INFO_CLT_REQ = 0x10, /* 信息收集命令 */
    OM_ERR_LOG_MSG_INFO_CLT_CNF = 0x11, /* 信息收集响应 */
    OM_ERR_LOG_MSG_INFO_CLT_END = 0x12,

    OM_ERR_LOG_MSG_SMARTCDRX_REPORT = 0x13, /* SmartCDRX维测信息主动上报 */
    OM_ERR_LOG_MSG_BLOCK_LIST = 0x14,
    OM_ERR_LOG_MSG_PRIORITY_CFG = 0x15,
    OM_ERR_LOG_MSG_PERIOD_CFG = 0x16,
    OM_ERR_LOG_MSG_CFG_CNF = 0x17,

    OM_ERR_LOG_MSG_TYPE_BUTT
} chr_msg_type_e;
/* 开关变量 */
typedef enum {
    OM_APP_STATUS_CLOSE = 0x00,
    OM_APP_STATUS_OPEN = 0x01,

    OM_APP_STATUS_BUTT
} chr_app_switch_e;

typedef enum {
    OM_ERR_LOG_MOUDLE_ID_GUNAS = 0x020001,   /* GU NAS */
    OM_ERR_LOG_MOUDLE_ID_GAS = 0x020002,     /* GAS */
    OM_ERR_LOG_MOUDLE_ID_WAS = 0x020003,     /* WAS */
    OM_ERR_LOG_MOUDLE_ID_GUL2 = 0x020004,    /* GUL2 */
    OM_ERR_LOG_MOUDLE_ID_GUPHY = 0x020005,   /* GUPHY */
    OM_ERR_LOG_MOUDLE_ID_USIMM = 0x020006,   /* USIMM */
    OM_ERR_LOG_MOUDLE_ID_DRV_SCI = 0x020007, /* SCI DRV */
    OM_ERR_LOG_MOUDLE_ID_HIFI = 0x020008,    /* HIFI */
    OM_ERR_LOG_MOUDLE_ID_RRM = 0x020009,     /* RRM */
    OM_ERR_LOG_MOUDLE_ID_VISP = 0x02000A,    /* VISP CHR */

    OM_ERR_LOG_MOUDLE_ID_GNAS = 0x020011, /* G NAS */
    OM_ERR_LOG_MOUDLE_ID_GL2 = 0x020014,  /* GL2 */
    OM_ERR_LOG_MOUDLE_ID_GPHY = 0x020015, /* GPHY */

    OM_ERR_LOG_MOUDLE_ID_UNAS = 0x020021, /* U NAS */
    OM_ERR_LOG_MOUDLE_ID_UL2 = 0x020024,  /* UL2 */
    OM_ERR_LOG_MOUDLE_ID_UPHY = 0x020025, /* UPHY */

    OM_ERR_LOG_MOUDLE_ID_LMM = 0x020030,   /* LTE EMM */
    OM_ERR_LOG_MOUDLE_ID_ESM = 0x020031,   /* LTE ESM */
    OM_ERR_LOG_MOUDLE_ID_LSMS = 0x020032,  /* LTE SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_LPP = 0x020033,   /* LTE LPP */
    OM_ERR_LOG_MOUDLE_ID_LRRC = 0x020034,  /* LTE RRC */
    OM_ERR_LOG_MOUDLE_ID_LTEL2 = 0x020035, /* LTEL2 */
    OM_ERR_LOG_MOUDLE_ID_LPHY = 0x020036,  /* LTE PHY */

    OM_ERR_LOG_MOUDLE_ID_TDS_MM = 0x020040,  /* TDS MM */
    OM_ERR_LOG_MOUDLE_ID_TDS_GMM = 0x020041, /* TDS GMM */
    OM_ERR_LOG_MOUDLE_ID_TDS_CC = 0x020042,  /* TDS CC */
    OM_ERR_LOG_MOUDLE_ID_TDS_SM = 0x020043,  /* TDS SM */
    OM_ERR_LOG_MOUDLE_ID_TDS_SMS = 0x020044, /* TDS SMS(复用GU SMS，不会使用，预留) */
    OM_ERR_LOG_MOUDLE_ID_TDS_LPP = 0x020045, /* TDS LPP */
    OM_ERR_LOG_MOUDLE_ID_TRRC = 0x020046,    /* TDS RRC */
    OM_ERR_LOG_MOUDLE_ID_TDSL2 = 0x020047,   /* TDSL2 */
    OM_ERR_LOG_MOUDLE_ID_TDS_PHY = 0x020048, /* TDS PHY */
    OM_ERR_LOG_MOUDLE_ID_LPSOM = 0x020049,   /* LPS OM */

    /* 以下模块不具备收发消息功能，需借助COM模块中转 */
    OM_ERR_LOG_MOUDLE_ID_IMS = 0x020050,     /* IMS */
    OM_ERR_LOG_MOUDLE_ID_IMSA = 0x020051,    /* IMSA */
    OM_ERR_LOG_MOUDLE_ID_BBA_CHR = 0x020052, /* BBA CHR */

    OM_ERR_LOG_MOUDLE_ID_CNAS = 0x020060,    /* CNAS */
    OM_ERR_LOG_MOUDLE_ID_CAS = 0x020061,     /* CAS */
    OM_ERR_LOG_MOUDLE_ID_CL2 = 0x020062,     /* CL2 */
    OM_ERR_LOG_MOUDLE_ID_CPROC = 0x020063,   /* CPROC */
    OM_ERR_LOG_MOUDLE_ID_CSDR = 0x020064,    /* CSDR */
    OM_ERR_LOG_MOUDLE_ID_CHIFI = 0x020065,   /* CHIFI */
    OM_ERR_LOG_MOUDLE_ID_PPPC = 0x020066,    /* PPPC */
    OM_ERR_LOG_MOUDLE_ID_DRV = 0x020067,     /* DRV 用于底软低功耗上报 */
    OM_ERR_LOG_MOUDLE_ID_CHR_STA = 0x020068, /* OM维测 */
    OM_ERR_LOG_MOUDLE_ID_NRRC = 0x020069,    /* NRRC */
    OM_ERR_LOG_MOUDLE_ID_NRL2    = 0x02006A,   /* NRL2 */

    OM_ERR_LOG_MOUDLE_ID_NRMM = 0x020070, /* NR MM */

    OM_ERR_LOG_MOUDLE_ID_NRSM = 0x020071, /* NR SM */

    OM_ERR_LOG_MOUDLE_ID_NR_AGENT = 0x020080, /* NPHY AGENT */

    OM_ERR_LOG_MOUDLE_ID_BUTT
} chr_module_id_e;

/* 错误码,与ap之间的错误码 */
typedef enum {
    OM_APP_MSG_OK = 0x00,
    OM_APP_VCOM_DEV_INDEX_ERR = 0x01,
    OM_APP_MSG_LENGTH_ERR = 0x02,
    OM_APP_MSG_TYPE_ERR = 0x03,
    OM_APP_SEND_MODEM_ID_ERR = 0x04,
    OM_APP_MEM_ALLOC_MSG_ERR = 0x06,
    OM_APP_READ_NV_ERR = 0x07,
    OM_APP_GET_NVID_ERR = 0x08,
    OM_APP_MSG_MODULE_ID_ERR = 0x09,
    OM_APP_REPORT_NOT_FINISH_ERR = 0x0A,
    OM_APP_ERRLOG_START_TIMER_ERR = 0x0B,
    OM_APP_MEM_ALLOC_ERR = 0x0C,
    OM_APP_PARAM_INAVALID = 0x0D,
    OM_APP_SEND_CCPU_FAIL = 0x0E,
    OM_APP_SAVE_LIST_FAIL = 0x0F,

    OM_APP_SEND_RESULT_BUTT
} chr_app_send_result_e;

/* 错误码,与IMS之间的错误码 */
typedef enum {
    OM_IMS_MSG_OK = 0x00,
    OM_IMS_MSG_TYPE_ERR = 0x01,
    OM_IMS_MSG_LENGTH_ERR = 0x02,
    OM_IMS_MSG_MODULE_ID_ERR = 0x03,
    OM_IMS_MSG_ALLOC_MSG_ERR = 0x04,
    OM_IMS_MSG_NULL = 0x05,
    OM_IMS_SEND_RESULT_BUTT
} chr_ims_send_result_e;

/* 各组件主动上报OM返回给各组件的错误码 */
typedef enum {
    CHR_BLOCK_LIST_NOT_ALLOW = 0x100,
    CHR_THRESHOLD_NOT_ALLOW = 0x101,
    CHR_MSG_NAME_ERR = 0x102,
    CHR_INPUT_PARAM_NULL = 0x103,
    CHR_NV_READ_FAIL = 0x104,
    CHR_NV_SWITCH_CLOSE = 0x105,

    CHR_REPORT_RESULT_BUTT
} chr_report_result_e;

typedef enum {
    OM_INFO_CLT_SCENE_TYPE_FREQ_BAND = 0x4001,
    OM_INFO_CLT_SCENE_TYPE_NETWORK_INFO = 0x4002,

    OM_INFO_CLT_SCENE_TYPE_STATS_INFO = 0x4003,

    OM_INFO_CLT_SCENE_TYPE_BUTT
} chr_clt_scene_type_e;

/* COMM提供给其他API接口组件的通知注册接口形式 */
typedef unsigned int (*chr_om_getdata_func_t)(unsigned int proj_mod, void *data, unsigned int len);

/* APP-->OM error log 通用消息头，OM私用，各组件不允许使用 */
typedef struct {
    unsigned int msgtype;
    unsigned int msgsn;
    unsigned int msglen;
} chr_alarm_msg_head_s;

/* APP请求Error log上报type=0x01 App->OM */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short faultid; /* 故障场景编号 */
} chr_app_om_req_s;

/* Result To App type=0x06  OM -> App */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short resv;
    unsigned int status; /* result */
} chr_app_result_cnf_s;

/* Result To App type=0x17  OM -> App */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned int status; /* result,app并不解析此状态，只检查type=0x17 */
} chr_app_result_cfg_cnf_s;

/* Error log 上报完成 type=0x02 OM-> APP */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short faultid;
    unsigned int report_status; /* result */
    unsigned int low_slice;     /* 记录触发上报请求时的时间戳 */
    unsigned int high_slice;
} chr_app_report_status_s;

/* APP-->OM  type=0x13  黑名单下发给 OM */
typedef struct {
    unsigned int moduleid; /* module id */
    unsigned short alarmid;      /*  alarm id */
    unsigned short alarm_type;
} chr_list_info_s;

typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    chr_list_info_s block_list[0]; /*lint !e43 */
} chr_app_block_list_s;

/* APP-->OM  type=0x14  优先级配置下发给 OM */
typedef struct {
    unsigned int moduleid;    /*  module id */
    unsigned short alarmid;    /*  alarm id */
    unsigned short priority;   /*  优先级 */
    unsigned short alarm_type; /*  alarm type */
    unsigned short resv;
} chr_priority_info_s;

typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned char packt_sn;           /* sn号，标识为第几包数据， 从0开始，最后一包数据值定义为0xFF */
    unsigned char count;              /* 当前包中AlarmID配置个数 */
    unsigned short resv;              /* 预留 */
    chr_priority_info_s prior_map[0]; /*lint !e43 */
} chr_app_priority_cfg_s;

/* APP-->OM  type=0x15  周期配置下发给 OM */
typedef struct {
    unsigned int moduleid; /* module id */
    unsigned int count;    /* 周期内1优先级允许上报次数 */
} chr_period_cfg_s;

typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned char msg_sn;           /* sn号，标识为第几包数据， 从0开始，最后一包数据值定义为0xFF */
    unsigned char count;            /*  当前包中AlarmID配置个数 */
    unsigned char period;           /* 上报周期，以第一包为准，单位小时 */
    unsigned char ucResved;         /*  预留 */
    chr_period_cfg_s period_map[0]; /*lint !e43 */
} chr_app_period_cfg_s;

/* OM和各组件接口STRUCT定义 */
/* 下发给各组件的商用ERR log上报请求 OM->各组 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msg_name;
    unsigned short modemid;
    unsigned short faultid;
    unsigned short alarmid;
    unsigned short resv;
    unsigned int msgsn;
} chr_log_report_req_s;

/* OM和IMS通信协议头 */
typedef struct {
    unsigned int moduleid;
    unsigned short faultid;
    unsigned short alarmid;
    unsigned short almtype;
    unsigned short resv;
    unsigned int msgtype;
    unsigned int msgsn;
    unsigned int rptlen;
} chr_ims_msg_head_s;

/* 各组件-->OM的通信协议头 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msgname;
    unsigned int moduleid;
    unsigned short faultid; /* 主动上报时，GUTL不关注 ,x模的需要填写 */
    unsigned short alarmid;
    unsigned short alarm_type; /* 被动上报时，不关注 */
    unsigned short resv;
} chr_rcv_data_header_s;

/* 各组件查询故障上报CNF & 故障主动上报内容 各组件 -> OM */
typedef struct {
    chr_rcv_data_header_s chr_om_header;
    unsigned int msgtype;
    unsigned int msgsn;
    unsigned int rptlen;
    unsigned char data[CHR_OM_PRE_ALLOC_LEN]; /* 上报的内容 */
} chr_fault_ind_s;

/* Error Log Payload header, OM不关注 */
typedef struct {
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short alarmid;
    unsigned short alarm_level;
    unsigned short alarm_type;
    unsigned int low_slice;
    unsigned int high_slice;
    unsigned int alarm_len;
} chr_data_header_s;

/* 返回给各组件的黑名单优先级列表 */
typedef struct {
    unsigned short alarm_type;
    unsigned short alarmid;
} chr_get_blacklist_s;

/* OM和IMS代理的接口STRUCT定义 */
/* OM --> IMS, REQ ， 讨论使用OM->各组件的结构体替代，归一化处理 */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short faultid; /* 故障场景编号 */
    unsigned short alarmid;
    unsigned short resv;
    unsigned int msgsn;
} chr_app_req_ccpu_s;

/* 结构说明  : OM商用Errlog运行上下文 */
typedef struct {
    unsigned int project_module; /* 设备号 */
    chr_om_getdata_func_t send_func;   /* 各组件注册 接收数据函数 */
} chr_register_proj_ctx_s;


/*
 * 结构名    : OM_ERRLOG_TRANS_MSG_STRU
 * 结构说明  : Error Log 消息勾包结构体
 */
typedef struct {
    unsigned char func_type;
    unsigned char cpuid;
    unsigned short lenth;
    unsigned int sn;         /* Sequence Number for Trace, Event, OTA msg. */
    unsigned int time_stamp; /* CPU time coming from ARM. */
    unsigned short prim_id;
    unsigned short tools_id;
    unsigned int date_type;
    unsigned char data[CHR_OM_PRE_ALLOC_LEN];
} chr_trans_msg_s;

/* CLT_INFO云端频点，接口STRUCT定义 */
/* APP请求Info Clt上报type=0x10 App->OM */
typedef struct {
    chr_alarm_msg_head_s chr_om_header;
    unsigned short modemid;
    unsigned short infoid;
    unsigned int msglen;
    unsigned char msgdata[CHR_OM_PRE_ALLOC_LEN];
} chr_app_info_clt_req_s;

/* 各组件上报的信息收集 各组 -> OM */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msg_name;
    unsigned int msg_type;
    unsigned int msg_sn;
    unsigned int rpt_len;
    unsigned char data[CHR_OM_PRE_ALLOC_LEN];
} chr_info_clt_report_cnf_s;
/* Info Clt Payload header */
typedef struct {
    unsigned int moduleid;
    unsigned short modemid;
    unsigned short scenetype;
    unsigned int msglen;
} chr_info_clt_header_s;

typedef struct {
    unsigned int mcc;
    unsigned int mnc;
} chr_info_clk_plmn_id_s;

/* 下发给各组件的信息收集上报请求 OM->各组 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msgname;
    unsigned short modemid;
    unsigned short scene_type;
    unsigned int msg_len;
    unsigned char data[CHR_OM_PRE_ALLOC_LEN];
} chr_info_clt_report_req_s;

/* Modem errlog report, modem chr to ap interface define begin */
typedef struct {
    unsigned char product_name[CHR_OM_PRODUCT_NAME_LEN];
    unsigned short sub_eventid;
    unsigned char sub_event_cause;
    unsigned char log_version;
    unsigned int en_modem_hids_log; /* 是否抓取HiDS Log */
} chr_modemlog_header_s;

/* Modem errlog report, modem chr to ap interface define end */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
/*
 * 功能描述：OM Ccpu接收各组件上报工程模式数据到APP回调函数注册接口
 * 输入参数：enProjectModule        ：回调组建编号，固定为OM_VOLTE_MOUDLE_PROJECT
 *           pFuncProjectCallbak    ：回调函数指针
 * 输出参数：NA
 * 返回值：VOS_OK/VOS_ERR
 * 修改历史：
 * 1.  日期：2014年1月2日
 * 修改内容：新生成函数
 *     1、工程模式主动上报开关通知各组件，Data域包含完整的OmHeard和OMPayload
 *     2、工程模式命令上报消息通知各组件，Data域包含完整的OmHeard和OMPayload
 */
extern unsigned int mdrv_chr_register_getdata(unsigned int project_module,
                                              chr_om_getdata_func_t project_callback_func);

/*
 * 功能描述：OM Ccpu接收各组件上报工程模式数据
 * 输入参数：enProjectModule    ：组建编号
 *           pData              ：发送数据
 *           ulLen              ：发送数据长度
 * 输出参数：NA
 * 返回值：VOS_OK/VOS_ERR
 * 修改历史：
 * 1.  日期：2014年1月2日
 * 修改内容：新生成函数
 * 说明：pData从OMHeader的MsgType域开始。相当于调用组件预先申请Header部分，其中Header部分的SN字段由COMM填写
 */
extern unsigned int mdrv_chr_getdata(unsigned int project_module, void *data, unsigned int len);
/*
 * 功能描述：黑名单获取列表
 * 输入参数：ulMsgModuleID          ：回调组建编号
 *           pstblockList           ：黑名单列表
 * 输出参数：NA
 * 返回值：VOS_OK/VOS_ERR
 * 修改历史：
 * 1.  日期：2017年11月28日
 * 修改内容：新生成函数
 */
extern unsigned int mdrv_chr_getblacklist(unsigned int moduleid, chr_get_blacklist_s *blacklist,
                                          unsigned int maxlen);
/*
 * 功能描述：提供给各组件主动上报调用接口,注意此接口会释放各组件内存，各组件不需要释放
 * 输出参数：上报消息
 * 修改历史：
 * 1.  日期：2017年11月28日
 * 修改内容：新生成函数
 */
extern unsigned int mdrv_chr_msgreport(chr_fault_ind_s *chr_msg);
/*
 * 功能描述：用于返回该组件是否在黑名单里面,返回结果是在还是不在黑名单
 * 输入参数：ulMsgModuleID
 *           usAlarmId
 *           usAlmType：直通填写6，非直通6之外的值
 * 返回值: FALSE    0：不在黑名单中
 *         TRUE     1：再黑名单中
 * 修改历史：
 * 1.  日期：2017年12月5日
 * 修改内容：新生成函数
 */
extern unsigned int mdrv_chr_checkblacklist(unsigned int moduleid, unsigned short alarmid,
                                            unsigned short alarm_type);

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