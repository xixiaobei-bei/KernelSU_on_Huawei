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
#include "at_parse_core.h"
#include "securec.h"
#include "at_check_func.h"
#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_input_proc.h"
#include "at_sms_taf_rslt_proc.h"
#include "at_common.h"

#include "acpu_reset.h"

#include "at_internal_msg.h"
#if (FEATURE_AT_PROXY == FEATURE_ON)
#include "at_atp_proc.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_AT_PARSECORE_C
#define AT_AUC_TMP_CMD_ARRAY_LEN 9
#define AT_LF_LEN 1
#define AT_ZZ_LEN 2
#define AT_PEND_DATA_ARRAY_LEN 4
#define AT_CMD_NAME_MAX_NUM 3
#define AT_CMD_NAME_MAX_LEN 16
#define AT_CMD_NAME_COLUMNS_INDEX 2
#define AT_A0X2F_AND_CR_LEN 3
#define AT_PARSE_AT_LEN 2
#define AT_PARSE_AT_SEMICOLON_LEN 3
#define AT_CMD_AT_LEN 2 /* 字符"AT"长度 */

/* 命令表头结点 */
HI_LIST_S g_cmdTblList = {0};

/* 解析过程参数 */
AT_PARSECMD g_atParseCmd;

/* 参数列表 */
VOS_UINT8 g_atParaIndex = 0;

AT_ParseParaType g_atParaList[AT_MAX_PARA_NUMBER];

/* 每个客户端的解析信息 */
AT_ParseContext g_parseContext[AT_MAX_CLIENT_NUM];

/* 由于SMS命令与普通命令格式不一致，用来缓存SMS命令的相应的内容 */
/*
 * 解决短信命令支持多通道并发命令名被覆盖问题
 * 由g_cmdElement=> g_cmdElement[AT_MAX_CLIENT_NUM]
 */
AT_ParCmdElement g_cmdElement[AT_MAX_CLIENT_NUM];
VOS_UINT8 AT_GetParaIndex(VOS_VOID)
{
    return g_atParaIndex;
}
AT_ParseParaType *AT_GetParaList(VOS_UINT8 indexNum)
{
    return &(g_atParaList[indexNum]);
}
AT_ParseContext *AT_GetParseContext(AT_ClientIdUint16 indexNum)
{
    return &(g_parseContext[indexNum]);
}

VOS_VOID At_ParseInit(VOS_VOID)
{
    VOS_UINT32 i = 0;

    (VOS_VOID)memset_s(&g_atParseCmd, sizeof(g_atParseCmd), 0x00, sizeof(g_atParseCmd));
    (VOS_VOID)memset_s(g_parseContext, sizeof(g_parseContext), 0x00, sizeof(g_parseContext));
    (VOS_VOID)memset_s(g_cmdElement, sizeof(g_cmdElement), 0x00, sizeof(g_cmdElement));

    /*lint -e717*/

    HI_INIT_LIST_HEAD(&g_cmdTblList);

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        HI_INIT_LIST_HEAD(&(g_parseContext[i].combineCmdInfo.combineCmdList));
    }

    /*lint -e717*/
}
/*lint -save -e429*/

VOS_UINT32 AT_RegisterCmdTable(const AT_ParCmdElement *cmdTblEntry, VOS_UINT16 cmdNum)
{
    HI_LIST_S        *cmdTblHeader = NULL;
    AT_ParCmdtblList *cmdTblNode   = NULL; /*lint !e830*/

    /* 命令表不为空，且命令个数不为0 */
    if ((cmdTblEntry == NULL) || (cmdNum == 0)) {
        return ERR_MSP_FAILURE;
    }

    cmdTblHeader = &g_cmdTblList;

    /* 把新请求的errcodetbl添加到链表中 */
    cmdTblNode = (AT_ParCmdtblList *)AT_MALLOC(sizeof(AT_ParCmdtblList));
    if (cmdTblNode == NULL) {
        return ERR_MSP_MALLOC_FAILUE;
    }

    (VOS_VOID)memset_s(cmdTblNode, sizeof(AT_ParCmdtblList), 0x00, sizeof(AT_ParCmdtblList));
    cmdTblNode->cmdNum = cmdNum;

    cmdTblNode->cmdElement = (AT_ParCmdElement *)cmdTblEntry;

    msp_list_add_tail((&cmdTblNode->cmdTblList), cmdTblHeader);

    return ERR_MSP_SUCCESS;
}
/*lint -restore */

VOS_UINT8 AT_BlockCmdCheck(VOS_VOID)
{
    VOS_UINT8 i       = 0;
    VOS_UINT8 blockid = AT_MAX_CLIENT_NUM;

    /* 查询所有通道状态 */
    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        /* 如果某通道处于非ready态，则直接返回，不处理 */
        if (g_parseContext[i].clientStatus != AT_FW_CLIENT_STATUS_READY) {
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
            /* 如果下发AT的是APP管道，且下发的AT中，带有正在等待的魔术字放行,这个函数会在上一个AT处理完成时被调用 */
            if((g_atClientTab[AT_CLIENT_ID_APP].isWaitAts == 1) &&
                (g_parseContext[AT_CLIENT_ID_APP].blockCmdLen != 0) &&
                (g_parseContext[AT_CLIENT_ID_APP].blockCmd != NULL_PTR)) {
                return AT_CLIENT_ID_APP;
            }
#endif
            return AT_MAX_CLIENT_NUM;
        }

        if ((g_parseContext[i].blockCmdLen != 0) && (g_parseContext[i].blockCmd != NULL)) {
            blockid = i;
        }
    }

    return blockid;
}

VOS_VOID AT_ClearBlockCmdInfo(VOS_UINT8 indexNum)
{
    if (g_parseContext[indexNum].blockCmd != NULL) {
        AT_FREE(g_parseContext[indexNum].blockCmd);
        g_parseContext[indexNum].blockCmd = VOS_NULL_PTR;
        g_parseContext[indexNum].blockCmd = NULL;
    }

    g_parseContext[indexNum].blockCmdLen = 0;
}

VOS_VOID AT_CheckProcBlockCmd(VOS_VOID)
{
    VOS_UINT8  indexNum;
    VOS_UINT32 ret;

    /* 判断是否有缓存的命令 */
    indexNum = AT_BlockCmdCheck();
    if (indexNum < AT_MAX_CLIENT_NUM) {
        /* 发送消息处理缓存命令 */
        ret = At_SendCmdMsg(indexNum, &indexNum, sizeof(indexNum), AT_COMBIN_BLOCK_MSG);
        if (ret != ERR_MSP_SUCCESS) {
            /* 由定时器超时去处理 */
        }
    }
}

VOS_VOID AT_ParseCmdOver(VOS_UINT8 indexNum)
{
    VOS_UINT32            ret            = ERR_MSP_FAILURE;
    AT_ParseContext      *clientContext  = NULL;
    AT_FW_CombineCmdInfo *combineCmdInfo = NULL;

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        return;
    }

    clientContext = &(g_parseContext[indexNum]);

    combineCmdInfo = &clientContext->combineCmdInfo;

    if (combineCmdInfo->procNum < combineCmdInfo->totalNum) {
        combineCmdInfo->procNum++;
    } else { /* 避免被误调用 */
        combineCmdInfo->procNum = combineCmdInfo->totalNum;
    }

    /* 判断组合命令是否处理完毕 */
    if ((combineCmdInfo->procNum < combineCmdInfo->totalNum)) {
        /* 发送消息处理下一条命令 */
        ret = At_SendCmdMsg(indexNum, &indexNum, sizeof(indexNum), AT_COMBIN_BLOCK_MSG);
        if (ret != ERR_MSP_SUCCESS) {
            /* 消息发送失败恢复解析器状态 */
            clientContext->clientStatus = AT_FW_CLIENT_STATUS_READY;
        }

        return;
    }
}

VOS_VOID AT_BlockCmdTimeOutProc(REL_TimerMsgBlock *msg)
{
    VOS_UINT8 indexNum;

    indexNum = (VOS_UINT8)((msg->name) >> 12);

    g_atSendDataBuff.bufLen = 0;

    At_FormatResultData(indexNum, AT_ERROR);

    AT_ClearBlockCmdInfo(indexNum);
}

/*
 * 功能描述: 判断当前字符串是否为STOP
 */
LOCAL VOS_UINT32 AT_IsStopCmdString(VOS_UINT8 *data, VOS_UINT16 len)
{
    if ((len == AT_PEND_DATA_ARRAY_LEN) && (At_UpChar(data[0]) == 'S') && (At_UpChar(data[1]) == 'T') &&
        (At_UpChar(data[2]) == 'O') && (At_UpChar(data[3]) == 'P')) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_PendClientProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    if (AT_IsStopCmdString(data, len) == VOS_TRUE) {
        AT_StopTimerCmdReady(indexNum);
        At_ResetCombinParseInfo(indexNum);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_A_SET) {
        if ((At_UpChar(data[0]) != 'A') || (At_UpChar(data[1]) != 'T') || (At_UpChar(data[2]) != 'H')) {
        } else {
            AT_StopTimerCmdReady(indexNum);
            At_ResetCombinParseInfo(indexNum);
        }
    } else if (AT_IsAbortCmdStr(indexNum, data, len) == VOS_TRUE) {
        /* AT打断当前PEND状态的处理 */
        AT_AbortCmdProc(indexNum);
    }
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    else if (g_atClientTab[indexNum].canAbort == VOS_TRUE) {
        /* g_AppAtList中有打断函数的AT都是可打断的，此时下发任意字符都会打断此AT
           * AT打断当前PEND状态的处理,会调用当前AT的打断函数
           */
        if ((sizeof(char) == len) && (data[0] == g_atS4)) {
            /* \r下来后，AT就会开始解析，如果这时候再下来个\n，就变成打断字符
                * 这种情况下，用户的本意不是中断，而是上次下发的AT还没有发完
                */
        } else {
            AT_AbortCmdProc(indexNum);
        }
    }
#endif
    else {
    }
}

