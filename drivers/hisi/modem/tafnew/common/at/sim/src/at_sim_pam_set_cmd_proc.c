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
#include "at_sim_pam_set_cmd_proc.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "at_ctx.h"
#include "at_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_check_func.h"
#include "at_mbb_cmd.h"
#include "at_msg_print.h"
#include "taf_std_lib.h"
#include "at_sim_comm.h"
#include "si_app_pb.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SIM_PAM_SET_CMD_PROC_C

#define AT_GSM_DATA_MAX_LEN 2
#define AT_SEARCH_GSM_NULL_DST_LEN 0
#define AT_SEARCH_GSM_UNICODE_BASIC_DST_LEN 1
#define AT_SEARCH_GSM_UNICODE_EXTEND_DST_LEN 2
#define AT_SRC_OFFSET_1 1
#define AT_CPBR_PARA_MIN_NUM 1
#define AT_CPBR_PARA_MAX_NUM 2
#define AT_CPBW_NUMBER 1
#define AT_CPBW_TYPE 2
#define AT_CPBW_TEXT 3
#define AT_CPBW_CODING 4
#define AT_SCPBW_NUM1 1
#define AT_SCPBW_TYPE1 2
#define AT_SCPBW_NUM2 3
#define AT_SCPBW_TYPE2 4
#define AT_SCPBW_NUM3 5
#define AT_SCPBW_TYPE3 6
#define AT_SCPBW_NUM4 7
#define AT_SCPBW_TYPE4 8
#define AT_SCPBW_TEXT 9
#define AT_SCPBW_ALPHATAG_TYPE 10
#define AT_SCPBW_EMAIL 11
#define AT_ADDITION_NUM_ARRAY_INDEX_2 2
#define AT_SCPBW_NUM4_NUM_TYPE 0
#define AT_CPBW_PRIVATA_PARA_MAX_NUM 5
#define AT_CPBW_PARA_MAX_NUM 4
#define AT_SCPBW_PARA_MAX_NUM 12
#define AT_SCPBR_PARA_MAX_NUM 2
#define AT_CCHP_PARA_MAX_NUM 2
#define AT_CRSM_PARA_MIN_NUM 2
#define AT_CPBW_TEXT_MIN_NUM 128
#define AT_EFID_4F_PATH_LEN 3
#define AT_EFID_6F_PATH_LEN 2
#define AT_PATH_INDEX_1 1
#define AT_PATH_INDEX_2 2
#define AT_CCHO_DFNAME 0
#define AT_CCHP_DFNAME 0
#define AT_CRSM_FILEID 1
#define AT_CRSM_P1 2
#define AT_CRSM_P2 3
#define AT_CRSM_P3 4
#define AT_CRSM_DATA 5
#define AT_CRSM_PATHIDE 6
#define AT_CPBW_NLENGTH 4
#define AT_CARDMODE_SIM_TYPE_NULL_CARD 0
#define AT_CARDMODE_SIM_TYPE_SIM 1
#define AT_CARDMODE_SIM_TYPE_USIM 2
#define AT_CRSM_UPDATA_BINARY_PARA_MIN_NUM 6
#define AT_CRSM_PARA_MAX_NUM 7
#define AT_HVSST_PARA_NUM 2
#define AT_CURSM_COMMAND 1
#define AT_CURSM_FILEID 2
#define AT_CURSM_RECORD_NUMBER 3
#define AT_CURSM_DATA 4
#define AT_CURSM_PATHID 5
#define AT_EFLOCIINFO_PARA_NUM 5
#define AT_EFLOCIINFO_PLMN 1
#define AT_EFLOCIINFO_TMSI 0
#define AT_EFLOCIINFO_LAC 2
#define AT_EFLOCIINFO_LOCATION_UPDATE_STATUS 3
#define AT_EFLOCIINFO_RFU 4
#define AT_EFLOCIINFO_PLMN_THIRD_PARA 3
#define AT_MCC_STR_LEN 3
#define AT_EFPSLOCIINFO_PARA_NUM 6
#define AT_EFPSLOCIINFO_PLMN 2
#define AT_EFPSLOCIINFO_LAC 3
#define AT_EFPSLOCIINFO_RAC 4
#define AT_EFPSLOCIINFO_ROUTING_AREA_UPDATE_STATUS 5
#define AT_EFPSLOCIINFO_PLMN_THIRD_PARA 3
#define AT_CURSM_MIN_PARA_NUM 4
#define AT_CURSM_MAX_PARA_NUM 6
#define AT_CURSM_APP_TYPE 0
#define AT_PLMN_MCC_CLASS_MASK 0xFFFFF000
#define AT_HALF_BYTE_TO_BITS_LENS 4
#define AT_AUC_GSM_DATA_LEN 2
#define AT_PUC_DATA_OFFSET_1 1
#define AT_PUC_DATA_INDEX_2 2
#define AT_PUC_DATA_INDEX_3 3
#define AT_CRLA_MAX_PARA_NUM 8
#define AT_CRLA_PARA_GETRSP_MIN_NUM 2
#define AT_CRLA_PARA_UPDATE_BINARY_MIN_NUM 6
#define AT_CRLA_PARA_SEARCH_RECORD_MIN_NUM 7
#define AT_CRLA_COMMAND 1
#define AT_CRLA_FILEID 2
#define AT_CRLA_P1 3
#define AT_CRLA_P2 4
#define AT_CRLA_P3 5
#define AT_CRLA_DATA 6
#define AT_CRLA_PATHID 7
#define AT_SIM_FILE_PATH_MASK 0xFF00
#define AT_SIM_FILE_PATH_LEN_2 2
#define AT_SIM_FILE_PATH_LEN_3 3
#define AT_SIM_FILE_PATH_INDEX_1 1
#define AT_SIM_FILE_PATH_INDEX_2 2

TAF_VOID At_PbIraToUnicode(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst, TAF_UINT16 *dstLen)
{
    TAF_UINT16 indexNum;
    TAF_UINT16 ucs2;

    for (indexNum = 0; indexNum < srcLen; indexNum++) {
        /* 查IRA到UNICODE转义表， 将IRA字符转换为unicode编码 */
        ucs2 = g_iraToUnicode[src[indexNum]].unicode;
        *dst = (TAF_UINT8)(ucs2 >> 8);
        dst++;
        *dst = (TAF_UINT8)ucs2;
        dst++;
    }

    /* 转换后长度为原来的2倍 */
    *dstLen = (TAF_UINT16)(srcLen << 1);
}

TAF_UINT32 At_OneUnicodeToGsm(TAF_UINT16 src, TAF_UINT8 *dst, TAF_UINT32 dstBufLen, TAF_UINT16 *dstLen)
{
    TAF_UINT16 indexNum;

    /* 避免输入码流为A0A1时存入SIM卡内容为1B40读出时上报为007C */
    if (src == 0x00A0) {
        AT_LOG1("At_OneUnicodeToGsm no found one unicode[0x%x] to gsm", src);

        return AT_FAILURE;
    }

    if (dstBufLen < AT_GSM_DATA_MAX_LEN) {
        AT_LOG1("At_OneUnicodeToGsm Dst len[%d] is not enough.", dstBufLen);
        return AT_FAILURE;
    }

    /* 查找GSM 到 UNICODE基本表 */
    for (indexNum = 0; indexNum < AT_PB_GSM_MAX_NUM; indexNum++) {
        if (src == g_gsmToUnicode[indexNum].unicode) {
            *dst = g_gsmToUnicode[indexNum].octet;

            *dstLen = AT_SEARCH_GSM_UNICODE_BASIC_DST_LEN;

            return AT_SUCCESS;
        }
    }

    /* 查找GSM 到 UNICODE扩展表 */
    for (indexNum = 0; indexNum < AT_PB_GSM7EXT_MAX_NUM; indexNum++) {
        if (src == g_gsm7extToUnicode[indexNum].unicode) {
            *dst++ = AT_PB_GSM7EXT_SYMBOL;
            *dst   = g_gsm7extToUnicode[indexNum].octet;

            *dstLen = AT_SEARCH_GSM_UNICODE_EXTEND_DST_LEN;

            return AT_SUCCESS;
        }
    }

    /* 未查找到时返回失败 */
    *dstLen = AT_SEARCH_GSM_NULL_DST_LEN;

    AT_LOG1("At_OneUnicodeToGsm no found one unicode[0x%x] to gsm", src);

    return AT_FAILURE;
}

