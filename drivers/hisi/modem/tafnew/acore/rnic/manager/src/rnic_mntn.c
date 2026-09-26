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

#include "mdrv.h"
#include "ps_tag.h"
#include "rnic_ctx.h"
#include "rnic_log.h"
#include "rnic_mntn.h"
#include "rnic_msg_proc.h"
#include "mn_comm_api.h"
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_RNIC_MNTN_C

RNIC_StatsInfo g_rnicStats;

VOS_VOID RNIC_MntnTransReport(VOS_UINT32 msgId, VOS_VOID *data, VOS_UINT32 len)
{
    mdrv_diag_trans_ind_s diagTransInd;

    diagTransInd.ulModule  = RNIC_MNTN_COMM_MOUDLE_ID;
    diagTransInd.ulPid     = ACPU_PID_RNIC;
    diagTransInd.ulMsgId   = msgId;
    diagTransInd.ulReserve = 0;
    diagTransInd.ulLength  = len;
    diagTransInd.pData     = data;

    (VOS_VOID)mdrv_diag_trans_report(&diagTransInd);
}

VOS_VOID RNIC_MntnReportPktStats(VOS_UINT8 rmNetId)
{
    struct rnic_data_stats_s *dataStats = VOS_NULL_PTR;
    RNIC_MntnPktStats         stats;

    /* 调用驱动接口获取数据统计信息 */
    dataStats = rnic_get_data_stats(rmNetId);
    if (dataStats == VOS_NULL_PTR) {
        return;
    }

    stats.commHeader.ver       = 104;
    stats.commHeader.reserved  = 0;
    stats.commHeader.reserved0 = 0;
    stats.commHeader.reserved1 = 0;
    stats.commHeader.reserved2 = 0;

    stats.rmNetId   = rmNetId;
    stats.reserved0 = 0;
    stats.reserved1 = 0;
    stats.reserved2 = 0;

    if (memcpy_s(&(stats.dataStats), sizeof(stats.dataStats), dataStats, sizeof(struct rnic_data_stats_s)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnReportPktStats, memcpy fail!");
    }

    RNIC_MntnTransReport(ID_DIAG_RNIC_PKT_STATS_IND, (VOS_VOID *)&stats, sizeof(RNIC_MntnPktStats));
}

VOS_VOID RNIC_MntnReportNapiInfo(VOS_UINT8 rmNetId)
{
    RNIC_MntnNaipInfo napiInfo;

    (VOS_VOID)memset_s(&napiInfo, sizeof(napiInfo), 0x00, sizeof(napiInfo));

    /* 调用驱动接口获取Napi统计信息 */
    if (rnic_get_napi_stats(rmNetId, &napiInfo.napiStats)) {
        return;
    }

    napiInfo.commHeader.ver       = 106;
    napiInfo.commHeader.reserved  = 0;
    napiInfo.commHeader.reserved0 = 0;
    napiInfo.commHeader.reserved1 = 0;
    napiInfo.commHeader.reserved2 = 0;

    napiInfo.rmNetId   = rmNetId;
    napiInfo.reserved0 = 0;
    napiInfo.reserved1 = 0;
    napiInfo.reserved2 = 0;

    RNIC_MntnTransReport(ID_DIAG_RNIC_NAPI_INFO_IND, (VOS_VOID *)&napiInfo, sizeof(RNIC_MntnNaipInfo));
}

VOS_VOID RNIC_MntnReportAllStatsInfo(VOS_VOID)
{
    RNIC_PS_IfaceInfo *psIfaceInfo = VOS_NULL_PTR;
    VOS_UINT32         i;

    /* 勾包所有激活网卡状态 */
    for (i = 0; i < RNIC_DEV_ID_BUTT; i++) {
        psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(i);
        if ((psIfaceInfo->opIpv4Act == VOS_TRUE) || (psIfaceInfo->opIpv6Act == VOS_TRUE) ||
            (psIfaceInfo->opEthAct == VOS_TRUE)) {
            RNIC_MntnReportPktStats((VOS_UINT8)i);
            RNIC_MntnReportNapiInfo((VOS_UINT8)i);
        }
    }
}

VOS_VOID RNIC_MntnTraceDialConnEvt(VOS_VOID)
{
    RNIC_MntnDialConnEvt *dialEvt = VOS_NULL_PTR;

    /* 构造消息 */
    dialEvt = (RNIC_MntnDialConnEvt *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_MntnDialConnEvt));
    if (dialEvt == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnTraceDialConnEvt: Memory alloc failed.");
        return;
    }

    /* 清空消息内容 */
    if (memset_s(RNIC_GET_MSG_ENTITY(dialEvt), RNIC_GET_MSG_LENGTH(dialEvt), 0x00, RNIC_GET_MSG_LENGTH(dialEvt)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnTraceDialConnEvt, memset fail!");
    }

    /* 填充消息头 */
    RNIC_CFG_INTRA_MSG_HDR(dialEvt, ID_RNIC_MNTN_EVT_DIAL_CONNECT);

    /* 钩出可维可测消息 */
    if (RNIC_SendMsg(dialEvt) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnTraceDialConnEvt():WARNING:SEND MSG FIAL");
    }
}

VOS_VOID RNIC_MntnTraceDialDisconnEvt(VOS_UINT32 pktNum, VOS_UINT32 usrExistFlg)
{
    RNIC_MntnDialDisconnEvt *dialEvt = VOS_NULL_PTR;

    /* 构造消息 */
    dialEvt = (RNIC_MntnDialDisconnEvt *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_MntnDialDisconnEvt));
    if (dialEvt == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnTraceDialDisconnEvt: Memory alloc failed.");
        return;
    }

    /* 清空消息内容 */
    if (memset_s(RNIC_GET_MSG_ENTITY(dialEvt), RNIC_GET_MSG_LENGTH(dialEvt), 0x00, RNIC_GET_MSG_LENGTH(dialEvt)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MntnTraceDialDisconnEvt, memset fail!");
    }

    /* 填充消息头 */
    RNIC_CFG_INTRA_MSG_HDR(dialEvt, ID_RNIC_MNTN_EVT_DIAL_DISCONNECT);

    /* 填写消息内容 */
    dialEvt->pktNum      = pktNum;
    dialEvt->usrExistFlg = usrExistFlg;

    /* 钩出可维可测消息 */
    if (RNIC_SendMsg(dialEvt) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_MNTN_TraceDialDisconnEvt():WARNING:SEND MSG FIAL");
    }
}

VOS_VOID RNIC_ShowULProcStats(VOS_UINT8 rmNetId)
{
    struct rnic_data_stats_s *dataStats = VOS_NULL_PTR;

    if (rmNetId >= RNIC_NET_ID_MAX_NUM) {
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d.\n", rmNetId);
        return;
    }

    /* 调用驱动接口获取数据统计信息 */
    dataStats = rnic_get_data_stats(rmNetId);
    if (dataStats == VOS_NULL_PTR) {
        PS_PRINTF_WARNING("Get data stats fail, RmnetId: %d.\n", rmNetId);
        return;
    }

    /* 上行统计量 */
    PS_PRINTF_INFO("[RMNET%d] tx_total_packets               %10u\n", rmNetId, dataStats->tx_total_packets);
    PS_PRINTF_INFO("[RMNET%d] tx_packets                     %10u\n", rmNetId, dataStats->tx_packets);
    PS_PRINTF_INFO("[RMNET%d] tx_dropped                     %10u\n", rmNetId, dataStats->tx_dropped);
    PS_PRINTF_INFO("[RMNET%d] tx_length_errors               %10u\n", rmNetId, dataStats->tx_length_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_proto_errors                %10u\n", rmNetId, dataStats->tx_proto_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_carrier_errors              %10u\n", rmNetId, dataStats->tx_carrier_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_clone_errors                %10u\n", rmNetId, dataStats->tx_clone_errors);
    PS_PRINTF_INFO("[RMNET%d] tx_linearize_errors            %10u\n", rmNetId, dataStats->tx_linearize_errors);
}

VOS_VOID RNIC_ShowDLProcStats(VOS_UINT8 rmNetId)
{
    struct rnic_data_stats_s *dataStats = VOS_NULL_PTR;

    if (rmNetId >= RNIC_NET_ID_MAX_NUM) {
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d\n", rmNetId);
        return;
    }
    /* 调用驱动接口获取数据统计信息 */
    dataStats = rnic_get_data_stats(rmNetId);
    if (dataStats == VOS_NULL_PTR) {
        return;
    }

    /* 下行统计量 */
    PS_PRINTF_INFO("[RMNET%d] rx_total_packets               %10u\n", rmNetId, dataStats->rx_total_packets);
    PS_PRINTF_INFO("[RMNET%d] rx_packets                     %10u\n", rmNetId, dataStats->rx_packets);
    PS_PRINTF_INFO("[RMNET%d] rx_dropped                     %10u\n", rmNetId, dataStats->rx_dropped);
    PS_PRINTF_INFO("[RMNET%d] rx_length_errors               %10u\n", rmNetId, dataStats->rx_length_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_link_errors                 %10u\n", rmNetId, dataStats->rx_link_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_enqueue_errors              %10u\n", rmNetId, dataStats->rx_enqueue_errors);
    PS_PRINTF_INFO("[RMNET%d] rx_trans_errors                %10u\n", rmNetId, dataStats->rx_trans_errors);
}

VOS_VOID RNIC_ShowPsIfaceInfo(VOS_UINT8 rmNetId)
{
    RNIC_PS_IfaceInfo *psIfaceInfo = VOS_NULL_PTR;

    if (rmNetId >= RNIC_NET_ID_MAX_NUM) {
        PS_PRINTF_WARNING("NetId overtop, RmnetId; %d\n", rmNetId);
        return;
    }

    psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    if (psIfaceInfo->modemId < MODEM_ID_BUTT) {
        PS_PRINTF_INFO("[RMNET%d] ModemId            %d\n", rmNetId, psIfaceInfo->modemId);
    } else {
        PS_PRINTF_INFO("[RMNET%d] ModemId            %s\n", rmNetId,
                       (psIfaceInfo->modemId == MODEM_ID_BUTT) ? "MODEM_ID_BUTT" : "Invalid MODEM ID");
    }

    if (psIfaceInfo->ratType < RNIC_PS_RAT_TYPE_BUTT) {
        PS_PRINTF_INFO("[RMNET%d] RatType            %s\n", rmNetId,
                       (psIfaceInfo->ratType == RNIC_PS_RAT_TYPE_3GPP) ? "3GPP" : "IWLAN");
    } else {
        PS_PRINTF_INFO("[RMNET%d] RatType            %s\n", rmNetId,
                       (psIfaceInfo->ratType == RNIC_PS_RAT_TYPE_BUTT) ? "RNIC_PS_RAT_TYPE_BUTT" : "Invalid RAT TYPE");
    }

    PS_PRINTF_INFO("[RMNET%d] IFACE IPV4 STATUS  %d\n", rmNetId, psIfaceInfo->opIpv4Act);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV4 RABID   %d\n", rmNetId, psIfaceInfo->ipv4RabId);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV6 STATUS  %d\n", rmNetId, psIfaceInfo->opIpv6Act);
    PS_PRINTF_INFO("[RMNET%d] IFACE IPV6 RABID   %d\n", rmNetId, psIfaceInfo->ipv6RabId);
    PS_PRINTF_INFO("[RMNET%d] IFACE ETH STATUS   %d\n", rmNetId, psIfaceInfo->opEthAct);
    PS_PRINTF_INFO("[RMNET%d] IFACE ETH RABID    %d\n", rmNetId, psIfaceInfo->ethRabId);
}

VOS_VOID RNIC_ShowNapiInfo(VOS_UINT8 rmNetId)
{
    struct rnic_napi_stats_s napiStats;

    if (rmNetId >= RNIC_NET_ID_MAX_NUM) {
        PS_PRINTF_WARNING("NetId overtop, RmnetId: %d\n", rmNetId);
        return;
    }

    (VOS_VOID)memset_s(&napiStats, sizeof(napiStats), 0x00, sizeof(napiStats));

    /* 调用驱动接口获取Napi统计信息 */
    if (rnic_get_napi_stats(rmNetId, &napiStats)) {
        PS_PRINTF("Get napi stats error.\n");
        return;
    }

    PS_PRINTF_INFO("[RMNET%d] NAPI enable       :(0:disable/1:enable)    %d\n", rmNetId, napiStats.napi_enable);
    PS_PRINTF_INFO("[RMNET%d] GRO  enable       :(0:disable/1:enable)    %d\n", rmNetId, napiStats.gro_enable);
    PS_PRINTF_INFO("[RMNET%d] NapiPollWeight    :                        %d\n", rmNetId, napiStats.napi_weight);
    PS_PRINTF_INFO("[RMNET%d] NapiWeightAdjMode :(0:static/1:dynamic)    %d\n", rmNetId,
                   RNIC_GET_NAPI_WEIGHT_ADJ_MODE(rmNetId));
    PS_PRINTF_INFO("[RMNET%d] NapiMaxQueLen     :                        %d\n", rmNetId,
                   RNIC_GET_NAPI_POLL_QUE_MAX_LEN(rmNetId));
    PS_PRINTF_INFO("[RMNET%d] NAPI LB enable    :(0:disable/1:enable)    %d\n", rmNetId, napiStats.napi_lb_enable);
    PS_PRINTF_INFO("[RMNET%d] LB current level  :                        %d\n", rmNetId, napiStats.lb_level);

    /* for non 0 cpu, select_cpu_num = work_pend_exec_num + napi_sched_hold_num + dispatch_fail_num + schedul_on_cpu_num
     * + schedule_fail_num + smp_fail_num */
    PS_PRINTF_INFO("[RMNET%d] select_cpu_num    : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].select_cpu_num, napiStats.lb_stats[1].select_cpu_num,
                   napiStats.lb_stats[2].select_cpu_num, napiStats.lb_stats[3].select_cpu_num,
                   napiStats.lb_stats[4].select_cpu_num, napiStats.lb_stats[5].select_cpu_num,
                   napiStats.lb_stats[6].select_cpu_num, napiStats.lb_stats[7].select_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] work_pend_exec_num  : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].work_pend_exec_num, napiStats.lb_stats[1].work_pend_exec_num,
                   napiStats.lb_stats[2].work_pend_exec_num, napiStats.lb_stats[3].work_pend_exec_num,
                   napiStats.lb_stats[4].work_pend_exec_num, napiStats.lb_stats[5].work_pend_exec_num,
                   napiStats.lb_stats[6].work_pend_exec_num, napiStats.lb_stats[7].work_pend_exec_num);
    PS_PRINTF_INFO("[RMNET%d] napi_sched_hold_num  : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].napi_sched_hold_num, napiStats.lb_stats[1].napi_sched_hold_num,
                   napiStats.lb_stats[2].napi_sched_hold_num, napiStats.lb_stats[3].napi_sched_hold_num,
                   napiStats.lb_stats[4].napi_sched_hold_num, napiStats.lb_stats[5].napi_sched_hold_num,
                   napiStats.lb_stats[6].napi_sched_hold_num, napiStats.lb_stats[7].napi_sched_hold_num);
    PS_PRINTF_INFO("[RMNET%d] dispatch_fail_num : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].dispatch_fail_num, napiStats.lb_stats[1].dispatch_fail_num,
                   napiStats.lb_stats[2].dispatch_fail_num, napiStats.lb_stats[3].dispatch_fail_num,
                   napiStats.lb_stats[4].dispatch_fail_num, napiStats.lb_stats[5].dispatch_fail_num,
                   napiStats.lb_stats[6].dispatch_fail_num, napiStats.lb_stats[7].dispatch_fail_num);
    PS_PRINTF_INFO("[RMNET%d] schedul_on_cpu_num: %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].schedul_on_cpu_num, napiStats.lb_stats[1].schedul_on_cpu_num,
                   napiStats.lb_stats[2].schedul_on_cpu_num, napiStats.lb_stats[3].schedul_on_cpu_num,
                   napiStats.lb_stats[4].schedul_on_cpu_num, napiStats.lb_stats[5].schedul_on_cpu_num,
                   napiStats.lb_stats[6].schedul_on_cpu_num, napiStats.lb_stats[7].schedul_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] schedule_fail_num : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].schedule_fail_num, napiStats.lb_stats[1].schedule_fail_num,
                   napiStats.lb_stats[2].schedule_fail_num, napiStats.lb_stats[3].schedule_fail_num,
                   napiStats.lb_stats[4].schedule_fail_num, napiStats.lb_stats[5].schedule_fail_num,
                   napiStats.lb_stats[6].schedule_fail_num, napiStats.lb_stats[7].schedule_fail_num);
    PS_PRINTF_INFO("[RMNET%d] smp_fail_num      : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].smp_fail_num, napiStats.lb_stats[1].smp_fail_num,
                   napiStats.lb_stats[2].smp_fail_num, napiStats.lb_stats[3].smp_fail_num,
                   napiStats.lb_stats[4].smp_fail_num, napiStats.lb_stats[5].smp_fail_num,
                   napiStats.lb_stats[6].smp_fail_num, napiStats.lb_stats[7].smp_fail_num);
    PS_PRINTF_INFO("[RMNET%d] smp_on_cpu_num    : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].smp_on_cpu_num, napiStats.lb_stats[1].smp_on_cpu_num,
                   napiStats.lb_stats[2].smp_on_cpu_num, napiStats.lb_stats[3].smp_on_cpu_num,
                   napiStats.lb_stats[4].smp_on_cpu_num, napiStats.lb_stats[5].smp_on_cpu_num,
                   napiStats.lb_stats[6].smp_on_cpu_num, napiStats.lb_stats[7].smp_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] poll_on_cpu_num   : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].poll_on_cpu_num, napiStats.lb_stats[1].poll_on_cpu_num,
                   napiStats.lb_stats[2].poll_on_cpu_num, napiStats.lb_stats[3].poll_on_cpu_num,
                   napiStats.lb_stats[4].poll_on_cpu_num, napiStats.lb_stats[5].poll_on_cpu_num,
                   napiStats.lb_stats[6].poll_on_cpu_num, napiStats.lb_stats[7].poll_on_cpu_num);
    PS_PRINTF_INFO("[RMNET%d] hotplug_online_num: %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].hotplug_online_num, napiStats.lb_stats[1].hotplug_online_num,
                   napiStats.lb_stats[2].hotplug_online_num, napiStats.lb_stats[3].hotplug_online_num,
                   napiStats.lb_stats[4].hotplug_online_num, napiStats.lb_stats[5].hotplug_online_num,
                   napiStats.lb_stats[6].hotplug_online_num, napiStats.lb_stats[7].hotplug_online_num);
    PS_PRINTF_INFO("[RMNET%d] hotplug_down_num  : %d %d %d %d %d %d %d %d\n", rmNetId,
                   napiStats.lb_stats[0].hotplug_down_num, napiStats.lb_stats[1].hotplug_down_num,
                   napiStats.lb_stats[2].hotplug_down_num, napiStats.lb_stats[3].hotplug_down_num,
                   napiStats.lb_stats[4].hotplug_down_num, napiStats.lb_stats[5].hotplug_down_num,
                   napiStats.lb_stats[6].hotplug_down_num, napiStats.lb_stats[7].hotplug_down_num);
    PS_PRINTF_INFO("TetherConnStat              :(0:disconn/1:conn)      %d\n", g_rnicCtx.tetherInfo.tetherConnStat);
    PS_PRINTF_INFO("TetherOrigGroEnable         :(0:disable/1:enable)    %d\n", g_rnicCtx.tetherInfo.origGroEnable);
    PS_PRINTF_INFO("TetherRmnetName             :                        %s\n", g_rnicCtx.tetherInfo.rmnetName);
}

