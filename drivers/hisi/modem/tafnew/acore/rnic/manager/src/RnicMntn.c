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
#include "ps_tag.h"
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

    TAF_MEM_SET_S(&stNapiInfo, sizeof(stNapiInfo),
              0x00, sizeof(RNIC_MNTN_NAIP_INFO_STRU));

    /* 调用驱动接口获取Napi统计信息 */
    if (rnic_get_napi_stats(ucRmNetId, &stNapiInfo.stNapiStats))
    {
        return;
    }

    stNapiInfo.stCommHeader.ucVer          = 105;
    stNapiInfo.stCommHeader.ucReserved     = 0;
    stNapiInfo.stCommHeader.usReserved0    = 0;
    stNapiInfo.stCommHeader.usReserved1    = 0;
    stNapiInfo.stCommHeader.usReserved2    = 0;

    stNapiInfo.ucRmNetId                   = ucRmNetId;
    stNapiInfo.ucReserved0                 = 0;
    stNapiInfo.ucReserved1                 = 0;
    stNapiInfo.ucReserved2                 = 0;

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
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d.\n", ucRmNetId);
        return;
    }

    /* 调用驱动接口获取数据统计信息 */
    pstDataStats = rnic_get_data_stats(ucRmNetId);
    if (VOS_NULL_PTR == pstDataStats)
    {
        PS_PRINTF_WARNING("Get data stats fail, RmnetId: %d.\n", ucRmNetId);
        return;
    }

    /* 上行统计量 */
    PS_PRINTF_INFO("[RMNET%d] tx_total_packets               %10u\n", ucRmNetId, pstDataStats->tx_total_packets);
    PS_PRINTF_INFO("[RMNET%d] tx_packets                     %10u\n", ucRmNetId, pstDataStats->tx_packets);
    PS_PRINTF_INFO("[RMNET%d] tx_dropped                     %10u\n", ucRmNetId, pstDataStats->tx_dropped);
    PS_PRINTF_INFO("[RMNET%d] tx_length_errors               %10u\n", ucRmNetId, pstDataStats->tx_length_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_proto_errors                %10u\n", ucRmNetId, pstDataStats->tx_proto_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_carrier_errors              %10u\n", ucRmNetId, pstDataStats->tx_carrier_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_iface_errors                %10u\n", ucRmNetId, pstDataStats->tx_iface_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_linearize_errors            %10u\n", ucRmNetId, pstDataStats->tx_linearize_errors);

    return;
}


VOS_VOID RNIC_ShowDLProcStats(VOS_UINT8 ucRmNetId)
{
    struct rnic_data_stats_s           *pstDataStats = VOS_NULL_PTR;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d\n", ucRmNetId);
        return;
    }
    /* 调用驱动接口获取数据统计信息 */
    pstDataStats = rnic_get_data_stats(ucRmNetId);
    if (VOS_NULL_PTR == pstDataStats)
    {
        return;
    }

    /* 下行统计量 */
    PS_PRINTF_INFO("[RMNET%d] rx_total_packets               %10u\n", ucRmNetId, pstDataStats->rx_total_packets);
    PS_PRINTF_INFO("[RMNET%d] rx_packets                     %10u\n", ucRmNetId, pstDataStats->rx_packets);
    PS_PRINTF_INFO("[RMNET%d] rx_dropped                     %10u\n", ucRmNetId, pstDataStats->rx_dropped);
    PS_PRINTF_INFO("[RMNET%d] rx_length_errors               %10u\n", ucRmNetId, pstDataStats->rx_length_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_link_errors                 %10u\n", ucRmNetId, pstDataStats->rx_link_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_enqueue_errors              %10u\n", ucRmNetId, pstDataStats->rx_enqueue_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_trans_errors                %10u\n", ucRmNetId, pstDataStats->rx_trans_errors);

    return;
}


VOS_VOID RNIC_ShowPsIfaceInfo(VOS_UINT8 ucRmNetId)
{
    RNIC_PS_IFACE_INFO_STRU              *pstPsIfaceInfo = VOS_NULL_PTR;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF_WARNING("NetId overtop, RmnetId; %d\n", ucRmNetId);
        return;
    }

    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(ucRmNetId);
    if (pstPsIfaceInfo->enModemId < MODEM_ID_BUTT)
    {
        PS_PRINTF_INFO("[RMNET%d] ModemId            %d\n", ucRmNetId, pstPsIfaceInfo->enModemId);
    }
    else
    {
        PS_PRINTF_INFO("[RMNET%d] ModemId            %s\n", ucRmNetId, (MODEM_ID_BUTT == pstPsIfaceInfo->enModemId) ? "MODEM_ID_BUTT" : "Invalid MODEM ID");
    }

    if(pstPsIfaceInfo->enRatType < RNIC_PS_RAT_TYPE_BUTT)
    {
        PS_PRINTF_INFO("[RMNET%d] RatType            %s\n", ucRmNetId, (RNIC_PS_RAT_TYPE_3GPP == pstPsIfaceInfo->enRatType) ? "3GPP" : "IWLAN");
    }
    else
    {
        PS_PRINTF_INFO("[RMNET%d] RatType            %s\n", ucRmNetId, (RNIC_PS_RAT_TYPE_BUTT == pstPsIfaceInfo->enRatType) ? "RNIC_PS_RAT_TYPE_BUTT" : "Invalid RAT TYPE");
    }

    PS_PRINTF_INFO("[RMNET%d] IFACE IPV4 STATUS  %d\n", ucRmNetId, pstPsIfaceInfo->bitOpIpv4Act);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV4 RABID   %d\n", ucRmNetId, pstPsIfaceInfo->ucIpv4RabId);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV6 STATUS  %d\n", ucRmNetId, pstPsIfaceInfo->bitOpIpv6Act);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV6 RABID   %d\n", ucRmNetId, pstPsIfaceInfo->ucIpv6RabId);

}


