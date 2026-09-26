/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "at_data_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_taf_agent_interface.h"
#include "at_ctx_packet.h"
#include "mn_comm_api.h"
#include "dms_port_i.h"
#include "at_mdrv_interface.h"
#include "taf_msg_chk_api.h"
#include "at_data_event_report.h"
#include "at_data_taf_rslt_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATAPROC_C
#define IPV4_ADDR_MAX_VALUE 255
#define AT_IPV4_ADDR_INDEX_1 0
#define AT_IPV4_ADDR_INDEX_2 1
#define AT_IPV4_ADDR_INDEX_3 2
#define AT_IPV4_ADDR_INDEX_4 3


#define AT_ITOA_HEX_RADIX 16
#define AT_ITOA_DEC_RADIX 10

#define AT_EVENT_INFO_SIZE 4
#define AT_BYTE_TO_BITS_LENS 8


#define AT_BYTE_LOCAL_IPV6_MAX_OFFSET 7 /* 本地IPV6的最大偏移量 */
#define IPV6_ADDR_PREFIX_MAX_VALUE 128

/* HiLink模式: 正常模式或网关模式 */
AT_HilinkModeUint8 g_hiLinkMode = AT_HILINK_NORMAL_MODE;

static const AT_PS_RmnetIfaceId g_atPsIfaceIdRmNetIdTab[] = {
    { PS_IFACE_ID_RMNET0, RNIC_DEV_ID_RMNET0, 0, 0 },   { PS_IFACE_ID_RMNET1, RNIC_DEV_ID_RMNET1, 0, 0 },
    { PS_IFACE_ID_RMNET2, RNIC_DEV_ID_RMNET2, 0, 0 },   { PS_IFACE_ID_RMNET3, RNIC_DEV_ID_RMNET3, 0, 0 },
    { PS_IFACE_ID_RMNET4, RNIC_DEV_ID_RMNET4, 0, 0 },   { PS_IFACE_ID_RMNET5, RNIC_DEV_ID_RMNET5, 0, 0 },
    { PS_IFACE_ID_RMNET6, RNIC_DEV_ID_RMNET6, 0, 0 },   { PS_IFACE_ID_RMNET7, RNIC_DEV_ID_RMNET7, 0, 0 },
    { PS_IFACE_ID_RMNET8, RNIC_DEV_ID_RMNET8, 0, 0 },   { PS_IFACE_ID_RMNET9, RNIC_DEV_ID_RMNET9, 0, 0 },
    { PS_IFACE_ID_RMNET10, RNIC_DEV_ID_RMNET10, 0, 0 }, { PS_IFACE_ID_RMNET11, RNIC_DEV_ID_RMNET11, 0, 0 },
    { PS_IFACE_ID_NDIS0, RNIC_DEV_ID_BUTT, 0, 0 },
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    { PS_IFACE_ID_NDIS1, RNIC_DEV_ID_BUTT, 0, 0 },
    { PS_IFACE_ID_NDIS2, RNIC_DEV_ID_BUTT, 0, 0 },
    { PS_IFACE_ID_NDIS3, RNIC_DEV_ID_BUTT, 0, 0 }
#endif
};

static const AT_PS_EvtFuncTbl g_atIfaceEvtFuncTbl[] = {
    { ID_EVT_TAF_IFACE_UP_CNF, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_UpCnf) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtIfaceUpCnf },
    { ID_EVT_TAF_IFACE_DOWN_CNF, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_DownCnf) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtIfaceDownCnf },
    { ID_EVT_TAF_IFACE_STATUS_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_StatusInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtIfaceStatusInd },
    { ID_EVT_TAF_IFACE_DATA_CHANNEL_STATE_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_DataChannelStateInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtDataChannelStateInd },
    { ID_EVT_TAF_IFACE_USBNET_OPER_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_UsbnetOperInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtUsbNetOperInd },
    { ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_GetDynamicParaCnf) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtDyamicParaCnf },
    { ID_EVT_TAF_IFACE_RAB_INFO_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_RabInfoInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtRabInfoInd },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_EVT_TAF_IFACE_IP_CHANGE_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_IpChangeInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtIpChangeInd },
#endif
    { ID_EVT_TAF_IFACE_IPV6_PDN_INFO_IND, sizeof(TAF_PS_Evt) + sizeof(TAF_IFACE_Ipv6PdnInfoInd) - 4,
      VOS_NULL_PTR, AT_RcvTafIfaceEvtIPv6PdnInfoInd },
};

/*
 * 3 函数、变量声明
 */

/*
 * 功能描述: 从字符串地址中获取IPV4整数类型的地址信息
 */
