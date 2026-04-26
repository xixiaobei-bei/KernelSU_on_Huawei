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
#ifndef __ATQUERYPARACMD_H__
#define __ATQUERYPARACMD_H__

#include "at_ctx.h"
#include "vos.h"
#include "mn_msg_api.h"
#include "at_mta_interface.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_GetPhynumMac(VOS_UINT8 aucMac[], VOS_UINT32 macLen);
TAF_UINT32 At_QryCrcPara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryCssnPara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryCtzrPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_QryTimeQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QrySysCfgExPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCvoicePara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryDdsetexPara(VOS_UINT8 indexNum);
VOS_VOID At_CovertMsInternalRxDivParaToUserSet(VOS_UINT16 curBandSwitch, VOS_UINT32 *userDivBandsLow,
                                                              VOS_UINT32 *userDivBandsHigh);
#if (FEATURE_LTE == FEATURE_ON)
TAF_VOID AT_CalculateLTESignalValue(VOS_INT16 *psRssi, VOS_UINT8 *level, VOS_INT16 *rsrp, VOS_INT16 *rsrq);
#endif

VOS_UINT32 At_ReadDpaCatFromNV(VOS_UINT8 *dpaRate);
VOS_UINT32 AT_ReadRrcVerFromNV(VOS_UINT8 *rrcVer);
VOS_UINT32 AT_QryHspaSpt(VOS_UINT8 indexNum);
VOS_UINT32 AT_OutputCsdfltDefault(VOS_UINT8 indexNum, VOS_BOOL bSimlockEnableFlg);
VOS_UINT32 At_QryXlemaPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryPacspPara(VOS_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryRoamImsServicePara(VOS_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_query_para_cmd.h */
