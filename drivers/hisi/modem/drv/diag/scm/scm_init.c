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

/*
 * 1 头文件包含
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <bsp_dt.h>
#include "scm_common.h"
#include "ppm_common.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "diag_system_debug.h"
#include "scm_rate_ctrl.h"


static struct platform_device diag_pdev = {
    .name = "modem_diag",
    .id = -1,
};

static const struct of_device_id diag_dev_of_match[] = {
    {
        .compatible = "hisilicon,diag_ap",
        .data = NULL,
    },
    {},
};

static int diag_driver_probe(struct platform_device *pdev);
static struct platform_driver g_diag_driver = {
    .driver = {
        .name = "modem_diag",
        .owner = THIS_MODULE,
        .of_match_table = diag_dev_of_match,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
    .probe = diag_driver_probe,
};

u32 scm_src_buff_init(void)
{
    u32 ret;

    ret = scm_init_ind_src_buff();
    if (ret) {
        diag_error("init ind src buff fail(0x%x)\n", ret);
        return ret;
    }
    ret = scm_init_cnf_src_buff();
    if (ret) {
        diag_error("init cnf src buff fail(0x%x)\n", ret);
        return ret;
    }

    return BSP_OK;
}

u32 scm_src_chan_init(void)
{
    u32 ret;

    ret = scm_ind_src_chan_init();
    if (ret) {
        diag_error("init ind src chann fail(0x%x)\n", ret);
        return ret;
    }

    ret = scm_cnf_src_chan_init();
    if (ret) {
        diag_error("init cnf src chan fail(0x%x)\n", ret);
        return ret;
    }

    return BSP_OK;
}
void scm_dst_func_reg(void)
{
    scm_reg_cnf_coder_dst_send_func();
}

u32 scm_dst_mem_init(void)
{
    u32 ret;

    ret = scm_ind_dst_buff_init();
    if (ret) {
        diag_error("init ind dst buff fail(0x%x)\n", ret);
        return ret;
    }

    ret = scm_cnf_dst_buff_init();
    if (ret) {
        diag_error("init cnf dst buff fail(0x%x)\n", ret);
        return ret;
    }

    return BSP_OK;
}
u32 scm_dst_chan_init(void)
{
    u32 ret;

    ret = scm_ind_dst_channel_init();
    if (ret) {
        diag_error("init ind dst chan fail(0x%x)\n", ret);
        return ret;
    }
    ret = scm_cnf_dst_channel_init();
    if (ret) {
        diag_error("init cnf dst chan fail(0x%x)\n", ret);
        return ret;
    }

    return BSP_OK;
}

/*
 * 函 数 名: SCM_Init
 * 功能描述: SCM模块相关初始化
 */
__init int scm_init(void)
{
    u32 ret;
    device_node_s *dev = NULL;

    dev = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,diag_ap");
    if (dev == NULL) {
        diag_error("get diag dtsi node err\n");
        return BSP_ERROR;
    }

    if (bsp_dt_device_is_available(dev) == false) {
        ret = platform_device_register(&diag_pdev);
        if (ret) {
            diag_error("register diag platform device err\n");
            return ret;
        }
    }

    ret = platform_driver_register(&g_diag_driver);
    if (ret) {
        diag_error("driver_register fail,ret=0x%x\n", ret);
        return ret;
    }

    diag_crit("[init]scm reg platform device ok\n");

    return BSP_OK;
}

static int diag_driver_probe(struct platform_device *pdev)
{
    int ret;

    of_dma_configure(&pdev->dev, NULL);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); //lint !e598 !e648

    scm_set_platform_dev(pdev);

    ret = scm_rate_ctrl_init();
    if (ret) {
        diag_error("rate ctrl init fail\n");
    }

    ret = scm_dst_mem_init();
    if (ret) {
        diag_error("init dst mem fail(0x%x)\n", ret);
        return BSP_ERROR;
    }
    ret = scm_dst_chan_init();
    if (ret) {
        diag_error("init dst chan fail(0x%x)\n", ret);
        return BSP_ERROR;
    }

    scm_dst_func_reg();

    ret = scm_src_buff_init();
    if (ret) {
        diag_error("init src mem fail(0x%x)\n", ret);
        return BSP_ERROR;
    }
    ret = scm_src_chan_init();
    if (ret) {
        diag_error("init src chan fail(0x%x)\n", ret);
        return BSP_ERROR;
    }

    diag_crit("[init]scm_init ok\n");

    return BSP_OK;
}

