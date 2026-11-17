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

#ifndef _AT_MT_COMM_H_
#define _AT_MT_COMM_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtMtInterface.h"


#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif


#pragma pack(4)


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define AT_TSELRF_PATH_TOTAL_MT            (16)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  7 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_UINT32  At_SetFChanPara(VOS_UINT8 ucIndex );
VOS_UINT32  AT_SetFwavePara(VOS_UINT8 ucIndex);
VOS_UINT32  At_SetFTxonPara(VOS_UINT8 ucIndex );
VOS_UINT32  At_SetFRxonPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetTSelRfPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_SetFpaPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetDcxotempcompPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_SetDpdtPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_SetQryDpdtPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFChanPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFTxonPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFRxonPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryFpowdetTPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryFPllStatusPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFrssiPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryTSelRfPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFpaPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryDcxotempcompPara(VOS_UINT8 ucIndex);

#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
extern VOS_UINT32 AT_SetPdmCtrlPara(VOS_UINT8 ucIndex);

extern VOS_UINT32  At_QryCltInfo(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetGlobalFchan(VOS_UINT8 ucRatMode);

VOS_UINT32 AT_SetTlRficSsiRdPara(VOS_UINT8 ucIndex);

VOS_UINT32  At_SetFlnaPara(VOS_UINT8 ucIndex );

VOS_UINT32  AT_SetFDac(VOS_UINT8 ucIndex );
VOS_UINT32  AT_SetMipiWrPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetSSIWrPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetSSIRdPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetPhyMipiWritePara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetMipiRdPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetMipiReadPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_SetCltPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_QryFlnaPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryFDac(VOS_UINT8 ucIndex );
VOS_UINT32 At_TestFdacPara(VOS_UINT8 ucIndex);
VOS_UINT8 At_GetDspLoadMode(VOS_UINT32 ulRatMode);

VOS_VOID AT_GetTseLrfLoadDspInfo(
    AT_TSELRF_PATH_ENUM_UINT32          enPath,
    VOS_BOOL                           *pbLoadDsp,
    DRV_AGENT_TSELRF_SET_REQ_STRU      *pstTseLrf
);

VOS_UINT32 atQryFPllStatusPara(VOS_UINT8 ucClientId);
VOS_UINT32 atQryFPllStatusParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock);

#else
VOS_UINT32  AT_SetMipiOpeRatePara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_SetFAgcgainPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryFAgcgainPara(VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryFtemprptPara(VOS_UINT8 ucIndex);
VOS_UINT32  At_CovertAtModeToBbicCal(
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   enAtMode,
    RAT_MODE_ENUM_UINT16               *penBbicMode
);
VOS_UINT32  At_CovertAtBandWidthToBbicCal(
    AT_BAND_WIDTH_ENUM_UINT16           enAtBandWidth,
    BANDWIDTH_ENUM_UINT16              *penBbicBandWidth
);
VOS_UINT32  At_CovertAtBandWidthToValue(
    AT_BAND_WIDTH_ENUM_UINT16           enAtBandWidth,
    AT_BAND_WIDTH_VALUE_ENUM_UINT32    *penBandWidthValue
);
VOS_UINT32  At_CovertAtScsToBbicCal(
    AT_SUB_CARRIER_SPACING_ENUM_UINT8   enAtScs,
    NR_SCS_TYPE_COMM_ENUM_UINT8        *penBbicScs
);
VOS_UINT32  At_CovertRatModeToBbicCal(
    AT_CMD_RATMODE_ENUM_UINT8           enRatMode,
    RAT_MODE_ENUM_UINT16               *penBbicMode
);
VOS_UINT32  At_CovertChannelTypeToBbicCal(
    AT_TEMP_CHANNEL_TYPE_ENUM_UINT16    enChannelType,
    BBIC_TEMP_CHANNEL_TYPE_ENUM_UINT16 *penBbicChannelType
);
VOS_UINT32  At_CovertAtPaLevelToBbicCal(
    AT_CMD_PALEVEL_ENUM_UINT8           enAtPaLevel,
    BBIC_CAL_PA_MODE_ENUM_UINT8        *penBbicPaLevel
);
VOS_UINT32 AT_GetNrFreqOffset(
    VOS_UINT32                          ulChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset
);
VOS_VOID AT_GetNrFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_NR_BAND_INFO_STRU         *pstBandInfo
);
VOS_VOID AT_GetNrFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_NR_BAND_INFO_STRU         *pstBandInfo
);
VOS_UINT32  AT_GetNrFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_VOID AT_GetLteFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_LTE_BAND_INFO_STRU        *pstBandInfo
);
VOS_VOID AT_GetLteFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_LTE_BAND_INFO_STRU        *pstBandInfo
);
VOS_UINT32  AT_GetLteFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_UINT32 AT_GetWFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_W_BAND_INFO_STRU          *pstBandInfo
);
VOS_UINT32 AT_GetWFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_W_BAND_INFO_STRU          *pstBandInfo
);
VOS_UINT32  AT_GetWFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_UINT32  AT_GetGFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_UINT32  AT_GetCFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_UINT32  AT_GetFreq(
    VOS_UINT32                          ulChannelNo
);
VOS_UINT32  AT_CheckNrFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32  AT_CheckLteFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32  AT_CheckCFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32  AT_CheckWFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32  AT_CheckGFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32  AT_CheckFwaveTypePara(VOS_UINT32 ulPara);
VOS_UINT32 AT_CovertAtFwaveTypeToBbicCal(
    AT_FWAVE_TYPE_ENUM_UINT8            enType,
    MODU_TYPE_ENUM_UINT16              *penType
);
VOS_UINT32 AT_CoverTselPathToPriOrDiv(
    AT_TSELRF_PATH_ENUM_UINT32          enTselPath,
    AT_ANT_TYPE_ENUM_UINT8             *penAntType
);
VOS_UINT32  At_CovertAtModeToLoadDspMode(
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   enAtMode,
    AT_LOAD_DSP_RATMODE_ENUM_UINT8     *penLoadMode
);

VOS_UINT32  At_SetFChanWifiProc(VOS_UINT32 ulBand);

VOS_UINT32 At_GetBaseFreq(RAT_MODE_ENUM_UINT16 enRatMode);

#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif

