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


#include <linux/mtd/hisi_nve_interface.h>
#include <linux/module.h>
#include <stddef.h>
#include <securec.h>
#include <bsp_cold_patch.h>
#include "load_image.h"
#include <bsp_print.h>
#include "bsp_dump.h"
#define THIS_MODU mod_loadm

int bsp_nvem_cold_patch_read(struct cold_patch_info_s *p_cold_patch_info)
{
    int ret = 0;
    struct hisi_nve_info_user nve_cold_patch = {0};
	ssize_t size = sizeof(struct cold_patch_info_s);

    nve_cold_patch.nv_operation = NV_READ;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = size;

    ret = hisi_nve_direct_access(&nve_cold_patch);
    if(ret)
    {
          bsp_err("nvem_cold_patch_read:nvem read failed!\n");
          return ret;
    }
    (void)memcpy_s(p_cold_patch_info,size,&(nve_cold_patch.nv_data[0]),size);
    return 0;
}
int bsp_nvem_cold_patch_write(struct cold_patch_info_s *p_cold_patch_info)
{
    int ret;
    struct hisi_nve_info_user nve_cold_patch = {0};
    ssize_t size = sizeof(struct cold_patch_info_s);

    nve_cold_patch.nv_operation = NV_WRITE;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = size;

    (void)memcpy_s(&(nve_cold_patch.nv_data[0]),size,p_cold_patch_info,sizeof(struct cold_patch_info_s));
    ret = hisi_nve_direct_access(&nve_cold_patch);
    if(ret)
    {
          bsp_err("nvem_cold_patch_write:nvem write failed!\n");
          return ret;
    }
    return 0;
}

void bsp_nvem_cold_patch_read_debug(void)
{
    struct cold_patch_info_s cold_patch_info = {0};
    (void)bsp_nvem_cold_patch_read(&cold_patch_info);
    bsp_err("cold_patch_status: 0x%x\n",cold_patch_info.cold_patch_status);
    bsp_err("modem_patch_update_count: 0x%x\n",cold_patch_info.modem_patch_update_count);
    bsp_err("modem_update_fail_count: 0x%x\n",cold_patch_info.modem_update_fail_count);
    bsp_err("modem_patch_info[NV_PATCH] patch_exist: 0x%x\n",cold_patch_info.modem_patch_info[NV_PATCH].patch_exist);
    bsp_err("modem_patch_info[NV_PATCH] patch_status: 0x%x\n",cold_patch_info.modem_patch_info[NV_PATCH].patch_status);
    bsp_err("modem_patch_info[CCORE_PATCH] patch_exist: 0x%x\n",cold_patch_info.modem_patch_info[CCORE_PATCH].patch_exist);
    bsp_err("modem_patch_info[CCORE_PATCH] patch_status: 0x%x\n",cold_patch_info.modem_patch_info[CCORE_PATCH].patch_status);
    bsp_err("modem_patch_info[DSP_PATCH] patch_exist: 0x%x\n",cold_patch_info.modem_patch_info[DSP_PATCH].patch_exist);
    bsp_err("modem_patch_info[DSP_PATCH] patch_status: 0x%x\n",cold_patch_info.modem_patch_info[DSP_PATCH].patch_status);
}

bool bsp_modem_cold_patch_is_exist(void)
{
    int ret;
    int result;
    struct cold_patch_info_s cold_patch_info;
    ret = bsp_nvem_cold_patch_read(&cold_patch_info);
    if(ret)
        return 0;
    result = cold_patch_info.modem_patch_info[DSP_PATCH].patch_exist || cold_patch_info.modem_patch_info[CCORE_PATCH].patch_exist || cold_patch_info.modem_patch_info[NV_PATCH].patch_exist;
    if( result )
        return 1;
    else 
        return 0;
}

void bsp_modem_cold_patch_update_modem_fail_count(void)
{
    int ret;

    struct cold_patch_info_s cold_patch_info;
    if(!bsp_modem_cold_patch_is_exist())
        return;
    ret = bsp_nvem_cold_patch_read(&cold_patch_info);
    if(ret)
        return;

    if(cold_patch_info.modem_update_fail_count >= 3)
        return;
    cold_patch_info.modem_update_fail_count++;
    ret = bsp_nvem_cold_patch_write(&cold_patch_info);
    if(ret)
        bsp_err("bsp_nvem_cold_patch_write :update nevm failed!\n");
}

ssize_t modem_imag_patch_status_store(struct device *dev, struct device_attribute *attr,
                                       const char *buf, size_t count)
{
    int ret;
    struct cold_patch_info_s nve_cold_patch ;
    ssize_t size = sizeof(struct cold_patch_info_s);
    long modem_cold_patch_clear = 0;
    if ((kstrtol(buf, 10, &modem_cold_patch_clear) < 0) && (modem_cold_patch_clear != 0))
        return 0;

    (void)memset_s(&nve_cold_patch, size, 0, size);
    ret = bsp_nvem_cold_patch_write(&nve_cold_patch);
    if(ret)
    {
        bsp_err("modem_imag_patch_status_store:write nvem failed!\n");
		return 0;
    }

    return count;
}


ssize_t modem_imag_patch_status_show(struct device *dev,
               struct device_attribute *attr, char *buf)
{
    char data_buf[20] = {0};
    struct cold_patch_info_s cold_patch_info;
    ssize_t size = 0;
    if(!buf)
        return 0;
    if(bsp_nvem_cold_patch_read(&cold_patch_info))
    {
        bsp_err("modem_imag_patch_status_show:read nvem failed!\n");
        return 0;
    }
    if(cold_patch_info.cold_patch_status)
    {
        size = strlen("HOTA_SUCCESS:");
        (void)strcat_s(buf, PAGE_SIZE, "HOTA_SUCCESS:");
    }
    else
    {
        size = strlen("HOTA_ERR_STACK:");
        (void)strcat_s(buf,PAGE_SIZE,"HOTA_ERR_STACK:");
    }
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.cold_patch_status) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_update_fail_count) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[NV_PATCH].patch_exist) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[NV_PATCH].patch_status) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[CCORE_PATCH].patch_exist) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[CCORE_PATCH].patch_status) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[DSP_PATCH].patch_exist) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,",");
    size += num_to_str(data_buf,sizeof(data_buf),cold_patch_info.modem_patch_info[DSP_PATCH].patch_status) + 1;
    (void)strcat_s(buf,PAGE_SIZE,data_buf);
    (void)strcat_s(buf,PAGE_SIZE,";");

    return size;
}

int bsp_modem_cold_patch_init(void)
{
    (void)bsp_dump_register_hook("modem_cold_patch",bsp_modem_cold_patch_update_modem_fail_count);
    bsp_err("[init]bsp_modem_cold_patch_init ok!\n");
    return 0;
}





