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
#ifndef __ATPHONEIMSSETCMDPROC_H__
#define __ATPHONEIMSSETCMDPROC_H__

#include "taf_type_def.h"
#include "AtParse.h"
#include "at_mta_interface.h"
#include "taf_app_mma.h"
#include "at_imsa_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_Ipv6PcscfDataToAddr(VOS_UINT8 *str, VOS_UINT8 *ipAddr, VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum);
VOS_UINT32 AT_PortAtoI(VOS_CHAR *pcString, VOS_UINT32 *value);


#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetBatteryInfoPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCiregPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCirepPara(TAF_UINT8 indexNum);

VOS_UINT32 AT_SetCallEncryptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_ParseIpv6PcscfData(VOS_UINT32 *addrExistFlg, VOS_UINT8 *ipAddr, VOS_UINT32 *portExistFlg,
                                 VOS_UINT32 *portNum, VOS_UINT32 indexNum);

VOS_UINT32 AT_FillIpv6PcscfData(AT_IMSA_PcscfSetReq *pcscf);

VOS_UINT32 AT_ParseIpv4PcscfData(VOS_UINT32 *addrExistFlg, VOS_UINT8 *ipAddr, VOS_UINT32 ipBufLen,
                                 VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum, VOS_UINT32 indexNum);

VOS_UINT32 AT_FillIpv4PcscfData(AT_IMSA_PcscfSetReq *pcscf);

VOS_UINT32 AT_FillDataToPcscf(AT_IMSA_PcscfSetReq *pcscf);

VOS_UINT32 AT_SetImsPcscfPara(VOS_UINT8 indexNum);
VOS_UINT32 At_FillImsPayloadTypePara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum);

VOS_UINT32 At_FillImsAmrWbModePara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum);

VOS_UINT32 At_FillImsRtpPortPara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum);

VOS_UINT32 At_FillImsaNumericPara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum);

VOS_UINT32 At_FillImsaStrPara(VOS_CHAR *str, VOS_UINT32 *strValidFlg, VOS_UINT32 maxLen, VOS_UINT32 indexNum);

VOS_UINT32 AT_SetDmDynPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_FillImsTimerReqBitAndPara(VOS_UINT32 *value, VOS_UINT32 *bitOpValue, VOS_UINT32 indexNum);
VOS_UINT32 AT_FillImsTimerReqData(AT_IMSA_ImstimerSetReq *imsTimer);
VOS_UINT32 AT_SetImsTimerPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetImsSmsPsiPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRoamImsServicePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetVolteRegPara(TAF_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
