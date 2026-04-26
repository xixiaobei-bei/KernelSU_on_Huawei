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

#ifndef __BSP_OM_LOG_H__
#define __BSP_OM_LOG_H__

#include <product_config.h>
#include "mntn_interface.h"
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <bsp_modem_log.h>

#define BSP_DIAG_IND_DATA_MAX_LEN 4096
#define BSP_PRINT_BUF_LEN 192
#define BSP_PRINT_LEN_LIMIT 100
#define DIAG_DRV_HDS_PID 0x8003
/* log缓冲区满阈值，buffer空闲空间小于200字符，认为buffer已满 */
#define LOG_BUFFER_FULL_THRESHOLD 200
/* log task调度间隔，2s */
#define LOG_TASK_SCHED_TIME 200
/* log buffer满，输出提示 */
#define LOG_DROPPED_MESSAGE "log buffer full, data dropped\n"
/* ccore log设备名 */
#define CCORE_LOG_DEV_NAME "ccorelog"
/* nrccore log设备名 */
#define NRCCORE_LOG_DEV_NAME "nrccorelog"
/* mcore log设备名 */
#define MCORE_LOG_DEV_NAME "mcorelog"
/* l2hac log设备名 */
#define L2HAC_LOG_DEV_NAME "l2haclog"
/* 错误码定义 */
#define BSP_ERR_LOG_BASE (int)(0x80000000 | (BSP_DEF_ERR(10, 0)))
#define BSP_ERR_LOG_INVALID_MODULE (BSP_ERR_LOG_BASE + 0x1)
#define BSP_ERR_LOG_INVALID_LEVEL (BSP_ERR_LOG_BASE + 0x2)
#define BSP_ERR_LOG_INVALID_PARAM (BSP_ERR_LOG_BASE + 0x3)
#define BSP_ERR_LOG_NO_BUF (BSP_ERR_LOG_BASE + 0x4)
#define BSP_ERR_LOG_SOCP_ERR (BSP_ERR_LOG_BASE + 0x5)

typedef enum {
    BSP_HIDS_LEVEL_ERROR = 1, /* 0x1:just for compatibility with previous version */
    BSP_HIDS_LEVEL_WARNING,   /* 0x2:warning conditions                           */
    BSP_HIDS_LEVEL_NOTICE,    /* 0x3:normal but significant condition             */
    BSP_HIDS_LEVEL_INFO,      /* 0x4:informational                                */
    BSP_HIDS_LEVEL_MAX
} bsp_hids_level_e;

typedef struct {
    void *addr;
    u16 is_edle;
    u16 is_valible;
} bsp_log_node_ctrl_s;

typedef struct {
    u32 print_level;
} bsp_log_swt_cfg_s;

struct log_mem_stru {
    struct log_mem log_info;
    u32 logged_chars; /* number of chars in log buffer */
    u32 w_mark;       /* water mark, if logged chars exceed w_mark, ccore send IPC to acore */
    u32 reserved;     /* reserved for 32-byte align */
    u8 *log_buf;      /* data area */
};

struct logger_info_stru {
    struct miscdevice misc; /* misc device */
    wait_queue_head_t wq;   /* wait queue */
    u32 in_use;             /* whether device is in use */
    u32 sleep_flag;         /* whether app is sleep */
    spinlock_t lock;
    struct list_head logs;
    struct log_mem_stru *log_mem;
};

struct new_om_log {
    struct log_usr_info ccore_log_info;
    struct log_usr_info nrccore_log_info;
    struct log_usr_info l2hac_log_info;
#ifndef BSP_CONFIG_PHONE_TYPE
    struct log_usr_info mcore_log_info;
#endif
};

#endif
