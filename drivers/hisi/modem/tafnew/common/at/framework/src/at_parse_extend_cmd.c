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

#include "at_parse_extend_cmd.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PARSEEXTENDCMD_C

/* AT扩展命令初始状态状态表 */
static const AT_SubState g_atECmdNoneStateTab[] = {
    /* 如果当前状态是ATCMD_T_STATE，atCheckCharPlus成功，则进入AT_E_CMD_NAME_STATE */
    { At_CheckCharPlus, AT_E_CMD_NAME_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT扩展命令名状态表 */
static const AT_SubState g_atECmdNameStateTab[] = {
    /* 如果当前状态是AT_E_CMD_NAME_STATE，atCheck_char成功，则进入AT_E_CMD_NAME_STATE */
    { At_CheckChar, AT_E_CMD_NAME_STATE },

    /* 如果当前状态是AT_E_CMD_NAME_STATE，atCheck_equ成功，则进入AT_E_CMD_SET_STATE */
    { At_CheckEqu, AT_E_CMD_SET_STATE },

    /* 如果当前状态是AT_E_CMD_NAME_STATE，atCheck_req成功，则进入AT_E_CMD_READ_STATE */
    { At_CheckReq, AT_E_CMD_READ_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT扩展命令等号状态表 */
static const AT_SubState g_atECmdSetStateTab[] = {
    /* 如果当前状态是AT_E_CMD_SET_STATE，atCheck_req成功，则进入AT_E_CMD_TEST_STATE */
    { At_CheckReq, AT_E_CMD_TEST_STATE },

    /* 如果当前状态是AT_E_CMD_SET_STATE，atCheck_char成功，则进入AT_E_CMD_PARA_STATE */
    { At_CheckChar, AT_E_CMD_PARA_STATE },

    /* 如果当前状态是AT_E_CMD_SET_STATE，atCheck_colon成功，则进入AT_E_CMD_COLON_STATE */
    { atCheckComma, AT_E_CMD_COLON_STATE },

    /* 如果当前状态是AT_E_CMD_SET_STATE，atCheck_quot成功，则进入AT_E_CMD_LEFT_QUOT_STATE */
    { At_CheckQuot, AT_E_CMD_LEFT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT扩展命令参数状态表 */
static const AT_SubState g_atECmdParaStateTab[] = {
    /* 如果当前状态是AT_E_CMD_PARA_STATE，atCheck_char成功，则进入AT_E_CMD_PARA_STATE */
    { At_CheckChar, AT_E_CMD_PARA_STATE },

    /* 如果当前状态是AT_E_CMD_PARA_STATE，atCheck_char成功，则进入AT_E_CMD_PARA_STATE */
    { atCheckblank, AT_E_CMD_PARA_STATE },

    /* 如果当前状态是AT_E_CMD_PARA_STATE，atCheckColon成功，则进入AT_E_CMD_PARA_STATE */
    { atCheckColon, AT_E_CMD_PARA_STATE },

    /* 如果当前状态是AT_E_CMD_PARA_STATE，atCheckComma成功，则进入AT_E_CMD_COLON_STATE */
    { atCheckComma, AT_E_CMD_COLON_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT扩展命令逗号状态表 */
static const AT_SubState g_atECmdColonStateTab[] = {
    /* 如果当前状态是AT_E_CMD_COLON_STATE，atCheck_char成功，则进入AT_E_CMD_PARA_STATE */
    { At_CheckChar, AT_E_CMD_PARA_STATE },

    /* 如果当前状态是AT_E_CMD_COLON_STATE，atCheck_colon成功，则进入AT_E_CMD_COLON_STATE */
    { atCheckComma, AT_E_CMD_COLON_STATE },

    /* 如果当前状态是AT_E_CMD_COLON_STATE，atCheck_quot成功，则进入AT_E_CMD_LEFT_QUOT_STATE */
    { At_CheckQuot, AT_E_CMD_LEFT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT扩展命令左引号状态表 */
static const AT_SubState g_atECmdLeftQuotStateTab[] = {
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    /* 如果当前状态是AT_E_CMD_LEFT_QUOT_STATE，At_CheckEspace成功，则进入AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE */
    { AT_CheckEscape, AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE },
#endif

    /* 如果当前状态是AT_E_CMD_LEFT_QUOT_STATE，atCheck_char成功，则进入AT_E_CMD_LEFT_QUOT_STATE */
    { At_CheckNoQuot, AT_E_CMD_LEFT_QUOT_STATE },

    /* 如果当前状态是AT_E_CMD_LEFT_QUOT_STATE，atCheck_quot成功，则进入AT_E_CMD_RIGHT_QUOT_STATE */
    { At_CheckQuot, AT_E_CMD_RIGHT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
/* AT扩展命令括号嵌套转义字符状态表 */
static const AT_SubState g_atECmdQuotNestEscapeCharStateTab[] = {
    /* 如果当前状态是AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE，At_CheckSuccess成功，则进入AT_E_CMD_LEFT_QUOT_STATE */
    { AT_CheckSuccess, AT_E_CMD_LEFT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};
#endif

/* AT扩展命令右引号状态表 */
static const AT_SubState g_atECmdRightQuotStateTab[] = {
    /* 如果当前状态是AT_E_CMD_RIGHT_QUOT_STATE，atCheck_colon成功，则进入AT_E_CMD_COLON_STATE */
    { atCheckComma, AT_E_CMD_COLON_STATE },

    /* 如果当前状态是AT_E_CMD_RIGHT_QUOT_STATE，atCheck_char成功，则进入AT_E_CMD_RIGHT_QUOT_STATE */
    { At_CheckChar, AT_E_CMD_RIGHT_QUOT_STATE },

    /* 如果当前状态是AT_E_CMD_RIGHT_QUOT_STATE，atCheck_quot成功，则进入AT_E_CMD_RIGHT_QUOT_STATE */
    { At_CheckQuot, AT_E_CMD_RIGHT_QUOT_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT命令主状态表 */
static const AT_MainState g_atECmdMainStateTab[] = {
    /* 如果当前状态是AT_NONE_STATE，则进入g_atECmdNoneStateTab子状态表 */
    { AT_NONE_STATE, g_atECmdNoneStateTab },

    /* 如果当前状态是AT_E_CMD_NAME_STATE，则进入g_atECmdNameStateTab子状态表 */
    { AT_E_CMD_NAME_STATE, g_atECmdNameStateTab },

    /* 如果当前状态是AT_E_CMD_SET_STATE，则进入g_atECmdSetStateTab子状态表 */
    { AT_E_CMD_SET_STATE, g_atECmdSetStateTab },

    /* 如果当前状态是AT_E_CMD_PARA_STATE，则进入g_atECmdParaStateTab子状态表 */
    { AT_E_CMD_PARA_STATE, g_atECmdParaStateTab },

    /* 如果当前状态是AT_E_CMD_COLON_STATE，则进入g_atECmdColonStateTab子状态表 */
    { AT_E_CMD_COLON_STATE, g_atECmdColonStateTab },

    /* 如果当前状态是AT_E_CMD_LEFT_QUOT_STATE，则进入g_atECmdLeftQuotStateTab子状态表 */
    { AT_E_CMD_LEFT_QUOT_STATE, g_atECmdLeftQuotStateTab },

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    /* 如果当前状态是AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE，则进入g_atECmdQuotNestEscapeCharStateTab子状态表 */
    { AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE, g_atECmdQuotNestEscapeCharStateTab },
#endif

    /* 如果当前状态是AT_E_CMD_RIGHT_QUOT_STATE，则进入g_atECmdRightQuotStateTab子状态表 */
    { AT_E_CMD_RIGHT_QUOT_STATE, g_atECmdRightQuotStateTab },

    /* 主状态表结束 */
    { AT_BUTT_STATE, NULL },
};

LOCAL VOS_VOID AT_PraseLeftQuotState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr,
    VOS_UINT8 *currPtr)
{
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    if ((currState != newState) && (currState != AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE)) {
        *copyPtr = currPtr;
    }
#else
    if (currState != newState) {
        *copyPtr = currPtr;
    }
#endif
}

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)

LOCAL VOS_UINT32 AT_PraseLeftQuotLastState(AT_StateType currState, VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    if (currState == AT_E_CMD_QUOT_NEST_ESCAPE_CHAR_STATE) {
        /* 存储参数到全局变量中 */
        if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
            return VOS_FALSE;
        }
    }  else {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

LOCAL VOS_UINT32 AT_PraseLastState(AT_StateType currState, AT_StateType newState, VOS_UINT8 *copyPtr,
    VOS_UINT8 *currPtr)
{
    switch (newState) {
        /* 根据最后的状态判断 */
        case AT_E_CMD_PARA_STATE:       /* AT扩展命令参数状态 */
        case AT_E_CMD_RIGHT_QUOT_STATE: /* AT扩展命令右引号状态 */

            /* 存储参数到全局变量中 */
            if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
                return VOS_FALSE;
            }

            break;

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
        /* 最后一个字符状态为左引号状态且前一个字符为转义字符状态，存储参数到全局变量中 */
        case AT_E_CMD_LEFT_QUOT_STATE:
            if (AT_PraseLeftQuotLastState(currState, copyPtr, currPtr) == VOS_FALSE) {
                return VOS_FALSE;
            }
            break;
#endif

        case AT_E_CMD_NAME_STATE: /* AT扩展命令查询参数状态 */

            /* 存储命令名到全局变量中 */
            g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)(currPtr - copyPtr);
            if (g_atParseCmd.cmdName.cmdNameLen < sizeof(g_atParseCmd.cmdName.cmdName)) {
                atRangeCopy(g_atParseCmd.cmdName.cmdName, copyPtr, currPtr);
            } else {
                g_atParseCmd.cmdName.cmdNameLen = 0;
                return VOS_FALSE; /* 返回错误 */
            }

            g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA; /* 设置命令操作类型 */

            break;

        case AT_E_CMD_COLON_STATE: /* AT命令后面跟有无参的逗号 */

            if (atfwParseSaveParam(copyPtr, 0) != ERR_MSP_SUCCESS) {
                return VOS_FALSE;
            }

            break;

        case AT_E_CMD_SET_STATE:  /* AT扩展命令等号状态 */
        case AT_E_CMD_TEST_STATE: /* AT扩展命令查询参数状态 */
        case AT_E_CMD_READ_STATE: /* AT扩展命令测试参数状态 */
            break;

        default:
            return VOS_FALSE; /* 返回错误 */
    }

    return VOS_TRUE;
}
/*
 * 功能描述: 在name state解析扩展命令
 */
LOCAL VOS_VOID AT_ParseExtendCmdNameState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr,
    VOS_UINT8 *currPtr)
{
    if (currState != newState) {                              /* 新状态部分不等于当前状态 */
        g_atParseCmd.cmdFmtType = AT_EXTEND_CMD_TYPE;         /* 设置命令类型 */
        g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA; /* 设置命令操作类型 */

        *copyPtr = currPtr; /* 准备接收扩展类型命令名 */
    }
}
/*
 * 功能描述: 在set state解析扩展命令
 */
LOCAL VOS_UINT32 AT_ParseExtendCmdSetState(VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    /* 存储命令名到全局变量中 */
    g_atParseCmd.cmdOptType         = AT_CMD_OPT_SET_PARA_CMD; /* 设置命令操作类型 */
    g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)(currPtr - copyPtr);
    if (g_atParseCmd.cmdName.cmdNameLen < sizeof(g_atParseCmd.cmdName.cmdName)) {
        atRangeCopy(g_atParseCmd.cmdName.cmdName, copyPtr, currPtr);
    } else {
        g_atParseCmd.cmdName.cmdNameLen = 0;
        return AT_ERROR; /* 返回错误 */
    }

    return AT_OK;
}
/*
 * 功能描述: 在para state解析扩展命令
 */
LOCAL VOS_VOID AT_ParseExtendCmdParaState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr,
    VOS_UINT8 *currPtr)
{
    if (currState != newState) { /* 新状态部分不等于当前状态 */
        *copyPtr = currPtr;       /* 准备接收参数内容 */
    }
}
/*
 * 功能描述: 在colon state解析扩展命令
 */
LOCAL VOS_UINT32 AT_ParseExtendCmdColonState(AT_StateType currState, AT_StateType newState, VOS_UINT8 *copyPtr,
    VOS_UINT8 *currPtr)
{
    if ((currState != newState) && (currState != AT_E_CMD_SET_STATE)) { /* 新状态部分不等于当前状态 */
        /* 存储参数到全局变量中 */
        if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
            return AT_ERROR;
        }

    } else {
        /* 连续两个逗号,参数不存在 */
        if (atfwParseSaveParam(copyPtr, 0) != ERR_MSP_SUCCESS) {
            return AT_ERROR;
        }
    }

    return AT_OK;
}
/*
 * 功能描述: 在read state解析扩展命令
 */
LOCAL VOS_UINT32 AT_ParseExtendCmdReadState(VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    /* 存储命令名到全局变量中 */
    g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)(currPtr - copyPtr);
    if (g_atParseCmd.cmdName.cmdNameLen < sizeof(g_atParseCmd.cmdName.cmdName)) {
        atRangeCopy(g_atParseCmd.cmdName.cmdName, copyPtr, currPtr);
    } else {
        g_atParseCmd.cmdName.cmdNameLen = 0;
        return AT_ERROR; /* 返回错误 */
    }

    g_atParseCmd.cmdOptType = AT_CMD_OPT_READ_CMD; /* 设置命令操作类型 */

    return AT_OK;
}

VOS_UINT32 AT_ParseExtendCmd(VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_StateType currState = AT_NONE_STATE; /*  设置初始状态 */
    AT_StateType newState  = AT_NONE_STATE; /*  设置初始状态 */
    VOS_UINT8   *currPtr   = data;          /*  指向当前正在处理的字符 */
    VOS_UINT8   *copyPtr   = data;          /*  拷贝内容的起始指针 */
    VOS_UINT16   length    = 0;             /*  记录当前已经处理的字符个数 */

    /* 依次分析字符串中的每个字符 */
    while (length++ < len) {
        /* 依次比较每个字符 */
        currState = newState; /* 当前状态设置为新状态 */

        /*  根据当前处理的字符和当前状态查表得到新状态 */
        newState = atFindNextMainState(g_atECmdMainStateTab, *currPtr, currState);

        switch (newState) {
            /* 状态处理 */
            case AT_E_CMD_NAME_STATE:                                     /* 扩展命令名状态 */
                AT_ParseExtendCmdNameState(currState, newState, &copyPtr, currPtr);
                break;

            case AT_E_CMD_SET_STATE: /* AT扩展命令等号状态 */
                if (AT_ParseExtendCmdSetState(copyPtr, currPtr) != AT_OK) {
                    return AT_ERROR;
                }
                break;

            case AT_E_CMD_PARA_STATE:        /* AT扩展命令参数状态 */
                AT_ParseExtendCmdParaState(currState, newState, &copyPtr, currPtr);
                break;

            case AT_E_CMD_LEFT_QUOT_STATE:   /* AT扩展命令左引号状态 */
                AT_PraseLeftQuotState(currState, newState, &copyPtr, currPtr);
                break;

            case AT_E_CMD_COLON_STATE:                                              /* AT扩展命令逗号状态 */
                if (AT_ParseExtendCmdColonState(currState, newState, copyPtr, currPtr) != AT_OK) {
                    return AT_ERROR;
                }
                break;

            case AT_E_CMD_READ_STATE: /* AT扩展命令查询参数状态 */
                if (AT_ParseExtendCmdReadState(copyPtr, currPtr) != AT_OK) {
                    return AT_ERROR;
                }

                break;

            case AT_E_CMD_TEST_STATE:                          /* AT扩展命令测试参数状态 */
                g_atParseCmd.cmdOptType = AT_CMD_OPT_TEST_CMD; /* 设置命令操作类型 */
                break;

            case AT_BUTT_STATE:  /* 无效状态 */
                return AT_ERROR; /* 返回错误 */

            default:
                break;
        }
        currPtr++; /* 继续分析下一个字符 */
    }

    if (AT_PraseLastState(currState, newState, copyPtr, currPtr) == VOS_FALSE) {
        return AT_ERROR;
    }

    return AT_SUCCESS; /*  返回正确 */
}

VOS_UINT32 AT_ParseSetDockCmd(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_INT8            ret;
    VOS_UINT16          cmdlen;
    VOS_UINT32          pos;
    errno_t             memResult;
    VOS_UINT8          *dataPara = VOS_NULL_PTR;
    AT_ParseCmdNameType atCmdName;

    (VOS_VOID)memset_s(&atCmdName, sizeof(atCmdName), 0x00, sizeof(atCmdName));

    if (len == 0) {
        return AT_FAILURE;
    }

    dataPara = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, len);

    if (dataPara == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_ParseSetDockCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(dataPara, len, 0x00, len);

    memResult = memcpy_s(dataPara, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    /* 去掉字符串中的退格符 */
    if (At_ScanDelChar(dataPara, &len) == AT_FAILURE) {
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        return AT_FAILURE;
    }

    /* 待处理的字符串长度小于"AT^DOCK="长度直接返回AT_FAILURE */
    cmdlen = (VOS_UINT16)VOS_StrLen("AT^DOCK=");
    if (len < cmdlen) {
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        return AT_FAILURE;
    }

    /* AT命令头字符转大写 */
    At_UpString(dataPara, cmdlen);

    /* 待处理的字符串头部不是"AT^DOCK="直接返回AT_FAILURE */
    ret = VOS_StrNiCmp((VOS_CHAR *)dataPara, "AT^DOCK=", cmdlen);
    if (ret != 0) {
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        return AT_FAILURE;
    }

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos                  = VOS_StrLen("AT");
    atCmdName.cmdNameLen = (VOS_UINT16)VOS_StrLen("^DOCK");
    memResult = memcpy_s(atCmdName.cmdName, sizeof(atCmdName.cmdName), (dataPara + pos), atCmdName.cmdNameLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmdName.cmdName), atCmdName.cmdNameLen);
    atCmdName.cmdName[atCmdName.cmdNameLen] = '\0';
    pos += atCmdName.cmdNameLen;

    pos += VOS_StrLen("=");

    /* 获取命令的参数字符串及长度 */
    g_atParaList[0].paraLen = len - (VOS_UINT16)pos;
    if (g_atParaList[0].paraLen > 0) {
        memResult = memcpy_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), (dataPara + pos),
                             g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParaList[0].para), g_atParaList[0].paraLen);
    }

    /* 设置命令类型，操作类型和参数个数 */
    g_atParaIndex           = 1;
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    g_atCmdFmtType          = AT_EXTEND_CMD_TYPE;

    PS_MEM_FREE(WUEPS_PID_AT, dataPara);
    return AT_SUCCESS;
}

