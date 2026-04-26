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

#ifndef __ATCMDMSGPROC_H__
#define __ATCMDMSGPROC_H__

#include "vos.h"
#include "at_type_def.h"
#include "at_ctx.h"
#include "acpu_reset.h"
#include "taf_drv_agent.h"
#include "at_mta_interface.h"
#include "at_internal_msg.h"
#if (FEATURE_IMS == FEATURE_ON)
#include "at_imsa_interface.h"
#endif

#include "taf_app_mma.h"

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
#include "at_xpds_interface.h"
#endif

#include "taf_ccm_api.h"

#include "at_lte_eventreport.h"
#include "at_set_para_cmd.h"
#include "dms_port_def.h"
#include "taf_msg_chk_api.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_AT_INVALID_MCC 0xffffffff
/* CESQ命令Ecno参数相关宏定义 */
#define AT_CMD_CESQ_ECNO_MIN_VALUE 0
#define AT_CMD_CESQ_ECNO_MAX_VALUE 49
#define AT_CMD_CESQ_ECNO_INVALID_VALUE 255
#define AT_CMD_CESQ_ECNO_CONVERT_BASE_VALUE 49
#define AT_CMD_CESQ_ECNO_LOWER_BOUNDARY_VALUE (-48)
#define AT_CMD_CESQ_ECNO_UPPER_BOUNDARY_VALUE 0

/* CESQ命令Rsrq参数相关宏定义 */
#define AT_CMD_CESQ_RSRQ_MIN_VALUE 0
#define AT_CMD_CESQ_RSRQ_MAX_VALUE 34
#define AT_CMD_CESQ_RSRQ_INVALID_VALUE 255
#define AT_CMD_CESQ_RSRQ_CONVERT_BASE_VALUE 40
#define AT_CMD_CESQ_RSRQ_LOWER_BOUNDARY_VALUE (-39)
#define AT_CMD_CESQ_RSRQ_UPPER_BOUNDARY_VALUE (-6)

/* CESQ命令Rsrp参数相关宏定义 */
#define AT_CMD_CESQ_RSRP_MIN_VALUE 0
#define AT_CMD_CESQ_RSRP_MAX_VALUE 97
#define AT_CMD_CESQ_RSRP_INVALID_VALUE 255
#define AT_CMD_CESQ_RSRP_CONVERT_BASE_VALUE 141
#define AT_CMD_CESQ_RSRP_LOWER_BOUNDARY_VALUE (-140)
#define AT_CMD_CESQ_RSRP_UPPER_BOUNDARY_VALUE (-44)
#define AT_PSEUD_BTS_PARAM_ENABLE 1 /* 查询伪基站是否支持 */

#define AT_FEATURE_BAND_STR_LEN_MAX 512

#define AT_PB_DECODE_LEN_MAX 500



extern VOS_INT8 g_ate5DissdPwd[AT_DISSD_PWD_LEN + 1];

/* 消息处理函数指针 */
typedef VOS_UINT32 (*pAtProcMsgFromDrvAgentFunc)(struct MsgCB *pMsg);

/* AT与MMA模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_MMA_MSG_PROC_FUNC)(struct MsgCB *pMsg);

typedef VOS_UINT32 (*AT_XCALL_MSG_PROC_FUNC)(struct MsgCB *pMsg);

typedef VOS_UINT32 (*AT_CCM_MSG_PROC_FUNC)(struct MsgCB *pMsg);

typedef VOS_VOID (*AT_MSG_PROC_FUNC)(struct MsgCB *pMsg);

typedef VOS_UINT32 (*AT_VNAS_ProcFunc)(struct MsgCB *msg);

typedef VOS_UINT32 (*AT_DMS_MSG_PROC_FUNC)(struct MsgCB *msg);

typedef VOS_UINT32 (*AT_DMS_EVENT_PROC_FUNC)(struct MsgCB *msg);

typedef VOS_VOID (*AT_VC_MSG_PROC_FUNC)(MN_AT_IndEvt *pMsg);

/*
 * 结构说明: 消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    DRV_AGENT_MsgTypeUint32    msgType;
    pAtProcMsgFromDrvAgentFunc procMsgFunc;
} AT_PROC_MsgFromDrvAgent;
/*lint +e958 +e959 ;cause:64bit*/

