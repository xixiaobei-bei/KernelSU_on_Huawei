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

#include <product_config.h>
#include <stdarg.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/rtc.h>
#include <mdrv.h>
#include <product_nv_id.h>
#include <product_nv_def.h>
#include <securec.h>
#include <bsp_slice.h>
#include <bsp_blk.h>
#include <bsp_dump.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_partition_img.h"
#include "nv_partition_bakup.h"
#include "nv_debug.h"
#include "nv_msg.h"


#define THIS_MODU mod_nv

struct nv_global_ctrl_stru *g_flash_emmc_info_ptr = NULL;

/*****************************************************************************
 函 数 名  : nv_create_flag_file
 功能描述  : 创建文件标志
*****************************************************************************/
void nv_create_flag_file(const s8 *path)
{
    FILE *fp = NULL;

    if (!mdrv_file_access(path, 0))
        return;
    fp = mdrv_file_open((char *)path, (char *)NV_FILE_WRITE);
    if (fp != NULL) {
        mdrv_file_close(fp);
        return;
    } else
        return;
}

/*****************************************************************************
 函 数 名  : nv_delete_flag_file
 功能描述  : 删除文件标志
*****************************************************************************/
void nv_delete_flag_file(const s8 *path)
{
    if (mdrv_file_access((char *)path, 0))
        return;
    else
        mdrv_file_remove((char *)path);
}

/*****************************************************************************
 函 数 名  : nv_flag_file_isExist
 功能描述  : 检查文件标志是否存在
*****************************************************************************/
bool nv_flag_file_isExist(const s8 *path)
{
    return (mdrv_file_access((char *)path, 0) == 0) ? true : false;
}

u32 nv_clear_upgrade_flag(void)
{
    u32 ret;

    nv_global_info_s *ddr_info = (nv_global_info_s *)NV_GLOBAL_INFO_ADDR;

    /* set upgrade flag */
    ret = nv_upgrade_set_flag((bool)false);
    if (ret) {
        nv_record("clear upgrade flag failed 0x%x!\n", ret);
        return ret;
    }

    ret = nv_upgrade_file_updata();
    if (ret) {
        nv_record("nv file update failed 0x%x!\n", ret);
        return ret;
    }

    ddr_info->mem_file_type = NV_MEM_UPGRADE_SUCCESS;
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_data_writeback
 功能描述  : 回写NV镜像到各分区，包括工作分区和备份区
*****************************************************************************/
u32 nv_data_writeback(void)
{
    u32 ret;

    ret = nv_img_flush_carrier();
    if (ret) {
        nv_record("write carrier file back to [img] fail! ret :0x%x\n", ret);
        return ret;
    }

    nv_img_remove_all();

    ret = nv_img_flush_all();
    if (ret) {
        nv_record("write nv data back to [img] fail! ret :0x%x\n", ret);
        return ret;
    }

    ret = nv_bak_flush();
    if (ret) {
        nv_record("write nv file back to [back] fail! ret :0x%x\n", ret);
        return ret;
    }

    return NV_OK;
}


/*
 * get nv read right,check the nv init state or upgrade state to read nv,
 * A core may read nv data after kernel init ,C core read nv data must behine the phase of
 *       acore kernel init or acore init ok
 */
bool nv_read_right(void)
{
    nv_global_info_s *ddr_info = NULL;

    ddr_info = (nv_global_info_s *)NV_GLOBAL_INFO_ADDR;
    if (ddr_info->acore_init_state < NV_KERNEL_INIT_DOING) {
        return false;
    }
    return true;
}

u32 nv_read_from_mem(nv_rdwr_req *rreq, nv_item_info_s *item_info, nv_ctrl_info_s *ctrl_info)
{
    u8 *mdm_base;
    int ret;

    mdm_base = nv_get_item_mdmbase(rreq->modemid, item_info, ctrl_info);
    if (mdm_base == NULL) {
        return NV_ERROR;
    }

    ret = memcpy_s((void *)rreq->pdata, rreq->size, (void *)(mdm_base + rreq->offset), rreq->size);
    if (ret) {
        nv_printf("memcpy err\n");
        return NV_ERROR;
    }

    return NV_OK;
}

EXPORT_SYMBOL(nv_data_writeback);
