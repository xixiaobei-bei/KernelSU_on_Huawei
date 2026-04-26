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

#include "at_check_func.h"
#include "securec.h"
#include "at_type_def.h"
#include "at_common.h"
#include "at_ctx.h"
#include "at_event_report.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CHECKFUNC_C

#define AT_STR_0_LEN 1
#define AT_STR_0X_LEN 2
#define AT_DOUBLE_COLON_LEN 2

#define AT_HOUR_MAX_VALUE 24
#define AT_MUNITE_MAX_VALUE 60
#define AT_SECOND_MAX_VALUE 60
#define AT_DATE_STR_VALID_LEN 19
#define AT_MONTH_INVALID_VALUE 0

const VOS_UINT8 g_atBasicCmdNameTbl[] = {
    'e', 'E', 'v', 'V', 'a', 'A', 'h', 'H', '&', 'F', 'f', 'i', 'I', 't', 'T', 'p', 'P', 'x', 'X', 'z', 'Z', 'c', 'C',
    'd', 'D', 'q', 'Q', 's', 'S', 'o', 'O', 'l', 'L', 'm', 'M'
};

VOS_UINT32 AT_CheckBasicCmdName(VOS_UINT8 Char)
{
    VOS_UINT32          i;

    for (i = 0; i < AT_ARRAY_SIZE(g_atBasicCmdNameTbl); i++) {
        if (Char == g_atBasicCmdNameTbl[i]) {
            return AT_SUCCESS;
        }
    }

    return AT_FAILURE;
}

/*
 * 功能描述: 比较、匹配十六进制(0x或0X)
 */
