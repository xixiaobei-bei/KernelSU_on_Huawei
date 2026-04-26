/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
#ifndef __BSP_FIQ_H
#define __BSP_FIQ_H

#include <product_config.h>
#include <drv_comm.h>
#include <bsp_shared_ddr.h>
#include <bsp_slice.h>
#include <osl_bio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CCPU_CORE_NUM
#define SHM_FIQ_BASE_ADDR ((uintptr_t)SHM_OFFSET_CCORE_FIQ + (uintptr_t)SHM_BASE_ADDR)
#define SHM_FIQ_BARRIER (SHM_FIQ_BASE_ADDR)
#define SHM_FIQ_STATUS_ADDR (SHM_FIQ_BARRIER + (uintptr_t)0x4)
#define SHM_FIQ_CLEAR_ADDR (SHM_FIQ_STATUS_ADDR + (uintptr_t)(0x4 * CCPU_CORE_NUM))
#define SHM_FIQ_CP_TIMER_ADDR (SHM_FIQ_CLEAR_ADDR + (uintptr_t)(0x4 * CCPU_CORE_NUM))
#define SHM_FIQ_TOTAL_SEND_CNT (SHM_FIQ_CP_TIMER_ADDR + (uintptr_t)(0x4 * CCPU_CORE_NUM))
#define SHM_FIQ_TOTAL_RECIVE_CNT (SHM_FIQ_TOTAL_SEND_CNT + (uintptr_t)(0x4 * CCPU_CORE_NUM))
#define SHM_FIQ_INFO_ADDR (SHM_FIQ_TOTAL_RECIVE_CNT + (uintptr_t)(0x4 * CCPU_CORE_NUM))
#else
#define SHM_FIQ_BASE_ADDR ((uintptr_t)SHM_OFFSET_CCORE_FIQ + (uintptr_t)SHM_BASE_ADDR)
#define SHM_FIQ_BARRIER (SHM_FIQ_BASE_ADDR)
#define SHM_FIQ_STATUS_ADDR (SHM_FIQ_BARRIER + (uintptr_t)0x4)
#define SHM_FIQ_CLEAR_ADDR (SHM_FIQ_STATUS_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_CP_TIMER_ADDR (SHM_FIQ_CLEAR_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_TOTAL_SEND_CNT (SHM_FIQ_CP_TIMER_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_TOTAL_RECIVE_CNT (SHM_FIQ_TOTAL_SEND_CNT + (uintptr_t)0x4)
#define SHM_FIQ_INFO_ADDR (SHM_FIQ_TOTAL_RECIVE_CNT + (uintptr_t)0x4)
#endif

#ifdef NRCCPU_CORE_NUM
#define SHM_FIQ_NR_BASE_ADDR ((uintptr_t)SHM_OFFSET_NRCCORE_FIQ + (uintptr_t)SHM_BASE_ADDR)
#define SHM_FIQ_NR_BARRIER (SHM_FIQ_NR_BASE_ADDR)
#define SHM_FIQ_NR_STATUS_ADDR (SHM_FIQ_NR_BARRIER + (uintptr_t)0x4)
#define SHM_FIQ_NR_CLEAR_ADDR (SHM_FIQ_NR_STATUS_ADDR + (uintptr_t)(0x4 * NRCCPU_CORE_NUM))
#define SHM_FIQ_NR_TIMER_ADDR (SHM_FIQ_NR_CLEAR_ADDR + (uintptr_t)(0x4 * NRCCPU_CORE_NUM))
#define SHM_FIQ_NR_TOTAL_SEND_CNT (SHM_FIQ_NR_TIMER_ADDR + (uintptr_t)(0x4 * NRCCPU_CORE_NUM))
#define SHM_FIQ_NR_TOTAL_RECIVE_CNT (SHM_FIQ_NR_TOTAL_SEND_CNT + (uintptr_t)(0x4 * NRCCPU_CORE_NUM))
#define SHM_FIQ_NR_INFO_ADDR (SHM_FIQ_NR_TOTAL_RECIVE_CNT + (uintptr_t)(0x4 * NRCCPU_CORE_NUM))
#else
#define SHM_FIQ_NR_BASE_ADDR ((uintptr_t)SHM_OFFSET_NRCCORE_FIQ + (uintptr_t)SHM_BASE_ADDR)
#define SHM_FIQ_NR_BARRIER (SHM_FIQ_NR_BASE_ADDR)
#define SHM_FIQ_NR_STATUS_ADDR (SHM_FIQ_NR_BARRIER + (uintptr_t)0x4)
#define SHM_FIQ_NR_CLEAR_ADDR (SHM_FIQ_NR_STATUS_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_NR_TIMER_ADDR (SHM_FIQ_NR_CLEAR_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_NR_TOTAL_SEND_CNT (SHM_FIQ_NR_TIMER_ADDR + (uintptr_t)0x4)
#define SHM_FIQ_NR_TOTAL_RECIVE_CNT (SHM_FIQ_NR_TOTAL_SEND_CNT + (uintptr_t)0x4)
#define SHM_FIQ_NR_INFO_ADDR (SHM_FIQ_NR_TOTAL_RECIVE_CNT + (uintptr_t)0x4)
#endif

#define FIQ_TRIGGER_TAG (0xFFFFFFF0)

/* 优先级高的先处理，优先级数字越小越高 */
typedef enum __fiq_num {
    FIQ_RESET = 0,
    FIQ_DUMP,
    FIQ_SEC_DUMP,
    FIQ_TEST,
    FIQ_MAX
} fiq_num;

enum fiq_handler_return_value {
    FIQ_RETURN_RUNNING = 0,             /* FIQ钩子退出后，系统继续运行 */
    FIQ_ENTER_LOOP_1_NO_RETRUN = 1,     /* FIQ钩子退出后，系统进行死循环，不响应FIQ */
    FIQ_ENTER_WFE_NO_RETRUN = 2,        /* FIQ钩子退出后，系统进入WFE循环，响应FIQ */
    FIQ_ENTER_WFE_DISABLE_FIQ_NO_RETRUN /* FIQ钩子退出后，系统进入WFE循环，不再响应FIQ */
};

typedef int (*FIQ_PROC_FUNC)(void *);

int bsp_send_cp_fiq(fiq_num fiq_num_t);
#ifdef NRCCPU_CORE_NUM
int bsp_send_nr_fiq(fiq_num fiq_num_t);
#endif

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */
#endif
