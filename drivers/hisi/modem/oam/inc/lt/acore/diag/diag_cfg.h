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



#ifndef __DIAG_CFG_H__
#define __DIAG_CFG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"
#include "VosPidDef.h"
#include "nv_stru_msp.h"
#include "msp_diag.h"
#include "diag_common.h"
#include "diag_cfg_comm.h"

#pragma pack(4)

/*****************************************************************************
  2 macro
*****************************************************************************/

#define DIAG_CFG_PID_NUM                        (VOS_PID_BUTT - VOS_PID_DOPRAEND)

#define DIAG_CFG_MODULE_IS_VALID(ulModuleId)    ((ulModuleId < VOS_PID_BUTT)&&(ulModuleId >= VOS_PID_DOPRAEND))
#define DIAG_CFG_MODULE_IS_INVALID(ulModuleId)  ((ulModuleId >= VOS_PID_BUTT)||(ulModuleId < VOS_PID_DOPRAEND))

/*层间消息模块ID合法性检查*/
#define DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId) ((ulModuleId >= VOS_PID_CPU_ID_1_DOPRAEND)&&(ulModuleId < VOS_CPU_ID_1_PID_BUTT))
#define DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId) ((ulModuleId >= VOS_PID_CPU_ID_0_DOPRAEND)&&(ulModuleId < VOS_CPU_ID_0_PID_BUTT))
#define DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId)   ((ulModuleId >= VOS_PID_CPU_ID_2_DOPRAEND)&&(ulModuleId < VOS_CPU_ID_2_PID_BUTT))

#define DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId)  (ulModuleId - VOS_PID_CPU_ID_1_DOPRAEND)
#define DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId)  (ulModuleId - VOS_PID_CPU_ID_0_DOPRAEND)
#define DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId)  (ulModuleId - VOS_PID_CPU_ID_2_DOPRAEND)


/* DIAG已初始化 */
#define DIAG_CFG_INIT                0x00000001

/* HSO已连接 */
#define DIAG_CFG_CONN                0x00000002

/* LT空口消息已打开 */
#define DIAG_CFG_LT_AIR              0x00000004

/* GU空口消息已打开 */
#define DIAG_CFG_GU_AIR              0x00000008

/* EVENT开关已打开 */
#define DIAG_CFG_EVT                 0x00000010

/* 开机log开关已打开 */
#define DIAG_CFG_POWERONLOG          0x00010000

#define DIAG_CMD_DATA_MAX_LEN                       (4096)

#define DIAG_CFG_CAT_CFG_NUM                        (50)

#define DIAG_CMD_CFG_SWT_CLOSE                      (0x00000000)
#define DIAG_CMD_CFG_SWT_OPEN                       (0x80000000)

#define DIAG_GET_PRINT_CFG_SWT(swt)                 (swt>>24)

#define DIAG_GET_CFG_SWT(swt)   ((DIAG_CMD_CFG_SWT_OPEN == swt) ? DIAG_CFG_SWT_OPEN : DIAG_CFG_SWT_CLOSE )

#define DIAG_EVENT_MODID(id)    (id & 0x7fffffff)   /* 低31 bits是PID */
#define DIAG_EVENT_SWT(id)      (id >> 31)          /* 最高bit是开关 */

#define DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE     (0xFFFFFFFF)
#define DIAG_CFG_PRINT_TOTAL_MODULE                 (0xFFFFFFFF)

/* 层间消息模块类型定义*/
#define DIAG_CMD_LAYER_MOD_SRC                      (0)
#define DIAG_CMD_LAYER_MOD_DST                      (1)

#define DIAG_GET_MODEM_INFO                         (0x02000002)

#define DIAG_CMD_LOG_CATETORY_LAYER_ID              (1<<12)

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

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/

/*****************************************************************************
  4 Enum
*****************************************************************************/
enum ENUM_DIAG_CFG_BIT
{
    DIAG_CFG_INIT_BIT     = 0,
    DIAG_CFG_CONN_BIT,
    DIAG_CFG_LT_AIR_BIT,
    DIAG_CFG_GU_AIR_BIT,
    DIAG_CFG_EVENT_BIT,
    DIAG_CFG_BIT_BUTT
};
typedef VOS_UINT32 ENUM_DIAG_CFG_BIT_U32 ;

/*层间、空口、用户面数据的开关采用如下宏定义，注意由U32改成U8*/
enum ENUM_DIAG_CFG_SWT
{
    DIAG_CFG_SWT_CLOSE  = 0,
    DIAG_CFG_SWT_OPEN,
    DIAG_CFG_SWT_BUTT
};
typedef VOS_UINT8 ENUM_DIAG_CFG_SWT_U8;


/*****************************************************************************
   5 STRUCT
*****************************************************************************/
/* 断开连接 */
typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
} DIAG_CMD_HOST_DISCONNECT_CNF_STRU;

/* 获取单板信息 */
typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID */
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理 */
    VOS_UINT32 ulInfo;          /* 查询信息 */
} DIAG_CMD_GET_MDM_INFO_REQ_STRU;

