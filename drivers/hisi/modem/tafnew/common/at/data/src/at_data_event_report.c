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
#include "at_data_event_report.h"
#include "taf_type_def.h"
#include "securec.h"
#include "at_taf_agent_interface.h"

#include "ppp_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "at_oam_interface.h"
#include "mn_comm_api.h"

#include "at_input_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_event_report.h"
#include "at_ss_comm.h"
#include "at_data_comm.h"
#include "at_data_event_report.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "at_custom_comm_rslt_proc.h"
#endif

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_EVENT_REPORT_C

#define AT_TOKENS_BIT_LEN 16

static const AT_PS_DynamicInfoReportFuncTbl g_atDynamicInfoReportFuncTbl[] = {
    { AT_CMD_IPV6TEMPADDR_SET, AT_PS_ReportSetIpv6TempAddrRst },
    { AT_CMD_DHCP_SET, AT_PS_ReportSetDhcpRst },
    { AT_CMD_DHCP_QRY, AT_PS_ReportQryDhcpRst },
    { AT_CMD_DHCPV6_SET, AT_PS_ReportSetDhcpv6Rst },
    { AT_CMD_DHCPV6_QRY, AT_PS_ReportQryDhcpv6Rst },
    { AT_CMD_APRAINFO_SET, AT_PS_ReportSetApRaInfoRst },
    { AT_CMD_APRAINFO_QRY, AT_PS_ReportQryApRaInfoRst },
    { AT_CMD_APLANADDR_SET, AT_PS_ReportSetApLanAddrRst },
    { AT_CMD_APLANADDR_QRY, AT_PS_ReportQryApLanAddrRst },
    { AT_CMD_APCONNST_SET, AT_PS_ReportSetApConnStRst },
    { AT_CMD_APCONNST_QRY, AT_PS_ReportQryApConnStRst },
    { AT_CMD_DCONNSTAT_QRY, AT_PS_ReportQryDconnStatRst },
    { AT_CMD_DCONNSTAT_TEST, AT_PS_ReportTestDconnStatRst },
    { AT_CMD_NDISSTATQRY_QRY, AT_PS_ReportQryNdisStatRst },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_NDISSTATQRY_SET, AT_PS_MbbReportQryNdisStatRst },
#endif
    { AT_CMD_CGMTU_SET, AT_PS_ReportSetCgmtuRst },
};

static const AT_PS_ReportIfaceResult g_atRptIfaceResultTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { TAF_IFACE_USER_TYPE_APP, AT_PS_ProcAppIfaceStatus },
    { TAF_IFACE_USER_TYPE_NDIS, AT_PS_ProcNdisIfaceStatus },
};

VOS_UINT32 AT_RcvTafIfaceEvtDyamicParaCnf(VOS_UINT8 indexNum, TAF_Ctrl *evt)
{
    MN_PS_EVT_FUNC evtFunc = VOS_NULL_PTR;
    VOS_UINT32     i;
    VOS_UINT32     result;

    /* 在事件处理表中查找处理函数 */
    for (i = 0; i < AT_ARRAY_SIZE(g_atDynamicInfoReportFuncTbl); i++) {
        if (g_atClientTab[indexNum].cmdCurrentOpt == g_atDynamicInfoReportFuncTbl[i].cmdCurrentOpt) {
            /* 事件ID匹配 */
            evtFunc = g_atDynamicInfoReportFuncTbl[i].evtFunc;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    result = VOS_ERR;
    if (evtFunc != VOS_NULL_PTR) {
        result = evtFunc(indexNum, evt);
    }

    if (result != VOS_OK) {
        AT_ERR_LOG1("AT_RcvTafIfaceEvtDyamicInfoCnf: Can not handle this message! <CmdCurrentOpt>",
                    g_atClientTab[indexNum].cmdCurrentOpt);
    }

    return result;
}

AT_APP_ConnStateUint32 AT_AppConvertPdpStateToConnStatus(TAF_IFACE_StateUint8 pdpState)
{
    AT_APP_ConnStateUint32 connStatus;

    switch (pdpState) {
        case TAF_IFACE_STATE_ACTING:
            connStatus = AT_APP_DIALING;
            break;

        case TAF_IFACE_STATE_ACTED:
        case TAF_IFACE_STATE_DEACTING:
            connStatus = AT_APP_DIALED;
            break;

        case TAF_IFACE_STATE_IDLE:
        default:
            connStatus = AT_APP_UNDIALED;
            break;
    }

    return connStatus;
}

TAF_IFACE_DynamicInfo* AT_GetDynamicInfoBaseAddr(TAF_IFACE_GetDynamicParaCnf *evt)
{
    return &(evt->dynamicInfo[0]);
}

VOS_UINT32 AT_GetCidByCidMask(VOS_UINT32 cidMask, VOS_UINT8 *cid)
{
    VOS_UINT32 cidTemp;

    for (cidTemp = 1; cidTemp <= TAF_MAX_CID_NV; cidTemp++) {
        /* 当前Bit位为1，则此位为对应的Cid */
        if ((cidMask & ((VOS_UINT32)0x00000001 << cidTemp)) != 0) {
            *cid = (VOS_UINT8)cidTemp;
            return VOS_OK;
        }
    }

    *cid = TAF_INVALID_CID;

    return VOS_ERR;
}

VOS_UINT32 AT_PS_Ipv6IfaceDynamicRstCheck(const VOS_UINT8 indexNum, TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo,
                                          VOS_UINT8 *cid)
{
    VOS_BOOL               preFixJudgeFlag;
    TAF_IFACE_DynamicInfo *dynamicInfo = VOS_NULL_PTR;
    VOS_UINT8              cidTemp = TAF_INVALID_CID;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cidTemp) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck:ERROR: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if (dynamicInfo[cidTemp].opIpv6Valid == VOS_FALSE) {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck:ERROR: bitOpIpv6Valid is FALSE");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    if (dynamicInfo[cidTemp].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck: PDP is not actived in cellular.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

#if (FEATURE_MBB_CUST == FEATURE_ON)
    preFixJudgeFlag = (dynamicInfo[cidTemp].ipv6Info.raInfo.opPrefixAddr != VOS_TRUE) &&
        (dynamicInfo[cidTemp].userType != TAF_IFACE_USER_TYPE_NDIS);
#else
    preFixJudgeFlag = (dynamicInfo[cidTemp].ipv6Info.raInfo.opPrefixAddr != VOS_TRUE);
#endif

    if (preFixJudgeFlag == VOS_TRUE) {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck: Prefix address is not received.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    *cid = cidTemp;

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportSetIpv6TempAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    VOS_UINT16                   length;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_PS_Ipv6IfaceDynamicRstCheck(indexNum, ifaceDynamicInfo, &cid) != VOS_OK) {
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if (!AT_PS_IS_IPV6_ADDR_IID_VALID(dynamicInfo[cid].ipv6Info.dhcpInfo.tmpAddr)) {
        AT_WARN_LOG("AT_PS_ReportSetIpv6TempAddrRst: IID is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[cid].ipv6Info.dhcpInfo.tmpAddr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", ipv6AddrStr);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_VOID AT_GetDhcpPara(AT_DHCP_Para *config, TAF_IFACE_Ipv4DhcpParam *ipv4Dhcp)
{
    /* 将DHCP参数转换为网络序 */
    config->dhcpCfg.iPAddr     = VOS_HTONL(ipv4Dhcp->addr);
    config->dhcpCfg.subNetMask = VOS_HTONL(ipv4Dhcp->netMask);
    config->dhcpCfg.gateWay    = VOS_HTONL(ipv4Dhcp->gateWay);
    config->dhcpCfg.primDNS    = VOS_HTONL(ipv4Dhcp->primDNS);
    config->dhcpCfg.sndDNS     = VOS_HTONL(ipv4Dhcp->secDNS);
}

VOS_UINT32 AT_GetConnectRateIndex(VOS_UINT8 subSysMode)
{
    VOS_UINT32             nvDialRateIndex;

    switch (subSysMode) {
        case TAF_SYS_SUBMODE_GSM:
        case TAF_SYS_SUBMODE_GPRS:
            nvDialRateIndex = g_dialConnectDisplayRate.gprsConnectRate;
            break;

        case TAF_SYS_SUBMODE_EDGE:
            nvDialRateIndex = g_dialConnectDisplayRate.edgeConnectRate;
            break;

        case TAF_SYS_SUBMODE_WCDMA:
            nvDialRateIndex = g_dialConnectDisplayRate.wcdmaConnectRate;
            break;

        case TAF_SYS_SUBMODE_HSDPA:
        case TAF_SYS_SUBMODE_HSDPA_HSUPA:
            nvDialRateIndex = g_dialConnectDisplayRate.dpaConnectRate;
            break;

        default:
            nvDialRateIndex = 0;
            break;
    }
    return nvDialRateIndex;
}

VOS_UINT32 AT_GetDisplayRate(VOS_UINT16 clientId, AT_DisplayRate *speed)
{
    const VOS_UINT8       *dlSpeed = VOS_NULL_PTR;
    const VOS_UINT8       *ulSpeed = VOS_NULL_PTR;
    PPP_RateDisplayUint32  rateDisplay;
    TAF_AGENT_SysMode      sysMode;
    VOS_UINT32             ret;
    VOS_UINT32             dlSpeedLen;
    VOS_UINT32             ulSpeedLen;
    VOS_UINT32             nvDialRateIndex;
    errno_t                memResult;
    VOS_UINT8              dlCategoryIndex;
    VOS_UINT8              ulCategoryIndex;

    /* 变量初始化 */
    (VOS_VOID)memset_s(&sysMode, sizeof(sysMode), 0x00, sizeof(sysMode));

    /* 从C核获取ucRatType和ucSysSubMode */
    ret = TAF_AGENT_GetSysMode(clientId, &sysMode);

    if (ret != VOS_OK) {
        sysMode.ratType    = TAF_PH_INFO_GSM_RAT;
        sysMode.sysSubMode = TAF_SYS_SUBMODE_BUTT;
    }

    nvDialRateIndex = AT_GetConnectRateIndex(sysMode.sysSubMode);

    if ((nvDialRateIndex == 0) || (nvDialRateIndex > AT_DIAL_RATE_DISPALY_NV_ARRAY_LEN)) {
        if ((sysMode.ratType == TAF_PH_INFO_WCDMA_RAT) || (sysMode.ratType == TAF_PH_INFO_TD_SCDMA_RAT)) {
            rateDisplay = AT_GetRateDisplayIndexForWcdma(&g_atDlRateCategory);
            dlSpeed = (VOS_UINT8 *)AT_GetPppDialRateDisplay(rateDisplay);
            ulSpeed = (VOS_UINT8 *)AT_GetPppDialRateDisplay(rateDisplay);
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (sysMode.ratType == TAF_PH_INFO_LTE_RAT) {
            dlCategoryIndex = AT_GetLteUeDlCategoryIndex();
            ulCategoryIndex = AT_GetLteUeUlCategoryIndex();

            dlSpeed = (VOS_UINT8 *)AT_GetLteRateDisplay(dlCategoryIndex).strDlSpeed;
            ulSpeed = (VOS_UINT8 *)AT_GetLteRateDisplay(ulCategoryIndex).strUlSpeed;
        }
#endif
        else {
            rateDisplay = AT_GetRateDisplayIndexForGsm(&sysMode);
            dlSpeed = (VOS_UINT8 *)AT_GetPppDialRateDisplay(rateDisplay);
            ulSpeed = (VOS_UINT8 *)AT_GetPppDialRateDisplay(rateDisplay);
        }
    } else {
        dlSpeed = (VOS_UINT8 *)AT_GetDialRateDisplayNv(nvDialRateIndex - 1);
        ulSpeed = (VOS_UINT8 *)AT_GetDialRateDisplayNv(nvDialRateIndex - 1);
    }

    dlSpeedLen = VOS_StrLen((TAF_CHAR *)dlSpeed);
    ulSpeedLen = VOS_StrLen((TAF_CHAR *)ulSpeed);

    /* 在CONNECT后附上速率信息 */
    if (dlSpeedLen > 0) {
        memResult = memcpy_s(speed->dlSpeed, AT_AP_SPEED_STRLEN + 1, dlSpeed, (VOS_UINT16)dlSpeedLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_AP_SPEED_STRLEN + 1, (VOS_UINT16)dlSpeedLen);
    }
    if (ulSpeedLen > 0) {
        memResult = memcpy_s(speed->ulSpeed, AT_AP_SPEED_STRLEN + 1, ulSpeed, (VOS_UINT16)ulSpeedLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_AP_SPEED_STRLEN + 1, (VOS_UINT16)ulSpeedLen);
    }
    speed->dlSpeed[dlSpeedLen] = '\0';
    speed->ulSpeed[ulSpeedLen] = '\0';

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportSetDhcpRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    AT_DisplayRate               speed;
    AT_DHCP_Para                 dhcpConfig;
    VOS_UINT16                   length;
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cid) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if (dynamicInfo[cid].opIpv4Valid == VOS_FALSE) {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: bitOpIpv4Valid is FALSE");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    if (dynamicInfo[cid].ipv4Info.state != TAF_IFACE_STATE_ACTED) {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: PDP is not actived.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 获取接入理论带宽 */
    (VOS_VOID)memset_s(&speed, sizeof(speed), 0x00, (VOS_SIZE_T)(sizeof(speed)));
    if (AT_GetDisplayRate(indexNum, &speed) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: AT_GetDisplayRate Error!");
    }

    /* 获取DHCP参数(网络序) */
    (VOS_VOID)memset_s(&dhcpConfig, sizeof(dhcpConfig), 0x00, (VOS_SIZE_T)(sizeof(dhcpConfig)));
    AT_GetDhcpPara(&dhcpConfig, &(dynamicInfo[cid].ipv4Info.dhcpInfo));

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.iPAddr);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.subNetMask);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.gateWay);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.gateWay);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.primDNS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.sndDNS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s,", speed.dlSpeed);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", speed.ulSpeed);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportQryDhcpRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    AT_DisplayRate               speed;
    AT_DHCP_Para                 dhcpConfig;
    VOS_UINT32                   i;
    VOS_UINT32                   rst = AT_ERROR;
    VOS_UINT16                   length = 0;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    (VOS_VOID)memset_s(&speed, sizeof(speed), 0x00, (VOS_SIZE_T)(sizeof(speed)));
    (VOS_VOID)memset_s(&dhcpConfig, sizeof(dhcpConfig), 0x00, (VOS_SIZE_T)(sizeof(dhcpConfig)));

    /* 目前DHCP的查询只支持1-11的cid */
    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
            continue;
        }

        if (dynamicInfo[i].opIpv4Valid == VOS_FALSE) {
            continue;
        }

        if (dynamicInfo[i].ipv4Info.state != TAF_IFACE_STATE_ACTED) {
            continue;
        }

        /* 获取接入理论带宽 */
        if (AT_GetDisplayRate(indexNum, &speed) == VOS_ERR) {
            AT_WARN_LOG("AT_PS_ReportQryDhcpRst: AT_GetDisplayRate Error!");
        }

        /* 获取DHCP参数(网络序) */
        AT_GetDhcpPara(&dhcpConfig, &(dynamicInfo[i].ipv4Info.dhcpInfo));

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.iPAddr);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.subNetMask);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.gateWay);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.gateWay);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.primDNS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,", dhcpConfig.dhcpCfg.sndDNS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s,", speed.dlSpeed);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", speed.ulSpeed);

        rst = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rst);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportSetDhcpv6Rst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    AT_DisplayRate               speed;
    VOS_UINT16                   length;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_PS_Ipv6IfaceDynamicRstCheck(indexNum, ifaceDynamicInfo, &cid) != VOS_OK) {
        return VOS_ERR;
    }

    /* 获取接入理论带宽 */
    (VOS_VOID)memset_s(&speed, sizeof(speed), 0x00, (VOS_SIZE_T)(sizeof(speed)));
    if (AT_GetDisplayRate(indexNum, &speed) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetDhcpv6Rst: AT_GetDisplayRate Error!");
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));
    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    /* 填写IPV6地址 */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[cid].ipv6Info.dhcpInfo.addr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", ipv6AddrStr);

    /* 填写IPV6掩码, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

    /* 填写IPV6网关, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

    /* 填写DHCP IPV6, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

    /* 填写IPV6 Primary DNS */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[cid].ipv6Info.dhcpInfo.primDns,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

    /* 填写IPV6 Secondary DNS */
    AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[cid].ipv6Info.dhcpInfo.secDns,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

    /* 填写MAX RX/TX Rate */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", speed.dlSpeed);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", speed.ulSpeed);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportQryDhcpv6Rst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    AT_DisplayRate               speed;
    VOS_UINT32                   i;
    VOS_UINT32                   rst = AT_ERROR;
    VOS_UINT16                   length = 0;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    (VOS_VOID)memset_s(&speed, sizeof(speed), 0x00, (VOS_SIZE_T)(sizeof(speed)));
    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));

    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
            continue;
        }

        if (dynamicInfo[i].opIpv6Valid == VOS_FALSE) {
            continue;
        }

        if (dynamicInfo[i].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
            continue;
        }

        if (dynamicInfo[i].ipv6Info.raInfo.opPrefixAddr != VOS_TRUE) {
            continue;
        }

        /* 获取接入理论带宽 */
        if (AT_GetDisplayRate(indexNum, &speed) == VOS_ERR) {
            AT_WARN_LOG("AT_PS_ReportQryDhcpv6Rst: AT_GetDisplayRate Error!");
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* 填写IPV6地址 */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[i].ipv6Info.dhcpInfo.addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", ipv6AddrStr);

        /* 填写IPV6掩码, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

        /* 填写IPV6网关, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

        /* 填写DHCP IPV6, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

        /* 填写IPV6 Primary DNS */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[i].ipv6Info.dhcpInfo.primDns,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

        /* 填写IPV6 Secondary DNS */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[i].ipv6Info.dhcpInfo.secDns,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);

        /* 填写MAX RX/TX Rate */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", speed.dlSpeed);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", speed.ulSpeed);

        rst = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rst);

    return VOS_OK;
}

