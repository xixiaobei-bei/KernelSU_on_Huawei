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

#include "RnicCtx.h"
#include "RnicLog.h"
#include "RnicMntn.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_RNIC_MNTN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
RNIC_STATS_INFO_STRU                    g_astRnicStats;


/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_VOID RNIC_MNTN_TransReport(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLen
)
{
    DIAG_TRANS_IND_STRU                 stDiagTransInd;

    stDiagTransInd.ulModule  = RNIC_MNTN_COMM_MOUDLE_ID;
    stDiagTransInd.ulPid     = ACPU_PID_RNIC;
    stDiagTransInd.ulMsgId   = ulMsgId;
    stDiagTransInd.ulReserve = 0;
    stDiagTransInd.ulLength  = ulLen;
    stDiagTransInd.pData     = pData;

    (VOS_VOID)DIAG_TransReport(&stDiagTransInd);
    return;
}


VOS_VOID RNIC_MNTN_ReportPktStats(VOS_UINT8 ucRmNetId)
{
    struct rnic_data_stats_s           *pstDataStats = VOS_NULL_PTR;
    RNIC_MNTN_PKT_STATS_STRU            stStats;

    /* 调用驱动接口获取数据统计信息 */
    pstDataStats = rnic_get_data_stats(ucRmNetId);
    if (VOS_NULL_PTR == pstDataStats)
    {
        return;
    }

    stStats.stCommHeader.ucVer          = 103;
    stStats.stCommHeader.ucReserved     = 0;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ucRmNetId                   = ucRmNetId;
    stStats.aucReserved0                = 0;
    stStats.aucReserved1                = 0;
    stStats.aucReserved2                = 0;

    TAF_MEM_CPY_S(&(stStats.stDataStats), sizeof(stStats.stDataStats),
                  pstDataStats, sizeof(struct rnic_data_stats_s));

    RNIC_MNTN_TransReport(ID_DIAG_RNIC_PKT_STATS_IND,
                          (VOS_VOID *)&stStats,
                          sizeof(RNIC_MNTN_PKT_STATS_STRU));
    return;
}


VOS_VOID RNIC_MNTN_ReportNapiInfo(VOS_UINT8 ucRmNetId)
{
    RNIC_MNTN_NAIP_INFO_STRU            stNapiInfo;

    stNapiInfo.stCommHeader.ucVer          = 102;
    stNapiInfo.stCommHeader.ucReserved     = 0;
    stNapiInfo.stCommHeader.usReserved0    = 0;
    stNapiInfo.stCommHeader.usReserved1    = 0;
    stNapiInfo.stCommHeader.usReserved2    = 0;

    stNapiInfo.ucRmNetId                   = ucRmNetId;
    stNapiInfo.ucNapiEnable                = RNIC_GET_NAPI_FEATURE(ucRmNetId);
    stNapiInfo.ucGroEnable                 = RNIC_GET_GRO_FEATURE(ucRmNetId);
    stNapiInfo.ucDlNapiWeight              = RNIC_GET_NAPI_WEIGHT(ucRmNetId);

    RNIC_MNTN_TransReport(ID_DIAG_RNIC_NAPI_INFO_IND,
                          (VOS_VOID *)&stNapiInfo,
                          sizeof(RNIC_MNTN_NAIP_INFO_STRU));
    return;
}


VOS_VOID RNIC_MNTN_ReportAllStatsInfo(VOS_VOID)
{
    RNIC_PS_IFACE_INFO_STRU            *pstPsIfaceInfo   = VOS_NULL_PTR;
    VOS_UINT32                          i;

    /* 勾包所有激活网卡状态 */
    for (i = 0; i < RNIC_DEV_ID_BUTT; i++)
    {
        pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(i);
        if ( (VOS_TRUE == pstPsIfaceInfo->bitOpIpv4Act)
          || (VOS_TRUE == pstPsIfaceInfo->bitOpIpv6Act) )
        {
            RNIC_MNTN_ReportPktStats((VOS_UINT8)i);
            RNIC_MNTN_ReportNapiInfo((VOS_UINT8)i);
        }
    }

    return;
}


