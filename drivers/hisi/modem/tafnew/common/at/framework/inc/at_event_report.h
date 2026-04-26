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

#ifndef __ATEVENTREPORT_H__
#define __ATEVENTREPORT_H__

#include "si_app_emat.h"
#include "taf_ps_api.h"
#include "si_app_stk.h"
#include "mn_call_api.h"
#include "AtParse.h"
#include "at_ctx.h"
#include "si_app_pb.h"

#include "at_cmd_proc.h"
#include "at_ndserver_interface.h"

#include "taf_ccm_api.h"
#include "taf_call_comm.h"
#include "at_set_para_cmd.h"
#include "taf_msg_chk_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)


#define AT_MINUTES_OF_ONE_QUARTER 15
#define AT_SECONDS_OF_ONE_MINUTE 60
#define AT_LOCAL_TIME_ZONE_ADD_MAX_VALUE 56
#define AT_LOCAL_TIME_ZONE_DEL_MAX_VALUE 48
#define AT_MNC_LENGTH_TWO_BYTES 2
#define AT_MNC_LENGTH_THREE_BYTES 3

#define AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_PTR() (&g_atCurcRptCmdTable[0])
#define AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_SIZE() (sizeof(g_atCurcRptCmdTable) / sizeof(AT_RptCmdIndexUint8))

#define AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_PTR() (&g_atUnsolicitedRptCmdTable[0])
#define AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_SIZE() \
    (sizeof(g_atUnsolicitedRptCmdTable) / sizeof(AT_RptCmdIndexUint8))

#define AT_GET_CME_CALL_ERR_CODE_MAP_TBL_PTR() (g_atCmeCallErrCodeMapTbl)
#define AT_GET_CME_CALL_ERR_CODE_MAP_TBL_SIZE() (sizeof(g_atCmeCallErrCodeMapTbl) / sizeof(AT_CmeCallErrCodeMap))

#define AT_DEC_SIGNED_MIN_NUM (-9)


enum AT_IMS_CallDomain {
    AT_IMS_CALL_DOMAIN_LTE  = 0,
    AT_IMS_CALL_DOMAIN_WIFI = 1,
    AT_IMS_CALL_DOMAIN_NR   = 2,

    AT_IMS_CALL_DOMAIN_BUTT = 255
};
typedef VOS_UINT8 AT_IMS_CallDomainUint8;


enum AT_ECALL_Type {
    AT_ECALL_TYPE_TEST = 0,     /* test ecall */
    AT_ECALL_TYPE_RECFGURATION, /* reconfiguration call */
    AT_ECALL_TYPE_MIEC,         /* manually initiated ecall */
    AT_ECALL_TYPE_AIEC,         /* automatic initiated ecall */

    AT_ECALL_TYPE_BUTT
};
typedef VOS_UINT8 AT_ECALL_TypeUint8;

extern VOS_UINT32 g_atAppDialModeCnf;

typedef TAF_VOID (*AT_QRY_PARA_PROC_FUNC)(TAF_UINT8 ucIndex, TAF_UINT8 OpId, TAF_UINT8 *pPara);

/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32            queryType;         /* 查询类型 */
    AT_QRY_PARA_PROC_FUNC atQryParaProcFunc; /* 查询类型对应的处理函数 */
} AT_QueryTypeFunc;
/*lint +e958 +e959 ;cause:64bit*/


typedef struct {
    VOS_UINT32         cmeCode;
    TAF_CS_CauseUint32 csCause;

} AT_CmeCallErrCodeMap;


typedef struct {
    VOS_UINT32 cmsCode;
    VOS_UINT32 smsCause;

} AT_CMS_SmsErrCodeMap;

typedef struct {
    TAF_ERROR_CodeUint32 tafErrCode;
    AT_RreturnCodeUint32 atReturnCode;

} AT_ChgTafErrCodeTbl;

typedef VOS_UINT32 (*AT_PIH_RSP_PROC_FUNC)(TAF_UINT8 ucIndex, SI_PIH_EventInfo *pstEvent, VOS_UINT16 *pusLength);

typedef struct {
    SI_PIH_EventUint32   eventType;
    VOS_UINT32           fixdedPartLen; /* 通过消息关联的结构可以直接计算出来消息长度的，直接标记消息有效长度 */
    TAF_ChkMsgLenFunc    chkFunc;       /* 消息结构中存在长度变量指示字段的，需要定义函数计算有效长度 */
    AT_PIH_RSP_PROC_FUNC atPihRspProcFunc;  //lint !e958
} AT_PIH_RspProcFunc;

