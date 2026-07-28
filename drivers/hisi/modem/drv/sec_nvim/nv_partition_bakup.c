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


#include <linux/vmalloc.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_partition_img.h"
#include "nv_partition_bakup.h"

#define THIS_MODU mod_nv

extern struct nv_path_info_stru g_nv_path;

/*****************************************************************************
 函 数 名  : nv_bak_flush
 功能描述  : 将内存中的NV数据同步至NV备份区，
             作用范围:1.升级NV启动刷新2.C核发起的同步操作
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_bak_flush(void)
{
    u32 ret;

    FILE* fp = NULL;
    nv_global_info_s* ddr_info = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;

    nv_create_flag_file((s8*)NV_BACK_FLAG_PATH);

    fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        nv_record("open %s fail!\n",NV_BACK_PATH);
        return BSP_ERR_NV_NO_FILE;
    }

    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ddr_info->file_len,fp);
    nv_file_close(fp);
    if(ret != ddr_info->file_len)
    {
        nv_record("write fail!write_len=0x%x,ret_len=0x%x.\n",ddr_info->file_len,ret);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    if(nv_file_update(NV_BACK_PATH))
    {
        nv_record("updata fail!\n");
    }

    nv_delete_flag_file((s8*)NV_BACK_FLAG_PATH);

    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_bak_is_need_flush
 功能描述  : 是否刷新备份区数据
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
bool nv_bak_is_need_flush(void)
{
    nv_global_info_s* global_info = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;

    if(global_info->flush_backnvm == FLUSH_NVM_BACK)
    {
        return true;
    }

    return false;
}

/*****************************************************************************
 函 数 名  : nv_bak_recover
 功能描述  : 从备份区恢复NV镜像至工作分区
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_bak_recover(void)
{
    u32 ret;
    FILE* fp = NULL;

    u8 *pdata;
    nv_file_info_s* file_info = (nv_file_info_s*)(NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    if(nv_flag_file_isExist((s8 *)NV_BACK_FLAG_PATH))
    {
        nv_record("%s file is invalid!\n", (s8*)NV_BACK_FLAG_PATH);
        return NV_ERROR;
    }

    fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("open %s file fail!\n", (s8*)NV_BACK_PATH);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    pdata = vmalloc(file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size);
    if(NULL == pdata)
    {
        nv_record("malloc fail!\n");
        (void)nv_file_close(fp);
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    (void)nv_file_seek(fp, file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_offset, SEEK_SET);
    ret = (u32)nv_file_read(pdata, 1, file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size, fp);
    (void)nv_file_close(fp);
    if(ret != file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size)
    {
        vfree(pdata);
        nv_record("file len err!ret 0x%x, datalen 0x%x\n", ret, file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    fp = nv_file_open((s8*)g_nv_path.file_path[NV_IMG_RDWR],(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        vfree(pdata);
        nv_record("open %s file fail!\n", (s8*)g_nv_path.file_path[NV_IMG_RDWR]);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    ret = (u32)nv_file_write((u8*)pdata,1,file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size,fp);
    nv_file_close(fp);
    if(ret != file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size)
    {
        vfree(pdata);
        nv_record("file len err!ret 0x%x, datalen 0x%x\n", ret,
                    file_info[NV_FILE_ATTRIBUTE_RDWR -1].file_size);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }

    vfree(pdata);
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_bak_reload
 功能描述  : 从备份区恢复NV镜像至工作分区
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_bak_reload(u32 type)
{
    u32 ret;
    u32 length;
    unsigned long offset;
    nv_file_info_s* file_info = (nv_file_info_s*)(NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);

    offset = NV_GLOBAL_CTRL_INFO_ADDR + file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_offset;
    length = file_info[NV_FILE_ATTRIBUTE_RDWR - 1].file_size;

    nv_printf("reload nv file from modemnvm_back start!\n");

    ret = nv_bak_recover();
    if(ret)
    {
        nv_record("recover nv file fail!\n");
        return NV_ERROR;
    }

    ret = nv_img_load_file(type, offset, length);
    if(ret)
    {
        nv_record("sec_os reload nv file fail!\n");
        return NV_ERROR;
    }

    nv_printf("reload nv file from modemnvm_back sucess!\n");
    return NV_OK;
}

EXPORT_SYMBOL(nv_bak_flush);

