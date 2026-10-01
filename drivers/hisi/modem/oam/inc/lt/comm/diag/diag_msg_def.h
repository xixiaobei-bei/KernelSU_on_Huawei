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

#ifndef __DIAG_MESSAGE_DEF_H__
#define __DIAG_MESSAGE_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(4)

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"
#include "diag_frame.h"
/*****************************************************************************
  2 Massage Declare
*****************************************************************************/
/* msgid ----------AP->LRM */
#define     DIAG_MSG_MSP_CONN_REQ               0x00000001
#define     DIAG_MSG_MSP_DISCONN_REQ            0x00000002
#define     DIAG_MSG_MSP_A_TRANS_C_REQ          0x00000003
#define     DIAG_MSG_BSP_A_TRANS_C_REQ          0x00000004
#define     DIAG_MSG_BBP_A_TRANS_C_REQ          0x00000005
#define     DIAG_MSG_PHY_A_TRANS_C_REQ          0x00000006
#define     DIAG_MSG_MSP_A_DEBUG_C_REQ          0x00000009      /* 通知C核保存debug信息 */
#define     DIAG_MSG_BSP_NV_AUTH_REQ            0x0000000A      /* 通知A核鉴权信息 */
#define     DIAG_MSG_HIFI_A_TRANS_C_REQ         0x0000000B
#define     DIAG_MSG_BSP_CMD_LIST_REQ           0x0000000C

/* msgid ----------AP->NRM*/
#define     DIAG_MSG_MSP_A_TRANS_NRM_REQ        (DIAG_MSG_MSP_CONN_REQ)
#define     DIAG_MSG_BSP_A_TRANS_NRM_REQ        (DIAG_MSG_BSP_A_TRANS_C_REQ)
#define     DIAG_MSG_BBP_A_TRANS_NRM_REQ        (DIAG_MSG_BBP_A_TRANS_C_REQ)
#define     DIAG_MSG_HL1C_A_TRANS_NRM_REQ       (DIAG_MSG_PHY_A_TRANS_C_REQ)
#define     DIAG_MSG_MSP_A_DEBUG_NRM_REQ        (DIAG_MSG_MSP_A_DEBUG_C_REQ)
#define     DIAG_MSG_BSP_CMD_LIST_NRM_REQ       (DIAG_MSG_BSP_CMD_LIST_REQ)

/* msgid ----------NRM->HAC */
#define     DIAG_MSG_MSP_NRM_TRANS_HAC_REQ      (DIAG_MSG_MSP_A_TRANS_C_REQ)
#define     DIAG_MSG_BSP_NRM_TRANS_HAC_REQ      (DIAG_MSG_BSP_A_TRANS_C_REQ)

/* msgid ----------HAC->NRM */
#define     DIAG_MSG_MSP_HAC_CNF_NRM            (0x0000000D)

/*****************************************************************************
  3 Enum
*****************************************************************************/
/*****************************************************************************
  4 UNION
*****************************************************************************/
/*****************************************************************************
  5 struct
*****************************************************************************/
/* 核间透传通信结构体 */
typedef struct
{
     VOS_MSG_HEADER                     /*VOS头 */
     VOS_UINT32                         ulMsgId;
     DIAG_FRAME_INFO_STRU               stInfo;
}DIAG_MSG_TRANS_STRU;

typedef  DIAG_MSG_TRANS_STRU DIAG_MSG_A_TRANS_C_STRU;
typedef  DIAG_MSG_TRANS_STRU DIAG_MSG_NRM_TRANS_HAC_STRU;

typedef struct
{
    union
    {
        VOS_UINT32          ulID;           /* 结构化ID */
        MSP_DIAG_STID_STRU  stID;
    };
    VOS_UINT32 ulTransId;       /* transid */
    VOS_UINT32 ulDataLen;
    VOS_UINT8  pData[0];        /*lint !e43 */
}DIAG_HAC_CNF_NRM_STRU;

/* 核间透传通信结构体 */
typedef struct
{
     VOS_MSG_HEADER                     /*VOS头 */
     VOS_UINT32                         ulMsgId;
     DIAG_FRAME_INFO_STRU               stInfo;
}DIAG_BSP_MSG_A_TRANS_C_STRU;

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
