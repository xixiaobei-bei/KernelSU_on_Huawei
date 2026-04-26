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

#ifndef __AT_XPDS_INTERFACE_H__
#define __AT_XPDS_INTERFACE_H__

#include "PsTypeDef.h"
#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_MSG_CDMA_MAX_SV_NUM (12)
#define TAF_MSG_CDMA_MAX_EPH_PRN_NUM (32)
#define TAF_MSG_CDMA_MAX_ALM_PRN_NUM (32)
#define AT_XPDS_MEID_DATA_LEN (7)


enum AT_XPDS_MsgType {
    /* 消息名称 */ /* 消息ID */ /* 备注, 生成ASN */
    /* AT->XPDS */
    /* _MSGPARSE_Interception AT_XPDS_GpsStartReq */
    ID_AT_XPDS_GPS_START_REQ             = 0x0001,
    /* _MSGPARSE_Interception AT_XPDS_GpsCfgMpcAddrReq */
    ID_AT_XPDS_GPS_CFG_MPC_ADDR_REQ      = 0x0003,
    /* _MSGPARSE_Interception AT_XPDS_GpsCfgPdeAddrReq */
    ID_AT_XPDS_GPS_CFG_PDE_ADDR_REQ      = 0x0005,
     /* _MSGPARSE_Interception AT_XPDS_GpsCfgPositionModeReq */
    ID_AT_XPDS_GPS_CFG_POSITION_MODE_REQ = 0x0007,
    /* _MSGPARSE_Interception AT_XPDS_GpsQryReflocInfoReq */
    ID_AT_XPDS_GPS_QRY_REFLOC_INFO_REQ   = 0x0009,
    /* _MSGPARSE_Interception AT_XPDS_GpsQryTimeInfoReq */
    ID_AT_XPDS_GPS_QRY_TIME_INFO_REQ     = 0x000b,
    /* _MSGPARSE_Interception AT_XPDS_GpsStopReq */
    ID_AT_XPDS_GPS_STOP_REQ              = 0x000d,
    /* _MSGPARSE_Interception AT_XPDS_GpsPrmInfoRsp */
    ID_AT_XPDS_GPS_PRM_INFO_RSP = 0x1001,
    /* _MSGPARSE_Interception AT_XPDS_GpsPosInfoRsp */
    ID_AT_XPDS_GPS_POS_INFO_RSP = 0x1003,
    /* _MSGPARSE_Interception AT_XPDS_GpsReplyNiRsp */
    ID_AT_XPDS_GPS_REPLY_NI_REQ = 0x1005,
     /* _MSGPARSE_Interception AT_XPDS_ApDataCallStatusInd */
    ID_AT_XPDS_AP_DATA_CALL_STATUS_IND   = 0x3002,
     /* _MSGPARSE_Interception AT_XPDS_ApForwardDataInd */
    ID_AT_XPDS_AP_FORWARD_DATA_IND       = 0x3006,
    /* _MSGPARSE_Interception AT_XPDS_Msg */
    ID_AT_XPDS_UTS_TEST_STOP_REQ         = 0x3009,
    /* XPDS->AT */
    /* _MSGPARSE_Interception XPDS_AT_ResultCnf */
    ID_XPDS_AT_GPS_START_CNF             = 0x0002,
    /* _MSGPARSE_Interception XPDS_AT_ResultCnf */
    ID_XPDS_AT_GPS_CFG_MPC_ADDR_CNF      = 0x0004,
    /* _MSGPARSE_Interception XPDS_AT_ResultCnf */
    ID_XPDS_AT_GPS_CFG_PDE_ADDR_CNF      = 0x0006,
    /* _MSGPARSE_Interception XPDS_AT_ResultCnf */
    ID_XPDS_AT_GPS_CFG_MODE_CNF          = 0x0008,
    /* _MSGPARSE_Interception XPDS_AT_GpsReflocInfoCnf */
    ID_XPDS_AT_GPS_REFLOC_INFO_CNF       = 0x000a,
    /* _MSGPARSE_Interception XPDS_AT_GpsSyncTimeInd */
    ID_XPDS_AT_GPS_TIME_INFO_IND         = 0x000c,
    /* _MSGPARSE_Interception XPDS_AT_ResultCnf */
    ID_XPDS_AT_GPS_STOP_CNF              = 0x000e,
    /* _MSGPARSE_Interception XPDS_AT_GpsIonInfoInd */
    ID_XPDS_AT_GPS_ION_INFO_IND        = 0x2002,
    /* _MSGPARSE_Interception XPDS_AT_GpsEphInfoInd */
    ID_XPDS_AT_GPS_EPH_INFO_IND        = 0x2004,
    /* _MSGPARSE_Interception XPDS_AT_GpsAlmInfoInd */
    ID_XPDS_AT_GPS_ALM_INFO_IND        = 0x2006,
    /* _MSGPARSE_Interception XPDS_AT_GpsPdePosiInfoInd */
    ID_XPDS_AT_GPS_PDE_POSI_INFO_IND   = 0x2008,
    /* _MSGPARSE_Interception XPDS_AT_GpsNiSessionInd */
    ID_XPDS_AT_GPS_NI_SESSION_IND      = 0x200a,
    /* _MSGPARSE_Interception XPDS_AT_GpsStartReq */
    ID_XPDS_AT_GPS_START_REQ           = 0x200c,
    /* _MSGPARSE_Interception XPDS_AT_GpsCancelInd */
    ID_XPDS_AT_GPS_CANCEL_IND          = 0x200e,
    /* _MSGPARSE_Interception XPDS_AT_GpsAcqAssistDataInd */
    ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND = 0x2012,
    /* _MSGPARSE_Interception XPDS_AT_GpsDelAssistDataInd */
    ID_XPDS_AT_GPS_DEL_ASSIST_DATA_IND = 0x2014,
    /* _MSGPARSE_Interception XPDS_AT_GpsAbortInd */
    ID_XPDS_AT_GPS_ABORT_IND           = 0x2016,
    /* _MSGPARSE_Interception XPDS_AT_GpsNiCpStart */
    ID_XPDS_AT_GPS_NI_CP_START         = 0x2018,
    /* _MSGPARSE_Interception XPDS_AT_GpsNiCpStop */
    ID_XPDS_AT_GPS_NI_CP_STOP          = 0x201a,
    /* _MSGPARSE_Interception XPDS_AT_ApDataCallReq */
    ID_XPDS_AT_AP_DATA_CALL_REQ          = 0x3001,
    /* _MSGPARSE_Interception XPDS_AT_ApServerBindReq */
    ID_XPDS_AT_AP_SERVER_BIND_REQ        = 0x3003,
    /* _MSGPARSE_Interception AT_XPDS_ApServerBindStatusInd */
    ID_AT_XPDS_AP_SERVER_BIND_STATUS_IND = 0x3004,
     /* _MSGPARSE_Interception XPDS_AT_ApReverseDataInd */
    ID_XPDS_AT_AP_REVERSE_DATA_IND       = 0x3005,
    /* _MSGPARSE_Interception AT_XPDS_Msg */
    ID_AT_XPDS_UTS_TEST_START_REQ        = 0x3007,
     /* _MSGPARSE_Interception AT_XPDS_Msg */
    ID_XPDS_AT_GPS_UTS_TEST_START_REQ    = 0x3008,
    /* _MSGPARSE_Interception AT_XPDS_Msg */
    ID_XPDS_AT_GPS_UTS_TEST_STOP_REQ     = 0x300a,
    /* _MSGPARSE_Interception XPDS_AT_UtsGpsPosInfoInd */
    ID_XPDS_AT_UTS_GPS_POS_INFO_IND  = 0x300b,
    /* _MSGPARSE_Interception XPDS_AT_GpsOmTestStartReq */
    ID_XPDS_AT_GPS_OM_TEST_START_REQ = 0x300c,
    /* _MSGPARSE_Interception XPDS_AT_GpsOmTestStopReq */
    ID_XPDS_AT_GPS_OM_TEST_STOP_REQ  = 0x300d,

