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
#ifndef __ATDATAEVENTREPORT_H__
#define __ATDATAEVENTREPORT_H__

#include "vos.h"
#include "si_app_emat.h"
#include "taf_ps_api.h"
#include "si_app_stk.h"
#include "taf_iface_api.h"
#include "AtParse.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_msg_chk_api.h"
#include "at_ndserver_interface.h"
#include "at_data_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * 结构说明: DIAL类型
 */
enum AT_DIALTYPE {
    AT_DIALTYPE_APP  = 1,
    AT_DIALTYPE_NDIS = 2,

    AT_DIALTYPE_BUTT
};

typedef VOS_UINT32 AT_DIALTYPE_Uint32;

/*
* 结构说明: 速率显示分为上行和下行速率
*/
typedef struct {
    VOS_UINT8 dlSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8 ulSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8 reserved[2];
} AT_DisplayRate;

#define AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_PTR() (g_atRptIfaceResultTab)
#define AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_SIZE() (AT_ARRAY_SIZE(g_atRptIfaceResultTab))

VOS_VOID AT_PS_ProcAppIfaceStatus(TAF_IFACE_StatusInd *ifaceStatus);
VOS_UINT32 AT_RcvTafPsCallEvtLimitPdpActInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtDyamicParaCnf(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_VOID AT_PS_ProcNdisIfaceStatus(TAF_IFACE_StatusInd *ifaceStatus);
VOS_UINT32 AT_IsNdisIface(VOS_UINT8 ifaceId);
#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaEmcPdnActivateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaEmcPdnDeactivateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvTafPsCallRptCmdCfgInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_PS_ReportSetApLanAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryApLanAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportSetIpv6TempAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportSetDhcpRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryDhcpRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportSetDhcpv6Rst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryDhcpv6Rst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportSetApRaInfoRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryApRaInfoRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportSetApConnStRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryApConnStRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryDconnStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportQryNdisStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_PS_ReportTestDconnStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_PS_ReportSetCgmtuRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
VOS_UINT32 AT_RcvTafIfaceEvtIfaceStatusInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtDataChannelStateInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtUsbNetOperInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafIfaceEvtRabInfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvTafIfaceEvtIpChangeInd(VOS_UINT8 idx, TAF_Ctrl *evtInfo);
#endif /* #if (FEATURE_UE_MODE_NR == FEATURE_ON) */
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_PS_MbbReportQryNdisStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evt);
#endif
VOS_UINT32 AT_RcvTafIfaceEvtIPv6PdnInfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsCallEvtPdpManageInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsCallEvtPdpModifiedInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
/*
 * 功能描述: A核收到ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_IND事件的处理
 */
VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivatedInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtReportDsFlowInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtReportVTFlowInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtPdpDisconnectInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtApDsFlowReportInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtEpdgCtrluNtf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtIPv6InfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_VOID AT_PS_PackNdserverIpv6ParaWithTafInfo(const TAF_PS_Ipv6InfoInd *srcIpv6Info,
    AT_NDSERVER_Ipv6PdnInfo *dstIpv6Info);
VOS_VOID AT_ModemPsRspPdpDeactivatedEvtProc(VOS_UINT8 indexNum, TAF_PS_CallPdpDeactivateInd *event);
VOS_VOID AT_ProcNdisDisconnectInd(VOS_UINT32 ifaceId, VOS_UINT32 specIfaceFlag);
VOS_UINT32 AT_RcvDmsLowWaterMarkEvent(struct MsgCB *msg);
VOS_UINT32 AT_RcvDmsNcmConnBreakEvent(struct MsgCB *msg);
VOS_UINT32 AT_RcvDmsUsbDisconnectEvent(struct MsgCB *msg);
VOS_UINT32 AT_RcvDmsSwitchGwModeEvent(struct MsgCB *msg);
#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_VOID AT_ProcDmsEscSeqInPppDataMode(AT_ClientIdUint16 clientId);
#endif
VOS_UINT32 AT_RcvDmsEscapeSequenceEvent(struct MsgCB *msg);
VOS_UINT32 AT_RcvDmsDtrDeassertedEvent(struct MsgCB *msg);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 AT_RcvDmsUartInitEvent(struct MsgCB *msg);
#endif

/*
 * 功能描述: 获取空口理论带宽，从NAS获取，且将字符串型式转为整形
 */
VOS_UINT32 AT_GetDisplayRate(VOS_UINT16 clientId, AT_DisplayRate *speed);
/*
 * 功能描述: 获取DHCP参数(DHCP参数为网络序)
 */
VOS_VOID AT_GetDhcpPara(AT_DHCP_Para *config, TAF_IFACE_Ipv4DhcpParam *ipv4Dhcp);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
