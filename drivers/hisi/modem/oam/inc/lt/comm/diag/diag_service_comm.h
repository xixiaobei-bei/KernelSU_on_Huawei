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


#ifndef __DIAG_SERVICE_COMM_H__
#define __DIAG_SERVICE_COMM_H__

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
#include <vos.h>
#include <msp.h>
#include "diag_frame.h"
/*****************************************************************************
  2 macro
*****************************************************************************/
/* 单帧最大长度 */
#define DIAG_FRAME_MAX_LEN      (4*1024)
/* 单消息最大帧个数 */
#define DIAG_FRMAE_MAX_CNT      (16)
/* 总长度最大值 */
#define DIAG_FRAME_SUM_LEN      (DIAG_FRAME_MAX_LEN * DIAG_FRMAE_MAX_CNT)

#define DIAG_SRV_HISI_HEADER_MAGIC                  (0x48495349) /*HISI*/
#define DIAG_SRV_SET_MODEM_ID(pstFrame, modemid)    ((pstFrame)->stService.mdmid3b = (modemid))
#define DIAG_SRV_SET_TRANS_ID(pstFrame, transid)    ((pstFrame)->stService.ulMsgTransId = (transid))
#define DIAG_SRV_SET_MSG_LEN(pstFrame, msglen)      ((pstFrame)->ulMsgLen = (msglen))
#define DIAG_SRV_SET_COMMAND_ID(pstFrame, pri, mode, sec, cmdid)\
do{\
    (pstFrame)->stID.pri4b  = pri;\
    (pstFrame)->stID.mode4b = mode;\
    (pstFrame)->stID.sec5b  = sec;\
    (pstFrame)->stID.cmdid19b = cmdid;\
}while(0);
#define DIAG_SRV_SET_COMMAND_ID_EX(pstFrame, cmdid) ((pstFrame)->ulCmdId = (cmdid))



/*4字节对齐*/
#define ALIGN_DDR_WITH_4BYTE(len)       (((len) + 3)&(~3))
/*8字节对齐*/
#define ALIGN_DDR_WITH_8BYTE(len)       (((len) + 7)&(~7))

/*****************************************************************************
  3 Enum
*****************************************************************************/
typedef struct
{
    VOS_UINT32                  ulHisiMagic;   /*"HISI"*/
    VOS_UINT32                  ulDataLen;      /*数据长度*/
}DIAG_SRV_SOCP_HEADER_STRU;

typedef struct
{
    DIAG_SRV_SOCP_HEADER_STRU       socp_header;
    DIAG_FRAME_INFO_STRU            frame_header;
}DIAG_SRV_HEADER_STRU;

/* ==============消息应答上报接口参数====================================== */

typedef struct
{
    VOS_UINT32          ulHeaderSize;   /* 数据头的长度 */
    VOS_VOID            *pHeaderData;   /* 数据头 */

    VOS_UINT32          ulDataSize;     /* ucData的长度 */
    VOS_VOID            *pData;         /* 数据 */
}DIAG_MSG_REPORT_HEAD_STRU;

/* ======================================================================== */
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
#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif
