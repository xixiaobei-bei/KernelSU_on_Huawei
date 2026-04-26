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
#include "at_phone_ims_set_cmd_proc.h"
#include "securec.h"
#include "taf_app_mma.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_IMS_SET_CMD_PROC_C

#define AT_DMDYN_PARA_NUM 15
#define AT_DMDYN_PHONECONTEXT 13

#define AT_IMSPCSCF_PARA_NUM 7
#define AT_DMTIMER_PARA_NUM 14

#define AT_IMSPCSCF_IPV6_ADDRESS1_INDEX 1
#define AT_IMSPCSCF_IPV6_ADDRESS2_INDEX 2
#define AT_IMSPCSCF_IPV6_ADDRESS3_INDEX 3
#define AT_IMSPCSCF_IPV4_ADDRESS1_INDEX 4
#define AT_IMSPCSCF_IPV4_ADDRESS2_INDEX 5
#define AT_IMSPCSCF_IPV4_ADDRESS3_INDEX 6

#define AT_DMDYN_AWR_WB_OCTET_ALIGNED_INDEX 0
#define AT_DMDYN_AWR_WB_BANDWIDTH_ECT_INDEX 1
#define AT_DMDYN_AWR_OCTET_ALIGNED_INDEX 2
#define AT_DMDYN_AWR_BANDWIDTH_ECT_INDEX 3
#define AT_DMDYN_AWR_WB_MODE_INDEX 4
#define AT_DMDYN_DTMF_WB_INDEX 5
#define AT_DMDYN_DTMF_NB_INDEX 6
#define AT_DMDYN_SPEECH_START_INDEX 7
#define AT_DMDYN_SPEECH_END_INDEX 8
#define AT_DMDYN_VIDEO_START_INDEX 9
#define AT_DMDYN_VIDEO_END_INDEX 10
#define AT_DMDYN_REGRETRYBASETIME_INDEX 11
#define AT_DMDYN_REGRETRYMAXTIME_INDEX 12
#define AT_DMDYN_PHONECONTEXT_INDEX 13
#define AT_DMDYN_PUBLIC_USER_ID_INDEX 14

#define AT_DMTIMER_TIMER_T1_INDEX 0
#define AT_DMTIMER_TIMER_T2_INDEX 1
#define AT_DMTIMER_TIMER_T4_INDEX 2
#define AT_DMTIMER_TIMER_TA_INDEX 3
#define AT_DMTIMER_TIMER_TB_INDEX 4
#define AT_DMTIMER_TIMER_TC_INDEX 5
#define AT_DMTIMER_TIMER_TD_INDEX 6
#define AT_DMTIMER_TIMER_TE_INDEX 7
#define AT_DMTIMER_TIMER_TF_INDEX 8
#define AT_DMTIMER_TIMER_TG_INDEX 9
#define AT_DMTIMER_TIMER_TH_INDEX 10
#define AT_DMTIMER_TIMER_TI_INDEX 11
#define AT_DMTIMER_TIMER_TJ_INDEX 12
#define AT_DMTIMER_TIMER_TK_INDEX 13
#define AT_IMS_PAYLOAD_TYPE_RANGE_MAX 0x7f
#define AT_IMS_PAYLOAD_TYPE_RANGE_MIN 0x60
#define AT_IMS_AMR_WB_MODE_MAX 8
#define AT_IMS_TIMER_DATA_RANGE_MAX 128000 /* 定时器最大时长:单位ms */

VOS_VOID AT_Ipv6LenStrToAddrProcCompressed(VOS_UINT8 *str, VOS_UINT8 colonCount, VOS_UINT8 dotCount,
                                           VOS_UINT8 strlength, VOS_UINT8 idxPos)
{
    VOS_UINT8 i;

    /* 把字符串从压缩位置开始顺次向后移动 */
    for (i = strlength; i >= idxPos; i--) {
        if (dotCount != AT_MAX_IPV6_STR_DOT_NUM) {
            str[i + AT_MAX_IPV6_STR_COLON_NUM - colonCount] = str[i];
        } else {
            str[i + AT_MAX_IPV4V6_STR_COLON_NUM - colonCount] = str[i];
        }
    }

    /* 补足压缩的冒号 */
    if (dotCount != AT_MAX_IPV6_STR_DOT_NUM) {
        for (i = idxPos; i < (idxPos + AT_MAX_IPV6_STR_COLON_NUM - colonCount); i++) {
            str[i + 1] = ':';
        }
    } else {
        for (i = idxPos; i < (idxPos + AT_MAX_IPV4V6_STR_COLON_NUM - colonCount); i++) {
            str[i + 1] = ':';
        }
    }
}

VOS_UINT32 AT_Ipv6LenStrToAddrAccess(VOS_UINT8 *str, VOS_UINT8 *colonCount, VOS_UINT8 *dotCount, VOS_UINT8 *strlength,
                                     VOS_UINT8 *idxPos)
{
    VOS_UINT32 i;

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && (str[i] != '\0')); i++) {
        if ((str[i] < '0' || str[i] > '9') && (str[i] < 'A' || str[i] > 'F') && (str[i] != '.') && (str[i] != ':')) {
            return VOS_ERR;
        }
        /* 取得补充冒号索引位置 */
        if ((i > 0) && (str[i - 1] == ':') && (str[i] == ':')) {
            *idxPos = (VOS_UINT8)i;
        }

        /* 统计冒号个数 */
        if (str[i] == ':') {
            (*colonCount)++;
        }

        /* 统计点号个数 */
        if (str[i] == '.') {
            (*dotCount)++;
        }
    }

    *strlength = (VOS_UINT8)i;

    return VOS_OK;
}

