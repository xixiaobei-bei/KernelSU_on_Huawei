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
#include "at_phone_ims_rslt_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_IMS_RSLT_PROC_C

#define AT_IPV4_ADDR_INDEX_1 0
#define AT_IPV4_ADDR_INDEX_2 1
#define AT_IPV4_ADDR_INDEX_3 2
#define AT_IPV4_ADDR_INDEX_4 3

VOS_VOID AT_PcscfIpv4Addr2Str(VOS_CHAR *pcString, VOS_UINT32 strBufLen, VOS_UINT8 *number)
{
    VOS_INT printResult;

    if (strBufLen < TAF_MAX_IPV4_ADDR_STR_LEN) {
        return;
    }

    (VOS_VOID)memset_s(pcString, strBufLen, 0, strBufLen);

    printResult = snprintf_s(pcString, strBufLen, strBufLen - 1, "%d.%d.%d.%d", number[AT_IPV4_ADDR_INDEX_1],
                             number[AT_IPV4_ADDR_INDEX_2], number[AT_IPV4_ADDR_INDEX_3], number[AT_IPV4_ADDR_INDEX_4]);
    TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(printResult, strBufLen, strBufLen - 1);
}

VOS_UINT32 AT_Ipv6AddrToStr(VOS_UINT8 aucAddrStr[], VOS_UINT32 strBufLen, VOS_UINT8 aucIpAddr[],
    AT_IPV6_StrTypeUint8 ipStrType)
{
    VOS_UINT8 *buffer = VOS_NULL_PTR;
    VOS_UINT16 addrValue[AT_IPV6_STR_MAX_TOKENS];
    VOS_UINT16 addrNum;
    VOS_UINT8  delimiter;
    VOS_UINT8  tokensNum;
    VOS_UINT8  radix;
    VOS_UINT32 i;

    buffer = aucAddrStr;

    (VOS_VOID)memset_s(addrValue, sizeof(addrValue), 0x00, sizeof(addrValue));

    /* 根据IP字符串格式表达类型, 配置对应的转换参数 */
    switch (ipStrType) {
        case AT_IPV6_STR_TYPE_HEX:
            delimiter = AT_IPV6_STR_DELIMITER;
            tokensNum = AT_IPV6_ADDR_HEX_TOKEN_NUM;
            radix     = AT_IPV6_STR_HEX_RADIX;
            break;

        case AT_IPV6_STR_TYPE_DEC:
            delimiter = AT_IPV4_STR_DELIMITER;
            tokensNum = AT_IPV6_STR_MAX_TOKENS;
            radix     = AT_IPV6_STR_DEC_RADIX;
            break;

        default:
            return VOS_ERR;
    }

    /* 根据IP字符串格式表达类型, 获取分段的IP地址数值 */
    for (i = 0; i < tokensNum; i++) {
        addrNum = *aucIpAddr++;

        if (ipStrType == AT_IPV6_STR_TYPE_HEX) {
            addrNum <<= 8;
            addrNum |= *aucIpAddr++;
        }

        addrValue[i] = addrNum;
    }

    /* 遍历IP地址分段, 创建有分隔符标记的IP地址字符串 */
    for (i = 0; i < tokensNum; i++) {
        buffer = (VOS_UINT8 *)AT_Itoa(addrValue[i], (VOS_CHAR *)buffer, radix,
                                      (strBufLen - (VOS_UINT32)(aucAddrStr - buffer)));
        *buffer++ = delimiter;
    }

    /* 取出最后一个分隔符, 补充字符串结束符 */
    if (aucAddrStr != buffer) {
        --buffer;
        *buffer = '\0';
    }

    return VOS_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaBatteryInfoSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_BatteryInfoSetCnf *batteryInfoSetCnf = (IMSA_AT_BatteryInfoSetCnf *)msg;
    AT_ModemImsContext        *localBatteryInfo  = AT_GetModemImsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);

    g_atSendDataBuff.bufLen = 0;

    /* 判断设置操作是否成功 */
    if (batteryInfoSetCnf->result == VOS_OK) {
        localBatteryInfo->batteryInfo.currBatteryInfo = localBatteryInfo->batteryInfo.tempBatteryInfo;
        localBatteryInfo->batteryInfo.tempBatteryInfo = AT_IMSA_BATTERY_STATUS_BUTT;
        AT_NORM_LOG1("AT_RcvImsaBatteryInfoSetCnf: Local enBatteryInfo is ",
                     localBatteryInfo->batteryInfo.currBatteryInfo);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_VOID At_FillIpv6AddrInCmd(VOS_UINT16 *length, VOS_UINT8 *addr, VOS_UINT32 addrValidFlg, VOS_UINT32 sipPort,
                              VOS_UINT32 portValidFlg)
{
    VOS_UINT8 iPv6Str[TAF_MAX_IPV6_ADDR_DOT_STR_LEN];

    (VOS_VOID)memset_s(iPv6Str, TAF_MAX_IPV6_ADDR_DOT_STR_LEN, 0, TAF_MAX_IPV6_ADDR_DOT_STR_LEN);

    /* IPV6地址有效 */
    if (addrValidFlg == VOS_TRUE) {
        /* 将IPV6地址从num转换为str */
        AT_Ipv6AddrToStr(iPv6Str, TAF_MAX_IPV6_ADDR_DOT_STR_LEN, addr, AT_IPV6_STR_TYPE_HEX);

        /* 是否存在端口号 */
        if (portValidFlg == VOS_TRUE) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"[%s]:%u\"", iPv6Str, sipPort);
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", iPv6Str);
        }
    }
    /* IPV6地址无效 */
    else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_VOID At_FillIpv4AddrInCmd(VOS_UINT16 *length, VOS_UINT8 *addr, VOS_UINT32 addrValidFlg, VOS_UINT32 sipPort,
                              VOS_UINT32 portValidFlg)
{
    VOS_CHAR iPv4Str[TAF_MAX_IPV4_ADDR_STR_LEN + 1];

    (VOS_VOID)memset_s(iPv4Str, TAF_MAX_IPV4_ADDR_STR_LEN + 1, 0, TAF_MAX_IPV4_ADDR_STR_LEN + 1);

    /* IPV4地址有效 */
    if (addrValidFlg == VOS_TRUE) {
        /* 将IPV4地址从num转换为str */
        AT_PcscfIpv4Addr2Str(iPv4Str, sizeof(iPv4Str), addr);

        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s", iPv4Str);

        /* 是否存在端口号 */
        if (portValidFlg == VOS_TRUE) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ":%u\"", sipPort);
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "\"");
        }
    }
    /* IPV4地址无效 */
    else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_UINT32 AT_RcvImsaPcscfQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_PcscfQryCnf *pcscfCnf = (IMSA_AT_PcscfQryCnf *)msg;
    VOS_UINT16           length = 0;

    AT_INFO_LOG("AT_RcvImsaPcscfQryCnf entered!");

    /* 判断查询操作是否成功 */
    if (pcscfCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            pcscfCnf->atPcscf.src);

        /* <PrimIpv6Pcscf> */
        At_FillIpv6AddrInCmd(&length, pcscfCnf->atPcscf.ipv6Pcscf.primPcscfAddr,
                             pcscfCnf->atPcscf.ipv6Pcscf.opPrimPcscfAddr, pcscfCnf->atPcscf.ipv6Pcscf.primPcscfSipPort,
                             pcscfCnf->atPcscf.ipv6Pcscf.opPrimPcscfSipPort);

        /* <SecIpv6Pcscf> */
        At_FillIpv6AddrInCmd(&length, pcscfCnf->atPcscf.ipv6Pcscf.secPcscfAddr,
                             pcscfCnf->atPcscf.ipv6Pcscf.opSecPcscfAddr, pcscfCnf->atPcscf.ipv6Pcscf.secPcscfSipPort,
                             pcscfCnf->atPcscf.ipv6Pcscf.opSecPcscfSipPort);

        /* <ThiIpv6Pcscf> */
        At_FillIpv6AddrInCmd(&length, pcscfCnf->atPcscf.ipv6Pcscf.thiPcscfAddr,
                             pcscfCnf->atPcscf.ipv6Pcscf.opThiPcscfAddr, pcscfCnf->atPcscf.ipv6Pcscf.thiPcscfSipPort,
                             pcscfCnf->atPcscf.ipv6Pcscf.opThiPcscfSipPort);

        /* <PrimIpv4Pcscf> */
        At_FillIpv4AddrInCmd(&length, pcscfCnf->atPcscf.ipv4Pcscf.primPcscfAddr,
                             pcscfCnf->atPcscf.ipv4Pcscf.opPrimPcscfAddr, pcscfCnf->atPcscf.ipv4Pcscf.primPcscfSipPort,
                             pcscfCnf->atPcscf.ipv4Pcscf.opPrimPcscfSipPort);

        /* <SecIpv4Pcscf> */
        At_FillIpv4AddrInCmd(&length, pcscfCnf->atPcscf.ipv4Pcscf.secPcscfAddr,
                             pcscfCnf->atPcscf.ipv4Pcscf.opSecPcscfAddr, pcscfCnf->atPcscf.ipv4Pcscf.secPcscfSipPort,
                             pcscfCnf->atPcscf.ipv4Pcscf.opSecPcscfSipPort);

        /* <ThiIpv4Pcscf> */
        At_FillIpv4AddrInCmd(&length, pcscfCnf->atPcscf.ipv4Pcscf.thiPcscfAddr,
                             pcscfCnf->atPcscf.ipv4Pcscf.opThiPcscfAddr, pcscfCnf->atPcscf.ipv4Pcscf.thiPcscfSipPort,
                             pcscfCnf->atPcscf.ipv4Pcscf.opThiPcscfSipPort);

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }
    /* 消息携带结果不是OK时，直接返回ERROR */
    else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_VOID At_FillDmdynNumParaInCmd(VOS_UINT16 *length, VOS_UINT32 value, VOS_UINT32 valueValidFlg)
{
    /* 数据有效 */
    if (valueValidFlg == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", value);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_VOID At_FillDmdynStrParaInCmd(VOS_UINT16 *length, VOS_CHAR *pcValue, VOS_UINT32 valueValidFlg,
                                  VOS_UINT32 lastParaFlg)
{
    /* 数据有效 */
    if (valueValidFlg == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%s\"", pcValue);
    }

    /* 不是最后一个参数需要在后面添加逗号 */
    if (lastParaFlg == VOS_FALSE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_UINT32 AT_RcvImsaDmDynQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_DmdynQryCnf *dmDynCnf = (IMSA_AT_DmdynQryCnf *)msg;
    VOS_UINT16           length = 0;

    AT_INFO_LOG("AT_RcvImsaDmDynQryCnf entered!");

    /* 判断查询操作是否成功 */
    if (dmDynCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <ulAmrWbOctetAcigned> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.amrWbOctetAcigned,
                                 (VOS_UINT32)(dmDynCnf->dmdyn.opAmrWbOctetAcigned));

        /* <ulAmrWbBandWidthEfficient> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.amrWbBandWidthEfficient,
                                 (VOS_UINT32)(dmDynCnf->dmdyn.opAmrWbBandWidthEfficient));

        /* <ulAmrOctetAcigned> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.amrOctetAcigned,
                                 (VOS_UINT32)(dmDynCnf->dmdyn.opAmrOctetAcigned));

        /* <ulAmrBandWidthEfficient> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.amrBandWidthEfficient,
                                 (VOS_UINT32)(dmDynCnf->dmdyn.opAmrBandWidthEfficient));

        /* <ulAmrWbMode> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.amrWbMode, (VOS_UINT32)(dmDynCnf->dmdyn.opAmrWbMode));

        /* <ulDtmfWb> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.dtmfWb, (VOS_UINT32)(dmDynCnf->dmdyn.opDtmfWb));

        /* <ulDtmfNb> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.dtmfNb, (VOS_UINT32)(dmDynCnf->dmdyn.opDtmfNb));

        /* <ulSpeechStart> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.speechStart, (VOS_UINT32)(dmDynCnf->dmdyn.opSpeechStart));

        /* <ulSpeechEnd> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.speechEnd, (VOS_UINT32)(dmDynCnf->dmdyn.opSpeechEnd));

        /* <ulSpeechStart> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.videoStart, (VOS_UINT32)(dmDynCnf->dmdyn.opVideoStart));

        /* <ulSpeechEnd> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.videoEnd, (VOS_UINT32)(dmDynCnf->dmdyn.opVideoEnd));

        /* <ulRetryBaseTime> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.retryBaseTime, (VOS_UINT32)(dmDynCnf->dmdyn.opRetryBaseTime));

        /* <ulRetryMaxTime> */
        At_FillDmdynNumParaInCmd(&length, dmDynCnf->dmdyn.retryMaxTime, (VOS_UINT32)(dmDynCnf->dmdyn.opRetryMaxTime));

        /* <acPhoneContext> */
        dmDynCnf->dmdyn.phoneContext[AT_IMSA_PHONECONTEXT_MAX_LENGTH] = '\0';
        At_FillDmdynStrParaInCmd(&length, dmDynCnf->dmdyn.phoneContext, (VOS_UINT32)(dmDynCnf->dmdyn.opPhoneContext),
                                 VOS_FALSE);

        /* <acPhoneContextImpu> */
        dmDynCnf->dmdyn.phoneContextImpu[AT_IMSA_PUBLICEUSERID_MAX_LENGTH] = '\0';
        At_FillDmdynStrParaInCmd(&length, dmDynCnf->dmdyn.phoneContextImpu,
                                 (VOS_UINT32)(dmDynCnf->dmdyn.opPhoneContextImpu), VOS_TRUE);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }
    /* 消息携带结果不是OK时，直接返回ERROR */
    else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_VOID At_FillImsaTimerParaInCmd(VOS_UINT16 *length, VOS_UINT32 value, VOS_UINT32 valueValidFlg,
                                   VOS_UINT32 lastParaFlg)
{
    /* 数据有效 */
    if (valueValidFlg == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", value);
    }
    if (lastParaFlg == VOS_FALSE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_UINT32 AT_RcvImsaImsTimerQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImstimerQryCnf *imsTimerCnf = (IMSA_AT_ImstimerQryCnf *)msg;
    VOS_UINT16              length = 0;

    /* 判断查询操作是否成功 */
    if (imsTimerCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <Timer_T1> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timer1Value, imsTimerCnf->imsTimer.opTimer1Value,
                                  VOS_FALSE);

        /* <Timer_T2> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timer2Value, imsTimerCnf->imsTimer.opTimer2Value,
                                  VOS_FALSE);

        /* <Timer_T4> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timer4Value, imsTimerCnf->imsTimer.opTimer4Value,
                                  VOS_FALSE);

        /* <Timer_TA> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerAValue, imsTimerCnf->imsTimer.opTimerAValue,
                                  VOS_FALSE);

        /* <Timer_TB> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerBValue, imsTimerCnf->imsTimer.opTimerBValue,
                                  VOS_FALSE);

        /* <Timer_TC> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerCValue, imsTimerCnf->imsTimer.opTimerCValue,
                                  VOS_FALSE);

        /* <Timer_TD> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerDValue, imsTimerCnf->imsTimer.opTimerDValue,
                                  VOS_FALSE);

        /* <Timer_TE> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerEValue, imsTimerCnf->imsTimer.opTimerEValue,
                                  VOS_FALSE);

        /* <Timer_TF> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerFValue, imsTimerCnf->imsTimer.opTimerFValue,
                                  VOS_FALSE);

        /* <Timer_TG> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerGValue, imsTimerCnf->imsTimer.opTimerGValue,
                                  VOS_FALSE);

        /* <Timer_TH> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerHValue, imsTimerCnf->imsTimer.opTimerHValue,
                                  VOS_FALSE);

        /* <Timer_TI> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerIValue, imsTimerCnf->imsTimer.opTimerIValue,
                                  VOS_FALSE);

        /* <Timer_TJ> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerJValue, imsTimerCnf->imsTimer.opTimerJValue,
                                  VOS_FALSE);

        /* <Timer_TK> */
        At_FillImsaTimerParaInCmd(&length, imsTimerCnf->imsTimer.timerKValue, imsTimerCnf->imsTimer.opTimerKValue,
                                  VOS_TRUE);

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaImsPsiQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_SmspsiQryCnf *smsPsiCnf = (IMSA_AT_SmspsiQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (smsPsiCnf->result == VOS_OK) {
        smsPsiCnf->smsPsi.smsPsi[AT_IMSA_MAX_SMSPSI_LEN] = '\0';
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: \"%s\"", g_parseContext[indexNum].cmdElement->cmdName,
            smsPsiCnf->smsPsi.smsPsi);

        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaDmUserQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_DmuserQryCnf *dmUserCnf = (IMSA_AT_DmuserQryCnf *)msg;
    TAF_NVIM_SmsDomain    nvSmsDomain;

    (VOS_VOID)memset_s(&nvSmsDomain, sizeof(TAF_NVIM_SmsDomain), 0x00, sizeof(TAF_NVIM_SmsDomain));

    /* 读NV项NV_ITEM_SMS_DOMAIN，失败，直接返回 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SMS_DOMAIN, &nvSmsDomain, (VOS_UINT32)sizeof(TAF_NVIM_SmsDomain)) !=
        NV_OK) {
        AT_ERR_LOG("AT_RcvImsaDmUserQryCnf():WARNING: read NV_ITEM_SMS_DOMAIN Error");

        return AT_ERROR;
    }

    /* 判断查询操作是否成功 */
    if (dmUserCnf->result == VOS_OK) {
        dmUserCnf->dmUser.impi[AT_IMSA_IMPI_MAX_LENGTH]             = '\0';
        dmUserCnf->dmUser.impu[AT_IMSA_IMPU_MAX_LENGTH]             = '\0';
        dmUserCnf->dmUser.homeNetWorkDomain[IMSA_IMS_STRING_LENGTH] = '\0';
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: \"%s\",\"%s\",\"%s\",%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, dmUserCnf->dmUser.impi, dmUserCnf->dmUser.impu,
                dmUserCnf->dmUser.homeNetWorkDomain, dmUserCnf->dmUser.voiceDomain, nvSmsDomain.smsDomain,
                dmUserCnf->dmUser.ipsecEnable);

        return AT_OK;

    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvMmaRoamImsSupportSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RoamImsSupportSetCnf *roamImsSupportSetCnf = (TAF_MMA_RoamImsSupportSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (roamImsSupportSetCnf->roamImsSupportResult == TAF_MMA_ROAM_IMS_SUPPORT_RESULT_SUCCESS) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaRoamImsServiceQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_RoamingImsQryCnf *roamImsServiceCnf = (IMSA_AT_RoamingImsQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (roamImsServiceCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                ((roamImsServiceCnf->roamingImsSupportFlag == AT_IMSA_ROAMING_IMS_SUPPORT) ? 1 : 0));

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaCiregQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_CiregQryCnf *ciregCnf = (IMSA_AT_CiregQryCnf *)msg;
    VOS_UINT32           result;
    VOS_UINT16           length = 0;

    /* 判断查询操作是否成功 */
    if (ciregCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName, ciregCnf->cireg);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", ciregCnf->regInfo);

        /* 如果IMS未注册，<ext_info>参数无意义，且不输出，详见3GPP 27007 v11 8.71 */
        if ((ciregCnf->regInfo != VOS_FALSE) && (ciregCnf->opExtInfo != VOS_FALSE)) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", ciregCnf->extInfo);
        }

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

VOS_UINT32 AT_RcvImsaCirepQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_CirepQryCnf *cirepCnf = (IMSA_AT_CirepQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (cirepCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            cirepCnf->report, cirepCnf->imsVops);

        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}
#endif

