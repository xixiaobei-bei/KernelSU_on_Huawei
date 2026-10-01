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
#include "RnicIntraMsg.h"
#include "rnic_dev_i.h"

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

/* 按需拨号统计信息 */
#define RNIC_DBG_SEND_APP_DIALUP_SUCC_NUM(n)    (g_astRnicStats.ulUlSendAppDialUpSucc += (n))
#define RNIC_DBG_SEND_APP_DIALUP_FAIL_NUM(n)    (g_astRnicStats.ulUlSendAppDialUpFail += (n))
#define RNIC_DBG_SEND_APP_DIALDOWN_SUCC_NUM(n)  (g_astRnicStats.ulUlSendAppDialDownSucc += (n))
#define RNIC_DBG_SEND_APP_DIALDOWN_FAIL_NUM(n)  (g_astRnicStats.ulUlSendAppDialDownFail += (n))

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
 结构名称  : RNIC_MNTN_PKT_STATS_STRU
 结构说明  : 上行报文统计信息上报结构
*****************************************************************************/
typedef struct
{
    RNIC_MNTN_COMM_HEADER_STRU          stCommHeader;

    VOS_UINT8                           ucRmNetId;
    VOS_UINT8                           aucReserved0;
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
    struct rnic_data_stats_s            stDataStats;

} RNIC_MNTN_PKT_STATS_STRU;

/*****************************************************************************
 结构名称  : RNIC_MNTN_NAIP_INFO_STRU
 结构说明  : NAPI统计信息上报结构
*****************************************************************************/
typedef struct
{
    RNIC_MNTN_COMM_HEADER_STRU          stCommHeader;

    VOS_UINT8                           ucRmNetId;
    VOS_UINT8                           ucNapiEnable;
    VOS_UINT8                           ucGroEnable;
    VOS_UINT8                           ucDlNapiWeight;

} RNIC_MNTN_NAIP_INFO_STRU;

/*****************************************************************************
 结构名称  : RNIC_MNTN_DIAL_EVT_STRU
 结构说明  : 按需拨号可维可测消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    RNIC_INTRA_MSG_ID_ENUM_UINT32       enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
} RNIC_MNTN_DIAL_CONN_EVT_STRU;

/*****************************************************************************
 结构名称  : RNIC_MNTN_DIAL_EVT_STRU
 结构说明  : 按需拨号可维可测消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    RNIC_INTRA_MSG_ID_ENUM_UINT32       enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
    VOS_UINT32                          ulPktNum;
    VOS_UINT32                          ulUsrExistFlg;
} RNIC_MNTN_DIAL_DISCONN_EVT_STRU;

/*****************************************************************************
 结构名称  : RNIC_STATS_INFO_STRU
 结构说明  : RNIC统计量
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulUlSendAppDialUpSucc;                  /* RNIC成功上报APP按需拨号 */
    VOS_UINT32                          ulUlSendAppDialUpFail;                  /* RNIC上报APP按需拨号失败 */
    VOS_UINT32                          ulUlSendAppDialDownSucc;                /* RNIC成功上报APP断开拨号 */
    VOS_UINT32                          ulUlSendAppDialDownFail;                /* RNIC上报APP断开拨号失败 */

}RNIC_STATS_INFO_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern RNIC_STATS_INFO_STRU             g_astRnicStats;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_VOID RNIC_MNTN_TransReport(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLen
);
VOS_VOID RNIC_MNTN_ReportPktStats(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_MNTN_ReportNapiInfo(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_MNTN_TraceDialConnEvt(VOS_VOID);
VOS_VOID RNIC_MNTN_TraceDialDisconnEvt(
    VOS_UINT32                          ulPktNum,
    VOS_UINT32                          ulUsrExistFlg
);



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
