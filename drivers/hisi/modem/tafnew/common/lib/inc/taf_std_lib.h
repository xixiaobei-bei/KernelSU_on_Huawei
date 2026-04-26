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
#ifndef _TAF_STD_LIB_H_
#define _TAF_STD_LIB_H_

#include "PsTypeDef.h"
#include "mn_msg_api.h"
#include "mn_error_code.h"
#include "taf_encode_decode_lib.h"
#include "nas_comm_packet_ser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
typedef VOS_INT32(*TAF_STD_Cmpfun)(const VOS_VOID *record1, const VOS_VOID *record2);
#define HALF_NUMBER 2
#endif

#define TAF_STD_SSC_MAX_LEN 256
#define TAF_STD_7BIT_MASK 0x7f

#define TAF_STD_MAX_GSM7BITDEFALPHA_NUM 128
#define TAF_STD_NOSTANDARD_ASCII_CODE 0xff
#define TAF_STD_GSM_7BIT_EXTENSION_FLAG 0xfe

#define TAF_STD_UINT8_MAX 255

/*
 * This is the number of days in a leap year set.
 * A leap year set includes 1 leap year, and 3 normal years.
 */
#define TAF_STD_TIME_ZONE_QUAD_YEAR (366 + (3 * 365))

/* 5 days (duration between Jan 1 and Jan 6), expressed as seconds. */

#define TAF_STD_TIME_ZONE_OFFSET_S (432000UL)

/*
 * This is the year upon which all time values used by the Clock Services
 * are based.  NOTE:  The user base day (GPS) is Jan 6 1980, but the internal
 * base date is Jan 1 1980 to simplify calculations
 */

#define TAF_STD_TIME_ZONE_BASE_YEAR 1980

#define TAF_STD_TIME_ZONE_MAX_YEAR 2100

#define TAF_STD_TIME_ZONE_YEAR_DIVISOR 100

#define TAF_STD_TIME_ZONE_YEAR_FAIL 80 /* 获取时区信息失败时候，以80/01/06,00:00:00,+0,00上报 */

#define TAF_STD_TIME_ZONE_MONTH_FAIL 1 /* 获取时区信息失败时候，以80/01/06,00:00:00,+0,00上报 */

#define TAF_STD_TIME_ZONE_DAY_FAIL 6 /* 获取时区信息失败时候，以80/01/06,00:00:00,+0,00上报 */

#define TAF_STD_SECONDS_PER_MINUTE 60 /* 60s */

#define TAF_STD_MINUTES_PER_HOUR 60 /* 60m */

#define TAF_STD_HOURS_PER_DAY 24 /* 24HOUR */

#define TAF_STD_NORMAL_MONTH_TAB_COUNT 13

#define TAF_STD_DAYS_ELAPSED_OF_A_LEAP_YEAR_SET_TAB_COUNT 5

#define TAF_STD_INVALID_MCC 0xFFFFFFFF

#define TAF_STD_BIT_LEN_1_BIT 1
#define TAF_STD_BIT_LEN_2_BIT 2
#define TAF_STD_BIT_LEN_3_BIT 3
#define TAF_STD_BIT_LEN_4_BIT 4
#define TAF_STD_BIT_LEN_5_BIT 5
#define TAF_STD_BIT_LEN_6_BIT 6
#define TAF_STD_BIT_LEN_7_BIT 7
#define TAF_STD_BIT_LEN_8_BIT 8
#define TAF_STD_BIT_LEN_32_BIT 32

#define TAF_STD_BIT_TO_BYTE(pBuf, bOffSet, bLen)                            \
    TAF_STD_ExtractBitStringToOctet(&pBuf[bOffSet / TAF_STD_BIT_LEN_8_BIT], \
                                    (VOS_UINT8)bOffSet % TAF_STD_BIT_LEN_8_BIT, bLen)

