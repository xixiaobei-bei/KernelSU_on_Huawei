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

#ifndef __PRINT_RECORD_H__
#define __PRINT_RECORD_H__

#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <securec.h>
#include <product_config.h>
#include <osl_types.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_bio.h>
#include <osl_spinlock.h>
#include <bsp_rfile.h>
#include <bsp_print.h>
#include <bsp_modem_log.h>
#include <bsp_mloader.h>
#include <mntn_interface.h>
#include <bsp_ddr.h>
#include "bsp_om_log.h"

#define PRINT_EARLY_LOG_PATH (MODEM_LOG_ROOT "/modem_boot_log.txt")
#define PRINT_EARLY_LOG_PATH_1 (MODEM_LOG_ROOT "/modem_boot_log_1.txt")
#define PRINT_EARLY_LOG_MAX_LEN MNTN_AREA_LOG_SIZE
#define PRINT_EARLY_LOG_TIME_LEN 64
#define PRINT_SAVE_MAX_LEN (2 * PRINT_EARLY_LOG_MAX_LEN)
#define PRINT_TIMESLICE_SIZE 32
#define WAIT_FOR_MDM_OK_SECOND 30
#ifdef DDR_EARLY_LOG_ADDR
#define PRINT_EARLY_LOG_HEAD_NUM 8
#else
#define PRINT_EARLY_LOG_HEAD_NUM 3
#endif
#define PRINT_SLICE_YEAR 1900
#define PRINT_DIR_CHMOD 0660
#define PRINT_SLEEP_1000S 1000
#define PRINT_SAVE_TASK_PRIO 96
struct print_save_buf_s {
    u32 head_len;
    char time_slice[PRINT_TIMESLICE_SIZE];
};

typedef struct print_save_ctrl {
    u32 init_ok;
    u32 sleep_s;
    u32 data_len;
    uintptr_t print_task_id;
    struct semaphore sem_print_task;
    mloader_load_notifier_s callback;
    u8 *data_addr;
} print_save_ctrl_s;
#endif
