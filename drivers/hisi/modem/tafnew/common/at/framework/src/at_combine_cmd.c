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

#include "at_combine_cmd.h"
#include "securec.h"
#include "at_check_func.h"

#include "at_cmd_proc.h"

#include "at_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_COMBINEDCMD_C

#define AT_ATD_CMD_MIN_LENGTH 3
#define AT_FW_LEN_AT_ADD_SEMI 3
#define AT_CMD_AT_INSIDE_SYMBOL_SEMI 2
#define AT_CALL_D_GI_PARA_LEN 2

VOS_UINT32 At_GetSecondAddr(VOS_UINT8 *data, VOS_UINT16 len, VOS_UINT8 **ppDataOut)
{
    errno_t    memResult;
    VOS_UINT8 *firAddr     = NULL;
    VOS_UINT8 *secAddr     = NULL;
    VOS_UINT32 basicCmdLen = 0;
    VOS_UINT8 *pcTmp       = NULL;

    /* 比较字符串需要保证字符串后面有\0 */
    pcTmp = (VOS_UINT8 *)AT_MALLOC(len + 1);
    if (pcTmp == NULL) {
        return ERR_MSP_FAILURE;
    }
    (VOS_VOID)memset_s(pcTmp, len + 1, 0x00, len + 1);

    if (len > 0) {
        memResult = memcpy_s(pcTmp, len + 1, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len + 1, len);
    }
    pcTmp[len] = '\0';

    firAddr = At_GetFirstBasicCmdAddr(pcTmp, &basicCmdLen);
    if (firAddr == NULL) {
        AT_FREE(pcTmp);
        return ERR_MSP_FAILURE;
    }

    secAddr = At_GetFirstBasicCmdAddr((firAddr + basicCmdLen), &basicCmdLen);

    if (secAddr == NULL) {
        *ppDataOut = data + len;
    } else {
        *ppDataOut = data + (secAddr - pcTmp);
    }

    AT_FREE(pcTmp);
    return ERR_MSP_SUCCESS;
}

VOS_UINT32 At_GetFirstCmdLen(VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8 *begin  = data;
    VOS_UINT8 *end    = data;
    VOS_UINT16 lenFir = 0;

    if (At_GetSecondAddr(begin, len, &end) == ERR_MSP_SUCCESS) {
        lenFir = (VOS_UINT16)(end - begin);
    } else {
        /* 如果查找第一个命令失败，则返回整个字符串的长度 */
        lenFir = len;
    }

    return lenFir;
}

VOS_VOID At_ResetCombinCmdInfo(HI_LIST_S *combList)
{
    AT_FW_CombineCmdInfo *combineCmdInfo = VOS_NULL_PTR;
    AT_FW_CombineCmdNode *node           = VOS_NULL_PTR;
    HI_LIST_S            *tmp            = VOS_NULL_PTR;
    HI_LIST_S            *me             = VOS_NULL_PTR;

    if ((combList == VOS_NULL_PTR) || (combList->next == VOS_NULL_PTR)) {
        return;
    }

    /* 在链表中查找每个子命令结点 */
    msp_list_for_each_safe(me, tmp, combList)
    {
        if (me == NULL) {
            combineCmdInfo           = msp_list_entry(combList, AT_FW_CombineCmdInfo, combineCmdList);
            combineCmdInfo->totalNum = 0;
            combineCmdInfo->procNum  = 0;
            /*lint -e717*/
            HI_INIT_LIST_HEAD(combList);
            /*lint +e717*/
            return;
        }

        node = msp_list_entry(me, AT_FW_CombineCmdNode, combCmdList);
        /*lint -e774*/
        if (node == VOS_NULL_PTR) {
            combineCmdInfo           = msp_list_entry(combList, AT_FW_CombineCmdInfo, combineCmdList);
            combineCmdInfo->totalNum = 0;
            combineCmdInfo->procNum  = 0;
            /*lint -e717*/
            HI_INIT_LIST_HEAD(combList);
            /*lint +e717*/
            return;
        }
        /*lint +e774*/

        msp_list_del(&(node->combCmdList));
        AT_FREE((VOS_UINT8 *)node->combCmd);
        node->combCmd = VOS_NULL_PTR;
        AT_FREE((VOS_UINT8 *)node);
        node = VOS_NULL_PTR;
    }

    combineCmdInfo           = msp_list_entry(combList, AT_FW_CombineCmdInfo, combineCmdList);
    combineCmdInfo->totalNum = 0;
    combineCmdInfo->procNum  = 0;
}

