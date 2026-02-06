/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: implement hisee casd certification function
 * Create: 2018-05-05
 */
#include "hisee_casdcert.h"
#include <crypto/hash.h>
#include <linux/atomic.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <securec.h>
#include "hisee.h"
#include "hisee_chip_test.h"
#include "hisee_fs.h"

/*
 * @brief  whether this platform support casd
 * Only when dts has "hisi_support_casd", set this true
 */
static bool g_support_casd;

/*
 * @brief  whether should bypass write casd
 *         Only when have recived CASD AT cmd, donot bypass write casd.
 */
static bool g_bypass_casd = true;

/*
 * @brief      : set_support_casd
 * @param[in]  : flag , true or flase
 * @return     : void
 */
void set_support_casd(bool flag)
{
	g_support_casd = flag;
}

/*
 * @brief      : get_support_casd
 * @return     : ::bool
 * @note       : interface for get whether support casd
 */
bool get_support_casd(void)
{
	return g_support_casd;
}

/*
 * @brief      : set_bypass_casd
 * @param[in]  : flag , true or flase
 * @return     : void
 */
void set_bypass_casd(bool flag)
{
	g_bypass_casd = flag;
}

/*
 * @brief      : interface for get whether bypass casd
 * @return     : ::bool
 */
bool get_bypass_casd(void)
{
	return g_bypass_casd;
}

/*
 * @brief      : trans from str to u16
 * @param[in]  : str , string buffer
 * @param[out] : len , string length
 * @return     : ::unsigned int
 */
static unsigned int casd_atoi_u16(const char *str, unsigned int *len)
{
	unsigned long value;
	char *endp = NULL; /* end of the parsed string */

	if (!str || !len)
		return CASD_U16_MAX_VALUE;

	value = simple_strtoul(str, &endp, HISI_DEC_BASE);
	*len = (unsigned int)(endp - str);

	if (*len == 0)
		return CASD_U16_MAX_VALUE;

	return (value > CASD_U16_MAX_VALUE) ? CASD_U16_MAX_VALUE : (unsigned int)value;
}

/*
 * @brief      : casd_parse_at_cmd
 * @param[in]  : buf , cmd buffer
 * @param[out]  : curr_pack , <Index>
 * @param[out]  : total_packs , <Total>
 * @param[out]  : header_len , strlen before real data
 * @return     : ::int
 * @note       : ^HISEE=<Action>,<Index>,<Total>,<Data>
 */
static int casd_parse_at_cmd(const char *buf, unsigned int *curr_pack,
			     unsigned int *total_packs, unsigned int *header_len)
{
	const char *p = buf;
	unsigned int len;

	if (!buf || !curr_pack || !total_packs || !header_len)
		return HISEE_ERROR;

	/* Parse <Index> */
	if (*(p++) != ',')
		return HISEE_ERROR;

	*curr_pack = casd_atoi_u16(p, &len);
	if (*curr_pack >= CASD_U16_MAX_VALUE)
		return HISEE_ERROR;

	p += len;
	/* Parse <Total> */
	if (*(p++) != ',')
		return HISEE_ERROR;

	*total_packs = casd_atoi_u16(p, &len);
	if (*total_packs >= CASD_U16_MAX_VALUE)
		return HISEE_ERROR;

	p += len;
	/* Prepare parse <Data> */
	if (*(p++) != ',')
		return HISEE_ERROR;

	*header_len = (unsigned int)(p - buf);

	return HISEE_OK;
}

/*
 * @brief      : calculate sha256 hash value
 * @param[in]  : message , input message
 * @param[in]  : len , message len
 * @param[out] : out , sha256 result buffer
 * @return     : ::int
 */
