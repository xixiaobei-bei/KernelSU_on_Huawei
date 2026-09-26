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
#include <linux/string.h>
#include <mdrv_rfile.h>
#include <bsp_partition.h>
#include <bsp_blk.h>
#include <bsp_slice.h>
#include "bsp_rfile.h"
#include "nv_comm.h"
#include "nv_debug.h"
#include "nv_ctrl.h"


#define THIS_MODU mod_nv

struct file_ops_table_stru g_nv_ops = {
#ifdef FEATURE_NV_EMMC_ON
    .fo = nv_emmc_open,
    .fc = nv_emmc_close,
    .frm = nv_emmc_remove,
    .fr = nv_emmc_read,
    .fw = nv_emmc_write,
    .fs = nv_emmc_seek,
    .ff = nv_emmc_ftell,
    .fa = nv_emmc_access,
    .fu = nv_emmc_update_info,
#endif
};

/*
 * Function:    nv_file_init
 * Discription: global info init,flash: get mtd device
 */
u32 nv_file_init(void)
{
    u32 ret;

#ifdef FEATURE_NV_EMMC_ON
    ret = nv_emmc_init();
    if (ret) {
        return ret;
    }
#endif
    return NV_OK;
}

int nv_file_getmode(const char *mode, int *flag)
{
    int ret;
    u32 m;
    u32 o;

    switch (*mode++) {
        case 'r': /* open for reading */
            ret = 0x0004;
            m = RFILE_RDONLY;
            o = 0;
            break;

        case 'w': /* open for writing */
            ret = 0x0008;
            m = RFILE_WRONLY;
            o = RFILE_CREAT | RFILE_TRUNC;
            break;

        case 'a': /* open for appending */
            ret = 0x0008;
            m = RFILE_WRONLY;
            o = RFILE_CREAT | RFILE_APPEND;
            break;

        default: /* illegal mode */

            return (0);
    }

    /* [rwa]\+ or [rwa]b\+ means read and write */
    if ((*mode == '+') || (*mode == 'b' && mode[1] == '+')) {
        ret = 0x0010;
        m = RFILE_RDWR;
    }

    *flag = (int)(m | o);

    /* check for garbage in second character */
    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        nv_printf("1. mode:%c.\n", *mode);
        return (0);
    }

    /* check for garbage in third character */
    if (*mode++ == '\0') {
        return (ret); /* no third char */
    }

    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        nv_printf("3. mode:%c.\n", *mode);
        return (0);
    }

    /* check for garbage in fourth character */
    if (*mode++ == '\0') {
        return (ret); /* no fourth char */
    }

    if (*mode != '\0') {
        nv_printf("5. mode:%c.\n", *mode);
        return (0);
    } else {
        return (ret);
    }
}

#define NON_FS_DIR "/system/"
/*
 * Function: nv_file_open
 * Discription: open file
 * Parameter:   path  :  file path
 *              mode  :  file ops type etc:"r+","rb+","w+","wb+"
 * Output   :   file pointer
 */
FILE *nv_file_open(const s8 *path, const s8 *mode)
{
    struct nv_file_p *fp;
    s32 ret;
    int oflags = 0;
    int flag;

    fp = (struct nv_file_p *)nv_malloc(sizeof(struct nv_file_p));
    if (fp == NULL) {
        return NULL;
    }

    if (strncasecmp((const s8 *)NON_FS_DIR, path, (sizeof(NON_FS_DIR) - 1)) != 0) {
        flag = nv_file_getmode((const char *)mode, &oflags);
        if (flag == 0) {
            nv_printf("nv file getmode fail!flag=%d\n", flag);
            nv_free(fp);
            return NULL;
        }
        ret = bsp_open(path, oflags, 0660);
        if (ret < 0) {
            nv_printf("nv file open fail!ret=%d\n", ret);
            nv_free(fp);
            return NULL;
        } else {
            fp->fd = (void *)(uintptr_t)(long)ret;
        }
        fp->stor_type = NV_FILE_STOR_FS;
    } else {
        fp->fd = g_nv_ops.fo(path, mode);
        fp->stor_type = NV_FILE_STOR_NON_FS;
    }

    return fp;
}

/*
 * Function   : nv_file_read
 * Discription: read nv file in flash
 * Parameter  : ptr:    user buff
 *              size:   unit size
 *              count:  unit count
 *              fp   :  file pointer
 * Output     : read size
 */
u32 nv_file_read(u8 *ptr, u32 size, u32 count, FILE *fp)
{
    struct nv_file_p *fd = (struct nv_file_p *)fp;

    if (fd->stor_type == NV_FILE_STOR_FS) {
        return (u32)bsp_read((u32)(uintptr_t)(fd->fd), (s8 *)ptr, (size * count));
    } else if (fd->stor_type == NV_FILE_STOR_NON_FS) {
        return g_nv_ops.fr(ptr, size, count, fd->fd);
    } else {
        return NV_ERROR;
    }
}
/*
 * Function   : nv_file_write
 * Discription: read nv file in flash
 * Parameter  : ptr:    user buff
 *              size:   unit size
 *              count:  unit count
 *              fp   :  file pointer
 * Output     : write size
 */
u32 nv_file_write(const u8 *ptr, u32 size, u32 count, FILE *fp)
{
    struct nv_file_p *fd = (struct nv_file_p *)fp;
    u32 ret;
    u32 start;
    u32 end;

    if (fd->stor_type == NV_FILE_STOR_FS) {
        start = bsp_get_slice_value();
        ret = (u32)bsp_write_sync((u32)(uintptr_t)(fd->fd), (s8 *)ptr, (size * count));
        end = bsp_get_slice_value();
        nv_debug_record_delta_time(NV_DEBUG_DELTA_WRITE_FILE, start, end);
        return ret;
    } else if (fd->stor_type == NV_FILE_STOR_NON_FS) {
        return g_nv_ops.fw(ptr, size, count, fd->fd);
    } else {
        return NV_ERROR;
    }
}