VOS_VOID At_ResetCombinParseInfo(VOS_UINT8 indexNum)
{
    HI_LIST_S       *combList      = NULL;
    AT_ParseContext *clientContext = NULL;

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        return;
    }

    clientContext = &(g_parseContext[indexNum]);

    combList = &clientContext->combineCmdInfo.combineCmdList;

    if (combList->next == NULL) {
        /*lint -e717*/
        HI_INIT_LIST_HEAD(&(clientContext->combineCmdInfo.combineCmdList));
        /*lint -e717*/
    }

    At_ResetCombinCmdInfo(combList);

    /* 清空输出buffer */
    g_atCombineSendData[indexNum].bufLen = 0;
}

LOCAL VOS_UINT32 At_StoreSubCombCmd(HI_LIST_S *combList, VOS_UINT8 *dataIn, VOS_UINT16 lenIn)
{
    AT_FW_CombineCmd     *combCmd        = NULL;
    AT_FW_CombineCmdNode *combCmdNode    = NULL; /*lint !e830*/
    AT_FW_CombineCmdInfo *combineCmdInfo = NULL;
    errno_t               memResult;

    /* 调用的地方保证指针不为空 */

    combineCmdInfo = msp_list_entry(combList, AT_FW_CombineCmdInfo, combineCmdList);
    if (combineCmdInfo->totalNum >= AT_MAX_NUM_COMBINE_CMD) {
        PS_PRINTF_WARNING("usTotalNum %d\n", combineCmdInfo->totalNum);
        return ERR_MSP_FAILURE;
    }

    combCmdNode = (AT_FW_CombineCmdNode *)AT_MALLOC(sizeof(AT_FW_CombineCmdNode));
    if (combCmdNode == NULL) {
        return ERR_MSP_MALLOC_FAILUE;
    }
    (VOS_VOID)memset_s(combCmdNode, sizeof(AT_FW_CombineCmdNode), 0x00, sizeof(AT_FW_CombineCmdNode));

    combCmd = (AT_FW_CombineCmd *)AT_MALLOC(sizeof(AT_FW_CombineCmd) + lenIn + AT_FW_LEN_AT);
    if (combCmd == NULL) {
        AT_FREE((VOS_UINT8 *)combCmdNode);

        return ERR_MSP_MALLOC_FAILUE;
    }
    (VOS_VOID)memset_s(combCmd, (VOS_SIZE_T)sizeof(AT_FW_CombineCmd) + lenIn + AT_FW_LEN_AT, 0x00,
                       (VOS_SIZE_T)sizeof(AT_FW_CombineCmd) + lenIn + AT_FW_LEN_AT);

    combCmdNode->combCmd = combCmd;
    combCmd->len         = (VOS_UINT32)lenIn + AT_FW_LEN_AT;
    memResult            = memcpy_s(combCmd->data, lenIn + AT_FW_LEN_AT, "AT", AT_FW_LEN_AT);
    TAF_MEM_CHK_RTN_VAL(memResult, lenIn + AT_FW_LEN_AT, AT_FW_LEN_AT);
    if (lenIn > 0) {
        memResult = memcpy_s(&(combCmd->data[AT_FW_LEN_AT]), lenIn, dataIn, lenIn);
        TAF_MEM_CHK_RTN_VAL(memResult, lenIn, lenIn);
    }

    msp_list_add_tail(&combCmdNode->combCmdList, combList);

    combineCmdInfo->totalNum++;

    return ERR_MSP_SUCCESS;
}
/*lint -restore */