VOS_VOID RNIC_ShowOndemandInfo(VOS_VOID)
{
    PS_PRINTF_INFO("DialMode                             %10u\n", g_rnicCtx.dialMode.dialMode);
    PS_PRINTF_INFO("EventReportFlag                      %10u\n", g_rnicCtx.dialMode.eventReportFlag);
    PS_PRINTF_INFO("IdleTime                             %10u\n", g_rnicCtx.dialMode.idleTime);
    PS_PRINTF_INFO("SendAppDialUpSucc                    %10u\n", g_rnicStats.sendAppDialUpSucc);
    PS_PRINTF_INFO("SendAppDialUpFail                    %10u\n", g_rnicStats.sendAppDialUpFail);
    PS_PRINTF_INFO("SendAppDialDownSucc                  %10u\n", g_rnicStats.sendAppDialDownSucc);
    PS_PRINTF_INFO("SendAppDialDownFail                  %10u\n", g_rnicStats.sendAppDialDownFail);
}

VOS_VOID RNIC_Help(VOS_VOID)
{
    PS_PRINTF_INFO("RNIC Debug Info                    \n");
    PS_PRINTF_INFO("<RNIC_ShowULProcStats(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowDLProcStats(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowPsIfaceInfo(RmnetId)>    \n");
    PS_PRINTF_INFO("<RNIC_ShowNapiInfo(RmnetId)>       \n");
    PS_PRINTF_INFO("<RNIC_ShowOndemandInfo>            \n");
}

