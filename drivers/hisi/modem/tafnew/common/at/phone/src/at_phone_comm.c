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
#include "at_phone_comm.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_event_report.h"
#include "at_msg_print.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_COMM_C
#define AT_GMT_TIME_DEFAULT 70 /* 判断标准时间是1900还是1970 */
#define AT_TAFMMA_CELL_ID_INDEX_0 0

VOS_VOID AT_PrintCclkTime(VOS_UINT8 indexNum, TIME_ZONE_Time *timeZoneTime, ModemIdUint16 modemId)
{
    TIME_ZONE_Time *timeZone = VOS_NULL_PTR;
    AT_ModemNetCtx *netCtx   = VOS_NULL_PTR;
    VOS_UINT32      year;

    timeZone = timeZoneTime;

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if (netCtx->csdfCfg.auxMode == 1) {
        /* "yy/mm/dd,hh:mm:ss(+/-)zz" */
        year = timeZone->year;
    } else {
        /* "yyyy/mm/dd,hh:mm:ss(+/-)zz" */
        if (timeZone->year > AT_GMT_TIME_DEFAULT) {
            year = 1900 + (VOS_UINT32)timeZone->year;
        } else {
            year = 2000 + (VOS_UINT32)timeZone->year;
        }
    }

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: \"%02d/%02d/%02d,%02d:%02d:%02d",
            g_parseContext[indexNum].cmdElement->cmdName, year, timeZone->month, timeZone->day, timeZone->hour,
            timeZone->minute, timeZone->second);

    if (timeZone->timeZone >= 0) {
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "+%02d\"", timeZone->timeZone);

    } else {
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "-%02d\"", -(timeZone->timeZone));
    }
}
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT16 AT_PrintMbbCclkTimeInfo(NAS_MM_InfoInd *mmInfo, CONST_T VOS_CHAR *pcCmd, VOS_UINT32 second)
{
    VOS_INT8 timeZone;
    VOS_UINT32 year;
    VOS_UINT16 length = 0;
    NAS_MM_InfoInd localAtTimeInfo;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&localAtTimeInfo, sizeof(localAtTimeInfo), 0x00, sizeof(localAtTimeInfo));

    /* 时间显示格式: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    AT_GetLiveTime(mmInfo, &localAtTimeInfo, second);

    netCtx = AT_GetModemNetCtxAddrFromModemId(MODEM_ID_0);
    /* 1 使用简写模式 */
    if (netCtx->csdfCfg.auxMode == 1) {
    /* "yy/mm/dd,hh:mm:ss(+/-)zz" */
        year = localAtTimeInfo.universalTimeandLocalTimeZone.year;
    } else {
        /* "yyyy/mm/dd,hh:mm:ss(+/-)zz" */
        if (localAtTimeInfo.universalTimeandLocalTimeZone.year > AT_GMT_TIME_DEFAULT) {
            year = 1900 + localAtTimeInfo.universalTimeandLocalTimeZone.year; /* 1900 基准年份 */
        } else {
            year = 2000 + localAtTimeInfo.universalTimeandLocalTimeZone.year; /* 2000 基准年份 */
        }
    }

    /* YY/MM/DD,HH:MM:SS */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"%02d/%02d/%02d,%02d:%02d:%02d", pcCmd, year,
        localAtTimeInfo.universalTimeandLocalTimeZone.month, localAtTimeInfo.universalTimeandLocalTimeZone.day,
        localAtTimeInfo.universalTimeandLocalTimeZone.hour, localAtTimeInfo.universalTimeandLocalTimeZone.minute,
        localAtTimeInfo.universalTimeandLocalTimeZone.second);

    /* 获得时区 */
    if ((localAtTimeInfo.ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
        timeZone = localAtTimeInfo.localTimeZone;
    } else {
        timeZone = localAtTimeInfo.universalTimeandLocalTimeZone.timeZone;
    }

    if (timeZone != AT_INVALID_TZ_VALUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%02d", (timeZone < 0) ? "-" : "+",
            (timeZone < 0) ? (-timeZone) : timeZone);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"");

    return length;
}
#endif

TAF_UINT32 At_PhReadCreg(TAF_PH_RegState *para, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if ((para->act == TAF_PH_ACCESS_TECH_CDMA_1X) || (para->act == TAF_PH_ACCESS_TECH_EVDO)) {
        /* lac */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, ",\"FFFF\"");
        /* ci */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, ",\"FFFFFFFF\"");

        if ((g_reportCregActParaFlg == VOS_TRUE) && (para->act < TAF_PH_ACCESS_TECH_BUTT)) {
            /* act */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",%d", para->act);
        }

        return length;
    }
#endif

    if (para->cellId.cellNum > 0) {
        /* lac */
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (para->act == TAF_PH_ACCESS_TECH_NR_5GC) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",\"%X%X%X%X%X%X\"", 0x000f & (para->lac >> 20), 0x000f & (para->lac >> 16),
                0x000f & (para->lac >> 12), 0x000f & (para->lac >> 8), 0x000f & (para->lac >> 4),
                0x000f & (para->lac >> 0));
        } else