/* 基础组合命令解析 */
static VOS_UINT32 At_BasicCombineCmdParse(HI_LIST_S *combList, VOS_UINT8 *dataIn, VOS_UINT16 lenIn)
{
    VOS_UINT32 ret    = ERR_MSP_FAILURE;
    VOS_UINT16 lenFir = 0;
    VOS_UINT16 chkLen = 0;
    VOS_UINT8 *data   = dataIn;

    while (chkLen < lenIn) {
        lenFir = (VOS_UINT16)At_GetFirstCmdLen(data, (lenIn - chkLen));

        ret = At_StoreSubCombCmd(combList, data, lenFir);
        if (ret != ERR_MSP_SUCCESS) {
            At_ResetCombinCmdInfo(combList);
            return ret;
        }

        chkLen = chkLen + lenFir;
        data   = dataIn + chkLen;
    }

    return ERR_MSP_SUCCESS;
}

/* 基础命令和扩展命令组合解析 */
VOS_UINT32 At_BasicExCombineCmdParse(HI_LIST_S *combList, VOS_UINT8 *dataIn, VOS_UINT16 lenIn, VOS_UINT16 firIndex)
{
    VOS_UINT32 ret  = ERR_MSP_FAILURE;
    VOS_UINT8 *data = dataIn;

    /* 调用的地方保证pstCombList、pDataIn不为空 */

    if (firIndex >= lenIn) {
        return ERR_MSP_FAILURE;
    }

    /* 第0个字符不是"+^$"，则之前的是基础命令 */
    if (firIndex != 0) {
        ret = At_BasicCombineCmdParse(combList, data, firIndex);
        if (ret != ERR_MSP_SUCCESS) {
            At_ResetCombinCmdInfo(combList);
            return ret;
        }
    }

    ret = At_StoreSubCombCmd(combList, (data + firIndex), (lenIn - firIndex));
    if (ret != ERR_MSP_SUCCESS) {
        At_ResetCombinCmdInfo(combList);
    }

    return ret;
}

static VOS_VOID At_UpStringCmdName(VOS_UINT8 *data, VOS_UINT16 len)
{
    TAF_UINT8 *tmp    = data; /* current Char */
    TAF_UINT32 chkLen = 0;

    if (len == 0) {
        return;
    }

    while ((chkLen++ < len) && ((*tmp) != '=')) {
        if ((*tmp >= 'a') && (*tmp <= 'z')) {
            *tmp = *tmp - 0x20;
        }
        tmp++;
    }
}

LOCAL VOS_UINT32 AT_CheckSemicolonCmd(HI_LIST_S *combList, VOS_UINT8 *dataIn)
{
    if ((combList == NULL) || (dataIn == NULL)) {
        return ERR_MSP_FAILURE;
    }

    return ERR_MSP_SUCCESS;
}
LOCAL VOS_UINT16 AT_CountExCommandCharNum(VOS_UINT8 *dataIn, VOS_UINT16 lenIn, VOS_UINT16 *firIndex)
{
    VOS_UINT16 i         = 0;
    VOS_UINT16 numQuota  = 0;
    VOS_UINT16 numSymbol = 0;

    while (i < lenIn) {
        if (dataIn[i] == '"') {
            numQuota++;
        /* 对2求余表示'"'个数应为偶数 */
        } else if (((dataIn[i] == '+') || (dataIn[i] == '^') || (dataIn[i] == '$')) && ((numQuota % 2) == 0)) {
            numSymbol++;
            if (numSymbol == 1) {
                *firIndex = i; /* 记录第一个"+^$"的位置 */
            }
        } else {
        }

        i++;
    }

    return numSymbol;
}

