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


#ifndef __DIAG_COMMON_COMM_H__
#define __DIAG_COMMON_COMM_H__

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
#include "msp_debug.h"
#include "diag_message_comm.h"
#include "diag_service_comm.h"
#include "diag_frame.h"
//#include "msp_tag.h"

/*****************************************************************************
  2 macro
*****************************************************************************/
#undef THIS_MODU
#define THIS_MODU "diag"

#define diag_crit(fmt,...)     mdrv_err(fmt, ##__VA_ARGS__)
#define diag_error(fmt,...)    mdrv_err("<%s %d>"fmt, __FUNCTION__, __LINE__,##__VA_ARGS__)
#define diag_info(fmt, ...)    mdrv_info(fmt, ##__VA_ARGS__)

/* debug定时器时长 */
#define DIAG_DEBUG_TIMER_LEN                            (5*1000)
#define DIAG_HIDP_DEBUG_TIMER_LEN                       (31*1000)

#define     DIAG_ERRORLOG_TIMER_NAME                    (0x00001001)
#define     DIAG_CLTINFO_TIMER_NAME                     (0x00002002)
#define     DIAG_DEBUG_TIMER_NAME                       (0x00003003)

#define     DIAG_ERRORLOG_TIMER_PARA                    (0x0000EFFE)
#define     DIAG_CLTINFO_TIMER_PARA                     (0x0000DFFD)
#define     DIAG_DEBUG_TIMER_PARA                       (0x0000CFFC)

/*****************************************************************************
  DIAG Cookie
*****************************************************************************/
enum
{
    DIAG_COOKIE_BASE        = 0xeb000000,
    DIAG_COOKIE_CREATE_DFR  = DIAG_COOKIE_BASE + 0x1,
    DIAG_COOKIE_MSGBBP_DFR  = DIAG_COOKIE_CREATE_DFR + 0x1,
    DIAG_COOKIE_MSGHL1C_DFR = DIAG_COOKIE_MSGBBP_DFR + 0x1,
    DIAG_COOKIE_BUTT        = 0xebffffff,
};

/* MSP_DIAG_STID_STRU:cmdid19b */

/* ======================================================================== */

enum DIAG_CONNECT_CMD_ENUM
{
    DIAG_CONNECT_CMD,       /* 连接命令 */
    DIAG_DISCONNECT_CMD,    /* 断开连接命令 */
    DIAG_CONNECT_CMD_BUTT
};
typedef VOS_UINT32 DIAG_CONNECT_CMD_ENUM_U32;

/*****************************************************************************
   5 STRUCT
*****************************************************************************/

/* 此结构体与OSA的MsgBlock对应，不能随意修改 */
#pragma pack(1)
typedef struct
{
    VOS_UINT32    ulSenderCpuId;
    VOS_UINT32    ulSenderPid;
    VOS_UINT32    ulReceiverCpuid;
    VOS_UINT32    ulReceiverPid;
    VOS_UINT32    ulLength;
    VOS_UINT32    ulMsgId;
    VOS_UINT8     aucValue[0];   /*lint !e43 */
}DIAG_OSA_MSG_STRU;
#pragma pack()


/* 只带结果的通用回复结构 */
typedef struct
{
    VOS_UINT32  ulAuid;
    VOS_UINT32  ulSn;
    VOS_UINT32  ulRet;
}DIAG_COMM_CNF_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32      ulMsgId;
    VOS_UINT32      ulLen;
    VOS_UINT8       pContext[0];     /*lint !e43 */
}DIAG_DATA_MSG_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32              ulMsgId;           /* 消息名 */
    VOS_UINT32              ulLen;             /* 数据长度 */
    VOS_UINT8               resv[12];       /* 数据其实地址*/
    VOS_UINT8               pContext[0];       /* 数据其实地址*/ /*lint !e43 */
}DIAG_PS_MSG_STRU;

/* ======================================================================== */
/* DIAG dump */

#define DIAG_DUMP_LEN 	    (0x2000)
#define DIAG_DUMP_MSG_LEN   (0x400)     /* 消息缓存1K/(4*4)=64条 */
#define DIAG_DUMP_DF_LEN    (0x800)     /* 码流缓存2K */

typedef struct
{
    VOS_CHAR *      pcDumpAddr;

    VOS_CHAR *      pcMsgAddr;          /* message起始地址 */
    VOS_UINT32      ulMsgLen;           /* message空间长度 */
    VOS_UINT32      ulMsgCur;           /* message空间当前指针 */

    VOS_CHAR *      pcDFAddr;           /* data flow起始地址 */
    VOS_UINT32      ulDFLen;            /* data flow空间长度 */
    VOS_UINT32      ulDFCur;            /* data flow空间当前指针 */
}DIAG_DUMP_INFO_STRU;


/*****************************************************************************
  9 OTHERS
*****************************************************************************/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of msp_common.h */