#define TAF_STD_BIT_TO_DWORD(pBuf, bOffSet, bLen)                           \
    TAF_STD_ExtractBitStringToDword(&pBuf[bOffSet / TAF_STD_BIT_LEN_8_BIT], \
                                    (VOS_UINT8)bOffSet % TAF_STD_BIT_LEN_8_BIT, bLen)

#define TAF_STD_UTF8_1BYTE_MAX 0x7F
#define TAF_STD_UTF8_2BYTE_MAX 0x7FF
#define TAF_STD_UTF8_3BYTE_MAX 0xFFFF
/* SSA解码错误码起始值，区分于taf通用错误码 */
#define TAF_SS_DECODE_ERR_BASE 0x1000

#define TAF_STD_LEAP_YEAR_DAYS 366
#define TAF_STD_NORMAL_YEAR_DAYS 365
#define TAF_STD_DAY_TO_SECOND (24 * 60 * 60)
#define TAF_STD_HOUR_TO_SECOND (60 * 60)
#define TAF_STD_MIN_TO_SECOND 60
#define TAF_STD_UTC_START_YEAR 1900
#define TAF_STD_MAX_MONTH_DAY_COUNT 13
#define TAF_STD_MAX_DATE_TO_SECOND 0x17830D57F /* 2099/12/31 23:59:59 */
#define TAF_STD_UPCASE(c) ((((c) >= 'a') && ((c) <= 'z')) ? ((c)-0x20) : (c))

#define TAF_STD_SLICE_TO_SECOND(slice) ((slice) >> 15)
#define TAF_STD_UINT64_MAX (0xFFFFFFFFFFFFFFFFULL)
#define TAF_STD_KBYTES_TO_BYTES(kbytes) ((kbytes) << 10)
#define TAF_STD_BYTE_TO_BITS(byte) ((byte) << 3)
#define TAF_STD_BITS_TO_KBITS(bits) ((bits) >> 10)
#define TAF_STD_BYTES_TO_KBITS(bytes) ((bytes) >> 7)
#define TAF_STD_BYTES_TO_KBYTES(bytes) ((bytes) >> 10)
#define TAF_STD_BITS_PER_UINT 32


enum TAF_STD_XmlCoding {
    TAF_STD_XML_CODING_UTF8,
    TAF_STD_XML_CODING_UTF16,
    TAF_STD_XML_CODING_UTF32,
    TAF_STD_XML_CODING_BUTT
};
typedef VOS_UINT8 TAF_STD_XmlCodingUint8;

enum TAF_STD_EncodingType {
    TAF_STD_ENCODING_7BIT = TAF_COMM_ENCODING_7BIT,
    TAF_STD_ENCODING_8BIT = TAF_COMM_ENCODING_8BIT,
    TAF_STD_ENCODING_UCS2 = TAF_COMM_ENCODING_UCS2,
    TAF_STD_ENCODING_UTF8 = TAF_COMM_ENCODING_UTF8,
    TAF_STD_ENCODING_UTF16 = TAF_COMM_ENCODING_UTF16,
    TAF_STD_ENCODING_UTF32 = TAF_COMM_ENCODING_UTF32,
    TAF_STD_ENCODING_BUTT
};
typedef TAF_COMM_EncodingTypeUint8 TAF_STD_EncodingTypeUint8;


typedef struct {
    /* Year [1980..2100) */
    VOS_UINT16 year;

    /* Month of year [1..12] */
    VOS_UINT16 month;

    /* Day of month [1..31] */
    VOS_UINT16 day;

    /* Hour of day [0..23] */
    VOS_UINT16 hour;

    /* Minute of hour [0..59] */
    VOS_UINT16 minute;

    /* Second of minute [0..59] */
    VOS_UINT16 second;

} TAF_STD_TimeZoneType;


typedef struct {
    VOS_UINT32 highTmp;
    VOS_UINT32 lowTmp;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 remainder;
} TAF_STD_TmpTimeZoneConvert;

