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
#include "at_sat_pam_rslt_proc.h"
#include "securec.h"
#include "at_check_func.h"
#include "ppp_interface.h"
#include "taf_mmi_str_parse.h"
#include "at_oam_interface.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "taf_std_lib.h"
#include "mn_comm_api.h"
#include "at_sim_comm.h"
#include "at_common.h"
#include "dms_msg_chk.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAT_PAM_RSLT_PROC_C
#define AT_ASCII_ARRAY_LEN 250

#if (VOS_WIN32 == VOS_OS_VER)
static const TAF_UINT8 g_atStin[] = "^STIN:";
static const TAF_UINT8 g_atStmn[] = "^STMN:";
static const TAF_UINT8 g_atStgi[] = "^STGI:";
static const TAF_UINT8 g_atStsf[] = "^STSF:";
static const TAF_UINT8 g_atCsin[] = "^CSIN:";
static const TAF_UINT8 g_atCstr[] = "^CSTR:";
static const TAF_UINT8 g_atCsen[] = "^CSEN:";
static const TAF_UINT8 g_atCsmn[] = "^CSMN:";
static const TAF_UINT8 g_atCcin[] = "^CCIN:";
#else
static const TAF_UINT8 g_atStin[] = "^STIN:";
static const TAF_UINT8 g_atStgi[] = "^STGI:";
static const TAF_UINT8 g_atCsin[] = "^CSIN:";
static const TAF_UINT8 g_atCstr[] = "^CSTR:";
static const TAF_UINT8 g_atCsen[] = "^CSEN:";
static const TAF_UINT8 g_atCsmn[] = "^CSMN:";
static const TAF_UINT8 g_atCcin[] = "^CCIN:";
#endif

VOS_VOID At_StkNumPrint(VOS_UINT16 *length, VOS_UINT8 *data, VOS_UINT16 dataLen, VOS_UINT8 numType)
{
    errno_t    memResult;
    VOS_UINT16 lengthTemp = *length;
    /* 打印数据 */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",\"");

    if (dataLen > 0) {
        memResult = memcpy_s(g_atSndCodeAddress + lengthTemp, AT_CMD_MAX_LEN - lengthTemp , data, dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, dataLen, dataLen);
    }

    lengthTemp += dataLen;

    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "\"");

    /* 打印类型 */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", numType);

    *length = lengthTemp;
}

VOS_VOID AT_BcdHalfByteToAscii(VOS_UINT8 bcdHalfByte, VOS_UINT8 *asciiNum)
{
    if (bcdHalfByte <= AT_DEC_MAX_NUM) { /* 转换数字 */
        *asciiNum = bcdHalfByte + 0x30;
    } else if (bcdHalfByte == 0x0A) { /* 转换*字符 */
        *asciiNum = 0x2a;
    } else if (bcdHalfByte == 0x0B) { /* 转换#字符 */
        *asciiNum = 0x23;
    } else if (bcdHalfByte == 0x0C) { /* 转换'P'字符 */
        *asciiNum = 0x50;
    } else if (bcdHalfByte == 0x0D) { /* 转换'?'字符 */
        *asciiNum = 0x3F;
    } else { /* 转换字母 */
        *asciiNum = bcdHalfByte + 0x57;
    }
}