    /* 最后一条消息 */
    ID_AT_XPDS_MSG_TYPE_BUTT

};
typedef VOS_UINT32 AT_XPDS_MsgTypeUint32;


enum XPDS_AT_Result {
    /* 与AT模块对应的标准错误码 */
    XPDS_AT_RESULT_NO_ERROR = 0x000000, /* 消息处理正常 */
    XPDS_AT_RESULT_ERROR,               /* 消息处理出错 */
    XPDS_AT_RESULT_INCORRECT_PARAMETERS,
    XPDS_AT_RESULT_OPTION_TIMEOUT,

    XPDS_AT_RESULT_BUTT
};
typedef VOS_UINT32 XPDS_AT_ResultUint32;

enum AT_XPDS_AgpsDataCallCtrl {
    AT_XPDS_AGPS_DATA_CALL_CTRL_RELEASE,
    AT_XPDS_AGPS_DATA_CALL_CTRL_CONNECT,

    AT_XPDS_AGPS_DATA_CALL_CTRL_BUTT
};
typedef VOS_UINT32 AT_XPDS_AgpsDataCallCtrlUint32;

enum AT_XPDS_ServerMode {
    AT_XPDS_SERVER_MODE_NULL,
    AT_XPDS_SERVER_MODE_MPC,
    AT_XPDS_SERVER_MODE_PDE,
    AT_XPDS_SERVER_MODE_BUTT
};
typedef VOS_UINT32 AT_XPDS_ServerModeUint32;

enum AT_XPDS_ServerBindStatus {
    AT_XPDS_SERVER_BIND_STATUS_IDLE,
    AT_XPDS_SERVER_BIND_STATUS_CONNECT,
    AT_XPDS_SERVER_BIND_STATUS_BUTT
};
typedef VOS_UINT32 AT_XPDS_ServerBindStatusUint32;

enum AT_XPDS_DataCallStatus {
    AT_XPDS_DATA_CALL_STATUS_IDLE,
    AT_XPDS_DATA_CALL_STATUS_CONNECT,
    AT_XPDS_DATA_CALL_STATUS_BUTT
};
typedef VOS_UINT32 AT_XPDS_DataCallStatusUint32;

