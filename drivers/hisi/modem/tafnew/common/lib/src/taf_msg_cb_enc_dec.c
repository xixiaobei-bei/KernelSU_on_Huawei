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

#include "product_config.h"
#include "PsTypeDef.h"
#include "ps_common_def.h"
#include "mn_error_code.h"
#include "mn_msg_api.h"
#include "mnmsgcbencdec.h"
#include "taf_msg_ts.h"
#include "taf_std_lib.h"
#include "securec.h"


#define THIS_FILE_ID PS_FILE_ID_MNMSG_CB_ENCDEC_C

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))

#define MSG_CBPAGE_HEADER_LEN 6

typedef struct {
    MN_MSG_Iso639LangUint16 enIso639Lang;
    VOS_UINT8               aucReserve1[1];
    MN_MSG_CblangUint8      enLang;
} MN_MSG_LANG_CONVERT_STRU;

LOCAL MN_MSG_LANG_CONVERT_STRU g_msgCbLangTable[MN_MSG_MAX_LANG_NUM] = {
    { MN_MSG_ISO639_LANG_GERMAN, {0}, MN_MSG_CBLANG_GERMAN },
    { MN_MSG_ISO639_LANG_ENGLISH, {0}, MN_MSG_CBLANG_ENGLISH },
    { MN_MSG_ISO639_LANG_ITALIAN, {0}, MN_MSG_CBLANG_ITALIAN },
    { MN_MSG_ISO639_LANG_FRENCH, {0}, MN_MSG_CBLANG_FRENCH },
    { MN_MSG_ISO639_LANG_SPANISH, {0}, MN_MSG_CBLANG_SPANISH },
    { MN_MSG_ISO639_LANG_DUTCH, {0}, MN_MSG_CBLANG_DUTCH },
    { MN_MSG_ISO639_LANG_SWEDISH, {0}, MN_MSG_CBLANG_SWEDISH },
    { MN_MSG_ISO639_LANG_DANISH, {0}, MN_MSG_CBLANG_DANISH },
    { MN_MSG_ISO639_LANG_PORTUGUESE, {0}, MN_MSG_CBLANG_PORTUGUESE },
    { MN_MSG_ISO639_LANG_FINNISH, {0}, MN_MSG_CBLANG_FINNISH },
    { MN_MSG_ISO639_LANG_NORWEGIAN, {0}, MN_MSG_CBLANG_NORWEGIAN },
    { MN_MSG_ISO639_LANG_GREEK, {0}, MN_MSG_CBLANG_GREEK },
    { MN_MSG_ISO639_LANG_TURKISH, {0}, MN_MSG_CBLANG_TURKISH },
    { MN_MSG_ISO639_LANG_HUNGARIAN, {0}, MN_MSG_CBLANG_HUNGARIAN },
    { MN_MSG_ISO639_LANG_POLISH, {0}, MN_MSG_CBLANG_POLISH },
    { MN_MSG_ISO639_LANG_CZECH, {0}, MN_MSG_CBLANG_CZECH },
    { MN_MSG_ISO639_LANG_HEBREW, {0}, MN_MSG_CBLANG_HEBREW },
    { MN_MSG_ISO639_LANG_ARABIC, {0}, MN_MSG_CBLANG_ARABIC },
    { MN_MSG_ISO639_LANG_RUSSIAN, {0}, MN_MSG_CBLANG_RUSSIAN },
    { MN_MSG_ISO639_LANG_ICELANDIC, {0}, MN_MSG_CBLANG_ICELANDIC }
};

MN_MSG_CblangUint8 MN_MSG_Iso639LangToDef(MN_MSG_Iso639LangUint16 iso639Lang)
{
    VOS_UINT32         loop;
    MN_MSG_CblangUint8 lang = MN_MSG_CBLANG_MAX;

    for (loop = 0; loop < MN_MSG_MAX_LANG_NUM; loop++) {
        if (iso639Lang == g_msgCbLangTable[loop].enIso639Lang) {
            lang = g_msgCbLangTable[loop].enLang;
            break;
        }
    }

    return lang;
}