LOCAL VOS_VOID AT_PS_FillIpv6FlagInfo(VOS_UINT8 cid, TAF_IFACE_DynamicInfo *dynamicInfo, VOS_UINT16 *length)
{
    VOS_UINT32                   bitM = 0;
    VOS_UINT32                   bitO = 0;

    /* 填写M flag */
    if (dynamicInfo[cid].ipv6Info.raInfo.mflag != VOS_FALSE) {
        bitM = dynamicInfo[cid].ipv6Info.raInfo.mflag;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
        (VOS_CHAR*)g_atSndCodeAddress + (*length), ",%d", bitM);

    /* 填写O flag */
    if (dynamicInfo[cid].ipv6Info.raInfo.oflag != VOS_FALSE) {
        bitO = dynamicInfo[cid].ipv6Info.raInfo.oflag;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
        (VOS_CHAR*)g_atSndCodeAddress + (*length), ",%d", bitO);
}

/*
 * 功能描述: 填写IPV6的RaInfo信息
 */
LOCAL VOS_VOID AT_PS_FillIpv6RaInfo(VOS_UINT8 cid, TAF_IFACE_DynamicInfo *dynamicInfo, VOS_UINT16 *length)
{
    VOS_UINT32                   mtuSize;
    VOS_UINT32                   prefixBitLen;
    VOS_UINT32                   preferredLifetime;
    VOS_UINT32                   validLifetime;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    /* 填写MTU */
    if (dynamicInfo[cid].ipv6Info.raInfo.opMtuSize == VOS_FALSE) {
        mtuSize = 0;
    } else {
        mtuSize = dynamicInfo[cid].ipv6Info.raInfo.mtuSize;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", mtuSize);
    /* 填写Prefix */
    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));

    if (dynamicInfo[cid].ipv6Info.raInfo.opPrefixAddr == VOS_FALSE) {
        prefixBitLen = 0;
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    } else {
        prefixBitLen = dynamicInfo[cid].ipv6Info.raInfo.prefixBitLen;
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, dynamicInfo[cid].ipv6Info.raInfo.prefixAddr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"%s/%d\"", ipv6AddrStr, prefixBitLen);

    /* 填写Preferred Lifetime */
    if (dynamicInfo[cid].ipv6Info.raInfo.opPreferredLifetime == VOS_FALSE) {
        preferredLifetime = 0;
    } else {
        preferredLifetime = dynamicInfo[cid].ipv6Info.raInfo.preferredLifetime;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%u", preferredLifetime);

    if (dynamicInfo[cid].ipv6Info.raInfo.opValidLifetime == VOS_FALSE) {
        validLifetime = 0;
    } else {
        validLifetime = dynamicInfo[cid].ipv6Info.raInfo.validLifetime;
    }

    /* 填写Valid Lifetime */
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%u", validLifetime);

    AT_PS_FillIpv6FlagInfo(cid, dynamicInfo, length);

    g_atSendDataBuff.bufLen = (*length);
}

VOS_UINT32 AT_PS_ReportSetApRaInfoRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    VOS_UINT16                   length;
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cid) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if (dynamicInfo[cid].opIpv6Valid == VOS_FALSE) {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: bitOpIpv6Valid is FALSE");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    if (dynamicInfo[cid].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: PDP is not actived in cellular.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    AT_PS_FillIpv6RaInfo(cid, dynamicInfo, &length);
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

LOCAL VOS_VOID AT_PS_FillAppRaFlagInfo(TAF_IFACE_Ipv6RaInfo *appRaInfoAddr, VOS_UINT16 *length)
{
    VOS_UINT32                   bitM = 0;
    VOS_UINT32                   bitO = 0;

    /* 填写M flag */
    if (appRaInfoAddr->mflag != VOS_FALSE) {
        bitM = appRaInfoAddr->mflag;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
        (VOS_CHAR*)g_atSndCodeAddress + (*length), ",%d", bitM);

    /* 填写O flag */
    if (appRaInfoAddr->oflag != VOS_FALSE) {
        bitO = appRaInfoAddr->oflag;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
        (VOS_CHAR*)g_atSndCodeAddress + (*length), ",%d%s", bitO, g_atCrLf);
}

/*
 * 功能描述: 填写RaInfo信息
 */
LOCAL VOS_VOID AT_PS_FillAppRaInfoPara(TAF_IFACE_Ipv6RaInfo *appRaInfoAddr, VOS_UINT16 *length)
{
    VOS_UINT32                   mtuSize;
    VOS_UINT32                   prefixBitLen;
    VOS_UINT32                   preferredLifetime;
    VOS_UINT32                   validLifetime;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));

    /* 填写MTU */
    if (appRaInfoAddr->opMtuSize == VOS_FALSE) {
        mtuSize = 0;
    } else {
        mtuSize = appRaInfoAddr->mtuSize;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", mtuSize);

    /* 填写Prefix */
    if (appRaInfoAddr->opPrefixAddr == VOS_FALSE) {
        prefixBitLen = 0;
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    } else {
        prefixBitLen = appRaInfoAddr->prefixBitLen;
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, appRaInfoAddr->prefixAddr,
                                          (VOS_UINT8)prefixBitLen / AT_TOKENS_BIT_LEN);
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"%s/%d\"", ipv6AddrStr, prefixBitLen);

    /* 填写Preferred Lifetime */
    if (appRaInfoAddr->opPreferredLifetime == VOS_FALSE) {
        preferredLifetime = 0;
    } else {
        preferredLifetime = appRaInfoAddr->preferredLifetime;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%u", preferredLifetime);

    if (appRaInfoAddr->opValidLifetime == VOS_FALSE) {
        validLifetime = 0;
    } else {
        validLifetime = appRaInfoAddr->validLifetime;
    }

    /* 填写Valid Lifetime */
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%u", validLifetime);

    AT_PS_FillAppRaFlagInfo(appRaInfoAddr, length);
}

