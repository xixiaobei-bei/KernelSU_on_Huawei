/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for generate key and
 *              encrypt key management.
 * Create: 2020-06-16
 */

#include "inc/tee/hwdps_keyinfo.h"
#include <keys/user-type.h>
#include <linux/key.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <securec.h>
#include <huawei_platform/hwdps/fscrypt_private.h>
#include "inc/base/hwdps_defines.h"
#include "inc/base/hwdps_utils.h"
#include "inc/tee/base_alg.h"
#include "inc/tee/hwdps_alg.h"
#include "inc/tee/hwdps_adapter.h"

s32 hwdps_get_key(const u8 *descriptor, buffer_t *aes_key, uid_t uid)
{
	struct key *keyring_key = NULL;
	const struct user_key_payload *ukp = NULL;
	struct fscrypt_key *primary_key = NULL;
	s32 res = 0;
	u8 tag[AES256_KEY_LEN] = {0};
	buffer_t tag_buffer = { tag, AES256_KEY_LEN };
	buffer_t msg_buffer = { (u8 *)&uid, sizeof(uid) };

	if (!descriptor || !aes_key || !aes_key->data ||
		aes_key->len != AES256_KEY_LEN)
		return -EINVAL;

	keyring_key = fscrypt_request_key(descriptor, FS_KEY_DESC_PREFIX,
		FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key)) {
		return PTR_ERR(keyring_key);
	}

	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon) {
		hwdps_pr_err("hwdps key type must be logon\n");
		res = -ENOKEY;
		goto out;
	}

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		/* key was revoked before we acquired its semaphore */
		hwdps_pr_err("hwdps key was revoked\n");
		res = -EKEYREVOKED;
		goto out;
	}
	if (ukp->datalen != sizeof(struct fscrypt_key)) {
		hwdps_pr_err("hwdps fscrypt key size err %d\n", ukp->datalen);
		res = -EINVAL;
		goto out;
	}
	primary_key = (struct fscrypt_key *)ukp->data;
	/* We only need AES256_KEY_LEN data as tag */
	if (memcpy_s(tag, AES256_KEY_LEN,
		primary_key->raw, AES256_KEY_LEN) != EOK) {
		hwdps_pr_err("master key error size %d\n", primary_key->size);
		res = -ENOKEY;
		goto out;
	}

	res = hash_generate_mac(&tag_buffer, &msg_buffer, aes_key);
	if (res != 0) {
		hwdps_pr_err("hash_generate_mac res %d\n", res);
		res = -ENOKEY;
		goto out;
	}

out:
	up_read(&keyring_key->sem);
	key_put(keyring_key);
	(void)memset_s(tag, AES256_KEY_LEN, 0, AES256_KEY_LEN);
	return res;
}

static void clear_new_key(buffer_t *fek,
	buffer_t *encoded_buffer, buffer_t *plaintext_fek_buffer)
{
	(void)memset_s(fek->data, fek->len, 0, fek->len);
	(void)memset_s(encoded_buffer->data, encoded_buffer->len,
		0, encoded_buffer->len);
	(void)memset_s(plaintext_fek_buffer->data, plaintext_fek_buffer->len,
		0, plaintext_fek_buffer->len);
}