LOCAL VOS_UINT32 AT_GetIntegerIpv4AddrInfo(const VOS_CHAR *pcString, VOS_UINT32 *valTmp,
    VOS_UINT32 *dotNum, VOS_UINT8 *addr)
{
    VOS_UINT32 strLen;
    VOS_UINT32 i = 0;
    VOS_UINT32 numLen = 0;

    strLen = VOS_StrLen(pcString);

    if (strLen > VOS_StrLen("255.255.255.255")) {
        AT_NORM_LOG("AT_GetIntegerIpv4AddrInfo: PCSCF IPV4 address length out of range");
        return VOS_ERR;
    }

    for (i = 0; i < strLen; i++) {
        if ((pcString[i] >= '0') && (pcString[i] <= '9')) {
            (*valTmp) = ((*valTmp) * 10) + (pcString[i] - '0'); /* 乘10是为了将字符串转换成十进制数 */

            numLen++;
            continue;
        } else if (pcString[i] == '.') {
            if ((numLen == 0) || (numLen > 3)) { /* '.'分割的IPV6地址数字格式最大为255，不超过3个宽度 */
                AT_NORM_LOG("AT_GetIntegerIpv4AddrInfo: the number between dots is out of range");
                return VOS_ERR;
            }

            if ((*valTmp) > IPV4_ADDR_MAX_VALUE) {
                AT_NORM_LOG("AT_GetIntegerIpv4AddrInfo: the number is larger than 255");
                return VOS_ERR;
            }

            addr[(*dotNum)] = (VOS_UINT8)(*valTmp);

            (*valTmp) = 0;
            numLen = 0;

            /* 统计'.'的个数 */
            (*dotNum)++;
            if ((*dotNum) >= TAF_IPV4_ADDR_LEN) {
                AT_NORM_LOG("AT_GetIntegerIpv4AddrInfo: dot num is more than 3, return ERROR");
                return VOS_ERR;
            }
            continue;
        } else {
            AT_NORM_LOG("AT_GetIntegerIpv4AddrInfo: character not number nor dot, return ERROR");
            /* 其他值直接返回失败 */
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_Ipv4AddrAtoi(VOS_CHAR *pcString, VOS_UINT8 *number, VOS_UINT32 numBufLen)
{
    errno_t    memResult;
    VOS_UINT32 dotNum  = 0;
    VOS_UINT32 valTmp  = 0;
    VOS_UINT8  addr[TAF_IPV4_ADDR_LEN] = {0};

    if ((pcString == VOS_NULL_PTR) || (number == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    if (numBufLen < TAF_IPV4_ADDR_LEN) {
        return VOS_ERR;
    }

    if (AT_GetIntegerIpv4AddrInfo(pcString, &valTmp, &dotNum, addr) == VOS_ERR) {
        return VOS_ERR;
    }

    /* 如果不是3个'.'则返回失败 */
    if (dotNum != 3) {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: dot number is not 3");
        return VOS_ERR;
    }

    /* 检查最后地址域的取值 */
    if (valTmp > IPV4_ADDR_MAX_VALUE) {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: last number is larger than 255");
        return VOS_ERR;
    }

    addr[dotNum] = (VOS_UINT8)valTmp;

    memResult = memcpy_s(number, numBufLen, addr, sizeof(addr));
    TAF_MEM_CHK_RTN_VAL(memResult, numBufLen, sizeof(addr));

    return VOS_OK;
}

VOS_UINT32 AT_Ipv4AddrItoa(VOS_CHAR *pcString, VOS_UINT32 strLen, VOS_UINT8 *number)
{
    VOS_INT printResult;

    if ((pcString == VOS_NULL_PTR) || (number == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    if (strLen < TAF_MAX_IPV4_ADDR_STR_LEN) {
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(pcString, strLen, 0x00, strLen);

    printResult = snprintf_s(pcString, strLen, strLen - 1, "%d.%d.%d.%d", number[AT_IPV4_ADDR_INDEX_1],
                             number[AT_IPV4_ADDR_INDEX_2], number[AT_IPV4_ADDR_INDEX_3], number[AT_IPV4_ADDR_INDEX_4]);
    TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(printResult, strLen, strLen - 1);
    return VOS_OK;
}

VOS_UINT32 AT_Ipv4Addr2Str(VOS_CHAR *pcString, VOS_UINT32 strBuflen, VOS_UINT8 *number, VOS_UINT32 addrNumCnt)
{
    errno_t memResult;
    VOS_INT printResult;

    if ((pcString == VOS_NULL_PTR) || (number == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    if ((strBuflen < TAF_MAX_IPV4_ADDR_STR_LEN) || (addrNumCnt < TAF_IPV4_ADDR_LEN)) {
        return VOS_ERR;
    }

    memResult = memset_s(pcString, strBuflen, 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, strBuflen, TAF_MAX_IPV4_ADDR_STR_LEN);

    /* 如果输入地址为空，则返回空字符串 */
    if ((number[AT_IPV4_ADDR_INDEX_1] == 0) && (number[AT_IPV4_ADDR_INDEX_2] == 0) &&
        (number[AT_IPV4_ADDR_INDEX_3] == 0) && (number[AT_IPV4_ADDR_INDEX_4] == 0)) {
        return VOS_OK;
    }

    printResult = snprintf_s(pcString, strBuflen, strBuflen - 1, "%d.%d.%d.%d", number[AT_IPV4_ADDR_INDEX_1],
                             number[AT_IPV4_ADDR_INDEX_2], number[AT_IPV4_ADDR_INDEX_3], number[AT_IPV4_ADDR_INDEX_4]);
    TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(printResult, strBuflen, strBuflen - 1);

    return VOS_OK;
}

VOS_UINT8 AT_CalcIpv6PrefixLength(VOS_UINT8 *localIpv6Mask, VOS_UINT32 ipv6MaskLen)
{
    VOS_UINT32 i          = 0;
    VOS_UINT32 j          = 0;
    VOS_UINT8  maskLength = 0;

    if (localIpv6Mask == VOS_NULL_PTR) {
        return maskLength;
    }

    for (i = 0; i < TAF_MIN(AT_IPV6_STR_MAX_TOKENS, ipv6MaskLen); i++) {
        if (*(localIpv6Mask + i) == 0xFF) {
            maskLength = maskLength + AT_BYTE_TO_BITS_LENS;
        } else {
            for (j = 0; j < AT_BYTE_TO_BITS_LENS; j++) {
                if (((*(localIpv6Mask + i)) & ((VOS_UINT32)1 << (AT_BYTE_LOCAL_IPV6_MAX_OFFSET - j))) != 0) {
                    maskLength++;
                } else {
                    break;
                }
            }
            break;
        }
    }

    return maskLength;
}

VOS_UINT32 AT_FindIpv6AddrZeroFieldsToBeCompressed(VOS_UINT8 *zeroFieldStart, VOS_UINT8 *zeroFieldCount,
                                                   VOS_UINT16 ausAddrValue[], VOS_UINT8 tokensNum)
{
    VOS_UINT8 start;
    VOS_UINT8 count;
    VOS_UINT8 i;

    start = 0xFF;
    count = 0;

    for (i = 0; i < tokensNum - 1; i++) {
        if ((ausAddrValue[i] == 0x0000) && (ausAddrValue[i + 1] == 0x0000)) {
            /* 记录数值连续为0的IP地址段起始位置 */
            if (start == 0xFF) {
                start = i;
            }

            /* 更新数值连续为0的IP地址段个数 */
            count++;
        } else {
            /* 更新待压缩的IP地址段位置, 以及IP地址段个数 */
            if (start != 0xFF) {
                if (count > *zeroFieldCount) {
                    *zeroFieldStart = start;
                    *zeroFieldCount = count;
                }

                start = 0xFF;
                count = 0;
            }
        }
    }

    /*
     * 数值连续为0的IP地址段在结尾时, 需要更新一次待压缩的IP地址段位置,
     * 以及IP地址段个数
     */
    if (start != 0xFF) {
        if (count > *zeroFieldCount) {
            *zeroFieldStart = start;
            *zeroFieldCount = count;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(VOS_UINT8 aucAddrStr[], VOS_UINT8 aucIpAddr[], VOS_UINT8 tokensNum)
{
    VOS_UINT8 *buffer = VOS_NULL_PTR;
    VOS_UINT16 addrValue[TAF_IPV6_STR_RFC2373_TOKENS]; /* TAF_IPV6_STR_RFC2373_TOKENS]; */
    VOS_UINT16 addrNum;
    VOS_UINT8  delimiter;
    VOS_UINT8  radix;
    VOS_UINT8  zeroFieldStart;
    VOS_UINT8  zeroFieldCount;
    VOS_UINT32 i;

    (VOS_VOID)memset_s(addrValue, sizeof(addrValue), 0x00, sizeof(addrValue));

    buffer         = aucAddrStr;
    delimiter      = TAF_IPV6_STR_DELIMITER;
    radix          = AT_IPV6_STR_HEX_RADIX;
    zeroFieldStart = 0xFF;
    zeroFieldCount = 0;

    /* 根据IP字符串格式表达类型, 获取分段的IP地址数值 */
    for (i = 0; i < tokensNum; i++) {
        addrNum = *aucIpAddr++;

        addrNum <<= 8;
        addrNum |= *aucIpAddr++;

        addrValue[i] = addrNum;
    }

    /* 找出需要使用"::"表示的IP地址段的起始位置  */
    AT_FindIpv6AddrZeroFieldsToBeCompressed(&zeroFieldStart, &zeroFieldCount, addrValue, tokensNum);

    /* 遍历IP地址分段, 创建有分隔符标记的IP地址字符串 */
    for (i = 0; i < tokensNum; i++) {
        if (zeroFieldStart == i) {
            *buffer++ = delimiter;

            i += zeroFieldCount;

            /* 如果已到IP地址分段的最后一段, 需要补充分隔符 */
            if ((tokensNum - 1) == i) {
                *buffer++ = delimiter;
            }
        } else {
            /* 如果是IP地址分段的第一段, 不需要补充分隔符 */
            if (i != 0) {
                *buffer++ = delimiter;
            }
            buffer = (VOS_UINT8 *)AT_Itoa(addrValue[i], (VOS_CHAR *)buffer, radix,
                                          (TAF_MAX_IPV6_ADDR_COLON_STR_LEN - (VOS_UINT32)(aucAddrStr - buffer)));
        }
    }

    /* 补充字符串结束符 */
    if (aucAddrStr != buffer) {
        *buffer = '\0';
    }

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_MacAddrAtoi(const VOS_CHAR *inputStr, VOS_UINT8 *addrValue, VOS_UINT32 numBufLen)
{
    VOS_UINT32 numLen = 0;
    VOS_UINT32 dotNum = 0;
    VOS_UINT32 valTmp = 0;
    VOS_UINT32 strLen;
    VOS_UINT32 i;

    if ((inputStr == VOS_NULL_PTR) || (addrValue == VOS_NULL_PTR) || (numBufLen < PS_MAC_ADDR_LEN)) {
        return VOS_ERR;
    }

    strLen = VOS_StrLen(inputStr);
    if (strLen != VOS_StrLen("ff:ff:ff:ff:ff:ff")) {
        AT_NORM_LOG("AT_MacAddrAtoi: mac address length out of range");
        return VOS_ERR;
    }

    for (i = 0; i < strLen; i++) {
        VOS_CHAR macCharacter = AT_UPCASE(inputStr[i]); /* 统一转换成大写字母 */
        if ((macCharacter >= '0') && (macCharacter <= '9')) {
            valTmp = (valTmp * 16) + (macCharacter - '0');

            numLen++;
        } else if ((macCharacter >= 'A') && (macCharacter <= 'F')) {
            valTmp = (valTmp * 16) + (macCharacter - 'A' + 0xA);

            numLen++;
        } else if (macCharacter == ':') {
            if (numLen != 2) {
                AT_NORM_LOG("AT_MacAddrAtoi: the number between dots is out of range");
                return VOS_ERR;
            }

            addrValue[dotNum++] = (VOS_UINT8)valTmp;
            valTmp = 0;
            numLen = 0;
        } else {
            AT_NORM_LOG("AT_MacAddrAtoi: character not number nor dot, return ERROR");
            /* 其他值直接返回失败 */
            return VOS_ERR;
        }
    }

    /* 如果不是5个':'则返回失败 */
    if (dotNum != (PS_MAC_ADDR_LEN - 1)) {
        AT_NORM_LOG("AT_MacAddrAtoi: dot number is not 5");
        return VOS_ERR;
    }

    addrValue[dotNum] = (VOS_UINT8)valTmp;

    return VOS_OK;
}

VOS_UINT32 AT_Ipv4AddrAndMaskAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *ipv4Addr, VOS_UINT8 *ipv4Mask)
{
    /* 示例: "10.140.23.144.255.255.255.0"，前4个是IPV4地址，后4个为掩码 */
    VOS_UINT32 count = AT_CountDigit(inputStr, len, '.', 7); /* 用来获取IPV4地址中'.'第7次出现的地址 */
    if ((count == 0) || (inputStr[count] == 0)) {
        return VOS_ERR;
    }

    count = AT_CountDigit(inputStr, len, '.', 4); /* 用来获取IPV4地址中'.'第4次出现的地址 */

    if ((len - count) > TAF_MAX_IPV4_ADDR_STR_LEN) {
        return VOS_ERR;
    }

    if (AT_Ipv4AddrAtoi((VOS_CHAR *)&(inputStr[count]), ipv4Mask, TAF_IPV4_ADDR_LEN) != VOS_OK) {
        return VOS_ERR;
    }

    if ((count - 1) > TAF_MAX_IPV4_ADDR_STR_LEN) {
        return VOS_ERR;
    }

    inputStr[count - 1] = '\0';

    if (AT_Ipv4AddrAtoi((VOS_CHAR*)inputStr, ipv4Addr, TAF_IPV4_ADDR_LEN) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_Ipv6AddrAndPrefixLenAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *ipv6AddrValue,
    VOS_UINT8 *prefixLen)
{
    /* ipv6 addr and prefix len格式示例: 12.123.0.25.5.78.78.123.82.7.77.88.78.78.45.1/12 */
    /* 先获取prefix len */
    VOS_UINT32 valTmp = 0;
    VOS_UINT32 prefixPos = 0;
    VOS_UINT32 existFlg = 0;
    VOS_UINT32 i;

    /* 先计算prefix len位置 */
    for (i = 0; i < len; i++) {
        if (inputStr[i] == '/') {
            prefixPos = i;
            existFlg = VOS_TRUE;
            break;
        }
    }

    if (existFlg != VOS_TRUE) {
        return VOS_ERR;
    }

    if (prefixPos == len - 1) {
        return VOS_ERR;
    }

    /* 计算前缀长度 */
    for (i = (prefixPos + 1); i < len; i++) {
        if ((inputStr[i] >= '0') && (inputStr[i] <= '9')) {
            valTmp = (valTmp * 10) + (inputStr[i] - '0');
            if (valTmp > IPV6_ADDR_PREFIX_MAX_VALUE) {
                return VOS_ERR;
            }
        } else {
            /* 其他值直接返回失败 */
            return VOS_ERR;
        }
    }
    *prefixLen = (VOS_UINT8)valTmp;

    inputStr[prefixPos] = '\0';

    if (AT_Ipv6AddrAtoi((VOS_CHAR*)inputStr, ipv6AddrValue, TAF_IPV6_ADDR_LEN) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_PortRangeAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT16 *lPort, VOS_UINT16 *hPort)
{
    VOS_UINT32 low = 0;
    VOS_UINT32 high = 0;

    VOS_UINT32 count = AT_CountDigit(inputStr, len, '.', 1);
    if ((count == 0) || (inputStr[count] == 0)) {
        return VOS_ERR;
    }

    if (atAuc2ul(inputStr, (VOS_UINT16)(count - 1), &low) == AT_FAILURE) {
        return VOS_ERR;
    }

    if (atAuc2ul(&inputStr[count], (VOS_UINT16)(len - count), &high) == AT_FAILURE) {
        return VOS_ERR;
    }

    if ((low > AT_LOW_DEST_PORT_MAX_VALUE_RANGE) || (high > AT_HIGH_DEST_PORT_MAX_VALUE_RANGE) || (low > high)) {
        return VOS_ERR;
    }

    *lPort = (VOS_UINT16)low;
    *hPort = (VOS_UINT16)high;

    return VOS_OK;
}

VOS_UINT32 AT_TosAndMaskAtoi(VOS_UINT8 *inputStr, VOS_UINT16 len, VOS_UINT8 *tos, VOS_UINT8 *mask)
{
    VOS_UINT32 first = 0;
    VOS_UINT32 second = 0;

    VOS_UINT32 count = AT_CountDigit(inputStr, len, '.', 1);
    if ((count == 0) || (inputStr[count] == 0)) {
        return VOS_ERR;
    }

    if (atAuc2ul(inputStr, (VOS_UINT16)(count - 1), &first) == AT_FAILURE) {
        return VOS_ERR;
    }

    if (atAuc2ul(&inputStr[count], (VOS_UINT16)(len - count), &second) == AT_FAILURE) {
        return VOS_ERR;
    }

    if ((first > AT_TYPE_OF_SERVICE_MAX_VALUE_RANGE) || (second > AT_TYPE_OF_SERVICE_MASK_MAX_VALUE_RANGE)) {
        return VOS_ERR;
    }

    *tos = (VOS_UINT8)first;
    *mask = (VOS_UINT8)second;

    return VOS_OK;
}
#endif

VOS_UINT64 AT_AtoI(VOS_CHAR *string, VOS_UINT32 stringLen)
{
    VOS_CHAR  *pcTmp = VOS_NULL_PTR;
    VOS_UINT32 chkLen = 0;
    VOS_UINT64 ret = 0;

    for (pcTmp = string; (*pcTmp != '\0') && (chkLen < stringLen); pcTmp++, chkLen++) {
        /* 非数字,则不处理 */
        if ((*pcTmp < '0') || (*pcTmp > '9')) {
            continue;
        }

        ret = (ret * 10) + (*pcTmp - '0');  /* 乘10是为了将字符串转换成十进制数 */
    }

    return ret;
}

VOS_CHAR* AT_Itoa(VOS_UINT16 value, VOS_CHAR *pcStr, VOS_UINT16 radix, VOS_UINT32 length)
{
    VOS_INT32 bufLen = 0;
    if (radix == AT_ITOA_HEX_RADIX) {
        if (length > 0) {
            bufLen = snprintf_s(pcStr, length, length - 1, "%x", value);
        }
    } else if (radix == AT_ITOA_DEC_RADIX) {
        if (length > 0) {
            bufLen = snprintf_s(pcStr, length, length - 1, "%d", value);
        }
    } else {
    }

    TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, length, length - 1);

    if (bufLen > 0) {
        pcStr += bufLen;
    }
    return pcStr;
}

VOS_INT32 AT_AtoInt(VOS_CHAR *string, VOS_UINT32 stringLen, VOS_INT32 *out)
{
    VOS_CHAR *pcTmp = VOS_NULL_PTR;
    VOS_INT32 flag = 0; /* negative flag */
    VOS_UINT32 chkLen = 0;

    pcTmp = string;

    if (*pcTmp == '-') {
        flag = VOS_TRUE;
        pcTmp++;
    }

    for (; (*pcTmp != '\0') && (chkLen < stringLen); pcTmp++, chkLen++) {
        /* 非数字, 直接返回错误 */
        if ((*pcTmp < '0') || (*pcTmp > '9')) {
            return VOS_ERR;
        }

        *out = (*out * 10) + (*pcTmp - '0'); /* 乘10是为了将字符串转换成十进制数 */
    }

    if (flag == VOS_TRUE) {
        *out = (0 - (*out));
    }

    return VOS_OK;
}

VOS_VOID AT_SendRelPppReq(VOS_UINT16 pppId, PPP_AtCtrlOperTypeUint32 operType)
{
    PPP_RcvAtCtrlOperEvent(pppId, operType);
    PPP_RcvAtCtrlOperEvent(pppId, PPP_AT_CTRL_HDLC_DISABLE);
}

VOS_UINT8 AT_IsPppUser(VOS_UINT8 indexNum)
{
    if ((g_atClientTab[indexNum].userType == AT_MODEM_USER) ||
        (g_atClientTab[indexNum].userType == AT_HSUART_USER) ||
        (g_atClientTab[indexNum].userType == AT_UART_USER)) {
        return VOS_TRUE;
    }

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if (AT_CheckCmuxUser(indexNum) == VOS_TRUE) {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}

MODULE_EXPORTED VOS_UINT32 At_RcvTeConfigInfoReq(VOS_UINT16 pppId, AT_PPP_ReqConfigInfo *pppReqConfigInfo)
{
    if (pppId >= AT_MAX_CLIENT_NUM) {
        AT_WARN_LOG("At_RcvTeConfigInfoReq usPppId Wrong");
        return AT_FAILURE;
    }

    if (pppReqConfigInfo == TAF_NULL_PTR) {
        AT_WARN_LOG("At_RcvTeConfigInfoReq pPppReqConfigInfo NULL");
        return AT_FAILURE;
    }

    if (g_atClientTab[g_atPppIndexTab[pppId]].cmdCurrentOpt != AT_CMD_D_PPP_CALL_SET) {
        AT_WARN_LOG("At_RcvTeConfigInfoReq NOT AT_CMD_D_PPP_CALL_SET");
        return AT_FAILURE;
    }

    if (TAF_PS_PPP_DIAL_UP(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[g_atPppIndexTab[pppId]].clientId), 0,
                           g_atClientTab[g_atPppIndexTab[pppId]].cid,
                           (TAF_PPP_ReqConfigInfo *)pppReqConfigInfo) == VOS_OK) {
        /* 开定时器 */
        if (At_StartTimer(AT_ACT_PDP_TIME, g_atPppIndexTab[pppId]) != AT_SUCCESS) {
            AT_ERR_LOG("At_RcvTeConfigInfoReq:ERROR:Start Timer");
            return AT_FAILURE;
        }

        /* 设置当前操作类型 */
        g_atClientTab[g_atPppIndexTab[pppId]].cmdCurrentOpt = AT_CMD_PPP_ORG_SET;

        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

MODULE_EXPORTED VOS_UINT32 At_RcvPppReleaseInd(VOS_UINT16 pppId)
{
    TAF_UINT8          eventInfo[AT_EVENT_INFO_SIZE];
    AT_PPP_ReleaseInd *msg = VOS_NULL_PTR;
    VOS_UINT32         length;

    if (pppId >= AT_MAX_CLIENT_NUM) {
        AT_WARN_LOG("At_RcvPppReleaseInd usPppId Wrong");
        return AT_FAILURE;
    }

    /* EVENT- At_RcvPppReleaseInd:usPppId / g_atPppIndexTab[usPppId] */
    eventInfo[AT_EVENT_INFO_ARRAY_INDEX_0] = (TAF_UINT8)(pppId >> 8);
    eventInfo[AT_EVENT_INFO_ARRAY_INDEX_1] = (TAF_UINT8)pppId;
    eventInfo[AT_EVENT_INFO_ARRAY_INDEX_2] = g_atPppIndexTab[pppId];
    AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_RELEASE_PPP, eventInfo, (VOS_UINT32)sizeof(eventInfo));

    /* 向AT模块发送AT_PPP_RELEASE_IND_MSG */
    length = sizeof(AT_PPP_ReleaseInd) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    msg = (AT_PPP_ReleaseInd *)TAF_AllocMsgWithoutHeaderLen(PS_PID_APP_PPP, length); /*lint !e830*/
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        /* 打印出错信息---申请消息包失败 */
        AT_ERR_LOG("At_RcvPppReleaseInd:ERROR:Allocates a message packet for AT_PPP_RELEASE_IND_MSG_STRU msg FAIL!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, PS_PID_APP_PPP, WUEPS_PID_AT, length);

    msg->msgHeader.msgName = AT_PPP_RELEASE_IND_MSG;
    /* 填写消息体 */
    msg->clientId = g_atPppIndexTab[pppId];

    /* 发送该消息 */
    if (TAF_TraceAndSendMsg(PS_PID_APP_PPP, msg) != VOS_OK) {
        /* 打印警告信息---发送消息失败 */
        AT_WARN_LOG("At_RcvPppReleaseInd:WARNING:SEND AT_PPP_RELEASE_IND_MSG msg FAIL!");
        return AT_FAILURE;
    } else {
        /* 打印流程信息---发送了消息 */
        AT_WARN_LOG("At_RcvPppReleaseInd:NORMAL:SEND AT_PPP_RELEASE_IND_MSG Msg");
        return AT_SUCCESS;
    }
}

/*
 * Description: 由已知Rab ID获取PPP ID
 *  1.Date: 2008-03-05
 *    Modification: Created function
 */
TAF_UINT32 At_PsRab2PppId(TAF_UINT8 exRabId, TAF_UINT16 *pppId)
{
    TAF_UINT16 pppIdLoop;
    TAF_UINT8  indexNum;

    if (pppId == VOS_NULL_PTR) {
        AT_WARN_LOG("At_PsRab2PppId, pusPppId NULL");
        return TAF_FAILURE;
    }

    /* 通过PPP ID方向查找，效率会比较高 */
    for (pppIdLoop = 1; pppIdLoop <= PPP_MAX_ID_NUM; pppIdLoop++) {
        indexNum = g_atPppIndexTab[pppIdLoop];

        if (indexNum >= AT_MAX_CLIENT_NUM) {
            continue;
        }

        if (g_atClientTab[indexNum].used != AT_CLIENT_USED) {
            continue;
        }

        if (DMS_PORT_IsPppPacketTransMode(g_atClientTab[indexNum].portNo) == VOS_FALSE) {
            continue;
        }

        if (g_atClientTab[indexNum].exPsRabId == exRabId) {
            *pppId = pppIdLoop; /* 返回结果 */
            return TAF_SUCCESS;
        }
    }

    AT_LOG1("AT, At_PsRab2PppId, WARNING, Get PppId from Rab <1> Fail", exRabId);

    return TAF_FAILURE;
} /* At_PsRab2PppId */

/*
 * Description: 由已知PPP ID获取Rab ID
 *  1.Date: 2008-03-05
 *    Modification: Created function
 */
TAF_UINT32 At_PppId2PsRab(TAF_UINT16 pppId, TAF_UINT8 *exRabId)
{
    TAF_UINT8 indexNum;

    if ((pppId < 1) || (pppId > PPP_MAX_ID_NUM)) {
        TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_WARNING, "AT, At_PppId2PsRab, WARNING, PppId <1> Wrong", pppId);
        return TAF_FAILURE;
    }

    if (exRabId == TAF_NULL_PTR) {
        AT_WARN_LOG("AT, At_PppId2PsRab, WARNING, pucRabId NULL");
        return TAF_FAILURE;
    }

    indexNum = g_atPppIndexTab[pppId];

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, indexNum <1> Wrong", indexNum);
        return TAF_FAILURE;
    }

    if (DMS_PORT_IsPppPacketTransMode(g_atClientTab[indexNum].portNo) == VOS_FALSE) {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, port not in ppp packet trans mode", indexNum);
        return TAF_FAILURE;
    }

    *exRabId = g_atClientTab[indexNum].exPsRabId;

    return TAF_SUCCESS;
} /* At_PppId2PsRab */

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
VOS_UINT32 At_IfaceId2PppId(VOS_UINT8 ifaceId, VOS_UINT16 *pppId)
{
    VOS_UINT16 pppIdLoop;
    VOS_UINT8  indexNum;

    if (pppId == VOS_NULL_PTR) {
        AT_WARN_LOG("At_IfaceId2PppId, pusPppId NULL");
        return VOS_ERR;
    }

    /* 通过PPP ID方向查找，效率会比较高 */
    for (pppIdLoop = 1; pppIdLoop <= PPP_MAX_ID_NUM; pppIdLoop++) {
        indexNum = g_atPppIndexTab[pppIdLoop];

        if (indexNum >= AT_MAX_CLIENT_NUM) {
            continue;
        }

        if (g_atClientTab[indexNum].used != AT_CLIENT_USED) {
            continue;
        }

        if (DMS_PORT_IsPppPacketTransMode(g_atClientTab[indexNum].portNo) == VOS_FALSE) {
            continue;
        }

        if (g_atClientTab[indexNum].ifaceId == ifaceId) {
            *pppId = pppIdLoop; /* 返回结果 */
            return VOS_OK;
        }
    }

    AT_LOG1("AT, At_IfaceId2PppId, WARNING, Get PppId from Rab <1> Fail", ifaceId);
    return VOS_ERR;
}

MODULE_EXPORTED VOS_UINT32 At_PppId2IfaceId(VOS_UINT16 pppId, VOS_UINT8 *ifaceId)
{
    VOS_UINT8 indexNum;

    if ((pppId < 1) || (pppId > PPP_MAX_ID_NUM)) {
        TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_WARNING, "AT, At_PppId2IfaceId, WARNING, PppId <1> Wrong", pppId);
        return VOS_ERR;
    }

    if (ifaceId == VOS_NULL_PTR) {
        AT_WARN_LOG("AT, At_PppId2IfaceId, WARNING, pucIfaceId NULL");
        return VOS_ERR;
    }

    indexNum = g_atPppIndexTab[pppId];
    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG1("AT, At_PppId2IfaceId, WARNING, indexNum <1> Wrong", indexNum);
        return VOS_ERR;
    }

    if (DMS_PORT_IsPppPacketTransMode(g_atClientTab[indexNum].portNo) == VOS_FALSE) {
        AT_ERR_LOG("AT, At_PppId2IfaceId, WARNING, not in Ppp Packet Trans Mode");
        return VOS_ERR;
    }

    *ifaceId = g_atClientTab[indexNum].ifaceId;

    return VOS_OK;
}
#endif

VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 capability)
{
    if ((capability == AT_IPV6_CAPABILITY_IPV4_ONLY) || (capability == AT_IPV6_CAPABILITY_IPV6_ONLY) ||
        (capability == AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(clientId, &modemId) != VOS_OK) {
        modemId = MODEM_ID_BUTT;
    }

    return TAF_PS_BUILD_EXCLIENTID(modemId, clientId);
}

VOS_VOID AT_PS_ReportImsCtrlMsgu(VOS_UINT8 indexNum, AT_IMS_CtrlMsgReceiveModuleUint8 module, VOS_UINT32 msgLen,
                                 VOS_UINT8 *dst)
{
    /* 定义局部变量 */
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^IMSCTRLMSGU: %d,%d,\"", g_atCrLf, module, msgLen);

    length += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                  (VOS_UINT8 *)g_atSndCodeAddress + length, dst, (VOS_UINT16)msgLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s", g_atCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_PS_BuildIfaceCtrl(const VOS_UINT32 moduleId, const VOS_UINT16 portClientId, const VOS_UINT8 opId,
                                TAF_Ctrl *ctrl)
{
    ModemIdUint16 modemId = MODEM_ID_0;

    /* 获取client id对应的Modem Id */
    if (AT_GetModemIdFromClient(portClientId, &modemId) == VOS_ERR) {
        AT_ERR_LOG("AT_PS_BuildIfaceCtrl:AT_GetModemIdFromClient is error");
        return VOS_ERR;
    }

    ctrl->moduleId = moduleId;
    ctrl->clientId = AT_PS_BuildExClientId(portClientId);
    ctrl->opId     = opId;

    return VOS_OK;
}

VOS_VOID AT_PS_SendNdisIPv4IfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg)
{
    errno_t memResult;
    DMS_PortIdUint16 portId;
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    AT_NDIS_IfaceUpConfigInd *ndisCfgUp = VOS_NULL_PTR;

    if (AT_IsNdisIface(rcvMsg->ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_SendNdisIPv4IfaceUpCfgInd, not NDIS iface", rcvMsg->ifaceId);
        return;
    }

    ndisCfgUp = (AT_NDIS_IfaceUpConfigInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_IfaceUpConfigInd));
#else
    AT_NDIS_PdnInfoCfgReq *ndisCfgUp = VOS_NULL_PTR;

    ndisCfgUp = (AT_NDIS_PdnInfoCfgReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_PdnInfoCfgReq));
#endif

    if (ndisCfgUp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_SendNdisIPv4IfaceUpCfgInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    (VOS_VOID)memset_s(AT_GET_MSG_ENTITY(ndisCfgUp), AT_GET_MSG_LENGTH(ndisCfgUp), 0x00,
                       AT_GET_MSG_LENGTH(ndisCfgUp));

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgUp, ID_AT_NDIS_IFACE_UP_CONFIG_IND);

    /* 填写消息体 */
    ndisCfgUp->iFaceId                    = rcvMsg->ifaceId;
    ndisCfgUp->ipv4IfaceInfo.ifaceId      = rcvMsg->ifaceId;
    ndisCfgUp->ipv4IfaceInfo.pduSessionId = rcvMsg->ipv4PdnInfo.sessionId;
    ndisCfgUp->ipv4IfaceInfo.fcHead       = AT_GET_IFACE_FC_HEAD_BY_MODEMID(rcvMsg->modemId);
    ndisCfgUp->ipv4IfaceInfo.modemId      = (VOS_UINT8)rcvMsg->modemId;
    portId = DMS_PORT_NCM_DATA + (rcvMsg->ifaceId - PS_IFACE_ID_NDIS0);
#else
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgUp, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* 填写消息体 */
    ndisCfgUp->rabId = rcvMsg->rabId;
    ndisCfgUp->modemId = rcvMsg->modemId;
    ndisCfgUp->iSpePort = AT_GetCommPsCtxAddr()->spePort;
    ndisCfgUp->ipfFlag = AT_GetCommPsCtxAddr()->ipfPortFlg;
    portId = DMS_PORT_NCM_DATA;
#endif

    ndisCfgUp->opIpv4PdnInfo = VOS_TRUE;
    ndisCfgUp->handle        = DMS_PORT_GetPortHandle(portId);

    /* 填写IPv4地址 */
    if (rcvMsg->ipv4PdnInfo.opPdnAddr == VOS_TRUE) {
        ndisCfgUp->ipv4PdnInfo.opPdnAddr = VOS_TRUE;
        memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.PdnAddrInfo.ipv4AddrU8,
                             sizeof(ndisCfgUp->ipv4PdnInfo.PdnAddrInfo.ipv4AddrU8),
                             rcvMsg->ipv4PdnInfo.pdnAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.PdnAddrInfo.ipv4AddrU8),
                            DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* 填写掩码地址 */
    memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.subnetMask.ipv4AddrU8,
                         sizeof(ndisCfgUp->ipv4PdnInfo.subnetMask.ipv4AddrU8), rcvMsg->ipv4PdnInfo.subnetMask.ipV4Addr,
                         DSM_NDIS_IPV4_ADDR_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.subnetMask.ipv4AddrU8), DSM_NDIS_IPV4_ADDR_LENGTH);

    /* 填写网关地址 */
    memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.gateWayAddrInfo.ipv4AddrU8,
                         sizeof(ndisCfgUp->ipv4PdnInfo.gateWayAddrInfo.ipv4AddrU8),
                         rcvMsg->ipv4PdnInfo.gateWayAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.gateWayAddrInfo.ipv4AddrU8),
                        DSM_NDIS_IPV4_ADDR_LENGTH);

    /* 填写主DNS地址 */
    if (rcvMsg->ipv4PdnInfo.opDnsPrim != 0) {
        ndisCfgUp->ipv4PdnInfo.opDnsPrim = VOS_TRUE;
        memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.dnsPrimAddrInfo.ipv4AddrU8,
                             sizeof(ndisCfgUp->ipv4PdnInfo.dnsPrimAddrInfo.ipv4AddrU8),
                             rcvMsg->ipv4PdnInfo.dnsPrimAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.dnsPrimAddrInfo.ipv4AddrU8),
                            DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* 填写辅DNS地址 */
    if (rcvMsg->ipv4PdnInfo.opDnsSec != 0) {
        ndisCfgUp->ipv4PdnInfo.opDnsSec = VOS_TRUE;
        memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.dnsSecAddrInfo.ipv4AddrU8,
                             sizeof(ndisCfgUp->ipv4PdnInfo.dnsSecAddrInfo.ipv4AddrU8),
                             rcvMsg->ipv4PdnInfo.dnsSecAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.dnsSecAddrInfo.ipv4AddrU8),
                            DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    if (rcvMsg->ipv4PdnInfo.opPcscfPrim == VOS_TRUE) {
        ndisCfgUp->ipv4PdnInfo.opPcscfPrim = VOS_TRUE;
        memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.pcscfPrimAddrInfo.ipv4AddrU8,
                             sizeof(ndisCfgUp->ipv4PdnInfo.pcscfPrimAddrInfo.ipv4AddrU8),
                             rcvMsg->ipv4PdnInfo.pcscfPrimAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.pcscfPrimAddrInfo.ipv4AddrU8),
                            DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    if (rcvMsg->ipv4PdnInfo.opPcscfSec == VOS_TRUE) {
        ndisCfgUp->ipv4PdnInfo.opPcscfSec = VOS_TRUE;
        memResult = memcpy_s(ndisCfgUp->ipv4PdnInfo.pcscfSecAddrInfo.ipv4AddrU8,
                             sizeof(ndisCfgUp->ipv4PdnInfo.pcscfSecAddrInfo.ipv4AddrU8),
                             rcvMsg->ipv4PdnInfo.pcscfSecAddrInfo.ipV4Addr, DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ndisCfgUp->ipv4PdnInfo.pcscfSecAddrInfo.ipv4AddrU8),
                            DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, ndisCfgUp);
}

/*
 * 功能描述: 填充IPV6的PDN信息
 */
LOCAL VOS_VOID AT_PS_FillIpv6PdnInfo(const DSM_NDIS_IfaceUpInd *rcvMsg,
    AT_NDIS_Ipv6PdnInfo *ipv6PdnInfo)
{
    errno_t memResult;

    /* 填充主副DNS */
    ipv6PdnInfo->dnsSer.serNum = 0;
    if (rcvMsg->ipv6PdnInfo.dnsSer.serNum >= 1) {
        memResult = memcpy_s(ipv6PdnInfo->dnsSer.priServer, sizeof(ipv6PdnInfo->dnsSer.priServer),
                             rcvMsg->ipv6PdnInfo.dnsSer.priServer, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->dnsSer.priServer), TAF_IPV6_ADDR_LEN);
        ipv6PdnInfo->dnsSer.serNum += 1;
    }

    /* IPV6 DNS服务器个数超过2个 */
    if (rcvMsg->ipv6PdnInfo.dnsSer.serNum >= 2) {
        memResult = memcpy_s(ipv6PdnInfo->dnsSer.secServer, sizeof(ipv6PdnInfo->dnsSer.secServer),
                             rcvMsg->ipv6PdnInfo.dnsSer.secServer, TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->dnsSer.secServer), TAF_IPV6_ADDR_LEN);
        ipv6PdnInfo->dnsSer.serNum += 1;
    }

    /* 填充MTU */
    if (rcvMsg->ipv6PdnInfo.bitOpMtu == VOS_TRUE) {
        ipv6PdnInfo->OpMtu = VOS_TRUE;
        ipv6PdnInfo->Mtu   = rcvMsg->ipv6PdnInfo.mtu;
    }

    ipv6PdnInfo->curHopLimit = rcvMsg->ipv6PdnInfo.bitCurHopLimit;
    ipv6PdnInfo->ipv6MValue  = rcvMsg->ipv6PdnInfo.bitM;
    ipv6PdnInfo->ipv6OValue  = rcvMsg->ipv6PdnInfo.bitO;
    ipv6PdnInfo->prefixNum   = rcvMsg->ipv6PdnInfo.prefixNum;
    memResult = memcpy_s(ipv6PdnInfo->prefixList, sizeof(ipv6PdnInfo->prefixList),
                         rcvMsg->ipv6PdnInfo.prefixList, sizeof(TAF_PDP_Ipv6Prefix) * TAF_MAX_PREFIX_NUM_IN_RA);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->prefixList),
                        sizeof(TAF_PDP_Ipv6Prefix) * TAF_MAX_PREFIX_NUM_IN_RA);

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    memResult = memcpy_s(ipv6PdnInfo->interfaceId, sizeof(ipv6PdnInfo->interfaceId),
                         rcvMsg->ipv6PdnInfo.interfaceId, sizeof(VOS_UINT8) * AT_NDIS_IPV6_IFID_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->interfaceId),
                        sizeof(VOS_UINT8) * AT_NDIS_IPV6_IFID_LENGTH);

    /* 填充主副PCSCF地址  */
    ipv6PdnInfo->pcscfSer.serNum = 0;
    if (rcvMsg->ipv6PdnInfo.pcscfSer.serNum > 0) {
        ipv6PdnInfo->pcscfSer.serNum++;

        memResult = memcpy_s(ipv6PdnInfo->pcscfSer.priServer,
                             sizeof(ipv6PdnInfo->pcscfSer.priServer), rcvMsg->ipv6PdnInfo.pcscfSer.priServer,
                             sizeof(rcvMsg->ipv6PdnInfo.pcscfSer.priServer));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->pcscfSer.priServer),
                            sizeof(rcvMsg->ipv6PdnInfo.pcscfSer.priServer));
    }

    if (rcvMsg->ipv6PdnInfo.pcscfSer.serNum > 1) {
        ipv6PdnInfo->pcscfSer.serNum++;

        memResult = memcpy_s(ipv6PdnInfo->pcscfSer.secServer,
                             sizeof(ipv6PdnInfo->pcscfSer.secServer), rcvMsg->ipv6PdnInfo.pcscfSer.secServer,
                             sizeof(rcvMsg->ipv6PdnInfo.pcscfSer.secServer));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ipv6PdnInfo->pcscfSer.secServer),
                            sizeof(rcvMsg->ipv6PdnInfo.pcscfSer.secServer));
    }
}