enum XPDS_AT_SyncTime {
    XPDS_AT_SYNC_TIME_NOT_VALID, /* not valid */
    XPDS_AT_SYNC_TIME_VALID,     /* valid */

    XPDS_AT_SYNC_TIME_BUTT
};
typedef VOS_UINT32 XPDS_AT_SyncTimeUint32;

enum AT_XPDS_ReplySyncTime {
    AT_XPDS_REPLY_SYNC_TIME_INFO            = 1, /* reply time sync info */
    AT_XPDS_REPLY_SYNC_TIME_AND_ASSIST_DATA = 2, /* reply time sync info and GPS assist data */
    AT_XPDS_REPLY_FORCE_SYNC_TIME           = 3,

    AT_XPDS_REPLY_SYNC_BUTT
};
typedef VOS_UINT8 AT_XPDS_ReplySyncTimeUint8;

enum XPDS_AT_AbortPosReason {
    XPDS_AT_ABORT_POS_REASON_ENUM_MODEM_NOT_START = 0,
    XPDS_AT_ABORT_POS_REASON_ENUM_PARA_INCORRECT  = 1,
    XPDS_AT_ABORT_POS_REASON_ENUM_DATA_LINK_ERR   = 2,
    XPDS_AT_ABORT_POS_REASON_ENUM_NETWORK_NO_RSP  = 3,
    XPDS_AT_ABORT_POS_REASON_ENUM_BUTT
};
typedef VOS_UINT16 XPDS_AT_AbortPosReasonUint16;

/*
 * Description:
 */
enum TAF_MSG_CdmaGpsFixQuality {
    /* *< Fix uses information from GPS satellites only. */
    TAF_MSG_CDMA_GPS_FIX_QUALITY_UNKNOWN = 0x00,
    /* *< Fix uses information from GPS satellites and also a differential GPS (DGPS) station. */
    TAF_MSG_CDMA_GPS_FIX_QUALITY_GPS       = 0x01,
    TAF_MSG_CDMA_GPS_FIX_QUALITY_DGPS      = 0x02,
    TAF_MSG_CDMA_GPS_FIX_QUALITY_MAX       = 0x10,
    TAF_MSG_CDMA_GPS_FIX_QUALITY_ENUM_BUTT = 0x11
};
typedef VOS_UINT8 TAF_MSG_CdmaGpsFixQualityUint8;

/*
 * Description:
 */
enum TAF_MSG_CdmaGpsFixType {
    TAF_MSG_CDMA_GPS_FIX_UNKNOWN        = 0x00,
    TAF_MSG_CDMA_GPS_FIX_2D             = 0x01,
    TAF_MSG_CDMA_GPS_FIX_3D             = 0x02,
    TAF_MSG_CDMA_GPS_FIX_TYPE_MAX       = 0x10,
    TAF_MSG_CDMA_GPS_FIX_TYPE_ENUM_BUTT = 0x11
};
typedef VOS_UINT8 TAF_MSG_CdmaGpsFixTypeUint8;

/*
 * Description:
 */
enum TAF_MSG_CdmaGpsSelectionType {
    TAF_MSG_CDMA_GPS_FIX_SELECTION_UNKNOWN    = 0x00,
    TAF_MSG_CDMA_GPS_FIX_SELECTION_AUTO       = 0x01,
    TAF_MSG_CDMA_GPS_FIX_SELECTION_MANUAL     = 0x02,
    TAF_MSG_CDMA_GPS_FIX_SELECTION_TYPE_MAX   = 0x10,
    TAF_MSG_CDMA_GPS_SELECTION_TYPE_ENUM_BUTT = 0x11
};
typedef VOS_UINT8 TAF_MSG_CdmaGpsSelectionTypeUint8;

/*
 * Description:
 */
enum AT_XPDS_GpsNiReqReplyResult {
    AT_XPDS_GPS_NI_REQ_REPLY_RESULT_REJECT = 0,
    AT_XPDS_GPS_NI_REQ_REPLY_RESULT_ACCEPT = 1,

    AT_XPDS_GPS_NI_REQ_REPLY_RESULT_ENUM_BUTT
};
typedef VOS_UINT8 AT_XPDS_GpsNiReqReplyResultUint8;

/*
 * Description:
 */
enum AT_XPDS_GpsNiReqRejectType {
    AT_XPDS_GPS_NI_REQ_REJECT_TYPE_INVALID     = 0,
    AT_XPDS_GPS_NI_REQ_REJECT_TYPE_TIMER_OUT   = 1, /* GPS应答第三方定位请求超时 */
    AT_XPDS_GPS_NI_REQ_REJECT_TYPE_USER_DENIED = 2, /* 用户拒绝第三方定位请求 */

    AT_XPDS_GPS_RESP_NI_REQ_REJECT_TYPE_ENUM_BUTT = 0x11
};
typedef VOS_UINT8 AT_XPDS_GpsNiReqRejectTypeUint8;


enum TAF_XPDS_CallType {
    TAF_XPDS_USER_PLANE = 0,
    TAF_XPDS_CTRL_PLANE,
    TAF_XPDS_CALL_BUTT
};
typedef VOS_UINT8 TAF_XPDS_CallTypeUint8;