#endif
        {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",\"%X%X%X%X\"", 0x000f & (para->lac >> 12), 0x000f & (para->lac >> 8),
                0x000f & (para->lac >> 4), 0x000f & (para->lac >> 0));
        }

        /* ci */
        if (g_ciRptByte == CREG_CGREG_CI_RPT_FOUR_BYTE) {
            /* VDF需求: CREG/CGREG的<CI>域以4字节方式上报 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",\"%08X\"", (para->cellId.cellId[0].cellIdLowBit));
        } else {
            /* <CI>域以2字节方式上报 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",\"%X%X%X%X\"",
                0x000f & (para->cellId.cellId[AT_TAFMMA_CELL_ID_INDEX_0].cellIdLowBit >> 12),
                0x000f & (para->cellId.cellId[AT_TAFMMA_CELL_ID_INDEX_0].cellIdLowBit >> 8),
                0x000f & (para->cellId.cellId[AT_TAFMMA_CELL_ID_INDEX_0].cellIdLowBit >> 4),
                0x000f & (para->cellId.cellId[AT_TAFMMA_CELL_ID_INDEX_0].cellIdLowBit >> 0));
        }

        if ((g_reportCregActParaFlg == VOS_TRUE) && (para->act < TAF_PH_ACCESS_TECH_BUTT)) {
            /* rat */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",%d", para->act);
        }
    }

    return length;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

TAF_UINT32 At_PhReadC5greg(TAF_PH_RegState *para, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    if (para->cellId.cellNum > 0) {
        /* tac */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, ",\"%X%X%X%X%X%X\"", 0x000f & (para->lac >> 20), 0x000f & (para->lac >> 16),
            0x000f & (para->lac >> 12), 0x000f & (para->lac >> 8), 0x000f & (para->lac >> 4),
            0x000f & (para->lac >> 0));

        /* ci */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, ",\"%08X%08X\"", para->cellId.cellId[0].cellIdHighBit,
            para->cellId.cellId[0].cellIdLowBit);

        if (g_reportCregActParaFlg == VOS_TRUE) {
            /* rat */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",%d", para->act);
        } else {
            /* rat */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, ",");
        }
    }

    return length;
}
#endif

/* 填充XLEMA参数<index>,<total>,<ecc_num>,<category>,<simpresent>, <mcc>,<mnc>,<abnormal_service>,<urns>,<source> */
VOS_UINT16 AT_FormatOneXlemaPara(MN_CALL_CustomEccNum *customEccNum, VOS_UINT32 eccNumCount, VOS_UINT32 xlemaFirstPara)
{
    VOS_UINT8          asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];
    VOS_UINT8          subServiceStr[MN_CALL_MAX_SUB_SERVICE_NUM_LEN + 1];
    VOS_UINT32         mcc = 0;
    VOS_UINT32         mnc = 0;
    errno_t            memResult;
    VOS_UINT16         length = 0;

    (VOS_VOID)memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));
    (VOS_VOID)memset_s(subServiceStr, sizeof(subServiceStr), 0x00, sizeof(subServiceStr));

    /* 将BCD码转化为ASCII码 */
    AT_BcdNumberToAscii(customEccNum->eccNum, AT_MIN(customEccNum->eccNumLen, MN_CALL_MAX_BCD_NUM_LEN),
        (VOS_CHAR *)asciiNum);

    /* 将NAS类型的MCC转化为BCD格式 */
    AT_ConvertNasMccToBcdType(customEccNum->mcc, &mcc);
    /* 将NAS类型的MNC转化为BCD格式 */
    AT_ConvertNasMncToBcdType(customEccNum->mnc, &mnc);

    memResult = memcpy_s(subServiceStr, sizeof(subServiceStr), customEccNum->urInfo.subServicesNum,
        AT_MIN(customEccNum->urInfo.subServicesLen, MN_CALL_MAX_SUB_SERVICE_NUM_LEN));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(subServiceStr),
        AT_MIN(customEccNum->urInfo.subServicesLen, MN_CALL_MAX_SUB_SERVICE_NUM_LEN));

    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s^XLEMA:", g_atCrLf);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,%d,%s,%d,%d,", (xlemaFirstPara + 1), eccNumCount, asciiNum,
            customEccNum->category, customEccNum->validSimPresent);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x,", (mcc & AT_MCC_DIGIT3_MASK) >> AT_MCC_DIGIT3_OFFSET,
        (mcc & AT_MCC_DIGIT2_MASK) >> AT_MCC_DIGIT2_OFFSET, (mcc & AT_MCC_DIGIT1_MASK));

    if ((mnc & AT_MNC_DIGIT3_MASK) == AT_MNC_DIGIT3_MASK) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x", (mnc & AT_MNC_DIGIT2_MASK) >> AT_MNC_DIGIT2_OFFSET,
            (mnc & AT_MNC_DIGIT1_MASK));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x", (mnc & AT_MNC_DIGIT3_MASK) >> AT_MNC_DIGIT3_OFFSET,
            (mnc & AT_MNC_DIGIT2_MASK) >> AT_MNC_DIGIT2_OFFSET, (mnc & AT_MNC_DIGIT1_MASK));
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d,\"%s\",%d%s", customEccNum->abnormalServiceFlg, subServiceStr,
        customEccNum->source, g_atCrLf);

    return length;
}

