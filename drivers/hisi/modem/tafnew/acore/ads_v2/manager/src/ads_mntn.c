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

#include "ps_common_def.h"
#include "taf_type_def.h"
#include "ips_mntn.h"
#include "ads_ctx.h"
#include "ads_mntn.h"
#include "ads_log.h"
#include "ads_public.h"
#include "ads_iface_i.h"
#include "ads_msg_proc.h"
#include "securec.h"

#include <linux/slab.h>


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_ADS_MNTN_C

VOS_VOID ADS_MntnTransReport(VOS_UINT32 msgId, VOS_VOID *data, VOS_UINT32 len)
{
    mdrv_diag_trans_ind_s diagTransInd;

    diagTransInd.ulModule  = ADS_MNTN_COMM_MOUDLE_ID;
    diagTransInd.ulPid     = ACPU_PID_ADS_UL;
    diagTransInd.ulMsgId   = msgId;
    diagTransInd.ulReserve = 0;
    diagTransInd.ulLength  = len;
    diagTransInd.pData     = data;

    (VOS_VOID)mdrv_diag_trans_report(&diagTransInd);
}

VOS_VOID ADS_MntnSndIfaceDataStatsInd(VOS_VOID)
{
    struct ads_data_stats_s   *adsDataStats      = VOS_NULL_PTR;
    ADS_V2_MntnIfaceDataStats *adsIfaceDataStats = VOS_NULL_PTR;
    VOS_UINT32                 i;

    adsIfaceDataStats = (ADS_V2_MntnIfaceDataStats *)kzalloc(sizeof(ADS_V2_MntnIfaceDataStats), GFP_ATOMIC);

    if (adsIfaceDataStats == VOS_NULL_PTR) {
        ADS_ERROR_LOG("Alloc ADS_V2_MNTN_IFACE_DATA_STATS_STRU memory fail.");
        return;
    }

    adsIfaceDataStats->commHeader.ver = 100;

    for (i = 0; i < PS_IFACE_ID_BUTT; i++) {
        adsDataStats = ads_iface_get_data_stats(i);

        if (memcpy_s(&(adsIfaceDataStats->dataStats[i]), sizeof(adsIfaceDataStats->dataStats[i]), adsDataStats,
            sizeof(struct ads_data_stats_s)) != EOK) {
            ADS_ERROR_LOG("ADS_MntnSndIfaceDataStatsInd: memcpy failed!");
        }
    }

    ADS_MntnTransReport(ID_DIAG_ADS_V2_IFACE_DATA_STATS_IND, adsIfaceDataStats, sizeof(ADS_V2_MntnIfaceDataStats));

    kfree(adsIfaceDataStats);
    adsIfaceDataStats = VOS_NULL_PTR;
}

VOS_VOID ADS_MntnSndEspeDataStatsInd(VOS_VOID)
{
    struct ads_espe_stats_s *espeStats = VOS_NULL_PTR;
    ADS_V2_MntnEspeStats     stats;

    (VOS_VOID)memset_s(&stats, sizeof(stats), 0x00, sizeof(stats));

    stats.commHeader.ver = 101;

    espeStats = ads_iface_get_espe_stats();

    if (memcpy_s(&(stats.espeStats), sizeof(stats.espeStats), espeStats, sizeof(struct ads_espe_stats_s)) != EOK) {
        ADS_ERROR_LOG("ADS_MntnSndEspeDataStatsInd: memcpy failed!");
    }
    ADS_MntnTransReport(ID_DIAG_ADS_V2_ESPE_DATA_STATS_IND, (VOS_VOID *)&stats, sizeof(ADS_V2_MntnEspeStats));
}

VOS_VOID ADS_MntnSndDsflowStatsInd(VOS_VOID)
{
    struct ads_dsflow_stats_s *dsflowStats = VOS_NULL_PTR;
    ADS_V2_MntnDsflowStats     stats;

    (VOS_VOID)memset_s(&stats, sizeof(stats), 0x00, sizeof(stats));

    stats.commHeader.ver = 100;

    dsflowStats = ads_iface_get_dsflow_stats();

    if (memcpy_s(&(stats.dsFlowStats), sizeof(stats.dsFlowStats), dsflowStats, sizeof(struct ads_dsflow_stats_s)) != EOK) {
        ADS_ERROR_LOG("ADS_MntnSndDsflowStatsInd: memcpy failed!");
    }

    ADS_MntnTransReport(ID_DIAG_ADS_V2_DSFLOW_STATS_IND, (VOS_VOID *)&stats, sizeof(ADS_V2_MntnDsflowStats));
}