VOS_VOID RNIC_ShowNapiInfo(VOS_UINT8 ucRmNetId)
{
    struct rnic_napi_stats_s            stNapiStats;

    if (ucRmNetId >= RNIC_NET_ID_MAX_NUM)
    {
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d\n", ucRmNetId);
        return;
    }

    TAF_MEM_SET_S(&stNapiStats, sizeof(stNapiStats),
              0x00, sizeof(struct rnic_napi_stats_s));

    /* 调用驱动接口获取Napi统计信息 */
    if (rnic_get_napi_stats(ucRmNetId, &stNapiStats))
    {
        PS_PRINTF("Get napi stats error.\n");
        return;
    }

    PS_PRINTF_INFO("[RMNET%d] NAPI enable       :(0:disable/1:enable)    %d\n", ucRmNetId, stNapiStats.napi_enable);
    PS_PRINTF_INFO("[RMNET%d] GRO  enable       :(0:disable/1:enable)    %d\n", ucRmNetId, stNapiStats.gro_enable);
    PS_PRINTF_INFO("[RMNET%d] NapiPollWeight    :                        %d\n", ucRmNetId, stNapiStats.napi_weight);
    PS_PRINTF_INFO("[RMNET%d] NapiWeightAdjMode :(0:static/1:dynamic)    %d\n", ucRmNetId, RNIC_GET_NAPI_WEIGHT_ADJ_MODE(ucRmNetId));
    PS_PRINTF_INFO("[RMNET%d] NapiMaxQueLen     :                        %d\n", ucRmNetId, RNIC_GET_NAPI_POLL_QUE_MAX_LEN(ucRmNetId));
    PS_PRINTF_INFO("[RMNET%d] NAPI LB enable    :(0:disable/1:enable)    %d\n", ucRmNetId, stNapiStats.napi_lb_enable);
    PS_PRINTF_INFO("[RMNET%d] LB current level  :                        %d\n", ucRmNetId, stNapiStats.lb_level);
    PS_PRINTF_INFO("[RMNET%d] select_cpu_num    : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].select_cpu_num,
               stNapiStats.lb_stats[1].select_cpu_num,
               stNapiStats.lb_stats[2].select_cpu_num,
               stNapiStats.lb_stats[3].select_cpu_num,
               stNapiStats.lb_stats[4].select_cpu_num,
               stNapiStats.lb_stats[5].select_cpu_num,
               stNapiStats.lb_stats[6].select_cpu_num,
               stNapiStats.lb_stats[7].select_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] schedul_on_cpu_num: %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].schedul_on_cpu_num,
               stNapiStats.lb_stats[1].schedul_on_cpu_num,
               stNapiStats.lb_stats[2].schedul_on_cpu_num,
               stNapiStats.lb_stats[3].schedul_on_cpu_num,
               stNapiStats.lb_stats[4].schedul_on_cpu_num,
               stNapiStats.lb_stats[5].schedul_on_cpu_num,
               stNapiStats.lb_stats[6].schedul_on_cpu_num,
               stNapiStats.lb_stats[7].schedul_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] poll_on_cpu_num   : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].poll_on_cpu_num,
               stNapiStats.lb_stats[1].poll_on_cpu_num,
               stNapiStats.lb_stats[2].poll_on_cpu_num,
               stNapiStats.lb_stats[3].poll_on_cpu_num,
               stNapiStats.lb_stats[4].poll_on_cpu_num,
               stNapiStats.lb_stats[5].poll_on_cpu_num,
               stNapiStats.lb_stats[6].poll_on_cpu_num,
               stNapiStats.lb_stats[7].poll_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] smp_on_cpu_num    : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].smp_on_cpu_num,
               stNapiStats.lb_stats[1].smp_on_cpu_num,
               stNapiStats.lb_stats[2].smp_on_cpu_num,
               stNapiStats.lb_stats[3].smp_on_cpu_num,
               stNapiStats.lb_stats[4].smp_on_cpu_num,
               stNapiStats.lb_stats[5].smp_on_cpu_num,
               stNapiStats.lb_stats[6].smp_on_cpu_num,
               stNapiStats.lb_stats[7].smp_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] dispatch_fail_num : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].dispatch_fail_num,
               stNapiStats.lb_stats[1].dispatch_fail_num,
               stNapiStats.lb_stats[2].dispatch_fail_num,
               stNapiStats.lb_stats[3].dispatch_fail_num,
               stNapiStats.lb_stats[4].dispatch_fail_num,
               stNapiStats.lb_stats[5].dispatch_fail_num,
               stNapiStats.lb_stats[6].dispatch_fail_num,
               stNapiStats.lb_stats[7].dispatch_fail_num);
    PS_PRINTF_INFO("[RMNET%d] smp_fail_num      : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].smp_fail_num,
               stNapiStats.lb_stats[1].smp_fail_num,
               stNapiStats.lb_stats[2].smp_fail_num,
               stNapiStats.lb_stats[3].smp_fail_num,
               stNapiStats.lb_stats[4].smp_fail_num,
               stNapiStats.lb_stats[5].smp_fail_num,
               stNapiStats.lb_stats[6].smp_fail_num,
               stNapiStats.lb_stats[7].smp_fail_num);
    PS_PRINTF_INFO("[RMNET%d] schedule_fail_num : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].schedule_fail_num,
               stNapiStats.lb_stats[1].schedule_fail_num,
               stNapiStats.lb_stats[2].schedule_fail_num,
               stNapiStats.lb_stats[3].schedule_fail_num,
               stNapiStats.lb_stats[4].schedule_fail_num,
               stNapiStats.lb_stats[5].schedule_fail_num,
               stNapiStats.lb_stats[6].schedule_fail_num,
               stNapiStats.lb_stats[7].schedule_fail_num);
    PS_PRINTF_INFO("[RMNET%d] hotplug_online_num: %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].hotplug_online_num,
               stNapiStats.lb_stats[1].hotplug_online_num,
               stNapiStats.lb_stats[2].hotplug_online_num,
               stNapiStats.lb_stats[3].hotplug_online_num,
               stNapiStats.lb_stats[4].hotplug_online_num,
               stNapiStats.lb_stats[5].hotplug_online_num,
               stNapiStats.lb_stats[6].hotplug_online_num,
               stNapiStats.lb_stats[7].hotplug_online_num);
    PS_PRINTF_INFO("[RMNET%d] hotplug_down_num  : %d %d %d %d %d %d %d %d\n",
               ucRmNetId,
               stNapiStats.lb_stats[0].hotplug_down_num,
               stNapiStats.lb_stats[1].hotplug_down_num,
               stNapiStats.lb_stats[2].hotplug_down_num,
               stNapiStats.lb_stats[3].hotplug_down_num,
               stNapiStats.lb_stats[4].hotplug_down_num,
               stNapiStats.lb_stats[5].hotplug_down_num,
               stNapiStats.lb_stats[6].hotplug_down_num,
               stNapiStats.lb_stats[7].hotplug_down_num);
    PS_PRINTF_INFO("TetherConnStat              :(0:disconn/1:conn)      %d\n", g_stRnicCtx.stTetherInfo.enTetherConnStat);
    PS_PRINTF_INFO("TetherOrigGroEnable         :(0:disable/1:enable)    %d\n", g_stRnicCtx.stTetherInfo.ucOrigGroEnable);
    PS_PRINTF_INFO("TetherRmnetName             :                        %s\n", g_stRnicCtx.stTetherInfo.aucRmnetName);

    return;
}


