/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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
 * *    documentation and/or other materials provided with the distribution
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
#ifndef __ATSETPARACMD_H__
#define __ATSETPARACMD_H__

#include "vos.h"
#include "taf_ps_api.h"
#include "pppa_ppps_at_type.h"
#include "taf_ps_type_def.h"
#include "taf_mmi_str_parse.h"
#include "si_app_pb.h"
#include "at_mta_interface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#define AT_SIMLOCKDATAWRITE_PARA_LEN 1032 /* 锁网锁卡改制命令参数结构体长度 */
#define AT_BITS_OF_ONE_BYTE 8
#define AT_OOS_SRCH_STGY_TIMER_LEN_PARA_LEN 2
#define AT_COMMAND_MAX_PARA_NUM 1
#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
#define AT_CSDBTRDP_PARA_MAX_NUM 2
#endif

typedef VOS_UINT8  AT_SYSCFG_RatPrioUint8;
typedef VOS_UINT32 AT_PhynumTypeUint32;

typedef struct {
    /* 结构体码流, 包含该锁网锁卡类型的包括状态, 锁网号段, CK, UK等所有信息(未做half-byte解析) */
    VOS_UINT8 categoryData[AT_SIMLOCKDATAWRITE_PARA_LEN];
} AT_SIMLOCKDATAWRITE_SetReq;

typedef struct {
    VOS_UINT8 *phynumTypeData;
    VOS_UINT8 *phonePhynumValue;
    VOS_UINT8 *hmacData;
    VOS_UINT16 phynumTypeLen;
    VOS_UINT16 phonePhynumValueLen;
    VOS_UINT16 hmacLen;
    VOS_UINT8  rsv[2];
} AT_PhonePhynumPara;

typedef VOS_UINT32 (*AT_CnmiOptTypeMatchFunc)(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
                                              ModemIdUint16 modemId);

typedef struct {
    VOS_UINT32                            optType;
    AT_CnmiOptTypeMatchFunc               pktTypeMatchFunc;
} AT_CnmiOptTypeMatchEntity;


typedef VOS_UINT32 (*AT_ClckFacTypeMatchFunc)(TAF_UINT8 indexNum);

typedef struct {
    VOS_UINT32                            facType;
    AT_ClckFacTypeMatchFunc               facTypeMatchFunc;
} AT_ClckFacTypeMatchEntity;


typedef struct {
    VOS_CHAR                        firstChar;
    VOS_CHAR                        secondChar;
    VOS_UINT16                      reserved;
} AT_PortSecParaCheckEntity;


typedef struct {
    VOS_UINT32                        atCmdType;
    VOS_UINT32                        siStkType;
} AT_AtCmdConvertSiStkTypeEntity;

TAF_UINT32 At_AsciiNum2HexString(TAF_UINT8 *src, TAF_UINT16 *srcLen);
VOS_BOOL AT_IsNVWRAllowedNvId(VOS_UINT16 nvId);
TAF_UINT32 At_AsciiString2HexSimple(TAF_UINT8 *textStr, TAF_UINT8 *src, TAF_UINT16 srcLen);
TAF_UINT32 AT_HexToAsciiString(TAF_UINT8 *dstStr, TAF_UINT32 dstLen, TAF_UINT8 *srcStr, TAF_UINT32 srcLen);
TAF_UINT32 At_UnicodePrint2Unicode(TAF_UINT8 *data, TAF_UINT16 *len);
TAF_UINT32 At_SetNumTypePara(TAF_UINT8 *dst, TAF_UINT32 dstLen, TAF_UINT8 *src, TAF_UINT16 srcLen);
VOS_UINT32 AT_DecodeSNssai(VOS_UINT8 *sNssai, VOS_UINT16 sNssaiLen, PS_S_NSSAI_STRU *sNssaiOut);
TAF_UINT32 At_SetCssnPara(TAF_UINT8 indexNum);
TAF_UINT32 At_CheckDialString(TAF_UINT8 *data, TAF_UINT16 len);
VOS_UINT32 AT_CheckApnFormat(VOS_UINT8 *apn, VOS_UINT16 apnLen, VOS_UINT16 clientId);