VOS_VOID AT_BcdToAscii(VOS_UINT8 bcdNumLen, VOS_UINT8 *bcdNum, VOS_UINT8 *asciiNum, VOS_UINT8 asciiNumBufflen,
                       VOS_UINT8 *len)
{
    VOS_UINT32 tmp;
    VOS_UINT8 lenTemp = 0;
    VOS_UINT8 firstNumber;
    VOS_UINT8 secondNumber;

    for (tmp = 0; tmp < bcdNumLen; tmp++) {
        if (bcdNum[tmp] == 0xFF) {
            break;
        }

        firstNumber = (VOS_UINT8)(bcdNum[tmp] & 0x0F); /* 取出高半字节 */

        secondNumber = (VOS_UINT8)((bcdNum[tmp] >> 4) & 0x0F); /* 取出低半字节 */

        /* 代码安全走读添加，防止越界访问 */
        if (asciiNumBufflen <= lenTemp) {
            lenTemp = 0;
            break;
        }

        AT_BcdHalfByteToAscii(firstNumber, asciiNum);

        asciiNum++;

        lenTemp++;

        if (secondNumber == 0x0F) {
            break;
        }

        /* 代码安全走读添加，防止越界访问 */
        if (asciiNumBufflen <= lenTemp) {
            lenTemp = 0;
            break;
        }

        AT_BcdHalfByteToAscii(secondNumber, asciiNum);

        asciiNum++;

        lenTemp++;
    }

    *len = lenTemp;
}

VOS_VOID At_StkCcinIndPrint(VOS_UINT8 indexNum, SI_STK_EventInfo *event)
{
    VOS_UINT16 length                    = 0;
    VOS_UINT8  ascii[AT_ASCII_ARRAY_LEN] = {0};
    VOS_UINT8  asciiLen                  = 0;
    /* 打印输入AT命令类型 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCcin);

    /* 打印call/sms control 类型 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", (event->cmdDataInfo.cmdStru.ccIndInfo.type));

    /* 打印Call/SMS Control的结果 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", (event->cmdDataInfo.cmdStru.ccIndInfo.result));

    /* 打印ALPHAID标识 */
    if (event->cmdDataInfo.cmdStru.ccIndInfo.alphaIdInfo.alphaLen != 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s",
            (event->cmdDataInfo.cmdStru.ccIndInfo.alphaIdInfo.alphaId));

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",\"\"");
    }

    if (event->cmdDataInfo.cmdStru.ccIndInfo.type == SI_STK_SMS_CTRL) {
        /* 将目的号码由BCD码转换成acsii */
        AT_BcdToAscii(event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.dstAddrInfo.addrLen,
                      event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.dstAddrInfo.addr, ascii, sizeof(ascii),
                      &asciiLen);

        /* 打印目的地址和类型 */
        At_StkNumPrint(&length, ascii, asciiLen,
                       event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.dstAddrInfo.numType);

        /* 将服务中心号码由BCD码转换成acsii */
        AT_BcdToAscii(event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.serCenterAddrInfo.addrLen,
                      event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.serCenterAddrInfo.addr, ascii,
                      sizeof(ascii), &asciiLen);

        /* 打印服务中心地址和类型 */
        At_StkNumPrint(&length, ascii, asciiLen,
                       event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.moSmsCtrlInfo.serCenterAddrInfo.numType);

    } else if (event->cmdDataInfo.cmdStru.ccIndInfo.type == SI_STK_USSD_CALL_CTRL) {
        /* 打印dcs字段和data字段 */
        At_StkNumPrint(&length, event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.data,
                       event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.dataLen,
                       event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.dataType);
    } else {
        /* 将目的号码由BCD码转换成acsii */
        AT_BcdToAscii((VOS_UINT8)event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.dataLen,
                      event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.data, ascii, sizeof(ascii), &asciiLen);

        /* 打印目的地址和类型 */
        At_StkNumPrint(&length, ascii, asciiLen, event->cmdDataInfo.cmdStru.ccIndInfo.uInfo.ctrlDataInfo.dataType);
    }

    /* 打印回车换行 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCrLfAddr, length + AT_CRLF_STR_LEN);
}

/* PC工程中AT从A核移到C核, At_sprintf有重复定义,故在此处添加条件编译宏 */
/*
 * Description: Sat消息处理函数
 * History:
 */
