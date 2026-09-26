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

#ifndef __RNIC_MNTN_H__
#define __RNIC_MNTN_H__

#include "vos.h"
#include "ps_common_def.h"
#include "taf_diag_comm.h"
#include "rnic_intra_msg.h"
#include "rnic_dev_i.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define RNIC_MNTN_COMM_MOUDLE_ID (MDRV_DIAG_GEN_MODULE(MODEM_ID_0, DIAG_MODE_COMM))

/* 按需拨号统计信息 */
#define RNIC_DBG_SEND_APP_DIALUP_SUCC_NUM(n) (g_rnicStats.sendAppDialUpSucc += (n))
#define RNIC_DBG_SEND_APP_DIALUP_FAIL_NUM(n) (g_rnicStats.sendAppDialUpFail += (n))
#define RNIC_DBG_SEND_APP_DIALDOWN_SUCC_NUM(n) (g_rnicStats.sendAppDialDownSucc += (n))
#define RNIC_DBG_SEND_APP_DIALDOWN_FAIL_NUM(n) (g_rnicStats.sendAppDialDownFail += (n))

/*
 * 结构说明: RNIC可维可测公共头结构
 */
typedef struct {
    VOS_UINT8 ver;
    VOS_UINT8 reserved;

    VOS_UINT16 reserved0;
    VOS_UINT16 reserved1;
    VOS_UINT16 reserved2;

} RNIC_MntnCommHeader;

/*
 * 结构说明: 上行报文统计信息上报结构
 */
typedef struct {
    RNIC_MntnCommHeader commHeader;

    VOS_UINT8                rmNetId;
    VOS_UINT8                reserved0;
    VOS_UINT8                reserved1;
    VOS_UINT8                reserved2;
    struct rnic_data_stats_s dataStats;

} RNIC_MntnPktStats;

/*
 * 结构说明: NAPI统计信息上报结构
 */
typedef struct {
    RNIC_MntnCommHeader commHeader;

    VOS_UINT8                rmNetId;
    VOS_UINT8                reserved0;
    VOS_UINT8                reserved1;
    VOS_UINT8                reserved2;
    struct rnic_napi_stats_s napiStats;

} RNIC_MntnNaipInfo;

/*
 * 结构说明: 按需拨号可维可测消息结构
 */
typedef struct {
    VOS_MSG_HEADER               /* 消息头 */ /* _H2ASN_Skip */
    RNIC_IntraMsgIdUint32 msgId; /* 消息ID */ /* _H2ASN_Skip */
} RNIC_MntnDialConnEvt;

/*
 * 结构说明: 按需拨号可维可测消息结构
 */
typedef struct {
    VOS_MSG_HEADER               /* 消息头 */ /* _H2ASN_Skip */
    RNIC_IntraMsgIdUint32 msgId; /* 消息ID */ /* _H2ASN_Skip */
    VOS_UINT32            pktNum;
    VOS_UINT32            usrExistFlg;
} RNIC_MntnDialDisconnEvt;

/*
 * 结构说明: RNIC统计量
 */
typedef struct {
    VOS_UINT32 sendAppDialUpSucc;   /* RNIC成功上报APP按需拨号 */
    VOS_UINT32 sendAppDialUpFail;   /* RNIC上报APP按需拨号失败 */
    VOS_UINT32 sendAppDialDownSucc; /* RNIC成功上报APP断开拨号 */
    VOS_UINT32 sendAppDialDownFail; /* RNIC上报APP断开拨号失败 */

} RNIC_StatsInfo;

extern RNIC_StatsInfo g_rnicStats;

VOS_VOID RNIC_MntnTransReport(VOS_UINT32 msgId, VOS_VOID *data, VOS_UINT32 len);
VOS_VOID RNIC_MntnReportPktStats(VOS_UINT8 rmNetId);
VOS_VOID RNIC_MntnReportNapiInfo(VOS_UINT8 rmNetId);
VOS_VOID RNIC_MntnTraceDialConnEvt(VOS_VOID);
VOS_VOID RNIC_MntnTraceDialDisconnEvt(VOS_UINT32 pktNum, VOS_UINT32 usrExistFlg);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of rnic_mntn.h */