VOS_UINT32 At_SemicolonCmdParse(HI_LIST_S *combList, VOS_UINT8 *dataIn, VOS_UINT16 lenIn)
{
    VOS_UINT32 ret       = ERR_MSP_FAILURE;
    VOS_UINT16 numSymbol = 0;
    VOS_UINT16 firIndex  = 0;

    if (AT_CheckSemicolonCmd(combList, dataIn) == ERR_MSP_FAILURE) {
        return ERR_MSP_FAILURE;
    }

    if (At_CheckCharD(*dataIn) != AT_SUCCESS) {
        At_UpStringCmdName(dataIn, lenIn);
    } else {
        At_UpStringCmdName(dataIn, 1);
    }

    /* ';'出现在命令的结尾，且是D命令，直接处理为语音呼叫命令 */
    if (lenIn > (AT_ATD_CMD_MIN_LENGTH - 1)) {
        if (At_CheckCharD(*dataIn) == AT_SUCCESS) {
            if (At_CheckSemicolon(*(dataIn + lenIn - 1)) == AT_SUCCESS) {
                ret = At_StoreSubCombCmd(combList, dataIn, lenIn);
                if (ret != ERR_MSP_SUCCESS) {
                    At_ResetCombinCmdInfo(combList);
                }

                return ret;
            }
        }
    }

    /* ATD+117类型，直接处理为数据呼叫命令 */
    if (lenIn > (AT_ATD_CMD_MIN_LENGTH - 1)) {
        if ((At_CheckCharD(*dataIn) == AT_SUCCESS) && (dataIn[1] == '+')) {
            ret = At_StoreSubCombCmd(combList, dataIn, lenIn);
            if (ret != ERR_MSP_SUCCESS) {
                At_ResetCombinCmdInfo(combList);
            }

            return ret;
        }
    }

    /* 统计引号以外的扩展命令标识符个数 */
    numSymbol = AT_CountExCommandCharNum(dataIn, lenIn, &firIndex);

    /* 扩展命令标识符个数为0，表明该组合命令为若干基本AT命令的组合 */
    if (numSymbol == 0) {
        ret = At_BasicCombineCmdParse(combList, dataIn, lenIn);
    }
    /* 扩展命令标识符个数为1，可能是单独一个扩展命令，也可能是若干基本命令组合和一个扩展命令  */
    else if (numSymbol == 1) {
        ret = At_BasicExCombineCmdParse(combList, dataIn, lenIn, firIndex);
    }
    /* 组合命令的每条命令中，扩展命令标识符('+'、'^'、'$')最多只能出现一次 */
    else {
        ret = ERR_MSP_FAILURE;
    }

    return ret;
}

PRIVATE VOS_UINT32 At_CombineCmdPreProc(HI_LIST_S *combList, VOS_UINT8 *dataIn, VOS_UINT16 lenIn)
{
    VOS_UINT32 i        = 0;
    VOS_UINT32 ret      = ERR_MSP_FAILURE;
    VOS_UINT16 numQuota = 0;
    VOS_UINT8 *data     = dataIn;
    VOS_UINT8 *dataHead = dataIn;

    if ((combList == NULL) || (dataIn == NULL)) {
        return ERR_MSP_FAILURE;
    }

    while (i++ < lenIn) {
        /* 分号在引号外 */
        /* 对2求余表示'"'个数应为偶数 */
        if ((*data == ';') && ((numQuota % 2) == 0)) {
            ret = At_SemicolonCmdParse(combList, dataHead, (VOS_UINT16)(data - dataHead + 1));
            if (ret != ERR_MSP_SUCCESS) {
                return ret;
            }

            dataHead = (data + 1);
        } else if (*data == '"') {
            numQuota++;
        } else {
        }

        data++;
    }

    /* 最后一个字符不是分号时要解析存储最后一个组合命令 */
    if (dataHead != (dataIn + lenIn)) {
        ret = At_SemicolonCmdParse(combList, dataHead, (VOS_UINT16)(data - dataHead));
        if (ret != ERR_MSP_SUCCESS) {
            return ret;
        }
    }

    return ERR_MSP_SUCCESS;
}