/*
 * 结构说明: 消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32       sndPid;
    AT_MSG_PROC_FUNC procMsgFunc;
} AT_MSG_Proc;
/*lint +e958 +e959 ;cause:64bit*/

typedef struct {
    VOS_UINT32                 msgName;
    AT_VC_MSG_PROC_FUNC        procMsgFunc;
} AT_PROC_MsgFromVc;

/*
 * Structure: NAS_AT_OUTSIDE_RUNNING_CONTEXT_PART_ST
 * Description: PC回放工程，存储所有AT相关的全局变量，目前仅有短信相关全局变量
 * Message origin:
 * Note:
 */
typedef struct {
    VOS_UINT8         used;     /* 指示当前索引是否已被使用 */
    AT_USER_TYPE      userType; /* 指示当前用户类型 */
    AT_IND_MODE_TYPE  indMode;  /* 指示当前命令模式，只针对MUX和APP */
    VOS_UINT8         reserv1[1];
    VOS_UINT16        clientId; /* 指示当前用户的 */
    MN_OPERATION_ID_T opId;     /* Operation ID, 标识本次操作             */
    VOS_UINT8         reserv2[1];
} NAS_AT_ClientManageSimple;

/*
 * Structure: NAS_AT_SdtAtClientTable
 * Description: PC回放工程，所有AT相关的全局变量通过以下消息结构发送
 * Message origin:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_InterMsgIdUint32       msgID;
    VOS_UINT8                 type;
    VOS_UINT8                 reserved[3]; /* 在PACK(1)到PACK(4)调整中定义的保留字节 */
    NAS_AT_ClientManageSimple atClientTab[AT_MAX_CLIENT_NUM];
} NAS_AT_SdtAtClientTable;

typedef struct {
    AT_CSCS_TYPE               atCscsType;
    AT_CSDH_TYPE               atCsdhType;
    MN_OPERATION_ID_T          opId;
    MN_MSG_CsmsMsgVersionUint8 atCsmsMsgVersion;
    AT_CnmiType                atCnmiType;
    AT_CmgfMsgFormatUint8      atCmgfMsgFormat;
    VOS_UINT8                  reserved[3]; /* 在PACK(1)到PACK(4)调整中定义的保留字节 */
    AT_CSCA_CsmpInfo           atCscaCsmpInfo;
    AT_MSG_Cpms                atCpmsInfo;
} NAS_AT_OUTSIDE_RUNNING_CONTEXT_PART_ST;

/*
 * Structure: NAS_AT_SDT_AT_PART_ST
 * Description: PC回放工程，所有AT相关的全局变量通过以下消息结构发送
 * Message origin:
 */
typedef struct {
    VOS_MSG_HEADER
    AT_InterMsgIdUint32                    msgID;       /* 匹配AT_MSG_STRU消息中的ulMsgID  */
    VOS_UINT8                              type;        /* 之前是ucMsgIDs */
    VOS_UINT8                              reserved[3]; /* 在PACK(1)到PACK(4)调整中定义的保留字节 */
    NAS_AT_OUTSIDE_RUNNING_CONTEXT_PART_ST outsideCtx[MODEM_ID_BUTT];
} NAS_AT_SDT_AT_PART_ST;

/*
 * 结构说明: AT与MTA消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32           msgName;
    AT_MMA_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_MsgFromMma;
/*lint +e958 +e959 ;cause:64bit*/

/*
 * 结构名    : AT_PROC_MSG_FROM_CALL_STRU
 * 结构说明  : AT与XCALL消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
/*lint +e958 +e959 ;cause:64bit*/

