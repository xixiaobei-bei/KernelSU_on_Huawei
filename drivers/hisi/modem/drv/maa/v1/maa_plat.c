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

#include "maa.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <bsp_dt.h>
#include <bsp_maa.h>
#include <securec.h>
#include <bsp_aximem.h>

#define MAA_U64_BITLEN 64
#define MAA_OPIPE_NAME_MAX 64
#define MAA_MEM_LINE_DEAULT 100

struct maa_para_s g_maa_para = { { { 0 } } };

struct maa_dt_field_u32_s {
    char *name;
    unsigned int *value;
    unsigned int *size;
};

static struct maa_dt_field_u32_s g_maa_dt_u32[] = {
    {
        "balong,maa-opipe-max-count",
        &g_maa_para.max_opipes,
    },
    {
        "balong,maa-ipipe-max-count",
        &g_maa_para.iparamter.cnt,
    },
    {
        "balong,maa-use-opipe-free",
        &g_maa_para.opipe_for_free,
    },
    {
        "balong,maa-opipe-for-free",
        &g_maa_para.opipe_free_level,
    },
    {
        "balong,maa-release-poll-depth",
        &g_maa_para.release_depth,
    },
    {
        "balong,maa-mem-protect-threshold",
        &g_maa_para.protect_thresh,
    },
    {
        "balong,maa-mem-line",
        &g_maa_para.mem_line,
    },
    {
        "balong,maa-release-in-task",
        &g_maa_para.release_in_task,
    },
    {
        "balong,maa-reserve-mem-size",
        &g_maa_para.reserve_mem_size,
    },
    {
        "balong,maa-opipe-fullfill-idx",
        &g_maa_para.fullfill_idx,
    },
    {
        "balong,maa-opipe-fullfill-count",
        &g_maa_para.fullfill_count,
    },
};

static struct maa_dt_field_u32_s g_maa_dt_array[] = {
    {
        "balong,maa-opipe-level-used",
        g_maa_para.opipes_used,
        &g_maa_para.opipe_cnt,
    },
    {
        "balong,maa-opipe-levels-depth",
        g_maa_para.oparamter.depth,
    },
    {
        "balong,maa-opipe-levels-dthr",
        g_maa_para.oparamter.dthr,
    },
    {
        "balong,maa-opipe-from",
        g_maa_para.oparamter.from,
    },
    {
        "balong,maa-opipe-tab-size",
        g_maa_para.oparamter.tab_size,
    },
    {
        "balong,maa-opipe-disable_irq",
        g_maa_para.oparamter.disable_irq,
    },
    {
        "balong,maa-ipipe-levels-idx",
        g_maa_para.iparamter.ipipeidx,
    },
    {
        "balong,maa-ipipe-levels-depth",
        g_maa_para.iparamter.depth,
    },
    {
        "balong,maa-ddr-buf-depth",
        g_maa_para.ddr_buf_depth,
    },
};

static struct maa_allc_para g_maa_alloc_para_set[] = {
    {
        .name = "MAA_IPIPE_FOR_SPE_512",
        .ipipe_id = MAA_IPIPE_FOR_SPE_512,
    },
    {
        .name = "MAA_IPIPE_FOR_IPF_512",
        .ipipe_id = MAA_IPIPE_FOR_IPF_512,
    },
    {
        .name = "MAA_IPIPE_FOR_SPE_2K",
        .ipipe_id = MAA_IPIPE_FOR_SPE_2K,
    },
    {
        .name = "MAA_IPIPE_FOR_IPF_2K",
        .ipipe_id = MAA_IPIPE_FOR_IPF_2K,
    },
};

static unsigned int maa_dt_get_regphy(device_node_s *np)
{
    const __be32 *reg_addr_p = NULL;
    int na;

    reg_addr_p = bsp_dt_get_property(np, "reg", NULL);
    if (reg_addr_p == NULL) {
        maa_err("get_address failed.\r\n");
        return 0;
    }
    na = bsp_dt_n_addr_cells(np);
    return bsp_dt_read_number(reg_addr_p, na);
}

static void maa_parse_dt(struct maa *maa, device_node_s *np)
{
    unsigned int i;
    int ret;
    int size = 0;

    for (i = 0; i < sizeof(g_maa_dt_u32) / sizeof(g_maa_dt_u32[0]); i++) {
        ret = bsp_dt_property_read_u32(np, g_maa_dt_u32[i].name, g_maa_dt_u32[i].value);
        if (ret) {
            maa_err("%s not found\n", g_maa_dt_u32[i].name);
        }
    }

    for (i = 0; i < sizeof(g_maa_dt_array) / sizeof(g_maa_dt_array[0]); i++) {
        bsp_dt_get_property(np, g_maa_dt_array[i].name, &size);
        if (size <= 0) {
            continue;
        }
        ret = bsp_dt_property_read_u32_array(np, g_maa_dt_array[i].name, g_maa_dt_array[i].value,
            size / sizeof(unsigned int));
        if (ret) {
            maa_err("%s not found\n", g_maa_dt_array[i].name);
        }

        if (g_maa_dt_array[i].size != NULL) {
            *g_maa_dt_array[i].size = size / sizeof(unsigned int);
        }
    }

    for (i = 0; i < sizeof(g_maa_alloc_para_set) / sizeof(g_maa_alloc_para_set[0]); i++) {
        ret = bsp_dt_property_read_u32_array(np, g_maa_alloc_para_set[i].name, &g_maa_alloc_para_set[i].idx, 2); // array size is 2
        if (ret) {
            g_maa_alloc_para_set[i].idx = g_maa_alloc_para_set[i].ipipe_id;
            g_maa_alloc_para_set[i].level = g_maa_alloc_para_set[i].ipipe_id / MAA_MAX_IPIPE_ENDPOINT;
        }
        g_maa_alloc_para_set[i].launch = 1;
    };
}

