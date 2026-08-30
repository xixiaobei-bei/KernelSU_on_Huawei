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
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_ADS_DEBUG_C


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AdsDebug.h"
#include "AdsCtx.h"



/*****************************************************************************
  2 外部函数声明
*****************************************************************************/


/******************************************************************************
   3 私有定义
******************************************************************************/


/******************************************************************************
   4 全局变量定义
******************************************************************************/

ADS_STATS_INFO_STRU                     g_stAdsStats;
ADS_LOG_LEVEL_ENUM_UINT32               g_enAdsLogLevel = ADS_LOG_LEVEL_ERROR;


/******************************************************************************
   5 函数实现
******************************************************************************/


VOS_VOID ADS_SetTxWakeLockTmrLen(VOS_UINT32 ulValue)
{
    g_stAdsCtx.stAdsIpfCtx.ulTxWakeLockTmrLen = ulValue;
    return;
}


VOS_VOID ADS_SetRxWakeLockTmrLen(VOS_UINT32 ulValue)
{
    g_stAdsCtx.stAdsIpfCtx.ulRxWakeLockTmrLen = ulValue;
    return;
}


VOS_VOID ADS_SetMaxQueueLength(
    VOS_UINT32                          ulLength,
    VOS_UINT32                          ulInstanceIndex
)
{
    g_stAdsCtx.astAdsSpecCtx[ulInstanceIndex].stAdsUlCtx.ulUlMaxQueueLength = ulLength;
}


VOS_VOID ADS_ShowEntityStats(VOS_VOID)
{
    VOS_UINT32                           i;
    VOS_UINT32                           j;

    for (i = 0; i < ADS_INSTANCE_MAX_NUM; i++)
    {
        PS_PRINTF_INFO("ADS Modem ID %d\n", i);

        for (j = ADS_RAB_ID_MIN; j <= ADS_RAB_ID_MAX; j++)
        {
            if (VOS_NULL_PTR != g_stAdsCtx.astAdsSpecCtx[i].stAdsUlCtx.astAdsUlQueue[j].pstAdsUlLink)
            {
                PS_PRINTF_INFO("ADS Queue length is %d\n", IMM_ZcQueueLen(g_stAdsCtx.astAdsSpecCtx[i].stAdsUlCtx.astAdsUlQueue[j].pstAdsUlLink));
            }

            PS_PRINTF_INFO("ADS DL RabId is %d\n", g_stAdsCtx.astAdsSpecCtx[i].stAdsDlCtx.astAdsDlRabInfo[j - ADS_RAB_ID_OFFSET].ulRabId);
        }
    }

    PS_PRINTF_INFO("ADS UL is sending flag, SendingFlg = %d\n",g_stAdsCtx.stAdsIpfCtx.ucSendingFlg);


    return;
}