int casd_sha256(unsigned char *message, int len, unsigned char *out)
{
	struct crypto_ahash *tfm = NULL;
	struct scatterlist sg = {0};
	struct ahash_request *req = NULL;

	if (!message || !out)
		return HISEE_ERROR;

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

/*
 * @brief      : casd_check_hash
 * @param[in]  : buf , string hash value from AT cmd
 * @param[in]  : compare_hash , compare with this
 * @return     : ::int
 */
static int casd_check_hash(const char *buf, const char *compare_hash)
{
	size_t buf_size; /* data str len */
	/* hash value translated from buf */
	unsigned char local_hash[HISI_256HASH_BYTE_LEN] = {0};

	if (!buf || !compare_hash) {
		pr_err("%s buf parameters is null\n", __func__);
		return HISEE_INVALID_PARAMS;
	}

	buf_size = strnlen(buf, HISEE_IOCTL_CMD_MAX_LEN);
	if (buf_size != HISI_256HASH_STR_LEN) {
		pr_err("256HASH len %d error!\n", (int)buf_size);
		return HISI_CASD_LENGTH_ERROR;
	}

	/* chk_hash: translate str hash value to bin */
	if (hex2bin(local_hash, buf, buf_size / HEX_STR_LEN) != 0) {
		pr_err("%s hex2bin error!\n", __func__);
		return HISI_CASD_DATA_PARAM_ERROR;
	}

	if (strncmp(local_hash, compare_hash, HISI_256HASH_BYTE_LEN) != 0) {
		pr_err("HASH check failed, l_hash:%2x %2x .., c_hash:%2x %2x ..\n",
		       local_hash[0], local_hash[1], compare_hash[0], compare_hash[1]);
		return HISI_CASD_HASH_ERROR;
	}

	return HISEE_OK;
}

/*
 * @brief      : casd_prepare_receive
 * @param[in]  : curr_pack , current package index
 * @param[in]  : total_packs , total package number
 * @return     : ::int
 */
static int casd_prepare_receive(unsigned int curr_pack, unsigned int total_packs)
{
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	/* first pack, malloc space */
	if (curr_pack == 0) {
		if (total_packs == 0) {
			pr_err("%s para err! total_packs cannot be 0.\n",
			       __func__);
			return HISI_CASD_AT_HEADER_ERROR;
		}

		if (!hisee_data_ptr->casd_data.buffer) {
			hisee_data_ptr->casd_data.buffer =
				kzalloc(HISI_CASD_RECV_SIZE, GFP_KERNEL);
			if (!hisee_data_ptr->casd_data.buffer) {
				pr_err("%s(): kmalloc failed\n", __func__);
				return HISEE_NO_RESOURCES;
			}
		}
		hisee_data_ptr->casd_data.total_packs = total_packs;
		hisee_data_ptr->casd_data.received_packs = 0;
		hisee_data_ptr->casd_data.size = 0;
	} else if (curr_pack > total_packs ||
		   curr_pack != hisee_data_ptr->casd_data.received_packs ||
		   total_packs != hisee_data_ptr->casd_data.total_packs) {
		return HISI_CASD_AT_MISMATCH_ERROR;
	}

	return HISEE_OK;
}

static int _receive_intermediate_packet(const char *buf)
{
	int ret;
	unsigned int data_str_size; /* data str len in each pack */
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	data_str_size = strnlen(buf, HISEE_IOCTL_CMD_MAX_LEN);
	if (data_str_size == 0) {
		pr_err("%s buf data is empty\n", __func__);
		return HISI_CASD_DATA_PARAM_ERROR;
	}

	if (hisee_data_ptr->casd_data.size + data_str_size > HISI_CASD_RECV_SIZE) {
		pr_err("%s(): %u > max size.\n", __func__,
		       hisee_data_ptr->casd_data.size + data_str_size);
		return HISI_CASD_LENGTH_ERROR;
	}

	ret = memcpy_s(hisee_data_ptr->casd_data.buffer + hisee_data_ptr->casd_data.size,
		       HISI_CASD_RECV_SIZE - hisee_data_ptr->casd_data.size,
		       buf, data_str_size);
	if (ret != EOK) {
		pr_err("%s(): memcpy err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	hisee_data_ptr->casd_data.size += data_str_size;
	hisee_data_ptr->casd_data.received_packs++;

	return HISEE_OK;
}

static int _receive_last_packet(const char *buf)
{
	int ret;
	/* hash value calculated from local */
	unsigned char cal_hash[HISI_256HASH_BYTE_LEN] = {0};
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	/* check data str size */
	if (hisee_data_ptr->casd_data.size % HEX_STR_LEN) {
		pr_err("%s(): data str size %u not even.\n",
		       __func__, hisee_data_ptr->casd_data.size);
		return HISI_CASD_DATA_PARAM_ERROR;
	}

	/* cal_hash: calculate hash of all received casd str data */
	if (casd_sha256(hisee_data_ptr->casd_data.buffer,
			hisee_data_ptr->casd_data.size,
			cal_hash) != HISEE_OK)
		return HISI_CASD_ERROR;

	/* check_hash: compare hash from buf and cal_hash */
	ret = casd_check_hash(buf, cal_hash);
	if (ret != HISEE_OK)
		return ret;

	/* casd str data hash check ok, then translate to bin and save in file */
	if (hex2bin(hisee_data_ptr->casd_data.buffer,
		    hisee_data_ptr->casd_data.buffer,
		    hisee_data_ptr->casd_data.size / HEX_STR_LEN) != 0) {
		pr_err("%s hex2bin error!\n", __func__);
		return HISI_CASD_DATA_PARAM_ERROR;
	}

	hisee_data_ptr->casd_data.size /= HEX_STR_LEN;
	ret = hisee_write_file(HISEE_CASD_IMG_FULLNAME,
			       hisee_data_ptr->casd_data.buffer,
			       hisee_data_ptr->casd_data.size);
	return ret;
}

/*
 * @brief      : casd_process_receive
 * @param[in]  : buf , casd certification data
 * @param[in]  : curr_pack , current package index
 * @param[in]  : total_packs , total package number
 * @return     : ::int
 */
static int casd_process_receive(const char *buf, unsigned int curr_pack,
				unsigned int total_packs)
{
	int ret;

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return HISEE_INVALID_PARAMS;
	}

	if (curr_pack < total_packs)
		ret = _receive_intermediate_packet(buf); /* save casd data */
	else
		ret = _receive_last_packet(buf); /* compare hash */

	return ret;
}

int hisi_receive_casd_func(const void *buf, int para)
{
	int ret;
	unsigned int total_packs = 0;
	unsigned int curr_pack = 0;
	unsigned int header_len = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	hisee_set_at_type(HISEE_AT_CASD);
	/* once received CASD AT command, then don't bypass writing CASD function */
	set_bypass_casd(false);

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	/* parse header for pack message */
	if (casd_parse_at_cmd(buf, &curr_pack,
			      &total_packs, &header_len) != HISEE_OK) {
		pr_err("hisee_parse_at_casd_cmd fail! buf = %s\n", (char *)buf);
		return set_errno_then_exit(HISI_CASD_AT_HEADER_ERROR);
	}

	mutex_lock(&hisee_data_ptr->hisee_mutex);
	hisee_data_ptr->casd_data.curr_pack = curr_pack;

	/* prepare for casd data */
	ret = casd_prepare_receive(curr_pack, total_packs);
	if (ret != HISEE_OK)
		goto end_process;

	buf += header_len;

	/* process received casd data */
	ret = casd_process_receive(buf, curr_pack, total_packs);

end_process:
	/* if the last pack, need to free memory */
	if (curr_pack == total_packs) {
		if (hisee_data_ptr->casd_data.buffer) {
			(void)memset_s(hisee_data_ptr->casd_data.buffer,
				       HISI_CASD_RECV_SIZE,
				       0, HISI_CASD_RECV_SIZE);
			kfree(hisee_data_ptr->casd_data.buffer);
			hisee_data_ptr->casd_data.buffer = NULL;
			hisee_data_ptr->casd_data.received_packs = 0;
			hisee_data_ptr->casd_data.total_packs = 0;
			hisee_data_ptr->casd_data.size = 0;
		}
	}
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	if (ret != HISEE_OK)
		pr_err("%s() fail! ret = %d\n", __func__, ret);

	return set_errno_then_exit(ret);
}

static void hisee_set_casd_pack(unsigned int pack)
{
	struct hisee_module_data *hisee_data_ptr = get_hisee_data_ptr();

	if (!hisee_data_ptr) {
		pr_err("%s:get hisee data failed!\n", __func__);
		return;
	}

	mutex_lock(&hisee_data_ptr->hisee_mutex);
	hisee_data_ptr->casd_data.curr_pack = pack;
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
}

/*
 * @brief      : hisi_verify_casd_func
 * @param[in]  : buf , command str
 * @param[in]  : para, not use
 * @return     : ::int
 * @note       : AT^HISEE=VERIFYCASD function
 */
int hisi_verify_casd_func(const void *buf, int para)
{
	int ret;
	unsigned int total_packs = 0;
	unsigned int curr_pack = 0;
	unsigned int header_len = 0;
	/* hash value calculated from local */
	unsigned char cal_hash[HISI_256HASH_BYTE_LEN] = {0};
	char *file_buff = NULL;
	size_t casd_size;

	hisee_set_at_type(HISEE_AT_VERIFYCASD);
	/* once received CASD AT command, then don't bypass writing CASD function */
	set_bypass_casd(false);

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	if (casd_parse_at_cmd(buf, &curr_pack,
			      &total_packs, &header_len) != HISEE_OK) {
		pr_err("hisee_parse_at_casd_cmd fail! buf = %s\n", (char *)buf);
		return set_errno_then_exit(HISI_CASD_AT_HEADER_ERROR);
	}
	hisee_set_casd_pack(curr_pack);

	/* 1 check at cmd para, fixed value <0,1> */
	if (curr_pack != 0 || total_packs != HISI_VERIFY_CASD_TOTAL_PACK) {
		pr_err("%s para err! <%u,%u> not <0,1>\n",
		       __func__, curr_pack, total_packs);
		return set_errno_then_exit(HISI_CASD_AT_HEADER_ERROR);
	}
	buf += header_len;

	/* 2 malloc file_buff */
	file_buff = kzalloc(HISI_CASD_FILE_SIZE, GFP_KERNEL);
	if (!file_buff) {
		pr_err("%s(): kmalloc failed\n", __func__);
		return set_errno_then_exit(HISEE_NO_RESOURCES);
	}

	/* 3 read given file to file_buff */
	ret = filesys_read_img_from_file(HISEE_CASD_IMG_FULLNAME, file_buff,
					 &casd_size, HISI_CASD_FILE_SIZE);
	if (ret != HISEE_OK) {
		pr_err("filesys_read_flash_file %s failed.\n",
		       HISEE_CASD_IMG_FULLNAME);
		goto end_process;
	}

	/* 4 cal file hash */
	if (casd_sha256(file_buff, casd_size, cal_hash) != HISEE_OK) {
		ret = HISI_CASD_ERROR;
		goto end_process;
	}

	/* 5 compare hash */
	ret = casd_check_hash(buf, cal_hash);

end_process:
	kfree(file_buff);

	if (ret != HISEE_OK)
		pr_err("%s() fail! ret = %d\n", __func__, ret);

	return set_errno_then_exit(ret);
}