static int maa_dt_init(struct maa *maa)
{
    device_node_s *np = NULL;

    np = bsp_dt_find_compatible_node(NULL, NULL, "balong,maa31303061");
    if (np == NULL) {
        return -1;
    }

    maa->regs = bsp_dt_iomap(np, 0);
    if (maa->regs == NULL) {
        maa_err("iomap failed.\r\n");
        return -1;
    }

    maa->irq = bsp_dt_irq_parse_and_map(np, 0);
    if (maa->irq < 0) {
        maa_err("bsp_dt_irq_parse_and_map failed.\r\n");
        return -1;
    }

    maa->phy_regs = maa_dt_get_regphy(np);
    if (maa->phy_regs == 0) {
        return -1;
    }

    maa_parse_dt(maa, np);

    return 0;
}

static void maa_clk_init(struct maa *maa)
{
    maa->clk = devm_clk_get(maa->dev, "maa_clk");
    if (IS_ERR_OR_NULL(maa->clk)) {
        maa_err("maa clock not available\n");
    } else {
        if (clk_prepare_enable(maa->clk)) {
            maa_err("failed to enable maa clock\n");
        }
    }
}

static int maa_task_init(struct maa *maa)
{
    wakeup_source_init(&maa->wake_lock, "modem_maa_wake");

    INIT_WORK(&maa->fill_work, maa_refill_bigpool_work);
    maa->workqueue = create_singlethread_workqueue("maa_refill");
    if (maa->workqueue == NULL) {
        maa_err("maa workqueue alloc\n");
        return -1;
    }
    return 0;
}

static void maa_init_reclaim_timer(struct maa *maa)
{
    maa->timeout_jiffies = msecs_to_jiffies(1000); // timer 1000 ms
    init_timer(&maa->timer);
    maa->timer.function = maa_bigpool_timer;
    maa->timer.data = (uintptr_t)maa;
    maa->timer.expires = jiffies + msecs_to_jiffies(1000); // timer 1000 ms
    add_timer(&maa->timer);
}

static void maa_para_update(struct maa *maa)
{
    unsigned int i;

    struct maa_para_s *para = &g_maa_para;
    maa->para = para;
    maa->allc_para = g_maa_alloc_para_set;
    maa->allc_para_size = sizeof(g_maa_alloc_para_set) / sizeof(g_maa_alloc_para_set[0]);
    maa->fullfill_count = para->fullfill_count;
    maa->fullfill_idx = para->fullfill_idx;
    maa->max_opipes = para->opipes_used[para->opipe_cnt - 1]; // get max opipe idx
    maa->mem_line = para->mem_line;
    maa->opipe_for_free = para->opipe_for_free;
    maa->opipe_free_level = para->opipe_free_level;
    maa->opipe_use_aximem = para->opipe_use_aximem;
    maa->protect_thresh = para->protect_thresh;
    maa->release.fifo.depth = para->release_depth;
    maa->release_in_task = para->release_in_task;
    maa->reserve_mem.size = para->reserve_mem_size;
    maa->dbg = (MAA_DEBUG_ALLOC_FREE_TIMESTAMP | MAA_DEBUG_TIMER_EN);
    maa->totalram = totalram_pages;

    for (i = 0; i < para->opipe_cnt; i++) {
        if (para->opipes_used[i] < MAA_OPIPE_MAX) {
            maa->opipes[para->opipes_used[i]].used = MAA_OPIPE_USED_MAGIC;
        }
    }
}

static int maa_probe(struct platform_device *pdev)
{
    struct maa *maa = NULL;

    maa_err("[init] start\n");
    maa = devm_kzalloc(&pdev->dev, sizeof(*maa), GFP_KERNEL);
    if (maa == NULL) {
        maa_err("no map tab alloc\n");
        return -ENOMEM;
    }

    of_dma_configure(&pdev->dev, NULL);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(MAA_U64_BITLEN));
    maa->dev = &pdev->dev;
    maa->irq_flags = 0;

    if (maa_dt_init(maa)) {
        return -1;
    }

    maa_clk_init(maa);

    if (maa_task_init(maa)) {
        return -1;
    }

    maa_init_reclaim_timer(maa);

    if (devm_request_irq(maa->dev, maa->irq, maa_interrupt, maa->irq_flags, "maa", maa)) {
        maa_err("irq=%u  request fail\n", maa->irq);
        return -1;
    }

    maa_para_update(maa);

    return maa_core_probe(maa);
}

static int maa_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct dev_pm_ops g_maa_pmops = {
    .suspend = maa_suspend,
    .resume = maa_resume,
    .prepare = maa_prepare,
    .complete = maa_complete,
};

static struct platform_driver g_maa_pltfm_driver = {
    .probe = maa_probe,
    .remove = maa_remove,
    .driver = {
        .name = "balong_maa",
        .pm = &g_maa_pmops,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

static struct platform_device g_maa_device = {
    .name = "balong_maa",
    .id = -1,
    .num_resources = 0,
};

int maa_init(void)
{
    maa_err("maa_init\n");
    if (platform_device_register(&g_maa_device)) {
        maa_err("dev register fail.\n");
        return -1;
    }

    if (platform_driver_register(&g_maa_pltfm_driver)) {
        maa_err("drv register fail.\n");
        return -1;
    }

    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
core_initcall(maa_init);
#endif
void maa_exit(void)
{
    /*
     * don't need kfree platform_data in exit process,
     * platform_driver_unregister will do it.
     */
    platform_driver_unregister(&g_maa_pltfm_driver);

    return;
}

MODULE_DESCRIPTION("Balong Specific MAA Driver");
MODULE_LICENSE("GPL v2");
