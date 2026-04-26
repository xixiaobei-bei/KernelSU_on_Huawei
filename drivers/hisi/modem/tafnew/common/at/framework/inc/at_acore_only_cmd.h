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

#ifndef _AT_ACORE_ONLY_CMD_H_
#define _AT_ACORE_ONLY_CMD_H_

#include "vos.h"
#include "taf_type_def.h"
#include "at_mta_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)

#define GPIO_GROUP_BASE 100

#define AT_SECURE_STATE_NOT_SET 0 /* Secure State状态未设置 */
#define AT_SECURE_STATE_SECURE 1  /* 当前Secure State为Secure */
#define AT_SECURE_STATE_RMA 2     /* 当前Secure State为RMA */

#define AT_SD_MAX_OPR_NUM 4             /* 命令at^sd=   各参数的最大值 */
#define AT_SD_MAX_ADDRESS_NUM 429496728
#define AT_SD_MAX_DATA_NUM 3
#define AT_SD_PARAM_MAX_NUM 3  /* 命令at^sd= 参数的最大个数 */
#define AT_SD_READ_PARA_VALID_NUM 2  // 对SD卡进行读操作时所需的参数个数为2
#define AT_SD_WRITE_PARA_VALID_NUM 3  // 对SD卡进行写操作时所需的参数个数为3
#define AT_SD_PARA_MAX_NUM 3
#define AT_SD_DATA 2

#define AT_EFUSE_OK 0     /* efuse返回成功 */
#define AT_EFUSE_REPEAT 1 /* efuse返回重复设置 */

#define AT_DRV_SOCID_LEN 32 /* 256Bits的SOCID码流长度 */

#define AT_WIINFO_PARA_MAX_NUM 2
#define AT_WIRX_PARA_MAX_NUM 3
#define AT_WIRX_SOURCE_MAC 1

#define AT_WIDATARATE_PARA_NUM 1
#define AT_WIDATARATE_RATE 0
#define AT_WIFI_N_RATE_MAX_VALUE 7

#define AT_WILOG_PARA_MAX_NUM 2
#define AT_SSID_PARA_MAX_NUM 2
#define AT_WIKEY_PARA_MAX_NUM 2

#define AT_WIFI_WEP_KEY1 0
#define AT_WIFI_WEP_KEY2 1
#define AT_WIFI_WEP_KEY3 2
#define AT_WIFI_WEP_KEY4 3

VOS_UINT32 At_CmdTestProcOK(VOS_UINT8 indexNum);
VOS_UINT32 At_CmdTestProcERROR(VOS_UINT8 indexNum);
VOS_UINT32 At_RegisterAcoreOnlyCmdTable(VOS_VOID);
VOS_UINT32 At_SetGpioPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestGpioPara(VOS_UINT8 indexNum);
VOS_INT32 AT_SetSecDbgStateVal(VOS_VOID);
VOS_UINT32 atQryDLoadInfo(VOS_UINT8 clientId);
VOS_UINT32 atQryDLoadVer(VOS_UINT8 clientId);
VOS_UINT32 AT_SetSecureStatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QrySecureStatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetSecDbgStatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QrySecDbgStatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QrySocidPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCbcPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryChrgInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetReset(VOS_UINT8 clientId);
VOS_UINT32 atSetNVRestore(VOS_UINT8 clientId);
VOS_UINT32 At_SDOprtRead(VOS_UINT8 indexNum);
VOS_UINT32 At_SDOprtWrtEraseFmt(TAF_UINT8 indexNum);
VOS_UINT32 AT_CheckSDParam(VOS_VOID);
VOS_UINT32 At_SetSD(VOS_UINT8 indexNum);
VOS_UINT32 At_QrySD(VOS_UINT8 indexNum);

#if (FEATURE_AGPS == FEATURE_ON)
VOS_INT AT_SetCgpsClock(VOS_BOOL bEnableFlg, MTA_AT_ModemIdUint8 modemId, MTA_AT_RatModeUint8 sysMode, VOS_UINT8 indexNum);

TAF_NV_GpsChipTypeUint8 At_GetGpsTypeFromNv(VOS_UINT32 modemId);

VOS_UINT32 AT_SetCgpsClockForBroadcom(VOS_UINT8 indexNum, ModemIdUint16 modemId);

VOS_UINT32 AT_SetCgpsClockForHisi1102(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetCgpsClockPara(VOS_UINT8 indexNum);

#ifndef MBB_COMMON
VOS_UINT32 At_SetSecuBootPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QrySecuBootPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QrySecuBootFeaturePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetChrgEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryChrgEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestChrgEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWifiPaRangePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWifiPaRangePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestWifiPaRangePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWifiInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiPacketPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiPacketPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiTxPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiRxPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiTxPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiRxPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiModePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiModePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiFreqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiFreqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiPowerPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiPowerPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiBandPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiRatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiLogPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiSsidPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWiFiKeyPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiBandPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiRatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiSsidPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiLogPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryWiFiKeyPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestSsidPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestWikeyPara(VOS_UINT8 indexNum);
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_QryWakelock(VOS_UINT8 indexNum);
VOS_UINT32 At_SetWakelock(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCcpuNidDisablePara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryAcpuNidDisablePara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCcpuNidDisablePara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetAcpuNidDisablePara(VOS_UINT8 indexNum);
VOS_UINT32 atSetGodLoad(VOS_UINT8 clientId);
VOS_UINT32 AT_SetKcePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApbatlvlPara(VOS_UINT8 indexNum);

#ifndef MBB_COMMON
VOS_UINT32 At_GetKeyInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetKeyPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetAuthKeyRdPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryAuthKeyRdPara(VOS_UINT8 indexNum);
#endif
#endif

#ifdef MBB_SLT
VOS_UINT32 AT_SetBsnExPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryBsnExPara(VOS_UINT8 indexNum);
#endif

#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif

