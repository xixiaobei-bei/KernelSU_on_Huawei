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

#ifndef _TAF_MMI_STR_PARSE_H_
#define _TAF_MMI_STR_PARSE_H_

#include "taf_type_def.h"
#include "mn_call_api.h"
#include "taf_app_ssa.h"
#include "taf_app_mma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* 因为新增的taf_mmi_encode.c也要使用下面宏，从taf_mmi_str_parse.c移动到taf_mmi_str_parse.c */
#define MMI_MAX_DTMF_STR_LEN 40
#define MN_MMI_MIN_USSD_LEN 2

#define MN_MMI_DONT_CARE_CHAR '?'
#define MN_MMI_START_SI_CHAR '*'
#define MN_MMI_STOP_CHAR '#'

#define MN_MMI_isdigit(c) (((c) >= '0') && ((c) <= '9'))

#define MN_MMI_STR_PTR_IS_VALID(pcFrom, pcTo) \
    ((VOS_NULL_PTR == (pcTo)) || ((VOS_NULL_PTR != (pcTo)) && ((pcFrom) <= (pcTo))))

#define MN_MMI_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define MN_MMI_MAX_SC_LEN 3   /* 目前来说，最大长度为3 */
#define MN_MMI_MAX_SIA_LEN 30 /* 来自于号码长度的最大限制 */
#define MN_MMI_MAX_SIB_LEN 8  /* PWD和PIN的长度 */
#define MN_MMI_MAX_SIC_LEN 8  /* PWD和PIN的长度 */

#define MN_MMI_MAX_BUF_SIZE 64
#define MN_MMI_MAX_PARA_NUM 4

#define TAF_MMI_MAX_UINT32_VALUE 0xffffffff

#define TAF_MMI_SS_MAX_OP_TYPE_LEN 3

#define TAF_MMI_SS_MAX_OP_STRING_LEN 3
#define TAF_MMI_CHLD_MAX_STRING_LEN 3
#define TAF_MMI_PIN_MAX_OP_STRING_LEN 7
#define TAF_MMI_PWD_MAX_OP_STRING_LEN 6
#define TAF_MMI_LI_MAX_OP_STRING_LEN 5
#define TAF_MMI_SC_MAX_STRING_LEN 4
#define TAF_MMI_IMEI_MAX_LEN 6
#define TAF_MMI_BS_MAX_LEN 3
/*
 * 3类型定义
 */
enum TAF_MMI_OperationType {
    TAF_MMI_CALL_ORIG, /* Setup a Call */
    TAF_MMI_CALL_CHLD_REQ,
    TAF_MMI_CHANGE_PIN,
    TAF_MMI_CHANGE_PIN2, /* Change Pin */
    TAF_MMI_UNBLOCK_PIN, /* Unblock Pin */
    TAF_MMI_UNBLOCK_PIN2,
    TAF_MMI_DISPLAY_IMEI,    /* Display IMEI */
    TAF_MMI_REGISTER_SS,     /* Register Operation */
    TAF_MMI_ERASE_SS,        /* Erase Operation */
    TAF_MMI_ACTIVATE_SS,     /* Activate Operation */
    TAF_MMI_DEACTIVATE_SS,   /* Deactivate Operation */
    TAF_MMI_INTERROGATE_SS,  /* Interrogate Operation */
    TAF_MMI_REGISTER_PASSWD, /* Register the password */
    /* Delete TAF_MMI_GET_PASSWD */
    TAF_MMI_FWD_CHECK_SS_IND, /* Forward Check Operation */
    TAF_MMI_PROCESS_USSD_REQ, /* Process USSD Request */
    TAF_MMI_SUPPRESS_CLIR,
    TAF_MMI_INVOKE_CLIR,
    TAF_MMI_SUPPRESS_CLIP,
    TAF_MMI_INVOKE_CLIP,
    TAF_MMI_SUPPRESS_COLR,
    TAF_MMI_INVOKE_COLR,
    TAF_MMI_SUPPRESS_COLP,
    TAF_MMI_INVOKE_COLP,
    TAF_MMI_NULL_OPERATION, /* Null Operation */
    TAF_MMI_DEACTIVATE_CCBS,
    TAF_MMI_INTERROGATE_CCBS,
    TAF_MMI_MAX_OPERATION
};
typedef TAF_UINT8 MN_MMI_OperationTypeUint8;

/* 因为新增的taf_mmi_encode.c也要使用下面结构，从taf_mmi_str_parse.c移动到taf_mmi_str_parse.c */
typedef struct {
    VOS_CHAR                 *string;
    MN_MMI_OperationTypeUint8 operationType;
    VOS_UINT8                 rsv[7];
} MN_MMI_StrOperationTbl;

typedef struct {
    VOS_CHAR ssCode[MN_MMI_MAX_SC_LEN + 1];
    VOS_CHAR sia[MN_MMI_MAX_SIA_LEN + 1];
    VOS_CHAR sib[MN_MMI_MAX_SIB_LEN + 1];
    VOS_CHAR sic[MN_MMI_MAX_SIC_LEN + 1];
} MN_MMI_ScSiPara;

