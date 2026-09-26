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
#define THIS_FILE_ID PS_FILE_ID_TAF_MMI_STRPARSE_C
#include "vos.h"
#include "taf_mmi_str_parse.h"
#include "taf_app_ssa.h"
#include "mn_error_code.h"
/* A核和C核编解码都要用到 */
#include "taf_msg_ts.h"
#include "taf_std_lib.h"
#include "securec.h"
#include "taf_type_def.h"


#define MN_MMI_SC_MAX_ENTRY (sizeof(g_mmiScInfo) / sizeof(MN_MMI_ScTable))

#define MN_MMI_BS_MAX_ENTRY (sizeof(g_mmiBSInfomation) / sizeof(MN_MMI_BsTable))

/* 通用补充业务操作码映射表: 第一列补充业务操作码对应的MMI字符串，第二列补充业务操作码 */
static const MN_MMI_SsOpTbl g_tafMmiOporationTypeTbl[] = {
    { "**", TAF_MMI_REGISTER_SS, { 0, 0, 0, 0, 0, 0, 0 }},  { "*", TAF_MMI_ACTIVATE_SS, { 0, 0, 0, 0, 0, 0, 0 }},
    { "#", TAF_MMI_DEACTIVATE_SS, { 0, 0, 0, 0, 0, 0, 0 }}, { "*#", TAF_MMI_INTERROGATE_SS, { 0, 0, 0, 0, 0, 0, 0 }},
    { "##", TAF_MMI_ERASE_SS, { 0, 0, 0, 0, 0, 0, 0 }},
};

static const MN_MMI_ScTable g_mmiScInfo[] = {
    { "", TAF_ALL_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "30", TAF_CLIP_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "31", TAF_CLIR_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "76", TAF_COLP_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "77", TAF_COLR_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "21", TAF_CFU_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "67", TAF_CFB_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "61", TAF_CFNRY_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "62", TAF_CFNRC_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "002", TAF_ALL_FORWARDING_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "004", TAF_ALL_COND_FORWARDING_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "43", TAF_CW_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "37", TAF_CCBS_A_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "33", TAF_BAOC_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "331", TAF_BOIC_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "332", TAF_BOICEXHC_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "35", TAF_BAIC_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "351", TAF_BICROAM_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "330", TAF_ALL_BARRING_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "333", TAF_BARRING_OF_OUTGOING_CALLS_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "353", TAF_BARRING_OF_INCOMING_CALLS_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
    { "300", TAF_CNAP_SS_CODE, { 0, 0, 0, 0, 0, 0, 0 }},
};

static const MN_MMI_BsTable g_mmiBSInfomation[] = {
    { "10", TAF_ALL_TELESERVICES_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "11", TAF_ALL_SPEECH_TRANSMISSION_SERVICES_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "12", TAF_ALL_DATA_TELESERVICES_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "13", TAF_ALL_FACSIMILE_TRANSMISSION_SERVICES_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "16", TAF_ALL_SMS_SERVICES_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "19", TAF_ALL_TELESERVICES_EXEPTSMS_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "20", TAF_ALL_BEARERSERVICES_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "21", TAF_ALL_ASYNCHRONOUS_SERVICES_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "22", TAF_ALL_SYNCHRONOUS_SERVICES_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "24", TAF_ALL_DATA_CIRCUIT_SYNCHRONOUS_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "25", TAF_ALL_DATA_CIRCUIT_ASYNCHRONOUS_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "50", TAF_ALL_PLMN_SPECIFICTS_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "51", TAF_PLMN_SPECIFICTS_1_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "52", TAF_PLMN_SPECIFICTS_2_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "53", TAF_PLMN_SPECIFICTS_3_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "54", TAF_PLMN_SPECIFICTS_4_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "55", TAF_PLMN_SPECIFICTS_5_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "56", TAF_PLMN_SPECIFICTS_6_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "57", TAF_PLMN_SPECIFICTS_7_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "58", TAF_PLMN_SPECIFICTS_8_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "59", TAF_PLMN_SPECIFICTS_9_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "60", TAF_PLMN_SPECIFICTS_A_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "61", TAF_PLMN_SPECIFICTS_B_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "62", TAF_PLMN_SPECIFICTS_C_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "63", TAF_PLMN_SPECIFICTS_D_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "64", TAF_PLMN_SPECIFICTS_E_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "65", TAF_PLMN_SPECIFICTS_F_TSCODE, TAF_SS_TELE_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "70", TAF_ALL_PLMN_SPECIFICBS_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "71", TAF_PLMN_SPECIFICBS_1_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "72", TAF_PLMN_SPECIFICBS_2_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "73", TAF_PLMN_SPECIFICBS_3_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "74", TAF_PLMN_SPECIFICBS_4_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "75", TAF_PLMN_SPECIFICBS_5_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "76", TAF_PLMN_SPECIFICBS_6_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "77", TAF_PLMN_SPECIFICBS_7_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "78", TAF_PLMN_SPECIFICBS_8_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "79", TAF_PLMN_SPECIFICBS_9_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "80", TAF_PLMN_SPECIFICBS_A_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "81", TAF_PLMN_SPECIFICBS_B_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "82", TAF_PLMN_SPECIFICBS_C_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "83", TAF_PLMN_SPECIFICBS_D_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "84", TAF_PLMN_SPECIFICBS_E_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }},
    { "85", TAF_PLMN_SPECIFICBS_F_BSCODE, TAF_SS_BEARER_SERVICE, { 0, 0, 0, 0, 0, 0 }}
};

MN_CALL_ClirCfgUint8 g_clirOperate = MN_CALL_CLIR_AS_SUBS;

VOS_UINT32 MMI_GetOporationTypeTblSize(VOS_VOID)
{
    VOS_UINT32 tblSize;

    tblSize = sizeof(g_tafMmiOporationTypeTbl) / sizeof(g_tafMmiOporationTypeTbl[0]);

    return tblSize;
}

const MN_MMI_SsOpTbl* MMI_GetOporationTypeTblAddr(VOS_VOID)
{
    return g_tafMmiOporationTypeTbl;
}

VOS_UINT32 MMI_GetBSTblSize(VOS_VOID)
{
    VOS_UINT32 tblSize;

    tblSize = sizeof(g_mmiBSInfomation) / sizeof(g_mmiBSInfomation[0]);

    return tblSize;
}

const MN_MMI_BsTable* MMI_GetBSTblAddr(VOS_VOID)
{
    return g_mmiBSInfomation;
}

VOS_UINT32 MMI_GetSCTblSize(VOS_VOID)
{
    VOS_UINT32 tblSize;

    tblSize = sizeof(g_mmiScInfo) / sizeof(g_mmiScInfo[0]);

    return tblSize;
}

const MN_MMI_ScTable* MMI_GetSCTblAddr(VOS_VOID)
{
    return g_mmiScInfo;
}

LOCAL VOS_UINT32 MMI_AtoI(const VOS_CHAR *pcSrc)
{
    VOS_UINT32 i = 0;
    VOS_UINT64 tmpValue = 0;


    if (pcSrc == VOS_NULL_PTR) {
        MN_ERR_LOG("MMI_AtoI: Null Ptr");
        return i;
    }

    while (MN_MMI_isdigit(*pcSrc)) {
        tmpValue = ((VOS_UINT64)i * 10) + (VOS_UINT32)(VOS_UINT8)(*(pcSrc) - '0');
        pcSrc++;

        /* 反转保护 */
        if (tmpValue > TAF_MMI_MAX_UINT32_VALUE) {
            MN_ERR_LOG("MMI_AtoI:U32 revert");
            return 0;
        }

        i = (VOS_UINT32)tmpValue;
    }

    return i;
}