VOS_VOID AT_PS_SendNdisIPv6IfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg)
{
    DMS_PortIdUint16 portId;
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    AT_NDIS_IfaceUpConfigInd *ndisCfgUp = VOS_NULL_PTR;

    if (AT_IsNdisIface(rcvMsg->ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_SendNdisIPv6IfaceUpCfgInd, not NDIS iface", rcvMsg->ifaceId);
        return;
    }

    ndisCfgUp = (AT_NDIS_IfaceUpConfigInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_IfaceUpConfigInd));
#else
    AT_NDIS_PdnInfoCfgReq *ndisCfgUp = VOS_NULL_PTR;

    ndisCfgUp = (AT_NDIS_PdnInfoCfgReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_PdnInfoCfgReq));
#endif

    if (ndisCfgUp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_SendNdisIPv6IfaceUpCfgInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    (VOS_VOID)memset_s(AT_GET_MSG_ENTITY(ndisCfgUp), AT_GET_MSG_LENGTH(ndisCfgUp), 0x00,
                       AT_GET_MSG_LENGTH(ndisCfgUp));

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgUp, ID_AT_NDIS_IFACE_UP_CONFIG_IND);

    /* 填写消息体 */
    ndisCfgUp->iFaceId                    = rcvMsg->ifaceId;
    ndisCfgUp->ipv6IfaceInfo.ifaceId      = rcvMsg->ifaceId;
    ndisCfgUp->ipv6IfaceInfo.pduSessionId = rcvMsg->ipv6PdnInfo.pduSessionId;
    ndisCfgUp->ipv6IfaceInfo.fcHead       = AT_GET_IFACE_FC_HEAD_BY_MODEMID(rcvMsg->modemId);
    ndisCfgUp->ipv6IfaceInfo.modemId      = (VOS_UINT8)rcvMsg->modemId;
    portId = DMS_PORT_NCM_DATA + (rcvMsg->ifaceId - PS_IFACE_ID_NDIS0);