enum TAF_XPDS_FixMode {
    TAF_XPDS_UNKOWN_MODE = 0,
    TAF_XPDS_MSA_MODE,
    TAF_XPDS_MSB_MODE,
    TAF_XPDS_MSS_MODE,
    TAF_XPDS_AFLT_MODE,
    TAF_XPDS_MONITOR_MODE,
    TAF_XPDS_MODE_BUTT
};
typedef VOS_UINT8 TAF_XPDS_FixModeUint8;

/*
 * Description:
 */
enum TAF_XPDS_NotiftAndVerifyIndicator {
    TAF_XPDS_NOTIFT_AND_VERIFY_INDICATOR_BOTH_NONE   = 0,
    TAF_XPDS_NOTIFT_AND_VERIFY_INDICATOR_BOTH_NEED   = 1,
    TAF_XPDS_NOTIFT_AND_VERIFY_INDICATOR_NOTIFY_ONLY = 2,

    TAF_XPDS_NOTIFT_AND_VERIFY_INDICATOR_BUTT
};

typedef VOS_UINT8 TAF_XPDS_NotiftAndVerifyIndicatorUint8;

/*
 * Description:
 */
enum TAF_XPDS_Is801Mode {
    TAF_XPDS_IS801_MODE_MSA_ONLY   = 0,
    TAF_XPDS_IS801_MODE_MSB_ONLY   = 1,
    TAF_XPDS_IS801_MODE_MSA_PREFER = 2,
    TAF_XPDS_IS801_MODE_MSB_PREFER = 3,

    TAF_XPDS_IS801_MODE_BUTT
};

typedef VOS_UINT8 TAF_XPDS_Is801ModeUint8;

/*
 * Description:
 */
enum TAF_XPDS_PosTechIndicator {
    TAF_XPDS_POS_TECH_INDICATOR_IS801             = 0,
    TAF_XPDS_POS_TECH_INDICATOR_RETURN_SID_NID    = 1,
    TAF_XPDS_POS_TECH_INDICATOR_NOTIFY_CACHED_POS = 2,

    TAF_XPDS_POS_TECH_INDICATOR_BUTT
};

typedef VOS_UINT8 TAF_XPDS_PosTechIndicatorUint8;


enum TAF_XPDS_InitialType {
    TAF_XPDS_SI = 0,
    TAF_XPDS_NI,
    TAF_XPDS_INITIAL_BUTT
};
typedef VOS_UINT32 TAF_XPDS_InitialTypeUint32;


enum TAF_XPDS_GpsStartMode {
    TAF_XPDS_GPS_START_MODE_COLD,
    TAF_XPDS_GPS_START_MODE_HOT,
    TAF_XPDS_GPS_START_MODE_BUTT
};
typedef VOS_UINT8 TAF_XPDS_GpsStartModeUint8;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT8             content[4]; /* 消息内容 */
} AT_XPDS_Msg;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    XPDS_AT_ResultUint32  result; /* 操作结果 */
} XPDS_AT_ResultCnf;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
} AT_XPDS_GpsQryReflocInfoReq;


typedef struct {
    VOS_UINT8 reserved[4]; /* 保留位 */
} AT_XPDS_Reserve;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32      msgId;
    AT_APPCTRL                 appCtrl;
    TAF_XPDS_InitialTypeUint32 agpsType; /* < ulAgpsType */ /* 0: SI; 1: NI */
} AT_XPDS_GpsStartReq;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
} AT_XPDS_GpsStopReq;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT32            ipAddr;
    VOS_UINT16            ipPort;
    VOS_UINT16            reserved;
} AT_XPDS_GpsCfgMpcAddrReq;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT8             addrValid; /* < 0: not valid, 1: valid */
    VOS_UINT8             ipType;    /* < 0: ipv4, 1:ipv6 */
    VOS_UINT8             urlValid;  /* < 0: not valid, 1: valid */
    VOS_UINT8             reserved;  /* < padding */
    VOS_UINT32            ip4Addr;
    VOS_UINT32            ip6Addr[4];
    VOS_UINT32            portNum;
    VOS_UINT8             urlAddr[128];
} AT_XPDS_GpsCfgPdeAddrReq;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32  msgId;
    AT_APPCTRL             appCtrl;
    TAF_XPDS_CallTypeUint8 callType; /* < Userplane: 0, Callplane: 1 */
    /* < Unkown: 0; MSA: 1, MSB: 2, MSS: 3, AFLT: 4 */
    TAF_XPDS_FixModeUint8 fixMode;
    /*
     * < A value of 1 means is interested in only one fix. A value > 1, multiple fixes with some time in btw the
     * attempts
     */
    VOS_UINT16 fixNum;
    /* < Time, in seconds, between position fix attempts */
    VOS_UINT32 fixRateTime;
    VOS_UINT32 qosHaccuracy; /* < Horizontal Accuracy, in meters */
    VOS_UINT32 qosVaccuracy; /* < Vertical Acuracy, in meters */
    /* < Performance response quality in terms of time, in seconds */
    VOS_UINT32 qosPerformance;
} AT_XPDS_GpsCfgPositionModeReq;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    /* < 1: reply time sync info, 2: reply time sync info and GPS assist data */
    AT_XPDS_ReplySyncTimeUint8 actionType;
    VOS_UINT8                  reserved[3]; /* < padding */
} AT_XPDS_GpsQryTimeInfoReq;

