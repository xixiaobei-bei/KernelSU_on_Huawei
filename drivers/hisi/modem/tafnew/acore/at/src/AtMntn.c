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
  1 其他头文件包含
*****************************************************************************/
#include "AtMntn.h"
#include "AtInputProc.h"
#include "AtCtx.h"
#include "AtDataProc.h"
#include "TafLogPrivacyMatch.h"
#include "ATCmdProc.h"
#include "TafAcoreLogPrivacy.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_MNTN_C

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
AT_DEBUG_INFO_STRU                      g_stAtDebugInfo = {VOS_FALSE};

AT_MNTN_STATS_STRU                      g_stAtStatsInfo;

AT_MNTN_MSG_RECORD_INFO_STRU            g_stAtMsgRecordInfo;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern VOS_VOID* GUNAS_FilterAtToAtMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
);

/*****************************************************************************
  10 函数实现
*****************************************************************************/

VOS_VOID AT_InitMntnCtx(VOS_VOID)
{
    TAF_MEM_SET_S(&g_stAtMsgRecordInfo, sizeof(g_stAtMsgRecordInfo), 0x00, sizeof(g_stAtMsgRecordInfo));
}


VOS_VOID AT_SetPcuiCtrlConcurrentFlag(VOS_UINT8 ucFlag)
{
    g_stAtDebugInfo.ucPcuiCtrlConcurrentFlg = ucFlag;
}


VOS_UINT8 AT_GetPcuiCtrlConcurrentFlag(VOS_VOID)
{
    return g_stAtDebugInfo.ucPcuiCtrlConcurrentFlg;
}


VOS_VOID AT_SetPcuiPsCallFlag(
    VOS_UINT8                           ucFlag,
    VOS_UINT8                           ucIndex
)
{
    g_stAtDebugInfo.ucPcuiPsCallFlg     = ucFlag;
    g_stAtDebugInfo.ucPcuiUserId        = AT_CLIENT_TAB_APP_INDEX;
}


VOS_UINT8 AT_GetPcuiPsCallFlag(VOS_VOID)
{
    return g_stAtDebugInfo.ucPcuiPsCallFlg;
}


VOS_UINT8 AT_GetPcuiUsertId(VOS_VOID)
{
    return g_stAtDebugInfo.ucPcuiUserId;
}


VOS_VOID AT_SetCtrlPsCallFlag(
    VOS_UINT8                           ucFlag,
    VOS_UINT8                           ucIndex
)
{
    g_stAtDebugInfo.ucCtrlPsCallFlg     = ucFlag;
#if (FEATURE_ON == FEATURE_VCOM_EXT)
    g_stAtDebugInfo.ucCtrlUserId        = AT_CLIENT_TAB_APP5_INDEX;
#else
    g_stAtDebugInfo.ucCtrlUserId        = AT_CLIENT_TAB_APP_INDEX;
#endif
}


VOS_UINT8 AT_GetCtrlPsCallFlag(VOS_VOID)
{
    return g_stAtDebugInfo.ucCtrlPsCallFlg;
}


VOS_UINT8 AT_GetCtrlUserId(VOS_VOID)
{
    return g_stAtDebugInfo.ucCtrlUserId;
}


VOS_VOID AT_SetPcui2PsCallFlag(
    VOS_UINT8                           ucFlag,
    VOS_UINT8                           ucIndex
)
{
    g_stAtDebugInfo.ucPcui2PsCallFlg    = ucFlag;
#if  (FEATURE_ON == FEATURE_VCOM_EXT)
    g_stAtDebugInfo.ucPcui2UserId       = AT_CLIENT_TAB_APP20_INDEX;
#else
    g_stAtDebugInfo.ucPcui2UserId       = AT_CLIENT_TAB_APP_INDEX;
#endif /* #if FEATURE_ON == FEATURE_VCOM_EXT */

}


VOS_UINT8 AT_GetPcui2PsCallFlag(VOS_VOID)
{
    return g_stAtDebugInfo.ucPcui2PsCallFlg;
}


VOS_UINT8 AT_GetPcui2UserId(VOS_VOID)
{
    return g_stAtDebugInfo.ucPcui2UserId;
}



VOS_VOID AT_SetUnCheckApPortFlg(
    VOS_UINT8                           ucFlag
)
{
    if ((0 == ucFlag) || (1 == ucFlag))
    {
        g_stAtDebugInfo.ucUnCheckApPortFlg = ucFlag;
    }
}


VOS_UINT8 AT_GetUnCheckApPortFlg(VOS_VOID)
{
    return g_stAtDebugInfo.ucUnCheckApPortFlg;
}


VOS_VOID AT_MNTN_TraceEvent(VOS_VOID *pMsg)
{
    VOS_VOID                           *pLogPrivacyMsg  = VOS_NULL_PTR;

    /* at模块内部维测消息, 增加脱敏处理 */
    pLogPrivacyMsg  = pMsg;

    if (VOS_TRUE == AT_GetPrivacyFilterEnableFlg())
    {
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        if (VOS_NULL_PTR == GUNAS_FilterAtToAtMsg((PS_MSG_HEADER_STRU *)pMsg))
        {
            return;
        }

        /* cnas at命令脱敏过滤函数处理 */
        pLogPrivacyMsg = AT_PrivacyMatchAtCmd(pMsg);
        if (VOS_NULL_PTR == pLogPrivacyMsg)
        {
            return;
        }
#endif
    }

    DIAG_TraceReport(pLogPrivacyMsg);

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    /* 如果脱敏处理函数申请了新的at命令字符串，释放掉 */
    if (pLogPrivacyMsg != pMsg)
    {
        VOS_MemFree(WUEPS_PID_AT, pLogPrivacyMsg);
    }
#endif

    return;
}