#else
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgUp, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* 填写消息体 */
    ndisCfgUp->rabId = rcvMsg->rabId;
    ndisCfgUp->modemId = rcvMsg->modemId;
    ndisCfgUp->iSpePort = AT_GetCommPsCtxAddr()->spePort;
    ndisCfgUp->ipfFlag = AT_GetCommPsCtxAddr()->ipfPortFlg;
    portId = DMS_PORT_NCM_DATA;
#endif

    ndisCfgUp->opIpv6PdnInfo = VOS_TRUE;
    ndisCfgUp->handle        = DMS_PORT_GetPortHandle(portId);

    /* 填充IPV6的PDN信息 */
    AT_PS_FillIpv6PdnInfo(rcvMsg, &ndisCfgUp->ipv6PdnInfo);

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, ndisCfgUp);
}

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
VOS_VOID AT_PS_SendNdserverPppIPv6CfgInd(const AT_NDSERVER_Ipv6PdnInfo *ipv6PdnInfo)
{
    AT_NDSERVER_PPPIpv6CfgInd *ipv6Cfg = VOS_NULL_PTR;
    errno_t memResult;

    ipv6Cfg = (AT_NDSERVER_PPPIpv6CfgInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDSERVER_PPPIpv6CfgInd));

    if (ipv6Cfg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_SendNdserverPppIPv6CfgInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    memResult = memset_s(AT_GET_MSG_ENTITY(ipv6Cfg), AT_GET_MSG_LENGTH(ipv6Cfg), 0x00,
        AT_GET_MSG_LENGTH(ipv6Cfg));
    TAF_MEM_CHK_RTN_VAL(memResult, AT_GET_MSG_LENGTH(ipv6Cfg), AT_GET_MSG_LENGTH(ipv6Cfg));

    /* 填写消息头 */
    AT_CFG_MSG_HDR(ipv6Cfg, UEPS_PID_NDSERVER, ID_AT_NDSERVER_PPP_IPV6_CFG_IND);

    /* 填写消息体 */
    ipv6Cfg->ifaceId = PS_IFACE_ID_PPP0;
    ipv6Cfg->ipv6Info = *ipv6PdnInfo;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, ipv6Cfg);
}