VOS_VOID ADS_MntnSndDlPktDataStatsInd(struct ads_rx_ip_pkt_rec_s *recStru)
{
    VOS_UINT16 reportSize = 0;

    if (recStru->rpt_num == 0) {
        return;
    }

    reportSize = (VOS_UINT16)(sizeof(struct ads_ip_pkt_info_s) * (recStru->rpt_num) + (sizeof(VOS_UINT8) * 8));

    recStru->ver = 103;

    ADS_MntnTransReport(ID_DIAG_ADS_V2_DL_PKT_INFO_STATS_IND, (VOS_VOID *)recStru, reportSize);
}

VOS_VOID ADS_MntnSndUlPktDataStatsInd(struct ads_tx_ip_pkt_rec_s *recStru)
{
    VOS_UINT16 reportSize = 0;

    if (recStru->rpt_num == 0) {
        return;
    }

    reportSize = (VOS_UINT16)(sizeof(struct ads_ip_pkt_info_s) * (recStru->rpt_num) + (sizeof(VOS_UINT8) * 8));

    recStru->ver = 103;

    ADS_MntnTransReport(ID_DIAG_ADS_V2_UL_PKT_INFO_STATS_IND, (VOS_VOID *)recStru, reportSize);
}

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)

VOS_VOID ADS_MntnSndLtevUlPktDataStatsInd(struct ads_tx_ltev_pkt_rec_s *ltevUlPktRec)
{
    VOS_UINT16 usReportSize = 0;

    if (ltevUlPktRec->rpt_num == 0) {
        return;
    }

    usReportSize = (VOS_UINT16)(sizeof(struct ads_ltev_ul_info_s) * (ltevUlPktRec->rpt_num) + (sizeof(VOS_UINT8) * 4) +
                                sizeof(VOS_UINT32));

    ltevUlPktRec->ver = 103;

    ADS_MntnTransReport(ID_DIAG_ADS_UL_PC5_INFO_STATS_IND, (VOS_VOID *)ltevUlPktRec, usReportSize);

    ltevUlPktRec->rpt_num = 0;
}

VOS_VOID ADS_MntnSndLtevDlPktDataStatsInd(struct ads_rx_ltev_pkt_rec_s *ltevDlPktRec)
{
    VOS_UINT16 usReportSize = 0;

    if (ltevDlPktRec->rpt_num == 0) {
        return;
    }

    usReportSize = (VOS_UINT16)(sizeof(struct ads_ltev_dl_info_s) * (ltevDlPktRec->rpt_num) + (sizeof(VOS_UINT8) * 4) +
                                sizeof(VOS_UINT32));

    ltevDlPktRec->ver = 103;

    ADS_MntnTransReport(ID_DIAG_ADS_DL_PC5_INFO_STATS_IND, (VOS_VOID *)ltevDlPktRec, usReportSize);

    ltevDlPktRec->rpt_num = 0;
}
#endif

VOS_VOID ADS_MntnReportAllStatsInfo(VOS_VOID)
{
    ADS_MntnSndDsflowStatsInd();
    ADS_MntnSndIfaceDataStatsInd();
    ADS_MntnSndEspeDataStatsInd();
}

VOS_VOID ADS_MntnDiagConnStateNotifyCB(mdrv_diag_state_e state)
{
    if (state == DIAG_STATE_DISCONN) {
        ADS_SndDplConfigInd(VOS_FALSE);
    }
}

VOS_VOID ADS_MntnTraceCfgNotifyCB(VOS_BOOL bTraceEnable)
{
    ADS_SndDplConfigInd(bTraceEnable);
}

VOS_VOID ADS_Help(VOS_VOID)
{
    PS_PRINTF_INFO("ADS DEBUG ENTRY                                    \n");
    PS_PRINTF_INFO("<ADS_ShowIfaceInfo>                            \n");
    PS_PRINTF_INFO("<ADS_ShowIfaceDataStats(ifaceid)>              \n");
    PS_PRINTF_INFO("<ADS_ShowDsFlowStats>                          \n");
    PS_PRINTF_INFO("<ADS_ShowEspeDataStats>                        \n");
}

VOS_VOID ADS_ShowIfaceInfo(VOS_VOID)
{
    ADS_CONTEXT *adsCtx = VOS_NULL_PTR;
    VOS_UINT32   i;

    adsCtx = ADS_GetCtx();

    for (i = 0; i < PS_IFACE_ID_BUTT; i++) {
        if (!ADS_BIT64_IS_SET(adsCtx->ifaceStatsBitMask, i)) {
            continue;
        }

        PS_PRINTF_INFO("ADS IFACE INFO                     \n");
        PS_PRINTF_INFO("Iface ID                     %d\n", adsCtx->ifaceInfo[i].ifaceId);
        PS_PRINTF_INFO("Iface Status                 %d\n", adsCtx->ifaceInfo[i].stats);
        PS_PRINTF_INFO("Iface TransMode              %d\n", adsCtx->ifaceInfo[i].transMode);
        PS_PRINTF_INFO("Iface ModemId                %d\n", adsCtx->ifaceInfo[i].modemId);
        PS_PRINTF_INFO("Iface IPv4 PduSessionId      %d\n", adsCtx->ifaceInfo[i].pduSessionTbl[0]);
        PS_PRINTF_INFO("Iface IPv6 PduSessionId      %d\n", adsCtx->ifaceInfo[i].pduSessionTbl[1]);
        PS_PRINTF_INFO("Iface PPP PduSessionId       %d\n", adsCtx->ifaceInfo[i].pduSessionTbl[2]);
    }
}