LOCAL VOS_CHAR* MMI_StrChr(const VOS_CHAR *pcFrom, const VOS_CHAR *pcTo, VOS_INT ichar)
{
    if (pcFrom == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    /* pcTo为空指针时调用点必须保证字符串中存在\0，否则可能发生死循环 */
    for (; (*pcFrom != (VOS_CHAR)ichar) && MN_MMI_STR_PTR_IS_VALID(pcFrom, pcTo); ++pcFrom) {
        if (*pcFrom == '\0') {
            return VOS_NULL_PTR;
        }
    }

    if ((pcTo != VOS_NULL_PTR) && (pcFrom > pcTo)) {
        return VOS_NULL_PTR;
    }

    return (VOS_CHAR *)pcFrom;
}

/* 不再使用 */

VOS_BOOL MMI_DecodeScAndSi(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam, MN_MMI_ScSiPara *scSiPara,
                           VOS_CHAR **ppOutMmiStr)
{
    errno_t    memResult;
    VOS_CHAR   tmpVal[MN_MMI_MAX_PARA_NUM][MN_MMI_MAX_BUF_SIZE];
    VOS_UINT32 i           = 0;
    VOS_UINT32 j           = 0;
    VOS_CHAR  *pcNextStart = VOS_NULL_PTR;
    VOS_CHAR  *pcEnd       = VOS_NULL_PTR;
    VOS_CHAR  *pcNextStop  = VOS_NULL_PTR;
    VOS_BOOL   bEndReached = VOS_FALSE;
    VOS_UINT16 offset      = 0;
    VOS_UINT32 tmpMmiNum = 0;


    (VOS_VOID)memset_s(scSiPara, sizeof(MN_MMI_ScSiPara), 0x00, sizeof(MN_MMI_ScSiPara));

    (VOS_VOID)memset_s(tmpVal, sizeof(tmpVal), 0x00, sizeof(tmpVal));

    if (mmiOpParam->mmiOperationType != TAF_MMI_REGISTER_PASSWD) {
        /* 跳过开始的几个字符 */
        for (i = 0; (!MN_MMI_isdigit(inMmiStr[i])) && (i < 2); i++) {
            offset++;
        }
    }

    pcNextStart = &inMmiStr[i];

    if ((pcEnd = MMI_StrChr(pcNextStart, VOS_NULL_PTR, MN_MMI_STOP_CHAR)) == VOS_NULL_PTR) {
        return VOS_FALSE;
    } else {
        pcNextStop = MMI_StrChr(pcNextStart, pcEnd, MN_MMI_START_SI_CHAR);

        j = 0;
        while ((j < MN_MMI_MAX_PARA_NUM) && (bEndReached == VOS_FALSE)) {
            if (pcNextStop != VOS_NULL_PTR) {
                tmpMmiNum = (VOS_UINT32)(pcNextStop - pcNextStart);
                if (tmpMmiNum > MN_MMI_MAX_BUF_SIZE) {
                    tmpMmiNum = MN_MMI_MAX_BUF_SIZE;
                }
                if (tmpMmiNum > 0) {
                    memResult = memcpy_s(&tmpVal[j][0], (MN_MMI_MAX_PARA_NUM - j) * MN_MMI_MAX_BUF_SIZE, pcNextStart,
                                         tmpMmiNum);
                    TAF_MEM_CHK_RTN_VAL(memResult, (MN_MMI_MAX_PARA_NUM - j) * MN_MMI_MAX_BUF_SIZE, tmpMmiNum);
                }
                offset += (VOS_UINT16)((pcNextStop - pcNextStart) + 1);
                pcNextStart = pcNextStop + 1;
                pcNextStop  = MMI_StrChr(pcNextStart, pcEnd, MN_MMI_START_SI_CHAR);
                j++;
            } else {
                tmpMmiNum = (VOS_UINT32)(pcEnd - pcNextStart);
                if (tmpMmiNum > MN_MMI_MAX_BUF_SIZE) {
                    tmpMmiNum = MN_MMI_MAX_BUF_SIZE;
                }
                if (tmpMmiNum > 0) {
                    memResult = memcpy_s(&tmpVal[j][0], (MN_MMI_MAX_PARA_NUM - j) * MN_MMI_MAX_BUF_SIZE, pcNextStart,
                                         tmpMmiNum);
                    TAF_MEM_CHK_RTN_VAL(memResult, (MN_MMI_MAX_PARA_NUM - j) * MN_MMI_MAX_BUF_SIZE, tmpMmiNum);
                }
                offset += (VOS_UINT16)((pcEnd - pcNextStart) + 1);
                bEndReached = VOS_TRUE;
            }
        }
    }

    memResult = memcpy_s(scSiPara->ssCode, sizeof(scSiPara->ssCode), &tmpVal[0][0], MN_MMI_MAX_SC_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(scSiPara->ssCode), MN_MMI_MAX_SC_LEN);
    memResult = memcpy_s(scSiPara->sia, sizeof(scSiPara->sia), &tmpVal[1][0], MN_MMI_MAX_SIA_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(scSiPara->sia), MN_MMI_MAX_SIA_LEN);
    memResult = memcpy_s(scSiPara->sib, sizeof(scSiPara->sib), &tmpVal[2][0], MN_MMI_MAX_SIB_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(scSiPara->sib), MN_MMI_MAX_SIB_LEN);
    memResult = memcpy_s(scSiPara->sic, sizeof(scSiPara->sic), &tmpVal[3][0], MN_MMI_MAX_SIC_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(scSiPara->sic), MN_MMI_MAX_SIC_LEN);

    *ppOutMmiStr = inMmiStr + offset;

    return VOS_TRUE;
}

LOCAL VOS_VOID MMI_ExtractPinOperationType(MN_MMI_OperationTypeUint8 mmiOperationType,
                                           MN_MMI_OperationParam    *mmiOpParam)
{
    switch (mmiOperationType) {
        case TAF_MMI_CHANGE_PIN2:
            mmiOpParam->pinReq.cmdType = TAF_PIN_CHANGE;
            mmiOpParam->pinReq.pinType = TAF_SIM_PIN2;
            break;

        case TAF_MMI_CHANGE_PIN:
            mmiOpParam->pinReq.cmdType = TAF_PIN_CHANGE;
            mmiOpParam->pinReq.pinType = TAF_SIM_PIN;
            break;

        case TAF_MMI_UNBLOCK_PIN2:
            mmiOpParam->pinReq.cmdType = TAF_PIN_UNBLOCK;
            mmiOpParam->pinReq.pinType = TAF_SIM_PIN2;
            break;

        case TAF_MMI_UNBLOCK_PIN:
            mmiOpParam->pinReq.cmdType = TAF_PIN_UNBLOCK;
            mmiOpParam->pinReq.pinType = TAF_SIM_PIN;
            break;

        default:
            /* 该分支永远不会走到 */
            break;
    }
}

LOCAL VOS_BOOL MMI_JudgePinOperation(VOS_CHAR *mmiStr, MN_MMI_OperationParam *mmiOpParam, VOS_CHAR **ppOutRestMmiStr,
                                     VOS_UINT32 *errCode)
{
    VOS_UINT32                   i = 0;
    MN_MMI_ScSiPara              scSiPara;
    const MN_MMI_StrOperationTbl mmiPinStrOpTbl[] = {
        { "**042*", TAF_MMI_CHANGE_PIN2, { 0, 0, 0, 0, 0, 0, 0 }},
        { "**04*", TAF_MMI_CHANGE_PIN, { 0, 0, 0, 0, 0, 0, 0 }},
        { "**052*", TAF_MMI_UNBLOCK_PIN2, { 0, 0, 0, 0, 0, 0, 0 }},
        { "**05*", TAF_MMI_UNBLOCK_PIN, { 0, 0, 0, 0, 0, 0, 0 }},
        { VOS_NULL_PTR, TAF_MMI_NULL_OPERATION, { 0, 0, 0, 0, 0, 0, 0 }}
    };
    errno_t returnValue;

    VOS_UINT32 srcStrNLen;
    VOS_UINT32 destStrNLen;

    *errCode = MN_ERR_NO_ERROR;

    srcStrNLen = VOS_StrNLen((VOS_CHAR *)mmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    while (mmiPinStrOpTbl[i].string != VOS_NULL_PTR) {
        destStrNLen = VOS_StrNLen((VOS_CHAR *)mmiPinStrOpTbl[i].string, TAF_MMI_PIN_MAX_OP_STRING_LEN);

        /* 字符串比较时按照目标字符串长度比较，保证输入字符创长度不小于目标字符串长度，不会发生读越界 */
        if (srcStrNLen >= destStrNLen) {
            if (VOS_MemCmp(mmiStr, mmiPinStrOpTbl[i].string, destStrNLen) == 0) {
                mmiOpParam->mmiOperationType = mmiPinStrOpTbl[i].operationType;
                break;
            }
        }

        i++;
    }

    if (mmiPinStrOpTbl[i].string == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    if (MMI_DecodeScAndSi(mmiStr, mmiOpParam, &scSiPara, ppOutRestMmiStr) == VOS_TRUE) {
        srcStrNLen  = VOS_StrNLen((VOS_CHAR *)scSiPara.sib, MN_MMI_MAX_SIB_LEN);
        destStrNLen = VOS_StrNLen((VOS_CHAR *)scSiPara.sic, MN_MMI_MAX_SIC_LEN);

        if ((srcStrNLen == destStrNLen) && (VOS_MemCmp(scSiPara.sib, scSiPara.sic, destStrNLen) == 0)) {
            (VOS_VOID)memset_s(mmiOpParam->pinReq.oldPin, TAF_PH_PINCODELENMAX + 1, 0, TAF_PH_PINCODELENMAX + 1);
            (VOS_VOID)memset_s(mmiOpParam->pinReq.newPin, TAF_PH_PINCODELENMAX + 1, 0, TAF_PH_PINCODELENMAX + 1);

            returnValue = strncpy_s((VOS_CHAR *)mmiOpParam->pinReq.oldPin, TAF_PH_PINCODELENMAX + 1,
                                    (VOS_CHAR *)scSiPara.sia,
                                    VOS_StrNLen((VOS_CHAR *)scSiPara.sia, TAF_PH_PINCODELENMAX));
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, TAF_PH_PINCODELENMAX + 1,
                                            VOS_StrNLen((VOS_CHAR *)scSiPara.sia, TAF_PH_PINCODELENMAX));
            returnValue = strncpy_s((VOS_CHAR *)mmiOpParam->pinReq.newPin, TAF_PH_PINCODELENMAX + 1,
                                    (VOS_CHAR *)scSiPara.sib, VOS_StrNLen((VOS_CHAR *)scSiPara.sib,
                                    MN_MMI_MAX_SIB_LEN));
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, TAF_PH_PINCODELENMAX + 1,
                                            VOS_StrNLen((VOS_CHAR *)scSiPara.sib, MN_MMI_MAX_SIB_LEN));
            /* 提圈 */
            MMI_ExtractPinOperationType(mmiOpParam->mmiOperationType, mmiOpParam);

            *errCode = MN_ERR_NO_ERROR;
        } else {
            *errCode = MN_ERR_INVALIDPARM;
        }
    } else {
        *errCode = MN_ERR_INVALIDPARM;
    }

    return VOS_TRUE;
}

LOCAL VOS_BOOL MMI_JudgePwdOperation(VOS_CHAR *mmiStr, MN_MMI_OperationParam *mmiOpParam, VOS_CHAR **ppOutRestMmiStr,
                                     VOS_UINT32 *errCode)
{
    VOS_UINT32             i = 0;
    MN_MMI_ScSiPara        scSiPara;
    MN_MMI_StrOperationTbl mmiPwdStrOpTbl[] = {
        { "**03*", TAF_MMI_REGISTER_PASSWD, { 0, 0, 0, 0, 0, 0, 0 }},
        { "*03*", TAF_MMI_REGISTER_PASSWD, { 0, 0, 0, 0, 0, 0, 0 }},
        { VOS_NULL_PTR, TAF_MMI_NULL_OPERATION, { 0, 0, 0, 0, 0, 0, 0 }}
    };
    VOS_UINT32 strNLen1;
    VOS_UINT32 strNLen2;
    VOS_UINT32 srcStrNLen;
    VOS_UINT32 desStrNLen;
    errno_t stringRet;

    *errCode = MN_ERR_NO_ERROR;

    srcStrNLen = VOS_StrNLen((VOS_CHAR *)mmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    while (mmiPwdStrOpTbl[i].string != VOS_NULL_PTR) {
        desStrNLen = VOS_StrNLen((VOS_CHAR *)mmiPwdStrOpTbl[i].string, TAF_MMI_PWD_MAX_OP_STRING_LEN);

        /* 字符串比较时按照目标字符串长度比较，保证输入字符创长度不小于目标字符串长度，不会发生读越界 */
        if (srcStrNLen >= desStrNLen) {
            if (VOS_MemCmp(mmiStr, mmiPwdStrOpTbl[i].string, desStrNLen) == 0) {
                mmiOpParam->mmiOperationType = mmiPwdStrOpTbl[i].operationType;
                break;
            }
        }

        i++;
    }

    if (mmiPwdStrOpTbl[i].string == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    if (MMI_DecodeScAndSi(mmiStr + VOS_StrNLen((VOS_CHAR *)mmiPwdStrOpTbl[i].string, TAF_MMI_PWD_MAX_OP_STRING_LEN),
                          mmiOpParam, &scSiPara, ppOutRestMmiStr) == VOS_TRUE) {
        /* for a common password for all appropriate services, delete the ZZ */
        if (VOS_StrNLen((VOS_CHAR *)scSiPara.ssCode, MN_MMI_MAX_SC_LEN) == 0) {
            mmiOpParam->regPwdReq.ssCode = TAF_ALL_SS_CODE;
        } else {
            i = 0;

            while (i < MN_MMI_SC_MAX_ENTRY) {
                strNLen1 = VOS_StrNLen((VOS_CHAR *)scSiPara.ssCode, MN_MMI_MAX_SC_LEN);
                strNLen2 = VOS_StrNLen((VOS_CHAR *)g_mmiScInfo[i].mmiSc, TAF_MMI_SC_MAX_STRING_LEN);

                if (strNLen1 == strNLen2) {
                    if (VOS_MemCmp(g_mmiScInfo[i].mmiSc, scSiPara.ssCode, strNLen1) == 0) {
                        mmiOpParam->regPwdReq.ssCode = g_mmiScInfo[i].netSc;
                        break;
                    }
                }

                i++;
            }

            if (i >= MN_MMI_SC_MAX_ENTRY) {
                *errCode = MN_ERR_INVALIDPARM;
                return VOS_TRUE;
            }
        }

        (VOS_VOID)memset_s(mmiOpParam->regPwdReq.oldPwdStr, TAF_SS_MAX_PASSWORD_LEN + 1, 0, TAF_SS_MAX_PASSWORD_LEN + 1);
        (VOS_VOID)memset_s(mmiOpParam->regPwdReq.newPwdStr, TAF_SS_MAX_PASSWORD_LEN + 1, 0, TAF_SS_MAX_PASSWORD_LEN + 1);
        (VOS_VOID)memset_s(mmiOpParam->regPwdReq.newPwdStrCnf, TAF_SS_MAX_PASSWORD_LEN + 1, 0, TAF_SS_MAX_PASSWORD_LEN + 1);
        stringRet = strncpy_s((VOS_CHAR *)mmiOpParam->regPwdReq.oldPwdStr, TAF_SS_MAX_PASSWORD_LEN + 1,
                              (VOS_CHAR *)scSiPara.sia, VOS_StrNLen((VOS_CHAR *)scSiPara.sia, TAF_SS_MAX_PASSWORD_LEN));
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, TAF_SS_MAX_PASSWORD_LEN + 1,
                                        VOS_StrNLen((VOS_CHAR *)scSiPara.sia, TAF_SS_MAX_PASSWORD_LEN));
        stringRet = strncpy_s((VOS_CHAR *)mmiOpParam->regPwdReq.newPwdStr, TAF_SS_MAX_PASSWORD_LEN + 1,
                              (VOS_CHAR *)scSiPara.sib, VOS_StrNLen((VOS_CHAR *)scSiPara.sib, TAF_SS_MAX_PASSWORD_LEN));
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, TAF_SS_MAX_PASSWORD_LEN + 1,
                                        VOS_StrNLen((VOS_CHAR *)scSiPara.sib, TAF_SS_MAX_PASSWORD_LEN));
        stringRet = strncpy_s((VOS_CHAR *)mmiOpParam->regPwdReq.newPwdStrCnf, TAF_SS_MAX_PASSWORD_LEN + 1,
                              (VOS_CHAR *)scSiPara.sic, VOS_StrNLen((VOS_CHAR *)scSiPara.sic, TAF_SS_MAX_PASSWORD_LEN));
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, TAF_SS_MAX_PASSWORD_LEN + 1,
                                        VOS_StrNLen((VOS_CHAR *)scSiPara.sic, TAF_SS_MAX_PASSWORD_LEN));
        *errCode = MN_ERR_NO_ERROR;
    } else {
        *errCode = MN_ERR_INVALIDPARM;
    }
    (VOS_VOID)memset_s(&scSiPara, sizeof(MN_MMI_ScSiPara), 0x00, sizeof(MN_MMI_ScSiPara));
    return VOS_TRUE;
}

