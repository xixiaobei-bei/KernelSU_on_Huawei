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
#ifndef _AT_CUSTOM_TAF_QRY_CMD_PROC_H_
#define _AT_CUSTOM_TAF_QRY_CMD_PROC_H_

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
VOS_UINT32 AT_QryApRptPortSelectPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCposrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryXcposrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestXcposrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryXcposrRptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestApRptPortSelectPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMipiClkValue(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryBestFreqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryLogEnable(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCtzuPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_QryCrrconnPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDataSwitchStatus(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDataRoamSwitchStatus(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryLteAttachInfoPara(VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryLendcPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryLadnInfoPara(VOS_UINT8 idx);
#endif
VOS_UINT32 At_QryLcStartTimePara(VOS_UINT8 indexNum);
VOS_UINT32 atQryAuthorityVer(VOS_UINT8 clientId);
VOS_UINT32 atQryAuthorityID(VOS_UINT8 clientId);
VOS_UINT32 AT_QryApRptSrvUrlPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryApXmlInfoTypePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryApXmlRptFlagPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRrcStatPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryHfreqInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRATCombinePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryTrustNumCtrlPara(VOS_UINT8 index);
VOS_UINT32 AT_TestTrustNumCtrlPara(VOS_UINT8 index);
VOS_UINT32 AT_QryLogNvePara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCabtsrPara(VOS_UINT8 indexNum);
VOS_UINT32 At_TestCabtsrPara(VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
VOS_UINT32 At_TestMBMSCMDPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMBMSCmdPara(VOS_UINT8 indexNum);
#endif
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_QryRsrpCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRscpCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEcioCfgPara(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_QryAuthverPara(VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
