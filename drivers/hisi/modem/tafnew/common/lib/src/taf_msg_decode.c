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

#include "ps_common_def.h"
#include "mn_error_code.h"
#include "mn_msg_api.h"
#include "taf_msg_ts.h"
#include "taf_std_lib.h"
#include "securec.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "nas_multi_instance_api.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_MNMSG_DECODE_C

/* 长短信包含短信的最大个数 */
#define MSG_LONG_MSG_VOLUME 255
/* 普通短信UD的最大长度 */
#define MSG_MAX_NORM_MSG_LEN 153

typedef struct {
    VOS_UINT8  ucFirstMsgSeqNum;
    VOS_UINT8  ucMr;
    VOS_UINT16 usMr;
    VOS_BOOL   b8bitConcatMsg;
} MN_MSG_CONCENTRATE_MSG_ATTR;

/* MN_BcdToAsciiCode、MN_BcdNumberToAscii、MN_MSG_BcdAddrToAscii调整到taf_std_lib.c中 */

/*lint +e429*/

VOS_VOID* MN_MSG_AllocForLib(VOS_UINT32 pid, VOS_UINT32 size)
{
    /* 该接口当前只有C核使用，下移C核后，ACPU宏控制部分不放开 */
    /* OSA_CPU_CCPU编译代码实现 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    return PS_MEM_ALLOC(pid, size);
#else

    /* OSA_CPU_CCPU编译代码实现 */
    VOS_VOID     *mem = VOS_NULL_PTR;
    ModemIdUint16 modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(pid);
    mem     = NAS_MULTIINSTANCE_MemAlloc(modemId, pid, size);

    return mem;
#endif
}
/*lint -e429*/

VOS_VOID MN_MSG_FreeForLib(VOS_UINT32 pid, VOS_VOID *addr)
{
    /* 该接口当前只有C核使用，下移C核后，ACPU宏控制部分不放开 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    PS_MEM_FREE(pid, addr);
    addr = VOS_NULL_PTR;
#else
    /* OSA_CPU_CCPU编译代码实现 */
    ModemIdUint16 modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(pid);

    NAS_MULTIINSTANCE_MemFree(modemId, pid, addr);
#endif
}

