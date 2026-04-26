/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              basic algorithm.
 * Create: 2020-06-16
 */

#include "inc/tee/base_alg.h"
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <securec.h>
#include <huawei_platform/hwdps/hwdps_error.h>
#include "inc/base/hwdps_utils.h"

#define HWDPS_CBC_AES_ALG "cbc(aes)"
#define HWDPS_HKDF_HMAC_ALG "hmac(sha256)"
#define HWDPS_HKDF_HASHLEN SHA256_DIGEST_SIZE
#define HWDPS_HMAC_TAG_LEN SHA256_DIGEST_SIZE
#define HWDPS_CBC_AES_MAX_INPUT 512
#define HWDPS_HKDF_MAX_INPUT_LEN 256
#define HWDPS_HKDF_MAX_OUTPUT_LEN 256
#define HWDPS_HMAC_MAX_MESSAGE_LEN 512
#define HWDPS_CRYPTO_ALLOC_SHASH 0
#define HWDPS_CRYPTO_ALLOC_MASK 0

struct tcrypt_result_t {
	struct completion completion;
	s32 err;
};

static void tcrypt_complete(struct crypto_async_request *req, s32 err)
{
	struct tcrypt_result_t *res = NULL;

	if ((err == -EINPROGRESS) || !req)
		return;
	res = req->data;
	if (res == 0)
		return;
	res->err = err;
	complete(&res->completion);
}

static s32 wait_async_op(s32 ret, struct tcrypt_result_t *tr)
{
	s32 result = ret;

	if ((result == -EINPROGRESS) || (result == -EBUSY)) {
		wait_for_completion(&tr->completion);
		reinit_completion(&tr->completion);
		result = tr->err;
	}
	return result;
}

static bool check_aes_cbc_para(buffer_t key, buffer_t iv,
	buffer_t in, out_buffer_t out)
{
	if (!key.data || (key.len != AES_KEYSIZE_256) || !iv.data ||
		(iv.len != AES_BLOCK_SIZE) || !in.data || (in.len == 0) ||
		((in.len % AES_BLOCK_SIZE) != 0) ||
		(in.len > HWDPS_CBC_AES_MAX_INPUT) || !out.len ||
		(*out.len < in.len))
		return false;
	return true;
}

/*
 * This function realize the aes cbc algorithm.
 * Which may exceed fifty lines.
 */
static s32 _aes_cbc_inter(buffer_t key, buffer_t iv,
	buffer_t in, out_buffer_t out, bool enc)
{
	s32 res;
	struct tcrypt_result_t result;
	struct scatterlist src_sg;
	struct scatterlist dst_sg;
	struct crypto_skcipher *tfm = NULL;
	struct skcipher_request *req = NULL;
	u8 *tmp_iv = NULL;

	if (!check_aes_cbc_para(key, iv, in, out)) {
		hwdps_pr_err("check_aes_cbc_para failed\n");
		return -EINVAL;
	}

	tfm = crypto_alloc_skcipher(HWDPS_CBC_AES_ALG, 0, 0);
	if (IS_ERR(tfm)) {
		res = PTR_ERR(tfm);
		hwdps_pr_err("alloc %s cipher failed res %d\n",
			HWDPS_CBC_AES_ALG, res);
		return res;
	}

	init_completion(&result.completion);

	crypto_skcipher_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);
	req = skcipher_request_alloc(tfm, GFP_NOFS);
	if (!req) {
		res = -ENOMEM;
		goto free_1;
	}
	skcipher_request_set_callback(req,
		CRYPTO_TFM_REQ_MAY_BACKLOG | CRYPTO_TFM_REQ_MAY_SLEEP,
		tcrypt_complete, &result);
	res = crypto_skcipher_setkey(tfm, key.data, key.len);
	if (res < 0)
		goto free_2;
	sg_init_one(&src_sg, in.data, in.len);
	sg_init_one(&dst_sg, out.data, in.len);
	tmp_iv = kzalloc(AES_BLOCK_SIZE, GFP_NOFS);
	if (!tmp_iv) {
		res = -ENOMEM;
		goto free_2;
	}
	if (memcpy_s(tmp_iv, AES_BLOCK_SIZE, iv.data, iv.len) != EOK) {
		res = -EINVAL;
		kzfree(tmp_iv);
		goto free_2;
	}

	skcipher_request_set_crypt(req, &src_sg, &dst_sg, in.len, tmp_iv);

	if (enc)
		res = wait_async_op(crypto_skcipher_encrypt(req), &result);
	else
		res = wait_async_op(crypto_skcipher_decrypt(req), &result);

	if (res == 0)
		*out.len = in.len;

	kzfree(tmp_iv);
