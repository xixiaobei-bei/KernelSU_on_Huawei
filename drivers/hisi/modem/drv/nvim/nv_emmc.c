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


#include <linux/err.h>
#include <product_config.h>
#include <bsp_blk.h>
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_comm.h"
#include "nv_index.h"
#include "nv_debug.h"
#include "nv_crc.h"
#include "../../adrv/adrv.h"
#include "nv_partition_upgrade.h"
#include "nv_cust.h"

static struct nv_emmc_file_header_stru g_nv_file[NV_FILE_BUTT] = {
    {NULL,NV_FILE_DLOAD,          0,0,0,0,NV_DLOAD_PATH,          NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_DLOAD_CUST,     0,0,0,0,NV_DLOAD_CUST_PATH,     NV_DLOAD_CUST_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_BACKUP,         0,0,0,0,NV_BACK_PATH,           NV_BACK_SEC_NAME,  NULL, {}},
    {NULL,NV_FILE_XNV_CARD_1,     0,0,0,0,NV_XNV_CARD1_PATH,      NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_XNV_CARD_2,     0,0,0,0,NV_XNV_CARD2_PATH,      NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_XNV_CARD_3,     0,0,0,0,NV_XNV_CARD3_PATH,      NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_CUST_CARD_1,    0,0,0,0,NV_CUST_CARD1_PATH,     NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_CUST_CARD_2,    0,0,0,0,NV_CUST_CARD2_PATH,     NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_CUST_CARD_3,    0,0,0,0,NV_CUST_CARD3_PATH,     NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_SYS_NV,         0,0,0,0,NV_FILE_SYS_NV_PATH,    NV_SYS_SEC_NAME,   NULL, {}},
    {NULL,NV_FILE_DEFAULT,        0,0,0,0,NV_DEFAULT_PATH,        NV_DEF_SEC_NAME,   NULL, {}},
    {NULL,NV_FILE_XNV_MAP_CARD_1, 0,0,0,0,NV_XNV_CARD1_MAP_PATH,  NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_XNV_MAP_CARD_2, 0,0,0,0,NV_XNV_CARD2_MAP_PATH,  NV_DLOAD_SEC_NAME, NULL, {}},
    {NULL,NV_FILE_XNV_MAP_CARD_3, 0,0,0,0,NV_XNV_CARD3_MAP_PATH,  NV_DLOAD_SEC_NAME, NULL, {}},
};

static struct nv_global_ctrl_stru g_emmc_info = {};

/*
 *get file info in back ,default,nvdload
 */