VOS_UINT32 MN_MSG_DecodeDcsIf01(VOS_UINT8 dcs, VOS_UINT8 *content, VOS_UINT32 contentLength, MN_MSG_CbdcsCode *dcsInfo)
{
    errno_t                 memResult;
    VOS_UINT32              ret;
    VOS_UINT8               lang[TAF_MSG_CBA_LANG_LENGTH];
    MN_MSG_Iso639LangUint16 enlang;

    memset_s(lang, sizeof(lang), 0x00, sizeof(lang));

    ret = MN_ERR_NO_ERROR;

    switch (dcs & 0x0F) {
        /*
         * 23038 5 CBS Data Coding Scheme
         * 0000 GSM 7 bit default alphabet; message preceded by language indication.
         * The first 3 characters of the message are a two-character representation of the
         * language encoded according to ISO 639 [12], followed by a CR character. The
         * CR character is then followed by 90 characters of text.
         */
        case 0:
            /*
             * 7 Bit编码 language Represetation Occupy 3 7bit，至少3个字节
             * 1) 前2个7BIT是语言
             * 2) 最后一个字节时<CR>
             */
            if (contentLength <= TAF_MSG_CBA_LANG_LENGTH) {
                MN_WARN_LOG("MN_MSG_DecodeDcsIf01: Invalid ulContentLength.");
                return MN_ERR_CLASS_SMS_INVALID_MSG_LANG;
            }

            dcsInfo->msgCoding  = MN_MSG_MSG_CODING_7_BIT;
            dcsInfo->langIndLen = TAF_MSG_CBA_LANG_LENGTH + 1;

            /* 根据消息内容的前两个字符得到CBS语言编码LangCode */
            ret = TAF_STD_UnPack7Bit(content, TAF_MSG_CBA_LANG_LENGTH, 0, lang);
            if (ret != VOS_OK) {
                MN_WARN_LOG("MN_MSG_DecodeDcsIf01: TAF_STD_UnPack7Bit Err.");
                return MN_ERR_CLASS_SMS_INVALID_MSG_LANG;
            }

            break;

            /*
             * 23038 5 CBS Data Coding Scheme
             * 0001 UCS2; message preceded by language indication
             * The message starts with a two GSM 7-bit default alphabet character
             * representation of the language encoded according to ISO 639 [12]. This is padded
             * to the octet boundary with two bits set to 0 and then followed by 40 characters of
             * UCS2-encoded message.
             * An MS not supporting UCS2 coding will present the two character language
             * identifier followed by improperly interpreted user data.
             */

        case 1:
            /* UCS2 language Represetation Occupy 2 8bit */
            if (contentLength < TAF_MSG_CBA_LANG_LENGTH) {
                MN_WARN_LOG("MN_MSG_DecodeDcsIf01: Invalid ulContentLength.");
                return MN_ERR_CLASS_SMS_INVALID_MSG_LANG;
            }

            dcsInfo->msgCoding  = MN_MSG_MSG_CODING_UCS2;
            dcsInfo->langIndLen = TAF_MSG_CBA_LANG_LENGTH;

            memResult = memcpy_s(lang, sizeof(lang), content, dcsInfo->langIndLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(lang), dcsInfo->langIndLen);

            break;

        default:
            /* Reserved selection we don't support; */
            /* 记录错误Trace               设置返回值 */
            ret = MN_ERR_CLASS_SMS_INVALID_MSG_CODING;
            return ret;
    }

    enlang           = ((lang[0] << 8) | lang[1]);
    dcsInfo->msgLang = MN_MSG_Iso639LangToDef(enlang);

    return ret;
}

LOCAL VOS_UINT32 MN_MSG_DecodeDcsIf07(VOS_UINT8 dcs, MN_MSG_CbdcsCode *dcsInfo)
{
    /*
     * General Data Coding indication
     * Bits 5..0 indicate the following:
     * Bit 5, if set to 0, indicates the text is uncompressed
     * Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in
     * 3GPP TS 23.042 [13]
     * Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
     * Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
     * Bit 1 Bit 0 Message Class:
     * 0 0 Class 0
     * 0 1 Class 1 Default meaning: ME-specific.
     * 1 0 Class 2 (U)SIM specific message.
     * 1 1 Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
     * Bits 3 and 2 indicate the character set being used, as follows:
     * Bit 3 Bit 2 Character set:
     * 0 0 GSM 7 bit default alphabet
     * 0 1 8 bit data
     * 1 0 UCS2 (16 bit) [10]
     * 1 1 Reserved
     */

    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    if ((dcs & 0x20) == 0x20) {
        dcsInfo->compressed = VOS_TRUE;
    } else {
        dcsInfo->compressed = VOS_FALSE;
    }
    switch ((dcs & 0x0C) >> 2) {
        /* bit2,3，具体为编码方式 */
        case 0:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_7_BIT;
            break;

        case 1:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_8_BIT;
            break;

        case 2:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_UCS2;
            break;

        default:
            MN_WARN_LOG("MSG_CbDecodeDcs: Invalid message coding.");
            ret = MN_ERR_CLASS_SMS_INVALID_MSG_CODING;
            break;
    }

    /* 判断bit4的值，0表示无Class含义，1表示有Class含义 */
    if ((dcs & 0x10) == 0) {
        dcsInfo->msgClass = MN_MSG_MSG_CLASS_NONE;
    } else {
        dcsInfo->msgClass = (MN_MSG_MsgClassUint8)(dcs & 0x3);
    }

    return ret;
}

