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

#include "at_parse_para.h"
#include "securec.h"
#include "at_check_func.h"


#define THIS_FILE_ID PS_FILE_ID_AT_PARSEPARA_C

/* 参数匹配初始空状态表 */
static const AT_SubState g_atParaNoneStateTab[] = {
    /* 如果当前状态是AT_PARA_NONE_STATE，atCheckLeftBracket成功，则进入AT_PARA_LEFT_BRACKET_STATE */
    { At_CheckLeftBracket, AT_PARA_LEFT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配左括号状态表 */
static const AT_SubState g_atParaLeftBracketStateTab[] = {
    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，atCheckZero成功，则进入AT_PARA_ZERO_STATE */
    { At_CheckChar0, AT_PARA_ZERO_STATE },

    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，atCheckDigit成功，则进入AT_PARA_NUM_STATE */
    { At_CheckDigit, AT_PARA_NUM_STATE },

    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，at_NoQuotLetter成功，则进入AT_PARA_NO_QUOT_LETTER_STATE */
    { atNoQuotLetter, AT_PARA_NO_QUOT_LETTER_STATE },

    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，atCheckLetter成功，则进入AT_PARA_LETTER_STATE */
    { At_CheckLetter, AT_PARA_LETTER_STATE },

    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，atCheckQuot成功，则进入AT_PARA_LEFT_QUOT_STATE */
    { At_CheckQuot, AT_PARA_LEFT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配参数状态表 */
static const AT_SubState g_atParaNumStateTab[] = {
    /* 如果当前状态是AT_PARA_NUM_STATE，atCheckDigit成功，则进入AT_PARA_NUM_STATE */
    { At_CheckDigit, AT_PARA_NUM_STATE },

    /* 如果当前状态是AT_PARA_NUM_STATE，atCheckColon成功，则进入AT_PARA_NUM_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_COLON_STATE },

    /* 如果当前状态是AT_PARA_NUM_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 如果当前状态是AT_PARA_NUM_STATE，atCheckCharSub成功，则进入AT_PARA_SUB_STATE */
    { At_CheckCharSub, AT_PARA_SUB_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围状态表 */
static const AT_SubState g_atParaSubStateTab[] = {
    /* 如果当前状态是AT_PARA_SUB_STATE，atCheckZero成功，则进入AT_PARA_ZERO_SUB_STATE */
    { At_CheckChar0, AT_PARA_ZERO_SUB_STATE },

    /* 如果当前状态是AT_PARA_SUB_STATE，atCheckDigit成功，则进入AT_PARA_NUM_SUB_STATE */
    { At_CheckDigit, AT_PARA_NUM_SUB_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围数字状态表 */
static const AT_SubState g_atParaNumSubStateTab[] = {
    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckDigit成功，则进入AT_PARA_NUM_SUB_STATE */
    { At_CheckDigit, AT_PARA_NUM_SUB_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckColon成功，则进入AT_PARA_NUM_SUB_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_SUB_COLON_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围数字逗号状态表 */
static const AT_SubState g_atParaNumSubColonStateTab[] = {
    /* 如果当前状态是AT_PARA_NUM_SUB_COLON_STATE，atCheckZero成功，则进入AT_PARA_ZERO_STATE */
    { At_CheckChar0, AT_PARA_ZERO_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_COLON_STATE，atCheckDigit成功，则进入AT_PARA_NUM_STATE */
    { At_CheckDigit, AT_PARA_NUM_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配右括号状态表 */
static const AT_SubState g_atParaRightBracketStateTab[] = {
    /* 如果当前状态是AT_PARA_RIGHT_BRACKET_STATE，atCheckColon成功，则进入AT_PARA_COLON_STATE */
    { atCheckComma, AT_PARA_COLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配逗号状态表 */
static const AT_SubState g_atParaColonStateTab[] = {
    /* 如果当前状态是AT_PARA_COLON_STATE，atCheckLeftBracket成功，则进入AT_PARA_LEFT_BRACKET_STATE */
    { At_CheckLeftBracket, AT_PARA_LEFT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配左引号状态表 */
static const AT_SubState g_atParaLeftQuotStateTab[] = {
    /* 如果当前状态是AT_PARA_LEFT_QUOT_STATE，atCheckNoQuot成功，则进入AT_PARA_LEFT_QUOT_STATE */
    { At_CheckNoQuot, AT_PARA_LEFT_QUOT_STATE },

    /* 如果当前状态是AT_PARA_LEFT_QUOT_STATE，atCheckQuot成功，则进入AT_PARA_RIGHT_QUOT_STATE */
    { At_CheckQuot, AT_PARA_RIGHT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配右引号状态表 */
static const AT_SubState g_atParaRightQuotStateTab[] = {
    /* 如果当前状态是AT_PARA_RIGHT_QUOT_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 如果当前状态是AT_PARA_RIGHT_QUOT_STATE，atCheckColon成功，则进入AT_PARA_QUOT_COLON_STATE */
    { atCheckComma, AT_PARA_QUOT_COLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配字母状态表 */
static const AT_SubState g_atParaLetterStateTab[] = {
    /* 如果当前状态是AT_PARA_LETTER_STATE，atCheckLetter成功，则进入AT_PARA_LETTER_STATE */
    { At_CheckLetter, AT_PARA_LETTER_STATE },

    /* 如果当前状态是AT_PARA_LETTER_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 如果当前状态是AT_PARA_LETTER_STATE，atCheckCharSub成功，则进入AT_PARA_LETTER_STATE */
    { At_CheckCharSub, AT_PARA_LETTER_STATE },

    /* 如果当前状态是AT_PARA_LETTER_STATE，atCheckColon成功，则进入AT_PARA_LETTER_STATE */
    { atCheckColon, AT_PARA_LETTER_STATE },

    /* 如果当前状态是AT_PARA_LETTER_STATE，atCheckblank成功，则进入AT_PARA_LETTER_STATE */
    { atCheckblank, AT_PARA_LETTER_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配数字逗号状态表 */
static const AT_SubState g_atParaNumColonStateTab[] = {
    /* 如果当前状态是AT_PARA_NUM_COLON_STATE，atCheckZero成功，则进入AT_PARA_ZERO_STATE */
    { At_CheckChar0, AT_PARA_ZERO_STATE },

    /* 如果当前状态是AT_PARA_NUM_COLON_STATE，atCheckDigit成功，则进入AT_PARA_NUM_STATE */
    { At_CheckDigit, AT_PARA_NUM_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配引号逗号状态表 */
static const AT_SubState g_atParaQuotColonStateTab[] = {
    /* 如果当前状态是AT_PARA_QUOT_COLON_STATE，atCheckQuot成功，则进入AT_PARA_LEFT_QUOT_STATE */
    { At_CheckQuot, AT_PARA_LEFT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 支持八进制、十六进制参数 BEGIN */

/* 参数匹配数字0状态表 */
static const AT_SubState g_atParaZeroStateTab[] = {
    /* 如果当前状态是AT_PARA_ZERO_STATE，at_CheckHex成功，则进入AT_PARA_HEX_STATE */
    { atCheckHex, AT_PARA_HEX_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckDigit成功，则进入AT_PARA_NUM_STATE */
    { At_CheckDigit, AT_PARA_NUM_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckColon成功，则进入AT_PARA_NUM_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_COLON_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckCharSub成功，则进入AT_PARA_SUB_STATE */
    { At_CheckCharSub, AT_PARA_SUB_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配十六进制状态表 */
static const AT_SubState g_atParaHexStateTab[] = {
    /* 如果当前状态是AT_PARA_HEX_STATE，at_CheckHex成功，则进入AT_PARA_HEX_NUM_STATE */
    { atCheckHexNum, AT_PARA_HEX_NUM_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配十六进制数字状态表 */
static const AT_SubState g_atParaHexNumStateTab[] = {
    /* 如果当前状态是AT_PARA_ZERO_STATE，at_CheckHex成功，则进入AT_PARA_HEX_STATE */
    { atCheckHexNum, AT_PARA_HEX_NUM_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckColon成功，则进入AT_PARA_NUM_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_COLON_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 如果当前状态是AT_PARA_ZERO_STATE，atCheckCharSub成功，则进入AT_PARA_SUB_STATE */
    { At_CheckCharSub, AT_PARA_SUB_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围数字0状态表 */
static const AT_SubState g_atParaZeroSubStateTab[] = {
    /* 如果当前状态是AT_PARA_ZERO_SUB_STATE，at_CheckHex成功，则进入AT_PARA_HEX_SUB_STATE */
    { atCheckHex, AT_PARA_HEX_SUB_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckDigit成功，则进入AT_PARA_NUM_SUB_STATE */
    { At_CheckDigit, AT_PARA_NUM_SUB_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckColon成功，则进入AT_PARA_NUM_SUB_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_SUB_COLON_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围十六进制状态表 */
static const AT_SubState g_atParaHexSubStateTab[] = {
    /* 如果当前状态是AT_PARA_HEX_SUB_STATE，at_CheckHex成功，则进入AT_PARA_HEX_NUM_SUB_STATE */
    { atCheckHexNum, AT_PARA_HEX_NUM_SUB_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 参数匹配范围十六进制数字状态表 */
static const AT_SubState g_atParaHexNumSubStateTab[] = {
    /* 如果当前状态是AT_PARA_HEX_NUM_SUB_STATE，at_CheckHexNum成功，则进入AT_PARA_HEX_NUM_SUB_STATE */
    { atCheckHexNum, AT_PARA_HEX_NUM_SUB_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckColon成功，则进入AT_PARA_NUM_SUB_COLON_STATE */
    { atCheckComma, AT_PARA_NUM_SUB_COLON_STATE },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 支持八进制、十六进制参数 END */

/* 支持无双引号包括字符串 BEGIN */

/* 参数匹配无双引号包括字母状态表 */
static const AT_SubState g_atParaNoQuotLetterStateTab[] = {
    /* 如果当前状态是AT_PARA_NO_QUOT_LETTER_STATE，atCheckLetter成功，则进入AT_PARA_NO_QUOT_LETTER_STATE */
    { At_CheckLetter, AT_PARA_NO_QUOT_LETTER_STATE },

    /* 如果当前状态是AT_PARA_NO_QUOT_LETTER_STATE，atCheckRightBracket成功，则进入AT_PARA_RIGHT_BRACKET_STATE */
    { At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* 支持无双引号包括字符串 END */

/* 参数匹配主状态表 */
static const AT_MainState g_atMainParaStateTab[] = {
    /* 如果当前状态是AT_NONE_STATE，则进入g_atParaNoneStateTab子状态表 */
    { AT_NONE_STATE, g_atParaNoneStateTab },

    /* 如果当前状态是AT_PARA_LEFT_BRACKET_STATE，则进入g_atParaLeftBracketStateTab子状态表 */
    { AT_PARA_LEFT_BRACKET_STATE, g_atParaLeftBracketStateTab },

    /* 如果当前状态是AT_PARA_RIGHT_BRACKET_STATE，则进入g_atParaRightBracketStateTab子状态表 */
    { AT_PARA_RIGHT_BRACKET_STATE, g_atParaRightBracketStateTab },

    /* 如果当前状态是AT_PARA_LETTER_STATE，则进入g_atParaLetterStateTab子状态表 */
    { AT_PARA_LETTER_STATE, g_atParaLetterStateTab },

    /* 如果当前状态是AT_PARA_NUM_STATE，则进入g_atParaNumStateTab子状态表 */
    { AT_PARA_NUM_STATE, g_atParaNumStateTab },

    /* 如果当前状态是AT_PARA_NUM_COLON_STATE，则进入g_atParaNumColonStateTab子状态表 */
    { AT_PARA_NUM_COLON_STATE, g_atParaNumColonStateTab },

    /* 如果当前状态是AT_PARA_SUB_STATE，则进入g_atParaSubStateTab子状态表 */
    { AT_PARA_SUB_STATE, g_atParaSubStateTab },

    /* 如果当前状态是AT_PARA_NUM_SUB_STATE，则进入g_atParaNumSubStateTab子状态表 */
    { AT_PARA_NUM_SUB_STATE, g_atParaNumSubStateTab },

    /* 如果当前状态是AT_PARA_NUM_SUB_COLON_STATE，则进入g_atParaNumSubColonStateTab子状态表 */
    { AT_PARA_NUM_SUB_COLON_STATE, g_atParaNumSubColonStateTab },

    /* 如果当前状态是AT_PARA_COLON_STATE，则进入g_atParaColonStateTab子状态表 */
    { AT_PARA_COLON_STATE, g_atParaColonStateTab },

    /* 如果当前状态是AT_PARA_LEFT_QUOT_STATE，则进入g_atParaLeftQuotStateTab子状态表 */
    { AT_PARA_LEFT_QUOT_STATE, g_atParaLeftQuotStateTab },

    /* 如果当前状态是AT_PARA_RIGHT_QUOT_STATE，则进入g_atParaRightQuotStateTab子状态表 */
    { AT_PARA_RIGHT_QUOT_STATE, g_atParaRightQuotStateTab },

    /* 如果当前状态是AT_PARA_QUOT_COLON_STATE，则进入g_atParaQuotColonStateTab子状态表 */
    { AT_PARA_QUOT_COLON_STATE, g_atParaQuotColonStateTab },

    /* 如果当前状态是AT_PARA_ZERO_STATE，则进入g_atParaZeroStateTab子状态表 */
    { AT_PARA_ZERO_STATE, g_atParaZeroStateTab },

    /* 如果当前状态是AT_PARA_ZERO_SUB_STATE，则进入g_atParaZeroSubStateTab子状态表 */
    { AT_PARA_ZERO_SUB_STATE, g_atParaZeroSubStateTab },

    /* 如果当前状态是AT_PARA_HEX_STATE，则进入g_atParaHexStateTab子状态表 */
    { AT_PARA_HEX_STATE, g_atParaHexStateTab },

    /* 如果当前状态是AT_PARA_HEX_NUM_STATE，则进入g_atParaHexNumStateTab子状态表 */
    { AT_PARA_HEX_NUM_STATE, g_atParaHexNumStateTab },

    /* 如果当前状态是AT_PARA_HEX_SUB_STATE，则进入g_atParaHexSubStateTab子状态表 */
    { AT_PARA_HEX_SUB_STATE, g_atParaHexSubStateTab },

    /* 如果当前状态是AT_PARA_HEX_NUM_SUB_STATE，则进入g_atParaHexNumSubStateTab子状态表 */
    { AT_PARA_HEX_NUM_SUB_STATE, g_atParaHexNumSubStateTab },

    /* 如果当前状态是AT_PARA_NO_QUOT_LETTER_STATE，则进入g_atParaNoQuotLetterStateTab子状态表 */
    { AT_PARA_NO_QUOT_LETTER_STATE, g_atParaNoQuotLetterStateTab },

    /* 主状态表结束 */
    { AT_BUTT_STATE, NULL },
};

pAtChkFuncType g_atCheckFunc = NULL;

VOS_UINT32 AT_CmparePara(VOS_VOID)
{
    /* 检查输入参数 */
    if (g_atCheckFunc != NULL) {
        /* 检查参数列表对应的参数是否存在 */
        if (g_atParaList[g_atParseCmd.paraCheckIndex].paraLen > 0) {
            /* 如果参数检查错误 */
            if (g_atCheckFunc(&g_atParaList[g_atParseCmd.paraCheckIndex]) != AT_SUCCESS) {
                return AT_FAILURE; /*  返回错误 */
            }
        }

        /* 记录已检查参数的全局变量加1 */
        g_atParseCmd.paraCheckIndex++;

        /* 清空，以备比较下一个参数,否则，对比参数继续增加 */
        if (g_atParseCmd.paraNumRangeIndex != 0) {
            g_atParseCmd.paraNumRangeIndex = 0;
            (VOS_VOID)memset_s(g_atParseCmd.paraNumRange, sizeof(g_atParseCmd.paraNumRange), 0x00,
                               sizeof(g_atParseCmd.paraNumRange));
        }

        if (g_atParseCmd.paraStrRangeIndex != 0) {
            g_atParseCmd.paraStrRangeIndex = 0;
            (VOS_VOID)memset_s(g_atParseCmd.strRange, sizeof(g_atParseCmd.strRange), 0x00, sizeof(g_atParseCmd.strRange));
        }

        g_atCheckFunc = NULL;

        return AT_SUCCESS; /*  返回正确 */
    } else {
        return AT_FAILURE; /*  返回错误 */
    }
}

LOCAL VOS_VOID AT_ParseParaRightBracketHexNumState(VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    /* 转成数字拷贝到数字参数脚本列表的下界 */
    g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].small = atRangeToU32(copyPtr,
                                                                                   currPtr);

    /* 上界等于下界 */
    g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].big =
        g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].small;

    /* 数字参数索引加1 */
    g_atParseCmd.paraNumRangeIndex++;

    g_atCheckFunc = atCheckNumPara; /* 设置参数比较函数 */
}
/*
 * 功能描述: 在状态RightBracket解析参数
 */
LOCAL VOS_UINT32 AT_ParseParaRightBracketState(AT_StateType currState, VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    switch (currState) {
        case AT_PARA_NUM_SUB_STATE: /* 参数匹配范围数字状态 */
        case AT_PARA_ZERO_SUB_STATE:
        case AT_PARA_HEX_NUM_SUB_STATE:

            /* 转成数字拷贝到数字参数脚本列表的上界 */
            /* 数字参数索引加1 */
            g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex++].big = atRangeToU32(copyPtr,
                                                                                           currPtr);

            g_atCheckFunc = atCheckNumPara; /* 设置参数比较函数 */

            break;

        case AT_PARA_NUM_STATE:
        case AT_PARA_ZERO_STATE:
        case AT_PARA_HEX_NUM_STATE:
            AT_ParseParaRightBracketHexNumState(copyPtr, currPtr);
            break;

        case AT_PARA_LETTER_STATE:

            /* 拷贝到字符参数脚本列表 */
            if ((VOS_UINT32)(currPtr - copyPtr) <
                sizeof(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex])) {
                atRangeCopy(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex++], copyPtr, currPtr);
            } else {
                return AT_FAILURE; /* 返回错误 */
            }

            /* 设置参数比较函数 */
            g_atCheckFunc = At_CheckStringPara;

            break;

        case AT_PARA_NO_QUOT_LETTER_STATE: /* 无双引号包括字母状态 */

            /* 拷贝到字符参数脚本列表 */
            if ((VOS_UINT32)(currPtr - copyPtr) <
                sizeof(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex])) {
                atRangeCopy(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex], copyPtr, currPtr);
            } else {
                return AT_FAILURE; /* 返回错误 */
            }

            /* 设置参数比较函数 */
            g_atCheckFunc = atCheckNoQuotStringPara;

            break;

        default: /* AT_PARA_RIGHT_QUOT_STATE */

            /* 拷贝到字符参数脚本列表 */
            if ((VOS_UINT32)(currPtr - copyPtr) <
                sizeof(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex])) {
                atRangeCopy(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex++], copyPtr, currPtr);
            } else {
                return AT_FAILURE; /* 返回错误 */
            }

            /* 设置参数比较函数 */
            g_atCheckFunc = atCheckCharPara;

            break;
    }

    if (AT_CmparePara() == AT_FAILURE) { /* 参数比较 */
        return AT_FAILURE;              /* 返回错误 */
    }
    return AT_SUCCESS;
}
/*
 * 功能描述: 检查解析参数的长度合法性
 */
LOCAL VOS_UINT32 AT_CheckParseParaLen(VOS_UINT16 *length, VOS_UINT16 len)
{
    if (((*length)++ < len) && (g_atParseCmd.paraNumRangeIndex < AT_MAX_PARA_NUMBER) &&
        (g_atParseCmd.paraStrRangeIndex < AT_MAX_PARA_NUMBER)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
LOCAL VOS_VOID AT_ParseParaNoQuotLetterState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr,
    VOS_UINT8 *currPtr)
{
    if (currState != newState) {   /* 新状态部分不等于当前状态 */
        *copyPtr = currPtr;         /*  准备接收参数 */
    }
}

LOCAL VOS_VOID AT_ParseParaNumColonState(VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    /* 转成数字拷贝到数字参数脚本列表的下界 */
    g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].small = atRangeToU32(copyPtr, currPtr);

    /* 上界等于下界 */
    g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].big =
        g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].small;

    /* 数字参数索引加1 */
    g_atParseCmd.paraNumRangeIndex++;
}
/*
 * 功能描述: 在状态QuotColon解析参数
 */
LOCAL VOS_UINT32 AT_ParseParaQuotColonState(VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    /* 拷贝到字符参数脚本列表 */
    /* 字符参数索引加1 */
    if ((VOS_UINT32)(currPtr - copyPtr) < sizeof(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex])) {
        atRangeCopy(g_atParseCmd.strRange[g_atParseCmd.paraStrRangeIndex++], copyPtr, currPtr);
    } else {
        return AT_FAILURE; /* 返回错误 */
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ParsePara(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16   length    = 0;             /*  记录当前已经处理的字符个数 */
    VOS_UINT8   *currPtr   = data;          /*  指向当前正在处理的字符 */
    VOS_UINT8   *copyPtr   = data;          /*  拷贝内容的起始指针 */
    AT_StateType currState = AT_NONE_STATE; /*  设置初始状态 */
    AT_StateType newState  = AT_NONE_STATE; /*  设置初始状态 */

    /* 依次分析字符串中的每个字符 */
    /* 依次比较每个字符 */
    while (AT_CheckParseParaLen(&length, len) == VOS_TRUE) {
        currState = newState; /*  当前状态设置为新状态 */

        /* 根据当前处理的字符和当前状态查表得到新状态 */
        newState = atFindNextMainState(g_atMainParaStateTab, *currPtr, currState);

        switch (newState) {
            /* 状态处理 */
            case AT_PARA_NUM_STATE:            /* 参数状态 */
            case AT_PARA_LETTER_STATE:         /* 字母状态 */
            case AT_PARA_LEFT_QUOT_STATE:      /* 参数左引号状态 */
            case AT_PARA_NUM_SUB_STATE:        /* 范围数字状态 */
            case AT_PARA_ZERO_STATE:           /* 数字0状态 */
            case AT_PARA_ZERO_SUB_STATE:       /* 范围数字0状态 */
            case AT_PARA_NO_QUOT_LETTER_STATE: /* 无双引号包括字母状态 */
                AT_ParseParaNoQuotLetterState(currState, newState, &copyPtr, currPtr);
                break;

            case AT_PARA_QUOT_COLON_STATE: /* 引号逗号状态 */
                if (AT_ParseParaQuotColonState(copyPtr, currPtr) == AT_FAILURE) {
                    return AT_FAILURE;
                }
                break;

            case AT_PARA_NUM_SUB_COLON_STATE:

                /* 转成数字拷贝到数字参数脚本列表的上界 */
                /* 数字参数索引加1 */
                g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex++].big = atRangeToU32(copyPtr, currPtr);

                break;

            case AT_PARA_NUM_COLON_STATE:
                AT_ParseParaNumColonState(copyPtr, currPtr);
                break;

            case AT_PARA_SUB_STATE:
                /* 转成数字拷贝到数字参数脚本列表的下界 */
                g_atParseCmd.paraNumRange[g_atParseCmd.paraNumRangeIndex].small = atRangeToU32(copyPtr, currPtr);
                break;

            case AT_PARA_RIGHT_BRACKET_STATE: /* 参数匹配右括号状态 */
                if (AT_ParseParaRightBracketState(currState, copyPtr, currPtr) == AT_FAILURE) {
                    return AT_FAILURE;
                }
                break;

            case AT_BUTT_STATE:    /* 无效状态 */
                return AT_FAILURE; /* 返回错误 */
            default:
                break;
        }
        currPtr++; /*  继续分析下一个字符 */
    }

    if (newState == AT_NONE_STATE) { /* 初始状态 */
        return AT_FAILURE;           /* 返回错误 */
    }

    return AT_SUCCESS; /*  返回正确 */
}