free_2:
	skcipher_request_free(req);
free_1:
	crypto_free_skcipher(tfm);
	return res;
}

static bool aes_cbc_check_para(buffer_t key, buffer_t iv,
	buffer_t in, secondary_buffer_t out)
{
	return (!key.data || (key.len != AES_KEYSIZE_256) ||
		!iv.data || (iv.len != AES_BLOCK_SIZE) || !in.data ||
		(in.len == 0) || (in.len > HWDPS_CBC_AES_MAX_INPUT) ||
		!out.data || !out.len);
}

s32 aes_cbc(buffer_t key, buffer_t iv, buffer_t in,
	secondary_buffer_t out, bool enc)
{
	s32 res;
	u8 *tmp_out = NULL;
	out_buffer_t temp_buffer = { NULL, NULL };

	if (aes_cbc_check_para(key, iv, in, out)) {
		hwdps_pr_err("%s aes_cbc_check_para failed\n", __func__);
		return -EINVAL;
	}

	if ((in.len % AES_BLOCK_SIZE) != 0 || in.len == 0) {
		hwdps_pr_err("in_len %u\n", in.len);
		res = -EINVAL;
		goto out;
	}
	tmp_out = kzalloc(in.len, GFP_NOFS);
	if (!tmp_out) {
		res = -ENOMEM;
		goto out;
	}
	*out.len = in.len;

	temp_buffer.data = tmp_out;
	temp_buffer.len = out.len;

	res = _aes_cbc_inter(key, iv, in, temp_buffer, enc);
	if (res != 0) {
		hwdps_pr_err("_aes_cbc_inter failed %d\n", res);
		goto out;
	}

	*out.data = tmp_out;
	tmp_out = NULL;
out:
	/* if NULL, kzfree does nothing. */
	kzfree(tmp_out);

	return res;
}

static s32 shash_digest(struct crypto_shash *hmac_alg, const buffer_t *salt,
	const buffer_t *ikm, u8 *prk)
{
	SHASH_DESC_ON_STACK(desc, hmac_alg);
	s32 res;

	desc->tfm = hmac_alg;
	desc->flags = 0;
	res = crypto_shash_setkey(hmac_alg, salt->data, salt->len);
	if (res != 0)
		goto free;
	res = crypto_shash_digest(desc, ikm->data, ikm->len, prk);
free:
	shash_desc_zero(desc);
	return res;
}

s32 hash_generate_mac(buffer_t *hmac_key, buffer_t *msg, buffer_t *tag)
{
	s32 res;
	struct crypto_shash *hmac_tfm = NULL;

	if (!hmac_key || !hmac_key->data ||
		(hmac_key->len != HWDPS_HKDF_HASHLEN) ||
		!msg || !msg->data || (msg->len == 0) ||
		(msg->len > HWDPS_HMAC_MAX_MESSAGE_LEN) ||
		!tag || !tag->data || (tag->len != HWDPS_HMAC_TAG_LEN))
		return -EINVAL;

	hmac_tfm = crypto_alloc_shash(HWDPS_HKDF_HMAC_ALG,
		HWDPS_CRYPTO_ALLOC_SHASH,
		HWDPS_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_tfm)) {
		res = (s32)PTR_ERR(hmac_tfm);
		return res;
	}

	if (crypto_shash_digestsize(hmac_tfm) != tag->len) {
		crypto_free_shash(hmac_tfm);
		return -EINVAL;
	}
	res = shash_digest(hmac_tfm, hmac_key, msg, tag->data);
	crypto_free_shash(hmac_tfm);
	return res;
}

static bool hkdf_check_param(const buffer_t *salt, const buffer_t *ikm,
	const buffer_t *info, const buffer_t *okm)
{
	if (!salt || !salt->data || (salt->len != HWDPS_HKDF_HASHLEN) || !ikm ||
		!ikm->data || (ikm->len != HWDPS_HKDF_HASHLEN) || !info ||
		!info->data || (info->len == 0) ||
		(info->len > HWDPS_HKDF_MAX_INPUT_LEN) || !okm || !okm->data ||
		(okm->len == 0) || (okm->len > HWDPS_HKDF_MAX_OUTPUT_LEN)) {
		return false;
	}
	return true;
}

