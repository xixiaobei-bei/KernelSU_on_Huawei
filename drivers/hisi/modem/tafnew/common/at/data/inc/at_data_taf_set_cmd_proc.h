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

#ifndef _ATDATATAFSETCMDPROC_H_
#define _ATDATATAFSETCMDPROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "at_data_proc.h"
#include "at_ctx_packet.h"
#include "taf_iface_api.h"
#include "at_data_comm.h"
#include "at_data_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

TAF_UINT32 AT_SetCgeqreqPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgeqminPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgautoPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgtftPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetCgdcontPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCgdscontPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_SetCgpaddrPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgeqnegPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgcmodPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgactPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetCgansPara(TAF_UINT8 indexNum);
VOS_UINT32 atSetCgcontrdpPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetCgscontrdpPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetCgtftrdpPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_SetCgdnsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCgdataPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetLFastDormPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SndSetFastDorm(VOS_UINT16 clientId, VOS_UINT8 opId, AT_RABM_FastdormPara *fastDormPara);
VOS_UINT32 AT_SetFastDormPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNdisdupPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetAuthdataPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetDsFlowClrPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetDsFlowQryPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetDsFlowRptPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetGlastErrPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetDnsQueryPara(VOS_UINT8 indexNum);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
#if (FEATURE_UE_MODE_G == FEATURE_ON)
TAF_UINT32 At_SetTrigPara(TAF_UINT8 indexNum);
#endif
#endif
VOS_UINT32 At_SetApConnStPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDhcpv6Para(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetIPv6TempAddrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApRaInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetDhcpPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApDsFlowRptCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDsFlowNvWriteCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetUsbTetherInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRmnetCfg(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCgmtuPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_SetRoamPdpTypePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCgerepPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCgpiafPara(VOS_UINT8 indexNum);

PPP_AuthTypeUint8 AT_CtrlGetPDPAuthType(VOS_UINT32 value, VOS_UINT16 totalLen);
TAF_PDP_AuthTypeUint8 AT_ClGetPdpAuthType(VOS_UINT32 value, VOS_UINT16 totalLen);
VOS_UINT32 AT_SetOpwordParaForApModem(VOS_UINT8 indexNum);
VOS_UINT32 AT_CheckIpv6Capability(VOS_UINT8 pdpType);

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_SetCustProfilePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCustProfileAttachSwitch(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNdisStatQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDsambrPara(VOS_UINT8 indexNum);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetC5gqosPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetC5gqosRdpPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCsUePolicy(VOS_UINT8 clientId);
VOS_UINT32 AT_SetCgcontexPara(VOS_UINT8 indexNum);
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetEmcStatus(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetPsCallRptCmdPara(VOS_UINT8 indexNum);
/*
 * 功能描述: 构造IP和UDP头部信息(用于构造测试使用的UDP包)
 */
VOS_UINT32 AT_BuildUdpHdr(AT_UdpPacketFormat *udpPkt, VOS_UINT16 len, VOS_UINT32 srcAddr, VOS_UINT32 dstAddr,
                          VOS_UINT16 srcPort, VOS_UINT16 dstPort);
VOS_UINT32 AT_SetApLanAddrPara(VOS_UINT8 indexNum);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
VOS_UINT32 AT_PS_GetIpAddrByIfaceId(const VOS_UINT16 clientId, const VOS_UINT8 ifaceId);
#else
/*
 * 功能描述: 根据RABID获取承载IP地址
 */
VOS_UINT32 AT_PS_GetIpAddrByRabId(VOS_UINT16 clientId, VOS_UINT8 rabId);
#endif

VOS_UINT32 AT_PS_ProcIfaceCmd(const VOS_UINT8 indexNum, const TAF_IFACE_UserTypeUint8 userType);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_PS_CheckDialRatType(VOS_UINT8 indexNum, VOS_UINT8 bitRatType);
#endif
VOS_VOID AT_GetMbbUsrDialPdpType(TAF_IFACE_DialParam *usrDialParam);

VOS_UINT32 AT_OpenDiagPort(VOS_VOID);
VOS_UINT32 AT_CloseDiagPort(VOS_VOID);
VOS_UINT32 AT_SetDislogPara(VOS_UINT8 indexNum);

VOS_UINT32 At_SndReleaseRrcReq(VOS_UINT16 clientId, VOS_UINT8 opId);
VOS_UINT32 At_SetReleaseRrc(VOS_UINT8 indexNum);

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_SndSetVoicePrefer(VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 voicePreferApStatus);
VOS_UINT32 AT_SetVoicePreferPara(VOS_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
