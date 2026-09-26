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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: OSA库函数功能实现
 * 生成日期: 2006年10月3日
 */


#include "v_lib.h"
#include "v_blk_mem.h"
#include "v_io.h"
#include "v_timer.h"
#include "v_private.h"
#include "mdrv.h"
#include "pam_tag.h"

#define THIS_FILE_ID PS_FILE_ID_V_LIB_C
#define THIS_MODU mod_pam_osa

#define COPY_LONG_SIZE (sizeof(VOS_UINT32))
#define COPY_WORD_SIZE (sizeof(VOS_UINT16))
#define COPY_LONG_MARK (COPY_LONG_SIZE - 1)
#define COPY_WORD_MARK (COPY_WORD_SIZE - 1)

#define MAX_SHIFT_NUM 32
#define MASK_BIT_ALL 0xFFFFFFFF
#define MASK_BIT_32 0x80000000
#define MASK_BIT_01 0x00000001
#define MASK_HI_NBITS(x) (MASK_BIT_ALL << (MAX_SHIFT_NUM - (x)))

#define VOS_STOL(S) (VOS_UINT_PTR)(S)

#define VOS_RAND_SEED_AND_ENTROPY_LEN (16)
#define VOS_RAND_NUM_LEN (4)

typedef enum {
    BIT64_GREAT,
    BIT64_EQUAL,
    BIT64_LESS
} Bit64CompareResult;

#define BIT64_COMPARE(argAHi, argALo, argBHi, argBLo, result) do { \
    if ((argAHi) > (argBHi)) {                                     \
        result = BIT64_GREAT;                                      \
    } else if (((argAHi) == (argBHi)) && ((argALo) > (argBLo))) {  \
        result = BIT64_GREAT;                                      \
    } else if (((argAHi) == (argBHi)) && ((argALo) == (argBLo))) { \
        result = BIT64_EQUAL;                                      \
    } else {                                                       \
        result = BIT64_LESS;                                       \
    }                                                              \
} while (0)

#define BIT64_ADD(argAHi, argALo, argBHi, argBLo) do { \
    if ((argALo) > 0xFFFFFFFF - (argBLo)) {   \
        (argAHi) += ((argBHi) + 1);           \
    } else {                                  \
        (argAHi) += (argBHi);                 \
    }                                         \
    (argALo) += (argBLo);                     \
} while (0)

/* Notes: we assumed argA always great than or equal to argB */
#define BIT64_SUB(argAHi, argALo, argBHi, argBLo) do { \
    if ((argALo) < (argBLo)) {                \
        (argAHi) -= ((argBHi) + 1);           \
    } else {                                  \
        (argAHi) -= (argBHi);                 \
    }                                         \
    (argALo) -= (argBLo);                     \
} while (0)

/* the seed of radom */
static VOS_UINT32 g_vosRadomSeed = 0;

VOS_UINT32 g_errorNo = 0; /* 存放错误码 */

#if ((VOS_WIN32 == VOS_OS_VER) || (VOS_LINUX == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
int errnoSet(int errorValue)
{
    return 0;
}
#endif

#if (VOS_RTOSCK == VOS_OS_VER)
extern VOS_UINT32 CRYPT_random(VOS_UINT8 *buf, VOS_UINT32 len);
#endif

/*
 * Description: Set global error number.
 */
MODULE_EXPORTED VOS_UINT32 V_SetErrorNo(VOS_UINT32 errorNo, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    g_errorNo = errorNo;
    mdrv_err("<VOS_SetErrorNo> F=%d, L=%d, ErrNo=%x.\n", (VOS_INT)fileId, lineNo, (VOS_INT)errorNo);
    return VOS_OK;
}

/*
 * Description: Get global error number.
 */
VOS_UINT32 VOS_GetErrorNo(VOS_VOID)
{
    return g_errorNo;
}

/*
 * Description: Is upper char?
 */
