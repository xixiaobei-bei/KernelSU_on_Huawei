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

#include "diag_nve.h"
#include <securec.h>
#include <linux/mtd/hisi_nve_interface.h>
#include "adrv.h"
#include "product_config.h"
#include "bsp_version.h"
#include "diag_system_debug.h"


static int nvme_mdmlog_write(u32 index, const u8 *data, u32 len)
{
    int ret;
    struct hisi_nve_info_user pinfo = { 0 };

    pinfo.nv_operation = NVME_WRITE;
    pinfo.nv_number = NVME_MDMLOG_NUM;
    pinfo.valid_size = NVME_MDMLOG_SIZE;

    ret = memcpy_s(pinfo.nv_data + index, NVME_MDMLOG_SIZE - index, data, len);
    if (ret) {
        diag_error("memcpy fail:%x\n", ret);
        return ret;
    }

    ret = hisi_nve_direct_access(&pinfo);
    if (ret) {
        diag_error("write MDMLOG nve failed, ret=%d\n", ret);
        return ret;
    }

    diag_crit("Write MDMLOG nve success!\n");
    return ERR_MSP_SUCCESS;
}

static int nvme_mdmlog_read(u32 index, u8 *buf, u32 len)
{
    int ret;
    struct hisi_nve_info_user pinfo = { 0 };

    pinfo.nv_operation = NVME_READ;
    pinfo.nv_number = NVME_MDMLOG_NUM;
    pinfo.valid_size = NVME_MDMLOG_SIZE;

    ret = hisi_nve_direct_access(&pinfo);
    if (ret) {
        diag_error("Read MDMLOG nve failed\n");
        return ret;
    }

    ret = memcpy_s(buf, NVME_MDMLOG_SIZE, pinfo.nv_data + index, len);
    if (ret) {
        diag_error("memcpy fail,ret=0x%x\n", ret);
        return ret;
    }

    diag_crit("Read MDMLOG nve success !\n");
    return ERR_MSP_SUCCESS;
}

static int diag_set_mdmlog_nve(void)
{
    int ret;
    char mdmlog_buf[NVME_MDMLOG_SIZE];

    (void)memset_s(mdmlog_buf, sizeof(mdmlog_buf), 0, sizeof(mdmlog_buf));

    ret = nvme_mdmlog_read(0, mdmlog_buf, NVME_MDMLOG_SIZE);
    if (ret) {
        return ret;
    }

    if (mdmlog_buf[0] != '1') {
        mdmlog_buf[0] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if (ret) {
            return ret;
        }
    }

    if (mdmlog_buf[2] != '1') {
        mdmlog_buf[2] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if (ret) {
            return ret;
        }
    }

    return ERR_MSP_SUCCESS;
}

__init int diag_mdmlog_nve_init(void)
{
    const bsp_version_info_s *ver_info = NULL;

    ver_info = bsp_get_version_info();
    if (ver_info == NULL) {
        diag_error("get_version_info fail\n");
        return ERR_MSP_PARA_NULL;
    }

    if (ver_info->udp_flag == HW_VER_HIONE_UDP_MAGIC) {
        return diag_set_mdmlog_nve();
    }

    return ERR_MSP_SUCCESS;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(diag_mdmlog_nve_init);
#endif

