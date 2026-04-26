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

#ifndef __ADS_MNTN_H__
#define __ADS_MNTN_H__

#include "vos.h"
#include "mdrv.h"
#include "ps_tag.h"
#include "taf_diag_comm.h"
#include "imm_interface.h"
#include "ads_public.h"
#include "ips_mntn.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define ADS_MNTN_COMM_MOUDLE_ID (MDRV_DIAG_GEN_MODULE(MODEM_ID_0, DIAG_MODE_COMM))

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU ps_nas
#endif

/*
 * 结构说明: ADS可维可测公共头结构
 */
typedef struct {
    VOS_UINT8 ver;
    VOS_UINT8 reserved;

    VOS_UINT16 reserved0;
    VOS_UINT16 reserved1;
    VOS_UINT16 reserved2;

} ADS_MntnCommHeader;

/*
 * 结构说明: 流量统计信息上报结构
 */
typedef struct {
    ADS_MntnCommHeader commHeader;

    struct ads_dsflow_stats_s dsFlowStats;
} ADS_V2_MntnDsflowStats;

/*
 * 结构说明: 网卡接口信息上报结构
 */
typedef struct {
    ADS_MntnCommHeader commHeader;

    struct ads_data_stats_s dataStats[PS_IFACE_ID_BUTT];
} ADS_V2_MntnIfaceDataStats;

/*
 * 结构说明: 网卡接口信息上报结构
 */
typedef struct {
    ADS_MntnCommHeader commHeader;

    struct ads_espe_stats_s espeStats;
} ADS_V2_MntnEspeStats;

VOS_VOID ADS_MntnReportAllStatsInfo(VOS_VOID);
VOS_VOID RNIC_MntnReportAllStatsInfo(VOS_VOID);

VOS_VOID ADS_MntnSndUlPktDataStatsInd(struct ads_tx_ip_pkt_rec_s *recStru);
VOS_VOID ADS_MntnSndDlPktDataStatsInd(struct ads_rx_ip_pkt_rec_s *recStru);
VOS_VOID ADS_MntnDiagConnStateNotifyCB(mdrv_diag_state_e state);
VOS_VOID ADS_MntnTraceCfgNotifyCB(VOS_BOOL bTraceEnable);
#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
VOS_VOID ADS_MntnSndLtevUlPktDataStatsInd(struct ads_tx_ltev_pkt_rec_s *ltevUlPktRec);
VOS_VOID ADS_MntnSndLtevDlPktDataStatsInd(struct ads_rx_ltev_pkt_rec_s *ltevDlPktRec);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_MNTN_H__ */
