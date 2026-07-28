/****************************************************************************//**
 * @file   : /hisi_casdcert.c
 * @brief  :
 * @par    : Copyright(c) 2018-2034, HUAWEI Technology Co., Ltd.
 * @date   : 2018/05/05
 * @author : c00370558
 * @note   :
********************************************************************************/
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <crypto/hash.h>
#include "hisi_hisee.h"
#include "hisi_hisee_fs.h"
#include "hisi_hisee_chip_test.h"
#include "hisi_casdcert.h"

/*===============================================================================
 *                                global objects                               *
===============================================================================*/
extern hisee_at_type g_at_cmd_type;

/**
 * @brief  whether this platform support casd
 * Only when dts has "hisi_support_casd", set this true
*/
static bool g_support_casd = false;

/**
 * @brief  whether should bypass write casd
 * Only when have recived CASD AT cmd, donot bypass write casd.
*/
static bool g_bypass_casd = true;

/*===============================================================================
 *                                  functions                                  *
===============================================================================*/
/****************************************************************************//**
 * @brief      : set_support_casd
 * @param[in]  : flag , true or flase
 * @return     : void
 * @note       :
********************************************************************************/
void set_support_casd(bool flag)
{
    g_support_casd = flag;
}

/****************************************************************************//**
 * @brief      : get_support_casd
 * @param[in]  :
 * @return     : ::bool
 * @note       : interface for get whether support casd
********************************************************************************/
bool get_support_casd(void)
{
    return g_support_casd;
}

/****************************************************************************//**
 * @brief      : set_bypass_casd
 * @param[in]  : flag , true or flase
 * @return     : void
 * @note       :
********************************************************************************/
void set_bypass_casd(bool flag)
{
    g_bypass_casd = flag;
}

/****************************************************************************//**
 * @brief      : get_bypass_casd
 * @param[in]  :
 * @return     : ::bool
 * @note       : interface for get whether bypass casd
********************************************************************************/
bool get_bypass_casd(void)
{
    return g_bypass_casd;
}



/****************************************************************************//**
 * @brief      : casd_atoi_u16
 * @param[in]  : str
 * @param[out]  : len , strlen
 * @return     : ::unsigned int
 * @note       : trans from str to u16
********************************************************************************/
static unsigned int casd_atoi_u16(const char *str, unsigned int *len)
{
    unsigned long value = 0;
    char *endp = NULL;    /*end of the parsed string*/

    if (NULL == str || NULL == len) {
        return CASD_U16_MAX_VALUE;
    }

    value = simple_strtoul(str, &endp, HISI_DEC_BASE);
    *len = (unsigned int)(endp - str);

    if (0 == *len) {
        return CASD_U16_MAX_VALUE;
    }

    return value > CASD_U16_MAX_VALUE ? CASD_U16_MAX_VALUE : (unsigned int)value;
}

/****************************************************************************//**
 * @brief      : casd_parse_at_cmd
 * @param[in]  : buf
 * @param[out]  : curr_pack , <Index>
 * @param[out]  : total_packs , <Total>
 * @param[out]  : header_len , strlen before real data
 * @return     : ::int
 * @note       : ^HISEE=<Action>,<Index>,<Total>,<Data>
********************************************************************************/
static int casd_parse_at_cmd(const char *buf, unsigned int *curr_pack,
                 unsigned int *total_packs, unsigned int *header_len)
{
    const char *p = buf;
    unsigned int len;

    if (NULL == buf || NULL == curr_pack || NULL == total_packs ||
        NULL == header_len) {
        return HISEE_ERROR;
    }

    /* Parse <Index> */
    if (*(p++) != ',') {
        return HISEE_ERROR;
    }
    *curr_pack = casd_atoi_u16(p, &len);
    if (CASD_U16_MAX_VALUE <= *curr_pack) {
        return HISEE_ERROR;
    }
    p += len;
    /* Parse <Total> */
    if (*(p++) != ',') {
        return HISEE_ERROR;
    }
    *total_packs = casd_atoi_u16(p, &len);
    if (CASD_U16_MAX_VALUE <= *total_packs) {
        return HISEE_ERROR;
    }
    p += len;
    /* Prepare parse <Data> */
    if (*(p++) != ',') {
        return HISEE_ERROR;
    }
    *header_len = (unsigned int)(p - buf);

    return HISEE_OK;
}


