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

#ifndef __DIAG_CFG_COMM_H__
#define __DIAG_CFG_COMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"

#pragma pack(4)
/*****************************************************************************
  2 macro
*****************************************************************************/
/* OSA msg max len */
#define DIAG_OSA_MSG_MAX_LEN      (100*1024)
/*****************************************************************************
  3 Enum
*****************************************************************************/
/*****************************************************************************
  4 UNION
*****************************************************************************/
/*****************************************************************************
  5 struct
*****************************************************************************/
/*****************************************************************************
 描述 : 针对模块的打印开关命令,支持多个命令参数
ID   : DIAG_CMD_LOG_CAT_PRINT
REQ : DIAG_CMD_LOG_CAT_PRINT_REQ_STRU
CNF : DIAG_CMD_LOG_CAT_PRINT_CNF_STRU
IND : DIAG_CMD_LOG_PRINT_RAW_TXT_IND_STRU
*****************************************************************************/

typedef struct
{
    VOS_UINT32 ulModuleId;      /* 如果为0xFFFFFFFF表示为所有模块的Level开关*/
    VOS_UINT32 ulLevelFilter;   /* (0|ERROR|WARNING|NORMAL|INFO|0|0|0) 1:OPEN, 0:CLOSE*/
} DIAG_CMD_LOG_CAT_PRINT_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
    VOS_UINT32 ulModuleId;
} DIAG_CMD_LOG_CAT_PRINT_CNF_STRU;

/*****************************************************************************
描述 : 针对模块的层间开关,支持多个命令参数
ID   : DIAG_CMD_LOG_CAT_LAYER
REQ : DIAG_CMD_LOG_CAT_LAYER_REQ_STRU
CNF : DIAG_CMD_LOG_CAT_LAYER_CNF_STRU
说明: 如果多包命令参数的情况，一个模块错误，就会返回失败，但是正确的模块参数依然会存储
IND : DIAG_CMD_LOG_LAYER_IND_STRU
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulModuleId;       /* 模块ID*/
    VOS_UINT32 ulIsDestModule;   /* 0: 表示Source Module， 1：表示Dest Module*/
    VOS_UINT32 ulSwitch;         /* (0x00000000): 关；(0x80000000)：开*/
} DIAG_CMD_LOG_CAT_LAYER_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
    VOS_UINT32 ulModuleId;
} DIAG_CMD_LOG_CAT_LAYER_CNF_STRU;

/*****************************************************************************
描述 : 针对AIR的总开关,支持多个命令参数
ID   : DIAG_CMD_LOG_CAT_AIR
REQ : DIAG_CMD_LOG_CAT_AIR_REQ_STRU
CNF : DIAG_CMD_LOG_CAT_AIR_CNF_STRU
IND : DIAG_CMD_LOG_AIR_IND_STRU
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulSwitch;        /* (0x00000000): 关；(0x80000000)：开*/
    VOS_UINT32 ulGuSwitch;      /*  1开，0关*/
} DIAG_CMD_LOG_CAT_AIR_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
} DIAG_CMD_LOG_CAT_AIR_CNF_STRU;

/*****************************************************************************
描述 : 针对EVENT的总开关,支持多个命令参数
ID   : DIAG_CMD_LOG_CAT_EVENT
REQ : DIAG_CMD_LOG_CAT_EVENT_REQ_STRU
CNF : DIAG_CMD_LOG_CAT_EVENT_CNF_STRU
IND : DIAG_CMD_LOG_EVENT_IND_STRU
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulSwt;
    VOS_UINT32 ulCount;
    VOS_UINT32 aulModuleId[0];  /*lint !e43 */
} DIAG_CMD_LOG_CAT_EVENT_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
} DIAG_CMD_LOG_CAT_EVENT_CNF_STRU;

/* user plane */
typedef struct
{
    VOS_UINT32 ulSwitch;        /* 1 打开 0关闭 */
}DIAG_CMD_LOG_USERPLANE_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
}DIAG_CMD_LOG_CAT_USERPLANE_CNF_STRU;

/* trans */
typedef struct
{
    VOS_UINT32 ulSwitch;        /* 1 打开 0关闭 */
}DIAG_CMD_LOG_TRANS_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
}DIAG_CMD_LOG_CAT_TRANS_CNF_STRU;

/* time stamp */
typedef struct
{
    VOS_UINT32 ulSwitch;        /* 1 打开 0关闭 */
    VOS_UINT32 ulPeriod;        /* period min */
}DIAG_CMD_LOG_TIMESTAMP_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
}DIAG_CMD_LOG_CAT_TIMESTAMP_CNF_STRU;

/* diag debug switch */
typedef struct
{
    VOS_UINT32 ulSwitch;        /* 1 打开 0关闭 */
}DIAG_CMD_LOG_DIAG_MNTN_REQ_STRU;

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/
}DIAG_CMD_LOG_CAT_DIAG_MNTN_CNF_STRU;

/*****************************************************************************
  6 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  7 Fuction Extern
*****************************************************************************/


#pragma pack()

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