VOS_VOID AT_HoldBlockCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    errno_t          memResult;
    VOS_UINT32       timerName;
    VOS_UINT32       tempIndex;
    AT_ParseContext *clientContext = NULL;

    clientContext = &(g_parseContext[indexNum]);

    /* 只支持缓存一条数据，当有新的AT命令字符流则覆盖本通道之前缓存的命令信息 */
    if (clientContext->blockCmd != NULL) {
        AT_FREE(clientContext->blockCmd);
        clientContext->blockCmd = VOS_NULL_PTR;
    }

    if (len == 0) {
       return;
    }

    clientContext->blockCmd = (VOS_UINT8 *)AT_MALLOC(len);
    if (clientContext->blockCmd == NULL) {
        clientContext->blockCmdLen = 0;
        return;
    }

    (VOS_VOID)memset_s(clientContext->blockCmd, len, 0x00, len);

    if (len > 0) {
        memResult = memcpy_s(clientContext->blockCmd, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }
    clientContext->blockCmdLen = len;

    tempIndex = (VOS_UINT32)indexNum;
    timerName = AT_HOLD_CMD_TIMER;
    timerName |= AT_INTERNAL_PROCESS_TYPE;
    timerName |= (tempIndex << 12);

    /* 重启定时器 */
    VOS_StopRelTimer(&clientContext->timer);

    AT_StartRelTimer(&clientContext->timer, AT_HOLD_CMD_TIMER_LEN, timerName, 0, VOS_RELTIMER_NOLOOP);
}

VOS_UINT32 AT_ParseCmdIsComb(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_ParseContext *clientContext = NULL;

    clientContext = &(g_parseContext[indexNum]);

    /* 表示本通道有组合命令正在处理 */
    if (clientContext->combineCmdInfo.procNum < clientContext->combineCmdInfo.totalNum) {
        CmdStringFormat(indexNum, data, &len);

        AT_PendClientProc(indexNum, data, len);

        return ERR_MSP_FAILURE;
    }

    return ERR_MSP_SUCCESS;
}

VOS_UINT32 AT_ParseCmdIsPend(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32       i             = 0;
    AT_ParseContext *clientContext = NULL;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        clientContext = &(g_parseContext[i]);

        if (i == indexNum) {
            /* 本通道处于PEND状态时需要判断是否是STOP或ATH */
            if (clientContext->clientStatus == AT_FW_CLIENT_STATUS_PEND) {
                CmdStringFormat(indexNum, data, &len);

                AT_PendClientProc(indexNum, data, len);
                return ERR_MSP_FAILURE;
            }
        }
        /* 其他通道处于PEND状态或短信状态 */
        else if ((clientContext->clientStatus == AT_FW_CLIENT_STATUS_PEND) || (clientContext->mode == AT_SMS_MODE)) {
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
             /* 用户态命令不阻塞端口，直接放行 */
             if ((indexNum == AT_CLIENT_ID_APP) && (g_atClientTab[AT_CLIENT_ID_APP].isWaitAts == 1)) {
                 continue;
             }
#endif

            /* 判断本通道以及遍历到的通道是否可以并发 */
            if (AT_IsConcurrentPorts(indexNum, (VOS_UINT8)i) == VOS_TRUE) {
                /* 如果是，则继续遍历通道 */
                continue;
            } else {
                /* 如果不是，则缓存该命令 */
                AT_HoldBlockCmd(indexNum, data, len);
                return ERR_MSP_FAILURE;
            }
        } else {
        }
    }

    return ERR_MSP_SUCCESS;
}

VOS_VOID AT_DiscardInvalidCharForSms(TAF_UINT8 *data, TAF_UINT16 *len)
{
    const TAF_CHAR cMGSCmd[] = "AT+CMGS=";
    const TAF_CHAR cMGWCmd[] = "AT+CMGW=";
    const TAF_CHAR cMGCCmd[] = "AT+CMGC=";
    TAF_CHAR       tmpCmd[AT_AUC_TMP_CMD_ARRAY_LEN];
    VOS_INT        iCMGSResult;
    VOS_INT        iCMGWResult;
    VOS_INT        iCMGCResult;
    errno_t  memResult;

    if (*len < VOS_StrLen(cMGSCmd)) {
        return;
    }

    memResult = memcpy_s(tmpCmd, sizeof(tmpCmd), data, sizeof(tmpCmd) - 1);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tmpCmd), sizeof(tmpCmd) - 1);
    At_UpString((TAF_UINT8 *)tmpCmd, sizeof(tmpCmd) - 1);
    tmpCmd[AT_AUC_TMP_CMD_ARRAY_LEN-1] = '\0';

    iCMGSResult = VOS_StrCmp(cMGSCmd, tmpCmd);
    iCMGWResult = VOS_StrCmp(cMGWCmd, tmpCmd);
    iCMGCResult = VOS_StrCmp(cMGCCmd, tmpCmd);
    if ((iCMGSResult != 0) && (iCMGWResult != 0) && (iCMGCResult != 0)) {
        return;
    }

    /*
     * MAC系统上的MP后台问题:AT+CMGS=**<CR><^z><Z>(或AT+CMGW=**<CR><^z><Z>)
     * 为了规避该问题，需要在接收到如上形式的码流后，
     * 需要将命令后的无效字符<^z><Z>删去
     */
    if ((g_atS3 == data[*len - 3]) && (data[*len - 2] == '\x1a') &&
        ((data[*len - 1] == 'z') || (data[*len - 1] == 'Z'))) {
        /* 删去结尾的<^z><Z>字符 */
        *len -= AT_ZZ_LEN;
    }
    /* MAC系统SFR后台问题: AT+CMGS=<LENGTH><CR><LF><CR> */
    else if ((g_atS3 == data[*len - 3]) && (g_atS4 == data[*len - 2]) && (g_atS3 == data[*len - 1])) {
        /* 删去结尾的<LF><CR>字符 */
        *len -= AT_CRLF_STR_LEN;
    }
    /* 判断pData码流的结尾是否为<CR><LF>形式 */
    else if ((g_atS3 == data[*len - 2]) && (g_atS4 == data[*len - 1])) {
        /* 删去结尾的<LF>字符 */
        *len -= AT_LF_LEN;
    } else {
    }
}

VOS_VOID AT_ResetParseVariable(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_atParseCmd, sizeof(g_atParseCmd), 0x00, sizeof(AT_PARSECMD));

    (VOS_VOID)memset_s(g_atParaList, sizeof(g_atParaList), 0x00, sizeof(g_atParaList));

    g_atParseCmd.cmdOptType = AT_CMD_OPT_BUTT;
    g_atParseCmd.cmdFmtType = AT_CMD_TYPE_BUTT;

    g_atParaIndex = 0;
}

VOS_UINT32 AT_ParseCmdType(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16 dataTmpLen = len - AT_PARSE_AT_LEN;
    VOS_UINT8 *dataTmp    = data + AT_PARSE_AT_LEN;
    VOS_UINT32 result     = 0;

    /* 调用的地方保证pData不为空 */

    if (len < AT_PARSE_AT_LEN) {
        return AT_ERROR;
    }

    if (At_CheckCharA(*data) == AT_SUCCESS) {           /* 如果是'A'/'a' */
        if (At_CheckCharT(*(data + 1)) == AT_SUCCESS) { /* 如果是'T'/'t' */
            if (len == AT_PARSE_AT_LEN) {
                return AT_OK;
            }

            if (len == AT_PARSE_AT_SEMICOLON_LEN) {
                if (At_CheckSemicolon(data[AT_PARSE_AT_SEMICOLON_LEN-1]) == AT_SUCCESS) {
                    return AT_OK;
                }
            }

            AT_ResetParseVariable(); /* 重置解释器所用的全局变量 */

            dataTmp    = data + AT_PARSE_AT_LEN;
            dataTmpLen = len - AT_PARSE_AT_LEN;

            /* 非D命令，需要去除命令结尾的';' */
            if ((At_CheckCharD(*dataTmp)) != AT_SUCCESS && (*(dataTmp + dataTmpLen - 1) == ';')) {
                dataTmpLen--;
            }

            switch (*dataTmp) {
                /* 根据第三个字符进行分发 */
                case '^': /* 扩展命令 */
                case '+': /* 扩展命令 */
                case '$': /* 扩展命令 */
                    return AT_ParseExtendCmd(dataTmp, dataTmpLen);

                case 'd':
                case 'D': /* D命令 */

                    return ParseDCmdPreProc(dataTmp, dataTmpLen);

                case 's':
                case 'S': /* S命令 */
                    return AT_ParseSCmd(dataTmp, dataTmpLen);

                default: /* 基本命令 */
                {
                    result = AT_ParseBasicCmd(dataTmp, dataTmpLen);

                    if (result == AT_FAILURE) {
                        result = AT_ERROR;
                    }

                    return result;
                }
            }
        }
    }

    return AT_ERROR;
}

VOS_UINT32 At_MatchSmsCmdName(VOS_UINT8 indexNum, VOS_CHAR *pszCmdName)
{
    VOS_UINT32      i      = 0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    for (i = 0; i < g_atSmsCmdNum; i++) {
        if ((TAF_CHAR *)g_atSmsCmdTab[i].cmdName != VOS_NULL_PTR) {
            if (AT_STRCMP(pszCmdName, (TAF_CHAR *)g_atSmsCmdTab[i].cmdName) == ERR_MSP_SUCCESS) {
                if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) {
                    g_cmdElement[indexNum].param = g_atSmsCmdTab[i].paraText;
                } else {
                    g_cmdElement[indexNum].param = g_atSmsCmdTab[i].paraPDU;
                }

                g_cmdElement[indexNum].cmdIndex    = g_atSmsCmdTab[i].cmdIndex;
                g_cmdElement[indexNum].setProc     = g_atSmsCmdTab[i].setProc;
                g_cmdElement[indexNum].setTimeOut  = g_atSmsCmdTab[i].setTimeOut;
                g_cmdElement[indexNum].qryProc     = NULL;
                g_cmdElement[indexNum].qryTimeOut  = 0;
                g_cmdElement[indexNum].testProc    = g_atSmsCmdTab[i].testProc;
                g_cmdElement[indexNum].testTimeOut = g_atSmsCmdTab[i].testTimeOut;

                g_cmdElement[indexNum].abortProc    = g_atSmsCmdTab[i].abortProc;
                g_cmdElement[indexNum].abortTimeOut = g_atSmsCmdTab[i].abortTimeOut;

                g_cmdElement[indexNum].paramErrCode = g_atSmsCmdTab[i].paramErrCode;
                g_cmdElement[indexNum].chkFlag      = g_atSmsCmdTab[i].chkFlag;
                g_cmdElement[indexNum].cmdName      = g_atSmsCmdTab[i].cmdName;

                g_parseContext[indexNum].cmdElement = &g_cmdElement[indexNum];

                return AT_SUCCESS;
            }
        }
    }

    g_parseContext[indexNum].cmdElement = NULL;

    return AT_FAILURE;
}

