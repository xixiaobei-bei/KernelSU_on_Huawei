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

#ifndef __ATINIT_H__
#define __ATINIT_H__

#include "vos.h"
#include "at_ctx.h"
#include "at_input_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* 多时隙等级最大宏定义 */
#define AT_GAS_GRR_MULTISLOT_CLASS_MAX 12

#define AT_MSG_SRV_RCV_SM_MEM_STORE_OFFSET 1 /* 短信接收存储介质在NV项中存储的偏移 */
/* 短信接收存储介质在NV项中存储标志在NV项中存储的偏移，对应字节取值含义0:不使能 1:使能 */
#define AT_MSG_SRV_SM_MEM_ENABLE_OFFSET 6


enum AT_GAS_HighMultislotClass {
    AT_GAS_HIGH_MULTISLOT_CLASS_0,
    AT_GAS_HIGH_MULTISLOT_CLASS_1,
    AT_GAS_HIGH_MULTISLOT_CLASS_2,
    AT_GAS_HIGH_MULTISLOT_CLASS_3,

    AT_GAS_HIGH_MULTISLOT_CLASS_BUTT
};
typedef VOS_UINT8 AT_GAS_HighMultislotClassUint8;

/*
 * 协议表格: 无
 * 结构说明: 存储在NV中的UE能力信息
 */
typedef struct {
    VOS_UINT8 reserved1[11];  /* 留作以后扩展 */
    VOS_UINT8 asRelIndicator; /* Access Stratum Release Indicator             */

    VOS_UINT8 hSDSCHSupport;     /* 是否支持enHSDSCHSupport的标志                */
    VOS_UINT8 hSDSCHPhyCategory; /* 支持HS-DSCH物理层的类型标志                  */

    VOS_UINT8 macEhsSupport;
    VOS_UINT8 hSDSCHPhyCategoryExt;
    VOS_UINT8 multiCellSupport;      /* 是否支持 Multi cell support,如果支持MultiCell,Ex2存在 */
    VOS_UINT8 hSDSCHPhyCategoryExt2; /* HS-DSCH physical layer category extension 2 */

    VOS_UINT8 reserved2[17]; /* 留作以后扩展 */

    VOS_UINT8 hSDSCHPhyCategoryExt3; /* HS-DSCH physical layer category extension 3 */
    VOS_UINT8 dcMimoSupport;         /* 是否支持DC+MIMO */

    VOS_UINT8 reserved3[3]; /* 留作以后扩展 */

} AT_NvUeCapability;

extern VOS_VOID AT_ReadPlatformNV(VOS_VOID);
extern VOS_VOID AT_ReadClientConfigNV(VOS_VOID);

extern MN_MSG_MemStoreUint8 AT_GetCpmsMtMem(ModemIdUint16 modemId);

extern VOS_VOID AT_ReadSmsMeStorageInfoNV(VOS_VOID);
extern VOS_VOID AT_ReadSmsClass0TailorNV(VOS_VOID);
extern VOS_VOID AT_ReadSmsNV(VOS_VOID);

extern VOS_VOID AT_ReadRoamCapaNV(VOS_VOID);
extern VOS_VOID AT_ReadPrivacyFilterCfgNv(VOS_VOID);
extern VOS_VOID AT_ReadSystemAppConfigNV(VOS_VOID);

extern VOS_VOID AT_ReadAtDislogPwdNV(VOS_VOID);
extern VOS_VOID AT_ReadAtRightPasswordNV(VOS_VOID);
extern VOS_VOID AT_ReadAtDissdPwdNV(VOS_VOID);
extern VOS_VOID AT_ReadNotSupportRetValueNV(VOS_VOID);
extern VOS_VOID AT_ReadE5NV(VOS_VOID);

extern VOS_UINT32 AT_IsAbortCmdCharValid(VOS_UINT8 *abortCmdChar, VOS_UINT32 len);
extern VOS_VOID   AT_ReadAbortCmdParaNV(VOS_VOID);

extern VOS_VOID AT_ReadSysNV(VOS_VOID);

extern VOS_VOID AT_ReadCellSignReportCfgNV(VOS_VOID);

extern VOS_VOID AT_ReadPppDialErrCodeNV(VOS_VOID);
extern VOS_VOID AT_ReadReportRegActFlgNV(VOS_VOID);
extern VOS_VOID AT_ReadCregAndCgregCiFourByteRptNV(VOS_VOID);
extern VOS_VOID AT_ReadIpv6CapabilityNV(VOS_VOID);
extern VOS_VOID AT_ReadIpv6AddrTestModeCfgNV(VOS_VOID);

extern VOS_VOID AT_ReadDialConnectDisplayRateNV(VOS_VOID);

extern VOS_VOID AT_ReadPsNV(VOS_VOID);
extern VOS_VOID AT_ReadCsNV(VOS_VOID);

#if (FEATURE_LTE == FEATURE_ON)
extern VOS_VOID AT_ReadLTENV(VOS_VOID);
#endif

extern VOS_UINT32 AT_ValidateWasCategory(AT_NvUeCapability *ueCapability);
extern VOS_UINT32 AT_ValidateWasCategoryExt(AT_NvUeCapability *ueCapability);
extern VOS_UINT32 AT_ValidateWasCategoryExt2(AT_NvUeCapability *ueCapability);
extern VOS_UINT32 AT_ValidateWasCategoryExt3(AT_NvUeCapability *ueCapability);
extern VOS_UINT8  AT_GetWasDefaultCategory(AT_NvUeCapability *ueCapability);
extern VOS_UINT8  AT_CalcWasCategory(AT_NvUeCapability *ueCapability);

extern VOS_VOID AT_ReadWasCapabilityNV(VOS_VOID);
extern VOS_VOID AT_ReadGasCapabilityNV(VOS_VOID);

extern VOS_UINT32 AT_ReadPhyNV(VOS_VOID);
extern VOS_VOID   AT_ReadAgpsNv(VOS_VOID);
VOS_VOID AT_ReadAtCmdTrustListCfgNv(VOS_VOID);
VOS_VOID AT_ReadMbbCustomizeNv(VOS_VOID);
extern VOS_VOID   AT_ReadNV(VOS_VOID);

extern VOS_VOID AT_InitDeviceCmd(VOS_VOID);
extern VOS_VOID AT_InitPort(VOS_VOID);
extern VOS_VOID AT_InitStk(VOS_VOID);
extern VOS_VOID AT_InitPara(VOS_VOID);

#if (defined(CONFIG_BALONG_SPE))
VOS_VOID AT_SpeInit(VOS_VOID);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_VOID AT_InitDevCmdCtrl(VOS_VOID);
#else
VOS_VOID AT_InitMTInfo(VOS_VOID);
#endif
extern VOS_UINT32 At_PidInit(enum VOS_InitPhaseDefine phase);

VOS_VOID AT_ReadSsNV(VOS_VOID);

VOS_VOID AT_ReadUartCfgNV(VOS_VOID);

VOS_VOID AT_ReadNvloadCarrierNV(VOS_VOID);

VOS_VOID AT_ReadPortBuffCfgNV(VOS_VOID);

VOS_VOID AT_ReadMtCustomizeInfo(VOS_VOID);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID AT_ReadCdmaModemSwitchNotResetCfgNv(VOS_VOID);
#endif
VOS_VOID AT_ReadApnCustomFormatCfgNV(VOS_VOID);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 AT_GetMuxGpsPortEnableFlag(VOS_VOID);
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
VOS_VOID AT_ReadStkReportAppConfig(VOS_VOID);
#endif
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_init.h */