/*
 * Description:
 */
typedef struct {
    VOS_UINT8 svId; /* < Satellite vehicle ID */
    /* < Satellite C/No. [db-HZ], range [0..63] */
    VOS_UINT8 svCn0;
    /* < Pseudorange Multipath Indicator, range [0..3] */
    VOS_UINT8 mulpathInd;
    /* < Pseudorange RMS Error, Range [0..63] */
    VOS_UINT8 psRmsErr;
    /* < Measured Doppler frequency, in units of 0.2Hz, range [-6553.6..+6553.4] Hz */
    VOS_INT16 psDopp;
    /* < Satellite code Phase Whole Chips, in units of 1 GPS chip, range [0..1022] */
    VOS_UINT16 svCodePhWhole;
    /* < Satellite code Phase Fractional Chips, in units of 1/2^10 of GPS chips, range [0.. (2^10-1)/2^10] GPS chips */
    VOS_UINT16 svCodePhFract;
    VOS_UINT8  reserved[2];
} AT_XPDS_GpsModemPrmdata;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    /* *< 0--not valid, 1--valid */
    VOS_UINT8 prmValid;
    /* *< Measurement GPS Time of Week Uncertainty */
    VOS_UINT8 measTowUnc;
    /* *< Number of measurements, Range [0..15] */
    VOS_UINT8 measNum;
    VOS_UINT8 reserved; /* *< padding */
    /* *< Measurement GPS Time of Week, in units of 1ms */
    VOS_UINT32              measTow;
    AT_XPDS_GpsModemPrmdata mseasData[TAF_MSG_CDMA_MAX_SV_NUM];
} AT_XPDS_GpsPrmInfoRsp;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32            msgId;
    AT_APPCTRL                       appCtrl;
    AT_XPDS_GpsNiReqReplyResultUint8 replyResult; /* *< 0: reject, 1: accept */
    /* 1:用户未应答; 2:用户拒绝定位请求 */
    AT_XPDS_GpsNiReqRejectTypeUint8 rejReason;
    VOS_UINT8                       reserved[2]; /* *< padding */
} AT_XPDS_GpsReplyNiRsp;

/*
 * Description:
 */
typedef struct {
    /* *< 0: Not valid, 1: Valid Time Zone Only,2: Valid Time Zone and BS Location. */
    VOS_UINT8 validRefLoc;
    /* *< padding */
    VOS_UINT8  reserved;
    VOS_UINT16 sid;  /* *< System ID, Range [0..32767] */
    VOS_UINT16 nid;  /* *< Network ID, Range [0..65535] */
    VOS_UINT16 bsid; /* *< Base Station ID, Range [0..65535] */
    VOS_UINT32 tzLat;
    /* *< Time zone Range [-16h ~+15.5h] and specific location is predefined in CBP */
    VOS_UINT32 tzLong;
    /* *< WGS84 Geodetic Latitude [degrees], latitude from base last registered on */
    VOS_UINT32 bslat;
    /* *< WGS84 Geodetic Longitude [degrees], Longitude from base last registered on */
    VOS_UINT32 bslong;
} XPDS_AT_GpsReflocInfo;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    XPDS_AT_GpsReflocInfo refLoc;
} XPDS_AT_GpsReflocInfoCnf;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT8             velocityIncl;
    VOS_UINT8             heightIncl;
    VOS_UINT8             clockIncl;
    VOS_UINT8             fixType;
    VOS_INT16             locUncAng;
    VOS_INT16             clockDrift;
    VOS_INT32             clockBias;
    VOS_INT32             latitude;
    VOS_INT32             longitude;
    VOS_UINT32            locUncA;
    VOS_UINT32            locUncP;
    VOS_UINT32            velocityHor;
    VOS_UINT32            heading;
    VOS_INT32             height;
    VOS_INT32             verticalVelo;
    VOS_UINT32            locUncV;
} XPDS_AT_GpsPdePosiInfoInd;

/*
 * Description:
 */
