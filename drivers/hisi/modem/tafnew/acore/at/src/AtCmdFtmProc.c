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
#include "mdrv.h"
#include "AtCmdFtmProc.h"
#include "ATCmdProc.h"
#include "AtMtaInterface.h"

#include "msp_diag.h"

#include "AtTestParaCmd.h"
#include "TafAppMma.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_FTM_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_UINT32 At_SetLogPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulOmLogPort;    // 准备切换的LogPort

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多或没有 */
    if ((2 < gucAtParaIndex) || (0 == gucAtParaIndex))
    {
        return AT_ERROR;
    }

    if (AT_LOG_PORT_USB == gastAtParaList[0].ulParaValue)
    {
        ulOmLogPort = CPM_OM_PORT_TYPE_USB;
    }
    else
    {
        ulOmLogPort = CPM_OM_PORT_TYPE_VCOM;
    }

    /* 参数只有一个，默认永久生效 */
    if (1 == gucAtParaIndex)
    {
        gastAtParaList[1].ulParaValue = VOS_TRUE;
    }

    /* 调用OM的接口 */
    ulRslt = DIAG_LogPortSwich(ulOmLogPort, gastAtParaList[1].ulParaValue);

    if (VOS_OK == ulRslt)
    {
        return AT_OK;
    }
    else if(ERR_MSP_AT_CHANNEL_BUSY == ulRslt)
    {
        return AT_CME_USB_TO_VCOM_IN_CONN_ERROR;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_QryLogPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulOmLogPort;
    VOS_UINT32                          ulAtLogPort;
    VOS_UINT32                          ulRslt;

    usLength                            = 0;
    ulOmLogPort                         = AT_LOG_PORT_USB;

    ulRslt = mdrv_PPM_QueryLogPort(&ulOmLogPort);

    if (VOS_OK != ulRslt)
    {
        return AT_ERROR;
    }

    if (COMM_LOG_PORT_USB == ulOmLogPort)
    {
        ulAtLogPort = AT_LOG_PORT_USB;
    }
    else
    {
        ulAtLogPort = AT_LOG_PORT_VCOM;
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulAtLogPort);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

VOS_UINT32 At_QryLogCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulAtLogCfg;
   
    usLength                            = 0;
    PS_PRINTF("at^logcfg?!\n");

    (void)mdrv_socp_get_cfg_ind_mode(&ulAtLogCfg);
      
    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulAtLogCfg);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : At_QryLogCpsPara
 功能描述  : ^LOGCPS的查询函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :


*****************************************************************************/
VOS_UINT32 At_QryLogCpsPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulAtLogCps;
    usLength                            = 0;
    
    PS_PRINTF("at^logcps?!\n");
   
    (void)mdrv_socp_get_cps_ind_mode(&ulAtLogCps);

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulAtLogCps);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

VOS_UINT32 At_SetDpdtTestFlagPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU   stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU));
    stAtCmd.enRatMode = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ucFlag    = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_DPDTTEST_FLAG_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTTEST_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_SetDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode   = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ulDpdtValue = gastAtParaList[1].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}


VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_QRY_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_QRY_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMtaSetDpdtTestFlagCnf(VOS_VOID *pMsg)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                    *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU  *pstSetDpdtFlagCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化消息变量 */
    ucIndex   = 0;
    pstMtaMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstSetDpdtFlagCnf = (MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_DPDTTEST_SET */
    if (AT_CMD_DPDTTEST_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:Not AT_CMD_DPDTTEST_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 判断查询操作是否成功 */
    if (MTA_AT_RESULT_NO_ERROR == pstSetDpdtFlagCnf->enResult)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaSetDpdtValueCnf(VOS_VOID *pMsg)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                    *pstMtaMsg          = VOS_NULL_PTR;
    MTA_AT_SET_DPDT_VALUE_CNF_STRU     *pstSetDpdtValueCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化消息变量 */
    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstSetDpdtValueCnf = (MTA_AT_SET_DPDT_VALUE_CNF_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_DPDT_SET */
    if (AT_CMD_DPDT_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:Not AT_CMD_DPDT_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 判断查询操作是否成功 */
    if (MTA_AT_RESULT_NO_ERROR == pstSetDpdtValueCnf->enResult)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryDpdtValueCnf(VOS_VOID *pMsg)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                    *pstMtaMsg          = VOS_NULL_PTR;
    MTA_AT_QRY_DPDT_VALUE_CNF_STRU     *pstQryDpdtValueCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化消息变量 */
    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstQryDpdtValueCnf = (MTA_AT_QRY_DPDT_VALUE_CNF_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_DPDTQRY_SET */
    if (AT_CMD_DPDTQRY_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:Not AT_CMD_DPDTQRY_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 判断查询操作是否成功 */
    if (MTA_AT_RESULT_NO_ERROR == pstQryDpdtValueCnf->enResult)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            "^DPDTQRY: %d",
                                            pstQryDpdtValueCnf->ulDpdtValue);
    }
    else
    {
        ulResult = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvMtaSetRatFreqLockCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_SET_FREQ_LOCK_CNF_STRU      *pstSetCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化 */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_FREQ_LOCK_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_RATFREQLOCK_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : Current Option is not AT_CMD_RATFREQLOCK_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstSetCnf->enResult)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}



VOS_UINT32 AT_RcvMtaSetGFreqLockCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_SET_GSM_FREQLOCK_CNF_STRU   *pstSetCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化 */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_GSM_FREQLOCK_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_GSM_FREQLOCK_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : Current Option is not AT_CMD_GSM_FREQLOCK_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstSetCnf->enResult)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}



VOS_UINT32 AT_RcvMtaGFreqLockQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg           = VOS_NULL_PTR;
    MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU   *pstQryGFreqlockCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstQryGFreqlockCnf   = (MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_GSM_FREQLOCK_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : Current Option is not AT_CMD_GSM_FREQLOCK_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化AT^GFREQLOCK查询命令返回 */
    gstAtSendData.usBufLen = 0;
    if (VOS_OK != pstQryGFreqlockCnf->ulResult)
    {
        ulResult = AT_CME_UNKNOWN;
    }
    else
    {
        ulResult = AT_OK;

        if (VOS_FALSE == pstQryGFreqlockCnf->enLockFlg)
        {
            gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                                            (TAF_CHAR*)pgucAtSndCodeAddr,
                                                            "%s: %d",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enLockFlg);
        }
        else
        {
            gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                                            (TAF_CHAR*)pgucAtSndCodeAddr,
                                                            "%s: %d,%d,%d",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enLockFlg,
                                                            (TAF_INT32)pstQryGFreqlockCnf->usFreq,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enBand);
        }
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_VOID AT_NetMonFmtPlmnId(
    VOS_UINT32                          ulMcc,
    VOS_UINT32                          ulMnc,
    VOS_CHAR                           *pstrPlmn,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT32                          ulMaxLength;
    VOS_UINT32                          ulLength;

    ulLength                          = 0;
    ulMaxLength                       = AT_NETMON_PLMN_STRING_MAX_LENGTH;

    /* 格式输出MCC MNC */
    if (0x0f0000 == (ulMnc & 0x0f0000))
    {
        ulLength = (VOS_UINT32)VOS_nsprintf_s( (VOS_CHAR *)pstrPlmn,
                                  (VOS_UINT32)ulMaxLength,
                                  (VOS_UINT32)ulMaxLength,
                                  "%X%X%X,%X%X",
                                  (ulMcc & 0x0f),
                                  (ulMcc & 0x0f00)>>8,
                                  (ulMcc & 0x0f0000)>>16,
                                  (ulMnc & 0x0f),
                                  (ulMnc & 0x0f00)>>8);
    }
    else
    {
        ulLength = (VOS_UINT32)VOS_nsprintf_s( (VOS_CHAR *)pstrPlmn,
                                  (VOS_UINT32)ulMaxLength,
                                  (VOS_UINT32)ulMaxLength,
                                  "%X%X%X,%X%X%X",
                                  (ulMcc & 0x0f),
                                  (ulMcc & 0x0f00)>>8,
                                  (ulMcc & 0x0f0000)>>16,
                                  (ulMnc & 0x0f),
                                  (ulMnc & 0xf00)>>8,
                                  (ulMnc & 0x0f0000)>>16);
    }

    /* 长度翻转保护 */
    if (ulLength >= AT_NETMON_PLMN_STRING_MAX_LENGTH)
    {
        AT_ERR_LOG("AT_NetMonFmtPlmnId: MCC ulLength is error!");
        *(pstrPlmn + AT_NETMON_PLMN_STRING_MAX_LENGTH - 1) = 0;
        *pusLength = AT_NETMON_PLMN_STRING_MAX_LENGTH;
        return;
    }

    *pusLength = (VOS_UINT16)ulLength;

    return;
}


VOS_VOID AT_NetMonFmtGsmSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    TAF_MEM_SET_S(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->unSCellInfo.stGsmSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->unSCellInfo.stGsmSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* 格式输出PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: GSM,%s,%d,%u,%d,%X,%X",
                                        pucPlmnstr,
                                        pstSCellInfo->unSCellInfo.stGsmSCellInfo.enBand,
                                        pstSCellInfo->unSCellInfo.stGsmSCellInfo.ulArfcn,
                                        pstSCellInfo->unSCellInfo.stGsmSCellInfo.ucBsic,
                                        pstSCellInfo->unSCellInfo.stGsmSCellInfo.ulCellID,
                                        pstSCellInfo->unSCellInfo.stGsmSCellInfo.usLac );

    /* RSSI无效值，不显示 */
    if (AT_NETMON_GSM_RSSI_INVALID_VALUE == pstSCellInfo->unSCellInfo.stGsmSCellInfo.sRssi)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stGsmSCellInfo.sRssi );
    }

    /* 无效值，不显示 */
    if (AT_NETMON_GSM_RX_QUALITY_INVALID_VALUE == pstSCellInfo->unSCellInfo.stGsmSCellInfo.ucRxQuality)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stGsmSCellInfo.ucRxQuality );
    }

    /*输出TA*/
    if (PS_IE_PRESENT == pstSCellInfo->unSCellInfo.stGsmSCellInfo.bitOpTa)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stGsmSCellInfo.usTa );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtUtranFddSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    TAF_MEM_SET_S(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* 格式输出PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: WCDMA,%s",
                                        pucPlmnstr );

    /*输出频点*/
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u",
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulArfcn );

    /* PSC无效值，不显示 */
    if ((AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP)
        && (AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0))
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.usPSC );
    }

    /*输出Cell ID*/
    if (PS_IE_PRESENT == pstSCellInfo->unSCellInfo.stUtranSCellInfo.bitOpCellID)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%X",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulCellID );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /*输出LAC*/
    if (PS_IE_PRESENT == pstSCellInfo->unSCellInfo.stUtranSCellInfo.bitOpLAC)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            ",%X",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.usLAC );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /* RSCP无效值，不显示 */
    if (AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP );
    }

    /* RSSI无效值，不显示 */
    if (AT_NETMON_UTRAN_FDD_RSSI_INVALID_VALUE == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSSI)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSSI );
    }

    /* ECN0无效值，不显示 */
    if (AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0 );
    }

    /*输出DRX*/
    if (PS_IE_PRESENT == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.bitOpDRX)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.ulDrx );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /*输出URA Id*/
    if (PS_IE_PRESENT == pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.bitOpURA)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.usURA );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtGsmNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->stNCellInfo.ucGsmNCellCnt > NETMON_MAX_GSM_NCELL_NUM)
    {
        pstNCellInfo->stNCellInfo.ucGsmNCellCnt = NETMON_MAX_GSM_NCELL_NUM;
    }

    /* GSM邻区显示 */
    for (ulLoop = 0; ulLoop < pstNCellInfo->stNCellInfo.ucGsmNCellCnt; ulLoop++)
    {
        /* 如果输出的不是第一个邻区，先打印回车换行 */
        if (0 != usLength)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf );
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: GSM,%d,%u",
                                            pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].enBand,
                                            pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].ulAfrcn );

        /*输出Bsic*/
        if (PS_IE_PRESENT == pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].bitOpBsic)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",%d",
                                                pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].ucBsic );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "," );
        }

        /*输出Cell ID*/
        if (PS_IE_PRESENT == pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].bitOpCellID)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",%X",
                                                pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].ulCellID );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                                "," );
        }

         /*输出LAC*/
        if (PS_IE_PRESENT == pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].bitOpLAC)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",%X",
                                               pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].usLAC );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "," );
        }

        /*输出RSSI*/
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstNCellInfo->stNCellInfo.astGsmNCellInfo[ulLoop].sRSSI );
    }

    *pusOutLen = usLength;

    return;
}