/****************************************************************************//**
 * @brief      : casd_sha256
 * @param[in]  : message
 * @param[in]  : len
 * @param[out]  : out
 * @return     : ::int
 * @note       : calculate sha256 hash value
********************************************************************************/
int casd_sha256(unsigned char *message, int len, unsigned char *out)
{
    struct crypto_ahash *tfm;
    struct scatterlist sg;
    struct ahash_request *req;

    if (NULL == message || NULL == out) {
        return HISEE_ERROR;
    }

    tfm = crypto_alloc_ahash("sha256", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(tfm)) {
        pr_err("%s() failed to load transform for sha256\n", __func__);
        return HISEE_ERROR;
    }

    req = ahash_request_alloc(tfm, GFP_ATOMIC);
    if (!req) {
        crypto_free_ahash(tfm);
        pr_err("%s() failed to request for sha256\n", __func__);
        return HISEE_ERROR;
    }

    sg_init_one(&sg, message, len);

    ahash_request_set_callback(req, 0, NULL, NULL);
    ahash_request_set_crypt(req, &sg, out, len);

    if (crypto_ahash_digest(req)) {
        pr_err("%s() failed to calculate sha256\n", __func__);
        ahash_request_free(req);
        crypto_free_ahash(tfm);
        return HISEE_ERROR;
    }

    ahash_request_free(req);
    crypto_free_ahash(tfm);
    return HISEE_OK;
}



/****************************************************************************//**
 * @brief      : casd_check_hash
 * @param[in]  : buf , string hash value from AT cmd
 * @param[in]  : compare_hash , compare with this
 * @return     : ::int
 * @note       :
********************************************************************************/
static int casd_check_hash(const char *buf, const char *compare_hash)
{
    size_t buf_size; /*data str len*/
    unsigned char local_hash[HISI_256HASH_BYTE_LEN]; /*hash value translated from buf */

    if (NULL == buf || NULL == compare_hash) {
        pr_err("%s buf paramters is null\n", __func__);
        return HISEE_INVALID_PARAMS;
    }

    buf_size = strnlen(buf, HISEE_IOCTL_CMD_MAX_LEN);
    if (HISI_256HASH_STR_LEN != buf_size) {
        pr_err("256HASH len %d error!\n", (int)buf_size);
        return HISI_CASD_LENGTH_ERROR;
    }

    /* chk_hash: translate str hash value to bin */
    if (0 != hex2bin(local_hash, buf, buf_size/HEX_STR_LEN)) {
        pr_err("%s hex2bin error!\n", __func__);
        return HISI_CASD_DATA_PARAM_ERROR;
    }

    if (0 != strncmp(local_hash, compare_hash, HISI_256HASH_BYTE_LEN)) {
        pr_err("HASH check failed, l_hash:%2x %2x .., c_hash:%2x %2x ..\n",
            local_hash[0], local_hash[1], compare_hash[0], compare_hash[1]);
        return HISI_CASD_HASH_ERROR;
    }

    return HISEE_OK;
}

/****************************************************************************//**
 * @brief      : casd_prepare_receive
 * @param[in]  : curr_pack
 * @param[in]  : total_packs
 * @return     : ::int
 * @note       :
********************************************************************************/
static int casd_prepare_receive(unsigned int curr_pack, unsigned int total_packs)
{
    /* first pack, malloc space */
    if (0 == curr_pack) {
        if (0 == total_packs) {
            pr_err("%s para err! total_packs cannot be 0.\n", __func__);
            return HISI_CASD_AT_HEADER_ERROR;
        }

        if (NULL == g_hisee_data.casd_data.buffer) {
            g_hisee_data.casd_data.buffer = (void *)kzalloc(HISI_CASD_RECV_SIZE, GFP_KERNEL);
            if (NULL == g_hisee_data.casd_data.buffer) {
                pr_err("%s(): kmalloc failed\n", __func__);
                return HISEE_NO_RESOURCES;
            }
        }
        g_hisee_data.casd_data.total_packs = total_packs;
        g_hisee_data.casd_data.received_packs = 0;
        g_hisee_data.casd_data.size = 0;
    } else if (curr_pack > total_packs || curr_pack != g_hisee_data.casd_data.received_packs ||
                total_packs != g_hisee_data.casd_data.total_packs) {
        return HISI_CASD_AT_MISMATCH_ERROR;
    }

    return HISEE_OK;
}