VOS_VOID RNIC_MNTN_TraceDialConnEvt(VOS_VOID)
{
    RNIC_MNTN_DIAL_CONN_EVT_STRU       *pstDialEvt = VOS_NULL_PTR;

    /* 构造消息 */
    pstDialEvt = (RNIC_MNTN_DIAL_CONN_EVT_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                        ACPU_PID_RNIC,
                        sizeof(RNIC_MNTN_DIAL_CONN_EVT_STRU));
    if (VOS_NULL_PTR == pstDialEvt)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MNTN_TraceDialEvent: Memory alloc failed.");
        return;
    }

    /* 填写消息 */
    pstDialEvt->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstDialEvt->ulReceiverPid   = ACPU_PID_RNIC;
    pstDialEvt->enMsgId         = ID_RNIC_MNTN_EVT_DIAL_CONNECT;

    /* 钩出可维可测消息 */
    if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstDialEvt))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MNTN_TraceDialConnEvt():WARNING:SEND MSG FIAL");
    }

    return;
}


VOS_VOID RNIC_MNTN_TraceDialDisconnEvt(
    VOS_UINT32                          ulPktNum,
    VOS_UINT32                          ulUsrExistFlg
)
{
    RNIC_MNTN_DIAL_DISCONN_EVT_STRU    *pstDialEvt = VOS_NULL_PTR;

    /* 构造消息 */
    pstDialEvt = (RNIC_MNTN_DIAL_DISCONN_EVT_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                        ACPU_PID_RNIC,
                        sizeof(RNIC_MNTN_DIAL_DISCONN_EVT_STRU));
    if (VOS_NULL_PTR == pstDialEvt)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MNTN_TraceDialEvent: Memory alloc failed.");
        return;
    }

    /* 填写消息头 */
    pstDialEvt->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstDialEvt->ulReceiverPid   = ACPU_PID_RNIC;
    pstDialEvt->enMsgId         = ID_RNIC_MNTN_EVT_DIAL_DISCONNECT;

    /* 填写消息内容 */
    pstDialEvt->ulPktNum        = ulPktNum;
    pstDialEvt->ulUsrExistFlg   = ulUsrExistFlg;

    /* 钩出可维可测消息 */
    if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstDialEvt))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MNTN_TraceDialDisconnEvt():WARNING:SEND MSG FIAL");
    }

    return;
}


VOS_VOID RNIC_ShowULProcStats(VOS_UINT8 ucRmNetId)
{
    struct rnic_data_stats_s           *pstDataStats = VOS_NULL_PTR;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF("NetId overtop, RmnetId: %d.\n", ucRmNetId);
        return;
    }

    /* 调用驱动接口获取数据统计信息 */
    pstDataStats = rnic_get_data_stats(ucRmNetId);
    if (VOS_NULL_PTR == pstDataStats)
    {
        PS_PRINTF("Get data stats fail, RmnetId: %d.\n", ucRmNetId);
        return;
    }

    /* 上行统计量 */
    PS_PRINTF("[RMNET%d] tx_total_packets               %10u\n", ucRmNetId, pstDataStats->tx_total_packets);
    PS_PRINTF("[RMNET%d] tx_packets                     %10u\n", ucRmNetId, pstDataStats->tx_packets);
    PS_PRINTF("[RMNET%d] tx_dropped                     %10u\n", ucRmNetId, pstDataStats->tx_dropped);
    PS_PRINTF("[RMNET%d] tx_length_errors               %10u\n", ucRmNetId, pstDataStats->tx_length_errors);
    PS_PRINTF("[RMNET%d] tx_proto_errors                %10u\n", ucRmNetId, pstDataStats->tx_proto_errors);
    PS_PRINTF("[RMNET%d] tx_carrier_errors              %10u\n", ucRmNetId, pstDataStats->tx_carrier_errors);
    PS_PRINTF("[RMNET%d] tx_iface_errors                %10u\n", ucRmNetId, pstDataStats->tx_iface_errors);
    PS_PRINTF("[RMNET%d] tx_linearize_errors            %10u\n", ucRmNetId, pstDataStats->tx_linearize_errors);
    PS_PRINTF("\n");

    return;
}