VOS_VOID AT_PS_SendNdserverPppIPv6RelInd(VOS_VOID)
{
    AT_NDSERVER_PPPIpv6RelInd *relInd = VOS_NULL_PTR;
    errno_t memResult;

    relInd = (AT_NDSERVER_PPPIpv6RelInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDSERVER_PPPIpv6RelInd));

    if (relInd == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_SendNdserverPppIPv6RelInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    memResult = memset_s(AT_GET_MSG_ENTITY(relInd), AT_GET_MSG_LENGTH(relInd), 0x00,
        AT_GET_MSG_LENGTH(relInd));
    TAF_MEM_CHK_RTN_VAL(memResult, AT_GET_MSG_LENGTH(relInd), AT_GET_MSG_LENGTH(relInd));

    /* 填写消息头 */
    AT_CFG_MSG_HDR(relInd, UEPS_PID_NDSERVER, ID_AT_NDSERVER_PPP_IPV6_REL_IND);

    /* 该消息 */
    relInd->ifaceId = PS_IFACE_ID_PPP0;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, relInd);
}
#endif

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)

VOS_VOID AT_PS_SendNdisEthIfaceUpCfgInd(const DSM_NDIS_IfaceUpInd *rcvMsg)
{
    AT_NDIS_IfaceUpConfigInd *ndisCfgUp = VOS_NULL_PTR;
    errno_t memResult;
    DMS_PortIdUint16 portId;

    if (AT_IsNdisIface(rcvMsg->ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_SendNdisEthIfaceUpCfgInd, not NDIS iface", rcvMsg->ifaceId);
        return;
    }

    ndisCfgUp = (AT_NDIS_IfaceUpConfigInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_IfaceUpConfigInd));
    if (ndisCfgUp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_SendNdisEthIfaceUpCfgInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    memResult = memset_s(AT_GET_MSG_ENTITY(ndisCfgUp), AT_GET_MSG_LENGTH(ndisCfgUp), 0x00,
                         AT_GET_MSG_LENGTH(ndisCfgUp));
    TAF_MEM_CHK_RTN_VAL(memResult, AT_GET_MSG_LENGTH(ndisCfgUp), AT_GET_MSG_LENGTH(ndisCfgUp));

    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgUp, ID_AT_NDIS_IFACE_UP_CONFIG_IND);

    /* 填写消息体 */
    ndisCfgUp->iFaceId = rcvMsg->ifaceId;
    portId = DMS_PORT_NCM_DATA + (rcvMsg->ifaceId - PS_IFACE_ID_NDIS0);

    ndisCfgUp->opEthPdnInfo = VOS_TRUE;
    ndisCfgUp->handle       = DMS_PORT_GetPortHandle(portId);

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, ndisCfgUp);
}
#endif