#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
VOS_UINT32 AT_MatchUserCustomCmdName(VOS_UINT8 indexNum, VOS_CHAR *cmdName)
{
    AT_ParCmdElement *appAtList = VOS_NULL_PTR;
    VOS_UINT32        appAtCount;
    VOS_UINT32        i;

    appAtList = AT_GetAppAtListTbl();
    appAtCount = AT_GetAppAtCount();
    if (appAtList == NULL) {
        AT_ERR_LOG("AT_MatchUserCustomCmdName g_AppAtList is NULL");
        return AT_CMD_NOT_SUPPORT;
    }
    for (i = 0; i < appAtCount; i++) {
        if (appAtList[i].cmdName == NULL) {
            continue;
        }

        if (AT_STRCMP(cmdName, (VOS_CHAR *)appAtList[i].cmdName) == ERR_MSP_SUCCESS) {
            g_parseContext[indexNum].cmdElement = &(appAtList[i]);

            return ERR_MSP_SUCCESS;
        }
    }

    AT_ERR_LOG("AT_MatchUserCustomCmdName return cmd not support");
    return AT_CMD_NOT_SUPPORT;
}
#endif

VOS_UINT32 AT_MatchTrustListCmdName(VOS_CHAR *cmdName, VOS_UINT16 cmdNameLen)
{
    VOS_UINT8  keyWord[AT_CMD_KEY_WORD_MAX_LEN + 1] = {0};
    VOS_UINT32 keyWordLen;
    VOS_UINT32 ret = ERR_MSP_FAILURE;

    keyWordLen = AT_GetCmdKeyWord(cmdName, cmdNameLen, keyWord, AT_CMD_KEY_WORD_MAX_LEN);
    if (keyWordLen == AT_CMD_KEY_WORD_LEN_ERR) {
        return AT_ERROR;
    }

    /* 设置和查询命令Trust名单检查 */
    ret = AT_IsCmdInTrustList(keyWord, keyWordLen);
    if(ret != AT_SUCCESS) {
        AT_PR_LOGE("Trustlist check fail, cmdName is %s", cmdName);
        return AT_CMD_NOT_SUPPORT;
    }

    return ERR_MSP_SUCCESS;
}
#endif

VOS_UINT32 atMatchCmdName(VOS_UINT8 clientId, VOS_UINT32 cmdType)
{
    VOS_UINT32        i          = 0;
    HI_LIST_S        *me         = NULL;
    HI_LIST_S        *cmdTblList = NULL;
    AT_ParCmdtblList *cmdNode    = NULL;
    VOS_CHAR         *pszCmdName = NULL;

    /* 调用的地方保证输入的指针不为空 */

    cmdTblList = &(g_cmdTblList);

    /* 全部格式化为大写字符 */
    if (At_UpString(g_atParseCmd.cmdName.cmdName, TAF_MIN(g_atParseCmd.cmdName.cmdNameLen, AT_CMD_NAME_LEN))
        == AT_FAILURE) {
        return AT_ERROR;
    }

    pszCmdName = (VOS_CHAR *)g_atParseCmd.cmdName.cmdName;

    if ((cmdType == AT_EXTEND_CMD_TYPE) || (cmdType == AT_BASIC_CMD_TYPE)) {
#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
        if (AT_MatchUserCustomCmdName(clientId, pszCmdName) == ERR_MSP_SUCCESS) {
            return ERR_MSP_SUCCESS;
        }
#endif
        if (AT_IsNeedMatchTrustList() == VOS_TRUE) {
            if (AT_MatchTrustListCmdName(pszCmdName, TAF_MIN(g_atParseCmd.cmdName.cmdNameLen, AT_CMD_NAME_LEN))
                != ERR_MSP_SUCCESS) {
                return AT_CMD_NOT_SUPPORT;
            }
        }
#endif
        /* 短信命令需要特殊处理 */
        if (At_MatchSmsCmdName(clientId, pszCmdName) == ERR_MSP_SUCCESS) {
            return ERR_MSP_SUCCESS;
        }

        msp_list_for_each(me, cmdTblList)
        {
            cmdNode = msp_list_entry(me, AT_ParCmdtblList, cmdTblList);

            for (i = 0; i < cmdNode->cmdNum; i++) {
                if ((cmdNode->cmdElement == NULL) || (cmdNode->cmdElement[i].cmdName == NULL)) {
                    continue;
                }

                if (AT_STRCMP((VOS_CHAR *)pszCmdName, (VOS_CHAR *)cmdNode->cmdElement[i].cmdName) == ERR_MSP_SUCCESS) {
                    g_parseContext[clientId].cmdElement = &(cmdNode->cmdElement[i]);

                    return ERR_MSP_SUCCESS;
                }
            }
        }
    }

    return AT_CMD_NOT_SUPPORT;
}

