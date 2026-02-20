/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include  "PsTypeDef.h"
#include  "PsCommonDef.h"
#include  "MnErrorCode.h"
#include  "MnMsgApi.h"
#include  "MnMsgTs.h"
#include "TafStdlib.h"



VOS_UINT32 MSG_EncodeUserData(
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding,
    MN_MSG_USER_DATA_STRU               *pstUserData,
    VOS_UINT8                           *pucUserData,
    VOS_UINT32                           ulMaxMemLength,
    VOS_UINT32                          *pucLen
);

/*****************************************************************************
  2 常量定义
*****************************************************************************/
#define THIS_FILE_ID                                        PS_FILE_ID_MNMSG_ENCODE_C

/*****************************************************************************
  3 宏定义
*****************************************************************************/

/*****************************************************************************
  4 变量定义
*****************************************************************************/

/*****************************************************************************
  5 函数实现
*****************************************************************************/



TAF_UINT32 MN_ChkNumType(
    MN_MSG_TON_ENUM_U8                  enNumType
)
{
    return MN_ERR_NO_ERROR;
}


TAF_UINT32 MN_ChkNumPlan(
    MN_MSG_NPI_ENUM_U8                  enNumPlan
)
{
    return MN_ERR_NO_ERROR;
}




VOS_UINT32 MN_MSG_ChkDate(
    const MN_MSG_TIMESTAMP_STRU         *pstTimeStamp,
    MN_MSG_DATE_INVALID_TYPE_ENUM_UINT8 *pucInvalidType
)
{
    VOS_UINT8                           ucYear;
    VOS_UINT8                           ucMonth;
    VOS_UINT8                           ucDay;
    VOS_UINT8                           ucHour;
    VOS_UINT8                           ucMinute;
    VOS_UINT8                           ucSecond;
    MN_MSG_DATE_INVALID_TYPE_ENUM_UINT8 ucInvalidType;
    VOS_UINT32                          ulRet;

    ucInvalidType = 0;

    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucYear, VOS_FALSE, &ucYear);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Month is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_YEAR;
    }

    /*将BCD码表示的月日小时分钟秒转换成十进制数进行有效性检查；*/
    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucMonth, VOS_FALSE, &ucMonth);
    if ((MN_ERR_NO_ERROR != ulRet)
     || (ucMonth > MN_MSG_MONTHS_IN_A_YEAR))
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Month is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_MONTH;
    }

    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucDay, VOS_FALSE, &ucDay);
    if ((MN_ERR_NO_ERROR != ulRet)
     || (ucDay > MN_MSG_MAX_DAYS_IN_A_MONTH))
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Date is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_DAY;
    }

    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucHour, VOS_FALSE, &ucHour);
    if ((MN_ERR_NO_ERROR != ulRet)
     || (ucHour > (MN_MSG_HOURS_IN_A_DAY - 1)))
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Hour is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_HOUR;
    }

    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucMinute, VOS_FALSE, &ucMinute);
    if ((MN_ERR_NO_ERROR != ulRet)
     || (ucMinute > (MN_MSG_MINUTES_IN_AN_HOUR - 1)))
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Minute is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_MINUTE;
    }

    ulRet = TAF_STD_ConvertBcdToDeciDigit(pstTimeStamp->ucSecond, VOS_FALSE, &ucSecond);
    if ((MN_ERR_NO_ERROR != ulRet)
     || (ucSecond > (MN_MSG_SECONDS_IN_A_MINUTE - 1)))
    {
        MN_WARN_LOG("MN_MSG_ChkDate: Second is invalid.");
        ucInvalidType |= MN_MSG_DATE_INVALID_SECOND;
    }

    *pucInvalidType = ucInvalidType;
    if (0 == ucInvalidType)
    {
        return MN_ERR_NO_ERROR;
    }
    else
    {
        return MN_ERR_CLASS_SMS_INVALID_DATE;
    }
}