VOS_VOID ADS_ShowDsFlowStats(VOS_VOID)
{
    struct ads_dsflow_stats_s *dsflowStats = VOS_NULL_PTR;

    dsflowStats = ads_iface_get_dsflow_stats();

    PS_PRINTF_INFO("ADS DSFLOW STATS START:                  \n");
    PS_PRINTF_INFO("rx_packets                       %10u\n", dsflowStats->rx_packets);
    PS_PRINTF_INFO("tx_packets                       %10u\n", dsflowStats->tx_packets);
    PS_PRINTF_INFO("rx_bytes                         %10u\n", dsflowStats->rx_bytes);
    PS_PRINTF_INFO("tx_bytes                         %10u\n", dsflowStats->tx_bytes);
    PS_PRINTF_INFO("ADS DSFLOW STATS END.                    \n");
}

VOS_VOID ADS_ShowIfaceDataStats(VOS_UINT8 ifaceId)
{
    struct ads_data_stats_s *dataStats = VOS_NULL_PTR;

    if (ifaceId >= PS_IFACE_ID_BUTT) {
        PS_PRINTF_INFO("Invaild IfaceId! Ifcace Id Range is 0 ~ %d\n", PS_IFACE_ID_BUTT - 1);
        return;
    }

    dataStats = ads_iface_get_data_stats(ifaceId);

    PS_PRINTF_INFO("ADS IFACE DATA STATS START:              \n");
    PS_PRINTF_INFO("iface_id                         %10u\n", ifaceId);
    PS_PRINTF_INFO("rx_total_packets                 %10u\n", dataStats->rx_total_packets);
    PS_PRINTF_INFO("tx_total_packets                 %10u\n", dataStats->tx_total_packets);
    PS_PRINTF_INFO("rx_packets                       %10u\n", dataStats->rx_packets);
    PS_PRINTF_INFO("tx_packets                       %10u\n", dataStats->tx_packets);
    PS_PRINTF_INFO("rx_dropped                       %10u\n", dataStats->rx_dropped);
    PS_PRINTF_INFO("tx_dropped                       %10u\n", dataStats->tx_dropped);
    PS_PRINTF_INFO("rx_state_errors                  %10u\n", dataStats->rx_state_errors);
    PS_PRINTF_INFO("rx_carrier_errors                %10u\n", dataStats->rx_carrier_errors);
    PS_PRINTF_INFO("tx_length_errors                 %10u\n", dataStats->tx_length_errors);
    PS_PRINTF_INFO("tx_state_errors                  %10u\n", dataStats->tx_state_errors);
    PS_PRINTF_INFO("ADS IFACE DATA STATS END.              \n");
}

VOS_VOID ADS_ShowEspeDataStats(VOS_VOID)
{
    struct ads_espe_stats_s *espeStats = VOS_NULL_PTR;

    espeStats = ads_iface_get_espe_stats();

    PS_PRINTF_INFO("ADS EPSE DATA STATS START:               \n");
    PS_PRINTF_INFO("rx_total_packets                 %10u\n", espeStats->rx_total_packets);
    PS_PRINTF_INFO("tx_total_packets                 %10u\n", espeStats->tx_total_packets);
    PS_PRINTF_INFO("rx_packets                       %10u\n", espeStats->rx_packets);
    PS_PRINTF_INFO("tx_packets                       %10u\n", espeStats->tx_packets);
    PS_PRINTF_INFO("rx_dropped                       %10u\n", espeStats->rx_dropped);
    PS_PRINTF_INFO("tx_dropped                       %10u\n", espeStats->tx_dropped);
    PS_PRINTF_INFO("rx_filter_errors                 %10u\n", espeStats->rx_filter_errors);
    PS_PRINTF_INFO("rx_iface_errors                  %10u\n", espeStats->rx_iface_errors);
    PS_PRINTF_INFO("rx_discard_errors                %10u\n", espeStats->rx_discard_errors);
    PS_PRINTF_INFO("tx_wan_errors                    %10u\n", espeStats->tx_wan_errors);
    PS_PRINTF_INFO("ADS EPSE DATA STATS END.                 \n");
}