VOS_VOID RNIC_ShowDLProcStats(VOS_UINT8 ucRmNetId)
{
    struct rnic_data_stats_s           *pstDataStats = VOS_NULL_PTR;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF("NetId overtop, RmnetId: %d\n", ucRmNetId);
        return;
    }
    /* 调用驱动接口获取数据统计信息 */
    pstDataStats = rnic_get_data_stats(ucRmNetId);
    if (VOS_NULL_PTR == pstDataStats)
    {
        return;
    }

    /* 下行统计量 */
    PS_PRINTF("[RMNET%d] rx_total_packets               %10u\n", ucRmNetId, pstDataStats->rx_total_packets);
    PS_PRINTF("[RMNET%d] rx_packets                     %10u\n", ucRmNetId, pstDataStats->rx_packets);
    PS_PRINTF("[RMNET%d] rx_dropped                     %10u\n", ucRmNetId, pstDataStats->rx_dropped);
    PS_PRINTF("[RMNET%d] rx_length_errors               %10u\n", ucRmNetId, pstDataStats->rx_length_errors);
    PS_PRINTF("[RMNET%d] rx_phys_errors                 %10u\n", ucRmNetId, pstDataStats->rx_phys_errors);
    PS_PRINTF("[RMNET%d] rx_enqueue_errors              %10u\n", ucRmNetId, pstDataStats->rx_enqueue_errors);
    PS_PRINTF("[RMNET%d] rx_trans_errors                %10u\n", ucRmNetId, pstDataStats->rx_trans_errors);
    PS_PRINTF("\n");

    return;
}


VOS_VOID RNIC_ShowPsIfaceInfo(VOS_UINT8 ucRmNetId)
{
    RNIC_PS_IFACE_INFO_STRU              *pstPsIfaceInfo = VOS_NULL_PTR;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF("NetId overtop, RmnetId; %d\n", ucRmNetId);
        return;
    }

    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(ucRmNetId);
    if (pstPsIfaceInfo->enModemId < MODEM_ID_BUTT)
    {
        PS_PRINTF("[RMNET%d] ModemId            %d\n", ucRmNetId, pstPsIfaceInfo->enModemId);
    }
    else
    {
        PS_PRINTF("[RMNET%d] ModemId            %s\n", ucRmNetId, (MODEM_ID_BUTT == pstPsIfaceInfo->enModemId) ? "MODEM_ID_BUTT" : "Invalid MODEM ID");
    }

    if(pstPsIfaceInfo->enRatType < RNIC_PS_RAT_TYPE_BUTT)
    {
        PS_PRINTF("[RMNET%d] RatType            %s\n", ucRmNetId, (RNIC_PS_RAT_TYPE_3GPP == pstPsIfaceInfo->enRatType) ? "3GPP" : "IWLAN");
    }
    else
    {
        PS_PRINTF("[RMNET%d] RatType            %s\n", ucRmNetId, (RNIC_PS_RAT_TYPE_BUTT == pstPsIfaceInfo->enRatType) ? "RNIC_PS_RAT_TYPE_BUTT" : "Invalid RAT TYPE");
    }

    PS_PRINTF("[RMNET%d] IFACE IPV4 STATUS  %d\n", ucRmNetId, pstPsIfaceInfo->bitOpIpv4Act);
    PS_PRINTF("[RMNET%d] IFACE IPV4 RABID   %d\n", ucRmNetId, pstPsIfaceInfo->ucIpv4RabId);
    PS_PRINTF("[RMNET%d] IFACE IPV6 STATUS  %d\n", ucRmNetId, pstPsIfaceInfo->bitOpIpv6Act);
    PS_PRINTF("[RMNET%d] IFACE IPV6 RABID   %d\n", ucRmNetId, pstPsIfaceInfo->ucIpv6RabId);
    PS_PRINTF("\n");

}


