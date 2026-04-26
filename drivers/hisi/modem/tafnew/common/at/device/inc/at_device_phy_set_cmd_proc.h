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
#ifndef _AT_DEVICE_PHY_SET_CMD_PROC_H_
#define _AT_DEVICE_PHY_SET_CMD_PROC_H_

#include "vos.h"
#include "at_mt_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
VOS_UINT32 AT_SetLTCommCmdPara(VOS_UINT8 clientId);
VOS_UINT32 atSetFWAVEPara(VOS_UINT8 clientId);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 atSetFCHANPara(VOS_UINT8 clientId);
#endif

VOS_UINT32 AT_SetFwavePara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetFChanPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetTselrfPara(VOS_UINT8 clientId);
VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetFTxonPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetDpdtPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetDpdtTestFlagPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetRxTestModePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRadverPara(VOS_UINT8 indexNum);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetLcaCellRptCfgPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetForceSyncPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetFRxonPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetFPllStatusQryPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetFRXONPara(VOS_UINT8 clientId);
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
extern TAF_UINT32 At_SendRxOnOffToHPA(VOS_UINT32 rxSwitch, VOS_UINT8 indexNum);
VOS_UINT32 At_SendRxOnOffToCHPA(VOS_UINT32 rxSwitch);
extern VOS_UINT32 At_SendRxOnOffToGHPA(VOS_UINT8 indexNum, VOS_UINT32 rxSwitch);
VOS_UINT32 At_SendContinuesWaveOnToHPA(VOS_UINT16 power, VOS_UINT8 indexNum);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 At_SendContinuesWaveOnToCHPA(VOS_UINT8 ctrlType, VOS_UINT16 power);
VOS_UINT32 At_SendTxOnOffToCHPA(VOS_UINT8 switchStatus);
#endif
VOS_UINT8  At_GetDspLoadMode(VOS_UINT32 ratMode);
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_UINT32 AT_SetWifiFwavePara(VOS_VOID);
VOS_UINT32 AT_ProcTSelRfWifiPara(VOS_VOID);
#endif
VOS_BOOL AT_GetTseLrfLoadDspInfo(AT_TSELRF_PathUint32 path, DRV_AGENT_TselrfSetReq *tseLrf);
VOS_UINT32 At_SetCltPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMipiRdPara(VOS_UINT8 indexNum);
extern TAF_UINT32 At_SendTxOnOffToHPA(TAF_UINT8 rxSwitch, TAF_UINT8 indexNum);
extern VOS_UINT32 At_SendTxOnOffToGHPA(VOS_UINT8 indexNum, VOS_UINT8 switchValue);
VOS_UINT32 atSetFTXONPara(VOS_UINT8 clientId);
VOS_UINT32 AT_SetMipiWrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetSSIWrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetPhyMipiWritePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMipiReadPara(VOS_UINT8 indexNum);

#else
VOS_UINT32 AT_CheckNrFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CheckLteFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CheckCFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CheckWFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CheckGFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CheckFwaveTypePara(VOS_UINT32 para);
VOS_UINT32 AT_CovertAtFwaveTypeToBbicCal(AT_FWAVE_TypeUint8 typeIn, MODU_TYPE_ENUM_UINT16 *typeOut);

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_UINT32 At_SetFChanWifiProc(VOS_UINT32 band);
VOS_UINT32 AT_ProcTSelRfWifiPara(VOS_VOID);
#endif
VOS_UINT32 At_CovertAtModeToBbicCal(AT_DEVICE_CmdRatModeUint8 atMode, RAT_MODE_ENUM_UINT16 *bbicMode);
VOS_UINT32 AT_GetNrFreqOffset(VOS_UINT32 channelNo, AT_NR_FreqOffsetTable *nrFreqOffset);
VOS_VOID   AT_GetNrFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_NR_FreqOffsetTable *nrFreqOffset,
                                       AT_DSP_BandFreq *dspBandFreq, const AT_NR_BandInfo *bandInfo);
VOS_VOID   AT_GetNrFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_NR_FreqOffsetTable *nrFreqOffset,
                                       AT_DSP_BandFreq *dspBandFreq, const AT_NR_BandInfo *bandInfo);
VOS_UINT32 AT_GetNrFreq(VOS_UINT32 channelNo);
VOS_VOID   AT_GetLteFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                        const AT_LTE_BandInfo *bandInfo);
VOS_VOID   AT_GetLteFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                        const AT_LTE_BandInfo *bandInfo);
VOS_UINT32 AT_GetLteFreq(VOS_UINT32 channelNo);
VOS_UINT32 AT_GetWFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_W_BandInfo *bandInfo);
VOS_UINT32 AT_GetWFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_W_BandInfo *bandInfo);
VOS_UINT32 AT_GetWFreq(VOS_UINT32 channelNo);
VOS_UINT32 AT_GetGFreq(VOS_UINT32 channelNo);
VOS_UINT32 AT_GetCFreq(VOS_UINT32 channelNo);
VOS_UINT32 AT_GetFreq(VOS_UINT32 channelNo);
VOS_UINT32 At_CovertAtBandWidthToBbicCal(AT_BAND_WidthUint16 atBandWidth, BANDWIDTH_ENUM_UINT16 *bbicBandWidth);
VOS_UINT32 At_CovertAtBandWidthToValue(AT_BAND_WidthUint16 atBandWidth, AT_BAND_WidthValueUint32 *bandWidthValue);
VOS_UINT32 At_CovertAtScsToBbicCal(AT_SubCarrierSpacingUint8 atScs, NR_SCS_TYPE_COMM_ENUM_UINT8 *bbicScs);
VOS_UINT32 At_LoadPhy(VOS_VOID);
VOS_UINT32 AT_CoverTselPathToPriOrDiv(AT_TSELRF_PathUint32 tselPath, AT_AntTypeUint8 *antType);
VOS_UINT32 At_SetRfIcMemTest(VOS_UINT8 indexNum);
VOS_UINT32 AT_SndBbicCalMipiReadReq(VOS_UINT32 mipiPortSel, VOS_UINT32 secondaryId, VOS_UINT32 regAddr,
                                    VOS_UINT32 byteCnt, VOS_UINT32 readSpeed);
VOS_UINT32 AT_SndBbicCalMipiWriteReq(VOS_UINT32 mipiPortSel, VOS_UINT32 secondaryId, VOS_UINT32 regAddr,
                                     VOS_UINT32 byteCnt, VOS_UINT32 value);
VOS_UINT32 At_SetFSerdesRt(VOS_UINT8 indexNum);
VOS_UINT32 At_SetSerdesAsyncTest(VOS_UINT8 indexNum);
VOS_UINT32            AT_SetMipiOpeRatePara(VOS_UINT8 indexNum);
VOS_UINT32            AT_SetFRbInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SndDspIdleSltTestReq(VOS_VOID);
VOS_UINT32 At_SndDcxoReq(VOS_VOID);

VOS_UINT32 At_SndUeCbtRfIcMemTestReq(VOS_VOID);
VOS_UINT32 At_SndRxOnOffReq(VOS_VOID);
VOS_UINT32 At_SndDspIdleSerdesRtReq(VOS_VOID);
VOS_UINT32 At_SndDspIdleSerdesTestAsyncReq(VOS_VOID);

#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    VOS_UINT32 AT_SetSltTestPara(VOS_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
