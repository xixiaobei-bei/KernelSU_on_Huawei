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

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/zlib.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#include <bsp_print.h>
#include <bsp_slice.h>
#include <bsp_cold_patch.h>

#include <securec.h>

#include <hi_mloader.h>
#include "mloader_comm.h"
#include "mloader_debug.h"

#define THIS_MODU mod_mloader

#define MODEM_IMAGE_PATH_ROOT "/modem_fw/"
static char *g_modem_fw_path = MODEM_IMAGE_PATH;

int modem_dir_init(void)
{
    struct file *fp;

    fp = filp_open(MODEM_IMAGE_PATH, O_RDONLY, 0);
    if (!IS_ERR(fp)) {
        g_modem_fw_path = MODEM_IMAGE_PATH;
        goto succeed_to_get_dir;
    }

    fp = filp_open(MODEM_IMAGE_PATH_ROOT, O_RDONLY, 0);
    if (!IS_ERR(fp)) {
        g_modem_fw_path = MODEM_IMAGE_PATH_ROOT;
        goto succeed_to_get_dir;
    }

    mloader_print_err("path(%s) and path(%s) can't access, return\n", MODEM_IMAGE_PATH, MODEM_IMAGE_PATH_ROOT);
    return -EACCES;

succeed_to_get_dir:
    filp_close(fp, NULL);
    mloader_print_err("modem image path: %s\n", g_modem_fw_path);

    return 0;
}

int mloader_get_file_size(const char *filename)
{
    int ret = 0;
    int len;
    int fail_time;
    struct file *fp = NULL;

    len = strnlen(filename, MLOADER_FILE_NAME_LEN);
    /* 如果是patch镜像，查询镜像的加载失败次数，如果失败次数达到3次，不再加载patch镜像 */
    if ((filename[len - 1] == 'p') && (filename[len - 0x2] == '.')) {
        fail_time = mloader_patch_get_load_fail_time();
        if (fail_time >= 0x3) {
            return 0;
        }
    }
    fp = filp_open(filename, O_RDONLY, 0);
    if (!IS_ERR_OR_NULL(fp)) {
        ret = i_size_read(file_inode(fp));
        if (ret <= 0) {
            filp_close(fp, NULL);
            mloader_print_err("fail to get file(%s) size, ret = %d\n", filename, ret);
            return MLOADER_INTERFACE_ERROR;
        }
        filp_close(fp, NULL);
        return ret;
    }
    return ret;
}

int mloader_get_file_name(char *file_name, const char *image_name, u32 is_sec)
{
    u32 ret;
    int len;
    file_name[0] = '\0';

    len = strnlen(image_name, MLOADER_FILE_NAME_LEN);
    /* 如果是patch镜像，则补全patch镜像的路径，否则补全各核镜像的路径 */
    if ((image_name[len - 0x1] == 'p') && (image_name[len - 0x2] == '.')) {
        ret = (u32)strcat_s(file_name, MLOADER_FILE_NAME_LEN, MLOADER_COLD_PATCH_PATH);
    } else {
        ret = (u32)strcat_s(file_name, MLOADER_FILE_NAME_LEN, g_modem_fw_path);
    }

    ret |= (u32)strcat_s(file_name, MLOADER_FILE_NAME_LEN, image_name);
    if (ret != EOK) {
        mloader_print_err("mloader_get_file_name:strncat_s failed1,ret=0x%x!\n", ret);
        return ret;
    }

    return 0;
}

int mloader_read_file(const char *file_name, unsigned int offset, unsigned int length, char *buffer)
{
    struct file *fp = NULL;
    int retval;
    loff_t offset_adp = (loff_t)offset;
    fp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        retval = (int)PTR_ERR(fp);
        mloader_print_err("filp_open(%s) failed, ret:%d", file_name, retval);
        return retval;
    }

    retval = kernel_read(fp, buffer, (unsigned long)length, (loff_t *)&offset_adp);
    if (retval != (int)length) {
        mloader_print_err("kernel_read(%s) failed, retval %d, require len %u\n", file_name, retval, length);
        if (retval >= 0) {
            retval = -EIO;
        }
    }

    filp_close(fp, NULL);
    return retval;
}

int gzip_header_check(const unsigned char *zbuf)
{
    if (zbuf[0x0] != 0x1f || zbuf[0x1] != 0x8b || zbuf[0x2] != 0x08) {
        return 0;
    } else {
        return 1;
    }
}

int mloader_decompress_zlib(u32 ddr_size, void *vaddr_load, void *vaddr, u32 file_size)
{
    char *zlib_next_in = NULL;
    unsigned int zlib_avail_in;
    int ret;
    u32 start, end;

    zlib_next_in = (char *)vaddr_load;

    /* skip over asciz filename */
    if ((unsigned char)zlib_next_in[0x3] & 0x8) {
        /*
         * skip over gzip header (1f,8b,08... 10 bytes total +
         * possible asciz filename)
         */
        zlib_next_in = zlib_next_in + 10U;
        zlib_avail_in = (unsigned)((file_size - 10U) - 8U);
        do {
            /*
             * If the filename doesn't fit into the buffer,
             * the file is very probably corrupt. Don't try
             * to read more data.
             */
            if (zlib_avail_in == 0) {
                mloader_print_err("gzip header error");
                return -EIO;
            }
            --zlib_avail_in;
        } while (*zlib_next_in++);
    } else {
        /*
         * skip over gzip header (1f,8b,08... 10 bytes total +
         * possible asciz filename)
         */
        zlib_next_in = zlib_next_in + 10U;
        zlib_avail_in = (unsigned)((file_size - 10U) - 8U);
    }
    start = bsp_get_slice_value();
    ret = zlib_inflate_blob(vaddr, ddr_size, (void *)zlib_next_in, zlib_avail_in);
    end = bsp_get_slice_value();
    mloader_print_err("zlib inflate time 0x%x.\n", end - start);
    if (ret < 0) {
        mloader_print_err("fail to decompress image , error code %d\n", ret);
        return ret;
    } else {
        mloader_print_err("decompress image success. file length = 0x%x\n", (unsigned)ret);
        return 0;
    }
}
int mloader_decompress(u32 ddr_size, void *vaddr_load, void *vaddr, u32 file_size)
{
    if (gzip_header_check(vaddr_load)) {
        return mloader_decompress_zlib(ddr_size, vaddr_load, vaddr, file_size);
    } else {
        mloader_print_err("unknown head info, please check.\n");
        return MLOADER_PARAM_ERROR;
    }
}