VOS_UINT32 At_ChangeSTKCmdNo(VOS_UINT32 cmdType, VOS_UINT8 *cmdNo)
{
    switch (cmdType) {
        case SI_STK_REFRESH:
            *cmdNo = SI_AT_CMD_REFRESH;
            break;
        case SI_STK_DISPLAYTET:
            *cmdNo = SI_AT_CMD_DISPLAY_TEXT;
            break;
        case SI_STK_GETINKEY:
            *cmdNo = SI_AT_CMD_GET_INKEY;
            break;
        case SI_STK_GETINPUT:
            *cmdNo = SI_AT_CMD_GET_INPUT;
            break;
        case SI_STK_PLAYTONE:
            *cmdNo = SI_AT_CMD_PLAY_TONE;
            break;
        case SI_STK_SELECTITEM:
            *cmdNo = SI_AT_CMD_SELECT_ITEM;
            break;
        case SI_STK_SETUPMENU:
            *cmdNo = SI_AT_CMD_SETUP_MENU;
            break;
        case SI_STK_SETUPIDLETEXT:
            *cmdNo = SI_AT_CMD_SETUP_IDLE_MODE_TEXT;
            break;
        case SI_STK_LAUNCHBROWSER:
            *cmdNo = SI_AT_CMD_LAUNCH_BROWSER;
            break;
        case SI_STK_SENDSS:
            *cmdNo = SI_AT_CMD_SEND_SS;
            break;
        case SI_STK_LANGUAGENOTIFICATION:
            *cmdNo = SI_AT_CMD_LANGUAGENOTIFICATION;
            break;
        case SI_STK_SETFRAMES:
            *cmdNo = SI_AT_CMD_SETFRAMES;
            break;
        case SI_STK_GETFRAMESSTATUS:
            *cmdNo = SI_AT_CMD_GETFRAMESSTATUS;
            break;
        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


TAF_UINT32 At_HexText2AsciiStringSimple(TAF_UINT32 maxLength, TAF_INT8 *headaddr, TAF_UINT8 *dst, TAF_UINT32 len,
                                        TAF_UINT8 *str)
{
    TAF_UINT16 lenValue = 0;
    TAF_UINT16 chkLen   = 0;
    TAF_UINT8 *write    = dst;
    TAF_UINT8 *read     = str;
    TAF_UINT8  high     = 0;
    TAF_UINT8  low      = 0;
    /* "\"、"\"、"\0"三个字符所以长度+3 */
    if (((TAF_UINT32)(dst - (TAF_UINT8 *)headaddr) + (AT_ASCII_AND_HEX_CONVERSION_FACTOR * len) + 3) >= maxLength) {
        AT_ERR_LOG("At_HexText2AsciiString too long");
        return 0;
    }

    if (len != 0) {
        lenValue += 1;

        *write++ = '\"';

        /* 扫完整个字串 */
        while (chkLen++ < len) {
            high = 0x0F & (*read >> 4);
            low  = 0x0F & *read;
            lenValue += 2; /* 记录长度,高位和低位长度为2 */

            if (high <= 0x09) { /* 0-9 */
                *write++ = high + 0x30;
            } else if (high >= 0x0A) { /* A-F */
                *write++ = high + 0x37;
            } else {
            }

            if (low <= 0x09) { /* 0-9 */
                *write++ = low + 0x30;
            } else if (low >= 0x0A) { /* A-F */
                *write++ = low + 0x37;
            } else {
            }
            /* 下一个字符 */
            read++;
        }

        lenValue++;

        *write++ = '\"';

        *write++ = '\0';
    }

    return lenValue;
}

/*
 * Description: CSIN上报主动命令的打印输出函数
 * History:
 *  1.Date: 2012-5-17
 *    Modification: Created function
 */
TAF_VOID At_StkCsinIndPrint(TAF_UINT8 indexNum, SI_STK_EventInfo *event)
{
    TAF_UINT16 length = 0;

    /* 打印输入AT命令类型 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s%s", g_atCrLf, g_atCsin);

    event->cmdDataInfo.satCmd.satDataLen = AT_MIN(event->cmdDataInfo.satCmd.satDataLen,
        sizeof(event->cmdDataInfo.satCmd.satCmdData));
    /* 打印输入主动命令类型长度和类型 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d",
        (event->cmdDataInfo.satCmd.satDataLen * AT_ASCII_AND_HEX_CONVERSION_FACTOR));

    /* 有主动命令时才输入 */
    if (event->cmdDataInfo.satCmd.satDataLen != 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ", %d, ", event->cmdDataInfo.satType);

        /* 将16进制数转换为ASCII码后输入主动命令内容 */
        length += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                           (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                           event->cmdDataInfo.satCmd.satDataLen,
                                                           event->cmdDataInfo.satCmd.satCmdData);
    }

    /* 打印回车换行 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

/*
 * Description: Sat消息处理函数
 * History:
 *  1.Date: 2009-07-04
 *    Author: zhuli
 *    Modification: Created function
 */
TAF_VOID At_STKCMDDataPrintSimple(TAF_UINT8 indexNum, SI_STK_EventInfo *event)
{
    TAF_UINT16 length = 0;

    if (event->cbEvent == SI_STK_CMD_IND_EVENT) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCsin);
    } else {
        if (event->cmdDataInfo.satType != SI_STK_SETUPMENU) {
            return;
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCsmn);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d, %d, ",
        (event->cmdDataInfo.satCmd.satDataLen * AT_ASCII_AND_HEX_CONVERSION_FACTOR), event->cmdDataInfo.satType);

    length += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                       (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                       event->cmdDataInfo.satCmd.satDataLen,
                                                       event->cmdDataInfo.satCmd.satCmdData);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCrLfAddr, length + AT_CRLF_STR_LEN);
}

