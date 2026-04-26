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

#ifndef _ATDATATAFQRYCMDPROC_H_
#define _ATDATATAFQRYCMDPROC_H_

#include "taf_type_def.h"
#include "vos.h"
#include "rnic_dev_def.h"
#include "at_ctx_packet.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

TAF_UINT32 AT_QryCgautoPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_QryCgtftPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_QryCgactPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_QryCgdcontPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_QryCgeqreqPara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryCgeqminPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_QryCgdscontPara(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgdcont(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgdscont(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgtft(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgact(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgcmod(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgpaddr(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgautoPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCgeqreqPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCgdataPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCgansPara(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgansExtPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SndQryFastDorm(VOS_UINT16 clientId, VOS_UINT8 opId);
VOS_UINT32 AT_QryFastDormPara(TAF_UINT8 indexNum);

TAF_UINT32 At_QryNdisdupPara(TAF_UINT8 indexNum);
TAF_UINT32              AT_QryAuthdataPara(TAF_UINT8 indexNum);
VOS_UINT32              AT_QryIpv6CapPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_QryCgdnsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryGlastErrPara(TAF_UINT8 indexNum);
extern VOS_UINT32 AT_SetChdataPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_QryChdataPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_TestChdataPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCgdnsPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCgcontrdp(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgscontrdp(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestCgtftrdp(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestAuhtdata(VOS_UINT8 indexNum);

extern VOS_UINT32 At_TestNdisDup(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCgpiafPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryDhcpPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestDhcpPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDhcpv6Para(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestDhcpv6Para(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryApRaInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestApRaInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestIPv6TempAddrPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryApConnStPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestApConnStPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryApDsFlowRptCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDsFlowNvWriteCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCgerepPara(VOS_UINT8 indexNum);
VOS_UINT32 At_TestUsbTetherInfo(VOS_UINT8 indexNum);
VOS_UINT32 At_QryDconnStatPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestDconnStatPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryNdisStatPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestIpv6capPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryPsCallRptCmdPara(VOS_UINT8 indexNum);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_TestCustProfilePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCustProfileAttachSwitch(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_QrySinglePdnSwitch(VOS_UINT8 indexNum);
#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_SndQryVoicePrefer(VOS_UINT16 clientId, VOS_UINT8 opId);
VOS_UINT32 AT_QryVoicePreferPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_QryCPsErrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDislogPara(VOS_UINT8 indexNum);
#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
VOS_UINT32 AT_QryShellPara(VOS_UINT8 indexNum);
#endif
TAF_UINT32 At_QryApDialModePara(TAF_UINT8 indexNum);
VOS_UINT32 AT_QryApLanAddrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestApLanAddrPara(VOS_UINT8 indexNum);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryC5gqosPara(VOS_UINT8 indexNum);
VOS_UINT32 At_TestC5gqos(VOS_UINT8 indexNum);

VOS_UINT32 At_TestC5gQosRdp(VOS_UINT8 indexNum);
VOS_UINT32 At_TestCgcontexPara(VOS_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
