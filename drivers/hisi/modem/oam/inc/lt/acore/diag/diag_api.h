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


#ifndef __DIAG_API_H__
#define __DIAG_API_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"
#include "msp.h"
#include "diag_cfg.h"
#include "diag_frame.h"
#include "diag_api_comm.h"

#pragma pack(push)
#pragma pack(4)

/*****************************************************************************
  2 macro
*****************************************************************************/

/* diag初始化成功且HSO连接上 */
#define  DIAG_IS_CONN_ON            ((g_ulDiagCfgInfo & (DIAG_CFG_INIT | DIAG_CFG_CONN )) == (DIAG_CFG_INIT | DIAG_CFG_CONN ))

/* 允许LT 空口上报 */
#define  DIAG_IS_LT_AIR_ON          ((g_ulDiagCfgInfo & (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_LT_AIR)) == (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_LT_AIR))

/* 允许GU 空口上报 */
#define  DIAG_IS_GU_AIR_ON          ((g_ulDiagCfgInfo & (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_GU_AIR)) == (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_GU_AIR))

/* 允许事件上报 */
#define  DIAG_IS_EVENT_ON           ((g_ulDiagCfgInfo & (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_EVT)) == (DIAG_CFG_INIT | DIAG_CFG_CONN | DIAG_CFG_EVT))

/* 允许开机(PowerOn)log上报 */
#define  DIAG_IS_POLOG_ON           ((g_ulDiagCfgInfo & (DIAG_CFG_INIT | DIAG_CFG_POWERONLOG)) == (DIAG_CFG_INIT | DIAG_CFG_POWERONLOG))


/* 日志类型定义*/
#define DIAG_CMD_LOG_CATETORY_PRINT_ID              (1<<15)
#define DIAG_CMD_LOG_CATETORY_EVENT_ID              (1<<14)
#define DIAG_CMD_LOG_CATETORY_AIR_ID                (1<<13)
#define DIAG_CMD_LOG_CATETORY_LAYER_ID              (1<<12)
#define DIAG_CMD_LOG_CATETORY_MSG_ID                (1<<10)
#define DIAG_CMD_LOG_CATETORY_USERPLANE_ID          (1<<9)

/* 单帧最大长度 */
#define DIAG_FRAME_MAX_LEN      (4*1024)
/* 单消息最大帧个数 */
#define DIAG_FRMAE_MAX_CNT      (16)
/* 总长度最大值 */
#define DIAG_FRAME_SUM_LEN      (DIAG_FRAME_MAX_LEN * DIAG_FRMAE_MAX_CNT)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
/*物理通道类型枚举*/
enum
{
    DIAG_CPM_OM_PORT_TYPE_USB    = 0,
    DIAG_CPM_OM_PORT_TYPE_VCOM   = 1,
    DIAG_CPM_OM_PORT_TYPE_WIFI   = 2,
    DIAG_CPM_OM_PORT_TYPE_SD     = 3,
    DIAG_CPM_OM_PORT_TYPE_FS     = 4,
    DIAG_CPM_OM_PORT_TYPE_HSIC   = 5,
    DIAG_CBP_OM_PORT_TYPE_BUTT
};

/*****************************************************************************
   5 STRUCT
*****************************************************************************/

/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/
extern VOS_UINT32 diag_FailedCmdCnf(DIAG_FRAME_INFO_STRU *pData, VOS_UINT32 ulErrcode);

/*****************************************************************************
  9 Fuction Extern
*****************************************************************************/
VOS_UINT32 diag_LogPortSwich(VOS_UINT32 ulPhyPort, VOS_BOOL ulEffect);
/*****************************************************************************
 函 数 名      : DIAG_LayerMsgReport
 功能描述  : 层间消息上报接口，用于对OSA消息进行勾包
 输入参数  : pMsg(标准的VOS消息体，源模块、目的模块信息从消息体中获取)
*****************************************************************************/
VOS_VOID DIAG_LayerMsgReport(VOS_VOID *pMsg);

VOS_UINT32 DIAG_ErrorLog(VOS_CHAR * cFileName,VOS_UINT32 ulFileId, VOS_UINT32 ulLine, VOS_UINT32 ulErrNo, VOS_VOID * pBuf, VOS_UINT32 ulLen);
VOS_UINT32 diag_GetPrintPowerOnCfg(VOS_UINT32 ulLevel);
VOS_UINT32 diag_GetLayerMsgCfg(VOS_UINT32 ulCatId, VOS_UINT32 ulMsgId);
VOS_UINT32 diag_GetLayerSrcCfg(VOS_UINT32 ulModuleId);
VOS_UINT32 diag_GetLayerDstCfg(VOS_UINT32 ulModuleId);
VOS_UINT32 diag_GetLayerCfg(VOS_UINT32 ulSrcModuleId, VOS_UINT32 ulDstModuleId, VOS_UINT32 ulMsgId);
VOS_UINT32 diag_GetPrintCfg(VOS_UINT32 ulModuleId, VOS_UINT32 ulLevel);

/*****************************************************************************
  10 OTHERS
*****************************************************************************/

#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of diag_api.h */