typedef struct {
    VOS_UINT8 svId; /* *< Satellite Vehicle ID */
    /* *< Doppler 1st order term Hz/s, BS shall set the field to the two's complement value */
    VOS_INT8 doppler1;
    /* of the 1st order doppler, in units of 1/64 Hz/s, in the range from -1Hz/s to +63/64 Hz/s */
    /* *< Satellite doppler uncertainty, range [0..4] Please refer to IS801-1 page 4.30 */
    VOS_UINT8 dopplerWin;
    /* *< Integer number of Code periods that have dlapsed since the latest GPS bit boundary, range [0..19] */
    VOS_UINT8  svCodePhaseInt;
    VOS_UINT8  gpsBitNum;      /* *< GPS bit number relative to GPS_TOW, range [0..3] */
    VOS_UINT8  svCodePhaseWin; /* *< Total code phase window, range [0..31] */
    VOS_UINT8  azimuth;        /* *< Satellite Azimuth, in units of 11.25 degrees */
    VOS_UINT8  elevation;      /* *< Satellite Elevation, in units of 11.25 degrees */
    VOS_UINT16 svCodePhase;    /* *< The GPS Code Phase, range [0..1022] chips */
    /* *< Doppler 0th order term, in units of 2.5Hz, in the range from -5120Hz to +5117.5 Hz */
    VOS_INT16 doppler0;
} TAF_MSG_CdmaAcqassistData;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    /* *< GPS time of week in seconds at AA's Time of Application */
    VOS_UINT32 refTow;
    /* *< Number of satellites for which data is available */
    VOS_UINT8 svNum;
    /* *< Doppler0 field included or not, 0: Not included, 1: included */
    VOS_UINT8 doppler0Inc;
    /* *< Doppler1 field included or not, 0: Not included, 1: included */
    VOS_UINT8 addDopplerInc;
    /* *< Code Phase information included or not, 0: Not included, 1: included */
    VOS_UINT8 codePhaseInc;
    /* *< Azimuth and Elevation angle included or not, 0: Not included, 1: included */
    VOS_UINT8                 azEl;
    VOS_UINT8                 reserved[3];
    TAF_MSG_CdmaAcqassistData aaData[TAF_MSG_CDMA_MAX_SV_NUM]; /* *< AA data array */
} XPDS_AT_GpsAcqAssistDataInd;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32          msgId;
    AT_APPCTRL                     appCtrl;
    TAF_XPDS_InitialTypeUint32     agpsMode;
    AT_XPDS_AgpsDataCallCtrlUint32 agpsOper;
} XPDS_AT_ApDataCallReq;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32          msgId;
    AT_APPCTRL                     appCtrl;
    AT_XPDS_ServerModeUint32       serverMode;
    AT_XPDS_AgpsDataCallCtrlUint32 dataCallCtrl;
    VOS_UINT32                     ipAddr;
    VOS_UINT32                     portNum;
} XPDS_AT_ApServerBindReq;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32    msgId;
    AT_APPCTRL               appCtrl;
    AT_XPDS_ServerModeUint32 serverMode;
    VOS_UINT32               dataLen;
    VOS_UINT8                data[4];
} XPDS_AT_ApReverseDataInd;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32        msgId;
    AT_APPCTRL                   appCtrl;
    AT_XPDS_DataCallStatusUint32 channelState;
} AT_XPDS_ApDataCallStatusInd;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32          msgId;
    AT_APPCTRL                     appCtrl;
    AT_XPDS_ServerModeUint32       serverMode;
    AT_XPDS_ServerBindStatusUint32 bindStatus;
} AT_XPDS_ApServerBindStatusInd;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32    msgId;
    AT_APPCTRL               appCtrl;
    AT_XPDS_ServerModeUint32 serverMode;
    VOS_UINT32               totalNum;
    VOS_UINT32               curNum;
    VOS_UINT32               dataLen;
    VOS_UINT8                data[4];
} AT_XPDS_ApForwardDataInd;

/*
 * Description: 系统时间数据结构
 */
typedef struct {
    VOS_UINT16 year;
    VOS_UINT16 month;
    VOS_UINT16 dayOfWeek;
    VOS_UINT16 day;
    VOS_UINT16 hour;
    VOS_UINT16 minute;
    VOS_UINT16 second;
    VOS_UINT16 milliseconds;
    /* *< GPS week as the number of whole weeks since GPS time zero */
    VOS_UINT32 gpsWeek;
    VOS_UINT32 gpsTimeOfWeek; /* *< GPS time of week in seconds */
} AT_XPDS_GpsSysTime;

/*
 * Description: GPS位置error参数
 */
typedef struct {
    VOS_UINT32 horizontalErrorAlong;
    VOS_UINT32 horizontalErrorAngle;
    VOS_UINT32 horizontalErrorPerp;
    VOS_UINT32 verticalError;
    VOS_UINT32 horizontalConfidence;
    VOS_UINT32 horizontalVelocityError; /* *< Horizontal velocity uncertainty in m/s */
    VOS_UINT32 verticalVelocityError;   /* *< Vertical velocity uncertainty in m/s */
    VOS_UINT32 horinzontalHeadingError; /* *< Horizontal heading uncertainty in degrees */
    VOS_UINT32 latitudeUncertainty;     /* *< Latitude uncertainty */
    VOS_UINT32 longitudeUncertainty;    /* *< Longitude Uncertainty */
} TAF_MSG_CdmaGpsPositionError;

/*
 * Description: GPS Module计算出的位置信息数据结构
 */