/*
 * Description: Sat消息处理函数
 * History:
 *  1.Date: 2009-07-04
 *    Author: zhuli
 *    Modification: Created function
 */
TAF_VOID At_STKCMDSWPrintSimple(TAF_UINT8 indexNum, STK_CallbackEventUint8 sTKCBEvent, SI_STK_SwInfo *sw)
{
    TAF_UINT16 length          = 0;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    /* 获取上层对接应用配置: MP/WEBUI/ANDROID */
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return;
    }

    if (sTKCBEvent == SI_STK_TERMINAL_RSP_EVENT) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCstr);
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCsen);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d, %d%s", sw->sw1, sw->sw2, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCrLfAddr, length + AT_CRLF_STR_LEN);
}

VOS_VOID AT_SendSTKCMDTypeResultData(VOS_UINT8 indexNum, VOS_UINT16 length)
{
    errno_t memResult;
    if (g_atVType == AT_V_ENTIRE_TYPE) {
        /* Code前面加\r\n */
        memResult = memcpy_s((TAF_CHAR *)g_atSndCrLfAddr, AT_SEND_CRLF_ADDR_MAX_LEN, (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN, AT_CRLF_STR_LEN);
        At_SendResultData(indexNum, g_atSndCrLfAddr, length + AT_CRLF_STR_LEN);
    } else {
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
}

VOS_UINT32 At_STKCMDTypePrint(TAF_UINT8 indexNum, TAF_UINT32 satType, TAF_UINT32 eventType)
{
    VOS_UINT8    *systemAppConfig = VOS_NULL_PTR;
    TAF_UINT16    length          = 0;
    TAF_UINT8     cmdType         = 0;
    TAF_UINT32    result          = AT_SUCCESS;
    ModemIdUint16 modemId;
    VOS_UINT32    rslt;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

    /* 初始化 */
    modemId = MODEM_ID_0;

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_STKCMDTypePrint: Get modem id fail.");
        return AT_FAILURE;
    }

    /* 对接AP不需要检查 */
    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        if (eventType != SI_STK_CMD_END_EVENT) {
            result = At_ChangeSTKCmdNo(satType, &cmdType);
        }

        if (result == AT_FAILURE) {
            return AT_FAILURE;
        }
    }

    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_FAILURE;
    }

    switch (eventType) {
        case SI_STK_CMD_QUERY_RSP_EVENT:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s %d, 0%s", g_atStgi, cmdType, g_atCrLf);
            break;
        case SI_STK_CMD_IND_EVENT:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s %d, 0, 0%s", g_atStin, cmdType, g_atCrLf);
            break;
        case SI_STK_CMD_END_EVENT:
            if (*systemAppConfig == SYSTEM_APP_ANDROID) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "%s 0, 0%s", g_atCsin, g_atCrLf);
            } else {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "%s 99, 0, 0%s", g_atStin, g_atCrLf);
            }
            break;
        default:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s %d, 0, 1%s", g_atStin, cmdType, g_atCrLf);
            break;
    }

    AT_SendSTKCMDTypeResultData(indexNum, length);

    return AT_SUCCESS;
}
/*
 * 判断是否是主动的STK命令
 */
