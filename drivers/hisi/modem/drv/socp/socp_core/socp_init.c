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


#include "socp.h"
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#include <osl_thread.h>
#include <securec.h>
#include "bsp_version.h"
#include "bsp_dt.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include "bsp_softtimer.h"
#include "product_config.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv.h>
#else
#include "bsp_bbpds.h"
#endif


struct socp_enc_dst_log_cfg g_enc_dst_buf_log_cfg;
socp_rsv_mem_s g_socp_rsv_mem_info;
socp_early_cfg_s g_socp_early_cfg;

struct platform_device *g_socp_pdev;
static struct platform_device socp_pdev = {
    .name = "modem_socp",
    .id = -1,
};

static const struct of_device_id socp_dev_of_match[] = {
    {
        .compatible = "hisilicon,socp_balong_app",
        .data = NULL,
    },
    {},
};
static int socp_driver_probe(struct platform_device *pdev);
static struct platform_driver g_socp_driver = {
    .driver = {
        .name = "modem_socp",
        .owner = THIS_MODULE,
        .of_match_table = socp_dev_of_match,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
        },
    .probe = socp_driver_probe,
};

static int socp_driver_probe(struct platform_device *pdev)
{
    of_dma_configure(&pdev->dev, NULL);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); //lint !e598 !e648
    g_socp_pdev = pdev;
    return socp_reserved_mem_init();
}

u8 *bsp_socp_get_pdev_addr(void)
{
    return (u8 *)g_socp_pdev;
}

s32 socp_clk_enable(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    struct clk *cSocp;
    int ret;

    /* 打开SOCP时钟 */
    cSocp = clk_get(NULL, "socp_clk");
    ret = clk_prepare(cSocp);
    if (ret) {
        socp_error("[init]clk prepare failed,ret=0x%x\n", ret);
        return ret;
    }
    ret = clk_enable(cSocp);
    if (ret) {
        socp_error("[init]clk open failed,ret=0x%x\n", ret);
        return ret;
    }
#endif
    return BSP_OK;
}

/*
 * 函 数 名: socp_init
 * 功能描述: 模块初始化函数
   初始化顺序 :
   1.全局变量初始化
   2.使能时钟，手机平台由kirin打开，无需关钟。 MBB由当前代码打开时钟
   3.初始化任务
   4.初始化基地址，挂中断(挂中断之后，有可能中断立即有上报，此时依赖基地址和任务状态ok)
   5.软复位
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 socp_init(void)
{
    s32 ret;
    device_node_s *dev = NULL;

    socp_crit("[init]start\n");

    ret = socp_clk_enable();
    if (ret) {
        return ret;
    }

    socp_enc_dst_spinlock_init();

    socp_hal_init();

    dev = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,socp_balong_app");
    if (dev == NULL) {
        socp_error("get socp dtsi node err\n");
        return BSP_ERROR;
    }

    if (bsp_dt_device_is_available(dev) == false) {
        ret = platform_device_register(&socp_pdev);
        if (ret) {
            socp_error("register socp platform device err\n");
            return ret;
        }
    }

    ret = platform_driver_register(&g_socp_driver);
    if (ret) {
        socp_error("register socp driver fail, ret=0x%x\n", ret);
        return ret;
    }

    socp_mode_swt_init();

    ret = socp_int_manager_init();
    if (ret) {
        return ret;
    }

    bsp_socp_set_timeout(SOCP_TIMEOUT_TFR_SHORT, SOCP_TIMEOUT_TFR_SHORT_VAL);

    socp_crit("[init]ok\n");

    return BSP_OK;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(socp_init);
#endif

/*
 * 函 数 名: socp_ind_delay_init
 * 功能描述: socp使用的预留内存申请函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 socp_reserved_mem_init(void)
{
    device_node_s *dev = NULL;
    s32 ret;

    /* get rsv mem info from kernel */
    socp_get_rsv_mem_info();

    /* 获取cmdline参数 */
    socp_get_cmdline_info();

    /* 对rsv mem进行map */
    ret = socp_rsv_mem_mmap();
    if (ret) {
        socp_error("rsv mem mmap fail\n");
        return ret;
    }

    dev = socp_hal_get_dev_info();
    if (dev == NULL) {
        return BSP_ERROR;
    }

    ret = socp_buffer_init(dev);
    if (ret) {
        socp_error("socp dst logbuffer init faield!\n");
        return ret;
    }

    socp_crit("reserved mem init sucess\n");

    return BSP_OK;
}

