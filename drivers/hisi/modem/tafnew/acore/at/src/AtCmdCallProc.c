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
#include "AtCmdCallProc.h"

#include "AtSndMsg.h"
#include "ATCmdProc.h"
#include "TafCcmApi.h"
#include "AtTafAgentInterface.h"
#include "AppVcApi.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_CALL_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_UINT32 At_RcvVcMsgDtmfDecoderIndProc(
    MN_AT_IND_EVT_STRU                 *pstData
)
{
    APP_VC_DTMF_DECODER_IND_STRU       *pstDtmfInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_CHAR                            aucOutput[2];

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstData->clientId, &ucIndex))
    {
        AT_WARN_LOG("At_RcvVcMsgDtmfDecoderIndProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 初始化 */
    pstDtmfInd = (APP_VC_DTMF_DECODER_IND_STRU *)pstData->aucContent;
    aucOutput[0] = pstDtmfInd->ucDtmfCode;
    aucOutput[1] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DDTMF: %s%s",
                                                    gaucAtCrLf,
                                                    aucOutput,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

VOS_UINT32 AT_CheckCfshNumber(
    VOS_UINT8                          *pucAtPara,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16      ucLoop;

    /* 号码长度有效性判断:+号开头的国际号码，最大长度不能大于33；否则不能大于32 */
    if ('+' == pucAtPara[0])
    {
        if (usLen > (TAF_CALL_MAX_FLASH_DIGIT_LEN + 1))
        {
            return VOS_ERR;
        }

        pucAtPara++;
        usLen--;
    }
    else
    {
        if (usLen > TAF_CALL_MAX_FLASH_DIGIT_LEN)
        {
            return VOS_ERR;
        }
    }

    /* 号码字符有效性判断(不包含国际号码的首字符'+') */
    for (ucLoop = 0; ucLoop < usLen; ucLoop++)
    {
        if (  ((pucAtPara[ucLoop] >= '0') && (pucAtPara[ucLoop] <= '9'))
            || (pucAtPara[ucLoop] == '*')
            || (pucAtPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetCfshPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    TAF_MEM_SET_S(&stFlashPara, sizeof(stFlashPara), 0x00, sizeof(stFlashPara));

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 若携带了参数<number>，检查其有效性 */
    if (1 == gucAtParaIndex)
    {
        if (VOS_OK != AT_CheckCfshNumber(gastAtParaList[0].aucPara,
                                         gastAtParaList[0].usParaLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        /* 这种AT命令AT^CFSH= 返回参数错误 */
        if(AT_CMD_OPT_SET_PARA_CMD == g_stATParseCmd.ucCmdOptType)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    TAF_MEM_SET_S(&stFlashPara, sizeof(stFlashPara), 0x00, sizeof(TAF_CALL_FLASH_PARA_STRU));

    stFlashPara.ucDigitNum = (VOS_UINT8)gastAtParaList[0].usParaLen;
    TAF_MEM_CPY_S(stFlashPara.aucDigit, sizeof(stFlashPara.aucDigit), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_FLASH_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stFlashPara,
                                  ID_TAF_CCM_SEND_FLASH_REQ,
                                  sizeof(stFlashPara),
                                  enModemId);

    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CFSH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_RcvTafCcmSndFlashRslt(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_FLASH_CNF_STRU        *pstSndFlashRslt = VOS_NULL_PTR;

    pstSndFlashRslt = (TAF_CCM_SEND_FLASH_CNF_STRU*)pMsg;

    /* 根据ClientID获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstSndFlashRslt->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT模块在等待^CFSH命令的操作结果事件上报 */
    if (AT_CMD_CFSH_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Error Option!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 根据临时响应的错误码打印命令的结果 */
    if (VOS_OK == pstSndFlashRslt->stResult.ucResult)
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16      usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CBURSTDTMF: (1,2),(0-9,*,#),(95,150,200,250,300,350),(60,100,150,200)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    VOS_UINT16                          ucLoop;

    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDTMFPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    TAF_MEM_SET_S(&stBurstDTMFPara, sizeof(stBurstDTMFPara), 0x00, sizeof(stBurstDTMFPara));

    /*参数有效性检查*/
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((gucAtParaIndex != 4)
        ||(0 == gastAtParaList[0].usParaLen)
        ||(0 == gastAtParaList[1].usParaLen)
        ||(0 == gastAtParaList[2].usParaLen)
        ||(0 == gastAtParaList[3].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key长度有效性判断 */
    if (gastAtParaList[1].usParaLen > TAF_CALL_MAX_BURST_DTMF_NUM)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key有效性判断 */
    for (ucLoop = 0; ucLoop < gastAtParaList[1].usParaLen; ucLoop++)
    {
        if (  ((gastAtParaList[1].aucPara[ucLoop] >= '0') && (gastAtParaList[1].aucPara[ucLoop] <= '9'))
            || (gastAtParaList[1].aucPara[ucLoop] == '*')
            || (gastAtParaList[1].aucPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }


    stBurstDTMFPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stBurstDTMFPara.ucDigitNum   = (VOS_UINT8)gastAtParaList[1].usParaLen;
    TAF_MEM_CPY_S(stBurstDTMFPara.aucDigit, sizeof(stBurstDTMFPara.aucDigit), gastAtParaList[1].aucPara, stBurstDTMFPara.ucDigitNum);
    stBurstDTMFPara.ulOnLength   = gastAtParaList[2].ulParaValue;
    stBurstDTMFPara.ulOffLength  = gastAtParaList[3].ulParaValue;

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_BURST_DTMF_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stBurstDTMFPara,
                                  ID_TAF_CCM_SEND_BURST_DTMF_REQ,
                                  sizeof(stBurstDTMFPara),
                                  enModemId);

    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CBURSTDTMF_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RcvTafCcmSndBurstDTMFCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_BURST_DTMF_CNF_STRU   *pstBurstDtmfCnf = VOS_NULL_PTR;

    pstBurstDtmfCnf = (TAF_CCM_SEND_BURST_DTMF_CNF_STRU*)pMsg;

    /* 根据ClientID获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstBurstDtmfCnf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT模块在等待^CBURSTDTMF命令命令的操作结果事件上报 */
    if ( AT_CMD_CBURSTDTMF_SET != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 根据临时响应的错误码打印命令的结果 */
    if (TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS != pstBurstDtmfCnf->stBurstDtmfCnfPara.enResult)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSndBurstDTMFRslt(VOS_VOID *pMsg)
{
    return VOS_OK;
}



VOS_UINT32 AT_RcvTafCcmCalledNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALLED_NUM_INFO_IND_STRU   *pstCalledNum = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM + 1];

    pstCalledNum = (TAF_CCM_CALLED_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstCalledNum->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCalledNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }


    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCalledNum->stCalledNumInfoPara.aucDigit, pstCalledNum->stCalledNumInfoPara.ucDigitNum);

    /* 在pstCalledNum->aucDigit的最后一位加'\0',防止因pstCalledNum->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCalledNum->stCalledNumInfoPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLEDNUM].pucText,
                                                    pstCalledNum->stCalledNumInfoPara.enNumType,
                                                    pstCalledNum->stCalledNumInfoPara.enNumPlan,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallingNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALLING_NUM_INFO_IND_STRU                      *pstCallingNum = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];

    pstCallingNum = (TAF_CCM_CALLING_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstCallingNum->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCallingNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));

    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCallingNum->stCallIngNumInfoPara.aucDigit, pstCallingNum->stCallIngNumInfoPara.ucDigitNum);

    /* 在pstCallingNum->aucDigit的最后一位加'\0',防止因pstCallingNum->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCallingNum->stCallIngNumInfoPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLINGNUM].pucText,
                                                    pstCallingNum->stCallIngNumInfoPara.enNumType,
                                                    pstCallingNum->stCallIngNumInfoPara.enNumPlan,
                                                    pstCallingNum->stCallIngNumInfoPara.ucPi,
                                                    pstCallingNum->stCallIngNumInfoPara.ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmDispInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_DISPLAY_INFO_IND_STRU       *pstDisplayInfo = VOS_NULL_PTR;
    VOS_UINT8                            ucIndex;
    VOS_UINT8                            aucDigit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM + 1];

    pstDisplayInfo = (TAF_CCM_DISPLAY_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstDisplayInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstDisplayInfo->stDisPlayInfoIndPara.aucDigit, pstDisplayInfo->stDisPlayInfoIndPara.ucDigitNum);

    /* 在pstDisplayInfo->aucDigit的最后一位加'\0',防止因pstDisplayInfo->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstDisplayInfo->stDisPlayInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%s,,,%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CDISP].pucText,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmExtDispInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_EXT_DISPLAY_INFO_IND_STRU                      *pstExtDispInfo = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulDigitNum;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM + 1];

    pstExtDispInfo = (TAF_CCM_EXT_DISPLAY_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstExtDispInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallExtDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    for (ulLoop = 0; ulLoop < pstExtDispInfo->stDisPlayInfoIndPara.ucInfoRecsDataNum; ulLoop++)
    {
        /* 在pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit的最后一位加'\0',
            防止因pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit无结束符，导致AT多上报 */
        ulDigitNum = pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].ucDigitNum;
        TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
        TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].aucDigit, ulDigitNum);
        aucDigit[ulDigitNum] = '\0';

        /* 输出查询结果 */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s%s%s,%d,%d,%d%s",
                                                        gaucAtCrLf,
                                                        gastAtStringTab[AT_STRING_CDISP].pucText,
                                                        aucDigit,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.ucExtDispInd,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.ucDisplayType,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].ucDispalyTag,
                                                        gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }



    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmConnNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CONN_NUM_INFO_IND_STRU      *pstConnNumInfo = VOS_NULL_PTR;
    VOS_UINT8                            ucIndex;
    VOS_UINT8                            aucDigit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM + 1];

    pstConnNumInfo = (TAF_CCM_CONN_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstConnNumInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmConnNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstConnNumInfo->stConnNumInfoIndPara.aucDigit, pstConnNumInfo->stConnNumInfoIndPara.ucDigitNum);

    /* 在pstConnNumInfo->aucDigit的最后一位加'\0',防止因pstConnNumInfo->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstConnNumInfo->stConnNumInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCONNNUM].pucText,
                                                    pstConnNumInfo->stConnNumInfoIndPara.enNumType,
                                                    pstConnNumInfo->stConnNumInfoIndPara.enNumPlan,
                                                    pstConnNumInfo->stConnNumInfoIndPara.ucPi,
                                                    pstConnNumInfo->stConnNumInfoIndPara.ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRedirNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_REDIR_NUM_INFO_IND_STRU                        *pstRedirNumInfo = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM + 1];

    usLength        = 0;
    pstRedirNumInfo = (TAF_CCM_REDIR_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstRedirNumInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmRedirNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstRedirNumInfo->stRedirNumInfoIndPara.aucDigitNum, pstRedirNumInfo->stRedirNumInfoIndPara.ucDigitNum);

    /* 在pstRedirNumInfo->aucDigitNum的最后一位加'\0',防止因pstRedirNumInfo->aucDigitNum无结束符，导致AT多上报 */
    aucDigit[pstRedirNumInfo->stRedirNumInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果，根据EXTENSIONBIT1，EXTENSIONBIT2输出可选项 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d,%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_CREDIRNUM].pucText,
                                       pstRedirNumInfo->stRedirNumInfoIndPara.enNumType,
                                       pstRedirNumInfo->stRedirNumInfoIndPara.enNumPlan,
                                       aucDigit);

    if (VOS_TRUE == pstRedirNumInfo->stRedirNumInfoIndPara.bitOpPi)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucPi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstRedirNumInfo->stRedirNumInfoIndPara.bitOpSi)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucSi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstRedirNumInfo->stRedirNumInfoIndPara.bitOpRedirReason)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucRedirReason);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSignalInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_SIGNAL_INFO_IND_STRU        *pstsignalInfo = VOS_NULL_PTR;
    VOS_UINT8                            ucIndex;

    pstsignalInfo = (TAF_CCM_SIGNAL_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstsignalInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmSignalInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CSIGTONE].pucText,
                                                    pstsignalInfo->stSignalInfoIndPara.ucSignalType,
                                                    pstsignalInfo->stSignalInfoIndPara.ucAlertPitch,
                                                    pstsignalInfo->stSignalInfoIndPara.ucSignal,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmLineCtrlInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_LINE_CTRL_INFO_IND_STRU                        *pstLineCtrlInfo = VOS_NULL_PTR;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               ucIndex;

    usLength = 0;
    pstLineCtrlInfo = (TAF_CCM_LINE_CTRL_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstLineCtrlInfo->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmLineCtrlInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 输出查询结果 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                    (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%s%s%d",
                                    gaucAtCrLf,
                                    gastAtStringTab[AT_STRING_CLCTR].pucText,
                                    pstLineCtrlInfo->stLineCtrlInfoIndPara.ucPolarityIncluded);

    if (VOS_TRUE == pstLineCtrlInfo->stLineCtrlInfoIndPara.ucToggleModePresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucToggleMode);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstLineCtrlInfo->stLineCtrlInfoIndPara.ucReversePolarityPresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucReversePolarity);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d%s",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucPowerDenialTime,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCCWACInd(VOS_VOID *pMsg)
{
    TAF_CCM_CCWAC_INFO_IND_STRU        *pstCCWAC = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];

    usLength = 0;
    pstCCWAC = (TAF_CCM_CCWAC_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(AT_FAILURE == At_ClientIdToUserId(pstCCWAC->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCCWACInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCCWAC->stCcwacInfoPara.aucDigit, pstCCWAC->stCcwacInfoPara.ucDigitNum);

    /* 在pstCCWAC->aucDigit的最后一位加'\0',防止因pstCCWAC->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCCWAC->stCcwacInfoPara.ucDigitNum] = '\0';


    /* 输出查询结果 */
    if (VOS_TRUE == pstCCWAC->stCcwacInfoPara.ucSignalIsPresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->stCcwacInfoPara.ucPi,
                                            pstCCWAC->stCcwacInfoPara.ucSi,
                                            pstCCWAC->stCcwacInfoPara.enNumType,
                                            pstCCWAC->stCcwacInfoPara.enNumPlan,
                                            pstCCWAC->stCcwacInfoPara.ucSignalIsPresent,
                                            pstCCWAC->stCcwacInfoPara.ucSignalType,
                                            pstCCWAC->stCcwacInfoPara.ucAlertPitch,
                                            pstCCWAC->stCcwacInfoPara.ucSignal,
                                            gaucAtCrLf);

    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,,,%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->stCcwacInfoPara.ucPi,
                                            pstCCWAC->stCcwacInfoPara.ucSi,
                                            pstCCWAC->stCcwacInfoPara.enNumType,
                                            pstCCWAC->stCcwacInfoPara.enNumPlan,
                                            pstCCWAC->stCcwacInfoPara.ucSignalIsPresent,
                                            gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 At_TestCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CCONTDTMF: (1,2),(0,1),(0-9,*,#)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDTMFPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    TAF_MEM_SET_S(&stContDTMFPara, sizeof(stContDTMFPara), 0x00, sizeof(stContDTMFPara));


    /* Check the validity of parameter */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Non set command!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  Check the validity of <Call_ID> and <Switch> */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Call_ID> or <Switch>!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start and the number of parameter isn't equal to 3.
       Or if the <Switch> is Stop and the number of parameter isn't equal to 2，both invalid */
    if (((TAF_CALL_CONT_DTMF_STOP == gastAtParaList[1].ulParaValue)
      && (AT_CCONTDTMF_PARA_NUM_MIN != gucAtParaIndex))
     || ((TAF_CALL_CONT_DTMF_START == gastAtParaList[1].ulParaValue)
      && (AT_CCONTDTMF_PARA_NUM_MAX != gucAtParaIndex)))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: The number of parameters mismatch!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start,the <Dtmf_Key> should be setted and check its validity */
    if (TAF_CALL_CONT_DTMF_START == gastAtParaList[1].ulParaValue)
    {
        if (VOS_ERR == AT_CheckCContDtmfKeyPara())
        {
            AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Dtmf_Key>!");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    stContDTMFPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stContDTMFPara.enSwitch     = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stContDTMFPara.ucDigit      = (VOS_UINT8)gastAtParaList[2].aucPara[0];

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_CONT_DTMF_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stContDTMFPara,
                                  ID_TAF_CCM_SEND_CONT_DTMF_REQ,
                                  sizeof(stContDTMFPara),
                                  enModemId);

    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCONTDTMF_SET;

        /* Return hang-up state */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID)
{
    if (1 != gastAtParaList[2].usParaLen)
    {
        return VOS_ERR;
    }

    if ((('0' <= gastAtParaList[2].aucPara[0]) && ('9' >= gastAtParaList[2].aucPara[0]))
      || ('*' == gastAtParaList[2].aucPara[0])
      || ('#' == gastAtParaList[2].aucPara[0]))
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_CONT_DTMF_CNF_STRU    *pstContDtmfCnf = VOS_NULL_PTR;

    pstContDtmfCnf = (TAF_CCM_SEND_CONT_DTMF_CNF_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pstContDtmfCnf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT module is waiting for report the result of ^CCONTDTMF command */
    if (AT_CMD_CCONTDTMF_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* Use AT_STOP_TIMER_CMD_READY to recover the AT command state to READY state */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* According to the error code of temporary respond, printf the result of command */
    if (TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS != pstContDtmfCnf->stContDtmfCnfPara.enResult)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFRslt(VOS_VOID *pMsg)
{
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRcvContinuousDtmfInd(VOS_VOID *pMsg)
{
    TAF_CCM_CONT_DTMF_IND_STRU         *pstRcvContDtmf = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[2];
    VOS_UINT8                           ucIndex;

    pstRcvContDtmf = (TAF_CCM_CONT_DTMF_IND_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pstRcvContDtmf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallRcvContinuousDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /*  Initialize aucDigit[0] with pstRcvContDtmf->ucDigit and  aucDigit[1] = '\0'
        Because At_sprintf does not allow to print pstRcvContDtmf->ucDigit with %c
        Hence, need to convert digit into string and print as string */
    aucDigit[0] = pstRcvContDtmf->stContDtmfIndPara.ucDigit;
    aucDigit[1] = '\0';

    /* Output the inquire result */
    if (TAF_CALL_CONT_DTMF_START == pstRcvContDtmf->stContDtmfIndPara.enSwitch)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d,\"%s\"%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->stCtrl.callId,
                                                        pstRcvContDtmf->stContDtmfIndPara.enSwitch,
                                                        aucDigit,
                                                        gaucAtCrLf);
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->stCtrl.callId,
                                                        pstRcvContDtmf->stContDtmfIndPara.enSwitch,
                                                        gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRcvBurstDtmfInd(VOS_VOID *pMsg)
{
    TAF_CCM_BURST_DTMF_IND_STRU        *pstRcvBurstDtmf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM + 1];

    pstRcvBurstDtmf = (TAF_CCM_BURST_DTMF_IND_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pstRcvBurstDtmf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallRcvBurstDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* initialization */
    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstRcvBurstDtmf->stBurstDtmfIndPara.aucDigit, pstRcvBurstDtmf->stBurstDtmfIndPara.ucDigitNum);

    /* Add the '\0' to the last byte of pstRcvBurstDtmf->aucDigit */
    aucDigit[pstRcvBurstDtmf->stBurstDtmfIndPara.ucDigitNum] = '\0';

    /* Output the inquire result */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^CBURSTDTMF: %d,\"%s\",%d,%d%s",
                                                    gaucAtCrLf,
                                                    pstRcvBurstDtmf->stCtrl.callId,
                                                    aucDigit,
                                                    pstRcvBurstDtmf->stBurstDtmfIndPara.ulOnLength,
                                                    pstRcvBurstDtmf->stBurstDtmfIndPara.ulOffLength,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

VOS_UINT32 AT_TestCclprPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-2)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetCclprPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_CLPR_PARA_STRU         stQryClprPara;
    MODEM_ID_ENUM_UINT16                enModemId;


    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    TAF_MEM_SET_S(&stQryClprPara, sizeof(stQryClprPara), 0x00, sizeof(stQryClprPara));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    stQryClprPara.callId                = (MN_CALL_ID_T)gastAtParaList[0].ulParaValue;
    stQryClprPara.enQryClprModeType     = TAF_CALL_QRY_CLPR_MODE_C;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_QRY_CLPR_REQ到C核 */
    ulResult = TAF_CCM_CallCommonReq(&stCtrl,
                                     (void *)&stQryClprPara,
                                     ID_TAF_CCM_QRY_CLPR_REQ,
                                     sizeof(stQryClprPara),
                                     enModemId);

    if (VOS_OK != ulResult)
    {
        AT_WARN_LOG("AT_SetCclprPara: TAF_XCALL_SendCclpr fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCLPR_SET;

    return AT_WAIT_ASYNC_RETURN;

}

VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;

    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SUPS_PARAM_STRU             stCallSupsPara;
    MODEM_ID_ENUM_UINT16                enModemId;

     /* 指令类型检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetRejCallPara : Current Option is not AT_CMD_REJCALL!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (2 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetRejCallPara : The number of input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        AT_WARN_LOG("AT_SetRejCallPara : Input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    TAF_MEM_SET_S(&stCallSupsPara, sizeof(stCallSupsPara), 0x00, sizeof(stCallSupsPara));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    stCallSupsPara.enCallSupsCmd  = MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING;
    stCallSupsPara.callId         = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stCallSupsPara.enCallRejCause = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    if (VOS_OK != TAF_CCM_CallCommonReq(&stCtrl,
                                      &stCallSupsPara,
                                      ID_TAF_CCM_CALL_SUPS_CMD_REQ,
                                      sizeof(stCallSupsPara),
                                      enModemId))
    {
        AT_WARN_LOG("AT_SetRejCallPara : Send Msg fail!");
        return AT_ERROR;
    }

    /* 停止自动接听 */
    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);

    if (VOS_TRUE == pstCcCtx->stS0TimeInfo.bTimerStart)
    {
        AT_StopRelTimer(pstCcCtx->stS0TimeInfo.ulTimerName, &(pstCcCtx->stS0TimeInfo.s0Timer));
        pstCcCtx->stS0TimeInfo.bTimerStart = TAF_FALSE;
        pstCcCtx->stS0TimeInfo.ulTimerName = 0;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_REJCALL_SET;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
}


VOS_UINT32 AT_TestRejCallPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_QryCimsErrPara(VOS_UINT8 ucIndex)
{
    TAF_CALL_ERROR_INFO_TEXT_STRU      *pstCallErrInfo = VOS_NULL_PTR;

    pstCallErrInfo = AT_GetCallErrInfoText(ucIndex);

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,\"%s\"",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          AT_GetCsCallErrCause(ucIndex),
                                          pstCallErrInfo->acErrInfoText);

    return AT_OK;
}


VOS_UINT32 AT_QryCsChannelInfoPara( VOS_UINT8 ucIndex )
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (VOS_OK != AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId))
    {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  VOS_NULL_PTR,
                                  ID_TAF_CCM_QRY_CHANNEL_INFO_REQ,
                                  0,
                                  enModemId);


    if (VOS_OK != ulRst)
    {
        AT_WARN_LOG("AT_QryCsChannelInfoPara: Send Msg fail!");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CSCHANNELINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}