LOCAL VOS_UINT32 MN_MSG_DecodeDcsIf09(VOS_UINT8 dcs, MN_MSG_CbdcsCode *dcsInfo)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    switch ((dcs & 0x0C) >> 2) {
        /* bit2,3，具体为编码方式 */
        case 0:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_7_BIT;
            break;
        case 1:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_8_BIT;
            break;
        case 2:
            dcsInfo->msgCoding = MN_MSG_MSG_CODING_UCS2;
            break;
        default:
            MN_WARN_LOG("MSG_CbDecodeDcs: Invalid message coding.");
            ret = MN_ERR_CLASS_SMS_INVALID_MSG_CODING;
            break;
    }

    dcsInfo->msgClass = (MN_MSG_MsgClassUint8)(dcs & 0x3);

    return ret;
}

LOCAL VOS_UINT32 MN_MSG_DecodeDcsIf0F(VOS_UINT8 dcs, MN_MSG_CbdcsCode *dcsInfo)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    if ((dcs & 0x04) == 0) {
        dcsInfo->msgCoding = MN_MSG_MSG_CODING_7_BIT;
    } else {
        dcsInfo->msgCoding = MN_MSG_MSG_CODING_8_BIT;
    }

    switch (dcs & 0x3) {
        case 0:
            dcsInfo->msgClass = MN_MSG_MSG_CLASS_NONE;
            break;
        default:
            dcsInfo->msgClass = (MN_MSG_MsgClassUint8)(dcs & 0x3);
            break;
    }

    return ret;
}

MODULE_EXPORTED VOS_UINT32 MN_MSG_DecodeCbsDcs(VOS_UINT8 dcs, VOS_UINT8 *content, VOS_UINT32 contentLength,
                                               MN_MSG_CbdcsCode *dcsInfo)
{
    VOS_UINT32 ret = MN_ERR_NO_ERROR;

    dcsInfo->rawDcsData     = dcs;
    dcsInfo->msgCodingGroup = (dcs >> 4) & 0x0f;
    dcsInfo->msgCoding      = MN_MSG_MSG_CODING_7_BIT;
    dcsInfo->compressed     = VOS_FALSE;
    dcsInfo->msgLang        = MN_MSG_CBLANG_ENGLISH;
    dcsInfo->msgClass       = MN_MSG_MSG_CLASS_MAX;
    dcsInfo->langIndLen     = 0;

    switch (dcsInfo->msgCodingGroup) {
        /* 判断高四位为0000,0011,0010 */
        case 0x00:
        case 0x02:
        case 0x03:
            dcsInfo->msgLang = (dcs & 0x0f);
            break;

        case 0x01: /* 判断高四位为0001 */
            ret = MN_MSG_DecodeDcsIf01(dcs, content, contentLength, dcsInfo);
            if (ret != MN_ERR_NO_ERROR) {
                MN_WARN_LOG("MSG_CbDecodeDcs: Invalid Dcs Info.");
            }
            break;

        /* 判断高四位为01xx  */
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            ret = MN_MSG_DecodeDcsIf07(dcs, dcsInfo);
            if (ret != MN_ERR_NO_ERROR) {
                MN_WARN_LOG("MSG_CbDecodeDcs: Invalid Dcs Info.");
            }
            break;

        case 0x09: /* Refer to 23038-610.doc */
            ret = MN_MSG_DecodeDcsIf09(dcs, dcsInfo);
            if (ret != MN_ERR_NO_ERROR) {
                MN_WARN_LOG("MSG_CbDecodeDcs: Invalid Dcs Info.");
            }
            break;

        case 0x0f:
            ret = MN_MSG_DecodeDcsIf0F(dcs, dcsInfo);
            if (ret != MN_ERR_NO_ERROR) {
                MN_WARN_LOG("MSG_CbDecodeDcs: Invalid Dcs Info.");
            }
            break;

        default:
            MN_WARN_LOG("MSG_CbDecodeDcs: Invalid coding group.");
            ret = MN_ERR_CLASS_SMS_INVALID_CODING_GRP;
            break;
    }

    if ((dcsInfo->msgLang == MN_MSG_CBLANG_MAX) || (dcsInfo->msgLang == MN_MSG_CBLANG_UNSPECIFIED)) {
        dcsInfo->msgLang = MN_MSG_CBLANG_ENGLISH;
    }

    return ret;
}

