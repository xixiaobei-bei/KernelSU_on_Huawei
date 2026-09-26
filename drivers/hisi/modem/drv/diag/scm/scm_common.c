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
 * 1 include files
 */
#include "scm_common.h"
#include <linux/of_platform.h>
#include <mdrv.h>
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include <securec.h>
#include <bsp_shared_ddr.h>
#include "diag_system_debug.h"


struct platform_device *g_diag_pdev;

void scm_set_platform_dev(struct platform_device *dev)
{
    g_diag_pdev = dev;
    return;
}

/*
 * Function: scm_uncache_mem_alloc
 * Description: allocate uncached memory.
 */
void *scm_uncache_mem_alloc(u32 size, unsigned long *phy_addr)
{
    void *virt_addr = NULL;
    dma_addr_t addr = 0;

    if (size == 0) {
        return NULL;
    }

    if (phy_addr == NULL) {
        return NULL;
    }

    /* keep lint happy */
    *phy_addr = 0;
    virt_addr = 0;

#ifdef CONFIG_ARM64

    virt_addr = dma_alloc_coherent(&g_diag_pdev->dev, size, &addr, GFP_KERNEL);
#else
    virt_addr = dma_alloc_coherent(NULL, size, &addr, GFP_KERNEL);
#endif

    *phy_addr = (uintptr_t)addr;

    return virt_addr;
}

unsigned long *scm_uncache_mem_phy_to_virt(const u8 *cur_phy_addr, const u8 *phy_start, const u8 *virt_start,
    u32 buf_len)
{
    if ((cur_phy_addr < phy_start) || (cur_phy_addr >= (phy_start + buf_len))) {
        diag_error("Phy to Virt error\n");

        return NULL;
    }

    return (unsigned long *)((cur_phy_addr - phy_start) + virt_start);
}

u32 diag_shared_mem_write(u32 type, u32 len, const char *data)
{
    u8 *ptr = NULL;
    errno_t ret = 0;

    if ((data == NULL) || (len == 0)) {
        diag_error("diag_shm_ops: para error, len=0x%x.\n", len);
        return ERR_MSP_FAILURE;
    }

    switch (type) {
#ifdef DIAG_SYSTEM_5G
        case POWER_ON_LOG_BUFF:
            ptr = (u8 *)(uintptr_t)(SHM_BASE_ADDR + SHM_OFFSET_DIAG_POWER_ON_LOG);
            ret = memcpy_s(&(((shm_power_on_log_info_s *)ptr)->buf_usable),
                sizeof(((shm_power_on_log_info_s *)ptr)->buf_usable), data, len);
            break;
#else
        case POWER_ON_LOG_A:
            if ((data == NULL) || (len == 0)) {
                diag_error("para error, len=0x%x\n", len);
                return ERR_MSP_FAILURE;
            }
            ptr = (u8 *)(uintptr_t)(((unsigned long)SHM_BASE_ADDR) + SHM_OFFSET_DIAG_POWER_ON_LOG);
            ret = memcpy_s(&(((shm_power_on_log_info_s *)ptr)->power_on_log_a),
                sizeof(((shm_power_on_log_info_s *)ptr)->power_on_log_a), data, len);
            break;

        case DS_DATA_BUFFER_STATE:
            if ((data == NULL) || (len == 0)) {
                diag_error("para error, len=0x%x\n", len);
                return ERR_MSP_FAILURE;
            }
            ptr = (u8 *)(uintptr_t)(((unsigned long)SHM_BASE_ADDR) + SHM_OFFSET_DIAG_POWER_ON_LOG);
            ret = memcpy_s(&(((shm_power_on_log_info_s *)ptr)->socp_buffer),
                sizeof(((shm_power_on_log_info_s *)ptr)->socp_buffer), data, len);
            break;

#endif
        default:
            return ERR_MSP_INVALID_ID;
    }
    if (ret != EOK) {
        diag_error("memory copy failed 0x%x\n", ret);
        return ret;
    }
    return ERR_MSP_SUCCESS;
}

u32 diag_shared_mem_read(u32 type)
{
    u8 *ptr = NULL;
    u32 val;

    switch (type) {
#ifdef DIAG_SYSTEM_5G
        case POWER_ON_LOG_BUFF:
            ptr = (u8 *)(uintptr_t)(SHM_BASE_ADDR + SHM_OFFSET_DIAG_POWER_ON_LOG);
            val = ((shm_power_on_log_info_s *)ptr)->buf_usable;
            return val;
#else
        case POWER_ON_LOG_A:
            ptr = (u8 *)(uintptr_t)(SHM_BASE_ADDR + SHM_OFFSET_DIAG_POWER_ON_LOG);
            val = ((shm_power_on_log_info_s *)ptr)->power_on_log_a;
            return val;

        case DS_DATA_BUFFER_STATE:
            ptr = (u8 *)(uintptr_t)(((unsigned long)SHM_BASE_ADDR) + SHM_OFFSET_DIAG_POWER_ON_LOG);
            val = (u32)(((shm_power_on_log_info_s *)ptr)->socp_buffer); //lint !e571
            return val;
#endif
        default:
            diag_error("type is invalid,type:0x%x\n", type);
            return ERR_MSP_FAILURE;
    }
}