typedef struct {
    MTA_AT_ResultUint32  mtaErrCode;
    AT_RreturnCodeUint32 atReturnCode;

} AT_ChgMtaErrCodeTbl;

typedef VOS_UINT32 (*AT_MnCallBackCmdCnfCmdOptMatchFunc)(TAF_UINT8 indexNum, TAF_UINT32 errorCode);


typedef struct {
    VOS_UINT32                              atCmdType;
    AT_MnCallBackCmdCnfCmdOptMatchFunc      pktTypeMatchFunc;
} AT_MnCallBackCmdCnfCmdOptEntity;

typedef VOS_UINT32 (*AT_VC_EventMatchFunc)(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvt);

typedef struct {
    APP_VC_EventUint32                      event;
    AT_VC_EventMatchFunc                    atVcEventMatchFunc;
} AT_VC_EventProcFunc;


VOS_UINT32 AT_ChkEmatAtEventCnfMsgLen(const MSG_Header *msgHeader);

/*
 * 功能描述: LOG打印消息处理
 */
VOS_VOID AT_LogPrintMsgProc(TAF_MntnLogPrint *msg);

/*
 * 功能描述: PS域事件处理函数
 */
VOS_VOID AT_RcvTafPsEvt(TAF_PS_Evt *evt);

VOS_UINT32 AT_QryParaRspCeregProc(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryParaRspC5gregProc(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_VOID AT_ConvertMultiSNssaiToString(VOS_UINT8 sNssaiNum, const PS_S_NSSAI_STRU *sNssai, VOS_CHAR *pcStrNssai,
                                       VOS_UINT32 srcNssaiLength, VOS_UINT32 *dsrLength);
#endif

TAF_VOID At_StkCsinIndPrint(TAF_UINT8 indexNum, SI_STK_EventInfo *event);

VOS_VOID AT_ConvertNasMccToBcdType(VOS_UINT32 nasMcc, VOS_UINT32 *mcc);

VOS_UINT32 AT_CheckRptCmdStatus(VOS_UINT8 *rptCfg, AT_CmdRptCtrlTypeUint8 rptCtrlType, AT_RptCmdIndexUint8 rptCmdIndex);

TAF_UINT32 At_CcClass2Print(MN_CALL_TypeUint8 callType, TAF_UINT8 *dst);

VOS_VOID At_StkCcinIndPrint(TAF_UINT8 indexNum, SI_STK_EventInfo *event);

TAF_UINT32 AT_ProcOperModeWhenLteOn(VOS_UINT8 indexNum);

VOS_VOID   AT_FlushSmsIndication(VOS_VOID);

VOS_UINT32 AT_IsClientBlock(VOS_VOID);

VOS_UINT32 AT_ConvertCallError(TAF_CS_CauseUint32 cause);

VOS_UINT32 AT_RcvTafCcmQryCallInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_VOID AT_ReportSmMeFull(VOS_UINT8 indexNum, MN_MSG_MemStoreUint8 memStore);

TAF_UINT32 At_HexText2AsciiStringSimple(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT32 len,
                                        TAF_UINT8 *str);

VOS_VOID AT_PB_ReadContinueProc(AT_ClientIdUint16 indexNum);

VOS_VOID AT_PB_ReadRspProc(VOS_UINT8 indexNum, SI_PB_EventInfo *event);

VOS_UINT32 AT_RcvTafPsEvtGet1xDormTimerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

VOS_VOID AT_ConvertNasMncToBcdType(VOS_UINT32 nasMnc, VOS_UINT32 *mnc);

VOS_UINT32 AT_Hex2AsciiStrLowHalfFirst(VOS_UINT32 maxLength, VOS_INT8 *pcHeadaddr, VOS_UINT8 *dst, VOS_UINT8 *src,
                                       VOS_UINT16 srcLen);

VOS_UINT32 AT_IsMonthHaveThirtyOneDays(VOS_UINT8 month);
VOS_UINT32 AT_IsMonthHaveThirtyDays(VOS_UINT8 month);
VOS_UINT32 AT_IsLeapYear(VOS_UINT16 year);
VOS_VOID AT_PrcoPsEvtErrCode(VOS_UINT8 indexNum, TAF_PS_CauseUint32 cuase);
#pragma pack(pop)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
