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
#ifndef __DIAG_TIME_STAMP_H__
#define __DIAG_TIME_STAMP_H__

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include <product_config.h>
#include <mdrv.h>
#include <msp.h>
#include <soc_socp_adapter.h>
#include <nv_stru_drv.h>
#include <nv_stru_lps.h>
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_api.h"
#include "diag_debug.h"

/******************************0x10015101 时间戳心跳结构体****************************************/
typedef struct
{
    VOS_UINT32 ulLowTimeStamp;
	VOS_UINT32 ulHighTimeStamp;    /* 高32bit时间戳*/
} DIAG_IND_HIGH_TS_STRU;

typedef struct
{
    DIAG_CMD_TRANS_IND_STRU stTransInfo;
    DIAG_IND_HIGH_TS_STRU   stHighTs;
}DIAG_TIME_STAMP_T;

#ifdef DIAG_SYSTEM_5G
VOS_VOID diag_PushHighTs(VOS_VOID);
VOS_VOID diag_StartHighTsTimer(VOS_VOID);
VOS_VOID diag_StopHighTsTimer(VOS_VOID);
VOS_UINT32 diag_HighTsCfgProc(VOS_UINT8* pstReq);
#else
static inline VOS_VOID diag_PushHighTs(VOS_VOID){}
static inline VOS_VOID diag_StopHighTsTimer(VOS_VOID){}
static inline VOS_UINT32 diag_HighTsCfgProc(VOS_UINT8* pstReq){return 0;}
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of msp_acore_common.h */

