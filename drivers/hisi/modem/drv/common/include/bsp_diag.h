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


#ifndef __BSP_DIAG_H__
#define __BSP_DIAG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <product_config.h>
#include <stdarg.h>
#include "osl_types.h"
#include "bsp_diag_frame.h"
/*****************************************************************************
  2 macro
*****************************************************************************/
/* 单帧最大长度 */
#define DIAG_FRAME_MAX_LEN      (4*1024)
/* 单消息最大帧个数 */
#define DIAG_FRMAE_MAX_CNT      (16)
/* 总长度最大值 */
#define DIAG_FRAME_SUM_LEN      (DIAG_FRAME_MAX_LEN * DIAG_FRMAE_MAX_CNT)

/*4字节对齐*/
#define ALIGN_DDR_WITH_4BYTE(len)       (((len) + 3)&(~3))
/*8字节对齐*/
#define ALIGN_DDR_WITH_8BYTE(len)       (((len) + 7)&(~7))

/*****************************************************************************
  3 Enum
*****************************************************************************/
typedef struct
{
    u32                  ulHisiMagic;   /*"HISI"*/
    u32                  ulDataLen;      /*数据长度*/
}DIAG_SRV_SOCP_HEADER_STRU;

typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
}DIAG_SRV_HEADER_STRU;

/* ==============消息应答上报接口参数====================================== */

/* diag event report上报信息的结构体 */
typedef struct
{
    u32 ulNo;      /* 序号*/
    u32 ulId;      /* 消息或者事件ID,主要针对消息,空口,事件,普通打印输出时该成员为零*/
    u32 ulModule;  /* 打印信息所在的模块ID */
    s8   aucDta[0]; /* 用户数据缓存区*/    /*lint !e43 */
} DIAG_CMD_LOG_EVENT_IND_STRU;

/* diag air report上报信息的结构体 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulSide;       /* 1: NET-->UE, 2: UE-->NET*/
    u32 ulNo;         /* 序号*/
    u32 ulId;         /* ID*/
    s8 aucDta[0];     /* 用户数据缓存区*/  /*lint !e43 */
} DIAG_CMD_LOG_AIR_IND_STRU;

/* diag volte report上报信息的结构体 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulSide;       /* 1: NET-->UE, 2: UE-->NET*/
    u32 ulNo;         /* 序号*/
    u32 ulId;         /* ID*/
    s8   aucDta[0];    /* 用户数据缓存区*/     /*lint !e43 */
} DIAG_CMD_LOG_VOLTE_IND_STRU;


/* diag userplane report上报信息的结构体 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulNo;         /* 序号*/
    u32 ulId;         /* ID*/
    s8 aucDta[0];     /* 用户数据缓存区*/  /*lint !e43 */
} DIAG_CMD_LOG_USERPLANE_IND_STRU;


/* diag 结构化数据上报信息的结构体 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulMsgId;      /* ID*/
    u32 ulNo;         /* 序号*/
    s8   aucDta[0];    /* 用户数据缓存区*/     /*lint !e43 */
} DIAG_CMD_TRANS_IND_STRU;

/* diag drive数据上报信息的结构体和TRANS相同 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulMsgId;      /* ID*/
    u32 ulNo;         /* 序号*/
    s8  aucDta[0];    /* 用户数据缓存区*/     /*lint !e43 */
} DIAG_CMD_DT_IND_STRU;

/* diag trace report上报信息的结构体 */
typedef struct
{
    u32 ulModule;     /* 源模块ID*/
    u32 ulDestMod;    /* 目的模块ID*/
    u32 ulNo;         /* 序号*/
    u32 ulId;         /* ID*/
    s8 aucDta[0];     /* 用户数据缓存区*/  /*lint !e43 */
} DIAG_CMD_LOG_LAYER_IND_STRU;

/* ======================================================================== */

/* CNF类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
}DIAG_API_CNF_HEADER_STRU;

/* 打点类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
}DIAG_SRV_LOG_ID_HEADER_STRU;

/* 打印类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
}DIAG_SRV_LOG_HEADER_STRU;

/* trans 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_TRANS_IND_STRU         trans_header;
}DIAG_SRV_TRANS_HEADER_STRU;

/* trans 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_DT_IND_STRU            dt_header;
}DIAG_SRV_DT_HEADER_STRU;

/* event 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_EVENT_IND_STRU     event_header;
}DIAG_SRV_EVENT_HEADER_STRU;

/* air 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_AIR_IND_STRU       air_header;
}DIAG_SRV_AIR_HEADER_STRU;

/* volte 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_VOLTE_IND_STRU     volte_header;
}DIAG_SRV_VOLTE_HEADER_STRU;

/* trace 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_LAYER_IND_STRU     trace_header;
}DIAG_SRV_TRACE_HEADER_STRU;

/* layer 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_LAYER_IND_STRU     layer_header;
}DIAG_SRV_LAYER_HEADER_STRU;

/* user 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_LOG_USERPLANE_IND_STRU user_header;
}DIAG_SRV_USER_HEADER_STRU;

/* drv log上报消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    diag_print_head_stru            print_head;
}DIAG_DRV_STRING_HEADER_STRU;

/* cnf 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
}DIAG_SRV_CNF_HEADER_STRU;


/* trans 类型消息头 */
typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    diag_frame_head_stru            frame_header;
    DIAG_CMD_TRANS_IND_STRU          comm_header;
}DIAG_SRV_COMM_HEADER_STRU;

/* ======================================================================== */

/* 此结构体与OSA的MsgBlock对应，不能随意修改 */
#pragma pack(1)
typedef struct
{
    u32    ulSenderCpuId;
    u32    ulSenderPid;
    u32    ulReceiverCpuid;
    u32    ulReceiverPid;
    u32    ulLength;
    u32    ulMsgId;
    u8     aucValue[0];   /*lint !e43 */
}DIAG_OSA_MSG_STRU;
#pragma pack()
/*************serviec start*********************/
typedef struct
{
    u32          ulHeaderSize;   /* 数据头的长度 */
    void            *pHeaderData;   /* 数据头 */

    u32          ulDataSize;     /* ucData的长度 */
    void            *pData;         /* 数据 */
}DIAG_MSG_REPORT_HEAD_STRU;



/*****************************************************************************
  4 UNION
*****************************************************************************/

/*****************************************************************************
  5 struct
*****************************************************************************/

/*****************************************************************************
  6 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  7 Fuction Extern
*****************************************************************************/
unsigned int bsp_diag_report_drv_log(unsigned int level, char* fmt, va_list arg);

#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif
