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

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/io.h>
#include <product_config.h>
#include "securec.h"
#include <osl_types.h>

#define AXIMEM_USERNAEM_MAX_LEN 64
#define AXI_CRG_BASE 0xFFF05000
#define AXI_CRG_RST_EN 0x90
#define AXI_CRG_RST_DIS 0x94
#define AXI_CRG_RST_MASK 0x2000000 // bit 25
#define AXI_GT_CLK 0x470
#define AXI_GT_CLK_MASK 0x400C0 // bit 6 7 18

struct aximem_node_info {
    struct list_head list;
    char name[AXIMEM_USERNAEM_MAX_LEN];
    unsigned int len;
    unsigned int user;
    unsigned int type; /* cache, noncache */
    unsigned long long phy;
    unsigned long long virt;
};

struct aximem_info {
    struct list_head head;
    unsigned int total;
    unsigned long long phy;
    unsigned long long virt;
    unsigned int curr;
    unsigned int left;
    spinlock_t lock;
};

struct aximem_info g_aximem_cache_info;
struct aximem_info g_aximem_noncache_info;


int aximem_init(void)
{
#ifdef CONFIG_AXIMEM_CLK_RST
    void *reg_addr = NULL;
    reg_addr = ioremap(AXI_CRG_BASE, 0x1000U);
    /* aximem clk en */
    writel(AXI_GT_CLK_MASK, (void *)(reg_addr + AXI_GT_CLK));
    /* aximem rst disable */
    writel(AXI_CRG_RST_MASK, (void *)(reg_addr + AXI_CRG_RST_EN));
    writel(AXI_CRG_RST_MASK, (void *)(reg_addr + AXI_CRG_RST_DIS));
    printk("[aximem] clk en rst dis ok\n");
#endif

    printk("[aximem] start\n");
    g_aximem_cache_info.total = AXIMEM_CACHE_MAX_LEN;
    g_aximem_cache_info.left = AXIMEM_CACHE_MAX_LEN;
    g_aximem_cache_info.phy = AXIMEM_BASE_ADDR;

    g_aximem_cache_info.virt =
        (unsigned long long)(uintptr_t)ioremap_cache(g_aximem_cache_info.phy, g_aximem_cache_info.total);
    if (!g_aximem_cache_info.virt) {
        printk("[aximem]  no cache mem\n");
    }
    INIT_LIST_HEAD(&g_aximem_cache_info.head);
    spin_lock_init(&g_aximem_cache_info.lock);
    g_aximem_noncache_info.total = AXIMEM_UNCACHE_MAX_LEN;
    g_aximem_noncache_info.left = AXIMEM_UNCACHE_MAX_LEN;
    g_aximem_noncache_info.phy = AXIMEM_BASE_ADDR + AXIMEM_CACHE_MAX_LEN;
    g_aximem_noncache_info.virt =
        (unsigned long long)(uintptr_t)ioremap(g_aximem_noncache_info.phy, g_aximem_noncache_info.total);
    if (!g_aximem_noncache_info.virt) {
        printk("[aximem]  no noncache mem\n");
    }
    INIT_LIST_HEAD(&g_aximem_noncache_info.head);
    spin_lock_init(&g_aximem_noncache_info.lock);
    printk("[aximem] ok\n");
    return 0;
}
/* return phy */
#define AXIMEM_ALIGN_MSK 0x3
unsigned long long bsp_aximem_get(const char *name, unsigned int cache, unsigned int size, unsigned int align,
    unsigned long long *virt)
{
    struct aximem_node_info *node = NULL;
    struct aximem_info *info = NULL;
    unsigned long flags;
    unsigned int curr;

    if (name == NULL) {
        printk("[aximem] input name is NULL \n");
        return 0;
    }
    if (virt == NULL || !size || (size & AXIMEM_ALIGN_MSK)) {
        printk("[aximem] input error virt %llx size %u \n", (unsigned long long)((uintptr_t)virt), size);
        return 0;
    }
    node = kzalloc(sizeof(struct aximem_node_info), GFP_KERNEL);
    if (node == NULL) {
        printk("[aximem] alloc node fail \n");
        return 0;
    }
    if (strncpy_s(node->name, sizeof(node->name), name, strlen(name))) {
        kfree(node);
        return 0;
    }
    if (cache) {
        info = &g_aximem_cache_info;
    } else {
        info = &g_aximem_noncache_info;
    }
    if (size > info->left) {
        printk("[aximem] no space left\n");
        kfree(node);
        return 0;
    }
    spin_lock_irqsave(&info->lock, flags);
    list_add_tail(&node->list, &info->head);
    info->left -= size;
    curr = info->curr;
    info->curr += size;
    spin_unlock_irqrestore(&info->lock, flags);
    node->phy = info->phy + curr;
    node->virt = info->virt + curr;
    node->len = size;
    node->user++;

    *virt = node->virt;
    return node->phy;
}

void aximem_status_show(void)
{
    struct list_head *entry = NULL;
    struct list_head *temp = NULL;
    unsigned long flags;
    struct aximem_node_info *node = NULL;

    spin_lock_irqsave(&g_aximem_cache_info.lock, flags);
    list_for_each_safe(entry, temp, &g_aximem_cache_info.head)
    {
        node = list_entry(entry, struct aximem_node_info, list);
        if (node != NULL) {
            printk("name:%s phy:%llx virt:%llx size:%u\n", node->name, node->phy, node->virt, node->len);
        }
    }
    spin_unlock_irqrestore(&g_aximem_cache_info.lock, flags);
    spin_lock_irqsave(&g_aximem_noncache_info.lock, flags);

    list_for_each_safe(entry, temp, &g_aximem_noncache_info.head)
    {
        node = list_entry(entry, struct aximem_node_info, list);
        if (node != NULL) {
            printk("name:%s phy:%llx virt:%llx size:%u\n", node->name, node->phy, node->virt, node->len);
        }
    }

    spin_unlock_irqrestore(&g_aximem_noncache_info.lock, flags);
}


void aximem_test_ut(unsigned int type, unsigned int size)
{
    unsigned long long phy;
    unsigned long long virt;
    phy = bsp_aximem_get("spe", type, size, 1, &virt);
    phy = bsp_aximem_get("ipf", type, size, 1, &virt);
    phy = bsp_aximem_get("maa", type, size, 1, &virt);
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
core_initcall(aximem_init)
#endif