u32 nv_sec_file_info_init(const s8* name, nv_file_map_s* sec_info)
{
    u32 ret;
    u32 file_len = 0;
    nv_file_map_s  info = {0};
    nv_ctrl_info_s ctrl_info = {0};
    u8* file_info;

    /*first: read nv ctrl file*/
    ret = (u32)bsp_blk_read((char*)name, (loff_t)0,&ctrl_info,sizeof(ctrl_info));
    if(NAND_OK != ret)
    {
        nv_printf("[%s]:patrition name %s,get file magic fail ret 0x%x,\n",__func__,name,ret);
        return ret;
    }

    /*second :check magic num in file head*/
    if(ctrl_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        nv_printf("[%s]:enter this way  1111! %s\n",__func__,name);
        return NV_OK;
    }

    if(ctrl_info.file_size > ctrl_info.file_num * sizeof(nv_file_info_s))
    {
        nv_printf("[%s]:ctrl_info.file_size:0x%x file num:0x%x\n",__func__,ctrl_info.file_size, ctrl_info.file_num);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    /*third: read all nv ctrl file*/
    file_info = (u8*)nv_malloc((size_t)ctrl_info.file_size);
    if(NULL == file_info)
    {
        nv_printf("[%s]:enter this way  2222! %s\n",__func__,name);
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    ret = (u32)bsp_blk_read((char*)name,sizeof(nv_ctrl_info_s),file_info, (size_t)ctrl_info.file_size);
    if(NAND_OK != ret)
    {
        nv_printf("[%s]:enter this way 3333! %s\n",__func__,name);
        goto init_end;
    }

    /*fourth: count nv file len base the ctrl file info*/
    ret = nv_get_bin_file_len(&ctrl_info,(nv_file_info_s*)(unsigned long)file_info,&file_len);
    if(ret)
    {
        nv_printf("[%s]:enter this way 4444! %s\n",__func__,name);
        goto init_end;
    }
    info.len       = file_len;
    info.magic_num = NV_FILE_EXIST;
    info.off       = 0;

    /* coverity[secure_coding] */
    nv_memcpy(sec_info,&info,sizeof(info));
init_end:
    nv_free(file_info);
    return NV_OK;
}

u32 nv_dload_file_info_init(const s8* name, nv_file_map_s* sec_info)
{
    u32 ret;
    u32 total_len;
    nv_dload_head nv_dload;

    ret = (u32)bsp_blk_read((char*)name, (loff_t)0, &nv_dload, sizeof(nv_dload_head));
    if(ret)
    {
        nv_record("read nvdload error in %s sec! ret = 0x%x\n", name, ret);
        return ret;
    }

    if(nv_dload.nv_bin.magic_num != NV_FILE_EXIST)
    {
        nv_record("updata file not exist in %s sec!", name);
        return BSP_ERR_NV_NO_FILE;
    }

    total_len = sizeof(nv_dload_head);
    total_len += ((nv_dload.nv_bin.magic_num == NV_FILE_EXIST)?nv_dload.nv_bin.len:0);
    total_len += ((nv_dload.xnv_map.magic_num == NV_FILE_EXIST)?nv_dload.xnv_map.len:0);
    total_len += ((nv_dload.xnv.magic_num == NV_FILE_EXIST)?nv_dload.xnv.len:0);
    total_len += ((nv_dload.cust_map.magic_num == NV_FILE_EXIST)?nv_dload.cust_map.len:0);
    total_len += ((nv_dload.cust.magic_num == NV_FILE_EXIST)?nv_dload.cust.len:0);
    total_len += sizeof(nv_dload_tail);

    sec_info->magic_num = NV_FILE_EXIST;
    sec_info->off = 0;
    sec_info->len = total_len;

    return NV_OK;
}

u32 nv_get_dload_nv_info(u32* offset,u32* len)
{
    if(g_emmc_info.dload_nv.magic_num == NV_FILE_EXIST)
    {
        *offset = g_emmc_info.dload_nv.off;
        *len    = g_emmc_info.dload_nv.len;
        return NV_OK;
    }
    else
    {
        return NV_ERROR;
    }
}

u32 nv_get_dload_nvcust_info(u32* offset,u32* len)
{
    if(g_emmc_info.dload_nvcust.magic_num == NV_FILE_EXIST)
    {
        *offset = g_emmc_info.dload_nvcust.off;
        *len    = g_emmc_info.dload_nvcust.len;
        return NV_OK;
    }
    else
    {
        return NV_ERROR;
    }
}

static inline u32 nv_get_sys_nv_info(const s8* mode,u32* offset,u32* len)
{
    s32 ret = strncmp(mode, NV_FILE_READ, sizeof(NV_FILE_READ));

    nv_check_file_mode(mode);

    if(B_READ  == ret)
    {
        if(g_emmc_info.sys_nv.magic_num != NV_FILE_EXIST)
        {
            return NV_ERROR;
        }
        *offset = g_emmc_info.sys_nv.off;
        *len    = g_emmc_info.sys_nv.len;
        return NV_OK;
    }
    else
    {
        *offset = 0;
        *len    = 0;
        return NV_OK;
    }

}
static inline u32 nv_get_default_nv_info(const s8* mode,u32* offset,u32* len)
{
    s32 ret = strncmp(mode, NV_FILE_READ, sizeof(NV_FILE_READ));

    nv_check_file_mode(mode);

    if(B_READ  == ret)
    {
        if(g_emmc_info.def_sec.magic_num == NV_FILE_EXIST)
        {
            *offset = g_emmc_info.def_sec.off;
            *len    = g_emmc_info.def_sec.len;
            return NV_OK;
        }
        return NV_ERROR;
    }
    else
    {
        *offset = 0;
        *len    = 0;
        return NV_OK;
    }
}
static inline u32 nv_get_back_nv_info(const s8* mode,u32* offset,u32* len)
{
    s32 ret = strncmp(mode, NV_FILE_READ, sizeof(NV_FILE_READ));

    nv_check_file_mode(mode);

    if(B_READ  == ret)
    {
        if(g_emmc_info.bak_sec.magic_num == NV_FILE_EXIST)
        {
            *offset = g_emmc_info.bak_sec.off;
            *len    = g_emmc_info.bak_sec.len;
            return NV_OK;
        }
        return NV_ERROR;
    }
    else
    {
        *offset = 0;
        *len    = 0;
        return NV_OK;
    }
}


/*****************************************************************************
 函 数 名  : nv_flash_update_info
 功能描述  : 更新各个分区的信息
 输入参数  : void
 输出参数  : 无
 返 回 值  : 0 成功 其他失败
*****************************************************************************/
u32 nv_emmc_update_info(const s8* path)
{
    u32 ret;
    u32 i;

    for(i=0; i<NV_FILE_BUTT; i++)
    {
        if(0 == strncmp(path, g_nv_file[i].path, strlen(g_nv_file[i].path) + 1))
        {
            break;
        }
    }
    switch(i)
    {
        case NV_FILE_DLOAD:
            ret = nv_dload_file_info_init(g_nv_file[NV_FILE_DLOAD].name,&g_emmc_info.dload_nv);
            if(ret)
            {
                if(ret == BSP_ERR_NV_NO_FILE)
                {
                    return NV_OK;
                }
                nv_printf("update nv_modem or nv file fail!if mbb, ignore!\n");
            }
            break;
        case NV_FILE_DLOAD_CUST:
            ret = nv_dload_file_info_init(g_nv_file[NV_FILE_DLOAD_CUST].name,&g_emmc_info.dload_nvcust);
            if(ret)
            {
                nv_printf("update nv_cust file fail!if mbb, ignore!\n");
            }
            break;
        case NV_FILE_BACKUP:
            ret = nv_sec_file_info_init(g_nv_file[NV_FILE_BACKUP].name,&g_emmc_info.bak_sec);
            if(ret)
            {
                nv_printf("update backup file fail\n");
            }
            break;
        case NV_FILE_SYS_NV:
            ret = nv_sec_file_info_init(g_nv_file[NV_FILE_SYS_NV].name,&g_emmc_info.sys_nv);
            if(ret)
            {
                nv_printf("update sys file fail\n");
            }
            break;
        case NV_FILE_DEFAULT:
            ret = nv_sec_file_info_init(g_nv_file[NV_FILE_DEFAULT].name,&g_emmc_info.def_sec);
            if(ret)
            {
                nv_printf("update default file fail\n");
            }
            break;
        default:
            ret = BSP_ERR_NV_INVALID_PARAM;
    }

    return ret;
}


u32 nv_emmc_init(void)
{
    u32 ret;
    u32 i;

    if(!nv_upgrade_xnv_compressed())
    {
        nv_printf("nv image not compress!\n");
        return NV_ERROR;
    }

    /*first init every file sem*/
    for(i = 0;i<NV_FILE_BUTT;i++)
    {
        osl_sem_init(1,&g_nv_file[i].file_sem);
    }
    /* coverity[secure_coding] */
    nv_memset(&g_emmc_info,0,sizeof(struct nv_global_ctrl_stru));

    /* get dlaod info */
    ret = nv_dload_file_info_init(g_nv_file[NV_FILE_DLOAD].name, &g_emmc_info.dload_nv);
    if(ret)
    {
        nv_printf("Init nvim_update fail!if mbb,ignore!\n");
    }

    /* get dload cust info */
    ret = nv_dload_file_info_init(g_nv_file[NV_FILE_DLOAD_CUST].name, &g_emmc_info.dload_nvcust);
    if(ret)
    {
        nv_printf("Init nvim_sys fail!if mbb,ignore!\n");
    }
    /*get sys nv info*/
    ret = nv_sec_file_info_init(g_nv_file[NV_FILE_SYS_NV].name,&g_emmc_info.sys_nv);
    if(ret)
    {
        nv_printf("Init nvim_sys fail!ret=0x%x.\n",ret);
        return NV_ERROR;
    }
    /*get backup nv info*/
    ret = nv_sec_file_info_init(g_nv_file[NV_FILE_BACKUP].name,&g_emmc_info.bak_sec);
    if(ret)
    {
        nv_printf("Init nvim_back fail!ret=0x%x.\n",ret);
        return NV_ERROR;
    }

    /*get default nv info*/
    ret = nv_sec_file_info_init(g_nv_file[NV_FILE_DEFAULT].name,&g_emmc_info.def_sec);
    if(ret)
    {
        nv_printf("Init nvim_fac fail!ret=0x%x.\n",ret);
        return NV_ERROR;
    }

    return NV_OK;
}




FILE* nv_emmc_open(const s8* path,const s8* mode)
{
    u32 ret = NV_ERROR;
    u32 i;
    struct nv_emmc_file_header_stru* fd = NULL;
    u32 offset = 0;
    u32 len = 0;

    for(i=0; i<NV_FILE_BUTT; i++)
    {
        if(0 == strncmp(path, g_nv_file[i].path, strlen(g_nv_file[i].path) + 1))
        {
            fd = &g_nv_file[i];
            break;
        }
    }

    if(NULL == fd)
    {
        nv_printf("no find %s!", path);
        return NULL;
    }

    osl_sem_down(&fd->file_sem);
    switch(fd->emmc_type)
    {
        case NV_FILE_DLOAD:
            ret = nv_get_dload_nv_info(&offset,&len);
            break;
        case NV_FILE_DLOAD_CUST:
            ret = nv_get_dload_nvcust_info(&offset,&len);
            break;
        case NV_FILE_BACKUP:
            ret = nv_sec_file_info_init(g_nv_file[NV_FILE_BACKUP].name,&g_emmc_info.bak_sec);
            ret |= nv_get_back_nv_info(mode,&offset,&len);
            break;
        case NV_FILE_SYS_NV:
            ret = nv_get_sys_nv_info(mode,&offset,&len);
            break;
        case NV_FILE_DEFAULT:
            ret = nv_get_default_nv_info(mode,&offset,&len);
            break;
        default:
            ret = BSP_ERR_NV_INVALID_PARAM;
    }

    if(NV_OK != ret)
    {
        osl_sem_up(&fd->file_sem);
        nv_printf("open %s err!ret=0x%x.\n", path, ret);
        return NULL;
    }

    fd->ops ++;
    fd->seek   = 0;
    fd->length = len;
    fd->off    = offset;
    fd->fp     = fd;

    return fd;
}
 
u32 nv_emmc_read(u8* ptr, u32 size, u32 count, FILE* fp)
{
    u32 real_size;
    u32 ret;
    struct nv_emmc_file_header_stru* fd = (struct nv_emmc_file_header_stru*)fp;
    u32 len = size*count;

    if((NULL == fd)||(fd->fp != fd))
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    real_size = ((fd->seek+len) < fd->length)? len: (fd->length - fd->seek );

    ret = (u32)bsp_blk_read((char*)fd->name,(loff_t)((unsigned long)fd->off+fd->seek),(void *)ptr,(size_t)real_size);
    if(ret != NAND_OK)
    {
        nv_printf("read %s err!len=0x%x,ret=0x%x.\n",(char *)fd->name,len,ret);
        return NV_ERROR;
    }

    fd->seek += real_size;
    return real_size;
}

 
u32 nv_emmc_write(u8* ptr, u32 size, u32 count, FILE* fp)
{
    u32 ret = NV_ERROR;
    u32 len = size*count;
    struct nv_emmc_file_header_stru* fd = (struct nv_emmc_file_header_stru*)fp;
    nv_file_map_s* file_info;

    if((NULL == fd)||(fd->fp != fd))
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    switch(fd->emmc_type)
    {
        case NV_FILE_BACKUP:
            file_info = &g_emmc_info.bak_sec;
            break;
        case NV_FILE_SYS_NV:
            file_info = &g_emmc_info.sys_nv;
            break;
        case NV_FILE_DEFAULT:
            file_info = &g_emmc_info.def_sec;
            break;
        case NV_FILE_DLOAD:
            file_info = &g_emmc_info.dload_nv;
            break;
        case NV_FILE_DLOAD_CUST:
            file_info = &g_emmc_info.dload_nvcust;
            break;
        default:
            return NV_ERROR;
    }

    ret = (u32)bsp_blk_write((char*)fd->name,(loff_t)((unsigned long)fd->off + fd->seek),(void *)ptr,(size_t)len);
    if(ret)
    {
        nv_printf("write %s err!len=0x%x,ret=0x%x.\n",(char *)fd->name,len,ret);
        return NV_ERROR;
    }

    file_info->magic_num = NV_FILE_EXIST;
    file_info->len = fd->off + fd->seek + len;
    file_info->off       = 0;
    fd->seek += len;
    return len;
}
 
u32 nv_emmc_seek(FILE* fp,u32 offset,s32 whence)
{

    u32 ret = 0;
    struct nv_emmc_file_header_stru* fd = (struct nv_emmc_file_header_stru*)fp;

    if((NULL == fd)||(fd->fp != fd))
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }
    ret = fd->seek;
    switch(whence)
    {
        case SEEK_SET:
            ret = offset;
            break;
        case SEEK_CUR:
            ret += offset;
            break;
        case SEEK_END:
            ret = fd->length + offset;
            break;
        default:
            nv_printf("invalid whence!\n");
            return NV_ERROR;
    }

    fd->seek = ret;
    return NV_OK;
}

u32 nv_emmc_remove(const s8* path)
{
    u32 ret = NV_ERROR;
    struct nv_emmc_file_header_stru* fd = NULL;
    u32 i;

    for(i=0;i<NV_FILE_BUTT;i++)
    {
        if(0 == strncmp(path, g_nv_file[i].path, strlen(g_nv_file[i].path) + 1))
        {
            fd = &g_nv_file[i];
            break;
        }
    }

    if(NULL == fd)
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }
    switch(fd->emmc_type)
    {
        case NV_FILE_DLOAD:
            g_emmc_info.nv_dload.nv_bin.magic_num = NV_FLASH_NULL;
            break;
        case NV_FILE_DLOAD_CUST:
            g_emmc_info.dload_nvcust.magic_num = NV_FLASH_NULL;
            break;
        case NV_FILE_BACKUP:
            /* coverity[secure_coding] */
            nv_memset(&g_emmc_info.bak_sec,NV_FLASH_FILL,sizeof(nv_file_map_s));
            break;
        case NV_FILE_DEFAULT:
            /* coverity[secure_coding] */
            nv_memset(&g_emmc_info.def_sec,NV_FLASH_FILL,sizeof(nv_file_map_s));
            break;
        case NV_FILE_SYS_NV:
            /* coverity[secure_coding] */
            nv_memset(&g_emmc_info.sys_nv,NV_FLASH_FILL,sizeof(g_emmc_info.sys_nv));
            break;
        default:
            return BSP_ERR_NV_INVALID_PARAM;
    }

    ret = (u32)bsp_blk_erase(fd->name);
    if(ret)
    {
        nv_printf("erase %s err!ret=0x%x.\n",fd->name,ret);
        return ret;
    }

    return NV_OK;
 }

