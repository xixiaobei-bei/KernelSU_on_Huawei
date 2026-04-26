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

#include "PsTypeDef.h"
#include "ps_common_def.h"
#include "mn_error_code.h"
#include "mn_msg_api.h"
#include "taf_msg_ts.h"
#include "taf_std_lib.h"
#include "securec.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "nas_multi_instance_api.h"
#endif


VOS_UINT32 MSG_EncodeUserData(MN_MSG_MsgCodingUint8 msgCoding, MN_MSG_UserData *userData, VOS_UINT8 *userDataInfo,
                              VOS_UINT32 maxMemLength, VOS_UINT32 *len);

VOS_UINT32 MSG_EncodeUserDataByCoding7BIT(MN_MSG_UserData *userData, VOS_UINT8 *userDataInfo, VOS_UINT32 maxMemLength,
                                          VOS_UINT32 *len);

#define THIS_FILE_ID PS_FILE_ID_MNMSG_ENCODE_C

VOS_UINT32 MN_MSG_ChkDate(const MN_MSG_Timestamp *timeStamp, MN_MSG_DateInvalidTypeUint8 *invalidType)
{
    VOS_UINT8                   year;
    VOS_UINT8                   month;
    VOS_UINT8                   day;
    VOS_UINT8                   hour;
    VOS_UINT8                   minute;
    VOS_UINT8                   second;
    MN_MSG_DateInvalidTypeUint8 invalidTypeTemp = 0;
    VOS_UINT32                  ret;

    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->year, VOS_FALSE, &year);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MN_MSG_ChkDate: Month is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_YEAR;
    }

    /* 将BCD码表示的月日小时分钟秒转换成十进制数进行有效性检查； */
    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->month, VOS_FALSE, &month);
    if ((ret != MN_ERR_NO_ERROR) || (month > MN_MSG_MONTHS_IN_A_YEAR)) {
        MN_WARN_LOG("MN_MSG_ChkDate: Month is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_MONTH;
    }

    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->day, VOS_FALSE, &day);
    if ((ret != MN_ERR_NO_ERROR) || (day > MN_MSG_MAX_DAYS_IN_A_MONTH)) {
        MN_WARN_LOG("MN_MSG_ChkDate: Date is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_DAY;
    }

    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->hour, VOS_FALSE, &hour);
    if ((ret != MN_ERR_NO_ERROR) || (hour > (MN_MSG_HOURS_IN_A_DAY - 1))) {
        MN_WARN_LOG("MN_MSG_ChkDate: Hour is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_HOUR;
    }

    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->minute, VOS_FALSE, &minute);
    if ((ret != MN_ERR_NO_ERROR) || (minute > (MN_MSG_MINUTES_IN_AN_HOUR - 1))) {
        MN_WARN_LOG("MN_MSG_ChkDate: Minute is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_MINUTE;
    }

    ret = TAF_STD_ConvertBcdToDeciDigit(timeStamp->second, VOS_FALSE, &second);
    if ((ret != MN_ERR_NO_ERROR) || (second > (MN_MSG_SECONDS_IN_A_MINUTE - 1))) {
        MN_WARN_LOG("MN_MSG_ChkDate: Second is invalid.");
        invalidTypeTemp |= MN_MSG_DATE_INVALID_SECOND;
    }

    *invalidType = invalidTypeTemp;
    if (invalidTypeTemp == 0) {
        return MN_ERR_NO_ERROR;
    } else {
        return MN_ERR_CLASS_SMS_INVALID_DATE;
    }
}