RNIC_DEV_ID_ENUM_UINT8 AT_PS_GetRmnetIdFromIfaceId(const PS_IFACE_IdUint8 ifaceId)
{
    VOS_UINT32 i;
    CONST AT_PS_RmnetIfaceId *rmnetIfaceIdTab;

    rmnetIfaceIdTab = AT_PS_RMNET_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_RMNET_IFACE_ID_TBL_SIZE(); i++) {
        if (ifaceId == rmnetIfaceIdTab[i].ifaceId) {
            return rmnetIfaceIdTab[i].rmNetId;
        }
    }

    AT_WARN_LOG("AT_PS_GetRmnetIdFromIfaceId: not find rmnet id");
    return RNIC_DEV_ID_BUTT;
}

VOS_VOID AT_PS_ProcNdisIfaceUpCfg(const DSM_NDIS_IfaceUpInd *rcvMsg)
{
    if (rcvMsg->opIpv4PdnInfo == VOS_TRUE) {
        AT_PS_SendNdisIPv4IfaceUpCfgInd(rcvMsg);
    }

    if (rcvMsg->opIpv6PdnInfo == VOS_TRUE) {
        AT_PS_SendNdisIPv6IfaceUpCfgInd(rcvMsg);
    }

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (rcvMsg->opEthPdnInfo == VOS_TRUE) {
        AT_PS_SendNdisEthIfaceUpCfgInd(rcvMsg);
    }
#endif
}

