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

#ifndef PM_OM_NRCCPU_H
#define PM_OM_NRCCPU_H

#include <product_config.h>
#include <bsp_pm_om.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NRCCPU_CORE_NUM
#define NR_CPU_NUM NRCCPU_CORE_NUM
#else
#define NR_CPU_NUM 1
#endif

#define PRIMARY_CPU_ID 0
#define NR_WAKEUP_IRQ_NUM 64
#define NR_WAKEUP_IRQ_CFG_REG_NUM 2
#define NRPM_DUMP_SLICE_SIZE 15
#define NRPM_DUMP_BEGIN_SLICE_INDEX 1
#define NRPM_DUMP_END_SLICE_INDEX (NRPM_DUMP_SLICE_SIZE - 2)

struct nr_wakeup_irq_s {
    char *name;
    u32 irq;
    u32 wakeup_cnt;
    u32 wakeup_slice;
};

struct pm_nr_om_debug {
    struct nr_wakeup_irq_s nr_wakeup_irq[NR_WAKEUP_IRQ_NUM];
    char *reg[NR_WAKEUP_IRQ_CFG_REG_NUM];
    char *cdrx_dump_addr;
    u32 max_logbuf_size;
};

typedef struct {
    u32 wakeup_cnt[NR_CPU_NUM];
    u32 sleep_cnt[NR_CPU_NUM];
    u32 wakeup_slice[NR_CPU_NUM][NRPM_DUMP_SLICE_SIZE];
    u32 sleep_slice[NR_CPU_NUM][NRPM_DUMP_SLICE_SIZE];
} nr_sr_dump_s;

void nr_wakeup_irq_debug_init(void);
void nrpm_suspend_prepare(void);
void print_nrccpu_lowpower_info(void);

#ifdef __cplusplus
}
#endif

#endif /* __PM_OM_DEBUG_H__ */