LOCAL VOS_UINT32 MSG_EncodeTimeStamp(
    const MN_MSG_TIMESTAMP_STRU         *pstTimeStamp,
    VOS_UINT8                           *pucValidPeriod,
    VOS_UINT32                          *pulLen
)
{
    VOS_UINT8                           ucAbsTimeZone;
    VOS_UINT8                           ucAlgebraicSign;
    VOS_UINT8                           ucDateInvalidType;
    VOS_UINT32                          ulRet;

    if ((VOS_NULL_PTR == pstTimeStamp)
     || (VOS_NULL_PTR == pucValidPeriod)
     || (VOS_NULL_PTR == pulLen))
    {
        MN_ERR_LOG("MSG_EncodeTimeStamp: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    ulRet = MN_MSG_ChkDate(pstTimeStamp, &ucDateInvalidType);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        MN_WARN_LOG("MSG_EncodeTimeStamp: Date is invalid.");
    }

    /*时区有效性检查，根据协议27005 <dt>示例可知(1hour <-> 4)必须在-48-48之间；*/
    if ((pstTimeStamp->cTimezone > MN_MSG_MAX_TIMEZONE_VALUE)
     || (pstTimeStamp->cTimezone < ((-1)*MN_MSG_MAX_TIMEZONE_VALUE)))
    {
        MN_WARN_LOG("MSG_EncodeTimeStamp: Time zone is invalid.");
    }

    /*根据协议23040 9.2.3.11和9.1.2.3 在VP中BCD码是反序放置的，
      所以将结构中BCD码反序后赋给输出 */
    MN_MSG_REVERSE_BCD(pucValidPeriod[0], pstTimeStamp->ucYear);
    MN_MSG_REVERSE_BCD(pucValidPeriod[1], pstTimeStamp->ucMonth);
    MN_MSG_REVERSE_BCD(pucValidPeriod[2], pstTimeStamp->ucDay);
    MN_MSG_REVERSE_BCD(pucValidPeriod[3], pstTimeStamp->ucHour);
    MN_MSG_REVERSE_BCD(pucValidPeriod[4], pstTimeStamp->ucMinute);
    MN_MSG_REVERSE_BCD(pucValidPeriod[5], pstTimeStamp->ucSecond);

    /*时区在协议23040 9.2.3.11中需要将绝对值转换成反序BCD码再或上表示负数的符号*/
    if (pstTimeStamp->cTimezone < 0)
    {
        ucAbsTimeZone = (VOS_UINT8)((-1)*pstTimeStamp->cTimezone);
        ucAlgebraicSign = MN_MSG_NEGATIVE_ALGEBRAICSIGN;
    }
    else
    {
        ucAbsTimeZone = (VOS_UINT8)pstTimeStamp->cTimezone;
        ucAlgebraicSign = 0x00;
    }
    pucValidPeriod[6]  = ucAlgebraicSign;
    pucValidPeriod[6] |= TAF_STD_ConvertDeciDigitToBcd(ucAbsTimeZone, VOS_TRUE);

    *pulLen = MN_MSG_ABSOLUTE_TIMESTAMP_LEN;
    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32 MSG_EncodeValidPeriod(
    const MN_MSG_VALID_PERIOD_STRU      *pstValidPeriod,
    VOS_UINT8                           *pucValidPeriod,
    VOS_UINT32                          *pulLen
)
{
    VOS_UINT32                          ulRet               = MN_ERR_NO_ERROR;

    if ((VOS_NULL_PTR == pstValidPeriod)
     || (VOS_NULL_PTR == pucValidPeriod)
     || (VOS_NULL_PTR == pulLen))
    {
        MN_ERR_LOG("MSG_EncodeValidPeriod: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    switch (pstValidPeriod->enValidPeriod)
    {
        case MN_MSG_VALID_PERIOD_NONE:
            *pulLen = 0;
            break;
        case MN_MSG_VALID_PERIOD_RELATIVE:
            *pulLen = 1;
            *pucValidPeriod = pstValidPeriod->u.ucOtherTime;
            break;
        case MN_MSG_VALID_PERIOD_ABSOLUTE:
            ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstValidPeriod->u.stAbsoluteTime),
                                         pucValidPeriod,
                                         pulLen);
            break;
        default:
            MN_WARN_LOG("MSG_EncodeValidPeriod: The type of valid period is not supported!");
            return MN_ERR_CLASS_SMS_INVALID_VPF;
    }

    return ulRet;
}


VOS_UINT32  MN_MSG_EncodeRelatTime(
    const MN_MSG_TIMESTAMP_STRU         *pstRelatTime,
    VOS_UINT8                           *pucRelatTime
)
{
    VOS_UINT16                          usTotalDay;
    VOS_UINT32                          ulTotalHour;
    VOS_UINT32                          ulTotalMinute;
    VOS_UINT8                           ucDateInvalidType;
    VOS_UINT32                          ulRet;

    if ((VOS_NULL_PTR == pstRelatTime)
     || (VOS_NULL_PTR == pucRelatTime))
    {
        MN_ERR_LOG("MN_MSG_EncodeRelatTime: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /*将BCD码表示的月日小时分钟秒转换成十进制数进行有效性检查；*/
    ulRet = MN_MSG_ChkDate(pstRelatTime, &ucDateInvalidType);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }

    /*将pstRelatTime转换成usTotalDay和ucTotalMinute*/
    usTotalDay      = (VOS_UINT16)(MN_MSG_DAYS_IN_A_YEAR * pstRelatTime->ucYear);
    usTotalDay     += (VOS_UINT16)(MN_MSG_DAYS_IN_A_MONTH * pstRelatTime->ucMonth);
    usTotalDay     += pstRelatTime->ucDay;

    /*TP-VP最多只能表示441天的有效期*/
    if (usTotalDay > MN_MSG_MAX_RELAT_VP_DAYS)
    {
        MN_WARN_LOG("MN_MSG_EncodeRelatTime: Parameter of the function is invalid.");
        return MN_ERR_CLASS_SMS_INVALID_RELATTTIME;
    }

    /*197～255：（VP–192）*1 周，表示时间范围为31天(约为5周) － 441天(196-166)*/
    if (usTotalDay > MN_MSG_DAYS_IN_A_MONTH)
    {
        *pucRelatTime = (VOS_UINT8)(((usTotalDay + 6)/7) + 192);
        return MN_ERR_NO_ERROR;
    }

    /*168～196：（VP–166）*1 日,表示时间范围为2(168-166)天 － 30天(196-166)*/
    if (usTotalDay > 1)
    {
        *pucRelatTime = (VOS_UINT8)(usTotalDay + 166);
        return MN_ERR_NO_ERROR;
    }

    ulTotalHour   = (MN_MSG_HOURS_IN_A_DAY * usTotalDay) + pstRelatTime->ucHour;
    ulTotalMinute = MN_MSG_MINUTES_IN_AN_HOUR * ulTotalHour;
    ulTotalMinute+= pstRelatTime->ucMinute;

    if (ulTotalMinute < 1)
    {
        *pucRelatTime = 0;
        return MN_ERR_NO_ERROR;
    }

    /*144～167：12 小时+（VP–143）*30 分钟,表示时间范围为12小时 -> 1天*/
    if (ulTotalMinute > (12 * MN_MSG_MINUTES_IN_AN_HOUR))
    {
        *pucRelatTime = (VOS_UINT8)((((ulTotalMinute - (12 * MN_MSG_MINUTES_IN_AN_HOUR)) + 29)/30) + 143);
        return MN_ERR_NO_ERROR;
    }

    /* 0～143：（VP+1）* 5 分钟,表示时间范围为5分钟 － 12小时*/
    *pucRelatTime = (VOS_UINT8)(((ulTotalMinute + 4)/5) - 1);
    return MN_ERR_NO_ERROR;
}


VOS_UINT32 MN_MSG_EncodeAddress(
    const MN_MSG_ASCII_ADDR_STRU        *pstAsciiAddr,
    VOS_BOOL                            bRpAddr,
    VOS_UINT8                           *pucAddr,
    VOS_UINT32                          *pulLen
)
{
    VOS_UINT32                          ulNumLen;                               /*号码长度*/
    VOS_UINT8                           *pucNum;                                /*指向实际号码（不包括+号）的指针*/
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT8                           ucAddrBcdLen;
    VOS_UINT32                          ulAlphaNumLen;
    VOS_UINT32                          ulRet;

    if ((VOS_NULL_PTR == pstAsciiAddr)
     || (VOS_NULL_PTR == pucAddr)
     || (VOS_NULL_PTR == pulLen))
    {
        MN_ERR_LOG("MN_MSG_EncodeAddress: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (0 == pstAsciiAddr->ulLen)
    {
        if (VOS_TRUE == bRpAddr)
        {
            *pucAddr = 0;
            *pulLen = 1;
        }
        else
        {
            *pucAddr = 0;
            *(pucAddr + 1) = 0;
            *pulLen = 2;
        }
        return MN_ERR_NO_ERROR;
    }

    /*如果号码的首字符是'+'号，表示国际号码*/
    if ('+' == pstAsciiAddr->aucAsciiNum[0])
    {
        if (MN_MSG_TON_INTERNATIONAL != pstAsciiAddr->enNumType)
        {
            MN_WARN_LOG("MN_MSG_EncodeAddress: The address string begins with '+'"
                       "while number type indicates non-international number type");
            return MN_ERR_CLASS_SMS_INVALID_NUMTYPE;
        }
        pucNum = (VOS_UINT8 *)&(pstAsciiAddr->aucAsciiNum[1]);
        ulNumLen = pstAsciiAddr->ulLen - 1;
    }
    else
    {
        pucNum = (VOS_UINT8 *)pstAsciiAddr->aucAsciiNum;
        ulNumLen = pstAsciiAddr->ulLen;
    }

    /*号码长度应该在范围1 - 20之间，参考协议24011 8.2.5.1和8.2.5.2
                                            23040 9.2.2 和 9.1.2.5*/
    if ((ulNumLen > MN_MSG_MAX_ADDR_LEN)
     || (ulNumLen < MN_MSG_MIN_ADDR_LEN))
    {
        MN_WARN_LOG("MN_MSG_EncodeAddress: The length of address is invalid.");
        return MN_ERR_CLASS_SMS_INVALID_ADDRLEN;
    }

    /*号码类型,拨号计划类型有效性检查*/
    ulRet = MN_ChkNumPlan(pstAsciiAddr->enNumPlan);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        MN_WARN_LOG("MN_MSG_EncodeAddress: Numbering plan is invalid");
        return ulRet;
    }
    ulRet = MN_ChkNumType(pstAsciiAddr->enNumType);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        MN_WARN_LOG("MN_MSG_EncodeAddress: Number type is invalid");
        return ulRet;
    }
    /*第0个字节为号码（包括号码类型）的字节长度:
    根据协议24011 8.2.5.1 8.2.5.2 和24008 10.5.4.9 RP层的地址IE中长度指BCD号码长度，包含地址类型的占位
    根据协议23040 9.1.2.5 TP层的地址长度为有效号码字符个数*/
    if (VOS_TRUE == bRpAddr)
    {
        pucAddr[ulPos++] = (VOS_UINT8)(((ulNumLen + 1)/2) + 1);
    }
    else
    {
        if (MN_MSG_TON_ALPHANUMERIC == pstAsciiAddr->enNumType)
        {
            pucAddr[ulPos++] = (VOS_UINT8)(((ulNumLen * 7) + 3)/4);
        }
        else
        {
            pucAddr[ulPos++] = (VOS_UINT8)ulNumLen;
        }
    }

    /* 根据协议23040 9.1.2.5 地址类型域格式如下所示
        bit7   bit6    -   bit4             bit3    -   bit0
         1      type of number      Numbering-plan-identification */
    pucAddr[ulPos]      = 0x80;
    pucAddr[ulPos]     |= (pstAsciiAddr->enNumPlan & 0x0f);
    pucAddr[ulPos]     |= (VOS_UINT8)((pstAsciiAddr->enNumType << 4) & 0x70);
    ulPos++;

    if ((VOS_TRUE != bRpAddr)
     && (MN_MSG_TON_ALPHANUMERIC == pstAsciiAddr->enNumType))
    {
        ulRet = TAF_STD_Pack7Bit(pstAsciiAddr->aucAsciiNum,
                                 pstAsciiAddr->ulLen,
                                 0,
                                 &(pucAddr[ulPos]),
                                 &ulAlphaNumLen);
        if (VOS_OK != ulRet)
        {
            return MN_ERR_CLASS_INVALID_TP_ADDRESS;
        }
        ucAddrBcdLen = (VOS_UINT8)ulAlphaNumLen;
    }
    else
    {
        ulRet = TAF_STD_ConvertAsciiNumberToBcd((VOS_CHAR *)pucNum,
                                    &(pucAddr[ulPos]),
                                    &ucAddrBcdLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
    }
    *pulLen = ulPos + ucAddrBcdLen;
    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32 MSG_EncodeUdhConcat8(
    const MN_MSG_UDH_CONCAT_8_STRU     *pstConcatenatedSms,
    VOS_UINT8                          *pucUdh,
    VOS_UINT8                           ucUdhlMax,
    VOS_UINT8                          *pucIeiLen
)
{
    VOS_UINT8                           ucLen               = 0;

    if ((VOS_NULL_PTR == pstConcatenatedSms)
     || (VOS_NULL_PTR == pucUdh)
     || (VOS_NULL_PTR == pucIeiLen))
    {
        MN_ERR_LOG("MSG_EncodeUdhConcat8: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (pstConcatenatedSms->ucSeqNum > pstConcatenatedSms->ucTotalNum)
    {
        MN_WARN_LOG("MSG_EncodeUdhConcat8: Sequence number is bigger than total number.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    if (ucUdhlMax < (MN_MSG_UDH_CONCAT_8_IEL + MN_MSG_IE_HEAD_LEN))
    {
        MN_WARN_LOG("MSG_EncodeUdhConcat8: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    pucUdh[ucLen++] = MN_MSG_UDH_CONCAT_8;
    pucUdh[ucLen++] = MN_MSG_UDH_CONCAT_8_IEL;
    pucUdh[ucLen++] = pstConcatenatedSms->ucMr;
    pucUdh[ucLen++] = pstConcatenatedSms->ucTotalNum;
    pucUdh[ucLen++] = pstConcatenatedSms->ucSeqNum;

    *pucIeiLen = ucLen;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32 MSG_EncodeUdhSpecialSm(
    const MN_MSG_UDH_SPECIAL_SMS_STRU  *pstSpecialSm,
    VOS_UINT8                          *pucUdh,
    VOS_UINT8                           ucUdhlMax,
    VOS_UINT8                          *pucIeiLen
)
{
    VOS_UINT8                           ucLen               = 0;

    if ((VOS_NULL_PTR == pstSpecialSm)
     || (VOS_NULL_PTR == pucUdh)
     || (VOS_NULL_PTR == pucIeiLen))
    {
        MN_ERR_LOG("MSG_EncodeUdhSpecialSm: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (ucUdhlMax < (MN_MSG_UDH_SPECIAL_SM_IEL + MN_MSG_IE_HEAD_LEN))
    {
        MN_WARN_LOG("MSG_EncodeUdhSpecialSm: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    pucUdh[ucLen++] = MN_MSG_UDH_SPECIAL_SM;
    pucUdh[ucLen++] = MN_MSG_UDH_SPECIAL_SM_IEL;

    /*Octet 1 Message Indication type and Storage.*/
    /*Bit 7 Indicates whether or not the message shall be stored.*/
    /*Bits 6 and 5 indicate the profile ID of the Multiple Subscriber Profile (see 3GPP TS 23.097 [41]).*/
    /*Bits 432 indicate the extended message indication type.*/
    /*Bits 0 and 1 indicate the basic message indication type.*/
    if (MN_MSG_MSG_WAITING_STORE == pstSpecialSm->enMsgWaiting)
    {
        pucUdh[ucLen] = 0x80;
    }
    else
    {
        pucUdh[ucLen] = 0x00;
    }
    pucUdh[ucLen] |= (pstSpecialSm->enProfileId << 5) & 0x60;
    pucUdh[ucLen] |= (pstSpecialSm->enExtMsgInd << 2) & 0x1c;
    pucUdh[ucLen] |= pstSpecialSm->enMsgWaitingKind   & 0x03;
    ucLen++;

    /*Octet 2 Message Count.*/
    pucUdh[ucLen++] = pstSpecialSm->ucMsgCount;

    *pucIeiLen = ucLen;
    

    return MN_ERR_NO_ERROR;
}

/*
MSG_EncodeUdhAppPort_8
MSG_EncodeUdhAppPort_16
MSG_EncodeUdhSmscCtrl
MSG_EncodeUdhTextFormat
MSG_EncodeUdhPreDefSound
MSG_EncodeUdhUserDefSound
MSG_EncodeUdhPreDefAnim
MSG_EncodeUdhLargeAnim
MSG_EncodeUdhSmallAnim
MSG_EncodeUdhLargePic
MSG_EncodeUdhSmallPic
MSG_EncodeUdhVarPic
MN_MSG_EncodeUdhUserPrompt
MSG_EncodeUdhRfc822
MSG_EncodeEo
MSG_EncodeReo
MSG_EncodeCc
MSG_EncodeObjDistrInd
MSG_EncodeHyperLinkFormat
MSG_EncodeReplyAddr
MSG_EncodeWvgObj
MSG_EncodeUdhConcat16

*/


LOCAL VOS_UINT32 MSG_EncodeUdhOther(
    const MN_MSG_UDH_OTHER_STRU        *pstOther,
    VOS_UINT8                          *pucUdh,
    VOS_UINT8                           ucUdhlMax,
    VOS_UINT8                          *pucIeiLen
)
{
    VOS_UINT8                           ucLen               = 0;

    if ((VOS_NULL_PTR == pstOther)
     || (VOS_NULL_PTR == pucUdh)
     || (VOS_NULL_PTR == pucIeiLen))
    {
        MN_ERR_LOG("MSG_EncodeUdhOther: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (pstOther->ucLen > MN_MSG_UDH_OTHER_SIZE)
    {
        MN_WARN_LOG("MSG_EncodeUdhOther: the length of IEI is invalid.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    if (ucUdhlMax < (pstOther->ucLen + MN_MSG_IE_HEAD_LEN))
    {
        MN_WARN_LOG("MSG_EncodeUdhOther: invalid len.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    pucUdh[ucLen++] = pstOther->enUdhType;
    pucUdh[ucLen++] = pstOther->ucLen;
    TAF_MEM_CPY_S(&pucUdh[ucLen], (ucUdhlMax - ucLen), pstOther->aucData, pstOther->ucLen);
    ucLen += pstOther->ucLen;

    *pucIeiLen = ucLen;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32  MSG_EncodeUdh(
    VOS_UINT8                           ucNumofHeaders,
    const MN_MSG_USER_HEADER_TYPE_STRU  *pstUserDataHeader,
    VOS_UINT8                           *pucUdh,
    VOS_UINT32                           ulMaxMemLength,
    VOS_UINT8                           *pucUdhl
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT8                           ucIeiLen;
    VOS_UINT8                           ucPos = 1;
    VOS_UINT32                          ulRet;

    if ((VOS_NULL_PTR == pstUserDataHeader)
     || (VOS_NULL_PTR == pucUdh)
     || (VOS_NULL_PTR == pucUdhl))
    {
        MN_ERR_LOG("MSG_EncodeUdh: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    for (ulLoop = 0; ulLoop < ucNumofHeaders; ulLoop++)
    {
        if (ulMaxMemLength < ucPos)
        {
            MN_NORM_LOG("MSG_EncodeUdh: invalid len.");
            return MN_ERR_CLASS_INVALID_TP_UD;
        }

        switch (pstUserDataHeader->enHeaderID)
        {
            case MN_MSG_UDH_CONCAT_8:
                ulRet = MSG_EncodeUdhConcat8((MN_MSG_UDH_CONCAT_8_STRU *)&(pstUserDataHeader->u.stConcat_8),
                                             &(pucUdh[ucPos]),
                                             (VOS_UINT8)(ulMaxMemLength - ucPos),
                                             &ucIeiLen);
                break;
            case MN_MSG_UDH_SPECIAL_SM:
                ulRet = MSG_EncodeUdhSpecialSm((MN_MSG_UDH_SPECIAL_SMS_STRU *)&(pstUserDataHeader->u.stSpecial_Sms),
                                               &(pucUdh[ucPos]),
                                               (VOS_UINT8)(ulMaxMemLength - ucPos),
                                               &ucIeiLen);
                break;

            /*
            MN_MSG_UDH_PORT_8
            MN_MSG_UDH_PORT_16
            MN_MSG_UDH_SMSC_CTRL
            MN_MSG_UDH_TEXT_FORMATING
            MN_MSG_UDH_PRE_DEF_SOUND
            MN_MSG_UDH_SOURCE
            MN_MSG_UDH_USER_DEF_SOUND
            MN_MSG_UDH_PRE_DEF_ANIM
            MN_MSG_UDH_LARGE_ANIM
            MN_MSG_UDH_SMALL_ANIM
            MN_MSG_UDH_LARGE_PIC
            MN_MSG_UDH_VAR_PIC
            MN_MSG_UDH_RFC822
            MN_MSG_UDH_EO
            MN_MSG_UDH_REO
            MN_MSG_UDH_CC
            MN_MSG_UDH_OBJ_DISTR_IND
            MN_MSG_UDH_HYPERLINK_FORMAT
            MN_MSG_UDH_REPLY_ADDR
            MN_MSG_UDH_STD_WVG_OBJ:
            MN_MSG_UDH_CHAR_SIZE_WVG_OBJ:
            MN_MSG_UDH_CONCAT_16
            */

            default:
                ulRet = MSG_EncodeUdhOther((MN_MSG_UDH_OTHER_STRU *)&(pstUserDataHeader->u.stOther),
                                           &(pucUdh[ucPos]),
                                           (VOS_UINT8)(ulMaxMemLength - ucPos),
                                           &ucIeiLen);
                break;
        }

        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }

        if (ucIeiLen > (MN_MSG_UINT8_MAX - ucPos))
        {
            MN_WARN_LOG("MSG_EncodeUdh: IE length is overflow.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        ucPos += ucIeiLen;
        if (ucPos >= MN_MSG_MAX_8_BIT_LEN)
        {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
        pstUserDataHeader++;
    }

    pucUdh[0]   = (VOS_UINT8)(ucPos - 1);/*UDHL*/
    *pucUdhl    = pucUdh[0];

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_VOID  MSG_GetUdhl(
    VOS_UINT8                           ucNumofHeaders,
    const MN_MSG_USER_HEADER_TYPE_STRU  *pstUserHeader,
    VOS_UINT32                          *pulUdhl
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulUdhl = 0;
    MN_MSG_UDH_OTHER_STRU              *pstOther = VOS_NULL_PTR;

    if ((VOS_NULL_PTR == pstUserHeader)
     || (VOS_NULL_PTR == pulUdhl))
    {
        MN_ERR_LOG("MSG_GetUdhl: Parameter of the function is null.");
        return;
    }

    for (ulLoop = 0; ulLoop < ucNumofHeaders; ulLoop++)
    {
        switch (pstUserHeader->enHeaderID)
        {
            case MN_MSG_UDH_CONCAT_8:
                ulUdhl += (MN_MSG_UDH_CONCAT_8_IEL + 2);
                break;
            case MN_MSG_UDH_SPECIAL_SM:
                ulUdhl += (MN_MSG_UDH_SPECIAL_SM_IEL + 2);
                break;
            /*
            MN_MSG_UDH_CONCAT_16
            MN_MSG_UDH_PORT_8
            MN_MSG_UDH_PORT_16
            MN_MSG_UDH_SMSC_CTRL
            MN_MSG_UDH_TEXT_FORMATING
            MN_MSG_UDH_PRE_DEF_SOUND
            MN_MSG_UDH_SOURCE
            MN_MSG_UDH_USER_DEF_SOUND
            MN_MSG_UDH_PRE_DEF_ANIM
            MN_MSG_UDH_LARGE_ANIM
            MN_MSG_UDH_LARGE_ANIM:
            MN_MSG_UDH_SMALL_ANIM:
            MN_MSG_UDH_LARGE_PIC:
            MN_MSG_UDH_SMALL_PIC:
            MN_MSG_UDH_VAR_PIC:
            MN_MSG_UDH_USER_PROMPT:
            MN_MSG_UDH_EO:
            MN_MSG_UDH_REO:
            MN_MSG_UDH_CC:
            MN_MSG_UDH_OBJ_DISTR_IND:
            MN_MSG_UDH_STD_WVG_OBJ:
            MN_MSG_UDH_CHAR_SIZE_WVG_OBJ:
            MN_MSG_UDH_RFC822:
            MN_MSG_UDH_HYPERLINK_FORMAT:
            MN_MSG_UDH_REPLY_ADDR:
            */
            default:
                pstOther  = (MN_MSG_UDH_OTHER_STRU *)&(pstUserHeader->u.stOther);
                ulUdhl += (pstOther->ucLen + 2);
                break;
        }
        pstUserHeader++;
    }

    *pulUdhl = ulUdhl;
    return;

}


VOS_UINT32 MSG_EncodeUserData(
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding,
    MN_MSG_USER_DATA_STRU               *pstUserData,
    VOS_UINT8                           *pucUserData,
    VOS_UINT32                           ulMaxMemLength,
    VOS_UINT32                          *pucLen
)
{
    VOS_UINT32                          ulPos               = 1;
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucUdhl;
    VOS_UINT8                           ucFillBit           = 0;
    VOS_UINT32                          ulLen;

    if ((VOS_NULL_PTR == pstUserData)
     || (VOS_NULL_PTR == pucUserData)
     || (VOS_NULL_PTR == pucLen))
    {
        MN_ERR_LOG("MSG_EncodeUserData: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /*TP UDL UD */
    if (MN_MSG_MSG_CODING_7_BIT == enMsgCoding)
    {
        if (0 != pstUserData->ucNumofHeaders)
        {
            /*填充UDHL和UDH数据区并输出UDHL数值*/
            ulRet = MSG_EncodeUdh(pstUserData->ucNumofHeaders,
                                  pstUserData->astUserDataHeader,
                                  &(pucUserData[ulPos]),
                                  ulMaxMemLength - ulPos,
                                  &ucUdhl);
            if (MN_ERR_NO_ERROR != ulRet)
            {
                return ulRet;
            }
            ucFillBit      = (7 - (((ucUdhl + 1) * 8) % 7)) % 7;

            /*pucUserData[0]存储UDL*/
            pucUserData[0] = (VOS_UINT8)(pstUserData->ulLen + ((((ucUdhl + 1) * 8) + ucFillBit)/7));

            /*将数据区数组下标移至UDHL UDH之后 */
            ulPos         += (ucUdhl + 1);
        }
        else
        {
            pucUserData[0] = (VOS_UINT8)pstUserData->ulLen;
            if (0 == pstUserData->ulLen)
            {
                *pucLen = 1;
                return MN_ERR_NO_ERROR;
            }
        }

        /*用户数据越界检查*/
        if (pucUserData[0] > MN_MSG_MAX_7_BIT_LEN)
        {
            MN_WARN_LOG("MSG_EncodeUserData: The length of 7 bit encoded user data is overflow.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        /*字符转换为GSM 7 bit default alphabet，填充UD中的FillBit SM数据区，并输出FillBit SM占用的字节数*/
        ulRet   = TAF_STD_Pack7Bit(pstUserData->aucOrgData,
                                   pstUserData->ulLen,
                                   ucFillBit,
                                   &(pucUserData[ulPos]),
                                   &ulLen);
        if (VOS_OK != ulRet)
        {
            return MN_ERR_CLASS_INVALID_TP_UD;
        }

        /*计算UDL UD总共占用的字节数*/
        *pucLen = 1 + (((pucUserData[0] * 7) + 7)/8);
    }
    else
    {
        if (0 != pstUserData->ucNumofHeaders)
        {
            /*填充UDHL和UDH数据区并输出UDHL数值*/
            ulRet = MSG_EncodeUdh(pstUserData->ucNumofHeaders,
                                  pstUserData->astUserDataHeader,
                                  &(pucUserData[ulPos]),
                                  ulMaxMemLength - ulPos,
                                  &ucUdhl);
            if (MN_ERR_NO_ERROR != ulRet)
            {
                return ulRet;
            }

            /*pucUserData[0]存储UDL*/
            pucUserData[0] = (VOS_UINT8)((ucUdhl + 1) + pstUserData->ulLen);
            /*将数据区数组下标移至UDHL UDH之后 */
            ulPos         += (ucUdhl + 1);
        }
        else
        {
            /*pucUserData[0]存储UDL*/
            pucUserData[0]     = (VOS_UINT8)pstUserData->ulLen;/*UDL*/
        }

        /*用户数据越界检查*/
        if (pucUserData[0] > MN_MSG_MAX_8_BIT_LEN)
        {
            MN_WARN_LOG("MSG_EncodeUserData: The length of 8 bit encoded user data is overflow.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        if (pstUserData->ulLen > (ulMaxMemLength - ulPos))
        {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        TAF_MEM_CPY_S(&(pucUserData[ulPos]), ulMaxMemLength - ulPos, pstUserData->aucOrgData, pstUserData->ulLen);

        /*计算UDL UD总共占用的字节数*/
        *pucLen = 1 + pucUserData[0];
    }

    return MN_ERR_NO_ERROR;

}


LOCAL VOS_UINT32   MSG_EncodeDeliver(
    const MN_MSG_DELIVER_STRU           *pstSmsDeliverInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulAddrLen;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulSctsLen;
    VOS_UINT32                          ulLen;
    MN_MSG_ASCII_ADDR_STRU              *pstAddr;
    MN_MSG_USER_DATA_STRU               *pstUserData;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsDeliverInfo))
    {
        MN_ERR_LOG("MSG_EncodeDeliver: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_DELIVER;

    /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI:*/
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_DELIVER);
    MSG_SET_TP_MMS(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverInfo->bMoreMsg);
    MSG_SET_TP_RP(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverInfo->bReplayPath);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverInfo->bUserDataHeaderInd);
    MSG_SET_TP_SRI(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverInfo->bStaRptInd);
    ulPos ++;

    /* TP-OA:2 - 12o*/
    pstAddr = (MN_MSG_ASCII_ADDR_STRU *)&pstSmsDeliverInfo->stOrigAddr;
    ulRet   = MN_MSG_EncodeAddress(pstAddr,
                                VOS_FALSE,
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulAddrLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulAddrLen;

    /* TP-PID*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverInfo->enPid;

    /* TP-DCS */
    ulRet = MN_MSG_EncodeDcs(&(pstSmsDeliverInfo->stDcs),
                             &(pstSmsRawDataInfo->aucData[ulPos]));
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos++;

    /* TP-SCTS:7o*/
    ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstSmsDeliverInfo->stTimeStamp),
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulSctsLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulSctsLen;

    /* TP-UDL TP-UD, Refer to 23040 9.2.3.24 TP User Data (TP UD) */
    pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsDeliverInfo->stUserData);
    ulRet = MSG_EncodeUserData(pstSmsDeliverInfo->stDcs.enMsgCoding,
                               pstUserData,
                               &(pstSmsRawDataInfo->aucData[ulPos]),
                               MN_MSG_MAX_LEN - ulPos,
                               &ulLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }

    pstSmsRawDataInfo->ulLen = ulPos + ulLen;
    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeDeliverRptAck(
    const MN_MSG_DELIVER_RPT_ACK_STRU   *pstSmsDeliverReportAckInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet               = MN_ERR_NO_ERROR;
    VOS_UINT32                          ulLen;
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding         = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_USER_DATA_STRU               *pstUserData;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsDeliverReportAckInfo))
    {
        MN_ERR_LOG("MSG_EncodeDeliverRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_DELIVER_RPT_ACK;

    /*TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_DELIVER_REPORT);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverReportAckInfo->bUserDataHeaderInd);
    ulPos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0
                                TP UDL  TP DCS  TP PID        */
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverReportAckInfo->ucParaInd;

    /*TP PID*/
    if (0 != (pstSmsDeliverReportAckInfo->ucParaInd & MN_MSG_TP_PID_MASK))
    {
        pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverReportAckInfo->enPid;
    }

    /*TP DCS*/
    if (0 != (pstSmsDeliverReportAckInfo->ucParaInd & MN_MSG_TP_DCS_MASK))
    {
        ulRet = MN_MSG_EncodeDcs(&(pstSmsDeliverReportAckInfo->stDcs),
                                 &(pstSmsRawDataInfo->aucData[ulPos]));
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        enMsgCoding = pstSmsDeliverReportAckInfo->stDcs.enMsgCoding;
        ulPos++;
    }

    /*TP UD TP UDL;*/
    pstSmsRawDataInfo->ulLen = ulPos;
    if (0 != (pstSmsDeliverReportAckInfo->ucParaInd & MN_MSG_TP_UDL_MASK))
    {
        pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsDeliverReportAckInfo->stUserData);
        ulRet = MSG_EncodeUserData(enMsgCoding,
                                   pstUserData,
                                   &(pstSmsRawDataInfo->aucData[ulPos]),
                                   MN_MSG_MAX_LEN - ulPos,
                                   &ulLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        pstSmsRawDataInfo->ulLen += ulLen;
    }

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeDeliverRptErr(
    const MN_MSG_DELIVER_RPT_ERR_STRU   *pstSmsDeliverReportErrorInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet               = MN_ERR_NO_ERROR;
    VOS_UINT32                          ulLen;
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding         = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_USER_DATA_STRU               *pstUserData;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsDeliverReportErrorInfo))
    {
        MN_ERR_LOG("MSG_EncodeDeliverRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_DELIVER_RPT_ERR;

    /*TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_DELIVER_REPORT);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsDeliverReportErrorInfo->bUserDataHeaderInd);
    ulPos++;

    /*TP FCS*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverReportErrorInfo->enFailCause;
    /*    TP PI  9.2.3.27 BIT   2       1       0
                                TP UDL  TP DCS  TP PID        */
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverReportErrorInfo->ucParaInd;

    /*TP PID*/
    if (0 != (pstSmsDeliverReportErrorInfo->ucParaInd & MN_MSG_TP_PID_MASK))
    {
        pstSmsRawDataInfo->aucData[ulPos++] = pstSmsDeliverReportErrorInfo->enPid;
    }

    /*  TP DCS*/
    if (0 != (pstSmsDeliverReportErrorInfo->ucParaInd & MN_MSG_TP_DCS_MASK))
    {
        ulRet = MN_MSG_EncodeDcs(&(pstSmsDeliverReportErrorInfo->stDcs),
                                 &(pstSmsRawDataInfo->aucData[ulPos]));
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        enMsgCoding = pstSmsDeliverReportErrorInfo->stDcs.enMsgCoding;
        ulPos++;
    }

    /*TP UD TP UDL;*/
    pstSmsRawDataInfo->ulLen = ulPos;
    if (0 != (pstSmsDeliverReportErrorInfo->ucParaInd & MN_MSG_TP_UDL_MASK))
    {
        pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsDeliverReportErrorInfo->stUserData);
        ulRet = MSG_EncodeUserData(enMsgCoding,
                                   pstUserData,
                                   &(pstSmsRawDataInfo->aucData[ulPos]),
                                   MN_MSG_MAX_LEN - ulPos,
                                   &ulLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        pstSmsRawDataInfo->ulLen += ulLen;
    }

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeStaRpt(
    const MN_MSG_STA_RPT_STRU           *pstSmsStatusReportInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulAddrLen;
    VOS_UINT32                          ulSctsLen;
    VOS_UINT32                          ulDtLen;
    VOS_UINT32                          ulLen;
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding         = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_USER_DATA_STRU               *pstUserData;
    MN_MSG_ASCII_ADDR_STRU              *pstAddr;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsStatusReportInfo))
    {
        MN_ERR_LOG("MSG_EncodeStaRpt: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_STARPT;

    /*TP MTI TP UDHI TP MMS TP SRQ*/
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_STATUS_REPORT);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsStatusReportInfo->bUserDataHeaderInd);
    MSG_SET_TP_MMS(pstSmsRawDataInfo->aucData[ulPos], pstSmsStatusReportInfo->bMoreMsg);
    MSG_SET_TP_SRQ(pstSmsRawDataInfo->aucData[ulPos], pstSmsStatusReportInfo->bStaRptQualCommand);/*??*/
    ulPos++;

    /*TP MR*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsStatusReportInfo->ucMr;

    /*TP RA 2 12o*/
    pstAddr = (MN_MSG_ASCII_ADDR_STRU *)&pstSmsStatusReportInfo->stRecipientAddr;
    ulRet   = MN_MSG_EncodeAddress(pstAddr,
                                VOS_FALSE,
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulAddrLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulAddrLen;

    /*TP SCTS 7o Parameter identifying time when the SC received the previously sent SMS SUBMIT*/
    ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstSmsStatusReportInfo->stTimeStamp),
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulSctsLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulSctsLen;

    /*TP DT Parameter identifying the time associated with a particular TP ST outcome*/
    ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstSmsStatusReportInfo->stDischargeTime),
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulDtLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulDtLen;

    /*TP ST*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsStatusReportInfo->enStatus;

    if (0 == pstSmsStatusReportInfo->ucParaInd)
    {
        pstSmsRawDataInfo->ulLen = ulPos;
        return MN_ERR_NO_ERROR;
    }

    /*    TP PI  9.2.3.27 BIT   2       1       0
                                TP UDL  TP DCS  TP PID        */
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsStatusReportInfo->ucParaInd;

    /*TP PID*/
    if (0 != (pstSmsStatusReportInfo->ucParaInd & MN_MSG_TP_PID_MASK))
    {
        pstSmsRawDataInfo->aucData[ulPos++] = pstSmsStatusReportInfo->enPid;
    }

    /*  TP DCS*/
    if (0 != (pstSmsStatusReportInfo->ucParaInd & MN_MSG_TP_DCS_MASK))
    {
        ulRet = MN_MSG_EncodeDcs(&(pstSmsStatusReportInfo->stDcs),
                                 &(pstSmsRawDataInfo->aucData[ulPos]));
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        enMsgCoding = pstSmsStatusReportInfo->stDcs.enMsgCoding;
        ulPos++;
    }

    /*TP UD TP UDL;*/
    if (0 != (pstSmsStatusReportInfo->ucParaInd & MN_MSG_TP_UDL_MASK))
    {
        pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsStatusReportInfo->stUserData);
        ulRet = MSG_EncodeUserData(enMsgCoding,
                                   pstUserData,
                                   &(pstSmsRawDataInfo->aucData[ulPos]),
                                   MN_MSG_MAX_LEN - ulPos,
                                   &ulLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        ulPos += ulLen;
    }
    pstSmsRawDataInfo->ulLen = ulPos;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeCommand(
    const MN_MSG_COMMAND_STRU           *pstSmsCommandInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulAddrLen;
    MN_MSG_ASCII_ADDR_STRU              *pstAddr;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsCommandInfo))
    {
        MN_ERR_LOG("MSG_EncodeCommand: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_COMMAND;

    /*TP MTI TP UDHI TP SRR */
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_COMMAND);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsCommandInfo->bUserDataHeaderInd);
    MSG_SET_TP_SRR(pstSmsRawDataInfo->aucData[ulPos], pstSmsCommandInfo->bStaRptReq);
    ulPos++;

    /*TP MR*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsCommandInfo->ucMr;

    /*TP PID*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsCommandInfo->enPid;

    /*TP CT*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsCommandInfo->enCmdType;

    /*TP MN*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsCommandInfo->ucMsgNumber;

    /*TP DA*/
    pstAddr = (MN_MSG_ASCII_ADDR_STRU *)&pstSmsCommandInfo->stDestAddr;
    ulRet   = MN_MSG_EncodeAddress(pstAddr,
                                VOS_FALSE,
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulAddrLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulAddrLen;

    /*TP CDL*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsCommandInfo->ucCommandDataLen;
    pstSmsRawDataInfo->ulLen = ulPos;

    /*TP CD*/
    if (0 == pstSmsCommandInfo->ucCommandDataLen)
    {
        return MN_ERR_NO_ERROR;
    }

    if (pstSmsCommandInfo->ucCommandDataLen > MN_MSG_MAX_COMMAND_DATA_LEN)
    {
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }
    TAF_MEM_CPY_S(&pstSmsRawDataInfo->aucData[ulPos],
               MN_MSG_MAX_LEN - ulPos,
               pstSmsCommandInfo->aucCmdData,
               pstSmsCommandInfo->ucCommandDataLen);
    pstSmsRawDataInfo->ulLen += pstSmsCommandInfo->ucCommandDataLen;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeSubmit(
    const MN_MSG_SUBMIT_STRU            *pstSmsSubmitInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulVpLen;
    VOS_UINT32                          ulAddrLen;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLen;
    MN_MSG_USER_DATA_STRU               *pstUserData;
    MN_MSG_ASCII_ADDR_STRU              *pstAddr;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsSubmitInfo))
    {
        MN_ERR_LOG("MSG_EncodeSubmit: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_SUBMIT;

    /*b7  b6   b5   b4   b3  b2  b1  b0
      RP  UDHI SRR  VPF      RD  MTI
      0   0    0    2        1   2
      TP MTI TP RD TP VPF TP RP TP UDHI TP SRR*/
    /*TP MTI 23040 9.2.3.1 填写bit0bit1:MIT     0011 0001 */
    /*TP RD  23040 9.2.3.25*/
    /*TP VPF 23040 9.2.3.3*/
    /*TP SRR 23040 9.2.3.5*/
    /*TP UDHI23040 9.2.3.23*/
    /*TP RP  23040 9.2.3.17*/
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_SUBMIT);
    MSG_SET_TP_RD(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitInfo->bRejectDuplicates);
    MSG_SET_TP_VPF(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitInfo->stValidPeriod.enValidPeriod);
    MSG_SET_TP_RP(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitInfo->bReplayPath);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitInfo->bUserDataHeaderInd);
    MSG_SET_TP_SRR(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitInfo->bStaRptReq);
    ulPos++;

    /*TP MR*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitInfo->ucMr;

    /*TP DA* Refer to 9.1.2.5*/
    pstAddr = (MN_MSG_ASCII_ADDR_STRU *)&pstSmsSubmitInfo->stDestAddr;
    ulRet   = MN_MSG_EncodeAddress(pstAddr,
                                VOS_FALSE,
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulAddrLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulAddrLen;

    /*TP PID*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitInfo->enPid;

    /*TP DCS 23038 4 */
    ulRet = MN_MSG_EncodeDcs(&(pstSmsSubmitInfo->stDcs),
                             &(pstSmsRawDataInfo->aucData[ulPos]));
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos++;

    /*TP VP*/
    ulRet = MSG_EncodeValidPeriod(&(pstSmsSubmitInfo->stValidPeriod),
                                  &(pstSmsRawDataInfo->aucData[ulPos]),
                                  &ulVpLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulVpLen;

    /*UDL UD*/
    pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsSubmitInfo->stUserData);
    ulRet = MSG_EncodeUserData(pstSmsSubmitInfo->stDcs.enMsgCoding,
                               pstUserData,
                               &(pstSmsRawDataInfo->aucData[ulPos]),
                               MN_MSG_MAX_LEN - ulPos,
                               &ulLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }

    pstSmsRawDataInfo->ulLen = ulPos + ulLen;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeSubmitRptAck(
    const MN_MSG_SUBMIT_RPT_ACK_STRU    *pstSmsSubmitReportAckInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLen;
    VOS_UINT32                          ulSctsLen;
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding         = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_USER_DATA_STRU               *pstUserData;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsSubmitReportAckInfo))
    {
        MN_ERR_LOG("MSG_EncodeSubmitRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_SUBMIT_RPT_ACK;

    /*TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_SUBMIT_REPORT);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitReportAckInfo->bUserDataHeaderInd);
    ulPos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitReportAckInfo->ucParaInd;

    /*TP SCTS*/
    ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstSmsSubmitReportAckInfo->stTimeStamp),
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulSctsLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulSctsLen;

    /*TP PID*/
    if (0 != (pstSmsSubmitReportAckInfo->ucParaInd & MN_MSG_TP_PID_MASK))
    {
        pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitReportAckInfo->enPid;
    }

    /*  TP DCS*/
    if (0 != (pstSmsSubmitReportAckInfo->ucParaInd & MN_MSG_TP_DCS_MASK))
    {
        ulRet = MN_MSG_EncodeDcs(&(pstSmsSubmitReportAckInfo->stDcs),
                                 &(pstSmsRawDataInfo->aucData[ulPos]));
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        enMsgCoding = pstSmsSubmitReportAckInfo->stDcs.enMsgCoding;
        ulPos++;
    }

    /*TP UD TP UDL;*/
    if (0 != (pstSmsSubmitReportAckInfo->ucParaInd & MN_MSG_TP_UDL_MASK))
    {
        pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsSubmitReportAckInfo->stUserData);
        ulRet = MSG_EncodeUserData(enMsgCoding,
                                   pstUserData,
                                   &(pstSmsRawDataInfo->aucData[ulPos]),
                                   MN_MSG_MAX_LEN - ulPos,
                                   &ulLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        ulPos += ulLen;
    }
    pstSmsRawDataInfo->ulLen = ulPos;

    return MN_ERR_NO_ERROR;
}


LOCAL VOS_UINT32   MSG_EncodeSubmitRptErr(
    const MN_MSG_SUBMIT_RPT_ERR_STRU    *pstSmsSubmitReportErrorInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstSmsRawDataInfo
)
{
    /* 根据23040 9.2.2.2a SMS SUBMIT REPORT type对SMS SUBMIT REPORT for RP ERROR的短信进行编码*/
    VOS_UINT32                          ulPos               = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLen;
    VOS_UINT32                          ulSctsLen;
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding         = MN_MSG_MSG_CODING_7_BIT;
    MN_MSG_USER_DATA_STRU               *pstUserData;

    if ((VOS_NULL_PTR == pstSmsRawDataInfo)
     || (VOS_NULL_PTR == pstSmsSubmitReportErrorInfo))
    {
        MN_ERR_LOG("MSG_EncodeSubmitRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    TAF_MEM_SET_S(pstSmsRawDataInfo, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    pstSmsRawDataInfo->enTpduType = MN_MSG_TPDU_SUBMIT_RPT_ERR;

    /*TP MTI  TP-UDHI  */
    MSG_SET_TP_MTI(pstSmsRawDataInfo->aucData[ulPos], MN_MSG_TP_MTI_SUBMIT_REPORT);
    MSG_SET_TP_UDHI(pstSmsRawDataInfo->aucData[ulPos], pstSmsSubmitReportErrorInfo->bUserDataHeaderInd);
    ulPos++;


    /*TP FCS*/
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitReportErrorInfo->enFailCause;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitReportErrorInfo->ucParaInd;

    /*TP SCTS*/
    ulRet = MSG_EncodeTimeStamp((MN_MSG_TIMESTAMP_STRU *)&(pstSmsSubmitReportErrorInfo->stTimeStamp),
                                &(pstSmsRawDataInfo->aucData[ulPos]),
                                &ulSctsLen);
    if (MN_ERR_NO_ERROR != ulRet)
    {
        return ulRet;
    }
    ulPos += ulSctsLen;

    /*TP PID*/
    if (0 != (pstSmsSubmitReportErrorInfo->ucParaInd & MN_MSG_TP_PID_MASK))
    {
        pstSmsRawDataInfo->aucData[ulPos++] = pstSmsSubmitReportErrorInfo->enPid;
    }

    /*  TP DCS*/
    if (0 != (pstSmsSubmitReportErrorInfo->ucParaInd & MN_MSG_TP_DCS_MASK))
    {
        ulRet = MN_MSG_EncodeDcs(&(pstSmsSubmitReportErrorInfo->stDcs),
                                 &(pstSmsRawDataInfo->aucData[ulPos]));
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        enMsgCoding = pstSmsSubmitReportErrorInfo->stDcs.enMsgCoding;
        ulPos++;
    }

    /*TP UD TP UDL;*/
    if (0 != (pstSmsSubmitReportErrorInfo->ucParaInd & MN_MSG_TP_UDL_MASK))
    {
        pstUserData = (MN_MSG_USER_DATA_STRU *)&(pstSmsSubmitReportErrorInfo->stUserData);
        ulRet = MSG_EncodeUserData(enMsgCoding,
                                   pstUserData,
                                   &(pstSmsRawDataInfo->aucData[ulPos]),
                                   MN_MSG_MAX_LEN - ulPos,
                                   &ulLen);
        if (MN_ERR_NO_ERROR != ulRet)
        {
            return ulRet;
        }
        ulPos += ulLen;
    }
    pstSmsRawDataInfo->ulLen = ulPos;

    return MN_ERR_NO_ERROR;
}


MODULE_EXPORTED VOS_UINT32  MN_MSG_EncodeDcs(
    const MN_MSG_DCS_CODE_STRU          *pstDcs,
    VOS_UINT8                           *pucDcsData
)
{
    VOS_UINT8                           ucDcs = 0;

    if ((VOS_NULL_PTR == pstDcs)
     || (VOS_NULL_PTR == pucDcsData))
    {
        MN_ERR_LOG("MN_MSG_EncodeDcs: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (VOS_TRUE == pstDcs->bRawDcsValid)
    {
        *pucDcsData = pstDcs->ucRawDcsData;
        return MN_ERR_NO_ERROR;
    }

    switch (pstDcs->enMsgWaiting)
    {
        case MN_MSG_MSG_WAITING_AUTO_DELETE:
            /*bit7 bit6*/
            ucDcs = 0x40;
            /* fall through */
        case MN_MSG_MSG_WAITING_NONE:/*00xx 01xx*/
            /* Bit 5  if set to 0, indicates the text is uncompressed */
            MSG_SET_COMPRESSED(ucDcs, pstDcs->bCompressed);
            /* Bit 3 Bit2 Character set:*/
            MSG_SET_CHARSET(ucDcs, pstDcs->enMsgCoding);
            /* Bit 1 Bit 0 Message Class 数据损失引入点*/
            if (MN_MSG_MSG_CLASS_NONE != pstDcs->enMsgClass)
            {
                /* Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class*/
                MSG_SET_CLASSFLAG(ucDcs, pstDcs->enMsgClass);
                MSG_SET_MSGCLASS(ucDcs, pstDcs->enMsgClass);
            }
            break;
        case MN_MSG_MSG_WAITING_DISCARD:/*1100*/
            /*bit7 bit6 bit5 bit4*/
            ucDcs = 0xc0;

            /*Bits 3 indicates Indication Sense*/
            MSG_SET_INDSENSE(ucDcs, pstDcs->bWaitingIndiActive);
            /*Bits 2 默认为0,数据损失引入点*/
            /*Bit 1 Bit 0 Indication Type*/
            MSG_SET_INDTYPE(ucDcs, pstDcs->enMsgWaitingKind);
            break;
        case MN_MSG_MSG_WAITING_STORE:/*1101 1110 */
            /*bit7 bit6 bit5 bit4*/
            if (MN_MSG_MSG_CODING_UCS2 == pstDcs->enMsgCoding)
            {
                ucDcs = 0xe0;/*1110*/
            }
            else
            {
                ucDcs = 0xd0;/*1101*/
            }

            /*Bits 3 indicates Indication Sense*/
            MSG_SET_INDSENSE(ucDcs, pstDcs->bWaitingIndiActive);
            /*Bits 2 默认为0,数据损失引入点*/
            /*Bit 1 Bit 0 Indication Type*/
            MSG_SET_INDTYPE(ucDcs, pstDcs->enMsgWaitingKind);
            break;
        case MN_MSG_MSG_WAITING_NONE_1111:/*1111 */
            /*bit7 bit6 bit5 bit4*/
            ucDcs = 0xf0;
            /*Bits 3 默认为0，数据损失引入点*/
            /*Bits 2 message coding , only be 7bit or 8bit*/
            if (MN_MSG_MSG_CODING_UCS2 == pstDcs->enMsgCoding)
            {
                MN_WARN_LOG("MN_MSG_EncodeDcs: invalid message waiting type.");
                return MN_ERR_CLASS_SMS_INVALID_CODING_GRP;
            }
            MSG_SET_MSGCODING(ucDcs, pstDcs->enMsgCoding);

            /* Bit 1 Bit 0 Message Class */
            MSG_SET_MSGCLASS(ucDcs, pstDcs->enMsgClass);
            break;
        default:
            MN_WARN_LOG("MN_MSG_EncodeDcs: invalid message waiting type.");
            return MN_ERR_CLASS_SMS_INVALID_CODING_GRP;
    }

    *pucDcsData = ucDcs;

    return MN_ERR_NO_ERROR;
}



VOS_UINT32  MN_MSG_Encode(
    const MN_MSG_TS_DATA_INFO_STRU      *pstTsDataInfo,
    MN_MSG_RAW_TS_DATA_STRU             *pstRawData
)
{
    VOS_UINT32                          ulRet;

    MN_INFO_LOG("MN_MSG_Encode: Step into MN_MSG_Encode.");

    if ((VOS_NULL_PTR == pstTsDataInfo)
     || (VOS_NULL_PTR == pstRawData))
    {
        MN_ERR_LOG("MN_MSG_Encode: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }
    else
    {
        TAF_MEM_SET_S(pstRawData, sizeof(MN_MSG_RAW_TS_DATA_STRU), 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU));
    }

    switch (pstTsDataInfo->enTpduType)
    {
        case MN_MSG_TPDU_DELIVER:
            ulRet = MSG_EncodeDeliver((MN_MSG_DELIVER_STRU *)&(pstTsDataInfo->u.stDeliver),
                                      pstRawData);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            ulRet = MSG_EncodeDeliverRptAck((MN_MSG_DELIVER_RPT_ACK_STRU *)&(pstTsDataInfo->u.stDeliverRptAck),
                                            pstRawData);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            ulRet = MSG_EncodeDeliverRptErr((MN_MSG_DELIVER_RPT_ERR_STRU *)&(pstTsDataInfo->u.stDeliverRptErr),
                                            pstRawData);
            break;
        case MN_MSG_TPDU_STARPT:
            ulRet = MSG_EncodeStaRpt((MN_MSG_STA_RPT_STRU *)&(pstTsDataInfo->u.stStaRpt),
                                        pstRawData);
            break;
        case MN_MSG_TPDU_COMMAND:
            ulRet = MSG_EncodeCommand((MN_MSG_COMMAND_STRU *)&(pstTsDataInfo->u.stCommand),
                                      pstRawData);
            break;
        case MN_MSG_TPDU_SUBMIT:
            ulRet = MSG_EncodeSubmit((MN_MSG_SUBMIT_STRU *)&(pstTsDataInfo->u.stSubmit),
                                     pstRawData);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            ulRet = MSG_EncodeSubmitRptAck((MN_MSG_SUBMIT_RPT_ACK_STRU *)&(pstTsDataInfo->u.stSubmitRptAck),
                                           pstRawData);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            ulRet = MSG_EncodeSubmitRptErr((MN_MSG_SUBMIT_RPT_ERR_STRU *)&(pstTsDataInfo->u.stSubmitRptErr),
                                           pstRawData);
            break;
        default:
            MN_WARN_LOG("MN_MSG_Encode: TPDU type is invalid.");
            ulRet = MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
            break;
    }
    return ulRet;
}


VOS_UINT32   MSG_RequireSegment(
    const MN_MSG_SUBMIT_LONG_STRU       *pstLongSubmit
)
{
    VOS_BOOL                            bSegmentFlag        =  VOS_FALSE;
    VOS_UINT32                          ulTotalLen;
    VOS_UINT32                          ulUdhl              = 0;

    ulTotalLen = pstLongSubmit->stLongUserData.ulLen;
    if (MN_MSG_MSG_CODING_7_BIT == pstLongSubmit->stDcs.enMsgCoding)
    {
        if (VOS_TRUE == pstLongSubmit->bUserDataHeaderInd)
        {
            /*计算初始消息包头长度*/
            MSG_GetUdhl(pstLongSubmit->stLongUserData.ucNumofHeaders,
                        pstLongSubmit->stLongUserData.astUserDataHeader,
                        &ulUdhl);
            ulTotalLen += (((ulUdhl + 1) * 8) + 6)/7;
        }

        if (ulTotalLen > MN_MSG_MAX_7_BIT_LEN)
        {
            bSegmentFlag = VOS_TRUE;
        }
    }
    else
    {
        if (VOS_TRUE == pstLongSubmit->bUserDataHeaderInd)
        {
            /*计算初始消息包头长度*/
            MSG_GetUdhl(pstLongSubmit->stLongUserData.ucNumofHeaders,
                        pstLongSubmit->stLongUserData.astUserDataHeader,
                        &ulUdhl);
            ulTotalLen += (ulUdhl + 1);
        }

        if (ulTotalLen > MN_MSG_MAX_8_BIT_LEN)
        {
            bSegmentFlag = VOS_TRUE;
        }
    }

    return bSegmentFlag;
}


MODULE_EXPORTED VOS_UINT32   MN_MSG_Segment(
    const MN_MSG_SUBMIT_LONG_STRU       *pstLongSubmit,
    VOS_UINT8                           *pucNum,
    MN_MSG_RAW_TS_DATA_STRU             *pstRawData,
    VOS_UINT32                           ulMaxSmsSegment
)
{
    VOS_UINT32                          ulUdhl              = 0;
    VOS_UINT32                          ulLen;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulConcatUdhPos;
    VOS_BOOL                            bSegmentFlag        =  VOS_FALSE;
    MN_MSG_SUBMIT_STRU                  *pstSubmit;
    MN_MSG_UDH_CONCAT_8_STRU            *pstConcat_8;
    MN_MSG_USER_HEADER_TYPE_STRU        *pstUserHeader;

    if ((VOS_NULL_PTR == pstLongSubmit)
     || (VOS_NULL_PTR == pucNum)
     || (VOS_NULL_PTR == pstRawData))
    {
        MN_ERR_LOG("MN_MSG_Segment: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /*分段消息的属性填充并根据分段属性按submit TPDU格式编码*/
    pstSubmit = (MN_MSG_SUBMIT_STRU *)PS_MEM_ALLOC(WUEPS_PID_TAF, sizeof(MN_MSG_SUBMIT_STRU));
    if (VOS_NULL_PTR == pstSubmit)
    {
        MN_ERR_LOG("MN_MSG_Segment: Fail to Alloc memory.");
        return MN_ERR_NOMEM;
    }
    pstSubmit->bRejectDuplicates    = pstLongSubmit->bRejectDuplicates;
    pstSubmit->bReplayPath          = pstLongSubmit->bReplayPath;
    pstSubmit->bUserDataHeaderInd   = pstLongSubmit->bUserDataHeaderInd;
    pstSubmit->bStaRptReq           = pstLongSubmit->bStaRptReq;
    pstSubmit->ucMr                 = pstLongSubmit->ucMr;
    pstSubmit->enPid                = pstLongSubmit->enPid;
    TAF_MEM_CPY_S(&(pstSubmit->stDestAddr),
               sizeof(pstSubmit->stDestAddr),
               &(pstLongSubmit->stDestAddr),
               sizeof(pstSubmit->stDestAddr));
    TAF_MEM_CPY_S(&(pstSubmit->stDcs),
               sizeof(pstSubmit->stDcs),
               &(pstLongSubmit->stDcs),
               sizeof(pstSubmit->stDcs));
    TAF_MEM_CPY_S(&(pstSubmit->stValidPeriod),
               sizeof(pstSubmit->stValidPeriod),
               &(pstLongSubmit->stValidPeriod),
               sizeof(pstSubmit->stValidPeriod));
    pstSubmit->stUserData.ucNumofHeaders = pstLongSubmit->stLongUserData.ucNumofHeaders;
    TAF_MEM_CPY_S(pstSubmit->stUserData.astUserDataHeader,
               sizeof(pstSubmit->stUserData.astUserDataHeader),
               pstLongSubmit->stLongUserData.astUserDataHeader,
               MN_MSG_MAX_UDH_NUM * sizeof(MN_MSG_USER_HEADER_TYPE_STRU));

    bSegmentFlag = MSG_RequireSegment(pstLongSubmit);
    /*对于不需要分段的消息，直接编码退出流程*/
    if (VOS_TRUE != bSegmentFlag)
    {
        *pucNum = 1;
        pstSubmit->stUserData.ulLen = pstLongSubmit->stLongUserData.ulLen;
        TAF_MEM_CPY_S(pstSubmit->stUserData.aucOrgData,
                   sizeof(pstSubmit->stUserData.aucOrgData),
                   pstLongSubmit->stLongUserData.pucOrgData,
                   pstSubmit->stUserData.ulLen);
        ulRet = MSG_EncodeSubmit(pstSubmit, pstRawData);
        PS_MEM_FREE(WUEPS_PID_TAF, pstSubmit);
        return ulRet;
    }

    /*对于需要分段的消息，*/
    /*1.直接编码填充Concatenated short messages消息头部*/
    ulConcatUdhPos              = pstLongSubmit->stLongUserData.ucNumofHeaders;
    pstUserHeader               = &pstSubmit->stUserData.astUserDataHeader[ulConcatUdhPos];
    pstUserHeader->enHeaderID   = MN_MSG_UDH_CONCAT_8;/*存在Extended Object IE时用MN_MSG_UDH_CONCAT_16*/
    pstConcat_8                 = (MN_MSG_UDH_CONCAT_8_STRU *)&(pstUserHeader->u.stConcat_8);
    pstConcat_8->ucSeqNum       = 1;
    pstConcat_8->ucMr           = pstLongSubmit->ucMr;
    pstSubmit->stUserData.ucNumofHeaders++;
    pstSubmit->bUserDataHeaderInd   = VOS_TRUE;

    /*2.计算分段后消息包头长度*/
    MSG_GetUdhl(pstSubmit->stUserData.ucNumofHeaders,
                pstSubmit->stUserData.astUserDataHeader,
                &ulUdhl);
    if (ulUdhl >= (MN_MSG_MAX_8_BIT_LEN - 1))
    {
        MN_WARN_LOG("MN_MSG_Segment: the length of message is invalid.");
        PS_MEM_FREE(WUEPS_PID_TAF, pstSubmit);
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    /*3.计算分段后每个消息段中SM的长度: UDL最大长度去除UDHL和UDH的长度*/
    if (MN_MSG_MSG_CODING_7_BIT == pstLongSubmit->stDcs.enMsgCoding)
    {
        ulLen = MN_MSG_MAX_7_BIT_LEN - ((((ulUdhl + 1) * 8) + 6)/7);
    }
    else
    {
        ulLen = MN_MSG_MAX_8_BIT_LEN - (ulUdhl + 1);
    }

    /*计算分段后分段的个数并填充各分段的MN_MSG_SUBMIT_STRU结构数据*/
    *pucNum = (VOS_UINT8)((pstLongSubmit->stLongUserData.ulLen + (ulLen - 1))/ulLen);

    if (*pucNum > ulMaxSmsSegment)
    {
        MN_WARN_LOG("MN_MSG_Segment: the length overflow STK segment capability.");
        PS_MEM_FREE(WUEPS_PID_TAF, pstSubmit);
        return MN_ERR_CLASS_SMS_INTERNAL;
    }

    pstConcat_8->ucTotalNum     = (*pucNum);
    for (ulLoop = 0; ulLoop < (*pucNum); ulLoop++)
    {
        if (pstConcat_8->ucSeqNum == (*pucNum))
        {
            pstSubmit->stUserData.ulLen = pstLongSubmit->stLongUserData.ulLen - (ulLen * ulLoop);
        }
        else
        {
            pstSubmit->stUserData.ulLen = ulLen;
        }
        TAF_MEM_CPY_S(pstSubmit->stUserData.aucOrgData,
                   sizeof(pstSubmit->stUserData.aucOrgData),
                   &(pstLongSubmit->stLongUserData.pucOrgData[((VOS_ULONG)ulLen * ulLoop)]),
                   pstSubmit->stUserData.ulLen);
        ulRet = MSG_EncodeSubmit(pstSubmit, pstRawData);
        if ( MN_ERR_NO_ERROR != ulRet )
        {
            PS_MEM_FREE(WUEPS_PID_TAF, pstSubmit);
            return ulRet;
        }
        pstConcat_8->ucSeqNum++;
        pstRawData++;
    }

    PS_MEM_FREE(WUEPS_PID_TAF, pstSubmit);
    return MN_ERR_NO_ERROR;
}



VOS_VOID MN_MSG_EncodeTpRd(
    VOS_BOOL                            bRejectDuplicates,
    VOS_UINT8                          *pucTpFo
)
{
    /* 设置TP-RD为入参中的bRejectDuplicates */
    MSG_SET_TP_RD(*pucTpFo, bRejectDuplicates);

    return;
}