VOS_VOID RNIC_ShowOndemandInfo(VOS_VOID)
{
    PS_PRINTF_INFO("DialMode                             %10u\n", g_stRnicCtx.stDialMode.enDialMode);
    PS_PRINTF_INFO("EventReportFlag                      %10u\n", g_stRnicCtx.stDialMode.enEventReportFlag);
    PS_PRINTF_INFO("IdleTime                             %10u\n", g_stRnicCtx.stDialMode.ulIdleTime);
    PS_PRINTF_INFO("SendAppDialUpSucc                    %10u\n", g_astRnicStats.ulUlSendAppDialUpSucc);
    PS_PRINTF_INFO("SendAppDialUpFail                    %10u\n", g_astRnicStats.ulUlSendAppDialUpFail);
    PS_PRINTF_INFO("SendAppDialDownSucc                  %10u\n", g_astRnicStats.ulUlSendAppDialDownSucc);
    PS_PRINTF_INFO("SendAppDialDownFail                  %10u\n", g_astRnicStats.ulUlSendAppDialDownFail);

    return;
}


VOS_VOID RNIC_Help(VOS_VOID)
{

    PS_PRINTF_INFO("RNIC Debug Info                    \n");
    PS_PRINTF_INFO("<RNIC_ShowULProcStats(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowDLProcStats(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowPsIfaceInfo(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowNapiInfo(RmnetId)>       \n");
    PS_PRINTF_INFO("<RNIC_ShowOndemandInfo>            \n");

    return;
}