u32 nv_emmc_close(FILE* fp)
{
    struct nv_emmc_file_header_stru* fd = (struct nv_emmc_file_header_stru*)fp;

    if((NULL == fd)||(fd->fp != fd))
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    osl_sem_up(&fd->file_sem);

    fd->fp = NULL;
    fd->seek = 0;
    fd->length = 0;
    fd->off = 0;
    fd->ops --;
    if(fd->ops != 0)
    {
        nv_printf("close %s err!ops=0x%x.\n",fd->name,fd->ops);
        return BSP_ERR_NV_CLOSE_FILE_FAIL;
    }

    return NV_OK;
}

u32 nv_emmc_ftell(FILE* fp)
{
    struct nv_emmc_file_header_stru* fd = (struct nv_emmc_file_header_stru*)fp;

    if((NULL == fd)||(fd->fp != fd))
    {
        nv_printf("fd is NULL!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }
    return fd->seek;
}


u32 nv_emmc_access(const s8* path,s32 mode)
{
    u32 ret = NV_ERROR;
    u32 i;
    struct nv_emmc_file_header_stru* fd = NULL;

    for(i=0; i<NV_FILE_BUTT; i++)
    {
        if(0 == strncmp(path, g_nv_file[i].path, strlen(g_nv_file[i].path) + 1))
        {
            fd = &g_nv_file[i];
            break;
        }
    }
    if(NULL == fd)
    {
        return NV_ERROR;
    }
	/* coverity[self_assign] */
    mode = mode;
    switch(fd->emmc_type)
    {
        case NV_FILE_DLOAD:
            ret = (g_emmc_info.dload_nv.magic_num == NV_FILE_EXIST)?NV_FILE_EXIST:0;
            break;
        case NV_FILE_DLOAD_CUST:
            ret = (g_emmc_info.dload_nvcust.magic_num == NV_FILE_EXIST)?NV_FILE_EXIST:0;
            break;
        case NV_FILE_BACKUP:
            ret = (g_emmc_info.bak_sec.magic_num== NV_FILE_EXIST)?NV_FILE_EXIST:0;
            break;
        case NV_FILE_SYS_NV:
            ret = (g_emmc_info.sys_nv.magic_num == NV_FILE_EXIST)?NV_FILE_EXIST:0;
            break;
        case NV_FILE_DEFAULT:
            ret = (g_emmc_info.def_sec.magic_num == NV_FILE_EXIST)?NV_FILE_EXIST:0;
            break;
        default:
            return NV_ERROR;
    }

    if(ret != NV_FILE_EXIST)
    {
        return NV_ERROR;
    }
    return 0;
}