typedef TAF_COMM_StrWithEncodingType TAF_STD_StrWithEncodingType;


typedef struct {
    TAF_STD_XmlCodingUint8 coding;
    VOS_UINT8              reserved1[3];
    VOS_UINT32             len;
    VOS_UINT8             *str;
} TAF_STD_XmlStr;

VOS_UINT32 TAF_STD_Itoa(VOS_UINT32 digit, VOS_CHAR *pcDigitStr, VOS_UINT32 stringlength, VOS_UINT32 *printLength);

VOS_UINT32 TAF_STD_AsciiNum2HexString(VOS_UINT8 *src, VOS_UINT16 *srcLen);

VOS_UINT16 TAF_STD_HexAlpha2AsciiString(VOS_UINT8 *src, VOS_UINT16 srcLen, VOS_UINT8 *dst);
VOS_UINT32 TAF_STD_ConvertStrToDecInt(VOS_UINT8 *src, VOS_UINT32 srcLen, VOS_UINT32 *dec);
/* 将7bit编码方式的字符转换为8bit字符 */
VOS_UINT32 TAF_STD_UnPack7Bit(const VOS_UINT8 *orgChar, VOS_UINT32 len, VOS_UINT8 fillBit, VOS_UINT8 *unPackedChar);

/* 将字符转换为7bit编码方式 */
VOS_UINT32 TAF_STD_Pack7Bit(const VOS_UINT8 *orgChar, VOS_UINT32 len, VOS_UINT8 fillBit, VOS_UINT8 *packedChar,
                            VOS_UINT32 *lenValue);

VOS_UINT32 TAF_STD_ConvertBcdNumberToAscii(const VOS_UINT8 *bcdNumber, VOS_UINT8 bcdLen, VOS_CHAR *pcAsciiNumber,
                                           VOS_UINT32 asciiLen);
VOS_UINT32 TAF_STD_ConvertBcdCodeToAscii(VOS_UINT8 bcdCode, VOS_CHAR *pcAsciiCode);
VOS_UINT32 TAF_STD_ConvertAsciiNumberToBcd(const VOS_CHAR *pcAsciiNumber, VOS_UINT8 *bcdNumber, VOS_UINT8 *bcdLen);
VOS_UINT32 TAF_STD_ConvertAsciiAddrToBcd(const MN_MSG_AsciiAddr *asciiAddr, MN_MSG_BcdAddr *bcdAddr);
VOS_UINT32 TAF_STD_ConvertAsciiCodeToBcd(VOS_CHAR asciiCode, VOS_UINT8 *bcdCode);
VOS_UINT8  TAF_STD_ConvertDeciDigitToBcd(VOS_UINT8 deciDigit, VOS_BOOL bReverseOrder);
VOS_UINT32 TAF_STD_ConvertBcdToDeciDigit(VOS_UINT8 bcdDigit, VOS_BOOL bReverseOrder, VOS_UINT8 *digit);
VOS_UINT32 TAF_STD_ConvertAsciiToDefAlpha(const VOS_UINT8 *asciiChar, VOS_UINT32 len, VOS_UINT8 *defAlpha,
                                          VOS_UINT32 *defAlphaLen, VOS_UINT32 defAlphaBuffLen);
VOS_VOID   TAF_STD_ConvertDefAlphaToAscii(const VOS_UINT8 *defAlpha, VOS_UINT32 defAlphaLen, VOS_UINT8 *asciiChar,
                                          VOS_UINT32 *asciiCharLen);

VOS_UINT32 TAF_STD_ConvertBcdCodeToDtmf(VOS_UINT8 bcdCode, VOS_UINT8 *dtmfCode);

VOS_UINT32 TAF_STD_ConvertBcdNumberToDtmf(const VOS_UINT8 *bcdNumber, VOS_UINT8 bcdLen, VOS_UINT8 *dtmfNumber,
                                          VOS_UINT8 dtmfLen);