VOS_UINT32 ParseParam(AT_ParCmdElement *cmdElement)
{
    VOS_UINT32 paraLen = 0;

    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) {
        if ((cmdElement != NULL) && (cmdElement->param != NULL)) {
            paraLen = AT_STRLEN((VOS_CHAR *)cmdElement->param);                                   /* 获取参数脚本长度 */
            if (AT_ParsePara((VOS_UINT8 *)cmdElement->param, (VOS_UINT16)paraLen) != AT_SUCCESS) { /* 参数匹配 */
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    return ERR_MSP_SUCCESS;
}

AT_RreturnCodeUint32 fwCmdTestProc(VOS_UINT8 indexNum, AT_ParCmdElement *cmdElement)
{
    AT_RreturnCodeUint32 result = AT_FAILURE;

    /* 调用的地方保证指针不为空 */

    if (cmdElement->testProc != NULL) {
        result = (AT_RreturnCodeUint32)cmdElement->testProc(indexNum);

        if (result == AT_WAIT_ASYNC_RETURN) {
            g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;

            /* 开定时器 */
            if (At_StartTimer(cmdElement->testTimeOut, indexNum) != AT_SUCCESS) {
                return AT_ERROR;
            }

            At_SetAtCmdAbortTickInfo(indexNum, VOS_GetTick());
        }

        return result;
    } else if (cmdElement->param != NULL) {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %s", cmdElement->cmdName, cmdElement->param);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

AT_RreturnCodeUint32 atCmdDispatch(VOS_UINT8 indexNum)
{
    AT_RreturnCodeUint32 result;
    PFN_AT_FW_CMD_PROC   pfnCmdProc = NULL;
    VOS_UINT32           timerLen   = 0;
    AT_ParCmdElement    *cmdElement = g_parseContext[indexNum].cmdElement;

    switch (g_atParseCmd.cmdOptType) {
        case AT_CMD_OPT_SET_PARA_CMD:
        case AT_CMD_OPT_SET_CMD_NO_PARA:
            pfnCmdProc = cmdElement->setProc; /* 设置无参数命令 */
            timerLen   = cmdElement->setTimeOut;
            break;

        case AT_CMD_OPT_READ_CMD:
            pfnCmdProc = cmdElement->qryProc; /* 查询参数 */
            timerLen   = cmdElement->qryTimeOut;
            break;

        case AT_CMD_OPT_TEST_CMD:
            return fwCmdTestProc(indexNum, cmdElement); /* 测试参数 */

        default:
            return AT_ERROR; /* 其它情况，则返回错误 */
    }

    if (pfnCmdProc == NULL) {
        return AT_ERROR;
    }

    result = (AT_RreturnCodeUint32)pfnCmdProc(indexNum);

    if (result == AT_WAIT_ASYNC_RETURN) {
        /* 开定时器 */
        if (At_StartTimer(timerLen, indexNum) != AT_SUCCESS) {
            return AT_ERROR;
        }

        At_SetAtCmdAbortTickInfo(indexNum, VOS_GetTick());

        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;
    }

    return result;
}

VOS_UINT32 LimitedCmdProc(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 len, AT_ParCmdElement *cmdElement)
{
    VOS_BOOL             bE5CheckRight = VOS_TRUE;
    AT_RreturnCodeUint32 result        = AT_FAILURE;

    if (cmdElement == NULL) {
        return AT_ERROR;
    }

    /* 如果E5密码保护受限，则调用查询E5受限条件接口，受限的话返回AT_ERROR，否则返回AT_OK */
    if ((cmdElement->chkFlag & CMD_TBL_E5_IS_LOCKED) == 0) {
        /* 非AT通道 非WIFI通道, 均不需密码保护 */
        if ((g_atClientTab[clientId].userType != AT_USBCOM_USER) &&
            (g_atClientTab[clientId].userType != AT_SOCK_USER)) {
            /* 继续进行PIN码保护的检查 */
        } else {
            bE5CheckRight = AT_E5CheckRight(clientId, data, len);
            if (bE5CheckRight == VOS_TRUE) {
                /* E5已解锁，继续进行PIN码保护的检查 */
            } else {
                return AT_SUCCESS; /* AT_E5CheckRight 内部已返回上报结果 */
            }
        }
    }

    /* 如果是E5 DOCK命令，则直接调用DOCK命令转发接口，返回AT_SUCCESS */
    if (cmdElement->chkFlag & CMD_TBL_IS_E5_DOCK) {
        if (g_atClientTab[clientId].userType == AT_USBCOM_USER) {
            result = (AT_RreturnCodeUint32)AT_DockHandleCmd(clientId, data, len);
            if (result == AT_SUCCESS) {
                return AT_SUCCESS; /* 命令内容已转发到E5通道，本通道不返回任何结果 */
            } else {
                return AT_ERROR; /* 处理失败，返回ERROR */
            }
        }
    }

    /* 如果PIN码保护受限，则调用查询PIN码受限条件接口，受限的话返回AT_ERROR，否则返回AT_OK */
    if ((cmdElement->chkFlag & CMD_TBL_PIN_IS_LOCKED) == 0) {
        result = (AT_RreturnCodeUint32)At_CheckUsimStatus((VOS_UINT8 *)cmdElement->cmdName, clientId);
        if (result == AT_SUCCESS) {
            /* PIN码已解锁，继续后续的检查处理 */
        } else {
            return result; /* 返回具体锁定的原因值 */
        }
    }

    return AT_OK;
}

VOS_UINT32 CmdParseProc(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_RreturnCodeUint32 result;
    AT_ParCmdElement    *cmdElement = NULL;

    /* 该函数调用处可保证ucClientId的合法性，pDataIn不为空 */

    /* 匹配命令名称 */
    result = (AT_RreturnCodeUint32)atMatchCmdName(clientId, g_atParseCmd.cmdFmtType);
    if (result != ERR_MSP_SUCCESS) {
        return result;
    }

    cmdElement = g_parseContext[clientId].cmdElement;

    if (cmdElement == NULL) {
        return AT_ERROR;
    }

    /* 受限命令判断和处理接口，返回失败表示处于受限条件，直接返回错误码 */
    result = (AT_RreturnCodeUint32)LimitedCmdProc(clientId, data, len, cmdElement);
    if (result != AT_OK) {
        return result;
    }

    /* 匹配解析参数 */
    result = (AT_RreturnCodeUint32)ParseParam(cmdElement);

    if (result != ERR_MSP_SUCCESS) {
        if (cmdElement->paramErrCode == AT_RRETURN_CODE_BUTT) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            return (AT_RreturnCodeUint32)(cmdElement->paramErrCode);
        }
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    /* 下发的AT命令是否与CL模式下需要保护的AT命令相匹配，匹配的话，直接返回操作不允许 */
    if (At_CheckCurrRatModeIsCL(clientId) == VOS_TRUE) {
        if (atCmdIsSupportedByCLMode(clientId) == VOS_TRUE) {
            return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
        }
    }
    else {
        if (atCmdIsSupportedByGULMode(clientId) == VOS_FALSE) {
            return AT_CME_OPERATION_NOT_ALLOWED;
        }
    }
#endif

    result = (AT_RreturnCodeUint32)atCmdDispatch(clientId);

    return result;
}

VOS_VOID RepeatCmdProc(AT_ParseContext *clientContext)
{
    VOS_UINT8 *data = clientContext->dataBuff;
    VOS_UINT32 len  = clientContext->dataLen;
    errno_t    memResult;

    if (g_atS4 == *data) {
        data++;
        len--;
    }

    /* 检查是否是"A/" */
    if (len == AT_A0X2F_AND_CR_LEN) {
        if (At_CheckCharA(*data) == AT_SUCCESS) {
            if (atCheckChar0x2f(*(data + 1)) == AT_SUCCESS) {
                if (clientContext->cmdLineLen == 0) {
                    clientContext->dataLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                        (VOS_CHAR *)clientContext->dataBuff, (VOS_CHAR *)clientContext->dataBuff, "AT%c", g_atS3);
                } else if (clientContext->cmdLineLen > AT_CMD_MAX_LEN) {
                        AT_ERR_LOG("clientContext->cmdLine cannot copy to clientContext->dataBuff");
                        return;
                } else {
                    memResult = memcpy_s(clientContext->dataBuff, sizeof(clientContext->dataBuff),
                                         (VOS_UINT8 *)clientContext->cmdLine, clientContext->cmdLineLen);
                    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(clientContext->dataBuff), clientContext->cmdLineLen);

                    clientContext->dataLen = clientContext->cmdLineLen;
                }
            }
        }
    }
}

VOS_VOID SaveRepeatCmd(AT_ParseContext *clientContext, VOS_UINT8 *data, VOS_UINT16 len)
{
    errno_t memResult;
    /* 保存A/命令缓存 */
    if (clientContext->cmdLine != NULL) {
        AT_FREE(clientContext->cmdLine);
        clientContext->cmdLine = VOS_NULL_PTR;
    }

    if (len == 0) {
       return;
    }

    clientContext->cmdLine = (VOS_UINT8 *)AT_MALLOC(len);
    if (clientContext->cmdLine == NULL) {
        return;
    }

    (VOS_VOID)memset_s(clientContext->cmdLine, len, 0x00, len);

    if (len > 0) {
        memResult = memcpy_s(clientContext->cmdLine, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }
    clientContext->cmdLineLen = len;
}

VOS_UINT32 ScanDelChar(VOS_UINT8 *data, VOS_UINT16 *len, VOS_UINT8 atS5)
{
    VOS_UINT16 chkLen  = 0;
    VOS_UINT16 lenTemp = 0;
    VOS_UINT8 *write   = data;
    VOS_UINT8 *read    = data;
    VOS_UINT16 num     = 0; /* 记录逗号的个数 */

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 输入参数检查 */
    while (chkLen++ < *len) {
        if (*read == '"') {
            num++;
            *write++ = *read;
            lenTemp++;
        } else if ((atS5 == *read) && ((num % 2) == 0)) { /* 删除上一字符,对2求余表示引号内的删除符不处理 */
            if (lenTemp > 0) {
                write--;
                lenTemp--;
            }
        } else { /* 选择有效字符 */
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }
    *len = lenTemp;
    return AT_SUCCESS;
}

VOS_UINT32 ScanCtlChar(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT8 *write   = data;
    VOS_UINT8 *read    = data;
    VOS_UINT16 chkLen  = 0;
    VOS_UINT16 lenTemp = 0;
    VOS_UINT16 num     = 0;

    if (*len == 0) {
        return AT_FAILURE;
    }

    while (chkLen++ < *len) {
        if (*read >= 0x20) { /* 选择有效字符 */
            if (*read == '"') {
                num++;
            }
            *write++ = *read;
            lenTemp++;
        } else {
            if (num % 2) { /* 双引号内的小于0x20的字符不去除 */
                *write++ = *read;
                lenTemp++;
            }
        }
        read++;
    }

    *len = lenTemp;
    return AT_SUCCESS;
}

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
/*
 * 功能描述: 判断当前是否在双引号内，加转义字符除外
 */
VOS_VOID IsInQuotationMarks(VOS_UINT16 chkLen, VOS_UINT8 *read, VOS_UINT16 *quotationMarksNum, VOS_BOOL *isInQuotationMarks)
{
    if ((chkLen > 1) && (*read == '"') && (*(read - 1) != '\\')) {
        (*quotationMarksNum)++;
        *isInQuotationMarks = *quotationMarksNum % 2;
    }
}
#endif

VOS_UINT32 ScanBlankChar(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT8 *write        = data;
    VOS_UINT8 *read         = data;
    VOS_UINT16 chkLen       = 0;
    VOS_UINT16 lenTemp      = 0;
    VOS_UINT16 colonBankCnt = 0;
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    VOS_UINT16 quotationMarksNum = 0;
    VOS_BOOL   isInQuotationMarks = FALSE;
#endif
    VOS_BOOL   bIsColonBack = FALSE;
    VOS_BOOL   bIsEqual     = FALSE;

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 检查 */
    while (chkLen++ < *len) {
        if (*read == '=') {
            bIsEqual = TRUE;
        }
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
        /* 判断当前是否在双引号内，加转义字符除外 */
        IsInQuotationMarks(chkLen, read, &quotationMarksNum, &isInQuotationMarks);
#endif
        if ((*read == ' ') && (bIsEqual != TRUE)) {
            /* 等号前的空格去除 */
        } else { /* rim逗号前后的空格 */
            if (*read != 0x20 && *read != ',') {
                /* rim','后空格 */
                if (bIsColonBack) {
                    write -= colonBankCnt;
                    lenTemp -= colonBankCnt;
                    colonBankCnt = 0;
                    bIsColonBack = FALSE;
                } else {
                    colonBankCnt = 0;
                }
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
            } else if ((*read == ',') && (isInQuotationMarks == FALSE)) {
                /* rim','前空格 */
                write -= colonBankCnt;
                lenTemp -= colonBankCnt;
                colonBankCnt = 0;
                bIsColonBack = TRUE;
            } else if (isInQuotationMarks == TRUE) {
                colonBankCnt = 0;
#else
            } else if (*read == ',') {
                /* rim','前空格 */
                write -= colonBankCnt;
                lenTemp -= colonBankCnt;
                colonBankCnt = 0;
                bIsColonBack = TRUE;
#endif
            } else {
                colonBankCnt++;
            }

            *write++ = *read;
            lenTemp++;
        }

        read++;
    }

    lenTemp -= colonBankCnt;

    *len = lenTemp;
    return AT_SUCCESS;
}

PRIVATE VOS_UINT32 ScanBlankCharAfterEq(IN OUT VOS_UINT8 *data, IN OUT VOS_UINT16 *len)
{
    VOS_UINT8 *write        = data;
    VOS_UINT8 *read         = data;
    VOS_UINT16 chkLen       = 0;
    VOS_UINT16 lenTemp      = 0;
    VOS_BOOL   bIsEqual     = FALSE;
    VOS_BOOL   bIsEqualNoSp = FALSE;

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 检查 */
    while (chkLen++ < *len) {
        if (*read == '=') {
            bIsEqual = TRUE;
        } else {
            if ((bIsEqual != TRUE) || (bIsEqualNoSp == TRUE)) {
                /* 等号前的字符全部复制，等号后的非空格字符全部复制 */
            } else {
                if (*read == ' ') {
                    read++;
                    continue;
                } else {
                    bIsEqualNoSp = TRUE;
                }
            }
        }

        *write++ = *read;
        lenTemp++;

        read++;
    }

    *len = lenTemp;
    return AT_SUCCESS;
}

VOS_UINT32 FormatCmdStr(VOS_UINT8 *data, VOS_UINT16 *len, VOS_UINT8 atS3)
{
    VOS_UINT8 *check    = data;
    VOS_UINT32 chkS3Len = 0;
    VOS_UINT32 flg      = 0;
    VOS_UINT16 lenTemp  = 0;

    /* 保留行结束符之前的字符 */
    while (chkS3Len++ < *len) {
        if (atS3 == *check++) {
            flg = 1;
            break;
        } else {
            lenTemp++;
        }
    }

    if (flg != 1) {
        return AT_FAILURE;
    } else {
        *len = lenTemp;
        return AT_SUCCESS;
    }
}

VOS_UINT32 AT_DiscardInvalidChar(VOS_UINT8 *data, VOS_UINT32 dataLength, VOS_UINT16 *len)
{
    VOS_UINT16 i;
    VOS_UINT16 lenTemp;
    errno_t    returnValue;

    if ((*len == 0) || (*len > AT_CMD_MAX_LEN)) {
        PS_PRINTF_WARNING("<AT_DiscardInvalidChar> usLen > AT_CMD_MAX_LEN or usLen = 0. usLen :%d", *len);
        return ERR_MSP_FAILURE;
    }
    for (i = 0; (i + 1) < (*len); i++) {
        if (At_CheckCharA(data[i]) != AT_SUCCESS) {
            continue;
        } else if (At_CheckCharT(data[i + 1]) != AT_SUCCESS) {
            continue;
        } else {
            if (i != 0) {
                lenTemp     = (*len - i);
                returnValue = memmove_s(data, dataLength, (data + i), lenTemp);
                TAF_MEM_CHK_RTN_VAL(returnValue, dataLength, lenTemp);
                *len = lenTemp;
            }

            return ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_FAILURE;
}

VOS_UINT32 CmdStringFormat(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT32 ret;

    ret = FormatCmdStr(data, len, g_atS3);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_ERROR;
    }

    /* 扫描退格符 */
    ret = ScanDelChar(data, len, g_atS5);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* 扫描控制符 */
    ret = ScanCtlChar(data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* 扫描空格符 */
    ret = ScanBlankChar(data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* 扫描紧挨着等号后面的空格符 */
    ret = ScanBlankCharAfterEq(data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    return ERR_MSP_SUCCESS;
}

VOS_BOOL CheckAnyCharCmdName(VOS_UINT8 *data, VOS_UINT16 *len, VOS_UINT8 *name, VOS_UINT32 nameLen)
{
    VOS_UINT16 i;
    VOS_UINT8  bFound = 0; /* check if found string ANY CHAR CMD */
    VOS_UINT8  offset = 0;
    VOS_UINT8 *header = data; /* remember the header of the pointer */
    errno_t    returnValue;

    for (i = 0; i < *len; i++, header++) {
        if (*header == ' ') {
            continue; /* skip space char */
        } else if ((offset < (nameLen - 1)) && (name[offset] == At_UpChar(*header))) {
            offset++;

            if (name[offset] == '?') {
                bFound = 1; /* found string ANY CHAR CMD  */
            }
        } else {
            break; /* not ANY CHAR CMD, go out */
        }
    }

    if (bFound) {
        if ((nameLen - 1) == offset) { /* found string ANY CHAR CMD */
            *len -= i;                 /* remove string "AT^CPBW=" */
            if (*len > 0) {
                returnValue = memmove_s(data, *len, data + i, *len);
                TAF_MEM_CHK_RTN_VAL(returnValue, *len, *len);
            }
            return TRUE;
        }
    }

    return FALSE;
}

LOCAL VOS_UINT32 AT_DeleteCharInvalidSpace(VOS_UINT8 *para, VOS_UINT16 *paraLen)
{
    VOS_UINT16 j;

    /* 入参*paraLen防止反转判断 */
    if (*paraLen < 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (j = (*paraLen) - 1; j > 0; j--) {
        if (para[j] == ' ') {
            (*paraLen)--;
        } else if (para[j] == '"') {
            break;
        } else {
            return AT_CME_INCORRECT_PARAMETERS; /* 最后一个双引号后面有其它字符，返回错误 */
        }
    }

    return AT_SUCCESS;
}
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
VOS_UINT32 AnyCharCmdParse(VOS_UINT8 *data, VOS_UINT16 len, VOS_UINT8 *name)
{
    VOS_UINT8 *para     = VOS_NULL_PTR;           /* 用来放置解析出的参数字符串 */
    VOS_UINT16 paraLen  = 0;   /* 用来标识参数字符串长度 */
    errno_t    memResult;
    VOS_UINT16 i;
    VOS_UINT8  bInQoute = 0;

    AT_ResetParseVariable(); /* 重置解释器所用的全局变量 */

    para = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, AT_PARA_MAX_LEN + 1);
    if (para == NULL) {
        AT_ERR_LOG("AnyCharCmdParse: Fail to malloc mem.");
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(para, AT_PARA_MAX_LEN + 1, 0, AT_PARA_MAX_LEN + 1);

    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD; /* 设置命令操作类型 */
    g_atParseCmd.cmdFmtType = AT_EXTEND_CMD_TYPE;      /* 设置命令类型 */

    g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)VOS_StrLen((VOS_CHAR *)name) - AT_CMD_AT_LEN; /* 命令名 ^CPBW 长度 */

    /* pName输入点是自定义的字符串长度大于2，可以保证参数4非0 */
    memResult = memcpy_s(g_atParseCmd.cmdName.cmdName, sizeof(g_atParseCmd.cmdName.cmdName), name,
                         g_atParseCmd.cmdName.cmdNameLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParseCmd.cmdName.cmdName), g_atParseCmd.cmdName.cmdNameLen);

    /* ^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]] */
    /* ^SCPBW= (1-65535),(numa),(0-255),(numb),(0-255),(numc),(0-255),(numd),(0-255),(text),(0,1),(email) */
    /* +CPBW=[<index>][,<number>[,<type>[,<text>]]] */

    /* seperate para by char ',' */
    for (i = 0; i < len; i++, data++) {
        if (paraLen > AT_PARA_MAX_LEN) {
            VOS_MemFree(WUEPS_PID_AT, para);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((*data == ' ') && (bInQoute == 0)) {
            continue; /* skip space char not in quatation */
        } else if (*data == ',') {
            if (bInQoute) {
                if (bInQoute == 1) {
                    /* 表明该','号是参数的一部分，而非参数分隔符 */
                    para[paraLen] = *data;
                    paraLen++;
                    continue;
                }

                /* 双引号内可能还有双引号，不能再根据第二个双引号认为字符串结束 , 以参数分割符','作为字符串结束符。 */
                bInQoute = 0;

                /* 处理以下异常 : 在字符串后面，','前面的无效空格 */
                if (AT_DeleteCharInvalidSpace(para, &paraLen) != AT_SUCCESS) {
                    VOS_MemFree(WUEPS_PID_AT, para);
                    return AT_CME_INCORRECT_PARAMETERS;
                }
            }

            if (i < (len - 1)) {
                if (atfwParseSaveParam(para, paraLen) != ERR_MSP_SUCCESS) {
                    VOS_MemFree(WUEPS_PID_AT, para);
                    return AT_ERROR;
                }
                paraLen = 0;
                /* too many para */
            }
            continue;
        } else {
            /* 存储参数到全局变量中 */
            para[paraLen] = *data;
            paraLen++;

            if (*data == '"') {
                bInQoute++;
            }
        }
    }

    if (atfwParseSaveParam(para, paraLen) != ERR_MSP_SUCCESS) {
        VOS_MemFree(WUEPS_PID_AT, para);
        return AT_ERROR;
    }

    VOS_MemFree(WUEPS_PID_AT, para);
    return AT_SUCCESS;
}
#else
VOS_UINT32 AnyCharCmdParse(VOS_UINT8 *data, VOS_UINT16 len, VOS_UINT8 *name)
{
    VOS_UINT16 i;
    VOS_UINT8  bInQoute = 0;

    VOS_UINT8  para[AT_PARA_MAX_LEN + 1] = {0}; /* 用来放置解析出的参数字符串 */
    VOS_UINT16 paraLen                   = 0;   /* 用来标识参数字符串长度 */
    errno_t    memResult;

    AT_ResetParseVariable(); /* 重置解释器所用的全局变量 */

    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD; /* 设置命令操作类型 */
    g_atParseCmd.cmdFmtType = AT_EXTEND_CMD_TYPE;      /* 设置命令类型 */

    g_atParseCmd.cmdName.cmdNameLen = (VOS_UINT16)VOS_StrLen((VOS_CHAR *)name) - AT_CMD_AT_LEN; /* 命令名 ^CPBW 长度 */

    /* pName输入点是自定义的字符串长度大于2，可以保证参数4非0 */
    memResult = memcpy_s(g_atParseCmd.cmdName.cmdName, sizeof(g_atParseCmd.cmdName.cmdName), name,
                         g_atParseCmd.cmdName.cmdNameLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParseCmd.cmdName.cmdName), g_atParseCmd.cmdName.cmdNameLen);

    /* ^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]] */
    /* ^SCPBW= (1-65535),(numa),(0-255),(numb),(0-255),(numc),(0-255),(numd),(0-255),(text),(0,1),(email) */
    /* +CPBW=[<index>][,<number>[,<type>[,<text>]]] */

    /* seperate para by char ',' */
    for (i = 0; i < len; i++, data++) {
        if (paraLen > AT_PARA_MAX_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((*data == ' ') && (bInQoute == 0)) {
            continue; /* skip space char not in quatation */
        } else if (*data == ',') {
            if (bInQoute) {
                if (bInQoute == 1) {
                    /* 表明该','号是参数的一部分，而非参数分隔符 */
                    para[paraLen] = *data;
                    paraLen++;

                    continue;
                }

                /* 双引号内可能还有双引号，不能再根据第二个双引号认为字符串结束 , 以参数分割符','作为字符串结束符。 */
                bInQoute = 0;

                /* 处理以下异常 : 在字符串后面，','前面的无效空格 */
                if (AT_DeleteCharInvalidSpace(para, &paraLen) != AT_SUCCESS) {
                    return AT_CME_INCORRECT_PARAMETERS;
                }
            }

            if (i < (len - 1)) {
                if (atfwParseSaveParam(para, paraLen) != ERR_MSP_SUCCESS) {
                    return AT_ERROR;
                }

                paraLen = 0;

                /* too many para */
            }

            continue;
        } else {
            /* 存储参数到全局变量中 */
            para[paraLen] = *data;
            paraLen++;

            if (*data == '"') {
                bInQoute++;
            }
        }
    }

    if (atfwParseSaveParam(para, paraLen) != ERR_MSP_SUCCESS) {
        return AT_ERROR;
    }

    return AT_SUCCESS;
}
#endif

VOS_UINT32 AnyCharCmdProc(VOS_UINT8 *data, VOS_UINT16 *len)
{
    VOS_UINT16 i;
    VOS_UINT32 ret                                                        = 0;
    VOS_UINT8  cmdNameUppercase[AT_CMD_NAME_MAX_NUM][AT_CMD_NAME_MAX_LEN] = { "AT^CPBW=?", "AT+CPBW=?", "AT^SCPBW=?" };

    for (i = 0; i < AT_CMD_NAME_MAX_NUM; i++) {
        if (CheckAnyCharCmdName(data, len, cmdNameUppercase[i],
                VOS_StrNLen((VOS_CHAR *)cmdNameUppercase[i], AT_CMD_NAME_MAX_LEN)) == TRUE) {
            ret = FormatCmdStr(data, len, g_atS3);
            if (ret != ERR_MSP_SUCCESS) {
                return AT_ERROR;
            }

            return AnyCharCmdParse(data, *len, &(cmdNameUppercase[i][AT_CMD_NAME_COLUMNS_INDEX]));
        }
    }

    return AT_FAILURE;
}

LOCAL VOS_VOID AT_ProcReadyClientCombineCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_RreturnCodeUint32 result;

    result = (AT_RreturnCodeUint32)At_CombineCmdChkProc(indexNum, data, len);

    /* 返回结果 */
    if ((result == AT_FAILURE) || (result == AT_SUCCESS)) {
        return;
    }

    if (result != AT_WAIT_ASYNC_RETURN) {
        At_FormatResultData(indexNum, result);
    }
}

VOS_VOID At_ReadyClientCmdProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT32 dataLength, VOS_UINT16 len)
{
    VOS_UINT32           ret;
    AT_ParseContext     *clientContext = NULL;

    if (len < AT_CMD_MIN_LEN) {
        return;
    }

    clientContext = &(g_parseContext[indexNum]);

    SaveRepeatCmd(clientContext, data, len);

    At_ResetCombinParseInfo(indexNum);

    ret = AT_DiscardInvalidChar(data, dataLength, &len);
    if (ret != ERR_MSP_SUCCESS) {
        return;
    }

    ret = AnyCharCmdProc(data, &len);

    if (ret == AT_SUCCESS) { /* 是任意字符参数的命令 */
        /* 命令处理 */
        ret = CmdParseProc(indexNum, data, len);
        if ((ret != AT_OK) && (ret != AT_WAIT_ASYNC_RETURN)) {
            At_ResetCombinParseInfo(indexNum);
        }

        At_FormatResultData(indexNum, ret);

        return;
    } else if (ret != AT_FAILURE) { /* 是任意字符参数的命令，但参数解析失败 */
        At_FormatResultData(indexNum, ret);

        return;
    } else { /* 不是任意字符参数的命令 */
        /* do nothing */
    }

    ret = CmdStringFormat(indexNum, data, &len);
    if (ret != ERR_MSP_SUCCESS) {
        At_FormatResultData(indexNum, ret);

        return;
    }

    /* 最短为"AT"两个字符 */
    if (len < AT_PARSE_AT_LEN) {
        return;
    }

    /* 根据V3R1要求，前两个字符不是"AT"，不返回任何信息 */
    if (At_CheckCharA(*data) != AT_SUCCESS) { /* 如果不是'A'/'a' */
        return;
    }

    if (At_CheckCharT(*(data + 1)) != AT_SUCCESS) { /* 如果不是'T'/'t' */
        return;
    }

    /* SIMLOCK命令可能有61个参数，特殊处理 */
    if (At_ProcSimLockPara(indexNum, data, len) == AT_SUCCESS) {
        return;
    }

    /* ^DOCK命令后面还有"^"和"="，特殊处理 */
    if (AT_HandleDockSetCmd(indexNum, data, len) == AT_SUCCESS) {
        return;
    }

    /*
     * AP-Modem形态下，产线命令AT^FACAUTHPUBKEY和AT^SIMLOCKDATAWRITE的设置命令，参数已经超过了解析器中
     * 参数长度上限512，需要特殊处理
     */
    if (At_HandleApModemSpecialCmd(indexNum, data, len) == AT_SUCCESS) {
        return;
    }

    AT_ProcReadyClientCombineCmd(indexNum, data, len);
}

VOS_VOID AT_ProcAgpsStream(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length, AT_ParseContext *clientCxt)
{
    AT_RreturnCodeUint32 result;
    AT_ModemAgpsCtx     *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(clientId);
    result  = (AT_RreturnCodeUint32)At_ProcXmlText(clientId, data, length);

    /* 处理成功，输出提示符">" */
    if (result == AT_WAIT_XML_INPUT) {
        At_FormatResultData(clientId, result);
        return;
    }

    /* 清空缓存区 */
    PS_MEM_FREE(WUEPS_PID_AT, agpsCtx->xml.xmlTextHead); /* XML BUFFER清空 */
    agpsCtx->xml.xmlTextHead = VOS_NULL_PTR;
    agpsCtx->xml.xmlTextCur  = VOS_NULL_PTR;

    At_SetCmdSubMode(clientId, AT_NORMAL_MODE);

    if (result == AT_WAIT_ASYNC_RETURN) {
        if (clientCxt->cmdElement != NULL) {
            if (At_StartTimer(clientCxt->cmdElement->setTimeOut, clientId) != AT_SUCCESS) {
                AT_ERR_LOG("AT_ProcAgpsStream():ERROR:Start Timer Failed");
            }

            clientCxt->clientStatus = AT_FW_CLIENT_STATUS_PEND;
        }
        return;
    }

    At_FormatResultData(clientId, result);
}

VOS_VOID atCmdMsgProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT32 dataLength, VOS_UINT16 len)
{
    VOS_UINT32       ret;
    AT_ParseContext *parseCtx = VOS_NULL_PTR;

    if ((len == 0) || (len > AT_CMD_MAX_LEN)) {
        return;
    }

    parseCtx = &(g_parseContext[indexNum]);

    switch (parseCtx->mode) {
        case AT_SMS_MODE:
            AT_ProcSmsStream(indexNum, data, len, parseCtx);
            return;

        case AT_XML_MODE:
            AT_ProcAgpsStream(indexNum, data, len, parseCtx);
            return;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case AT_UE_POLICY_MODE:
            At_ProcUePolicyInfoStream(indexNum, data, len, parseCtx);
            return;

        case AT_IMS_URSP_MODE:
            At_ProcImsUrspStream(indexNum, data, len, parseCtx);
            return;
#endif
        default:
            break;
    }

    /* 避免组合命令处理过程中其他通道有命令输入 */
    ret = AT_ParseCmdIsComb(indexNum, data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return;
    }

    /* 判断是否有PEND状态的通道 */
    ret = AT_ParseCmdIsPend(indexNum, data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return;
    }

    At_ReadyClientCmdProc(indexNum, data, dataLength, len);
}

VOS_UINT32 At_CmdStreamRcv(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8       *head          = NULL;
    VOS_UINT16       count         = 0; /* 当前指向的字符位置 */
    VOS_UINT16       total         = 0; /* 已经处理的字符个数 */
    AT_ParseContext *clientContext = NULL;
    errno_t          memResult;

    /* ucIndex只检查一次，后续的子函数不再检查 */
    if ((data == NULL) || (indexNum >= AT_MAX_CLIENT_NUM)) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    clientContext = &(g_parseContext[indexNum]);

    head = data;

    /* 行结束符(<CR>或者用户指定) */
    while (count++ < len) {
        if (At_CheckCmdSms(*(data + count - 1), clientContext->mode)) {
            /* 缓存中已经有数据 */
            if (clientContext->dataLen > 0) {
                if ((clientContext->dataLen + count) >= AT_CMD_MAX_LEN) {
                    clientContext->dataLen = 0;
                    return ERR_MSP_INSUFFICIENT_BUFFER;
                }

                memResult = memcpy_s(&clientContext->dataBuff[clientContext->dataLen],
                                     AT_CMD_MAX_LEN - clientContext->dataLen, head, count);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN - clientContext->dataLen, count);
                clientContext->dataLen += count;
            } else { /* 缓存中无数据 */
                memResult = memcpy_s(&clientContext->dataBuff[0], AT_CMD_MAX_LEN, head, (VOS_SIZE_T)(count - total));
                TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN, (VOS_SIZE_T)(count - total));
                clientContext->dataLen = count - total;
            }

            RepeatCmdProc(clientContext);

            atCmdMsgProc(indexNum, clientContext->dataBuff, sizeof(clientContext->dataBuff), clientContext->dataLen);

            head                   = data + count;
            total                  = count;
            clientContext->dataLen = 0;
        }
    }

    /* 有部分字符未处理，需要缓存 */
    if (total < len) {
        if ((clientContext->dataLen + (len - total)) >= AT_CMD_MAX_LEN) {
            clientContext->dataLen = 0;
            return ERR_MSP_INSUFFICIENT_BUFFER;
        }

        memResult = memcpy_s(&clientContext->dataBuff[clientContext->dataLen], AT_CMD_MAX_LEN - clientContext->dataLen,
                             head, (VOS_UINT32)(len - total));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN - clientContext->dataLen, (VOS_UINT32)(len - total));

        clientContext->dataLen += (VOS_UINT16)(len - total); /* (pData-pHead+ulLen); */
    }

    return ERR_MSP_SUCCESS;
}

VOS_VOID At_CombineBlockCmdProc(VOS_UINT8 indexNum)
{
    errno_t               memResult;
    AT_RreturnCodeUint32  result         = AT_WAIT_ASYNC_RETURN;
    AT_ParseContext      *clientContext  = NULL;
    AT_FW_CombineCmdInfo *combineCmdInfo = NULL;
    VOS_UINT8            *blockCmd       = NULL;
    VOS_UINT16            blockCmdLen    = 0;

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        return;
    }

    clientContext = &(g_parseContext[indexNum]);

    combineCmdInfo = &clientContext->combineCmdInfo;

    /* 判断组合命令是否处理完毕 */
    if ((combineCmdInfo->procNum < combineCmdInfo->totalNum)) {
        /* 处理下一条命令 */
        result = (AT_RreturnCodeUint32)At_CombineCmdProc(indexNum);

        /* 返回结果 */
        if ((result == AT_FAILURE) || (result == AT_SUCCESS)) {
            return;
        }

        if (result != AT_WAIT_ASYNC_RETURN) {
            At_FormatResultData(indexNum, result);
        }

        return;
    }

    /* 判断是否有缓存的命令 */
    indexNum = AT_BlockCmdCheck();
    if (indexNum < AT_MAX_CLIENT_NUM) {
        clientContext = &(g_parseContext[indexNum]);

        /* 停止缓存命令的定时器 */
        AT_StopRelTimer(indexNum, &clientContext->timer);

        blockCmdLen = clientContext->blockCmdLen;

        if (blockCmdLen == 0) {
            return;
        }

        blockCmd    = (VOS_UINT8 *)AT_MALLOC(blockCmdLen);
        if (blockCmd == NULL) {
            /* 留待超时返回或下条命令处理结束后触发 */
            return;
        }

        (VOS_VOID)memset_s(blockCmd, blockCmdLen, 0x00, blockCmdLen);

        if (blockCmdLen > 0) {
            memResult = memcpy_s(blockCmd, blockCmdLen, clientContext->blockCmd, blockCmdLen);
            TAF_MEM_CHK_RTN_VAL(memResult, blockCmdLen, blockCmdLen);
        }

        AT_ClearBlockCmdInfo(indexNum);

        /* 处理缓存命令 */
        At_ReadyClientCmdProc(indexNum, blockCmd, blockCmdLen, blockCmdLen);

        AT_FREE(blockCmd);

        return;
    }
}

VOS_VOID At_MsgNumReduce(AT_Msg *msgTemp)
{
    AT_CmdMsgNumCtrl *msgNumCtrlCtx = VOS_NULL_PTR;
    VOS_ULONG         lockLevel;
    VOS_UINT32        qosType;

    if ((msgTemp->type == AT_NORMAL_TYPE_MSG) || (msgTemp->type == AT_ATNLPROXY_MSG)) {
        msgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();
        qosType = (msgTemp->type == AT_ATNLPROXY_MSG) ? AT_MSG_QOSTYPE_ATNLPROXY: AT_MSG_QOSTYPE_NORMAL;

        lockLevel = 0;
        /* 队列加锁 */
        /*lint -e571*/
        VOS_SpinLockIntLock(&(msgNumCtrlCtx->spinLock), lockLevel);
        /*lint +e571*/

        if (msgNumCtrlCtx->msgCount[qosType] > 0) {
            msgNumCtrlCtx->msgCount[qosType]--;
        }
#if (FEATURE_IOT_ATNLPROXY== FEATURE_ON)
        if ((msgTemp->type == AT_ATNLPROXY_MSG) && (msgNumCtrlCtx->msgCount[qosType] <= AT_GetMaxCtrlNum(qosType))) {
            atnlproxy_wakeup_tsk();
        }
#endif
        VOS_SpinUnlockIntUnlock(&(msgNumCtrlCtx->spinLock), lockLevel);
    }
}

VOS_VOID At_CmdMsgDistr(struct MsgCB *msg)
{
    AT_Msg           *msgTemp       = VOS_NULL_PTR;
    VOS_UINT16        len;
    VOS_UINT8        *data = NULL;
    VOS_UINT8         indexNum;

    msgTemp = (AT_Msg *)msg;

    At_MsgNumReduce(msgTemp);

    len      = msgTemp->len;
    data     = msgTemp->value;
    indexNum = msgTemp->indexNum;

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    if (msgTemp->type == AT_ATNLPROXY_MSG) {
        AtNlProxyDealwithUspaceData((VOS_VOID*)data, (VOS_INT32)len);
        return;
    }
#endif

#if (FEATURE_AT_PROXY == FEATURE_ON)
    if (msgTemp->type == AT_ATP_MSG) {
        AT_ProcAtpMsg((VOS_VOID*)data, (VOS_UINT32)len);
        return;
    }
#endif

    /* C核/HIFI复位时，模拟的是AT给AT发的消息 */
    switch (msgTemp->type) {
        case ID_CCPU_AT_RESET_START_IND:
            AT_RcvCcpuResetStartInd((struct MsgCB *)msgTemp);
            return;

        case ID_CCPU_AT_RESET_END_IND:
            AT_RcvCcpuResetEndInd((struct MsgCB *)msgTemp);
            return;

        case ID_HIFI_AT_RESET_BEGIN_IND:
            AT_RcvHifiResetBeginInd((struct MsgCB *)msgTemp);
            return;

        case ID_HIFI_AT_RESET_END_IND:
            AT_RcvHifiResetEndInd((struct MsgCB *)msgTemp);
            return;

        default:
            break;
    }

    AT_AddUsedClientId2Tab(indexNum);

    /* 如果处于C核复位前则不处理任何AT命令 */
    if (AT_GetResetFlag() == VOS_TRUE) {
        return;
    }


    if (msgTemp->type == AT_COMBIN_BLOCK_MSG) {
        At_CombineBlockCmdProc(indexNum);
        return;
    }

    At_CmdStreamRcv(indexNum, data, len);
}

VOS_UINT32 atfwParseSaveParam(VOS_UINT8 *stringBuf, VOS_UINT16 len)
{
    errno_t memResult;

    if ((len > AT_PARA_MAX_LEN) || (g_atParaIndex >= AT_MAX_PARA_NUMBER)) {
        return ERR_MSP_FAILURE;
    }

    /* 如果长度为0，则允许字符串指针为空，否则返回失败 */
    if ((stringBuf == NULL) && (len != 0)) {
        return ERR_MSP_FAILURE;
    }

    /* 如果字符串长度为0，则不需要拷贝 */
    if ((len != 0) && (stringBuf != NULL)) {
        memResult = memcpy_s(g_atParaList[g_atParaIndex].para, sizeof(g_atParaList[g_atParaIndex].para), stringBuf,
                             len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParaList[g_atParaIndex].para), len);
    }

    g_atParaList[g_atParaIndex].paraLen = len;
    g_atParaIndex++;

    return ERR_MSP_SUCCESS;
}

