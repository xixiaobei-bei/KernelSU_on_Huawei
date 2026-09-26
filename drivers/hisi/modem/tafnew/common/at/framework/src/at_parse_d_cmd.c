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

#include "at_parse_d_cmd.h"
#include "securec.h"
#include "at_check_func.h"
#include "at_mntn.h"

#include "msp_errno.h"

#include "at_common.h"
#include "ps_tag.h"


/* AT命令初始状态表 */
static const AT_SubState g_atDCmdNoneStateTab[] = {
    /* 如果当前状态是ATCMD_NONE_STATE，atCheckCharD成功，则进入AT_D_CMD_NAME_STATE */
    { At_CheckCharD, AT_D_CMD_NAME_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令命令名状态表 */
static const AT_SubState g_atDCmdNameStateTab[] = {
    /* 如果当前状态是AT_D_CMD_NAME_STATE，At_CheckCharRightArrow成功，则进入AT_D_CMD_RIGHT_ARROW_STATE */
    { At_CheckCharRightArrow, AT_D_CMD_RIGHT_ARROW_STATE },

    /* 如果当前状态是AT_D_CMD_NAME_STATE，atCheckDailString成功，则进入AT_D_CMD_DIALSTRING_STATE */
    { At_CheckDialNum, AT_D_CMD_DIALSTRING_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令拨号字符串状态表 */
static const AT_SubState g_atDCmdDialstringStateTab[] = {
    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，atCheckDailString成功，则进入AT_D_CMD_DIALSTRING_STATE */
    { At_CheckDialNum, AT_D_CMD_DIALSTRING_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckCharI成功，则进入AT_D_CMD_CHAR_I_STATE */
    { At_CheckCharI, AT_D_CMD_CHAR_I_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckCharG成功，则进入AT_D_CMD_CHAR_G_STATE */
    { At_CheckCharG, AT_D_CMD_CHAR_G_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckSemicolon成功，则进入AT_D_CMD_SEMICOLON_STATE */
    { At_CheckSemicolon, AT_D_CMD_SEMICOLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令字符I状态表 */
static const AT_SubState g_atDCmdCharIStateTab[] = {
    /* 如果当前状态是AT_D_CMD_CHAR_I_STATE，At_CheckCharG成功，则进入AT_D_CMD_CHAR_G_STATE */
    { At_CheckCharG, AT_D_CMD_CHAR_G_STATE },

    /* 如果当前状态是AT_D_CMD_CHAR_I_STATE，At_CheckSemicolon成功，则进入AT_D_CMD_SEMICOLON_STATE */
    { At_CheckSemicolon, AT_D_CMD_SEMICOLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令字符G状态表 */
static const AT_SubState g_atDCmdCharGStateTab[] = {
    /* 如果当前状态是AT_D_CMD_CHAR_G_STATE，At_CheckSemicolon成功，则进入AT_D_CMD_SEMICOLON_STATE */
    { At_CheckSemicolon, AT_D_CMD_SEMICOLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令右箭头状态表 */
static const AT_SubState g_atDCmdRightArrowStateTab[] = {
    /* 如果当前状态是AT_D_CMD_RIGHT_ARROW_STATE，atCheck_quot成功，则进入AT_D_CMD_LEFT_QUOT_STATE */
    { At_CheckQuot, AT_D_CMD_LEFT_QUOT_STATE },

    /* 如果当前状态是AT_D_CMD_RIGHT_ARROW_STATE，atCheck_num成功，则进入AT_D_CMD_DIGIT_STATE */
    { At_CheckDigit, AT_D_CMD_DIGIT_STATE },

    /* 如果当前状态是AT_D_CMD_RIGHT_ARROW_STATE，At_CheckUpLetter成功，则进入AT_D_CMD_CHAR_STATE */
    { At_CheckUpLetter, AT_D_CMD_CHAR_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令字母状态表 */
static const AT_SubState g_atDCmdCharStateTab[] = {
    /* 如果当前状态是AT_D_CMD_CHAR_STATE，atCheckCharStr成功，则进入AT_D_CMD_CHAR_STATE */
    { At_CheckUpLetter, AT_D_CMD_CHAR_STATE },

    /* 如果当前状态是AT_D_CMD_CHAR_STATE，atCheck_quot成功，则进入AT_D_CMD_RIGHT_QUOT_STATE */
    { At_CheckDigit, AT_D_CMD_DIGIT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令左引号状态表 */
static const AT_SubState g_atDCmdLeftQuotStateTab[] = {
    /* 如果当前状态是AT_D_CMD_LEFT_QUOT_STATE，atCheckCharStr成功，则进入AT_D_CMD_LEFT_QUOT_STATE */
    { At_CheckNoQuot, AT_D_CMD_LEFT_QUOT_STATE },

    /* 如果当前状态是AT_D_CMD_LEFT_QUOT_STATE，atCheck_quot成功，则进入AT_D_CMD_RIGHT_QUOT_STATE */
    { At_CheckQuot, AT_D_CMD_RIGHT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT D命令右引号状态表 */
static const AT_SubState g_atDCmdRightQuotStateTab[] = {
    /* 如果当前状态是AT_D_CMD_RIGHT_ARROW_STATE，atCheck_num成功，则进入AT_D_CMD_DIGIT_STATE */
    { At_CheckDigit, AT_D_CMD_DIGIT_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckCharI成功，则进入AT_D_CMD_CHAR_I_STATE */
    { At_CheckCharI, AT_D_CMD_CHAR_I_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckCharG成功，则进入AT_D_CMD_CHAR_G_STATE */
    { At_CheckCharG, AT_D_CMD_CHAR_G_STATE },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，At_CheckSemicolon成功，则进入AT_D_CMD_SEMICOLON_STATE */
    { At_CheckSemicolon, AT_D_CMD_SEMICOLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT命令主状态表 */
static const AT_MainState g_atDCmdMainStateTab[] = {
    /* 如果当前状态是AT_NONE_STATE，则进入g_atDCmdNoneStateTab子状态表 */
    { AT_NONE_STATE, g_atDCmdNoneStateTab },

    /* 如果当前状态是AT_D_CMD_NAME_STATE，则进入g_atDCmdNameStateTab子状态表 */
    { AT_D_CMD_NAME_STATE, g_atDCmdNameStateTab },

    /* 如果当前状态是AT_D_CMD_DIALSTRING_STATE，则进入g_atDCmdDialstringStateTab子状态表 */
    { AT_D_CMD_DIALSTRING_STATE, g_atDCmdDialstringStateTab },

    /* 如果当前状态是AT_D_CMD_CHAR_I_STATE，则进入g_atDCmdCharIStateTab子状态表 */
    { AT_D_CMD_CHAR_I_STATE, g_atDCmdCharIStateTab },

    /* 如果当前状态是AT_D_CMD_CHAR_G_STATE，则进入g_atDCmdCharGStateTab子状态表 */
    { AT_D_CMD_CHAR_G_STATE, g_atDCmdCharGStateTab },

    /* 如果当前状态是AT_D_CMD_CHAR_STATE，则进入g_atDCmdCharStateTab子状态表 */
    { AT_D_CMD_CHAR_STATE, g_atDCmdCharStateTab },

    /* 如果当前状态是AT_D_CMD_RIGHT_ARROW_STATE，则进入g_atDCmdRightArrowStateTab子状态表 */
    { AT_D_CMD_RIGHT_ARROW_STATE, g_atDCmdRightArrowStateTab },

    /* 如果当前状态是AT_D_CMD_LEFT_QUOT_STATE，则进入g_atDCmdLeftQuotStateTab子状态表 */
    { AT_D_CMD_LEFT_QUOT_STATE, g_atDCmdLeftQuotStateTab },

    /* 如果当前状态是AT_D_CMD_RIGHT_QUOT_STATE，则进入g_atDCmdRightQuotStateTab子状态表 */
    { AT_D_CMD_RIGHT_QUOT_STATE, g_atDCmdRightQuotStateTab },

    /* 如果当前状态是AT_D_CMD_DIGIT_STATE，则进入AT_D_CMD_DIGIT_STATE_TAB子状态表 */
    { AT_D_CMD_DIGIT_STATE, g_atDCmdRightQuotStateTab },

    /* 主状态表结束 */
    { AT_BUTT_STATE, NULL },
};

LOCAL VOS_VOID AT_ParseDCmdNameState(VOS_UINT8 **copyPtr, VOS_UINT8 *currPtr)
{
    g_atParseCmd.cmdFmtType = AT_BASIC_CMD_TYPE;          /* 设置命令类型 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA; /* 设置命令操作类型 */
    *copyPtr                 = currPtr;                    /* 准备接收D类型命令名 */
}

LOCAL VOS_UINT32 AT_ParseDCmdDialStringState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr,
    VOS_UINT8 *currPtr)
{
    if (currState != newState) { /* 新状态部分不等于当前状态 */
        g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)(currPtr - *copyPtr);
        if (g_atParseCmd.cmdName.cmdNameLen < sizeof(g_atParseCmd.cmdName.cmdName)) {
            atRangeCopy(g_atParseCmd.cmdName.cmdName, *copyPtr, currPtr);
        } else {
            g_atParseCmd.cmdName.cmdNameLen = 0;
            return AT_FAILURE; /* 返回错误 */
        }
        *copyPtr = currPtr; /* 准备接收参数 */
    }

    return AT_SUCCESS;
}
LOCAL VOS_UINT32 AT_PraseDLastState(AT_StateType newState, VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    switch (newState) {
        /* 根据最后的状态判断 */
        case AT_D_CMD_SEMICOLON_STATE: /* AT D命令分号状态 */
            break;

        case AT_D_CMD_DIALSTRING_STATE: /* AT D命令拨号字符串状态 */
        case AT_D_CMD_DIGIT_STATE:      /* AT D命令数字状态 */
        case AT_D_CMD_RIGHT_QUOT_STATE: /* AT D命令右引号状态 */
        case AT_D_CMD_CHAR_G_STATE:     /* AT D命令字符G状态 */
        case AT_D_CMD_CHAR_I_STATE:     /* AT D命令字符I状态 */
            /* 存储参数到全局变量中 */
            if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
                return AT_FAILURE;
            }

            break;

        default:
            return AT_FAILURE; /* 返回错误 */
    }
    return AT_SUCCESS; /* 返回正确 */
}
LOCAL VOS_UINT32 AT_ParseDCmd(VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_StateType currState = AT_NONE_STATE; /*  设置初始状态 */
    AT_StateType newState  = AT_NONE_STATE; /*  设置初始状态 */
    VOS_UINT8   *currPtr   = data;          /*  指向当前正在处理的字符 */
    VOS_UINT8   *copyPtr   = data;          /*  拷贝内容的起始指针 */
    VOS_UINT16   length    = 0;             /*  记录当前已经处理的字符个数 */

    /* 依次分析字符串中的每个字符 */
    /* 依次比较每个字符 */
    while ((length++ < len) && (g_atParseCmd.paraIndex < AT_MAX_PARA_NUMBER)) {
        currState = newState; /* 当前状态设置为新状态 */

        /* 根据当前处理的字符和当前状态查表得到新状态 */
        newState = atFindNextMainState(g_atDCmdMainStateTab, *currPtr, currState);

        switch (newState) {
            /* 状态处理 */
            case AT_D_CMD_NAME_STATE:                                 /* AT D命令命令名状态 */
                AT_ParseDCmdNameState(&copyPtr, currPtr);
                break;

            case AT_D_CMD_RIGHT_ARROW_STATE: /* AT D命令右箭头状态 */
            case AT_D_CMD_DIALSTRING_STATE:  /* AT D命令拨号字符串状态 */
                if (AT_ParseDCmdDialStringState(currState, newState, &copyPtr, currPtr) != AT_SUCCESS) {
                    return AT_FAILURE;
                }
                break;

            case AT_D_CMD_DIGIT_STATE:     /* AT D命令数字状态 */
            case AT_D_CMD_LEFT_QUOT_STATE: /* AT D命令左引号状态 */
            case AT_D_CMD_CHAR_STATE:
                if (currState != newState) { /* 新状态部分不等于当前状态 */
                    /* 存储参数到全局变量中 */
                    if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
                        return AT_FAILURE;
                    }

                    copyPtr = currPtr; /* 准备接收下一个参数 */
                }
                break;

            case AT_D_CMD_CHAR_I_STATE:    /* AT D命令字符I状态 */
            case AT_D_CMD_CHAR_G_STATE:    /* AT D命令字符G状态 */
            case AT_D_CMD_SEMICOLON_STATE: /* AT D命令分号状态 */
                /* 存储参数到全局变量中 */
                if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
                    return AT_FAILURE;
                }

                copyPtr = currPtr; /* 准备接收下一个参数 */
                break;

            case AT_BUTT_STATE:    /* 无效状态 */
                return AT_FAILURE; /* 返回错误 */

            default:
                break;
        }
        currPtr++; /* 继续分析下一个字符 */
    }

    return AT_PraseDLastState(newState, copyPtr, currPtr);
}

VOS_VOID atCdataParseDCmd(VOS_UINT8 *data, VOS_UINT16 len)
{
    g_atParseCmd.cmdFmtType = AT_BASIC_CMD_TYPE;          /* 设置命令类型 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA; /* 设置命令操作类型 */

    /* 接收D类型命令名 */
    g_atParseCmd.cmdName.cmdNameLen = 1;
    g_atParseCmd.cmdName.cmdName[0] = *data;
}

LOCAL VOS_VOID atScanDcmdModifier(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT16 chkLen  = 0;
    VOS_UINT16 lenTemp = 0;
    VOS_UINT8 *write   = data;
    VOS_UINT8 *read    = data;

    /* 输入参数检查 */
    while (chkLen++ < *len) {
        /* 删除修饰符，',','T','P','!','W','@' */
        if ((*read != ',') && (*read != 'T') && (*read != 'P') && (*read != '!') && (*read != 'W') && (*read != '@')) {
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }

    *len = lenTemp;
}

LOCAL VOS_UINT32 AT_IsDmCmdModifierChar(VOS_UINT8 read)
{
    if ((read != ',') && (read != 'T') && (read != 'P') && (read != '!') && (read != 'W') &&
        (read != '@')) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

LOCAL VOS_VOID AT_ScanDmcmdModifier(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT8         *write   = VOS_NULL_PTR;
    VOS_UINT8         *read    = VOS_NULL_PTR;
    VOS_UINT8         *tmp     = VOS_NULL_PTR;
    VOS_UINT8         *dataTmp = VOS_NULL_PTR;
    const VOS_UINT8    str[]   = "*99**PPP";
    VOS_UINT16         lenTemp = 0;

    dataTmp = (VOS_UINT8 *)AT_MALLOC(*len + 1);
    if (dataTmp == VOS_NULL_PTR) {
        PS_PRINTF_WARNING("<AT_ScanDmcmdModifier> AT_MALLOC Failed!\n");
        return;
    }

    (VOS_VOID)memset_s(dataTmp, *len + 1, 0x00, *len + 1);
    atRangeCopy(dataTmp, data, (data + *len));

    /* 保证字符串有结束符 */
    dataTmp[*len] = 0;

    read  = dataTmp;
    write = dataTmp;

    tmp = (VOS_UINT8 *)AT_STRSTR((VOS_CHAR *)dataTmp, (VOS_CHAR *)str);
    if (tmp != NULL) {
        /* 检查前一半参数 */
        while (read < tmp) {
            /* 删除修饰符，',','T','P','!','W','@' */
            if (AT_IsDmCmdModifierChar(*read) == VOS_FALSE) {
                *write++ = *read;
                lenTemp++;
            }
            read++;
        }
        while (read < (tmp + (VOS_UINT16)AT_STRLEN((VOS_CHAR *)str))) {
            *write++ = *read++;
            lenTemp++;
        }
        while (read < (dataTmp + (*len))) {
            /* 删除修饰符，',','T','P','!','W','@' */
            if (AT_IsDmCmdModifierChar(*read) == VOS_FALSE) {
                *write++ = *read;
                lenTemp++;
            }
            read++;
        }
    } else {
        /* 输入参数检查 */
        while (read < (dataTmp + (*len))) {
            /* 删除修饰符，',','T','P','!','W','@' */
            if (AT_IsDmCmdModifierChar(*read) == VOS_FALSE) {
                *write++ = *read;
                lenTemp++;
            }
            read++;
        }
    }

    atRangeCopy(data, dataTmp, (dataTmp + *len));
    *len = lenTemp;
    AT_FREE(dataTmp);
}

LOCAL VOS_UINT32 ScanQuateChar(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT8 *write   = data;
    VOS_UINT8 *read    = data;
    VOS_UINT16 chkLen  = 0;
    VOS_UINT16 lenTemp = 0;

    if (*len == 0) {
        return AT_FAILURE;
    }

    while (chkLen++ < *len) {
        /* 去除引号 */
        if (*read != '"') {
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }
    *len = lenTemp;

    return AT_SUCCESS;
}

VOS_UINT32 ParseDCmdPreProc(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16 tmpLen = len;
    VOS_UINT8 *tmp    = data;
    VOS_UINT32 result = AT_FAILURE;

    /* 为了规避LINUX下拨号命令中带有引号(ATDT"*99#")的问题，将引号去除 */
    ScanQuateChar(tmp, &tmpLen);

    {
        if (At_CheckCharWell(tmp[tmpLen - 1]) == AT_SUCCESS) { /* 如果不是以#号结尾 */
            AT_ScanDmcmdModifier(tmp, &tmpLen);                 /* 扫描修饰符 */

            /*
             * 因为要把命令名拷贝到gstATCmdName.usCmdNameLen中以备命令匹配，
             * 所以，把内容拷贝到参数列表的第一个缓存中
             */
            if (atfwParseSaveParam(tmp, tmpLen) != ERR_MSP_SUCCESS) {
                return AT_CME_DIAL_STRING_TOO_LONG;
            }

            result = AT_ParseDMCmd(tmp, tmpLen); /* 如果带分号 */
            if (result == AT_FAILURE) {
                return AT_ERROR;
            }
        }
        /* 解析#777,判断"#777"命令字符 */
        else if (AT_CheckStrStartWith(tmp, tmpLen, (VOS_UINT8 *)"D#777", AT_CDATA_DIAL_777_LEN + 1)) {
            result = AT_SaveCdataDialParam(tmp, tmpLen);

            if (result == AT_FAILURE) {
                return AT_ERROR;
            }

            atCdataParseDCmd(tmp, tmpLen);
        } else {
            if (atCheckRightArrowStr(tmp, tmpLen) == AT_FAILURE) {
                atScanDcmdModifier(tmp, &tmpLen); /* 扫描修饰符 */
            }

            /*
             * 因为要把命令名拷贝到gstATCmdName.usCmdNameLen中以备命令匹配，
             * 所以，把内容拷贝到参数列表的第一个缓存中
             */
            if (atfwParseSaveParam(tmp, tmpLen) != ERR_MSP_SUCCESS) {
                return AT_CME_DIAL_STRING_TOO_LONG;
            }

            result = AT_ParseDCmd(tmp, tmpLen); /* D命令 */

            if (result == AT_FAILURE) {
                return AT_ERROR;
            }
        }
    }

    return result;
}

