/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <bsp_shared_ddr.h>
#include <mdrv_memory.h>
#include "memory_driver.h"


#undef THIS_MODU
#define THIS_MODU mod_mem

struct share_mem_ctrl {
    const char *name;
    unsigned long base_addr;
    unsigned int size;
};


struct share_mem_ctrl g_share_mem_mgr_ctrl[] = {
    {"nsroshm_nv", SHM_OFFSET_NV, SHM_SIZE_NV},
    {"nsroshm_version", SHM_OFFSET_VERSION, SHM_SIZE_VERSION},
#ifdef SHM_SIZE_NV_UNSAFE
    {"usshm_nv", SHM_OFFSET_NV_UNSAFE, SHM_SIZE_NV_UNSAFE},
#endif
#ifdef SHM_SIZE_PRODUCT_MEM
    {"usshm_product", SHM_OFFSET_PRODUCT_MEM, SHM_SIZE_PRODUCT_MEM},
#endif
    {"nsroshm_adc", SHM_OFFSET_TEMPERATURE, SHM_SIZE_TEMPERATURE},
#ifdef SHM_SIZE_CCORE_RESET
    {"usshm_reset", SHM_OFFSET_CCORE_RESET, SHM_SIZE_CCORE_RESET},
#endif
    
};

void *mdrv_mem_share_get(const char *name, phy_addr_t *addr, u32 *size, u32 flag)
{
    u32 i;

    if (name == NULL) {
        mem_err("name error!\n");
        return NULL;
    }
    if (size == NULL || addr == NULL) {
        mem_err("size or addr error!\n");
        return NULL;
    }
    for (i = 0; i < sizeof(g_share_mem_mgr_ctrl) / sizeof(g_share_mem_mgr_ctrl[0]); i++) {
        if (strcmp(g_share_mem_mgr_ctrl[i].name, name) == 0) { /*lint !e421 */
            *size = g_share_mem_mgr_ctrl[i].size;
            *addr = SHM_BASE_ADDR + g_share_mem_mgr_ctrl[i].base_addr;
            return (void *)(uintptr_t)(SHM_BASE_ADDR + g_share_mem_mgr_ctrl[i].base_addr);
        }
    }
    mem_err("%s mem_dtsinfo get failed\n", name);
    return NULL;
}