VOS_UINT32 At_SetCsndPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_DigitString2Hex(VOS_UINT8 *digitStr, VOS_UINT16 len, VOS_UINT32 *destHex);
TAF_UINT32 At_SetCgmiPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCsqPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetCLACPara(VOS_UINT8 indexNum);
#if (VOS_OS_VER == VOS_WIN32)
VOS_UINT32 At_SetDelayBgStub(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_FillSsBsService(TAF_SS_ErasessReq *sSPara, AT_ParseParaType *atPara);
AT_MTA_PersCategoryUint8 AT_GetSimlockUnlockCategoryFromClck(VOS_VOID);
VOS_UINT32 AT_FillCalledNumPara(VOS_UINT8 *atPara, VOS_UINT16 len, MN_CALL_CalledNum *calledNum);
VOS_VOID AT_SetDefaultRatPrioList(ModemIdUint16 modemId, TAF_MMA_MultimodeRatCfg *sysCfgRatOrder,
                                  VOS_UINT8 userSptLteFlag, VOS_UINT8 userSptNrFlag);

TAF_UINT32 At_SetCurcPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetParaCmd(TAF_UINT8 indexNum);
VOS_UINT32 AT_PhyNumIsNull(ModemIdUint16 modemId, AT_PhynumTypeUint32 setType, VOS_BOOL *pbPhyNumIsNull);
VOS_UINT32 AT_OpenSpecificPort(VOS_UINT8 port);
VOS_UINT32 AT_CloseSpecificPort(VOS_UINT8 port);
VOS_UINT32 AT_SetOpwordPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCpwordPara(VOS_UINT8 indexNum);

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
VOS_UINT32 AT_SetShellPara(VOS_UINT8 indexNum);
#endif
TAF_UINT32 At_SetCrpnPara(TAF_UINT8 indexNum);
VOS_UINT32 At_GetParaCnt(VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 At_CovertAtParaToSimlockPlmnInfo(VOS_UINT32 paraCnt, AT_ParseParaType *paralist,
                                            TAF_CUSTOM_SimLockPlmnInfo *simLockPlmnInfo);
VOS_UINT32 At_SetSimLockPlmnInfo(VOS_UINT8 indexNum, VOS_UINT32 paraCnt, AT_ParseParaType *paralist);
VOS_UINT32 AT_SetFacAuthPubkeyExPara(VOS_UINT8 indexNum, VOS_UINT32 currIndex, VOS_UINT32 total, VOS_UINT32 paraLen,
                                     VOS_UINT8 *pubKeyData);
VOS_UINT32 AT_SetSimlockDataWritePara(VOS_UINT8 indexNum, AT_SIMLOCKDATAWRITE_SetReq *simlockDataWrite);
VOS_VOID AT_ConvertMccToNasType(VOS_UINT32 mcc, VOS_UINT32 *nasMcc);
VOS_UINT32 AT_SetCISAPara(VOS_UINT8 indexNum, VOS_UINT32 lengthValue, VOS_UINT8 *command, VOS_UINT16 commandLength);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_SetCcmgsPara(VOS_UINT8 indexNum, VOS_UINT32 lengthValue, VOS_UINT8 *command, VOS_UINT16 commandLength);
VOS_UINT32 AT_SetCcmgwPara(VOS_UINT8 indexNum, VOS_UINT32 lengthValue, VOS_UINT32 statValue, VOS_UINT8 *command,
                           VOS_UINT16 commandLength);
VOS_UINT32 AT_IsPlatformSupportHrpdMode(PLATAFORM_RatCapability *platform);
VOS_UINT32 AT_IsPlatformSupport1XMode(PLATAFORM_RatCapability *platform);
#endif

TAF_UINT32 At_SetCeerPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetMsIdInfo(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetSinglePdnSwitch(VOS_UINT8 indexNum);
TAF_UINT32 AT_SetDwinsPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_IsSpecRatSupported(TAF_MMA_RatTypeUint8 ratMode, TAF_MMA_MultimodeRatCfg *ratOrder);
TAF_MMA_UserSetPrioRatUint8 AT_GetSysCfgPrioRat(TAF_MMA_SysCfgPara *sysCfgExSetPara);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_UpdateImei(ModemIdUint16 modemId, VOS_UINT8 aucImei[], VOS_UINT16 imeiLength);
#endif
TAF_UINT32 At_AsciiNum2Num(TAF_UINT8 *dst, TAF_UINT8 *src, TAF_UINT16 srcLen);
VOS_VOID At_SetCopsActPara(TAF_MMA_RatTypeUint8 *phRat, VOS_UINT16 paraLen, VOS_UINT32 paraValue);
VOS_UINT32 AT_SetHighFourBitHexString(VOS_UINT8 *outPut, VOS_UINT8 *src, VOS_UINT16 indexNum);
VOS_UINT32 AT_SetLowFourBitHexString(VOS_UINT8 *outPut, VOS_UINT8 *src, VOS_UINT16 indexNum);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_set_para_cmd.h */