VOS_UINT32 AT_PS_ReportQryApRaInfoRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_Ipv6RaInfo        *appRaInfoAddr    = VOS_NULL_PTR;
    VOS_UINT32                   i;
    VOS_UINT32                   rst = AT_ERROR;
    VOS_UINT16                   length = 0;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        appRaInfoAddr = &(dynamicInfo[i].ipv6Info.raInfo);

        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & ((VOS_UINT32)1 << i)) == 0) {
            continue;
        }

        if (dynamicInfo[i].opIpv6Valid == VOS_FALSE) {
            continue;
        }

        if (dynamicInfo[i].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
            continue;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName, i);

        AT_PS_FillAppRaInfoPara(appRaInfoAddr, &length);
    }

    if (length < (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf)) {
        AT_ERR_LOG("AT_PS_ReportQryApRaInfoRst: CID INFO is invalid.");
    } else {
        length -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
        rst = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rst);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportSetApLanAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_Ipv6RaInfo        *ipv6RaInfo       = VOS_NULL_PTR;
    VOS_UINT16                   length;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cid) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if (dynamicInfo[cid].opIpv6Valid == VOS_FALSE) {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: bitOpIpv6Valid is FALSE");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 当前未激活，直接返回error */
    if (dynamicInfo[cid].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: PDP is not actived in cellular.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    ipv6RaInfo = &(dynamicInfo[cid].ipv6Info.raInfo);
    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (ipv6RaInfo->opLanAddr == VOS_FALSE) {
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
    } else {
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, ipv6RaInfo->lanAddr, TAF_IPV6_STR_RFC2373_TOKENS);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", ipv6AddrStr);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", ipv6RaInfo->prefixBitLen);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportQryApLanAddrRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_Ipv6RaInfo        *appRaInfoAddr    = VOS_NULL_PTR;
    VOS_UINT32                   i;
    VOS_UINT32                   rst = AT_ERROR;
    VOS_UINT16                   length = 0;
    VOS_UINT8                    ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                    invalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));
    (VOS_VOID)memset_s(invalidIpv6Addr, sizeof(invalidIpv6Addr), 0x00, sizeof(invalidIpv6Addr));

    if (g_atClientTab[indexNum].userType == AT_APP_USER) {
        for (i = 1; i <= TAF_MAX_CID_NV; i++) {
            appRaInfoAddr = &(dynamicInfo[i].ipv6Info.raInfo);

            /* 判断此CID是否有效 */
            if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
                continue;
            }

            if (dynamicInfo[i].opIpv6Valid == VOS_FALSE) {
                continue;
            }

            /* 当前未激活 */
            if (dynamicInfo[i].ipv6Info.state != TAF_IFACE_STATE_ACTED) {
                continue;
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName, i);

            if (appRaInfoAddr->opLanAddr == VOS_FALSE) {
                AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, invalidIpv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);
            } else {
                AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, appRaInfoAddr->lanAddr, TAF_IPV6_STR_RFC2373_TOKENS);
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", ipv6AddrStr);

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d%s", appRaInfoAddr->prefixBitLen, g_atCrLf);
        }
    }

    if (length < (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf)) {
        AT_ERR_LOG("AT_PS_ReportQryApLanAddrRst: CID INFO is invalid.");
    } else {
        length -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
        rst = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rst);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportSetApConnStRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);
    AT_APP_ConnStateUint32       ipv4Status = AT_APP_UNDIALED;
    AT_APP_ConnStateUint32       ipv6Status = AT_APP_UNDIALED;
    VOS_UINT16                   length = 0;
    VOS_UINT8                    cid = TAF_INVALID_CID;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cid) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetApConnStRst: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    if (dynamicInfo[cid].opEtherValid == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,\"IPV4\",%d,\"IPV6\",%d,\"Ethernet\"",
            g_parseContext[indexNum].cmdElement->cmdName, AT_APP_UNDIALED, AT_APP_UNDIALED,
            AT_AppConvertPdpStateToConnStatus(dynamicInfo[cid].etherInfo.state));
    } else {
        if ((dynamicInfo[cid].opIpv4Valid == VOS_FALSE) && (dynamicInfo[cid].opIpv6Valid == VOS_FALSE)) {
            AT_WARN_LOG("AT_PS_ReportSetApConnStRst: bitOpIpv4v6Valid both FALSE");
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, AT_ERROR);
            return VOS_ERR;
        }

        switch (AT_GetIpv6Capability()) {
            case AT_IPV6_CAPABILITY_IPV4_ONLY:
                ipv4Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[cid].ipv4Info.state);

                length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d,\"IPV4\"", ipv4Status);
                break;

            case AT_IPV6_CAPABILITY_IPV6_ONLY:
                ipv6Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[cid].ipv6Info.state);

                length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d,\"IPV6\"", ipv6Status);
                break;

            case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                ipv4Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[cid].ipv4Info.state);
                ipv6Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[cid].ipv6Info.state);

                length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d,\"IPV4\"", ipv4Status);
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,\"IPV6\"", ipv6Status);
                break;

            default:
                break;
        }
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportQryApConnStRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);
    AT_APP_ConnStateUint32       ipv4Status = AT_APP_UNDIALED;
    AT_APP_ConnStateUint32       ipv6Status = AT_APP_UNDIALED;
    VOS_UINT32                   i;
    VOS_UINT16                   length = 0;

    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & ((VOS_UINT32)1 << i)) == 0) {
            continue;
        }

        if (dynamicInfo[i].opEtherValid == VOS_TRUE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,\"IPV4\",%d,\"IPV6\",%d,\"Ethernet\"%s",
                g_parseContext[indexNum].cmdElement->cmdName, i, AT_APP_UNDIALED, AT_APP_UNDIALED,
                AT_AppConvertPdpStateToConnStatus(dynamicInfo[i].etherInfo.state), g_atCrLf);
            continue;
        }

        if ((dynamicInfo[i].opIpv4Valid == VOS_FALSE) && (dynamicInfo[i].opIpv6Valid == VOS_FALSE)) {
            continue;
        }

        switch (AT_GetIpv6Capability()) {
            case AT_IPV6_CAPABILITY_IPV4_ONLY:
                ipv4Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[i].ipv4Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,\"IPV4\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv4Status, g_atCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV6_ONLY:
                ipv6Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[i].ipv6Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,\"IPV6\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv6Status, g_atCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                ipv4Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[i].ipv4Info.state);
                ipv6Status = AT_AppConvertPdpStateToConnStatus(dynamicInfo[i].ipv6Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,\"IPV4\",%d,\"IPV6\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv4Status, ipv6Status, g_atCrLf);
                break;

            default:
                break;
        }
    }

    if (length >= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf)) {
        length -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

AT_PDP_StatusUint32 AT_NdisGetConnStatus(TAF_IFACE_StateUint8 pdpState)
{
    AT_PDP_StatusUint32 connStatus = AT_PDP_STATUS_DEACT;

    if ((pdpState == TAF_IFACE_STATE_ACTED) || (pdpState == TAF_IFACE_STATE_DEACTING)) {
        connStatus = AT_PDP_STATUS_ACT;
    }

    return connStatus;
}

VOS_UINT32 AT_PS_ReportQryDconnStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);
    TAF_PDP_PrimContext          pdpPriPara;
    AT_PDP_StatusUint32          ipv4Status;
    AT_PDP_StatusUint32          ipv6Status;
    AT_PDP_StatusUint32          etherStatus;
    AT_DIALTYPE_Uint32           dialType;
    VOS_UINT32                   i;
    errno_t                      memResult;
    VOS_UINT16                   length = 0;
    VOS_UINT8                    str[TAF_MAX_APN_LEN + 1];

    (VOS_VOID)memset_s(&pdpPriPara, sizeof(pdpPriPara), 0x00, sizeof(pdpPriPara));
    AT_StopTimerCmdReady(indexNum);

    /* 目前查询只支持1-11的cid */
    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* ^DCONNSTAT: */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* CID */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", i);

        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            continue;
        }

        if ((dynamicInfo[i].opIpv4Valid == VOS_FALSE) && (dynamicInfo[i].opIpv6Valid == VOS_FALSE) &&
            (dynamicInfo[i].opEtherValid == VOS_FALSE)) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            continue;
        }

        ipv4Status  = AT_NdisGetConnStatus(dynamicInfo[i].ipv4Info.state);
        ipv6Status  = AT_NdisGetConnStatus(dynamicInfo[i].ipv6Info.state);
        etherStatus = AT_NdisGetConnStatus(dynamicInfo[i].etherInfo.state);

        if ((ipv4Status == AT_PDP_STATUS_DEACT) && (ipv6Status == AT_PDP_STATUS_DEACT) &&
            (etherStatus == AT_PDP_STATUS_DEACT)) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            continue;
        }

        if (dynamicInfo[i].userType == TAF_IFACE_USER_TYPE_NDIS) {
            dialType = AT_DIALTYPE_NDIS;
        } else if (dynamicInfo[i].userType == TAF_IFACE_USER_TYPE_APP) {
            dialType = AT_DIALTYPE_APP;
        } else {
            dialType = AT_DIALTYPE_BUTT;
        }

        /* APN */
        (VOS_VOID)memset_s(str, sizeof(str), 0x00, sizeof(str));
        if (dynamicInfo[i].apnLen > 0) {
            memResult = memcpy_s(str, sizeof(str), dynamicInfo[i].apn, TAF_MIN(dynamicInfo[i].apnLen, TAF_MAX_APN_LEN));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(str), TAF_MIN(dynamicInfo[i].apnLen, TAF_MAX_APN_LEN));
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", str);

        if (AT_GetEthernetCap() != VOS_TRUE) {
            /* IPV4 IPV6 STATUS */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d%s", ipv4Status, ipv6Status, dialType, g_atCrLf);
        } else {
            /* IPV4 IPV6 Ethernet STATUS */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d,%d%s", ipv4Status, ipv6Status, dialType,
                etherStatus, g_atCrLf);
        }
    }

    length -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportTestDconnStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);
    AT_PDP_StatusUint32          ipv4Status;
    AT_PDP_StatusUint32          ipv6Status;
    AT_PDP_StatusUint32          etherStatus;
    VOS_UINT32                   i;
    VOS_UINT16                   length = 0;
    VOS_UINT8                    hasDialedFlg = VOS_FALSE;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName, "(");

    /* 目前查询只支持1-11的cid */
    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
            continue;
        }

        if ((dynamicInfo[i].opIpv4Valid == VOS_FALSE) && (dynamicInfo[i].opIpv6Valid == VOS_FALSE) &&
            (dynamicInfo[i].opEtherValid == VOS_FALSE)) {
            continue;
        }

        ipv4Status  = AT_NdisGetConnStatus(dynamicInfo[i].ipv4Info.state);
        ipv6Status  = AT_NdisGetConnStatus(dynamicInfo[i].ipv6Info.state);
        etherStatus = AT_NdisGetConnStatus(dynamicInfo[i].etherInfo.state);

        if ((ipv4Status == AT_PDP_STATUS_ACT) || (ipv6Status == AT_PDP_STATUS_ACT) ||
            (etherStatus == AT_PDP_STATUS_ACT)) {
            hasDialedFlg = VOS_TRUE;

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", i);
        }
    }

    if (hasDialedFlg == VOS_TRUE) {
        /* 删除最后一个逗号 */
        length--;

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", ")");
    } else {
        /* 若无连接态CID,直接返回OK */
        length = 0;
    }

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_PS_ReportQryNdisStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);
    AT_PDP_StatusUint32          ipv4Status = AT_PDP_STATUS_DEACT;
    AT_PDP_StatusUint32          ipv6Status = AT_PDP_STATUS_DEACT;
    VOS_UINT32                   i;
    VOS_UINT16                   length = 0;
    const VOS_UINT8              strIpv4[] = "IPV4";
    const VOS_UINT8              strIpv6[] = "IPV6";
    VOS_UINT8                    strEther[] = "Ethernet";

    for (i = 1; i <= TAF_MAX_CID_NV; i++) {
        /* 判断此CID是否有效 */
        if ((ifaceDynamicInfo->cid & (0x01UL << i)) == 0) {
            continue;
        }

        if (dynamicInfo[i].opEtherValid == VOS_TRUE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,,,\"%s\"%s",
                g_parseContext[indexNum].cmdElement->cmdName, i, AT_NdisGetConnStatus(dynamicInfo[i].etherInfo.state),
                strEther, g_atCrLf);
            continue;
        }

        if ((dynamicInfo[i].opIpv4Valid == VOS_FALSE) && (dynamicInfo[i].opIpv6Valid == VOS_FALSE)) {
            continue;
        }

        switch (AT_GetIpv6Capability()) {
            case AT_IPV6_CAPABILITY_IPV4_ONLY:
                ipv4Status = AT_NdisGetConnStatus(dynamicInfo[i].ipv4Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,,,\"%s\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv4Status, strIpv4, g_atCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV6_ONLY:
                ipv6Status = AT_NdisGetConnStatus(dynamicInfo[i].ipv6Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,,,\"%s\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv6Status, strIpv6, g_atCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                ipv4Status = AT_NdisGetConnStatus(dynamicInfo[i].ipv4Info.state);
                ipv6Status = AT_NdisGetConnStatus(dynamicInfo[i].ipv6Info.state);

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,,,\"%s\",%d,,,\"%s\"%s",
                    g_parseContext[indexNum].cmdElement->cmdName, i, ipv4Status, strIpv4, ipv6Status, strIpv6,
                    g_atCrLf);
                break;

            default:
                break;
        }
    }

    if (length >= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf)) {
        length -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
    }

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: 上报NDISSTATQRY查询命令结果
 * 输入参数: index: AT通道ID
 *           evtInfo: 查询结果信息指针
 */
VOS_UINT32 AT_PS_MbbReportQryNdisStatRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_DynamicInfo *dynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    VOS_UINT32 ipv4Status;
    VOS_UINT32 ipv6Status;
    VOS_UINT16 stringLen;
    VOS_UINT8 userCid;

    /* 入参指针检查 */
    if (evtInfo == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;
    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    /* 获取USER CID */
    userCid = AT_PS_GetUsrCidFromCidMask(ifaceDynamicInfo->cid);
    if (userCid > TAF_MAX_CID_NV) {
        return VOS_ERR;
    }

    /* 获取IPv4和IPv6的拨号结果 */
    ipv4Status = AT_NdisGetConnStatus(dynamicInfo[userCid].ipv4Info.state);
    ipv6Status = AT_NdisGetConnStatus(dynamicInfo[userCid].ipv6Info.state);

    /* 格式化字符串结果 */
    stringLen = AT_PS_FormatNdisStatPrint(indexNum, userCid, ipv4Status, ipv6Status);

    g_atSendDataBuff.bufLen = stringLen;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}
