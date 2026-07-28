
#include <securec.h>
#include <bsp_nvim.h>
#include "nv_debug.h"
#include "nv_verify.h"
#include <crypto/hash.h>
#include "nv_ctrl.h"

#define THIS_MODU mod_nv


/************************************************************************
 函 数 名  : nv_hash_sha256_calc
 功能描述  : 计算sha256 hash值
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*************************************************************************/
u32 nv_hash_sha256_calc(const u8 *data, u32 len, u8 *hash)
{
    u32 ret = NV_OK;
    struct crypto_shash *tfm;
    struct shash_desc *desc;
    size_t desc_size;
    int flag;

    tfm = crypto_alloc_shash("sha256", 0, 0);
    if(NULL == tfm)
    {
        nv_record("crypto_alloc_shash failed!\n");
        return NV_ERROR;
    }

    desc_size = crypto_shash_descsize(tfm) + sizeof(struct shash_desc);
    desc = (struct shash_desc *)vmalloc(desc_size);
    if(NULL == desc)
    {
        nv_record("vmalloc desc failed!\n");
        crypto_free_shash(tfm);
        return NV_ERROR;
    }

    desc->tfm   = tfm;
    desc->flags = 0;

    flag = crypto_shash_init(desc);
    if (flag < 0)
    {
        nv_record("crypto_shash_init failed!ret=0x%x\n", ret);
        goto error;
    }

    ret = crypto_shash_digest(desc, data, len, hash);
    if (ret)
    {
        nv_record("crypto_shash_update failed!ret=0x%x \n", ret);
        goto error;
    }

error:
    if (desc)
    {
        if (desc->tfm)
            crypto_free_shash(desc->tfm);

        vfree(desc);
    }
    return ret;
}
/************************************************************************
 函 数 名  : nv_verify_sha256_data
 功能描述  : SHA256计算
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*************************************************************************/
u32 nv_verify_sha256_data(u8 *data, u32 len, u8 *output, u32 outlen)
{
    u32 i;
    u32 ret = NV_OK;
    u32 block = NVM_HASH_BLOCK_LEN;
    u8 hash[SHA256_DIGEST_LENGTH];
    u8 hash1[SHA256_DIGEST_LENGTH] = {0};

    u8 * data_buff = data;

    /* 计算data hash */
    while(len)
    {
        if(len < block)
        {
            block = len;
        }

        ret = nv_hash_sha256_calc(data_buff, block, hash);
        if(ret)
        {
            nv_record("nv_hash_calc fail!\n");
            return NV_ERROR;
        }

        for(i=0;i<SHA256_DIGEST_LENGTH;i++)
        {
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
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*************************************************************************/
u32 nv_verify_check_sec(s8 * path)
{
    u32 ret;

    u8 * data = NULL;
    u8 * rd_sha256;
    u8 hash[SHA256_DIGEST_LENGTH];

    FILE * fp;
    u32 length;
    
    if(nv_file_access((s8 *)path, 0))
    {
        return BSP_ERR_NV_NO_FILE;
    }

    fp = nv_file_open((s8 *)path, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("open %s file failed!\n", (s8 *)path);
        return BSP_ERR_NV_NO_FILE;
    }

    length = nv_get_file_len(fp);
    if(length <= 2 *MAX_DIGEST_SIZE)
    {
         nv_record("open %s file len %d invalid!\n", (s8 *)path, length);
         ret = BSP_ERR_NV_FILE_ERROR;
         goto error;
    }

    data = vmalloc(length);
    if (NULL == data)
    {
        nv_record("malloc path %s fail!\n", path);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto error;
    }

    ret = (u32)nv_file_read((u8*)data, 1, length, fp);
    if (length != ret)
    {
        nv_record("read file fail!\n");
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto error;
    }

    ret= nv_verify_sha256_data(data, length - 2 * MAX_DIGEST_SIZE, hash, MAX_DIGEST_SIZE);
    if(ret)
    {
        nv_record("calc sha256 fail!\n");
        goto error;
    }

    rd_sha256 = data + (length - 2 *MAX_DIGEST_SIZE);
    if(strncmp((char *)rd_sha256, (char *)hash, MAX_DIGEST_SIZE))
    {
        nv_record("chek sha256 fail!\n");
        ret = BSP_ERR_NV_CRC_CTRL_ERR;
        goto error;
    }

error:
    (void)nv_file_close(fp);
    if (NULL != data)
        vfree(data);

    return ret;
}