LOCAL VOS_UINT32 MSG_EncodeTimeStamp(const MN_MSG_Timestamp *timeStamp, VOS_UINT8 *validPeriod, VOS_UINT32 *len)
{
    VOS_UINT8  absTimeZone;
    VOS_UINT8  algebraicSign;
    VOS_UINT8  dateInvalidType;
    VOS_UINT32 ret;

    if ((timeStamp == VOS_NULL_PTR) || (validPeriod == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeTimeStamp: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    ret = MN_MSG_ChkDate(timeStamp, &dateInvalidType);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_EncodeTimeStamp: Date is invalid.");
    }

    /* 时区有效性检查，根据协议27005 <dt>示例可知(1hour <-> 4)必须在-48-48之间； */
    if ((timeStamp->timezone > MN_MSG_MAX_TIMEZONE_VALUE) ||
        (timeStamp->timezone < ((-1) * MN_MSG_MAX_TIMEZONE_VALUE))) {
        MN_WARN_LOG("MSG_EncodeTimeStamp: Time zone is invalid.");
    }

    /*
     * 根据协议23040 9.2.3.11和9.1.2.3 在VP中BCD码是反序放置的，
     * 所以将结构中BCD码反序后赋给输出
     */
    MN_MSG_REVERSE_BCD(validPeriod[0], timeStamp->year);
    MN_MSG_REVERSE_BCD(validPeriod[1], timeStamp->month);
    MN_MSG_REVERSE_BCD(validPeriod[2], timeStamp->day);
    MN_MSG_REVERSE_BCD(validPeriod[3], timeStamp->hour);
    MN_MSG_REVERSE_BCD(validPeriod[4], timeStamp->minute);
    MN_MSG_REVERSE_BCD(validPeriod[5], timeStamp->second);

    /* 时区在协议23040 9.2.3.11中需要将绝对值转换成反序BCD码再或上表示负数的符号 */
    if (timeStamp->timezone < 0) {
        absTimeZone   = (VOS_UINT8)((-1) * timeStamp->timezone);
        algebraicSign = MN_MSG_NEGATIVE_ALGEBRAICSIGN;
    } else {
        absTimeZone   = (VOS_UINT8)timeStamp->timezone;
        algebraicSign = 0x00;
    }
    validPeriod[6] = algebraicSign;
    validPeriod[6] |= TAF_STD_ConvertDeciDigitToBcd(absTimeZone, VOS_TRUE);

    *len = MN_MSG_ABSOLUTE_TIMESTAMP_LEN;
    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeValidPeriod(const MN_MSG_ValidPeriod *validPeriod, VOS_UINT8 *validPeriodInfo,
                                       VOS_UINT32 *len)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    if ((validPeriod == VOS_NULL_PTR) || (validPeriodInfo == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeValidPeriod: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    switch (validPeriod->validPeriod) {
        case MN_MSG_VALID_PERIOD_NONE:
            *len = 0;
            break;
        case MN_MSG_VALID_PERIOD_RELATIVE:
            *len             = 1;
            *validPeriodInfo = validPeriod->u.otherTime;
            break;
        case MN_MSG_VALID_PERIOD_ABSOLUTE:
            ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(validPeriod->u.absoluteTime), validPeriodInfo, len);
            break;
        default:
            MN_WARN_LOG("MSG_EncodeValidPeriod: The type of valid period is not supported!");
            return MN_ERR_CLASS_SMS_INVALID_VPF;
    }

    return ret;
}

VOS_UINT32 MN_MSG_EncodeRelatTime(const MN_MSG_Timestamp *relatTime, VOS_UINT8 *relatTimeInfo)
{
    VOS_UINT16 totalDay;
    VOS_UINT32 totalHour;
    VOS_UINT32 totalMinute;
    VOS_UINT8  dateInvalidType;
    VOS_UINT32 ret;

    if ((relatTime == VOS_NULL_PTR) || (relatTimeInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_EncodeRelatTime: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /* 将BCD码表示的月日小时分钟秒转换成十进制数进行有效性检查； */
    ret = MN_MSG_ChkDate(relatTime, &dateInvalidType);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    /* 将pstRelatTime转换成usTotalDay和ucTotalMinute */
    totalDay = (VOS_UINT16)(MN_MSG_DAYS_IN_A_YEAR * relatTime->year);
    totalDay += (VOS_UINT16)(MN_MSG_DAYS_IN_A_MONTH * relatTime->month);
    totalDay += relatTime->day;

    /* TP-VP最多只能表示441天的有效期 */
    if (totalDay > MN_MSG_MAX_RELAT_VP_DAYS) {
        MN_WARN_LOG("MN_MSG_EncodeRelatTime: Parameter of the function is invalid.");
        return MN_ERR_CLASS_SMS_INVALID_RELATTTIME;
    }

    /* 197～255：（VP–192）*1 周，表示时间范围为31天(约为5周) － 441天(196-166) */
    if (totalDay > MN_MSG_DAYS_IN_A_MONTH) {
        *relatTimeInfo = (VOS_UINT8)(((totalDay + 6) / 7) + 192);
        return MN_ERR_NO_ERROR;
    }

    /* 168～196：（VP–166）*1 日,表示时间范围为2(168-166)天 － 30天(196-166) */
    if (totalDay > 1) {
        *relatTimeInfo = (VOS_UINT8)(totalDay + 166);
        return MN_ERR_NO_ERROR;
    }

    totalHour   = (MN_MSG_HOURS_IN_A_DAY * totalDay) + relatTime->hour;
    totalMinute = MN_MSG_MINUTES_IN_AN_HOUR * totalHour;
    totalMinute += relatTime->minute;

    if (totalMinute < 1) {
        *relatTimeInfo = 0;
        return MN_ERR_NO_ERROR;
    }

    /* 144～167：12 小时+（VP–143）*30 分钟,表示时间范围为12小时 -> 1天 */
    if (totalMinute > (12 * MN_MSG_MINUTES_IN_AN_HOUR)) {
        *relatTimeInfo = (VOS_UINT8)((((totalMinute - (12 * MN_MSG_MINUTES_IN_AN_HOUR)) + 29) / 30) + 143);
        return MN_ERR_NO_ERROR;
    }

    /* 0～143：（VP+1）* 5 分钟,表示时间范围为5分钟 － 12小时 */
    *relatTimeInfo = (VOS_UINT8)(((totalMinute + 4) / 5) - 1);
    return MN_ERR_NO_ERROR;
}

VOS_UINT32 TAF_MSG_EncodeAddressPreCheck(const MN_MSG_AsciiAddr *asciiAddr, VOS_BOOL bRpAddr, VOS_UINT8 *addr,
                                         VOS_UINT32 *len)
{
    if ((asciiAddr == VOS_NULL_PTR) || (addr == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        MN_ERR_LOG("TAF_MSG_EncodeAddressPreCheck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (asciiAddr->len == 0) {
        if (bRpAddr == VOS_TRUE) {
            *addr = 0;
            *len  = 1;
        } else {
            *addr       = 0;
            *(addr + 1) = 0;
            *len        = 2;
        }
        return MN_ERR_NO_ERROR;
    }

    return VOS_TRUE;
}

VOS_UINT32 MN_MSG_EncodeAddress(const MN_MSG_AsciiAddr *asciiAddr, VOS_BOOL bRpAddr, VOS_UINT8 *addr, VOS_UINT32 *len)
{
    VOS_UINT32 numLen;             /* 号码长度 */
    VOS_UINT8 *num = VOS_NULL_PTR; /* 指向实际号码（不包括+号）的指针 */
    VOS_UINT32 pos = 0;
    VOS_UINT8  addrBcdLen;
    VOS_UINT32 alphaNumLen = 0;
    VOS_UINT32 ret;

    ret = TAF_MSG_EncodeAddressPreCheck(asciiAddr, bRpAddr, addr, len);
    if (ret != VOS_TRUE) {
        return ret;
    }

    /* 如果号码的首字符是'+'号，表示国际号码 */
    if (asciiAddr->asciiNum[0] == '+') {
        if (asciiAddr->numType != MN_MSG_TON_INTERNATIONAL) {
            MN_WARN_LOG("MN_MSG_EncodeAddress: The address string begins with '+'"
                        "while number type indicates non-international number type");
            return MN_ERR_CLASS_SMS_INVALID_NUMTYPE;
        }
        num    = (VOS_UINT8 *)&(asciiAddr->asciiNum[1]);
        numLen = asciiAddr->len - 1;
    } else {
        num    = (VOS_UINT8 *)asciiAddr->asciiNum;
        numLen = asciiAddr->len;
    }

    /*
     * 号码长度应该在范围1 - 20之间，参考协议24011 8.2.5.1和8.2.5.2
     * 23040 9.2.2 和 9.1.2.5
     */
    if ((numLen > MN_MSG_MAX_ADDR_LEN) || (numLen < MN_MSG_MIN_ADDR_LEN)) {
        MN_WARN_LOG("MN_MSG_EncodeAddress: The length of address is invalid.");
        return MN_ERR_CLASS_SMS_INVALID_ADDRLEN;
    }

    /*
     * 第0个字节为号码（包括号码类型）的字节长度:
     * 根据协议24011 8.2.5.1 8.2.5.2 和24008 10.5.4.9 RP层的地址IE中长度指BCD号码长度，包含地址类型的占位
     * 根据协议23040 9.1.2.5 TP层的地址长度为有效号码字符个数
     */
    if (bRpAddr == VOS_TRUE) {
        addr[pos] = (VOS_UINT8)(((numLen + 1) / 2) + 1);
        pos++;
    } else {
        if (asciiAddr->numType == MN_MSG_TON_ALPHANUMERIC) {
            addr[pos] = (VOS_UINT8)(((numLen * 7) + 3) / 4);
            pos++;
        } else {
            addr[pos] = (VOS_UINT8)numLen;
            pos++;
        }
    }

    /*
     * 根据协议23040 9.1.2.5 地址类型域格式如下所示
     * bit7   bit6    -   bit4             bit3    -   bit0
     *  1      type of number      Numbering-plan-identification
     */
    addr[pos] = 0x80;
    addr[pos] |= (asciiAddr->numPlan & 0x0f);
    addr[pos] |= (VOS_UINT8)((asciiAddr->numType << 4) & 0x70);
    pos++;

    if ((bRpAddr != VOS_TRUE) && (asciiAddr->numType == MN_MSG_TON_ALPHANUMERIC)) {
        ret = TAF_STD_Pack7Bit(asciiAddr->asciiNum, asciiAddr->len, 0, &(addr[pos]), &alphaNumLen);
        if (ret != VOS_OK) {
            return MN_ERR_CLASS_INVALID_TP_ADDRESS;
        }
        addrBcdLen = (VOS_UINT8)alphaNumLen;
    } else {
        ret = TAF_STD_ConvertAsciiNumberToBcd((VOS_CHAR *)num, &(addr[pos]), &addrBcdLen);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
    }
    *len = pos + addrBcdLen;
    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_EncodeBcdAddress(const MN_MSG_BcdAddr *bcdAddr, VOS_BOOL bRpAddr, VOS_UINT8 *addr, VOS_UINT32 *len)
{
    MN_MSG_AsciiAddr asciiAddr = {0};
    VOS_UINT32       ret;

    ret = MN_MSG_BcdAddrToAscii(bcdAddr, &asciiAddr);

    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    ret = MN_MSG_EncodeAddress(&asciiAddr, bRpAddr, addr, len);

    return ret;
}

LOCAL VOS_UINT32 MSG_EncodeUdhConcat8(const MN_MSG_UdhConcat8 *concatenatedSms, VOS_UINT8 *udh, VOS_UINT8 udhlMax,
                                      VOS_UINT8 *ieiLen)
{
    VOS_UINT8 len = 0;

    if ((concatenatedSms == VOS_NULL_PTR) || (udh == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeUdhConcat8: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (concatenatedSms->seqNum > concatenatedSms->totalNum) {
        MN_WARN_LOG("MSG_EncodeUdhConcat8: Sequence number is bigger than total number.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    if (udhlMax < (MN_MSG_UDH_CONCAT_8_IEL + MN_MSG_IE_HEAD_LEN)) {
        MN_WARN_LOG("MSG_EncodeUdhConcat8: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    udh[len] = MN_MSG_UDH_CONCAT_8;
    len++;
    udh[len] = MN_MSG_UDH_CONCAT_8_IEL;
    len++;
    udh[len] = concatenatedSms->mr;
    len++;
    udh[len] = concatenatedSms->totalNum;
    len++;
    udh[len] = concatenatedSms->seqNum;
    len++;

    *ieiLen = len;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeUdhSpecialSm(const MN_MSG_UdhSpecialSms *specialSm, VOS_UINT8 *udh, VOS_UINT8 udhlMax,
                                        VOS_UINT8 *ieiLen)
{
    VOS_UINT8 len = 0;

    if ((specialSm == VOS_NULL_PTR) || (udh == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeUdhSpecialSm: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (udhlMax < (MN_MSG_UDH_SPECIAL_SM_IEL + MN_MSG_IE_HEAD_LEN)) {
        MN_WARN_LOG("MSG_EncodeUdhSpecialSm: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    udh[len] = MN_MSG_UDH_SPECIAL_SM;
    len++;
    udh[len] = MN_MSG_UDH_SPECIAL_SM_IEL;
    len++;

    /* Octet 1 Message Indication type and Storage. */
    /* Bit 7 Indicates whether or not the message shall be stored. */
    /* Bits 6 and 5 indicate the profile ID of the Multiple Subscriber Profile (see 3GPP TS 23.097 [41]). */
    /* Bits 432 indicate the extended message indication type. */
    /* Bits 0 and 1 indicate the basic message indication type. */
    if (specialSm->msgWaiting == MN_MSG_MSG_WAITING_STORE) {
        udh[len] = 0x80;
    } else {
        udh[len] = 0x00;
    }
    udh[len] |= (specialSm->profileId << 5) & 0x60;
    udh[len] |= (specialSm->extMsgInd << 2) & 0x1c;
    udh[len] |= specialSm->msgWaitingKind & 0x03;
    len++;

    /* Octet 2 Message Count. */
    udh[len] = specialSm->msgCount;
    len++;
    *ieiLen = len;

    return MN_ERR_NO_ERROR;
}

/*
 * MSG_EncodeUdhAppPort_8
 * MSG_EncodeUdhAppPort_16
 * MSG_EncodeUdhSmscCtrl
 * MSG_EncodeUdhTextFormat
 * MSG_EncodeUdhPreDefSound
 * MSG_EncodeUdhUserDefSound
 * MSG_EncodeUdhPreDefAnim
 * MSG_EncodeUdhLargeAnim
 * MSG_EncodeUdhSmallAnim
 * MSG_EncodeUdhLargePic
 * MSG_EncodeUdhSmallPic
 * MSG_EncodeUdhVarPic
 * MN_MSG_EncodeUdhUserPrompt
 * MSG_EncodeUdhRfc822
 * MSG_EncodeEo
 * MSG_EncodeReo
 * MSG_EncodeCc
 * MSG_EncodeObjDistrInd
 * MSG_EncodeHyperLinkFormat
 * MSG_EncodeReplyAddr
 * MSG_EncodeWvgObj
 * MSG_EncodeUdhConcat16
 */

LOCAL VOS_UINT32 MSG_EncodeUdhOther(const MN_MSG_UdhOther *other, VOS_UINT8 *udh, VOS_UINT8 udhlMax, VOS_UINT8 *ieiLen)
{
    errno_t   memResult;
    VOS_UINT8 len = 0;

    if ((other == VOS_NULL_PTR) || (udh == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeUdhOther: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (other->len > MN_MSG_UDH_OTHER_SIZE) {
        MN_WARN_LOG("MSG_EncodeUdhOther: the length of IEI is invalid.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    if (udhlMax < (other->len + MN_MSG_IE_HEAD_LEN)) {
        MN_WARN_LOG("MSG_EncodeUdhOther: invalid len.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    udh[len] = other->udhType;
    len++;
    udh[len] = other->len;
    len++;
    if (other->len > 0) {
        memResult = memcpy_s(&udh[len], (udhlMax - len), other->data, other->len);
        TAF_MEM_CHK_RTN_VAL(memResult, (udhlMax - len), other->len);
    }
    len += other->len;

    *ieiLen = len;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeUdh(VOS_UINT8 numofHeaders, const MN_MSG_UserHeaderType *userDataHeader, VOS_UINT8 *udh,
                               VOS_UINT32 maxMemLength, VOS_UINT8 *udhl)
{
    VOS_UINT32 loop;
    VOS_UINT8  ieiLen;
    VOS_UINT8  pos = 1;
    VOS_UINT32 ret;

    if ((userDataHeader == VOS_NULL_PTR) || (udh == VOS_NULL_PTR) || (udhl == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeUdh: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    for (loop = 0; loop < numofHeaders; loop++) {
        if (maxMemLength < pos) {
            MN_NORM_LOG("MSG_EncodeUdh: invalid len.");
            return MN_ERR_CLASS_INVALID_TP_UD;
        }

        switch (userDataHeader->headerID) {
            case MN_MSG_UDH_CONCAT_8:
                ret = MSG_EncodeUdhConcat8((MN_MSG_UdhConcat8 *)&(userDataHeader->u.concat8), &(udh[pos]),
                                           (VOS_UINT8)(maxMemLength - pos), &ieiLen);
                break;
            case MN_MSG_UDH_SPECIAL_SM:
                ret = MSG_EncodeUdhSpecialSm((MN_MSG_UdhSpecialSms *)&(userDataHeader->u.specialSms), &(udh[pos]),
                                             (VOS_UINT8)(maxMemLength - pos), &ieiLen);
                break;

                /*
                 * MN_MSG_UDH_PORT_8
                 * MN_MSG_UDH_PORT_16
                 * MN_MSG_UDH_SMSC_CTRL
                 * MN_MSG_UDH_TEXT_FORMATING
                 * MN_MSG_UDH_PRE_DEF_SOUND
                 * MN_MSG_UDH_SOURCE
                 * MN_MSG_UDH_USER_DEF_SOUND
                 * MN_MSG_UDH_PRE_DEF_ANIM
                 * MN_MSG_UDH_LARGE_ANIM
                 * MN_MSG_UDH_SMALL_ANIM
                 * MN_MSG_UDH_LARGE_PIC
                 * MN_MSG_UDH_VAR_PIC
                 * MN_MSG_UDH_RFC822
                 * MN_MSG_UDH_EO
                 * MN_MSG_UDH_REO
                 * MN_MSG_UDH_CC
                 * MN_MSG_UDH_OBJ_DISTR_IND
                 * MN_MSG_UDH_HYPERLINK_FORMAT
                 * MN_MSG_UDH_REPLY_ADDR
                 * MN_MSG_UDH_STD_WVG_OBJ:
                 * MN_MSG_UDH_CHAR_SIZE_WVG_OBJ:
                 * MN_MSG_UDH_CONCAT_16
                 */

            default:
                ret = MSG_EncodeUdhOther((MN_MSG_UdhOther *)&(userDataHeader->u.other), &(udh[pos]),
                                         (VOS_UINT8)(maxMemLength - pos), &ieiLen);
                break;
        }

        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }

        if (ieiLen > (MN_MSG_UINT8_MAX - pos)) {
            MN_WARN_LOG("MSG_EncodeUdh: IE length is overflow.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        pos += ieiLen;
        if (pos >= MN_MSG_MAX_8_BIT_LEN) {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
        userDataHeader++;
    }

    udh[0] = (VOS_UINT8)(pos - 1); /* UDHL */
    *udhl  = udh[0];

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_VOID MSG_GetUdhl(VOS_UINT8 numofHeaders, const MN_MSG_UserHeaderType *userHeader, VOS_UINT32 *udhl)
{
    VOS_UINT32 loop;
    VOS_UINT32 udhlTemp = 0;
    MN_MSG_UdhOther *other = VOS_NULL_PTR;

    if ((userHeader == VOS_NULL_PTR) || (udhl == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_GetUdhl: Parameter of the function is null.");
        return;
    }

    for (loop = 0; loop < numofHeaders; loop++) {
        switch (userHeader->headerID) {
            case MN_MSG_UDH_CONCAT_8:
                udhlTemp += (MN_MSG_UDH_CONCAT_8_IEL + 2);
                break;
            case MN_MSG_UDH_SPECIAL_SM:
                udhlTemp += (MN_MSG_UDH_SPECIAL_SM_IEL + 2);
                break;
            /*
             * MN_MSG_UDH_CONCAT_16
             * MN_MSG_UDH_PORT_8
             * MN_MSG_UDH_PORT_16
             * MN_MSG_UDH_SMSC_CTRL
             * MN_MSG_UDH_TEXT_FORMATING
             * MN_MSG_UDH_PRE_DEF_SOUND
             * MN_MSG_UDH_SOURCE
             * MN_MSG_UDH_USER_DEF_SOUND
             * MN_MSG_UDH_PRE_DEF_ANIM
             * MN_MSG_UDH_LARGE_ANIM
             * MN_MSG_UDH_LARGE_ANIM:
             * MN_MSG_UDH_SMALL_ANIM:
             * MN_MSG_UDH_LARGE_PIC:
             * MN_MSG_UDH_SMALL_PIC:
             * MN_MSG_UDH_VAR_PIC:
             * MN_MSG_UDH_USER_PROMPT:
             * MN_MSG_UDH_EO:
             * MN_MSG_UDH_REO:
             * MN_MSG_UDH_CC:
             * MN_MSG_UDH_OBJ_DISTR_IND:
             * MN_MSG_UDH_STD_WVG_OBJ:
             * MN_MSG_UDH_CHAR_SIZE_WVG_OBJ:
             * MN_MSG_UDH_RFC822:
             * MN_MSG_UDH_HYPERLINK_FORMAT:
             * MN_MSG_UDH_REPLY_ADDR:
             */
            default:
                other = (MN_MSG_UdhOther *)&(userHeader->u.other);
                udhlTemp += (other->len + 2);
                break;
        }
        userHeader++;
    }

    *udhl = udhlTemp;
}

VOS_UINT32 MSG_EncodeUserDataByCoding7BIT(MN_MSG_UserData *userData, VOS_UINT8 *userDataInfo, VOS_UINT32 maxMemLength,
                                          VOS_UINT32 *len)
{
    VOS_UINT32 pos = 1;
    VOS_UINT32 ret;
    VOS_UINT8  udhl;
    VOS_UINT8  fillBit = 0;
    VOS_UINT32 lenTemp;
    VOS_UINT32 udl;

    if (userData->numofHeaders != 0) {
        /* 填充UDHL和UDH数据区并输出UDHL数值 */
        ret = MSG_EncodeUdh(TAF_MIN(userData->numofHeaders, MN_MSG_MAX_UDH_NUM), userData->userDataHeader, &(userDataInfo[pos]), maxMemLength - pos,
                            &udhl);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        fillBit = (MN_MSG_BITS_PER_SEPTET - (((udhl + 1) * MN_MSG_BITS_PER_OCTET) % MN_MSG_BITS_PER_SEPTET)) %
                  MN_MSG_BITS_PER_SEPTET;

        /* pucUserData[0]存储UDL */
        udl = (VOS_UINT32)(userData->len + ((((udhl + 1) * MN_MSG_BITS_PER_OCTET) + fillBit) / MN_MSG_BITS_PER_SEPTET));

        /* 将数据区数组下标移至UDHL UDH之后 */
        pos += (udhl + 1);
    } else {
        udl = userData->len;
        if (udl == 0) {
            userDataInfo[0] = (VOS_UINT8)udl;
            *len            = 1;
            return MN_ERR_NO_ERROR;
        }
    }

    /* 用户数据越界检查 */
    if (udl > MN_MSG_MAX_7_BIT_LEN) {
        MN_WARN_LOG("MSG_EncodeUserData: The length of 7 bit encoded user data is overflow.");
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    userDataInfo[0] = (VOS_UINT8)udl;

    /* 字符转换为GSM 7 bit default alphabet，填充UD中的FillBit SM数据区，并输出FillBit SM占用的字节数 */
    ret = TAF_STD_Pack7Bit(userData->orgData, userData->len, fillBit, &(userDataInfo[pos]), &lenTemp);
    if (ret != VOS_OK) {
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    /* 计算UDL UD总共占用的字节数 */
    *len = 1 + (((userDataInfo[0] * MN_MSG_BITS_PER_SEPTET) + MN_MSG_BITS_PER_SEPTET) / MN_MSG_BITS_PER_OCTET);

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MSG_EncodeUserData(MN_MSG_MsgCodingUint8 msgCoding, MN_MSG_UserData *userData, VOS_UINT8 *userDataInfo,
                              VOS_UINT32 maxMemLength,
                              VOS_UINT32 *len)
{
    errno_t    memResult;
    VOS_UINT32 pos = 1;
    VOS_UINT32 ret;
    VOS_UINT8  udhl;
    VOS_UINT32 udl;

    if ((userData == VOS_NULL_PTR) || (userDataInfo == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeUserData: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /* TP UDL UD */
    if (msgCoding == MN_MSG_MSG_CODING_7_BIT) {
        ret = MSG_EncodeUserDataByCoding7BIT(userData, userDataInfo, maxMemLength, len);

        return ret;
    } else {
        if (userData->numofHeaders != 0) {
            /* 填充UDHL和UDH数据区并输出UDHL数值 */
            ret = MSG_EncodeUdh(TAF_MIN(userData->numofHeaders, MN_MSG_MAX_UDH_NUM),
                                userData->userDataHeader,
                                &(userDataInfo[pos]),
                                maxMemLength - pos,
                                &udhl);
            if (ret != MN_ERR_NO_ERROR) {
                return ret;
            }

            /* pucUserData[0]存储UDL */
            udl = (VOS_UINT32)((udhl + 1) + userData->len);
            /* 将数据区数组下标移至UDHL UDH之后 */
            pos += (udhl + 1);
        } else {
            /* pucUserData[0]存储UDL */
            udl = userData->len; /* UDL */
        }

        /* 用户数据越界检查 */
        if (udl > MN_MSG_MAX_8_BIT_LEN) {
            MN_WARN_LOG("MSG_EncodeUserData: The length of 8 bit encoded user data is overflow.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
        userDataInfo[0] = (VOS_UINT8)udl;

        if ((maxMemLength <= pos) || (userData->len > (maxMemLength - pos))) {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        if (userData->len > 0) {
            memResult = memcpy_s(&(userDataInfo[pos]), maxMemLength - pos, userData->orgData, userData->len);
            TAF_MEM_CHK_RTN_VAL(memResult, maxMemLength - pos, userData->len);
        }

        /* 计算UDL UD总共占用的字节数 */
        *len = 1 + userDataInfo[0];
    }

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeDeliver(const MN_MSG_Deliver *smsDeliverInfo, MN_MSG_RawTsData *smsRawDataInfo)
{
    VOS_UINT32        pos = 0;
    VOS_UINT32        addrLen;
    VOS_UINT32        ret;
    VOS_UINT32        sctsLen;
    VOS_UINT32        len;
    MN_MSG_AsciiAddr *addr     = VOS_NULL_PTR;
    MN_MSG_UserData  *userData = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeDeliver: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_DELIVER;

    /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI: */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_DELIVER);
    MSG_SET_TP_MMS(smsRawDataInfo->data[pos], smsDeliverInfo->moreMsg);
    MSG_SET_TP_RP(smsRawDataInfo->data[pos], smsDeliverInfo->replayPath);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsDeliverInfo->userDataHeaderInd);
    MSG_SET_TP_SRI(smsRawDataInfo->data[pos], smsDeliverInfo->staRptInd);
    pos++;

    /* TP-OA:2 - 12o */
    addr = (MN_MSG_AsciiAddr *)&smsDeliverInfo->origAddr;
    ret  = MN_MSG_EncodeAddress(addr, VOS_FALSE, &(smsRawDataInfo->data[pos]), &addrLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += addrLen;

    /* TP-PID */
    smsRawDataInfo->data[pos] = smsDeliverInfo->pid;
    pos++;

    /* TP-DCS */
    ret = MN_MSG_EncodeDcs(&(smsDeliverInfo->dcs), &(smsRawDataInfo->data[pos]));
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos++;

    /* TP-SCTS:7o */
    ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(smsDeliverInfo->timeStamp), &(smsRawDataInfo->data[pos]), &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += sctsLen;

    /* TP-UDL TP-UD, Refer to 23040 9.2.3.24 TP User Data (TP UD) */
    userData = (MN_MSG_UserData *)&(smsDeliverInfo->userData);
    ret      = MSG_EncodeUserData(smsDeliverInfo->dcs.msgCoding, userData, &(smsRawDataInfo->data[pos]),
                                  MN_MSG_MAX_LEN - pos, &len);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    smsRawDataInfo->len = pos + len;
    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeDeliverRptAck(const MN_MSG_DeliverRptAck *smsDeliverReportAckInfo,
                                         MN_MSG_RawTsData           *smsRawDataInfo)
{
    VOS_UINT32            pos       = 0;
    VOS_UINT32            ret       = MN_ERR_NO_ERROR;
    VOS_UINT32            len       = 0;
    MN_MSG_MsgCodingUint8 msgCoding = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_UserData      *userData  = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverReportAckInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeDeliverRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_DELIVER_RPT_ACK;

    /* TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_DELIVER_REPORT);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsDeliverReportAckInfo->userDataHeaderInd);
    pos++;

    /*
     * TP PI  9.2.3.27 BIT   2       1       0
     * TP UDL  TP DCS  TP PID
     */
    smsRawDataInfo->data[pos] = smsDeliverReportAckInfo->paraInd;
    pos++;

    /* TP PID */
    if ((smsDeliverReportAckInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsRawDataInfo->data[pos] = smsDeliverReportAckInfo->pid;
        pos++;
    }

    /* TP DCS */
    if ((smsDeliverReportAckInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_EncodeDcs(&(smsDeliverReportAckInfo->dcs), &(smsRawDataInfo->data[pos]));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        msgCoding = smsDeliverReportAckInfo->dcs.msgCoding;
        pos++;
    }

    /* TP UD TP UDL; */
    smsRawDataInfo->len = pos;
    if ((smsDeliverReportAckInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        userData = (MN_MSG_UserData *)&(smsDeliverReportAckInfo->userData);
        ret      = MSG_EncodeUserData(msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos, &len);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        smsRawDataInfo->len += len;
    }

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeDeliverRptErr(const MN_MSG_DeliverRptErr *smsDeliverReportErrorInfo,
                                         MN_MSG_RawTsData           *smsRawDataInfo)
{
    VOS_UINT32            pos       = 0;
    VOS_UINT32            ret       = MN_ERR_NO_ERROR;
    VOS_UINT32            len       = 0;
    MN_MSG_MsgCodingUint8 msgCoding = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_UserData      *userData  = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverReportErrorInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeDeliverRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_DELIVER_RPT_ERR;

    /* TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_DELIVER_REPORT);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsDeliverReportErrorInfo->userDataHeaderInd);
    pos++;

    /* TP FCS */
    smsRawDataInfo->data[pos] = smsDeliverReportErrorInfo->failCause;
    pos++;
    /*
     * TP PI  9.2.3.27 BIT   2       1       0
     * TP UDL  TP DCS  TP PID
     */
    smsRawDataInfo->data[pos] = smsDeliverReportErrorInfo->paraInd;
    pos++;
    /* TP PID */
    if ((smsDeliverReportErrorInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsRawDataInfo->data[pos] = smsDeliverReportErrorInfo->pid;
        pos++;
    }

    /*  TP DCS */
    if ((smsDeliverReportErrorInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_EncodeDcs(&(smsDeliverReportErrorInfo->dcs), &(smsRawDataInfo->data[pos]));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        msgCoding = smsDeliverReportErrorInfo->dcs.msgCoding;
        pos++;
    }

    /* TP UD TP UDL; */
    smsRawDataInfo->len = pos;
    if ((smsDeliverReportErrorInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        userData = (MN_MSG_UserData *)&(smsDeliverReportErrorInfo->userData);
        ret      = MSG_EncodeUserData(msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos, &len);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        smsRawDataInfo->len += len;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 TAF_MSG_EncodeStaRptExceptUserData(const MN_MSG_StaRpt *smsStatusReportInfo,
                                              MN_MSG_RawTsData *smsRawDataInfo, VOS_UINT32 *pos)
{
    MN_MSG_AsciiAddr *addr = VOS_NULL_PTR;
    VOS_UINT32        addrLen = 0;
    VOS_UINT32        sctsLen = 0;
    VOS_UINT32        dtLen = 0;
    VOS_UINT32        ret = MN_ERR_NO_ERROR;

    /* TP RA 2 12o */
    addr = (MN_MSG_AsciiAddr *)&smsStatusReportInfo->recipientAddr;
    ret  = MN_MSG_EncodeAddress(addr, VOS_FALSE, &(smsRawDataInfo->data[*pos]), &addrLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += addrLen;

    /* TP SCTS 7o Parameter identifying time when the SC received the previously sent SMS SUBMIT */
    ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(smsStatusReportInfo->timeStamp), &(smsRawDataInfo->data[*pos]),
                              &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += sctsLen;

    /* TP DT Parameter identifying the time associated with a particular TP ST outcome */
    ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(smsStatusReportInfo->dischargeTime), &(smsRawDataInfo->data[*pos]),
                              &dtLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += dtLen;

    /* TP ST */
    smsRawDataInfo->data[(*pos)++] = smsStatusReportInfo->status;

    if (smsStatusReportInfo->paraInd == 0) {
        smsRawDataInfo->len = *pos;
        return MN_ERR_NO_ERROR;
    }

    /*
     * TP PI  9.2.3.27 BIT   2       1       0
     * TP UDL  TP DCS  TP PID
     */
    smsRawDataInfo->data[(*pos)++] = smsStatusReportInfo->paraInd;

    /* TP PID */
    if ((smsStatusReportInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsRawDataInfo->data[(*pos)++] = smsStatusReportInfo->pid;
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_EncodeStaRpt(const MN_MSG_StaRpt *smsStatusReportInfo, MN_MSG_RawTsData *smsRawDataInfo)
{
    VOS_UINT32            pos = 0;
    VOS_UINT32            ret;
    VOS_UINT32            len       = 0;
    MN_MSG_MsgCodingUint8 msgCoding = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_UserData      *userData  = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsStatusReportInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeStaRpt: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    (VOS_VOID)memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_STARPT;

    /* TP MTI TP UDHI TP MMS TP SRQ */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_STATUS_REPORT);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsStatusReportInfo->userDataHeaderInd);
    MSG_SET_TP_MMS(smsRawDataInfo->data[pos], smsStatusReportInfo->moreMsg);
    MSG_SET_TP_SRQ(smsRawDataInfo->data[pos], smsStatusReportInfo->staRptQualCommand); /* ?? */
    pos++;

    /* TP MR */
    smsRawDataInfo->data[pos] = smsStatusReportInfo->mr;
    pos++;
    ret = TAF_MSG_EncodeStaRptExceptUserData(smsStatusReportInfo, smsRawDataInfo, &pos);

    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    /*  TP DCS */
    if ((smsStatusReportInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_EncodeDcs(&(smsStatusReportInfo->dcs), &(smsRawDataInfo->data[pos]));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        msgCoding = smsStatusReportInfo->dcs.msgCoding;
        pos++;
    }

    /* TP UD TP UDL; */
    if ((smsStatusReportInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        userData = (MN_MSG_UserData *)&(smsStatusReportInfo->userData);
        ret      = MSG_EncodeUserData(msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos, &len);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos += len;
    }
    smsRawDataInfo->len = pos;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeCommand(const MN_MSG_CommandParam *smsCommandInfo, MN_MSG_RawTsData *smsRawDataInfo)
{
    errno_t           memResult;
    VOS_UINT32        pos = 0;
    VOS_UINT32        ret;
    VOS_UINT32        addrLen;
    MN_MSG_AsciiAddr *addr       = VOS_NULL_PTR;
    VOS_UINT8         maxDataLen = 0;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsCommandInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeCommand: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_COMMAND;

    /* TP MTI TP UDHI TP SRR */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_COMMAND);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsCommandInfo->userDataHeaderInd);
    MSG_SET_TP_SRR(smsRawDataInfo->data[pos], smsCommandInfo->staRptReq);
    pos++;

    /* TP MR */
    smsRawDataInfo->data[pos] = smsCommandInfo->mr;
    pos++;
    /* TP PID */
    smsRawDataInfo->data[pos] = smsCommandInfo->pid;
    pos++;
    /* TP CT */
    smsRawDataInfo->data[pos] = smsCommandInfo->cmdType;
    pos++;
    /* TP MN */
    smsRawDataInfo->data[pos] = smsCommandInfo->msgNumber;
    pos++;
    /* TP DA */
    addr = (MN_MSG_AsciiAddr *)&smsCommandInfo->destAddr;
    ret  = MN_MSG_EncodeAddress(addr, VOS_FALSE, &(smsRawDataInfo->data[pos]), &addrLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += addrLen;

    maxDataLen = (VOS_UINT8)TAF_MIN((MN_MSG_MAX_COMMAND_TPDU_MSG_LEN - pos - 1), MN_MSG_MAX_COMMAND_DATA_LEN);

    if (smsCommandInfo->commandDataLen > maxDataLen) {
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    /* TP CDL */
    smsRawDataInfo->data[pos] = smsCommandInfo->commandDataLen;
    pos++;
    smsRawDataInfo->len = pos;

    /* TP CD */
    if (smsCommandInfo->commandDataLen == 0) {
        return MN_ERR_NO_ERROR;
    }

    if (MN_MSG_MAX_LEN - pos > 0) {
        memResult = memcpy_s(&smsRawDataInfo->data[pos], MN_MSG_MAX_LEN - pos, smsCommandInfo->cmdData,
                             smsCommandInfo->commandDataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, MN_MSG_MAX_LEN - pos, smsCommandInfo->commandDataLen);
    }
    smsRawDataInfo->len += smsCommandInfo->commandDataLen;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeSubmit(const MN_MSG_Submit *smsSubmitInfo, MN_MSG_RawTsData *smsRawDataInfo)
{
    VOS_UINT32        pos = 0;
    VOS_UINT32        vpLen;
    VOS_UINT32        addrLen;
    VOS_UINT32        ret;
    VOS_UINT32        len;
    MN_MSG_UserData  *userData = VOS_NULL_PTR;
    MN_MSG_AsciiAddr *addr     = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeSubmit: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_SUBMIT;

    /*
     * b7  b6   b5   b4   b3  b2  b1  b0
     * RP  UDHI SRR  VPF      RD  MTI
     * 0   0    0    2        1   2
     * TP MTI TP RD TP VPF TP RP TP UDHI TP SRR
     */
    /* TP MTI 23040 9.2.3.1 填写bit0bit1:MIT     0011 0001 */
    /* TP RD  23040 9.2.3.25 */
    /* TP VPF 23040 9.2.3.3 */
    /* TP SRR 23040 9.2.3.5 */
    /* TP UDHI23040 9.2.3.23 */
    /* TP RP  23040 9.2.3.17 */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_SUBMIT);
    MSG_SET_TP_RD(smsRawDataInfo->data[pos], smsSubmitInfo->rejectDuplicates);
    MSG_SET_TP_VPF(smsRawDataInfo->data[pos], smsSubmitInfo->validPeriod.validPeriod);
    MSG_SET_TP_RP(smsRawDataInfo->data[pos], smsSubmitInfo->replayPath);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsSubmitInfo->userDataHeaderInd);
    MSG_SET_TP_SRR(smsRawDataInfo->data[pos], smsSubmitInfo->staRptReq);
    pos++;

    /* TP MR */
    smsRawDataInfo->data[pos] = smsSubmitInfo->mr;
    pos++;

    /* TP DA* Refer to 9.1.2.5 */
    addr = (MN_MSG_AsciiAddr *)&smsSubmitInfo->destAddr;
    ret  = MN_MSG_EncodeAddress(addr, VOS_FALSE, &(smsRawDataInfo->data[pos]), &addrLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += addrLen;

    /* TP PID */
    smsRawDataInfo->data[pos] = smsSubmitInfo->pid;
    pos++;

    /* TP DCS 23038 4 */
    ret = MN_MSG_EncodeDcs(&(smsSubmitInfo->dcs), &(smsRawDataInfo->data[pos]));
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos++;

    /* TP VP */
    ret = MSG_EncodeValidPeriod(&(smsSubmitInfo->validPeriod), &(smsRawDataInfo->data[pos]), &vpLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += vpLen;

    /* UDL UD */
    userData = (MN_MSG_UserData *)&(smsSubmitInfo->userData);
    ret = MSG_EncodeUserData(smsSubmitInfo->dcs.msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos,
                             &len);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    smsRawDataInfo->len = pos + len;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeSubmitRptAck(const MN_MSG_SubmitRptAck *smsSubmitReportAckInfo,
                                        MN_MSG_RawTsData          *smsRawDataInfo)
{
    VOS_UINT32            pos = 0;
    VOS_UINT32            ret;
    VOS_UINT32            len = 0;
    VOS_UINT32            sctsLen;
    MN_MSG_MsgCodingUint8 msgCoding = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_UserData      *userData  = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitReportAckInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeSubmitRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_SUBMIT_RPT_ACK;

    /* TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_SUBMIT_REPORT);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsSubmitReportAckInfo->userDataHeaderInd);
    pos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsRawDataInfo->data[pos] = smsSubmitReportAckInfo->paraInd;
    pos++;

    /* TP SCTS */
    ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(smsSubmitReportAckInfo->timeStamp), &(smsRawDataInfo->data[pos]),
                              &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += sctsLen;

    /* TP PID */
    if ((smsSubmitReportAckInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsRawDataInfo->data[pos] = smsSubmitReportAckInfo->pid;
        pos++;
    }

    /*  TP DCS */
    if ((smsSubmitReportAckInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_EncodeDcs(&(smsSubmitReportAckInfo->dcs), &(smsRawDataInfo->data[pos]));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        msgCoding = smsSubmitReportAckInfo->dcs.msgCoding;
        pos++;
    }

    /* TP UD TP UDL; */
    if ((smsSubmitReportAckInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        userData = (MN_MSG_UserData *)&(smsSubmitReportAckInfo->userData);
        ret      = MSG_EncodeUserData(msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos, &len);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos += len;
    }
    smsRawDataInfo->len = pos;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_EncodeSubmitRptErr(const MN_MSG_SubmitRptErr *smsSubmitReportErrorInfo,
                                        MN_MSG_RawTsData          *smsRawDataInfo)
{
    /* 根据23040 9.2.2.2a SMS SUBMIT REPORT type对SMS SUBMIT REPORT for RP ERROR的短信进行编码 */
    VOS_UINT32            pos = 0;
    VOS_UINT32            ret;
    VOS_UINT32            len = 0;
    VOS_UINT32            sctsLen;
    MN_MSG_MsgCodingUint8 msgCoding = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_UserData      *userData  = VOS_NULL_PTR;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitReportErrorInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_EncodeSubmitRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsRawDataInfo, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    smsRawDataInfo->tpduType = MN_MSG_TPDU_SUBMIT_RPT_ERR;

    /* TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(smsRawDataInfo->data[pos], MN_MSG_TP_MTI_SUBMIT_REPORT);
    MSG_SET_TP_UDHI(smsRawDataInfo->data[pos], smsSubmitReportErrorInfo->userDataHeaderInd);
    pos++;

    /* TP FCS */
    smsRawDataInfo->data[pos] = smsSubmitReportErrorInfo->failCause;
    pos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsRawDataInfo->data[pos] = smsSubmitReportErrorInfo->paraInd;
    pos++;

    /* TP SCTS */
    ret = MSG_EncodeTimeStamp((MN_MSG_Timestamp *)&(smsSubmitReportErrorInfo->timeStamp), &(smsRawDataInfo->data[pos]),
                              &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += sctsLen;

    /* TP PID */
    if ((smsSubmitReportErrorInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsRawDataInfo->data[pos] = smsSubmitReportErrorInfo->pid;
        pos++;
    }

    /*  TP DCS */
    if ((smsSubmitReportErrorInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_EncodeDcs(&(smsSubmitReportErrorInfo->dcs), &(smsRawDataInfo->data[pos]));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        msgCoding = smsSubmitReportErrorInfo->dcs.msgCoding;
        pos++;
    }

    /* TP UD TP UDL; */
    if ((smsSubmitReportErrorInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        userData = (MN_MSG_UserData *)&(smsSubmitReportErrorInfo->userData);
        ret      = MSG_EncodeUserData(msgCoding, userData, &(smsRawDataInfo->data[pos]), MN_MSG_MAX_LEN - pos, &len);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos += len;
    }
    smsRawDataInfo->len = pos;

    return MN_ERR_NO_ERROR;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_EncodeDcs(const MN_MSG_DcsCode *dcs, VOS_UINT8 *dcsData)
{
    VOS_UINT8 dcsTemp = 0;

    if ((dcs == VOS_NULL_PTR) || (dcsData == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_EncodeDcs: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (dcs->rawDcsValid == VOS_TRUE) {
        *dcsData = dcs->rawDcsData;
        return MN_ERR_NO_ERROR;
    }

    switch (dcs->msgWaiting) {
        case MN_MSG_MSG_WAITING_AUTO_DELETE:
            /* bit7 bit6 */
            dcsTemp = 0x40;
            /* fall through */
        case MN_MSG_MSG_WAITING_NONE: /* 00xx 01xx */
            /* Bit 5  if set to 0, indicates the text is uncompressed */
            MSG_SET_COMPRESSED(dcsTemp, dcs->compressed);
            /* Bit 3 Bit2 Character set: */
            MSG_SET_CHARSET(dcsTemp, dcs->msgCoding);
            /* Bit 1 Bit 0 Message Class 数据损失引入点 */
            if (dcs->msgClass != MN_MSG_MSG_CLASS_NONE) {
                /* Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class */
                MSG_SET_CLASSFLAG(dcsTemp, dcs->msgClass);
                MSG_SET_MSGCLASS(dcsTemp, dcs->msgClass);
            }
            break;
        case MN_MSG_MSG_WAITING_DISCARD: /* 1100 */
            /* bit7 bit6 bit5 bit4 */
            dcsTemp = 0xc0;

            /* Bits 3 indicates Indication Sense */
            MSG_SET_INDSENSE(dcsTemp, dcs->waitingIndiActive);
            /* Bits 2 默认为0,数据损失引入点 */
            /* Bit 1 Bit 0 Indication Type */
            MSG_SET_INDTYPE(dcsTemp, dcs->msgWaitingKind);
            break;
        case MN_MSG_MSG_WAITING_STORE: /* 1101 1110 */
            /* bit7 bit6 bit5 bit4 */
            if (dcs->msgCoding == MN_MSG_MSG_CODING_UCS2) {
                dcsTemp = 0xe0; /* 1110 */
            } else {
                dcsTemp = 0xd0; /* 1101 */
            }

            /* Bits 3 indicates Indication Sense */
            MSG_SET_INDSENSE(dcsTemp, dcs->waitingIndiActive);
            /* Bits 2 默认为0,数据损失引入点 */
            /* Bit 1 Bit 0 Indication Type */
            MSG_SET_INDTYPE(dcsTemp, dcs->msgWaitingKind);
            break;
        case MN_MSG_MSG_WAITING_NONE_1111: /* 1111 */
            /* bit7 bit6 bit5 bit4 */
            dcsTemp = 0xf0;
            /* Bits 3 默认为0，数据损失引入点 */
            /* Bits 2 message coding , only be 7bit or 8bit */
            if (dcs->msgCoding == MN_MSG_MSG_CODING_UCS2) {
                MN_WARN_LOG("MN_MSG_EncodeDcs: invalid message waiting type.");
                return MN_ERR_CLASS_SMS_INVALID_CODING_GRP;
            }
            MSG_SET_MSGCODING(dcsTemp, dcs->msgCoding);

            /* Bit 1 Bit 0 Message Class */
            MSG_SET_MSGCLASS(dcsTemp, dcs->msgClass);
            break;
        default:
            MN_WARN_LOG("MN_MSG_EncodeDcs: invalid message waiting type.");
            return MN_ERR_CLASS_SMS_INVALID_CODING_GRP;
    }

    *dcsData = dcsTemp;

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_Encode(const MN_MSG_TsDataInfo *tsDataInfo, MN_MSG_RawTsData *rawData)
{
    VOS_UINT32 ret;

    MN_INFO_LOG("MN_MSG_Encode: Step into MN_MSG_Encode.");

    if ((tsDataInfo == VOS_NULL_PTR) || (rawData == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_Encode: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    } else {
        memset_s(rawData, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));
    }

    switch (tsDataInfo->tpduType) {
        case MN_MSG_TPDU_DELIVER:
            ret = MSG_EncodeDeliver((MN_MSG_Deliver *)&(tsDataInfo->u.deliver), rawData);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            ret = MSG_EncodeDeliverRptAck((MN_MSG_DeliverRptAck *)&(tsDataInfo->u.deliverRptAck), rawData);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            ret = MSG_EncodeDeliverRptErr((MN_MSG_DeliverRptErr *)&(tsDataInfo->u.deliverRptErr), rawData);
            break;
        case MN_MSG_TPDU_STARPT:
            ret = MSG_EncodeStaRpt((MN_MSG_StaRpt *)&(tsDataInfo->u.staRpt), rawData);
            break;
        case MN_MSG_TPDU_COMMAND:
            ret = MSG_EncodeCommand((MN_MSG_CommandParam *)&(tsDataInfo->u.command), rawData);
            break;
        case MN_MSG_TPDU_SUBMIT:
            ret = MSG_EncodeSubmit((MN_MSG_Submit *)&(tsDataInfo->u.submit), rawData);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            ret = MSG_EncodeSubmitRptAck((MN_MSG_SubmitRptAck *)&(tsDataInfo->u.submitRptAck), rawData);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            ret = MSG_EncodeSubmitRptErr((MN_MSG_SubmitRptErr *)&(tsDataInfo->u.submitRptErr), rawData);
            break;
        default:
            MN_WARN_LOG("MN_MSG_Encode: TPDU type is invalid.");
            ret = MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
            break;
    }
    return ret;
}

VOS_UINT32 MSG_RequireSegment(const MN_MSG_SubmitLong *longSubmit)
{
    VOS_BOOL   bSegmentFlag = VOS_FALSE;
    VOS_UINT32 totalLen;
    VOS_UINT32 udhl = 0;

    totalLen = longSubmit->longUserData.len;
    if (longSubmit->dcs.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
        if (longSubmit->userDataHeaderInd == VOS_TRUE) {
            /* 计算初始消息包头长度 */
            MSG_GetUdhl(TAF_MIN(longSubmit->longUserData.numofHeaders, MN_MSG_MAX_UDH_NUM),
                        longSubmit->longUserData.userDataHeader,
                        &udhl);
            totalLen += (((udhl + 1) * 8) + 6) / 7;
        }

        if (totalLen > MN_MSG_MAX_7_BIT_LEN) {
            bSegmentFlag = VOS_TRUE;
        }
    } else {
        if (longSubmit->userDataHeaderInd == VOS_TRUE) {
            /* 计算初始消息包头长度 */
            MSG_GetUdhl(TAF_MIN(longSubmit->longUserData.numofHeaders, MN_MSG_MAX_UDH_NUM),
                        longSubmit->longUserData.userDataHeader,
                        &udhl);
            totalLen += (udhl + 1);
        }

        if (totalLen > MN_MSG_MAX_8_BIT_LEN) {
            bSegmentFlag = VOS_TRUE;
        }
    }

    return bSegmentFlag;
}

VOS_VOID TAF_MSG_InitSubmitTPDU(const MN_MSG_SubmitLong *longSubmit, MN_MSG_Submit *submit)
{
    errno_t memResult;

    submit->rejectDuplicates  = longSubmit->rejectDuplicates;
    submit->replayPath        = longSubmit->replayPath;
    submit->userDataHeaderInd = longSubmit->userDataHeaderInd;
    submit->staRptReq         = longSubmit->staRptReq;
    submit->mr                = longSubmit->mr;
    submit->pid               = longSubmit->pid;
    memResult = memcpy_s(&(submit->destAddr), sizeof(submit->destAddr), &(longSubmit->destAddr),
                         sizeof(submit->destAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->destAddr), sizeof(submit->destAddr));
    memResult = memcpy_s(&(submit->dcs), sizeof(submit->dcs), &(longSubmit->dcs), sizeof(submit->dcs));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->dcs), sizeof(submit->dcs));
    memResult = memcpy_s(&(submit->validPeriod), sizeof(submit->validPeriod), &(longSubmit->validPeriod),
                         sizeof(submit->validPeriod));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->validPeriod), sizeof(submit->validPeriod));
    submit->userData.numofHeaders = longSubmit->longUserData.numofHeaders;
    memResult = memcpy_s(submit->userData.userDataHeader, sizeof(submit->userData.userDataHeader),
                         longSubmit->longUserData.userDataHeader, MN_MSG_MAX_UDH_NUM * sizeof(MN_MSG_UserHeaderType));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->userData.userDataHeader),
                        MN_MSG_MAX_UDH_NUM * sizeof(MN_MSG_UserHeaderType));
}

VOS_UINT32 TAF_MSG_PocSegmentFlagFalse(const MN_MSG_SubmitLong *longSubmit, VOS_UINT8 *num, MN_MSG_Submit *submit,
                                       MN_MSG_RawTsData *rawData)
{
    VOS_UINT32 ret;
    errno_t    memResult;

    *num = 1;

    submit->userData.len = longSubmit->longUserData.len;
    if (submit->userData.len > 0) {
        memResult = memcpy_s(submit->userData.orgData, sizeof(submit->userData.orgData),
                             longSubmit->longUserData.orgData, submit->userData.len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->userData.orgData), submit->userData.len);
    }

    ret = MSG_EncodeSubmit(submit, rawData);

    return ret;
}

VOS_UINT32 TAF_MSG_ProcEncodeSubmit_Segment(const MN_MSG_SubmitLong *longSubmit, VOS_UINT8 *num, MN_MSG_Submit *submit,
                                            MN_MSG_RawTsData *rawData, VOS_UINT32 maxSmsSegment)
{
    MN_MSG_UdhConcat8     *concat8    = VOS_NULL_PTR;
    MN_MSG_UserHeaderType *userHeader = VOS_NULL_PTR;
    VOS_UINT32             loop;
    VOS_UINT32             ret = MN_ERR_NO_ERROR;
    VOS_UINT32             udhl = 0;
    VOS_UINT32             len;
    VOS_UINT32             concatUdhPos;
    errno_t                memResult;

    /* 对于需要分段的消息， */
    /* 1.直接编码填充Concatenated short messages消息头部 */
    concatUdhPos         = (VOS_UINT32)longSubmit->longUserData.numofHeaders;
    userHeader           = &submit->userData.userDataHeader[concatUdhPos];
    userHeader->headerID = MN_MSG_UDH_CONCAT_8; /* 存在Extended Object IE时用MN_MSG_UDH_CONCAT_16 */
    concat8              = (MN_MSG_UdhConcat8 *)&(userHeader->u.concat8);
    concat8->seqNum      = 1;
    concat8->mr          = longSubmit->mr;
    submit->userData.numofHeaders++;
    submit->userDataHeaderInd = VOS_TRUE;

    /* 2.计算分段后消息包头长度 */
    MSG_GetUdhl(TAF_MIN(submit->userData.numofHeaders, MN_MSG_MAX_UDH_NUM), submit->userData.userDataHeader, &udhl);
    if (udhl >= (MN_MSG_MAX_8_BIT_LEN - 1)) {
        MN_WARN_LOG("TAF_MSG_ProcEncodeSubmit_Segment: the length of message is invalid.");

        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    /* 3.计算分段后每个消息段中SM的长度: UDL最大长度去除UDHL和UDH的长度 */
    if (longSubmit->dcs.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
        len = MN_MSG_MAX_7_BIT_LEN - ((((udhl + 1) * 8) + 6) / 7);
    } else {
        len = MN_MSG_MAX_8_BIT_LEN - (udhl + 1);
    }

    /* 计算分段后分段的个数并填充各分段的MN_MSG_Submit结构数据 */
    *num = (VOS_UINT8)((longSubmit->longUserData.len + (len - 1)) / len);

    if (*num > maxSmsSegment) {
        MN_WARN_LOG("TAF_MSG_ProcEncodeSubmit_Segment: the length overflow STK segment capability.");
        return MN_ERR_CLASS_SMS_INTERNAL;
    }

    concat8->totalNum = (*num);
    for (loop = 0; loop < (*num); loop++) {
        if (concat8->seqNum == (*num)) {
            submit->userData.len = longSubmit->longUserData.len - (len * loop);
        } else {
            submit->userData.len = len;
        }
        if (submit->userData.len > 0) {
            memResult = memcpy_s(submit->userData.orgData, sizeof(submit->userData.orgData),
                                 &(longSubmit->longUserData.orgData[((VOS_ULONG)len * loop)]), submit->userData.len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->userData.orgData), submit->userData.len);
        }
        ret = MSG_EncodeSubmit(submit, rawData);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        concat8->seqNum++;
        rawData++;
    }

    return ret;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_Segment(const MN_MSG_SubmitLong *longSubmit, VOS_UINT8 *num,
                                          MN_MSG_RawTsData *rawData, VOS_UINT32 maxSmsSegment)
{
    VOS_UINT32     ret;
    VOS_BOOL       bSegmentFlag = VOS_FALSE;
    MN_MSG_Submit *submit       = VOS_NULL_PTR;
    /* 该接口当前只有C核使用，下移C核后，ACPU宏控制部分不放开 */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    ModemIdUint16 modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(WUEPS_PID_TAF);
#endif

    if ((longSubmit == VOS_NULL_PTR) || (num == VOS_NULL_PTR) || (rawData == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_Segment: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /* 分段消息的属性填充并根据分段属性按submit TPDU格式编码 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    submit = (MN_MSG_Submit *)PS_MEM_ALLOC(WUEPS_PID_TAF, sizeof(MN_MSG_Submit));
#else
    submit = (MN_MSG_Submit *)NAS_MULTIINSTANCE_MemAlloc(modemId, WUEPS_PID_TAF, sizeof(MN_MSG_Submit));
#endif
    if (submit == VOS_NULL_PTR) {
        MN_ERR_LOG("MN_MSG_Segment: Fail to Alloc memory.");
        return MN_ERR_NOMEM;
    }
    TAF_MSG_InitSubmitTPDU(longSubmit, submit);

    bSegmentFlag = MSG_RequireSegment(longSubmit);
    /* 对于不需要分段的消息，直接编码退出流程 */
    if (bSegmentFlag != VOS_TRUE) {
        ret = TAF_MSG_PocSegmentFlagFalse(longSubmit, num, submit, rawData);

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        PS_MEM_FREE(WUEPS_PID_TAF, submit);
        submit = VOS_NULL_PTR;
#else
        NAS_MULTIINSTANCE_MemFree(modemId, WUEPS_PID_TAF, submit);
#endif
        return ret;
    }

    ret = TAF_MSG_ProcEncodeSubmit_Segment(longSubmit, num, submit, rawData, maxSmsSegment);

    if (ret != MN_ERR_NO_ERROR) {
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        PS_MEM_FREE(WUEPS_PID_TAF, submit);
        submit = VOS_NULL_PTR;
#else
        NAS_MULTIINSTANCE_MemFree(modemId, WUEPS_PID_TAF, submit);
#endif
        return ret;
    }

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    PS_MEM_FREE(WUEPS_PID_TAF, submit);
    submit = VOS_NULL_PTR;
#else
    NAS_MULTIINSTANCE_MemFree(modemId, WUEPS_PID_TAF, submit);
#endif
    return MN_ERR_NO_ERROR;
}

VOS_VOID MN_MSG_EncodeTpRd(VOS_BOOL bRejectDuplicates, VOS_UINT8 *tpFo)
{
    /* 设置TP-RD为入参中的bRejectDuplicates */
    MSG_SET_TP_RD(*tpFo, bRejectDuplicates);
}