#endif

VOS_UINT32 AT_PS_ReportSetCgmtuRst(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_GetDynamicParaCnf *ifaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DynamicInfo       *dynamicInfo      = VOS_NULL_PTR;
    VOS_UINT16                   length;
    VOS_UINT8                    cid = TAF_INVALID_CID;

    ifaceDynamicInfo = (TAF_IFACE_GetDynamicParaCnf *)evtInfo;

    if (AT_GetCidByCidMask(ifaceDynamicInfo->cid, &cid) == VOS_ERR) {
        AT_WARN_LOG("AT_PS_ReportSetCgmtuRst: ulBitCid is invalid.");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    dynamicInfo = AT_GetDynamicInfoBaseAddr(ifaceDynamicInfo);

    if ((dynamicInfo[cid].opIpv4Valid == VOS_FALSE) && (dynamicInfo[cid].opIpv6Valid == VOS_FALSE) &&
        (dynamicInfo[cid].opEtherValid == VOS_FALSE)) {
        AT_WARN_LOG("AT_PS_ReportSetCgmtuRst: bitOpIpv4Valid and bitOpIpv6Valid is FALSE");
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 上报查询结果 */
    if (AT_GetEthernetCap() == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName, cid,
            dynamicInfo[cid].ipv4Info.mtu, dynamicInfo[cid].ipv6Info.raInfo.mtuSize, dynamicInfo[cid].etherInfo.mtu);
    } else {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName, cid,
            dynamicInfo[cid].ipv4Info.mtu, dynamicInfo[cid].ipv6Info.raInfo.mtuSize);
    }
    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_VOID AT_PS_ReportAppIfaceUp(const TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT16 length = 0;

    if ((ifaceStatus->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DCONN:%d,\"IPV4\"%s", g_atCrLf, ifaceStatus->cid, g_atCrLf);
    }

    if ((ifaceStatus->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DCONN:%d,\"IPV6\"%s", g_atCrLf, ifaceStatus->cid, g_atCrLf);
    }

    if (ifaceStatus->pdpType == TAF_PDP_ETHERNET) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DCONN:%d,\"Ethernet\"%s", g_atCrLf, ifaceStatus->cid,
            g_atCrLf);
    }

    At_SendResultData((VOS_UINT8)ifaceStatus->ctrl.clientId, g_atSndCodeAddress, length);
}

LOCAL VOS_VOID AT_ReportDownExPara(const TAF_IFACE_StatusInd *ifaceStatus, VOS_UINT16 *length)
{
    /* ^DEND:<CID>,<CAUSE>,<IP-TYPE>[,<back-off-timer>,<allowed_ssc_mode>] */
    if ((ifaceStatus->opBackOffTimer == VOS_TRUE) && (ifaceStatus->opAllowedSscMode == VOS_TRUE)) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,0x%02x%s", ifaceStatus->backOffTimer,
            ifaceStatus->allowedSscMode, g_atCrLf);
    } else if (ifaceStatus->opBackOffTimer == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,%s", ifaceStatus->backOffTimer, g_atCrLf);
    } else if (ifaceStatus->opAllowedSscMode == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",,0x%02x%s", ifaceStatus->allowedSscMode, g_atCrLf);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
    }
}

VOS_VOID AT_PS_ReportAppIfaceDown(const TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT16 length = 0;

    if ((ifaceStatus->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DEND:%d,%d,\"IPV4\"", g_atCrLf, ifaceStatus->cid,
            ifaceStatus->cause);

        AT_ReportDownExPara(ifaceStatus, &length);
    }

    if ((ifaceStatus->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DEND:%d,%d,\"IPV6\"", g_atCrLf, ifaceStatus->cid,
            ifaceStatus->cause);

        AT_ReportDownExPara(ifaceStatus, &length);
    }

    if (ifaceStatus->pdpType == TAF_PDP_ETHERNET) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DEND:%d,%d,\"Ethernet\"", g_atCrLf, ifaceStatus->cid,
            ifaceStatus->cause);

        AT_ReportDownExPara(ifaceStatus, &length);
    }

    At_SendResultData((VOS_UINT8)ifaceStatus->ctrl.clientId, g_atSndCodeAddress, length);
}

VOS_VOID AT_PS_ReportNdisIfaceStatEx(const TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT32 ul3gppSmCause = AT_Get3gppSmCauseByPsCause(ifaceStatus->cause);
    VOS_UINT16 length = 0;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 按照拨号结果处理保存的拨号拒绝原因值 */
    AT_PS_ProcSmCauseByPsResult(ifaceStatus, ul3gppSmCause);
#endif
    if ((ifaceStatus->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    if ((ifaceStatus->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    if (ifaceStatus->pdpType == TAF_PDP_ETHERNET) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTATEX].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    At_SendResultData((VOS_UINT8)ifaceStatus->ctrl.clientId, g_atSndCodeAddress, length);
}

VOS_VOID AT_PS_ReportNdisIfaceStat(const TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT32 ul3gppSmCause = AT_Get3gppSmCauseByPsCause(ifaceStatus->cause);
    VOS_UINT16 length = 0;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 按照拨号结果处理保存的拨号拒绝原因值 */
    AT_PS_ProcSmCauseByPsResult(ifaceStatus, ul3gppSmCause);
#endif
    if ((ifaceStatus->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:0,%d,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ul3gppSmCause, g_atCrLf);
        }
        else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:1,,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, g_atCrLf);
        }
    }

    if ((ifaceStatus->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:0,%d,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ul3gppSmCause, g_atCrLf);
        }
        else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:1,,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, g_atCrLf);
        }
    }

    if (ifaceStatus->pdpType == TAF_PDP_ETHERNET) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:0,%d,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ul3gppSmCause, g_atCrLf);
        }
        else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:1,,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, g_atCrLf);
        }
    }

    At_SendResultData((VOS_UINT8)ifaceStatus->ctrl.clientId, g_atSndCodeAddress, length);
}

VOS_VOID AT_PS_ProcAppIfaceStatus(TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 处理E5、闪电卡、E355等形态的拨号 */
    /* PCUI口下发上报^NDISSTAT，APP口下发上报^NDISSTATEX */
    if ((*systemAppConfig) == SYSTEM_APP_WEBUI) {
        if ((ifaceStatus->status == TAF_IFACE_STATUS_ACT) || (ifaceStatus->status == TAF_IFACE_STATUS_DEACT)) {
            if (AT_CheckAppUser((VOS_UINT8)ifaceStatus->ctrl.clientId) == VOS_TRUE) {
                AT_PS_ReportNdisIfaceStatEx(ifaceStatus);
                return;
            }

            AT_PS_ReportNdisIfaceStat(ifaceStatus);
        }

        return;
    }

    if (ifaceStatus->status == TAF_IFACE_STATUS_ACT) {
        AT_PS_ReportAppIfaceUp(ifaceStatus);
    }

    if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
        AT_PS_ReportAppIfaceDown(ifaceStatus);
    }
}

VOS_UINT32 AT_RcvTafPsCallEvtLimitPdpActInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallLimitPdpActInd *limitPdpActInd = VOS_NULL_PTR;
    ModemIdUint16              modemId = MODEM_ID_0;
    VOS_UINT16                 length = 0;

    limitPdpActInd = (TAF_PS_CallLimitPdpActInd *)evtInfo;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsCallEvtLimitPdpActInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* ^LIMITPDPACT: <FLG>,<CAUSE><CR><LF> */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s: %d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_LIMITPDPACT].text,
        limitPdpActInd->limitFlg, limitPdpActInd->cause, g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_VOID AT_PS_ReportMultiNdisIfaceStat(const TAF_IFACE_StatusInd *ifaceStatus)
{
    VOS_UINT32 ul3gppSmCause = AT_Get3gppSmCauseByPsCause(ifaceStatus->cause);
    VOS_UINT16 length = 0;

    if ((ifaceStatus->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"IPV4\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    if ((ifaceStatus->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"IPV6\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    if (ifaceStatus->pdpType == TAF_PDP_ETHERNET) {
        if (ifaceStatus->status == TAF_IFACE_STATUS_DEACT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,0,%d,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, ul3gppSmCause, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d,1,,,\"Ethernet\"%s", g_atCrLf,
                g_atStringTab[AT_STRING_NDISSTAT].text, ifaceStatus->cid, g_atCrLf);
        }
    }

    At_SendResultData((VOS_UINT8)ifaceStatus->ctrl.clientId, g_atSndCodeAddress, length);
}

VOS_VOID AT_PS_ProcNdisIfaceStatus(TAF_IFACE_StatusInd *ifaceStatus)
{
    TAF_PS_CallRptCmdUint8 cmdType = AT_GetCommPsCtxAddr()->stickCmd;
    if ((ifaceStatus->status == TAF_IFACE_STATUS_ACT) || (ifaceStatus->status == TAF_IFACE_STATUS_DEACT)) {
        switch (cmdType) {
            case TAF_PS_CALL_RPT_CMD_NDISSTAT_WITH_CID:
                AT_PS_ReportMultiNdisIfaceStat(ifaceStatus);
                return;
            case TAF_PS_CALL_RPT_CMD_NDISSTATEX:
                AT_PS_ReportNdisIfaceStatEx(ifaceStatus);
                return;
            case TAF_PS_CALL_RPT_CMD_NDISSTAT:
            default:
                AT_PS_ReportNdisIfaceStat(ifaceStatus);
        }
    }
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaEmcPdnActivateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_EmcPdnActivateInd *pdnActivateInd = (IMSA_AT_EmcPdnActivateInd *)msg;
    AT_IMS_EmcRdp             *imsEmcRdp      = VOS_NULL_PTR;
    errno_t                    memResult;

    /* 获取IMS EMC RDP */
    imsEmcRdp = AT_GetImsEmcRdpByClientId(indexNum);
    if (imsEmcRdp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvImsaEmcPdnActivateInd: ImsEmcRdp not found.");
        return VOS_ERR;
    }

    /* 清除IMS EMC信息 */
    (VOS_VOID)memset_s(imsEmcRdp, sizeof(AT_IMS_EmcRdp), 0x00, sizeof(AT_IMS_EmcRdp));

    /* 更新IPv4 PDN信息 */
    if ((pdnActivateInd->pdpAddr.pdpType == TAF_PDP_IPV4) || (pdnActivateInd->pdpAddr.pdpType == TAF_PDP_IPV4V6)) {
        imsEmcRdp->opIPv4PdnInfo = VOS_TRUE;

        memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.ipAddr, sizeof(imsEmcRdp->iPv4PdnInfo.ipAddr),
                             pdnActivateInd->pdpAddr.ipv4Addr, sizeof(pdnActivateInd->pdpAddr.ipv4Addr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.ipAddr), sizeof(pdnActivateInd->pdpAddr.ipv4Addr));

        if (pdnActivateInd->ipv4Dns.bitOpPrimDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr, sizeof(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr),
                                 pdnActivateInd->ipv4Dns.primDnsAddr, sizeof(pdnActivateInd->ipv4Dns.primDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr),
                                sizeof(pdnActivateInd->ipv4Dns.primDnsAddr));
        }

        if (pdnActivateInd->ipv4Dns.bitOpSecDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.dnsSecAddr, sizeof(imsEmcRdp->iPv4PdnInfo.dnsSecAddr),
                                 pdnActivateInd->ipv4Dns.secDnsAddr, sizeof(pdnActivateInd->ipv4Dns.secDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.dnsSecAddr),
                                sizeof(pdnActivateInd->ipv4Dns.secDnsAddr));
        }

        imsEmcRdp->iPv4PdnInfo.mtu = pdnActivateInd->mtu;
    }

    /* 更新IPv6 PDN信息 */
    if ((pdnActivateInd->pdpAddr.pdpType == TAF_PDP_IPV6) || (pdnActivateInd->pdpAddr.pdpType == TAF_PDP_IPV4V6)) {
        imsEmcRdp->opIPv6PdnInfo = VOS_TRUE;

        memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.ipAddr, sizeof(imsEmcRdp->iPv6PdnInfo.ipAddr),
                             pdnActivateInd->pdpAddr.ipv6Addr, sizeof(pdnActivateInd->pdpAddr.ipv6Addr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.ipAddr), sizeof(pdnActivateInd->pdpAddr.ipv6Addr));

        if (pdnActivateInd->ipv6Dns.bitOpPrimDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr, sizeof(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr),
                                 pdnActivateInd->ipv6Dns.primDnsAddr, sizeof(pdnActivateInd->ipv6Dns.primDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr),
                                sizeof(pdnActivateInd->ipv6Dns.primDnsAddr));
        }

        if (pdnActivateInd->ipv6Dns.bitOpSecDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.dnsSecAddr, sizeof(imsEmcRdp->iPv6PdnInfo.dnsSecAddr),
                                 pdnActivateInd->ipv6Dns.secDnsAddr, sizeof(pdnActivateInd->ipv6Dns.secDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.dnsSecAddr),
                                sizeof(pdnActivateInd->ipv6Dns.secDnsAddr));
        }

        imsEmcRdp->iPv6PdnInfo.mtu = pdnActivateInd->mtu;
    }

    /* 上报连接状态 */
    AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_ACT);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaEmcPdnDeactivateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_IMS_EmcRdp *imsEmcRdp = AT_GetImsEmcRdpByClientId(indexNum);

    if (imsEmcRdp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvImsaEmcPdnDeactivateInd: ImsEmcRdp not found.");
        return VOS_ERR;
    }

    /* 检查IMS EMC状态 */
    if ((imsEmcRdp->opIPv4PdnInfo != VOS_TRUE) && (imsEmcRdp->opIPv6PdnInfo != VOS_TRUE)) {
        AT_WARN_LOG("AT_RcvImsaEmcPdnDeactivateInd: IMS EMC PDN not active.");
        return VOS_ERR;
    }

    /* 清除IMS EMC信息 */
    (VOS_VOID)memset_s(imsEmcRdp, sizeof(AT_IMS_EmcRdp), 0x00, sizeof(AT_IMS_EmcRdp));

    /* 上报连接状态 */
    AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_DEACT);

    return VOS_OK;
}
#endif