VOS_UINT32 AT_IsDCmdValidChar(VOS_UINT8 para)
{
    /* 支持dialing digits: 0-9,*,#,A-D */
    if ((para >= '0') && (para <= '9')) {
        return VOS_TRUE;
    }

    if ((para >= 'a') && (para <= 'c')) {
        return VOS_TRUE;
    }

    if ((para >= 'A') && (para <= 'C')) {
        return VOS_TRUE;
    }

    if ((para == '*') || (para == '#')) {
        return VOS_TRUE;
    }

    /* 支持GSM/UMTS modifier characters: > i g I G */
    if (para == '>') {
        return VOS_TRUE;
    }

    if ((para == 'i') || (para == 'I')) {
        return VOS_TRUE;
    }

    if ((para == 'g') || (para == 'G')) {
        return VOS_TRUE;
    }

    if (para == '+') {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_InsertDCmdGIPara(VOS_UINT32 *srcStrLen, VOS_UINT8 *srcStr, VOS_UINT32 maxMemLength, VOS_UINT32 insertStrLen,
                             VOS_UINT8 *insertStr)
{
    errno_t    memResult;
    VOS_UINT32 srcStrLenTemp;

    srcStrLenTemp = *srcStrLen;

    if (srcStr[srcStrLenTemp - 1] == ';') {
        if ((insertStrLen > 0) && (maxMemLength >= srcStrLenTemp)) {
            memResult = memcpy_s(&(srcStr[srcStrLenTemp - 1]), maxMemLength - (srcStrLenTemp - 1), insertStr,
                                 insertStrLen);
            TAF_MEM_CHK_RTN_VAL(memResult, maxMemLength - (srcStrLenTemp - 1), insertStrLen);
        }

        srcStrLenTemp += insertStrLen;

        srcStr[srcStrLenTemp - 1] = ';';
    } else {
        if ((insertStrLen > 0) && (maxMemLength > srcStrLenTemp)) {
            memResult = memcpy_s(&(srcStr[srcStrLenTemp]), maxMemLength - srcStrLenTemp, insertStr, insertStrLen);
            TAF_MEM_CHK_RTN_VAL(memResult, maxMemLength - srcStrLenTemp, insertStrLen);
        }

        srcStrLenTemp += insertStrLen;
    }

    *srcStrLen = srcStrLenTemp;
}

LOCAL VOS_UINT32 AT_GetCmdGIExistFlag(VOS_UINT8 str, VOS_BOOL *bIExist, VOS_BOOL *biExist,
    VOS_BOOL *bGExist, VOS_BOOL *bgExist)
{
    if (str == 'I') {
        *bIExist = VOS_TRUE;
        *biExist = VOS_FALSE;
        return VOS_TRUE;
    }

    if (str == 'i') {
        *biExist = VOS_TRUE;
        *bIExist = VOS_FALSE;
        return VOS_TRUE;
    }

    if (str == 'G') {
        *bGExist = VOS_TRUE;
        return VOS_TRUE;
    }

    if (str == 'g') {
        *bgExist = VOS_TRUE;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_ProcDCmdGIPara(VOS_UINT32 *srcStrLen, VOS_UINT8 *srcStr)
{
    VOS_UINT32 loop;
    VOS_BOOL   bIExist = VOS_FALSE;
    VOS_BOOL   biExist = VOS_FALSE;
    VOS_BOOL   bGExist = VOS_FALSE;
    VOS_BOOL   bgExist = VOS_FALSE;
    VOS_UINT8  insertStr[AT_CALL_D_GI_PARA_LEN];
    VOS_UINT32 insertStrLen;
    VOS_UINT32 pos;
    VOS_UINT32 dstStrLen;
    VOS_UINT8 *dstPara = VOS_NULL_PTR;
    VOS_UINT32 maxMemLength;
    errno_t    returnValue;

    dstStrLen    = *srcStrLen;
    maxMemLength = *srcStrLen;

    if (dstStrLen == 0) {
        AT_ERR_LOG("dstPara cannot malloc space !");
        return;
    }
    dstPara      = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, dstStrLen);
    if (dstPara == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_ProcDCmdGIPara : fail to alloc memory . ");
        return;
    }
    (VOS_VOID)memset_s(dstPara, dstStrLen, 0x00, dstStrLen);

    /*
     * 获取D命令的GI参数属性，如果存在冲突的I和i字符以最后一个字符属性为准，
     * 删除字符串中的属性字符GgIi
     */
    for (loop = 0, pos = 0; loop < dstStrLen; loop++) {

        if (AT_GetCmdGIExistFlag(srcStr[loop], &bIExist, &biExist, &bGExist, &bgExist) == VOS_TRUE) {
            continue;
        }

        *(dstPara + pos) = *(srcStr + loop);

        pos++;
    }

    dstStrLen = pos;

    /* 更新修改到用户拨号字符串 */
    *srcStrLen = dstStrLen;
    if (dstStrLen > 0) {
        returnValue = memmove_s(srcStr, dstStrLen, dstPara, dstStrLen);
        TAF_MEM_CHK_RTN_VAL(returnValue, dstStrLen, dstStrLen);
    }

    PS_MEM_FREE(WUEPS_PID_AT, dstPara);

    /* 生成GI属性字符串 */
    (VOS_VOID)memset_s(insertStr, sizeof(insertStr), 0x00, sizeof(insertStr));

    insertStrLen = 0;
    if (bIExist == VOS_TRUE) {
        insertStr[insertStrLen] = 'I';
        insertStrLen++;
    } else {
        if (biExist == VOS_TRUE) {
            insertStr[insertStrLen] = 'i';
            insertStrLen++;
        }
    }

    if ((bGExist == VOS_TRUE) || (bgExist == VOS_TRUE)) {
        insertStr[insertStrLen] = 'G';
        insertStrLen++;
    }

    /* 将GI属性字符串插入到被叫号码和分号字符之间 */
    AT_InsertDCmdGIPara(srcStrLen, srcStr, maxMemLength, insertStrLen, insertStr);
}

VOS_VOID At_FilterDCmdSpecCharacter(VOS_UINT32 *paraLen, VOS_UINT8 *para)
{
    VOS_UINT32 i;
    VOS_UINT16 beginPos;
    VOS_UINT32 ret;
    errno_t    returnValue;

    if (*paraLen <= AT_ATD_CMD_MIN_LENGTH) {
        return;
    }

    /* 如果不是ATD命令，不处理返回 */
    /* 不为'a'或者'A'时返回 */
    if (At_CheckCharA(para[0]) != AT_SUCCESS) {
        return;
    }
    /* 不为't'或者'T'时返回 */
    if (At_CheckCharT(para[1]) != AT_SUCCESS) {
        return;
    }
    /* 不为'D'或者'd'时返回 */
    if (At_CheckCharD(para[AT_ATD_CMD_MIN_LENGTH - 1]) != AT_SUCCESS) {
        return;
    }

    beginPos = AT_ATD_CMD_MIN_LENGTH;
    /* ISDN拨号的ATDI<STRING>与ATD<STRING>相同，删除I字符保持处理函数的一致性 */
    if (para[beginPos] == 'I') {
        *paraLen = *paraLen - 1;
        if (*paraLen > beginPos) {
            returnValue = memmove_s(&(para[beginPos]), *paraLen - beginPos, &(para[beginPos + 1]), *paraLen - beginPos);
            TAF_MEM_CHK_RTN_VAL(returnValue, *paraLen - beginPos, *paraLen - beginPos);
        }
    }

    /* 第一个字符为'+'时作为国际号码标示,不能过滤 */
    if (para[beginPos] == '+') {
        beginPos++;
    }

    /* 因为ATD命令的最后一个字符一定是字符串结束符'\0',所以在下面的移位过程中不考虑取值是否越界 */
    for (i = beginPos; i < *paraLen; i++) {
        ret = AT_IsDCmdValidChar(para[i]);
        if (ret == VOS_TRUE) {
            continue;
        }

        /* 最后一个数字是';'时，跳过 */
        if (i == (*paraLen - 1)) {
            if (para[i] == ';') {
                continue;
            }
        }

        *paraLen = *paraLen - 1;
        if (*paraLen > i) {
            returnValue = memmove_s(&(para[i]), *paraLen - i, &(para[i + 1]), *paraLen - i);
            TAF_MEM_CHK_RTN_VAL(returnValue, *paraLen - i, *paraLen - i);
        }
        i--;
    }

    AT_ProcDCmdGIPara(paraLen, para);
}

VOS_UINT32 At_CombineCmdProc(VOS_UINT8 clientId)
{
    VOS_UINT32       i        = 0;
    HI_LIST_S       *me       = NULL;
    HI_LIST_S       *listHead = NULL;
    VOS_UINT32       ret;
    AT_ParseContext *clientContext = NULL;

    AT_FW_CombineCmd     *combCmd        = NULL;
    AT_FW_CombineCmdNode *combCmdNode    = NULL;
    AT_FW_CombineCmdInfo *combineCmdInfo = NULL;

    AT_RreturnCodeUint32 result;

    /* 该函数调用处可保证ucClientId的合法性，pDataIn不为空 */

    clientContext = &(g_parseContext[clientId]);

    combineCmdInfo = &clientContext->combineCmdInfo;
    listHead       = &combineCmdInfo->combineCmdList;

    /* 判断合法性 */
    if ((combineCmdInfo->totalNum == 0) || (combineCmdInfo->totalNum <= combineCmdInfo->procNum)) {
        return AT_ERROR;
    }

    /* 在组合命令链表中查找待处理的命令 */
    msp_list_for_each(me, listHead)
    {
        if (i < combineCmdInfo->procNum) {
            i++;
            continue;
        }

        combCmdNode = msp_list_entry(me, AT_FW_CombineCmdNode, combCmdList);
        combCmd     = combCmdNode->combCmd;
        break;
    }

    /* 没有找到待处理的命令 */
    if ((i != combineCmdInfo->procNum) || (combCmd == NULL)) {
        return AT_ERROR;
    }

    /*
     * 根据协议《T-REC-V[1].250-200307-I_MSW-E.doc》
     * 6.3.1	Dial
     *     Any characters appearing in the dial string that the DCE does not recognize
     *     as a valid part of the call addressing information or as a valid modifier
     *     shall be ignored. This permits characters such as parentheses and hyphens
     *     to be included that are typically used in formatting of telephone numbers.
     *   所以把特殊字符过滤掉
     */

    At_FilterDCmdSpecCharacter(&(combCmd->len), combCmd->data);

    /* 解析命令字符串 */
    result = (AT_RreturnCodeUint32)AT_ParseCmdType(combCmd->data, (VOS_UINT16)combCmd->len);

    /* 解析出错，返回错误码 */
    if (result != ERR_MSP_SUCCESS) {
        if (atMatchCmdName(clientId, g_atParseCmd.cmdFmtType) == ERR_MSP_SUCCESS) {
            if (g_parseContext[clientId].cmdElement != NULL) {
                return g_parseContext[clientId].cmdElement->paramErrCode;
            }
        }

        return result;
    }

    AT_MntnSaveCurCmdName(clientId);

    /* 命令处理 */
    ret = CmdParseProc(clientId, combCmd->data, (VOS_UINT16)combCmd->len);
    if ((ret != AT_OK) && (ret != AT_WAIT_ASYNC_RETURN)) {
        At_ResetCombinCmdInfo(listHead);
    }

    return ret;
}

VOS_UINT32 At_CombineCmdChkProc(VOS_UINT8 clientId, VOS_UINT8 *dataIn, VOS_UINT16 lenIn)
{
    VOS_UINT16 len  = lenIn;
    VOS_UINT8 *data = dataIn;
    VOS_UINT32 ret;

    AT_ParseContext *clientContext = NULL;
    HI_LIST_S       *listHead      = NULL;

    /* 该函数调用处可保证ucClientId的合法性，pDataIn不为空 */

    clientContext = &(g_parseContext[clientId]);
    listHead      = &clientContext->combineCmdInfo.combineCmdList;

    /* 前两个字符不是AT，直接返回失败 */
    if (At_CheckCharA(data[0]) != AT_SUCCESS) {
        return AT_ERROR;
    }

    if (At_CheckCharT(data[1]) != AT_SUCCESS) {
        return AT_ERROR;
    }

    /* "AT" or "AT;" */
    if (lenIn == AT_FW_LEN_AT) {
        return AT_OK;
    }

    /* 第三个字符是分号 */
    if (lenIn == AT_FW_LEN_AT_ADD_SEMI) {
        if (At_CheckSemicolon(data[AT_CMD_AT_INSIDE_SYMBOL_SEMI]) == AT_SUCCESS) {
            return AT_OK;
        }
    }

    /* 过滤前两个"AT"字符，后面统一补充 */
    data = data + AT_FW_LEN_AT;
    len  = len - AT_FW_LEN_AT;

    /* 组合命令的预处理，把命令拆分放入缓存中 */
    ret = At_CombineCmdPreProc(listHead, data, len);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_ERROR;
    }

    ret = At_CombineCmdProc(clientId);

    return ret;
}