VOS_VOID ADS_ShowEventProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS EVENT PROC STATS START:              \n");
    PS_PRINTF_INFO("ULQueNonEmptyTrigEvent           %10u\n", g_stAdsStats.stUlComStatsInfo.ulULQueNonEmptyTrigEvent);
    PS_PRINTF_INFO("ULQueFullTrigEvent               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULQueFullTrigEvent);
    PS_PRINTF_INFO("ULQueHitThresTrigEvent           %10u\n", g_stAdsStats.stUlComStatsInfo.ulULQueHitThresTrigEvent);
    PS_PRINTF_INFO("ULTmrHitThresTrigEvent           %10u\n", g_stAdsStats.stUlComStatsInfo.ulULTmrHitThresTrigEvent);
    PS_PRINTF_INFO("UL10MsTmrTrigEvent               %10u\n", g_stAdsStats.stUlComStatsInfo.ulUL10MsTmrTrigEvent);
    PS_PRINTF_INFO("ULSpeIntTrigEvent                %10u\n", g_stAdsStats.stUlComStatsInfo.ulULSpeIntTrigEvent);
    PS_PRINTF_INFO("ULProcEventNum                   %10u\n", g_stAdsStats.stUlComStatsInfo.ulULProcEventNum);
    PS_PRINTF_INFO("DLRcvIpfRdIntNum                 %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRcvIpfRdIntNum);
    PS_PRINTF_INFO("DLCCoreResetTrigRdEvent          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLCCoreResetTrigEvent);
    PS_PRINTF_INFO("DLProcIpfRdEventNum              %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLProcIpfRdEventNum);
    PS_PRINTF_INFO("DLRcvIpfAdqEmptyIntNum           %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRcvIpfAdqEmptyIntNum);
    PS_PRINTF_INFO("DLRecycleMemTrigAdEvent          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRecycleMemTrigEvent);
    PS_PRINTF_INFO("DLProcIpfAdEventNum              %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLProcIpfAdEventNum);
    PS_PRINTF_INFO("ADS EVENT PROC STATS END.                \n");

    return;
}


VOS_VOID ADS_ShowULPktProcStats(VOS_VOID)
{
    VOS_UINT32                           ulInstance;

    PS_PRINTF_INFO("ADS UL PKT PROC STATS START:             \n");
    PS_PRINTF_INFO("ULRmnetRxPktNum                  %10u\n", g_stAdsStats.stUlComStatsInfo.ulULRmnetRxPktNum);
    PS_PRINTF_INFO("ULRmnetModemIdErrNum             %10u\n", g_stAdsStats.stUlComStatsInfo.ulULRmnetModemIdErrNum);
    PS_PRINTF_INFO("ULRmnetRabIdErrNum               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULRmnetRabIdErrNum);
    PS_PRINTF_INFO("ULRmnetEnQueSuccNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULRmnetEnQueSuccNum);
    PS_PRINTF_INFO("ULRmnetEnQueFailNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULRmnetEnQueFailNum);
    PS_PRINTF_INFO("ULPktEnQueSuccNum                %10u\n", g_stAdsStats.stUlComStatsInfo.ulULPktEnQueSuccNum);
    PS_PRINTF_INFO("ULPktEnQueFailNum                %10u\n", g_stAdsStats.stUlComStatsInfo.ulULPktEnQueFailNum);

    for (ulInstance = 0; ulInstance < ADS_INSTANCE_MAX_NUM; ulInstance++)
    {
        PS_PRINTF_INFO("ULBuffPktNum[MDOEM:%d]            %10u\n", ulInstance, ADS_UL_GetInstanceAllQueueDataNum(ulInstance));
    }

    PS_PRINTF_INFO("ULBuffThresholdCurrent           %10u\n", g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum);
    PS_PRINTF_INFO("ULBuffThreshold1                 %10u\n", g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold1);
    PS_PRINTF_INFO("ULBuffThreshold2                 %10u\n", g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold1);
    PS_PRINTF_INFO("ULBuffThreshold3                 %10u\n", g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold1);
    PS_PRINTF_INFO("ULBuffThreshold4                 %10u\n", g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold1);
    PS_PRINTF_INFO("ULWaterLevel1HitNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULWmLevel1HitNum);
    PS_PRINTF_INFO("ULWaterLevel2HitNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULWmLevel2HitNum);
    PS_PRINTF_INFO("ULWaterLevel3HitNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULWmLevel3HitNum);
    PS_PRINTF_INFO("ULWaterLevel4HitNum              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULWmLevel4HitNum);
    PS_PRINTF_INFO("ADS UL PKT PROC STATS END.               \n");

    return;
}


VOS_VOID ADS_ShowULBdProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS UL BD PROC STATS START:              \n");
    PS_PRINTF_INFO("ULBdqCfgIpfHaveNoBd              %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfHaveNoBd);
    PS_PRINTF_INFO("ULBdqCfgBdSuccNum                %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqCfgBdSuccNum);
    PS_PRINTF_INFO("ULBdqCfgBdFailNum                %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqCfgBdFailNum);
    PS_PRINTF_INFO("ULBdqCfgIpfSuccNum               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfSuccNum);
    PS_PRINTF_INFO("ULBdqCfgIpfFailNum               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfFailNum);
    PS_PRINTF_INFO("ULBdqSaveSrcMemNum               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqSaveSrcMemNum);
    PS_PRINTF_INFO("ULBdqFreeSrcMemNum               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqFreeSrcMemNum);
    PS_PRINTF_INFO("ULBdqFreeSrcMemErr               %10u\n", g_stAdsStats.stUlComStatsInfo.ulULBdqFreeSrcMemErr);
    PS_PRINTF_INFO("ADS UL BD PROC STATS END.                \n");

    return;
}


VOS_VOID ADS_ShowDLPktProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DL PKT PROC STATS START:             \n");
    PS_PRINTF_INFO("DLRmnetTxPktNum                  %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRmnetTxPktNum);
    PS_PRINTF_INFO("DLRmnetModemIdErrNum             %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRmnetModemIdErrNum);
    PS_PRINTF_INFO("DLRmnetRabIdErrNum               %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRmnetRabIdErrNum);
    PS_PRINTF_INFO("DLRmnetNoFuncFreePktNum          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRmnetNoFuncFreePktNum);
    PS_PRINTF_INFO("ADS DL PKT PROC STATS END.               \n");

    return;
 }


VOS_VOID ADS_ShowDLRdProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DL RD PROC STATS START:              \n");
    PS_PRINTF_INFO("DLRdqRxRdNum                     %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqRxRdNum);
    PS_PRINTF_INFO("DLRdqGetRd0Num                   %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqGetRd0Num);
    PS_PRINTF_INFO("DLRdqTransMemFailNum             %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqTransMemFailNum);
    PS_PRINTF_INFO("DLRdqRxNormPktNum                %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqRxNormPktNum);
    PS_PRINTF_INFO("DLRdqRxNdPktNum                  %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqRxNdPktNum);
    PS_PRINTF_INFO("DLRdqRxDhcpPktNum                %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqRxDhcpPktNum);
    PS_PRINTF_INFO("DLRdqRxErrPktNum                 %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqRxErrPktNum);
    PS_PRINTF_INFO("DLRdqFilterErrNum                %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLRdqFilterErrNum);
    PS_PRINTF_INFO("ADS DL RD PROC STATS END.                \n");

    return;
}


VOS_VOID ADS_ShowDLAdProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DL ADQ PROC STATS START:             \n");
    PS_PRINTF_INFO("DLAdqAllocSysMemSuccNum          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocSysMemSuccNum);
    PS_PRINTF_INFO("DLAdqAllocSysMemFailNum          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocSysMemFailNum);
    PS_PRINTF_INFO("DLAdqAllocMemSuccNum             %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocMemSuccNum);
    PS_PRINTF_INFO("DLAdqAllocMemFailNum             %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocMemFailNum);
    PS_PRINTF_INFO("DLAdqFreeMemNum                  %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeMemNum);
    PS_PRINTF_INFO("DLAdqRecycleMemSuccNum           %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqRecycleMemSuccNum);
    PS_PRINTF_INFO("DLAdqRecycleMemFailNum           %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqRecycleMemFailNum);
    PS_PRINTF_INFO("DLAdqGetFreeAdSuccNum            %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqGetFreeAdSuccNum);
    PS_PRINTF_INFO("DLAdqGetFreeAdFailNum            %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqGetFreeAdFailNum);
    PS_PRINTF_INFO("DLAdqCfgAdNum                    %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAdNum);
    PS_PRINTF_INFO("DLAdqCfgAd0Num                   %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd0Num);
    PS_PRINTF_INFO("DLAdqCfgAd1Num                   %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd1Num);
    PS_PRINTF_INFO("DLAdqFreeAd0Num                  %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeAd0Num);
    PS_PRINTF_INFO("DLAdqFreeAd1Num                  %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeAd1Num);
    PS_PRINTF_INFO("DLAdqCfgIpfSuccNum               %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgIpfSuccNum);
    PS_PRINTF_INFO("DLAdqCfgIpfFailNum               %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgIpfFailNum);
    PS_PRINTF_INFO("DLAdqStartEmptyTmrNum            %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqStartEmptyTmrNum);
    PS_PRINTF_INFO("DLAdqEmptyTmrTimeoutNum          %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqEmptyTmrTimeoutNum);
    PS_PRINTF_INFO("DLAdqRcvAd0EmptyIntNum           %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqRcvAd0EmptyIntNum);
    PS_PRINTF_INFO("DLAdqRcvAd1EmptyIntNum           %10u\n", g_stAdsStats.stDlComStatsInfo.ulDLAdqRcvAd1EmptyIntNum);
    PS_PRINTF_INFO("ADS DL ADQ PROC STATS END.               \n");

    return;
}


VOS_VOID ADS_ShowResetProcStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS RESET PROC STATS START:             \n");
    PS_PRINTF_INFO("ULResetSem                       %ld\n",   g_stAdsCtx.hULResetSem);
    PS_PRINTF_INFO("ULResetCreateSemFailNum          %10u\n", g_stAdsStats.stResetStatsInfo.ulULResetCreateSemFailNum);
    PS_PRINTF_INFO("ULResetLockFailNum               %10u\n", g_stAdsStats.stResetStatsInfo.ulULResetLockFailNum);
    PS_PRINTF_INFO("ULResetSuccNum                   %10u\n", g_stAdsStats.stResetStatsInfo.ulULResetSuccNum);
    PS_PRINTF_INFO("DLResetSem                       %ld\n",   g_stAdsCtx.hDLResetSem);
    PS_PRINTF_INFO("DLResetCreateSemFailNum          %10u\n", g_stAdsStats.stResetStatsInfo.ulDLResetCreateSemFailNum);
    PS_PRINTF_INFO("DLResetLockFailNum               %10u\n", g_stAdsStats.stResetStatsInfo.ulDLResetLockFailNum);
    PS_PRINTF_INFO("DLResetSuccNum                   %10u\n", g_stAdsStats.stResetStatsInfo.ulDLResetSuccNum);
    PS_PRINTF_INFO("ADS RESET PROC STATS END.                \n");

    return;
}


VOS_VOID ADS_ShowDLMemStats(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DL MEM STATS START:                   \n");
    PS_PRINTF_INFO("DLUsingAd0Num                     %10u\n", (g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd0Num - g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeAd0Num));
    PS_PRINTF_INFO("DLUsingAd1Num                     %10u\n", (g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd1Num - g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeAd1Num));
    PS_PRINTF_INFO("DLRdqRxRdNum                      %10u\n",  g_stAdsStats.stDlComStatsInfo.ulDLRdqRxRdNum);
    PS_PRINTF_INFO("DLRmnetNoFuncFreePktNum           %10u\n",  g_stAdsStats.stDlComStatsInfo.ulDLRmnetNoFuncFreePktNum);
    PS_PRINTF_INFO("DLGetIpfAd0FailedNum              %10u\n",  g_stAdsStats.stDlComStatsInfo.ulDLAdqGetIpfAd0FailNum);
    PS_PRINTF_INFO("DLGetIpfAd1FailedNum              %10u\n",  g_stAdsStats.stDlComStatsInfo.ulDLAdqGetIpfAd1FailNum);
    PS_PRINTF_INFO("ADS DL MEM STATS END.                     \n");

    return;
}


VOS_VOID ADS_ShowDLInfoStats(VOS_VOID)
{
    ADS_ShowEventProcStats();
    ADS_ShowDLPktProcStats();
    ADS_ShowDLAdProcStats();
    ADS_ShowDLRdProcStats();
    ADS_ShowDLMemStats();

    return;
}



VOS_VOID ADS_Help(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DEBUG ENTRY                    \n");
    PS_PRINTF_INFO("<ADS_ShowEntityStats>          \n");
    PS_PRINTF_INFO("<ADS_ShowEventProcStats>       \n");
    PS_PRINTF_INFO("<ADS_ShowULPktProcStats>       \n");
    PS_PRINTF_INFO("<ADS_ShowULBdProcStats>        \n");
    PS_PRINTF_INFO("<ADS_ShowDLInfoStats>          \n");
    PS_PRINTF_INFO("<ADS_ShowDLPktProcStats>       \n");
    PS_PRINTF_INFO("<ADS_ShowDLRdProcStats>        \n");
    PS_PRINTF_INFO("<ADS_ShowDLAdProcStats>        \n");
    PS_PRINTF_INFO("<ADS_ShowDLMemStats>           \n");
    PS_PRINTF_INFO("<ADS_ShowResetProcStats>       \n");
    PS_PRINTF_INFO("<ADS_ShowFeatureState>         \n");
    PS_PRINTF_INFO("<ADS_ResetDebugInfo>           \n");

    return;
}


VOS_VOID ADS_ResetDebugInfo(VOS_VOID)
{
    TAF_MEM_SET_S(&g_stAdsStats.stUlComStatsInfo, sizeof(g_stAdsStats.stUlComStatsInfo), 0x00, sizeof(ADS_UL_COM_STATS_INFO_STRU));
    TAF_MEM_SET_S(&g_stAdsStats.stDlComStatsInfo, sizeof(g_stAdsStats.stDlComStatsInfo), 0x00, sizeof(ADS_DL_COM_STATS_INFO_STRU));
    return;
}


VOS_VOID ADS_SetFlowDebugFlag(VOS_UINT32  ulFlowDebugFlag)
{
    switch (ulFlowDebugFlag)
    {
        case ADS_FLOW_DEBUG_DL_ON:
            g_stAdsStats.stDlComStatsInfo.ulDLFlowDebugFlag     = PS_TRUE;
            g_stAdsStats.stUlComStatsInfo.ulULFlowDebugFlag     = PS_FALSE;
            g_stAdsStats.stDlComStatsInfo.ulDLFlowRptThreshold  = ADS_FLOW_DL_DEFAULT_RPT_THRESHOLD;
            break;

        case ADS_FLOW_DEBUG_UL_ON:
            g_stAdsStats.stDlComStatsInfo.ulDLFlowDebugFlag     = PS_FALSE;
            g_stAdsStats.stUlComStatsInfo.ulULFlowDebugFlag     = PS_TRUE;
            g_stAdsStats.stUlComStatsInfo.ulULFlowRptThreshold  = ADS_FLOW_UL_DEFAULT_RPT_THRESHOLD;
            break;

        case ADS_FLOW_DEBUG_ALL_ON:
            g_stAdsStats.stDlComStatsInfo.ulDLFlowDebugFlag     = PS_TRUE;
            g_stAdsStats.stUlComStatsInfo.ulULFlowDebugFlag     = PS_TRUE;
            g_stAdsStats.stDlComStatsInfo.ulDLFlowRptThreshold  = ADS_FLOW_DL_DEFAULT_RPT_THRESHOLD;
            g_stAdsStats.stUlComStatsInfo.ulULFlowRptThreshold  = ADS_FLOW_UL_DEFAULT_RPT_THRESHOLD;
            break;

        default:
            g_stAdsStats.stDlComStatsInfo.ulDLFlowDebugFlag     = PS_FALSE;
            g_stAdsStats.stUlComStatsInfo.ulULFlowDebugFlag     = PS_FALSE;
            break;
    }

    return;
}


VOS_VOID ADS_SetFlowDLRptThreshold(VOS_UINT32  ulFlowDLRptThreshold)
{
    g_stAdsStats.stDlComStatsInfo.ulDLFlowRptThreshold  = ulFlowDLRptThreshold;
    return;
}


VOS_VOID ADS_SetFlowULRptThreshold(VOS_UINT32  ulFlowULRptThreshold)
{
    g_stAdsStats.stUlComStatsInfo.ulULFlowRptThreshold  = ulFlowULRptThreshold;
    return;
}


VOS_VOID ADS_SetDlDiscardPktFlag(VOS_UINT32 ulValue)
{
    g_ulAdsDlDiscardPktFlag = ulValue;
    return;
}


VOS_VOID ADS_DLFlowAdd(VOS_UINT32 ulSduLen)
{
    if (PS_TRUE == g_stAdsStats.stDlComStatsInfo.ulDLFlowDebugFlag)
    {
        /* 流量统计 */
        g_stAdsStats.stDlComStatsInfo.ulDLFlowInfo += ulSduLen;

        /* 流量统计上报 */
        if (g_stAdsStats.stDlComStatsInfo.ulDLFlowInfo >= g_stAdsStats.stDlComStatsInfo.ulDLFlowRptThreshold)
        {
            g_stAdsStats.stDlComStatsInfo.ulDLEndSlice   = VOS_GetSlice();

            PS_PRINTF_INFO("DL Flow Info = %10d, Pkt Num = %10d, Slice = %10d, Time = %10d\n",
                g_stAdsStats.stDlComStatsInfo.ulDLFlowInfo,
                g_stAdsStats.stDlComStatsInfo.ulDLRmnetTxPktNum, g_stAdsStats.stDlComStatsInfo.ulDLEndSlice,
                (g_stAdsStats.stDlComStatsInfo.ulDLEndSlice - g_stAdsStats.stDlComStatsInfo.ulDLStartSlice));

            g_stAdsStats.stDlComStatsInfo.ulDLStartSlice = g_stAdsStats.stDlComStatsInfo.ulDLEndSlice;
            g_stAdsStats.stDlComStatsInfo.ulDLFlowInfo   = 0;
        }
    }

    return;
}


VOS_VOID ADS_ULFlowAdd(VOS_UINT32 ulSduLen)
{
    if (PS_TRUE == g_stAdsStats.stUlComStatsInfo.ulULFlowDebugFlag)
    {
        /* 流量统计 */
        g_stAdsStats.stUlComStatsInfo.ulULFlowInfo += ulSduLen;

        /* 流量统计上报 */
        if (g_stAdsStats.stUlComStatsInfo.ulULFlowInfo >= g_stAdsStats.stUlComStatsInfo.ulULFlowRptThreshold)
        {
            g_stAdsStats.stUlComStatsInfo.ulULEndSlice   = VOS_GetSlice();

            PS_PRINTF_INFO("UL Flow Info = %10d, Pkt Num = %10d, Slice = %10d, Time = %10d\n",
                g_stAdsStats.stUlComStatsInfo.ulULFlowInfo,
                g_stAdsStats.stUlComStatsInfo.ulULRmnetRxPktNum, g_stAdsStats.stUlComStatsInfo.ulULEndSlice,
                (g_stAdsStats.stUlComStatsInfo.ulULEndSlice - g_stAdsStats.stUlComStatsInfo.ulULStartSlice));

            g_stAdsStats.stUlComStatsInfo.ulULStartSlice = g_stAdsStats.stUlComStatsInfo.ulULEndSlice;
            g_stAdsStats.stUlComStatsInfo.ulULFlowInfo   = 0;
        }
    }

    return;
}


VOS_VOID ADS_LogPrintf(ADS_LOG_LEVEL_ENUM_UINT32 enLevel, VOS_CHAR *pcFmt, ...)
{
    VOS_CHAR                            acBuff[ADS_LOG_BUFF_LEN] = {0};
    va_list                             pArgList;
    VOS_UINT32                          ulPrintLength = 0;

    /* 打印级别过滤 */
    if (enLevel < g_enAdsLogLevel)
    {
        return;
    }

    /*lint -e713 -e507 -e530*/
    va_start(pArgList, pcFmt);
    ulPrintLength += VOS_nvsprintf_s(acBuff, ADS_LOG_BUFF_LEN, ADS_LOG_BUFF_LEN, pcFmt, pArgList);
    va_end(pArgList);
    /*lint +e713 +e507 +e530*/

    if (ulPrintLength > (ADS_LOG_BUFF_LEN - 1))
    {
        ulPrintLength = ADS_LOG_BUFF_LEN - 1;
    }

    acBuff[ulPrintLength] = '\0';

    /* 目前只有ERR打印的调用，以后可以再进行扩展 */
    ADS_PR_LOGI("%s", acBuff);

    return;
}





