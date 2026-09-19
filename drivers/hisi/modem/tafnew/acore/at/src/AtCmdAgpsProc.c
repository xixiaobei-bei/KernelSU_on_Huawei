/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtCmdAgpsProc.h"
#include "ATCmdProc.h"
#include "AtCmdAgpsProc.h"
#include "AtMsgPrint.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_AGPS_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_UINT32 At_SetEpduPara(VOS_UINT8 ucIndex)
{
    AT_MTA_EPDU_SET_REQ_STRU            stAtCmd;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usEpduLen;

    /* 参数检查 */
    if (AT_CMD_EPDU_PARA_NUM != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_MTA_EPDU_NAME_LENGTH_MAX < gastAtParaList[7].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_MTA_EPDU_CONTENT_STRING_LEN_MAX < gastAtParaList[10].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* gastAtParaList[8].ulParaValue表示total值； gastAtParaList[9].ulParaValue表示index值
       如果total值小于index值直接返回
    */
    if (gastAtParaList[8].ulParaValue < gastAtParaList[9].ulParaValue)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usEpduLen = 0;

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_EPDU_SET_REQ_STRU));
    stAtCmd.usTransactionId     = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stAtCmd.ucMsgBodyType       = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stAtCmd.ucCommonIEValidFlg  = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    stAtCmd.ucEndFlag           = (VOS_UINT8)gastAtParaList[3].ulParaValue;
    stAtCmd.ucLocSource         = (VOS_UINT8)gastAtParaList[4].ulParaValue;
    stAtCmd.ucLocCalcErr        = (VOS_UINT8)gastAtParaList[5].ulParaValue;
    stAtCmd.ucId                = (VOS_UINT8)gastAtParaList[6].ulParaValue;
    stAtCmd.ucTotalNum          = (VOS_UINT8)gastAtParaList[8].ulParaValue;
    stAtCmd.ucIndex             = (VOS_UINT8)gastAtParaList[9].ulParaValue;

    if (0 != gastAtParaList[7].usParaLen)
    {
        stAtCmd.ucNameLength    = (VOS_UINT8)gastAtParaList[7].usParaLen;
        TAF_MEM_CPY_S(stAtCmd.aucName, AT_MTA_EPDU_NAME_LENGTH_MAX, gastAtParaList[7].aucPara, gastAtParaList[7].usParaLen);
    }

    if (0 != gastAtParaList[10].usParaLen)
    {
        usEpduLen = gastAtParaList[10].usParaLen;

        if (AT_FAILURE == At_AsciiNum2HexString(gastAtParaList[10].aucPara, &usEpduLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stAtCmd.usEpduLength = usEpduLen;
        TAF_MEM_CPY_S(stAtCmd.aucEpduContent, AT_MTA_EPDU_CONTENT_LENGTH_MAX, gastAtParaList[10].aucPara, usEpduLen);
    }

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_EPDU_SET_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_EPDU_SET_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EPDU_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}

VOS_UINT32 AT_RcvMtaEpduDataInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                          *pucName                 = VOS_NULL_PTR;
    AT_MTA_MSG_STRU                    *pstRcvMsg               = VOS_NULL_PTR;
    MTA_AT_EPDU_DATA_IND_STRU          *pstMtaAtEpduDataInd     = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* 初始化 */
    pstRcvMsg               = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtEpduDataInd     = (MTA_AT_EPDU_DATA_IND_STRU *)pstRcvMsg->aucContent;
    ucIndex                 = 0;
    usLength                = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvMtaEpduDataInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 打印^EPDUR: */
    /* transaction_id, msg_type, common_info_valid_flg, end_flag, id*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d,%d,%d,%d,",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_EPDUR].pucText,
                                       pstMtaAtEpduDataInd->usTransactionID,
                                       pstMtaAtEpduDataInd->ucMsgBodyType,
                                       pstMtaAtEpduDataInd->ucCommonIEValidFlg,
                                       pstMtaAtEpduDataInd->ucEndFlg,
                                       pstMtaAtEpduDataInd->ucID
                                       );

    /* name */
    if (0 < pstMtaAtEpduDataInd->ucNameLength)
    {
        pucName = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, pstMtaAtEpduDataInd->ucNameLength + 1);

        /* 分配内存失败，直接返回 */
        if (VOS_NULL_PTR == pucName)
        {
            AT_ERR_LOG("AT_RcvMtaEpduDataInd: name, Alloc mem fail");

            return VOS_ERR;
        }

        TAF_MEM_CPY_S(pucName, pstMtaAtEpduDataInd->ucNameLength, pstMtaAtEpduDataInd->aucName, pstMtaAtEpduDataInd->ucNameLength);

        pucName[pstMtaAtEpduDataInd->ucNameLength] = '\0';

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"%s\"",
                                       pucName
                                       );

        PS_MEM_FREE(WUEPS_PID_AT, pucName);
    }

    /* total, index */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                   ",%d,%d,",
                                   pstMtaAtEpduDataInd->ucTotal,
                                   pstMtaAtEpduDataInd->ucIndex
                                   );

    for (i = 0; i < pstMtaAtEpduDataInd->usDataLength; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%02X",
                                           pstMtaAtEpduDataInd->aucData[i]);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "%s",
                                  gaucAtCrLf
                                  );

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaEpduSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_EPDU_SET_CNF_STRU           *pstEpduSetCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化 */
    pstRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstEpduSetCnf         = (MTA_AT_EPDU_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex               = 0;
    ulResult              = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEpduSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEpduSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EPDU_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaEpduSetCnf : Current Option is not AT_CMD_EPDU_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstEpduSetCnf->enResult)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}


