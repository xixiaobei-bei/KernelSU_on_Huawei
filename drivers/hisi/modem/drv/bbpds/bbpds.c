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

#include "bbpds.h"
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include "securec.h"
#include "osl_malloc.h"
#include <mdrv_memory_layout.h>
#include "product_config.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv.h>
#endif
#include "bsp_dt.h"
#include "bsp_bbpds.h"


#define THIS_MODU mod_bbpds

bbpds_resv_mem_info_s g_bbpds_reserved_mem;

static s32 bbpds_task_init(void)
{

    return BSP_OK;
}

#ifdef BSP_CONFIG_PHONE_TYPE
static s32 bbpds_type_phone_init(void)
{
    s32 ret;

    if (bsp_socp_ds_mem_enable()) {
        g_bbpds_reserved_mem.phy_addr = mdrv_mem_region_get("socp_bbpds_ddr", &g_bbpds_reserved_mem.buf_size);
        if (g_bbpds_reserved_mem.phy_addr == 0 || g_bbpds_reserved_mem.buf_size == 0) {
            g_bbpds_reserved_mem.buf_usable = BSP_FALSE;
        } else {
            g_bbpds_reserved_mem.buf_usable = BSP_TRUE;
        }
    }

    ret = bbpds_task_init();
    if (ret) {
        bbpds_error("[init] task init fail, ret = 0x%x\n", ret);
        return ret;
    }

    bbpds_crit("[init] ok, ret = 0x%x\n", ret);
    return BSP_OK;
}
#else
static s32 bbpds_type_mbb_init(void)
{
    struct device_node *dev = NULL;
    s32 ret;

    dev = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,modem_bbpds_app");
    if (dev == NULL) {
        bbpds_error("[init]no bbpds dev\n");
        return BSP_ERROR;
    }

    ret = bbpds_buf_init(dev);
    if (ret) {
        bbpds_error("[init]bbpds buf init failed,ret=0x%x\n", ret);
        return ret;
    }

    ret = bbpds_task_init();
    if (ret) {
        bbpds_error("[init] task init fail, ret = 0x%x\n", ret);
        return ret;
    }

    bbpds_crit("[init] ok, ret = 0x%x\n", ret);
    return BSP_OK;
}
#endif

/*
 * 函 数 名: bbpds_init
 * 功能描述: 模块初始化函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 bbpds_init(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    return bbpds_type_phone_init();
#else
    return bbpds_type_mbb_init();
#endif
}


s32 bbpds_buf_init(struct device_node *dev)
{
    const char *buf_status = NULL;
    s32 ret;
    unsigned long socp_addr;
    u32 socp_size = 0;

    ret = bsp_dt_property_read_string(dev, "status", &buf_status);
    if (ret) {
        bbpds_error("[init]read buff attr fail,ret=0x%x\n", ret);
        return BSP_ERROR;
    }

    socp_addr = mdrv_mem_region_get("socp_bbpds_ddr", &socp_size);
    if (socp_addr == 0 || socp_size == 0) {
        return BSP_ERROR;
    }
    if (strncmp(buf_status, "ok", strlen("ok")) == 0) {
        g_bbpds_reserved_mem.buf_usable = BSP_TRUE;
        g_bbpds_reserved_mem.phy_addr = socp_addr;
        g_bbpds_reserved_mem.buf_size = socp_size;
    } else {
        g_bbpds_reserved_mem.buf_usable = BSP_FALSE;
        g_bbpds_reserved_mem.phy_addr = 0;
        g_bbpds_reserved_mem.buf_size = 0;
    }

    return BSP_OK;
}

u32 bsp_bbpds_get_mem_flag(void)
{
    return g_bbpds_reserved_mem.buf_usable;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bbpds_init);
#endif

