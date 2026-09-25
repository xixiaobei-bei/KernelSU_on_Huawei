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
#include "securec.h"



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

AT_MNTN_PORT_INFO_LOG_STRU              g_astAtMntnPortInfo[AT_MNTN_MAX_PORT_NUM];

VOS_UINT8                              *gpucAtExcAddr    = VOS_NULL_PTR;

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
    memset_s(&g_stAtMsgRecordInfo, sizeof(g_stAtMsgRecordInfo), 0x00, sizeof(g_stAtMsgRecordInfo));
    memset_s(g_astAtMntnPortInfo, sizeof(g_astAtMntnPortInfo), 0x00, sizeof(g_astAtMntnPortInfo));
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
#if (FEATURE_VCOM_EXT == FEATURE_ON)
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
#if  (FEATURE_VCOM_EXT == FEATURE_ON)
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
    if ((ucFlag == 0) || (ucFlag == 1))
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

    if (AT_GetPrivacyFilterEnableFlg() == VOS_TRUE)
    {
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        if (GUNAS_FilterAtToAtMsg((PS_MSG_HEADER_STRU *)pMsg) == VOS_NULL_PTR)
        {
            return;
        }

        /* cnas at命令脱敏过滤函数处理 */
        pLogPrivacyMsg = AT_PrivacyMatchAtCmd(pMsg);
        if (pLogPrivacyMsg == VOS_NULL_PTR)
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
    errno_t                             lMemResult;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnMsc, ID_AT_MNTN_INPUT_MSC, (sizeof(AT_MNTN_MSC_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnMsc.ulPortId = ucIndex;
    lMemResult = memcpy_s(&(stMntnMsc.stDceMscInfo), sizeof(stMntnMsc.stDceMscInfo), pstDceMsc, sizeof(AT_DCE_MSC_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stMntnMsc.stDceMscInfo), sizeof(AT_DCE_MSC_STRU));

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
    errno_t                             lMemResult;

    /* 填写消息头 */
    AT_MNTN_CFG_MSG_HDR(&stMntnMsc, ID_AT_MNTN_OUTPUT_MSC, (sizeof(AT_MNTN_MSC_STRU) - VOS_MSG_HEAD_LENGTH));

    /* 填写消息内容 */
    stMntnMsc.ulPortId = ucIndex;
    lMemResult = memcpy_s(&(stMntnMsc.stDceMscInfo), sizeof(stMntnMsc.stDceMscInfo), pstDceMsc, sizeof(AT_DCE_MSC_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stMntnMsc.stDceMscInfo), sizeof(AT_DCE_MSC_STRU));

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
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;
    AT_INTER_MSG_ID_ENUM_UINT32         enEventId;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;
    MODEM_ID_ENUM_UINT16                enModemId;

    AT_GetAtMsgStruMsgLength(usDataLen, &ulLength);

    /* 申请消息内存 */
    pstMsg = (AT_MSG_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, (ulLength + VOS_MSG_HEAD_LENGTH));
    if (pstMsg == VOS_NULL_PTR)
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
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        enModemId = MODEM_ID_0;
    }

    pstMsg->enModemId       = (VOS_UINT8)enModemId;
    pstMsg->enVersionId     = 0xAA;

    pstMsg->ucFilterAtType  = (VOS_UINT8)g_enLogPrivacyAtCmd;

    AT_GetUserTypeFromIndex(ucIndex, &pstMsg->ucUserType);

    lMemResult = memcpy_s((TAF_VOID*)pstMsg->aucValue, usDataLen, pucData, usDataLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usDataLen, usDataLen);

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
    errno_t                                 lMemResult;
    MODEM_ID_ENUM_UINT16                    enModemId;
    /*lint -save -e516 */
    pstSndMsgCB = (NAS_AT_SDT_AT_PART_ST *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(NAS_AT_SDT_AT_PART_ST));
    /*lint -restore */
    if (pstSndMsgCB == VOS_NULL_PTR)
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

        lMemResult = memcpy_s(&pstOutsideCtx->gstAtCnmiType,
                              sizeof(pstOutsideCtx->gstAtCnmiType),
                              &(pstSmsCtx->stCnmiType),
                              sizeof(pstSmsCtx->stCnmiType));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstOutsideCtx->gstAtCnmiType), sizeof(pstSmsCtx->stCnmiType));

        lMemResult = memcpy_s(&pstOutsideCtx->g_stAtCscaCsmpInfo,
                              sizeof(pstOutsideCtx->g_stAtCscaCsmpInfo),
                              &(pstSmsCtx->stCscaCsmpInfo),
                              sizeof(pstSmsCtx->stCscaCsmpInfo));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstOutsideCtx->g_stAtCscaCsmpInfo), sizeof(pstSmsCtx->stCscaCsmpInfo));

        lMemResult = memcpy_s(&pstOutsideCtx->g_stAtCpmsInfo,
                              sizeof(pstOutsideCtx->g_stAtCpmsInfo),
                              &(pstSmsCtx->stCpmsInfo),
                              sizeof(pstSmsCtx->stCpmsInfo));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstOutsideCtx->g_stAtCpmsInfo), sizeof(pstSmsCtx->stCpmsInfo));

    }

    AT_MNTN_TraceEvent(pstSndMsgCB);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, pstSndMsgCB);
    /*lint -restore */

    return;
}


