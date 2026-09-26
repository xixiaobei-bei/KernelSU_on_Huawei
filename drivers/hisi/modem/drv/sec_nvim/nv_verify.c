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


#include <securec.h>
#include <crypto/hash.h>
#include <bsp_nvim.h>
#include "nv_debug.h"
#include "nv_verify.h"
#include "nv_ctrl.h"


#define THIS_MODU mod_nv

/************************************************************************
 函 数 名  : nv_hash_sha256_calc
 功能描述  : 计算sha256 hash值
*************************************************************************/
u32 nv_hash_sha256_calc(const u8 *data, u32 len, u8 *hash)
{
    u32 ret = NV_OK;
    struct crypto_shash *tfm = NULL;
    struct shash_desc *desc = NULL;
    size_t desc_size;
    int flag;

    tfm = crypto_alloc_shash("sha256", 0, 0);
    if ((tfm == NULL) || (IS_ERR(tfm))) {
        nv_record("crypto_alloc_shash failed!\n");
        return NV_ERROR;
    }

    desc_size = crypto_shash_descsize(tfm) + sizeof(struct shash_desc);
    desc = (struct shash_desc *)vmalloc(desc_size);
    if (desc == NULL) {
        nv_record("vmalloc desc failed!\n");
        crypto_free_shash(tfm);
        return NV_ERROR;
    }

    desc->tfm = tfm;
    desc->flags = 0;

    flag = crypto_shash_init(desc);
    if (flag < 0) {
        nv_record("crypto_shash_init failed!ret=0x%x\n", ret);
        goto error;
    }

    ret = crypto_shash_digest(desc, data, len, hash);
    if (ret) {
        nv_record("crypto_shash_update failed!ret=0x%x \n", ret);
        goto error;
    }

error:
    if (desc != NULL) {
        if (desc->tfm)
            crypto_free_shash(desc->tfm);

        vfree(desc);
    }
    return ret;
}
/************************************************************************
 函 数 名  : nv_verify_sha256_data
 功能描述  : SHA256计算
*************************************************************************/
u32 nv_verify_sha256_data(u8 *data, u32 len, u8 *output, u32 outlen)
{
    u32 i;
    u32 ret;
    u32 block = NVM_HASH_BLOCK_LEN;
    u8 hash[SHA256_DIGEST_LENGTH];
    u8 hash1[SHA256_DIGEST_LENGTH] = {0};

    u8 *data_buff = data;

    /* 计算data hash */
    while (len) {
        if (len < block) {
            block = len;
        }

        ret = nv_hash_sha256_calc(data_buff, block, hash);
        if (ret) {
            nv_record("nv_hash_calc fail!\n");
            return NV_ERROR;
        }

        for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            hash1[i] ^= hash[i];
        }

        len -= block;
        data_buff += block;
    }

    (void)memcpy_s(output, outlen, hash1, sizeof(hash1));

    return NV_OK;
}

/************************************************************************
 函 数 名  : nv_verify_check_sec
 功能描述  : 给img不同的段签名校验
*************************************************************************/
u32 nv_verify_check_sec(s8 *path)
{
    u32 ret;

    u8 *data = NULL;
    u8 *rd_sha256 = NULL;
    u8 hash[SHA256_DIGEST_LENGTH] = {0};

    FILE *fp = NULL;
    u32 length;

    fp = nv_file_open((s8 *)path, (s8 *)NV_FILE_READ);
    if (fp == NULL) {
        nv_record("open %s file failed!\n", (s8 *)path);
        return BSP_ERR_NV_NO_FILE;
    }

    length = nv_get_file_len(fp);
    if (length <= (SHA256_DIGEST_LENGTH + MAX_DIGEST_SIZE)) {
        nv_record("open %s file len %d invalid!\n", (s8 *)path, length);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto error;
    }

    data = vmalloc(length);
    if (data == NULL) {
        nv_record("malloc path %s fail!\n", path);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto error;
    }

    ret = (u32)nv_file_read((u8 *)data, 1, length, fp);
    if (length != ret) {
        nv_record("read file fail!\n");
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto error;
    }

    ret = nv_verify_sha256_data(data, length - SHA256_DIGEST_LENGTH - MAX_DIGEST_SIZE, hash, MAX_DIGEST_SIZE);
    if (ret) {
        nv_record("calc sha256 fail!\n");
        goto error;
    }

    rd_sha256 = data + (length - SHA256_DIGEST_LENGTH - MAX_DIGEST_SIZE);
    if (strncmp((char *)rd_sha256, (char *)hash, MAX_DIGEST_SIZE)) {
        nv_record("chek sha256 fail!\n");
        ret = BSP_ERR_NV_CRC_CTRL_ERR;
        goto error;
    }

error:
    (void)nv_file_close(fp);
    if (data != NULL)
        vfree(data);

    return ret;
}