void socp_get_rsv_mem_info(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    socp_attr_s *socp_mem_attr = NULL;

    if (g_socp_rsv_mem_info.init_flag != BSP_TRUE) {
        socp_mem_attr = bsp_socp_get_mem_info();
        if (socp_mem_attr != NULL) {
            g_socp_rsv_mem_info.virt_addr = NULL;
            g_socp_rsv_mem_info.phy_addr = socp_mem_attr->socp_mem_info.base_addr;
            g_socp_rsv_mem_info.buff_size = socp_mem_attr->socp_mem_info.buffer_size;
            g_socp_rsv_mem_info.buff_useable = socp_mem_attr->socp_mem_info.rsv_mem_usable;

            g_socp_rsv_mem_info.init_flag = BSP_TRUE;
        }
    } else {
        g_socp_rsv_mem_info.virt_addr = NULL;
    }
#else
    g_socp_rsv_mem_info.virt_addr = NULL;
#endif

    return;
}


static s32 socp_get_rsv_mem_addr(void)
{
    /* 物理地址是32位的实地址并且是8字节对齐的 */
    if ((g_socp_rsv_mem_info.phy_addr % 8) || (g_socp_rsv_mem_info.phy_addr == 0)) {
        socp_crit("rsv mem addr invalid, 0x%lx\n", g_socp_rsv_mem_info.phy_addr);
        return -1;
    }

    g_socp_early_cfg.phy_addr = g_socp_rsv_mem_info.phy_addr;

    return 0;
}

/*
 * 函 数 名: socp_get_logbuffer_sizeparse
 * 功能描述: 获取socp buffer大小
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
static s32 socp_get_rsv_mem_size(void)
{
    u32 buff_size;

    buff_size = g_socp_rsv_mem_info.buff_size;

    /* socp rsv mem需要长度8字节对齐 */
    if (buff_size % 8) {
        socp_error("BuffSize no 8 byte allignment,BuffSize: 0x%x\n", buff_size);
        return BSP_ERROR;
    }
    g_socp_early_cfg.buff_size = buff_size;
    socp_crit("buff_size 0x%x\n", g_socp_early_cfg.buff_size);

    return BSP_OK;
}

/*
 * 函 数 名: socp_get_cmdline_param
 * 功能描述: 获取cmdline参数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
void socp_get_cmdline_info(void)
{
    s32 ret;

    ret = socp_get_rsv_mem_addr();
    if (ret) {
        g_socp_early_cfg.buff_useable = BSP_FALSE;
        return;
    }

    ret = socp_get_rsv_mem_size();
    if (ret) {
        g_socp_early_cfg.buff_useable = BSP_FALSE;
        return;
    }

    g_socp_early_cfg.buff_useable = BSP_TRUE;
    return;
}

/*
 * 函 数 名: socp_memremap
 * 功能描述: phy_addr -> virt_addr映射
 * 输入参数: phys_addr:需要remap的物理地址
 * size:     需要remap的数据大小
 * 输出参数: 无
 * 返 回 值: remap后的虚拟地址
 */
static void *socp_mem_remap(unsigned long phys_addr, size_t size)
{
    unsigned long i;
    u8 *vaddr = NULL;
    unsigned long npages = ((unsigned long)PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT);
    unsigned long offset = phys_addr & (PAGE_SIZE - 1);
    struct page **pages = NULL;

    pages = vmalloc(sizeof(struct page *) * npages);
    if (pages == NULL) {
        socp_error("vmalloc fail, len=0x%lx\n", sizeof(struct page *) * npages);
        return NULL;
    }

    pages[0] = phys_to_page((uintptr_t)phys_addr);

    for (i = 0; i < npages - 1; i++) {
        pages[i + 1] = pages[i] + 1;
    }

    vaddr = (u8 *)vmap(pages, (unsigned int)npages, (unsigned long)VM_MAP, pgprot_writecombine(PAGE_KERNEL));
    if (vaddr != NULL) {
        vaddr += offset;
    } else {
        socp_error("vmap fail, npages=0x%lx, VM_MAP=0x%lx\n", npages, (unsigned long)VM_MAP);
    }

    vfree(pages);
    return (void *)vaddr;
}