VOS_VOID RNIC_ShowNapiInfo(VOS_UINT8 ucRmNetId)
{
    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF("NetId overtop, RmnetId: %d\n", ucRmNetId);
        return;
    }

    PS_PRINTF("[RMNET%d] NAPI enable       :(0:disable/1:enable)    %d\n", ucRmNetId, RNIC_GET_NAPI_FEATURE(ucRmNetId));
    PS_PRINTF("[RMNET%d] GRO  enable       :(0:disable/1:enable)    %d\n", ucRmNetId, RNIC_GET_GRO_FEATURE(ucRmNetId));
    PS_PRINTF("[RMNET%d] NapiWeightAdjMode :(0:static/1:dynamic)    %d\n", ucRmNetId, RNIC_GET_NAPI_WEIGHT_ADJ_MODE(ucRmNetId));
    PS_PRINTF("[RMNET%d] NapiPollWeight    :                        %d\n", ucRmNetId, RNIC_GET_NAPI_WEIGHT(ucRmNetId));
    PS_PRINTF("[RMNET%d] NapiMaxQueLen     :                        %d\n", ucRmNetId, RNIC_GET_NAPI_POLL_QUE_MAX_LEN(ucRmNetId));
    PS_PRINTF("TetherConnStat              :(0:disconn/1:conn)      %d\n", g_stRnicCtx.stTetherInfo.enTetherConnStat);
    PS_PRINTF("OrigGroEnable               :(0:disable/1:enable)    %d\n", g_stRnicCtx.stTetherInfo.ucOrigGroEnable);
    PS_PRINTF("RmnetName                   :                        %s\n", g_stRnicCtx.stTetherInfo.aucRmnetName);
    PS_PRINTF("\n");

    return;
}


VOS_VOID RNIC_ShowOndemandInfo(VOS_VOID)
{
    PS_PRINTF("DialMode                             %10u\n", g_stRnicCtx.stDialMode.enDialMode);
    PS_PRINTF("EventReportFlag                      %10u\n", g_stRnicCtx.stDialMode.enEventReportFlag);
    PS_PRINTF("IdleTime                             %10u\n", g_stRnicCtx.stDialMode.ulIdleTime);
    PS_PRINTF("SendAppDialUpSucc                    %10u\n", g_astRnicStats.ulUlSendAppDialUpSucc);
    PS_PRINTF("SendAppDialUpFail                    %10u\n", g_astRnicStats.ulUlSendAppDialUpFail);
    PS_PRINTF("SendAppDialDownSucc                  %10u\n", g_astRnicStats.ulUlSendAppDialDownSucc);
    PS_PRINTF("SendAppDialDownFail                  %10u\n", g_astRnicStats.ulUlSendAppDialDownFail);
    PS_PRINTF("\n");

    return;
}


VOS_VOID RNIC_Help(VOS_VOID)
{

    PS_PRINTF("********************RNIC调试信息************************\n");
    PS_PRINTF("RNIC_ShowULProcStats(RmnetId)    显示指定RNIC网卡上行统计信息\n");
    PS_PRINTF("RNIC_ShowDLProcStats(RmnetId)    显示指定RNIC网卡下行统计信息\n");
    PS_PRINTF("RNIC_ShowPsIfaceInfo(RmnetId)    显示指定RNIC网卡PDP激活信息\n");
    PS_PRINTF("RNIC_ShowNapiInfo(RmnetId)       显示指定RNIC网卡的NAPI相关信息\n");
    PS_PRINTF("RNIC_ShowOndemandInfo            显示按需拨号统计信息\n");

    return;
}


