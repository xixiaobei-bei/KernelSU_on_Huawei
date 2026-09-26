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
#ifndef _AT_DEVICE_COMM_H_
#define _AT_DEVICE_COMM_H_

#include "vos.h"
#include "at_mt_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_MNC_MIN_LEN 2
#define AT_MNC_MAX_LEN 3

#define AT_OUTPUT_INDEX_0 0
#define AT_OUTPUT_INDEX_1 1
#define AT_OUTPUT_INDEX_2 2
#define AT_OUTPUT_INDEX_3 3
#define AT_INPUT_INDEX_0 0
#define AT_INPUT_INDEX_1 1
#define AT_INPUT_INDEX_2 2
#define AT_INPUT_INDEX_3 3

#define AT_BORD_MIN_WIDTH 1
#define AT_BORD_MAX_WIDTH 2

/* 输入数据不足3的倍数时 输出字符后面填充'='号 */
#define AT_BASE64_PADDING '='

#define AT_RSFR_RSFW_NAME_LEN 7    /* 目前NAME只支持(VERSION/SIMLOCK/vSIM) */
#define AT_RSFR_RSFW_SUBNAME_LEN 7 /* SUBNAME长度不超过7 */

/* (TAF_PH_SIMLOCK_PLMN_STR_LEN*2+1)*TAF_MAX_SIM_LOCK_RANGE_NUM 等于 340 */
/* SIMLOCK的字符串最大340个字符，编码后也不会超过1024 */
#define AT_RSFR_RSFW_MAX_LEN 1024

#define AT_TSELRF_PATH_TOTAL_MT 16

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
#if (FEATURE_LTEV == FEATURE_ON)
#define AT_COMPONENTTYPE_RAT_LTEV 0x7
#endif
#endif


#if (FEATURE_UE_MODE_NR == FEATURE_ON)
extern const AT_G_BandInfo g_atGBandInfoTable[];
extern const VOS_UINT8     g_atGBandInfoTableLen;

#define AT_GET_GSM_BAND_INFO_TAB_LEN() (g_atGBandInfoTableLen)
#endif

VOS_VOID AtBase64Encode(const VOS_UINT8 *pdata, const VOS_UINT32 dataSize, VOS_UINT8 *outPcode);
VOS_UINT32 At_SimlockPlmnNumToAscii(const VOS_UINT8 *plmnRange, VOS_UINT8 plmnRangeLen, VOS_UINT8 *asciiStr);
VOS_UINT32 Calc_CRC32(VOS_UINT8 *packet, VOS_UINT32 length);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 AT_SetGlobalFchan(VOS_UINT8 ratMode);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 At_IsLteRatMode(VOS_UINT32 paraValue);
#endif
#else
VOS_UINT32 At_CovertRatModeToBbicCal(AT_CmdRatmodeUint8 ratMode, RAT_MODE_ENUM_UINT16 *bbicMode);
VOS_UINT32 AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_ENUM_UINT16 operType, VOS_UINT32 value, VOS_UINT32 workType);
VOS_VOID              AT_SetUartTestState(AT_UartTestStateUint8 uartTestState);
AT_UartTestStateUint8 AT_GetUartTestState(VOS_VOID);
VOS_VOID              AT_SetI2sTestState(AT_I2sTestStateUint8 i2sTestState);
AT_I2sTestStateUint8  AT_GetI2sTestState(VOS_VOID);
VOS_UINT16 AT_GetGsmUlPathByBandNo(VOS_UINT16 bandNo);
VOS_UINT32 At_SndGsmTxOnOffReq_ModulatedWave(VOS_UINT16 powerDetFlg);
VOS_UINT32 At_SndTxOnOffReq(VOS_UINT16 powerDetFlg);
VOS_UINT32 AT_SndBbicPllStatusReq(VOS_VOID);
#endif

VOS_UINT32 atSetTmodePara(VOS_UINT8 clientId, VOS_UINT32 tmode);
VOS_UINT32 AT_TestHsicCmdPara(VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 At_ProcLteTxCltInfoReport(struct MsgCB *msgBlock);
#endif


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