s32 kernel_new_fek(const u8 *desc, uid_t uid,
	secondary_buffer_t *encoded_buf, buffer_t *fek)
{
	s32 err_code;
	u8 aes_key[AES256_KEY_LEN] = {0};
	u8 encoded_ciphertext[PHASE3_CIPHERTEXT_LENGTH] = {0};
	u8 plaintext_fek[FEK_LENGTH] = {0};
	buffer_t aes_key_buffer = { aes_key, AES256_KEY_LEN };
	buffer_t encoded_ciphertext_buffer = {
		encoded_ciphertext, PHASE3_CIPHERTEXT_LENGTH
	};
	buffer_t plaintext_fek_buffer = { plaintext_fek, FEK_LENGTH };

	if (!desc || !encoded_buf ||
		!encoded_buf->data || !fek || !fek->data ||
		fek->len != FEK_LENGTH) {
		return ERR_MSG_BAD_PARAM;
	}

	err_code = hwdps_get_key(desc, &aes_key_buffer, uid);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_get_key failed:%d\n", err_code);
		goto cleanup;
	}

	err_code = hwdps_generate_enc(aes_key_buffer, encoded_ciphertext_buffer,
		plaintext_fek_buffer, false);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_generate_enc failed:%d\n", err_code);
		goto cleanup;
	}

	if (memcpy_s(fek->data, fek->len, plaintext_fek,
		FEK_LENGTH) != EOK) {
		hwdps_pr_err("%s memcpy fek failed", __func__);
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	if (memcpy_s(*encoded_buf->data, *encoded_buf->len,
		encoded_ciphertext_buffer.data,
		encoded_ciphertext_buffer.len) != EOK) {
		hwdps_pr_err("%s memcpy material failed %u, %u", __func__,
			*encoded_buf->len, encoded_ciphertext_buffer.len);
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	hwdps_pr_info("%s success gcm\n", __func__);
cleanup:
	clear_new_key(&aes_key_buffer, &encoded_ciphertext_buffer,
		&plaintext_fek_buffer);
	return err_code;
}

s32 kernel_encrypt_key(const u8 *desc, uid_t uid,
	secondary_buffer_t *encoded_buf, buffer_t *fek)
{
	s32 err_code;
	u8 aes_key[AES256_KEY_LEN] = {0};
	u8 encoded_ciphertext[PHASE3_CIPHERTEXT_LENGTH] = {0};
	buffer_t aes_key_buffer = { aes_key, AES256_KEY_LEN };
	buffer_t encoded_ciphertext_buffer = {
		encoded_ciphertext, PHASE3_CIPHERTEXT_LENGTH
	};

	if (!desc || !encoded_buf ||
		!encoded_buf->data || !fek || !fek->data ||
		fek->len != FEK_LENGTH) {
		hwdps_pr_err("invalid param\n");
		return ERR_MSG_BAD_PARAM;
	}

	err_code = hwdps_get_key(desc, &aes_key_buffer, uid);
	if (err_code != 0) {
		hwdps_pr_err("%s, hwdps_get_key failed %d\n",
			__func__, err_code);
		goto cleanup;
	}

	err_code = hwdps_refresh_enc(aes_key_buffer, encoded_ciphertext_buffer,
		*fek);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_refresh_enc failed, err is %d\n", err_code);
		goto cleanup;
	}

	if (memcpy_s(*encoded_buf->data, *encoded_buf->len,
		encoded_ciphertext_buffer.data,
		encoded_ciphertext_buffer.len) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		hwdps_pr_err("%s, memcpy_s fail\n", __func__);
		goto cleanup;
	}
	hwdps_pr_info("%s success\n", __func__);
cleanup:
	(void)memset_s(aes_key, AES256_KEY_LEN, 0, AES256_KEY_LEN);
	(void)memset_s(encoded_ciphertext, PHASE3_CIPHERTEXT_LENGTH,
		0, PHASE3_CIPHERTEXT_LENGTH);
	return err_code;
}

static void clear_exist_key(buffer_t *fek, buffer_t *plaintext_fek_buffer)
{
	(void)memset_s(fek->data, fek->len, 0, fek->len);
	(void)memset_s(plaintext_fek_buffer->data, plaintext_fek_buffer->len,
		0, plaintext_fek_buffer->len);
}