/*
 * 函 数 名: socp_rsv_mem_mmap
 * 功能描述: 物理地址转换成虚拟内存
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
s32 socp_rsv_mem_mmap(void)
{
    /* step1: if logcfg is on, mmap kernel reserved memory */
    if (g_socp_early_cfg.buff_useable == BSP_TRUE) {
        g_socp_rsv_mem_info.virt_addr =
            socp_mem_remap(g_socp_rsv_mem_info.phy_addr, (size_t)g_socp_rsv_mem_info.buff_size);
        if (g_socp_rsv_mem_info.virt_addr == NULL) {
            socp_error("remap socp buffer fail\n");
            g_socp_rsv_mem_info.buff_useable = BSP_FALSE;
            return BSP_ERROR;
        }

        socp_crit("kernel reserved buffer mmap success\n");
    } else {
        socp_crit("do not use kernel rsv mem\n");
    }

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_timeout_init
 * 功能描述: 模块初始化目的buffer上报超时配置的函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 socp_buffer_init(device_node_s *dev)
{
    s32 ret;

    /* step1: if mdmlog rsv mem is useable, use kernel reserved buffer */
    if (g_socp_early_cfg.buff_useable == BSP_TRUE) {
        g_enc_dst_buf_log_cfg.virt_addr = g_socp_rsv_mem_info.virt_addr;
        g_enc_dst_buf_log_cfg.phy_addr = g_socp_rsv_mem_info.phy_addr;
        g_enc_dst_buf_log_cfg.buff_size = g_socp_rsv_mem_info.buff_size;
        g_enc_dst_buf_log_cfg.over_time = g_socp_rsv_mem_info.timeout;
        g_enc_dst_buf_log_cfg.log_on_flag = SOCP_DST_CHAN_DELAY;
    } else { /* step2: if mdmlog rsv mem is not useable, update logOnFlag state */
        g_enc_dst_buf_log_cfg.log_on_flag = SOCP_DST_CHAN_NOT_CFG;
    }

    /* step3: if mdmlog rsv mem is not useable, alloc dma mem for dst channel */
    if (g_enc_dst_buf_log_cfg.log_on_flag == SOCP_DST_CHAN_NOT_CFG) {
        ret = socp_uncacheable_mem_alloc(dev);
        if (ret) {
            socp_error("of_property_read_u32_array failed!\n");
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

/*
 * 函 数 名: socp_uncacheable_mem_alloc
 * 功能描述: 动态申请一致性内存
 * 输入参数: 设备节点
 * 输出参数: 无
 * 返 回 值: 设置成功与否标志
 */
s32 socp_uncacheable_mem_alloc(device_node_s *dev)
{
    dma_addr_t address = 0;
    u8 *buffer = NULL;
    int ret;
    u32 dst_chan[SOCP_DST_CHAN_CFG_BUTT] = {0};
    u32 size;

    ret = bsp_dt_property_read_u32_array(dev, "dst_chan_cfg", dst_chan, (size_t)SOCP_DST_CHAN_CFG_BUTT);
    if (ret == 0) {
        socp_crit("of_property_read_u32_array get size 0x%x!\n", dst_chan[SOCP_DST_CHAN_CFG_SIZE]);
        size = dst_chan[SOCP_DST_CHAN_CFG_SIZE];

        buffer = (u8 *)dma_alloc_coherent(&g_socp_pdev->dev, (size_t)size, &address, GFP_KERNEL);
        if (buffer != NULL) {
            g_enc_dst_buf_log_cfg.phy_addr = (unsigned long)address;
            g_enc_dst_buf_log_cfg.virt_addr = buffer;
            g_enc_dst_buf_log_cfg.buff_size = size;
            g_enc_dst_buf_log_cfg.log_on_flag = SOCP_DST_CHAN_DTS;

            socp_crit("socp_logbuffer_dmalloc success\n");
            return BSP_OK;
        }
    }

    buffer = (u8 *)dma_alloc_coherent(&g_socp_pdev->dev, (size_t)SOCP_BUFFER_MIN, &address, GFP_KERNEL);
    if (buffer == NULL) {
        socp_error("alloc min buffer size fail\n");
        return BSP_ERROR;
    }

    g_enc_dst_buf_log_cfg.phy_addr = (unsigned long)address;
    g_enc_dst_buf_log_cfg.virt_addr = buffer;
    g_enc_dst_buf_log_cfg.buff_size = SOCP_BUFFER_MIN;
    g_enc_dst_buf_log_cfg.log_on_flag = SOCP_DST_CHAN_MIN;

    return BSP_OK;
}


/*
 * 函 数 名: socp_reserve_area
 * 功能描述: 获取系统预留的base和size
 * 输入参数: 内存结构体参数
 * 输出参数: 无
 * 返 回 值: 成功与否
 */
static int __init socp_reserve_area(struct reserved_mem *rmem)
{
    char *status;

    g_socp_rsv_mem_info.init_flag = BSP_TRUE;

    status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
    socp_error("status is %s\n", status);
    if (status && (strncmp(status, "ok", strlen("ok")) != 0)) {
        return 0;
    }

    g_socp_rsv_mem_info.buff_size = (unsigned int)rmem->size;
    g_socp_rsv_mem_info.phy_addr = rmem->base;

    /* if reserved buffer is too small, set kernel reserved buffer is not usable */
    if ((g_socp_rsv_mem_info.phy_addr == 0) || (g_socp_rsv_mem_info.phy_addr % 8)) {
        socp_error("kernel reserved addr is null or not 8bits align!\n");
        g_socp_rsv_mem_info.buff_useable = BSP_FALSE;
        return -1;
    }

    if (g_socp_rsv_mem_info.buff_size % 8) {
        socp_error("kernel reserved buffer size is not 8bits align\n");
        g_socp_rsv_mem_info.buff_useable = BSP_FALSE;
        return -1;
    }

    g_socp_rsv_mem_info.buff_useable = BSP_TRUE;
    socp_crit("kernel reserved buffer is useful\n");

    return 0;
}

RESERVEDMEM_OF_DECLARE(modem_socp, "hisi_mdmlog", (reservedmem_of_init_fn)socp_reserve_area);

#ifndef BSP_CONFIG_PHONE_TYPE
u32 bsp_socp_ds_mem_enable(void)
{
    return bsp_bbpds_get_mem_flag();
}
#endif

u32 bsp_socp_get_log_cfg(socp_encdst_buf_log_cfg_s *cfg)
{
    struct socp_enc_dst_log_cfg *log_cfg = &g_enc_dst_buf_log_cfg;

    if (cfg == NULL) {
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    cfg->log_on_flag = log_cfg->log_on_flag;
    cfg->buffer_size = log_cfg->buff_size;
    cfg->cur_time_out = socp_get_ind_dst_cur_timouet();
    cfg->vir_buffer = log_cfg->virt_addr;
    cfg->phy_buffer_addr = log_cfg->phy_addr;

    return BSP_OK;
}

/*
 * 函 数 名: socp_set_timeout_mode
 * 功能描述: 超时阈值设置函数
 * 输入参数: time_out_en 设置对象选择及使能
 * time_out 超时阈值
 * 输出参数:
 * 返 回 值: 设置成功与否的标识码
 */
void bsp_socp_set_timeout(socp_timeout_en_e time_out_en, u32 time_out)
{
    if (socp_hal_get_timeout_mode() == 0) {
        socp_hal_set_timeout_mode(1);
    }

    socp_hal_set_timeout_threshold(time_out_en, time_out);

    return;
}

u32 socp_get_mem_log_on_flag(void)
{
    return g_enc_dst_buf_log_cfg.log_on_flag;
}