VOS_VOID AT_NetMonFmtUtranFddNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->stNCellInfo.ucUtranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM)
    {
        pstNCellInfo->stNCellInfo.ucUtranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /*WCDMA 临区显示*/
    for (ulLoop = 0; ulLoop < pstNCellInfo->stNCellInfo.ucUtranNCellCnt; ulLoop++)
    {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (0 != usLength)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            "^MONNC: WCDMA,%u,%d,%d,%d",
                                            pstNCellInfo->stNCellInfo.u.astFddNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->stNCellInfo.u.astFddNCellInfo[ulLoop].usPSC,
                                            pstNCellInfo->stNCellInfo.u.astFddNCellInfo[ulLoop].sRSCP,
                                            pstNCellInfo->stNCellInfo.u.astFddNCellInfo[ulLoop].sECN0 );
    }

    *pusOutLen = usLength;

    return;
}


VOS_UINT32 At_SetNetMonSCellPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;

    if (AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = AT_FillAndSndAppReqMsg( gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_SET_NETMON_SCELL_REQ,
                                    VOS_NULL_PTR,
                                    0,
                                    I0_UEPS_PID_MTA );

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MONSC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetNetMonNCellPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;

    if (AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = AT_FillAndSndAppReqMsg( gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_SET_NETMON_NCELL_REQ,
                                    VOS_NULL_PTR,
                                    0,
                                    I0_UEPS_PID_MTA );

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MONNC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}

VOS_UINT32 AT_RcvMtaSetNetMonSCellCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* 初始化 */
    pstRcvMsg                         = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                         = (MTA_AT_NETMON_CELL_INFO_STRU *)pstRcvMsg->aucContent;

    usLength                          = 0;
    ucIndex                           = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MONSC_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : Current Option is not AT_CMD_JDETEX_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( (MTA_AT_RESULT_ERROR   == pstSetCnf->enResult)
      || (MTA_NETMON_SCELL_TYPE != pstSetCnf->enCellType) )
    {
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    switch(pstSetCnf->enRatType)
    {
        case MTA_AT_NETMON_CELL_INFO_GSM:
        {
            AT_NetMonFmtGsmSCellData(pstSetCnf, &usLength);
            break;
        }
        case MTA_AT_NETMON_CELL_INFO_UTRAN_FDD:
        {
            AT_NetMonFmtUtranFddSCellData(pstSetCnf, &usLength);
            break;
        }
        case MTA_AT_NETMON_CELL_INFO_UTRAN_TDD:
        {
            AT_NetMonFmtUtranTddSCellData(pstSetCnf, &usLength);
            break;
        }
        case MTA_AT_NETMON_CELL_INFO_LTE:
        {
            AT_NetMonFmtEutranSCellData(pstSetCnf, &usLength);
            break;
        }
        default:
            usLength += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "^MONSC: NONE" );
            break;
    }


    /* 输出结果 */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaSetNetMonNCellCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_UINT16                          usLengthTemp;

    /* 初始化 */
    pstRcvMsg                         = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                         = (MTA_AT_NETMON_CELL_INFO_STRU *)pstRcvMsg->aucContent;

    ucIndex                           = 0;
    usLength                          = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MONNC_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : Current Option is not AT_CMD_JDETEX_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( (MTA_AT_RESULT_ERROR   == pstSetCnf->enResult)
      || (MTA_NETMON_NCELL_TYPE != pstSetCnf->enCellType) )
    {
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    usLengthTemp   = 0;
    usLength       = 0;

    /* GSM邻区显示 */
    AT_NetMonFmtGsmNCellData(pstSetCnf, usLengthTemp, &usLength);

    /* UTRAN邻区显示 */
    usLengthTemp = usLength;

    if (MTA_NETMON_UTRAN_FDD_TYPE == pstSetCnf->stNCellInfo.enCellMeasTypeChoice)
    {
        AT_NetMonFmtUtranFddNCellData(pstSetCnf, usLengthTemp, &usLength);
    }
    else if (MTA_NETMON_UTRAN_TDD_TYPE == pstSetCnf->stNCellInfo.enCellMeasTypeChoice)
    {
        AT_NetMonFmtUtranTddNCellData(pstSetCnf, usLengthTemp, &usLength);
    }
    else
    {
        /*类型不对，不进行任何处理*/
        ;
    }

    /* LTE邻区显示 */
    usLengthTemp = usLength;

    AT_NetMonFmtEutranNCellData(pstSetCnf, usLengthTemp, &usLength);

    /* 无邻区，返回NONE */
    if ( 0 == ( pstSetCnf->stNCellInfo.ucGsmNCellCnt +
                pstSetCnf->stNCellInfo.ucUtranNCellCnt + pstSetCnf->stNCellInfo.ucLteNCellCnt ) )
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: NONE" );
    }

    /* 输出结果 */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_VOID AT_NetMonFmtUtranTddSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    TAF_MEM_SET_S(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* 格式输出PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: TD_SCDMA,%s",
                                        pucPlmnstr );

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u,%d,%d,%X,%X,%d,%d,%d",
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulArfcn,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usSyncID,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usSC,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.ulCellID,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.usLAC,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.sRSCP,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.ulDrx,
                                        pstSCellInfo->unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usRac );

    *pusLength = usLength;

    return;
}