VOS_UINT32 AT_PcscfIpv6StrToAddr(VOS_UINT8 *str, VOS_UINT8 *ipAddr, VOS_UINT8 colonCount)
{
    VOS_UINT32 i;
    VOS_UINT32 j = 0;
    VOS_UINT16 value = 0;     /* Ipv6十六进制转换用 */
    VOS_UINT8  valueTemp = 0; /* Ipv4十六进制转换用 */
    VOS_UINT32 numLen = 0;

    if ((str == VOS_NULL_PTR) || (ipAddr == VOS_NULL_PTR)) {
        AT_NORM_LOG("AT_PcscfIpv6StrToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && (str[i] != '\0')); i++) {
        /* 匹配字符 */
        if (str[i] != ':') {
            value <<= 4;

            if ((str[i] >= '0') && (str[i] <= '9')) {
                /* 十进制格式转换 */
                value += (VOS_UINT16)(str[i] - '0');
                /* 乘10是为了获得高位，在其后加低位，0x30为char型的'0' */
                valueTemp = (VOS_UINT8)((valueTemp * 10) + (str[i] - 0x30));
            } else {
                /* 将十六进制"A"到"F"转换为十进制，需要在末尾加10 */
                value += (VOS_UINT16)((str[i] - 'A') + AT_DECIMAL_BASE_NUM);
            }

            numLen++;
        }
        /* 匹配到冒号 */
        else {
            /* 冒号之间的字符超过4个则认为格式错误 */
            if (numLen > 4) {
                AT_ERR_LOG("AT_PcscfIpv6StrToAddr: the number of char betwwen colons is more than 4, return ERROR");
                return VOS_ERR;
            }

            /* IPV6十六进制取高八位数据 */
            ipAddr[j] = (VOS_UINT8)((value >> 8) & 0x00FF);
            j++;
            /* IPV6十六进制取低八位数据 */
            ipAddr[j] = (VOS_UINT8)(value & 0x00FF);
            j++;
            value  = 0;
            numLen = 0;
        }
    }

    /* 匹配最后一次转换 */
    if (colonCount == AT_MAX_IPV6_STR_COLON_NUM) {
        ipAddr[j] = (VOS_UINT8)((value >> 8) & 0x00FF);
        j++;
        ipAddr[j] = (VOS_UINT8)(value & 0x00FF);
    }

    return VOS_OK;
}

VOS_UINT32 AT_ParseAddrFromPcscfIpv6Addr(VOS_UINT8 *str, VOS_UINT32 strLen, VOS_UINT8 *ipAddr)
{
    VOS_UINT8  colonCount = 0; /* 字符串中冒号个数 */
    VOS_UINT8  dotCount = 0;   /* 字符串中点号个数 */
    VOS_UINT8  strlength = 0;  /* 字符串长度 */
    VOS_UINT8  idxPos = 0xFF;  /* 需要补充冒号的位置 */
    VOS_UINT32 result;

    if ((str == VOS_NULL_PTR) || (ipAddr == VOS_NULL_PTR)) {
        AT_NORM_LOG("AT_ParseAddrFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    /* 遍历IPV6地址字符串 */
    if (AT_Ipv6LenStrToAddrAccess(str, &colonCount, &dotCount, &strlength, &idxPos) != VOS_OK) {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: AT_Ipv6LenStrToAddrAccess FAIL, return ERROR");
        return VOS_ERR;
    }

    /* 不支持IPV4IPV6混合型的格式 */
    if (dotCount != 0) {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: There have dot symbol in address format, return ERROR");
        return VOS_ERR;
    }

    /* 字符串为空返回失败 */
    if (strlength == 0) {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IP address length is 0, return ERROR");
        return VOS_ERR;
    }

    /* 冒号个数大于7则返回失败 */
    if (colonCount > AT_MAX_IPV6_STR_COLON_NUM) {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IPV6 address Colon number is larger than 7, return ERROR");
        return VOS_ERR;
    }

    if (colonCount == AT_MAX_IPV6_STR_COLON_NUM) {
        /* 非压缩格式处理 */
        result = AT_PcscfIpv6StrToAddr(str, ipAddr, colonCount);
    } else {
        if (idxPos != 0xFF) {
            /* 压缩格式处理 */
            AT_Ipv6LenStrToAddrProcCompressed(str, colonCount, dotCount, strlength, idxPos);
            /* 映射IPV6地址格式 */
            result = AT_PcscfIpv6StrToAddr(str, ipAddr, AT_MAX_IPV6_STR_COLON_NUM);
        }
        /* 压缩IPV6地址中找不到两个相邻的冒号，格式错误 */
        else {
            AT_ERR_LOG(
                "AT_ParseAddrFromPcscfIpv6Addr: Can not find two consecutive colons in compressed IPV6 address , return ERROR");
            return VOS_ERR;
        }
    }

    return result;
}

VOS_UINT32 AT_CheckPcscfIpv6Addr(VOS_CHAR *ipv6Str, VOS_UINT32 *portExistFlg)
{
    VOS_CHAR *ipv6Start = VOS_NULL_PTR;
    VOS_CHAR *ipv6End   = VOS_NULL_PTR;

    if ((ipv6Str == VOS_NULL_PTR) || (portExistFlg == VOS_NULL_PTR)) {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    ipv6End   = VOS_StrStr(ipv6Str, "]");
    ipv6Start = VOS_StrStr(ipv6Str, "[");

    if ((ipv6End == VOS_NULL_PTR) && (ipv6Start == VOS_NULL_PTR)) {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, port not exists");
        *portExistFlg = VOS_FALSE;
        return VOS_OK;
    }

    if ((ipv6End != VOS_NULL_PTR) && (ipv6Start != VOS_NULL_PTR) && (ipv6End > ipv6Start)) {
        if ((ipv6End - ipv6Start) > TAF_MAX_IPV6_ADDR_COLON_STR_LEN) {
            AT_ERR_LOG("AT_CheckPcscfIpv6Addr: length of IPV6 addr in [] is larger than 39, return ERROR");
            return VOS_ERR;
        }

        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: Have both [ ] symbol in IPV6 addr");
        *portExistFlg = VOS_TRUE;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_CheckPcscfIpv6Addr: IPV6 addr format incorrect");
    return VOS_ERR;
}

VOS_UINT32 AT_PortAtoI(VOS_CHAR *pcString, VOS_UINT32 *value)
{
    VOS_CHAR  *tmp = VOS_NULL_PTR;
    VOS_UINT32 ret;

    if ((pcString == VOS_NULL_PTR) || (value == VOS_NULL_PTR)) {
        AT_ERR_LOG("AT_PortAtoI: pcString or pulValue is NULL, return ERROR");
        return VOS_ERR;
    }

    tmp = pcString;
    ret = 0;

    *value = 0;

    for (tmp = pcString; *tmp != '\0'; tmp++) {
        /* 非数字, 则返回失败 */
        if ((*tmp < '0') || (*tmp > '9')) {
            AT_ERR_LOG("AT_PortAtoI: Not all number type in pcString , return ERROR");
            return VOS_ERR;
        }

        ret = (ret * 10) + (*tmp - '0'); /* 乘10是为了将字符串转换成十进制数 */

        if (ret > IMS_PCSCF_ADDR_PORT_MAX) {
            AT_ERR_LOG("AT_PortAtoI: Port number is larger than 65535, return ERROR");
            return VOS_ERR;
        }
    }

    if ((ret > 0) && (ret <= IMS_PCSCF_ADDR_PORT_MAX)) {
        *value = (VOS_UINT32)ret;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_PortAtoI: return ERROR");
    return VOS_ERR;
}

VOS_UINT32 AT_ParsePortFromPcscfIpv6Addr(VOS_UINT8 *str, VOS_UINT8 *ipv6Addr, VOS_UINT32 ipv6BufLen,
                                         VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum)
{
    VOS_CHAR *ipv6Start = VOS_NULL_PTR;
    VOS_CHAR *ipv6End   = VOS_NULL_PTR;
    VOS_CHAR *ipv6Str   = VOS_NULL_PTR;
    VOS_CHAR *pcStrPort = VOS_NULL_PTR;
    errno_t   memResult;

    if ((str == VOS_NULL_PTR) || (ipv6Addr == VOS_NULL_PTR) || (portExistFlg == VOS_NULL_PTR) ||
        (portNum == VOS_NULL_PTR)) {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (AT_CheckPcscfIpv6Addr((VOS_CHAR *)str, portExistFlg) != VOS_OK) {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: AT_CheckPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* [ipv6]:port格式兼容处理，获取ipv6端口号 */
    ipv6Str   = (VOS_CHAR *)str;
    ipv6End   = VOS_StrStr(ipv6Str, "]");
    ipv6Start = VOS_StrStr(ipv6Str, "[");

    if (*portExistFlg == VOS_TRUE) {
        if ((ipv6End == VOS_NULL_PTR) || (ipv6Start == VOS_NULL_PTR)) {
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, return ERROR");
            return VOS_ERR;
        }

        /* 保存中括号内的IPV6地址信息，移除端口号 */
        if (ipv6End > (ipv6Start + 1)) {
            memResult = memcpy_s(ipv6Addr, ipv6BufLen, ipv6Start + 1, (VOS_UINT32)((ipv6End - ipv6Start) - 1));
            TAF_MEM_CHK_RTN_VAL(memResult, ipv6BufLen, (VOS_UINT32)((ipv6End - ipv6Start) - 1));
        }

        /* 记录分隔地址和端口的冒号地址 */
        pcStrPort = VOS_StrStr(ipv6End, ":");

        /* [ipv6]:port格式没有冒号，返回ERROR */
        if (pcStrPort == VOS_NULL_PTR) {
            *portExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon missed, return ERROR ");
            return VOS_ERR;
        }

        /* 冒号前面不是"]"，返回ERROR */
        if (pcStrPort != ipv6End + 1) {
            *portExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon location incorrect, return ERROR ");
            return VOS_ERR;
        }

        /* [ipv6]:port格式端口解析错误，返回ERROR */
        if (AT_PortAtoI(pcStrPort + 1, portNum) != VOS_OK) {
            *portExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port decode ERROR");
            return VOS_ERR;
        }

        *portExistFlg = VOS_TRUE;
    } else {
        /* 没有端口号，传入的字符串只包含IPV6地址 */
        memResult = memcpy_s(ipv6Addr, ipv6BufLen, str, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, ipv6BufLen, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    }

    return VOS_OK;
}

VOS_UINT32 AT_Ipv6PcscfDataToAddr(VOS_UINT8 *str, VOS_UINT8 *ipAddr, VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum

)
{
    VOS_UINT8 strTmp[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];

    (VOS_VOID)memset_s(strTmp, TAF_MAX_IPV6_ADDR_COLON_STR_LEN, 0, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);

    if ((str == VOS_NULL_PTR) || (ipAddr == VOS_NULL_PTR) || (portExistFlg == VOS_NULL_PTR) ||
        (portNum == VOS_NULL_PTR)) {
        AT_NORM_LOG("AT_Ipv6PcscfDataToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (AT_ParsePortFromPcscfIpv6Addr(str, strTmp, sizeof(strTmp), portExistFlg, portNum) != VOS_OK) {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParsePortFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* 将IPV6地址格式转换为大写 */
    VOS_StrToUpper((VOS_CHAR *)strTmp);

    if (AT_ParseAddrFromPcscfIpv6Addr(strTmp, sizeof(strTmp), ipAddr) != VOS_OK) {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParseAddrFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    return VOS_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetBatteryInfoPara(TAF_UINT8 indexNum)
{
    AT_ModemImsContext       *localBatteryInfo = VOS_NULL_PTR;
    AT_IMSA_BatteryInfoSetReq batteryInfoSetReq;
    VOS_UINT32                result;

    (VOS_VOID)memset_s(&batteryInfoSetReq, sizeof(batteryInfoSetReq), 0x00, sizeof(batteryInfoSetReq));
    localBatteryInfo = AT_GetModemImsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetBatteryInfoPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetBatteryInfoPara: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetBatteryInfoPara: para len is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    batteryInfoSetReq.batteryStatusInfo.batteryStatus = (AT_IMSA_BatteryStatusUint8)g_atParaList[0].paraValue;

    /* 给IMSA发送^BATTERYINFO设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_BATTERYINFO_SET_REQ,
                                    (VOS_UINT8 *)&(batteryInfoSetReq.batteryStatusInfo),
                                    (VOS_UINT32)sizeof(AT_IMSA_BatteryStatusInfo), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetBatteryInfoPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    localBatteryInfo->batteryInfo.tempBatteryInfo = batteryInfoSetReq.batteryStatusInfo.batteryStatus;

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_BATTERYINFO_SET;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 AT_SetCirepPara(TAF_UINT8 indexNum)
{
    AT_IMSA_CirepSetReq cirep;
    VOS_UINT32          result;

    (VOS_VOID)memset_s(&cirep, sizeof(cirep), 0x00, sizeof(AT_IMSA_CirepSetReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        cirep.report = g_atParaList[0].paraValue;

    } else {
        /* 如果参数为空，默认设置为不主动上报 */
        cirep.report = AT_IMSA_CIREP_REPORT_DISENABLE;
    }

    /* 给IMSA发送+CIREG设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_CIREP_SET_REQ,
                                    (VOS_UINT8 *)&cirep.report, sizeof(AT_IMSA_CirepReportUint32), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIREP_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCallEncryptPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                result;
    AT_IMSA_CallEncryptSetReq callEncrypt;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    callEncrypt.encrypt = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给IMSA发送^CALLENCRYPT设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_CALL_ENCRYPT_SET_REQ,
                                    &callEncrypt.encrypt, sizeof(VOS_UINT8), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CALLENCRYPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_ParseIpv6PcscfData(VOS_UINT32 *addrExistFlg, VOS_UINT8 *ipAddr, VOS_UINT32 *portExistFlg,
                                 VOS_UINT32 *portNum, VOS_UINT32 indexNum)
{
    /* IPV6地址长度超过限制 */
    if ((AT_PARA_MAX_LEN + 1) < g_atParaList[indexNum].paraLen) {
        *portExistFlg = VOS_FALSE;
        *addrExistFlg = VOS_FALSE;
        AT_ERR_LOG("AT_FillIpv6PcscfData: PCSCF IPV6 address length OUT OF RANGE");
        return VOS_ERR;
    }

    /* AT参数为空，代表地址不存在 */
    if (g_atParaList[indexNum].paraLen == 0) {
        AT_NORM_LOG("AT_ParseIpv6PcscfData: PCSCF IPV6 address is NULL");
        *portExistFlg = VOS_FALSE;
        *addrExistFlg = VOS_FALSE;
        return VOS_OK;
    }

    /* 解析IPV6地址和端口号 */
    if (AT_Ipv6PcscfDataToAddr(g_atParaList[indexNum].para, ipAddr, portExistFlg, portNum) != VOS_OK) {
        *addrExistFlg = VOS_FALSE;
        AT_ERR_LOG("AT_ParseIpv6PcscfData: PCSCF IPV6 address decode ERROR");
        return VOS_ERR;
    } else {
        AT_NORM_LOG("AT_ParseIpv6PcscfData: PCSCF IPV6 address decode SUCC");
        *addrExistFlg = VOS_TRUE;
        return VOS_OK;
    }
}

VOS_UINT32 AT_FillIpv6PcscfData(AT_IMSA_PcscfSetReq *pcscf)
{
    VOS_UINT32 result;
    VOS_UINT32 indexNum = AT_IMSPCSCF_IPV6_ADDRESS1_INDEX;
    VOS_UINT32 addrExistsFlg = VOS_FALSE;
    VOS_UINT32 portExistsFlg = VOS_FALSE;

    /*
     * 解析AT，组装发给IMSA的消息
     * ^ IMSPCSCF =<Source>,
     *             <IPV6Address1>,
     *             <IPV6Address2>，
     *             <IPV6Address3>,
     *             <IPV4Address1>,
     *             <IPV4Address2>,
     *             <IPV4Address3>
     */

    /* 解析<IPV6Address1>，将索引设置为AT命令的第2个参数 */

    result = AT_ParseIpv6PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv6Pcscf.primPcscfAddr, &portExistsFlg,
                                   &(pcscf->atPcscf.ipv6Pcscf.primPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv6Pcscf.opPrimPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv6Pcscf.opPrimPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv6PcscfData: Primary PCSCF IPV6 address decode ERROR");
        return result;
    }

    /* 解析<IPV6Address2>,将索引设置为AT命令的第3个参数 */
    indexNum      = AT_IMSPCSCF_IPV6_ADDRESS2_INDEX;
    addrExistsFlg = VOS_FALSE;
    portExistsFlg = VOS_FALSE;

    result = AT_ParseIpv6PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv6Pcscf.secPcscfAddr, &portExistsFlg,
                                   &(pcscf->atPcscf.ipv6Pcscf.secPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv6Pcscf.opSecPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv6Pcscf.opSecPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv6PcscfData: Secondary PCSCF IPV6 address decode ERROR");
        return result;
    }

    /* 解析<IPV6Address3>,将索引设置为AT命令的第4个参数 */
    indexNum      = AT_IMSPCSCF_IPV6_ADDRESS3_INDEX;
    addrExistsFlg = VOS_FALSE;
    portExistsFlg = VOS_FALSE;

    result = AT_ParseIpv6PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv6Pcscf.thiPcscfAddr, &portExistsFlg,
                                   &(pcscf->atPcscf.ipv6Pcscf.thiPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv6Pcscf.opThiPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv6Pcscf.opThiPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv6PcscfData: Third PCSCF IPV6 address decode ERROR");
        return result;
    }

    return result;
}

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
VOS_UINT32 AT_ParseIpv4PcscfData(VOS_UINT32 *addrExistFlg, VOS_UINT8 *ipAddr, VOS_UINT32 ipBufLen,
    VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum, VOS_UINT32 indexNum)
{
    VOS_CHAR  *pcPortStr = VOS_NULL_PTR;
    VOS_UINT8 *tmpIpAddr;
    VOS_UINT32 strLen = 0;
    errno_t    memResult;

    tmpIpAddr = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, AT_PARA_MAX_LEN + 1);

    if (tmpIpAddr == NULL) {
        AT_ERR_LOG("AT_ParseIpv4PcscfData: Fail to malloc mem.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(tmpIpAddr, AT_PARA_MAX_LEN + 1, 0, AT_PARA_MAX_LEN + 1);

    /* IPV4地址长度超过限制 */
    if (AT_PARA_MAX_LEN + 1 < g_atParaList[indexNum].paraLen) {
        AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address length OUT OF RANGE");
        VOS_MemFree(WUEPS_PID_AT, tmpIpAddr);
        return VOS_ERR;
    }

    /* 参数为空，代表此地址不存在 */
    if (g_atParaList[indexNum].paraLen == 0) {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address is NULL");
        *portExistFlg = VOS_FALSE;
        *addrExistFlg = VOS_FALSE;
        VOS_MemFree(WUEPS_PID_AT, tmpIpAddr);
        return VOS_OK;
    }

    pcPortStr = VOS_StrStr((VOS_CHAR *)(g_atParaList[indexNum].para), ":");

    /* 检查是否有端口号 */
    if (pcPortStr != VOS_NULL_PTR) {
        if (AT_PortAtoI(pcPortStr + 1, portNum) == VOS_OK) {
            AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 port num decode SUCC");
            *portExistFlg = VOS_TRUE;
            strLen        = (VOS_UINT32)(pcPortStr - (VOS_CHAR *)(g_atParaList[indexNum].para));
        } else { /* 解析端口号失败 */
            *portExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 port num decode ERROR");
            VOS_MemFree(WUEPS_PID_AT, tmpIpAddr);
            return VOS_ERR;
        }
    } else {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: No port in PCSCF IPV4 addr");
        *portExistFlg = VOS_FALSE;
        strLen        = VOS_StrLen((VOS_CHAR *)(g_atParaList[indexNum].para));
    }

    if (strLen > 0) {
        memResult = memcpy_s(tmpIpAddr, AT_PARA_MAX_LEN + 1, g_atParaList[indexNum].para,
                             AT_MIN(strLen, AT_PARA_MAX_LEN + 1));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, AT_MIN(strLen, AT_PARA_MAX_LEN + 1));
    }

    /* 解析IPV4地址 */
    if (AT_Ipv4AddrAtoi((VOS_CHAR *)tmpIpAddr, ipAddr, ipBufLen) != VOS_OK) {
        *addrExistFlg = VOS_FALSE;
        AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address decode ERROR");
        VOS_MemFree(WUEPS_PID_AT, tmpIpAddr);
        return VOS_ERR;
    } else {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address decode SUCC");
        *addrExistFlg = VOS_TRUE;
    }
    VOS_MemFree(WUEPS_PID_AT, tmpIpAddr);
    return VOS_OK;
}
#else
VOS_UINT32 AT_ParseIpv4PcscfData(VOS_UINT32 *addrExistFlg, VOS_UINT8 *ipAddr, VOS_UINT32 ipBufLen,
                                 VOS_UINT32 *portExistFlg, VOS_UINT32 *portNum, VOS_UINT32 indexNum)
{
    VOS_CHAR  *pcPortStr = VOS_NULL_PTR;
    VOS_UINT8  tmpIpAddr[AT_PARA_MAX_LEN + 1];
    VOS_UINT32 strLen = 0;
    errno_t    memResult;

    (VOS_VOID)memset_s(tmpIpAddr, AT_PARA_MAX_LEN + 1, 0, AT_PARA_MAX_LEN + 1);

    /* IPV4地址长度超过限制 */
    if ((AT_PARA_MAX_LEN + 1) < g_atParaList[indexNum].paraLen) {
        AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address length OUT OF RANGE");
        return VOS_ERR;
    }

    /* 参数为空，代表此地址不存在 */
    if (g_atParaList[indexNum].paraLen == 0) {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address is NULL");
        *portExistFlg = VOS_FALSE;
        *addrExistFlg = VOS_FALSE;
        return VOS_OK;
    }

    pcPortStr = VOS_StrStr((VOS_CHAR *)(g_atParaList[indexNum].para), ":");

    /* 检查是否有端口号 */
    if (pcPortStr != VOS_NULL_PTR) {
        if (AT_PortAtoI(pcPortStr + 1, portNum) == VOS_OK) {
            AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 port num decode SUCC");
            *portExistFlg = VOS_TRUE;
            strLen        = (VOS_UINT32)(pcPortStr - (VOS_CHAR *)(g_atParaList[indexNum].para));
        }
        /* 解析端口号失败 */
        else {
            *portExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 port num decode ERROR");
            return VOS_ERR;
        }
    } else {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: No port in PCSCF IPV4 addr");
        *portExistFlg = VOS_FALSE;
        strLen        = VOS_StrLen((VOS_CHAR *)(g_atParaList[indexNum].para));
    }

    if (strLen > 0) {
        memResult = memcpy_s(tmpIpAddr, AT_PARA_MAX_LEN + 1, g_atParaList[indexNum].para,
                             AT_MIN(strLen, AT_PARA_MAX_LEN + 1));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, AT_MIN(strLen, AT_PARA_MAX_LEN + 1));
    }

    /* 解析IPV4地址 */
    if (AT_Ipv4AddrAtoi((VOS_CHAR *)tmpIpAddr, ipAddr, ipBufLen) != VOS_OK) {
        *addrExistFlg = VOS_FALSE;
        AT_ERR_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address decode ERROR");
        return VOS_ERR;
    } else {
        AT_NORM_LOG("AT_ParseIpv4PcscfData: PCSCF IPV4 address decode SUCC");
        *addrExistFlg = VOS_TRUE;
    }

    return VOS_OK;
}
#endif

VOS_UINT32 AT_FillIpv4PcscfData(AT_IMSA_PcscfSetReq *pcscf)
{
    VOS_UINT32 result;
    VOS_UINT32 indexNum = AT_IMSPCSCF_IPV4_ADDRESS1_INDEX;
    VOS_UINT32 addrExistsFlg = VOS_FALSE;
    VOS_UINT32 portExistsFlg = VOS_FALSE;

    /*
     * 解析AT，组装发给IMSA的消息
     * ^ IMSPCSCF =<Source>,
     *             <IPV6Address1>,
     *             <IPV6Address2>，
     *             <IPV6Address3>,
     *             <IPV4Address1>,
     *             <IPV4Address2>,
     *             <IPV4Address3>
     */

    /* 解析Primary IPV4，将索引设置为AT命令的第5个参数 */

    result = AT_ParseIpv4PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv4Pcscf.primPcscfAddr, TAF_IPV4_ADDR_LEN,
                                   &portExistsFlg, &(pcscf->atPcscf.ipv4Pcscf.primPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv4Pcscf.opPrimPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv4Pcscf.opPrimPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv4PcscfData: Primary PCSCF IPV4 address Decode ERROR");
        return result;
    }

    /* 解析Secondary IPV4，将索引设置为AT命令的第6个参数 */
    indexNum      = AT_IMSPCSCF_IPV4_ADDRESS2_INDEX;
    addrExistsFlg = VOS_FALSE;
    portExistsFlg = VOS_FALSE;

    result = AT_ParseIpv4PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv4Pcscf.secPcscfAddr, TAF_IPV4_ADDR_LEN,
                                   &portExistsFlg, &(pcscf->atPcscf.ipv4Pcscf.secPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv4Pcscf.opSecPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv4Pcscf.opSecPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv4PcscfData: Secondary PCSCF IPV4 address Decode ERROR");
        return result;
    }

    /* 解析Third IPV4，将索引设置为AT命令的第7个参数 */
    indexNum      = AT_IMSPCSCF_IPV4_ADDRESS3_INDEX;
    addrExistsFlg = VOS_FALSE;
    portExistsFlg = VOS_FALSE;

    result = AT_ParseIpv4PcscfData(&addrExistsFlg, pcscf->atPcscf.ipv4Pcscf.thiPcscfAddr, TAF_IPV4_ADDR_LEN,
                                   &portExistsFlg, &(pcscf->atPcscf.ipv4Pcscf.thiPcscfSipPort), indexNum);

    pcscf->atPcscf.ipv4Pcscf.opThiPcscfAddr    = addrExistsFlg;
    pcscf->atPcscf.ipv4Pcscf.opThiPcscfSipPort = portExistsFlg;

    /* 解码错误直接返回 */
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_FillIpv4PcscfData: Third PCSCF IPV4 address Decode ERROR");
        return result;
    }

    return result;
}

VOS_UINT32 AT_FillDataToPcscf(AT_IMSA_PcscfSetReq *pcscf)
{
    VOS_UINT32 result;

    if (pcscf == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_FillDataToPcscf: pstPcscf is NULL, return ERROR");
        return VOS_ERR;
    }

    /* 没有填写<Source>或者超出范围 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[0].paraValue >= IMSA_AT_PCSCF_SRC_BUTT)) {
        AT_ERR_LOG("AT_FillDataToPcscf: No <source> parameter or out of range, return ERROR");
        return VOS_ERR;
    }

    /* <Source> */
    pcscf->atPcscf.src = g_atParaList[0].paraValue;

    result = AT_FillIpv6PcscfData(pcscf);

    /* IPV6和IPV4都解析成功才返回OK */
    if ((AT_FillIpv4PcscfData(pcscf) == VOS_OK) && (result == VOS_OK)) {
        return VOS_OK;
    }

    return VOS_ERR;
}

VOS_UINT32 AT_SetImsPcscfPara(VOS_UINT8 indexNum)
{
    AT_IMSA_PcscfSetReq pcscf;
    VOS_UINT32          result;

    AT_INFO_LOG("AT_SetImsPcscfPara Entered");

    (VOS_VOID)memset_s(&pcscf, sizeof(AT_IMSA_PcscfSetReq), 0x00, sizeof(AT_IMSA_PcscfSetReq));

    /* 不是设置命令则出错 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetImsPcscfPara: NOT SET CMD, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 解析AT，组装发给IMSA的消息
     * ^IMSPCSCF =<Source>,
     *            <IPV6Address1>,
     *            <IPV6Address2>，
     *            <IPV6Address3>,
     *            <IPV4Address1>,
     *            <IPV4Address2>,
     *            <IPV4Address3>
     */

    /* ^IMSPCSCF携带7个参数，否则出错 */
    if (g_atParaIndex != AT_IMSPCSCF_PARA_NUM) {
        AT_ERR_LOG("AT_SetImsPcscfPara: Para number incorrect, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 组装发给IMSA的消息 */
    if (AT_FillDataToPcscf(&pcscf) != VOS_OK) {
        AT_ERR_LOG("AT_SetImsPcscfPara: AT_FillDataToPcscf FAIL, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_PCSCF_SET_REQ, (VOS_UINT8 *)&pcscf.atPcscf,
                                    (VOS_UINT32)sizeof(IMSA_AT_Pcscf), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetImsPcscfPara: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PCSCF_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_FillImsPayloadTypePara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum)
{
    /* 初始化 */
    *valueValidFlg = VOS_FALSE;

    /* IMS要求PAYLOAD TYPE的参数范围只能使用动态解码范围0X60-0X7F */
    if (g_atParaList[indexNum].paraLen != 0) {
        if ((g_atParaList[indexNum].paraValue < AT_IMS_PAYLOAD_TYPE_RANGE_MIN) ||
            (g_atParaList[indexNum].paraValue > AT_IMS_PAYLOAD_TYPE_RANGE_MAX)) {
            AT_ERR_LOG("At_FillImsPayloadTypePara: IMS Payload type para OUT OF RANGE");
            return VOS_ERR;
        }

        *value         = g_atParaList[indexNum].paraValue;
        *valueValidFlg = VOS_TRUE;
    } else {
        AT_NORM_LOG("At_FillImsPayloadTypePara: IMS Payload type para length is 0");
        *valueValidFlg = VOS_FALSE;
    }

    return VOS_OK;
}

VOS_UINT32 At_FillImsAmrWbModePara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum)
{
    /* 初始化 */
    *valueValidFlg = VOS_FALSE;

    /* IMS要求AMR WB MODE的参数取值范围为0-8 */
    if (g_atParaList[indexNum].paraLen != 0) {
        if (g_atParaList[indexNum].paraValue > AT_IMS_AMR_WB_MODE_MAX) {
            AT_ERR_LOG("At_FillImsPayloadTypePara: IMS AMR WB MODE OUT OF RANGE");
            return VOS_ERR;
        }

        *value         = g_atParaList[indexNum].paraValue;
        *valueValidFlg = VOS_TRUE;
    } else {
        AT_NORM_LOG("At_FillImsPayloadTypePara: IMS AMR WB MODE para length is 0");
        *valueValidFlg = VOS_FALSE;
    }

    return VOS_OK;
}

VOS_UINT32 At_FillImsRtpPortPara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum)
{
    /* 初始化 */
    *valueValidFlg = VOS_FALSE;

    /* IMS要求RTP PORT的参数只能使用偶数且不能为0 */
    if (g_atParaList[indexNum].paraLen != 0) {
        if (((g_atParaList[indexNum].paraValue) % 2 != 0) || (g_atParaList[indexNum].paraValue == 0)) {
            AT_ERR_LOG("At_FillImsPayloadTypePara: IMS RTP Port para incorrect");
            return VOS_ERR;
        }

        *value         = g_atParaList[indexNum].paraValue;
        *valueValidFlg = VOS_TRUE;
    } else {
        AT_NORM_LOG("At_FillImsPayloadTypePara: IMS RTP Port para length is 0");
        *valueValidFlg = VOS_FALSE;
    }

    return VOS_OK;
}

VOS_UINT32 At_FillImsaNumericPara(VOS_UINT32 *value, VOS_UINT32 *valueValidFlg, VOS_UINT32 indexNum)
{
    /* 初始化 */
    *valueValidFlg = VOS_FALSE;

    if (g_atParaList[indexNum].paraLen != 0) {
        *value         = g_atParaList[indexNum].paraValue;
        *valueValidFlg = VOS_TRUE;
    } else {
        AT_NORM_LOG("At_FillImsaNumericPara: IMSA numeric para length is 0");
        *valueValidFlg = VOS_FALSE;
    }

    return VOS_OK;
}

VOS_UINT32 At_FillImsaStrPara(VOS_CHAR *str, VOS_UINT32 *strValidFlg, VOS_UINT32 maxLen, VOS_UINT32 indexNum)
{
    errno_t memResult;
    /* 初始化 */
    *strValidFlg = VOS_FALSE;

    if (g_atParaList[indexNum].paraLen != 0) {
        if (maxLen < g_atParaList[AT_DMDYN_PHONECONTEXT].paraLen) {
            AT_ERR_LOG("At_FillImsPayloadTypePara: IMS string para out of range");
            return VOS_ERR;
        }

        if (maxLen > 0) {
            memResult = memcpy_s(str, maxLen, (VOS_CHAR *)g_atParaList[indexNum].para,
                                 AT_MIN(maxLen, g_atParaList[indexNum].paraLen));
            TAF_MEM_CHK_RTN_VAL(memResult, maxLen, AT_MIN(maxLen, g_atParaList[indexNum].paraLen));
        }
        *strValidFlg = VOS_TRUE;
    } else {
        AT_NORM_LOG("At_FillImsaNumericPara: IMSA string para length is 0");
        *strValidFlg = VOS_FALSE;
    }

    return VOS_OK;
}

LOCAL VOS_VOID AT_ParseDmDynParaAmrWbOctetAcigned(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.amrWbOctetAcigned),
                                          &valueValidFlg, AT_DMDYN_AWR_WB_OCTET_ALIGNED_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opAmrWbOctetAcigned = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaAmrWbBandWidthEfficient(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.amrWbBandWidthEfficient),
                                          &valueValidFlg, AT_DMDYN_AWR_WB_BANDWIDTH_ECT_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opAmrWbBandWidthEfficient = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaAmrOctetAcigned(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.amrOctetAcigned),
                                          &valueValidFlg, AT_DMDYN_AWR_OCTET_ALIGNED_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opAmrOctetAcigned = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaAmrBandWidthEfficient(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.amrBandWidthEfficient),
                                          &valueValidFlg, AT_DMDYN_AWR_BANDWIDTH_ECT_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opAmrBandWidthEfficient = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaAmrWbMode(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsAmrWbModePara(&(dmdynSetReq->dmdyn.amrWbMode), &valueValidFlg, AT_DMDYN_AWR_WB_MODE_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opAmrWbMode = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaDtmfWb(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.dtmfWb), &valueValidFlg, AT_DMDYN_DTMF_WB_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opDtmfWb = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaDtmfNb(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsPayloadTypePara(&(dmdynSetReq->dmdyn.dtmfNb), &valueValidFlg, AT_DMDYN_DTMF_NB_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opDtmfNb = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaSpeechStart(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsRtpPortPara(&(dmdynSetReq->dmdyn.speechStart), &valueValidFlg, AT_DMDYN_SPEECH_START_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opSpeechStart = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaSpeechEnd(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsRtpPortPara(&(dmdynSetReq->dmdyn.speechEnd), &valueValidFlg, AT_DMDYN_SPEECH_END_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opSpeechEnd = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaVideoStart(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsRtpPortPara(&(dmdynSetReq->dmdyn.videoStart), &valueValidFlg, AT_DMDYN_VIDEO_START_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opVideoStart = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaVideoEnd(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsRtpPortPara(&(dmdynSetReq->dmdyn.videoEnd), &valueValidFlg, AT_DMDYN_VIDEO_END_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opVideoEnd = valueValidFlg;
}

LOCAL VOS_VOID AT_ParseDmDynParaRetryBaseTime(AT_IMSA_DmdynSetReq *dmdynSetReq, VOS_UINT32 *result)
{
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg = VOS_FALSE;

    tmpResult = At_FillImsaNumericPara(&(dmdynSetReq->dmdyn.retryBaseTime),
                                       &valueValidFlg, AT_DMDYN_REGRETRYBASETIME_INDEX);
    *result    = (*result) | tmpResult;

    dmdynSetReq->dmdyn.opRetryBaseTime = valueValidFlg;
}

VOS_UINT32 AT_SetDmDynPara(TAF_UINT8 indexNum)
{
    AT_IMSA_DmdynSetReq dmdynSetReq = {0};
    VOS_UINT32          result = AT_SUCCESS;
    VOS_UINT32          tmpResult;
    VOS_UINT32          valueValidFlg;

    AT_INFO_LOG("AT_SetDmDynPara Entered");

    /* 不是设置命令则出错 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetDmDynPara: NOT SET CMD, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ^DMDYN携带15个参数，否则出错 */
    if (g_atParaIndex != AT_DMDYN_PARA_NUM) {
        AT_ERR_LOG("AT_SetDmDynPara: Para number incorrect, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 解析AT，组装发给IMSA的消息
     * ^DMDYN=<AMR_WB_octet_aligned>,
     *        <AMR_WB_bandwidth_efficient>,
     *        <AMR_octet_aligned>,
     *        <AMR_bandwidth_efficient>,
     *        <AMR_WB_mode>,
     *        <DTMF_WB>,
     *        <DTMF_NB>,
     *        <Speech_start>,
     *        <Speech_end>,
     *        <Video_start>,
     *        <Video_end>,
     *        <RegRetryBaseTime>,
     *        <RegRetryMaxTime>,
     *        <PhoneContext>,
     *        <Public_user_identity>
     */

    /* <AMR_WB_octet_aligned> */
    AT_ParseDmDynParaAmrWbOctetAcigned(&dmdynSetReq, &result);

    /* <AMR_WB_bandwidth_efficient> */
    AT_ParseDmDynParaAmrWbBandWidthEfficient(&dmdynSetReq, &result);

    /* <AMR_octet_aligned> */
    AT_ParseDmDynParaAmrOctetAcigned(&dmdynSetReq, &result);

    /* <AMR_bandwidth_efficient> */
    AT_ParseDmDynParaAmrBandWidthEfficient(&dmdynSetReq, &result);

    /* <AMR_WB_mode> */
    AT_ParseDmDynParaAmrWbMode(&dmdynSetReq, &result);

    /* <DTMF_WB> */
    AT_ParseDmDynParaDtmfWb(&dmdynSetReq, &result);

    /* <DTMF_NB> */
    AT_ParseDmDynParaDtmfNb(&dmdynSetReq, &result);

    /* <Speech_start> */
    AT_ParseDmDynParaSpeechStart(&dmdynSetReq, &result);

    /* <Speech_end> */
    AT_ParseDmDynParaSpeechEnd(&dmdynSetReq, &result);

    /* <Video_start> */
    AT_ParseDmDynParaVideoStart(&dmdynSetReq, &result);

    /* <Video_end> */
    AT_ParseDmDynParaVideoEnd(&dmdynSetReq, &result);

    /* <RegRetryBaseTime> */
    AT_ParseDmDynParaRetryBaseTime(&dmdynSetReq, &result);

    /* <RegRetryMaxTime> */
    tmpResult = At_FillImsaNumericPara(&(dmdynSetReq.dmdyn.retryMaxTime),
                                       &valueValidFlg, AT_DMDYN_REGRETRYMAXTIME_INDEX);
    result    = result | tmpResult;

    dmdynSetReq.dmdyn.opRetryMaxTime = valueValidFlg;

    /* 因IMSA要求Base Time的时间不能超过Max Time */
    if (dmdynSetReq.dmdyn.retryBaseTime > dmdynSetReq.dmdyn.retryMaxTime) {
        AT_ERR_LOG("AT_SetDmDynPara: ulRetryBaseTime is larger than ulRetryMaxTime, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <PhoneContext> */
    tmpResult = At_FillImsaStrPara(dmdynSetReq.dmdyn.phoneContext, &valueValidFlg, AT_IMSA_PHONECONTEXT_MAX_LENGTH,
                                   AT_DMDYN_PHONECONTEXT_INDEX);
    result    = result | tmpResult;

    dmdynSetReq.dmdyn.opPhoneContext = valueValidFlg;

    /* <Public_user_identity> */
    tmpResult = At_FillImsaStrPara(dmdynSetReq.dmdyn.phoneContextImpu, &valueValidFlg, AT_IMSA_PUBLICEUSERID_MAX_LENGTH,
                                   AT_DMDYN_PUBLIC_USER_ID_INDEX);
    result    = result | tmpResult;

    dmdynSetReq.dmdyn.opPhoneContextImpu = valueValidFlg;

    if (result != VOS_OK) {
        AT_ERR_LOG("AT_SetDmDynPara: There have out of range para in setting command");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_DMDYN_SET_REQ, (VOS_UINT8 *)&dmdynSetReq.dmdyn,
                                    (VOS_UINT32)sizeof(AT_IMSA_Dmdyn), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetDmDynPara: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMDYN_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_FillImsTimerReqBitAndPara(VOS_UINT32 *value, VOS_UINT32 *bitOpValue, VOS_UINT32 indexNum)
{
    /* 初始化 */
    *bitOpValue = VOS_FALSE;

    /* 判断AT命令参数长度是否为0,定时器时长范围是0-128000ms */
    if (g_atParaList[indexNum].paraLen != 0) {
        if (g_atParaList[indexNum].paraValue > AT_IMS_TIMER_DATA_RANGE_MAX) {
            AT_ERR_LOG("AT_FillImsTimerReqBitAndPara: IMS Payload type para OUT OF RANGE");
            return VOS_ERR;
        }

        *value      = g_atParaList[indexNum].paraValue;
        *bitOpValue = VOS_TRUE;
    } else {
        AT_ERR_LOG("AT_FillImsTimerReqBitAndPara: IMS Timer para length is 0");
        *value      = 0;
        *bitOpValue = VOS_FALSE;
    }

    return VOS_OK;
}

VOS_UINT32 AT_FillImsTimerReqData(AT_IMSA_ImstimerSetReq *imsTimer)
{
    VOS_UINT32 bitOpValueFlg;

    /* 初始化 */
    (VOS_VOID)memset_s(imsTimer, sizeof(AT_IMSA_ImstimerSetReq), 0x00, sizeof(AT_IMSA_ImstimerSetReq));

    /* 填入AT命令参数<Timer_T1> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timer1Value),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_T1_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimer1Value = bitOpValueFlg;

    /* <Timer_T2> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timer2Value),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_T2_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimer2Value = bitOpValueFlg;

    /* <Timer_T4> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timer4Value),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_T4_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimer4Value = bitOpValueFlg;

    /* <Timer_TA> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerAValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TA_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerAValue = bitOpValueFlg;

    /* <Timer_TB> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerBValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TB_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerBValue = bitOpValueFlg;

    /* <Timer_TC> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerCValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TC_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerCValue = bitOpValueFlg;

    /* <Timer_TD> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerDValue),\
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TD_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerDValue = bitOpValueFlg;

    /* <Timer_TE> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerEValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TE_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerEValue = bitOpValueFlg;

    /* <Timer_TF> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerFValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TF_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerFValue = bitOpValueFlg;

    /* <Timer_TG> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerGValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TG_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerGValue = bitOpValueFlg;

    /* <Timer_TH> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerHValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TH_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerHValue = bitOpValueFlg;

    /* <Timer_TI> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerIValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TI_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerIValue = bitOpValueFlg;

    /* <Timer_TJ> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerJValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TJ_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerJValue = bitOpValueFlg;

    /* <Timer_TK> */
    if (AT_FillImsTimerReqBitAndPara(&(imsTimer->imsTimer.timerKValue),
                                     &(bitOpValueFlg), AT_DMTIMER_TIMER_TK_INDEX) != VOS_OK) {
        return VOS_ERR;
    }

    imsTimer->imsTimer.opTimerKValue = bitOpValueFlg;

    return VOS_OK;
}

VOS_UINT32 AT_SetImsTimerPara(VOS_UINT8 indexNum)
{
    AT_IMSA_ImstimerSetReq timer;

    VOS_UINT32 result;

    /* 不是设置命令则出错 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetImsTimerPara():ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 14个参数 */
    if (g_atParaIndex != AT_DMTIMER_PARA_NUM) {
        AT_ERR_LOG("AT_SetImsTimerPara():incorrect parameters");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 组装发给IMSA的消息 */
    if (AT_FillImsTimerReqData(&timer) != VOS_OK) {
        AT_ERR_LOG("AT_SetImsTimerPara():incorrect parameters");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_IMSTIMER_SET_REQ, (VOS_UINT8 *)&timer.imsTimer,
                                    (VOS_UINT32)sizeof(IMSA_AT_ImsTimer), PS_PID_IMSA);
    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetImsTimerPara():AT_FillAndSndAppReqMsg fail");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMTIMER_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetImsSmsPsiPara(VOS_UINT8 indexNum)
{
    AT_IMSA_SmspsiSetReq smsPsi;
    VOS_UINT32           result;
    errno_t              memResult;

    /* 清空 */
    (VOS_VOID)memset_s(&smsPsi, sizeof(AT_IMSA_SmspsiSetReq), 0x00, sizeof(AT_IMSA_SmspsiSetReq));

    /* 不是设置命令则出错 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetImsSmsPsiPara():WARNING:ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 判断参数个数 */
    if (g_atParaIndex != 1) {
        AT_ERR_LOG("AT_SetImsSmsPsiPara: incorrect parameter, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen > AT_IMSA_MAX_SMSPSI_LEN) {
        AT_ERR_LOG("AT_SetImsSmsPsiPara: <SMS_PSI> parameter over boundary , return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 组装发给IMSA的消息 */
    if (g_atParaList[0].paraLen != 0) {
        memResult = memcpy_s(smsPsi.smsPsi.smsPsi, sizeof(smsPsi.smsPsi.smsPsi), g_atParaList[0].para,
                             g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsPsi.smsPsi.smsPsi), g_atParaList[0].paraLen);
    }

    /* 给IMSA发送设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_SMSPSI_SET_REQ, (VOS_UINT8 *)&smsPsi.smsPsi,
                                    (VOS_UINT32)sizeof(IMSA_SMS_Psi), PS_PID_IMSA);
    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetImsSmsPsiPara():WARNING:AT_FillAndSndAppReqMsg fail");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSPSI_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetRoamImsServicePara(VOS_UINT8 indexNum)
{
    TAF_MMA_RoamImsSupportUint32 roamImsSupport = TAF_MMA_ROAM_IMS_BUTT;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if ((g_atParaList[0].paraLen != 1) || (g_atParaIndex != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 0) {
        roamImsSupport = TAF_MMA_ROAM_IMS_UNSUPPORT;
    } else {
        roamImsSupport = TAF_MMA_ROAM_IMS_SUPPORT;
    }

    if (TAF_MMA_SetRoamImsSupportReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, roamImsSupport) == VOS_TRUE) {
        /* 设置AT模块实体的状态为等待异步返回 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ROAM_IMS_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_SetRoamImsServicePara: TAF_MMA_SetRoamImsSupportReq fail.");

        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetVolteRegPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送^VOLTEREG查询请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_VOLTEREG_NTF, VOS_NULL_PTR, 0,
                                    PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetVolteRegPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_UINT32 AT_SetCiregPara(TAF_UINT8 indexNum)
{
    AT_IMSA_CiregSetReq cireg;
    VOS_UINT32          result;

    (VOS_VOID)memset_s(&cireg, sizeof(cireg), 0x00, sizeof(AT_IMSA_CiregSetReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        cireg.cireg = g_atParaList[0].paraValue;

    } else {
        /* 如果参数为空，默认设置为不主动上报 */
        cireg.cireg = AT_IMSA_IMS_REG_STATE_DISABLE_REPORT;
    }

    /* 给IMSA发送+CIREG设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_CIREG_SET_REQ,
                                    (VOS_UINT8 *)&cireg.cireg, sizeof(AT_IMSA_ImsRegStateReportUint32), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIREG_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

