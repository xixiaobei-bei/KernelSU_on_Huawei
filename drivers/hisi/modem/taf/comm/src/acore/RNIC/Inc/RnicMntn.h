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

#ifndef __RNIC_MNTN_H__
#define __RNIC_MNTN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafDiagComm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define RNIC_MNTN_COMM_MOUDLE_ID         (DIAG_GEN_MODULE(MODEM_ID_0, DIAG_MODE_COMM))

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
 结构名称  : RNIC_MNTN_COMM_HEADER_STRU
 结构说明  : RNIC可维可测公共头结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucVer;
    VOS_UINT8                           ucReserved;

    VOS_UINT16                          usReserved0;
    VOS_UINT16                          usReserved1;
    VOS_UINT16                          usReserved2;

} RNIC_MNTN_COMM_HEADER_STRU;

/*****************************************************************************
 结构名称  : RNIC_MNTN_UL_PKT_STATS_STRU
 结构说明  : 上行报文统计信息上报结构
*****************************************************************************/
typedef struct
{
    RNIC_MNTN_COMM_HEADER_STRU          stCommHeader;

    VOS_UINT8                           ucRmNetId;
    VOS_UINT8                           aucReserved0;
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
    VOS_UINT32                          ulUlRecvIpv4PktNum;                     /* RNIC收到上行IPV4数据的个数 */
    VOS_UINT32                          ulUlRecvIpv6PktNum;                     /* RNIC收到上行IPV6数据的个数 */
    VOS_UINT32                          ulUlRecvIpv4v6PktNum;                   /* IPV4V6类型激活时RNIC收到上行数据的个数(IPV4、IPV6) */
    VOS_UINT32                          ulUlRecvUndiaPktNum;                    /* RNIC未激活时收到上行数据的个数 */
    VOS_UINT32                          ulUlSendPktNum;                         /* RNIC上行发送给ADS数据的个数 */
    VOS_UINT32                          ulUlSendPktFailNum;                     /* RNIC调ADS发送上行数据失败的个数 */
    VOS_UINT32                          ulUlIpv4BrdcstPktNum;                   /* RNIC未拨上号前上行收到IPV4广播包的个数 */
    VOS_UINT32                          ulUlImmzcFailPktNum;                    /* RNIC上行转换为IMM_ZC失败的个数 */
    VOS_UINT32                          ulUlRmvMacHdrFailPktNum;                /* RNIC上行去除MAC头失败的个数 */
    VOS_UINT32                          ulUlNetCardDiscardNum;                  /* RNIC网卡私有数据错误丢掉上行数据包的个数 */
    VOS_UINT32                          ulUlFlowCtrlDiscardNum;                 /* RNIC网卡流控丢掉上行数据包的个数 */
    VOS_UINT32                          ulUlRecvErrPktNum;                      /* RNIC收到错误数据包的个数(非ipv4、ipv6包) */
    VOS_UINT32                          ulUlSendAppDialUpSucc;                  /* RNIC成功上报APP按需拨号 */
    VOS_UINT32                          ulUlSendAppDialUpFail;                  /* RNIC上报APP按需拨号失败 */
    VOS_UINT32                          ulUlSendAppDialDownSucc;                /* RNIC成功上报APP断开拨号 */
    VOS_UINT32                          ulUlSendAppDialDownFail;                /* RNIC上报APP断开拨号失败 */
    VOS_UINT32                          ulUlRabIdErr;                           /* Rab id错误 */
    VOS_UINT32                          ulUlNetIdDiscardNum;                    /* RNIC网卡ID错误丢掉上行数据包的个数 */
    VOS_UINT32                          ulUlModemIdDiscardNum;                  /* RNIC Modem ID错误丢掉上行数据包的个数 */

} RNIC_MNTN_UL_PKT_STATS_STRU;

/*****************************************************************************
 结构名称  : RNIC_MNTN_DL_PKT_STATS_STRU
 结构说明  : 下行报文统计信息上报结构
*****************************************************************************/
typedef struct
{
    RNIC_MNTN_COMM_HEADER_STRU          stCommHeader;

    VOS_UINT8                           ucRmNetId;
    VOS_UINT8                           aucReserved0;
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
    VOS_UINT32                          ulDlRecvIpv4PktNum;                     /* RNIC收到下行IPV4数据的个数 */
    VOS_UINT32                          ulDlRecvIpv6PktNum;                     /* RNIC收到下行IPV6数据的个数 */
    VOS_UINT32                          ulDlSendPktNum;                         /* RNIC发送下行数据的个数 */
    VOS_UINT32                          ulDlSendPktFailNum;                     /* RNIC发送下行数据失败的个数 */
    VOS_UINT32                          ulDlRecvBigPktNum;                      /* RNIC收到下行数据包大于MTU的个数 */
    VOS_UINT32                          ulDlDiscardPktNum;                      /* RNIC网卡未激活丢弃的数据个数 */
    VOS_UINT32                          ulDlAddMacHdFailNum;                    /* RNIC下行加MAC头失败的个数 */
    VOS_UINT32                          ulDlNetCardDiscardNum;                  /* RNIC网卡私有数据错误丢掉下行数据包的个数 */
    VOS_UINT32                          ulDlRecvErrPktNum;                      /* RNIC收到错误数据包的个数(非ipv4、ipv6包) */
    VOS_UINT32                          ulDlNetIdDiscardNum;                    /* RNIC网卡ID错误丢掉下行数据包的个数 */

    VOS_UINT32                          ulDlNapiPollQueDiscardPktNum;           /* RNIC网卡下行数据入NAPI Poll buffer队列时，超出队列最大限长，主动丢包个数 */
    VOS_UINT32                          ulDlNapiWeight;                         /* RNIC网卡下行NAPI Weight值 */
    VOS_UINT32                          ulDlNapiRecvPktNum;                     /* RNIC网卡下行NAPI每秒接收的报文数 */

} RNIC_MNTN_DL_PKT_STATS_STRU;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_VOID RNIC_MNTN_TransReport(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLen
);
VOS_VOID RNIC_MNTN_ReportAllStatsInfo(VOS_UINT8 ucRmNetId);


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of RnicMntn.h */