VOS_VOID AT_NetMonFmtUtranTddNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->stNCellInfo.ucUtranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM)
    {
        pstNCellInfo->stNCellInfo.ucUtranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /*WCDMA 临区显示*/
    for (ulLoop = 0; ulLoop < pstNCellInfo->stNCellInfo.ucUtranNCellCnt; ulLoop++)
    {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (0 != usLength)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                 "%s",
                                                 gaucAtCrLf );
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: TD_SCDMA,%u,%d,%d,%d",
                                            pstNCellInfo->stNCellInfo.u.astTddNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->stNCellInfo.u.astTddNCellInfo[ulLoop].usSyncID,
                                            pstNCellInfo->stNCellInfo.u.astTddNCellInfo[ulLoop].usSC,
                                            pstNCellInfo->stNCellInfo.u.astTddNCellInfo[ulLoop].sRSCP );
    }

    *pusOutLen = usLength;

    return;
}


VOS_VOID AT_NetMonFmtEutranSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    TAF_MEM_SET_S(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->unSCellInfo.stLteSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->unSCellInfo.stLteSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* 格式输出PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: LTE,%s",
                                        pucPlmnstr );

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u,%X,%X,%X,%d,%d,%d",
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.ulArfcn,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.ulCellID,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.ulPID,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.usTAC,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.sRSRP,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.sRSRQ,
                                        pstSCellInfo->unSCellInfo.stLteSCellInfo.sRSSI );
    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtEutranNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->stNCellInfo.ucLteNCellCnt > NETMON_MAX_LTE_NCELL_NUM)
    {
        pstNCellInfo->stNCellInfo.ucLteNCellCnt = NETMON_MAX_LTE_NCELL_NUM;
    }

     /* LTE邻区显示 */
    for (ulLoop = 0; ulLoop < pstNCellInfo->stNCellInfo.ucLteNCellCnt; ulLoop++)
    {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (0 != usLength)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: LTE,%u,%X,%d,%d,%d",
                                            pstNCellInfo->stNCellInfo.astLteNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->stNCellInfo.astLteNCellInfo[ulLoop].ulPID,
                                            pstNCellInfo->stNCellInfo.astLteNCellInfo[ulLoop].sRSRP,
                                            pstNCellInfo->stNCellInfo.astLteNCellInfo[ulLoop].sRSRQ,
                                            pstNCellInfo->stNCellInfo.astLteNCellInfo[ulLoop].sRSSI );
    }

    *pusOutLen = usLength;

    return;
}


VOS_VOID AT_FmtTimeStru(
    MTA_AT_TIME_STRU                   *pstTimeStru,
    VOS_UINT16                         *pusOutLen,
    VOS_UINT16                          usInLen
)
{
    VOS_UINT16                          usLength;

    usLength = usInLen;

    /* YY */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d/",
                                       pstTimeStru->usYear / 10,
                                       pstTimeStru->usYear % 10);
    /* MM */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d/",
                                       pstTimeStru->ucMonth / 10,
                                       pstTimeStru->ucMonth % 10);
    /* dd */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d,",
                                       pstTimeStru->ucDay / 10,
                                       pstTimeStru->ucDay % 10);

    /* hh */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d:",
                                       pstTimeStru->ucHour / 10,
                                       pstTimeStru->ucHour % 10);

    /* mm */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d:",
                                       pstTimeStru->ucMinute / 10,
                                       pstTimeStru->ucMinute % 10);

    /* ss */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d, ",
                                       pstTimeStru->ucSecond / 10,
                                       pstTimeStru->ucSecond % 10);

   *pusOutLen = usLength;

    return;
}