TAF_UINT32 At_UnicodeTransferToGsmCalcLength(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT16 *dstLen)
{
    TAF_UINT16 indexNum;
    TAF_UINT16 unicodeChar;
    TAF_UINT8  gsmData[AT_GSM_DATA_MAX_LEN];
    TAF_UINT16 gsmDataLen;

    for (indexNum = 0; indexNum < (srcLen >> 1); indexNum++) {
        unicodeChar = (*src << 8) | (*(src + AT_SRC_OFFSET_1));

        /* 判断UNICODE是否可以转义为GSM7BIT */
        if (At_OneUnicodeToGsm(unicodeChar, gsmData, sizeof(gsmData), &gsmDataLen) != AT_SUCCESS) {
            AT_ERR_LOG("At_UnicodeTransferToGsmCalcLength error");

            return AT_FAILURE;
        }

        src += 2; /* Unicode编码占两字节，每次偏移量为2 */

        *dstLen += gsmDataLen;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_UnicodeTransferTo81CalcLength(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 *baseCode,
                                            TAF_UINT16 *dstLen)
{
    TAF_UINT16 unicodeChar;
    TAF_UINT16 baseCodeTemp = 0;
    TAF_UINT16 indexNum;
    TAF_BOOL   bIsBaseSet = TAF_FALSE;
    TAF_UINT16 dstLenTemp = 0;
    TAF_UINT8  gsmData[AT_AUC_GSM_DATA_LEN];
    TAF_UINT16 gsmDataLen = 0;

    /* 输入参数赋初值 */
    *dstLen   = 0;
    *baseCode = 0;

    for (indexNum = 0; indexNum < (len >> 1); indexNum++) {
        unicodeChar = (*data << 8) | (*(data + AT_PUC_DATA_OFFSET_1));

        data += AT_DOUBLE_LENGTH;

        /* 反向查找GSM到UCS的基本表和扩展表 */
        if (At_OneUnicodeToGsm(unicodeChar, gsmData, sizeof(gsmData), &gsmDataLen) == AT_SUCCESS) {
            dstLenTemp += gsmDataLen;

            continue;
        }

        /* 81编码用第8到第15bit进行编码，即0hhh hhhh hXXX XXXX，因此第16bit为1一定不能进行81编码 */
        if ((unicodeChar & 0x8000) != 0) {
            AT_ERR_LOG("At_UnicodeTransferTo81CalcLength error: no16 bit is 1");

            return AT_FAILURE;
        }

        if (bIsBaseSet == TAF_FALSE) {
            /* 取第一个UCS2的第8到第15bit作为BaseCode，BaseCode右移7位即为基址针 */
            bIsBaseSet   = TAF_TRUE;
            baseCodeTemp = unicodeChar & AT_PB_81_CODE_BASE_POINTER;
        } else {
            if (baseCodeTemp != (unicodeChar & AT_PB_81_CODE_BASE_POINTER)) {
                AT_ERR_LOG("At_UnicodeTransferTo81CalcLength error: code base error");

                return AT_FAILURE;
            }
        }

        dstLenTemp++;
    }

    /* 能进行81编码，返回基本码和81编码总长度 */
    *baseCode = baseCodeTemp;
    *dstLen   = dstLenTemp + AT_PB_81CODE_HEADER_LEN;

    return AT_SUCCESS;
}

TAF_UINT32 At_UnicodeTransferTo82CalcLength(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 *baseCode,
                                            TAF_UINT16 *dstLen)
{
    TAF_UINT16 unicodeChar;
    TAF_UINT16 indexNum;
    TAF_BOOL   bIsBaseSet   = TAF_FALSE;
    TAF_UINT16 us82BaseHigh = 0;
    TAF_UINT16 us82BaseLow  = 0;
    TAF_UINT16 dstLenTemp   = 0;
    TAF_UINT8  gsmData[AT_AUC_GSM_DATA_LEN];
    TAF_UINT16 gsmDataLen = 0;

    /* 输入参数赋初值 */
    *dstLen   = 0;
    *baseCode = 0;

    for (indexNum = 0; indexNum < (len >> 1); indexNum++) {
        unicodeChar = (*data << 8) | (*(data + AT_PUC_DATA_OFFSET_1));

        data += AT_DOUBLE_LENGTH;

        /* 反向查找GSM到UCS的基本表和扩展表 */
        if (At_OneUnicodeToGsm(unicodeChar, gsmData, sizeof(gsmData), &gsmDataLen) == AT_SUCCESS) {
            dstLenTemp += gsmDataLen;

            continue;
        }

        if (bIsBaseSet == TAF_FALSE) {
            bIsBaseSet   = TAF_TRUE;
            us82BaseHigh = unicodeChar;
            us82BaseLow  = unicodeChar;
        } else {
            if (unicodeChar < us82BaseLow) {
                us82BaseLow = unicodeChar;
            }

            if (unicodeChar > us82BaseHigh) {
                us82BaseHigh = unicodeChar;
            }

            /* UCS2码流中最大编码和最小编码差值超过127就不能进行82编码 */
            if ((us82BaseHigh - us82BaseLow) > AT_PB_GSM7_CODE_MAX_VALUE) {
                AT_ERR_LOG("At_UnicodeTransferTo82CalcLength error: code base error");

                return AT_FAILURE;
            }
        }

        dstLenTemp++;
    }

    /* 能进行82编码，返回基本码和82编码总长度 */
    *baseCode = us82BaseLow;
    *dstLen   = dstLenTemp + AT_PB_82CODE_HEADER_LEN;

    return AT_SUCCESS;
}

TAF_UINT32 At_UnicodeTransferTo81(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 codeBase, TAF_UINT8 *dest)
{
    errno_t    memResult;
    TAF_UINT16 unicodeChar;
    TAF_UINT16 i;
    TAF_UINT8  gsmData[AT_AUC_GSM_DATA_LEN];
    TAF_UINT16 gsmDataLen = 0;
    TAF_UINT16 dstLen     = AT_PB_81CODE_HEADER_LEN;

    for (i = 0; i < (len >> 1); i++) {
        unicodeChar = (*data << 8) | (*(data + AT_PUC_DATA_OFFSET_1));

        data += AT_DOUBLE_LENGTH;

        /* 反向查找GSM到UCS的基本表和扩展表 */
        if (At_OneUnicodeToGsm(unicodeChar, gsmData, sizeof(gsmData), &gsmDataLen) == AT_SUCCESS) {
            if (gsmDataLen > 0) {
                memResult = memcpy_s((dest + dstLen), SI_PB_ALPHATAG_MAX_LEN - dstLen, gsmData, gsmDataLen);
                TAF_MEM_CHK_RTN_VAL(memResult, SI_PB_ALPHATAG_MAX_LEN - dstLen, gsmDataLen);
            }
            dstLen += gsmDataLen;

            continue;
        }

        /* 容错处理 */
        if ((codeBase | (unicodeChar & AT_PB_GSM7_CODE_MAX_VALUE)) == unicodeChar) {
            /* 转为81的编码方式 */
            dest[dstLen] = (TAF_UINT8)(AT_PB_CODE_NO8_BIT | (unicodeChar & AT_PB_GSM7_CODE_MAX_VALUE));
            dstLen++;
        } else {
            AT_ERR_LOG("At_UnicodeTransferTo81 error");

            return AT_FAILURE;
        }
    }

    /* 设置81编码header信息 */
    dest[0] = SI_PB_ALPHATAG_TYPE_UCS2_81;
    dest[1] = (TAF_UINT8)(dstLen - AT_PB_81CODE_HEADER_LEN);
    dest[AT_PUC_DATA_INDEX_2] = (TAF_UINT8)((codeBase & AT_PB_81_CODE_BASE_POINTER) >> 7);

    return AT_SUCCESS;
}

TAF_UINT32 At_UnicodeTransferTo82(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 codeBase, TAF_UINT8 *dest)
{
    errno_t    memResult;
    TAF_UINT16 unicodeChar;
    TAF_UINT16 i;
    TAF_UINT8  gsmData[AT_AUC_GSM_DATA_LEN];
    TAF_UINT16 gsmDataLen = 0;
    TAF_UINT16 dstLen     = AT_PB_82CODE_HEADER_LEN;

    for (i = 0; i < (len >> 1); i++) {
        unicodeChar = (*data << 8) | (*(data + AT_PUC_DATA_OFFSET_1));

        data += AT_DOUBLE_LENGTH;

        /* 反向查找GSM到UCS的基本表和扩展表 */
        if (At_OneUnicodeToGsm(unicodeChar, gsmData, sizeof(gsmData), &gsmDataLen) == AT_SUCCESS) {
            if (gsmDataLen > 0) {
                memResult = memcpy_s(dest + dstLen, SI_PB_ALPHATAG_MAX_LEN - dstLen, gsmData, gsmDataLen);
                TAF_MEM_CHK_RTN_VAL(memResult, SI_PB_ALPHATAG_MAX_LEN - dstLen, gsmDataLen);
            }
            dstLen += gsmDataLen;

            continue;
        }

        /* 容错处理 */
        if ((unicodeChar - codeBase) <= AT_PB_GSM7_CODE_MAX_VALUE) {
            /* 转为82的编码方式 */
            dest[dstLen] = ((TAF_UINT8)(unicodeChar - codeBase)) | AT_PB_CODE_NO8_BIT;
            dstLen++;
        } else {
            AT_ERR_LOG("At_UnicodeTransferTo81 error");

            return AT_FAILURE;
        }
    }

    /* 设置82编码header信息 */
    dest[0] = SI_PB_ALPHATAG_TYPE_UCS2_82;
    dest[1] = (TAF_UINT8)(dstLen - AT_PB_82CODE_HEADER_LEN);
    dest[AT_PUC_DATA_INDEX_2] = (TAF_UINT8)((codeBase & 0xff00) >> 8);
    dest[AT_PUC_DATA_INDEX_3] = (TAF_UINT8)(codeBase & 0x00ff);

    return AT_SUCCESS;
}

TAF_VOID At_PbUnicodeParse(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT32 *alphaTagType, TAF_UINT16 *dstLen,
                           TAF_UINT16 *codeBase)
{
    TAF_UINT16 gsmCodeLen  = 0;
    TAF_UINT16 us81CodeLen = 0;
    TAF_UINT16 us82CodeLen = 0;
    TAF_UINT16 us81Base;
    TAF_UINT16 us82Base;
    TAF_UINT16 minCodeLen;
    TAF_UINT32 alphaTagTypeTemp;
    TAF_UINT16 codeBaseTemp;

    /* 先尝试转成GSM模式,如果能转换成功，则返回 */
    if (At_UnicodeTransferToGsmCalcLength(src, srcLen, &gsmCodeLen) == AT_SUCCESS) {
        *alphaTagType = SI_PB_ALPHATAG_TYPE_GSM;
        *dstLen       = gsmCodeLen;
        *codeBase     = 0;

        return;
    }

    /*
     * 默认置为80编码
     */
    minCodeLen       = srcLen + 1;
    alphaTagTypeTemp = SI_PB_ALPHATAG_TYPE_UCS2_80;
    codeBaseTemp     = 0;

    /* 计算81编码长度,选定最小编码长度及相应的编码方式 */
    if (At_UnicodeTransferTo81CalcLength(src, srcLen, &us81Base, &us81CodeLen) == AT_SUCCESS) {
        if (us81CodeLen <= minCodeLen) {
            minCodeLen       = us81CodeLen;
            alphaTagTypeTemp = SI_PB_ALPHATAG_TYPE_UCS2_81;
            codeBaseTemp     = us81Base;
        }
    }

    /* 计算82编码长度,选定最小编码长度及相应的编码方式 */
    if (At_UnicodeTransferTo82CalcLength(src, srcLen, &us82Base, &us82CodeLen) == AT_SUCCESS) {
        if (us82CodeLen <= minCodeLen) {
            minCodeLen       = us82CodeLen;
            alphaTagTypeTemp = SI_PB_ALPHATAG_TYPE_UCS2_82;
            codeBaseTemp     = us82Base;
        }
    }

    *dstLen       = minCodeLen;
    *codeBase     = codeBaseTemp;
    *alphaTagType = alphaTagTypeTemp;
}

TAF_UINT32 At_UnicodeTransferToGsm(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst, TAF_UINT32 dstLen)
{
    TAF_UINT16 indexNum;
    TAF_UINT16 unicodeChar;
    TAF_UINT16 gsmDataLen = 0;

    for (indexNum = 0; indexNum < (srcLen >> 1); indexNum++) {
        unicodeChar = (*src << 8) | (*(src + AT_SRC_OFFSET_1));

        /* 逐个字符进行转换 */
        if (At_OneUnicodeToGsm(unicodeChar, dst, (dstLen - gsmDataLen), &gsmDataLen) != AT_SUCCESS) {
            AT_ERR_LOG("At_UnicodeTransferToGsm error");

            return AT_FAILURE;
        }

        dst += gsmDataLen;
        src += 2; /* UCS2 Unicode编码占两字节，每次偏移量为2 */
    }

    return AT_SUCCESS;
}

TAF_VOID At_UnicodeTransferTo80(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst)
{
    errno_t    memResult;
    VOS_UINT32 srcSize;
    dst[0] = SI_PB_ALPHATAG_TYPE_UCS2_80;

    srcSize = TAF_STD_MemFunc_Min(SI_PB_ALPHATAG_MAX_LEN - 1, srcLen);
    if (srcSize > 0) {
        memResult = memcpy_s(&dst[1], SI_PB_ALPHATAG_MAX_LEN - 1, src, srcSize);
        TAF_MEM_CHK_RTN_VAL(memResult, SI_PB_ALPHATAG_MAX_LEN - 1, srcLen);
    }
}

TAF_UINT32 At_PbUnicodeTransfer(TAF_UINT8 *src, TAF_UINT16 srcLen, SI_PB_Record *record, TAF_UINT16 codeBase)
{
    TAF_UINT32 result = AT_FAILURE;

    switch (record->alphaTagType) {
        case SI_PB_ALPHATAG_TYPE_GSM:
            result = At_UnicodeTransferToGsm(src, srcLen, record->alphaTag, sizeof(record->alphaTag));

            break;

        case SI_PB_ALPHATAG_TYPE_UCS2_81:
            result = At_UnicodeTransferTo81(src, srcLen, codeBase, record->alphaTag);

            break;

        case SI_PB_ALPHATAG_TYPE_UCS2_82:
            result = At_UnicodeTransferTo82(src, srcLen, codeBase, record->alphaTag);

            break;

        case SI_PB_ALPHATAG_TYPE_UCS2_80:
            At_UnicodeTransferTo80(src, srcLen, record->alphaTag);
            result = AT_SUCCESS;

            break;

        default:

            break;
    }

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_PbUnicodeTransfer: UCS2 transfer to gsm, 80, 81 or 82 error");
    }

    return result;
}

TAF_UINT32 At_Gsm7BitFormat(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst, TAF_UINT8 *dstLen)
{
    TAF_UINT16 indexNum;

    for (indexNum = 0; indexNum < srcLen; indexNum++) {
        /* GSM模式下输入码流大于0x7f时提示用户输入中含有非法字符 */
        if (src[indexNum] > AT_PB_GSM7_CODE_MAX_VALUE) {
            return AT_FAILURE;
        }

        dst[indexNum] = src[indexNum];
    }

    *dstLen = (TAF_UINT8)srcLen;

    return AT_SUCCESS;
}

TAF_UINT32 At_PbAlphaTagProc(SI_PB_Record *record, AT_ParseParaType *atParaList)
{
    errno_t    memResult;
    TAF_UINT8 *alphaTag = VOS_NULL_PTR;
    TAF_UINT16 alphaTagLen;
    TAF_UINT16 alphaCodeLen;
    TAF_UINT16 codeBase;

    /* 姓名字段为空时，不用处理姓名字段，但电话号码应该保存，所以返回成功 */
    if (atParaList->paraLen == 0) {
        return AT_SUCCESS;
    }

    if (g_atCscsType == AT_CSCS_GSM_7Bit_CODE) {
        if (atParaList->paraLen > g_pbatInfo.nameMaxLen) {
            AT_ERR_LOG("At_PbAlphaTagProc error: text string to long in gsm7");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        record->alphaTagType = SI_PB_ALPHATAG_TYPE_GSM;

        /* GSM模式下判断是否所有输入码流小于0x7f，若大于0x7f时提示用户输入中含有非法字符 */
        if (At_Gsm7BitFormat(atParaList->para, atParaList->paraLen, record->alphaTag, &record->alphaTagLength) !=
            AT_SUCCESS) {
            AT_ERR_LOG("At_PbAlphaTagProc: error:the eighth bit is 1 in gsm7");

            return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
        }

        return AT_SUCCESS;
    }

    /* 分配空间用于存储中间转换的姓名字段，最多保存为80编码，因此最大空间为输入长度2倍 */
    alphaTagLen = (TAF_UINT16)(atParaList->paraLen << 1);

    alphaTag = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, alphaTagLen);

    if (alphaTag == VOS_NULL_PTR) {
        AT_ERR_LOG("At_PbAlphaTagProc error:malloc failed");

        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(alphaTag, alphaTagLen, 0x00, alphaTagLen);

    /* IRA模式输入时先查表转换为UCS2，再处理 */
    if (g_atCscsType == AT_CSCS_IRA_CODE) {
        At_PbIraToUnicode(atParaList->para, atParaList->paraLen, alphaTag, &alphaTagLen);
    } else {
        /* UCS2编码方式下，两字节标识一个字符，即码流是四的倍数 */
        if ((atParaList->paraLen % 4) != 0) {
            VOS_MemFree(WUEPS_PID_AT, alphaTag);

            AT_LOG1("At_PbAlphaTagProc error: ucs2 codes length [%d] is nor 4 multiple", atParaList->paraLen);

            return AT_ERROR;
        }

        if (At_UnicodePrint2Unicode(atParaList->para, &(atParaList->paraLen)) == AT_FAILURE) {
            VOS_MemFree(WUEPS_PID_AT, alphaTag);

            AT_ERR_LOG("At_PbAlphaTagProc error: Printunicode to unicode failed");

            return AT_ERROR;
        }

        alphaTagLen = atParaList->paraLen;

        memResult = memcpy_s(alphaTag, alphaTagLen, atParaList->para, alphaTagLen);
        TAF_MEM_CHK_RTN_VAL(memResult, alphaTagLen, alphaTagLen);
    }

    /* 对UCS2或IRA模式转换后的UCS2码流确定存储格式 */
    At_PbUnicodeParse(alphaTag, alphaTagLen, &record->alphaTagType, &alphaCodeLen, &codeBase);

    /* 最终确定的编码长度大于电话本姓名字段能存储的长度时，提醒用户输入太长 */
    if (alphaCodeLen > g_pbatInfo.nameMaxLen) {
        VOS_MemFree(WUEPS_PID_AT, alphaTag);

        AT_LOG1("At_PbAlphaTagProc error: name length [%d] too long", alphaCodeLen);

        return AT_CME_TEXT_STRING_TOO_LONG;
    }

    record->alphaTagLength = (TAF_UINT8)alphaCodeLen;

    if (At_PbUnicodeTransfer(alphaTag, alphaTagLen, record, codeBase) != AT_SUCCESS) {
        VOS_MemFree(WUEPS_PID_AT, alphaTag);

        AT_ERR_LOG("At_PbAlphaTagProc: fail to convert UCS2");

        return AT_ERROR;
    }

    VOS_MemFree(WUEPS_PID_AT, alphaTag);

    return AT_SUCCESS;
}

VOS_BOOL AT_CheckCpbsPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

TAF_UINT32 At_SetCpbsPara(TAF_UINT8 indexNum)
{
    SI_PB_StorateTypeUint32 storage = SI_PB_STORAGE_UNSPECIFIED;
    TAF_UINT32              i;

    if (AT_CheckCpbsPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < g_atParaList[0].paraLen; i++) {
        g_atParaList[0].para[i] = At_UpChar(g_atParaList[0].para[i]);
    }

    if (VOS_StrCmp((TAF_CHAR *)g_atStringTab[AT_STRING_ON].text, (TAF_CHAR *)g_atParaList[0].para) == 0) {
        storage = SI_PB_STORAGE_ON;
    } else if (VOS_StrCmp((TAF_CHAR *)g_atStringTab[AT_STRING_SM].text, (TAF_CHAR *)g_atParaList[0].para) == 0) {
        storage = SI_PB_STORAGE_SM;
    } else if (VOS_StrCmp((TAF_CHAR *)g_atStringTab[AT_STRING_FD].text, (TAF_CHAR *)g_atParaList[0].para) == 0) {
        storage = SI_PB_STORAGE_FD;
    } else if (VOS_StrCmp((TAF_CHAR *)g_atStringTab[AT_STRING_EN].text, (TAF_CHAR *)g_atParaList[0].para) == 0) {
        storage = SI_PB_STORAGE_EC;
    } else if (VOS_StrCmp((TAF_CHAR *)g_atStringTab[AT_STRING_BD].text, (TAF_CHAR *)g_atParaList[0].para) == 0) {
        storage = SI_PB_STORAGE_BD;
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PB_Set(g_atClientTab[indexNum].clientId, 0, storage) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBS_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpbrPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CPBR_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[1].paraLen == 0) {
        g_atParaList[1].paraValue = g_atParaList[0].paraValue;
    }

    if (AT_CheckHsUartUser(indexNum) == VOS_TRUE) {
        AT_GetCommPbCtxAddr()->currIdx       = (VOS_UINT16)g_atParaList[0].paraValue;
        AT_GetCommPbCtxAddr()->lastIdx       = (VOS_UINT16)g_atParaList[1].paraValue;
        AT_GetCommPbCtxAddr()->singleReadFlg = (g_atParaList[0].paraValue == g_atParaList[1].paraValue) ? VOS_TRUE :
                                                                                                          VOS_FALSE;

        result = SI_PB_Read(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED,
                            AT_GetCommPbCtxAddr()->currIdx, AT_GetCommPbCtxAddr()->currIdx);
    } else {
        result = SI_PB_Read(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED,
                            (TAF_UINT16)g_atParaList[0].paraValue, (TAF_UINT16)g_atParaList[1].paraValue);
    }

    if (result == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBR_SET;

        g_pbPrintTag = TAF_FALSE;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ExecuteAddModifyCpbw(SI_PB_Record *records, TAF_UINT8 indexNum)
{
    TAF_UINT32   resultTemp;
    TAF_UINT32   result;

    if (g_atParaList[0].paraLen == 0) { /* 添加 */
        if (SI_PB_Add(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }

        return AT_ERROR;
    }

    result     = SI_PB_GetSPBFlag();
    resultTemp = SI_PB_GetStorateType();

    if ((result == 1) && (resultTemp == SI_PB_STORAGE_SM)) {
        if (SI_PB_SModify(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }
    }

    if (SI_PB_Modify(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_CME_INCORRECT_PARAMETERS;
}

TAF_UINT32 At_PbDeleteProc(TAF_UINT8 indexNum)
{
    /* 针对AT+CPBW=,和AT+CPBW=0的情况 */
    if (g_atParaList[0].paraLen == 0 || g_atParaList[0].paraValue == 0) {
        return AT_CME_INVALID_INDEX;
    }

    if (SI_PB_Delete(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED,
                     (TAF_UINT16)g_atParaList[0].paraValue) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_ERR_LOG("At_PbDeleteProc error: delete pb record failed");

    return AT_ERROR;
}

SI_PB_AlphaTagTypeUint32 AT_SetCpbwAlphaTagType(VOS_VOID)
{
    SI_PB_AlphaTagTypeUint32 alphaTagType;
    if ((g_atParaList[AT_CPBW_CODING].paraValue == AT_CSCS_UCS2_CODE) &&
        (g_atParaIndex == AT_CPBW_PRIVATA_PARA_MAX_NUM)) {
        alphaTagType = AT_CSCS_UCS2_CODE;
    } else if ((g_atParaList[AT_CPBW_CODING].paraValue == AT_CSCS_IRA_CODE) &&
               (g_atParaIndex == AT_CPBW_PRIVATA_PARA_MAX_NUM)) {
        alphaTagType = AT_CSCS_IRA_CODE;
    } else if (g_atParaIndex == AT_CPBW_NLENGTH) {
        alphaTagType = AT_CSCS_IRA_CODE;
    } else {
        alphaTagType = AT_CSCS_IRA_CODE;
    }
    return alphaTagType;
}

VOS_UINT32 AT_CheckCpbwPara(TAF_UINT8 indexNum)
{
    TAF_UINT32   result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) { /* 参数检查 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_CPBW_PRIVATA_PARA_MAX_NUM) { /* 参数过多 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 只有一个参数时为删除操作 */
    if (g_atParaIndex == 1) {
        result = At_PbDeleteProc(indexNum);

        return result;
    }

    if ((g_atParaList[AT_CPBW_NUMBER].paraLen == 0) && (g_atParaList[AT_CPBW_TEXT].paraLen == 0)) {
        return AT_ERROR;
    }

    return AT_OK;
}

/*
 * Description: 检查字符是否是有效的PB字符
 * History:
 *  1.Date: 2020-08-11
 *    Modification: Created function
 */
VOS_BOOL AT_IsValidPBCharacter(VOS_UINT8 data)
{
    if (((data >= '0') && (data <= '9')) || (data == '*') || (data == '#') || (data == '?') || (data == ',') ||
          (data == 'P') || (data == 'p')) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

TAF_UINT32 At_CheckPBString(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT16 count      = 0;
    TAF_UINT8 *tmp        = data;
    TAF_UINT8  invalidNum = 0;
    errno_t    returnValue;

    while (count++ < *len) {
        if ((*tmp == '(') || (*tmp == ')') || (*tmp == '-') || (*tmp == '"')) {
            if (count < *len) {
                returnValue = memmove_s(tmp, (VOS_SIZE_T)(*len - count), tmp + 1, (VOS_SIZE_T)(*len - count));
                TAF_MEM_CHK_RTN_VAL(returnValue, (VOS_SIZE_T)(*len - count), (VOS_SIZE_T)(*len - count));
            }
            invalidNum++;
        } else {
            tmp++;
        }
    }

    count = 0;
    tmp   = data;
    *len -= invalidNum;

    while (count++ < *len) {
        if (AT_IsValidPBCharacter(*tmp) != VOS_TRUE) {
            return AT_FAILURE;
        }
        tmp++;
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCpbwParaNumber(SI_PB_Record *records)
{
    TAF_UINT8   *number = VOS_NULL_PTR;
    TAF_UINT16   len;
    errno_t      memResult;

    if (g_atParaList[AT_CPBW_NUMBER].paraLen != 0) { /* 设置<number> */
        number = g_atParaList[AT_CPBW_NUMBER].para;
        len    = g_atParaList[AT_CPBW_NUMBER].paraLen;

        if (g_atParaList[AT_CPBW_NUMBER].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[AT_CPBW_NUMBER].para + 1;
            len    = g_atParaList[AT_CPBW_NUMBER].paraLen - 1;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        if (len > SI_PB_PHONENUM_MAX_LEN) {
            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->number, sizeof(records->number), number, len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->number), len);
        }

        records->numberLength = (TAF_UINT8)len;
    }

    return AT_OK;
}

VOS_VOID AT_SetCpbwParaType(SI_PB_Record *records)
{
    if (g_atParaList[AT_CPBW_TYPE].paraLen != 0) { /* 设置<type> */
        /* 号码类型待修改 */
        records->numberType = (At_GetCodeType(g_atParaList[AT_CPBW_NUMBER].para[0]) == PB_NUMBER_TYPE_INTERNATIONAL) ?
                                 PB_NUMBER_TYPE_INTERNATIONAL :
                                 (TAF_UINT8)g_atParaList[AT_CPBW_TYPE].paraValue;
    } else {
        records->numberType = (TAF_UINT8)At_GetCodeType(g_atParaList[AT_CPBW_NUMBER].para[0]);
    }
}

TAF_UINT32 At_CheckNameUCS2Code(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT8  alphType = *data;
    TAF_UINT16 alphLen  = *(data + 1);

    if ((alphType == SI_PB_ALPHATAG_TYPE_UCS2_80) && (((*len - 1) % 2) == 0)) {
        /* 80编码，除了标记字节其余长度必须为偶数字节 */
        return AT_SUCCESS;
    }

    /* 3表示81编码格式中81标记字节，表示整个字符串字符个数，以及基址所占字节数，其中一个字节为基址 */
    if ((alphType == SI_PB_ALPHATAG_TYPE_UCS2_81) && (*len >= (alphLen + 3))) {
        /* 81编码，实际字符长度必须大于码流中长度字节的值 */
        *len = alphLen + 3;
        return AT_SUCCESS;
    }

    /* 4表示82编码格式中82标记字节，表示整个字符串字符个数，以及基址所占字节数，其中两个字节为基址 */
    if ((alphType == SI_PB_ALPHATAG_TYPE_UCS2_82) && (*len >= (alphLen + 4))) {
        /* 82编码，同上，区别是基址是2个字节 */
        *len = alphLen + 4;
        return AT_SUCCESS;
    }

    return AT_FAILURE;
}

VOS_UINT32 AT_SetCpbwParaText(SI_PB_Record *records)
{
    TAF_UINT16   alphaTagLength;
    errno_t      memResult;

    if (g_atParaList[AT_CPBW_TEXT].paraLen != 0) { /* 设置<text> */
        /* 如果输入的AT命令已经指示是UNICODE编码要进行编码转换成ASCII码 */
        alphaTagLength = g_atParaList[AT_CPBW_TEXT].paraLen;

        if (records->alphaTagType == AT_CSCS_UCS2_CODE) {
            /* 当ASCII位数必须是UNICODE的偶数倍 */
            if ((alphaTagLength % 2) != 0) {
                return AT_ERROR;
            }

            if (At_UnicodePrint2Unicode(g_atParaList[AT_CPBW_TEXT].para, &alphaTagLength) == AT_FAILURE) {
                return AT_ERROR;
            }

            if (At_CheckNameUCS2Code(g_atParaList[AT_CPBW_TEXT].para, &alphaTagLength) == AT_FAILURE) {
                return AT_ERROR;
            }

            records->alphaTagType = g_atParaList[AT_CPBW_TEXT].para[0];
        }

        /*
         * 最终得到的姓名字段的长度不能超过TAF_PB_RECORD_STRU
         * 结构中的最大长度，否则会导致堆栈溢出
         */
        if (alphaTagLength > SI_PB_ALPHATAG_MAX_LEN) {
            alphaTagLength = SI_PB_ALPHATAG_MAX_LEN;
        }

        if (alphaTagLength > 0) {
            memResult = memcpy_s(records->alphaTag, sizeof(records->alphaTag), g_atParaList[AT_CPBW_TEXT].para,
                                 alphaTagLength);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->alphaTag), alphaTagLength);
        }

        records->alphaTagLength = (TAF_UINT8)alphaTagLength;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCpbwPara(TAF_UINT8 indexNum)
{
    SI_PB_Record records;
    TAF_UINT32   result;

    result = AT_CheckCpbwPara(indexNum);
    if (result != AT_OK) {
        return result;
    }

    /* 确认编码类型，UNICODE或是ASCII码 */
    (VOS_VOID)memset_s(&records, sizeof(records), 0x00, sizeof(records)); /* 初始化 */

    records.alphaTagType = AT_SetCpbwAlphaTagType();

    if (g_atParaList[0].paraLen != 0) { /* 设置<index> */
        records.index = (TAF_UINT16)g_atParaList[0].paraValue;
    }

    result = AT_SetCpbwParaNumber(&records);
    if (result != AT_OK) {
        return result;
    }

    AT_SetCpbwParaType(&records);

    if (records.numberType < AT_CPBW_TEXT_MIN_NUM) {
        return AT_CME_INVALID_INDEX;
    }

    result = AT_SetCpbwParaText(&records);
    if (result != AT_OK) {
        return result;
    }

    return AT_ExecuteAddModifyCpbw(&records, indexNum);
}

TAF_UINT32 At_SetScpbwNum1Type1(SI_PB_Record *records)
{
    TAF_UINT8 *number = VOS_NULL_PTR;
    errno_t    memResult;
    TAF_UINT16 len;

    if (g_atParaList[AT_SCPBW_NUM1].paraLen != 0) { /* 设置<num1> */
        if (g_atParaList[AT_SCPBW_NUM1].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[AT_SCPBW_NUM1].para + 1;
            len    = g_atParaList[AT_SCPBW_NUM1].paraLen - 1;
        } else {
            number = g_atParaList[AT_SCPBW_NUM1].para;
            len    = g_atParaList[AT_SCPBW_NUM1].paraLen;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            AT_ERR_LOG("At_SetScpbwNum1Type1: invalid characters in dial string!");

            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        /* 长度保护 */
        if (len > SI_PB_PHONENUM_MAX_LEN) {
            AT_ERR_LOG("At_SetScpbwNum1Type1: text string too long!");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->number, sizeof(records->number), number, len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->number), len);
        }

        records->numberLength = (TAF_UINT8)len;
    }

    /* 当号码为空时，类型字段按协议置为0xFF */
    if (g_atParaList[AT_SCPBW_NUM1].paraLen == 0) {
        records->numberType   = 0xFF;
        records->numberLength = 0;
    } else {
        if (g_atParaList[AT_SCPBW_TYPE1].paraLen != 0) { /* 设置<type1> */
            /* 号码类型待修改 */
            records->numberType = (At_GetCodeType(g_atParaList[AT_SCPBW_NUM1].para[0]) == PB_NUMBER_TYPE_INTERNATIONAL)
                                   ? PB_NUMBER_TYPE_INTERNATIONAL :
                                   (TAF_UINT8)g_atParaList[AT_SCPBW_TYPE1].paraValue;
        } else {
            records->numberType = (TAF_UINT8)At_GetCodeType(g_atParaList[AT_SCPBW_NUM1].para[0]);
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetScpbwNum2Type2(SI_PB_Record *records)
{
    TAF_UINT8 *number = VOS_NULL_PTR;
    errno_t    memResult;
    TAF_UINT16 len;

    if (g_atParaList[AT_SCPBW_NUM2].paraLen != 0) { /* 设置<num2> */
        if (g_atParaList[AT_SCPBW_NUM2].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[AT_SCPBW_NUM2].para + 1;
            len    = g_atParaList[AT_SCPBW_NUM2].paraLen - 1;
        } else {
            number = g_atParaList[AT_SCPBW_NUM2].para;
            len    = g_atParaList[AT_SCPBW_NUM2].paraLen;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            AT_ERR_LOG("At_SetScpbwNum2Type2: invalid characters in dial string!");

            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        /* 长度保护 */
        if (len > (SI_PB_PHONENUM_MAX_LEN + 2)) { /* +2是为了防止写越界 */
            AT_ERR_LOG("At_SetScpbwNum2Type2: text string too long!");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->additionNumber[0].number, sizeof(records->additionNumber[0].number), number,
                                 len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->additionNumber[0].number), len);
        }

        records->additionNumber[0].numberLength = (TAF_UINT8)len;
    }

    if (g_atParaList[AT_SCPBW_NUM2].paraLen == 0) {
        records->additionNumber[0].numberType   = 0xFF;
        records->additionNumber[0].numberLength = 0;
    } else {
        if (g_atParaList[AT_SCPBW_TYPE2].paraLen != 0) { /* 设置<type2> */
            /* 号码类型待修改 */
            records->additionNumber[0].numberType = (At_GetCodeType(g_atParaList[AT_SCPBW_NUM2].para[0]) ==
                                                     PB_NUMBER_TYPE_INTERNATIONAL) ?
                                                     PB_NUMBER_TYPE_INTERNATIONAL :
                                                     (TAF_UINT8)g_atParaList[AT_SCPBW_TYPE2].paraValue;
        } else {
            records->additionNumber[0].numberType = (TAF_UINT8)At_GetCodeType(g_atParaList[AT_SCPBW_NUM2].para[0]);
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetScpbwNum3Type3(SI_PB_Record *records)
{
    TAF_UINT8 *number = VOS_NULL_PTR;
    errno_t    memResult;
    TAF_UINT16 len;

    if (g_atParaList[AT_SCPBW_NUM3].paraLen != 0) { /* 设置<num3> */
        if (g_atParaList[AT_SCPBW_NUM3].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[AT_SCPBW_NUM3].para + 1;
            len    = g_atParaList[AT_SCPBW_NUM3].paraLen - 1;
        } else {
            number = g_atParaList[AT_SCPBW_NUM3].para;
            len    = g_atParaList[AT_SCPBW_NUM3].paraLen;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            AT_ERR_LOG("At_SetScpbwNum3Type3: invalid characters in dial string!");

            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        /* 长度保护 */
        if (len > (SI_PB_PHONENUM_MAX_LEN + 2)) { /* +2是为了防止写越界 */
            AT_ERR_LOG("At_SetScpbwNum3Type3: text string too long!");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->additionNumber[1].number, sizeof(records->additionNumber[1].number), number,
                                 len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->additionNumber[1].number), len);
        }

        records->additionNumber[1].numberLength = (TAF_UINT8)len;
    }

    if (g_atParaList[AT_SCPBW_NUM3].paraLen == 0) {
        records->additionNumber[1].numberType   = 0xFF;
        records->additionNumber[1].numberLength = 0;
    } else {
        if (g_atParaList[AT_SCPBW_TYPE3].paraLen != 0) { /* 设置<type3> */
            /* 号码类型待修改 */
            records->additionNumber[1].numberType = (At_GetCodeType(g_atParaList[AT_SCPBW_NUM3].para[0]) ==
                                                     PB_NUMBER_TYPE_INTERNATIONAL) ?
                                                     PB_NUMBER_TYPE_INTERNATIONAL :
                                                     (TAF_UINT8)g_atParaList[AT_SCPBW_TYPE3].paraValue;
        } else {
            records->additionNumber[1].numberType = (TAF_UINT8)At_GetCodeType(g_atParaList[AT_SCPBW_NUM3].para[0]);
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetScpbwNum4Type4(SI_PB_Record *records)
{
    TAF_UINT8 *number = VOS_NULL_PTR;
    errno_t    memResult;
    TAF_UINT16 len;
    if (g_atParaList[AT_SCPBW_NUM4].paraLen != 0) { /* 设置<num4> */
        if (g_atParaList[AT_SCPBW_NUM4].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[AT_SCPBW_NUM4].para + 1;
            len    = g_atParaList[AT_SCPBW_NUM4].paraLen - 1;
        } else {
            number = g_atParaList[AT_SCPBW_NUM4].para;
            len    = g_atParaList[AT_SCPBW_NUM4].paraLen;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            AT_ERR_LOG("At_SetScpbwNum4Type4: invalid characters in dial string!");

            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        /* 长度保护 */
        if (len > (SI_PB_PHONENUM_MAX_LEN + 2)) { /* +2是为了防止写越界 */
            AT_ERR_LOG("At_SetScpbwNum4Type4: text string too long!");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].number,
                                 sizeof(records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].number), number, len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].number), len);
        }

        records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].numberLength = (TAF_UINT8)len;
    }

    if (g_atParaList[AT_SCPBW_NUM4].paraLen == 0) {
        records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].numberType   = 0xFF;
        records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].numberLength = 0;
    } else {
        if (g_atParaList[AT_SCPBW_TYPE4].paraLen != 0) { /* 设置<type4> */
            /* 号码类型待修改 */
            records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].numberType =
                (At_GetCodeType(g_atParaList[AT_SCPBW_NUM4].para[AT_SCPBW_NUM4_NUM_TYPE]) ==
                PB_NUMBER_TYPE_INTERNATIONAL) ?
                PB_NUMBER_TYPE_INTERNATIONAL :
                (TAF_UINT8)g_atParaList[AT_SCPBW_TYPE4].paraValue;
        } else {
            records->additionNumber[AT_ADDITION_NUM_ARRAY_INDEX_2].numberType =
                (TAF_UINT8)At_GetCodeType(g_atParaList[AT_SCPBW_NUM4].para[AT_SCPBW_NUM4_NUM_TYPE]);
        }
    }

    return AT_SUCCESS;
}

VOS_VOID At_SetScpbwAlphaTagType(SI_PB_Record *records)
{
    if (g_atParaList[AT_SCPBW_ALPHATAG_TYPE].paraValue == AT_CSCS_UCS2_CODE) {
        records->alphaTagType = AT_CSCS_UCS2_CODE;
    } else if (g_atParaList[AT_SCPBW_ALPHATAG_TYPE].paraValue == AT_CSCS_IRA_CODE) {
        records->alphaTagType = AT_CSCS_IRA_CODE;
    } else {
        records->alphaTagType = AT_CSCS_IRA_CODE;
    }
}

TAF_UINT32 At_SetScpbwText(SI_PB_Record *records)
{
    errno_t    memResult;
    TAF_UINT16 alphaTagLength;

    if (g_atParaList[AT_SCPBW_TEXT].paraLen != 0) { /* 设置<text> */
        /* 如果输入的AT命令已经指示是UNICODE编码要进行编码转换成ASCII码 */
        alphaTagLength = g_atParaList[AT_SCPBW_TEXT].paraLen;

        if (records->alphaTagType == AT_CSCS_UCS2_CODE) {
            /* 当ASCII位数必须是UNICODE的偶数倍 */
            if ((alphaTagLength % 2) != 0) {
                return AT_ERROR;
            }

            if (At_UnicodePrint2Unicode(g_atParaList[AT_SCPBW_TEXT].para, &alphaTagLength) == AT_FAILURE) {
                return AT_ERROR;
            }

            if (At_CheckNameUCS2Code(g_atParaList[AT_SCPBW_TEXT].para, &alphaTagLength) == AT_FAILURE) {
                return AT_ERROR;
            }

            records->alphaTagType = g_atParaList[AT_SCPBW_TEXT].para[0];
        }

        /*
         * 最终得到的姓名字段的长度不能超过TAF_PB_RECORD_STRU
         * 结构中的最大长度，否则会导致堆栈溢出
         */
        if (alphaTagLength > SI_PB_ALPHATAG_MAX_LEN) {
            AT_ERR_LOG("At_SetScpbwText: cme incorrect parameters!");

            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (alphaTagLength > 0) {
            memResult = memcpy_s(records->alphaTag, sizeof(records->alphaTag), g_atParaList[AT_SCPBW_TEXT].para,
                                 alphaTagLength);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->alphaTag), alphaTagLength);
        }

        records->alphaTagLength = (TAF_UINT8)alphaTagLength;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_PbEmailTransferToGsm(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst, VOS_UINT32 dstBufLen,
                                   VOS_UINT32 *dstLen)
{
    TAF_UINT16 gsmCodeLen = 0;
    TAF_UINT8  emailUCS2[2 * SI_PB_EMAIL_MAX_LEN];
    TAF_UINT16 uCS2Len;

    (VOS_VOID)memset_s(emailUCS2, sizeof(emailUCS2), 0x00, sizeof(emailUCS2));

    /* 先将输入码流转换为UCS2再做处理 */
    At_PbIraToUnicode(src, srcLen, emailUCS2, &uCS2Len);

    /* 判断UCS2能否转换为GSM7bit存储 */
    if (At_UnicodeTransferToGsmCalcLength(emailUCS2, uCS2Len, &gsmCodeLen) != AT_SUCCESS) {
        AT_ERR_LOG("At_PbEmailTransferToGsm error: failed to transfer to Gsm");

        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* EMAIL的长度大于电话本EMAIL字段能存储的长度时，提醒用户输入太长 */
    if (gsmCodeLen > g_pbatInfo.emailLen) {
        AT_LOG1("At_PbEmailTransferToGsm error: mail length [%d] too long", gsmCodeLen);

        return AT_CME_TEXT_STRING_TOO_LONG;
    }

    *dstLen = gsmCodeLen;

    /* 若能转换为GSM7bit则进行转换 */
    At_UnicodeTransferToGsm(emailUCS2, uCS2Len, dst, dstBufLen);

    return AT_SUCCESS;
}

TAF_UINT32 At_SetScpbwEmail(SI_PB_Record *records)
{
    TAF_UINT32 result;

    if (g_atParaList[AT_SCPBW_EMAIL].paraLen != 0) { /* 设置<email> */
        if (g_atParaList[AT_SCPBW_EMAIL].paraLen > SI_PB_EMAIL_MAX_LEN) {
            AT_ERR_LOG("At_SetScpbwEmail: cme text string too long!");

            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        result = At_PbEmailTransferToGsm(g_atParaList[AT_SCPBW_EMAIL].para, g_atParaList[AT_SCPBW_EMAIL].paraLen,
                                         records->email.email, sizeof(records->email.email), &records->email.emailLen);

        if (result != AT_SUCCESS) {
            return result;
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_CheckScpbwPara(TAF_UINT8 indexNum)
{
    TAF_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) { /* 参数检查 */
        AT_ERR_LOG("At_CheckScpbwPara: cmd opt type error");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_SCPBW_PARA_MAX_NUM) { /* 参数过多 */
        AT_ERR_LOG("At_CheckScpbwPara: para num too many");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 只有一个参数时为删除操作 */
    if (g_atParaIndex == 1) {
        result = At_PbDeleteProc(indexNum);

        return result;
    }

    if ((g_atParaList[AT_SCPBW_NUM1].paraLen == 0) && (g_atParaList[AT_SCPBW_NUM2].paraLen == 0) &&
        (g_atParaList[AT_SCPBW_NUM3].paraLen == 0) && (g_atParaList[AT_SCPBW_NUM4].paraLen == 0) &&
        (g_atParaList[AT_SCPBW_TEXT].paraLen == 0) && (g_atParaList[AT_SCPBW_EMAIL].paraLen == 0)) {
        AT_ERR_LOG("At_CheckScpbwPara: para len error");

        return AT_ERROR;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetScpbwPara(TAF_UINT8 indexNum)
{
    SI_PB_Record records;
    TAF_UINT32   rslt;

    rslt = At_CheckScpbwPara(indexNum);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    /* 确认编码类型，UNICODE或是ASCII码 */
    (VOS_VOID)memset_s(&records, sizeof(records), 0x00, sizeof(records)); /* 初始化 */

    /* 设置<index> */
    if (g_atParaList[0].paraLen != 0) {
        records.index = (TAF_UINT16)g_atParaList[0].paraValue;
    }

    rslt = At_SetScpbwNum1Type1(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    rslt = At_SetScpbwNum2Type2(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    rslt = At_SetScpbwNum3Type3(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    rslt = At_SetScpbwNum4Type4(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    /* 设置ucAlphaTagType */
    At_SetScpbwAlphaTagType(&records);

    /* 设置text */
    rslt = At_SetScpbwText(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    /* 设置Email */
    rslt = At_SetScpbwEmail(&records);

    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    if (g_atParaList[0].paraLen == 0) { /* 添加 */
        if (SI_PB_SAdd(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, &records) == AT_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }

        return AT_ERROR;
    }

    /* 修改 */
    if (SI_PB_SModify(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, &records) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

/*
 * Description: ^SCPBR
 * History:
 *  1.Date: 2009-05-23
 *    Modification: Created
 */
TAF_UINT32 At_SetScpbrPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_SCPBR_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[1].paraLen == 0) {
        g_atParaList[1].paraValue = g_atParaList[0].paraValue;
    }

    if (SI_PB_SRead(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED,
                    (TAF_UINT16)g_atParaList[0].paraValue, (TAF_UINT16)g_atParaList[1].paraValue) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SCPBR_SET;

        g_pbPrintTag = TAF_FALSE;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpbr2Para(TAF_UINT8 indexNum)
{
    AT_CommPbCtx *commPbCntxt = VOS_NULL_PTR;
    VOS_UINT32    result;
    VOS_UINT16    index1;
    VOS_UINT16    index2;

    commPbCntxt = AT_GetCommPbCtxAddr();

    /* 命令格式检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 命令参数个数检查 */
    if ((g_atParaIndex < AT_CPBR_PARA_MIN_NUM) || (g_atParaIndex > AT_CPBR_PARA_MAX_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT+CPBR=, */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<index1> */
    index1 = (VOS_UINT16)g_atParaList[0].paraValue;

    if (g_atParaIndex > AT_CPBR_PARA_MIN_NUM) {
        if (g_atParaList[1].paraLen == 0) {
            /* AT+CPBR=<index1>, */
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            /* AT+CPBR=<index1>,<index2> */
            index2 = (VOS_UINT16)g_atParaList[1].paraValue;
        }
    } else {
        /* AT+CPBR=<index1> */
        index2 = index1;
    }

    /* HSUART端口受限发送瓶颈, 需要逐条发送 */
    if (AT_IsHsuartBaseUser(indexNum) == VOS_TRUE) {
        commPbCntxt->currIdx       = index1;
        commPbCntxt->lastIdx       = index2;
        commPbCntxt->singleReadFlg = (index1 == index2) ? VOS_TRUE : VOS_FALSE;

        result = SI_PB_Read(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, commPbCntxt->currIdx,
                            commPbCntxt->currIdx);
    } else {
        result = SI_PB_Read(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, index1, index2);
    }

    if (result == TAF_SUCCESS) {
        g_pbPrintTag                          = TAF_FALSE;
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBR2_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_CheckCpbwPara2(TAF_UINT8 indexNum)
{
    TAF_UINT32   result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) { /* 参数检查 */
        AT_ERR_LOG("AT_CheckCpbwPara2 error:incorect parameters");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_CPBW_PARA_MAX_NUM) { /* 参数过多 */
        AT_ERR_LOG("AT_CheckCpbwPara2 error: too many parameters");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 只有一个参数时为删除操作 */
    if (g_atParaIndex == 1) {
        result = At_PbDeleteProc(indexNum);

        return result;
    }

    if ((g_atParaList[AT_CPBW_NUMBER].paraLen == 0) && (g_atParaList[AT_CPBW_TEXT].paraLen == 0)) {
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCpbwPara2Number(SI_PB_Record *records)
{
    TAF_UINT8   *number = VOS_NULL_PTR;
    TAF_UINT16   len;
    TAF_UINT32   result;
    errno_t      memResult;

    if (g_atParaList[1].paraLen != 0) { /* 设置<number> */
        number = g_atParaList[1].para;
        len    = g_atParaList[1].paraLen;
        if (g_atParaList[1].para[0] == '+') { /* 判断号码地址类型 */
            number = g_atParaList[1].para + 1;
            len    = g_atParaList[1].paraLen - 1;
        }

        if (At_CheckPBString(number, &len) == AT_FAILURE) {
            AT_ERR_LOG("AT_SetCpbwPara2Number error: invalid characters in dial string");

            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }

        /* 长度保护 */
        if (len > SI_PB_PHONENUM_MAX_LEN) {
            return AT_CME_TEXT_STRING_TOO_LONG;
        }

        if (len > 0) {
            memResult = memcpy_s(records->number, sizeof(records->number), number, len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(records->number), len);
        }

        records->numberLength = (TAF_UINT8)len;
    }

    result = At_PbAlphaTagProc(records, &g_atParaList[AT_CPBW_TEXT]);

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetCpbwPara2Number:something wrong in At_PbAlphaTagProc");
        return result;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCpbwPara2NumberType(SI_PB_Record *records)
{
    if (g_atParaList[AT_CPBW_TYPE].paraLen != 0) { /* 设置<type> */
        /* 号码类型待修改 */
        records->numberType = (At_GetCodeType(g_atParaList[1].para[0]) == PB_NUMBER_TYPE_INTERNATIONAL) ?
                              PB_NUMBER_TYPE_INTERNATIONAL :
                              (TAF_UINT8)g_atParaList[AT_CPBW_TYPE].paraValue;
    } else {
        records->numberType = (TAF_UINT8)At_GetCodeType(g_atParaList[1].para[0]);
    }

    if (records->numberType < AT_CPBW_TEXT_MIN_NUM) {
        AT_ERR_LOG("AT_SetCpbwPara2NumberType error: invalid index");

        return AT_CME_INVALID_INDEX;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCpbwPara2AddRecord(SI_PB_Record *records, TAF_UINT8 indexNum)
{
    if (g_atParaList[0].paraLen == 0) {
        /* 添加记录 */
        if (SI_PB_Add(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }

        AT_ERR_LOG("AT_SetCpbwPara2AddRecord error: add pb record failed");

        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCpbwPara2modifyRecord(SI_PB_Record *records, TAF_UINT8 indexNum)
{
    TAF_UINT32   result;
    TAF_UINT32   resultTemp;

    result     = SI_PB_GetSPBFlag();
    resultTemp = SI_PB_GetStorateType();

    if ((result == 1) && (resultTemp == SI_PB_STORAGE_SM)) {
        if (SI_PB_SModify(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }
    }

    if (SI_PB_Modify(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_UNSPECIFIED, records) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBW_SET; /* 设置当前操作类型 */

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_ERR_LOG("AT_SetCpbwPara2modifyRecord error: modify pb record failed");

    return AT_ERROR;
}

TAF_UINT32 At_SetCpbwPara2(TAF_UINT8 indexNum)
{
    SI_PB_Record records;
    TAF_UINT32   result;

    result = AT_CheckCpbwPara2(indexNum);
    if (result != AT_SUCCESS) {
        return result;
    }

    (VOS_VOID)memset_s(&records, sizeof(records), 0x00, sizeof(records)); /* 初始化 */

    if (g_atParaList[0].paraLen != 0) { /* 设置<index> */
        records.index = (TAF_UINT16)g_atParaList[0].paraValue;
    }

    result = AT_SetCpbwPara2Number(&records);
    if (result != AT_SUCCESS) {
        return result;
    }

    result = AT_SetCpbwPara2NumberType(&records);
    if (result != AT_SUCCESS) {
        return result;
    }

    result = AT_SetCpbwPara2AddRecord(&records, indexNum);
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 修改记录 */
    return AT_SetCpbwPara2modifyRecord(&records, indexNum);
}

/*
 * Description: +CNUM
 * History:
 *  1.Date: 2007-06-11
 *    Modification: Created function
 */
TAF_UINT32 At_SetCnumPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PB_Read(g_atClientTab[indexNum].clientId, 0, SI_PB_STORAGE_ON, 0, 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNUM_READ;

        g_pbPrintTag = TAF_FALSE;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpbfPara(TAF_UINT8 indexNum)
{
    SI_PB_StorateTypeUint32 storage = SI_PB_STORAGE_UNSPECIFIED;
    SI_PB_Record            records;
    TAF_UINT32              result;

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = At_PbAlphaTagProc(&records, &g_atParaList[0]);

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCpbfPara:something wrong in At_PbAlphaTagProc");
        return result;
    }

    /* 执行命令操作 */
    if (SI_PB_Search(g_atClientTab[indexNum].clientId, 0, storage, records.alphaTagLength, records.alphaTag) ==
        AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPBF_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_BOOL AT_CheckCrsmEfid(TAF_UINT32 efId, TAF_UINT16 *path, VOS_UINT32 maxPathLen, TAF_UINT16 *len)
{
    if ((efId & 0xFF00) == EFIDUNDERMF) {
        *len = 1;
    }
    /* 4F文件要在5F下，路径长度为3 */
    else if ((efId & 0xFF00) == EFIDUNDERMFDFDF) {
        if (((*len) != AT_EFID_4F_PATH_LEN) || ((path[AT_PATH_INDEX_1] & 0xFF00) != DFIDUNDERMF) ||
            ((path[AT_PATH_INDEX_2] & 0xFF00) != DFIDUNDERMFDF)) {
            return VOS_FALSE;
        }
    }
    /* 6F文件要在7F下，路径长度为2 */
    else if ((efId & 0xFF00) == EFIDUNDERMFDF) {
        if (((*len) != AT_EFID_6F_PATH_LEN) || ((path[AT_PATH_INDEX_1] & 0xFF00) != DFIDUNDERMF)) {
            return VOS_FALSE;
        }
    } else {
    }

    return VOS_TRUE;
}

TAF_UINT32 At_CrsmFilePathCheck(TAF_UINT32 efId, TAF_UINT8 *filePath, TAF_UINT16 *pathLen)
{
    errno_t    memResult;
    TAF_UINT16 len;
    TAF_UINT16 path[USIMM_MAX_PATH_LEN]    = {0};
    TAF_UINT16 tmpPath[USIMM_MAX_PATH_LEN] = {0};
    TAF_UINT16 pathLenTemp;
    TAF_UINT16 i;

    pathLenTemp = *pathLen;
    len         = 0;

    if (pathLenTemp > USIMM_MAX_PATH_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < (pathLenTemp / sizeof(TAF_UINT16)); i++) {
        /* 将UINT8存入UINT16时，高8位存偶数位，低8位存奇数位 */
        tmpPath[i] = ((filePath[i * 2] << 0x08) & 0xFF00) + filePath[(i * 2) + 1];
    }

    /* 如果路径不是以3F00开始，需要添加3F00作开头 */
    if (tmpPath[0] != MFID) {
        if (pathLenTemp == USIMM_MAX_PATH_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        path[0] = MFID;

        len++;
    }

    if (pathLenTemp > 0) {
        memResult = memcpy_s(&path[len], (USIMM_MAX_PATH_LEN - len) * ((VOS_SIZE_T)sizeof(TAF_UINT16)), tmpPath,
                             pathLenTemp);
        TAF_MEM_CHK_RTN_VAL(memResult, (USIMM_MAX_PATH_LEN - len) * ((VOS_SIZE_T)sizeof(TAF_UINT16)), pathLenTemp);
    }

    len += (pathLenTemp / sizeof(TAF_UINT16));

    if (AT_CheckCrsmEfid(efId, path, USIMM_MAX_PATH_LEN, &len) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    *pathLen = len;

    if (len > 0) {
        /* 将UINT8存入UINT16时，长度变为原来的2倍 */
        memResult = memcpy_s(filePath, AT_PARA_MAX_LEN + 1, path, (VOS_SIZE_T)(len * 2));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, (VOS_SIZE_T)(len * 2));
    }

    return AT_SUCCESS;
}

/*
 * Function:       At_CrsmApduParaCheck
 * Description:    执行CRSM命令输入的参数的匹配检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrsmApduParaCheck(VOS_VOID)
{
    TAF_UINT16 fileTag;

    /* 命令类型参数检查，第一个参数不能为空 */
    if (g_atParaList[0].paraLen == 0) {
        AT_ERR_LOG("At_SetCrsmPara: command type null");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 除STATUS命令外，文件ID输入不能为空 */
    if ((g_atParaList[AT_CRSM_FILEID].paraValue == 0) && (g_atParaList[0].paraValue != USIMM_STATUS)) {
        AT_ERR_LOG("At_CrsmApduParaCheck: File Id null.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取文件ID前两位 */
    fileTag = (g_atParaList[AT_CRSM_FILEID].paraValue >> 8) & (0x00FF);

    /* 输入的文件ID必须是EF文件，前两位不可以是3F/5F/7F */
    if ((fileTag == MFLAB) || (fileTag == DFUNDERMFLAB) || (fileTag == DFUNDERDFLAB)) {
        AT_ERR_LOG("At_CrsmApduParaCheck: File Id error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <P1><P2><P3>这三个参数全部为空 */
    if ((g_atParaList[AT_CRSM_P1].paraLen == 0) && (g_atParaList[AT_CRSM_P2].paraLen == 0) &&
        (g_atParaList[AT_CRSM_P3].paraLen == 0)) {
        return AT_SUCCESS;
    }

    /* <P1><P2><P3>这三个参数全部不为空 */
    if ((g_atParaList[AT_CRSM_P1].paraLen != 0) && (g_atParaList[AT_CRSM_P2].paraLen != 0) &&
        (g_atParaList[AT_CRSM_P3].paraLen != 0)) {
        return AT_SUCCESS;
    }

    /* 其它情况下属于输入AT命令参数不完整 */
    return AT_CME_INCORRECT_PARAMETERS;
}

/*
 * Function:       At_CrsmFilePathParse
 * Description:    执行CRSM命令输入的路径进行解析
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrsmFilePathParse(SI_PIH_Crsm *command)
{
    errno_t    memResult;
    TAF_UINT32 result;

    /* 如果词法解析器解析第七个参数为空，说明没有文件路径输入，直接返回成功 */
    if ((g_atParaList[AT_CRSM_PATHIDE].paraLen == 0) && (command->efId != VOS_NULL_WORD)) {
        return AT_SUCCESS;
    }

    /* 在转换前输入的文件路径长度必须是4的整数倍 */
    if ((g_atParaList[AT_CRSM_PATHIDE].paraLen % 4) != 0) {
        AT_ERR_LOG("At_CrsmFilePathParse: Path error");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将输入的字符串转换成十六进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CRSM_PATHIDE].para,
                              &g_atParaList[AT_CRSM_PATHIDE].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrsmFilePathParse: At_AsciiNum2HexString error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果有填写文件ID和路径，要做文件路径检查，输入的路径长度以U16为单位 */
    result = At_CrsmFilePathCheck((TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue,
                                  g_atParaList[AT_CRSM_PATHIDE].para,
                                  &g_atParaList[AT_CRSM_PATHIDE].paraLen);

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_CrsmFilePathParse: At_CrsmFilePathCheck error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充文件路径和长度 */
    command->pathLen = g_atParaList[AT_CRSM_PATHIDE].paraLen;

    /* 文件路径长度是U16为单位的，路径拷贝的长度要乘2 */
    if ((g_atParaList[AT_CRSM_PATHIDE].paraLen > 0) &&
        (((VOS_SIZE_T)(g_atParaList[AT_CRSM_PATHIDE].paraLen * sizeof(VOS_UINT16))) <= sizeof(command->path))) {
        memResult = memcpy_s(command->path, sizeof(command->path), g_atParaList[AT_CRSM_PATHIDE].para,
                             (VOS_SIZE_T)(g_atParaList[AT_CRSM_PATHIDE].paraLen * sizeof(VOS_UINT16)));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->path),
                            (VOS_SIZE_T)(g_atParaList[AT_CRSM_PATHIDE].paraLen * sizeof(VOS_UINT16)));
    }

    return AT_SUCCESS;
}

/*
 * Function:       At_CrsmParaStatusCheck
 * Description:    执行CRSM命令对STATUS命令的参数检查
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrsmParaStatusCheck(SI_PIH_Crsm *command)
{
    /* STATUS命令如果没有输入文件ID，就不需要做选文件操作，直接发STATUS命令 */
    if (g_atParaList[AT_CRSM_FILEID].paraValue == 0) {
        command->efId = VOS_NULL_WORD;
    } else {
        command->efId = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->cmdType = USIMM_STATUS;

    return At_CrsmFilePathParse(command);
}

/*
 * Function:       At_CrsmParaReadBinaryCheck
 * Description:    执行CRSM命令对Read Binary命令的参数检查
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrsmParaReadBinaryCheck(SI_PIH_Crsm *command)
{
    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    command->cmdType = USIMM_READ_BINARY;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrsmFilePathParse(command);
}

/*
 * Function:       At_CrsmParaReadBinaryCheck
 * Description:    执行CRSM命令对Read Record的参数检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrsmParaReadRecordCheck(SI_PIH_Crsm *command)
{
    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    command->cmdType = USIMM_READ_RECORD;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrsmFilePathParse(command);
}

/*
 * Function:       At_CrsmParaGetRspCheck
 * Description:    执行CRSM命令的Get Response命令参数检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
VOS_UINT32 At_CrsmParaGetRspCheck(SI_PIH_Crsm *command)
{
    /* 参数个数不能少于2个，至少要有命令类型和文件ID */
    if (g_atParaIndex < AT_CRSM_PARA_MIN_NUM) {
        AT_ERR_LOG("At_CrsmParaGetRspCheck: Para less than 2.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    command->cmdType = USIMM_GET_RESPONSE;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrsmFilePathParse(command);
}

/*
 * Function:       At_CrsmParaUpdateBinaryCheck
 * Description:    执行CRSM命令的Update Binary参数检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
VOS_UINT32 At_CrsmParaUpdateBinaryCheck(SI_PIH_Crsm *command)
{
    errno_t memResult;

    /* Update Binary命令至少要有6个参数，可以没有文件路径 */
    if (g_atParaIndex < AT_CRSM_UPDATA_BINARY_PARA_MIN_NUM) {
        AT_ERR_LOG("At_CrsmParaUpdateBinaryCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    command->cmdType = USIMM_UPDATE_BINARY;

    /* 第六个参数输入的<data>字符串在转换前数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRSM_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRSM_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrsmParaUpdateBinaryCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRSM_DATA].para, &g_atParaList[AT_CRSM_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrsmParaUpdateBinaryCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 防止因为数据长度过长导致单板复位 */
    if (g_atParaList[AT_CRSM_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrsmParaUpdateBinaryCheck: g_atParaList[5] too long.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    memResult = memcpy_s((TAF_VOID *)command->content, sizeof(command->content),
                         (TAF_VOID *)g_atParaList[AT_CRSM_DATA].para, g_atParaList[AT_CRSM_DATA].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->content), g_atParaList[AT_CRSM_DATA].paraLen);

    return At_CrsmFilePathParse(command);
}

/*
 * Function:       At_CrsmParaUpdateRecordCheck
 * Description:    执行CRSM命令的参数检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
VOS_UINT32 At_CrsmParaUpdateRecordCheck(SI_PIH_Crsm *command)
{
    errno_t memResult;

    /* Update Binary命令至少要有6个参数，可以没有文件路径 */
    if (g_atParaIndex < AT_CRSM_UPDATA_BINARY_PARA_MIN_NUM) {
        AT_ERR_LOG("At_CrsmParaUpdateRecordCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRSM_FILEID].paraValue;
    command->cmdType = USIMM_UPDATE_RECORD;

    /* 第六个参数输入的<data>字符串数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRSM_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRSM_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrsmParaUpdateRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRSM_DATA].para,
                              &g_atParaList[AT_CRSM_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrsmParaUpdateRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 防止因为数据长度过长导致单板复位 */
    if (g_atParaList[AT_CRSM_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrsmParaUpdateRecordCheck: g_atParaList[5] too long.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    memResult = memcpy_s(command->content, sizeof(command->content),
                         g_atParaList[AT_CRSM_DATA].para, g_atParaList[AT_CRSM_DATA].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->content), g_atParaList[AT_CRSM_DATA].paraLen);

    return At_CrsmFilePathParse(command);
}

VOS_UINT32 At_CrsmParaSearchRecordCheck(SI_PIH_Crsm *command)
{
    /* Search Record命令至少要有7个参数 */
    if (g_atParaIndex < AT_CRSM_UPDATA_BINARY_PARA_MIN_NUM) {
        AT_ERR_LOG("At_CrsmParaSearchRecordCheck: Para less than 7.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1 = (TAF_UINT8)g_atParaList[AT_CRSM_P1].paraValue;
    command->p2 = (TAF_UINT8)g_atParaList[AT_CRSM_P2].paraValue;
    command->p3 = (TAF_UINT8)g_atParaList[AT_CRSM_P3].paraValue;

    /* 填写数据结构中的<fileid> */
    command->efId = (TAF_UINT16)g_atParaList[1].paraValue;

    command->cmdType = USIMM_SEARCH_RECORD;

    /* 第六个参数输入的<data>字符串数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRSM_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRSM_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrsmParaSearchRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRSM_DATA].para, &g_atParaList[AT_CRSM_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrsmParaSearchRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 防止因为数据长度过长导致单板复位 */
    if (g_atParaList[AT_CRSM_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrsmParaSearchRecordCheck: g_atParaList[5] too long.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    if (memcpy_s(command->content, sizeof(command->content),
                 g_atParaList[AT_CRSM_DATA].para, g_atParaList[AT_CRSM_DATA].paraLen) != EOK) {
        AT_ERR_LOG("At_CrsmParaSearchRecordCheck: memcpy_s fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    return At_CrsmFilePathParse(command);
}

/*
 * Description: +CRSM=<command>[,<fileid>[,<P1>,<P2>,<P3>[,<data>,<pathid>]]]
 * History:
 *  1.Date: 2008-03-02
 *    Modification: Created function
 *  2.Date: 2012-05-05
 *    Modification: 增加对第七个参数的处理
 */
TAF_UINT32 At_SetCrsmPara(TAF_UINT8 indexNum)
{
    SI_PIH_Crsm command;
    TAF_UINT32  result;

    /* 参数过多 */
    if (g_atParaIndex > AT_CRSM_PARA_MAX_NUM) {
        AT_ERR_LOG("At_SetCrsmPara: too many para");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果有输入<P1><P2><P3>，这三个参数不能只填写部分参数 */
    if (At_CrsmApduParaCheck() != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCrsmPara: At_CrsmApduParaCheck fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&command, sizeof(command), 0x00, sizeof(SI_PIH_Crsm));

    switch (g_atParaList[0].paraValue) {
        case USIMM_STATUS:
            result = At_CrsmParaStatusCheck(&command);
            break;
        case USIMM_READ_BINARY:
            result = At_CrsmParaReadBinaryCheck(&command);
            break;
        case USIMM_READ_RECORD:
            result = At_CrsmParaReadRecordCheck(&command);
            break;
        case USIMM_GET_RESPONSE:
            result = At_CrsmParaGetRspCheck(&command);
            break;
        case USIMM_UPDATE_BINARY:
            result = At_CrsmParaUpdateBinaryCheck(&command);
            break;
        case USIMM_UPDATE_RECORD:
            result = At_CrsmParaUpdateRecordCheck(&command);
            break;
        case USIMM_SEARCH_RECORD:
            result = At_CrsmParaSearchRecordCheck(&command);
            break;
        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCrsmPara: para parse fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PIH_CrsmSetReq(g_atClientTab[indexNum].clientId, 0, &command) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CRSM_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetSimstPara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptSetReq atCmd;
    VOS_UINT32                  result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.reqType       = AT_MTA_SET_SIMST_RPT_TYPE;
    atCmd.u.simstRptFlg = AT_SIMST_ENABLE_TYPE;

    if (g_atParaList[0].paraLen != 0) {
        atCmd.u.simstRptFlg = (VOS_UINT8)g_atParaList[0].paraValue;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetParaRspSimsqPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数只能为一个 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_GetSimsqEnable() != g_atParaList[0].paraValue) {
        At_SetSimsqEnable((VOS_UINT8)g_atParaList[0].paraValue);

        result = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_CUSTOM_USIMM_CFG,
                                    (VOS_UINT8 *)&AT_GetCommCtxAddr()->customUsimmCfg, sizeof(NAS_NVIM_CustomUsimmCfg));

        if (result != NV_OK) {
            AT_WARN_LOG("AT_SetParaRspSimsqPara: write nv failed!");
            return AT_ERROR;
        }
    }

    return AT_OK;
}

TAF_UINT32 At_SetPNNPara(TAF_UINT8 indexNum)
{
    TAF_PH_QryUsimInfo usimInfo;

    (VOS_VOID)memset_s(&usimInfo, sizeof(usimInfo), 0x00, sizeof(usimInfo));

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if (g_atParaList[0].paraValue == 0) {
        usimInfo.icctype = TAF_PH_ICC_SIM;
    } else {
        usimInfo.icctype = TAF_PH_ICC_USIM;
    }
    usimInfo.efId = TAF_PH_PNN_FILE;
    /* linear fixed file only support 254 reocrd */
    if (g_atParaList[1].paraLen != 0) {
        if ((g_atParaList[1].paraValue > 0xFE) || (g_atParaList[1].paraValue == 0)) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            usimInfo.recNum = g_atParaList[1].paraValue;
        }
    } else {
        usimInfo.recNum = 0;

        /* 在读取PNN列表的时候进行分次下发，每次只读取TAF_MMA_PNN_INFO_MAX_NUM个，以防止A核申请内存失败造成复位 */
        /* 第一次下发的时候，Index为0，后面在收到TAF的CNF消息的时候再进行分批次下发 */
        usimInfo.pnnQryIndex.pnnCurrIndex = 0;
        usimInfo.pnnQryIndex.pnnNum       = TAF_MMA_PNN_INFO_MAX_NUM;
    }

    /* 执行命令操作 */
    if (TAF_QryUsimInfo(g_atClientTab[indexNum].clientId, 0, &usimInfo) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PNN_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCPNNPara(TAF_UINT8 indexNum)
{
    TAF_PH_IccType iccType;

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if (g_atParaList[0].paraValue == 0) {
        iccType = TAF_PH_ICC_SIM;
    } else {
        iccType = TAF_PH_ICC_USIM;
    }

    /* 执行命令操作 */
    if (TAF_QryCpnnInfo(g_atClientTab[indexNum].clientId, 0, iccType) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPNN_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

/*
 * *Function: At_SetOPLPara
 * *Description: 该函数主要负责检查输出的AT命令的参数，通过调用Taf_ParaQuery的接口执
 *               行AT命令的操作，调用Taf_ParaQuery时OpId参数为0xFF，标识查询OPL文件内
 *               容操作。
 * *Return: TAF_UINT32
 *   1.Date: 2008-03-06
 *     Modification: Created function
 */
TAF_UINT32 At_SetOPLPara(TAF_UINT8 indexNum)
{
    TAF_PH_QryUsimInfo usimInfo;

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 0) {
        usimInfo.icctype = TAF_PH_ICC_SIM;
    } else {
        usimInfo.icctype = TAF_PH_ICC_USIM;
    }
    usimInfo.efId = TAF_PH_OPL_FILE;
    /* linear fixed file only support 254 reocrd */
    if ((g_atParaList[1].paraLen != 0)) {
        if ((g_atParaList[1].paraValue > 0xFE) || (g_atParaList[1].paraValue == 0)) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            usimInfo.recNum = g_atParaList[1].paraValue;
        }
    } else {
        usimInfo.recNum = 0;
    }

    /* 执行命令操作 */
    if (TAF_QryUsimInfo(g_atClientTab[indexNum].clientId, 0, &usimInfo) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_OPL_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetSPNPara(TAF_UINT8 indexNum)
{
    VOS_UINT8       spnType;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 输入参数取值错误 */
    spnType = (TAF_UINT8)g_atParaList[0].paraValue;
    if (spnType > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录AT^SPN查询的SPN类型 */
    netCtx->spnType = spnType;

    /* 给MMA发送消息，查询新参数类型TAF_PH_SPN_VALUE_PARA */
    if (TAF_MMA_QrySpnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SPN_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCardModePara(TAF_UINT8 indexNum)
{
    TAF_UINT8        cardMode = 0; /* 0: card not exists, 1: SIM card, 2: USIM card */
    TAF_UINT16       length   = 0;
    ModemIdUint16    modemId = MODEM_ID_0;
    VOS_UINT32       getModemIdRslt;
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;

    /* 参数过多 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    getModemIdRslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (getModemIdRslt != VOS_OK) {
        AT_ERR_LOG("At_SetCardModePara:Get Modem Id fail!");
        return AT_ERROR;
    }

    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    if (usimInfoCtx->cardStatus == USIMM_CARDAPP_SERVIC_BUTT) {
        AT_ERR_LOG("At_SetCardModePara:Sim card status butt!");
        return AT_CME_SIM_BUSY;
    }

    /* 获取卡模式 */
    if (usimInfoCtx->cardStatus != USIMM_CARDAPP_SERVIC_ABSENT) {
        if (usimInfoCtx->cardType == TAF_MMA_USIMM_CARD_SIM) {
            cardMode = AT_CARDMODE_SIM_TYPE_SIM;
        } else if (usimInfoCtx->cardType == TAF_MMA_USIMM_CARD_USIM) {
            cardMode = AT_CARDMODE_SIM_TYPE_USIM;
        } else {
            cardMode = AT_CARDMODE_SIM_TYPE_NULL_CARD;
        }
    } else {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        return AT_CME_SIM_FAILURE;
#else
        cardMode = AT_CARDMODE_SIM_TYPE_NULL_CARD;
#endif
    }

    /* 输出结果: ^CARDMODE:cardmode */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d", cardMode);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_SetHvsstPara(VOS_UINT8 indexNum)
{
    VOS_UINT32      result;
    SI_PIH_HvsstSet hvSStSet;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_HVSST_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&hvSStSet, sizeof(hvSStSet), 0x00, sizeof(hvSStSet));

    hvSStSet.index  = (VOS_UINT8)g_atParaList[0].paraValue;
    hvSStSet.simSet = (VOS_UINT8)g_atParaList[1].paraValue;

#if (FEATURE_VSIM == FEATURE_OFF)
    if (hvSStSet.index != SI_PIH_SIM_REAL_SIM1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    result = SI_PIH_HvSstSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &hvSStSet);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetHvsstPara: SI_PIH_HvSstSet fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HVSST_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_VOID AT_RSMParaFillCommandAndPath(SI_PIH_AccessFile *command)
{
    errno_t memResult;

    if ((g_atParaList[AT_CURSM_DATA].paraLen > 0) &&
        (g_atParaList[AT_CURSM_DATA].paraLen <= sizeof(command->command))) {
        memResult = memcpy_s(command->command, sizeof(command->command), g_atParaList[AT_CURSM_DATA].para,
                             g_atParaList[AT_CURSM_DATA].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->command), g_atParaList[AT_CURSM_DATA].paraLen);
    }
    if ((g_atParaList[AT_CURSM_PATHID].paraLen > 0) &&
        (g_atParaList[AT_CURSM_PATHID].paraLen <= sizeof(command->path))) {
        memResult = memcpy_s(command->path, sizeof(command->path), g_atParaList[AT_CURSM_PATHID].para,
                             g_atParaList[AT_CURSM_PATHID].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->path), g_atParaList[AT_CURSM_PATHID].paraLen);
    }
}

VOS_UINT32 AT_CheckSetUrsmParaNum(VOS_VOID)
{
    /* 参数过多 */
    if ((g_atParaIndex > AT_CURSM_MAX_PARA_NUM) || (g_atParaIndex < AT_CURSM_MIN_PARA_NUM)) {
        AT_ERR_LOG("AT_CheckSetUrsmParaCommand: too many para");

        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckSetUrsmParaCommand(VOS_VOID)
{
    if ((g_atParaList[AT_CURSM_COMMAND].paraValue == SI_PIH_ACCESS_WRITE) &&
        (g_atParaList[AT_CURSM_DATA].paraLen == 0)) {
        AT_ERR_LOG("AT_CheckSetUrsmParaCommand: Write para wrong");

        return VOS_FALSE;
    }

    if ((g_atParaList[AT_CURSM_COMMAND].paraValue == SI_PIH_ACCESS_READ) &&
        (g_atParaList[AT_CURSM_DATA].paraLen != 0)) {
        AT_ERR_LOG("AT_CheckSetUrsmParaCommand: Read para wrong");

        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckSetUrsmParaFileid(VOS_VOID)
{
    VOS_UINT16        fileID;
    /* 获取文件ID前两位 */
    fileID = (g_atParaList[AT_CURSM_FILEID].paraValue >> 8) & (0x00FF);

    /* 输入的文件ID必须是EF文件，前两位不可以是3F/5F/7F */
    if ((fileID == MFLAB) || (fileID == DFUNDERMFLAB) || (fileID == DFUNDERDFLAB)) {
        AT_ERR_LOG("AT_SetUrsmPara: File Id error.");

        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckSetUrsmParaLen(VOS_VOID)
{
    if ((g_atParaList[AT_CURSM_DATA].paraLen > USIMM_T0_APDU_MAX_LEN) ||
        (g_atParaList[AT_CURSM_PATHID].paraLen > USIMM_MAX_PATH_LEN) ||
        (g_atParaList[AT_CURSM_APP_TYPE].paraValue >= SI_PIH_UICCAPP_BUTT)) {
        AT_ERR_LOG("AT_SetUrsmPara: AT para 4 or 5 is fail.");

        return VOS_FALSE;
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CheckSetUrsmPara(VOS_UINT8 indexNum)
{
    if (AT_CheckSetUrsmParaNum() != VOS_TRUE) {
        return VOS_FALSE;
    }

    if (AT_CheckSetUrsmParaCommand() != VOS_TRUE) {
        return VOS_FALSE;
    }

    if (AT_CheckSetUrsmParaFileid() != VOS_TRUE) {
        return VOS_FALSE;
    }

    if (g_atParaList[AT_CURSM_DATA].paraLen != 0) {
        if (At_AsciiNum2HexString(g_atParaList[AT_CURSM_DATA].para,
                                  &g_atParaList[AT_CURSM_DATA].paraLen) == AT_FAILURE) {
            AT_ERR_LOG("AT_CheckSetUrsmPara: At_AsciiNum2HexString para4 fail.");

            return VOS_FALSE;
        }
    }

    if (g_atParaList[AT_CURSM_PATHID].paraLen != 0) {
        if (At_AsciiNum2HexString(g_atParaList[AT_CURSM_PATHID].para,
                                  &g_atParaList[AT_CURSM_PATHID].paraLen) == AT_FAILURE) {
            AT_ERR_LOG("AT_CheckSetUrsmPara: At_AsciiNum2HexString para5 fail.");

            return VOS_FALSE;
        }
    }

    if (AT_CheckSetUrsmParaLen() != VOS_TRUE) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_SetUrsmPara(VOS_UINT8 indexNum)
{
    SI_PIH_AccessFile command;

    if (AT_CheckSetUrsmPara(indexNum) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    command.efId      = (VOS_UINT16)g_atParaList[AT_CURSM_FILEID].paraValue;
    command.appType   = g_atParaList[AT_CURSM_APP_TYPE].paraValue;
    command.cmdType   = (VOS_UINT8)g_atParaList[AT_CURSM_COMMAND].paraValue;
    command.recordNum = (VOS_UINT8)g_atParaList[AT_CURSM_RECORD_NUMBER].paraValue;
    command.dataLen   = g_atParaList[AT_CURSM_DATA].paraLen;
    command.pathLen   = g_atParaList[AT_CURSM_PATHID].paraLen;

    AT_RSMParaFillCommandAndPath(&command);

    /* 执行命令操作 */
    if (SI_PIH_AccessUICCFileReq(g_atClientTab[indexNum].clientId, 0, &command) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURSM_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_ERR_LOG("AT_SetUrsmPara: SI_PIH_AccessUICCFileReq error.");

        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_SetEflociInfoPara(VOS_UINT8 indexNum)
{
    TAF_MMA_Eflociinfo efLociInfo;
    VOS_UINT32         plmnHex;

    /* 初始化结构体 */
    (VOS_VOID)memset_s(&efLociInfo, sizeof(efLociInfo), 0x00, sizeof(efLociInfo));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("At_SetEflociInfoPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != AT_EFLOCIINFO_PARA_NUM) {
        AT_WARN_LOG("At_SetEflociInfoPara: PARA NUM IS NOT EQUAL 5!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 转换Tmsi */
    if (g_atParaList[AT_EFLOCIINFO_TMSI].paraLen == 0) {
        AT_WARN_LOG("At_SetEflociInfoPara: Tmsi LEN IS 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    efLociInfo.tmsi = g_atParaList[AT_EFLOCIINFO_TMSI].paraValue;

    /* 转换PLMN */
    if ((g_atParaList[AT_EFLOCIINFO_PLMN].paraLen != AT_PLMN_STR_MAX_LEN) &&
        (g_atParaList[AT_EFLOCIINFO_PLMN].paraLen != AT_PLMN_STR_MIN_LEN)) {
        AT_WARN_LOG("At_SetEflociInfoPara: PLMN LEN IS ERROR!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    plmnHex = 0;

    if (AT_String2Hex(g_atParaList[AT_EFLOCIINFO_PLMN].para,
        g_atParaList[AT_EFLOCIINFO_PLMN].paraLen, &plmnHex) != AT_SUCCESS) {
        AT_WARN_LOG("At_SetEflociInfoPara: String2Hex error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (plmnHex == 0xffffff) {
        efLociInfo.plmnId.mcc = 0xFFFFFFFF;
        efLociInfo.plmnId.mnc = 0xFFFFFFFF;
    } else {
        if (AT_DigitString2Hex(g_atParaList[AT_EFLOCIINFO_PLMN].para,
            AT_MCC_STR_LEN, &efLociInfo.plmnId.mcc) != VOS_TRUE) {
            AT_WARN_LOG("At_SetEflociInfoPara: Mcc IS ERROR!");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (AT_DigitString2Hex(&(g_atParaList[AT_EFLOCIINFO_PLMN].para[AT_EFLOCIINFO_PLMN_THIRD_PARA]),
                               g_atParaList[AT_EFLOCIINFO_PLMN].paraLen - AT_MCC_STR_LEN,
                               &(efLociInfo.plmnId.mnc)) != VOS_TRUE) {
            AT_WARN_LOG("At_SetEflociInfoPara: Mnc IS ERROR!");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        efLociInfo.plmnId.mcc |= AT_PLMN_MCC_CLASS_MASK;
        /* mnc一个数字只占半个字节，也就是4bit */
        efLociInfo.plmnId.mnc |= (0xFFFFFFFF << ((g_atParaList[AT_EFLOCIINFO_PLMN].paraLen - AT_MCC_STR_LEN) *
        AT_HALF_BYTE_TO_BITS_LENS));
    }

    /* 转换LAC */
    if (g_atParaList[AT_EFLOCIINFO_LAC].paraLen == 0) {
        AT_WARN_LOG("At_SetEflociInfoPara: LAC LEN IS 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    efLociInfo.lac = (VOS_UINT16)g_atParaList[AT_EFLOCIINFO_LAC].paraValue;

    /* 转换location_update_Status */
    if (g_atParaList[AT_EFLOCIINFO_LOCATION_UPDATE_STATUS].paraLen == 0) {
        AT_WARN_LOG("At_SetEflociInfoPara: location_update_Status LEN IS 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    efLociInfo.locationUpdateStatus = (VOS_UINT8)g_atParaList[AT_EFLOCIINFO_LOCATION_UPDATE_STATUS].paraValue;

    /* 转换rfu */
    if (g_atParaList[AT_EFLOCIINFO_RFU].paraLen == 0) {
        AT_WARN_LOG("At_SetEflociInfoPara: rfu LEN IS 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    efLociInfo.rfu = (VOS_UINT8)g_atParaList[AT_EFLOCIINFO_RFU].paraValue;

    /* 执行操作命令 */
    if (TAF_MMA_SetEflociInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &efLociInfo) != VOS_TRUE) {
        AT_WARN_LOG("At_SetEflociInfoPara: TAF_MMA_SetEflociInfo return is not VOS_TRUE !");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EFLOCIINFO_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 AT_CheckPsEflociInfoPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckPsEflociInfoPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return VOS_FALSE;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != AT_EFPSLOCIINFO_PARA_NUM) {
        AT_WARN_LOG("AT_CheckPsEflociInfoPara: Para num is error!");
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 At_SetPsEflociInfoParaPlmn(TAF_MMA_Efpslociinfo *psefLociInfo)
{
    VOS_UINT32           plmnHex;

    if ((g_atParaList[AT_EFPSLOCIINFO_PLMN].paraLen != AT_PLMN_STR_MAX_LEN) &&
        (g_atParaList[AT_EFPSLOCIINFO_PLMN].paraLen != AT_PLMN_STR_MIN_LEN)) {
        AT_WARN_LOG("At_SetPsEflociInfoParaPlmn: PLMN len is Error!");
        return VOS_FALSE;
    }

    plmnHex = 0;

    if (AT_String2Hex(g_atParaList[AT_EFPSLOCIINFO_PLMN].para,
        g_atParaList[AT_EFPSLOCIINFO_PLMN].paraLen, &plmnHex) != AT_SUCCESS) {
        AT_WARN_LOG("At_SetPsEflociInfoParaPlmn: String2Hex error!");
        return VOS_FALSE;
    }

    if (plmnHex == 0xffffff) {
        psefLociInfo->plmnId.mcc = 0xFFFFFFFF;
        psefLociInfo->plmnId.mnc = 0xFFFFFFFF;
    } else {
        if (AT_DigitString2Hex(g_atParaList[AT_EFPSLOCIINFO_PLMN].para, AT_MCC_STR_LEN,
                               &psefLociInfo->plmnId.mcc) != VOS_TRUE) {
            AT_WARN_LOG("At_SetPsEflociInfoParaPlmn: Mcc num is Error!");
            return VOS_FALSE;
        }

        if (AT_DigitString2Hex(&(g_atParaList[AT_EFPSLOCIINFO_PLMN].para[AT_EFPSLOCIINFO_PLMN_THIRD_PARA]),
                               g_atParaList[AT_EFPSLOCIINFO_PLMN].paraLen - AT_MCC_STR_LEN,
                               &(psefLociInfo->plmnId.mnc)) != VOS_TRUE) {
            AT_WARN_LOG("At_SetPsEflociInfoParaPlmn: Mnc num is Error!");
            return VOS_FALSE;
        }

        psefLociInfo->plmnId.mcc |= AT_PLMN_MCC_CLASS_MASK;
        psefLociInfo->plmnId.mnc |= (0xFFFFFFFF << ((g_atParaList[AT_EFPSLOCIINFO_PLMN].paraLen -
        AT_MCC_STR_LEN) * AT_HALF_BYTE_TO_BITS_LENS));
    }

    return VOS_TRUE;
}

VOS_UINT32 At_SetPsEflociInfoPara(VOS_UINT8 indexNum)
{
    TAF_MMA_Efpslociinfo psefLociInfo = {0};

    if (AT_CheckPsEflociInfoPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 转换PTMSI */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: PTMSI len is 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psefLociInfo.pTmsi = g_atParaList[0].paraValue;

    /* 转换PtmsiSignature */
    if (g_atParaList[1].paraLen == 0) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: PtmsiSignature len is 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psefLociInfo.pTmsiSignature = g_atParaList[1].paraValue;

    if (At_SetPsEflociInfoParaPlmn(&psefLociInfo) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 转换PLMN */


    /* 转换LAC */
    if (g_atParaList[AT_EFPSLOCIINFO_LAC].paraLen == 0) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: LAC len is 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psefLociInfo.lac = (VOS_UINT16)g_atParaList[AT_EFPSLOCIINFO_LAC].paraValue;

    /* 转换RAC */
    if (g_atParaList[AT_EFPSLOCIINFO_RAC].paraLen == 0) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: RAC len is 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psefLociInfo.rac = (VOS_UINT8)g_atParaList[AT_EFPSLOCIINFO_RAC].paraValue;

    /* 转换location_update_Status */
    if (g_atParaList[AT_EFPSLOCIINFO_ROUTING_AREA_UPDATE_STATUS].paraLen == 0) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: location_update_Status len is 0!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psefLociInfo.psLocationUpdateStatus = (VOS_UINT8)g_atParaList[AT_EFPSLOCIINFO_ROUTING_AREA_UPDATE_STATUS].paraValue;

    /* 执行操作命令 */
    if (TAF_MMA_SetPsEflociInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &psefLociInfo) != VOS_TRUE) {
        AT_WARN_LOG("At_SetPsEflociInfoPara: TAF_MMA_SetPsEflociInfo return is not VOS_TRUE !");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EFPSLOCIINFO_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

/*
 * Description: +CCHO=<dfname>
 * History:
 *  1.Date: 2013-05-15
 *    Modification: Created function
 */
TAF_UINT32 At_SetCchoPara(TAF_UINT8 indexNum)
{
    SI_PIH_CchoCommand cchoCmd;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CCHO_DFNAME].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CCHO_DFNAME].para,
                              &g_atParaList[AT_CCHO_DFNAME].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetCchoPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    cchoCmd.aidLen  = g_atParaList[AT_CCHO_DFNAME].paraLen;
    cchoCmd.adfName = g_atParaList[AT_CCHO_DFNAME].para;

    /* 执行命令操作 */
    if (SI_PIH_CchoSetReq(g_atClientTab[indexNum].clientId, 0, &cchoCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCHO_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

TAF_UINT32 At_SetCchpPara(TAF_UINT8 indexNum)
{
    SI_PIH_CchpCommand cchpCmd;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CCHP_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CCHP_DFNAME].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CCHP_DFNAME].para,
                              &g_atParaList[AT_CCHP_DFNAME].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetCchopPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    cchpCmd.aidLen  = g_atParaList[AT_CCHP_DFNAME].paraLen;
    cchpCmd.adfName = g_atParaList[AT_CCHP_DFNAME].para;
    cchpCmd.apdup2  = (VOS_UINT8)g_atParaList[1].paraValue;

    /* 执行命令操作 */
    if (SI_PIH_CchpSetReq(g_atClientTab[indexNum].clientId, 0, &cchpCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCHP_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

TAF_UINT32 At_SetPrivateCchoPara(TAF_UINT8 indexNum)
{
    SI_PIH_CchoCommand cchoCmd = {0};

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CCHO_DFNAME].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[0].para, &g_atParaList[0].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetPrivateCchoPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    cchoCmd.aidLen  = g_atParaList[0].paraLen;
    cchoCmd.adfName = g_atParaList[0].para;

    /* 执行命令操作 */
    if (SI_PIH_PrivateCchoSetReq(g_atClientTab[indexNum].clientId, 0, &cchoCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PRIVATECCHO_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

TAF_UINT32 At_SetPrivateCchpPara(TAF_UINT8 indexNum)
{
    SI_PIH_CchpCommand cchpCmd = {0};

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CCHP_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CCHP_DFNAME].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CCHP_DFNAME].para,
                              &g_atParaList[AT_CCHP_DFNAME].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetPrivateCchpPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    cchpCmd.aidLen   = g_atParaList[AT_CCHP_DFNAME].paraLen;
    cchpCmd.adfName  = g_atParaList[AT_CCHP_DFNAME].para;
    cchpCmd.apdup2   = (VOS_UINT8)g_atParaList[1].paraValue;

    /* 执行命令操作 */
    if (SI_PIH_PrivateCchpSetReq(g_atClientTab[indexNum].clientId, 0, &cchpCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PRIVATECCHP_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

/*
 * Description: +CCHC=<sessionid>
 * History:
 *  1.Date: 2013-05-15
 *    Modification: Created function
 */
TAF_UINT32 At_SetCchcPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PIH_CchcSetReq(g_atClientTab[indexNum].clientId, 0, g_atParaList[0].paraValue) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCHC_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 At_SetBwtPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_TOO_MANY_PARA;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_PIH_BwtSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                           (VOS_UINT16)g_atParaList[0].paraValue);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetBwtPara: SI_PIH_HvSstSet fail.");

        return AT_CME_PHONE_FAILURE;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_BWT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_SetCardHistoryATRPara(TAF_UINT8 indexNum)
{
    if (SI_PIH_GetCardHistoryATRReq(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARD_HISTORY_ATR_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

/*
 * Function:       At_CrlaApduParaCheck
 * Description:    执行CRLA命令输入的参数的匹配检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaApduParaCheck(VOS_VOID)
{
    TAF_UINT16 fileTag;

    /* 命令类型参数检查，第二个参数不能为空 */
    if (g_atParaList[AT_CRLA_COMMAND].paraLen == 0) {
        AT_ERR_LOG("At_CrlaApduParaCheck: command type null");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 除STATUS命令外，文件ID输入不能为空 */
    if ((g_atParaList[AT_CRLA_FILEID].paraValue == 0) && (g_atParaList[AT_CRLA_COMMAND].paraValue != USIMM_STATUS)) {
        AT_ERR_LOG("At_CrlaApduParaCheck: File Id null.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取文件ID前两位 */
    fileTag = (g_atParaList[AT_CRLA_FILEID].paraValue >> 8) & (0x00FF);

    /* 输入的文件ID必须是EF文件，前两位不可以是3F/5F/7F */
    if ((fileTag == MFLAB) || (fileTag == DFUNDERMFLAB) || (fileTag == DFUNDERDFLAB)) {
        AT_ERR_LOG("At_CrlaApduParaCheck: File Id error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <P1><P2><P3>这三个参数全部为空 */
    if ((g_atParaList[AT_CRLA_P1].paraLen == 0) && (g_atParaList[AT_CRLA_P2].paraLen == 0) &&
        (g_atParaList[AT_CRLA_P3].paraLen == 0)) {
        return AT_SUCCESS;
    }

    /* <P1><P2><P3>这三个参数全部不为空 */
    if ((g_atParaList[AT_CRLA_P1].paraLen != 0) && (g_atParaList[AT_CRLA_P2].paraLen != 0) &&
        (g_atParaList[AT_CRLA_P3].paraLen != 0)) {
        return AT_SUCCESS;
    }

    /* 其它情况下属于输入AT命令参数不完整 */
    return AT_CME_INCORRECT_PARAMETERS;
}

/*
 * Function:       At_CrlaFilePathCheck
 * Description:    执行CRLA命令输入的<path>参数(文件路径进行检查)
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaFilePathCheck(TAF_UINT32 efId, TAF_UINT8 *filePath, TAF_UINT16 *pathLen)
{
    errno_t    memResult;
    TAF_UINT32 pathInvalidFlg;
    TAF_UINT16 len;
    TAF_UINT16 path[USIMM_MAX_PATH_LEN]    = {0};
    TAF_UINT16 tmpPath[USIMM_MAX_PATH_LEN] = {0};
    TAF_UINT16 pathLenTemp;
    TAF_UINT32 i;

    pathLenTemp = *pathLen;
    len         = 0;

    if (pathLenTemp > USIMM_MAX_PATH_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < (pathLenTemp / sizeof(TAF_UINT16)); i++) {
        tmpPath[i] = (TAF_UINT16)((filePath[i * sizeof(TAF_UINT16)] << 0x08) & AT_SIM_FILE_PATH_MASK) +
                     filePath[(i * sizeof(TAF_UINT16)) + 1];
    }

    /* 如果路径不是以3F00开始，需要添加3F00作开头 */
    if (tmpPath[0] != MFID) {
        if (pathLenTemp == USIMM_MAX_PATH_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        path[0] = MFID;

        len++;
    }

    if (pathLenTemp > 0) {
        memResult = memcpy_s(&path[len], sizeof(TAF_UINT16) * (USIMM_MAX_PATH_LEN - len), tmpPath, pathLenTemp);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_UINT16) * (USIMM_MAX_PATH_LEN - len), pathLenTemp);
    }

    len += (pathLenTemp / sizeof(TAF_UINT16));

    if ((efId & AT_SIM_FILE_PATH_MASK) == EFIDUNDERMF) {
        len = 1;
    }
    /* 4F文件要在5F下，路径长度为3 */
    else if ((efId & AT_SIM_FILE_PATH_MASK) == EFIDUNDERMFDFDF) {
        pathInvalidFlg = (len != AT_SIM_FILE_PATH_LEN_3) ||
                         ((path[AT_SIM_FILE_PATH_INDEX_1] & AT_SIM_FILE_PATH_MASK) != DFIDUNDERMF) ||
                         ((path[AT_SIM_FILE_PATH_INDEX_2] & AT_SIM_FILE_PATH_MASK) != DFIDUNDERMFDF);

        if (pathInvalidFlg == VOS_TRUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    /* 6F文件要在7F下，路径长度为2 */
    else if ((efId & AT_SIM_FILE_PATH_MASK) == EFIDUNDERMFDF) {
        if ((len != AT_SIM_FILE_PATH_LEN_2) ||
            ((path[AT_SIM_FILE_PATH_INDEX_1] & AT_SIM_FILE_PATH_MASK) != DFIDUNDERMF)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
    }

    *pathLen = len;

    if (len > 0) {
        memResult = memcpy_s(filePath, AT_PARA_MAX_LEN + 1, path, (VOS_SIZE_T)(len * AT_DOUBLE_LENGTH));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, (VOS_SIZE_T)(len * AT_DOUBLE_LENGTH));
    }

    return AT_SUCCESS;
}

/*
 * Function:       At_CrlaFilePathParse
 * Description:    执行CRSM命令输入的路径进行解析
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaFilePathParse(SI_PIH_Crla *command)
{
    errno_t    memResult;
    TAF_UINT32 result;

    /* 如果词法解析器解析第八个参数为空，说明没有文件路径输入，直接返回成功 */
    if ((g_atParaList[AT_CRLA_PATHID].paraLen == 0) && (command->efId != VOS_NULL_WORD)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 在转换前输入的文件路径长度必须是4的整数倍 */
    if ((g_atParaList[AT_CRLA_PATHID].paraLen % 4) != 0) {
        AT_ERR_LOG("At_CrlaFilePathParse: Path error");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将输入的字符串转换成十六进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CRLA_PATHID].para, &g_atParaList[AT_CRLA_PATHID].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrlaFilePathParse: At_AsciiNum2HexString error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果有填写文件ID和路径，要做文件路径检查，输入的路径长度以U16为单位 */
    result = At_CrlaFilePathCheck((TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue, g_atParaList[AT_CRLA_PATHID].para,
                                  &g_atParaList[AT_CRLA_PATHID].paraLen);

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_CrlaFilePathParse: At_CrsmFilePathCheck error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充文件路径和长度 */
    command->pathLen = g_atParaList[AT_CRLA_PATHID].paraLen;

    /* 文件路径长度是U16为单位的，路径拷贝的长度要乘2 */
    if (((g_atParaList[AT_CRLA_PATHID].paraLen) > 0) &&
        ((VOS_SIZE_T)(g_atParaList[AT_CRLA_PATHID].paraLen * sizeof(VOS_UINT16)) <= sizeof(command->path))) {
        memResult = memcpy_s(command->path, sizeof(command->path), g_atParaList[AT_CRLA_PATHID].para,
                             (VOS_SIZE_T)(g_atParaList[AT_CRLA_PATHID].paraLen * sizeof(VOS_UINT16)));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->path),
                            (VOS_SIZE_T)(g_atParaList[AT_CRLA_PATHID].paraLen * sizeof(VOS_UINT16)));
    }

    return AT_SUCCESS;
}

/*
 * Function:       At_CrlaParaStatusCheck
 * Description:    执行CRLA命令对STATUS命令的参数检查
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaParaStatusCheck(SI_PIH_Crla *command)
{
    /* STATUS命令如果没有输入文件ID，就不需要做选文件操作，直接发STATUS命令 */
    if (g_atParaList[AT_CRLA_FILEID].paraValue == 0) {
        command->efId = VOS_NULL_WORD;
    } else {
        command->efId = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->cmdType = USIMM_STATUS;

    return At_CrlaFilePathParse(command);
}

/*
 * Function:       At_CrlaParaReadBinaryCheck
 * Description:    执行CRLA命令对Read Binary命令的参数检查
 * Return:          AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaParaReadBinaryCheck(SI_PIH_Crla *command)
{
    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    command->cmdType = USIMM_READ_BINARY;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrlaFilePathParse(command);
}

/*
 * Function:       At_CrlaParaReadRecordCheck
 * Description:    执行CRLA命令对Read Record的参数检查
 * Return:         AT_SUCCESS：成功，其他为失败
 * History:
 * 1.Date: 2012-05-04
 *   Modification: Created function
 */
TAF_UINT32 At_CrlaParaReadRecordCheck(SI_PIH_Crla *command)
{
    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    command->cmdType = USIMM_READ_RECORD;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrlaFilePathParse(command);
}

VOS_UINT32 At_CrlaParaGetRspCheck(SI_PIH_Crla *command)
{
    /* 参数个数不能少于2个，至少要有命令类型和文件ID */
    if (g_atParaIndex < AT_CRLA_PARA_GETRSP_MIN_NUM) {
        AT_ERR_LOG("At_CrlaParaGetRspCheck: Para less than 2.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    command->cmdType = USIMM_GET_RESPONSE;

    /* 如果有输入文件路径需要检查输入参数 */
    return At_CrlaFilePathParse(command);
}

VOS_UINT32 At_CrlaParaUpdateBinaryCheck(SI_PIH_Crla *command)
{
    errno_t memResult;
    /* Update Binary命令至少要有6个参数，可以没有文件路径 */
    if (g_atParaIndex < AT_CRLA_PARA_UPDATE_BINARY_MIN_NUM) {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    command->cmdType = USIMM_UPDATE_BINARY;

    /* 第六个参数输入的<data>字符串在转换前数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRLA_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRLA_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRLA_DATA].para, &g_atParaList[AT_CRLA_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_CRLA_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: g_atParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    memResult = memcpy_s((TAF_VOID *)command->content, sizeof(command->content),
                         (TAF_VOID *)g_atParaList[AT_CRLA_DATA].para, g_atParaList[AT_CRLA_DATA].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->content), g_atParaList[AT_CRLA_DATA].paraLen);

    return At_CrlaFilePathParse(command);
}

VOS_UINT32 At_CrlaParaUpdateRecordCheck(SI_PIH_Crla *command)
{
    errno_t memResult;

    /* Update Binary命令至少要有6个参数，可以没有文件路径 */
    if (g_atParaIndex < AT_CRLA_PARA_UPDATE_BINARY_MIN_NUM) {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3>对应的IE项 */
    command->p1      = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2      = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3      = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;
    command->efId    = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;
    command->cmdType = USIMM_UPDATE_RECORD;

    /* 第六个参数输入的<data>字符串数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRLA_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRLA_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRLA_DATA].para, &g_atParaList[AT_CRLA_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 防止因为数据长度过长导致单板复位 */
    if (g_atParaList[AT_CRLA_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: g_atParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    memResult = memcpy_s((TAF_VOID *)command->content, sizeof(command->content),
                         (TAF_VOID *)g_atParaList[AT_CRLA_DATA].para,
                         g_atParaList[AT_CRLA_DATA].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command->content), g_atParaList[AT_CRLA_DATA].paraLen);

    return At_CrlaFilePathParse(command);
}

VOS_UINT32 At_CrlaParaSearchRecordCheck(SI_PIH_Crla *command)
{
    /* Search Record命令至少要有8个参数 */
    if (g_atParaIndex < AT_CRLA_PARA_SEARCH_RECORD_MIN_NUM) {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: Para less than 8.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写数据结构中的<P1><P2><P3> */
    command->p1 = (TAF_UINT8)g_atParaList[AT_CRLA_P1].paraValue;
    command->p2 = (TAF_UINT8)g_atParaList[AT_CRLA_P2].paraValue;
    command->p3 = (TAF_UINT8)g_atParaList[AT_CRLA_P3].paraValue;

    /* 填写数据结构中的<fileid> */
    command->efId = (TAF_UINT16)g_atParaList[AT_CRLA_FILEID].paraValue;

    /* 填写数据结构中的<command> */
    command->cmdType = USIMM_SEARCH_RECORD;

    /* 第七个参数输入的<data>字符串数据长度必须是2的倍数且不能为0 */
    if (((g_atParaList[AT_CRLA_DATA].paraLen % 2) != 0) || (g_atParaList[AT_CRLA_DATA].paraLen == 0)) {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CRLA_DATA].para, &g_atParaList[AT_CRLA_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 防止因为数据长度过长导致单板复位 */
    if (g_atParaList[AT_CRLA_DATA].paraLen > sizeof(command->content)) {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: g_atParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<data>，其长度由<data>参数输入确定，P3参数照常下发，不关心<data>的长度是否和P3的值匹配 */
    if (memcpy_s((TAF_VOID *)command->content, sizeof(command->content), (TAF_VOID *)g_atParaList[AT_CRLA_DATA].para,
                 g_atParaList[AT_CRLA_DATA].paraLen) != EOK) {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: memcpy_s fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    return At_CrlaFilePathParse(command);
}

TAF_UINT32 At_SetCrlaPara(TAF_UINT8 indexNum)
{
    SI_PIH_Crla command;
    TAF_UINT32  result;

    /* 参数过多 */
    if (g_atParaIndex > AT_CRLA_MAX_PARA_NUM) {
        AT_ERR_LOG("At_SetCrlaPara: too many para");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果有输入<P1><P2><P3>，这三个参数不能只填写部分参数 */
    if (At_CrlaApduParaCheck() != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCrlaPara: At_CrlaApduParaCheck fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&command, sizeof(command), 0x00, sizeof(command));

    command.sessionID = g_atParaList[0].paraValue;

    switch (g_atParaList[1].paraValue) {
        case USIMM_STATUS:
            result = At_CrlaParaStatusCheck(&command);
            break;
        case USIMM_READ_BINARY:
            result = At_CrlaParaReadBinaryCheck(&command);
            break;
        case USIMM_READ_RECORD:
            result = At_CrlaParaReadRecordCheck(&command);
            break;
        case USIMM_GET_RESPONSE:
            result = At_CrlaParaGetRspCheck(&command);
            break;
        case USIMM_UPDATE_BINARY:
            result = At_CrlaParaUpdateBinaryCheck(&command);
            break;
        case USIMM_UPDATE_RECORD:
            result = At_CrlaParaUpdateRecordCheck(&command);
            break;
        case USIMM_SEARCH_RECORD:
            result = At_CrlaParaSearchRecordCheck(&command);
            break;
        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCrlaPara: para parse fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PIH_CrlaSetReq(g_atClientTab[indexNum].clientId, 0, &command) == TAF_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CRLA_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