typedef struct {
    VOS_UINT32         validityMask;
    AT_XPDS_GpsSysTime utcTime;
    /* *< in degrees, positive number indicate north latitude */
    VOS_INT32 latitude;
    /* *< in degrees, positive number indicate east longitude */
    VOS_INT32 longitude;
    /* *< in knots (nautical miles) */
    VOS_UINT32 speed;
    /* *< in degrees, a heading of zero is true north */
    VOS_UINT32 heading;
    /* *< the difference between the bearing to true north and the bearing shown on a magnetic compass, positive numbers
     * indicate east */
    VOS_UINT32 magneticVariation;
    /* *< in meters, with respect to sea level */
    VOS_UINT32 altitudeWRTSeaLevel;
    /* *< in meters, with respect to the WGS84 ellipsoid */
    VOS_UINT32                        altitudeWRTEllipsoid;
    TAF_MSG_CdmaGpsFixQualityUint8    fixQuality;
    TAF_MSG_CdmaGpsFixTypeUint8       fixType;
    TAF_MSG_CdmaGpsSelectionTypeUint8 selectionType;
    /* *< padding */
    VOS_UINT8  reserved;
    VOS_UINT32 positionDilutionOfPrecision;
    VOS_UINT32 horizontalDilutionOfPrecision;
    VOS_UINT32 verticalDilutionOfPrecision;
    /* *< number of satellites used to obtain the position */
    VOS_UINT16                   satelliteCount;
    VOS_UINT16                   satellitesUsedPRNs[TAF_MSG_CDMA_MAX_SV_NUM];
    VOS_UINT16                   satellitesInView;
    VOS_UINT16                   satellitesInViewPRNs[TAF_MSG_CDMA_MAX_SV_NUM];
    VOS_INT16                    satellitesInViewElevation[TAF_MSG_CDMA_MAX_SV_NUM];
    VOS_UINT16                   satellitesInViewAzimuth[TAF_MSG_CDMA_MAX_SV_NUM];
    VOS_UINT16                   satellitesInViewSNR[TAF_MSG_CDMA_MAX_SV_NUM];
    TAF_MSG_CdmaGpsPositionError stGPSPositionError;

    /* *< the number of GPS fixes attempted */
    VOS_UINT32 gpsSessionCount;
    /* *< the number of positions */
    VOS_UINT32 numberOfPositions;
    /* *< Horizontal velocity in m/s */
    VOS_UINT32 horizontalVelocity;
    /* *< Vertical velocity in m/s */
    VOS_UINT32 verticalVelocity;
} AT_XPDS_GpsPosInfo;

/*
 * Description: GPS Module计算出的位置信息数据结构
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    AT_XPDS_GpsPosInfo    posInfo;
} AT_XPDS_GpsPosInfoRsp;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32                  msgId;
    AT_APPCTRL                             appCtrl;
    VOS_UINT8                              len;
    TAF_XPDS_NotiftAndVerifyIndicatorUint8 notificationInd;
    TAF_XPDS_PosTechIndicatorUint8         posTechInd;
    VOS_UINT8                              posQoSInc;
    VOS_UINT8                              posQoS;
    TAF_XPDS_Is801ModeUint8                is801PosMode;
    VOS_UINT8                              correlationId;
    VOS_UINT8                              reqIdEncode;
    VOS_UINT8                              reqIdLen;
    TAF_XPDS_CallTypeUint8                 callType;
    VOS_UINT8                              rsv[2];
    VOS_UINT8                              data[256];
    VOS_UINT16                             numOfFixs;
    VOS_UINT16                             timeBetweenFixs;
} XPDS_AT_GpsNiSessionInd;

/*
 * Description:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
} XPDS_AT_GpsStartReq;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT32            data; /* 默认1 */
} XPDS_AT_GpsCancelInd;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32        msgId;
    AT_APPCTRL                   appCtrl;
    XPDS_AT_AbortPosReasonUint16 abortReason;
    VOS_UINT16                   rsv;
} XPDS_AT_GpsAbortInd;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32  msgId;
    AT_APPCTRL             appCtrl;
    XPDS_AT_SyncTimeUint32 result;
    VOS_UINT32             pulseTime[2];
} XPDS_AT_GpsSyncTimeInd;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    /* *< Alpha and beta parameters included or not. 0: Not include, 1: included */
    VOS_UINT8 abParIncl;
    VOS_UINT8 alpha0;      /* *< Ionospheric correction parameter Alpha0 */
    VOS_UINT8 alpha1;      /* *< Ionospheric correction parameter Alpha1 */
    VOS_UINT8 alpha2;      /* *< Ionospheric correction parameter Alpha2 */
    VOS_UINT8 alpha3;      /* *< Ionospheric correction parameter Alpha3 */
    VOS_UINT8 beta0;       /* *< Ionospheric correction parameter Beta0 */
    VOS_UINT8 beta1;       /* *< Ionospheric correction parameter Beta1 */
    VOS_UINT8 beta2;       /* *< Ionospheric correction parameter Beta2 */
    VOS_UINT8 beta3;       /* *< Ionospheric correction parameter Beta3 */
    VOS_UINT8 reserved[3]; /* *< padding */
} XPDS_AT_GpsIonInfoInd;