VOS_UINT32 AT_SaveCdataDialParam(VOS_UINT8 *stringBuf, VOS_UINT16 len)
{
    VOS_UINT8 *currPtr = stringBuf;
    errno_t    memResult;
    VOS_UINT16 length;

    if ((len > AT_PARA_MAX_LEN) || (stringBuf == NULL)) {
        return ERR_MSP_FAILURE;
    }
    /*
     * 为适配解析"*99#", "*98#",
     * 将"#777"解析存为:
     * #,
     * #777,
     * 777
     */

    if (len > AT_CDATA_DIAL_777_LEN) {
        /* 指针跳过'D' */
        currPtr++;

        /* 保存'#' */
        g_atParaList[g_atParaIndex].para[0] = *currPtr;
        g_atParaList[g_atParaIndex].paraLen = 1;

        /* 保存'#777' */
        g_atParaIndex++;
        length    = len - 1;
        memResult = memcpy_s(g_atParaList[g_atParaIndex].para, sizeof(g_atParaList[g_atParaIndex].para), currPtr,
                             length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParaList[g_atParaIndex].para), length);
        g_atParaList[g_atParaIndex].paraLen = length;

        /* 保存'777' */
        currPtr++;
        g_atParaIndex++;
        length--;
        memResult = memcpy_s(g_atParaList[g_atParaIndex].para, sizeof(g_atParaList[g_atParaIndex].para), currPtr,
                             length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atParaList[g_atParaIndex].para), length);
        g_atParaList[g_atParaIndex].paraLen = length;
    }

    g_atParaIndex++;

    return ERR_MSP_SUCCESS;
}

