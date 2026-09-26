/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
/*
 * 1 Include HeadFile
 */

#include "ndserver_debug.h"
#include "ps_tag.h"
#include "securec.h"
#include "ip_comm.h"
#include "ip_nd_server.h"
#include "ndserver_timer_proc.h"
#include "ndserver_om.h"
#include "ndserver_entity.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERDEBUG_C
/*lint +e767*/

/*
 * Description: 清除统计信息
 * History: VOS_VOID
 *    1.      2011-04-11  Draft Enact
 */
VOS_VOID IP_NDSERVER_DebugInit(VOS_VOID)
{
    VOS_UINT32 addrIndex;
    for (addrIndex = 0; addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM; addrIndex++) {
        (VOS_VOID)memset_s(IP_NDSERVER_GET_STATINFO_ADDR(addrIndex), sizeof(IP_NdserverPacketStatisticsInfo),
                           IP_NULL, sizeof(IP_NdserverPacketStatisticsInfo));
    }

    return;
}

/*
 * 2 Declare the Global Variable
 */
/*lint -e550*/
/*
 * Description: 打印定时器启动信息
 * Input: enTimerType------------------定时器类型
 *                   ulIndex----------------------定时器索引号
 * History:
 *    1.      2010-04-02  Draft Enact
 */
VOS_VOID IP_NDSERVER_PrintTimeStartInfo(VOS_UINT32 addrIndex, NDSER_TimerUint32 timerType)
{
    /* 根据定时器不同类型，打印相应信息 */
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStartInfo:NORM:IP TimerStart(ulIndex): IP_ND_SERVER_TIMER_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStartInfo:NORM:IP TimerStart(ulIndex): IP_ND_SERVER_TIMER_PERIODIC_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_FIRST_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStartInfo:NORM:IP TimerStart(ulIndex): IP_ND_SERVER_TIMER_FIRST_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_RA:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStartInfo:NORM:IP TimerStart(ulIndex): IP_ND_SERVER_TIMER_RA",
                           addrIndex);
            break;
        default:
            break;
    }

    return;
}


/*
 * Description: 打印定时器关闭信息
 * Input: enTimerType------------------定时器类型
 *                   ulIndex----------------------定时器索引号
 * History:
 *    1.      2010-04-02  Draft Enact
 */
VOS_VOID IP_NDSERVER_PrintTimeStopInfo(VOS_UINT32 addrIndex, NDSER_TimerUint32 timerType)
{
    /* 根据定时器不同类型，打印相应信息 */
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStopInfo:NORM:IP TimerStop(ulIndex): IP_ND_SERVER_TIMER_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStopInfo:NORM:IP TimerStop(ulIndex): IP_ND_SERVER_TIMER_PERIODIC_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_FIRST_NS:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStopInfo:NORM:IP TimerStop(ulIndex): IP_ND_SERVER_TIMER_FIRST_NS",
                           addrIndex);
            break;
        case IP_ND_SERVER_TIMER_RA:
            IPND_INFO_LOG1(NDIS_NDSERVER_PID,
                           "IP_NDSERVER_PrintTimeStopInfo:NORM:IP TimerStop(ulIndex): IP_ND_SERVER_TIMER_RA",
                           addrIndex);
            break;
        default:
            break;
    }

    return;
}
/*
 * Function Name: IP_NDSERVER_CmdHelp
 * Description: IP NDSERVER模块命令显示
 * History:
 *    1.      2011-04-11  Draft Enact
 */
VOS_VOID IP_NDSERVER_CmdHelp(VOS_VOID)
{
    PS_PRINTF_ERR("\r\n");
    PS_PRINTF_ERR("********************** IP NDSERVER CMD LIST*********************\r\n");
    PS_PRINTF_ERR("%-30s : %s\r\n", "IP_NDSERVER_ShowLocalNwParamInfo", "show local net para");
    PS_PRINTF_ERR("%-30s : %s\r\n", "IP_NDSERVER_ShowStatInfo(index)", "show stat info(0)");
    PS_PRINTF_ERR("*******************************************************************\r\n");
    PS_PRINTF_ERR("\r\n");

    return;
}

/*
 * Function Name: IP_NDSERVER_ShowLocalNwParamInfo
 * Description: 显示本地保存的网络参数信息
 * History:
 *    1.      2011-04-11  Draft Enact
 */