/*
 * Function   : nv_file_seek
 * Discription: file seek,jump the file pointer to set position
 * Parameter  : fp:     file pointer
 *              offset: comparatity whence position
 *              whence: SEEK_SET,SEEK_CUR,SEEK_SET
 * Output     : seek result
 */
u32 nv_file_seek(FILE *fp, u32 offset, s32 whence)
{
    struct nv_file_p *fd = (struct nv_file_p *)fp;
    s32 ret;

    if (fd->stor_type == NV_FILE_STOR_FS) {
        ret = bsp_lseek((u32)(uintptr_t)(fd->fd), (long)offset, whence);
        if (ret < 0) {
            return NV_ERROR;
        }
        return NV_OK;
    } else if (fd->stor_type == NV_FILE_STOR_NON_FS) {
        return g_nv_ops.fs(fd->fd, offset, whence);
    } else {
        return NV_ERROR;
    }
}

/*
 *  Function   :  nv_file_close
 *  Discription:  file close,reset file pointer
 *  Parameter  :  file pointer
 *  Output     :  result of close
 */
u32 nv_file_close(FILE *fp)
{
    struct nv_file_p *fd = (struct nv_file_p *)fp;
    u32 ret;

    if (fd->stor_type == NV_FILE_STOR_FS) {
        ret = (u32)bsp_close((u32)(uintptr_t)(fd->fd));
    } else if (fd->stor_type == NV_FILE_STOR_NON_FS) {
        ret = g_nv_ops.fc(fd->fd);
    } else {
        ret = NV_ERROR;
    }
    nv_free(fp);
    return ret;
}
/*
 * Function   : nv_file_remove
 * Discription: remove the file
 * Parameter  : path:   file path
 * Output     : the result of remove
 */
u32 nv_file_remove(const s8 *path)
{
    if (strncasecmp((const s8 *)NON_FS_DIR, path, (sizeof(NON_FS_DIR) - 1)) != 0) {
        return (u32)bsp_remove(path);
    } else {
        return g_nv_ops.frm(path);
    }
}

/*
 * Function   : nv_file_ftell
 * Discription: remove the file
 * Parameter  : fp: file pointer
 * Output     : seek of this file
 */
u32 nv_file_ftell(FILE *fp)
{
    struct nv_file_p *fd = (struct nv_file_p *)fp;
    if (fd->stor_type == NV_FILE_STOR_FS) {
        return (u32)bsp_tell((u32)(uintptr_t)(fd->fd));
    } else if (fd->stor_type == NV_FILE_STOR_NON_FS) {
        return g_nv_ops.ff(fd->fd);
    } else {
        return NV_ERROR;
    }
}

/*
 * Function   : nv_file_access
 * Discription: check the file is exist?
 * Parameter  : fp:   file pointer
 * return     : 0:表示有文件 其他没文件
 */
u32 nv_file_access(const s8 *path, s32 mode)
{
    if (strncasecmp((const s8 *)NON_FS_DIR, path, (sizeof(NON_FS_DIR) - 1)) != 0) {
        return (u32)bsp_access((s8 *)path, mode);
    } else {
        return g_nv_ops.fa(path, mode);
    }
}

/*
 * Function   : nv_file_update
 * Discription: update the file info?
 * Parameter  : fp:   file pointer
 * return     : 0:刷新信息成功 其他失败
 */
u32 nv_file_update(const s8 *path)
{
    if (strncasecmp((const s8 *)NON_FS_DIR, path, (sizeof(NON_FS_DIR) - 1)) != 0) {
        return (u32)NV_ERROR;
    } else {
        return g_nv_ops.fu(path);
    }
}

/*
 * get file len
 * return : file len
 */
u32 nv_get_file_len(FILE *fp)
{
    s32 ret;
    u32 seek;

    ret = (s32)nv_file_seek(fp, 0, SEEK_END);
    if (ret) {
        goto out;
    }

    seek = (u32)nv_file_ftell(fp);

    ret = (s32)nv_file_seek(fp, 0, SEEK_SET);
    if (ret) {
        goto out;
    }

    return seek;

out:
    nv_printf("seek file fail\n");
    return NV_ERROR;
}

/*****************************************************************************
函 数 名  : nv_get_bin_file_len
功能描述  : 计算nv.bin文件的大小
输入参数  : fp:待计算的文件
返 回 值  : 文件大小
*****************************************************************************/
u32 nv_get_bin_file_len(nv_ctrl_info_s *ctrl_info, nv_file_info_s *file_info, u32 *file_len)
{
    u32 i;
    *file_len = ctrl_info->ctrl_size;

    for (i = 0; i < ctrl_info->file_num; i++) {
        *file_len += file_info->file_size;
        file_info++;
    }
    if (*file_len >= NV_MAX_FILE_SIZE) {
        nv_printf("file len 0x%x,MAX size 0x%x\n", *file_len, (u32)NV_MAX_FILE_SIZE);
        return BSP_ERR_NV_ITEM_LEN_ERR;
    }
    return NV_OK;
}

EXPORT_SYMBOL(nv_file_close);
EXPORT_SYMBOL(nv_file_open);
EXPORT_SYMBOL(nv_file_seek);
EXPORT_SYMBOL(nv_file_write);
EXPORT_SYMBOL(nv_file_read);
EXPORT_SYMBOL(nv_file_remove);
EXPORT_SYMBOL(nv_file_access);