VOS_UINT32 AT_IsAbortCmdStr(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_ABORT_CmdPara       *abortCmdPara = VOS_NULL_PTR;
    VOS_UINT32              abortCmdStrLen;
    VOS_UINT32              atCurrSetTick;
    AT_CMD_ABORT_TICK_INFO *cmdAbortTick = VOS_NULL_PTR;
    VOS_UINT32              timeInternal;

    atCurrSetTick = VOS_GetTick();
    cmdAbortTick  = At_GetAtCmdAbortTickInfo();

    /* 输入的字符已经去除了不可见字符(<0x20的ASCII字符),空格, S3, S5等字符. 但没有字符串结束符 */

    abortCmdStrLen = 0;

    /* 获取打断命令的参数 */
    abortCmdPara = AT_GetAbortCmdPara();

    if (abortCmdPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 判断是否使能打断命令功能 */
    if (abortCmdPara->abortEnableFlg != VOS_TRUE) {
        return VOS_FALSE;
    }

    /* 检测当前打断tick值是否满足相隔125ms条件,如果不满足，则直接返回不打断 */
    if (atCurrSetTick >= cmdAbortTick->atSetTick[indexNum]) {
        timeInternal = atCurrSetTick - cmdAbortTick->atSetTick[indexNum];
    } else {
        timeInternal = atCurrSetTick + (AT_MAX_TICK_TIME_VALUE - cmdAbortTick->atSetTick[indexNum]);
    }

    if (timeInternal < AT_MIN_ABORT_TIME_INTERNAL) {
        return VOS_FALSE;
    }

    /* 检测当前是否是任意字符打断，如果是，则直接返回打断 */
    if (At_GetAtCmdAnyAbortFlg() == VOS_TRUE) {
        return VOS_TRUE;
    }

    /* 长度判断 */
    abortCmdStrLen = VOS_StrLen((VOS_CHAR *)abortCmdPara->abortAtCmdStr);
    if ((len == 0) || (len != abortCmdStrLen)) {
        return VOS_FALSE;
    }

    /* 当前不是任意字符打断，则需要检测AT打断内容，是打断命令，则返回打断 */
    if ((VOS_StrNiCmp((VOS_CHAR *)data, (VOS_CHAR *)abortCmdPara->abortAtCmdStr, len) == 0)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_AbortCmdProc(VOS_UINT8 indexNum)
{
    VOS_UINT32 timerLen;
    VOS_UINT32 result;

    /* 空指针检查 */
    if (g_parseContext[indexNum].cmdElement == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_AbortCmdProc: pstCmdElement NULL.");
        return;
    }

    if (g_parseContext[indexNum].cmdElement->abortProc == VOS_NULL_PTR) {
        return;
    }

    /* 调用当前正在处理的AT命令的Abort函数 */
    result = g_parseContext[indexNum].cmdElement->abortProc(indexNum);
    if (result == AT_WAIT_ASYNC_RETURN) {
        /* 由于当前AT端口的状态已经处于PEND状态, 此处不需要再更新端口状态 */

        /* 启动打断保护定时器, 超时后需要向AT端口上报ABORT */
        timerLen = g_parseContext[indexNum].cmdElement->abortTimeOut;
        if (timerLen == 0) {
            AT_WARN_LOG("AT_AbortCmdProc: TimerLen 0.");
            return;
        }

        /* 起保护定时器 */
        if (At_StartTimer(timerLen, indexNum) != AT_SUCCESS) {
            AT_WARN_LOG("AT_AbortCmdProc: StartTimer Err.");
            return;
        }
    }
    /* 表示AT直接打断 不需要等异步消息 */
    else if ((result == AT_ABORT) || (result == AT_OK)) {
        result                  = AT_ABORT;
        g_atSendDataBuff.bufLen = 0;

        AT_StopTimerCmdReady(indexNum);

        At_FormatResultData(indexNum, result);
    }
    /*
     * 其它异常，如消息发送失败 内存申请失败，当前命令操作类型不支持打断
     * (如在指定搜过程收到打断命令, 但当前指定搜不支持打断)等，丢弃 不做打断处理
     */
    else {
        AT_WARN_LOG1("AT_AbortCmdProc, WARNING, Return Code %d!", result);
    }
}

AT_ParCmdElement* AT_GetCmdElementInfo(VOS_UINT8 *cmdName, VOS_UINT32 cmdType)
{
    HI_LIST_S        *curList    = VOS_NULL_PTR;
    HI_LIST_S        *cmdTblList = VOS_NULL_PTR;
    AT_ParCmdtblList *curCmdNode = VOS_NULL_PTR;
    VOS_UINT32        i;

    /* 调用的地方保证输入的指针不为空 */

    cmdTblList = &(g_cmdTblList);

    /* 如果命令类型不是扩展命令也不是基本命令，直接返回空指针 */
    if ((cmdType != AT_EXTEND_CMD_TYPE) && (cmdType != AT_BASIC_CMD_TYPE)) {
        return VOS_NULL_PTR;
    }

    /* 在各命令表中查找指定命令 */
    msp_list_for_each(curList, cmdTblList)
    {
        curCmdNode = msp_list_entry(curList, AT_ParCmdtblList, cmdTblList);

        for (i = 0; i < curCmdNode->cmdNum; i++) {
            /* 没找到时，在命令表中查找下一条命令 */
            if ((curCmdNode->cmdElement == NULL) || (curCmdNode->cmdElement[i].cmdName == NULL)) {
                continue;
            }
            /* 找到时，指针指到相应的命令 */
            if (AT_STRCMP((VOS_CHAR *)cmdName, (VOS_CHAR *)curCmdNode->cmdElement[i].cmdName) == ERR_MSP_SUCCESS) {
                return &(curCmdNode->cmdElement[i]);
            }
        }
    }

    return VOS_NULL_PTR;
}

VOS_VOID AT_SaveCmdElementInfo(VOS_UINT8 indexNum, VOS_UINT8 *cmdName, VOS_UINT32 cmdType)
{
    AT_ParCmdElement *getCmdElemnet = VOS_NULL_PTR;

    if (cmdName == VOS_NULL_PTR) {
        return;
    }

    /* 没有找到相应的指定命令时，直接返回 */
    getCmdElemnet = AT_GetCmdElementInfo(cmdName, cmdType);
    if (getCmdElemnet == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SaveCmdElementInfo: GetCmdElementInfo Failed.");
        return;
    }

    /* 找到时，保存到全局变量 */
    g_parseContext[indexNum].cmdElement = getCmdElemnet;
}

VOS_UINT32 AT_IsAnyParseClientPend(VOS_VOID)
{
    AT_PortBuffCfgInfo *portBuffCfg = VOS_NULL_PTR;
    VOS_UINT32          i;
    VOS_UINT8           clientIndex;

    portBuffCfg = AT_GetPortBuffCfgInfo();

    if (portBuffCfg->num > AT_MAX_CLIENT_NUM) {
        portBuffCfg->num = AT_MAX_CLIENT_NUM;
    }

    /* 根据当前记录的通道查询通道状态 */
    for (i = 0; i < portBuffCfg->num; i++) {
        clientIndex = (VOS_UINT8)portBuffCfg->usedClientId[i];

        if (clientIndex >= AT_CLIENT_ID_BUTT) {
            continue;
        }

        if (g_parseContext[clientIndex].clientStatus == AT_FW_CLIENT_STATUS_PEND) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_IsAllClientDataMode(VOS_VOID)
{
    AT_PortBuffCfgInfo *portBuffCfg = VOS_NULL_PTR;
    VOS_UINT32          i;
    VOS_UINT8           clientIndex;
    VOS_UINT32          dataModeNum = 0;
    DMS_PortModeUint8   mode;

    portBuffCfg = AT_GetPortBuffCfgInfo();

    if (portBuffCfg->num > AT_MAX_CLIENT_NUM) {
        portBuffCfg->num = AT_MAX_CLIENT_NUM;
    }

    /* 查看有多少个通道处于data模式 */
    for (i = 0; i < portBuffCfg->num; i++) {
        clientIndex = (VOS_UINT8)portBuffCfg->usedClientId[i];
        if (clientIndex >= AT_CLIENT_ID_BUTT) {
            continue;
        }

        mode = DMS_PORT_GetMode(g_atClientTab[clientIndex].portNo);
        if (mode == DMS_PORT_MODE_ONLINE_DATA) {
            dataModeNum++;
        }
    }

    /* 所有通道有处理data模式 */
    if (dataModeNum == portBuffCfg->num) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT8 atCmdIsSupportedByCLMode(VOS_UINT8 indexNum)
{
    AT_ParCmdElement *cmdElement = VOS_NULL_PTR;
    VOS_UINT32        loop;
    VOS_UINT32        cmdArraySize;
    const VOS_UINT32  cmdIndex[] = {
        /*
         * AT_CMD_CLIP,
         * AT_CMD_DTMF,
         */
        AT_CMD_CARDMODE,

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
        AT_CMD_CSCB,
#endif
        /* AT_CMD_CNMI, */

        /* AT_CMD_CSASM, */
        AT_CMD_CUSD,
        /*
         * AT_CMD_CMGW,
         * AT_CMD_CMGD,
         */
        AT_CMD_CCWA,
        AT_CMD_CCFC,
        /* AT_CMD_CLIR, */

        AT_CMD_CELLINFO,
        AT_CMD_CPMS,
        AT_CMD_RELEASERRC,
        AT_CMD_EOPLMN,
        AT_CMD_CNMR
        /*  电信Volte功能增加，at命令限制解除 */
    };

    cmdArraySize = sizeof((cmdIndex)) / sizeof((cmdIndex[0]));

    cmdElement = g_parseContext[indexNum].cmdElement;

    /* CLCK的操作类型为SC和FD不需要禁，其它的需要禁掉 */
    if (cmdElement->cmdIndex == AT_CMD_CLCK) {
        if ((g_atParaList[0].paraValue == AT_CLCK_SC_TYPE) || (g_atParaList[0].paraValue == AT_CLCK_FD_TYPE)) {
            return VOS_FALSE;
        } else {
            return VOS_TRUE;
        }
    }

    /* CPWD的操作类型为SC和FD不需要禁，其它的需要禁掉 */
    if (cmdElement->cmdIndex == AT_CMD_CPWD) {
        if ((g_atParaList[0].paraValue == AT_CLCK_SC_TYPE) || (g_atParaList[0].paraValue == AT_CLCK_P2_TYPE)) {
            return VOS_FALSE;
        } else {
            return VOS_TRUE;
        }
    }

    /* 判断下发的AT命令的cmdindex是否属于下列的在CL模式下待保护的AT命令的index */
    for (loop = 0; loop < cmdArraySize; loop++) {
        if (cmdElement->cmdIndex == cmdIndex[loop]) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT8 atCmdIsSupportedByGULMode(VOS_UINT8 indexNum)
{
    AT_ParCmdElement *cmdElement = VOS_NULL_PTR;
    VOS_UINT32        loop;
    VOS_UINT32        cmdArraySize;
    ModemIdUint16     modemId;
    const VOS_UINT32  cmdIndex[] = {
        AT_CMD_ECCALL, AT_CMD_ECCTRL, AT_CMD_ECCAP, AT_CMD_ECRANDOM, AT_CMD_ECKMC, AT_CMD_ECCTEST
    };

    modemId = MODEM_ID_0;

    /* 获取modemid失败，默认不拦截相应的AT命令 */
    if (AT_GetModemIdFromClient((VOS_UINT16)indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("atCmdIsSupportedByGULMode:Get Modem Id fail!");
        return VOS_TRUE;
    }

    AT_NORM_LOG1("atCmdIsSupportedByGULMode: enModemId:", (VOS_INT32)modemId);

    cmdArraySize = sizeof((cmdIndex)) / sizeof((cmdIndex[0]));

    cmdElement = g_parseContext[indexNum].cmdElement;

    /* 判断下发的AT命令的cmdindex是否属于下列的在GUL模式下不进行处理的AT命令的index */
    for (loop = 0; loop < cmdArraySize; loop++) {
        if (cmdElement->cmdIndex == cmdIndex[loop]) {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}
#endif

