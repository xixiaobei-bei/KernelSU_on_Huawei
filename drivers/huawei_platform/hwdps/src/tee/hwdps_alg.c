/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              hwdps algorithm.
 * Create: 2020-06-16
 */

#include "inc/tee/hwdps_alg.h"
#include <linux/key.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/string.h>
#include <securec.h>
#include <huawei_platform/hwdps/fscrypt_private.h>
#include "inc/base/hwdps_utils.h"
#include "inc/tee/base_alg.h"

#define FS_AES_256_GCM_KEY_SIZE 32

static s32 hwdps_check_param_v1(const u8 *phase1_key, u32 phase1_key_len,
	const buffer_t *encoded_wfek, const u8 *plaintext_fek,
	const u32 *plaintext_fek_len)
{
	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES) ||
		!encoded_wfek || !encoded_wfek->data ||
		(encoded_wfek->len != HWDPS_ENCODE_WFEK_SIZE_OLD) ||
		!plaintext_fek || !plaintext_fek_len ||
		*plaintext_fek_len != FEK_LENGTH)
		return -EINVAL;
	return HWDPS_SUCCESS;
}

static bool check_hwdps_gen_dec_key_para(const u8 *phase1_key,
	u32 phase1_key_len, const buffer_t *encoded_wfek,
	const u8 *out_key, const u32 *out_key_len)
{
	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES) ||
		!encoded_wfek->data ||
		(encoded_wfek->len != HWDPS_ENCODE_WFEK_SIZE_OLD) ||
		!out_key || !out_key_len ||
		(*out_key_len != AES256_KEY_LEN))
		return false;
	return true;
}

static s32 hwdps_gen_dec_key(const u8 *phase1_key, u32 phase1_key_len,
	const buffer_t *encoded_wfek, u8 *out_key,
	u32 *out_key_len)
{
	s32 ret;
	u8 aad[AAD_V1_LEN] = {0};
	struct xattr_v1_t *xattr = NULL;
	buffer_t salt_buf = { NULL, 0 };
	const buffer_t key_buf = { (u8 *)phase1_key, phase1_key_len };
	buffer_t aad_buf = { NULL, 0 };
	buffer_t out_buf = { out_key, *out_key_len };

	if (!check_hwdps_gen_dec_key_para(phase1_key, phase1_key_len,
		encoded_wfek, out_key, out_key_len))
		return -HWDPS_ERR_INVALID_ARGS;

	xattr = (struct xattr_v1_t *)(encoded_wfek->data);
	if (memcpy_s(aad, sizeof(aad), &xattr->aad,
		sizeof(xattr->aad) - PERMS_LENGTH) != EOK)
		return -EINVAL;

	salt_buf.data = xattr->index;
	salt_buf.len = sizeof(xattr->index);
	aad_buf.data = aad;
	aad_buf.len = sizeof(aad);
	ret = hkdf_256(&salt_buf, &key_buf, &aad_buf, &out_buf);
	*out_key_len = out_buf.len;
	return ret;
}

static s32 hwdps_dec_fek_v1_inner(const u8 *phase1_key, u32 phase1_key_len,
	const buffer_t *encoded_wfek, u8 *plaintext_fek,
	u32 *plaintext_fek_len)
{
	s32 ret;
	u8 dec_key[AES256_KEY_LEN] = {0};
	u32 dec_key_len = sizeof(dec_key);
	struct xattr_v1_t *xattr = (struct xattr_v1_t *)(encoded_wfek->data);
	u8 *out = NULL;
	buffer_t dec_key_buf = { dec_key, dec_key_len };
	buffer_t iv_buf = { xattr->iv, sizeof(xattr->iv) };
	buffer_t in_buf = { xattr->enc_fek, sizeof(xattr->enc_fek) };
	secondary_buffer_t out_buf = { &out, plaintext_fek_len };

	ret = hwdps_gen_dec_key(phase1_key, phase1_key_len, encoded_wfek,
		dec_key, &dec_key_len);
	if (ret != 0)
		return ret;

	ret = aes_cbc(dec_key_buf, iv_buf, in_buf, out_buf, false);
	*plaintext_fek_len = *(out_buf.len);
	if ((ret != 0) || (*plaintext_fek_len != FEK_LENGTH)) {
		hwdps_pr_err("%s aes failed :%d\n", __func__, ret);
		kzfree(out);
		return -EFAULT;
	}

	if (memcpy_s(plaintext_fek, *plaintext_fek_len, *(out_buf.data),
		FEK_LENGTH) != EOK) {
		kzfree(out);
		return -EINVAL;
	}
	kzfree(out);
	return ret;
}

static s32 hwdps_dec_enc_fek_check_param(buffer_t aes_key, buffer_t cipher,
	buffer_t plaintext_fek)
{
	if (!aes_key.data || !cipher.data || !plaintext_fek.data)
		return -HWDPS_ERR_INVALID_ARGS;

	if (aes_key.len != FS_AES_256_GCM_KEY_SIZE) {
		hwdps_pr_err("%s aes len err:%u\n", __func__, aes_key.len);
		return -HWDPS_ERR_INVALID_ARGS;
	}

	if (cipher.len != PHASE3_CIPHERTEXT_LENGTH) {
		hwdps_pr_err("%s cipher len err:%u\n", __func__, cipher.len);
		return -HWDPS_ERR_INVALID_ARGS;
	}

	if (plaintext_fek.len != FEK_LENGTH) {
		hwdps_pr_err("%s plaintext len err:%u\n", __func__,
			plaintext_fek.len);
		return -HWDPS_ERR_INVALID_ARGS;
	}
	return HWDPS_SUCCESS;
}