VOS_VOID AT_PS_ProcNdisIfaceDownCfg(const DSM_NDIS_IfaceDownInd *rcvMsg)
{
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    AT_NDIS_IfaceDownConfigInd *ndisCfgDown = VOS_NULL_PTR;

    ndisCfgDown = (AT_NDIS_IfaceDownConfigInd *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_IfaceDownConfigInd));
#else
    AT_NDIS_PdnInfoRelReq *ndisCfgDown = VOS_NULL_PTR;

    ndisCfgDown = (AT_NDIS_PdnInfoRelReq *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_NDIS_PdnInfoRelReq));
#endif

    if (ndisCfgDown == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_PS_ProcNdisIfaceDownCfg: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    (VOS_VOID)memset_s(AT_GET_MSG_ENTITY(ndisCfgDown), AT_GET_MSG_LENGTH(ndisCfgDown), 0x00,
                       AT_GET_MSG_LENGTH(ndisCfgDown));

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgDown, ID_AT_NDIS_IFACE_DOWN_CONFIG_IND);

    /* 填写消息体 */
    ndisCfgDown->opIpv4PdnInfo = rcvMsg->opIpv4PdnInfo;
    ndisCfgDown->opIpv6PdnInfo = rcvMsg->opIpv6PdnInfo;
    ndisCfgDown->iFaceId       = rcvMsg->ifaceId;
