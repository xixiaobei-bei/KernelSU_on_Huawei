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
/*
 * Device registers:
 * 0x000 - 0x2FC: Trace     registers
 * 0x300 - 0x314: Management    registers
 * 0x318 - 0xEFC: Trace     registers
 *
 * Coresight registers
 * 0xF00 - 0xF9C: Management    registers
 * 0xFA0 - 0xFA4: Management    registers in PFTv1.0
 *        Trace     registers in PFTv1.1
 * 0xFA8 - 0xFFC: Management    registers
 */

#ifndef __MODEM_ETB_H__
#define __MODEM_ETB_H__
/*lint -save -e537*/
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "bsp_dump.h"
#include "bsp_ipc.h"
#include "bsp_memmap.h"
#include "bsp_wdt.h"
#include "bsp_icc.h"
#include "bsp_onoff.h"
#include "bsp_nvim.h"
#include "bsp_softtimer.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_dump_mem.h"
#include "bsp_coresight.h"
#include "bsp_reset.h"
#include "bsp_sysctrl.h"
#include "drv_nv_def.h"
#include "of.h"
#include "bsp_wdt.h"
/* Trace registers (0x000-0x2FC) */
#define ETMCR               (0x000)
#define ETMSR               (0x010)
#define ETMTEEVR            (0x020)
#define CORESIGHT_LAR       (0xFB0)


#define CORESIGHT_UNLOCK    (0xC5ACCE55)
#define BIT(nr)                      (1UL << (nr))

#define BVAL(val, n)        (((val) & BIT(n)) >> n)


#define cs_etm_writel(addr, val, off)   (*(unsigned long*)((uintptr_t)addr + off) = (val))
#define cs_etm_readl(addr, off)         (*(unsigned long*)((uintptr_t)addr + off))

#define CS_ETM_LOCK(addr)                       \
do {                                    \
    cs_etm_writel(addr, 0x0, CORESIGHT_LAR);            \
} while (0)
#define CS_ETM_UNLOCK(addr)                     \
do {                                    \
    cs_etm_writel(addr, CORESIGHT_UNLOCK, CORESIGHT_LAR);   \
} while (0)

typedef struct modem_sys_cfg
{
    void*           mdm_ctrl_sys_virt_addr;
    unsigned long   mdm_ctrl_sys_phy_addr;
    unsigned long   mdm_ctrl_sys_mem_size;
    struct
    {
        u32 offset;
        u32 mcpu_dbg_clk_status;
        u32 mcpu_pd_clk_status;
    }clk;
    struct
    {
        u32 offset;
        u32 mdma9_pd_srst_status;
        u32 moda9_cpu_srst_status;
    }rst;
    struct
    {
        u32 offset;
        u32 cpu_mtcmos_strl_status;
    }mtcmos;
}modem_sysctrl_cfg_t;

typedef struct coresight_mem_ctrl_ss
{
    void*          cp_cs_virt_addr;
    unsigned long   cp_cs_phy_addr;
    unsigned long  cp_cs_mem_size;
}coresight_mem_ctrl_t;

typedef struct perctrl2
{
    unsigned long phy_addr;
    void* virt_addr;
    u32 offset;
    u32 pclkdbg_clkoff_sys;           /*bit 1*/
    u32 atclkoff_sys;                 /*bit 7*/
    u32 pclkdbg_to_modem_clk_off_sys; /*bit 17*/
    u32 atclk_to_modem_clkoff_sys;    /*bit 16*/
    u32 modem_cssys_rst_req;          /*bit 18*/
    
}PERCTRL2_REG;


s32 dump_coresight_memmap(void);
void dump_cs_etb(WDT_CORE_ID core_id);

#endif
