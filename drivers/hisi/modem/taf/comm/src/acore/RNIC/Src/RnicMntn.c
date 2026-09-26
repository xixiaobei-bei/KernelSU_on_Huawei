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
#include "RnicDebug.h"
#include "RnicMntn.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_RNIC_MNTN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

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


VOS_VOID RNIC_MNTN_ReportULPktStats(VOS_UINT8 ucRmNetId)
{
    RNIC_MNTN_UL_PKT_STATS_STRU         stStats;

    stStats.stCommHeader.ucVer          = 101;
    stStats.stCommHeader.ucReserved     = 0;

    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ucRmNetId                   = ucRmNetId;
    stStats.aucReserved0                = 0;
    stStats.aucReserved1                = 0;
    stStats.aucReserved2                = 0;
    stStats.ulUlRecvIpv4PktNum          = g_astRnicStats[ucRmNetId].ulUlRecvIpv4PktNum;
    stStats.ulUlRecvIpv6PktNum          = g_astRnicStats[ucRmNetId].ulUlRecvIpv6PktNum;
    stStats.ulUlRecvIpv4v6PktNum        = g_astRnicStats[ucRmNetId].ulUlRecvIpv4v6PktNum;
    stStats.ulUlRecvUndiaPktNum         = g_astRnicStats[ucRmNetId].ulUlRecvUndiaPktNum;
    stStats.ulUlSendPktNum              = g_astRnicStats[ucRmNetId].ulUlSendPktNum;
    stStats.ulUlSendPktFailNum          = g_astRnicStats[ucRmNetId].ulUlSendPktFailNum;
    stStats.ulUlIpv4BrdcstPktNum        = g_astRnicStats[ucRmNetId].ulUlIpv4BrdcstPktNum;
    stStats.ulUlImmzcFailPktNum         = g_astRnicStats[ucRmNetId].ulUlImmzcFailPktNum;
    stStats.ulUlRmvMacHdrFailPktNum     = g_astRnicStats[ucRmNetId].ulUlRmvMacHdrFailPktNum;
    stStats.ulUlNetCardDiscardNum       = g_astRnicStats[ucRmNetId].ulUlNetCardDiscardNum;
    stStats.ulUlFlowCtrlDiscardNum      = g_astRnicStats[ucRmNetId].ulUlFlowCtrlDiscardNum;
    stStats.ulUlRecvErrPktNum           = g_astRnicStats[ucRmNetId].ulUlRecvErrPktNum;
    stStats.ulUlSendAppDialUpSucc       = g_astRnicStats[ucRmNetId].ulUlSendAppDialUpSucc;
    stStats.ulUlSendAppDialUpFail       = g_astRnicStats[ucRmNetId].ulUlSendAppDialUpFail;
    stStats.ulUlSendAppDialDownSucc     = g_astRnicStats[ucRmNetId].ulUlSendAppDialDownSucc;
    stStats.ulUlSendAppDialDownFail     = g_astRnicStats[ucRmNetId].ulUlSendAppDialDownFail;
    stStats.ulUlRabIdErr                = g_astRnicStats[ucRmNetId].ulUlRabIdErr;
    stStats.ulUlNetIdDiscardNum         = g_astRnicStats[ucRmNetId].ulUlNetIdDiscardNum;
    stStats.ulUlModemIdDiscardNum       = g_astRnicStats[ucRmNetId].ulUlModemIdDiscardNum;


    RNIC_MNTN_TransReport(ID_DIAG_RNIC_UL_PKT_STATS_IND,
                          (VOS_VOID *)&stStats,
                          sizeof(RNIC_MNTN_UL_PKT_STATS_STRU));
    return;
}


VOS_VOID RNIC_MNTN_ReportDLPktStats(VOS_UINT8 ucRmNetId)
{
    RNIC_MNTN_DL_PKT_STATS_STRU         stStats;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv;

    pstPriv = RNIC_GET_SPEC_NET_PRIV_PTR(ucRmNetId);
    stStats.stCommHeader.ucVer          = 101;
    stStats.stCommHeader.ucReserved     = 0;

    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ucRmNetId                   = ucRmNetId;
    stStats.aucReserved0                = 0;
    stStats.aucReserved1                = 0;
    stStats.aucReserved2                = 0;
    stStats.ulDlRecvIpv4PktNum          = g_astRnicStats[ucRmNetId].ulDlRecvIpv4PktNum;
    stStats.ulDlRecvIpv6PktNum          = g_astRnicStats[ucRmNetId].ulDlRecvIpv6PktNum;
    stStats.ulDlSendPktNum              = g_astRnicStats[ucRmNetId].ulDlSendPktNum;
    stStats.ulDlSendPktFailNum          = g_astRnicStats[ucRmNetId].ulDlSendPktFailNum;
    stStats.ulDlRecvBigPktNum           = g_astRnicStats[ucRmNetId].ulDlRecvBigPktNum;
    stStats.ulDlDiscardPktNum           = g_astRnicStats[ucRmNetId].ulDlDiscardPktNum;
    stStats.ulDlAddMacHdFailNum         = g_astRnicStats[ucRmNetId].ulDlAddMacHdFailNum;
    stStats.ulDlNetCardDiscardNum       = g_astRnicStats[ucRmNetId].ulDlNetCardDiscardNum;
    stStats.ulDlRecvErrPktNum           = g_astRnicStats[ucRmNetId].ulDlRecvErrPktNum;
    stStats.ulDlNetIdDiscardNum         = g_astRnicStats[ucRmNetId].ulDlNetIdDiscardNum;
    if (VOS_NULL_PTR != pstPriv)
    {
        stStats.ulDlNapiWeight          = (VOS_UINT32)pstPriv->stNapi.weight;
    }
    else
    {
        stStats.ulDlNapiWeight          = 255;
    }
    stStats.ulDlNapiRecvPktNum          = RNIC_GET_NAPI_RECV_PKT_NUM(ucRmNetId);
    stStats.ulDlNapiPollQueDiscardPktNum = g_astRnicStats[ucRmNetId].ulDlNapiPollQueDiscardPktNum;

    RNIC_MNTN_TransReport(ID_DIAG_RNIC_DL_PKT_STATS_IND,
                          (VOS_VOID *)&stStats,
                          sizeof(RNIC_MNTN_DL_PKT_STATS_STRU));
    return;
}


VOS_VOID RNIC_MNTN_ReportAllStatsInfo(VOS_UINT8 ucRmNetId)
{
    RNIC_MNTN_ReportULPktStats(ucRmNetId);
    RNIC_MNTN_ReportDLPktStats(ucRmNetId);

    return;
}


