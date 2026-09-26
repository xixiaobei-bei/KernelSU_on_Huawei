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

#ifndef _AT_PARSE_CMD_H_
#define _AT_PARSE_CMD_H_

#include "taf_type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_CHECK_BASE_HEX 16
#define AT_CHECK_BASE_OCT 8
#define AT_CHECK_BASE_DEC 10
#define AT_BINSTRARRY_PSIZE 17
#define AT_BINSTRARRY_SSIZE 5
#define AT_HEX2BIN_SIZE 4  /* 1位16进制数转为2进制数的长度 */

typedef enum {
    AT_NONE_STATE, /* 初始状态 */

    AT_B_CMD_NAME_STATE, /* AT基本命令名状态 */
    AT_B_CMD_PARA_STATE, /* AT基本命令参数状态 */

    AT_D_CMD_NAME_STATE,        /* AT D命令命令名状态 */
    AT_D_CMD_DIGIT_STATE,       /* AT D命令数字状态 */
    AT_D_CMD_CHAR_STATE,        /* AT D命令字母状态 */
    AT_D_CMD_RIGHT_ARROW_STATE, /* AT D命令右箭头状态 */
    AT_D_CMD_DIALSTRING_STATE,  /* AT D命令拨号字符串状态 */
    AT_D_CMD_SEMICOLON_STATE,   /* AT D命令分号状态 */
    AT_D_CMD_CHAR_G_STATE,      /* AT D命令字符G状态 */
    AT_D_CMD_CHAR_I_STATE,      /* AT D命令字符I状态 */
    AT_D_CMD_LEFT_QUOT_STATE,   /* AT D命令左引号状态 */
    AT_D_CMD_RIGHT_QUOT_STATE,  /* AT D命令右引号状态 */

    AT_DM_CMD_NAME_STATE,
    AT_DM_CMD_STAR_STATE, /* AT D命令第一个*状态 */
    AT_DM_CMD_WELL_STATE, /* AT D命令字符#状态 */
    AT_DM_CMD_NUM_STATE,  /* AT D命令字符#状态 */

    AT_S_CMD_NAME_STATE, /* AT S命令名状态 */
    AT_S_CMD_SET_STATE,  /* AT S命令设置状态 */
    AT_S_CMD_READ_STATE, /* AT S命令查询状态 */
    AT_S_CMD_TEST_STATE, /* AT S命令测试状态 */
    AT_S_CMD_PARA_STATE, /* AT S命令参数状态 */

    AT_E_CMD_NAME_STATE,       /* AT扩展命令名状态 */
    AT_E_CMD_SET_STATE,        /* AT扩展命令等号状态 */
    AT_E_CMD_TEST_STATE,       /* AT扩展命令查询参数状态 */
    AT_E_CMD_READ_STATE,       /* AT扩展命令测试参数状态 */
    AT_E_CMD_PARA_STATE,       /* AT扩展命令参数状态 */
    AT_E_CMD_COLON_STATE,      /* AT扩展命令逗号状态 */
    AT_E_CMD_LEFT_QUOT_STATE,  /* AT扩展命令左引号状态 */
    AT_E_CMD_RIGHT_QUOT_STATE, /* AT扩展命令右引号状态 */

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE,  /* AT扩展命令引号嵌套转义状态 */
#endif

    AT_PARA_LEFT_BRACKET_STATE,   /* 参数匹配左括号状态 */
    AT_PARA_NUM_STATE,            /* 参数匹配参数状态 */
    AT_PARA_LETTER_STATE,         /* 参数匹配字母状态 */
    AT_PARA_NUM_COLON_STATE,      /* 参数匹配数字逗号状态 */
    AT_PARA_NUM_SUB_STATE,        /* 参数匹配范围数字状态 */
    AT_PARA_NUM_SUB_COLON_STATE,  /* 参数匹配范围数字逗号状态 */
    AT_PARA_QUOT_COLON_STATE,     /* 参数匹配引号逗号状态 */
    AT_PARA_RIGHT_BRACKET_STATE,  /* 参数匹配右括号状态 */
    AT_PARA_SUB_STATE,            /* 参数匹配范围状态 */
    AT_PARA_LEFT_QUOT_STATE,      /* 参数匹配左引号状态 */
    AT_PARA_RIGHT_QUOT_STATE,     /* 参数匹配右引号状态 */
    AT_PARA_COLON_STATE,          /* 参数匹配逗号状态 */
    AT_PARA_ZERO_STATE,           /* 参数匹配数字0状态 */
    AT_PARA_ZERO_SUB_STATE,       /* 参数匹配范围数字0状态 */
    AT_PARA_HEX_STATE,            /* 参数匹配十六进制状态, 0x或0X */
    AT_PARA_HEX_SUB_STATE,        /* 参数匹配范围十六进制状态 */
    AT_PARA_HEX_NUM_STATE,        /* 参数匹十六进制数状态 */
    AT_PARA_HEX_NUM_SUB_STATE,    /* 参数匹配范围十六进制数字状态 */
    AT_PARA_NO_QUOT_LETTER_STATE, /* 参数匹配无双引号包括字母状态 */

    AT_W_CMD_F_STATE,    /* AT基本命令&状态 */
    AT_W_CMD_NAME_STATE, /* AT基本命令名W状态 */
    AT_W_CMD_PARA_STATE, /* AT基本命令名W状态 */

    AT_BUTT_STATE /* 无效状态 */
} AT_StateType;

