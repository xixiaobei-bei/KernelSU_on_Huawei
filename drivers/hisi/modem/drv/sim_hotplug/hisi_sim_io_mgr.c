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

#include "hisi_sim_io_mgr.h"
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/pm_wakeup.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include "securec.h"
#include <adrv.h>
#include <bsp_print.h>
#include <bsp_icc.h>
#include <product_config.h>

#undef THIS_MODU
#define THIS_MODU mod_simhotplug

s32 set_sim_io_simfun(struct hisi_sim_platform_info *info)
{
    u32 i = 0;
    if (info->sim_io_cfg.virt_addr != NULL) {
        for (i = 0; i < (sizeof(info->sim_io_cfg.iomg_usim_offset) / sizeof(info->sim_io_cfg.iomg_usim_offset[0]));
            i++) {
            writel(USIM_IO_MUX,
                (void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iomg_usim_offset[i])));
        }
        for (i = 0; i < (sizeof(info->sim_io_cfg.iocg_usim_offset) / sizeof(info->sim_io_cfg.iocg_usim_offset[0]));
            i++) {
            writel(USIM_IOCG_VALUE,
                (void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iocg_usim_offset[i])));
        }

        sim_log_err("set gpio fun is USIM\n");
    } else {
        sim_log_err("info->sim_io_cfg.virt_addr is null\n");
    }

    return 0;
}

s32 set_sim_io_gpiofun(struct hisi_sim_platform_info *info)
{
    u32 i = 0;
    if (info->sim_io_cfg.virt_addr != NULL) {
        for (i = 0; i < (sizeof(info->sim_io_cfg.iomg_usim_offset) / sizeof(info->sim_io_cfg.iomg_usim_offset[0]));
            i++) {
            writel(GPIO_IO_MUX,
                (void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iomg_usim_offset[i])));
        }
        for (i = 0; i < (sizeof(info->sim_io_cfg.iocg_usim_offset) / sizeof(info->sim_io_cfg.iocg_usim_offset[0]));
            i++) {
            writel(USIM_IOCG_VALUE,
                (void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iocg_usim_offset[i])));
        }

        sim_log_err("set gpio fun is GPIO\n");
    } else {
        sim_log_err("info->sim_io_cfg.virt_addr is null\n");
    }
    return 0;
}

int set_nanosd_io_simfun(struct hisi_sim_platform_info *info)
{
    return 0;
}

int set_nanosd_io_sdfun(struct hisi_sim_platform_info *info)
{
    return 0;
}

int set_gpio_det_fun(struct hisi_sim_platform_info *info)
{
    return 0;
}

int set_sim_idle_gpio_fun(struct hisi_sim_platform_info *info)
{
    u32 i = 0;
    sim_log_err("set idle gpio start.\n");

    if (info->sd_io_cfg.virt_addr != NULL) {
        for (i = 0; i < (sizeof(info->sd_io_cfg.iomg_usim_offset) / sizeof(info->sd_io_cfg.iomg_usim_offset[0])); i++) {
            writel(SD_GPIO_IO_MUX,
                (void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iomg_usim_offset[i])));
        }

        writel(0x10, (void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[0x0])));
        writel(0x11, (void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[0x1])));
        writel(0x11, (void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[0x2])));

        sim_log_err("set idle gpio fun done.\n");
    } else {
        sim_log_err("info->sim_io_cfg.virt_addr is null\n");
    }

    return 0;
}

u32 sim_io_mux_init(struct hisi_sim_platform_info *info, struct device_node *np)
{
    u32 ret;

    ret = (u32)of_property_read_u32(np, "iocfg_base_adrr", &(info->sim_io_cfg.phy_addr));
    if (ret) {
        sim_log_err("no sim io cfg addr.\n");
        return ret;
    }
    info->sim_io_cfg.virt_addr = ioremap(info->sim_io_cfg.phy_addr, 0x1000);
    if (info->sim_io_cfg.virt_addr == NULL) {
        sim_log_err("sim io cfg ioremap failed.\n");
        return EINVAL;
    }
    ret = (u32)of_property_read_u32_array(np, "iomg_usim_offset", (u32 *)(&(info->sim_io_cfg.iomg_usim_offset)), 0x3);
    ret |= (u32)of_property_read_u32_array(np, "iocg_usim_offset", (u32 *)(&(info->sim_io_cfg.iocg_usim_offset)), 0x3);
    if (ret) {
        sim_log_err("no sim io offset.\n");
        return ret;
    }
    return 0;
}