s32 hwdps_generate_dec(buffer_t aes_key,
	buffer_t ciphertext, buffer_t plaintext_fek)
{
	s32 ret;
	struct xattribs_v3_t *xattr = NULL;
	struct crypto_aead *tfm = NULL;
	u8 plaintext_fek_buf[FS_KEY_DERIVATION_CIPHER_SIZE] = {0};

	hwdps_pr_debug("%s enter\n", __func__);
	ret = hwdps_dec_enc_fek_check_param(aes_key, ciphertext, plaintext_fek);
	if (ret != 0)
		return ret;
	xattr = (struct xattribs_v3_t *)ciphertext.data;
	tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm))
		return (int)PTR_ERR(tfm);

	ret = fscrypt_set_gcm_key(tfm, aes_key.data);
	if (ret != 0) {
		hwdps_pr_err("%s, set key failed, ret:%d", __func__, ret);
		goto out;
	}

	ret = fscrypt_derive_gcm_key(tfm, xattr->enc_fek, plaintext_fek_buf,
		xattr->iv, 0); /* 0 means decrypt */
	if (ret != 0) {
		hwdps_pr_err("%s, derive key failed, ret:%d", __func__, ret);
		goto out;
	}
	if (memcpy_s(plaintext_fek.data, plaintext_fek.len,
		plaintext_fek_buf,  plaintext_fek.len) != EOK) {
		hwdps_pr_err("%s, memcpy failed\n", __func__);
		ret = -HWDPS_ERR_INTERNAL;
	}
	hwdps_pr_debug("%s end\n", __func__);
out:
	crypto_free_aead(tfm);
	(void)memset_s(plaintext_fek_buf, sizeof(plaintext_fek_buf), 0,
		sizeof(plaintext_fek_buf));
	return ret;
}

static s32 hwdps_enc_fek(buffer_t aes_key, buffer_t ciphertext,
	buffer_t plaintext_fek)
{
	s32 ret;
	struct xattribs_v3_t *xattr = NULL;
	u32 fek_temp_len = FS_KEY_DERIVATION_CIPHER_SIZE;
	u8 fek_temp[FS_KEY_DERIVATION_CIPHER_SIZE] = {0};
	struct crypto_aead *tfm = NULL;

	hwdps_pr_debug("%s enter gcm\n", __func__);
	ret = hwdps_dec_enc_fek_check_param(aes_key, ciphertext, plaintext_fek);
	if (ret != HWDPS_SUCCESS)
		return ret;

	xattr = (struct xattribs_v3_t *)ciphertext.data;
	xattr->version[0] = VERSION_3;
	get_random_bytes(xattr->iv, sizeof(xattr->iv));

	if (memcpy_s(fek_temp, fek_temp_len, plaintext_fek.data,
		plaintext_fek.len) != EOK) {
		hwdps_pr_err("memcpy fek temp failed\n");
		return -EINVAL;
	}

	tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm))
		return (s32)PTR_ERR(tfm);

	ret = fscrypt_set_gcm_key(tfm, aes_key.data);
	if (ret != 0) {
		hwdps_pr_err("%s, set key failed ret:%d", __func__, ret);
		goto out;
	}

	ret = fscrypt_derive_gcm_key(tfm, fek_temp, xattr->enc_fek,
		xattr->iv, 1); /* 1 means encrypt */
	if (ret != 0) {
		hwdps_pr_err("%s, derive key failed ret:%d", __func__, ret);
		goto out;
	}

	hwdps_pr_debug("%s end\n", __func__);
out:
	crypto_free_aead(tfm);
	(void)memset_s(fek_temp, fek_temp_len, 0, fek_temp_len);
	return ret;
}

s32 hwdps_dec_fek_v1(const u8 *phase1_key, u32 phase1_key_len,
	const buffer_t *encoded_wfek, u8 *plaintext_fek, u32 *plaintext_fek_len)
{
	s32 ret = hwdps_check_param_v1(phase1_key, phase1_key_len,
		encoded_wfek, plaintext_fek, plaintext_fek_len);

	if (ret != 0) {
		hwdps_pr_err("%s res = %d\n", __func__, ret);
		return ret;
	}

	return hwdps_dec_fek_v1_inner(phase1_key, phase1_key_len, encoded_wfek,
		plaintext_fek, plaintext_fek_len);
}

s32 hwdps_generate_enc(buffer_t aes_key, buffer_t ciphertext,
	buffer_t plaintext_fek, bool is_update)
{
	hwdps_pr_debug("%s enter\n", __func__);
	if (!is_update)
		get_random_bytes(plaintext_fek.data, plaintext_fek.len);

	return hwdps_enc_fek(aes_key, ciphertext, plaintext_fek);
}

s32 hwdps_refresh_enc(buffer_t aes_key, buffer_t ciphertext,
	buffer_t plaintext_fek)
{
	hwdps_pr_debug("%s enter\n", __func__);
	return hwdps_enc_fek(aes_key, ciphertext, plaintext_fek);
}