LOCAL VOS_UINT32 MSG_CbDecodeSn(const MN_MSG_Cbgsmpage *gsmPage, MN_MSG_Cbsn *sn)
{
    sn->rawSnData    = gsmPage->snHigh;
    sn->rawSnData    = (VOS_UINT16)(sn->rawSnData << 8) | gsmPage->snLow;
    sn->gs           = (sn->rawSnData >> 14) & 0x0003;
    sn->messageCode  = (VOS_UINT16)((sn->rawSnData >> 4) & 0x03ff);
    sn->updateNumber = sn->rawSnData & 0x000f;

    return MN_ERR_NO_ERROR;
}

VOS_UINT32 MN_MSG_DecodeCbmPage(const MN_MSG_CbrawTsData *cbRawInfo, MN_MSG_Cbpage *cbmPageInfo)
{
    VOS_UINT32        ret;
    MN_MSG_Cbgsmpage *gsmPage = VOS_NULL_PTR;
    VOS_UINT32        dataLen;
    errno_t           memResult;

    /* 判断输入参数的合法性 */
    if ((cbRawInfo == VOS_NULL_PTR) || (cbmPageInfo == VOS_NULL_PTR)) {
        MN_ERR_LOG("MN_MSG_DecodeCbmPage: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    gsmPage = (MN_MSG_Cbgsmpage *)cbRawInfo->data;

    /* 解析DCS */
    ret = MN_MSG_DecodeCbsDcs(gsmPage->dcs, gsmPage->content, TAF_CBA_MAX_CBDATA_LEN, &(cbmPageInfo->dcs));
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG1("MN_MSG_DecodeCbmPage:DCS Invalid:ulRet", (VOS_INT32)ret);
    }

    /* 解析SN */
    ret = MSG_CbDecodeSn(gsmPage, &(cbmPageInfo->sn));
    if (ret != MN_ERR_NO_ERROR) {
        MN_WARN_LOG1("MN_MSG_DecodeCbmPage:SN Invalid:ulRet", (VOS_INT32)ret);
    }

    /* 解析MID */
    cbmPageInfo->mid = gsmPage->mIdHigh;
    cbmPageInfo->mid = (VOS_UINT16)(cbmPageInfo->mid << 8) | gsmPage->mIdLow;

    /* page info */
    cbmPageInfo->pageIndex = gsmPage->pagebit4;
    cbmPageInfo->pageNum   = gsmPage->pagesbit4;

    /*
     * This parameter is coded as two 4-bit fields. The first field (bits 0-3) indicates the binary value of the total
     * number of pages in the CBS message and the second field (bits 4-7) indicates binary the page number within that
     * sequence. The coding starts at 0001, with 0000 reserved. If a mobile receives the code 0000 in either the first
     * field or the second field then it shall treat the CBS message exactly the same as a CBS message with page
     * parameter 0001 0001 (i.e. a single page message).
     */
    if ((cbmPageInfo->pageIndex == 0) || (cbmPageInfo->pageNum == 0)) {
        cbmPageInfo->pageIndex = 1;
        cbmPageInfo->pageNum   = 1;
    }
    dataLen = cbRawInfo->len - MSG_CBPAGE_HEADER_LEN;

    /* 如果是 7bit编码需要将其转化为8bit */
    if (cbmPageInfo->dcs.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
        cbmPageInfo->content.len = (dataLen * 8) / 7;

        if (cbmPageInfo->content.len > TAF_CBA_MAX_RAW_CBDATA_LEN) {
            cbmPageInfo->content.len = TAF_CBA_MAX_RAW_CBDATA_LEN;
        }

        ret = TAF_STD_UnPack7Bit(gsmPage->content, cbmPageInfo->content.len, 0, cbmPageInfo->content.content);
        if (ret != VOS_OK) {
            MN_WARN_LOG("MN_MSG_DecodeCbmPage:TAF_STD_UnPack7Bit fail");
        }
    } else {
        cbmPageInfo->content.len = dataLen;
        if (cbmPageInfo->content.len > TAF_CBA_MAX_CBDATA_LEN) {
            cbmPageInfo->content.len = TAF_CBA_MAX_CBDATA_LEN;
        }
        if (cbmPageInfo->content.len > 0) {
            memResult = memcpy_s(cbmPageInfo->content.content, sizeof(cbmPageInfo->content.content), gsmPage->content,
                                 cbmPageInfo->content.len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cbmPageInfo->content.content), cbmPageInfo->content.len);
        }
    }
    return MN_ERR_NO_ERROR;
}

/* Deleted MN_MSG_DecodeCbsDcs */

#endif