/*lint -save -e845 -specific(-e845)*/

VOS_VOID AT_FormatRsrp(
    VOS_UINT8                           ucIndex,
    MTA_AT_RS_INFO_QRY_CNF_STRU        *pstRsInfoQryCnf
)
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* 格式化AT+RSRP?查询命令返回 */
    gstAtSendData.usBufLen = 0;

    /* +RSRP: */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* 判断查询操作是否成功 */
    if ( (MTA_AT_RESULT_NO_ERROR != pstRsInfoQryCnf->enResult)
      || (0 == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum) )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "NONE");

        gstAtSendData.usBufLen = usLength;
        return;
    }

    for (i = 0; i < pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].ulCellId);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].ulEarfcn);

        if (pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp >= 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d.%02d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp / AT_RS_INFO_MULTI,
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp % AT_RS_INFO_MULTI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",-%d.%02d",
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp) / AT_RS_INFO_MULTI,
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp) % AT_RS_INFO_MULTI);
        }


        if (i == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum - 1)
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    gstAtSendData.usBufLen = usLength;
    return;
}


VOS_VOID AT_FormatRsrq(
    VOS_UINT8                           ucIndex,
    MTA_AT_RS_INFO_QRY_CNF_STRU        *pstRsInfoQryCnf
)
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* 格式化AT+RSRQ?查询命令返回 */
    gstAtSendData.usBufLen = 0;

    /* +RSRQ: */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* 判断查询操作是否成功 */
    if ( (MTA_AT_RESULT_NO_ERROR != pstRsInfoQryCnf->enResult)
      || (0 == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum) )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "NONE");

        gstAtSendData.usBufLen = usLength;
        return;
    }

    for (i = 0; i < pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].ulCellId);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].ulEarfcn);

        if (pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq >= 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d.%02d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq / AT_RS_INFO_MULTI,
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq % AT_RS_INFO_MULTI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",-%d.%02d",
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq) / AT_RS_INFO_MULTI,
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq) % AT_RS_INFO_MULTI);
        }

        if (i == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum - 1)
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    gstAtSendData.usBufLen = usLength;
    return;
}
/*lint -restore*/


VOS_VOID AT_FormatGasAtCmdRslt(
    MTA_AT_GAS_AUTOTEST_QRY_RSLT_STRU                      *pstAtCmdRslt)
{
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    VOS_UINT16                          usLength;

    i = 0;
    j = 0;
    usLength = 0;

    for (i = 0; i< (pstAtCmdRslt->ulRsltNum/3); i++)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "%d:%d,",
                                           (i+1),
                                           pstAtCmdRslt->aulRslt[j++]);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "0x%X,",
                                           pstAtCmdRslt->aulRslt[j++]&0xff);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "%d\r\n",
                                           pstAtCmdRslt->aulRslt[j++]);
    }

    gstAtSendData.usBufLen = usLength;
}


