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


/*lint -save -e537*/
#include <linux/vmalloc.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_crc.h"
#include "nv_partition_img.h"
#include "bsp_dump.h"
/*lint -restore +e537*/

u32 nv_img_write(u8* pdata, u32 len, u32 file_offset)
{
    nv_global_info_s* ddr_info = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;
    u32 ret         = NV_ERROR;
    FILE* fp = NULL;

    nv_debug(NV_API_FLUSH,0,file_offset,len,0);
    nv_debug_record(NV_DEBUG_FLUSHEX_START);

    if((file_offset + len) > (ddr_info->file_len))
    {
        nv_debug(NV_API_FLUSH,1,file_offset,len,ddr_info->file_len);
        nv_printf("nv len is to large! 0x%x 0x%x\n", file_offset, len);
        goto nv_flush_err;
    }

    nv_debug_record(NV_DEBUG_FLUSHEX_OPEN_START);

    if(nv_file_access((s8*)NV_IMG_PATH,0))
    {
        nv_printf("no nv file, create when first write!\n");
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
        if(NULL == fp)
        {
            nv_printf("create nv file failed!\n");
            nv_debug(NV_API_FLUSH, 2, ret,0,0);
            goto nv_flush_err;
        }
        nv_file_close(fp);

        return nv_img_flush_all();
    }
    else
    {
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_RW);
        if(NULL == fp)
        {
            nv_printf("open nv file failed!\n");
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_API_FLUSH,3,ret,0,0);
            goto nv_flush_err;
        }
    }
    nv_debug_record(NV_DEBUG_FLUSHEX_OPEN_END);
    
    (void)nv_file_seek(fp, file_offset ,SEEK_SET);/*jump to write*/
    ret = (u32)nv_file_write((u8*)pdata,1,len,fp);
    nv_file_close(fp);
    if(ret != len)
    {
        nv_printf("write nv file failed!\n");
        nv_debug(NV_API_FLUSH,4, file_offset , ret,len);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s] len :0x%x, pdata :0x%x offset: 0x%x\n",__FUNCTION__,len,pdata, file_offset);
    nv_help(NV_API_FLUSH);
    return ret;
}


u32 nv_img_flush_all(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    nv_global_info_s* ddr_info  = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;
    u32 writeLen = 0;

    nv_create_flag_file((s8*)NV_IMG_FLAG_PATH);
    nv_debug(NV_API_FLUSH,0,0,0,0);
    
    fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_FLUSH,1,ret,0,0);
        goto nv_flush_err;
    }

    writeLen = ddr_info->file_len;
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1, writeLen, fp);
    nv_file_close(fp);
    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_FLUSH,5,(u32)(unsigned long)fp,ret,writeLen);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }
    nv_delete_flag_file((s8*)NV_IMG_FLAG_PATH);

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_FLUSH);
    return ret;
}

/*****************************************************************************
 函 数 名  : nv_resume_item_from_img
 功能描述  : 从工作恢复一个NV项
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
u32 nv_img_resume_item(nv_item_info_s *item_info, u32 modem_id)
{
    u32 ret         = NV_ERROR;
    u8* item_base   = NULL;
    u8* mdm_base    = NULL;
    u32 item_offset = 0;
    u32 mdm_offset  = 0;
    u32 item_len    = 0;
    u32 mdm_len     = 0;
    u8* resume_data = NULL;
    u32 resume_size = 0;
    u32 file_offset = 0;
    u8* temp_buff   = NULL;
    u32 buff_size   = 0;
    bool need_crc   = 0;
    u32 crc_code    = 0;

    item_base   = nv_get_item_base(item_info);
    mdm_base    = nv_get_item_mdmbase(item_info, modem_id, NULL);
    item_offset = nv_get_item_fileoffset(item_info, NULL);
    mdm_offset  = nv_get_item_filemdmoffset(item_info, modem_id, NULL, NULL);
    item_len    = nv_get_item_len(item_info, NULL);
    mdm_len     = nv_get_item_mdmlen(item_info, NULL);

    /* nvid crc */
    if(NV_ITEM_CRC_CHECK_YES)
    {
        resume_data     = item_base;
        resume_size     = item_len;
        file_offset     = item_offset;
        buff_size       = resume_size;
        need_crc        = true;
    }
    /* modemid crc */
    else if(NV_MODEM_CRC_CHECK_YES)
    {
        resume_data     = mdm_base;
        resume_size     = mdm_len;
        file_offset     = mdm_offset;
        buff_size       = resume_size;
        need_crc        = true;
    }
    else
    {
        resume_data     = mdm_base;
        resume_size     = mdm_len;
        file_offset     = mdm_offset;
        buff_size       = resume_size;
        need_crc        = false;
    }

    if(true != nv_check_file_validity((s8 *)NV_IMG_PATH, (s8 *)NV_IMG_FLAG_PATH))
    {
        nv_debug(NV_FUN_RESUME_IMG_ITEM,1,0,0,0);
        nv_mntn_record("resume nvid, img file is invalid itemid=0x%x!\n", item_info->itemid);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    temp_buff = (u8*)vmalloc(buff_size);
    if(NULL == temp_buff)
    {
        nv_debug(NV_FUN_RESUME_IMG_ITEM,2,0,0,0);
        nv_mntn_record("resume nvid vmalloc fail 0x%x ...%s %s \n", item_info->itemid, __DATE__,__TIME__);
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    
    ret = nv_read_file(NV_IMG_PATH, file_offset, temp_buff, &buff_size);
    if(ret)
    {
        vfree(temp_buff);
        nv_debug(NV_FUN_RESUME_IMG_ITEM,3,ret,0,0);
        nv_mntn_record("resume nvid read file fail 0x%x 0x%x ...%s %s \n", ret, item_info->itemid, __DATE__,__TIME__);
        return ret;
    }

    if(need_crc)
    {
        crc_code = *(u32*)(temp_buff+buff_size-4);
        if(nv_crc_check(temp_buff, (buff_size-4), crc_code))
        {
            vfree(temp_buff);
            nv_debug(NV_FUN_RESUME_IMG_ITEM,4,ret,0,0);
            nv_mntn_record("resume nvid crc check fail 0x%x 0x%x ...%s %s \n", ret, item_info->itemid, __DATE__,__TIME__);
            return BSP_ERR_NV_CRC_CODE_ERR;
        }
    }

    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    memcpy(resume_data, temp_buff, resume_size);
    nv_flush_cache(resume_data, resume_size);
    nv_ipc_sem_give(IPC_SEM_NV_CRC);

    vfree(temp_buff);

    return NV_OK;
}

EXPORT_SYMBOL(nv_img_write);
EXPORT_SYMBOL(nv_img_flush_all);
EXPORT_SYMBOL(nv_img_resume_item);


/*lint -restore*/