AT_PS_RPT_IFACE_RSLT_FUNC AT_PS_GetRptIfaceResultFunc(const TAF_IFACE_UserTypeUint8 userType)
{
    const AT_PS_ReportIfaceResult *rptIfaceRsltFuncTblPtr = VOS_NULL_PTR;
    AT_PS_RPT_IFACE_RSLT_FUNC      rptIfaceRsltFunc       = VOS_NULL_PTR;
    VOS_UINT32                     cnt;

    rptIfaceRsltFuncTblPtr = AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_PTR();

    /* 用户类型匹配 */
    for (cnt = 0; cnt < AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_SIZE(); cnt++) {
        if (userType == rptIfaceRsltFuncTblPtr[cnt].userType) {
            rptIfaceRsltFunc = rptIfaceRsltFuncTblPtr[cnt].rptIfaceRsltFunc;
            break;
        }
    }

    return rptIfaceRsltFunc;
}

VOS_UINT32 AT_RcvTafPsCallRptCmdCfgInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallRptCmdCfgInd *cmdCfg = (TAF_PS_CallRptCmdCfgInd *)evtInfo;

    AT_GetCommPsCtxAddr()->stickCmd = cmdCfg->stickCmd;
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafIfaceEvtIfaceStatusInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_StatusInd      *ifaceStatusInd = VOS_NULL_PTR;
    AT_PS_RPT_IFACE_RSLT_FUNC rptIfaceRsltFunc;

    ifaceStatusInd = (TAF_IFACE_StatusInd *)evtInfo;

    if (ifaceStatusInd->status == TAF_IFACE_STATUS_ACT) {
        AT_PS_SetPsCallErrCause(ifaceStatusInd->ctrl.clientId, TAF_PS_CAUSE_SUCCESS);
    }

    if (ifaceStatusInd->status == TAF_IFACE_STATUS_DEACT) {
        AT_PS_SetPsCallErrCause(ifaceStatusInd->ctrl.clientId, ifaceStatusInd->cause);
    }

    /* 在事件处理表中查找处理函数 */
    rptIfaceRsltFunc = AT_PS_GetRptIfaceResultFunc(ifaceStatusInd->userType);

    /* 如果处理函数存在则调用 */
    if (rptIfaceRsltFunc != VOS_NULL_PTR) {
        rptIfaceRsltFunc(ifaceStatusInd);
    } else {
        AT_ERR_LOG("AT_RcvTafIfaceEvtIfaceStatusInd:ERROR: User type or enStatus is invalid!");
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafIfaceEvtDataChannelStateInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_DataChannelStateInd *dataChannelStateInd = VOS_NULL_PTR;
    AT_PS_DataChanlCfg            *chanCfg             = VOS_NULL_PTR;
    VOS_UINT8                      cid;

    dataChannelStateInd = (TAF_IFACE_DataChannelStateInd *)evtInfo;
    cid                 = dataChannelStateInd->cid;

    /* 检查CID合法性 */
    if (cid > TAF_MAX_CID) {
        AT_ERR_LOG1("AT_RcvTafIfaceEvtDataChannelStateInd, WARNING, CID error:%d", cid);
        return VOS_ERR;
    }

    chanCfg = AT_PS_GetDataChanlCfg(dataChannelStateInd->ctrl.clientId, cid);

    /* 网卡配置未使能 */
    if (chanCfg->used == VOS_FALSE) {
        AT_WARN_LOG("AT_RcvTafIfaceEvtDataChannelStateInd: Channel is not config!");
        AT_CleanDataChannelCfg(chanCfg);
        return VOS_ERR;
    }

    if (dataChannelStateInd->opActFlg == VOS_TRUE) {
        /* 将IFACE激活标志置上 */
        chanCfg->ifaceActFlg = VOS_TRUE;
        return VOS_OK;
    }

    if (dataChannelStateInd->opCleanFlg == VOS_TRUE) {
        /* 清除CID与数传通道的映射关系 */
        AT_CleanDataChannelCfg(chanCfg);
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtDataChannelStateInd: state is not act or clean!");
    return VOS_ERR;
}

VOS_UINT32 AT_IsNdisIface(VOS_UINT8 ifaceId)
{
    switch (ifaceId) {
    case PS_IFACE_ID_NDIS0:
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    case PS_IFACE_ID_NDIS1:
    case PS_IFACE_ID_NDIS2:
    case PS_IFACE_ID_NDIS3:
#endif
        return VOS_TRUE;
    default:
        return VOS_FALSE;
    }
}

VOS_VOID AT_PS_ActiveUsbNet(VOS_UINT8  ifaceId)
{
    VOS_UINT32       linkstus;
    VOS_UINT32       rtn;
    VOS_UINT32       downBitrate;
    VOS_UINT32       upBitrate;
    AT_DisplayRate   speed;
    DMS_PortIdUint16 portId;

    (VOS_VOID)memset_s(&speed, sizeof(speed), 0x00, (VOS_SIZE_T)(sizeof(speed)));

    if (AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &speed) != VOS_OK) {
        AT_ERR_LOG("AT_PS_ActiveUsbNet : ERROR : AT_GetDisplayRate Error!");
    }
    /* 如果速率超出U32的范围，取最大值0xffffffff */
    downBitrate = (AT_AtoI((VOS_CHAR *)speed.dlSpeed, sizeof(speed.dlSpeed)) >= 0xffffffff) ?
                                    0xffffffff :
                                    (VOS_UINT32)AT_AtoI((VOS_CHAR *)speed.dlSpeed, sizeof(speed.dlSpeed));
    upBitrate = (AT_AtoI((VOS_CHAR *)speed.ulSpeed, sizeof(speed.ulSpeed)) >= 0xffffffff) ?
                                  0xffffffff :
                                  (VOS_UINT32)AT_AtoI((VOS_CHAR *)speed.ulSpeed, sizeof(speed.ulSpeed));

    if (AT_IsNdisIface(ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_ActiveUsbNet, not NDIS iface", ifaceId);
        return;
    }

    portId = DMS_PORT_NCM_DATA + (ifaceId - PS_IFACE_ID_NDIS0);

    rtn = DMS_PORT_ChangeConnSpeed(portId, downBitrate, upBitrate);
    if (rtn != VOS_OK) {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Ctrl Speed Fail!");
        return;
    }

    linkstus = DMS_NCM_CONNECTION_LINKUP;
    rtn       = DMS_PORT_ChangeConnLinkState(portId, linkstus);
    if (rtn != VOS_OK) {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Active usb net Fail!");
        return;
    }
}

VOS_VOID AT_PS_DeActiveUsbNet(VOS_UINT8  ifaceId)
{
    DMS_PortIdUint16 portId;
    VOS_UINT32 linkstus = DMS_NCM_CONNECTION_LINKDOWN;
    VOS_UINT32 rtn;

    /* 去激活，已和BSP确认，如果本来是去激活，再去激活并没有影响 */

    if (AT_IsNdisIface(ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_DeActiveUsbNet, not NDIS iface", ifaceId);
        return;
    }

    portId = DMS_PORT_NCM_DATA + (ifaceId - PS_IFACE_ID_NDIS0);

    rtn = DMS_PORT_ChangeConnLinkState(portId, linkstus);
    if (rtn != VOS_OK) {
        AT_ERR_LOG("AT_PS_DeActiveUsbNet, Deactive usb net Fail!");
        return;
    }
}

VOS_UINT32 AT_RcvTafIfaceEvtUsbNetOperInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_UsbnetOperInd *usbNetOperInd = VOS_NULL_PTR;

    usbNetOperInd = (TAF_IFACE_UsbnetOperInd *)evtInfo;

    if (usbNetOperInd->opActUsbNet == VOS_TRUE) {
        AT_PS_ActiveUsbNet(usbNetOperInd->ifaceId);
        return VOS_OK;
    }

    if (usbNetOperInd->opDeactUsbNet == VOS_TRUE) {
        AT_PS_DeActiveUsbNet(usbNetOperInd->ifaceId);
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtUsbNetOperInd: oper is not act or deact usb net!");
    return VOS_ERR;
}

VOS_UINT32 AT_RcvTafIfaceEvtRabInfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_RabInfoInd *rabInfoInd = VOS_NULL_PTR;
    AT_ModemPsCtx        *psModemCtx = VOS_NULL_PTR;
    ModemIdUint16         modemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: Get modem id fail.");
        return VOS_ERR;
    }

    rabInfoInd = (TAF_IFACE_RabInfoInd *)evtInfo;

    if (!AT_PS_IS_RABID_VALID(rabInfoInd->newRabId)) {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: New RabId is invalid.");
        return VOS_ERR;
    }

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (rabInfoInd->ifaceId >= PS_IFACE_ID_BUTT) {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: enIfaceId is invalid.");
        return VOS_ERR;
    }
#endif

    psModemCtx = AT_GetModemPsCtxAddrFromClientId(indexNum);

    switch (rabInfoInd->operType) {
        case TAF_IFACE_RAB_OPER_ADD:
            /* 保存为扩展RABID 等于 modemId + rabId */
            g_atClientTab[indexNum].exPsRabId = AT_BUILD_EXRABID(modemId, rabInfoInd->newRabId);

            if (rabInfoInd->opPdpAddr) {
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
                psModemCtx->ipAddrIfaceIdMap[rabInfoInd->ifaceId] = rabInfoInd->ipAddr;
#else
                psModemCtx->ipAddrRabIdMap[rabInfoInd->newRabId - AT_PS_RABID_OFFSET] = rabInfoInd->ipAddr;
#endif
            }
            break;

        case TAF_IFACE_RAB_OPER_DELETE:
            g_atClientTab[indexNum].exPsRabId = 0;
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
            if ((rabInfoInd->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
                psModemCtx->ipAddrIfaceIdMap[rabInfoInd->ifaceId] = 0;
            }
#else
            psModemCtx->ipAddrRabIdMap[rabInfoInd->newRabId - AT_PS_RABID_OFFSET] = 0;
#endif
            break;

        case TAF_IFACE_RAB_OPER_CHANGE:
            if (!AT_PS_IS_RABID_VALID(rabInfoInd->oldRabId)) {
                AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: Old RabId is invalid.");
                return VOS_ERR;
            }

            /* 保存为扩展RABID 等于 modemId + rabId */
            g_atClientTab[indexNum].exPsRabId = AT_BUILD_EXRABID(modemId, rabInfoInd->newRabId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM != FEATURE_ON)
            psModemCtx->ipAddrRabIdMap[rabInfoInd->newRabId - AT_PS_RABID_OFFSET] =
                psModemCtx->ipAddrRabIdMap[rabInfoInd->oldRabId - AT_PS_RABID_OFFSET];
            psModemCtx->ipAddrRabIdMap[rabInfoInd->oldRabId - AT_PS_RABID_OFFSET] = 0;
#endif

            break;

        default:
            AT_WARN_LOG("AT_RcvTafIfaceEvtRabInfoInd: enOperType is invalid!");
            break;
    }

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_PS_ReportIpChange(VOS_UINT8 idx, TAF_IFACE_IpChangeInd *ipChangeInd)
{
    VOS_UINT16 length = 0;
    VOS_UINT8  ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];

    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^IPCHANGE: %d,%d", g_atCrLf, ipChangeInd->cid,
        ipChangeInd->operateType);
    /* <PDP_type> */
    if (ipChangeInd->pdpType == TAF_PDP_IPV4) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", g_atStringTab[AT_STRING_IP].text);
    } else if (ipChangeInd->pdpType == TAF_PDP_IPV6) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s,", g_atStringTab[AT_STRING_IPV6].text);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", g_atStringTab[AT_STRING_IPV4V6].text);
    }

    if ((ipChangeInd->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%08X", ipChangeInd->ipv4Addr);
    }

    if ((ipChangeInd->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        /* 转换IPV6地址 */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, ipChangeInd->ipv6Addr, TAF_IPV6_STR_RFC2373_TOKENS);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", ipv6AddrStr);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(idx, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvTafIfaceEvtIpChangeInd(VOS_UINT8 idx, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_IpChangeInd *ipChangeInd = VOS_NULL_PTR;
    AT_ModemPsCtx         *atPsCtx     = VOS_NULL_PTR;
    ModemIdUint16          modemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(idx, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafIfaceEvtIpChangeInd: Get modem id fail.");
        return VOS_ERR;
    }

    ipChangeInd = (TAF_IFACE_IpChangeInd *)evtInfo;

    /* 注:此消息只会在新平台使用 */
    if (ipChangeInd->ifaceId >= PS_IFACE_ID_BUTT) {
        AT_ERR_LOG("AT_RcvTafIfaceEvtIpChangeInd: ifaceId is invalid.");
        return VOS_ERR;
    }

    atPsCtx = AT_GetModemPsCtxAddrFromClientId(idx);

    switch (ipChangeInd->operateType) {
        case TAF_IFACE_IP_CHANGE_OPER_ADD:
            /* 在重构PPP拨号时，再考虑是否更新g_atClientTab[index].ucExPsRabId */
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
            if ((ipChangeInd->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4) {
                /* 为CSDN命令触发上行数据包时使用，仅限IPv4类型 */
                atPsCtx->ipAddrIfaceIdMap[ipChangeInd->ifaceId] = ipChangeInd->ipv4Addr;
            }
#endif
            AT_PS_ReportIpChange(idx, ipChangeInd);
            break;

        case TAF_IFACE_IP_CHANGE_OPER_DELETE:
            AT_PS_ReportIpChange(idx, ipChangeInd);
            break;

        case TAF_IFACE_IP_CHANGE_OPER_CHANGE:
            /* CHANGE类型的暂不支持，后续的MHV6时实现 */
        default:
            AT_WARN_LOG("AT_RcvTafIfaceEvtIpChangeInd: operateType is invalid!");
            break;
    }

    return VOS_OK;
}
#endif

LOCAL VOS_VOID AT_PS_PackNdserverIpv6ParaWithIfaceInfo(const TAF_IFACE_Ipv6Info *srcIpv6Info,
    AT_NDSERVER_Ipv6PdnInfo *dstIpv6Info)
{
    errno_t memResult;

    /* 填充主副DNS */
    dstIpv6Info->dnsSer.serNum = 0;
    if (srcIpv6Info->dhcpInfo.opPriDns == VOS_TRUE) {
        memResult = memcpy_s(dstIpv6Info->dnsSer.priServer, sizeof(dstIpv6Info->dnsSer.priServer),
            srcIpv6Info->dhcpInfo.primDns, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->dnsSer.priServer), TAF_IPV6_ADDR_LEN);
        dstIpv6Info->dnsSer.serNum += 1;
    }

    if (srcIpv6Info->dhcpInfo.opSecDns == VOS_TRUE) {
        memResult = memcpy_s(dstIpv6Info->dnsSer.secServer, sizeof(dstIpv6Info->dnsSer.secServer),
            srcIpv6Info->dhcpInfo.secDns, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->dnsSer.secServer), TAF_IPV6_ADDR_LEN);
        dstIpv6Info->dnsSer.serNum += 1;
    }

    /* 填充MTU */
    if (srcIpv6Info->raInfo.opMtuSize == VOS_TRUE) {
        dstIpv6Info->OpMtu = VOS_TRUE;
        dstIpv6Info->Mtu = srcIpv6Info->raInfo.mtuSize;
    }

    dstIpv6Info->curHopLimit = srcIpv6Info->raInfo.curHopLimit;
    dstIpv6Info->ipv6MValue = srcIpv6Info->raInfo.mflag;
    dstIpv6Info->ipv6OValue = srcIpv6Info->raInfo.oflag;
    dstIpv6Info->prefixNum = 1;
    dstIpv6Info->prefixList[0].ipv6LValue = srcIpv6Info->raInfo.lflag;
    dstIpv6Info->prefixList[0].ipv6AValue = srcIpv6Info->raInfo.aflag;
    dstIpv6Info->prefixList[0].ipv6PrefixLen = (VOS_UINT8)srcIpv6Info->raInfo.prefixBitLen;

    dstIpv6Info->prefixList[0].validLifeTime = srcIpv6Info->raInfo.validLifetime;
    dstIpv6Info->prefixList[0].preferredLifeTime = srcIpv6Info->raInfo.preferredLifetime;

    memResult = memcpy_s(dstIpv6Info->prefixList[0].prefix, sizeof(dstIpv6Info->prefixList[0].prefix),
        srcIpv6Info->raInfo.prefixAddr, TAF_IPV6_ADDR_LEN);

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    memResult = memcpy_s(dstIpv6Info->interfaceId, sizeof(dstIpv6Info->interfaceId),
        srcIpv6Info->dhcpInfo.addr, AT_NDIS_IPV6_IFID_LENGTH);

    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->interfaceId), AT_NDSERVER_IPV6_IFID_LENGTH);

    /* 填充主副PCSCF地址  */
    dstIpv6Info->pcscfSer.serNum = 0;
    if (srcIpv6Info->dhcpInfo.pcscfList.pcscfAddrNum > 0) {
        dstIpv6Info->pcscfSer.serNum++;

        memResult = memcpy_s(dstIpv6Info->pcscfSer.priServer, sizeof(dstIpv6Info->pcscfSer.priServer),
            srcIpv6Info->dhcpInfo.pcscfList.pcscfAddrList[0].pcscfAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->pcscfSer.priServer), TAF_IPV6_ADDR_LEN);
    }

    if (srcIpv6Info->dhcpInfo.pcscfList.pcscfAddrNum > 1) {
        dstIpv6Info->pcscfSer.serNum++;

        memResult = memcpy_s(dstIpv6Info->pcscfSer.secServer, sizeof(dstIpv6Info->pcscfSer.secServer),
            srcIpv6Info->dhcpInfo.pcscfList.pcscfAddrList[1].pcscfAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->pcscfSer.secServer), TAF_IPV6_ADDR_LEN);
    }
}