LOCAL VOS_BOOL MMI_JudgeTmpModeClirOp(VOS_CHAR *inMmiStr, VOS_CHAR **ppOutRestMmiStr, MN_MMI_OperationParam *mmiOpParam)
{
    VOS_UINT32             i               = 0;
    MN_MMI_StrOperationTbl mmiLiStrOpTbl[] = {
        { "*31#", TAF_MMI_SUPPRESS_CLIR, { 0, 0, 0, 0, 0, 0, 0 }},
        { "#31#", TAF_MMI_INVOKE_CLIR, { 0, 0, 0, 0, 0, 0, 0 }},
        { VOS_NULL_PTR, TAF_MMI_NULL_OPERATION, { 0, 0, 0, 0, 0, 0, 0 }}
    };

    VOS_UINT32 srcStrNLen;
    VOS_UINT32 desStrNLen;

    srcStrNLen = VOS_StrNLen((VOS_CHAR *)inMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    while (mmiLiStrOpTbl[i].string != VOS_NULL_PTR) {
        desStrNLen = VOS_StrNLen((VOS_CHAR *)mmiLiStrOpTbl[i].string, TAF_MMI_LI_MAX_OP_STRING_LEN);

        /* 字符串比较时按照目标字符串长度比较，保证输入字符创长度不小于目标字符串长度，不会发生读越界 */
        if (srcStrNLen >= desStrNLen) {
            if (VOS_MemCmp(inMmiStr, mmiLiStrOpTbl[i].string, desStrNLen) == 0) {
                mmiOpParam->mmiOperationType = mmiLiStrOpTbl[i].operationType;
                *ppOutRestMmiStr = inMmiStr + desStrNLen;
                return VOS_TRUE;
            }
        }

        i++;
    }

    return VOS_FALSE;
}

LOCAL VOS_BOOL MMI_JudgeImeiOperation(VOS_CHAR *pcInMmiStr, VOS_CHAR **ppcOutRestMmiStr,
                                      MN_MMI_OperationParam *mmiOpParam)
{
    VOS_CHAR *pcImeiStr = "*#06#";
    VOS_UINT32 srcStrNLen;
    VOS_UINT32 desStrNLen;

    srcStrNLen = VOS_StrNLen((VOS_CHAR *)pcInMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);
    desStrNLen = VOS_StrNLen((VOS_CHAR *)pcImeiStr, TAF_MMI_IMEI_MAX_LEN);

    if (srcStrNLen < desStrNLen) {
        return VOS_FALSE;
    }

    if (VOS_MemCmp(pcInMmiStr, pcImeiStr, desStrNLen) == 0) {
        mmiOpParam->mmiOperationType = TAF_MMI_DISPLAY_IMEI;
        *ppcOutRestMmiStr            = pcInMmiStr + VOS_StrNLen((VOS_CHAR *)pcImeiStr, TAF_MMI_IMEI_MAX_LEN);
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_BOOL MMI_JudgeUssdOperation(VOS_CHAR *pcMmiStr)
{
    VOS_UINT32 strLen;

    /* 判断依据 */
    /*
     * 3) #-string:
     * Input of the form.
     * "Entry of any characters defined in the 3GPP TS 23.038 [8] Default Alphabet
     * (up to the maximum defined in 3GPP TS 24.080 [10]), followed by #SEND".
     */
    strLen = VOS_StrNLen((VOS_CHAR *)pcMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    if ((strLen >= MN_MMI_MIN_USSD_LEN) && (pcMmiStr[strLen - 1] == MN_MMI_STOP_CHAR)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 MMI_TransMmiSsCodeToNetSsCode(const MN_MMI_ScSiPara *scSiPara, VOS_UINT8 *netSsCode)
{
    VOS_UINT32 i = 0;

    VOS_UINT32 strNLen1;
    VOS_UINT32 strNLen2;

    /* 转换SS Code */
    while (i < MN_MMI_SC_MAX_ENTRY) {
        strNLen1 = VOS_StrNLen((VOS_CHAR *)g_mmiScInfo[i].mmiSc, TAF_MMI_SC_MAX_STRING_LEN);
        strNLen2 = VOS_StrNLen((VOS_CHAR *)scSiPara->ssCode, MN_MMI_MAX_SC_LEN);

        if (strNLen1 == strNLen2) {
            if (VOS_MemCmp(g_mmiScInfo[i].mmiSc, scSiPara->ssCode, strNLen1) == 0) {
                *netSsCode = g_mmiScInfo[i].netSc;
                break;
            }
        }
        i++;
    }

    if (i >= MN_MMI_SC_MAX_ENTRY) {
        return VOS_ERR;
    }

    return VOS_OK;
}

LOCAL VOS_UINT32 MMI_TransMmiBsCodeToNetBsCode(VOS_UINT8 netSsCode, const MN_MMI_ScSiPara *scSiPara,
                                               VOS_UINT8 *netBsCode, VOS_UINT8 *netBsType

)
{
    VOS_CHAR   acBs[MN_MMI_MAX_SIA_LEN + 1];
    VOS_UINT32 i;
    VOS_UINT32 srcStrLen;
    VOS_UINT32 destStrLen;
    errno_t    memResult;

    /* 是不是需要设定那些需要调用此函数的限定? */

    (VOS_VOID)memset_s(acBs, sizeof(acBs), 0, sizeof(acBs));

    /* 这一段是转换对应的BS code */
    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE)) {
        if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) > 0) {
            memResult = memcpy_s(acBs, sizeof(acBs), scSiPara->sib,
                                 VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acBs), VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN));
        }
    } else if (netSsCode == TAF_CW_SS_CODE) {
        if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) > 0) {
            memResult = memcpy_s(acBs, sizeof(acBs), scSiPara->sia,
                                 VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acBs), VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN));
        }
    } else {
        acBs[0] = '\0';
    }

    i = 0;
    srcStrLen = VOS_StrNLen((VOS_CHAR *)acBs, MN_MMI_MAX_SIA_LEN);

    while (i < MN_MMI_BS_MAX_ENTRY) {
        destStrLen = VOS_StrNLen((VOS_CHAR *)g_mmiBSInfomation[i].mmiBs, TAF_MMI_BS_MAX_LEN);

        if (srcStrLen == destStrLen) {
            if (VOS_MemCmp(g_mmiBSInfomation[i].mmiBs, acBs, destStrLen) == 0) {
                *netBsCode = g_mmiBSInfomation[i].netBsCode;
                *netBsType = g_mmiBSInfomation[i].netBsType;
                break;
            }
        }

        i++;
    }

    if (i == MN_MMI_BS_MAX_ENTRY) {
        return VOS_ERR;
    }

    return VOS_OK;

    /* BS Code 的转换完成 */
}