/*
 * 结构说明: AT与CCM消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    TAF_CCM_MsgTypeUint32 msgName;
    AT_CCM_MSG_PROC_FUNC  procMsgFunc;
} AT_PROC_MsgFromCcm;
/*lint +e958 +e959 ;cause:64bit*/

/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32       msgType;
    AT_VNAS_ProcFunc procMsgFunc;
}AT_VNAS_ProcMsgTbl;
/*lint +e958 +e959 ;cause:64bit*/

/*
 * 结构说明: AT与DMS消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32           msgName;
    AT_DMS_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_MsgFromDms;
/*lint +e958 +e959 ;cause:64bit*/

/*
 * 结构说明: AT与DMS消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    DMS_SubscripEventIdUint32 eventId;
    VOS_UINT32                fixdedPartLen;
    TAF_ChkMsgLenFunc         chkFunc;
    AT_DMS_EVENT_PROC_FUNC    procEventFunc;
} AT_PROC_EventFromDms;
/*lint +e958 +e959 ;cause:64bit*/

#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON && FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_VOID AT_SetCsCallStateWakeLock(TAF_CCM_MsgTypeUint32 msgName);
#endif

extern VOS_VOID At_CmdMsgDistr(struct MsgCB *msg);
extern VOS_BOOL AT_E5CheckRight(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_UINT32 AT_RcvMmaLocInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_VOID   At_PppReleaseIndProc(AT_ClientIdUint16 clientId);
extern VOS_UINT32 AT_RcvMtaCgpsClockSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_VOID   AT_ReportResetCmd(AT_ResetReportCauseUint32 cause);
extern VOS_VOID   AT_StopAllTimer(VOS_VOID);
extern VOS_VOID   AT_ResetParseCtx(VOS_VOID);
extern VOS_VOID   AT_ResetClientTab(VOS_VOID);
extern VOS_VOID   AT_ResetOtherCtx(VOS_VOID);
extern VOS_UINT32 AT_RcvCcpuResetStartInd(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvCcpuResetEndInd(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvHifiResetBeginInd(struct MsgCB *msg);
VOS_UINT32 AT_RcvHifiResetEndInd(struct MsgCB *msg);

#if (VOS_WIN32 == VOS_OS_VER)
extern VOS_UINT32 At_PidInit(enum VOS_InitPhaseDefine phase);
#endif
VOS_VOID AT_ReportNetScanInfo(VOS_UINT8 indexNum, VOS_UINT8 freqNum, TAF_MMA_NetScanInfo *netScanInfo);
VOS_UINT32 AT_RcvMmaCbcQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID At_TafAndDmsMsgProc(struct MsgCB *msg);
TAF_VOID At_PppMsgProc(struct MsgCB *msg);
VOS_VOID AT_RcvNdisMsg(struct MsgCB *msg);
VOS_VOID At_HPAMsgProc(struct MsgCB *msg);
VOS_VOID At_GHPAMsgProc(struct MsgCB *msg);
VOS_VOID At_ProcMsgFromDrvAgent(struct MsgCB *msg);
VOS_VOID At_ProcMsgFromVc(struct MsgCB *msg);
VOS_VOID At_ProcMsgFromCc(struct MsgCB *msg);
VOS_VOID At_RcvRnicMsg(struct MsgCB *msg);
TAF_VOID AT_RabmMsgProc(struct MsgCB *msg);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID At_CHPAMsgProc(struct MsgCB *msg);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMta5gNssaiQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_VOID AT_ProcDmsMsg(struct MsgCB *msg);
VOS_UINT32 AT_ChkDmsSubscripEventNotify(const MSG_Header *msg);
VOS_UINT32 AT_RcvDmsSubscripEventNotify(struct MsgCB *msg);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_cmd_msg_proc.h */