/* 模块名＋意义＋HEADER */

/* 判断条件函数指针 */
typedef TAF_UINT32 (*pATChkCharFuncType)(TAF_UINT8);

/* 子状态表类型：如果判断条件函数成功，则进入返回对应的子状态 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    pATChkCharFuncType funcName;  /*    判断条件函数,如果成功，返回next_state */
    AT_StateType       nextState; /*    下一个状态 */
} AT_SubState;
/*lint +e958 +e959 ;cause:64bit*/

/* 当前状态表类型：如果新状态等于当前状态，则进入对应的子状态表 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    AT_StateType        currState;   /* 当前状态 */
    const AT_SubState  *subStateTab; /* 对应的子态表 */
} AT_MainState;
/*lint +e958 +e959 ;cause:64bit*/

/*lint -esym(752,At_RangeCopy)*/
TAF_VOID At_RangeCopy(TAF_UINT8 *dst, TAF_UINT8 *begain, TAF_UINT8 *end);

/* 把字符串中的某一段拷贝到指定地 */
/*lint -esym(752,atRangeCopy)*/
VOS_VOID atRangeCopy(VOS_UINT8 *dst, VOS_UINT8 *begain, VOS_UINT8 *end);

/* 把字符串中的某一段转成无符号整型值 */
/*lint -esym(752,atRangeToU32)*/
VOS_UINT32 atRangeToU32(VOS_UINT8 *begain, VOS_UINT8 *end);

/*lint -esym(752,atFindNextSubState)*/
AT_StateType atFindNextSubState(const AT_SubState *subStateTab, VOS_UINT8 inputChar);

/*lint -esym(752,atFindNextMainState)*/
AT_StateType atFindNextMainState(const AT_MainState *mainStateTab, VOS_UINT8 inputChar, AT_StateType inputState);

/*lint -esym(752,atAuc2ul)*/
extern VOS_UINT32 atAuc2ul(VOS_UINT8 *nptr, VOS_UINT16 len, VOS_UINT32 *rtn);

/*lint -esym(752,At_ul2Auc)*/
VOS_VOID At_ul2Auc(VOS_UINT32 value, TAF_UINT16 len, VOS_UINT8 *rtn);

VOS_VOID AT_HexStr2BinStr(const VOS_CHAR *hexStr, TAF_UINT32 hexStrSize, VOS_CHAR *binStr, TAF_UINT32 binStrSize);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _AT_PARSE_CMD_H_ */