VOS_UINT32 AT_RcvTafIfaceEvtIPv6PdnInfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_Ipv6PdnInfoInd *ipv6InfoInd = (TAF_IFACE_Ipv6PdnInfoInd *)evtInfo;
    AT_NDSERVER_Ipv6PdnInfo ndIpv6Info;

    (VOS_VOID)memset_s(&ndIpv6Info, sizeof(ndIpv6Info), 0, sizeof(ndIpv6Info));

    AT_PS_PackNdserverIpv6ParaWithIfaceInfo(&(ipv6InfoInd->ipv6PdnInfo), &ndIpv6Info);
    AT_PS_SendNdserverPppIPv6CfgInd(&ndIpv6Info);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpManageInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                  memResult;
    VOS_UINT16               length = 0;
    TAF_PS_CallPdpManageInd *event      = VOS_NULL_PTR;
    AT_ModemSsCtx           *modemSsCtx = VOS_NULL_PTR;
    VOS_UINT8 tempValue[TAF_MAX_APN_LEN + 1];

    /* 初始化 */
    event  = (TAF_PS_CallPdpManageInd *)evtInfo;

    /* 命令与协议不符 */
    modemSsCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (modemSsCtx->crcType == AT_CRC_ENABLE_TYPE) {
        /* +CRC -- +CRING: GPRS <PDP_type>, <PDP_addr>[,[<L2P>][,<APN>]] */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s+CRING: GPRS ", g_atCrLf);

        /* <PDP_type> */
        if (event->pdpAddr.pdpType == TAF_PDP_IPV4) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atStringTab[AT_STRING_IP].text);
        } else if (event->pdpAddr.pdpType == TAF_PDP_IPV6) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atStringTab[AT_STRING_IPV6].text);
        } else if (event->pdpAddr.pdpType == TAF_PDP_IPV4V6) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atStringTab[AT_STRING_IPV4V6].text);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atStringTab[AT_STRING_PPP].text);
        }

        /* <PDP_addr> */
        (VOS_VOID)memset_s(tempValue, sizeof(tempValue), 0x00, sizeof(tempValue));
        AT_Ipv4Addr2Str((VOS_CHAR *)tempValue, sizeof(tempValue), event->pdpAddr.ipv4Addr, TAF_IPV4_ADDR_LEN);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", tempValue);

        /* <L2P>没有，<APN> */
        (VOS_VOID)memset_s(tempValue, sizeof(tempValue), 0x00, sizeof(tempValue));
        if (event->apn.length > sizeof(event->apn.value)) {
            AT_WARN_LOG1("AT_RcvTafPsCallEvtPdpManageInd: Invalid pstEvent->stApn.ucLength: ", event->apn.length);
            event->apn.length = sizeof(event->apn.value);
        }

        if (event->apn.length > 0) {
            memResult = memcpy_s(tempValue, sizeof(tempValue), event->apn.value, event->apn.length);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tempValue), event->apn.length);
        }
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",,\"%s\"%s", tempValue, g_atCrLf);
    } else {
        /* +CRC -- RING */
        if (g_atVType == AT_V_ENTIRE_TYPE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%sRING%s", g_atCrLf, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "2\r");
        }
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    /* 不处理 */
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpModifiedInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    return VOS_OK;
}

VOS_VOID AT_ModemPsRspPdpDeactivatedEvtProc(VOS_UINT8 indexNum, TAF_PS_CallPdpDeactivateCnf *event)
{
    VOS_UINT32              aTHCmdFlg;
    VOS_UINT32              result = AT_FAILURE;
    VOS_UINT16              length = 0;
    DMS_PortIdUint16        portId;
    DMS_PortModeUint8       mode;
    DMS_PortDataModeUint8   dataMode;

    aTHCmdFlg = (AT_PS_GET_CURR_CMD_OPT(indexNum) == AT_CMD_H_PS_SET) ? VOS_TRUE : VOS_FALSE;

    portId   = AT_GetDmsPortIdByClientId((AT_ClientIdUint16)indexNum);
    mode     = DMS_PORT_GetMode(portId);
    dataMode = DMS_PORT_GetDataMode(portId);

    if ((event->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        AT_PS_SendNdserverPppIPv6RelInd();
    }

    if (dataMode == DMS_PORT_DATA_PPP) {
        /* 释放PPP实体 & HDLC去使能 */
        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_REQ);

        if (mode == DMS_PORT_MODE_ONLINE_DATA) {
            /*
             * 根据问题单AT2D13296，在被动去激活情景下，向PPP发送了
             * PPP_AT_CTRL_REL_PPP_REQ后，不立即切入命令态，而是等
             * 待PPP回应AT_PPP_PROTOCOL_REL_IND_MSG之后再切入命令态
             */
            /* 开保护定时器，用于等待PPP回应AT_PPP_PROTOCOL_REL_IND_MSG */
            AT_StopTimerCmdReady(indexNum);

            if (At_StartTimer(AT_PPP_PROTOCOL_REL_TIME, indexNum) != AT_SUCCESS) {
                AT_ERR_LOG("At_UsbModemStatusPreProc:ERROR:Start Timer fail");
            }

            /* 设置当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_WAIT_PPP_PROTOCOL_REL_SET;

            return;
        }
    } else if (dataMode == DMS_PORT_DATA_PPP_RAW) {
        /* 释放PPP实体 & HDLC去使能 */
        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
    } else {
        /* 其它数传模式，不用处理 */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVATED OTHER MODE");
    }

    /* 返回命令模式 */
    DMS_PORT_ResumeCmdMode(portId);

    if (aTHCmdFlg == VOS_TRUE) {
        result = AT_OK;
    } else {
        result = AT_NO_CARRIER;
    }

    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ((result == AT_OK) && (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE)) {
        DMS_PORT_DeassertDcd(portId);
    }
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivatedInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallPdpDeactivateInd *event = VOS_NULL_PTR;

    event = (TAF_PS_CallPdpDeactivateInd *)evtInfo;

    AT_PS_DeleteIpAddrMap(indexNum, event);

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGACT_END_SET) {
        AT_StopTimerCmdReady(indexNum);
        DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);
        At_FormatResultData(indexNum, AT_OK);

        return VOS_OK;
    }

    /* 记录PS域呼叫错误码 */
    AT_PS_SetPsCallErrCause(indexNum, event->cause);

    switch (g_atClientTab[indexNum].userType) {
        case AT_HSUART_USER:
        case AT_UART_USER:
        case AT_USBCOM_USER:
        case AT_MODEM_USER:
        case AT_CTR_USER:
        /* 部分CPE产品由于版本一致性要求，不允许存在pcui端口 */
        case AT_SOCK_USER:
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        case AT_CMUXAT_USER:
        case AT_CMUXMDM_USER:
        case AT_CMUXEXT_USER:
        case AT_CMUXGPS_USER:
#endif
            AT_ModemPsRspPdpDeactivatedEvtProc(indexNum, event);
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtReportDsFlowInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ReportDsflowInd *evtMsg  = VOS_NULL_PTR;
    TAF_DSFLOW_Report      *rptInfo = VOS_NULL_PTR;
    ModemIdUint16           modemId = MODEM_ID_0;
    VOS_UINT16              length  = 0;

    evtMsg  = (TAF_PS_ReportDsflowInd *)evtInfo;
    rptInfo = &evtMsg->reportInfo;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsEvtReportDsFlowInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* 上报流量信息 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^DSFLOWRPT: %08X,%08X,%08X,%08X%08X,%08X%08X,%08X,%08X%s",
        g_atCrLf, rptInfo->flowInfo.linkTime, rptInfo->sendRate, rptInfo->recvRate, rptInfo->flowInfo.sendFluxHigh,
        rptInfo->flowInfo.sendFluxLow, rptInfo->flowInfo.recvFluxHigh, rptInfo->flowInfo.recvFluxLow,
        rptInfo->qosSendRate, rptInfo->qosRecvRate, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtReportVTFlowInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ReportVtflowInd *evtMsg  = VOS_NULL_PTR;
    TAF_VTFLOW_Report      *rptInfo = VOS_NULL_PTR;
    ModemIdUint16           modemId = MODEM_ID_0;
    VOS_UINT16              length  = 0;

    evtMsg  = (TAF_PS_ReportVtflowInd *)evtInfo;
    rptInfo = &evtMsg->reportInfo;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsEvtReportVTFlowInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* 上报视频流量信息 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%08X,%08X%08X,%08X%08X%s", g_atCrLf,
        g_atStringTab[AT_STRING_VT_FLOW_RPT].text, rptInfo->linkTime, rptInfo->sendFluxHigh,
        rptInfo->sendFluxLow, rptInfo->recvFluxHigh, rptInfo->recvFluxLow, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtPdpDisconnectInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32 result;

    VOS_UINT16 length = 0;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        /* 未应答的场景直接上报NO CARRIER */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", "NO CARRIER");

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);

        return VOS_OK;
    }

    /* IP类型网络激活^CGANS应答过程中上报ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PPP_ORG_SET) {
        result = AT_NO_CARRIER;

        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* 返回命令模式 */
        DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);

    } else if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGANS_ANS_EXT_SET) ||
               (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGANS_ANS_SET)) {
        /*
         * 1.PPP类型网络激活^CGANS应答过程中上报ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND
         * 2.+CGANS应答
         * 以上两种情况都还没有切数传通道，直接回ERROR
         */
        result = AT_ERROR;
    } else {
        result = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtApDsFlowReportInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ApdsflowReportInd *evtMsg  = VOS_NULL_PTR;
    TAF_APDSFLOW_Report      *rptInfo = VOS_NULL_PTR;
    ModemIdUint16             modemId = MODEM_ID_0;
    VOS_UINT16                length;

    evtMsg  = (TAF_PS_ApdsflowReportInd *)evtInfo;
    rptInfo = &evtMsg->reportInfo;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsEvtApDsFlowReportInd: Get modem id fail.");
        return VOS_ERR;
    }

    /*
     * ^APDSFLOWRPT: <curr_ds_time>,<tx_rate>,<rx_rate>,
     * <curr_tx_flow>,<curr_rx_flow>,<total_tx_flow>,<total_rx_flow>
     */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^APDSFLOWRPT: %08X,%08X,%08X,%08X%08X,%08X%08X,%08X%08X,%08X%08X%s",
        g_atCrLf, rptInfo->currentFlowInfo.linkTime, rptInfo->sendRate, rptInfo->recvRate,
        rptInfo->currentFlowInfo.sendFluxHigh, rptInfo->currentFlowInfo.sendFluxLow,
        rptInfo->currentFlowInfo.recvFluxHigh, rptInfo->currentFlowInfo.recvFluxLow,
        rptInfo->totalFlowInfo.sendFluxHigh, rptInfo->totalFlowInfo.sendFluxLow,
        rptInfo->totalFlowInfo.recvFluxHigh, rptInfo->totalFlowInfo.recvFluxLow, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtEpdgCtrluNtf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_EpdgCtrluNtf *epdgCtrluNtf = VOS_NULL_PTR;
    VOS_UINT8            broadCastIndex;

    epdgCtrluNtf = (TAF_PS_EpdgCtrluNtf *)evtInfo;

    if (At_ClientIdToUserBroadCastId(epdgCtrluNtf->ctrl.clientId, &broadCastIndex) != AT_SUCCESS) {
        AT_WARN_LOG("AT_RcvTafPsEvtEpdgCtrluNtf: At_ClientIdToUserBroadCastId is err!");
        return VOS_ERR;
    }

    if (epdgCtrluNtf->epdgCtrlu.opActReq == VOS_TRUE) {
        AT_PS_ReportImsCtrlMsgu(broadCastIndex, AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(TAF_PS_EpdgActReqInfo)),
                                (VOS_UINT8 *)(&(epdgCtrluNtf->epdgCtrlu.actReqInfo)));
        return VOS_OK;
    }

    if (epdgCtrluNtf->epdgCtrlu.opDeActReq == VOS_TRUE) {
        AT_PS_ReportImsCtrlMsgu(broadCastIndex, AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(TAF_PS_EpdgDeactReqInfo)),
                                (VOS_UINT8 *)(&(epdgCtrluNtf->epdgCtrlu.deActReqInfo)));
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafPsEvtEpdgCtrluNtf: not ACT or DEACT REQ!");
    return VOS_ERR;
}

VOS_VOID AT_PS_PackNdserverIpv6ParaWithTafInfo(const TAF_PS_Ipv6InfoInd *srcIpv6Info,
    AT_NDSERVER_Ipv6PdnInfo *dstIpv6Info)
{
    errno_t memResult;

    /* 填充主副DNS */
    dstIpv6Info->dnsSer.serNum = 0;
    if (srcIpv6Info->ipv6DnsAddr.bitOpPrimDnsAddr == VOS_TRUE) {
        memResult = memcpy_s(dstIpv6Info->dnsSer.priServer, sizeof(dstIpv6Info->dnsSer.priServer),
            srcIpv6Info->ipv6DnsAddr.primDnsAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->dnsSer.priServer), TAF_IPV6_ADDR_LEN);
        dstIpv6Info->dnsSer.serNum += 1;
    }

    /* IPV6 DNS服务器个数超过2个 */
    if (srcIpv6Info->ipv6DnsAddr.bitOpSecDnsAddr == VOS_TRUE) {
        memResult = memcpy_s(dstIpv6Info->dnsSer.secServer, sizeof(dstIpv6Info->dnsSer.secServer),
            srcIpv6Info->ipv6DnsAddr.secDnsAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->dnsSer.secServer), TAF_IPV6_ADDR_LEN);
        dstIpv6Info->dnsSer.serNum += 1;
    }

    /* 填充MTU */
    if (srcIpv6Info->ipv6RaInfo.bitOpMtu == VOS_TRUE) {
        dstIpv6Info->OpMtu = VOS_TRUE;
        dstIpv6Info->Mtu   = srcIpv6Info->ipv6RaInfo.mtu;
    }

    dstIpv6Info->curHopLimit = srcIpv6Info->ipv6RaInfo.ulBitCurHopLimit;
    dstIpv6Info->ipv6MValue  = srcIpv6Info->ipv6RaInfo.ulBitM;
    dstIpv6Info->ipv6OValue  = srcIpv6Info->ipv6RaInfo.ulBitO;
    dstIpv6Info->prefixNum   = 1;
    dstIpv6Info->prefixList[0].ipv6LValue = srcIpv6Info->ipv6RaInfo.prefixList[0].ulBitL;
    dstIpv6Info->prefixList[0].ipv6AValue = srcIpv6Info->ipv6RaInfo.prefixList[0].ulBitA;
    dstIpv6Info->prefixList[0].ipv6PrefixLen = srcIpv6Info->ipv6RaInfo.prefixList[0].ulBitPrefixLen;

    dstIpv6Info->prefixList[0].validLifeTime = srcIpv6Info->ipv6RaInfo.prefixList[0].validLifeTime;
    dstIpv6Info->prefixList[0].preferredLifeTime = srcIpv6Info->ipv6RaInfo.prefixList[0].preferredLifeTime;

    memResult = memcpy_s(dstIpv6Info->prefixList[0].prefix, sizeof(dstIpv6Info->prefixList[0].prefix),
        srcIpv6Info->ipv6RaInfo.prefixList[0].prefix, TAF_IPV6_ADDR_LEN);

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    memResult = memcpy_s(dstIpv6Info->interfaceId, sizeof(dstIpv6Info->interfaceId),
        srcIpv6Info->ipv6PdnAddr.addr, AT_NDIS_IPV6_IFID_LENGTH);

    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->interfaceId), AT_NDSERVER_IPV6_IFID_LENGTH);

    /* 填充主副PCSCF地址  */
    dstIpv6Info->pcscfSer.serNum = 0;
    if (srcIpv6Info->ipv6PcscfList.ipv6PcscfAddrNum > 0) {
        dstIpv6Info->pcscfSer.serNum++;
        memResult = memcpy_s(dstIpv6Info->pcscfSer.priServer, sizeof(dstIpv6Info->pcscfSer.priServer),
            srcIpv6Info->ipv6PcscfList.ipv6PcscfAddrList[0].pcscfAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->pcscfSer.priServer), TAF_IPV6_ADDR_LEN);
    }

    if (srcIpv6Info->ipv6PcscfList.ipv6PcscfAddrNum > 1) {
        dstIpv6Info->pcscfSer.serNum++;
        memResult = memcpy_s(dstIpv6Info->pcscfSer.secServer, sizeof(dstIpv6Info->pcscfSer.secServer),
            srcIpv6Info->ipv6PcscfList.ipv6PcscfAddrList[1].pcscfAddr, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dstIpv6Info->pcscfSer.secServer), TAF_IPV6_ADDR_LEN);
    }
}

VOS_UINT32 AT_RcvTafPsEvtIPv6InfoInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_Ipv6InfoInd *ipv6InfoInd = (TAF_PS_Ipv6InfoInd *)evtInfo;
    AT_NDSERVER_Ipv6PdnInfo ndIpv6Info;

    if ((g_atClientTab[indexNum].userType != AT_MODEM_USER) &&
        (g_atClientTab[indexNum].userType != AT_UART_USER)) {
        AT_WARN_LOG("AT_RcvTafPsEvtIPv6InfoInd: userType not support");
        return VOS_ERR;
    }
    (VOS_VOID)memset_s(&ndIpv6Info, sizeof(ndIpv6Info), 0, sizeof(ndIpv6Info));

    /* 通知NDServerIPv6地址信息 */
    AT_PS_PackNdserverIpv6ParaWithTafInfo(ipv6InfoInd, &ndIpv6Info);
    AT_PS_SendNdserverPppIPv6CfgInd(&ndIpv6Info);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDmsLowWaterMarkEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify *subscripEvent = VOS_NULL_PTR;
    AT_UartCtx                          *uartCtx         = VOS_NULL_PTR;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;
    uartCtx       = AT_GetUartCtxAddr();

    /* CMUX功能打开时，此时操作的可能为CMUX端口 */
#if (FEATURE_IOT_CMUX == FEATURE_ON)
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) {
        subscripEvent->clientId = uartCtx->index;
    }
#endif
#endif

    /* 端口索引检查 */
    if (subscripEvent->clientId >= AT_CLIENT_ID_BUTT) {
        AT_ERR_LOG1("AT_RcvDmsLowWaterMarkEvent : AT INDEX NOT FOUND!", subscripEvent->clientId);
        return VOS_ERR;
    }

    /* 只处理UART端口的低水线消息 */
    if (AT_IsHsuartBaseUser((VOS_UINT8)subscripEvent->clientId) != VOS_TRUE) {
        AT_ERR_LOG1("AT_RcvDmsLowWaterMarkEvent : not hsuart user!", subscripEvent->clientId);
        return VOS_ERR;
    }

    /* TX低水线处理 */
    if (uartCtx->wmLowFunc != VOS_NULL_PTR) {
        uartCtx->wmLowFunc(subscripEvent->clientId);
    }

    return VOS_OK;
}