/****************************************************************************//**
 * @brief      : casd_process_receive
 * @param[in]  : buf
 * @param[in]  : curr_pack
 * @param[in]  : total_packs
 * @return     : ::int
 * @note       :
********************************************************************************/
static int casd_process_receive(const char *buf, unsigned int curr_pack, unsigned int total_packs)
{
    int ret = HISEE_OK;
    unsigned int data_str_size; /*data str len in each pack */
    unsigned char cal_hash[HISI_256HASH_BYTE_LEN]; /*hash value calculated from local */

    if (NULL == buf) {
        pr_err("%s buf paramters is null\n", __func__);
        set_errno_and_return(HISEE_INVALID_PARAMS);
    }

    data_str_size = strnlen(buf, HISEE_IOCTL_CMD_MAX_LEN);

    if (0 == data_str_size) {
        pr_err("%s buf data is empty\n", __func__);
        set_errno_and_return(HISI_CASD_DATA_PARAM_ERROR);
    }

    if (curr_pack < total_packs ) {  /* save casd data */
        if (g_hisee_data.casd_data.size + data_str_size > HISI_CASD_RECV_SIZE) {
            pr_err("%s(): %d > max size.\n", __func__, g_hisee_data.casd_data.size + data_str_size);
            return HISI_CASD_LENGTH_ERROR;
        }

        memcpy(g_hisee_data.casd_data.buffer + g_hisee_data.casd_data.size, buf, data_str_size);
        g_hisee_data.casd_data.size += data_str_size;
        g_hisee_data.casd_data.received_packs++;

    } else { /* compare hash */
        /* check data str size */
        if (g_hisee_data.casd_data.size%HEX_STR_LEN) {
            pr_err("%s(): data str size %d not even.\n", __func__, g_hisee_data.casd_data.size);
            return HISI_CASD_DATA_PARAM_ERROR;
        }

        /* cal_hash: calculate hash of all received casd str data */
        if (HISEE_OK != casd_sha256(g_hisee_data.casd_data.buffer, g_hisee_data.casd_data.size, cal_hash)) {
            return HISI_CASD_ERROR;
        }

        /* check_hash: compare hash from buf and cal_hash */
        ret = casd_check_hash(buf, cal_hash);
        if (HISEE_OK != ret) {
            return ret;
        }

        /* casd str data hash check ok, then translate to bin and save in file */
        if (0 != hex2bin(g_hisee_data.casd_data.buffer, g_hisee_data.casd_data.buffer,
                    g_hisee_data.casd_data.size/HEX_STR_LEN)) {
            pr_err("%s hex2bin error!\n", __func__);
            return HISI_CASD_DATA_PARAM_ERROR;
        }

        g_hisee_data.casd_data.size /= HEX_STR_LEN;
        ret = hisee_write_file(HISEE_CASD_IMG_FULLNAME, g_hisee_data.casd_data.buffer, g_hisee_data.casd_data.size);
    }

    return ret;
}