VOS_VOID AT_MNTN_TraceInputMsc(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    AT_MNTN_MSC_STRU                    stMntnMsc;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnMsc, ID_AT_MNTN_INPUT_MSC, (sizeof(AT_MNTN_MSC_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnMsc.ulPortId = ucIndex;
    TAF_MEM_CPY_S(&(stMntnMsc.stDceMscInfo), sizeof(stMntnMsc.stDceMscInfo), pstDceMsc, sizeof(AT_DCE_MSC_STRU));

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnMsc);

    return;
}


VOS_VOID AT_MNTN_TraceOutputMsc(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    AT_MNTN_MSC_STRU                    stMntnMsc;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnMsc, ID_AT_MNTN_OUTPUT_MSC, (sizeof(AT_MNTN_MSC_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnMsc.ulPortId = ucIndex;
    TAF_MEM_CPY_S(&(stMntnMsc.stDceMscInfo), sizeof(stMntnMsc.stDceMscInfo), pstDceMsc, sizeof(AT_DCE_MSC_STRU));

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnMsc);

    return;
}


VOS_VOID AT_MNTN_TraceStartFlowCtrl(
    VOS_UINT8                           ucIndex,
    AT_FC_DEVICE_TYPE_ENUM_UINT32       enFcDevive
)
{
    AT_MNTN_FLOW_CTRL_STRU              stMntnFlowCtrl;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnFlowCtrl, ID_AT_MNTN_START_FLOW_CTRL, (sizeof(AT_MNTN_FLOW_CTRL_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnFlowCtrl.ulPortId = ucIndex;
    stMntnFlowCtrl.enDevice = enFcDevive;

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnFlowCtrl);

    return;
}


VOS_VOID AT_MNTN_TraceStopFlowCtrl(
    VOS_UINT8                           ucIndex,
    AT_FC_DEVICE_TYPE_ENUM_UINT32       enFcDevive
)
{
    AT_MNTN_FLOW_CTRL_STRU              stMntnFlowCtrl;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnFlowCtrl, ID_AT_MNTN_STOP_FLOW_CTRL, (sizeof(AT_MNTN_FLOW_CTRL_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnFlowCtrl.ulPortId = ucIndex;
    stMntnFlowCtrl.enDevice = enFcDevive;

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnFlowCtrl);

    return;
}


VOS_VOID AT_MNTN_TraceRegFcPoint(
    VOS_UINT8                           ucIndex,
    AT_FC_POINT_TYPE_ENUM_UINT32        enFcPoint
)
{
    AT_MNTN_FC_POINT_STRU               stMntnFcPoint;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnFcPoint, ID_AT_MNTN_REG_FC_POINT, (sizeof(AT_MNTN_FC_POINT_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnFcPoint.ulPortId = ucIndex;
    stMntnFcPoint.enPoint  = enFcPoint;

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnFcPoint);

    return;
}


VOS_VOID AT_MNTN_TraceDeregFcPoint(
    VOS_UINT8                           ucIndex,
    AT_FC_POINT_TYPE_ENUM_UINT32        enFcPoint
)
{
    AT_MNTN_FC_POINT_STRU               stMntnFcPoint;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnFcPoint, ID_AT_MNTN_DEREG_FC_POINT, (sizeof(AT_MNTN_FC_POINT_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnFcPoint.ulPortId = ucIndex;
    stMntnFcPoint.enPoint  = enFcPoint;

    /* 发送消息 */
    AT_MNTN_TraceEvent(&stMntnFcPoint);

    return;
}


VOS_VOID AT_MNTN_TraceCmdResult(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usDataLen
)
{
    AT_MSG_STRU                        *pstMsg;
    AT_INTER_MSG_ID_ENUM_UINT32         enEventId;
    VOS_UINT32                          ulLength;
    MODEM_ID_ENUM_UINT16                enModemId;

    AT_GetAtMsgStruMsgLength(usDataLen, &ulLength);

    /* 申请消息内存 */
    pstMsg = (AT_MSG_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, (ulLength + VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_MNTN_TraceCmdResult:ERROR:Alloc Mem Fail.");
        return;
    }

    /* 填写消息头 */
    enEventId       = AT_GetResultMsgID(ucIndex);
    AT_MNTN_CFG_MSG_HDR(pstMsg, enEventId, ulLength);

    /* 填写消息内容 */
    pstMsg->ucType  = 0x1;
    pstMsg->ucIndex = ucIndex;
    pstMsg->usLen   = usDataLen;

    enModemId = MODEM_ID_0;
    if (VOS_OK != AT_GetModemIdFromClient(ucIndex, &enModemId))
    {
        enModemId = MODEM_ID_0;
    }

    pstMsg->enModemId       = (VOS_UINT8)enModemId;
    pstMsg->enVersionId     = 0xAA;

    pstMsg->ucFilterAtType  = (VOS_UINT8)g_enLogPrivacyAtCmd;

    AT_GetUserTypeFromIndex(ucIndex, &pstMsg->ucUserType);

    TAF_MEM_CPY_S((TAF_VOID*)pstMsg->aucValue, usDataLen, pucData, usDataLen);

    /* 填写消息内容 */
    AT_MNTN_TraceEvent(pstMsg);

    /* 释放消息内存 */
    /*lint -save -e830 */
    PS_MEM_FREE(WUEPS_PID_AT, pstMsg);
    /*lint -restore */
    return;
}



VOS_VOID AT_MNTN_TraceContextData(VOS_VOID)
{
    NAS_AT_SDT_AT_PART_ST                  *pstSndMsgCB     = VOS_NULL_PTR;
    NAS_AT_OUTSIDE_RUNNING_CONTEXT_PART_ST *pstOutsideCtx   = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU                  *pstSmsCtx       = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                    enModemId;
    /*lint -save -e516 */
    pstSndMsgCB = (NAS_AT_SDT_AT_PART_ST *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(NAS_AT_SDT_AT_PART_ST));
    /*lint -restore */
    if (VOS_NULL_PTR == pstSndMsgCB)
    {
        AT_ERR_LOG("AT_MNTN_TraceContextData:ERROR: Alloc Memory Fail.");
        return;
    }

    pstSndMsgCB->ulReceiverPid      = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderPid        = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulLength           = sizeof(NAS_AT_OUTSIDE_RUNNING_CONTEXT_PART_ST) + 4;
    pstSndMsgCB->ucType             = AT_PC_REPLAY_MSG;
    pstSndMsgCB->enMsgID            = ID_AT_MNTN_PC_REPLAY_MSG;
    pstSndMsgCB->aucReserved[0]     = 0;
    pstSndMsgCB->aucReserved[1]     = 0;
    pstSndMsgCB->aucReserved[2]     = 0;


    for (enModemId = 0; enModemId < MODEM_ID_BUTT; enModemId++)
    {
        pstSmsCtx                           = AT_GetModemSmsCtxAddrFromModemId(enModemId);
        pstOutsideCtx                       = &pstSndMsgCB->astOutsideCtx[enModemId];

        pstOutsideCtx->gucAtCscsType        = gucAtCscsType;
        pstOutsideCtx->gucAtCsdhType        = pstSmsCtx->ucCsdhType;
        pstOutsideCtx->g_OpId               = g_OpId;
        pstOutsideCtx->g_enAtCsmsMsgVersion = pstSmsCtx->enCsmsMsgVersion;
        pstOutsideCtx->g_enAtCmgfMsgFormat  = pstSmsCtx->enCmgfMsgFormat;

        TAF_MEM_CPY_S(&pstOutsideCtx->gstAtCnmiType,
                   sizeof(pstOutsideCtx->gstAtCnmiType),
                   &(pstSmsCtx->stCnmiType),
                   sizeof(pstSmsCtx->stCnmiType));

        TAF_MEM_CPY_S(&pstOutsideCtx->g_stAtCscaCsmpInfo,
                   sizeof(pstOutsideCtx->g_stAtCscaCsmpInfo),
                   &(pstSmsCtx->stCscaCsmpInfo),
                   sizeof(pstSmsCtx->stCscaCsmpInfo));

        TAF_MEM_CPY_S(&pstOutsideCtx->g_stAtCpmsInfo,
                   sizeof(pstOutsideCtx->g_stAtCpmsInfo),
                   &(pstSmsCtx->stCpmsInfo),
                   sizeof(pstSmsCtx->stCpmsInfo));

    }

    AT_MNTN_TraceEvent(pstSndMsgCB);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, pstSndMsgCB);
    /*lint -restore */

    return;
}


VOS_VOID AT_MNTN_TraceClientData(VOS_VOID)
{
    NAS_AT_SDT_AT_CLIENT_TABLE_STRU         *pstSndMsgCB;
    TAF_UINT8                               ucLoop;
    /*lint -save -e516 */
    pstSndMsgCB = (NAS_AT_SDT_AT_CLIENT_TABLE_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(NAS_AT_SDT_AT_CLIENT_TABLE_STRU));
    /*lint -restore */
    if ( VOS_NULL_PTR == pstSndMsgCB )
    {
        AT_ERR_LOG("AT_MNTN_TraceClientData:ERROR: Alloc Memory Fail.");
        return;
    }

    pstSndMsgCB->ulReceiverPid      = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderPid        = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulLength           = sizeof(NAS_AT_SDT_AT_CLIENT_TABLE_STRU) - 20;
    pstSndMsgCB->enMsgID            = ID_AT_MNTN_PC_REPLAY_CLIENT_TAB;
    pstSndMsgCB->ucType             = AT_PC_REPLAY_MSG_CLIENT_TAB;
    pstSndMsgCB->aucReserved[0]     = 0;
    pstSndMsgCB->aucReserved[1]     = 0;
    pstSndMsgCB->aucReserved[2]     = 0;

    for (ucLoop = 0; ucLoop < AT_MAX_CLIENT_NUM; ucLoop++)
    {
        pstSndMsgCB->gastAtClientTab[ucLoop].usClientId   = gastAtClientTab[ucLoop].usClientId;
        pstSndMsgCB->gastAtClientTab[ucLoop].opId         = gastAtClientTab[ucLoop].opId;
        pstSndMsgCB->gastAtClientTab[ucLoop].ucUsed       = gastAtClientTab[ucLoop].ucUsed;
        pstSndMsgCB->gastAtClientTab[ucLoop].UserType     = gastAtClientTab[ucLoop].UserType;
        pstSndMsgCB->gastAtClientTab[ucLoop].Mode         = gastAtClientTab[ucLoop].Mode;
        pstSndMsgCB->gastAtClientTab[ucLoop].IndMode      = gastAtClientTab[ucLoop].IndMode;
    }

    AT_MNTN_TraceEvent(pstSndMsgCB);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, pstSndMsgCB);
    /*lint -restore */
    return;
}


VOS_VOID AT_MNTN_TraceRPTPORT(VOS_VOID)
{
    AT_MNTN_RPTPORT_STRU                   *pstSndMsgCB;
    VOS_UINT8                               ucLoop;
    /*lint -save -e516 */
    pstSndMsgCB = (AT_MNTN_RPTPORT_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(AT_MNTN_RPTPORT_STRU));
    /*lint -restore */
    if ( VOS_NULL_PTR == pstSndMsgCB )
    {
        AT_ERR_LOG("AT_MNTN_TraceRPTPORT:ERROR: Alloc Memory Fail.");
        return;
    }

    pstSndMsgCB->ulReceiverPid      = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderPid        = WUEPS_PID_AT;
    pstSndMsgCB->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstSndMsgCB->ulLength           = sizeof(AT_MNTN_RPTPORT_STRU) - 20;
    pstSndMsgCB->enMsgId            = ID_AT_MNTN_RPT_PORT;

    for (ucLoop = 0; ucLoop < AT_MAX_CLIENT_NUM; ucLoop++)
    {
        pstSndMsgCB->astAtRptPort[ucLoop].enAtClientTabIndex = (AT_CLIENT_TAB_INDEX_UINT8)ucLoop;
        pstSndMsgCB->astAtRptPort[ucLoop].enModemId          = g_astAtClientCtx[ucLoop].stClientConfiguration.enModemId;
        pstSndMsgCB->astAtRptPort[ucLoop].ucReportFlg        = g_astAtClientCtx[ucLoop].stClientConfiguration.ucReportFlg;
    }

    AT_MNTN_TraceEvent(pstSndMsgCB);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, pstSndMsgCB);
    /*lint -restore */
    return;
}


VOS_VOID AT_InitHsUartStats(VOS_VOID)
{
    TAF_MEM_SET_S(&g_stAtStatsInfo.stHsUartStats, sizeof(g_stAtStatsInfo.stHsUartStats), 0x00, sizeof(AT_MNTN_HSUART_STATS_STRU));
    return;
}


VOS_VOID AT_InitModemStats(VOS_VOID)
{
    TAF_MEM_SET_S(&g_stAtStatsInfo.stModemStats, sizeof(g_stAtStatsInfo.stModemStats), 0x00, sizeof(AT_MNTN_MODEM_STATS_STRU));
    return;
}


VOS_VOID AT_RecordAtMsgInfo(
    VOS_UINT32                          ulSendPid,
    VOS_UINT32                          ulMsgName,
    VOS_UINT32                          ulSliceStart,
    VOS_UINT32                          ulSliceEnd
)
{
    VOS_UINT32                          ulIndex;

    if (g_stAtMsgRecordInfo.ulCurrentIndex >= AT_MNTN_MSG_RECORD_MAX_NUM)
    {
        g_stAtMsgRecordInfo.ulCurrentIndex = 0;
    }

    ulIndex = g_stAtMsgRecordInfo.ulCurrentIndex;

    g_stAtMsgRecordInfo.astAtMntnMsgRecord[ulIndex].ulSendPid    = ulSendPid;
    g_stAtMsgRecordInfo.astAtMntnMsgRecord[ulIndex].ulMsgName    = ulMsgName;
    g_stAtMsgRecordInfo.astAtMntnMsgRecord[ulIndex].ulSliceStart = ulSliceStart;
    g_stAtMsgRecordInfo.astAtMntnMsgRecord[ulIndex].ulSliceEnd   = ulSliceEnd;

    g_stAtMsgRecordInfo.ulCurrentIndex++;

    return;
}

#if (FEATURE_ON == FEATURE_AT_HSUART)

VOS_VOID AT_ShowHsUartConfigInfo(VOS_VOID)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    AT_UART_FLOW_CTRL_STRU             *pstFlowCtrl = VOS_NULL_PTR;
    AT_UART_RI_CFG_STRU                *pstUartRiCfgInfo = VOS_NULL_PTR;
    AT_UART_PHY_CFG_STRU               *pstUartPhyCfgInfo = VOS_NULL_PTR;
    AT_UART_FORMAT_PARAM_STRU          *pstFormatParam = VOS_NULL_PTR;
    VOS_CHAR                            acParityStr[][20] = {"ODD", "EVEN", "MARK", "SPACE", "NONE"};
    VOS_CHAR                            acDcdStr[][20] = {"ALWAYS ON", "CONNECT ON"};
    VOS_CHAR                            acDtrStr[][20] = {"IGNORE", "SWITCH CMD MODE", "HANGUP CALL"};
    VOS_CHAR                            acDsrStr[][20] = {"ALWAYS ON", "CONNECT ON"};
    VOS_CHAR                            acDceByDteStr[][20] = {"NONE", "XON OR XOFF REMOVE", "RTS", "XON_OR_XOFF_PASS"};
    VOS_CHAR                            acDteByDceStr[][20] = {"NONE", "XON OR XOFF REMOVE", "CTS"};

    pstLineCtrl       = AT_GetUartLineCtrlInfo();
    pstFlowCtrl       = AT_GetUartFlowCtrlInfo();
    pstUartRiCfgInfo  = AT_GetUartRiCfgInfo();
    pstUartPhyCfgInfo = AT_GetUartPhyCfgInfo();
    pstFormatParam    = AT_HSUART_GetFormatParam(pstUartPhyCfgInfo->stFrame.enFormat);

    PS_PRINTF_INFO("[1] HSUART PHY                      \n");
    PS_PRINTF_INFO("BaudRate:                   %d\n", pstUartPhyCfgInfo->enBaudRate);
    PS_PRINTF_INFO("Format:                     %d\n", pstUartPhyCfgInfo->stFrame.enFormat);
    PS_PRINTF_INFO("->Data Bit Length:          %d\n", pstFormatParam->enDataBitLength);
    PS_PRINTF_INFO("->Stop Bit Length:          %d\n", pstFormatParam->enStopBitLength);
    PS_PRINTF_INFO("->Parity Bit Length:        %d\n", pstFormatParam->enParityBitLength);
    PS_PRINTF_INFO("Parity Type:                %s\n", acParityStr[pstUartPhyCfgInfo->stFrame.enParity]);

    PS_PRINTF_INFO("[2] HSUART LINE                     \n");
    PS_PRINTF_INFO("DCD[%d]:                    %s\n", pstLineCtrl->enDcdMode, acDcdStr[pstLineCtrl->enDcdMode]);
    PS_PRINTF_INFO("DTR[%d]:                    %s\n", pstLineCtrl->enDtrMode, acDtrStr[pstLineCtrl->enDtrMode]);
    PS_PRINTF_INFO("DSR[%d]:                    %s\n", pstLineCtrl->enDsrMode, acDsrStr[pstLineCtrl->enDsrMode]);

    PS_PRINTF_INFO("[3] HSUART FLOW CTRL                \n");
    PS_PRINTF_INFO("DCE BY DTE[%d]:             %s\n", pstFlowCtrl->enDceByDte, acDceByDteStr[pstFlowCtrl->enDceByDte]);
    PS_PRINTF_INFO("DTE BY DCE[%d]:             %s\n", pstFlowCtrl->enDteByDce, acDteByDceStr[pstFlowCtrl->enDteByDce]);

    PS_PRINTF_INFO("[4] HSUART RI                       \n");
    PS_PRINTF_INFO("SMS RI ON (ms):             %u\n", pstUartRiCfgInfo->ulSmsRiOnInterval);
    PS_PRINTF_INFO("SMS RI OFF (ms):            %u\n", pstUartRiCfgInfo->ulSmsRiOffInterval);
    PS_PRINTF_INFO("VOICE RI ON (ms):           %u\n", pstUartRiCfgInfo->ulVoiceRiOnInterval);
    PS_PRINTF_INFO("VOICE RI OFF (ms):          %u\n", pstUartRiCfgInfo->ulVoiceRiOffInterval);
    PS_PRINTF_INFO("VOICE RI Cycle Times:       %d\n", pstUartRiCfgInfo->ucVoiceRiCycleTimes);

    return;
}


VOS_VOID AT_ShowHsUartNvStats(VOS_VOID)
{
    PS_PRINTF_INFO("HSUART NV STATS                     \n");
    PS_PRINTF_INFO("Read NV Fail NUM:           %d\n", g_stAtStatsInfo.stHsUartStats.ucReadNvFailNum);
    PS_PRINTF_INFO("Write NV Fail NUM:          %d\n", g_stAtStatsInfo.stHsUartStats.ucWriteNvFailNum);
    PS_PRINTF_INFO("BaudRate Invalid NUM:       %d\n", g_stAtStatsInfo.stHsUartStats.ucBaudRateERR);
    PS_PRINTF_INFO("Format Invalid NUM:         %d\n", g_stAtStatsInfo.stHsUartStats.ucFormatERR);

    return;
}


VOS_VOID AT_ShowHsUartIoctlStats(VOS_VOID)
{
    PS_PRINTF_INFO("HSUART IOCTL STATS                  \n");
    PS_PRINTF_INFO("Set Read CB ERR:            %d\n", g_stAtStatsInfo.stHsUartStats.ucSetReadCbERR);
    PS_PRINTF_INFO("Relloc Read BUFF ERR:       %d\n", g_stAtStatsInfo.stHsUartStats.ucRellocReadBuffERR);
    PS_PRINTF_INFO("Set Free CB ERR:            %d\n", g_stAtStatsInfo.stHsUartStats.ucSetFreeBuffCbERR);
    PS_PRINTF_INFO("Set MSC Read CB ERR:        %d\n", g_stAtStatsInfo.stHsUartStats.ucSetMscReadCbERR);
    PS_PRINTF_INFO("Set Switch CB ERR:          %d\n", g_stAtStatsInfo.stHsUartStats.ucSetSwitchCmdCbERR);
    PS_PRINTF_INFO("Set Water Detect CB ERR:    %d\n", g_stAtStatsInfo.stHsUartStats.ucSetWaterCbERR);
    PS_PRINTF_INFO("Set BaudRate FAIL NUM:      %d\n", g_stAtStatsInfo.stHsUartStats.ulSetBaudRateFailNum);
    PS_PRINTF_INFO("Set WLEN FAIL NUM:          %d\n", g_stAtStatsInfo.stHsUartStats.ulSetWlenFailNum);
    PS_PRINTF_INFO("Set STP FAIL NUM:           %d\n", g_stAtStatsInfo.stHsUartStats.ulSetStpFailNum);
    PS_PRINTF_INFO("Set PARITY FAIL NUM:        %d\n", g_stAtStatsInfo.stHsUartStats.ulSetParityFailNum);
    PS_PRINTF_INFO("Config FLOW CTL SUCC NUM:   %d\n", g_stAtStatsInfo.stHsUartStats.ulCfgFlowCtrlSuccNum);
    PS_PRINTF_INFO("Config FLOW CTL FAIL NUM:   %d\n", g_stAtStatsInfo.stHsUartStats.ulCfgFlowCtrlFailNum);
    PS_PRINTF_INFO("Clear BUFF SUCC NUM:        %d\n", g_stAtStatsInfo.stHsUartStats.ulClearBuffSuccNum);
    PS_PRINTF_INFO("Clear BUFF FAIL NUM:        %d\n", g_stAtStatsInfo.stHsUartStats.ulClearBuffFailNum);
    PS_PRINTF_INFO("MSC Read CB NUM:            %d\n", g_stAtStatsInfo.stHsUartStats.ulMscReadCBNum);
    PS_PRINTF_INFO("MSC Write SUCC NUM:         %d\n", g_stAtStatsInfo.stHsUartStats.ulMscWriteSuccNum);
    PS_PRINTF_INFO("MSC Write FAIL NUM:         %d\n", g_stAtStatsInfo.stHsUartStats.ulMscWriteFailNum);
    PS_PRINTF_INFO("MSC Switch CMD CB NUM:      %d\n", g_stAtStatsInfo.stHsUartStats.ulSwitchCmdCBNum);

    return;
}


VOS_VOID AT_ShowHsUartDataStats(VOS_VOID)
{
    PS_PRINTF_INFO("HSUART UL DATA STATS                \n");
    PS_PRINTF_INFO("UL Read CB NUM:             %d\n", g_stAtStatsInfo.stHsUartStats.ulUlDataReadCBNum);
    PS_PRINTF_INFO("UL RD SUCC NUM:             %d\n", g_stAtStatsInfo.stHsUartStats.ulUlGetRDSuccNum);
    PS_PRINTF_INFO("UL RD Fail NUM:             %d\n", g_stAtStatsInfo.stHsUartStats.ulUlGetRDFailNum);
    PS_PRINTF_INFO("UL Invalid RD NUM:          %d\n", g_stAtStatsInfo.stHsUartStats.ulUlInvalidRDNum);
    PS_PRINTF_INFO("UL Invalid CMD DATA NUM:    %d\n", g_stAtStatsInfo.stHsUartStats.ulUlRcvInvalidCmdNum);
    PS_PRINTF_INFO("UL Valid CMD DATA NUM:      %d\n", g_stAtStatsInfo.stHsUartStats.ulUlValidCmdNum);
    PS_PRINTF_INFO("UL IP Data NUM:             %d\n", g_stAtStatsInfo.stHsUartStats.ulUlIpDataNum);
    PS_PRINTF_INFO("UL PPP Data NUM:            %d\n", g_stAtStatsInfo.stHsUartStats.ulUlPppDataNum);
    PS_PRINTF_INFO("UL CSD Data NUM:            %d\n", g_stAtStatsInfo.stHsUartStats.ulUlOmDataNum);
    PS_PRINTF_INFO("UL OM Data NUM:             %d\n", g_stAtStatsInfo.stHsUartStats.ulUlOmDataNum);
    PS_PRINTF_INFO("UL DIAG Data NUM:           %d\n", g_stAtStatsInfo.stHsUartStats.ulUlDiagDataNum);
    PS_PRINTF_INFO("UL Invalid MODE DATA NUM:   %d\n", g_stAtStatsInfo.stHsUartStats.ulUlInvalidModeDataNum);
    PS_PRINTF_INFO("UL Retrun BUFF SUCC NUM:    %d\n", g_stAtStatsInfo.stHsUartStats.ulUlReturnBuffSuccNum);
    PS_PRINTF_INFO("UL Retrun BUFF FAIL NUM:    %d\n", g_stAtStatsInfo.stHsUartStats.ulUlReturnBuffFailNum);
    PS_PRINTF_INFO("HSUART DL DATA STATS                \n");
    PS_PRINTF_INFO("DL Write Async SUCC NUM:    %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteAsyncSuccNum);
    PS_PRINTF_INFO("DL Write Async FAIL NUM:    %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteAsyncFailNum);
    PS_PRINTF_INFO("DL Write Sync SUCC NUM:     %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteSyncSuccNum);
    PS_PRINTF_INFO("DL Write Sync FAIL NUM:     %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteSyncFailNum);
    PS_PRINTF_INFO("DL Write Sync SUCC LEN:     %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteSyncSuccLen);
    PS_PRINTF_INFO("DL Write Sync FAIL LEN:     %d\n", g_stAtStatsInfo.stHsUartStats.ulDlWriteSyncFailLen);
    PS_PRINTF_INFO("DL Free BUFF NUM:           %d\n", g_stAtStatsInfo.stHsUartStats.ulDlFreeBuffNum);

    return;
}


VOS_VOID AT_ShowHsUartFcState(VOS_VOID)
{
    VOS_CHAR                            acFcStateStr[][20] = {"START", "STOP"};
    AT_IO_LEVEL_ENUM_UINT8              enIoLevel;

    enIoLevel = AT_GetIoLevel(AT_CLIENT_TAB_HSUART_INDEX, IO_CTRL_CTS);

    PS_PRINTF_INFO("HSUART FLOW CTRL STATE              \n");
    PS_PRINTF_INFO("FLOW CTRL STATE:            %s\n", acFcStateStr[enIoLevel]);

    return;
}
#endif


VOS_VOID AT_ShowModemDataStats(VOS_VOID)
{
    PS_PRINTF_INFO("MODEM UL DATA STATS                 \n");
    PS_PRINTF_INFO("UL Read CB NUM:             %d\n", g_stAtStatsInfo.stModemStats.ulUlDataReadCBNum);
    PS_PRINTF_INFO("UL RD SUCC NUM:             %d\n", g_stAtStatsInfo.stModemStats.ulUlGetRDSuccNum);
    PS_PRINTF_INFO("UL RD Fail NUM:             %d\n", g_stAtStatsInfo.stModemStats.ulUlGetRDFailNum);
    PS_PRINTF_INFO("UL Invalid RD NUM:          %d\n", g_stAtStatsInfo.stModemStats.ulUlInvalidRDNum);
    PS_PRINTF_INFO("UL Retrun BUFF SUCC NUM:    %d\n", g_stAtStatsInfo.stModemStats.ulUlReturnBuffSuccNum);
    PS_PRINTF_INFO("UL Retrun BUFF FAIL NUM:    %d\n", g_stAtStatsInfo.stModemStats.ulUlReturnBuffFailNum);
    PS_PRINTF_INFO("MODEM DL DATA STATS                 \n");
    PS_PRINTF_INFO("DL Write Async SUCC NUM:    %d\n", g_stAtStatsInfo.stModemStats.ulDlWriteAsyncSuccNum);
    PS_PRINTF_INFO("DL Write Async FAIL NUM:    %d\n", g_stAtStatsInfo.stModemStats.ulDlWriteAsyncFailNum);
    PS_PRINTF_INFO("DL Free BUFF NUM:           %d\n", g_stAtStatsInfo.stModemStats.ulDlFreeBuffNum);


    return;
}


VOS_VOID AT_ShowPsEntityInfo(VOS_UINT8 ucCallId)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
#if (FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                           aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                           aucIpv6PrimDnsStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                           aucIpv6SecDnsStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
#endif

    for (enModemId = 0; enModemId < MODEM_ID_BUTT; enModemId++)
    {
        PS_PRINTF_INFO("MODEM ID is %d                \n", enModemId);
        pstCallEntity = &(AT_GetModemPsCtxAddrFromModemId(enModemId)->astCallEntity[ucCallId]);

        PS_PRINTF_INFO("PS Call Entity Info                  \n");
        PS_PRINTF_INFO("Active Flag                             %d\n", pstCallEntity->ulUsedFlg);
        PS_PRINTF_INFO("PS CALL TYPE                            %d\n", pstCallEntity->enPsCallType);
        PS_PRINTF_INFO("Current Call Type                       %d\n", pstCallEntity->stDialPdpType.enCurrPdpType);
        PS_PRINTF_INFO("Wlan Call Type                          %d\n", pstCallEntity->stDialPdpType.enWlanPdpType);
        PS_PRINTF_INFO("HandOver Call Type                      %d\n", pstCallEntity->stDialPdpType.enHoPdpType);
        PS_PRINTF_INFO("User Call Entity Info                \n");
        PS_PRINTF_INFO("User Actual Index                       %d\n", pstCallEntity->stUserInfo.enPortIndex);
        PS_PRINTF_INFO("User Expect Index                       %d\n", pstCallEntity->stUserInfo.enUserIndex);
        PS_PRINTF_INFO("User Type                               %d\n", pstCallEntity->stUserInfo.ucUsrType);
        PS_PRINTF_INFO("User CID                                %d\n", pstCallEntity->stUserInfo.ucUsrCid);
        PS_PRINTF_INFO("IPv4 Connect Info                    \n");
        PS_PRINTF_INFO("IPv4 Bearer CID                         %d\n", pstCallEntity->stIpv4Info.ucIpv4Cid);
        PS_PRINTF_INFO("IPv4 Bearer State                       %d\n", pstCallEntity->stIpv4Info.enIpv4State);
        PS_PRINTF_INFO("IPv4 Bearer Wlan State                  %d\n", pstCallEntity->stIpv4Info.enWlanIpv4State);
        PS_PRINTF_INFO("IPv4 Bearer ID (RAB ID)                 %d\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ucRabId);
        PS_PRINTF_INFO("IPv4 Bearer Address                     0x%x\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr);
        PS_PRINTF_INFO("IPv4 Bearer Primary DNS                 0x%x\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS);
        PS_PRINTF_INFO("IPv4 Bearer Secondary DNS               0x%x\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS);
        PS_PRINTF_INFO("IPv4 Bearer Gate Way                    0x%x\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4GateWay);
        PS_PRINTF_INFO("IPv4 Bearer NET Mask                    0x%x\n", pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4NetMask);

#if (FEATURE_ON == FEATURE_IPV6)
        TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, sizeof(aucIpv6AddrStr));
        TAF_MEM_SET_S(aucIpv6PrimDnsStr, sizeof(aucIpv6PrimDnsStr), 0x00, sizeof(aucIpv6PrimDnsStr));
        TAF_MEM_SET_S(aucIpv6SecDnsStr, sizeof(aucIpv6SecDnsStr), 0x00, sizeof(aucIpv6SecDnsStr));

        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        AT_ConvertIpv6AddrToCompressedStr(aucIpv6PrimDnsStr,
                                          pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        AT_ConvertIpv6AddrToCompressedStr(aucIpv6SecDnsStr,
                                          pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        PS_PRINTF_INFO("IPv6 Connect Info                         \n");
        PS_PRINTF_INFO("IPv6 Bearer CID                         %d\n", pstCallEntity->stIpv6Info.ucIpv6Cid);
        PS_PRINTF_INFO("IPv6 Bearer State                       %d\n", pstCallEntity->stIpv6Info.enIpv6State);
        PS_PRINTF_INFO("IPv6 Bearer Wlan State                  %d\n", pstCallEntity->stIpv6Info.enWlanIpv6State);
        PS_PRINTF_INFO("IPv6 Bearer ID (RAB ID)                 %d\n", pstCallEntity->stIpv6Info.stIpv6DhcpInfo.ucRabId);
        PS_PRINTF_INFO("IPv6 Bearer Global Address              %s\n", aucIpv6AddrStr);
        PS_PRINTF_INFO("IPv6 Bearer Primary DNS                 %s\n", aucIpv6PrimDnsStr);
        PS_PRINTF_INFO("IPv6 Bearer Secondary DNS               %s\n", aucIpv6SecDnsStr);
#endif
        PS_PRINTF_INFO("Domain Select Info                       \n");
        PS_PRINTF_INFO("DataSysInfoFlg                         %d\n", pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg);
        PS_PRINTF_INFO("DataSysPolicyIndex                     %d\n", pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);
        PS_PRINTF_INFO("CurrentDataSys                         %d\n", pstCallEntity->stApnDataSysInfo.enCurrentDataSys);
        PS_PRINTF_INFO("IsHandOver                             %d\n", pstCallEntity->stApnDataSysInfo.ucIsHandOver);
        PS_PRINTF_INFO("HoFailCause                            %d\n", pstCallEntity->stApnDataSysInfo.enHoFailCause);
    }

    return;
}


VOS_VOID AT_ShowPsFcIdState(VOS_UINT32 ulFcid)
{
    if (ulFcid >= FC_ID_BUTT)
    {
        PS_PRINTF_WARNING("<AT_ShowFcStatsInfo> Fcid overtop, ulFcid = %d\n", ulFcid);
        return;
    }

    PS_PRINTF_INFO("Indicate the validity of the corresponding node of FCID,      Used = %d\n", g_stFcIdMaptoFcPri[ulFcid].ulUsed);
    PS_PRINTF_INFO("Indicate the priority of the corresponding node of FCID,      FcPri = %d\n", g_stFcIdMaptoFcPri[ulFcid].enFcPri);
    PS_PRINTF_INFO("Indicate the RABID mask of the corresponding node of FCID,    RabIdMask = %d\n", g_stFcIdMaptoFcPri[ulFcid].ulRabIdMask);
    PS_PRINTF_INFO("Indicate the ModemId of the corresponding node of FCID,       ModemId = %d\n", g_stFcIdMaptoFcPri[ulFcid].enModemId);

    return;
}



VOS_VOID AT_ShowAllClientState(VOS_VOID)
{
    VOS_UINT8                           i;
    VOS_CHAR                            acStateStr[][20] = {"READY", "PEND"};
    VOS_CHAR                            acModeStr[][20] = {"CMD", "DATA", "ONLINE_CMD"};

    PS_PRINTF_INFO("The All Client State: \n");

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        PS_PRINTF_INFO("Client[%d] State: %s    Mode: %s\n", i,
            acStateStr[g_stParseContext[i].ucClientStatus], acModeStr[gastAtClientTab[i].Mode]);
    }

    return;
}


VOS_VOID AT_SetClientState(VOS_UINT8 ucIndex, VOS_UINT8 ucState)
{
    if ((ucIndex >= AT_MAX_CLIENT_NUM)||(ucState > AT_FW_CLIENT_STATUS_PEND))
    {
        return;
    }

    g_stParseContext[ucIndex].ucClientStatus = ucState;

    return;
}


VOS_VOID AT_SetClientMode(VOS_UINT8 ucIndex, VOS_UINT8 ucMode)
{
    if ((ucIndex >= AT_MAX_CLIENT_NUM)||(ucMode > AT_ONLINE_CMD_MODE))
    {
        return;
    }

    gastAtClientTab[ucIndex].Mode = ucMode;

    return;
}


VOS_VOID AT_ShowUsedClient(VOS_VOID)
{
    AT_PORT_BUFF_CFG_STRU              *pstPortCfg;
    VOS_UINT32                          ulIndex;
    VOS_UINT8                           i;
    VOS_CHAR                            acStateStr[][20] = {"READY", "PEND"};
    VOS_CHAR                            acModeStr[][20] = {"CMD", "DATA", "ONLINE_CMD"};

    pstPortCfg = AT_GetPortBuffCfgInfo();

    PS_PRINTF_INFO("The All Used Client State: \n");
    for (i = 0; i < pstPortCfg->ucNum; i++)
    {
        ulIndex = pstPortCfg->ulUsedClientID[i];
        PS_PRINTF_INFO("Client[%d] State: %s    Mode: %s\n", ulIndex,
            acStateStr[g_stParseContext[ulIndex].ucClientStatus], acModeStr[gastAtClientTab[ulIndex].Mode]);
    }

    return;
}



VOS_VOID AT_ShowClientCtxInfo(VOS_VOID)
{
    VOS_UINT8                           i;
    AT_CLIENT_CFG_MAP_TAB_STRU         *pstCfgMapTbl;
    AT_CLIENT_CONFIGURATION_STRU       *pstClientCfg;

    PS_PRINTF_INFO("The All Used Client Config: \n");
    for (i = 0; i < AT_GET_CLIENT_CFG_TAB_LEN(); i++)
    {
        pstCfgMapTbl = AT_GetClientCfgMapTbl(i);
        pstClientCfg = AT_GetClientConfig(pstCfgMapTbl->enClientId);
        PS_PRINTF_INFO("Client[%s] modem:%d, reportFlag:%d\n",
                   pstCfgMapTbl->aucPortName,
                   pstClientCfg->enModemId,
                   pstClientCfg->ucReportFlg);
    }
}

#if (FEATURE_ON == FEATURE_IPV6)

VOS_VOID AT_ShowIPv6IIDMgrInfo(VOS_VOID)
{

    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo = VOS_NULL_PTR;
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry   = VOS_NULL_PTR;
    HI_LIST_S                          *pstMe      = VOS_NULL_PTR;
    VOS_UINT8                           aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT32                          i;


    PS_PRINTF_INFO("IPv6 IID MANGER INFO                         \n");

    for (i = 0; i < MODEM_ID_BUTT; i++)
    {
        pstMgrInfo = AT_GetPsIPv6IIDMgrInfoByModemId((VOS_UINT16)i);

        msp_list_for_each(pstMe, &(pstMgrInfo->stListHead))
        {
            pstEntry = msp_list_entry(pstMe, AT_PS_IPV6_IID_ENTRY_STRU, stList);

            TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr),
                          0x00, (TAF_MAX_IPV6_ADDR_COLON_STR_LEN));
            AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                              pstEntry->aucPrefix,
                                              TAF_IPV6_STR_RFC2373_TOKENS);

            PS_PRINTF_INFO("[MODEM%d] APN            %s\n", i, pstEntry->aucApn);
            PS_PRINTF_INFO("[MODEM%d] IPv6 Prefix    %s\n", i, aucIpv6AddrStr);
            PS_PRINTF_INFO("[MODEM%d] IPv6 IID       0x%llx\n", i, pstEntry->ullIID);
        }
    }

    return;
}
#endif


VOS_VOID AT_Help(VOS_VOID)
{
    PS_PRINTF_INFO("PS Debug Info               \n");
    PS_PRINTF_INFO("<AT_ShowPsEntityInfo>         Show PS Call Entity Info           \n");
    PS_PRINTF_INFO("<AT_ShowPsFcIdState(ulFcid)>  Show Flow Control Node Info        \n");
    PS_PRINTF_INFO("UART Debug Info             \n");
    PS_PRINTF_INFO("<AT_ShowHsUartConfigInfo>     Show HSUART Config Info            \n");
    PS_PRINTF_INFO("<AT_ShowHsUartNvStats>        Show HSUART NV Stats              \n");
    PS_PRINTF_INFO("<AT_ShowHsUartIoctlStats>     Show HSUART IOCTL Stats           \n");
    PS_PRINTF_INFO("<AT_ShowHsUartDataStats>      Show HAUART Data Stats            \n");
    PS_PRINTF_INFO("<AT_ShowHsUartFcState>        Show HAUART Flow Control State     \n");
    PS_PRINTF_INFO("MODEM Debug Info            \n");
    PS_PRINTF_INFO("<AT_ShowModemDataStats>       Show Modem Data Stats             \n");
    PS_PRINTF_INFO("Others Debug Info           \n");
    PS_PRINTF_INFO("<AT_ShowAllClientState>       Show All Client Port State         \n");
    PS_PRINTF_INFO("<AT_ShowClientCtxInfo>        Show Client Configure Context Info \n");
    PS_PRINTF_INFO("<AT_ShowIPv6IIDMgrInfo>       Show IPv6 Interface ID Manage Info \n");

    return;
}


VOS_VOID AT_ShowCurrentDataSys(VOS_UINT16 usModemId)
{
    TAF_PS_DATA_SYSTEM_STATUS_STRU     *pstDataSystemStatus = VOS_NULL_PTR;

    pstDataSystemStatus = &(g_stAtCommCtx.stPsCtx.astDataSystemStatus[usModemId]);

    PS_PRINTF_INFO("Sys Domain State Info         \n");
    PS_PRINTF_INFO("CellularPowerState:      %d\n", pstDataSystemStatus->bCellularPowerState);
    PS_PRINTF_INFO("CellularRoamingStatus:   %d\n", pstDataSystemStatus->bCellularRoamingStatus);
    PS_PRINTF_INFO("RatType:                 %d\n", pstDataSystemStatus->enRatType);
    PS_PRINTF_INFO("PsServiceStatus:         %d\n", pstDataSystemStatus->enPsServiceStatus);
    PS_PRINTF_INFO("WlanPowerState:          %d\n", pstDataSystemStatus->bWlanPowerState);
    PS_PRINTF_INFO("WiFiServiceStatu:        %d\n", pstDataSystemStatus->enWiFiServiceStatus);

    return;
}



