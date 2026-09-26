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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include <securec.h>
#include "espe_sysfs.h"
#include "../espe_dbg.h"

struct device g_spe_cmd_dev;
struct device_attribute g_spe_cmd_dev_attr;

#define CMD_INIT_NAME "spe_cmd"

struct device_attribute g_dev_attr;

static ssize_t spe_cmd_dev_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    struct spe *spe = &g_espe;

    return sprintf_s(buf, PAGE_SIZE, "SPE status=%d\n", spe->mask_flags);
}

static ssize_t spe_cmd_dev_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    // need to copy buf because we need to modify it
    int ret;
    char cmd_line[512] = {0};

    if (count >= 512) {
        SPE_TRACE("count is %d, too long\n", count);
        return count;
    }

    ret = memcpy_s(cmd_line, sizeof(cmd_line), buf, count);
    if (ret) {
        SPE_TRACE("memcpy_s fail\n");
        return count;
    }

    ret = filter_cmd_parser(cmd_line, sizeof(cmd_line));
    if (ret != SPE_OK) {
        return count;
    }

    ret = exe_spe_cmd();
    if (ret) {
        SPE_TRACE("exe_spe_cmd fail\n");
    }

    return count;
}

int spe_filter_init(void)
{
    int ret;

    SPE_TRACE("100 :enter\n");

    g_spe_cmd_dev.init_name = CMD_INIT_NAME;

    device_initialize(&g_spe_cmd_dev);

    ret = device_add(&g_spe_cmd_dev);
    if (ret) {
        SPE_TRACE("200:device_add failed.   %d\n", ret);
        return -EIO;
    }

    g_spe_cmd_dev_attr.show = spe_cmd_dev_show;
    g_spe_cmd_dev_attr.store = spe_cmd_dev_store;
    g_spe_cmd_dev_attr.attr.name = "spe_cmd";
    g_spe_cmd_dev_attr.attr.mode = S_IRUGO | S_IWUSR;

    ret = device_create_file(&g_spe_cmd_dev, &g_spe_cmd_dev_attr);
    if (ret) {
        SPE_TRACE("espe device_create_file \n");
        return -EIO;
    }

    SPE_TRACE(" leave\n");

    return 0;
}

void spe_filter_exit(void)
{
    device_remove_file(&g_spe_cmd_dev, &g_spe_cmd_dev_attr);
    device_del(&g_spe_cmd_dev);

    return;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(spe_filter_init);
module_exit(spe_filter_exit);
#endif

#define DRV_COPYRIGHT "Copyright(c) Hisilicon, wireless termination chipset dep II"
#define DRV_AUTHOR "ryan, guoyuan"

MODULE_DESCRIPTION("DRV_DESCRIPTION");
MODULE_AUTHOR(DRV_COPYRIGHT " " DRV_AUTHOR);
MODULE_LICENSE("GPL");
