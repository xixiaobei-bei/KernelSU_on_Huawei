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

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <securec.h>
#include <bsp_print.h>
#include <linux/version.h>
#include <asm/dma-mapping.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/of_device.h>
#include <linux/platform_device.h>
#endif

#undef THIS_MODU
#define THIS_MODU mod_malloc_m

#define  MALLOC_PRINT(fmt, ...) \
    (bsp_err("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

struct vtop {
    void *virt;
    void *phy;
    unsigned int size;
    struct list_head next;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))

struct platform_device *g_modem_hwadp_pdev;

#define HWADP_DMA_BIT_MASK 64
static int hwadp_driver_probe(struct platform_device *pdev)
{
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(HWADP_DMA_BIT_MASK));

    g_modem_hwadp_pdev = pdev;

    return 0;
}

static int hwadp_driver_remove(struct platform_device *pdev)
{
    return 0;
}

static struct platform_device g_hwadp_device = {
    .name    = "malloc-balong",
    .id    = -1,
    .num_resources   = 0,
};

static struct platform_driver g_hwadp_driver = {
    .probe  = hwadp_driver_probe,
    .remove = hwadp_driver_remove,
    .driver = {
        .name = "malloc-balong",
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};
#else
static struct device g_dev;
#endif


static LIST_HEAD(g_map_list);

void* mdrv_cached_mem_malloc(unsigned int size)
{
    return kmalloc(size, GFP_KERNEL);
}
EXPORT_SYMBOL_GPL(mdrv_cached_mem_malloc);

void mdrv_cached_mem_free(const void* addr)
{
    kfree(addr);
}
EXPORT_SYMBOL_GPL(mdrv_cached_mem_free);

void* mdrv_uncached_mem_malloc(unsigned int size)
{
    void *virt_addr = NULL;
    dma_addr_t phy_addr = 0;
    struct vtop *mem_map = vmalloc(sizeof(struct vtop));
    if (mem_map == NULL) {
        return NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    virt_addr = dma_alloc_coherent(&g_modem_hwadp_pdev->dev, (size_t)size, &phy_addr, GFP_DMA);
#else
    virt_addr = dma_alloc_coherent(&g_dev, (size_t)size, &phy_addr, GFP_DMA);
#endif
    if (virt_addr == NULL) {
        vfree(mem_map);
        return NULL;
    }

    mem_map->virt = virt_addr;
    mem_map->phy = (void *)(uintptr_t)phy_addr;
    mem_map->size = size;
    list_add_tail(&(mem_map->next), &g_map_list);

    MALLOC_PRINT("mdrv_uncached_mem_malloc=> size(0x%08x)\n", mem_map->size);
    return virt_addr;
}
EXPORT_SYMBOL_GPL(mdrv_uncached_mem_malloc);

int mdrv_uncached_mem_free(const void* addr)
{
    struct vtop *mem_map = NULL;
    list_for_each_entry(mem_map, &g_map_list, next) {
        if (mem_map->virt == addr) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
            dma_free_coherent(&g_modem_hwadp_pdev->dev, (size_t)mem_map->size, mem_map->virt,
                              (dma_addr_t)(uintptr_t)mem_map->phy);
#else
            dma_free_coherent(&g_dev, (size_t)mem_map->size, mem_map->virt,
                              (dma_addr_t)(uintptr_t)mem_map->phy);
#endif
            list_del(&(mem_map->next));
            vfree(mem_map);
            return 0;
        }
    }
    return -1;
}
EXPORT_SYMBOL_GPL(mdrv_uncached_mem_free);

void* mdrv_malloc_virt_to_phy(const void *addr)
{
    struct vtop *mem_map = NULL;
    list_for_each_entry(mem_map, &g_map_list, next) {
        if (((uintptr_t)mem_map->virt <= (uintptr_t)addr)
            && (((uintptr_t)mem_map->virt + mem_map->size) > (uintptr_t)addr)) {
            return mem_map->phy + (addr - mem_map->virt);
        }
    }
    return NULL;
}
EXPORT_SYMBOL_GPL(mdrv_malloc_virt_to_phy);


int bsp_malloc_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    int ret = platform_device_register(&g_hwadp_device);
    if (ret != 0) {
        MALLOC_PRINT("bsp_malloc_init platform_device_register failed\n");
        return 1;
    }

    ret = platform_driver_register(&g_hwadp_driver);
    if (ret != 0) {
        MALLOC_PRINT("bsp_malloc_init failed\n");
        return 1;
    }
#else
    (void)memset_s(&g_dev, sizeof(g_dev), 0, sizeof(g_dev));
    of_dma_configure(&g_dev, g_dev.of_node);
#endif
    MALLOC_PRINT("bsp_malloc_init ok\n");
    return 0;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    subsys_initcall(bsp_malloc_init); /*lint !e528*/
#else
    core_initcall(bsp_malloc_init); /*lint !e528*/
#endif
#endif