extern VOS_UINT32 TAF_STD_ConvertTimeFromSecsToTimeZone(VOS_UINT32 highSystemTime, VOS_UINT32 lowSystemTime,
                                                        TAF_STD_TimeZoneType *universalTimeandLocalTimeZone);

extern VOS_VOID TAF_STD_ConvertSystemTimeToHighLow(VOS_UINT8 *sysTimeByte, VOS_UINT32 *highSystemTime,
    VOS_UINT32 *lowSystemTime);

extern VOS_UINT32 TAF_STD_64Add32(VOS_UINT32 highAddend, VOS_UINT32 lowAddend, VOS_UINT32 addFactor,
                                  VOS_UINT32 *highRslt, VOS_UINT32 *lowRslt);

extern VOS_UINT32 TAF_STD_64Sub32(VOS_UINT32 highMinuend, VOS_UINT32 lowMinuend, VOS_UINT32 subFactor,
                                  VOS_UINT32 *highRslt, VOS_UINT32 *lowRslt);

VOS_VOID TAF_STD_U64ToStr(VOS_UINT32 productHigh, VOS_UINT32 productLow, VOS_UINT8 *string);

VOS_UINT16 TAF_STD_TransformBcdMccToDeciDigit(VOS_UINT32 bcdMcc);

VOS_UINT8 TAF_STD_TransformBcdImsi1112ToDeciDigit(VOS_UINT16 bcdImsi1112);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT16 TAF_STD_TransformCLBcdMncToDeciDigit(VOS_UINT16 bcdMnc);
#endif
VOS_UINT32 TAF_STD_TransformDeciDigitToBcdMcc(VOS_UINT32 deciDigitMcc);

VOS_UINT16 TAF_STD_TransformBcdMncToDeciDigit(VOS_UINT32 bcdMnc);

VOS_UINT8 TAF_STD_ExtractBitStringToOctet(VOS_UINT8 *srcAddr, VOS_UINT8 offsetPos, VOS_UINT8 bitLen);

VOS_UINT32 TAF_STD_ExtractBitStringToDword(VOS_UINT8 *srcAddr, VOS_UINT8 offsetPos, VOS_UINT8 bitLen);

VOS_UINT32 TAF_STD_ConvertStrEncodingType(TAF_STD_StrWithEncodingType *srcStr, TAF_STD_EncodingTypeUint8 dstCoding,
                                          VOS_UINT32 dstBuffLen, TAF_STD_StrWithEncodingType *dstStr);

VOS_UINT32 TAF_STD_IsLeapYear(VOS_UINT16 year);
VOS_UINT32 TAF_STD_GetDaysForYear(VOS_UINT16 year);
VOS_VOID   TAF_STD_SecondTimeToDateTime(VOS_UINT64 secondTime, TAF_STD_TimeZoneType *dateTime);
VOS_UINT64 TAF_STD_DateTimeToSecondTime(TAF_STD_TimeZoneType *dateTime);
VOS_VOID TAF_STD_Get64BitSlice(VOS_UINT32 *highBitValue, VOS_UINT32 *lowBitValue);
VOS_BOOL   TAF_STD_TestUintBit(VOS_UINT8 bitNum, VOS_UINT32 bitMaskValue);
VOS_VOID   TAF_STD_SetUintBit(VOS_UINT8 bitNum, VOS_UINT32 *bitMaskValue);
VOS_VOID   TAF_STD_ClearUintBit(VOS_UINT8 bitNum, VOS_UINT32 *bitMaskValue);
VOS_UINT8  TAF_STD_GetUintBitNum(VOS_UINT32 bitMaskValue);
VOS_UINT32 TAF_STD_IsTwoSNssaiMatch(const PS_S_NSSAI_STRU *sNssai1, const PS_S_NSSAI_STRU *sNssai2);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of taf_spm_ctx.h */