VOS_UINT16 AT_PrintRejinfo(VOS_UINT16 length, TAF_PH_RegRejInfo *rejinfo)
{
    VOS_UINT16 len = length;

    len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + len, "%x%x%x", (rejinfo->plmnId.mcc & 0x0f00) >> 8,
        (rejinfo->plmnId.mcc & 0xf0) >> 4, (rejinfo->plmnId.mcc & 0x0f));

    if ((rejinfo->plmnId.mnc & 0x0f00) == 0x0f00) {
        len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%x%x,", (rejinfo->plmnId.mnc & 0xf0) >> 4,
            (rejinfo->plmnId.mnc & 0x0f));
    } else {
        len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%x%x%x,", (rejinfo->plmnId.mnc & 0x0f00) >> 8,
            (rejinfo->plmnId.mnc & 0xf0) >> 4, (rejinfo->plmnId.mnc & 0x0f));
    }

    len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + len, "%d,%d,%d,%d,%d,", rejinfo->srvDomain, rejinfo->rejCause, rejinfo->rat,
        rejinfo->rejType, rejinfo->originalRejCause);

    /* Lac */
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (rejinfo->rat == TAF_MMA_REJ_RAT_NR) {
        len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "\"%X%X%X%X%X%X\",", 0x000f & (rejinfo->lac >> 20),
            0x000f & (rejinfo->lac >> 16), 0x000f & (rejinfo->lac >> 12), 0x000f & (rejinfo->lac >> 8),
            0x000f & (rejinfo->lac >> 4), 0x000f & (rejinfo->lac >> 0));
    } else
#endif
    {
        len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "\"%X%X%X%X\",", 0x000f & (rejinfo->lac >> 12),
            0x000f & (rejinfo->lac >> 8), 0x000f & (rejinfo->lac >> 4), 0x000f & (rejinfo->lac >> 0));
    }

    /* Rac */
    len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + len, "\"%X%X\",", 0x000f & (rejinfo->rac >> 4), 0x000f & (rejinfo->rac >> 0));

    /* <CI>域以8字节方式上报 */
    len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + len, "\"%08X%08X\"", rejinfo->cellId.cellIdHighBit,
        rejinfo->cellId.cellIdLowBit);

    /* ESM cause */
    if (rejinfo->pdnRejCause != 0) {
        len += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, ",%d", rejinfo->pdnRejCause);
    }

    return len;
}

VOS_UINT16 AT_PrintNwTimeInfo(NAS_MM_InfoInd *mmInfo, CONST_T VOS_CHAR *pcCmd, VOS_UINT32 nwSecond)
{
    VOS_INT8       timeZone;
    VOS_UINT8      dst;
    VOS_UINT16     length = 0;
    NAS_MM_InfoInd localAtTimeInfo;

    (VOS_VOID)memset_s(&localAtTimeInfo, sizeof(NAS_MM_InfoInd), 0x00, sizeof(NAS_MM_InfoInd));

    /* 时间显示格式: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        AT_GetLiveTime(mmInfo, &localAtTimeInfo, nwSecond);

        /* YY/MM/DD,HH:MM:SS */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d%d/%d%d/%d%d,%d%d:%d%d:%d%d", (VOS_CHAR *)pcCmd,
            localAtTimeInfo.universalTimeandLocalTimeZone.year / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.year % 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.month / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.month % 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.day / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.day % 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.hour / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.hour % 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.minute / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.minute % 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.second / 10,
            localAtTimeInfo.universalTimeandLocalTimeZone.second % 10);
        /* 获得时区 */
        if ((localAtTimeInfo.ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
            timeZone = localAtTimeInfo.localTimeZone;
        } else {
            timeZone = localAtTimeInfo.universalTimeandLocalTimeZone.timeZone;
        }

        if (timeZone != AT_INVALID_TZ_VALUE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%02d", (timeZone < 0) ? "-" : "+",
                (timeZone < 0) ? (-timeZone) : timeZone);
        }

        /* 显示夏时制或冬时制信息 */
        if (((localAtTimeInfo.ieFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST) && (localAtTimeInfo.dst > 0)) {
            dst = localAtTimeInfo.dst;
        } else {
            dst = 0;
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d%d", dst / 10, dst % 10);
    }
    return length;
}

