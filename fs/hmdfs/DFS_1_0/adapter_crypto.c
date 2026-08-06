/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_crypto.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: Crypto for fusion communication
 * Author: wangminmin4@huawei.com
 * Create: 2020-03-31
 *
 */

#include <linux/bug.h>
#include <linux/random.h>

#include "adapter_crypto.h"
#include "comm/transport.h"
#include "hmdfs.h"

#define ENCRYPT_FLAG 1
#define DECRYPT_FLAG 0

struct aeadcrypt_result {
	struct completion completion;
	int err;
};

static void aeadcipher_cb(struct crypto_async_request *req, int error)
{
	struct aeadcrypt_result *result = req->data;

	if (error == -EINPROGRESS)
		return;
	result->err = error;
	complete(&result->completion);
}

static int aeadcipher_en_de(struct aead_request *req,
			    struct aeadcrypt_result result, int flag)
{
	int rc = 0;

	if (flag)
		rc = crypto_aead_encrypt(req);
	else
		rc = crypto_aead_decrypt(req);
	switch (rc) {
	case 0:
		break;
	case -EINPROGRESS:
	case -EBUSY:
		rc = wait_for_completion_interruptible(&result.completion);
		if (!rc && !result.err)
			reinit_completion(&result.completion);
		break;
	default:
		hmdfs_err("returned rc %d result %d", rc, result.err);
		break;
	}
	return rc;
}

static int set_aeadcipher(struct crypto_aead *tfm, struct aead_request *req,
			  struct aeadcrypt_result *result)
{
	init_completion(&result->completion);
	aead_request_set_callback(
		req, CRYPTO_TFM_REQ_MAY_BACKLOG | CRYPTO_TFM_REQ_MAY_SLEEP,
		aeadcipher_cb, result);
	return 0;
}

int aeadcipher_encrypt_buffer(struct crypto_aead *tfm, __u8 *src_buf,
			      size_t src_len, __u8 *dst_buf, size_t dst_len)
{
	int ret = 0;
	struct scatterlist src, dst;
	struct aead_request *req = NULL;
	struct aeadcrypt_result result;
	__u8 cipher_iv[HMDFS_IV_SIZE];

	if (src_len <= 0)
		return -EINVAL;
	if (!virt_addr_valid(src_buf) || !virt_addr_valid(dst_buf)) {
		WARN_ON(1);
		hmdfs_err("encrypt address is invalid");
		return -EPERM;
	}

	get_random_bytes(cipher_iv, HMDFS_IV_SIZE);
	memcpy(dst_buf, cipher_iv, HMDFS_IV_SIZE);
	req = aead_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		hmdfs_err("aead_request_alloc() failed");
		return -ENOMEM;
	}
	ret = set_aeadcipher(tfm, req, &result);
	if (ret) {
		hmdfs_err("set_enaeadcipher exit fault");
		goto out;
	}

	sg_init_one(&src, src_buf, src_len);
	sg_init_one(&dst, dst_buf + HMDFS_IV_SIZE, dst_len - HMDFS_IV_SIZE);
	aead_request_set_crypt(req, &src, &dst, src_len, cipher_iv);
	aead_request_set_ad(req, 0);
	ret = aeadcipher_en_de(req, result, ENCRYPT_FLAG);
out:
	aead_request_free(req);
	return ret;
}

int aeadcipher_decrypt_buffer(struct crypto_aead *tfm, __u8 *src_buf,
			      size_t src_len, __u8 *dst_buf, size_t dst_len)
{
	int ret = 0;
	struct scatterlist src, dst;
	struct aead_request *req = NULL;
	struct aeadcrypt_result result;
	__u8 cipher_iv[HMDFS_IV_SIZE];

	if (src_len <= HMDFS_IV_SIZE + HMDFS_TAG_SIZE)
		return -EINVAL;
	if (!virt_addr_valid(src_buf) || !virt_addr_valid(dst_buf)) {
		WARN_ON(1);
		hmdfs_err("decrypt address is invalid");
		return -EPERM;
	}

	memcpy(cipher_iv, src_buf, HMDFS_IV_SIZE);
	req = aead_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		hmdfs_err("aead_request_alloc() failed");
		return -ENOMEM;
	}
	ret = set_aeadcipher(tfm, req, &result);
	if (ret) {
		hmdfs_err("set_deaeadcipher exit fault");
		goto out;
	}

	sg_init_one(&src, src_buf + HMDFS_IV_SIZE, src_len - HMDFS_IV_SIZE);
	sg_init_one(&dst, dst_buf, dst_len);
	aead_request_set_crypt(req, &src, &dst, src_len - HMDFS_IV_SIZE,
			       cipher_iv);
	aead_request_set_ad(req, 0);
	ret = aeadcipher_en_de(req, result, DECRYPT_FLAG);
out:
	aead_request_free(req);
	return ret;
}

static int set_tfm(__u8 *master_key, struct crypto_aead *tfm)
{
	int ret = 0;
	int iv_len;
	__u8 *sec_key = NULL;

	sec_key = master_key;
	crypto_aead_clear_flags(tfm, ~0);
	ret = crypto_aead_setkey(tfm, sec_key, HMDFS_KEY_SIZE);
	if (ret) {
		hmdfs_err("failed to set the key");
		goto out;
	}
	ret = crypto_aead_setauthsize(tfm, HMDFS_TAG_SIZE);
	if (ret) {
		hmdfs_err("authsize length is error");
		goto out;
	}

	iv_len = crypto_aead_ivsize(tfm);
	if (iv_len != HMDFS_IV_SIZE) {
		hmdfs_err("IV recommended value should be set %d", iv_len);
		ret = -ENODATA;
	}
out:
	return ret;
}

struct crypto_aead *hmdfs_alloc_aead(uint8_t *master_key, size_t len)
{
	int err = 0;
	struct crypto_aead *tfm = NULL;

	BUG_ON(len != HMDFS_KEY_SIZE);

	tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		hmdfs_err("failed to alloc gcm(aes): %ld", PTR_ERR(tfm));
		return NULL;
	}

	err = set_tfm(master_key, tfm);
	if (err) {
		hmdfs_err("failed to set key to tfm, exit fault: %d", err);
		goto out;
	}

	return tfm;

out:
	crypto_free_aead(tfm);
	return NULL;
}