VOS_UINT32 MMI_FillInRegisterSSPara(MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 netSsCode)
{
    TAF_SS_RegisterssReq *registerSsReq = VOS_NULL_PTR;
    errno_t               memResult;
    VOS_UINT8             netBsCode;
    VOS_UINT8             netBsType;

    registerSsReq = &mmiOpParam->registerSsReq;

    (VOS_VOID)memset_s(registerSsReq, sizeof(TAF_SS_RegisterssReq), 0x00, sizeof(TAF_SS_RegisterssReq));

    registerSsReq->ssCode = netSsCode;

    if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
        registerSsReq->opFwdToNum = 1;
        registerSsReq->opNumType  = 1;
        /* 如果是带'+'号，认为是国际号码，号码类型的值为0x91 */
        if (scSiPara->sia[0] == '+') {
            /* 去除头部的'+'号 */
            registerSsReq->numType = 0x91;

            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) != 0) {
                memResult = memcpy_s(registerSsReq->fwdToNum, sizeof(registerSsReq->fwdToNum), scSiPara->sia + 1,
                                     VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN));
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(registerSsReq->fwdToNum),
                                    VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN));
            }

        } else {
            registerSsReq->numType = 0x81;
            memResult = memcpy_s(registerSsReq->fwdToNum, sizeof(registerSsReq->fwdToNum), scSiPara->sia,
                                 VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) + 1);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(registerSsReq->fwdToNum),
                                VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) + 1);
        }

        if (scSiPara->sic[0] != '\0') {
            scSiPara->sic[MN_MMI_MAX_SIC_LEN] = '\0';
            registerSsReq->opNoRepCondTime = 1;
            registerSsReq->noRepCondTime   = (VOS_UINT8)MMI_AtoI(scSiPara->sic);
        }
    }

    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE) ||
        (netSsCode == TAF_CW_SS_CODE)) {
        if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else if (netSsCode == TAF_CW_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else {
            /* Delete TAF_ALL_BARRING_SS_CODE密码相关操作 */
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        }
        if (MMI_TransMmiBsCodeToNetBsCode(netSsCode, scSiPara, &netBsCode, &netBsType) != VOS_OK) {
            return MN_ERR_INVALIDPARM;
        }

        registerSsReq->opBsService             = 1;
        registerSsReq->bsService.bsServiceCode = netBsCode;
        registerSsReq->bsService.bsType        = netBsType;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MMI_FillInEraseSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                 VOS_UINT8 netSsCode)
{
    TAF_SS_ErasessReq *eraseSsReq = VOS_NULL_PTR;

    VOS_UINT8 netBsCode;
    VOS_UINT8 netBsType;

    eraseSsReq = &mmiOpParam->eraseSsReq;

    (VOS_VOID)memset_s(eraseSsReq, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));

    eraseSsReq->ssCode = netSsCode;

    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE) ||
        (netSsCode == TAF_CW_SS_CODE)) {
        if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else if (netSsCode == TAF_CW_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else {
            /* Delete TAF_ALL_BARRING_SS_CODE密码相关操作 */
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        }
        if (MMI_TransMmiBsCodeToNetBsCode(netSsCode, scSiPara, &netBsCode, &netBsType) != VOS_OK) {
            return MN_ERR_INVALIDPARM;
        }

        eraseSsReq->opBsService             = 1;
        eraseSsReq->bsService.bsServiceCode = netBsCode;
        eraseSsReq->bsService.bsType        = netBsType;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MMI_FillInActivateSSPara(MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 netSsCode)
{
    TAF_SS_ActivatessReq *activateSsReq = VOS_NULL_PTR;
    VOS_UINT8             netBsCode;
    VOS_UINT8             netBsType;
    VOS_UINT32            passwordLen;
    errno_t               memResult;

    /*
     * 呼叫转移业务注册前缀是*且存在参数A情况需要按注册处理
     * 参考协议3GPP 22030 6.5.2 struct of MMI
     */
    if ((netSsCode & TAF_SS_CODE_MASK) == TAF_ALL_FORWARDING_SS_CODE) {
        if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) != 0) {
            mmiOpParam->mmiOperationType = TAF_MMI_REGISTER_SS;
            return MMI_FillInRegisterSSPara(scSiPara, mmiOpParam, netSsCode);
        }
    }

    activateSsReq = &mmiOpParam->activateSsReq;

    (VOS_VOID)memset_s(activateSsReq, sizeof(TAF_SS_ActivatessReq), 0x00, sizeof(TAF_SS_ActivatessReq));

    activateSsReq->ssCode = netSsCode;

    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE) ||
        (netSsCode == TAF_CW_SS_CODE)) {
        if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else if (netSsCode == TAF_CW_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else {
            /* 保存密码到激活消息参数结构 */
            passwordLen = VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN);
            if (passwordLen != TAF_SS_MAX_PASSWORD_LEN) {
                return MN_ERR_INVALIDPARM;
            }

            memResult = memcpy_s(mmiOpParam->activateSsReq.password, sizeof(mmiOpParam->activateSsReq.password),
                                 scSiPara->sia, TAF_SS_MAX_PASSWORD_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mmiOpParam->activateSsReq.password), TAF_SS_MAX_PASSWORD_LEN);
            mmiOpParam->activateSsReq.opPassword = VOS_TRUE;

            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        }

        if (MMI_TransMmiBsCodeToNetBsCode(netSsCode, scSiPara, &netBsCode, &netBsType) != VOS_OK) {
            return MN_ERR_INVALIDPARM;
        }

        activateSsReq->opBsService             = 1;
        activateSsReq->bsService.bsServiceCode = netBsCode;
        activateSsReq->bsService.bsType        = netBsType;
    }

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MMI_FillInDeactivateCCBSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                              VOS_UINT8 netSsCode)
{
    TAF_SS_EraseccEntryReq *ccbsEraseReq = VOS_NULL_PTR;
    VOS_UINT32              ccbsIndex;

    ccbsEraseReq = &mmiOpParam->ccbsEraseReq;

    (VOS_VOID)memset_s(ccbsEraseReq, sizeof(TAF_SS_EraseccEntryReq), 0x00, sizeof(TAF_SS_EraseccEntryReq));
    ccbsIndex = MMI_AtoI(scSiPara->sia);
    if (ccbsIndex != 0) {
        if ((ccbsIndex > 5) || (ccbsIndex < 1)) {
            return MN_ERR_INVALIDPARM;
        } else {
            ccbsEraseReq->opCcbsIndex = VOS_TRUE;
            ccbsEraseReq->ccbsIndex   = (TAF_UINT8)(ccbsIndex);
        }
    }
    ccbsEraseReq->ssCode = netSsCode;

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MMI_FillInDeactivateSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                      VOS_UINT8 netSsCode)
{
    TAF_SS_DeactivatessReq *deactivateSsReq = VOS_NULL_PTR;
    VOS_UINT8               netBsCode;
    VOS_UINT8               netBsType;
    VOS_UINT32              passwordLen;
    errno_t                 memResult;

    deactivateSsReq = &mmiOpParam->deactivateSsReq;

    (VOS_VOID)memset_s(deactivateSsReq, sizeof(TAF_SS_DeactivatessReq), 0x00, sizeof(TAF_SS_DeactivatessReq));

    deactivateSsReq->ssCode = netSsCode;

    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE) ||
        (netSsCode == TAF_CW_SS_CODE)) {
        if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else if (netSsCode == TAF_CW_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else {
            /* 保存密码到去激活消息参数结构 */
            passwordLen = VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN);
            if (passwordLen != TAF_SS_MAX_PASSWORD_LEN) {
                return MN_ERR_INVALIDPARM;
            }

            memResult = memcpy_s(mmiOpParam->deactivateSsReq.password, sizeof(mmiOpParam->deactivateSsReq.password),
                                 scSiPara->sia, TAF_SS_MAX_PASSWORD_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mmiOpParam->deactivateSsReq.password), TAF_SS_MAX_PASSWORD_LEN);
            mmiOpParam->deactivateSsReq.opPassword = VOS_TRUE;

            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        }
        if (MMI_TransMmiBsCodeToNetBsCode(netSsCode, scSiPara, &netBsCode, &netBsType) != VOS_OK) {
            return MN_ERR_INVALIDPARM;
        }

        deactivateSsReq->opBsService             = 1;
        deactivateSsReq->bsService.bsServiceCode = netBsCode;
        deactivateSsReq->bsService.bsType        = netBsType;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MMI_FillInInterrogateSSPara(const MN_MMI_ScSiPara *scSiPara, MN_MMI_OperationParam *mmiOpParam,
                                       VOS_UINT8 netSsCode)
{
    TAF_SS_InterrogatessReq *interrogateSsReq = VOS_NULL_PTR;
    VOS_UINT8                netBsCode;
    VOS_UINT8                netBsType;

    interrogateSsReq = &mmiOpParam->interrogateSsReq;

    (VOS_VOID)memset_s(interrogateSsReq, sizeof(TAF_SS_InterrogatessReq), 0x00, sizeof(TAF_SS_InterrogatessReq));

    interrogateSsReq->ssCode = netSsCode;

    if (((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) || ((netSsCode & 0xF0) == TAF_ALL_BARRING_SS_CODE) ||
        (netSsCode == TAF_CW_SS_CODE)) {
        if ((netSsCode & 0xF0) == TAF_ALL_FORWARDING_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else if (netSsCode == TAF_CW_SS_CODE) {
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sia, MN_MMI_MAX_SIA_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        } else {
            /* Delete TAF_MMI_GET_PASSWD密码相关操作 */
            if (VOS_StrNLen((VOS_CHAR *)scSiPara->sib, MN_MMI_MAX_SIB_LEN) == 0) {
                return MN_ERR_NO_ERROR;
            }
        }
        if (MMI_TransMmiBsCodeToNetBsCode(netSsCode, scSiPara, &netBsCode, &netBsType) != VOS_OK) {
            return MN_ERR_INVALIDPARM;
        }

        interrogateSsReq->opBsService             = 1;
        interrogateSsReq->bsService.bsServiceCode = netBsCode;
        interrogateSsReq->bsService.bsType        = netBsType;
    }

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MMI_FillInProcessUssdReqPara(VOS_CHAR *pcInMmiStr, VOS_CHAR **ppcOutRestMmiStr,
                                        MN_MMI_OperationParam *mmiOpParam)
{
    TAF_SS_ProcessUssReq *processUssdReq = VOS_NULL_PTR;
    VOS_UINT32            strLen;
    errno_t               memResult;

    processUssdReq = &mmiOpParam->processUssdReq;

    strLen = VOS_StrNLen((VOS_CHAR *)pcInMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);
    if (strLen > TAF_SS_MAX_USSDSTRING_LEN) {
        return MN_ERR_INVALIDPARM;
    }

    (VOS_VOID)memset_s(processUssdReq, sizeof(TAF_SS_ProcessUssReq), 0x00, sizeof(TAF_SS_ProcessUssReq));

    /* pstProcessUssdReq->DatacodingScheme = pstMmiOpParam->ProcessUssdReq.DatacodingScheme 这个赋值没有意义 */
    processUssdReq->ussdStr.cnt = (VOS_UINT8)VOS_StrNLen((VOS_CHAR *)pcInMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    /* pstProcessUssdReq->UssdStr.usCnt 不可能大于 TAF_SS_MAX_USSDSTRING_LEN*2 */

    if (processUssdReq->ussdStr.cnt > 0) {
        memResult = memcpy_s(processUssdReq->ussdStr.ussdStr, sizeof(processUssdReq->ussdStr.ussdStr), pcInMmiStr,
                             processUssdReq->ussdStr.cnt);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(processUssdReq->ussdStr.ussdStr), processUssdReq->ussdStr.cnt);
    }

    *ppcOutRestMmiStr = pcInMmiStr + VOS_StrNLen((VOS_CHAR *)pcInMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    return MN_ERR_NO_ERROR;
}

LOCAL VOS_UINT32 MMI_FillInCallOrigPara(VOS_CHAR *pcMmiStr, MN_MMI_OperationParam *mmiOpParam,
                                        VOS_CHAR **ppOutRestMmiStr)
{
    VOS_UINT32 rslt;

    /* 输入的字串不能超过语音呼叫允许的最大长度 */
    if (VOS_StrNLen((VOS_CHAR *)pcMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN) > (MN_CALL_MAX_BCD_NUM_LEN * 2)) {
        return MN_ERR_INVALIDPARM;
    }

    if (pcMmiStr[0] == '+') {
        /* 将Ascii码转换成对应的BCD码 */
        rslt = TAF_STD_ConvertAsciiNumberToBcd(pcMmiStr + 1, mmiOpParam->mnCallOrig.dialNumber.bcdNum,
                                               &mmiOpParam->mnCallOrig.dialNumber.numLen);
        mmiOpParam->mnCallOrig.dialNumber.numType = (VOS_UINT8)(0x80 | (MN_CALL_TON_INTERNATIONAL << 4) |
                                                                MN_CALL_NPI_ISDN);
    } else {
        /* 将Ascii码转换成对应的BCD码 */
        rslt = TAF_STD_ConvertAsciiNumberToBcd(pcMmiStr, mmiOpParam->mnCallOrig.dialNumber.bcdNum,
                                               &mmiOpParam->mnCallOrig.dialNumber.numLen);
        mmiOpParam->mnCallOrig.dialNumber.numType = (VOS_UINT8)(0x80 | (MN_CALL_TON_UNKNOWN << 4) | MN_CALL_NPI_ISDN);
    }

    if (rslt != MN_ERR_NO_ERROR) {
        return rslt;
    }

    /* 填写其他需要的参数 */
    mmiOpParam->mmiOperationType    = TAF_MMI_CALL_ORIG;
    mmiOpParam->mnCallOrig.callType = MN_CALL_TYPE_VOICE;

    mmiOpParam->mnCallOrig.voiceDomain = TAF_CALL_VOICE_DOMAIN_AUTO;

    mmiOpParam->mnCallOrig.clirCfg       = g_clirOperate;
    mmiOpParam->mnCallOrig.cugCfg.enable = VOS_FALSE;
    mmiOpParam->mnCallOrig.callMode      = MN_CALL_MODE_SINGLE;

    *ppOutRestMmiStr = pcMmiStr + VOS_StrNLen((VOS_CHAR *)pcMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    return MN_ERR_NO_ERROR;
}

VOS_VOID MMI_JudgeMmiOperationType(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam, MN_MMI_ScSiPara *scSiPara,
                                   VOS_CHAR **ppOutRestMmiStr, MN_ERROR_NetSsCode *errNetSsCodeInfo)
{
    switch (mmiOpParam->mmiOperationType) {
        case TAF_MMI_REGISTER_SS:
            errNetSsCodeInfo->errCode = MMI_FillInRegisterSSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        case TAF_MMI_ERASE_SS:
            errNetSsCodeInfo->errCode = MMI_FillInEraseSSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        case TAF_MMI_ACTIVATE_SS:
            errNetSsCodeInfo->errCode = MMI_FillInActivateSSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        case TAF_MMI_DEACTIVATE_SS:
            errNetSsCodeInfo->errCode = MMI_FillInDeactivateSSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        case TAF_MMI_INTERROGATE_SS:
        case TAF_MMI_INTERROGATE_CCBS:
            errNetSsCodeInfo->errCode = MMI_FillInInterrogateSSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        case TAF_MMI_PROCESS_USSD_REQ:
            errNetSsCodeInfo->errCode = MMI_FillInProcessUssdReqPara(inMmiStr, ppOutRestMmiStr, mmiOpParam);
            break;

        case TAF_MMI_DEACTIVATE_CCBS:
            errNetSsCodeInfo->errCode = MMI_FillInDeactivateCCBSPara(scSiPara, mmiOpParam, errNetSsCodeInfo->netSsCode);
            break;

        default:
            break;
    }
}

VOS_BOOL MMI_MatchSsOpTbl(VOS_CHAR *inMmiStr, MN_MMI_OperationParam *mmiOpParam, MN_MMI_ScSiPara *scSiPara,
                          VOS_CHAR **ppOutRestMmiStr, MN_ERROR_NetSsCode *errNetSsCodeInfo)
{
    /* 获取特性控制NV地址 */

    if (MMI_DecodeScAndSi(inMmiStr, mmiOpParam, scSiPara, ppOutRestMmiStr) == VOS_FALSE) {
        errNetSsCodeInfo->errCode = MN_ERR_INVALIDPARM;
        return VOS_FALSE;
    }

    if (MMI_TransMmiSsCodeToNetSsCode(scSiPara, &(errNetSsCodeInfo->netSsCode)) != VOS_OK) {
        if (MMI_JudgeUssdOperation(inMmiStr) == VOS_FALSE) {
            errNetSsCodeInfo->errCode = MN_ERR_INVALIDPARM;
            return VOS_FALSE;
        }
        mmiOpParam->mmiOperationType = TAF_MMI_PROCESS_USSD_REQ;
    }

    if ((mmiOpParam->mmiOperationType == TAF_MMI_DEACTIVATE_SS) &&
        ((errNetSsCodeInfo->netSsCode) == TAF_CCBS_A_SS_CODE)) {
        mmiOpParam->mmiOperationType = TAF_MMI_DEACTIVATE_CCBS;
    } else if ((mmiOpParam->mmiOperationType == TAF_MMI_INTERROGATE_SS) &&
               ((errNetSsCodeInfo->netSsCode) == TAF_CCBS_A_SS_CODE)) {
        mmiOpParam->mmiOperationType = TAF_MMI_INTERROGATE_CCBS;
    } else {

    }

    return VOS_TRUE;
}

VOS_BOOL MMI_JudgeSsOperation(VOS_CHAR *inMmiStr, VOS_CHAR **ppOutRestMmiStr, MN_MMI_OperationParam *mmiOpParam,
                              MN_ERROR_NetSsCode *errorNeTSsCodeInfo)
{
    VOS_BOOL                bMatch = VOS_FALSE;
    VOS_CHAR                acOpType[TAF_MMI_SS_MAX_OP_TYPE_LEN];
    MN_MMI_ScSiPara         scSiPara;
    VOS_UINT32              i = 0;
    VOS_UINT32              tableSize;
    const MN_MMI_SsOpTbl   *operationType = VOS_NULL_PTR;
    VOS_UINT32              srcStrLen;
    VOS_UINT32              destStrLen;

    errorNeTSsCodeInfo->errCode   = MN_ERR_NO_ERROR;
    errorNeTSsCodeInfo->netSsCode = TAF_ALL_SS_CODE;

    (VOS_VOID)memset_s(acOpType, sizeof(acOpType), 0x00, sizeof(acOpType));

    for (i = 0; (!MN_MMI_isdigit(inMmiStr[i])) && (i < 2); i++) {
        acOpType[i] = inMmiStr[i];
    }

    srcStrLen = VOS_StrNLen((VOS_CHAR *)acOpType, TAF_MMI_SS_MAX_OP_TYPE_LEN);
    tableSize     = MMI_GetOporationTypeTblSize();
    operationType = MMI_GetOporationTypeTblAddr();

    for (i = 0; i < tableSize; i++) {
        destStrLen = VOS_StrNLen((VOS_CHAR *)operationType->ssOpStr, TAF_MMI_SS_MAX_OP_STRING_LEN);

        if (srcStrLen == destStrLen) {
            if (VOS_MemCmp(operationType->ssOpStr, acOpType, destStrLen) == 0) {
                mmiOpParam->mmiOperationType = operationType->ssOpType;
                bMatch                       = VOS_TRUE;
                break;
            }
        }

        operationType++;
    }

    if (bMatch == VOS_FALSE) {
        if (MMI_JudgeUssdOperation(inMmiStr) == VOS_FALSE) {
            return VOS_FALSE;
        }
        mmiOpParam->mmiOperationType = TAF_MMI_PROCESS_USSD_REQ;
    } else {
        if (MMI_MatchSsOpTbl(inMmiStr, mmiOpParam, &scSiPara, ppOutRestMmiStr, errorNeTSsCodeInfo) == VOS_FALSE) {
            return VOS_FALSE;
        }
    }
    MMI_JudgeMmiOperationType(inMmiStr, mmiOpParam, &scSiPara, ppOutRestMmiStr, errorNeTSsCodeInfo);

    return VOS_TRUE;
}

LOCAL VOS_BOOL MMI_JudgeChldOperation(const VOS_CHAR *pcInMmiStr, MN_MMI_OperationParam *mmiOpParam,
                                      VOS_UINT32 *errCode)
{
    VOS_UINT32       i           = 0;
    MN_MMI_ChldOpTbl chldOpTbl[] = {
        { "0", MN_CALL_SUPS_CMD_REL_HELD_OR_UDUB, { 0, 0, 0, 0, 0, 0, 0 }},
        { "1", MN_CALL_SUPS_CMD_REL_ACT_ACPT_OTH, { 0, 0, 0, 0, 0, 0, 0 }},
        { "10", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "11", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "12", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "13", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "14", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "15", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "16", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "17", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "18", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "19", MN_CALL_SUPS_CMD_REL_CALL_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "2", MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH, { 0, 0, 0, 0, 0, 0, 0 }},
        { "20", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "21", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "22", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "23", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "24", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "25", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "26", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "27", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "28", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "29", MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X, { 0, 0, 0, 0, 0, 0, 0 }},
        { "3", MN_CALL_SUPS_CMD_BUILD_MPTY, { 0, 0, 0, 0, 0, 0, 0 }},
        { "4", MN_CALL_SUPS_CMD_ECT, { 0, 0, 0, 0, 0, 0, 0 }},
        { "4*", MN_CALL_SUPS_CMD_DEFLECT_CALL, { 0, 0, 0, 0, 0, 0, 0 }},
        { "5", MN_CALL_SUPS_CMD_ACT_CCBS, { 0, 0, 0, 0, 0, 0, 0 }},
        { VOS_NULL_PTR, 0, { 0, 0, 0, 0, 0, 0, 0 }}
    };
    VOS_UINT32 srcStrLen;
    VOS_UINT32 destStrLen;

    *errCode = MN_ERR_NO_ERROR;

    srcStrLen = VOS_StrNLen((VOS_CHAR *)pcInMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);

    while (chldOpTbl[i].mmiChldStr != VOS_NULL_PTR) {
        destStrLen = VOS_StrNLen((VOS_CHAR *)chldOpTbl[i].mmiChldStr, TAF_MMI_CHLD_MAX_STRING_LEN);

        if (srcStrLen == destStrLen) {
            if (VOS_MemCmp(pcInMmiStr, chldOpTbl[i].mmiChldStr, destStrLen) == 0) {
                break;
            }
        }

        i++;
    }

    if (chldOpTbl[i].mmiChldStr == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    mmiOpParam->mmiOperationType = TAF_MMI_CALL_CHLD_REQ;

    switch (chldOpTbl[i].chldOpType) {
        case MN_CALL_SUPS_CMD_REL_HELD_OR_UDUB:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_REL_HELD_OR_UDUB;
            break;

        case MN_CALL_SUPS_CMD_REL_ACT_ACPT_OTH:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_REL_ACT_ACPT_OTH;
            break;

        case MN_CALL_SUPS_CMD_REL_CALL_X:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_REL_CALL_X;
            mmiOpParam->mnCallSupsReq.callId      = (VOS_UINT8)(pcInMmiStr[1] - '0');
            break;

        case MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH;
            break;

        case MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X;
            mmiOpParam->mnCallSupsReq.callId      = (VOS_UINT8)(pcInMmiStr[1] - '0');
            break;

        case MN_CALL_SUPS_CMD_BUILD_MPTY:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_BUILD_MPTY;
            break;

        case MN_CALL_SUPS_CMD_ECT:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_ECT;
            break;

        case MN_CALL_SUPS_CMD_DEFLECT_CALL:
            mmiOpParam->mnCallSupsReq.callSupsCmd = MN_CALL_SUPS_CMD_DEFLECT_CALL;
            break;

        default:
            /* 认为是CCBS,因为目前不支持CCBS,暂时没写处理代码 */
            *errCode = MN_ERR_INVALIDPARM;
            break;
    }

    return VOS_TRUE;
}

MODULE_EXPORTED TAF_UINT32 MN_MmiStringParse(TAF_CHAR *inMmiStr, TAF_BOOL inCall, MN_MMI_OperationParam *mmiOpParam,
                                             TAF_CHAR **ppOutRestMmiStr)
{
    TAF_UINT32         rslt = MN_ERR_NO_ERROR;
    TAF_UINT32         rtrnRslt;
    TAF_UINT32         strLen;
    MN_ERROR_NetSsCode errorNeTSsCodeInfo = {0};

    if (inMmiStr == VOS_NULL_PTR) {
        MN_ERR_LOG("MN_MmiStringParse: Input Invalid Param");
        return MN_ERR_INVALIDPARM;
    }

    strLen = VOS_StrNLen((VOS_CHAR *)inMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);
    if ((strLen == 0) || ((inCall != VOS_TRUE) && (inCall != VOS_FALSE)) || (mmiOpParam == VOS_NULL_PTR) ||
        (ppOutRestMmiStr == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MmiStringParse: Input Invalid Param");
        return MN_ERR_INVALIDPARM;
    }

    (VOS_VOID)memset_s(mmiOpParam, sizeof(MN_MMI_OperationParam), 0x00, sizeof(MN_MMI_OperationParam));

    /* 判断当前是不是显示IMEI操作 */
    if (MMI_JudgeImeiOperation(inMmiStr, ppOutRestMmiStr, mmiOpParam) == VOS_TRUE) {
        return MN_ERR_NO_ERROR;
    }

    /* 判断当前是不是临时模式下抑制或者激活CLIR操作 */
    if (MMI_JudgeTmpModeClirOp(inMmiStr, ppOutRestMmiStr, mmiOpParam) == VOS_TRUE) {
        /* 作为独立的解码函数，不应与AT模块的业务功能耦合，删除AT模块业务全局变量的赋值操作 */
        return MN_ERR_NO_ERROR;
    }

    /* 判断当前的操作类型是不是PIN操作类型 */
    if (MMI_JudgePinOperation(inMmiStr, mmiOpParam, ppOutRestMmiStr, &rtrnRslt) == VOS_TRUE) {
        return rtrnRslt;
    }

    /* 判断当前的操作类型是不是修改密码操作 */
    if (MMI_JudgePwdOperation(inMmiStr, mmiOpParam, ppOutRestMmiStr, &rtrnRslt) == VOS_TRUE) {
        return rtrnRslt;
    }

    /* 判断当前的操作类型是不是其他已知的呼叫无关补充业务操作 */
    if (MMI_JudgeSsOperation(inMmiStr, ppOutRestMmiStr, mmiOpParam, &errorNeTSsCodeInfo) == VOS_TRUE) {
        return errorNeTSsCodeInfo.errCode;
    }

    /*
     * 对于短字串的处理，根据协议22.030中的规定:
     * "Entry of 1 or 2 characters defined in the 3GPP TS 23.038 [8] Default Alphabet followed by SEND"
     * 以及22.030中的对应的流程图处理
     */
    if ((strLen == 2) || (strLen == 1)) {
        if (inCall == VOS_TRUE) {
            /* 判定是不是CHLD命令 */
            if (MMI_JudgeChldOperation(inMmiStr, mmiOpParam, &rtrnRslt) == VOS_TRUE) {
                *ppOutRestMmiStr = inMmiStr + VOS_StrNLen((VOS_CHAR *)inMmiStr, TAF_SS_MAX_UNPARSE_PARA_LEN);
                return rtrnRslt;
            }

            /* 如果不是CHLD命令，那么认为是USSD操作，填写相应的参数 */
            mmiOpParam->mmiOperationType = TAF_MMI_PROCESS_USSD_REQ;
            return MMI_FillInProcessUssdReqPara(inMmiStr, ppOutRestMmiStr, mmiOpParam);
        } else {
            /* 在这种情况下，如果是'1'开头，那么认为应该发起一个呼叫 */
            if (inMmiStr[0] == '1') {
                rslt = MMI_FillInCallOrigPara(inMmiStr, mmiOpParam, ppOutRestMmiStr);
                if (rslt != MN_ERR_NO_ERROR) {
                    return rslt;
                }
                return MN_ERR_NO_ERROR;
            } else {
                /* 如果不是'1'开头，那么也是作为USSD字串来处理 */
                mmiOpParam->mmiOperationType = TAF_MMI_PROCESS_USSD_REQ;
                return MMI_FillInProcessUssdReqPara(inMmiStr, ppOutRestMmiStr, mmiOpParam);
            }
        }
    }

    return MMI_FillInCallOrigPara(inMmiStr, mmiOpParam, ppOutRestMmiStr);

    /*
     * 先删除，对于字符'P','W'的处理，是否放在这里，尚待讨论，
     * 暂时不支持，
     */
}

