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


#ifndef __DIAG_MSGBBP_H__
#define __DIAG_MSGBBP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "vos.h"
#include "msp_diag_comm.h"
#include "diag_common.h"
#include "msp_errno.h"
#include "diag_bbp_ds.h"
#include "diag_msgbbp_comm.h"

/*****************************************************************************
  2 macro
*****************************************************************************/
#define ACCESS_REGISTER_FN_SUB_ID_DDR_MODEM_SEC (0x55bbcce9UL)
#define ACCESS_REGISTER_FN_MAIN_ID              (0xc500aa01UL)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/


/*****************************************************************************
   5 STRUCT
*****************************************************************************/
#define DIAG_BBP_XDATA_DS_DISABLE    (0)
#define DIAG_BBP_XDATA_DS_ENABLE     (1)

/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/
VOS_VOID   diag_DsSendChanInfoToLrm(DIAG_FRAME_INFO_STRU *pData);
#ifdef DIAG_SYSTEM_5G
VOS_VOID   diag_DsSendChanInfoToNrm(DIAG_FRAME_INFO_STRU *pData);
#endif
VOS_UINT32 diag_DrxSampleGetChnSizeProc(DIAG_FRAME_INFO_STRU *pData);
VOS_UINT32 diag_BbpMsgProc(DIAG_FRAME_INFO_STRU *pData);
VOS_VOID   diag_BbpMsgInit(VOS_VOID);
VOS_VOID   diag_BbpShowDebugInfo(VOS_VOID);

/*****************************************************************************
  9 OTHERS
*****************************************************************************/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of  */