PS_IFACE_IdUint8 AT_GetNdisIfaceIdByDmsPortId(DMS_PortIdUint16 portId)
{
    switch (portId) {
        case DMS_PORT_NCM_DATA:
            return PS_IFACE_ID_NDIS0;
#if (FEATURE_MULTI_NCM == FEATURE_ON)
        case DMS_PORT_NCM_DATA1:
            return PS_IFACE_ID_NDIS1;
        case DMS_PORT_NCM_DATA2:
            return PS_IFACE_ID_NDIS2;
        case DMS_PORT_NCM_DATA3:
            return PS_IFACE_ID_NDIS3;
#endif
        default:
            return PS_IFACE_ID_NDIS0;
    }
}

LOCAL VOS_VOID AT_ProcNdisIfaceDown(const AT_PS_DataChanlCfg *chanCfg)
{
    TAF_IFACE_Deactive ifaceDown;
    TAF_Ctrl           ctrl;

    (VOS_VOID)memset_s(&ifaceDown, sizeof(ifaceDown), 0x00, sizeof(ifaceDown));
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ifaceDown.ifaceId  = (VOS_UINT8)chanCfg->ifaceId;
    ifaceDown.cause    = TAF_PS_CALL_END_CAUSE_NORMAL;
    ifaceDown.userType = TAF_IFACE_USER_TYPE_NDIS;

    /* 构造控制结构体 */
    (VOS_VOID)AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, g_atClientTab[chanCfg->portIndex].clientId, 0, &ctrl);
    (VOS_VOID)TAF_IFACE_Down(&ctrl, &ifaceDown);
}

VOS_VOID AT_ProcNdisDisconnectInd(VOS_UINT32 ifaceId, VOS_UINT32 specIfaceFlag)
{
    AT_PS_DataChanlCfg *chanCfg = VOS_NULL_PTR;
    VOS_UINT32          i;

    for (i = 1; i <= TAF_MAX_CID; i++) {
        chanCfg = AT_PS_GetDataChanlCfg(AT_NDIS_GET_CLIENT_ID(), (VOS_UINT8)i);

        if ((chanCfg->used == VOS_FALSE) || (chanCfg->portIndex >= AT_CLIENT_ID_BUTT)) {
            continue;
        }
        if (AT_IsNdisIface((VOS_UINT8)chanCfg->ifaceId) == VOS_FALSE) {
            continue;
        }
        /* usb断开 去激活所有ndis网卡 禁用网卡场景只断开特定网卡 */
        if ((specIfaceFlag == VOS_FALSE) || (chanCfg->ifaceId == ifaceId)) {
            AT_ProcNdisIfaceDown(chanCfg);
        }
    }
}

VOS_UINT32 AT_RcvDmsNcmConnBreakEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify *subscripEvent = VOS_NULL_PTR;
    struct DMS_NcmConnBreakInfo *ncmConnBreakInfo = VOS_NULL_PTR;
    VOS_UINT32 ifaceId;
    AT_ClientIdUint16 clientId;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;
    ncmConnBreakInfo = (struct DMS_NcmConnBreakInfo *)subscripEvent->data;
    clientId = subscripEvent->clientId;

    /* NDIS ctrl口发送网卡断开事件 */
    if (clientId != AT_CLIENT_ID_NDIS) {
        return VOS_ERR;
    }

    ifaceId = AT_GetNdisIfaceIdByDmsPortId(ncmConnBreakInfo->ncmDataPortId);

    AT_PR_LOGH("iface id %d", ifaceId);
    AT_ProcNdisDisconnectInd(ifaceId, VOS_TRUE);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDmsUsbDisconnectEvent(struct MsgCB *msg)
{
    AT_ProcNdisDisconnectInd(PS_IFACE_ID_BUTT, VOS_FALSE);

    /* modem端口处理 */
    AT_ModemEnableCB((VOS_UINT8)AT_CLIENT_ID_MODEM, VOS_FALSE);

    /* 从记录表中删除USB相关的client ID */
    AT_RmUsedClientIdFromTab(AT_CLIENT_ID_PCUI);
    AT_RmUsedClientIdFromTab(AT_CLIENT_ID_CTRL);
    AT_RmUsedClientIdFromTab(AT_CLIENT_ID_NDIS);
    AT_RmUsedClientIdFromTab(AT_CLIENT_ID_MODEM);
    AT_RmUsedClientIdFromTab(AT_CLIENT_ID_PCUI2);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDmsSwitchGwModeEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify   *subscripEvent = VOS_NULL_PTR;
    VOS_UINT8                             *systemAppConfig = VOS_NULL_PTR;
    AT_PS_DataChanlCfg                    *chanCfg         = VOS_NULL_PTR;
    TAF_IFACE_Deactive                     ifaceDown;
    TAF_Ctrl                               ctrl;
    VOS_UINT32                             i;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;

    if (subscripEvent->clientId != AT_CLIENT_ID_APP) {
        AT_ERR_LOG1("AT_RcvDmsSwitchGwModeEvent clientid uncorrect.", subscripEvent->clientId);
        return VOS_ERR;
    }

    g_hiLinkMode     = AT_HILINK_GATEWAY_MODE;
    systemAppConfig = AT_GetSystemAppConfigAddr();

    if (*systemAppConfig == SYSTEM_APP_WEBUI) {
        for (i = 1; i <= TAF_MAX_CID; i++) {
            chanCfg = AT_PS_GetDataChanlCfg(g_atClientTab[AT_CLIENT_ID_APP].clientId, (VOS_UINT8)i);

            if ((chanCfg->used == VOS_FALSE) || (chanCfg->portIndex >= AT_CLIENT_ID_BUTT)) {
                continue;
            }

            if ((chanCfg->ifaceId >= PS_IFACE_ID_RMNET0) && (chanCfg->ifaceId <= PS_IFACE_ID_RMNET2)) {
                (VOS_VOID)memset_s(&ifaceDown, sizeof(ifaceDown), 0x00, sizeof(ifaceDown));
                (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

                ifaceDown.ifaceId  = (VOS_UINT8)chanCfg->ifaceId;
                ifaceDown.cause    = TAF_PS_CALL_END_CAUSE_NORMAL;
                ifaceDown.userType = TAF_IFACE_USER_TYPE_APP;

                /* 构造控制结构体 */
                (VOS_VOID)AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, g_atClientTab[chanCfg->portIndex].clientId, 0,
                                               &ctrl);
                (VOS_VOID)TAF_IFACE_Down(&ctrl, &ifaceDown);
            }
        }
    }

    return VOS_OK;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_VOID AT_ProcDmsEscSeqInPppDataMode(AT_ClientIdUint16 clientId)
{
    DMS_PortIdUint16    portId;

    /*
     * 处理原则(enPortIndex对应的端口):
     * (1) ONLINE-DATA模式 - 上报OK
     * (2) 其他模式        - 丢弃
     */
    portId   = AT_GetDmsPortIdByClientId(clientId);

    /* 目前只支持PPP和IP模式下切换为ONLINE-COMMAND模式 */
    if (DMS_PORT_IsPppPacketTransMode(portId) == VOS_TRUE) {
        if (DMS_PORT_IsDsrModeConnectOn() == VOS_TRUE) {
            DMS_PORT_DeassertDsr(portId);
        }

        /*
         * HSUART端口由DATA模式切换到ONLINE_CMD模式:
         * 清除HSUART的缓存队列数据, 防止当前缓存队列满时, 主动上报的命令丢失
         */
        DMS_PORT_FlushTxData(portId);

        /* 切换到online-cmd模式，更新通道的模式，由于是临时切换到online-cmd模式，不做数据子模式更新 */
        DMS_PORT_SetMode(portId, DMS_PORT_MODE_ONLINE_CMD);

        DMS_PORT_SetDataState(portId, DMS_PORT_DATA_STATE_STOP);

        At_FormatResultData((VOS_UINT8)clientId, AT_OK);
    }
}
#endif

VOS_UINT32 AT_RcvDmsEscapeSequenceEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify *subscripEvent = VOS_NULL_PTR;
    DMS_PortIdUint16                     portId;
    DMS_PortModeUint8                    mode;
    DMS_PortDataModeUint8                dataMode;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;

    if (subscripEvent->clientId >= AT_CLIENT_ID_BUTT) {
        AT_ERR_LOG1("AT_RcvDmsEscapeSequenceEvent : AT INDEX abnormal!", subscripEvent->clientId);
        return VOS_ERR;
    }

    portId = AT_GetDmsPortIdByClientId(subscripEvent->clientId);

    mode     = DMS_PORT_GetMode(portId);
    dataMode = DMS_PORT_GetDataMode(portId);

    if (mode != DMS_PORT_MODE_ONLINE_DATA)
    {
        AT_WARN_LOG2("AT_RcvDmsEscapeSequenceEvent abnormal mode!", subscripEvent->clientId, mode);
        return VOS_ERR;
    }

    switch (dataMode)
    {
#if (FEATURE_AT_HSUART == FEATURE_ON)
        case DMS_PORT_DATA_PPP_RAW:
        case DMS_PORT_DATA_PPP:
            AT_ProcDmsEscSeqInPppDataMode(subscripEvent->clientId);
            break;
#endif

        case DMS_PORT_DATA_OM:
            DMS_PORT_ResumeCmdMode(portId);
            break;

        default:
            AT_ERR_LOG1("AT_RcvDmsEscapeSequenceEvent : abnormal data mode !", dataMode);
            break;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvDmsDtrDeassertedEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify *subscripEvent = VOS_NULL_PTR;
    DMS_PortModeUint8                    mode;
    DMS_PortDataModeUint8                dataMode;
    DMS_PortIdUint16                     portId;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;

    /* 端口索引检查 */
    if (subscripEvent->clientId >= AT_CLIENT_ID_BUTT) {
        AT_ERR_LOG1("AT_RcvDmsDtrDeassertedEvent : AT INDEX NOT FOUND!", subscripEvent->clientId);
        return VOS_ERR;
    }

    portId   = AT_GetDmsPortIdByClientId(subscripEvent->clientId);
    mode     = DMS_PORT_GetMode(portId);
    dataMode = DMS_PORT_GetDataMode(portId);

    /* 数传模式响应dtr 变低处理 */
    if ((mode == DMS_PORT_MODE_ONLINE_DATA) || (mode == DMS_PORT_MODE_ONLINE_CMD)) {
        switch (dataMode) {
            case DMS_PORT_DATA_PPP:
                AT_ProcDtrDeassertedIndPppDataMode(subscripEvent->clientId);
                break;

            case DMS_PORT_DATA_PPP_RAW:
                AT_ProcDtrDeassertedIndRawDataMode(subscripEvent->clientId);
                break;

            default:
                AT_WARN_LOG("AT_RcvDmsDtrDeassertedEvent: DataMode Wrong!");
                return VOS_ERR;
        }
    }

    return VOS_OK;
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 AT_RcvDmsUartInitEvent(struct MsgCB *msg)
{
    struct DMS_PORT_SubscripEventNotify *subscripEvent = VOS_NULL_PTR;
    cmux_info_type* cmux_info = VOS_NULL_PTR;
    VOS_UINT32 i = 0;
    VOS_UINT8 indexNum;

    subscripEvent = (struct DMS_PORT_SubscripEventNotify *)msg;

    for (i = 0; i < MAX_CMUX_PORT_NUM; i++) {
        indexNum = (VOS_UINT8)AT_CLIENT_ID_CMUXAT + i;
        /*修改DLC0 DISC无法断开拨号的问题*/
        g_atClientTab[indexNum].dataId = indexNum;
        g_atClientTab[indexNum].ctrlId = indexNum;
    }

    /*恢复AT与UART的通道*/
    if (subscripEvent->clientId == AT_CLIENT_ID_UART) {
        AT_UartInitPort();
    }

    if (subscripEvent->clientId == AT_CLIENT_ID_HSUART) {
        AT_HSUART_InitPort();
    }

    DMS_PORT_SetCmuxOpenFlg(VOS_FALSE);
    DMS_PORT_SetCmuxPhyPort(DMS_PORT_BUTT);

    cmux_info = DMS_PORT_GetCmuxInfoType();
    (VOS_VOID)memset_s(cmux_info, sizeof(cmux_info_type), 0x00, sizeof(cmux_info_type));

    AT_PR_LOGI("CMUX init uart success");

    return VOS_OK;
}
#endif

