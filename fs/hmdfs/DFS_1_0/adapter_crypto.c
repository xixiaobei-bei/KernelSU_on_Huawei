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

#include "adapter_crypto.h"
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

int aeadcipher_encrypt_buffer(struct connection *con, __u8 *src_buf,
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
	req = aead_request_alloc(con->tfm, GFP_KERNEL);
	if (!req) {
		hmdfs_err("aead_request_alloc() failed");
		return -ENOMEM;
	}
	ret = set_aeadcipher(con->tfm, req, &result);
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

int aeadcipher_decrypt_buffer(struct connection *con, __u8 *src_buf,
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
	req = aead_request_alloc(con->tfm, GFP_KERNEL);
	if (!req) {
		hmdfs_err("aead_request_alloc() failed");
		return -ENOMEM;
	}
	ret = set_aeadcipher(con->tfm, req, &result);
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
