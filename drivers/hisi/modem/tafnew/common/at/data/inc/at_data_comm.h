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

#ifndef _ATDATACOMM_H_
#define _ATDATACOMM_H_

#include "vos.h"
#include "rnic_dev_def.h"
#include "at_ctx_packet.h"
#include "ps_iface_global_def.h"
#include "at_data_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_MBB_CUST == FEATURE_ON)
#define TAF_MAX_LTE_APN_IMSI_PREFIX_SUPPORT 10
#define TAF_MIN_LTE_APN_IMSI_PREFIX_SUPPORT 5

#define LTE_PROFILE_PDP_TYPE 2
#define LTE_PROFILE_IMSIPREFIX 3
#endif

/*
 * 结构说明: AT^CHDATA与RNIC网卡映射关系的结构
 */
typedef struct {
    AT_CH_DataChannelUint32 chdataValue;
    RNIC_DEV_ID_ENUM_UINT8  rnicRmNetId;
    PS_IFACE_IdUint8        ifaceId;
    VOS_UINT8               reserved[2];

} AT_ChdataRnicRmnetId;

extern const AT_ChdataRnicRmnetId g_atChdataRnicRmNetIdTab[];
#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR() (g_atChdataRnicRmNetIdTab)
#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE() (AT_ARRAY_SIZE(g_atChdataRnicRmNetIdTab))

VOS_UINT8 AT_GetUserIndexByClientIndex(VOS_UINT8 clientIndex);
#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID AT_ReportImsEmcStatResult(VOS_UINT8 indexNum, AT_PDP_StatusUint32 status);
#endif

VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(RNIC_DEV_ID_ENUM_UINT8   rnicRmNetId,
                                               AT_CH_DataChannelUint32 *dataChannelId);
VOS_UINT32 AT_PS_GetChDataValueFromIfaceId(PS_IFACE_IdUint8 ifaceId, AT_CH_DataChannelUint32 *dataChannelId);
CONST AT_ChdataRnicRmnetId* AT_PS_GetChDataCfgByChannelId(AT_CH_DataChannelUint32 dataChannelId);
VOS_UINT32 At_TestCgeqnegPara(VOS_UINT8 indexNum);
/*
 * 功能描述: 删除承载IP与RABID的映射
 */
VOS_VOID AT_PS_DeleteIpAddrMap(VOS_UINT16 clientId, TAF_PS_CallPdpDeactivateCnf *event);
VOS_UINT32 AT_ChkTafPsEvtGetPrimPdpContextInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetSecPdpContextInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetTftInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetUmtsQosInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetUmtsQosMinInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamicUmtsQosInfoCnfMsgLen(const MSG_Header *msg);
/*
 * 功能描述: Cgeqneg测试命令返回
 */
VOS_UINT32 AT_ChkTafPsEvtGetPdpStateCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetPdpIpAddrInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamicPrimPdpContextInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamicSecPdpContextInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamicTftInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetEpsQosInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetDynamicEpsQosInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetPdpDnsInfoCnfMsgLen(const MSG_Header *msg);
VOS_UINT32 AT_ChkTafPsEvtGetAuthDataInfoCnfMsgLen(const MSG_Header *msg);
/*
 * 功能描述: 将PS域呼叫错误码转换成3GPP协议定义的(E)SM Cause, 非3GPP协议定义
 *           的错误码统一转换成0(Unknown error/Unspecified error)
 */
VOS_UINT32 AT_Get3gppSmCauseByPsCause(TAF_PS_CauseUint32 cause);
VOS_VOID AT_PS_SetPsCallErrCause(VOS_UINT16 clientId, TAF_PS_CauseUint32 psErrCause);
TAF_PS_CauseUint32 AT_PS_GetPsCallErrCause(VOS_UINT16 clientId);
#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
extern VOS_UINT32 AT_PS_CheckIfaceMatchWithClient(VOS_UINT16 clientId, VOS_UINT8 ifaceId, VOS_UINT32 usedFlg);
#endif
AT_PS_DataChanlCfg* AT_PS_GetDataChanlCfg(VOS_UINT16 clientId, VOS_UINT8 cid);
TAF_IFACE_UserTypeUint8 AT_PS_GetUserType(const VOS_UINT8 indexNum);


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