VOS_UINT32 atCheckHex(VOS_UINT8 Char)
{
    if (Char == 'x' || Char == 'X') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * 功能描述: 比较、匹配十六进制数字
 */
VOS_UINT32 atCheckHexNum(VOS_UINT8 Char)
{
    if (isdigit(Char) || (Char >= 'a' && Char <= 'f') || (Char >= 'A' && Char <= 'F')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * 功能描述: 比较、匹配无双引号包括字母起始
 */
VOS_UINT32 atNoQuotLetter(VOS_UINT8 Char)
{
    if (Char == '@') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 atCheckblank(VOS_UINT8 Char)
{
    /* 添加-支持作为字符 */
    if (Char == ' ') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 atCheckComma(VOS_UINT8 Char)
{
    if (Char == ',') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 atCheckColon(VOS_UINT8 Char)
{
    if (Char == ':') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 atCheckChar0x2f(VOS_UINT8 Char)
{
    if (Char == '/') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * 功能描述: 比较匹配十六进制数字
 *    AT_SUCCESS: 匹配
 *    AT_FAILURE: 不匹配
 */
static VOS_UINT32 checkHexNumString(VOS_UINT16 len, VOS_UINT8 *pcData)
{
    VOS_UINT32 length = AT_STR_0X_LEN;          /* 从0x后开始比较 */
    VOS_UINT8 *pcTmp  = pcData + AT_STR_0X_LEN; /* 从0x后开始比较 */

    /* 参数指针由调用者保证不为NULL, 该处不做判断 */

    while (length++ < len) {
        /* 判断是否是数字 */
        if (isdigit(*pcTmp) || (*pcTmp >= 'A' && *pcTmp <= 'F') || (*pcTmp >= 'a' && *pcTmp <= 'f')) {
            pcTmp++;
        } else {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 比较匹配十进制数字
 *    AT_SUCCESS: 匹配
 *    AT_FAILURE: 不匹配
 */
static VOS_UINT32 checkDecNumString(VOS_UINT16 len, VOS_UINT8 *pcData)
{
    VOS_UINT32 length = 0;      /* 从0后开始比较 */
    VOS_UINT8 *pcTmp  = pcData; /* 从0后开始比较 */

    /* 参数指针由调用者保证不为NULL, 该处不做判断 */

    while (length++ < len) {
        /* 判断是否是数字 */
        if (isdigit(*pcTmp)) {
            pcTmp++;
        } else {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 比较、匹配数字
 */
VOS_UINT32 atCheckNumString(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8 *pcTmp = data;

    if ((data == NULL) || (len == 0)) {
        return AT_FAILURE;
    }

    pcTmp = data;

    if (*pcTmp == '0') {
        if (len > AT_STR_0X_LEN && ((*(pcTmp + AT_STR_0_LEN) == 'x') ||(*(pcTmp + AT_STR_0_LEN) == 'X'))) {
            return checkHexNumString(len, pcTmp);
        }
        else {
        }
    }

    return checkDecNumString(len, pcTmp);
}

VOS_UINT32 atCheckRightArrowStr(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32 length = 0;
    VOS_UINT8 *pcTmp  = data;

    while (length++ < len) {
        /* 判断是否是MMI字符 */
        if (*pcTmp == '>') {
            return AT_SUCCESS;
        } else {
            pcTmp++;
        }
    }
    return AT_FAILURE;
}

VOS_UINT32 atCheckMmiString(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32 length = 0;
    VOS_UINT8 *pcTmp  = data;

    while (length++ < len) {
        /* 判断是否是MMI字符 */
        if ((*pcTmp == '*') || (*pcTmp == '#')) {
            return AT_SUCCESS;
        } else {
            pcTmp++;
        }
    }
    return AT_FAILURE;
}

VOS_UINT32 At_CheckStringPara(AT_ParseParaType *para)
{
    VOS_UINT8 *data = para->para;
    errno_t    returnValue;

    /* 检查是否是字符串 */
    if (para->paraLen < AT_DOUBLE_COLON_LEN) {
        return AT_FAILURE;
    }

    if ((*data != '"') || (*((data + para->paraLen) - 1) != '"')) {
        return AT_FAILURE;
    }

    /* 去除两头的双引号 */
    para->paraLen -= AT_DOUBLE_COLON_LEN; /* 修改参数长度 */
    if ((para->paraLen > 0) && (para->paraLen < sizeof(para->para))) {
        returnValue = memmove_s(data, sizeof(para->para), data + 1, para->paraLen);
        TAF_MEM_CHK_RTN_VAL(returnValue, sizeof(para->para), para->paraLen);
    }

    if (para->paraLen < sizeof(para->para)) {
        data[para->paraLen] = '\0';
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 比较、匹配无双引号包括字符串类型
 */
VOS_UINT32 atCheckNoQuotStringPara(AT_ParseParaType *para)
{
    VOS_UINT8 *data = para->para;
    errno_t    returnValue;

    /* 检查是否是带引号的字符串 */
    if ((*data == '"') && (*((data + para->paraLen) - 1) == '"')) {
        if (para->paraLen < AT_DOUBLE_COLON_LEN) {
            return AT_FAILURE;
        }

        /* 去除两头的双引号 */
        para->paraLen -= AT_DOUBLE_COLON_LEN; /* 修改参数长度 */
        if (para->paraLen > 0) {
            returnValue = memmove_s(data, sizeof(para->para), data + 1, para->paraLen);
            TAF_MEM_CHK_RTN_VAL(returnValue, sizeof(para->para), para->paraLen);
        }

        data[para->paraLen] = '\0';
    }

    return AT_SUCCESS;
}

VOS_UINT32 atCheckNumPara(AT_ParseParaType *para)
{
    VOS_UINT32 i = 0;

    /* 检查字符串 */
    if (atCheckNumString(para->para, para->paraLen) == AT_SUCCESS) {
        if (atAuc2ul(para->para, para->paraLen, &para->paraValue) != AT_SUCCESS) {
            return AT_FAILURE;
        }

        for (i = 0; i < g_atParseCmd.paraNumRangeIndex; i++) {
            /* 如果参数值匹配 */
            if ((g_atParseCmd.paraNumRange[i].small <= para->paraValue) &&
                (para->paraValue <= g_atParseCmd.paraNumRange[i].big)) {
                return AT_SUCCESS;
            }
        }
    }

    return AT_FAILURE;
}

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
VOS_UINT32 atCheckCharPara(AT_ParseParaType *para)
{
    VOS_UINT8 *buf = VOS_NULL_PTR;
    errno_t    memResult;
    VOS_UINT8  indexNum = 0;

    buf = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, AT_PARA_MAX_LEN + 1);

    if (buf == NULL) {
        AT_NORM_LOG("atCheckCharPara: Fail to malloc mem.");
        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(buf, AT_PARA_MAX_LEN + 1, 0, AT_PARA_MAX_LEN + 1);
    if (para->paraLen > AT_PARA_MAX_LEN) {
        para->paraLen = AT_PARA_MAX_LEN;
    }

    if (para->paraLen > 0) {
        memResult = memcpy_s(buf, AT_PARA_MAX_LEN + 1, para->para, para->paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, para->paraLen);
    }

    At_UpString(buf, para->paraLen);

    buf[para->paraLen] = '\0';

    for (indexNum = 0; indexNum < g_atParseCmd.paraStrRangeIndex; indexNum++) {
        if (AT_STRCMP((VOS_CHAR *)g_atParseCmd.strRange[indexNum], (VOS_CHAR *)buf) == 0) {
            para->paraValue = indexNum;
            VOS_MemFree(WUEPS_PID_AT, buf);
            return AT_SUCCESS;
        }
    }
    VOS_MemFree(WUEPS_PID_AT, buf);
    return AT_FAILURE;
}
#else
VOS_UINT32 atCheckCharPara(AT_ParseParaType *para)
{
    VOS_UINT8  buf[AT_PARA_MAX_LEN + 1] = {0};
    errno_t    memResult;
    VOS_UINT8  indexNum = 0;
    VOS_UINT16 bufLen   = sizeof(buf) - 1;

    if (para->paraLen > bufLen) {
        para->paraLen = bufLen;
    }

    if (para->paraLen > 0) {
        memResult = memcpy_s(buf, sizeof(buf), para->para, para->paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(buf), para->paraLen);
    }

    At_UpString(buf, para->paraLen);

    buf[para->paraLen] = '\0';

    for (indexNum = 0; indexNum < g_atParseCmd.paraStrRangeIndex; indexNum++) {
        if (AT_STRCMP((VOS_CHAR *)g_atParseCmd.strRange[indexNum], (VOS_CHAR *)buf) == 0) {
            para->paraValue = indexNum;

            return AT_SUCCESS;
        }
    }

    return AT_FAILURE;
}
#endif
/*
 * Description: 比较、匹配分号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckSemicolon(TAF_UINT8 Char)
{
    if (Char == ';') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckDialNum(TAF_UINT8 Char)
{
    if ((Char >= '0') && (Char <= '9')) {
        return AT_SUCCESS;
    } else if ((Char >= 'a') && (Char <= 'c')) {
        return AT_SUCCESS;
    } else if ((Char >= 'A') && (Char <= 'C')) {
        return AT_SUCCESS;
    } else if ((Char == '*') || (Char == '#') || (Char == '+')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckDmChar(TAF_UINT8 Char)
{
    if ((Char != '*') && (Char != '#')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckCharStar(TAF_UINT8 Char)
{
    if (Char == '*') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符#
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckCharWell(TAF_UINT8 Char)
{
    if (Char == '#') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符>
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckCharRightArrow(TAF_UINT8 Char)
{
    if (Char == '>') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharG(TAF_UINT8 Char)
{
    if ((Char == 'G') || (Char == 'g')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharI(TAF_UINT8 Char)
{
    if ((Char == 'I') || (Char == 'i')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharS(TAF_UINT8 Char)
{
    if ((Char == 's') || (Char == 'S')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharD(TAF_UINT8 Char)
{
    if ((Char == 'd') || (Char == 'D')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharA(TAF_UINT8 Char)
{
    if ((Char == 'a') || (Char == 'A')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharT(TAF_UINT8 Char)
{
    if ((Char == 't') || (Char == 'T')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharPlus(TAF_UINT8 Char)
{
    if ((Char == '+') || (Char == '^') || (Char == '$')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符(
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckLeftBracket(TAF_UINT8 Char)
{
    if (Char == '(') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符)
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckRightBracket(TAF_UINT8 Char)
{
    if (Char == ')') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckDigit(TAF_UINT8 Char)
{
    if ((Char >= '0') && (Char <= '9')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckLetter(TAF_UINT8 Char)
{
    if (((Char >= 'a') && (Char <= 'z')) || ((Char >= 'A') && (Char <= 'Z')) || (Char == '_') || (Char == '&')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckUpLetter(TAF_UINT8 Char)
{
    if ((Char >= 'A') && (Char <= 'Z')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckChar(TAF_UINT8 Char)
{
    if (((Char >= 'a') && (Char <= 'z')) || ((Char >= 'A') && (Char <= 'Z')) || ((Char >= '0') && (Char <= '9')) ||
        (Char == '*') || (Char == '#') || (Char == '-') || (Char == '.')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * Description: 比较、匹配字符('a'~'z' 和'A'~'Z')
 * History:
 *  1.Date: 2010-07-1
 *    Author:
 *    Modification: Created function
 */

TAF_UINT32 At_CheckAlpha(TAF_UINT8 Char)
{
    if (((Char >= 'a') && (Char <= 'z')) || ((Char >= 'A') && (Char <= 'Z'))) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配逗号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckColon(TAF_UINT8 Char)
{
    if (Char == ',') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配减号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckCharSub(TAF_UINT8 Char)
{
    if (Char == '-') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配引号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckQuot(TAF_UINT8 Char)
{
    if (Char == '"') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckHorizontalLine(TAF_UINT8 Char)
{
    if (Char == '-') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * Description: 比较、匹配等号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckEqu(TAF_UINT8 Char)
{
    if (Char == '=') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配问号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckReq(TAF_UINT8 Char)
{
    if (Char == '?') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配非引号
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckNoQuot(TAF_UINT8 Char)
{
    if (Char != '"') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)

 TAF_UINT32 AT_CheckSuccess(TAF_UINT8 Char)
{
    return AT_SUCCESS;
}

 TAF_UINT32 AT_CheckEscape(TAF_UINT8 Char)
{
    if (Char == '\\') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
#endif

TAF_UINT32 At_CheckCharE(TAF_UINT8 Char)
{
    if ((Char == 'e') || (Char == 'E')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符1
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckChar1(TAF_UINT8 Char)
{
    if (Char == '1') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符0
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckChar0(TAF_UINT8 Char)
{
    if (Char == '0') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharV(TAF_UINT8 Char)
{
    if ((Char == 'v') || (Char == 'V')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckCharF(TAF_UINT8 Char)
{
    if ((Char == 'f') || (Char == 'F')) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}
/*
 * Description: 比较、匹配字符&
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckChar0x26(TAF_UINT8 Char)
{
    if (Char == '&') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

TAF_UINT32 At_CheckNumString(TAF_UINT8 *data, TAF_UINT16 len)
{
    TAF_UINT32 length = 0;
    TAF_UINT8 *pcTmp  = data;

    if ((data == TAF_NULL_PTR) || (len == 0)) {
        return AT_FAILURE;
    }

    pcTmp = data;

    while (length++ < len) {
        /* 判断是否是数字 */
        if ((*pcTmp >= '0') && (*pcTmp <= '9')) {
            pcTmp++;
        } else {
            return AT_FAILURE;
        }
    }
    return AT_SUCCESS;
}

/*
 * Description: 比较、匹配数字
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_CheckRightArrowStr(TAF_UINT8 *data, TAF_UINT16 len)
{
    TAF_UINT32 length = 0;
    TAF_UINT8 *pcTmp  = data;

    if (pcTmp == VOS_NULL_PTR) {
        AT_ERR_LOG("At_CheckRightArrowStr: Parameter of the function is null.");
        return AT_FAILURE;
    }

    while (length++ < len) {
        /* 判断是否是MMI字符 */
        if (*pcTmp == '>') {
            return AT_SUCCESS;
        } else {
            pcTmp++;
        }
    }
    return AT_FAILURE;
}

TAF_UINT32 At_CheckMmiString(TAF_UINT8 *data, TAF_UINT16 len)
{
    TAF_UINT32 length = 0;
    TAF_UINT8 *pcTmp  = data;

    if (pcTmp == VOS_NULL_PTR) {
        AT_ERR_LOG("At_CheckMmiString: Parameter of the function is null.");
        return AT_FAILURE;
    }

    while (length++ < len) {
        /* 判断是否是MMI字符 */
        if ((*pcTmp == '*') || (*pcTmp == '#')) {
            return AT_SUCCESS;
        } else {
            pcTmp++;
        }
    }
    return AT_FAILURE;
}

/*
 * Description: 比较、匹配结束符
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
VOS_UINT32 At_CheckEnd(VOS_UINT8 Char)
{
    if (Char == '\0') {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

VOS_UINT32 AT_CheckDateFormat(VOS_UINT8 *dateStr, VOS_UINT32 dateStrLen)
{
    VOS_UINT8 firstJuncture;
    VOS_UINT8 secondJuncture;
    VOS_UINT8 firstColon;
    VOS_UINT8 secondColon;
    VOS_UINT8 space;

    /* 格式为2009-12-31 10:10:10的日期字符串: 总长度必须为19 */
    if (dateStrLen != AT_DATE_STR_VALID_LEN) {
        AT_NORM_LOG1("AT_CheckDateFormat: length error.", (VOS_INT32)dateStrLen);
        return AT_ERROR;
    }

    /* 格式为2009-12-31 10:10:10的日期字符串: 第5个字节为'-', 第8个字节为'-' */
    firstJuncture  = *(dateStr + AT_MODEM_YEAR_LEN);
    /* 1为日期的分隔符宽度 */
    secondJuncture = *(dateStr + (AT_MODEM_YEAR_LEN + 1+ AT_MODEM_MONTH_LEN));
    if ((firstJuncture != '-') || (secondJuncture != '-')) {
        AT_NORM_LOG("AT_CheckDateFormat: Fail to check juncture.");
        return AT_ERROR;
    }

    /* 格式为2009-12-31 10:10:10的日期字符串: 第11个字节为' ' */
    /* 1为日期的分隔符宽度 */
    space = *(dateStr + (AT_MODEM_YEAR_LEN + 1 + AT_MODEM_MONTH_LEN + 1 + AT_MODEM_DATE_LEN));
    if (space != ' ') {
        AT_NORM_LOG("AT_CheckDateFormat: Fail to check space.");
        return AT_ERROR;
    }

    /* 格式为2009-12-31 10:10:10的日期字符串: 第14个字节为':' 第17个字节为':' */
    /* 1为日期的分隔符宽度 */
    firstColon  = *(dateStr + (AT_MODEM_YEAR_LEN + 1+ AT_MODEM_MONTH_LEN + 1 +AT_MODEM_DATE_LEN + 1 +
                    AT_MODEM_HOUR_LEN));
    secondColon = *(dateStr + (AT_MODEM_YEAR_LEN + 1+ AT_MODEM_MONTH_LEN + 1+ AT_MODEM_DATE_LEN + 1 +
                    AT_MODEM_HOUR_LEN + 1 +AT_MODEM_MIN_LEN));
    if ((firstColon != ':') || (secondColon != ':')) {
        AT_NORM_LOG("AT_CheckDateFormat: Fail to check colon.");
        return AT_ERROR;
    }

    return AT_OK;
}

/*
 * 功能描述: 判断月份是否有31天
 */
VOS_UINT32 AT_IsMonthHaveThirtyOneDays(VOS_UINT8 month)
{
    /* 1,3,5,7,8,10,12月有31天 */
    if ((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 10) ||
        (month == 12)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
/*
 * 功能描述: 判断月份是否有30天
 */
VOS_UINT32 AT_IsMonthHaveThirtyDays(VOS_UINT8 month)
{
    /* 4,6,9,11月有30天 */
    if ((month == 4) || (month == 6) || (month == 9) || (month == 11)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
/*
 * 功能描述: 判断是否是闰年
 */
VOS_UINT32 AT_IsLeapYear(VOS_UINT16 year)
{
    if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_GetDaysForEachMonth(VOS_UINT32 year, VOS_UINT32 month)
{
    /* 1,3,5,7,8,10,12月有31天 */
    if (AT_IsMonthHaveThirtyOneDays((VOS_UINT8)month) == VOS_TRUE) {
        return 31;
    /* 4,6,9,11月有30天 */
    } else if (AT_IsMonthHaveThirtyDays((VOS_UINT8)month) == VOS_TRUE) {
        return 30;
    } else {
        /* 2月看是否为闰年，是则为29天，否则为28天 */
        if (AT_IsLeapYear((VOS_UINT16)year) == VOS_TRUE) {
            /* 润年 */
            return 29;
        } else {
            /* 非润年 */
            return 28;
        }
    }
}

VOS_UINT8 At_GetDaysForEachMonth(VOS_UINT8 year, VOS_UINT8 month)
{
    VOS_UINT16 adjustYear;

    /* 输入的year值为两位数，默认从2000年开始计算 */
    adjustYear = 2000 + (VOS_UINT16)year;

    if (AT_IsMonthHaveThirtyOneDays(month) == VOS_TRUE) {
        return 31;
    } else if (AT_IsMonthHaveThirtyDays(month) == VOS_TRUE) {
        return 30;
    } else {
        /* 看2月是否为闰年，是则为29天，否则为28天 */
        if (AT_IsLeapYear(adjustYear) == VOS_TRUE) {
            /* 闰年，2月天数 */
            return 29;
        } else {
            /* 非闰年，2月天数 */
            return 28;
        }
    }
}

VOS_UINT32 AT_CheckDate(AT_DATE *date)
{
    VOS_UINT32 daysEachMonth;

    /* 月份有效性检查: 月份数值必须不为0，且小于等于12 */
    if ((date->month > AT_MODEM_MONTH_MAX) || (date->month == AT_MONTH_INVALID_VALUE)) {
        AT_NORM_LOG("AT_CheckDate: Fail to check month.");
        return AT_ERROR;
    }

    /* 日期有效性检查: 日期数值必须不为0，且小于等于当月的总天数 */
    daysEachMonth = AT_GetDaysForEachMonth(date->year, date->month);
    if ((date->day > daysEachMonth) || (date->day == 0)) {
        AT_NORM_LOG("AT_CheckDate: Fail to check day.");
        return AT_ERROR;
    }

    /* 时间有效性检查: 小时数必须小于24，分钟或秒数必须小于60 */
    if ((date->hour >= AT_HOUR_MAX_VALUE) || (date->munite >= AT_MUNITE_MAX_VALUE) ||
        (date->second >= AT_SECOND_MAX_VALUE)) {
        AT_NORM_LOG("AT_CheckDate: Fail to check time.");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_GetDate(VOS_UINT8 *dateStr, VOS_UINT32 dateStrLen, AT_DATE *date)
{
    VOS_UINT32 ret;

    /* 获取日期中的年份: 年份占用4个字节 */
    ret = At_Auc2ul(dateStr, AT_MODEM_YEAR_LEN, &date->year);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get year.");
        return AT_ERROR;
    }

    /* 获取日期中的月份: 月份占用2个字节，偏移年份占用的4个字节和'-'一个字节 */
    ret = At_Auc2ul((dateStr + (AT_MODEM_YEAR_LEN + 1)), AT_MODEM_MONTH_LEN, &date->month);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get month.");
        return AT_ERROR;
    }

    /* 获取日期中的日期: 日期占用2个字节，偏移月份占用的2个字节和'-'一个字节 */
    ret = At_Auc2ul((dateStr + (AT_MODEM_YEAR_LEN+1+AT_MODEM_MONTH_LEN+1)), AT_MODEM_DATE_LEN, &date->day);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get day.");
        return AT_ERROR;
    }

    /* 获取日期中的小时: 小时占用2个字节，偏移日期占用的2个字节和' '一个字节 */
    ret = At_Auc2ul((dateStr +
        (AT_MODEM_YEAR_LEN + 1 + AT_MODEM_MONTH_LEN + 1 + AT_MODEM_DATE_LEN + 1)), AT_MODEM_HOUR_LEN, &date->hour);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get hour.");
        return AT_ERROR;
    }

    /* 获取日期中的分钟: 分钟占用2个字节，偏移小时占用的2个字节和':'一个字节 */
    ret = At_Auc2ul((dateStr + (AT_MODEM_YEAR_LEN + 1 + AT_MODEM_MONTH_LEN + 1 + AT_MODEM_DATE_LEN + 1 +
                    AT_MODEM_HOUR_LEN + 1)), AT_MODEM_MIN_LEN, &date->munite);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get minute.");
        return AT_ERROR;
    }

    /* 获取日期中的秒钟: 秒钟占用2个字节，偏移分钟占用的2个字节和':'一个字节 */
    ret = At_Auc2ul((dateStr + (AT_MODEM_YEAR_LEN + 1 + AT_MODEM_MONTH_LEN + 1 + AT_MODEM_DATE_LEN + 1 +
                    AT_MODEM_HOUR_LEN + 1 + AT_MODEM_MIN_LEN + 1)), AT_MODEM_SEC_LEN, &date->second);
    if (ret != AT_SUCCESS) {
        AT_NORM_LOG("AT_GetDate: Fail to get second.");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_CheckAndParsePara(AT_ParseParaType *atPara, VOS_UINT8 *begain, VOS_UINT8 *end)
{
    VOS_UINT32 paraLen;

    paraLen = (VOS_UINT32)(end - begain);

    if ((paraLen == 0) || (paraLen > AT_PARA_MAX_LEN)) {
        return AT_ERROR;
    }

    atPara->paraLen = (VOS_UINT16)paraLen;

    At_RangeCopy(atPara->para, begain, end);

    return AT_OK;
}

VOS_UINT32 AT_CheckStrStartWith(VOS_UINT8 *lineStr,
                                VOS_UINT16 lineStrLen, VOS_UINT8 *pefixStr, VOS_UINT16 pefixStrLen
)
{
    VOS_UINT32 loop;
    VOS_UINT16 safeLen;

    if (lineStrLen < pefixStrLen) {
        return VOS_FALSE;
    }

    safeLen = AT_MIN(lineStrLen, pefixStrLen);

    for (loop = 0; (loop < safeLen) && (*lineStr != '\0') && (*pefixStr != '\0'); lineStr++, pefixStr++) {
        if (*lineStr != *pefixStr) {
            return VOS_FALSE;
        }

        loop++;
    }

    if (*pefixStr == '\0') {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
}