/*****************************************************************************
 描述 : 针对模块的打印开关命令,支持多个命令参数
ID   : DIAG_CMD_REPLAY_SET_CNF_STRU
REQ : DIAG_CMD_REPLAY_SET_REQ_STRU
CNF : DIAG_CMD_REPLAY_SET_CNF_STRU
*****************************************************************************/

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRet;
} DIAG_CMD_REPLAY_SET_CNF_STRU;
/*****************************************************************************
描述 : 针对消息ID/命令ID开关,支持多个命令参数
ID   : DIAG_CMD_LOG_CAT_CFG
REQ : DIAG_CMD_LOG_CAT_CFG_REQ_STRU
CNF : DIAG_CMD_LOG_CAT_CFG_CNF_STRU
说明:   如果多包命令参数的情况，一包MSG ID错误，就会返回失败,但是正确的参数依然会存储;
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulId;            /* 消息ID/命令ID*/
    VOS_UINT32 ulCategory;      /* 取值为 DIAG_CMD_LOG_CATETORY_XXX*/
    VOS_UINT32 ulSwitch;
} DIAG_CMD_LOG_CAT_CFG_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
    VOS_UINT32 ulId;            /* 消息ID/命令ID*/
} DIAG_CMD_LOG_CAT_CFG_CNF_STRU;



typedef struct
{
    VOS_UINT32 ulId;           // 消息ID
    VOS_UINT32 ulSwt;
}DIAG_CFG_LOG_CAT_MSG_CFG_STRU;

typedef struct
{
    VOS_UINT32 ulCatId;
    VOS_UINT32 ulCfgCnt;
    DIAG_CFG_LOG_CAT_MSG_CFG_STRU astMsgCfgList[DIAG_CFG_CAT_CFG_NUM];
}DIAG_CFG_LOG_CAT_CFG_STRU;

/*****************************************************************************
描述 : 针对获取时间戳的参数
CNF : DIAG_TIMESTAMP_CNF_STRU
*****************************************************************************/

typedef struct
{
    VOS_UINT32  ulAuid;                     /* 原AUID*/
    VOS_UINT32  ulSn;                       /* HSO分发，插件命令管理*/
    VOS_UINT    ulErrcode;                  /* 0:成功 1:64位时间戳获取失败 2:32位时间戳获取失败*/
    VOS_UINT    ulGuTimeStampInitValue;     /* 32位时间戳 */
    VOS_UINT64  ulTLTimeStampInitValue;     /* 64位时间戳 */
}DIAG_TIMESTAMP_CNF_STRU;

/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/
extern VOS_UINT32 g_ulDiagCfgInfo;

extern VOS_UINT8 g_ALayerSrcModuleCfg[VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND];
extern VOS_UINT8 g_CLayerSrcModuleCfg[VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND];
extern VOS_UINT8 g_NrmLayerSrcModuleCfg[VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND];
extern VOS_UINT8 g_ALayerDstModuleCfg[VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND];
extern VOS_UINT8 g_CLayerDstModuleCfg[VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND];
extern VOS_UINT8 g_NrmLayerDstModuleCfg[VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND];
extern VOS_UINT32 g_ulTransCfg;
extern VOS_UINT8 g_PrintModuleCfg[DIAG_CFG_PID_NUM];
extern VOS_UINT32 g_PrintTotalCfg;
extern DIAG_CFG_LOG_CAT_CFG_STRU g_stMsgCfg;

/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/


/*****************************************************************************
  9 OTHERS
*****************************************************************************/


VOS_VOID diag_CfgResetAllSwt(VOS_VOID);
VOS_UINT32 diag_CfgSetGlobalBitValue(VOS_UINT32* pstDiagGlobal,ENUM_DIAG_CFG_BIT_U32 enBit,ENUM_DIAG_CFG_SWT_U8 enSwtich);
VOS_UINT32 diag_CfgSetLayerSwt(DIAG_CMD_LOG_CAT_LAYER_REQ_STRU* pstLayerReq, VOS_UINT32 ulCfgSize);
VOS_UINT32 diag_CfgSetMsgSwt(DIAG_CMD_LOG_CAT_CFG_REQ_STRU *pstCatCfgReq,VOS_UINT32 ulCfgSize);
VOS_UINT32 diag_CfgSetPrintSwt(DIAG_CMD_LOG_CAT_PRINT_REQ_STRU* pstPrintReq, VOS_UINT32 ulCfgSize);
VOS_UINT32 diag_GetTimeStampInitValue(VOS_UINT8* pstReq);
VOS_UINT32 diag_GetModemNum(VOS_UINT8* pstReq);
VOS_UINT32 diag_GetPidTable(VOS_UINT8* pstReq);
VOS_UINT32 diag_DisConnProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_DisConnNotifyOtherCpu(VOS_VOID);
VOS_UINT32 diag_EventCfgProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_LayerCfgProc (VOS_UINT8* pstReq);
VOS_UINT32 diag_MsgCfgProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_PrintCfgProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_UsrPlaneCfgProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_AirCfgProc (VOS_UINT8* pstReq);
VOS_VOID diag_AppAgentTimeOutProcEntry(VOS_VOID* pstPara);
VOS_UINT32 diag_TransCfgProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_UserPlaneCfgProc(VOS_UINT8* pstReq);

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

#endif /* end of diag_cfg.h */

