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
#include "at_custom_ims_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"

#include "at_data_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_IMS_QRY_CMD_PROC_C

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryImsSmsCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给MMA 发送查询请求消息 */
    result = TAF_MMA_QryImsSmsCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);

    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSSMSCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryImsRegErrRpt(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG1("AT_QryImsRegErrRpt: cmt type is not AT_CMD_OPT_READ_CMD", g_atParseCmd.cmdOptType);
        return AT_ERROR;
    }

    /* 给IMSA发送^IMSREGERRRPT查询请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_REGERR_REPORT_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSREGERRRPT_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCimsErrPara(VOS_UINT8 indexNum)
{
    TAF_CALL_ErrorInfoText *callErrInfo = VOS_NULL_PTR;

    callErrInfo = AT_GetCallErrInfoText(indexNum);

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,\"%s\"", g_parseContext[indexNum].cmdElement->cmdName,
        AT_GetCsCallErrCause(indexNum), callErrInfo->errInfoText);

    return AT_OK;
}

VOS_UINT32 AT_QryImsSrvStatRptCfgPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG1("AT_QryImsSrvStatRptCfgPara: cmt type is not AT_CMD_OPT_READ_CMD", g_atParseCmd.cmdOptType);
        return AT_ERROR;
    }

    /* 给IMSA发送^IMSSRVSTATRPT查询请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_IMS_SRV_STAT_RPT_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMS_SRV_STAT_RPT_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryImsSrvStatusPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        AT_ERR_LOG1("AT_QryImsSrvStatusPara: cmt type is not AT_CMD_OPT_READ_CMD", g_atParseCmd.cmdOptType);
        return AT_ERROR;
    }

    /* 给IMSA发送^IMSSRVSTATUS查询请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_IMS_SERVICE_STATUS_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMS_SERVICE_STATUS_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QrySipTransPort(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_TRANSPORT_TYPE_QRY_REQ, VOS_NULL_PTR, 0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QrySipTransPort: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIPPORT_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryImsEmcRdpPara(VOS_UINT8 indexNum)
{
    AT_IMS_EmcRdp *imsEmcRdp = VOS_NULL_PTR;
    VOS_CHAR       acIpv4AddrStr[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_UINT8      ipv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT16     length = 0;

    (VOS_VOID)memset_s(acIpv4AddrStr, sizeof(acIpv4AddrStr), 0x00, sizeof(acIpv4AddrStr));
    (VOS_VOID)memset_s(ipv6AddrStr, sizeof(ipv6AddrStr), 0x00, sizeof(ipv6AddrStr));

    /* 获取IMS EMC RDP */
    imsEmcRdp = AT_GetImsEmcRdpByClientId(indexNum);
    if (imsEmcRdp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_QryImsEmcRdpPara: ImsEmcRdp not found.");
        return AT_ERROR;
    }

    /* 检查当前是否有IMS EMC PDN连接 */
    if ((imsEmcRdp->opIPv4PdnInfo != VOS_TRUE) && (imsEmcRdp->opIPv6PdnInfo != VOS_TRUE)) {
        return AT_ERROR;
    }

    if (imsEmcRdp->opIPv4PdnInfo == VOS_TRUE) {
        /* ^IMSEMCRDP: */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        /* IP Type */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", "\"IPV4\"");
        /* IP Addr */
        AT_Ipv4AddrItoa(acIpv4AddrStr, sizeof(acIpv4AddrStr), imsEmcRdp->iPv4PdnInfo.ipAddr);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acIpv4AddrStr);

        /* Primary DNS */
        AT_Ipv4AddrItoa(acIpv4AddrStr, sizeof(acIpv4AddrStr), imsEmcRdp->iPv4PdnInfo.dnsPrimAddr);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acIpv4AddrStr);

        /* Secondary DNS */
        AT_Ipv4AddrItoa(acIpv4AddrStr, sizeof(acIpv4AddrStr), imsEmcRdp->iPv4PdnInfo.dnsSecAddr);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acIpv4AddrStr);

        /* MTU */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", imsEmcRdp->iPv4PdnInfo.mtu);
    }

    if ((imsEmcRdp->opIPv4PdnInfo == VOS_TRUE) && (imsEmcRdp->opIPv6PdnInfo == VOS_TRUE)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
    }

    if (imsEmcRdp->opIPv6PdnInfo == VOS_TRUE) {
        /* ^IMSEMCRDP: */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        /* IP Type */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", "\"IPV6\"");

        /* IP Addr */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, imsEmcRdp->iPv6PdnInfo.ipAddr, TAF_IPV6_STR_RFC2373_TOKENS);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", ipv6AddrStr);

        /* Primary DNS */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, imsEmcRdp->iPv6PdnInfo.dnsPrimAddr, TAF_IPV6_STR_RFC2373_TOKENS);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", ipv6AddrStr);

        /* Secondary DNS */
        AT_ConvertIpv6AddrToCompressedStr(ipv6AddrStr, imsEmcRdp->iPv6PdnInfo.dnsSecAddr, TAF_IPV6_STR_RFC2373_TOKENS);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", ipv6AddrStr);

        /* MTU */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", imsEmcRdp->iPv6PdnInfo.mtu);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}
#endif

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_QryEclImsCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                          result;
    TAF_NVIM_CustomEcallCfg             ecallCfg;

    (VOS_VOID)memset_s(&ecallCfg, sizeof(ecallCfg), 0x00, sizeof(ecallCfg));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 先读取NV的值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, &ecallCfg, sizeof(TAF_NVIM_CustomEcallCfg));
    /* NV读取失败或NV未激活时，返回error */
    if (result != NV_OK) {
        AT_ERR_LOG("AT_QryEclImsCfgPara: NV read!");
        return AT_ERROR;
    }

    /* ecallDomainMode 为CS_PREFER时，不需要去IMSA查询 */
    if (ecallCfg.ecallDomainMode == TAF_CALL_ECALL_DOMAIN_CS_PREFER) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                AT_IMSA_ECALL_CONTENT_TYPE_MODE_CS_PREFER);
        return AT_OK;
    }
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                ID_AT_IMSA_ECALL_ECONTENT_TYPE_QRY_REQ, VOS_NULL_PTR,
                0, PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_QryEclImsCfgPara: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLIMSCFG_QRY;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