LOCAL VOS_UINT32 AT_IsStkCnfMsgCbEventActiveCmd(STK_CallbackEventUint8 cbEvent)
{
    if ((cbEvent != SI_STK_CMD_IND_EVENT) &&
        (cbEvent != SI_STK_CMD_TIMEOUT_IND_EVENT) &&
        (cbEvent != SI_STK_CMD_END_EVENT) &&
        (cbEvent != SI_STK_CC_RESULT_IND_EVENT) &&
        (cbEvent != SI_STK_SMSCTRL_RESULT_IND_EVENT)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
/*
 * stk cnf消息按照call back event来分发
 */
LOCAL VOS_VOID AT_StkCnfMsgCbEventDistr(MN_APP_StkAtCnf *sTKCnfMsg, VOS_UINT8 indexNum)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif
    TAF_UINT32 result = AT_OK;

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    switch (sTKCnfMsg->stkAtCnf.cbEvent) {
        case SI_STK_CMD_IND_EVENT:
            if (*systemAppConfig == SYSTEM_APP_ANDROID) {
                At_StkCsinIndPrint(indexNum, &(sTKCnfMsg->stkAtCnf));
            } else {
                At_STKCMDTypePrint(indexNum, sTKCnfMsg->stkAtCnf.cmdDataInfo.satType, sTKCnfMsg->stkAtCnf.cbEvent);
            }

            break;
        case SI_STK_CMD_END_EVENT:
        case SI_STK_CMD_TIMEOUT_IND_EVENT:
            At_STKCMDTypePrint(indexNum, sTKCnfMsg->stkAtCnf.cmdDataInfo.satType, sTKCnfMsg->stkAtCnf.cbEvent);
            break;

        case SI_STK_CMD_QUERY_RSP_EVENT:
            At_STKCMDTypePrint(indexNum, sTKCnfMsg->stkAtCnf.cmdDataInfo.satType, sTKCnfMsg->stkAtCnf.cbEvent);
            At_FormatResultData(indexNum, result);
            break;

        case SI_STK_GET_CMD_RSP_EVENT:
            if (*systemAppConfig == SYSTEM_APP_ANDROID) {
                At_StkCsinIndPrint(indexNum, &(sTKCnfMsg->stkAtCnf));
            } else {
                At_STKCMDDataPrintSimple(indexNum, &(sTKCnfMsg->stkAtCnf));
            }

            At_FormatResultData(indexNum, result);
            break;

        case SI_STK_ENVELPOE_RSP_EVENT:
        case SI_STK_TERMINAL_RSP_EVENT:
            At_STKCMDSWPrintSimple(indexNum, sTKCnfMsg->stkAtCnf.cbEvent, &sTKCnfMsg->stkAtCnf.swInfo);
            At_FormatResultData(indexNum, result);
            break;

        case SI_STK_CC_RESULT_IND_EVENT:
        case SI_STK_SMSCTRL_RESULT_IND_EVENT:
            At_StkCcinIndPrint(indexNum, &(sTKCnfMsg->stkAtCnf));
            break;

        default:
            At_FormatResultData(indexNum, result);
            break;
    }
}

TAF_VOID AT_StkCnfMsgProc(MN_APP_StkAtCnf *stkCnfMsg)
{
    TAF_UINT8  indexNum = 0;
    TAF_UINT32 result = AT_OK;

    AT_LOG1("AT_StkCnfMsgProc pEvent->ClientId", stkCnfMsg->stkAtCnf.clientId);
    AT_LOG1("AT_StkCnfMsgProc EventType", stkCnfMsg->stkAtCnf.cbEvent);
    AT_LOG1("AT_StkCnfMsgProc SIM Event Error", stkCnfMsg->stkAtCnf.errorNo);

    g_atSendDataBuff.bufLen = 0;

    if (At_ClientIdToUserId(stkCnfMsg->stkAtCnf.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("AT_StkCnfMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    /* 如果不是主动命令，则停止定时器 */
    if (AT_IsStkCnfMsgCbEventActiveCmd(stkCnfMsg->stkAtCnf.cbEvent) != VOS_TRUE) {
        if (AT_IsBroadcastClientIndex(indexNum)) {
            AT_WARN_LOG("AT_StkCnfMsgProc: AT_BROADCAST_INDEX.");
            return;
        }

        AT_StopTimerCmdReady(indexNum);

        AT_LOG1("AT_StkCnfMsgProc ucIndex", indexNum);
        AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);
    }

    if (stkCnfMsg->stkAtCnf.errorNo != AT_SUCCESS) {
        result = At_ChgTafErrorCode(indexNum, (TAF_UINT16)stkCnfMsg->stkAtCnf.errorNo); /* 发生错误 */

        At_FormatResultData(indexNum, result);
    } else {
        AT_StkCnfMsgCbEventDistr(stkCnfMsg, indexNum);
    }
}

TAF_VOID AT_STKPrintMsgProc(MN_APP_StkAtDataprint *sTKPrintMsg)
{
    errno_t   memResult;
    TAF_UINT8 indexNum = 0;

    if (At_ClientIdToUserId(sTKPrintMsg->stkAtPrint.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("AT_STKPrintMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_STKPrintMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_StopTimerCmdReady(indexNum);

    AT_LOG1("At_StkMsgProc pEvent->ClientId", sTKPrintMsg->stkAtPrint.clientId);
    AT_LOG1("At_StkMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    AT_SetStkCmdQualify(sTKPrintMsg->stkAtPrint.cmdQualify);

    sTKPrintMsg->stkAtPrint.dataLen = TAF_MIN(sTKPrintMsg->stkAtPrint.dataLen, STK_PRINT_MAX_LEN);
    if (sTKPrintMsg->stkAtPrint.dataLen > 0) {
        memResult = memcpy_s(g_atSndCodeAddress, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                             sTKPrintMsg->stkAtPrint.data, sTKPrintMsg->stkAtPrint.dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                            sTKPrintMsg->stkAtPrint.dataLen);
    }

    At_SendResultData(indexNum, g_atSndCrLfAddr, (VOS_UINT16)sTKPrintMsg->stkAtPrint.dataLen + AT_CRLF_STR_LEN);

    At_FormatResultData(indexNum, AT_OK);
}

TAF_VOID At_StkMsgProc(struct MsgCB *msg)
{
    MN_APP_StkAtDataprint *sTKPrintMsg = VOS_NULL_PTR;
    MN_APP_StkAtCnf       *sTKCnfMsg   = VOS_NULL_PTR;

    sTKCnfMsg   = (MN_APP_StkAtCnf *)msg;
    sTKPrintMsg = (MN_APP_StkAtDataprint *)msg;

    if (sTKCnfMsg->msgId == STK_AT_DATAPRINT_CNF) {
        AT_STKPrintMsgProc(sTKPrintMsg);
    } else if (sTKCnfMsg->msgId == STK_AT_EVENT_CNF) {
        AT_StkCnfMsgProc(sTKCnfMsg);
    } else {
        AT_ERR_LOG1("At_StkMsgProc:Msg ID Error", sTKPrintMsg->msgId);
    }
}
