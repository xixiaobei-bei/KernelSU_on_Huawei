/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef __BSP_HDS_IND_H__
#define __BSP_HDS_IND_H__
#endif

#include <product_config.h>
#include "osl_types.h"
#include "osl_sem.h"
#include "osl_spinlock.h"
#include "osl_thread.h"
#include "osl_common.h"
#include "acore_nv_stru_drv.h"
#include <linux/dma-mapping.h>
#include "bsp_diag.h"
#include <bsp_socp.h>
#include "soc_socp_adapter.h"

extern unsigned long long g_log_dma_test_mask;
#define HDS_OK (0)
#define HDS_ERR (-1)
#define HDS_NULL (void *)0
#define LOG_SRC_BUF_LEN 0x10000

#define PRINTLOG_CHN_INIT ((s32)1)   /* 打印数据通道已经分配 */
#define PRINTLOG_CHN_UNINIT ((s32)0) /* 打印数据通道没有分配 */
#define TRANSLOG_CHN_INIT ((s32)1)   /* 结构体数据通道已经分配 */
#define TRANSLOG_CHN_UNINIT ((s32)0) /* 结构体数据通道没有分配 */

#define PRINTLOG_MAX_FILENAME_LEN 15
#define PRINTLOG_MAX_BUFF_LEN 256
#define PRINTLOG_MAX_HIDS_BUFF_LEN (PRINTLOG_MAX_BUFF_LEN + PRINTLOG_MAX_FILENAME_LEN)
#define TRANSLOG_MAX_HIDS_BUFF_LEN ((4 * 1024) - (DIAG_SOCP_HEAD_SIZE + DIAG_FRAME_HEAD_SIZE + DIAG_TRANS_HEAD_SIZE))

#ifdef DIAG_SYSTEM_5G
#define SOCP_CODER_SRC_LOG_IND SOCP_CODER_SRC_ACPU_AP_BSP_IND
#else
#define SOCP_CODER_SRC_LOG_IND SOCP_CODER_SRC_BSP_ACORE
#endif

#define LOG_PHYS_TO_VIRT(phy) (void *)(phys_to_virt((unsigned long)(phy)))

typedef enum _bsp_log_level_e {
    BSP_LOG_LEVEL_DEBUG = 0, /* 0x0:debug-level                                  */
    BSP_LOG_LEVEL_INFO,      /* 0x1:informational                                */
    BSP_LOG_LEVEL_NOTICE,    /* 0x2:normal but significant condition             */
    BSP_LOG_LEVEL_WARNING,   /* 0x3:warning conditions                           */
    BSP_LOG_LEVEL_ERROR,     /* 0x4:error conditions                             */
    BSP_LOG_LEVEL_CRIT,      /* 0x5:critical conditions                          */
    BSP_LOG_LEVEL_ALERT,     /* 0x6:action must be taken immediately             */
    BSP_LOG_LEVEL_FATAL,     /* 0x7:just for compatibility with previous version */
    BSP_LOG_LEVEL_MAX        /* Max */
} bsp_log_level_e;

typedef enum _bsp_hids_level_e {
    BSP_HIDS_LEVEL_ERROR = 1, /* 0x1:just for compatibility with previous version */
    BSP_HIDS_LEVEL_WARNING,   /* 0x2:warning conditions                           */
    BSP_HIDS_LEVEL_NOTICE,    /* 0x3:normal but significant condition             */
    BSP_HIDS_LEVEL_INFO,      /* 0x4:informational                                */
    BSP_HIDS_LEVEL_MAX
} bsp_hids_level_e;

typedef int (*print_report_hook)(u32 module_id, u32 level, u32 sel, const char *print_buff);

struct hds_trans_ind {
    u32 ulModule;
    u32 ulPid;
    u32 ulMsgId;
    u32 ulReserve;
    u32 ulLength;
    void *pData;
};

struct printlog_ctrl_info {
    u32 u32PrintSwitchOnOff;
    u32 u32PrintInvalidParameter;
    u32 u32PrintLevel;
    u32 u32PrintLength;
    u32 u32PrintReport;
    u32 u32PrintSendData;
};

struct translog_ctrl_info {
    u32 u32TransSwitchOnOff;
    u32 u32TransInvalidParameter;
    u32 u32TransLevel;
    u32 u32TransLength;
    u32 u32TransReport;
    u32 u32TransSendData;
};

extern u32 g_printlog_conn;
extern u32 g_printlog_enable;
extern u32 g_translog_conn;
extern u32 g_printlog_level;

int bsp_hds_translog_conn(void);
int bsp_hds_translog_disconn(void);
#if (FEATURE_HDS_TRANSLOG == FEATURE_ON)
s32 bsp_transreport(struct hds_trans_ind *pstData);
#else
#define HDS_TRANS_CFG_OFF (0x00abcd00 + 0x6)
static inline s32 bsp_transreport(struct hds_trans_ind *pstData)
{
    return HDS_TRANS_CFG_OFF;
}
#endif