VOS_BOOL VOS_CharIsUpper(VOS_CHAR Char)
{
    if (Char >= 'A' && Char <= 'Z') {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: Is lower char?
 */
VOS_BOOL VOS_CharIsLower(VOS_CHAR Char)
{
    if (Char >= 'a' && Char <= 'z') {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: translate upper char
 */
MODULE_EXPORTED VOS_CHAR VOS_CharToUpper(VOS_CHAR Char)
{
    if (VOS_CharIsLower(Char)) {
        Char = (VOS_CHAR)(Char + ('A' - 'a'));
    }

    return Char;
}

/*
 * Description: translate lower char
 */
MODULE_EXPORTED VOS_CHAR VOS_CharToLower(VOS_CHAR Char)
{
    if (VOS_CharIsUpper(Char)) {
        Char = (VOS_CHAR)(Char - ('A' - 'a'));
    }

    return Char;
}

/*
 * Description: compare a string with another string
 */
MODULE_EXPORTED VOS_INT8 VOS_StrNiCmp(const VOS_CHAR *str1, const VOS_CHAR *str2, VOS_SIZE_T length)
{
    VOS_CHAR  char1, char2;
    const VOS_CHAR *tempStr1 = str1;
    const VOS_CHAR *tempStr2 = str2;

    if ((VOS_NULL_PTR == str1) || (VOS_NULL_PTR == str2)) {
        return 0;
    }

    char1 = 0;
    char2 = 0;

    while (length > 0) {
        length--;
        char1 = *tempStr1;
        char2 = *tempStr2;
        tempStr1++;
        tempStr2++;

        if ((!char1) || (!char2)) {
            break;
        }

        if (char1 == char2) {
            continue;
        }

        if ((char1 >= 'a' && char1 <= 'z') && (char2 >= 'A' && char2 <= 'Z')) {
            if ((char1 - char2) != 0x20) {
                break;
            }
        } else if ((char1 >= 'A' && char1 <= 'Z') && (char2 >= 'a' && char2 <= 'z')) {
            if ((char2 - char1) != 0x20) {
                break;
            }
        } else {
            break;
        }
    }

    char1 = VOS_CharToLower(char1);
    char2 = VOS_CharToLower(char2);

    return (VOS_INT8)((VOS_INT8)char1 - (VOS_INT8)char2);
}

/*
 * Description: Transform a string to upper
 */
VOS_VOID VOS_StrToUpper(VOS_CHAR *str)
{
    VOS_CHAR *tempStr = str;

    if (VOS_NULL_PTR == str) {
        return;
    }

    while (*tempStr) {
        *tempStr = VOS_CharToUpper(*tempStr);
        tempStr++;
    }

    return;
}

/*
 * Description: Transform a string to lower
 */
VOS_VOID VOS_StrToLower(VOS_CHAR *str)
{
    VOS_CHAR *tempStr = str;

    if (VOS_NULL_PTR == str) {
        return;
    }

    while (*tempStr) {
        *tempStr = VOS_CharToLower(*tempStr);
        tempStr++;
    }

    return;
}

/*
 * Description: determine the length of a string
 */
MODULE_EXPORTED VOS_UINT32 VOS_StrLen(const VOS_CHAR *str)
{
    const char *sc = VOS_NULL_PTR;

    if (VOS_NULL_PTR == str) {
        return 0;
    }

    for (sc = str; *sc != '\0'; ++sc) {
        /* nothing */;
    }

    return (VOS_UINT32)(VOS_STOL(sc) - VOS_STOL(str));
}

/*
 * Description: find the last occurrence of a character in a string
 */
VOS_CHAR *VOS_StrRChr(const VOS_CHAR *str, VOS_CHAR Char)
{
    const VOS_CHAR *temp = VOS_NULL_PTR;

    if (VOS_NULL_PTR == str) {
        return VOS_NULL_PTR;
    }

    temp = str + VOS_StrNLen(str, VOS_STR_MAX_LEN);

    do {
        if (*temp == Char) {
            return (VOS_CHAR *)temp;
        }
    } while (--temp >= str);

    return VOS_NULL_PTR;
}

/*
 * Description: comp two memory info
 */
MODULE_EXPORTED VOS_INT VOS_MemCmp(const VOS_VOID *dest, const VOS_VOID *src, VOS_SIZE_T count)
{
#if (VOS_LINUX == VOS_OS_VER)
    const unsigned char *su1 = VOS_NULL_PTR;
    const unsigned char *su2 = VOS_NULL_PTR;
    int                  res = 0;
#endif

#if (VOS_YES == VOS_CHECK_PARA)
    if ((VOS_NULL_PTR == dest) || (VOS_NULL_PTR == src)) {
        return 0;
    }
#endif

#if (VOS_LINUX == VOS_OS_VER)

    for (su1 = dest, su2 = src; 0 < count; ++su1, ++su2, count--) {
        if ((res = *su1 - *su2) != 0) {
            break;
        }
    }

    return res;
#else

    return memcmp(dest, src, count);
#endif
}

/*
 * Description: find the first occurrence of a substring in a string
 */
MODULE_EXPORTED VOS_CHAR *VOS_StrStr(const VOS_CHAR *str1, const VOS_CHAR *str2)
{
    VOS_UINT32 len1, len2;
    const VOS_CHAR *tempStr1 = str1;
    const VOS_CHAR *tempStr2 = str2;

    if ((VOS_NULL_PTR == str1) || (VOS_NULL_PTR == str2)) {
        return VOS_NULL_PTR;
    }

    len2 = VOS_StrNLen(tempStr2, VOS_STR_MAX_LEN);
    if (!len2) {
        return (VOS_CHAR *)tempStr1;
    }

    len1 = VOS_StrNLen(tempStr1, VOS_STR_MAX_LEN);

    while (len1 >= len2) {
        len1--;

        if (!VOS_MemCmp(tempStr1, tempStr2, len2)) {
            return (VOS_CHAR *)tempStr1;
        }

        tempStr1++;
    }

    return VOS_NULL_PTR;
}

/*
 * Description: compare two strings lexicographically
 */
VOS_INT VOS_StrCmp(const VOS_CHAR *str1, const VOS_CHAR *str2)
{
    register const VOS_CHAR *s1 = str1;
    register const VOS_CHAR *s2 = str2;

    if ((VOS_NULL_PTR == str1) || (VOS_NULL_PTR == str2)) {
        return 0;
    }

    while ((*s1 == *s2) && *s1) {
        s1++;
        s2++;
    }

    return (*s1 - *s2);
}

/*
 * Description: compute a string's len
 */
MODULE_EXPORTED VOS_UINT32 VOS_StrNLen(const VOS_CHAR *str, VOS_UINT32 count)
{
    const VOS_CHAR *temp = VOS_NULL_PTR;

    if (VOS_NULL_PTR == str) {
        return 0;
    }

    for (temp = str; count-- && (*temp != '\0'); ++temp) {
        /* nothing */
    }

    return (VOS_UINT32)(VOS_STOL(temp) - VOS_STOL(str));
}

/*
 * Description: search a block of memory for a character
 */
VOS_VOID *VOS_MemChr(const VOS_VOID *buf, VOS_UCHAR c, VOS_INT n)
{
    VOS_UCHAR *p = (VOS_UCHAR *)buf;

    if (VOS_NULL_PTR == buf) {
        return VOS_NULL_PTR;
    }

    if (0 == n) {
        return VOS_NULL_PTR;
    }

    do {
        if (*p++ == c) {
            return ((VOS_VOID *)(p - 1));
        }
    } while (--n != 0);

    return VOS_NULL_PTR;
}

/*
 * Description: A 64-bit number divided by a 64-bit one with the operation.
 * When input is 32-bit, it's 32bit implement.
 */
VOS_UINT32 VOS_64Div64(VOS_UINT32 dividendHigh, VOS_UINT32 dividendLow, VOS_UINT32 divisorHigh,
                       VOS_UINT32 divisorLow, VOS_UINT32 *quotientHigh, VOS_UINT32 *quotientLow,
                       VOS_UINT32 *remainderHigh, VOS_UINT32 *remainderLow)

{
    VOS_INT8           chShiftNumHi = 0;
    VOS_INT8           chShiftNumLo = 0;
    VOS_UINT32         tmpQuoHi, tmpQuoLo;
    VOS_UINT32         tmpDividendHi, tmpDividendLo;
    VOS_UINT32         tmpDivisorHi, tmpDivisorLo;
    Bit64CompareResult tmpResult;

    if (VOS_NULL_PTR == quotientHigh || VOS_NULL_PTR == quotientLow) {
        return VOS_ERRNO_LIB_64DIV64_QUOTIENT_NULL;
    }

    if (0 == divisorHigh) {
        if (0 == divisorLow) {
            return VOS_ERRNO_LIB_64DIV64_DIVIDE_ZERO;
        } else if (1 == divisorLow) {
            *quotientHigh = dividendHigh;
            *quotientLow  = dividendLow;

            if (remainderHigh != VOS_NULL_PTR && remainderLow != VOS_NULL_PTR) {
                *remainderHigh = 0;
                *remainderLow  = 0;
            }

            return VOS_OK;
        }
    }

    tmpQuoHi = tmpQuoLo = 0;
    tmpDividendHi         = dividendHigh;
    tmpDividendLo         = dividendLow;

    /* if divisor is larger than dividend, quotient equals to zero,remainder equals to dividends */
    /*lint -e717 */
    BIT64_COMPARE(dividendHigh, dividendLow, divisorHigh, divisorLow, tmpResult);
    /*lint +e717 */
    if (BIT64_LESS == tmpResult) {
        /*lint -e801 */
        goto returnHandle;
        /*lint +e801 */
    } else if (BIT64_EQUAL == tmpResult) {
        *quotientHigh = 0;
        *quotientLow  = 1;

        if (remainderHigh != VOS_NULL_PTR && remainderLow != VOS_NULL_PTR) {
            *remainderHigh = 0;
            *remainderLow  = 0;
        }

        return VOS_OK;
    }

    /* get shift number to implement divide arithmetic */
    if (divisorHigh > 0) {
        for (chShiftNumHi = 0; chShiftNumHi < MAX_SHIFT_NUM; chShiftNumHi++) {
            if ((divisorHigh << chShiftNumHi) & MASK_BIT_32) {
                break;
            }
        }
    } else {
        for (chShiftNumLo = 0; chShiftNumLo < MAX_SHIFT_NUM; chShiftNumLo++) {
            if ((divisorLow << chShiftNumLo) & MASK_BIT_32) {
                break;
            }
        }
    }

    if (divisorHigh > 0) {
        /* divisor's high 32 bits doesn't equal to zero */
        /*lint -save -e644*/
        for (; chShiftNumHi >= 0; chShiftNumHi--) {
            /*lint -restore*/
            if (0 == chShiftNumHi) {
                tmpDivisorHi = divisorHigh;
            } else {
                tmpDivisorHi = (divisorHigh << chShiftNumHi) | (divisorLow >> (MAX_SHIFT_NUM - chShiftNumHi));
            }

            tmpDivisorLo = divisorLow << chShiftNumHi;

            /*lint -e717 */
            BIT64_COMPARE(tmpDividendHi, tmpDividendLo, tmpDivisorHi, tmpDivisorLo, tmpResult);
            /*lint +e717 */

            /*lint -e701 */
            if (tmpResult != BIT64_LESS) {
                /*lint +e701 */
                /*lint -e717 */
                BIT64_SUB(tmpDividendHi, tmpDividendLo, tmpDivisorHi, tmpDivisorLo);
                /*lint +e717 */
                /*lint -e701 */
                tmpQuoLo |= (((VOS_UINT32)1) << chShiftNumHi);
                /*lint +e701 */
                if (0 == tmpDividendHi && 0 == tmpDividendLo) {
                    /*lint -e801 */
                    goto returnHandle;
                    /*lint +e801 */
                }
            }
        }
    } else {
        /* divisor's high 32 bits equals to zero */
        /*lint -save -e644*/
        chShiftNumHi = chShiftNumLo;
        /*lint -restore*/
        for (; chShiftNumHi >= 0; chShiftNumHi--) {
            tmpDivisorHi = divisorLow << chShiftNumHi;
            /*lint -e717 */
            BIT64_COMPARE(tmpDividendHi, tmpDividendLo, tmpDivisorHi, 0, tmpResult);
            /*lint +e717 */

            /*lint -e701 */
            if (tmpResult != BIT64_LESS) {
            /*lint +e701 */
                /*lint -save -e568*/
                /*lint -e717 -e685 */
                BIT64_SUB(tmpDividendHi, tmpDividendLo, tmpDivisorHi, 0);
                /*lint +e717 +e685 */
                /*lint -restore*/
                /*lint -e701 */
                tmpQuoHi |= (((VOS_UINT32)1) << chShiftNumHi);
                /*lint +e701 */
                if ((0 == tmpDividendHi) && (0 == tmpDividendLo)) {
                    /*lint -e801 */
                    goto returnHandle;
                    /*lint +e801 */
                }
            }
        }

        for (chShiftNumHi = MAX_SHIFT_NUM - 1; chShiftNumHi >= 0; chShiftNumHi--) {
            if (0 == chShiftNumHi) {
                tmpDivisorHi = 0;
            } else {
                tmpDivisorHi = divisorLow >> (MAX_SHIFT_NUM - chShiftNumHi);
            }

            tmpDivisorLo = divisorLow << chShiftNumHi;

            /*lint -e717 */
            BIT64_COMPARE(tmpDividendHi, tmpDividendLo, tmpDivisorHi, tmpDivisorLo, tmpResult);
            /*lint +e717 */
            /*lint -e701 */
            if (tmpResult != BIT64_LESS) {
            /*lint +e701 */
                /*lint -e717 */
                BIT64_SUB(tmpDividendHi, tmpDividendLo, tmpDivisorHi, tmpDivisorLo);
                /*lint +e717 */
                /*lint -e701 */
                tmpQuoLo |= (((VOS_UINT32)1) << chShiftNumHi);
                /*lint +e701 */
                if ((0 == tmpDividendHi) && (0 == tmpDividendLo)) {
                    /*lint -e801 */
                    goto returnHandle;
                    /*lint +e801 */
                }
            }
        }
    }

returnHandle:

    *quotientHigh = tmpQuoHi;
    *quotientLow  = tmpQuoLo;

    if (remainderHigh != VOS_NULL_PTR && remainderLow != VOS_NULL_PTR) {
        *remainderHigh = tmpDividendHi;
        *remainderLow  = tmpDividendLo;
    }

    return VOS_OK;
}

/*
 * Description: A 64-bit number divided by a 32-bit one with the operation.
 */
MODULE_EXPORTED VOS_UINT32 VOS_64Div32(VOS_UINT32 dividendHigh, VOS_UINT32 dividendLow, VOS_UINT32 divisor,
                                       VOS_UINT32 *quotientHigh, VOS_UINT32 *quotientLow,
                                       VOS_UINT32 *rtcRemainder)
{
    VOS_UINT32 tmpRemainderHi, tmpRemainderLo = 0;
    VOS_UINT32 ret;

    if (VOS_NULL_PTR == quotientHigh || VOS_NULL_PTR == quotientLow) {
        return VOS_ERRNO_LIB_64DIV32_QUOTIENT_NULL;
    }
    if (0 == divisor) {
        return VOS_ERRNO_LIB_64DIV32_DIVIDE_ZERO;
    }

    ret = VOS_64Div64(dividendHigh, dividendLow, 0, divisor, quotientHigh, quotientLow, &tmpRemainderHi,
                        &tmpRemainderLo);
    if (ret != VOS_OK) {
        return ret;
    }

    if (rtcRemainder != VOS_NULL_PTR) {
        *rtcRemainder = tmpRemainderLo;
    }

    return VOS_OK;
}

#if (VOS_RTOSCK == VOS_OS_VER)
/*
 * Description: This function returns Crypt random value within the range specified
 * 0..n-1 where n is the range specified.
 */
MODULE_EXPORTED VOS_UINT32 VOS_CryptRand(VOS_UINT32 *randomData, VOS_UINT32 range)
{
    register VOS_UINT32 genTemp;
    register VOS_UINT32 genTempHigh, rangeHigh, rangeLow;
    register VOS_UINT32 randomNumber;
    VOS_UINT8           vosRand[VOS_RAND_NUM_LEN] = {0};
    VOS_UINT32          i;

    /* 检测入参 */
    if (randomData == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    /* 获取4个随机数 */
    if (VOS_OK == CRYPT_random(vosRand, VOS_RAND_NUM_LEN)) {
        /* 组成1个U32的随机数 */
        genTemp = 0;
        for (i = 0; i < VOS_RAND_NUM_LEN; i++) {
            genTemp |= (VOS_UINT32)vosRand[i] << (i * 8);
        }
    } else {
        return VOS_ERR;
    }

    /* 将随机数匹配到范围内 */
    genTempHigh = genTemp >> 16;
    genTemp &= 0xffff;

    rangeHigh = (range & 0xffffffff) >> 16;
    rangeLow  = range & 0xffff;

    randomNumber = (genTemp * rangeLow) >> 16;
    randomNumber += genTempHigh * rangeLow;

    if (rangeHigh == 0) {
        randomNumber >>= 16;
    } else {
        randomNumber += genTemp * rangeHigh;
        randomNumber = (randomNumber >> 16) + (genTempHigh * rangeHigh);
    }

    *randomData = randomNumber;

    return VOS_OK;
}
#endif

/*
 * Description: This function initializes the array of random numbers generated
 * using with the seed value given
 */
MODULE_EXPORTED VOS_VOID VOS_SetSeed(VOS_UINT32 seed)
{
    g_vosRadomSeed = seed;

    return;
}

/*
 * Description: This function returns random value within the range specified
 * 0..n-1 where n is the range specified.
 */
MODULE_EXPORTED VOS_UINT32 VOS_Rand(VOS_UINT32 range)
{
    register VOS_UINT32 genTemp;
    register VOS_UINT32 genTempHigh, rangeHigh, rangeLow;
    register VOS_UINT32 randomNumber;

    /* 设置了种子用设置的种子，否则用系统时间做种子 */
    if (0 == g_vosRadomSeed) {
        genTemp = VOS_GetSlice();
    } else {
        genTemp = g_vosRadomSeed;
    }

    /* 网络搜索算法生成伪随机数 */
    genTemp = (genTemp * 1664525L + 1013904223L);

    g_vosRadomSeed = genTemp;

    /* 将伪随机匹配到范围内 */
    genTempHigh = genTemp >> 16;
    genTemp &= 0xffff;

    rangeHigh = (range & 0xffffffff) >> 16;
    rangeLow = range & 0xffff;

    randomNumber = (genTemp * rangeLow) >> 16;
    randomNumber += genTempHigh * rangeLow;

    if (rangeHigh == 0) {
        randomNumber >>= 16;
    } else {
        randomNumber += genTemp * rangeHigh;
        randomNumber = (randomNumber >> 16) + (genTempHigh * rangeHigh);
    }

    return randomNumber;
}

/*
 * Description: A 64-bit number add  another 64-bit number
 * when inputs are big enough, the overflow maybe occur,please be aware of it in use.
 */
void add64(VOS_UINT32 *low, VOS_UINT32 *high, VOS_UINT32 oldlow, VOS_UINT32 oldhigh)
{
    if (*low > 0xFFFFFFFF - oldlow) {
        *high += (oldhigh + 1);
    } else {
        *high += oldhigh;
    }
    *low += oldlow;
}

/*
 * Description: A 64-bit number multiplied by a 32-bit one with the operation.
 */
MODULE_EXPORTED VOS_UINT32 VOS_64Multi32(VOS_UINT32 multiplicandHigh, VOS_UINT32 multiplicandLow,
                                         VOS_UINT32 multiplicator, VOS_UINT32 *productHigh,
                                         VOS_UINT32 *productLow)
{
    VOS_UINT32 count, xbit, record, tmpHigh, tmpLow;

    if (VOS_NULL_PTR == productHigh || VOS_NULL_PTR == productLow) {
        return VOS_ERRNO_LIB_64MUL32_QUOTIENT_NULL;
    }

    *productHigh = *productLow = 0;

    if ((0 == multiplicandHigh && 0 == multiplicandLow) || (0 == multiplicator)) {
        return VOS_OK;
    }

    for (count = 32, xbit = MASK_BIT_32; count > 1; xbit >>= 1, count--) {
        if (multiplicator & xbit) {
            if (0 != (multiplicandHigh & MASK_HI_NBITS(count - 1))) {
                /*lint -e801 */
                goto overflowHandle;
                /*lint +e801 */
            }
            record = multiplicandLow & MASK_HI_NBITS(count - 1);
            record >>= (32 - (count - 1));
            tmpHigh = multiplicandHigh << (count - 1) | record;
            tmpLow  = multiplicandLow << (count - 1);

            add64(productLow, productHigh, tmpLow, tmpHigh);
            /* If check failed, means overflow occurred */
            if (*productHigh < tmpHigh) {
                /*lint -e801 */
                goto overflowHandle;
                /*lint +e801 */
            }
        }
    }

    if (multiplicator & MASK_BIT_01) {
        add64(productLow, productHigh, multiplicandLow, multiplicandHigh);
        /* If check failed, means overflow occurred */
        if (*productHigh < multiplicandHigh) {
            /*lint -e801 */
            goto overflowHandle;
            /*lint +e801 */
        }
    }

    return VOS_OK;

overflowHandle:

    *productHigh = *productLow = 0;

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_LIB_64MUL32_OVERFLOW);
    return VOS_ERRNO_LIB_64MUL32_OVERFLOW;
}

/*
 * Description: return the string length
 */
VOS_SIZE_T VOS_StrSpn(const VOS_CHAR *str1, const VOS_CHAR *str2)
{
    const VOS_CHAR *save = VOS_NULL_PTR;
    const VOS_CHAR *temp = VOS_NULL_PTR;
    VOS_CHAR        c1;
    VOS_CHAR        c2;

    if( (str1  == VOS_NULL_PTR) || (str2  == VOS_NULL_PTR) ) {
        return 0;
    }

    for (save = str1 + 1; '\0' != (c1 = *str1++);) {
        for (temp = str2; (c2 = *temp++) != c1;) {
            if ('\0' == c2) {
                return (VOS_SIZE_T)(str1 - save);
            }
        }
    }

    return (VOS_SIZE_T)(str1 - save);
}

/*
 * Description: find the first occurrence in a string of a character
 * from a given set
 */
VOS_CHAR *VOS_StrPbrk(const VOS_CHAR *str1, const VOS_CHAR *str2)
{
    VOS_CHAR *scan = VOS_NULL_PTR;
    VOS_CHAR  c;
    VOS_CHAR  sc;

    if( (str1  == VOS_NULL_PTR) || (str2  == VOS_NULL_PTR) ) {
        return VOS_NULL_PTR;
    }

    while ('\0' != (c = *str1++)) { /* wait until end of string */
        /* loop, searching for character */
        for (scan = (VOS_CHAR *)str2; '\0' != (sc = *scan++);) {
            if (sc == c) {
                return ((VOS_CHAR *)(str1 - 1));
            }
        }
    }

    return VOS_NULL_PTR;
}

/*
 * 功能描述: VOS动态申请内存/消息空间大小检查(其他非VOS分配检测不了，直接返回OK)
 */
MODULE_EXPORTED VOS_UINT32 V_CheckAllocedMem(const VOS_VOID *dest, VOS_SIZE_T destSize, VOS_UINT32 fileId,
                                             VOS_INT32 lineNo)
{
    VOS_UINT32 size;

    if (VOS_NULL_PTR == dest) {
        return VOS_ERR;
    }

    if (0 == destSize) {
        return VOS_OK;
    }

    if (VOS_OK != VOS_LocationMem(dest, &size, fileId, lineNo)) {
        mdrv_err("<V_CheckMem_s> destination error.file=%d l=%d.\n", fileId, lineNo);

        return VOS_ERR;
    }

    if (destSize > size) {
        mdrv_err("<V_CheckMem_s> big.file=%d l=%d.size=%d real size=%d.\n", fileId, lineNo, destSize, size);

        return VOS_ERR;
    }

    return VOS_OK;
}