VOS_UINT32 AT_CheckJDCfgGsmPara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* 判断平台是否支持GSM */
    if (VOS_TRUE != AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_GSM))
    {
        AT_ERR_LOG("AT_CheckJDCfgGsmPara: Not Support GSM.");
        return VOS_ERR;
    }

    /* 设置命令格式:AT^JDCFG=0,<rssi_thresh>,<rssi_num> */
    if (AT_JAM_DETECT_GSM_PARA_NUM != gucAtParaIndex)
    {
        return VOS_ERR;
    }

    /* 配置参数范围检查 */
    if ((0 == gastAtParaList[1].usParaLen) 
        || (AT_JAM_DETECT_GSM_THRESHOLD_MAX < gastAtParaList[1].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[2].usParaLen) 
        || (AT_JAM_DETECT_GSM_FREQNUM_MAX < gastAtParaList[2].ulParaValue))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgWcdmaPara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* 判断平台是否支持WCDMA */
    if (VOS_TRUE != AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_WCDMA))
    {
        AT_ERR_LOG("AT_CheckJDCfgWcdmaPara: Not Support WCDMA.");
        return VOS_ERR;
    }

    /* 设置命令格式:AT^JDCFG=1,<rssi_thresh>,<rssi_percent>,<psch_thresh>,<psch_percent> */
    if (AT_JAM_DETECT_WL_PARA_NUM != gucAtParaIndex)
    {
        return VOS_ERR;
    }

    /* 配置参数范围检查 */
    if ((0 == gastAtParaList[1].usParaLen) ||
        (AT_JAM_DETECT_WCDMA_RSSI_THRESHOLD_MAX < gastAtParaList[1].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[2].usParaLen) ||
        (AT_JAM_DETECT_WCDMA_RSSI_PERCENT_MAX < gastAtParaList[2].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[3].usParaLen) ||
        (AT_JAM_DETECT_WCDMA_PSCH_THRESHOLD_MAX < gastAtParaList[3].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[4].usParaLen) ||
        (AT_JAM_DETECT_WCDMA_PSCH_PERCENT_MAX < gastAtParaList[4].ulParaValue))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgLtePara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* 判断平台是否支持LTE */
    if (VOS_TRUE != AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_LTE))
    {
        AT_ERR_LOG("AT_CheckJDCfgLtePara: Not Support LTE.");
        return VOS_ERR;
    }

    /* 设置命令格式:AT^JDCFG=1,<rssi_thresh>,<rssi_percent>,<pssratio_thresh>,<pssratio_percent> */
    if (AT_JAM_DETECT_WL_PARA_NUM != gucAtParaIndex)
    {
        return VOS_ERR;
    }

    /* 配置参数范围检查 */
    if ((0 == gastAtParaList[1].usParaLen) ||
        (AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX < gastAtParaList[1].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[2].usParaLen) ||
        (AT_JAM_DETECT_LTE_RSSI_PERCENT_MAX < gastAtParaList[2].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[3].usParaLen) ||
        (AT_JAM_DETECT_LTE_PSSRATIO_THRESHOLD_MAX < gastAtParaList[3].ulParaValue))
    {
        return VOS_ERR;
    }

    if ((0 == gastAtParaList[4].usParaLen) ||
        (AT_JAM_DETECT_LTE_PSSRATIO_PERCENT_MAX < gastAtParaList[4].ulParaValue))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgPara(VOS_VOID)
{
    VOS_UINT32                          ulRst = VOS_ERR;

    /* 参数个数检测 */
    if ((AT_JAM_DETECT_GSM_PARA_NUM != gucAtParaIndex) && (AT_JAM_DETECT_WL_PARA_NUM != gucAtParaIndex))
    {
        return VOS_ERR;
    }

    /* 检测GUL制式下，JD配置参数有效性 */
    switch ((AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue)
    {
        case AT_MTA_CMD_RATMODE_GSM:
            ulRst = AT_CheckJDCfgGsmPara();
            break;

        case AT_MTA_CMD_RATMODE_WCDMA:
            ulRst = AT_CheckJDCfgWcdmaPara();
            break;

        case AT_MTA_CMD_RATMODE_LTE:     
            ulRst = AT_CheckJDCfgLtePara();
            break;

        default:
            break;
    }

    return ulRst;
}


VOS_UINT32 AT_ProcJDCfgPara(AT_MTA_SET_JAM_DETECT_REQ_STRU *pstAtCmd)
{
    /* 获取GUL制式下，干扰检测配置参数 */
    pstAtCmd->ucMode = AT_MTA_JAM_DETECT_MODE_UPDATE;          
    pstAtCmd->ucRat  = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    switch ((AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue)
    {
        case AT_MTA_CMD_RATMODE_GSM:
            pstAtCmd->unJamPara.stGsmPara.ucThreshold   = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstAtCmd->unJamPara.stGsmPara.ucFreqNum     = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            break;

        case AT_MTA_CMD_RATMODE_WCDMA:
            pstAtCmd->unJamPara.stWcdmaPara.ucRssiSrhThreshold    = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.ucRssiSrhFreqPercent  = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.usPschSrhThreshold    = (VOS_UINT16)gastAtParaList[3].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.ucPschSrhFreqPercent  = (VOS_UINT8)gastAtParaList[4].ulParaValue;
            break;

        case AT_MTA_CMD_RATMODE_LTE:
            pstAtCmd->unJamPara.stLtePara.sRssiThresh         = (VOS_INT16)gastAtParaList[1].ulParaValue - AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX;
            pstAtCmd->unJamPara.stLtePara.ucRssiPercent       = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            pstAtCmd->unJamPara.stLtePara.usPssratioThresh    = (VOS_UINT16)gastAtParaList[3].ulParaValue;
            pstAtCmd->unJamPara.stLtePara.ucPssratioPercent   = (VOS_UINT8)gastAtParaList[4].ulParaValue;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetJDCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_JAM_DETECT_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst   = TAF_SUCCESS;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    ulRst = AT_CheckJDCfgPara(); 

    if (VOS_OK != ulRst)
    {    
        AT_ERR_LOG("AT_SetJDCfgPara: AT_CheckJDCfgPara error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU));  

    /* 获取干扰检测配置参数 */
    ulRst = AT_ProcJDCfgPara(&stAtCmd); 

    if (VOS_OK != ulRst)
    {
        return AT_ERROR;
    }

    /* 下发干扰检测参数更新请求消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SET_JAM_DETECT_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDCFG_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryJDCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRst = TAF_SUCCESS;

    /* 下发干扰检测配置查询请求消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_QRY_JAM_DETECT_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDCFG_READ;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestJDCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;

    /* 输出GSM的参数列表 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (0),(0-70),(0-255)%s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       gaucAtCrLf);

    /* 输出WCDMA的参数列表 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (1),(0-70),(0-100),(0-65535),(0-100)%s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       gaucAtCrLf);

    /* 输出LTE的参数列表 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (2),(0-70),(0-100),(0-1000),(0-100)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetJDSwitchPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_JAM_DETECT_REQ_STRU      stAtCmd = {0};   
    VOS_UINT32                          ulRst   = TAF_SUCCESS;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_ERROR;
    }

    /* 下发干扰检测开关配置请求消息给C核处理 */
    stAtCmd.ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ucRat  = AT_MTA_CMD_RATMODE_BUTT;

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SET_JAM_DETECT_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDSWITCH_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryJDSwitchPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRst = TAF_SUCCESS;

    /* 下发JD查询请求消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_QRY_JAM_DETECT_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDSWITCH_READ;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMtaSetJamDetectCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_JAM_DETECT_CNF_STRU *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                      ulResult  = AT_OK;
    VOS_UINT8                       ucIndex   = 0;

    /* 初始化 */
    pstRcvMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf = (MTA_AT_SET_JAM_DETECT_CNF_STRU *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if ((AT_CMD_JDCFG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt) 
        && (AT_CMD_JDSWITCH_SET != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : Current Option is not AT_CMD_JDCFG_SET or AT_CMD_JDSWITCH_SET.");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR != pstSetCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryJamDetectCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_QRY_JAM_DETECT_CNF_STRU *pstQryCnf = VOS_NULL_PTR;
    VOS_UINT8                       ucIndex   = 0;   
    VOS_UINT16                      usLength  = 0;

    /* 初始化 */
    pstRcvMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstQryCnf = (MTA_AT_QRY_JAM_DETECT_CNF_STRU *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryJamDetectCfgCnf : WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryJamDetectCfgCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_JDCFG_READ:
            /* GSM制式，干扰检测配置参数 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_GSM,
                                               pstQryCnf->stGsmPara.ucThreshold,
                                               pstQryCnf->stGsmPara.ucFreqNum,
                                               gaucAtCrLf);

            /* WCDMA制式，干扰检测配置参数 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d,%d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_WCDMA,
                                               pstQryCnf->stWcdmaPara.ucRssiSrhThreshold,
                                               pstQryCnf->stWcdmaPara.ucRssiSrhFreqPercent,
                                               pstQryCnf->stWcdmaPara.usPschSrhThreshold,
                                               pstQryCnf->stWcdmaPara.ucPschSrhFreqPercent,
                                               gaucAtCrLf);

            /* LTE制式，干扰检测配置参数 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d,%d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_LTE,
                                               pstQryCnf->stLtePara.sRssiThresh + AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX,
                                               pstQryCnf->stLtePara.ucRssiPercent,
                                               pstQryCnf->stLtePara.usPssratioThresh,
                                               pstQryCnf->stLtePara.ucPssratioPercent);

            gstAtSendData.usBufLen = usLength;

            /* 复位AT状态 */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_OK);

            return VOS_OK;

        case AT_CMD_JDSWITCH_READ:
            /* GSM制式，干扰检测开关状态 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucGsmJamMode,
                                               AT_MTA_CMD_RATMODE_GSM,
                                               gaucAtCrLf);

            /* WCDMA制式，干扰检测开关状态 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucWcdmaJamMode,
                                               AT_MTA_CMD_RATMODE_WCDMA,
                                               gaucAtCrLf);

            /* LTE制式，干扰检测开关状态 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucLteJamMode,
                                               AT_MTA_CMD_RATMODE_LTE); 

            gstAtSendData.usBufLen = usLength;

            /* 复位AT状态 */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_OK);

            return VOS_OK;

        default:
            AT_WARN_LOG("AT_RcvMtaMbbQryJamDetectCnf : Current Option is not AT_CMD_JDCFG_READ or AT_CMD_JDSWITCH_READ.");

            return VOS_ERR;
    }
}


VOS_UINT32 AT_RcvMtaJamDetectInd(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex         = 0;
    VOS_UINT16                          usLength        = 0;
    AT_MTA_MSG_STRU                    *pstMtaMsg       = VOS_NULL_PTR;
    MTA_AT_JAM_DETECT_IND_STRU         *pstJamDetectInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    pstMtaMsg       = (AT_MTA_MSG_STRU *)pMsg;
    pstJamDetectInd = (MTA_AT_JAM_DETECT_IND_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaJamDetectInd: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    /* 上报干扰检测结果 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s^JDINFO: %d,%d%s",
                                       gaucAtCrLf,
                                       pstJamDetectInd->enJamResult,
                                       pstJamDetectInd->ucRat,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_SetEmRssiCfgPara(VOS_UINT8 ucIndex)
{
    TAF_MMA_EMRSSICFG_REQ_STRU          stEmRssiCfgPara;

    TAF_MEM_SET_S(&stEmRssiCfgPara, sizeof(stEmRssiCfgPara), 0x00, sizeof(TAF_MMA_EMRSSICFG_REQ_STRU));

    /* 检查是否设置命令 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Not Set Command!");
        return AT_ERROR;
    }

    /* 检查参数个数 */
    if (2 != gucAtParaIndex)
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Input parameters go wrong!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将入参封装到结构体中再发起请求 */
    stEmRssiCfgPara.ucEmRssiCfgRat       = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stEmRssiCfgPara.ucEmRssiCfgThreshold = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* 发送消息给C核处理 */
    if (VOS_TRUE == TAF_MMA_SetEmRssiCfgReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            &stEmRssiCfgPara))
    {

        /* 返回命令处理挂起状态 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSICFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: AT send ERROR!");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEmRssiCfgPara(VOS_UINT8 ucIndex)
{
    /* 执行命令操作 */
    if (VOS_TRUE == TAF_MMA_QryEmRssiCfgReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId))
    {

        /* 返回命令处理挂起状态 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSICFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEmRssiRptPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucEmRssiRptSwitch;

    /* 检查是否设置命令 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("At_SetEmRssiRptPara: Not Set Command!");
        return AT_ERROR;
    }

    /* 检查参数个数 */
    if (1 != gucAtParaIndex)
    {
        AT_WARN_LOG("At_SetEmRssiRptPara: Input parameters go wrong!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ucEmRssiRptSwitch = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送消息给C核处理 */
    if (VOS_TRUE == TAF_MMA_SetEmRssiRptReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            &ucEmRssiRptSwitch))
    {

        /* 返回命令处理挂起状态 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSIRPT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEmRssiRptPara(VOS_UINT8 ucIndex)
{
    /* 执行命令操作 */
    if (VOS_TRUE == TAF_MMA_QryEmRssiRptReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId))
    {

        /* 返回命令处理挂起状态 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSIRPT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMmaEmRssiCfgSetCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_EMRSSICFG_SET_CNF_STRU     *pstCnfMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex = 0;
    VOS_UINT32                          ulResult = AT_OK;

    pstCnfMsg = (TAF_MMA_EMRSSICFG_SET_CNF_STRU *)pMsg;

    /* 通过clientid获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstCnfMsg->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EMRSSICFG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: Current Option is not AT_CMD_EMRSSICFG_SET.");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (TAF_ERR_NO_ERROR != pstCnfMsg->enErrorCause)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: Set command go wrong!");
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiCfgQryCnf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex            = 0;
    VOS_UINT16                          usLength           = 0;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult           = AT_FAILURE;
    TAF_MMA_EMRSSICFG_QRY_CNF_STRU     *pstEmRssiCfgQryCnf = VOS_NULL_PTR;

    pstEmRssiCfgQryCnf = (TAF_MMA_EMRSSICFG_QRY_CNF_STRU *)pMsg;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstEmRssiCfgQryCnf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EMRSSICFG_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: Current Option is not AT_CMD_EMRSSICFG_QRY.");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (TAF_ERR_NO_ERROR != pstEmRssiCfgQryCnf->enErrorCause)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        /* GSM EMRSSICFG输出 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_GSM,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgGsmThreshold,
                                           gaucAtCrLf);
        
        /* WCDMA EMRSSICFG输出 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_WCDMA,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgWcdmaThreshold,
                                           gaucAtCrLf);
        
        /* LTE EMRSSICFG输出 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_LTE,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgLteThreshold);

        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiRptSetCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_EMRSSIRPT_SET_CNF_STRU     *pstCnfMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex   = 0;
    VOS_UINT32                          ulResult  = AT_ERROR;

    pstCnfMsg = (TAF_MMA_EMRSSIRPT_SET_CNF_STRU *)pMsg;

    /* 通过clientid获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstCnfMsg->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EMRSSIRPT_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: Current Option is not AT_CMD_EMRSSIRPT_SET.");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (TAF_ERR_NO_ERROR != pstCnfMsg->enErrorCause)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: Set command go wrong!");
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiRptQryCnf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex            = 0;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult           = AT_FAILURE;
    TAF_UINT16                          usLength           = 0;
    TAF_MMA_EMRSSIRPT_QRY_CNF_STRU     *pstEmRssiRptQryCnf = VOS_NULL_PTR;

    pstEmRssiRptQryCnf = (TAF_MMA_EMRSSIRPT_QRY_CNF_STRU *)pMsg;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstEmRssiRptQryCnf->stCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EMRSSIRPT_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: Current Option is not AT_CMD_EMRSSIRPT_QRY.");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (TAF_ERR_NO_ERROR != pstEmRssiRptQryCnf->enErrorCause)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEmRssiRptQryCnf->ucEmRssiRptSwitch);

        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_VOID AT_ConvertRssiLevel(VOS_INT16 sRssiValue, VOS_UINT8 *pucRssiLevel)
{
    if(AT_HCSQ_VALUE_INVALID <= sRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_RSSI_VALUE_MAX <= sRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_RSSI_LEVEL_MAX;
    }
    else if (AT_HCSQ_RSSI_VALUE_MIN > sRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRssiLevel = (VOS_UINT8)((sRssiValue - AT_HCSQ_RSSI_VALUE_MIN) + 1);
    }

    return;
}


VOS_UINT32 AT_RcvMmaEmRssiRptInd(VOS_VOID *pstMsg)
{
    TAF_MMA_RSSI_INFO_IND_STRU         *pstEmRssiRptInd = VOS_NULL_PTR;
    VOS_UINT8                           ucRssiValue     = 0;
    VOS_UINT8                           ucIndex         = 0;
    VOS_UINT16                          usLength        = 0;
    VOS_INT16                           sEmRssi         = 0;

    pstEmRssiRptInd = (TAF_MMA_RSSI_INFO_IND_STRU *)pstMsg;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstEmRssiRptInd->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptInd:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    switch (pstEmRssiRptInd->stRssiInfo.enRatType)
    {
        case TAF_MMA_RAT_GSM:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stGCellSignInfo.sRssiValue;
            break;
        }
        case TAF_MMA_RAT_WCDMA:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue
                      - pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sEcioValue;
            break;
        }
        case TAF_MMA_RAT_LTE:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRssi;
            break;
        }
        default:
        {
            return VOS_ERR;
        }
    }

    AT_ConvertRssiLevel(sEmRssi, &ucRssiValue);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_EMRSSIRPT].pucText,
                                       pstEmRssiRptInd->stRssiInfo.enRatType,
                                       ucRssiValue,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength);

    return VOS_OK;
}