/****************************************************************************//**
 * @brief      : hisi_receive_casd_func
 * @param[in]  : buf , command str
 * @param[in]  : para, not use
 * @return     : ::int
 * @note       : AT^HISEE=CASD function
********************************************************************************/
int hisi_receive_casd_func(void * buf, int para)
{
    int ret = HISEE_OK;
    unsigned int total_packs = 0;
    unsigned int curr_pack = 0;
    unsigned int header_len = 0;

    g_at_cmd_type = HISEE_AT_CASD;
    /* 只要接收过CASD AT命令，就不bypass写CASD */
    set_bypass_casd(false);

    if (NULL == buf) {
        pr_err("%s buf paramters is null\n", __func__);
        set_errno_and_return(HISEE_INVALID_PARAMS);
    }

    /* parse header for pack message */
    if (HISEE_OK != casd_parse_at_cmd(buf, &curr_pack, &total_packs, &header_len)) {
        pr_err("hisee_parse_at_casd_cmd fail! buf = %s\n", (char *)buf);
        set_errno_and_return(HISI_CASD_AT_HEADER_ERROR);
    }

    mutex_lock(&g_hisee_data.hisee_mutex);
    g_hisee_data.casd_data.curr_pack = curr_pack;

    /* prepare for casd data */
    ret = casd_prepare_receive(curr_pack, total_packs);
    if (HISEE_OK != ret) {
        goto end_process;
    }
    buf += header_len;

    /* process received casd data */
    ret = casd_process_receive(buf, curr_pack, total_packs);

end_process:
    /* if the last pack, need to free memory */
    if(curr_pack == total_packs) {
        if(NULL != g_hisee_data.casd_data.buffer) {
            memset(g_hisee_data.casd_data.buffer, 0, HISI_CASD_RECV_SIZE);
            kfree(g_hisee_data.casd_data.buffer);
            g_hisee_data.casd_data.buffer = NULL;
            g_hisee_data.casd_data.received_packs = 0;
            g_hisee_data.casd_data.total_packs = 0;
            g_hisee_data.casd_data.size = 0;
        }
    }
    mutex_unlock(&g_hisee_data.hisee_mutex);
    if(HISEE_OK != ret) {
        pr_err("%s() fail! ret = %d\n", __func__, ret);
    }
    set_errno_and_return(ret);
}

/****************************************************************************//**
 * @brief      : hisi_verify_casd_func
 * @param[in]  : buf , command str
 * @param[in]  : para, not use
 * @return     : ::int
 * @note       : AT^HISEE=VERIFYCASD function
********************************************************************************/
int hisi_verify_casd_func(void * buf, int para)
{
    int ret = HISEE_OK;
    unsigned int total_packs = 0;
    unsigned int curr_pack = 0;
    unsigned int header_len = 0;
    unsigned char cal_hash[HISI_256HASH_BYTE_LEN]; /*hash value calculated from local */
    char *file_buff = NULL;
    size_t casd_size;

    g_at_cmd_type = HISEE_AT_VERIFYCASD;
    /* 只要接收过CASD AT命令，就不bypass写CASD */
    set_bypass_casd(false);

    if (NULL == buf) {
        pr_err("%s buf paramters is null\n", __func__);
        set_errno_and_return(HISEE_INVALID_PARAMS);
    }

    if (HISEE_OK != casd_parse_at_cmd(buf, &curr_pack, &total_packs, &header_len)) {
        pr_err("hisee_parse_at_casd_cmd fail! buf = %s\n", (char *)buf);
        set_errno_and_return(HISI_CASD_AT_HEADER_ERROR);
    }
    g_hisee_data.casd_data.curr_pack = curr_pack;

    /* 1 check at cmd para, fixed value <0,1> */
    if (0 != curr_pack || HISI_VERIFY_CASD_TOTAL_PACK != total_packs) {
        pr_err("%s para err! <%d,%d> not <0,1>\n", __func__, curr_pack, total_packs);
        set_errno_and_return(HISI_CASD_AT_HEADER_ERROR);
    }
    buf += header_len;

    /* 2 malloc file_buff */
    file_buff = (void *)kzalloc(HISI_CASD_FILE_SIZE, GFP_KERNEL);
    if (NULL == file_buff) {
        pr_err("%s(): kmalloc failed\n", __func__);
        set_errno_and_return(HISEE_NO_RESOURCES);
    }

    /* 3 read given file to file_buff */
    ret = filesys_read_img_from_file(HISEE_CASD_IMG_FULLNAME, file_buff, &casd_size, HISI_CASD_FILE_SIZE);
    if (HISEE_OK != ret) {
        pr_err("filesys_read_flash_file %s failed.\n", HISEE_CASD_IMG_FULLNAME);
        goto end_process;
    }

    /* 4 cal file hash */
    if(HISEE_OK != casd_sha256(file_buff, casd_size, cal_hash)) {
        ret = HISI_CASD_ERROR;
        goto end_process;
    }

    /* 5 compare hash */
    ret = casd_check_hash(buf, cal_hash);

end_process:
    kfree(file_buff);

    if(HISEE_OK != ret) {
        pr_err("%s() fail! ret = %d\n", __func__, ret);
    }
    set_errno_and_return(ret);
}