typedef struct {
    VOS_CHAR *mmiSc; /* MMI value of SC */
    VOS_UINT8 netSc; /* Network SC */
    VOS_UINT8 rsv[7];
} MN_MMI_ScTable;

typedef struct {
    VOS_CHAR *mmiBs;     /* MMI value of BS */
    VOS_UINT8 netBsCode; /* Network Basic Service Code */
    VOS_UINT8 netBsType; /* Network Basic Service Type */
    VOS_UINT8 rsv[6];
} MN_MMI_BsTable;

typedef struct {
    VOS_CHAR            *mmiChldStr;
    MN_CALL_SupsCmdUint8 chldOpType;
    VOS_UINT8            rsv[7];
} MN_MMI_ChldOpTbl;

typedef struct {
    VOS_CHAR                 *ssOpStr;
    MN_MMI_OperationTypeUint8 ssOpType;
    VOS_UINT8                 rsv[7];
} MN_MMI_SsOpTbl;


typedef struct {
    MN_MMI_OperationTypeUint8 mmiOperationType; /* 当前的操作类型 */

    TAF_UINT8 reserved[2];

    MN_CALL_ClirCfgUint8 clir;

    MN_CALL_OrigParam       mnCallOrig;
    MN_CALL_SupsParam       mnCallSupsReq;
    TAF_SS_RegisterssReq    registerSsReq;
    TAF_SS_ErasessReq       eraseSsReq;
    TAF_SS_ActivatessReq    activateSsReq;
    TAF_SS_DeactivatessReq  deactivateSsReq;
    TAF_SS_InterrogatessReq interrogateSsReq;
    TAF_SS_RegpwdReq        regPwdReq;
    TAF_SS_GetpwdRsp        getPwdReq;
    TAF_SS_ProcessUssReq    processUssdReq;
    TAF_PH_PinData          pinReq;
    TAF_SS_EraseccEntryReq  ccbsEraseReq;

} MN_MMI_OperationParam;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT8  netSsCode;
    VOS_UINT8  reserved[3];
} MN_ERROR_NetSsCode;

TAF_UINT32 MN_MmiStringParse(TAF_CHAR *inMmiStr, TAF_BOOL inCall, MN_MMI_OperationParam *mmiOpParam,
                             TAF_CHAR **ppOutRestMmiStr);

extern MN_CALL_ClirCfgUint8 g_clirOperate;

VOS_UINT32 MMI_GetOporationTypeTblSize(VOS_VOID);

const MN_MMI_SsOpTbl* MMI_GetOporationTypeTblAddr(VOS_VOID);

VOS_UINT32 MMI_GetBSTblSize(VOS_VOID);

const MN_MMI_BsTable* MMI_GetBSTblAddr(VOS_VOID);

VOS_UINT32 MMI_GetSCTblSize(VOS_VOID);

const MN_MMI_ScTable* MMI_GetSCTblAddr(VOS_VOID);

extern VOS_BOOL   MMI_DecodeScAndSi(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam, MN_MMI_ScSiPara *scSiPara,
                                    VOS_CHAR **ppOutMmiStr);
extern VOS_BOOL   MMI_JudgeUssdOperation(VOS_CHAR *pcMmiStr);
extern VOS_UINT32 MMI_TransMmiSsCodeToNetSsCode(const MN_MMI_ScSiPara *scSiPara, VOS_UINT8 *netSsCode);
extern VOS_UINT32 MMI_FillInRegisterSSPara(MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                           VOS_UINT8 netSsCode);
extern VOS_UINT32 MMI_FillInEraseSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                        VOS_UINT8 netSsCode);
extern VOS_UINT32 MMI_FillInActivateSSPara(MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                           VOS_UINT8 netSsCode);
extern VOS_UINT32 MMI_FillInDeactivateSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                             VOS_UINT8 netSsCode);
extern VOS_UINT32 MMI_FillInInterrogateSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                              VOS_UINT8 netSsCode);
extern VOS_UINT32 MMI_FillInProcessUssdReqPara(VOS_CHAR *pcInMmiStr, VOS_CHAR **ppcOutRestMmiStr,
                                               MN_MMI_OperationParam *mmiOpParam);
extern VOS_VOID   MMI_JudgeMmiOperationType(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam,
                                            MN_MMI_ScSiPara *scSiPara, VOS_CHAR **ppOutRestMmiStr,
                                            MN_ERROR_NetSsCode *errNetSsCodeInfo);
extern VOS_BOOL   MMI_MatchSsOpTbl(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam, MN_MMI_ScSiPara *scSiPara,
                                   VOS_CHAR **ppOutRestMmiStr, MN_ERROR_NetSsCode *errNetSsCodeInfo);
extern VOS_BOOL MMI_JudgeSsOperation(VOS_CHAR *inMmiStr, VOS_CHAR **ppOutRestMmiStr, MN_MMI_OperationParam *mmiOpParam,
                                     MN_ERROR_NetSsCode *errorNeTSsCodeInfo);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _TAF_MMI_STR_PARSE_H_ */