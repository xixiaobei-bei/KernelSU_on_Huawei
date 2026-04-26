/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef _AT_DEVICE_TAF_SET_CMD_PROC_H_
#define _AT_DEVICE_TAF_SET_CMD_PROC_H_

#include "vos.h"
#include "AtParse.h"
#include "at_mt_interface.h"
#include "taf_drv_agent.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#define AT_SIM_LOCK_DATA_WRITEEX_MAX_TOTAL 42 /* 对total进行限制，不能超过42，防止超过OSA规格60k */

typedef VOS_VOID (*AT_SpecialCmdProcFunc)(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len,
            const VOS_CHAR *specialCmdName);

typedef struct {
    const VOS_CHAR        *specialCmdName; /* 特殊命令名 */
    AT_SpecialCmdProcFunc  specialCmdFunc; /* 特殊命令处理函数 */
} AT_SpecialCmdFuncTbl;

VOS_UINT32 AT_NVWRGetParaInfo(AT_ParseParaType *para, VOS_UINT8 *pu8Data, VOS_UINT32 bufLen, VOS_UINT32 *len);

VOS_UINT32 AT_SetNVReadPara(VOS_UINT8 clientId);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_BOOL AT_JudgeNvOperationValid(VOS_VOID);
#endif
#ifdef FEATURE_AT_NV_WRITE_SUPPORT
VOS_UINT32 AT_SetNVWritePara(VOS_UINT8 clientId);
#endif
#ifdef FEATURE_AT_NV_WRITE_SUPPORT
extern VOS_UINT32 AT_SetNVWRPartPara(VOS_UINT8 clientId);
#endif

VOS_UINT32 atSetNVRDLenPara(VOS_UINT8 clientId);
VOS_UINT32 atSetNVRDExPara(VOS_UINT8 clientId);
#ifdef FEATURE_AT_NV_WRITE_SUPPORT
VOS_UINT32 AT_SetNvWrExPara(VOS_UINT8 clientId);
#endif
unsigned int AtBase64Decode(const VOS_UINT8 *pcode, const unsigned int codeSize, VOS_UINT8 *outPdata,
                            const unsigned int dataSize, unsigned int *outPwritedDataSize);
VOS_UINT32 AT_SetIdentifyEndPara(VOS_UINT8 indexNum, VOS_UINT8 *rsaData, VOS_UINT32 rsaLen, VOS_UINT8 otaRsaFlag);
VOS_UINT32 AT_SetPhonePhynumPara(VOS_UINT8 indexNum);
TAF_UINT32 At_CheckNumCharString(TAF_UINT8 *data, TAF_UINT16 len);
VOS_UINT32 At_SetBsn(VOS_UINT8 indexNum);
VOS_UINT32 At_WriteSnPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetSnPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetTmodeAutoPowerOff(TAF_PH_TmodeUint8 *currentTMode, VOS_UINT8 indexNum);
TAF_UINT32 At_SetTModePara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetTmmiPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetTestScreenPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetProdNamePara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetDataLock(VOS_UINT8 indexNum);
VOS_UINT32 At_SetMaxLockTimes(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCallSrvPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMDatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetFacInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_UpdateMacPara(VOS_UINT8 aucMac[], VOS_UINT16 macLength);
VOS_UINT32 AT_SetPhyNumPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetQosPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiwepPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCmdlenPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRsfwPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetIccidPara(TAF_UINT8 indexNum);
VOS_UINT32 atSetFLNAPara(VOS_UINT8 clientId);
#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetUartTest(VOS_UINT8 indexId);
VOS_UINT32 AT_SetI2sTest(VOS_UINT8 indexId);
#endif

VOS_UINT32 At_SetFpaPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetFAgcgainPara(VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 AT_SetFDac(VOS_UINT8 indexNum);
#else
VOS_UINT32 At_CovertAtPaLevelToBbicCal(AT_CmdPalevelUint8 atPaLevel, BBIC_CAL_PA_MODE_ENUM_UINT8 *bbicPaLevel);
#endif

VOS_UINT32 AT_SetWebPwdPara(VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetSfm(VOS_UINT8 indexNum);
#endif
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