typedef struct {
    VOS_UINT8  svId; /* *< Satelite Vehicle ID, Range[1..32] */
    VOS_UINT8  reserved[3];
    VOS_UINT8  af2;     /* *< Apparent satelite clock correction af2 */
    VOS_UINT8  iode;    /* *< Issue of data */
    VOS_UINT16 toc;     /* *< Clock data reference time */
    VOS_UINT16 toe;     /* *< Ephemeris reference time */
    VOS_UINT16 af1;     /* *< Apparent satelite clock correction af1 */
    VOS_UINT16 delta_n; /* *< Mean motion difference from the computed value */
    /* *< Rate of inclination angle, If negative number and Masked with 0xE000 */
    VOS_UINT16 idot;
    /* *< Amplitude of the sine harmonic correction term to the orbit radius */
    VOS_UINT16 crs;
    /* *< Amplitude of the cosine harmonic correction term to the orbit radius */
    VOS_UINT16 crc;
    /* *< Amplitude of the sine harmonic correction term to the argument of latitude */
    VOS_UINT16 cus;
    /* *< Amplitude of the cosine harmonic correction term to the argument of latitude. */
    VOS_UINT16 cuc;
    /* *< Amplitude of the sine harmonic correction term to the angle of inclination */
    VOS_UINT16 cis;
    /* *< Amplitude of the cosine harmonic correction term to the angle of inclination */
    VOS_UINT16 cic;
    /* *< Apparent satellite clock correction af0,If negative number & Masked w/ 0xFFE00000 */
    VOS_UINT32 af0;
    VOS_UINT32 m0;           /* *< Mean anomaly at the reference time */
    VOS_UINT32 asqrt;        /* *< Square root of the semi-major axis */
    VOS_UINT32 eccentricity; /* *< eccentricity */
    VOS_UINT32 iangle;       /* *< Inclination angle at the reference time */
    /* *< Longitude of ascending node of orbit plane at weekly epoch */
    VOS_UINT32 omega0;
    VOS_UINT32 omega; /* *< Argument of perigee */
    /* *< Rate of right ascension, If negative number and Masked with 0xFF000000 */
    VOS_UINT32 omegadot;
} XPDS_AT_EphData;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT8             svNum; /* *< Number of SVs,Range [1..32] */
    VOS_UINT8             reserved[3];
    XPDS_AT_EphData       ephData[TAF_MSG_CDMA_MAX_EPH_PRN_NUM];
} XPDS_AT_GpsEphInfoInd;


typedef struct {
    VOS_UINT8  svId; /* *< Satelite vehicle id, Range [1..32] */
    VOS_UINT8  reserved;
    VOS_UINT16 deltai; /* *< Correction to inclination */
    /* *< Apparent satellite clock correction af0. If negative number and Masked with 0xF800 */
    VOS_UINT16 af0;
    /* *< Apparent satellite clock correction af1. If negative number and Masked with 0xF800 */
    VOS_UINT16 af1;
    VOS_UINT16 omegadot;     /* *< rate of right ascension */
    VOS_UINT16 eccentricity; /* *< Eccentricity */
    VOS_UINT32 asqrt;        /* *< Square root of the semi-major axis */
    /* *< Longitude of ascending node of orbit plane. If negative number and Masked with 0xFF000000 */
    VOS_UINT32 omega_0;
    /* *< Argument of perigee. If negative number and Masked with 0xFF000000 */
    VOS_UINT32 omega;
    /* *< Mean anoaly at reference time. If negative number and Masked with 0xFF000000 */
    VOS_UINT32 m0;
} XPDS_AT_AlmData;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    /* *< Number of SVs, Range [1..32] */
    VOS_UINT8 svNum;
    /* *< GPS week number, Range [0..255] */
    VOS_UINT8 weekNum;
    /* *< Time of almanac, in units of 4096 seconds, Range [0..602112] */
    VOS_UINT8 toa;
    VOS_UINT8 reserved;
    /* *< Set Max 32 PRNs, 5 + (27 * 32)= 869 bytes */
    XPDS_AT_AlmData almData[TAF_MSG_CDMA_MAX_ALM_PRN_NUM];
} XPDS_AT_GpsAlmInfoInd;


typedef struct {
    VOS_UINT32 deleteFlag; /* 1:删除辅助数据 */
} XPDS_AT_GpsDelAssistDataInd;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT32            startCode; /* 0:默认开启 */
} XPDS_AT_GpsNiCpStart;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT32            startCode; /* 0:默认开启 */
} XPDS_AT_GpsNiCpStop;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    AT_XPDS_GpsPosInfo    posInfo;
} XPDS_AT_UtsGpsPosInfoInd;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32      msgId;
    AT_APPCTRL                 appCtrl;
    TAF_XPDS_FixModeUint8      fixMode; /* *< Unkown: 0; MSA: 1, MSB: 2, MSS: 3, AFLT: 4 */
    TAF_XPDS_GpsStartModeUint8 startMode;
    VOS_UINT8                  rsv[2];
} XPDS_AT_GpsOmTestStartReq;


typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
} XPDS_AT_GpsOmTestStopReq;

/*
 * H2ASN顶级消息结构定义
 */
typedef struct {
    /* _H2ASN_MsgChoice_Export AT_XPDS_MSG_TYPE_ENUM_UINT32 */
    AT_XPDS_MsgTypeUint32 msgId;
    AT_APPCTRL            appCtrl;
    VOS_UINT8             msgBlock[4];
    /*
     * _H2ASN_MsgChoice_When_Comment          AT_XPDS_MsgTypeUint32
     */
} AT_XPDS_MsgData;

typedef struct {
    VOS_MSG_HEADER
    AT_XPDS_MsgData msgData;
} AtXpdsInterface_Msg;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
