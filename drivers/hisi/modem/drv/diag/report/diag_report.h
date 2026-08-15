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


#ifndef __DIAG_REPORT_H__
#define __DIAG_REPORT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <product_config.h>
#include <mdrv.h>
#include <osl_spinlock.h>
#include "diag_service.h"

#pragma pack(push)
#pragma pack(4)

/*****************************************************************************
  2 macro
*****************************************************************************/

#define DIAG_GET_MODEM_ID(id)               (id >> 24)
#define DIAG_GET_MODE_ID(id)                ((id & 0x000F0000)>>16)
#define DIAG_GET_PRINTF_LEVEL(id)           ((id & 0x0000F000)>>12)
#define DIAG_GET_GROUP_ID(id)               ((id & 0x00000F00)>>8)
#define DIAG_GET_MODULE_ID(id)              ( id & 0x00000FFF)

#define LTE_DIAG_PRINTF_PARAM_MAX_NUM       (6)

#define DIAG_DRV_PRINTLOG_MAX_BUFF_LEN      (300)

/*diag printV最大允许字节数,包括前面文件名和行号长度，-1是预留\0结束符*/
#define DIAG_PRINTF_MAX_LEN 	(1000-1)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/

/*****************************************************************************
  4 Enum
*****************************************************************************/

/*****************************************************************************
   5 STRUCT
*****************************************************************************/
typedef struct
{
    u32 ulPrintNum;
    u32 ulAirNum;
    u32 ulVoLTENum;
    u32 ulTraceNum;
    u32 ulUserNum;
    u32 ulEventNum;
    u32 ulTransNum;

    spinlock_t    ulPrintLock;
    spinlock_t    ulAirLock;
    spinlock_t    ulVoLTELock;
    spinlock_t    ulTraceLock;
    spinlock_t    ulUserLock;
    spinlock_t    ulEventLock;
    spinlock_t    ulTransLock;
} DIAG_LOG_PKT_NUM_ACC_STRU;

typedef struct
{
    unsigned int ulModule;                        /* 打印信息所在的模块ID */
    unsigned int ulLevel;                         /* 输出级别 */
    unsigned int ulNo;                            /* IND标号 */
    char   szText[DIAG_PRINTF_MAX_LEN+1];   /* 所有打印文本内容，可能包括文件和行号,以'\0'结尾 */
} DIAG_CMD_LOG_PRINT_RAW_TXT_IND_STRU;

/*****************************************************************************
  6 UNION
*****************************************************************************/

/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/
u32 diag_report_log(u32 ulModuleId, u32 ulPid, u32 ullevel, char *cFileName, u32 ulLineNum, char *pszFmt, va_list arg);
u32 diag_report_trans(DRV_DIAG_TRANS_IND_STRU *pstData);
u32 diag_report_event(DRV_DIAG_EVENT_IND_STRU *pstEvent);
u32 diag_report_air(DRV_DIAG_AIR_IND_STRU *pstAir);
u32 diag_report_trace(void *pMsg, u32 modemid);
u32 diag_report_msg_trans(DRV_DIAG_TRANS_IND_STRU *pstData, u32 ulcmdid);
u32 diag_report_cnf(DRV_DIAG_CNF_INFO_STRU *pstDiagInfo, void *pstData, u32 ulLen);
u32 diag_report_drv_log(u32 level, char* fmt, va_list arg);
void diag_report_init(void);
void diag_report_reset(void);
char * diag_GetFileNameFromPath(char* pcFileName);
u32 diag_report_reset_msg(DRV_DIAG_TRANS_IND_STRU *pstData);
/*****************************************************************************
  9 OTHERS
*****************************************************************************/

#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of diag_report.h */