static s32 hkdf_extract(const buffer_t *salt, const buffer_t *ikm,
	buffer_t *prk)
{
	s32 res;
	struct crypto_shash *hmac_alg = NULL;

	hmac_alg = crypto_alloc_shash(HWDPS_HKDF_HMAC_ALG,
		HWDPS_CRYPTO_ALLOC_SHASH,
		HWDPS_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_alg)) {
		res = (s32)PTR_ERR(hmac_alg);
		return res;
	}

	if (crypto_shash_digestsize(hmac_alg) != prk->len) {
		crypto_free_shash(hmac_alg);
		return -EINVAL;
	}

	res = shash_digest(hmac_alg, salt, ikm, prk->data);
	crypto_free_shash(hmac_alg);
	return res;
}

static s32 hwdps_shash(struct crypto_shash *hmac_tfm, const buffer_t *prk,
	const buffer_t *info, buffer_t *okm)
{
	s32 res;
	const u8 *prev = NULL;
	u8 tmp[HWDPS_HKDF_HASHLEN] = {0};
	u8 counter = 1; /* counter starts from 1 */
	u32 i = 0;

	SHASH_DESC_ON_STACK(desc, hmac_tfm);

	desc->tfm = hmac_tfm;
	desc->flags = 0;
	res = crypto_shash_setkey(hmac_tfm, prk->data, prk->len);
	if (res != 0)
		goto free;
	for (; i < okm->len; i += HWDPS_HKDF_HASHLEN) {
		res = crypto_shash_init(desc);
		if (res != 0)
			goto free;
		if (prev) {
			res = crypto_shash_update(desc, prev,
				HWDPS_HKDF_HASHLEN);
			if (res != 0)
				goto free;
		}
		res = crypto_shash_update(desc, info->data, info->len);
		if (res != 0)
			goto free;
		// okm_len is always bigger than i
		if ((okm->len - i) < HWDPS_HKDF_HASHLEN) {
			res = crypto_shash_finup(desc, &counter,
				sizeof(counter), tmp);
			if (res != 0)
				goto free;
			// tmp has enough len and so is ok
			if (memcpy_s(&(okm->data)[i], okm->len - i,
				tmp, okm->len - i) != EOK)
				goto free;
			memzero_explicit(tmp, sizeof(tmp));
		} else {
			res = crypto_shash_finup(desc, &counter,
				sizeof(counter), &(okm->data)[i]);
			if (res != 0)
				goto free;
		}
		counter++;
		prev = &(okm->data)[i];
	}
free:
	shash_desc_zero(desc);
	return res;
}

static s32 hkdf_expand(const buffer_t *prk, const buffer_t *info,
	buffer_t *okm)
{
	s32 res;
	struct crypto_shash *hmac_tfm = NULL;

	hmac_tfm = crypto_alloc_shash(HWDPS_HKDF_HMAC_ALG,
		HWDPS_CRYPTO_ALLOC_SHASH,
		HWDPS_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_tfm)) {
		res = (s32)PTR_ERR(hmac_tfm);
		return res;
	}
	if (crypto_shash_digestsize(hmac_tfm) != prk->len) {
		crypto_free_shash(hmac_tfm);
		return -EINVAL;
	}
	res = hwdps_shash(hmac_tfm, prk, info, okm);
	crypto_free_shash(hmac_tfm);
	return res;
}

s32 hkdf_256(const buffer_t *salt, const buffer_t *ikm,
	const buffer_t *info, buffer_t *okm)
{
	s32 res;
	u8 tmp_key[HWDPS_HKDF_HASHLEN] = {0};
	u32 tmp_key_len = sizeof(tmp_key);
	buffer_t tmp_key_buf = { tmp_key, tmp_key_len };

	if (!hkdf_check_param(salt, ikm, info, okm))
		return -HWDPS_ERR_INVALID_ARGS;

	res = hkdf_extract(salt, ikm, &tmp_key_buf);
	if (res != 0) {
		hwdps_pr_err("hkdf_extract failed res = %d.\n", res);
		res = -HWDPS_ERR_HKDF_EXTRACT;
		goto out;
	}

	res = hkdf_expand(&tmp_key_buf, info, okm);
	if (res != 0) {
		hwdps_pr_err("hkdf_expand failed res = %d.\n", res);
		res = -HWDPS_ERR_HKDF_EXPAND;
		goto out;
	}
out:
	(void)memset_s(tmp_key, HWDPS_HKDF_HASHLEN, 0, HWDPS_HKDF_HASHLEN);
	return res;
}