s32 kernel_get_fek_v1(u64 profile_id, const buffer_t *encoded_wfek,
	secondary_buffer_t *fek)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u32 plaintext_fek_len = FEK_LENGTH;

	if (!fek || !fek->data || !fek->len || !encoded_wfek ||
		!encoded_wfek->data) {
		hwdps_pr_err("invalid param %s\n", __func__);
		return ERR_MSG_NULL_PTR;
	} else if (encoded_wfek->len != HWDPS_ENCODE_WFEK_SIZE_OLD) {
		hwdps_pr_err("invalid length:%u!\n", encoded_wfek->len);
		return ERR_MSG_BAD_PARAM;
	}
	err_code = get_phase1_key_from_list(profile_id, &phase1_key,
		&phase1_key_size);
	if (err_code != ERR_MSG_SUCCESS) {
		hwdps_pr_err("get phase1_key failed:%d\n", err_code);
		return err_code;
	}
	err_code = hwdps_dec_fek_v1(phase1_key, phase1_key_size,
		encoded_wfek, plaintext_fek, &plaintext_fek_len);
	if ((err_code != ERR_MSG_SUCCESS) ||
		(plaintext_fek_len != FEK_LENGTH)) {
		hwdps_pr_err("get fek failed:%d\n", err_code);
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	*(fek->data) = kzalloc(plaintext_fek_len, GFP_KERNEL);
	if (!*(fek->data)) {
		hwdps_pr_err("malloc failed\n");
		err_code = ERR_MSG_OUT_OF_MEMORY;
		goto cleanup;
	}
	if (memcpy_s(*(fek->data), plaintext_fek_len,
		plaintext_fek, FEK_LENGTH) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	*(fek->len) = plaintext_fek_len;
cleanup:
	kzfree(phase1_key);
	(void)memset_s(plaintext_fek, plaintext_fek_len, 0, plaintext_fek_len);
	return err_code;
}

s32 kernel_get_fek_v2(const u8 *desc, uid_t uid,
	const buffer_t *encoded_buf, secondary_buffer_t *fek)
{
	s32 err_code;
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u32 plaintext_fek_len = FEK_LENGTH;
	u8 aes_key[AES256_KEY_LEN] = {0};
	buffer_t aes_key_buffer = { aes_key, AES256_KEY_LEN };
	buffer_t plaintext_fek_buffer = { plaintext_fek, FEK_LENGTH };

	hwdps_pr_info("%s enter\n", __func__);
	if (!desc || !fek || !encoded_buf || !encoded_buf->data ||
		!fek->len || !fek->data) {
		hwdps_pr_err("invalid fek params\n");
		return ERR_MSG_NULL_PTR;
	} else if (encoded_buf->len != PHASE3_CIPHERTEXT_LENGTH) {
		hwdps_pr_err("invalid encoded_buf len:%u\n", encoded_buf->len);
		return ERR_MSG_BAD_PARAM;
	}

	err_code = hwdps_get_key(desc, &aes_key_buffer, uid);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_get_key in get key failed:%d\n", err_code);
		goto cleanup;
	}

	err_code = hwdps_generate_dec(aes_key_buffer,
		*encoded_buf, plaintext_fek_buffer);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_generate_dec failed:%d\n", err_code);
		goto cleanup;
	}

	*fek->data = kzalloc(plaintext_fek_len, GFP_KERNEL);
	if (!*(fek->data)) {
		hwdps_pr_err("bp_file_key malloc\n");
		err_code = ERR_MSG_OUT_OF_MEMORY;
		goto cleanup;
	}
	if (memcpy_s(*fek->data, plaintext_fek_len,
		plaintext_fek, FEK_LENGTH) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	*fek->len = plaintext_fek_len;
	hwdps_pr_info("%s success\n", __func__);
cleanup:
	/* the *fek->data will be free out the funtion, not memory leak */
	clear_exist_key(&aes_key_buffer, &plaintext_fek_buffer);
	return err_code;
}

static bool check_update_params(const u8 *desc, const buffer_t *encoded_buf,
	const secondary_buffer_t *fek)
{
	return (!desc || !fek || !encoded_buf || !fek->len ||
		!fek->data || !encoded_buf->data ||
		encoded_buf->len != PHASE3_CIPHERTEXT_LENGTH);
}

s32 kernel_update_fek(const u8 *desc, const buffer_t *encoded_buf,
	secondary_buffer_t *fek, uid_t new_uid, uid_t old_uid)
{
	s32 err_code;
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u8 aes_key[AES256_KEY_LEN] = {0};
	buffer_t aes_key_buffer = { aes_key, AES256_KEY_LEN };
	buffer_t plaintext_fek_buffer = { plaintext_fek, FEK_LENGTH };

	if (check_update_params(desc, encoded_buf, fek)) {
		hwdps_pr_err("invalid fek or encoded_buf\n");
		return ERR_MSG_NULL_PTR;
	}

	err_code = hwdps_get_key(desc, &aes_key_buffer, old_uid);
	if (err_code != 0) {
		hwdps_pr_err("%s hwdps_get_key in get key failed:%d\n",
			__func__, err_code);
		goto cleanup;
	}

	err_code = hwdps_generate_dec(aes_key_buffer,
		*encoded_buf, plaintext_fek_buffer);
	if (err_code != 0) {
		hwdps_pr_err("%s hwdps_generate_dec failed:%d\n", __func__,
			err_code);
		goto cleanup;
	}

	err_code = hwdps_get_key(desc, &aes_key_buffer, new_uid);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_get_key in get key failed:%d\n", err_code);
		goto cleanup;
	}

	err_code = hwdps_generate_enc(aes_key_buffer,
		*encoded_buf, plaintext_fek_buffer, true);
	if (err_code != 0) {
		hwdps_pr_err("hwdps_generate_enc failed:%d\n", err_code);
		goto cleanup;
	}

	*fek->data = kzalloc(FEK_LENGTH, GFP_KERNEL);
	if (!*(fek->data)) {
		err_code = ERR_MSG_OUT_OF_MEMORY;
		goto cleanup;
	}
	if (memcpy_s(*fek->data, FEK_LENGTH,
		plaintext_fek, FEK_LENGTH) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	*fek->len = FEK_LENGTH;
	hwdps_pr_info("%s success\n", __func__);
cleanup:
	/* the *fek->data will be free out the funtion, not memory leak */
	clear_exist_key(&aes_key_buffer, &plaintext_fek_buffer);
	return err_code;
}
