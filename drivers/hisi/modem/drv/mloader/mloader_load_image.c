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

#include <bsp_sysctrl.h>
#include <bsp_sec.h>
#include <bsp_efuse.h>
#include <bsp_mloader.h>
#include <param_cfg_to_sec.h>

#include <hi_mloader.h>
#include <securec.h>
#include "mloader_comm.h"
#include "mloader_load_image.h"

#define THIS_MODU mod_mloader
struct semaphore g_mloader_load_img_sem;

int bsp_mloader_load_image_unsec(const char *file_name, unsigned long image_addr, u32 *image_size, u32 ddr_size,
                                 u32 image_offset)
{
    int ret = 0;
    int file_size;
    int readed_bytes;

    unsigned long paddr;
    void *vaddr = NULL;
    void *vaddr_load = NULL;

    file_size = mloader_get_file_size(file_name);
    if ((file_size <= SECBOOT_VRL_TABLE_SIZE) || ((u32)file_size > ddr_size)) {
        mloader_print_err("file %s size error, file size = 0x%x, ddr size = 0x%x\n", file_name, file_size, ddr_size);
        return file_size;
    }

    if (image_size != NULL) {
        *image_size = file_size;
    }

    paddr = image_addr;
    vaddr = ioremap_wc(paddr, ddr_size);
    if (vaddr == NULL) {
        mloader_print_err("ioremap_wc error\n");
        return -ENOMEM;
    }

#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    vaddr_load = vaddr + ddr_size - file_size;
#else
    vaddr_load = vaddr;
#endif

    file_size = file_size - SECBOOT_VRL_TABLE_SIZE;
    readed_bytes = mloader_read_file(file_name, SECBOOT_VRL_TABLE_SIZE, (u32)file_size, (s8 *)vaddr_load);
    if (readed_bytes != file_size) {
        ret = -EIO;
        mloader_print_err("read_file %s err: readed_bytes 0x%x\n", file_name, file_size);
        goto error_unmap;
    }

#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    ret = mloader_decompress(ddr_size, vaddr_load, vaddr, (u32)file_size);
    if (ret) {
        mloader_print_err("image %s is decompress fail\n", file_name);
        goto error_unmap;
    }
#endif

    mloader_print_err("load image %s success\n", file_name);

error_unmap:
    iounmap(vaddr);

    return ret;
}

/*
 * Function:       load_data_to_secos
 * Description:    从指定偏移开始传送指定大小的镜像
 * Input:
 *             part_name   - 要发送镜像的名称
 *             offset    - 偏移地址
 *             sizeToRead  - 输入参数，要写入的镜像的bytes大小
 * Output:         none
 * Return:         SEC_OK: OK  SEC_ERROR: ERROR码
 */
#if (((defined CONFIG_TZDRIVER) && (defined CONFIG_LOAD_SEC_IMAGE)) || \
    ((defined CONFIG_TRUSTZONE_HM) && (defined CONFIG_LOAD_SEC_IMAGE)))

static u32 get_load_offset(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 ddr_size, int remain_bytes, u32 load_offset)
{
    if (!load_offset) {
        load_offset = mloader_patch_get_load_position_offset(ecoretype, ddr_size, remain_bytes);
    }

#ifdef CONFIG_COMPRESS_CCORE_IMAGE
    if ((!load_offset) && (ecoretype == MODEM))
        /* 将整个gzip格式的压缩镜像放在DDR空间结束位置 */
        load_offset = (u32)((ddr_size - (u32)remain_bytes) & ~0x3F);
#endif
    return load_offset;
}

int load_data_to_secos(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 reserved,
                       u32 ddr_size, u32 index)
{
    int ret = 0;
    int read_bytes;
    int readed_bytes;
    int remain_bytes;
    u32 file_offset = 0;
    u32 skip_offset;
    u32 load_position_offset = 0;
    mloader_addr_s *mloader_addr;

    mloader_addr = bsp_mloader_get_addr();
    /* 读取指定偏移的指定大小 */
    if (offset || size) {
        skip_offset = offset;
        remain_bytes = (int)size;
    } else { /* 读取整个文件 */
        remain_bytes = mloader_get_file_size(file_name);
        if (remain_bytes <= 0) {
            mloader_print_err("error file_size 0x%x\n", remain_bytes);
            return remain_bytes;
        }

        if (remain_bytes <= SECBOOT_VRL_TABLE_SIZE) {
            mloader_print_err("error file_size (0x%x) less than VRL_SIZE\n", remain_bytes);
            return -EIO;
        }
        remain_bytes -= SECBOOT_VRL_TABLE_SIZE;
        skip_offset = SECBOOT_VRL_TABLE_SIZE;
    }

    /* split the size to be read to each SECBOOT_BUFLEN bytes. */
    while (remain_bytes) {
        read_bytes = remain_bytes > MLOADER_SECBOOT_BUFLEN ? MLOADER_SECBOOT_BUFLEN : remain_bytes;
        readed_bytes = mloader_read_file(file_name, skip_offset + file_offset, (u32)read_bytes,
                                         (s8 *)mloader_addr->mloader_secboot_virt_addr_for_ccpu);
        if (readed_bytes < 0 || readed_bytes != read_bytes) {
            mloader_print_err("read_file %s err: readed_bytes 0x%x\n", file_name, readed_bytes);
            return readed_bytes;
        }

        load_position_offset = get_load_offset(ecoretype, ddr_size, remain_bytes, load_position_offset);

        ret = mloader_trans_data_to_os(ecoretype, 0,
                                       mloader_addr->mloader_secboot_phy_addr_for_ccpu,
                                       load_position_offset + file_offset, (u32)read_bytes);
        mloader_print_info(
            "trans data ot os: etype 0x%x ,load_offset 0x%x, to secos file_offset 0x%x, bytes 0x%x success\n",
            ecoretype, load_position_offset + file_offset, file_offset, read_bytes);

        remain_bytes -= read_bytes;
        file_offset += (u32)read_bytes;
    }

    return ret;
}
#else
int load_data_to_secos(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr,
                       u32 ddr_size, u32 index)
{
    return 0;
}
#endif