VOS_UINT32 MN_MSG_CheckUdl(const MN_MSG_DcsCode *dcsInfo, VOS_UINT8 udl, VOS_UINT32 userDataLen, VOS_UINT8 isStkSend)
{
    VOS_UINT32 septets;

    /* GSM压缩编码长度有效性检查 */
    if ((dcsInfo->msgCoding == MN_MSG_MSG_CODING_7_BIT) && (dcsInfo->compressed != VOS_TRUE)) {
        /* 消息中携带的用户数据长度超过协议允许的最大长度 */
        if ((udl > MN_MSG_MAX_7_BIT_LEN) && (isStkSend == VOS_FALSE)) {
            MN_WARN_LOG("MN_MSG_CheckUdl: syntax error.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        /* 消息中携带的用户数据长度超过消息的有效长度 */
        septets = userDataLen * MN_MSG_BITS_PER_OCTET / MN_MSG_BITS_PER_SEPTET;
        if (udl > septets) {
            MN_WARN_LOG("MN_MSG_CheckUdl: udl is invalid.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
    } else {
        /* 消息中携带的用户数据长度超过协议允许的最大长度 */
        if ((udl > MN_MSG_MAX_8_BIT_LEN) && (isStkSend == VOS_FALSE)) {
            MN_WARN_LOG("MN_MSG_CheckUdl: syntax error.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        /* 消息中携带的用户数据长度超过消息的有效长度 */
        if (udl > userDataLen) {
            MN_WARN_LOG("MN_MSG_CheckUdl: udl is invalid.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_CheckStaRptUdl(const MN_MSG_DcsCode *dcsInfo, VOS_UINT8 udl, VOS_UINT32 userDataLen)
{
    VOS_UINT32 maxDataLen;

    maxDataLen = TAF_MIN(userDataLen, MN_MSG_MAX_STARPT_DATA_LEN);

    if ((dcsInfo->msgCoding == MN_MSG_MSG_CODING_7_BIT) && (dcsInfo->compressed != VOS_TRUE)) {
        maxDataLen = maxDataLen * MN_MSG_BITS_PER_OCTET / MN_MSG_BITS_PER_SEPTET;
    }

    if (udl > maxDataLen) {
        MN_WARN_LOG("MN_MSG_CheckStaRptUdl: syntax error.");
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_BcdAddrToAscii(const MN_MSG_BcdAddr *bcdAddr, MN_MSG_AsciiAddr *asciiAddr)
{
    VOS_UINT32 ret;

    if ((bcdAddr == VOS_NULL_PTR) || (asciiAddr == VOS_NULL_PTR)) {
        MN_WARN_LOG("MN_MSG_BcdAddrToAscii: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    asciiAddr->numType = ((bcdAddr->addrType >> 4) & 0x07);
    asciiAddr->numPlan = (bcdAddr->addrType & 0x0f);
    if ((bcdAddr->bcdNum[bcdAddr->bcdLen - 1] & 0xF0) != 0xF0) {
        asciiAddr->len = bcdAddr->bcdLen * 2;
    } else {
        asciiAddr->len = (bcdAddr->bcdLen * 2) - 1;
    }

    if (asciiAddr->len > MN_MAX_ASCII_ADDRESS_NUM) {
        MN_WARN_LOG("MN_MSG_BcdAddrToAscii: length of number is invalid.");
        return MN_ERR_INVALIDPARM;
    }

    ret = TAF_STD_ConvertBcdNumberToAscii(bcdAddr->bcdNum, bcdAddr->bcdLen, (VOS_CHAR *)asciiAddr->asciiNum,
                                          sizeof(asciiAddr->asciiNum));
    if (ret == MN_ERR_NO_ERROR) {
        return MN_ERR_NO_ERROR;
    } else {
        return MN_ERR_INVALIDPARM;
    }
}

LOCAL VOS_UINT32 MSG_DecodeTimeStamp(const VOS_UINT8 *timeStamp, MN_MSG_Timestamp *timeStampInfo, VOS_UINT32 *len)
{
    VOS_BOOL   bNegativeFlag = VOS_FALSE;
    VOS_UINT8  timeZone;
    VOS_UINT8  absTimeZone;
    VOS_UINT32 ret;
    VOS_UINT8  dateInvalidType;

    if ((timeStamp == VOS_NULL_PTR) || (timeStampInfo == VOS_NULL_PTR) || (len == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeTimeStamp: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    MN_MSG_REVERSE_BCD(timeStampInfo->year, timeStamp[0]);
    MN_MSG_REVERSE_BCD(timeStampInfo->month, timeStamp[1]);
    MN_MSG_REVERSE_BCD(timeStampInfo->day, timeStamp[2]);
    MN_MSG_REVERSE_BCD(timeStampInfo->hour, timeStamp[3]);
    MN_MSG_REVERSE_BCD(timeStampInfo->minute, timeStamp[4]);
    MN_MSG_REVERSE_BCD(timeStampInfo->second, timeStamp[5]);

    ret = MN_MSG_ChkDate(timeStampInfo, &dateInvalidType);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeTimeStamp: Date is invalid.");
    }

    if ((MN_MSG_TIMESTAMP_SIGN_MASK & timeStamp[6]) != 0) {
        bNegativeFlag = VOS_TRUE;
        /* 将时区符号标志位BIT3置0 */
        absTimeZone = timeStamp[6] ^ MN_MSG_TIMESTAMP_SIGN_MASK;
    } else {
        absTimeZone = timeStamp[6];
    }

    ret = TAF_STD_ConvertBcdToDeciDigit(absTimeZone, VOS_TRUE, &timeZone);
    if ((ret != MN_ERR_NO_ERROR) || (timeZone > MN_MSG_MAX_TIMEZONE_VALUE)) {
        MN_WARN_LOG("MSG_DecodeTimeStamp: Time zone is invalid.");
    }

    timeStampInfo->timezone = (VOS_INT8)timeZone;
    if (bNegativeFlag == VOS_TRUE) {
        timeStampInfo->timezone *= (-1);
    }

    *len = MN_MSG_ABSOLUTE_TIMESTAMP_LEN;
    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MSG_ConvertBcdNumberToAsciiWithCheckParaAvail(const VOS_UINT8 *bcdNumber, VOS_UINT8 bcdLen,
                                                         const VOS_CHAR *pcAsciiNumber, VOS_UINT8 *numberLength)
{
    VOS_UINT32 len;

    if ((bcdNumber == VOS_NULL_PTR) || (pcAsciiNumber == VOS_NULL_PTR)) {
        return MN_ERR_NULLPTR;
    }

    /* 整理号码字符串，去除无效的0XFF数据 */
    while (bcdLen > 1) {
        if (bcdNumber[bcdLen - 1] == 0xFF) {
            bcdLen--;
        } else {
            break;
        }
    }
    if (bcdLen == 0) {
        return MN_ERR_INVALIDPARM;
    }

    /*
     * 判断pucBcdAddress所指向的字符串的最后一个字节的高位是否为1111，
     * 如果是，说明号码位数为奇数，否则为偶数
     */
    if ((bcdNumber[bcdLen - 1] & 0xF0) == 0xF0) {
        len = (VOS_UINT32)(bcdLen * 2) - 1;
    } else {
        len = (VOS_UINT32)(bcdLen * 2);
    }

    if (len > MN_MSG_UINT8_MAX) {
        return MN_ERR_INVALIDPARM;
    }

    *numberLength = (VOS_UINT8)len;

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MSG_ConvertBcdNumberToAscii(const VOS_UINT8 *bcdNumber, VOS_UINT8 bcdLen, VOS_CHAR *pcAsciiNumber)
{
    VOS_CHAR   asciiNumber;
    VOS_UINT8  loop;
    VOS_UINT8  len;
    VOS_UINT8  bcdCode;
    VOS_UINT32 ret;

    ret = MSG_ConvertBcdNumberToAsciiWithCheckParaAvail(bcdNumber, bcdLen, pcAsciiNumber, &len);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    /* 解析号码 */
    /* 目标地址总长度为MN_MAX_ASCII_ADDRESS_NUM + 1,循环留1个字节写\0 */
    for (loop = 0; loop < TAF_MIN(len, MN_MAX_ASCII_ADDRESS_NUM); loop++) {
        /* 判断当前解码的是奇数位号码还是偶数位号码，从0开始，是偶数 */
        if ((loop % 2) == 1) {
            /* 如果是奇数位号码，则取高4位的值 */
            bcdCode = ((bcdNumber[(loop / 2)] >> 4) & 0x0F);
        } else {
            /* 如果是偶数位号码，则取低4位的值 */
            bcdCode = (bcdNumber[(loop / 2)] & 0x0F);
        }

        /* 将二进制数字转换成Ascii码形式, 无效填充值字符F用字符‘0’替代 */
        asciiNumber = 0;

        ret = TAF_STD_ConvertBcdCodeToAscii(bcdCode, &asciiNumber);
        if (ret != MN_ERR_NO_ERROR) {
            if (ret != MN_ERR_INVALID_BCD) {
                return ret;
            }

            MN_WARN_LOG("MSG_ConvertBcdNumberToAscii: The BCD number is invalid.");
            asciiNumber = '0';
        }

        pcAsciiNumber[loop] = asciiNumber;
    }

    pcAsciiNumber[loop] = '\0'; /* 字符串末尾为0 */

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_DecodeAddress(const VOS_UINT8 *addr, VOS_BOOL bRpAddr, MN_MSG_AsciiAddr *asciiAddr, VOS_UINT32 *len)
{
    VOS_UINT32 ret;
    VOS_UINT32 pos = 0;
    VOS_UINT8  bcdLen;

    if ((len == VOS_NULL_PTR) || (addr == VOS_NULL_PTR) || (asciiAddr == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_DecodeAddress: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(asciiAddr, sizeof(MN_MSG_AsciiAddr), 0x00, sizeof(MN_MSG_AsciiAddr));
    /* 3GPP 24011 RP DATA (Network to Mobile Station) */
    if (bRpAddr == VOS_TRUE) {
        /* 从长度中去除号码类型及编码类型的长度(1 OCTET) */
        bcdLen = addr[pos];
        pos++;
        /* 地址长度为0直接返回 */
        if (bcdLen == 0) {
            *len = pos;
            return MN_ERR_NO_ERROR;
        }
        bcdLen -= 1;

        if ((bcdLen == 0) || (bcdLen > MN_MSG_MAX_BCD_NUM_LEN)) {
            MN_WARN_LOG("MN_MSG_DecodeAddress:Invalid RP-ADDR.");
            return MN_ERR_CLASS_SMS_INVALID_SCADDR;
        }

        if ((addr[(VOS_ULONG)((pos + 1) + (bcdLen - (VOS_UINT8)1))] & 0xF0) != 0xF0) {
            asciiAddr->len = bcdLen * 2;
        } else {
            asciiAddr->len = (bcdLen * 2) - 1;
        }
    } else {
        asciiAddr->len = addr[pos];
        pos++;
        if (asciiAddr->len == 0) {
            asciiAddr->numPlan = addr[pos] & 0x0f;
            asciiAddr->numType = (addr[pos] >> 4) & 0x07;
            *len               = pos + 1;
            return MN_ERR_NO_ERROR;
        }

        bcdLen = (VOS_UINT8)((asciiAddr->len + 1) / 2);
    }

    if ((bcdLen == 0) || (bcdLen > MN_MSG_MAX_BCD_NUM_LEN)) {
        MN_WARN_LOG("MN_MSG_DecodeAddress: invalid address length.");
        return MN_ERR_CLASS_SMS_INVALID_ADDRLEN;
    }

    asciiAddr->numPlan = addr[pos] & 0x0f;
    asciiAddr->numType = (addr[pos] >> 4) & 0x07;

    pos++;
    if ((bRpAddr != VOS_TRUE) && (asciiAddr->numType == MN_MSG_TON_ALPHANUMERIC)) {
        asciiAddr->len = TAF_MIN((asciiAddr->len * 4) / 7, (MN_MAX_ASCII_ADDRESS_NUM + 1));
        ret            = TAF_STD_UnPack7Bit(&(addr[pos]), asciiAddr->len, 0, asciiAddr->asciiNum);
        if (ret != VOS_OK) {
            ret = MN_ERR_CLASS_INVALID_TP_ADDRESS;
        } else {
            ret = MN_ERR_NO_ERROR;
        }
    } else {
        ret = TAF_STD_ConvertBcdNumberToAscii(&(addr[pos]), bcdLen, (VOS_CHAR *)asciiAddr->asciiNum,
                                              sizeof(asciiAddr->asciiNum));
        if (ret == MN_ERR_INVALID_BCD) {
            ret = MSG_ConvertBcdNumberToAscii(&(addr[pos]), bcdLen, (VOS_CHAR *)asciiAddr->asciiNum);
        }
    }
    *len = 2 + bcdLen;

    return ret;
}

VOS_UINT32 MN_MSG_DecodeValidPeriod(MN_MSG_ValidPeriodUint8 vpf, const VOS_UINT8 *validPeriod,
                                    MN_MSG_ValidPeriod *validPeriodInfo, VOS_UINT32 *len)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    if ((len == VOS_NULL_PTR) || (validPeriod == VOS_NULL_PTR) || (validPeriodInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_DecodeValidPeriod: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    switch (vpf) {
        case MN_MSG_VALID_PERIOD_NONE:
            *len = 0;
            break;
        case MN_MSG_VALID_PERIOD_RELATIVE:
            *len                         = 1;
            validPeriodInfo->u.otherTime = validPeriod[0];
            break;
        case MN_MSG_VALID_PERIOD_ABSOLUTE:
            ret = MSG_DecodeTimeStamp((VOS_UINT8 *)validPeriod, &(validPeriodInfo->u.absoluteTime), len);
            break;
        default:
            MN_WARN_LOG("MN_MSG_DecodeValidPeriod: invalid VP flag.");
            ret = MN_ERR_CLASS_SMS_INVALID_VPF;
            break;
    }
    validPeriodInfo->validPeriod = vpf;

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeUdhConcat8(const VOS_UINT8 udhlMax, const VOS_UINT8 *udh, MN_MSG_UdhConcat8 *concatenatedSms,
                                      VOS_UINT8 *ieiLen)
{
    VOS_UINT8 pos        = 1;
    VOS_UINT8 ieiLenTemp = 0;

    if ((udh == VOS_NULL_PTR) || (concatenatedSms == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUdhConcat8: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if ((MN_MSG_UDH_CONCAT_8_IEL + MN_MSG_IE_HEAD_LEN) > udhlMax) {
        MN_WARN_LOG("MSG_DecodeUdhConcat8: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    ieiLenTemp = udh[pos];
    pos++;
    if (ieiLenTemp != MN_MSG_UDH_CONCAT_8_IEL) {
        MN_WARN_LOG("MSG_DecodeUdhConcat8: invalid IE length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    concatenatedSms->mr = udh[pos];
    pos++;
    concatenatedSms->totalNum = udh[pos];
    pos++;
    concatenatedSms->seqNum = udh[pos];

    ieiLenTemp += 2;

    *ieiLen = ieiLenTemp;
    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_DecodeUdhSpecialSms(const VOS_UINT8 udhlMax, const VOS_UINT8 *udh,
                                         MN_MSG_UdhSpecialSms *specialSms, VOS_UINT8 *ieiLen)
{
    VOS_UINT8 pos        = 1;
    VOS_UINT8 ieiLenTemp = 0;

    if ((udh == VOS_NULL_PTR) || (specialSms == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUdhSpecialSms: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (udhlMax < (MN_MSG_UDH_SPECIAL_SM_IEL + MN_MSG_IE_HEAD_LEN)) {
        MN_WARN_LOG("MSG_DecodeUdhSpecialSms: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    ieiLenTemp = udh[pos];
    pos++;
    if (ieiLenTemp != MN_MSG_UDH_SPECIAL_SM_IEL) {
        MN_WARN_LOG("MSG_DecodeUdhSpecialSms: invalid IE length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }
    if ((udh[pos] & 0x80) != 0) {
        specialSms->msgWaiting = MN_MSG_MSG_WAITING_STORE;
    } else {
        specialSms->msgWaiting = MN_MSG_MSG_WAITING_DISCARD;
    }
    specialSms->profileId      = (udh[pos] >> 5) & 0x03;
    specialSms->extMsgInd      = (udh[pos] >> 2) & 0x07;
    specialSms->msgWaitingKind = udh[pos] & 0x03;
    pos++;
    specialSms->msgCount = udh[pos];

    ieiLenTemp += 2;

    *ieiLen = ieiLenTemp;

    return MN_ERR_NO_ERROR;
}

/*
 * MSG_DecodeUdhAppPort_8
 * MSG_DecodeUdhAppPort_16
 * MSG_DecodeUdhSmscCtrl
 * MSG_DecodeUdhSourceInd
 * MSG_DecodeUdhUserDefSound
 * MSG_DecodeUdhPreDefAnim
 * MSG_DecodeUdhTextFormat
 * MSG_DecodeUdhSmallAnim
 * MSG_DecodeUdhLargePic
 * MSG_DecodeUdhSmallPic
 * MSG_DecodeUdhVarPic
 * MSG_DecodeUdhUserPrompt
 * MSG_DecodeUdhRfc822
 * MSG_DecodeEo
 * MSG_DecodeReo
 * MSG_DecodeCc
 * MSG_DecodeObjDistrInd
 * MSG_DecodeHyperLinkFormat
 * MSG_DecodeReplyAddress
 * MN_MSG_UnpackSmWithOutUdh
 * MSG_DecodeWvgObj
 * MSG_DecodeUdhConcat16
 */

LOCAL VOS_UINT32 MSG_DecodeUdhOther(const VOS_UINT8 udhlMax, const VOS_UINT8 *udh, MN_MSG_UdhOther *other,
                                    VOS_UINT8 *ieiLen)
{
    errno_t memResult;
    VOS_UINT8 pos;
    VOS_UINT8 ieiLenTemp = 0;

    if ((udh == VOS_NULL_PTR) || (other == VOS_NULL_PTR) || (ieiLen == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUdhOther: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    pos = 0;

    other->udhType = udh[pos];
    pos++;

    ieiLenTemp = udh[pos];

    if ((ieiLenTemp + MN_MSG_IE_HEAD_LEN) > MN_MSG_UDH_OTHER_SIZE) {
        MN_WARN_LOG("MSG_DecodeUdhOther: invalid length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    if ((ieiLenTemp + MN_MSG_IE_HEAD_LEN) > udhlMax) {
        MN_WARN_LOG("MSG_DecodeUdhOther: invalid IE length.");
        return MN_ERR_CLASS_INVALID_TP_UD;
    }

    other->len = ieiLenTemp;
    if (other->len > 0) {
        memResult = memcpy_s(other->data, sizeof(other->data), (udh + 2), other->len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(other->data), other->len);
    }

    ieiLenTemp += 2;

    *ieiLen = ieiLenTemp;

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_DecodeUdh(const VOS_UINT8 *udh, VOS_UINT8 udhl, VOS_UINT8 *numofHeaders,
                               MN_MSG_UserHeaderType *userDataHeader)
{
    VOS_UINT32 len;
    VOS_UINT8  ieiLen;
    VOS_UINT32 ret;
    VOS_UINT8  waitDecodeLen;

    if ((udh == VOS_NULL_PTR) || (numofHeaders == VOS_NULL_PTR) || (userDataHeader == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUdh: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    len           = 0;
    waitDecodeLen = udhl;
    *numofHeaders = 0;

    while (len < udhl) {
        if (*numofHeaders >= MN_MSG_MAX_UDH_NUM) {
            MN_WARN_LOG("MSG_DecodeUdh: UDH number is overflow.");
            return MN_ERR_CLASS_INVALID_TP_UD;
        }
        waitDecodeLen = udhl - (VOS_UINT8)len;
        if (waitDecodeLen < MN_MSG_IE_HEAD_LEN) {
            MN_WARN_LOG("MSG_DecodeUdh: IE length is invalid.");
            return MN_ERR_CLASS_INVALID_TP_UD;
        }
        userDataHeader->headerID = udh[len];
        switch (userDataHeader->headerID) {
            case MN_MSG_UDH_CONCAT_8:
                ret = MSG_DecodeUdhConcat8(waitDecodeLen, &udh[len], &userDataHeader->u.concat8, &ieiLen);
                break;
            case MN_MSG_UDH_SPECIAL_SM:
                ret = MSG_DecodeUdhSpecialSms(waitDecodeLen, &udh[len], &userDataHeader->u.specialSms, &ieiLen);
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
             * MN_MSG_UDH_SMALL_ANIM
             * MN_MSG_UDH_LARGE_PIC
             * MN_MSG_UDH_SMALL_PIC
             * MN_MSG_UDH_VAR_PIC
             * MN_MSG_UDH_USER_PROMPT
             * MN_MSG_UDH_RFC822
             * MN_MSG_UDH_EO
             * MN_MSG_UDH_REO
             * MN_MSG_UDH_OBJ_DISTR_IND
             * MN_MSG_UDH_HYPERLINK_FORMAT
             * MN_MSG_UDH_REPLY_ADDR
             * MN_MSG_UDH_CHAR_SIZE_WVG_OBJ
             * MN_MSG_UDH_STD_WVG_OBJ
             */
            default:
                ret = MSG_DecodeUdhOther(waitDecodeLen, &udh[len], &userDataHeader->u.other, &ieiLen);
                break;
        }

        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }

        if ((ieiLen == 0) || (ieiLen > (MN_MSG_UINT8_MAX - len))) {
            MN_WARN_LOG("MSG_DecodeUdh: IE length is overflow.");
            return MN_ERR_CLASS_INVALID_TP_UD;
        }

        len += ieiLen;


        (*numofHeaders)++;
        userDataHeader++;
    }
    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MN_MSG_UnpackSmWithOutUdh(const MN_MSG_DcsCode *dcsInfo, const VOS_UINT8 *userData,
                                           VOS_UINT8 *unpackedSm)
{
    errno_t    memResult;
    VOS_UINT32 ret;
    VOS_UINT8  udl;
    VOS_UINT32 pos = 0;

    udl = userData[pos];
    pos++;
    /* decode SM, Refer to 23040 9.2.3.16 */
    if ((dcsInfo->msgCoding == MN_MSG_MSG_CODING_7_BIT) && (dcsInfo->compressed != VOS_TRUE)) {
        ret = TAF_STD_UnPack7Bit(&userData[pos], udl, 0, unpackedSm);

        if (ret != VOS_OK) {
            ret = MN_ERR_CLASS_INVALID_TP_UD;
        } else {
            ret = MN_ERR_NO_ERROR;
        }
    } else {
        if (udl > 0) {
            memResult = memcpy_s(unpackedSm, MN_MSG_MAX_LEN, &userData[pos], udl);
            TAF_MEM_CHK_RTN_VAL(memResult, MN_MSG_MAX_LEN, udl);
        }
        ret = MN_ERR_NO_ERROR;
    }

    return ret;
}

LOCAL VOS_UINT32 MN_MSG_CheckUdhl(const MN_MSG_DcsCode *dcsInfo, VOS_UINT8 udl, VOS_UINT8 udhl)
{
    if ((dcsInfo->msgCoding == MN_MSG_MSG_CODING_7_BIT) && (dcsInfo->compressed != VOS_TRUE)) {
        if (udl < ((((udhl + 1) * 8) + 6) / 7)) {
            MN_WARN_LOG("MN_MSG_CheckUdhl: udhl is invalid.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
    } else {
        if ((udhl + 1) > udl) {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
    }

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MN_MSG_UpackSmWithUdh(const MN_MSG_DcsCode *dcsInfo, const VOS_UINT8 *userData, VOS_BOOL bLongMsg,
                                       VOS_VOID *userDataInfo)
{
    errno_t                memResult;
    MN_MSG_UserData       *userDataTemp    = VOS_NULL_PTR;
    MN_MSG_LongUserData   *longMsgUserData = VOS_NULL_PTR;
    VOS_UINT32             ret;
    VOS_UINT32             pos;
    VOS_UINT8              udhl;
    VOS_UINT8              udl;
    VOS_UINT8              fillBit;
    VOS_UINT8             *numofHeaders   = VOS_NULL_PTR;
    MN_MSG_UserHeaderType *userDataHeader = VOS_NULL_PTR;
    VOS_UINT8             *unpackedSm     = VOS_NULL_PTR;
    VOS_UINT32            *len            = VOS_NULL_PTR;

    if (bLongMsg == VOS_TRUE) {
        longMsgUserData = (MN_MSG_LongUserData *)userDataInfo;
        numofHeaders    = &longMsgUserData->numofHeaders;
        userDataHeader  = longMsgUserData->userDataHeader;
        unpackedSm      = longMsgUserData->orgData;
        len             = &longMsgUserData->len;
    } else {
        userDataTemp   = (MN_MSG_UserData *)userDataInfo;
        numofHeaders   = &userDataTemp->numofHeaders;
        userDataHeader = userDataTemp->userDataHeader;
        unpackedSm     = userDataTemp->orgData;
        len            = &userDataTemp->len;
    }

    pos = 0;
    udl = userData[pos];
    pos++;
    udhl = userData[pos];
    pos++;

    ret = MN_MSG_CheckUdhl(dcsInfo, udl, udhl);

    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    ret = MSG_DecodeUdh(&(userData[pos]), udhl, numofHeaders, userDataHeader);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    /* 将数据区数组下标移至UDHL UDH之后 */
    pos += udhl;

    /* decode SM, Refer to 23040 9.2.3.16 */
    if ((dcsInfo->msgCoding == MN_MSG_MSG_CODING_7_BIT) && (dcsInfo->compressed != VOS_TRUE)) {
        fillBit = (7 - (((udhl + 1) * 8) % 7)) % 7;
        if (udl < ((((udhl + 1) * 8) + 6) / 7)) {
            MN_WARN_LOG("MN_MSG_UpackSmWithUdh: udhl is invalid.");
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }
        *len = udl - ((((udhl + 1) * 8) + 6) / 7);
        ret = TAF_STD_UnPack7Bit(&(userData[pos]), *len, fillBit, unpackedSm);
        if (ret != VOS_OK) {
            ret = MN_ERR_CLASS_INVALID_TP_UD;
        } else {
            ret = MN_ERR_NO_ERROR;
        }
    } else {
        if ((udhl + 1) > udl) {
            return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
        }

        *len = udl - (udhl + 1);

        if (*len > 0) {
            memResult = memcpy_s(unpackedSm, MN_MSG_MAX_LEN, &(userData[pos]), *len);
            TAF_MEM_CHK_RTN_VAL(memResult, MN_MSG_MAX_LEN, *len);
        }
        ret = MN_ERR_NO_ERROR;
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeUserData_ForUsimMsg(VOS_BOOL bUserDataHeaderInd, const MN_MSG_DcsCode *dcsInfo,
                                               const MN_MSG_DecodeUserData *smsUserData, MN_MSG_LongUserData *userData)
{
    VOS_UINT32 ret;
    VOS_UINT32 pos = 0;
    VOS_UINT8  udl;

    if ((dcsInfo == VOS_NULL_PTR) || (smsUserData == VOS_NULL_PTR) || (userData == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUserData_ForUsimMsg: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    userData->numofHeaders = 0;
    memset_s(userData->userDataHeader, sizeof(userData->userDataHeader), 0x00,
             (VOS_SIZE_T)(sizeof(MN_MSG_UserHeaderType) * MN_MSG_MAX_UDH_NUM));
    udl = smsUserData->srcOct[pos];
    if (udl == 0) {
        userData->len = 0;
        return MN_ERR_NO_ERROR;
    }

    pos++;

    ret = MN_MSG_CheckUdl(dcsInfo, udl, (smsUserData->smsUserDataLen - pos), VOS_TRUE);
    if (ret != MN_ERR_NO_ERROR) {
        userData->len = 0;
        return MN_ERR_NO_ERROR;
    }
    if (bUserDataHeaderInd == VOS_TRUE) {
        ret = MN_MSG_UpackSmWithUdh(dcsInfo, smsUserData->srcOct, VOS_TRUE, (VOS_VOID *)userData);
    } else {
        /* decode SM, Refer to 23040 9.2.3.16 */
        userData->len = udl;

        ret = MN_MSG_UnpackSmWithOutUdh(dcsInfo, smsUserData->srcOct, userData->orgData);
    }

    return ret;
}

LOCAL VOS_UINT32 MN_MSG_GetOrigUserData(const MN_MSG_RawTsData *smsRawDataInfo, VOS_UINT32 pos,
                                        MN_MSG_DecodeUserData *origUserData)
{
    VOS_UINT32 userDataLen;

    userDataLen = TAF_MIN(smsRawDataInfo->len, MN_MSG_MAX_LEN);

    /* 如果此时已经越界，直接返回异常，并保证下面的减法运算不会反转 */
    if ((smsRawDataInfo->tpduType == MN_MSG_TPDU_DELIVER) || (smsRawDataInfo->tpduType == MN_MSG_TPDU_SUBMIT)) {
        if (pos >= userDataLen) {
            MN_ERR_LOG("MN_MSG_GetOrigUserData: Invalid TPDU");
            return MN_ERR_CLASS_SMS_INVALID_TPDU;
        }
    } else {
        if (pos > userDataLen) {
            MN_ERR_LOG("MN_MSG_GetOrigUserData: Invalid TPDU");
            return MN_ERR_CLASS_SMS_INVALID_TPDU;
        }
    }

    origUserData->smsUserDataLen = userDataLen - pos;
    origUserData->srcOct         = (VOS_UINT8 *)&(smsRawDataInfo->data[pos]);

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 TAF_MSG_DecodeLongSubmitExceptUserData_UsimMsg(const MN_MSG_RawTsData *tsRawData, VOS_UINT32 *pos,
                                                          MN_MSG_SubmitLong *longSubmit)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;
    VOS_UINT32 daLen = 0;
    VOS_UINT32 vpLen = 0;

    /* TP DA* Refer to 9.1.2.5 */
    ret = MN_MSG_DecodeAddress(&(tsRawData->data[*pos]), VOS_FALSE, &(longSubmit->destAddr), &daLen);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("TAF_MSG_DecodeLongSubmitExceptUserData_UsimMsg: Fail to MN_MSG_DecodeAddress.");
        return ret;
    }
    (*pos) += daLen;

    /* TP PID */
    longSubmit->pid = tsRawData->data[(*pos)++];

    /* TP DCS 23038 4 */
    ret = MN_MSG_DecodeDcs(tsRawData->data[*pos], &(longSubmit->dcs));
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("TAF_MSG_DecodeLongSubmitExceptUserData_UsimMsg: Fail to MN_MSG_DecodeDcs..");
        return ret;
    }
    (*pos)++;

    /* TP VP */
    ret = MN_MSG_DecodeValidPeriod(longSubmit->validPeriod.validPeriod, &(tsRawData->data[*pos]),
                                   &(longSubmit->validPeriod), &vpLen);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("TAF_MSG_DecodeLongSubmitExceptUserData_UsimMsg: Fail to MN_MSG_DecodeValidPeriod.");
        return ret;
    }
    (*pos) += vpLen;

    return ret;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_Decode_UsimMsg(VOS_UINT8 *data, VOS_UINT32 len, MN_MSG_SubmitLong *longSubmit)
{
    errno_t               memResult;
    VOS_UINT32            ret          = MN_ERR_NO_ERROR;
    VOS_UINT32            pos          = 0;
    MN_MSG_LongUserData  *longUserData = VOS_NULL_PTR;
    MN_MSG_RawTsData     *tsRawData    = VOS_NULL_PTR;
    MN_MSG_DecodeUserData origUserData;

    if ((data == VOS_NULL_PTR) || (longSubmit == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_Decode_UsimMsg: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    tsRawData = (MN_MSG_RawTsData *)MN_MSG_AllocForLib(WUEPS_PID_TAF, sizeof(MN_MSG_RawTsData));
    if (tsRawData == VOS_NULL_PTR) {
        MN_WARN_LOG("MN_MSG_Decode_UsimMsg: fail to alloc memory for pstTsRawData. ");
        return MN_ERR_NOMEM;
    }

    memset_s(tsRawData, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));

    tsRawData->tpduType = MN_MSG_TPDU_SUBMIT;

    tsRawData->len = TAF_MIN(len, MN_MSG_MAX_LEN);

    if (tsRawData->len > 0) {
        memResult = memcpy_s(tsRawData->data, sizeof(tsRawData->data), data, tsRawData->len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsRawData->data), tsRawData->len);
    }

    /*
     * b7  b6   b5   b4   b3  b2  b1  b0
     * RP  UDHI SRR  VPF      RD  MTI
     * 0   0    0    2        1   2
     * TP MTI TP RD TP VPF TP RP TP UDHI TP SRR
     */
    /* TP MTI ignore 23040 9.2.3.1 填写bit0bit1:MIT     0011 0001 */

    /* TP RD  23040 9.2.3.25 */
    MSG_GET_TP_RD(longSubmit->rejectDuplicates, tsRawData->data[pos]);
    /* TP VPF 23040 9.2.3.3 */
    MSG_GET_TP_VPF(longSubmit->validPeriod.validPeriod, tsRawData->data[pos]);
    /* TP RP  23040 9.2.3.17 */
    MSG_GET_TP_RP(longSubmit->replayPath, tsRawData->data[pos]);
    /* TP UDHI23040 9.2.3.23 */
    MSG_GET_TP_UDHI(longSubmit->userDataHeaderInd, tsRawData->data[pos]);
    /* TP SRR 23040 9.2.3.5 */
    MSG_GET_TP_SRR(longSubmit->staRptReq, tsRawData->data[pos]);
    pos++;

    /* TP MR */
    longSubmit->mr = tsRawData->data[pos];
    pos++;
    ret = TAF_MSG_DecodeLongSubmitExceptUserData_UsimMsg(tsRawData, &pos, longSubmit);

    if (ret != MN_ERR_NO_ERROR) {
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        return ret;
    }

    /* UDL UD */
    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(tsRawData, pos, &origUserData);

    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MN_MSG_Decode_UsimMsg: Invalid TPDU");
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        return ret;
    }

    longUserData = &(longSubmit->longUserData);
    ret = MSG_DecodeUserData_ForUsimMsg(longSubmit->userDataHeaderInd, &(longSubmit->dcs), &origUserData, longUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_NORM_LOG("MN_MSG_Decode_UsimMsg: Fail to MSG_DecodeUserData_ForUsimMsg.");
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        return ret;
    }

    MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MSG_DecodeUserData(VOS_BOOL bUserDataHeaderInd, const MN_MSG_DcsCode *dcsInfo,
                                    MN_MSG_DecodeUserData *smsUserData, MN_MSG_UserData *userData)
{
    VOS_UINT32 ret;
    VOS_UINT32 pos = 0;
    VOS_UINT8  udl;

    if ((dcsInfo == VOS_NULL_PTR) || (smsUserData == VOS_NULL_PTR) || (userData == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeUserData: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (smsUserData->smsUserDataLen == 0) {
        MN_WARN_LOG("MSG_DecodeUserData: SmsUserDataLen Too Short");
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }
    udl = smsUserData->srcOct[pos];
    pos++;

    if (udl == 0) {
        userData->len = 0;
        return MN_ERR_NO_ERROR;
    }

    if (smsUserData->isSmsStaRptType == VOS_TRUE) {
        ret = MN_MSG_CheckStaRptUdl(dcsInfo, udl, (smsUserData->smsUserDataLen - pos));
        if (ret != MN_ERR_NO_ERROR) {
            MN_ERR_LOG("MSG_DecodeUserData: UDL is invalid.");
            return ret;
        }
    }
    else {
        /* UDL长度有效性检查 */
        ret = MN_MSG_CheckUdl(dcsInfo, udl, (smsUserData->smsUserDataLen - pos), VOS_FALSE);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
    }

    if (bUserDataHeaderInd == VOS_TRUE) {
        ret = MN_MSG_UpackSmWithUdh(dcsInfo, smsUserData->srcOct, VOS_FALSE, (VOS_VOID *)userData);
        if (ret == MN_ERR_NO_ERROR) {
            return ret;
        } else {
            memset_s(userData, sizeof(MN_MSG_UserData), 0x00, sizeof(MN_MSG_UserData));
        }
    }

    /* decode SM, Refer to 23040 9.2.3.16 */
    userData->len = udl;

    ret = MN_MSG_UnpackSmWithOutUdh(dcsInfo, smsUserData->srcOct, userData->orgData);

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeDeliver(const MN_MSG_RawTsData *smsRawDataInfo, MN_MSG_Deliver *smsDeliverInfo)
{
    VOS_UINT32            pos = 0;
    VOS_UINT32            oaLen;
    VOS_UINT32            ret;
    VOS_UINT32            sctsLen;
    MN_MSG_DecodeUserData origUserData;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeDeliver: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (smsRawDataInfo->len < MN_MSG_DELIVER_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    memset_s(smsDeliverInfo, sizeof(MN_MSG_Deliver), 0x00, sizeof(MN_MSG_Deliver));

    /* decode fo:TP MTI TP MMS TP RP TP UDHI TP SRI */
    MSG_GET_TP_MMS(smsDeliverInfo->moreMsg, smsRawDataInfo->data[pos]);
    MSG_GET_TP_RP(smsDeliverInfo->replayPath, smsRawDataInfo->data[pos]);
    MSG_GET_TP_UDHI(smsDeliverInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    MSG_GET_TP_SRI(smsDeliverInfo->staRptInd, smsRawDataInfo->data[pos]);

    pos++;

    /* decode TP OA: */
    ret = MN_MSG_DecodeAddress(&(smsRawDataInfo->data[pos]), VOS_FALSE, &(smsDeliverInfo->origAddr), &oaLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += oaLen;

    /* decode TP PID */
    smsDeliverInfo->pid = smsRawDataInfo->data[pos];
    pos++;

    /* decode TP DCS */
    ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &smsDeliverInfo->dcs);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos++;

    /* decode TP SCTS */
    ret = MSG_DecodeTimeStamp((VOS_UINT8 *)&(smsRawDataInfo->data[pos]), &(smsDeliverInfo->timeStamp), &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeDeliver: Invalid SCTS.");
        return ret;
    }
    pos += sctsLen; /* TAF_SMS_SCTS_LEN 7 */

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);

    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeDeliver: Invalid TPDU");
        return ret;
    }

    /* decode TP UDL TP UD */
    ret = MSG_DecodeUserData(smsDeliverInfo->userDataHeaderInd, &(smsDeliverInfo->dcs), &origUserData,
                             &(smsDeliverInfo->userData));

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeDeliverRptAck(const MN_MSG_RawTsData *smsRawDataInfo,
                                         MN_MSG_DeliverRptAck   *smsDeliverRptAckInfo)
{
    VOS_UINT32            pos = 0;
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    MN_MSG_DecodeUserData origUserData;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverRptAckInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeDeliverRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsDeliverRptAckInfo, sizeof(MN_MSG_DeliverRptAck), 0x00, sizeof(MN_MSG_DeliverRptAck));

    if (smsRawDataInfo->len == 0) {
        MN_WARN_LOG("MSG_DecodeDeliverRptAck: no TPDU");
        return MN_ERR_NO_ERROR;
    }

    if (smsRawDataInfo->len < MN_MSG_DELIVER_RPTACK_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    /* TP MTI ignore  TP-UDHI  */
    MSG_GET_TP_UDHI(smsDeliverRptAckInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    pos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */

    smsDeliverRptAckInfo->paraInd = smsRawDataInfo->data[pos];
    pos++;

    /*
     * 解码到TP-Parameter-indication必选项已经解码完毕，如果ulPos已经与TPDU的总长度相等则直接返回，读越界问题在
     * MN_MSG_GetOrigUserData进行判断，读越界返回MN_ERR_CLASS_SMS_INVALID_TPDU
     */
    if (pos == smsRawDataInfo->len) {
        return MN_ERR_NO_ERROR;
    }

    /* TP PID */
    if ((smsDeliverRptAckInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsDeliverRptAckInfo->pid = smsRawDataInfo->data[pos];
        pos++;
    }
    /*  TP DCS */
    if ((smsDeliverRptAckInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &(smsDeliverRptAckInfo->dcs));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos++;
    } else {
        memset_s(&(smsDeliverRptAckInfo->dcs), sizeof(smsDeliverRptAckInfo->dcs), 0x00,
                 sizeof(smsDeliverRptAckInfo->dcs));
        smsDeliverRptAckInfo->dcs.msgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeDeliverRptAck: Invalid TPDU");
        return ret;
    }

    /* TP UD TP UDL; */
    if ((smsDeliverRptAckInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        ret = MSG_DecodeUserData(smsDeliverRptAckInfo->userDataHeaderInd, &(smsDeliverRptAckInfo->dcs), &origUserData,
                                 &(smsDeliverRptAckInfo->userData));
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeDeliverRptErr(const MN_MSG_RawTsData *smsRawDataInfo,
                                         MN_MSG_DeliverRptErr   *smsDeliverRptErrInfo)
{
    VOS_UINT32            pos = 0;
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    MN_MSG_DecodeUserData origUserData;

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsDeliverRptErrInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeDeliverRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsDeliverRptErrInfo, sizeof(MN_MSG_DeliverRptErr), 0x00, sizeof(MN_MSG_DeliverRptErr));

    if (smsRawDataInfo->len == 0) {
        MN_WARN_LOG("MSG_DecodeDeliverRptErr: no TPDU");
        return MN_ERR_NO_ERROR;
    }

    if (smsRawDataInfo->len < MN_MSG_DELIVER_RPTERR_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    /* TP MTI ignore  TP-UDHI  */
    MSG_GET_TP_UDHI(smsDeliverRptErrInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    pos++;
    /* TP FCS */
    smsDeliverRptErrInfo->failCause = smsRawDataInfo->data[pos];
    pos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsDeliverRptErrInfo->paraInd = smsRawDataInfo->data[pos];
    pos++;

    /*
     * 解码到TP-Parameter-indication必选项已经解码完毕，如果ulPos已经与TPDU的总长度相等则直接返回，读越界问题在
     * MN_MSG_GetOrigUserData进行判断，读越界返回MN_ERR_CLASS_SMS_INVALID_TPDU
     */
    if (pos == smsRawDataInfo->len) {
        return MN_ERR_NO_ERROR;
    }

    /* TP PID */
    if ((smsDeliverRptErrInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsDeliverRptErrInfo->pid = smsRawDataInfo->data[pos];
        pos++;
    }
    /*  TP DCS */
    if ((smsDeliverRptErrInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &smsDeliverRptErrInfo->dcs);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos++;
    } else {
        memset_s(&(smsDeliverRptErrInfo->dcs), sizeof(smsDeliverRptErrInfo->dcs), 0x00,
                 sizeof(smsDeliverRptErrInfo->dcs));
        smsDeliverRptErrInfo->dcs.msgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeDeliverRptErr: Invalid TPDU");
        return ret;
    }

    /* TP UD TP UDL; */
    if ((smsDeliverRptErrInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        ret = MSG_DecodeUserData(smsDeliverRptErrInfo->userDataHeaderInd, &(smsDeliverRptErrInfo->dcs), &origUserData,
                                 &(smsDeliverRptErrInfo->userData));
    }

    return ret;
}

VOS_UINT32 TAF_MSG_DecodeStaRptExceptUserData(const MN_MSG_RawTsData *smsRawDataInfo, VOS_UINT32 *pos,
                                              MN_MSG_StaRpt *smsStatusRptInfo)
{
    VOS_UINT32 raLen = 0;
    VOS_UINT32 sctsLen = 0;
    VOS_UINT32 dtLen = 0;
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    /* TP RA 2 12o */
    ret = MN_MSG_DecodeAddress(&(smsRawDataInfo->data[*pos]), VOS_FALSE, &(smsStatusRptInfo->recipientAddr), &raLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += raLen;

    /* TP SCTS 7o Parameter identifying time when the SC received the previously sent SMS SUBMIT */
    ret = MSG_DecodeTimeStamp((VOS_UINT8 *)&(smsRawDataInfo->data[*pos]), &(smsStatusRptInfo->timeStamp), &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += sctsLen;

    /* TP DT Parameter identifying the time associated with a particular TP ST outcome */
    ret = MSG_DecodeTimeStamp((VOS_UINT8 *)&(smsRawDataInfo->data[*pos]), &(smsStatusRptInfo->dischargeTime), &dtLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += dtLen;

    /* TP ST */
    smsStatusRptInfo->status = smsRawDataInfo->data[*pos];
    (*pos)++;
    /*
     * 解码到TP-Status必选项已经解码完毕，如果ulPos已经与TPDU的总长度相等则直接返回，越界问题在
     * MN_MSG_GetOrigUserData进行判断，并且读越界后需要返回MN_ERR_CLASS_SMS_INVALID_TPDU
     */
    if ((*pos) == smsRawDataInfo->len) {
        return MN_ERR_NO_ERROR;
    }

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsStatusRptInfo->paraInd = smsRawDataInfo->data[*pos];
    (*pos)++;
    /* TP PID */
    if ((smsStatusRptInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsStatusRptInfo->pid = smsRawDataInfo->data[*pos];
        (*pos)++;
    }
    /*  TP DCS */
    if ((smsStatusRptInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[*pos], &smsStatusRptInfo->dcs);
        if (ret != MN_ERR_NO_ERROR) {
            MN_WARN_LOG("TAF_MSG_DecodeStaRptExceptUserData: invalid DCS.");
            return MN_ERR_NO_ERROR;
        }
        (*pos)++;
    } else {
        memset_s(&(smsStatusRptInfo->dcs), sizeof(smsStatusRptInfo->dcs), 0x00, sizeof(smsStatusRptInfo->dcs));
        smsStatusRptInfo->dcs.msgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeStaRpt(const MN_MSG_RawTsData *smsRawDataInfo, MN_MSG_StaRpt *smsStatusRptInfo)
{
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    VOS_UINT32            pos = 0;
    MN_MSG_DecodeUserData origUserData;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsStatusRptInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeStaRpt: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (smsRawDataInfo->len < MN_MSG_STATUS_REPORT_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    (VOS_VOID)memset_s(smsStatusRptInfo, sizeof(MN_MSG_StaRpt), 0x00, sizeof(MN_MSG_StaRpt));

    /* TP MTI ignore TP UDHI TP MMS TP SRQ */
    MSG_GET_TP_UDHI(smsStatusRptInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    MSG_GET_TP_MMS(smsStatusRptInfo->moreMsg, smsRawDataInfo->data[pos]);
    MSG_GET_TP_SRQ(smsStatusRptInfo->staRptQualCommand, smsRawDataInfo->data[pos]);
    pos++;

    /* TP MR */
    smsStatusRptInfo->mr = smsRawDataInfo->data[pos];
    pos++;

    ret = TAF_MSG_DecodeStaRptExceptUserData(smsRawDataInfo, &pos, smsStatusRptInfo);

    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    (VOS_VOID)memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeDeliver: Invalid TPDU");
        return ret;
    }

    origUserData.isSmsStaRptType = VOS_TRUE;

    /* TP UD TP UDL; */
    if ((smsStatusRptInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        ret = MSG_DecodeUserData(smsStatusRptInfo->userDataHeaderInd, &(smsStatusRptInfo->dcs), &origUserData,
                                 &(smsStatusRptInfo->userData));
        if (ret != MN_ERR_NO_ERROR) {
            MN_WARN_LOG("MSG_DecodeStaRpt: invalid UDL or UD.");
            return MN_ERR_NO_ERROR;
        }
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeCommand(const MN_MSG_RawTsData *smsRawDataInfo, MN_MSG_CommandParam *smsCommandInfo)
{
    errno_t    memResult;
    VOS_UINT32 ret = MN_ERR_NO_ERROR;
    VOS_UINT32 pos = 0;
    VOS_UINT32 addrLen;
    VOS_UINT8  maxDataLen = 0;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsCommandInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeCommand: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (smsRawDataInfo->len < MN_MSG_COMMAND_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    memset_s(smsCommandInfo, sizeof(MN_MSG_CommandParam), 0x00, sizeof(MN_MSG_CommandParam));

    /* TP MTI TP UDHI TP SRR */
    MSG_GET_TP_UDHI(smsCommandInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    MSG_GET_TP_SRR(smsCommandInfo->staRptReq, smsRawDataInfo->data[pos]);
    pos++;

    /* TP MR */
    smsCommandInfo->mr = smsRawDataInfo->data[pos];
    pos++;

    /* TP PID */
    smsCommandInfo->pid = smsRawDataInfo->data[pos];
    pos++;

    /* TP CT */
    smsCommandInfo->cmdType = smsRawDataInfo->data[pos];
    pos++;

    /* TP MN */
    smsCommandInfo->msgNumber = smsRawDataInfo->data[pos];
    pos++;

    /* TP DA */
    ret = MN_MSG_DecodeAddress(&(smsRawDataInfo->data[pos]), VOS_FALSE, &(smsCommandInfo->destAddr), &addrLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += addrLen;

    /* TP CDL */
    smsCommandInfo->commandDataLen = smsRawDataInfo->data[pos];
    pos++;
    maxDataLen = (VOS_UINT8)TAF_MIN((MN_MSG_MAX_COMMAND_TPDU_MSG_LEN - pos), MN_MSG_MAX_COMMAND_DATA_LEN);

    /* TP CD */
    if (smsCommandInfo->commandDataLen == 0) {
        return MN_ERR_NO_ERROR;
    }

    if (smsCommandInfo->commandDataLen > maxDataLen) {
        smsCommandInfo->commandDataLen = maxDataLen;
    }
    if (smsCommandInfo->commandDataLen > 0) {
        memResult = memcpy_s(smsCommandInfo->cmdData, sizeof(smsCommandInfo->cmdData), &smsRawDataInfo->data[pos],
                             smsCommandInfo->commandDataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCommandInfo->cmdData), smsCommandInfo->commandDataLen);
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeSubmit(const MN_MSG_RawTsData *smsRawDataInfo, MN_MSG_Submit *smsSubmitInfo)
{
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    VOS_UINT32            pos = 0;
    VOS_UINT32            daLen;
    VOS_UINT32            vpLen;
    MN_MSG_DecodeUserData origUserData;
    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeSubmit: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (smsRawDataInfo->len < MN_MSG_SUBMIT_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    memset_s(smsSubmitInfo, sizeof(MN_MSG_Submit), 0x00, sizeof(MN_MSG_Submit));
    /*
     * b7  b6   b5   b4   b3  b2  b1  b0
     * RP  UDHI SRR  VPF      RD  MTI
     * 0   0    0    2        1   2
     * TP MTI TP RD TP VPF TP RP TP UDHI TP SRR
     */
    /* TP MTI ignore 23040 9.2.3.1 填写bit0bit1:MIT     0011 0001 */

    /* TP RD  23040 9.2.3.25 */
    MSG_GET_TP_RD(smsSubmitInfo->rejectDuplicates, smsRawDataInfo->data[pos]);
    /* TP VPF 23040 9.2.3.3 */
    MSG_GET_TP_VPF(smsSubmitInfo->validPeriod.validPeriod, smsRawDataInfo->data[pos]);
    /* TP RP  23040 9.2.3.17 */
    MSG_GET_TP_RP(smsSubmitInfo->replayPath, smsRawDataInfo->data[pos]);
    /* TP UDHI23040 9.2.3.23 */
    MSG_GET_TP_UDHI(smsSubmitInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    /* TP SRR 23040 9.2.3.5 */
    MSG_GET_TP_SRR(smsSubmitInfo->staRptReq, smsRawDataInfo->data[pos]);
    pos++;

    /* TP MR */
    smsSubmitInfo->mr = smsRawDataInfo->data[pos];
    pos++;

    /* TP DA* Refer to 9.1.2.5 */
    ret = MN_MSG_DecodeAddress(&(smsRawDataInfo->data[pos]), VOS_FALSE, &(smsSubmitInfo->destAddr), &daLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += daLen;

    /* TP PID */
    smsSubmitInfo->pid = smsRawDataInfo->data[pos];
    pos++;

    /* TP DCS 23038 4 */
    ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &(smsSubmitInfo->dcs));
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos++;

    /* TP VP */
    ret = MN_MSG_DecodeValidPeriod(smsSubmitInfo->validPeriod.validPeriod, &(smsRawDataInfo->data[pos]),
                                   &(smsSubmitInfo->validPeriod), &vpLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += vpLen;

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeSubmit: Invalid TPDU");
        return ret;
    }
    /* UDL UD */
    ret = MSG_DecodeUserData(smsSubmitInfo->userDataHeaderInd, &(smsSubmitInfo->dcs), &origUserData,
                             &(smsSubmitInfo->userData));

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeSubmitRptAck(const MN_MSG_RawTsData *smsRawDataInfo,
                                        MN_MSG_SubmitRptAck    *smsSubmitRptAckInfo)
{
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    VOS_UINT32            pos = 0;
    VOS_UINT32            sctsLen;
    MN_MSG_DecodeUserData origUserData;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitRptAckInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeSubmitRptAck: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsSubmitRptAckInfo, sizeof(MN_MSG_SubmitRptAck), 0x00, sizeof(MN_MSG_SubmitRptAck));

    if (smsRawDataInfo->len == 0) {
        MN_WARN_LOG("MSG_DecodeSubmitRptAck: no TPDU");
        return MN_ERR_NO_ERROR;
    }

    if (smsRawDataInfo->len < MN_MSG_SUBMIT_RPTACK_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    /* TP MTI ignore TP-UDHI  */
    MSG_GET_TP_UDHI(smsSubmitRptAckInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    pos++;

    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsSubmitRptAckInfo->paraInd = smsRawDataInfo->data[pos];
    pos++;
    /* TP SCTS */
    ret = MSG_DecodeTimeStamp((VOS_UINT8 *)&(smsRawDataInfo->data[pos]), &(smsSubmitRptAckInfo->timeStamp), &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += sctsLen;

    /*
     * 解码到TP-Service-Centre-Time-Stamp必选项已经解码完毕，如果ulPos已经与TPDU的总长度相等则直接返回，读越界问题在
     * MN_MSG_GetOrigUserData进行判断，读越界返回MN_ERR_CLASS_SMS_INVALID_TPDU
     */
    if (pos == smsRawDataInfo->len) {
        return MN_ERR_NO_ERROR;
    }

    /* TP PID */
    if ((smsSubmitRptAckInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsSubmitRptAckInfo->pid = smsRawDataInfo->data[pos];
        pos++;
    }

    /*  TP DCS */
    if ((smsSubmitRptAckInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &(smsSubmitRptAckInfo->dcs));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos++;
    } else {
        memset_s(&(smsSubmitRptAckInfo->dcs), sizeof(smsSubmitRptAckInfo->dcs), 0x00, sizeof(smsSubmitRptAckInfo->dcs));
        smsSubmitRptAckInfo->dcs.msgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeSubmitRptAck: Invalid TPDU");
        return ret;
    }

    /* TP UD TP UDL; */
    if ((smsSubmitRptAckInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        ret = MSG_DecodeUserData(smsSubmitRptAckInfo->userDataHeaderInd, &(smsSubmitRptAckInfo->dcs), &origUserData,
                                 &(smsSubmitRptAckInfo->userData));
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_DecodeSubmitRptErr(const MN_MSG_RawTsData *smsRawDataInfo,
                                        MN_MSG_SubmitRptErr    *smsSubmitRptErrInfo)
{
    VOS_UINT32            ret = MN_ERR_NO_ERROR;
    VOS_UINT32            pos = 0;
    VOS_UINT32            sctsLen;
    MN_MSG_DecodeUserData origUserData;

    if ((smsRawDataInfo == VOS_NULL_PTR) || (smsSubmitRptErrInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MSG_DecodeSubmitRptErr: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    memset_s(smsSubmitRptErrInfo, sizeof(MN_MSG_SubmitRptErr), 0x00, sizeof(MN_MSG_SubmitRptErr));

    if (smsRawDataInfo->len == 0) {
        MN_WARN_LOG("MSG_DecodeSubmitRptErr: no TPDU");
        return MN_ERR_NO_ERROR;
    }

    if (smsRawDataInfo->len < MN_MSG_SUBMIT_RPTERR_TPDU_MANDATORY_IE_LEN) {
        MN_ERR_LOG("MSG_DecodeDeliver: Length of the SMS Raw Data is lack.");
        return MN_ERR_NOMEM;
    }

    /* TP MTI ignore TP-UDHI  */
    MSG_GET_TP_UDHI(smsSubmitRptErrInfo->userDataHeaderInd, smsRawDataInfo->data[pos]);
    pos++;

    /* TP FCS */
    smsSubmitRptErrInfo->failCause = smsRawDataInfo->data[pos];
    pos++;
    /*    TP PI  9.2.3.27 BIT   2       1       0             */
    /*                          TP UDL  TP DCS  TP PID        */
    smsSubmitRptErrInfo->paraInd = smsRawDataInfo->data[pos];
    pos++;
    /* TP SCTS */
    ret = MSG_DecodeTimeStamp((VOS_UINT8 *)&(smsRawDataInfo->data[pos]), &(smsSubmitRptErrInfo->timeStamp), &sctsLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    pos += sctsLen;

    /*
     * 解码到TP-Service-Centre-Time-Stamp必选项已经解码完毕，如果ulPos已经与TPDU的总长度相等则直接返回，读越界问题在
     * MN_MSG_GetOrigUserData进行判断，读越界返回MN_ERR_CLASS_SMS_INVALID_TPDU
     */
    if (pos == smsRawDataInfo->len) {
        return MN_ERR_NO_ERROR;
    }

    /* TP PID */
    if ((smsSubmitRptErrInfo->paraInd & MN_MSG_TP_PID_MASK) != 0) {
        smsSubmitRptErrInfo->pid = smsRawDataInfo->data[pos];
        pos++;
    }

    /*  TP DCS */
    if ((smsSubmitRptErrInfo->paraInd & MN_MSG_TP_DCS_MASK) != 0) {
        ret = MN_MSG_DecodeDcs(smsRawDataInfo->data[pos], &(smsSubmitRptErrInfo->dcs));
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }
        pos++;
    } else {
        memset_s(&(smsSubmitRptErrInfo->dcs), sizeof(smsSubmitRptErrInfo->dcs), 0x00, sizeof(smsSubmitRptErrInfo->dcs));
        smsSubmitRptErrInfo->dcs.msgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    memset_s(&origUserData, sizeof(MN_MSG_DecodeUserData), 0x00, sizeof(MN_MSG_DecodeUserData));
    ret = MN_MSG_GetOrigUserData(smsRawDataInfo, pos, &origUserData);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MSG_DecodeSubmitRptErr: Invalid TPDU");
        return ret;
    }

    /* TP UD TP UDL; */
    if ((smsSubmitRptErrInfo->paraInd & MN_MSG_TP_UDL_MASK) != 0) {
        ret = MSG_DecodeUserData(smsSubmitRptErrInfo->userDataHeaderInd, &(smsSubmitRptErrInfo->dcs), &origUserData,
                                 &(smsSubmitRptErrInfo->userData));
    }

    return ret;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_DecodeDcs(VOS_UINT8 dcsData, MN_MSG_DcsCode *dcs)
{
    VOS_UINT8 codingGroup;

    /* 判断输入参数的合法性 */
    if (dcs == VOS_NULL_PTR) {
        MN_ERR_LOG("MN_MSG_DecodeDcs: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /* 给pstDcsCode初始化为一个默认值 */
    dcs->compressed        = VOS_FALSE;
    dcs->waitingIndiActive = VOS_FALSE;
    dcs->msgClass          = MN_MSG_MSG_CLASS_NONE;
    dcs->msgCoding         = MN_MSG_MSG_CODING_7_BIT;
    dcs->msgWaiting        = MN_MSG_MSG_WAITING_NONE;
    dcs->msgWaitingKind    = MN_MSG_MSG_WAITING_OTHER;
    dcs->rawDcsData        = dcsData;
    dcs->rawDcsValid       = VOS_TRUE;
    /* ucDcsData 的bit 6 bit 7两位 */
    codingGroup = (dcsData >> 6) & 0x03;

    /*
     * LGU+网络下短信使用的DCS类型为0x84，按照协议属于Reserved coding group，需要特殊处理
     * 以保证短信的正常收发，按照协议解码DCS后，当前使用的character set为8 bit，没有所用
     * 压缩模式，且没有指定message class
     */
    if (dcsData == MN_MSG_DCS_DATA_LGU_VALUE) {
        dcs->msgCoding = MN_MSG_MSG_CODING_8_BIT;

        return MN_ERR_NO_ERROR;
    }

    switch (codingGroup) {
        /* (pattern 00xx xxxx) */
        case 1:
            /* Message Marked for Automatic Deletion Group */
            dcs->msgWaiting = MN_MSG_MSG_WAITING_AUTO_DELETE;
            /* fall through */
        case 0:

            /*lint -e961*/
            /* Bit 5  if set to 0, indicates the text is uncompressed */
            MSG_GET_COMPRESSED(dcs->compressed, dcsData);
            /*lint +e961*/

            /* Bit 3 Bit2 Character set: */
            MSG_GET_CHARSET(dcs->msgCoding, dcsData);
            /* Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class */
            if ((dcsData & 0x10) != 0) { /* (bit 4) */
                MSG_GET_MSGCLASS(dcs->msgClass, dcsData);
            }
            /* 数据损失引入点 */
            break;
        case 3:
            if ((dcsData & 0x30) == 0x30) { /* (pattern 1111 xxxx) */
                dcs->msgWaiting = MN_MSG_MSG_WAITING_NONE_1111;
                MSG_GET_MSGCODING(dcs->msgCoding, dcsData);
                MSG_GET_MSGCLASS(dcs->msgClass, dcsData);
                /* bit3 默认为0，数据损失引入点 */
            } else {
                /* bit2 默认为0，数据损失引入点 */
                /*lint -e961*/
                MSG_GET_INDSENSE(dcs->waitingIndiActive, dcsData);
                MSG_GET_INDTYPE(dcs->msgWaitingKind, dcsData);
                /*lint +e961*/

                if ((dcsData & 0x30) == 0x00) { /* (pattern 1100 xxxx) */
                    dcs->msgWaiting = MN_MSG_MSG_WAITING_DISCARD;
                } else {
                    dcs->msgWaiting = MN_MSG_MSG_WAITING_STORE;
                    if ((dcsData & 0x30) == 0x20) {
                        dcs->msgCoding = MN_MSG_MSG_CODING_UCS2;
                    }
                }
            }
            break;
        default:
            
            MN_ERR_LOG("MN_MSG_DecodeDcs: ucCodingGroup is two.");
            break;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_DecodeRelatTime(VOS_UINT8 relatTimeData, MN_MSG_Timestamp *relatTime)
{
    VOS_UINT32 munite;
    VOS_UINT32 day;

    if (relatTime == VOS_NULL_PTR) {
        MN_ERR_LOG("MN_MSG_DecodeRelatTime: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    } else {
        memset_s(relatTime, sizeof(MN_MSG_Timestamp), 0x00, sizeof(MN_MSG_Timestamp));
    }

    /*
     * 0～143：（VP+1）* 5 分钟；可表示5分钟到12小时的时间段
     * 144～167：12 小时+（（VP–143）*30 分钟）；12 -> 23.5小时
     * 168～196：（VP–166）*1 日；最多表示(196 - 166)== 30天
     * 197～255：（VP–192）*1 周。可表示(255-192)*7 == 441天
     */
    if (relatTimeData < 144) {
        munite            = (relatTimeData + 1) * 5;
        relatTime->hour   = (VOS_UINT8)(munite / MN_MSG_MINUTES_IN_AN_HOUR);
        relatTime->minute = munite % MN_MSG_MINUTES_IN_AN_HOUR;
    } else if (relatTimeData < 168) {
        munite            = (12 * MN_MSG_MINUTES_IN_AN_HOUR) + ((relatTimeData - 143) * 30);
        relatTime->hour   = (VOS_UINT8)(munite / MN_MSG_MINUTES_IN_AN_HOUR);
        relatTime->minute = munite % MN_MSG_MINUTES_IN_AN_HOUR;
    } else if (relatTimeData < 197) {
        relatTime->day = relatTimeData - 166;
    } else {
        day              = (relatTimeData - 192) * 7;
        relatTime->year  = (VOS_UINT8)(day / MN_MSG_DAYS_IN_A_YEAR);
        relatTime->day   = (VOS_UINT8)(day % MN_MSG_DAYS_IN_A_YEAR);
        relatTime->month = relatTime->day / 30;
        relatTime->day %= 30;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_Decode(const MN_MSG_RawTsData *rawData, MN_MSG_TsDataInfo *tsDataInfo)
{
    VOS_UINT32 ret;

    if ((rawData == VOS_NULL_PTR) || (tsDataInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_Decode: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    } else {
        memset_s(tsDataInfo, sizeof(MN_MSG_TsDataInfo), 0x00, sizeof(MN_MSG_TsDataInfo));
    }

    if (rawData->len > MSG_MAX_TPDU_MSG_LEN) {
        MN_WARN_LOG("MN_MSG_Decode: Parameter of the function is null.");
        return MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW;
    }

    tsDataInfo->tpduType = rawData->tpduType;

    MN_INFO_LOG1("MN_MSG_Decode: Tpdu type is ", (VOS_INT32)rawData->tpduType);
    switch (rawData->tpduType) {
        case MN_MSG_TPDU_DELIVER:
            ret = MSG_DecodeDeliver(rawData, &(tsDataInfo->u.deliver));
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            ret = MSG_DecodeDeliverRptAck(rawData, &(tsDataInfo->u.deliverRptAck));
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            ret = MSG_DecodeDeliverRptErr(rawData, &(tsDataInfo->u.deliverRptErr));
            break;

        case MN_MSG_TPDU_STARPT:
            ret = MSG_DecodeStaRpt(rawData, &(tsDataInfo->u.staRpt));
            break;

        case MN_MSG_TPDU_COMMAND:
            ret = MSG_DecodeCommand(rawData, &(tsDataInfo->u.command));
            break;

        case MN_MSG_TPDU_SUBMIT:
            ret = MSG_DecodeSubmit(rawData, &(tsDataInfo->u.submit));
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            ret = MSG_DecodeSubmitRptAck(rawData, &(tsDataInfo->u.submitRptAck));
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            ret = MSG_DecodeSubmitRptErr(rawData, &(tsDataInfo->u.submitRptErr));
            break;

        default:
            MN_WARN_LOG("MN_MSG_Decode: invalid pdu type.");
            ret = MN_ERR_CLASS_SMS_INVALID_TPDUTYPE;
            break;
    }

    MN_INFO_LOG1("MN_MSG_Decode: result is ", (VOS_INT32)ret);
    return ret;
}

MN_MSG_UserHeaderType* MSG_GetSpecIdUdhIe(VOS_UINT8              numofHeaders, /* number of user header */
                                          MN_MSG_UserHeaderType *userDataHeader, MN_MSG_UdhTypeUint8 headerID)
{
    VOS_BOOL   bIeExist = VOS_FALSE;
    VOS_UINT32 loop;

    if (userDataHeader == VOS_NULL_PTR) {
        MN_ERR_LOG("MSG_GetSpecIdUdhIe: Parameter of the function is null.");
        return VOS_NULL_PTR;
    }

    for (loop = 0; loop < numofHeaders; loop++) {
        if (userDataHeader->headerID == headerID) {
            bIeExist = VOS_TRUE;
            break;
        }
        userDataHeader++;
    }

    if (bIeExist == VOS_TRUE) {
        MN_INFO_LOG("MSG_ConcatenateUdh: ie is exist in long message struct.");
        return userDataHeader;
    } else {
        return VOS_NULL_PTR;
    }
}

VOS_UINT32 MSG_FillMsgUdhEo(const MN_MSG_Deliver *smsDeliverInfo, const MN_MSG_UserHeaderType *userDataHeader,
                            MN_MSG_DeliverLong *longDeliver)
{
    /* detail of user header */
    MN_MSG_UserHeaderType *longUserDataHeader = VOS_NULL_PTR;
    errno_t                memResult;
    VOS_UINT16             currentLen;

    if (userDataHeader->u.eo.firstSegment == VOS_FALSE) {
        longUserDataHeader = MSG_GetSpecIdUdhIe(longDeliver->longUserData.numofHeaders,
                                                longDeliver->longUserData.userDataHeader, MN_MSG_UDH_EO);
        if (longUserDataHeader == VOS_NULL_PTR) {
            MN_ERR_LOG("MSG_FillMsgUdhEo: The subsequent Extended Object IEs without first Extended Object IE");
            return MN_ERR_INVALIDPARM;
        }

        currentLen = longUserDataHeader->u.eo.dataLen;
        if ((currentLen + userDataHeader->u.eo.dataLen) > MN_MSG_MAX_UDH_LONG_EO_DATA_LEN) {
            MN_WARN_LOG("MSG_FillMsgUdhEo: Extended Object IE length is overflow, Check to get a right length for EO.");
            return MN_ERR_INVALIDPARM;
        }
        if (userDataHeader->u.eo.dataLen > 0) {
            memResult = memcpy_s(&longUserDataHeader->u.eo.data[currentLen],
                                 sizeof(VOS_UINT8) * (MN_MSG_MAX_UDH_LONG_EO_DATA_LEN - currentLen),
                                 userDataHeader->u.eo.data, userDataHeader->u.eo.dataLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(VOS_UINT8) * (MN_MSG_MAX_UDH_LONG_EO_DATA_LEN - currentLen),
                                userDataHeader->u.eo.dataLen);
        }
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MSG_FillMsgUdhCc(const MN_MSG_Deliver *smsDeliverInfo, const MN_MSG_UserHeaderType *userDataHeader,
                            MN_MSG_DeliverLong *longDeliver)
{
    /* detail of user header */
    MN_MSG_UserHeaderType *longUserDataHeader = VOS_NULL_PTR;
    errno_t                memResult;
    VOS_UINT16             currentLen;

    longUserDataHeader = MSG_GetSpecIdUdhIe(longDeliver->longUserData.numofHeaders,
                                            longDeliver->longUserData.userDataHeader, MN_MSG_UDH_CC);
    if (longUserDataHeader == VOS_NULL_PTR) {
        MN_ERR_LOG("MSG_FillMsgUdhCc: invalid Extended Object IE ");
        return MN_ERR_INVALIDPARM;
    }

    currentLen = longUserDataHeader->u.eo.dataLen;
    if ((currentLen + userDataHeader->u.cc.dataLen) > MN_MSG_UDH_MAX_COMPRESSION_DATA_LEN) {
        MN_WARN_LOG("MSG_FillMsgUdhCc: Extended Object IE length is overflow, Check to get a right length for CC.");
        return MN_ERR_INVALIDPARM;
    }

    if (userDataHeader->u.cc.dataLen > 0) {
        memResult = memcpy_s(&longUserDataHeader->u.cc.data[currentLen],
                             sizeof(VOS_UINT8) * (MN_MSG_UDH_MAX_COMPRESSION_DATA_LEN - currentLen),
                             userDataHeader->u.cc.data, userDataHeader->u.cc.dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(VOS_UINT8) * (MN_MSG_UDH_MAX_COMPRESSION_DATA_LEN - currentLen),
                            userDataHeader->u.cc.dataLen);
    }

    return MN_ERR_NO_ERROR;
}

/*
 * MSG_FillLongMsgUdh
 * MSG_FillLongMsg
 */
/* MSG_FillLongMsg MSG_FillLongMsgUdh */

VOS_UINT32 MN_MSG_ParseConcatenateMsg(VOS_UINT8 num, const MN_MSG_RawTsData *rawData,
                                      MN_MSG_CONCENTRATE_MSG_ATTR *concentrateMsgAttr, MN_MSG_Deliver *smsDeliverInfo,
                                      VOS_UINT8 *queueMsg)
{
    VOS_UINT32             loop;
    VOS_UINT32             ret;
    MN_MSG_UserHeaderType *userHeader = VOS_NULL_PTR;
    VOS_UINT8              totalNum   = 0;
    VOS_UINT8              seqNum;

    if (concentrateMsgAttr->ucFirstMsgSeqNum < 1) {
        MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: invalid SeqNum. ");
        return MN_ERR_INVALIDPARM;
    }

    queueMsg[concentrateMsgAttr->ucFirstMsgSeqNum - 1] = 1;
    /* 为连续短消息排序 */
    for (loop = 1; loop < num; loop++) {
        ret = MSG_DecodeDeliver(rawData, smsDeliverInfo);
        if (ret != MN_ERR_NO_ERROR) {
            return ret;
        }

        if (concentrateMsgAttr->b8bitConcatMsg == VOS_TRUE) {
            userHeader = MSG_GetSpecIdUdhIe(smsDeliverInfo->userData.numofHeaders,
                                            smsDeliverInfo->userData.userDataHeader, MN_MSG_UDH_CONCAT_8);
            if (userHeader == VOS_NULL_PTR) {
                MN_ERR_LOG("MN_MSG_ParseConcatenateMsg: concatenate message without right concatenate IE. ");
                return MN_ERR_INVALIDPARM;
            }

            if (concentrateMsgAttr->ucMr != userHeader->u.concat8.mr) {
                MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: the reference number is not constant. ");
                return MN_ERR_INVALIDPARM;
            }
            seqNum   = userHeader->u.concat8.seqNum;
            totalNum = userHeader->u.concat8.totalNum;
        } else {
            userHeader = MSG_GetSpecIdUdhIe(smsDeliverInfo->userData.numofHeaders,
                                            smsDeliverInfo->userData.userDataHeader, MN_MSG_UDH_CONCAT_16);
            if (userHeader == VOS_NULL_PTR) {
                MN_ERR_LOG("MN_MSG_ParseConcatenateMsg: concatenate message without right concatenate IE. ");
                return MN_ERR_INVALIDPARM;
            }

            if (concentrateMsgAttr->usMr != userHeader->u.concat16.mr) {
                MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: the reference number is not constant. ");
                return MN_ERR_INVALIDPARM;
            }
            seqNum   = userHeader->u.concat16.seqNum;
            totalNum = userHeader->u.concat16.totalNum;
        }

        if (seqNum > totalNum) {
            MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: Sequence number is bigger than Maximum number.");
            return MN_ERR_INVALIDPARM;
        }

        if (seqNum == 0) {
            MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: Sequence number is zero.");
            return MN_ERR_INVALIDPARM;
        }

        /* 消息重复检查 */
        if (queueMsg[seqNum - 1] != 0) {
            MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: Repeated message.");
            return MN_ERR_INVALIDPARM;
        }
        queueMsg[seqNum - 1] = (VOS_UINT8)(loop + 1);
        rawData++;
    }

    /* 消息间断检查 */
    for (loop = 0; loop < num; loop++) {
        if (queueMsg[loop] == 0) {
            MN_WARN_LOG("MN_MSG_ParseConcatenateMsg: incontinuous message.");
            return MN_ERR_INVALIDPARM;
        }
    }

    return MN_ERR_NO_ERROR;
}

VOS_VOID MN_MSG_GetAddressFromSubmit(const VOS_UINT8 *rpduContent, VOS_UINT32 rpDataLen,
                                     NAS_MNTN_ASCII_ADDR_STRU *scAddr, NAS_MNTN_ASCII_ADDR_STRU *destAddr)
{
    VOS_UINT8  scLen;
    VOS_UINT8  pos = 0;
    VOS_UINT8  mti;
    VOS_UINT32 len;
    VOS_UINT32 ret;

    /* 第一个字节是RP层消息类型； */
    pos++;

    /* 第二个字节是RP-MR */
    pos++;

    /* 从第三个字节开始是RP-OA */
    pos++;

    /* 从第四个字节开始是是RP-DA，即短信中心 */
    scLen = rpduContent[pos];

    ret = MN_MSG_DecodeAddress(&rpduContent[pos], VOS_TRUE, (MN_MSG_AsciiAddr *)scAddr, &len);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("NAS_MNTN_GetAddressFromSubmit:Error SC.");
        return;
    }
    /* 偏移到短信中心后的数据区: BCD码长度加一位长度位 */
    pos += (scLen + 1);

    /* RP-DA后一个字节RP-DATA长度 */
    pos++;

    /* 消息类型过滤：消息不是SUBMIT消息直接退出 */
    MSG_GET_TP_MTI(mti, rpduContent[pos]);
    if (mti != MN_MSG_TP_MTI_SUBMIT) {
        MN_WARN_LOG("NAS_MNTN_GetAddressFromSubmit:Error MsgName");
        return;
    }
    pos++;

    /* 移动到TP-MR后 */
    pos++;

    /* 获取到目的号码的首地址后解析得到目的号码 */
    ret = MN_MSG_DecodeAddress(&rpduContent[pos], VOS_FALSE, (MN_MSG_AsciiAddr *)destAddr, &len);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("NAS_MNTN_GetAddressFromSubmit:Error DA.");
        return;
    }
}

MODULE_EXPORTED VOS_VOID MN_MSG_GetMsgMti(VOS_UINT8 fo, VOS_UINT8 *mti)
{
    MSG_GET_TP_MTI(*mti, fo);
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_GetSubmitUserDataInfoForStk(const MN_MSG_Submit    *smsSubmitInfo,
                                                              const MN_MSG_RawTsData *tsRawData, VOS_UINT8 *userDataLen,
                                                              VOS_UINT8 *udhl, VOS_UINT32 pos)
{
    VOS_UINT32 ret;
    VOS_UINT8  udl;
    VOS_UINT8  udhlTemp;

    udl = tsRawData->data[pos];
    pos++;

    ret = MN_MSG_CheckUdl(&(smsSubmitInfo->dcs), udl, (tsRawData->len - pos), VOS_TRUE);
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG("MN_MSG_GetSubmitUserDataInfoForStk: MN_MSG_CheckUdl fail.");
        return ret;
    }

    if (smsSubmitInfo->userDataHeaderInd == VOS_TRUE) {
        udhlTemp = tsRawData->data[pos];
        ret      = MN_MSG_CheckUdhl(&(smsSubmitInfo->dcs), udl, udhlTemp);
        if (ret != MN_ERR_NO_ERROR) {
            MN_WARN_LOG("MN_MSG_GetSubmitUserDataInfoForStk: MN_MSG_CheckUdhl fail.");
            return ret;
        }

        if (smsSubmitInfo->dcs.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
            *userDataLen = (VOS_UINT8)(udl - ((((udhlTemp + 1) * 8) + 6) / 7));
        } else {
            *userDataLen = (VOS_UINT8)(udl - (udhlTemp + 1));
        }

        *udhl = udhlTemp;
    } else {
        *userDataLen = udl;
        *udhl        = 0;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 TAF_MSG_DecodeSubmitInfoExceptUserDataForStk(MN_MSG_Submit *smsSubmitInfo, VOS_UINT32 *pos,
                                                        const MN_MSG_RawTsData *tsRawData, VOS_UINT32 *dcsOffset)
{
    VOS_UINT32 daLen = 0;
    VOS_UINT32 vpLen = 0;
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    /* TP DA Refer to 9.1.2.5 */

    ret = MN_MSG_DecodeAddress(&(tsRawData->data[*pos]), VOS_FALSE, &(smsSubmitInfo->destAddr), &daLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += daLen;

    /* TP PID */
    smsSubmitInfo->pid = tsRawData->data[*pos];
    (*pos)++;
    *dcsOffset = *pos;

    /* TP DCS 23038 4 */
    ret = MN_MSG_DecodeDcs(tsRawData->data[*pos], &(smsSubmitInfo->dcs));
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos)++;

    /* TP VP */
    ret = MN_MSG_DecodeValidPeriod(smsSubmitInfo->validPeriod.validPeriod, &(tsRawData->data[*pos]),
                                   &(smsSubmitInfo->validPeriod), &vpLen);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }
    (*pos) += vpLen;

    return ret;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_GetSubmitInfoForStk(MN_MSG_Tpdu *tpdu, VOS_UINT8 *userDataLen, VOS_UINT8 *udhl,
                                                      MN_MSG_MsgCodingUint8 *msgCoding, VOS_UINT32 *dcsOffset)
{
    errno_t           memResult;
    VOS_UINT32        ret;
    VOS_UINT32        pos;
    MN_MSG_Submit    *smsSubmitInfo = VOS_NULL_PTR;
    MN_MSG_RawTsData *tsRawData     = VOS_NULL_PTR;

    if ((tpdu == VOS_NULL_PTR) || (tpdu->tpdu == VOS_NULL_PTR) || (userDataLen == VOS_NULL_PTR) ||
        (udhl == VOS_NULL_PTR) || (msgCoding == VOS_NULL_PTR)) {
        MN_WARN_LOG("MN_MSG_GetSubmitInfoForStk: NULL PTR. ");
        return MN_ERR_NULLPTR;
    }

    MN_NORM_LOG1("MN_MSG_GetSubmitInfoForStk: pstTpdu->len is ", (VOS_INT32)tpdu->len);

    smsSubmitInfo = (MN_MSG_Submit *)MN_MSG_AllocForLib(WUEPS_PID_TAF, sizeof(MN_MSG_Submit));
    if (smsSubmitInfo == VOS_NULL_PTR) {
        MN_WARN_LOG("MN_MSG_GetSubmitInfoForStk: fail to alloc memory for pstSmsSubmitInfo. ");
        return MN_ERR_NOMEM;
    }

    tsRawData = (MN_MSG_RawTsData *)MN_MSG_AllocForLib(WUEPS_PID_TAF, sizeof(MN_MSG_RawTsData));
    if (tsRawData == VOS_NULL_PTR) {
        MN_WARN_LOG("MN_MSG_GetSubmitInfoForStk: fail to alloc memory for pstTsRawData. ");
        MN_MSG_FreeForLib(WUEPS_PID_TAF, smsSubmitInfo);
        return MN_ERR_NOMEM;
    }

    memset_s(tsRawData, sizeof(MN_MSG_RawTsData), 0x00, sizeof(MN_MSG_RawTsData));

    tsRawData->tpduType = MN_MSG_TPDU_SUBMIT;

    tsRawData->len = TAF_MIN(tpdu->len, MN_MSG_MAX_LEN);

    if (tsRawData->len > 0) {
        memResult = memcpy_s(tsRawData->data, sizeof(tsRawData->data), tpdu->tpdu, tsRawData->len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tsRawData->data), tsRawData->len);
    }

    memset_s(smsSubmitInfo, sizeof(MN_MSG_Submit), 0x00, sizeof(MN_MSG_Submit));

    /* FO */
    pos = 0;
    MSG_GET_TP_UDHI(smsSubmitInfo->userDataHeaderInd, tsRawData->data[pos]);
    /* TP VPF 23040 9.2.3.3 */
    MSG_GET_TP_VPF(smsSubmitInfo->validPeriod.validPeriod, tsRawData->data[pos]);
    pos++;

    /* TP MR */
    pos++;

    ret = TAF_MSG_DecodeSubmitInfoExceptUserDataForStk(smsSubmitInfo, &pos, tsRawData, dcsOffset);

    if (ret != MN_ERR_NO_ERROR) {
        MN_MSG_FreeForLib(WUEPS_PID_TAF, smsSubmitInfo);
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        return ret;
    }

    /*
     * 如果此时已经大于等于TPDU总长度，后面再读取数据就是脏数据，直接返回异常,
     * 同时此处做判断，保证后面的MN_MSG_CheckUdl第三个参数的减法运算不会越界
     */
    if (pos >= tsRawData->len) {
        MN_MSG_FreeForLib(WUEPS_PID_TAF, smsSubmitInfo);
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        MN_ERR_LOG("MN_MSG_GetSubmitInfoForStk: Invalid TPDU");
        return MN_ERR_CLASS_SMS_INVALID_TPDU;
    }

    ret = MN_MSG_GetSubmitUserDataInfoForStk(smsSubmitInfo, tsRawData, userDataLen, udhl, pos);
    if (ret != MN_ERR_NO_ERROR) {
        MN_MSG_FreeForLib(WUEPS_PID_TAF, smsSubmitInfo);
        MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
        MN_ERR_LOG("MN_MSG_GetSubmitInfoForStk: MN_MSG_GetSubmitUserDataInfoForStk fail.");
        return ret;
    }

    *msgCoding = smsSubmitInfo->dcs.msgCoding;

    /*lint -save -e516 */
    MN_MSG_FreeForLib(WUEPS_PID_TAF, smsSubmitInfo);
    MN_MSG_FreeForLib(WUEPS_PID_TAF, tsRawData);
    /*lint -restore */

    return MN_ERR_NO_ERROR;
}

