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

#include "at_parse_dm_cmd.h"
#include "at_check_func.h"

#include "msp_errno.h"


/* AT命令初始状态表 */
static const AT_SubState g_atDmCmdNoneStateTab[] = {
    /* 如果当前状态是ATCMD_NONE_STATE，atCheckCharD成功，则进入AT_DM_CMD_NAME_STATE */
    { At_CheckCharD, AT_DM_CMD_NAME_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT DM命令命令名状态表 */
static const AT_SubState g_atDmCmdNameStateTab[] = {
    /* 如果当前状态是AT_DM_CMD_NAME_STATE，At_CheckCharStar成功，则进入AT_DM_CMD_STAR_STATE */
    { At_CheckCharStar, AT_DM_CMD_STAR_STATE },

    /* 如果当前状态是AT_D_CMD_NAME_STATE，At_CheckCharWell成功，则进入AT_DM_CMD_WELL_STATE */
    { At_CheckCharWell, AT_DM_CMD_WELL_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT DM命令字符状态表 */
static const AT_SubState g_atDmCmdCharStateTab[] = {
    /* 如果当前状态是AT_DM_CMD_NAME_STATE，At_CheckCharStar成功，则进入AT_DM_CMD_STAR_STATE */
    { At_CheckCharStar, AT_DM_CMD_STAR_STATE },

    /* 如果当前状态是AT_DM_CMD_NAME_STATE，At_CheckCharStar成功，则进入AT_DM_CMD_WELL_STATE */
    { At_CheckCharWell, AT_DM_CMD_WELL_STATE },

    /* 如果当前状态是AT_DM_CMD_NAME_STATE，At_CheckCharStar成功，则进入AT_DM_CMD_NUM_STATE */
    { At_CheckDmChar, AT_DM_CMD_NUM_STATE },

    /* 子状态表结束 */
    { NULL, AT_BUTT_STATE },
};

/* AT命令主状态表 */
static const AT_MainState g_atDmCmdMainStateTab[] = {
    /* 如果当前状态是AT_NONE_STATE，则进入g_atDmCmdNoneStateTab子状态表 */
    { AT_NONE_STATE, g_atDmCmdNoneStateTab },

    /* 如果当前状态是AT_DM_CMD_NAME_STATE，则进入g_atDmCmdNameStateTab子状态表 */
    { AT_DM_CMD_NAME_STATE, g_atDmCmdNameStateTab },

    /* 如果当前状态是AT_DM_CMD_NUM_STATE，则进入g_atDmCmdCharStateTab子状态表 */
    { AT_DM_CMD_NUM_STATE, g_atDmCmdCharStateTab },

    /* 如果当前状态是AT_DM_CMD_STAR_STATE，则进入g_atDmCmdCharStateTab子状态表 */
    { AT_DM_CMD_STAR_STATE, g_atDmCmdCharStateTab },

    /* 如果当前状态是AT_DM_CMD_WELL_STATE，则进入g_atDmCmdCharStateTab子状态表 */
    { AT_DM_CMD_WELL_STATE, g_atDmCmdCharStateTab },

    /* 主状态表结束 */
    { AT_BUTT_STATE, NULL },
};
VOS_UINT32 AT_ProcDmCmdNumState(AT_StateType currState, AT_StateType newState, VOS_UINT8 **copyPtr, VOS_UINT8 *currPtr)
{
    if (currState != newState) { /* 新状态部分不等于当前状态 */
        /* 存储参数到全局变量中 */
        if (atfwParseSaveParam(*copyPtr, (VOS_UINT16)(currPtr - *copyPtr)) != ERR_MSP_SUCCESS) {
            return AT_FAILURE;
        }

        *copyPtr = currPtr; /* 准备接收下一个参数 */
    }

    return AT_SUCCESS;
}
VOS_UINT32 AT_ProcDmCmdWellStarState(AT_StateType currState, VOS_UINT8 *copyPtr, VOS_UINT8 *currPtr)
{
    if (currState == AT_DM_CMD_NAME_STATE) {
        g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)(currPtr - copyPtr);
        if (g_atParseCmd.cmdName.cmdNameLen < sizeof(g_atParseCmd.cmdName.cmdName)) {
            atRangeCopy(g_atParseCmd.cmdName.cmdName, copyPtr, currPtr);
        } else {
            g_atParseCmd.cmdName.cmdNameLen = 0;
            return AT_FAILURE; /* 返回错误 */
        }
    } else {
        /* 存储参数到全局变量中 */
        if (atfwParseSaveParam(copyPtr, (VOS_UINT16)(currPtr - copyPtr)) != ERR_MSP_SUCCESS) {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ParseDMCmd(VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_StateType currState = AT_NONE_STATE; /*  设置初始状态 */
    AT_StateType newState  = AT_NONE_STATE; /*  设置初始状态 */
    VOS_UINT8   *currPtr   = data;          /*  指向当前正在处理的字符 */
    VOS_UINT8   *copyPtr   = data;          /*  拷贝内容的起始指针 */
    VOS_UINT16   length    = 0;             /*  记录当前已经处理的字符个数 */

    /* 依次分析字符串中的每个字符 */
    while ((length++ < len) && (g_atParseCmd.paraIndex < AT_MAX_PARA_NUMBER)) {
        /* 依次比较每个字符 */
        currState = newState; /* 当前状态设置为新状态 */

        /* 根据当前处理的字符和当前状态查表得到新状态 */
        newState = atFindNextMainState(g_atDmCmdMainStateTab, *currPtr, currState);

        switch (newState) {
            /* 状态处理 */
            case AT_DM_CMD_NAME_STATE:                                /* AT DM命令命令名状态 */
                g_atParseCmd.cmdFmtType = AT_BASIC_CMD_TYPE;          /* 设置命令类型 */
                g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA; /* 设置命令操作类型 */
                copyPtr                 = currPtr;                    /* 准备接收DM类型命令名 */
                break;

            /* 有可能数字长度过界，或者g_atParaIndex过界 */
            case AT_DM_CMD_NUM_STATE:        /* AT DM命令字符状态 */
                if (AT_ProcDmCmdNumState(currState, newState, &copyPtr, currPtr) == AT_FAILURE) {
                    return AT_FAILURE;
                }
                break;

            case AT_DM_CMD_WELL_STATE: /* AT DM命令字符#状态 */
            case AT_DM_CMD_STAR_STATE: /* AT DM命令字符*状态 */
                if (AT_ProcDmCmdWellStarState(currState, copyPtr, currPtr) == AT_FAILURE) {
                    return AT_FAILURE;
                }
                copyPtr = currPtr; /* 准备接收下一个参数 */
                break;

            default:
                return AT_FAILURE; /* 返回错误 */
        }

        currPtr++; /* 继续分析下一个字符 */
    }

    switch (newState) {
        /* 根据最后的状态判断 */
        case AT_DM_CMD_WELL_STATE: /* AT DM命令字符#状态 */
            break;

        default:
            return AT_FAILURE; /* 返回错误 */
    }

    return AT_SUCCESS; /* 返回正确 */
}