#if (((defined CONFIG_TZDRIVER) && (defined CONFIG_LOAD_SEC_IMAGE)) || \
    ((defined CONFIG_TRUSTZONE_HM) && (defined CONFIG_LOAD_SEC_IMAGE)))

s32 load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size, u32 index)
{
    s32 ret;
    int readed_bytes;
    mloader_addr_s *mloader_addr;

    mloader_addr = bsp_mloader_get_addr();

    /* load vrl data to sec os */
    readed_bytes = mloader_read_file(file_name, 0, SECBOOT_VRL_TABLE_SIZE,
                                     (char *)(mloader_addr->mloader_secboot_virt_addr_for_ccpu));
    if (readed_bytes < 0 || readed_bytes != SECBOOT_VRL_TABLE_SIZE) {
        mloader_print_err("read_file %s error, readed_bytes 0x%x!\n", file_name, readed_bytes);
        ret = readed_bytes;
        return ret;
    }

    ret = mloader_trans_vrl_to_os(ecoretype, (void *)(mloader_addr->mloader_secboot_virt_addr_for_ccpu),
                                  SECBOOT_VRL_TABLE_SIZE);
    if (ret) {
        /* cov_verified_start */
        mloader_print_err("trans_vrl_to_os error, ret 0x%x!\n", ret);
        return ret;
        /* cov_verified_stop */
    }
    mloader_print_err("trans vrl to secos success\n");

    /* load image data to sec os */
    ret = load_data_to_secos(file_name, 0, 0, ecoretype, run_addr, ddr_size, index);
    if (ret) {
        /* cov_verified_start */
        mloader_print_err("load image %s to secos failed, ret = 0x%x\n", file_name, ret);
        return ret;
        /* cov_verified_stop */
    }
    mloader_print_err("load image %s to secos success\n", file_name);
    return ret;
}

int bsp_mloader_load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size)
{
    int ret;
    osl_sem_down(&g_mloader_load_img_sem);
    ret = load_image(file_name, ecoretype, run_addr, ddr_size, 0);
    osl_sem_up(&g_mloader_load_img_sem);
    if (ret) {
        mloader_print_err("load image %s fail, ret = %d\n", file_name, ret);
        return ret;
    }
    mloader_print_err("load image %s succeed\n", file_name);

    return ret;
}

int bsp_mloader_load_single_image(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype,
                                  u32 run_addr, u32 ddr_size)
{
    int ret;

    osl_sem_down(&g_mloader_load_img_sem);
    ret = load_data_to_secos(file_name, offset, size, ecoretype, run_addr, ddr_size, 0);
    osl_sem_up(&g_mloader_load_img_sem);
    if (ret) {
        mloader_print_err("load image %s fail, ret = %d\n", file_name, ret);
        return ret;
    }
    mloader_print_err("load vrl and image %s succeed\n", file_name);
    return ret;
}

int bsp_mloader_load_single_vrl(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype)
{
    int ret;
    mloader_addr_s *mloader_addr;

    mloader_addr = bsp_mloader_get_addr();

    osl_sem_down(&g_mloader_load_img_sem);
    ret = mloader_trans_vrl_to_os(ecoretype, (void *)(mloader_addr->mloader_secboot_virt_addr_for_ccpu),
                                  SECBOOT_VRL_TABLE_SIZE);
    osl_sem_up(&g_mloader_load_img_sem);
    if (ret) {
        mloader_print_err("trans %s(%d) 's vrl to os failed, ret = %d\n", file_name, ecoretype, ret);
        return ret;
    }
    mloader_print_err("trans %s(%d) 's vrl to os succeed\n", file_name, ecoretype);
    return ret;
}

int bsp_mloader_verify_single_image(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype)
{
    int ret;

    ret = mloader_verify_soc_image(ecoretype, 0);
    if (ret) {
        mloader_print_err("verify image(%s) failed,image type is %d, ret = 0x%x\n", file_name, ecoretype, ret);
        return ret;
    }
    mloader_print_err("verify image(%s) succeed,image type is %d\n", file_name, ecoretype);
    return ret;
}
#else
s32 load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size, u32 index)
{
    return 0;
}

int bsp_mloader_load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size)
{
    return 0;
}

int bsp_mloader_load_single_image(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype,
                                  u32 run_addr, u32 ddr_size)
{
    return 0;
}

int bsp_mloader_load_single_vrl(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype)
{
    return 0;
}

int bsp_mloader_verify_single_image(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype)
{
    return 0;
}
#endif

void bsp_mloader_load_image_init(void) {
    osl_sem_init(1, &g_mloader_load_img_sem);
}