VOS_VOID IP_NDSERVER_ShowLocalNwParamInfo(VOS_VOID)
{
    PS_PRINTF_ERR("************************Local Nw Param Info***********************\r\n");
    PS_PRINTF_ERR("manage addr config flag %d\r\n", g_ndServerMFlag);
    PS_PRINTF_ERR("other stated config flag %d\r\n", g_ndServerOFlag);
    PS_PRINTF_ERR("router lifetime(s) %d\r\n", g_routerLifetime);
    PS_PRINTF_ERR("Reachable Time(ms) %d\r\n", g_reachableTime);
    PS_PRINTF_ERR("Retrans Timer(ms) %d\r\n", g_retransTimer);
    PS_PRINTF_ERR("Neighbor solicition TimerLen %d\r\n", g_nsTimerLen);
    PS_PRINTF_ERR("Neighbor solicition Timer Max Timerout Num %d\r\n", g_nsTimerMaxExpNum);
    PS_PRINTF_ERR("Periodic NsTimer Len %d\r\n", g_periodicNsTimerLen);
    PS_PRINTF_ERR("PeriodicRaTimerLen(ms) %d\r\n", g_periodicRaTimerLen);
    PS_PRINTF_ERR("FirstNsTimerLen(ms) %d\r\n", g_firstNsTimerLen);
    PS_PRINTF_ERR("RaTimerLen(ms) %d\r\n", g_raTimerLen);

    return;
}

/*
 * Function Name: IP_NDSERVER_ShowStatInfo
 * Description: 显示报文统计信息
 * Input: ulIndex --- 实体索引
 * History:
 *    1.      2011-04-11  Draft Enact
 */
VOS_VOID IP_NDSERVER_ShowStatInfo(VOS_UINT32 addrIndex)
{
    IP_NdserverPacketStatisticsInfo *pktStatInfo = IP_NULL_PTR;
    VOS_UINT32                       tmp;

    if (addrIndex >= IP_NDSERVER_ADDRINFO_MAX_NUM) {
        tmp = IP_NDSERVER_ADDRINFO_MAX_NUM;
        PS_PRINTF_ERR("IP_NDSERVER_ShowStatInfo:input para range:0-%d\r\n", tmp - 1);
        return;
    }

    pktStatInfo = IP_NDSERVER_GET_STATINFO_ADDR(addrIndex);

    PS_PRINTF_ERR("****************************packet stat info(%d)****************************\r\n", addrIndex);
    PS_PRINTF_ERR("Receive Packet Total Num:            %d\r\n", pktStatInfo->rcvPktTotalNum);
    PS_PRINTF_ERR("Discard Packet Num:                  %d\r\n", pktStatInfo->discPktNum);
    PS_PRINTF_ERR("RcvNsPktNum:                         %d\r\n", pktStatInfo->rcvNsPktNum);
    PS_PRINTF_ERR("ulRcvNaPktNum :                      %d\r\n", pktStatInfo->rcvNaPktNum);
    PS_PRINTF_ERR("RcvRsPktNum:                         %d\r\n", pktStatInfo->rcvRsPktNum);
    PS_PRINTF_ERR("RcvEchoPktNum:                       %d\r\n", pktStatInfo->rcvEchoPktNum);
    PS_PRINTF_ERR("RcvTooBigPktNum:                     %d\r\n", pktStatInfo->rcvTooBigPktNum);
    PS_PRINTF_ERR("RcvDhcpv6PktNum:                     %d\r\n", pktStatInfo->rcvDhcpv6PktNum);
    PS_PRINTF_ERR("ErrNsPktNum:                         %d\r\n", pktStatInfo->errNsPktNum);
    PS_PRINTF_ERR("ErrNaPktNum:                         %d\r\n", pktStatInfo->errNaPktNum);
    PS_PRINTF_ERR("ErrRsPktNum :                        %d\r\n", pktStatInfo->errRsPktNum);
    PS_PRINTF_ERR("ErrEchoPktNum:                       %d\r\n", pktStatInfo->errEchoPktNum);
    PS_PRINTF_ERR("ErrTooBigPktNum:                     %d\r\n", pktStatInfo->errTooBigPktNum);
    PS_PRINTF_ERR("ErrDhcpv6PktNum:                     %d\r\n", pktStatInfo->errDhcpv6PktNum);
    PS_PRINTF_ERR("TransPktTotalNum:                    %d\r\n", pktStatInfo->transPktTotalNum);
    PS_PRINTF_ERR("TransPktFailNum:                     %d\r\n", pktStatInfo->transPktFailNum);
    PS_PRINTF_ERR("TransNsPktNum:                       %d\r\n", pktStatInfo->transNsPktNum);
    PS_PRINTF_ERR("TransNaPktNum :                      %d\r\n", pktStatInfo->transNaPktNum);
    PS_PRINTF_ERR("TransRaPktNum :                      %d\r\n", pktStatInfo->transRaPktNum);
    PS_PRINTF_ERR("TransEchoPktNum :                    %d\r\n", pktStatInfo->transEchoPktNum);
    PS_PRINTF_ERR("TransTooBigPktNum :                  %d\r\n", pktStatInfo->transTooBigPktNum);
    PS_PRINTF_ERR("TransDhcpv6PktNum :                  %d\r\n", pktStatInfo->transDhcpv6PktNum);
    PS_PRINTF_ERR("MacInvalidPktNum :                   %d\r\n", pktStatInfo->macInvalidPktNum);
    PS_PRINTF_ERR("EnquePktNum :                        %d\r\n", pktStatInfo->enquePktNum);
    PS_PRINTF_ERR("EnquePktNum :                        %d\r\n", pktStatInfo->sendQuePktNum);

    return;
}




