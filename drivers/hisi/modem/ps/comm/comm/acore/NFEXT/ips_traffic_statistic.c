/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019.All rights reserved.
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

#include "ips_traffic_statistic.h"
#include "ps_common_def.h"
#include "PsLogdef.h"
#include "gucttf_tag.h"
#include "ttf_comm.h"

#define THIS_FILE_ID PS_FILE_ID_ACPU_IPS_TRAFFIC_STATISTIC_C
#define THIS_MODU mod_nfext

#define IPS_FILTER_ALPHA_TATOL 10
#define IPS_FILTER_ALPHA 1
#define IPS_FILTER_DELTA_ALPHA (IPS_FILTER_ALPHA_TATOL - IPS_FILTER_ALPHA)
#define IPS_TRACE_OVERDUE_TIME 1000
#define IPS_TRACE_SPEED_PERIOD 100
#define TIME_STAMP_CNT_TO_MS(a, b) (((a) * 1000) / (b))
#define IPS_SPEED_BYTE_TO_KB(a) ((a) / 1024)
#define IPS_FILTER_MAX_SPEED_DEFAULT 5000  /* USB2.0 要求IPS LOG速率大于5MBps(5%误差)时关闭 */

typedef struct {
    VOS_UINT32 pktLen;
    VOS_UINT32 lastTimeStamp;
    VOS_UINT32 avgSpeed;  /* 速度计算以KBps为单位 */
} IPS_MntnPktState;

STATIC VOS_BOOL         g_trafficCtrlOn          = VOS_FALSE;
STATIC VOS_UINT32       g_timeStampFreq          = 0;
STATIC VOS_UINT32       g_ipsTrafficCtrlMaxSpeed = IPS_FILTER_MAX_SPEED_DEFAULT;
STATIC VOS_SPINLOCK     g_lockIpsMntnPktStateFresh;
STATIC IPS_MntnPktState g_ipsTrafficCtrlEntity   = {0};

VOS_VOID IPSMNTN_TrafficCtrlInit(VOS_VOID)
{
    g_timeStampFreq = mdrv_get_normal_timestamp_freq();
    VOS_SpinLockInit(&g_lockIpsMntnPktStateFresh);
}

/* 接收到OM配置可维可测信息捕获配置请求 */
VOS_VOID IPSMNTN_TrafficCtrlReq(const MsgBlock* msg)
{
    const OM_IpsMntnTrafficCtrlReq* rcvMsg = (OM_IpsMntnTrafficCtrlReq*)msg;
    g_trafficCtrlOn = rcvMsg->trafficCtrlOn;
    g_ipsTrafficCtrlMaxSpeed = (rcvMsg->speedKBps != 0) ? rcvMsg->speedKBps : IPS_FILTER_MAX_SPEED_DEFAULT;

    TTF_LOG3(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,
        "IPS Traffic Ctrl State: <1>, At Max Speed <2>, Now avg Speed: <3>\n", g_trafficCtrlOn,
        g_ipsTrafficCtrlMaxSpeed, g_ipsTrafficCtrlEntity.avgSpeed);
}

STATIC VOS_UINT64 IPSMNTN_GetPktSpeed(VOS_UINT32 deltaTime)
{
    VOS_UINT32 alphaSpeed, nowSpeed;
    if (deltaTime == 0) {
        return 0;
    }

    alphaSpeed = (IPS_SPEED_BYTE_TO_KB(g_ipsTrafficCtrlEntity.pktLen) * g_timeStampFreq) / deltaTime;
    nowSpeed   = (alphaSpeed * IPS_FILTER_ALPHA) + (g_ipsTrafficCtrlEntity.avgSpeed * IPS_FILTER_DELTA_ALPHA);
    nowSpeed  /= IPS_FILTER_ALPHA_TATOL;
    return nowSpeed;
}

STATIC VOS_VOID IPSMNTN_UpdateTrafficCtrlInfo(VOS_UINT32 infoLen)
{
    VOS_UINT32 deltaTime, deltaTimeMs;
    VOS_UINT32 nowTimeStamp = mdrv_timer_get_normal_timestamp();
    VOS_ULONG  flag         = 0UL;
    VOS_SpinLockIntLock(&g_lockIpsMntnPktStateFresh, flag);
    deltaTime = nowTimeStamp - g_ipsTrafficCtrlEntity.lastTimeStamp;

    deltaTimeMs = TIME_STAMP_CNT_TO_MS(deltaTime, g_timeStampFreq);
    if (deltaTimeMs < IPS_TRACE_SPEED_PERIOD) {
        g_ipsTrafficCtrlEntity.pktLen += infoLen;
        VOS_SpinUnlockIntUnlock(&g_lockIpsMntnPktStateFresh, flag);
        return;
    }

    g_ipsTrafficCtrlEntity.avgSpeed      = (deltaTimeMs >= IPS_TRACE_OVERDUE_TIME) ? 0 : IPSMNTN_GetPktSpeed(deltaTime);
    g_ipsTrafficCtrlEntity.lastTimeStamp = nowTimeStamp;
    g_ipsTrafficCtrlEntity.pktLen        = infoLen;
    VOS_SpinUnlockIntUnlock(&g_lockIpsMntnPktStateFresh, flag);
}

VOS_BOOL IPSMNTN_PktIsOverFlow(VOS_UINT32 infoLen)
{
    if ((g_trafficCtrlOn == VOS_FALSE) || (g_timeStampFreq == 0)) {
        return VOS_FALSE;
    }

    IPSMNTN_UpdateTrafficCtrlInfo(infoLen);
    return ((g_ipsTrafficCtrlEntity.avgSpeed >= g_ipsTrafficCtrlMaxSpeed) ? VOS_TRUE : VOS_FALSE);
}