#else
    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(ndisCfgDown, ID_AT_NDIS_PDNINFO_REL_REQ);

    /* 填写消息体 */
    ndisCfgDown->modemId = rcvMsg->modemId;
    ndisCfgDown->rabId = rcvMsg->rabId;
#endif

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(WUEPS_PID_AT, ndisCfgDown);
}

VOS_VOID AT_PS_ProcNdisConfigIpv6Dns(const DSM_NDIS_ConfigIpv6DnsInd *rcvMsg)
{
    VOS_UINT8          *ipv6DnsInfo = VOS_NULL_PTR;
    VOS_UINT32          rslt;
    errno_t             memResult;
    VOS_UINT32          len;
    DMS_PortIdUint16    portId;

    if (AT_IsNdisIface(rcvMsg->ifaceId) == VOS_FALSE) {
        AT_ERR_LOG1("AT_PS_ProcNdisConfigIpv6Dns, not NDIS iface", rcvMsg->ifaceId);
        return;
    }

    portId = DMS_PORT_NCM_DATA + (rcvMsg->ifaceId - PS_IFACE_ID_NDIS0);

    ipv6DnsInfo = (unsigned char *)PS_MEM_ALLOC(WUEPS_PID_AT, DMS_NCM_IPV6_DNS_LEN);

    if (ipv6DnsInfo == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_PS_ProcNdisConfigIpv6Dns:Invalid stIPv6Dns.ipv6_dns_info");
        return;
    }

    (VOS_VOID)memset_s(ipv6DnsInfo, DMS_NCM_IPV6_DNS_LEN, 0x00, DMS_NCM_IPV6_DNS_LEN);

    /* 上报给底软的DNS长度固定为32(Primary DNS LEN + Secondary DNS LEN) */
    len = DMS_NCM_IPV6_DNS_LEN;

    /* 如果有DNS，需要调用DRV的接口上报DNS给PC */
    if (rcvMsg->opIpv6PriDns == VOS_TRUE) {
        memResult = memcpy_s(ipv6DnsInfo, DMS_NCM_IPV6_DNS_LEN, rcvMsg->ipv6PrimDns, AT_MAX_IPV6_DNS_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, DMS_NCM_IPV6_DNS_LEN, AT_MAX_IPV6_DNS_LEN);
    }

    if (rcvMsg->opIpv6SecDns == VOS_TRUE) {
        memResult = memcpy_s(ipv6DnsInfo + AT_MAX_IPV6_DNS_LEN, DMS_NCM_IPV6_DNS_LEN - AT_MAX_IPV6_DNS_LEN,
                             rcvMsg->ipv6SecDns, AT_MAX_IPV6_DNS_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, DMS_NCM_IPV6_DNS_LEN - AT_MAX_IPV6_DNS_LEN, AT_MAX_IPV6_DNS_LEN);
    }

    /* 设置低软主副DNS信息 */
    rslt = DMS_PORT_SetIpv6Dns(portId, ipv6DnsInfo, len);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_PS_ProcNdisConfigIpv6Dns, DRV_UDI_IOCTL Fail!");
    }

    /* 释放申请的内存 */
    PS_MEM_FREE(WUEPS_PID_AT, ipv6DnsInfo);
}

VOS_VOID AT_RcvTafIfaceEvt(TAF_PS_Evt *evt)
{
    MN_PS_EVT_FUNC evtFunc = VOS_NULL_PTR;
    TAF_Ctrl      *ctrl    = VOS_NULL_PTR;
    VOS_UINT32     i;
    VOS_UINT32     result;
    VOS_UINT8      portIndex;

    /* 初始化 */
    ctrl      = (TAF_Ctrl *)(evt->content);
    result    = VOS_ERR;
    portIndex = 0;

    if (At_ClientIdToUserId(ctrl->clientId, &portIndex) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvTafIfaceEvt: usPortClientId At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(portIndex)) {
        /*
         * 广播IDNEX不可以作为数组下标使用，需要在事件处理函数中仔细核对，避免数组越界。
         * 目前没有广播事件，请仔细核对，
         */
        AT_WARN_LOG("AT_RcvTafIfaceEvt: AT_BROADCAST_INDEX,but not Broadcast Event.");
        return;
    }

    /* 在事件处理表中查找处理函数 */
    for (i = 0; i < AT_ARRAY_SIZE(g_atIfaceEvtFuncTbl); i++) {
        if (evt->evtId == g_atIfaceEvtFuncTbl[i].evtId) {
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)evt, g_atIfaceEvtFuncTbl[i].msgLen,
                    g_atIfaceEvtFuncTbl[i].chkFunc) == VOS_FALSE) {
                AT_ERR_LOG("AT_RcvTafIfaceEvt: Check MsgLength Err");
                return;
            }
            /* 事件ID匹配 */
            evtFunc = g_atIfaceEvtFuncTbl[i].evtFunc;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    if (evtFunc != VOS_NULL_PTR) {
        result = evtFunc(portIndex, (TAF_Ctrl *)evt->content);
    }

    if (result != VOS_OK) {
        AT_ERR_LOG1("AT_RcvTafIfaceEvt: Can not handle this message! <MsgId>", evt->evtId);
    }
}