VOS_VOID AT_MNTN_TraceClientData(VOS_VOID)
{
    NAS_AT_SDT_AT_CLIENT_TABLE_STRU         *pstSndMsgCB = VOS_NULL_PTR;
    TAF_UINT8                               ucLoop;
    /*lint -save -e516 */
    pstSndMsgCB = (NAS_AT_SDT_AT_CLIENT_TABLE_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(NAS_AT_SDT_AT_CLIENT_TABLE_STRU));
    /*lint -restore */
    if ( pstSndMsgCB == VOS_NULL_PTR )
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
    AT_MNTN_RPTPORT_STRU                   *pstSndMsgCB = VOS_NULL_PTR;
    VOS_UINT8                               ucLoop;
    /*lint -save -e516 */
    pstSndMsgCB = (AT_MNTN_RPTPORT_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                            sizeof(AT_MNTN_RPTPORT_STRU));
    /*lint -restore */
    if ( pstSndMsgCB == VOS_NULL_PTR )
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
    memset_s(&g_stAtStatsInfo.stHsUartStats, sizeof(g_stAtStatsInfo.stHsUartStats), 0x00, sizeof(AT_MNTN_HSUART_STATS_STRU));
    return;
}


VOS_VOID AT_InitModemStats(VOS_VOID)
{
    memset_s(&g_stAtStatsInfo.stModemStats, sizeof(g_stAtStatsInfo.stModemStats), 0x00, sizeof(AT_MNTN_MODEM_STATS_STRU));
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


VOS_UINT8 AT_MNTN_TransferClientIdToMntnIndex(
    VOS_UINT8                           ucClientId
)
{
    VOS_UINT8                           ucMntnIndex = AT_CLIENT_ID_BUTT;

    /*当前仅对pcui ctrl口以及appvcom-appvcom34共37个端口抓取维测*/
    if (ucClientId <= AT_CLIENT_ID_CTRL)
    {
        ucMntnIndex = ucClientId;
    }

    if ((ucClientId >= AT_CLIENT_ID_APP) &&
        (ucClientId < AT_MNTN_MAX_PORT_CLIENT_ID))
    {
        ucMntnIndex = ucClientId - AT_CLIENT_ID_APP + 2;
    }

    return ucMntnIndex;
}



VOS_VOID AT_MNTN_IncRcvStreamCnt(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           ucMntnIndex;

    ucMntnIndex = AT_MNTN_TransferClientIdToMntnIndex(ucIndex);

    if (ucMntnIndex == AT_CLIENT_ID_BUTT)
    {
        return;
    }

    g_astAtMntnPortInfo[ucMntnIndex].ulRcvStreamCnt++;
}


VOS_VOID AT_MNTN_IncSndStreamCnt(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           ucMntnIndex;

    ucMntnIndex = AT_MNTN_TransferClientIdToMntnIndex(ucIndex);

    if (ucMntnIndex == AT_CLIENT_ID_BUTT)
    {
        return;
    }

    g_astAtMntnPortInfo[ucMntnIndex].ulSndStreamCnt++;
}


VOS_VOID AT_MNTN_SaveCurCmdName(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           ucMntnIndex;

    ucMntnIndex = AT_MNTN_TransferClientIdToMntnIndex(ucIndex);

    if (ucMntnIndex == AT_CLIENT_ID_BUTT)
    {
        return;
    }

    if ((g_stATParseCmd.stCmdName.usCmdNameLen > 0) &&
        (g_stATParseCmd.stCmdName.usCmdNameLen <= AT_CMD_NAME_LEN))
    {
        /* 最后保留一个字符串结束符 0 */
        TAF_MEM_CPY_S(g_astAtMntnPortInfo[ucMntnIndex].aucCmdName,
                      sizeof(g_astAtMntnPortInfo[ucMntnIndex].aucCmdName) - 1,
                      g_stATParseCmd.stCmdName.aucCmdName,
                      sizeof(g_astAtMntnPortInfo[ucMntnIndex].aucCmdName) - 1);
    }
}


VOS_VOID AT_MNTN_SaveExcLog(VOS_VOID)
{
    AT_MNTN_SAVE_EXC_LOG_STRU          *pstExcLogBuffer = VOS_NULL_PTR;
    VOS_UINT32                          ulAtClientId;
    VOS_UINT32                          i;

    /* 申请失败，直接退出 */
    if (gpucAtExcAddr == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_MNTN_SaveExcLog:No memory allocated");
        return;
    }

    TAF_MEM_SET_S(gpucAtExcAddr, AT_MNTN_SAVE_EXC_LOG_LENGTH, 0, AT_MNTN_SAVE_EXC_LOG_LENGTH);

    pstExcLogBuffer = (AT_MNTN_SAVE_EXC_LOG_STRU *)gpucAtExcAddr;
    pstExcLogBuffer->ulBeginTag = AT_MNTN_DUMP_BEGIN_TAG;
    pstExcLogBuffer->ulEndTag   = AT_MNTN_DUMP_END_TAG;

    TAF_MEM_CPY_S(&(pstExcLogBuffer->stMsgInfo),
                  sizeof(pstExcLogBuffer->stMsgInfo),
                  &g_stAtMsgRecordInfo,
                  sizeof(pstExcLogBuffer->stMsgInfo));

    TAF_MEM_CPY_S(&pstExcLogBuffer->astPortInfo[0],
                  sizeof(pstExcLogBuffer->astPortInfo),
                  g_astAtMntnPortInfo,
                  sizeof(g_astAtMntnPortInfo));

    ulAtClientId = AT_CLIENT_ID_PCUI;

    for (i = 0; i < AT_MNTN_MAX_PORT_NUM; i++)
    {
        if (i == 0)
        {
            ulAtClientId = AT_CLIENT_ID_PCUI;
        }

        /* 仅统计pcui、ctrl以及appvcom0到appvcom34端口，所以当index为2时，需要把clientid置位app */
        if (i == 2)
        {
            ulAtClientId = AT_CLIENT_ID_APP;
        }

        pstExcLogBuffer->astPortInfo[i].ulClientStatus = g_stParseContext[ulAtClientId].ucClientStatus;
        ulAtClientId++;
    }
}


VOS_VOID AT_RegisterDumpCallBack(VOS_VOID)
{
    /* 调用底软接口进行申请内存 */
    /*分配内存*/
    gpucAtExcAddr = (VOS_UINT8 *)mdrv_om_register_field(OM_AP_AT,
                                                       "AT dump",
                                                       VOS_NULL_PTR,
                                                       VOS_NULL_PTR,
                                                       AT_MNTN_SAVE_EXC_LOG_LENGTH,
                                                       0);

    /* 申请失败打印异常信息，内存申请异常时仍然注册，回调函数中有地址申请失败保护 */
    if (gpucAtExcAddr == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_RegisterDumpCallBack:No memory allocated");
    }

    (VOS_VOID)mdrv_om_register_callback("AT_EXCLOG", (dump_hook)AT_MNTN_SaveExcLog);

}

#if (FEATURE_AT_HSUART == FEATURE_ON)

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
    AT_PORT_BUFF_CFG_STRU              *pstPortCfg = VOS_NULL_PTR;
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
    AT_CLIENT_CFG_MAP_TAB_STRU         *pstCfgMapTbl = VOS_NULL_PTR;
    AT_CLIENT_CONFIGURATION_STRU       *pstClientCfg = VOS_NULL_PTR;

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