u32 sd_io_mux_init(struct hisi_sim_platform_info *info, struct device_node *np)
{
    u32 ret;

    ret = (u32)of_property_read_u32(np, "iocfg_sdbase_adrr", &(info->sd_io_cfg.phy_addr));
    if (ret) {
        sim_log_err("no sdbase addr.\n");
        return ret;
    }
    info->sd_io_cfg.virt_addr = ioremap(info->sd_io_cfg.phy_addr, 0x1000);
    if (info->sd_io_cfg.virt_addr == NULL) {
        sim_log_err("ioremap failed.\n");
        return EINVAL;
    }

    ret = (u32)of_property_read_u32_array(np, "iomg_sd_offset", (u32 *)(&(info->sd_io_cfg.iomg_usim_offset)), 0x3);
    ret |= (u32)of_property_read_u32_array(np, "iocg_sd_offset", (u32 *)(&(info->sd_io_cfg.iocg_usim_offset)), 0x3);
    if (ret) {
        sim_log_err("no sd io offset.\n");
        return ret;
    }

    ret = (u32)of_property_read_u32(np, "esim_detect_en", (u32 *)(&(info->sd_io_cfg.esim_detect_en)));
    if (ret) {
        sim_log_err("esim_detect disabled.\n");
        return ret;
    } else {
        if (info->sd_io_cfg.esim_detect_en == 1) {
            ret = (u32)of_property_read_u32_array(np, "esim_det_func_offset",
                (u32 *)(&(info->sd_io_cfg.esim_det_func_offset)), 0x4);
            ret |= (u32)of_property_read_u32_array(np, "esim_det_ctrl_offset",
                (u32 *)(&(info->sd_io_cfg.esim_det_ctrl_offset)), 0x4);
            if (ret) {
                sim_log_err("no esim det and sd offset.\n");
                return ret;
            }
        }
    }

    return 0;
}

int hisi_set_sim_io_mux(struct hisi_sim_platform_info *info)
{
    int status = CARD_MSG_IO_MUX_SUCC;
    u32 ret = 0;

    switch (info->msgfromsci.msg_value) {
        case SIM1_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST:
            ret = (u32)set_sim_io_simfun(info);
            ret |= (u32)set_nanosd_io_sdfun(info);
            sim_log_err("SIM1_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST");
            break;
        case SIM1_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST:
            ret = (u32)set_sim_io_gpiofun(info);
            ret |= (u32)set_nanosd_io_simfun(info);
            sim_log_err("SIM1_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST");
            break;
        case SIM1_IO_MUX_NANOSDIO_USIM_ONLY_REQUEST:
            ret = (u32)set_nanosd_io_simfun(info);
            sim_log_err("SIM1_IO_MUX_NANOSDIO_USIM_ONLY_REQUEST");
            break;
        case SIM1_IO_MUX_SIMIO_USIM_ONLY_REQUEST:
            ret = (u32)set_sim_io_simfun(info);
            sim_log_err("SIM1_IO_MUX_SIMIO_USIM_ONLY_REQUEST");
            break;
        case SIM1_IO_MUX_GPIO_DET_REQUEST:
            ret = (u32)set_gpio_det_fun(info);
            sim_log_err("SIM1_IO_MUX_GPIO_DET_REQUEST");
            break;
        case SIM1_IO_MUX_IDLE_GPIO_REQUEST:
            ret = (u32)set_sim_idle_gpio_fun(info);
            sim_log_err("SIM1_IO_MUX_IDLE_GPIO_REQUEST");
            break;

        default:
            ret = 1;
            break;
    }

    if (ret == 0) {
        status = CARD_MSG_IO_MUX_SUCC;
    } else {
        status = CARD_MSG_IO_MUX_FAIL;
    }
    return status;
}

static char *card_status_to_string(int status)
{
    switch (status) {
        case STATUS_SIM:
            return "STATUS_SIM";

        case STATUS_SD:
            return "STATUS_SD";

        case STATUS_SD2JTAG:
            return "STATUS_SD2JTAG";

        case STATUS_NO_CARD:
        default:
            return "STATUS_NO_CARD";
    }
}

int hisi_sim_get_card_type(struct hisi_sim_platform_info *info)
{
    u8 status;
    status = get_card1_status(info);
    sim_log_err("bsp_icc_send cardtype to cp, status: %d(%s), simid %d, mux_sdsim %d\n", status,
        card_status_to_string(status), info->sim_id, info->mux_sdsim);
    return (int)status;
}
