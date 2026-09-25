// SPDX-License-Identifier: GPL-2.0
/*
 * key management facility for FS encryption support.
 *
 * Copyright (C) 2015, Google, Inc.
 *
 * This contains encryption key functions.
 *
 * Written by Michael Halcrow, Ildar Muslukhov, and Uday Savagaonkar, 2015.
 */

#include <keys/user-type.h>
#include <linux/scatterlist.h>
#include <linux/ratelimit.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/skcipher.h>
#include "fscrypt_private.h"
#ifdef CONFIG_HWAA
#include <linux/security.h>
#include <securec.h>
#include <huawei_platform/hwaa/hwaa_limits.h>
#include <huawei_platform/hwaa/hwaa_fs_hooks.h>
#define HWAA_KEY_DESC_STANDARD_FLAG 0x42
#endif

static struct crypto_shash *essiv_hash_tfm;

static void derive_crypt_complete(struct crypto_async_request *req, int rc)
{
        struct fscrypt_completion_result *ecr = req->data;

        if (rc == -EINPROGRESS)
                return;

        ecr->res = rc;
        complete(&ecr->completion);
}

int fscrypt_set_gcm_key(struct crypto_aead *tfm,
                       const u8 deriving_key[FS_AES_256_GCM_KEY_SIZE])
{
	int res = 0;
	unsigned int iv_len;

	crypto_aead_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);

	iv_len = crypto_aead_ivsize(tfm);
	if (iv_len > FS_KEY_DERIVATION_IV_SIZE) {
		res = -EINVAL;
		pr_err("fscrypt %s : IV length is incompatible\n", __func__);
		goto out;
	}

	res = crypto_aead_setauthsize(tfm, FS_KEY_DERIVATION_TAG_SIZE);
	if (res < 0) {
		pr_err("fscrypt %s : Failed to set authsize\n", __func__);
		goto out;
	}

	res = crypto_aead_setkey(tfm, deriving_key,
					FS_AES_256_GCM_KEY_SIZE);
	if (res < 0)
	        pr_err("fscrypt %s : Failed to set deriving key\n", __func__);
	out:
      return res;
}

int fscrypt_derive_gcm_key(struct crypto_aead *tfm,
			 const u8 source_key[FS_KEY_DERIVATION_CIPHER_SIZE],
			       u8 derived_key[FS_KEY_DERIVATION_CIPHER_SIZE],
			       u8 iv[FS_KEY_DERIVATION_IV_SIZE],
			       int enc)
{
	int res = 0;
	struct aead_request *req = NULL;
	DECLARE_FS_COMPLETION_RESULT(ecr);
	struct scatterlist src_sg, dst_sg;
	unsigned int ilen;

	if (!tfm) {
		res = -EINVAL;
		goto out;
	}

	if (IS_ERR(tfm)) {
		res = PTR_ERR(tfm);
		goto out;
	}

	req = aead_request_alloc(tfm, GFP_NOFS);
	if (!req) {
		res = -ENOMEM;
		goto out;
	}

	aead_request_set_callback(req,
			CRYPTO_TFM_REQ_MAY_BACKLOG | CRYPTO_TFM_REQ_MAY_SLEEP,
			derive_crypt_complete, &ecr);


	ilen = enc ? FS_KEY_DERIVATION_NONCE_SIZE :
			FS_KEY_DERIVATION_CIPHER_SIZE;

	sg_init_one(&src_sg, source_key, FS_KEY_DERIVATION_CIPHER_SIZE);
	sg_init_one(&dst_sg, derived_key, FS_KEY_DERIVATION_CIPHER_SIZE);

	aead_request_set_ad(req, 0);

	aead_request_set_crypt(req, &src_sg, &dst_sg, ilen, iv);
	res = enc ? crypto_aead_encrypt(req) : crypto_aead_decrypt(req);
	if (res == -EINPROGRESS || res == -EBUSY) {
		wait_for_completion(&ecr.completion);
		res = ecr.res;
	}
out:
	if (req)
		aead_request_free(req);
	return res;
}

struct key *fscrypt_request_key(const u8 *descriptor, const u8 *prefix,
			       int prefix_size)
{
	u8 *full_key_descriptor = NULL;
	struct key *keyring_key = NULL;
	int full_key_len = prefix_size + (FS_KEY_DESCRIPTOR_SIZE * 2) + 1;

	full_key_descriptor = kmalloc(full_key_len, GFP_NOFS);
	if (!full_key_descriptor)
		return (struct key *)ERR_PTR(-ENOMEM);

	memcpy(full_key_descriptor, prefix, prefix_size);
	sprintf(full_key_descriptor + prefix_size,
			"%*phN", FS_KEY_DESCRIPTOR_SIZE,
			descriptor);
	full_key_descriptor[full_key_len - 1] = '\0';
	keyring_key = request_key(&key_type_logon, full_key_descriptor, NULL);
	kfree(full_key_descriptor);

	return keyring_key;
}

/*
 * Search the current task's subscribed keyrings for a "logon" key with
 * description prefix:descriptor, and if found acquire a read lock on it and
 * return a pointer to its validated payload in *payload_ret.
 */
static struct key *
find_and_lock_process_key(const struct fscrypt_context *ctx,
			  const char *prefix,
			  const u8 descriptor[FS_KEY_DESCRIPTOR_SIZE],
			  unsigned int min_keysize,
			  const struct fscrypt_key **payload_ret)
{
	struct key *key;
	const struct user_key_payload *ukp;
	int prefix_size = strlen(prefix);
	struct fscrypt_key *payload;

	key = fscrypt_request_key(ctx->master_key_descriptor,
				prefix, prefix_size);
	if (IS_ERR(key))
		return key;

	down_read(&key->sem);
	ukp = user_key_payload_locked(key);

	if (!ukp) /* was the key revoked before we acquired its semaphore? */
		goto invalid;

	payload = (struct fscrypt_key *)ukp->data;

	if (ukp->datalen != sizeof(struct fscrypt_key) ||
	    payload->size < 1 || payload->size > FS_MAX_KEY_SIZE) {
		fscrypt_warn(NULL,
			     "key with description '%s' has invalid payload",
			     key->description);
		goto invalid;
	}
	//force the size equal to FS_AES_256_GCM_KEY_SIZE since user space might pass FS_AES_256_XTS_KEY_SIZE
	payload->size = FS_AES_256_GCM_KEY_SIZE;
	if (payload->size != FS_AES_256_GCM_KEY_SIZE) {
		fscrypt_warn(NULL,
			     "key with description '%s' is too short (got %u bytes, need %u+ bytes)",
			     key->description, payload->size, min_keysize);
		goto invalid;
	}

	*payload_ret = payload;
	return key;

invalid:
	up_read(&key->sem);
	key_put(key);
	return ERR_PTR(-ENOKEY);
}

/* Find the master key, then derive the inode's actual encryption key */
static int find_and_derive_key(const struct inode *inode,
			       struct fscrypt_info *crypt_info,
			       const struct fscrypt_context *ctx,
			       u8 *derived_key, unsigned int derived_keysize)
{
	struct key *key;
	const struct fscrypt_key *payload;
	struct crypto_aead *tfm = NULL;
	u8 plain_text[FS_KEY_DERIVATION_CIPHER_SIZE] = {0};
	int err;

	key = find_and_lock_process_key(ctx, FS_KEY_DESC_PREFIX,
					ctx->master_key_descriptor,
					derived_keysize, &payload);
	if (key == ERR_PTR(-ENOKEY) && inode->i_sb->s_cop->key_prefix) {
		key = find_and_lock_process_key(ctx, inode->i_sb->s_cop->key_prefix,
						ctx->master_key_descriptor,
						derived_keysize, &payload);
	}
	if (IS_ERR(key))
		return PTR_ERR(key);

	crypt_info->ci_key_index = (int) *(payload->raw + 63) & 0xff;

	tfm = (struct crypto_aead *)crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		up_read(&key->sem);
		err = (int)PTR_ERR(tfm);
		tfm = NULL;
		pr_err("fscrypt %s : tfm allocation failed!\n", __func__);
		goto out;
	}
	err = fscrypt_set_gcm_key(tfm, payload->raw);
	if (err)
		goto out;
	err = fscrypt_derive_gcm_key(tfm, ctx->nonce, plain_text, (u8 *)ctx->iv, 0);
	if (err)
		goto out;
	memcpy(derived_key, plain_text, derived_keysize);
	crypt_info->ci_gtfm = tfm;
	up_read(&key->sem);
	key_put(key);
	return 0;

out:
	if (tfm)
		crypto_free_aead(tfm);
	up_read(&key->sem);
	key_put(key);
	return err;
}

static struct fscrypt_mode {
	const char *friendly_name;
	const char *cipher_str;
	int keysize;
	bool logged_impl_name;
} available_modes[] = {
	[FS_ENCRYPTION_MODE_AES_256_XTS] = {
		.friendly_name = "AES-256-XTS",
		.cipher_str = "xts(aes)",
		.keysize = 64,
	},
	[FS_ENCRYPTION_MODE_AES_256_CTS] = {
		.friendly_name = "AES-256-CTS-CBC",
		.cipher_str = "cts(cbc(aes))",
		.keysize = 32,
	},
	[FS_ENCRYPTION_MODE_AES_128_CBC] = {
		.friendly_name = "AES-128-CBC",
		.cipher_str = "cbc(aes)",
		.keysize = 16,
	},
	[FS_ENCRYPTION_MODE_AES_128_CTS] = {
		.friendly_name = "AES-128-CTS-CBC",
		.cipher_str = "cts(cbc(aes))",
		.keysize = 16,
	},
};

static struct fscrypt_mode *
select_encryption_mode(const struct fscrypt_info *ci, const struct inode *inode)
{
	if (!fscrypt_valid_enc_modes(ci->ci_data_mode, ci->ci_filename_mode)) {
		fscrypt_warn(inode->i_sb,
			     "inode %lu uses unsupported encryption modes(contents mode %d, filenames mode %d)",
			     inode->i_ino, ci->ci_data_mode,
			     ci->ci_filename_mode);
		return ERR_PTR(-EINVAL);
	}

	if (S_ISREG(inode->i_mode))
		return &available_modes[ci->ci_data_mode];

	if (S_ISDIR(inode->i_mode) || S_ISLNK(inode->i_mode))
		return &available_modes[ci->ci_filename_mode];

	WARN_ONCE(1, "fscrypt: filesystem tried to load encryption info for inode %lu, which is not encryptable (file type %d)\n",
		  inode->i_ino, (inode->i_mode & S_IFMT));
	return ERR_PTR(-EINVAL);
}

static int determine_cipher_type(struct fscrypt_info *ci, struct inode *inode,
				 const char **cipher_str_ret, int *keysize_ret)
{
	u32 mode;
	if (!fscrypt_valid_enc_modes(ci->ci_data_mode, ci->ci_filename_mode)) {
		pr_warn_ratelimited("inode %lu err enc mode (contents %d,%d)\n",
				    inode->i_ino,
				    ci->ci_data_mode, ci->ci_filename_mode);
		return -EINVAL;
	}

	if (S_ISREG(inode->i_mode)) {
		mode = ci->ci_data_mode;
	} else if (S_ISDIR(inode->i_mode) || S_ISLNK(inode->i_mode)) {
		mode = ci->ci_filename_mode;
	} else {
		WARN_ONCE(1, "inode %lu not encryptable, file type %d\n",
			  inode->i_ino, (inode->i_mode & S_IFMT));
		return -EINVAL;
	}

	*cipher_str_ret = available_modes[mode].cipher_str;
	*keysize_ret = available_modes[mode].keysize;
	return 0;
}

static void put_crypt_info(struct fscrypt_info *ci)
{
	void *prev = NULL;
	void *key = NULL;
	if (!ci)
		return;

	/*lint -save -e529 -e438*/
	key = ACCESS_ONCE(ci->ci_key);
	/*lint -restore*/
	/*lint -save -e1072 -e747 -e50*/
	prev = cmpxchg(&ci->ci_key, key, NULL);
	/*lint -restore*/
	if (prev == key && key) {
		memzero_explicit(key, (size_t)FS_MAX_KEY_SIZE);
		kfree(key);
		ci->ci_key_len = 0;
		ci->ci_key_index = -1;
	}
	if (ci->ci_gtfm)
		crypto_free_aead(ci->ci_gtfm);
	crypto_free_skcipher(ci->ci_ctfm);
	crypto_free_cipher(ci->ci_essiv_tfm);
	kmem_cache_free(fscrypt_info_cachep, ci);
}

static int derive_essiv_salt(const u8 *key, int keysize, u8 *salt)
{
	struct crypto_shash *tfm = READ_ONCE(essiv_hash_tfm);

	/* init hash transform on demand */
	if (unlikely(!tfm)) {
		struct crypto_shash *prev_tfm;

		tfm = crypto_alloc_shash("sha256", 0, 0);
		if (IS_ERR(tfm)) {
			fscrypt_warn(NULL,
				     "error allocating SHA-256 transform: %ld",
				     PTR_ERR(tfm));
			return PTR_ERR(tfm);
		}
		prev_tfm = cmpxchg(&essiv_hash_tfm, NULL, tfm);
		if (prev_tfm) {
			crypto_free_shash(tfm);
			tfm = prev_tfm;
		}
	}

	{
		SHASH_DESC_ON_STACK(desc, tfm);
		desc->tfm = tfm;
		desc->flags = 0;

		return crypto_shash_digest(desc, key, keysize, salt);
	}
}

static int init_essiv_generator(struct fscrypt_info *ci, const u8 *raw_key,
				int keysize)
{
	int err;
	struct crypto_cipher *essiv_tfm;
	u8 salt[SHA256_DIGEST_SIZE];

	essiv_tfm = crypto_alloc_cipher("aes", 0, 0);
	if (IS_ERR(essiv_tfm))
		return PTR_ERR(essiv_tfm);

	ci->ci_essiv_tfm = essiv_tfm;

	err = derive_essiv_salt(raw_key, keysize, salt);
	if (err)
		goto out;

	/*
	 * Using SHA256 to derive the salt/key will result in AES-256 being
	 * used for IV generation. File contents encryption will still use the
	 * configured keysize (AES-128) nevertheless.
	 */
	err = crypto_cipher_setkey(essiv_tfm, salt, sizeof(salt));
	if (err)
		goto out;

out:
	memzero_explicit(salt, sizeof(salt));
	return err;
}

void __exit fscrypt_essiv_cleanup(void)
{
	crypto_free_shash(essiv_hash_tfm);
}

int fscrypt_get_encryption_info(struct inode *inode)
{
	struct fscrypt_info *crypt_info;
	struct fscrypt_context ctx;
	struct crypto_skcipher *ctfm;
	struct fscrypt_mode *mode;
	u8 *raw_key = NULL;
	int res;
	int flag = 0;

	/* TicketNo:AR000B5MB3 -- HWAA file needs to check access control */
	/* TicketNo:AR0009DF3P -- SDP file needs to check master key */
	if (inode->i_crypt_info && !inode->i_crypt_info->ci_hw_enc_flag)
		return 0;
	/* TicketNo:AR0009DF3P END */
	/* TicketNo:AR000B5MB3 END */

	res = fscrypt_initialize(inode->i_sb->s_cop->flags);
	if (res)
		return res;

	/* TicketNo:AR000B5MB3 -- For HWAA protection */
	/* TicketNo:AR0009DF3P -- For SDP protection */
	if (inode->i_sb->s_cop && inode->i_sb->s_cop->get_keyinfo) {
		res = inode->i_sb->s_cop->get_keyinfo(inode, NULL, &flag);
		if (res) /* err case */
			return res;
		if (flag) /* get sdp crypt info success */
			return 0;
	}
	/* TicketNo:AR0009DF3P END */
	/* TicketNo:AR000B5MB3 END */

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
	if (res < 0) {
		if (!fscrypt_dummy_context_enabled(inode) ||
		    IS_ENCRYPTED(inode))
			return res;
		/* Fake up a context for an unencrypted directory */
		memset(&ctx, 0, sizeof(ctx));
		ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V2;
		ctx.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
		ctx.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
		memset(ctx.master_key_descriptor, 0x42, FS_KEY_DESCRIPTOR_SIZE);
	} else if (res != sizeof(ctx)) {
		pr_err("%s: inode %lu incorrect ctx size [%u : %lu]\n",
			inode->i_sb->s_type->name, inode->i_ino, res, sizeof(ctx));
		return -EINVAL;
	}

	if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V2)
		return -EINVAL;

	if (ctx.flags & ~FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	crypt_info = kmem_cache_alloc(fscrypt_info_cachep, GFP_NOFS);
	if (!crypt_info)
		return -ENOMEM;

	crypt_info->ci_flags = ctx.flags;
	crypt_info->ci_data_mode = ctx.contents_encryption_mode;
	crypt_info->ci_filename_mode = ctx.filenames_encryption_mode;
	crypt_info->ci_ctfm = NULL;
	crypt_info->ci_gtfm = NULL;
	crypt_info->ci_essiv_tfm = NULL;
	crypt_info->ci_key = NULL;
	crypt_info->ci_key_len = 0;
	crypt_info->ci_key_index = -1;
	crypt_info->ci_hw_enc_flag = 0;
	memcpy(crypt_info->ci_master_key, ctx.master_key_descriptor,
				sizeof(crypt_info->ci_master_key));

	mode = select_encryption_mode(crypt_info, inode);
	if (IS_ERR(mode)) {
		res = PTR_ERR(mode);
		goto out;
	}

	/*
	 * This cannot be a stack buffer because it is passed to the scatterlist
	 * crypto API as part of key derivation.
	 */
	res = -ENOMEM;
	raw_key = kmalloc(mode->keysize, GFP_NOFS);
	if (!raw_key)
		goto out;

	res = find_and_derive_key(inode, crypt_info, &ctx, raw_key, mode->keysize);
	if (res)
		goto out;

	ctfm = crypto_alloc_skcipher(mode->cipher_str, 0, 0);
	if (IS_ERR(ctfm)) {
		res = PTR_ERR(ctfm);
		fscrypt_warn(inode->i_sb,
			     "error allocating '%s' transform for inode %lu: %d",
			     mode->cipher_str, inode->i_ino, res);
		goto out;
	}
	if (unlikely(!mode->logged_impl_name)) {
		/*
		 * fscrypt performance can vary greatly depending on which
		 * crypto algorithm implementation is used.  Help people debug
		 * performance problems by logging the ->cra_driver_name the
		 * first time a mode is used.  Note that multiple threads can
		 * race here, but it doesn't really matter.
		 */
		mode->logged_impl_name = true;
		pr_info("fscrypt: %s using implementation \"%s\"\n",
			mode->friendly_name,
			crypto_skcipher_alg(ctfm)->base.cra_driver_name);
	}
	crypt_info->ci_ctfm = ctfm;
	crypto_skcipher_set_flags(ctfm, CRYPTO_TFM_REQ_WEAK_KEY);
	res = crypto_skcipher_setkey(ctfm, raw_key, mode->keysize);
	if (res)
		goto out;

       if (S_ISREG(inode->i_mode) &&
			inode->i_sb->s_cop->is_inline_encrypted &&
			inode->i_sb->s_cop->is_inline_encrypted(inode)) {
		crypt_info->ci_key = kzalloc((size_t)FS_MAX_KEY_SIZE, GFP_NOFS);
		if (!crypt_info->ci_key) {
			res = -ENOMEM;
			goto out;
		}
		crypt_info->ci_key_len = mode->keysize;
		/*lint -save -e732 -e747*/
		memcpy(crypt_info->ci_key, raw_key, crypt_info->ci_key_len);
		/*lint -restore*/
	}

	if (S_ISREG(inode->i_mode) &&
	    crypt_info->ci_data_mode == FS_ENCRYPTION_MODE_AES_128_CBC) {
		res = init_essiv_generator(crypt_info, raw_key, mode->keysize);
		if (res) {
			fscrypt_warn(inode->i_sb,
				     "error initializing ESSIV generator for inode %lu: %d",
				     inode->i_ino, res);
			goto out;
		}
	}
	if (cmpxchg(&inode->i_crypt_info, NULL, crypt_info) == NULL)
		crypt_info = NULL;
out:
	if (res == -ENOKEY)
		res = 0;
	put_crypt_info(crypt_info);
	kzfree(raw_key);
	return res;
}
EXPORT_SYMBOL(fscrypt_get_encryption_info);

void fscrypt_put_encryption_info(struct inode *inode)
{
	put_crypt_info(inode->i_crypt_info);
	inode->i_crypt_info = NULL;
}
EXPORT_SYMBOL(fscrypt_put_encryption_info);

#ifdef CONFIG_HWAA
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#define MAX_HISI_KEY_INDEX 31
#define FS_KEY_INDEX_OFFSET 63
static int hwaa_get_key_index(u8 *descriptor)
{
	struct key *keyring_key;
	const struct user_key_payload *ukp;
	struct fscrypt_key *master_key;
	int res;

	keyring_key = fscrypt_request_key(descriptor, FS_KEY_DESC_PREFIX,
		FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key)) {
		pr_err("hwaa request_key failed!\n");
		return PTR_ERR(keyring_key);
	}

	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon) {
		pr_err("hwaa key type must be logon\n");
		res = -ENOKEY;
		goto out;
	}
	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		/* key was revoked before we acquired its semaphore */
		pr_warn_once("hwaa key was revoked\n");
		res = -EKEYREVOKED;
		goto out;
	}
	if (ukp->datalen != sizeof(struct fscrypt_key)) {
		pr_warn_once("hwaa fscrypt key size err %d", ukp->datalen);
		res = -EINVAL;
		goto out;
	}
	master_key = (struct fscrypt_key *)ukp->data;
	if (master_key->size != FS_AES_256_GCM_KEY_SIZE) {
		pr_warn_once("hwaa master key size err %d", master_key->size);
		res = -ENOKEY;
		goto out;
	}
	res = (int) (*(master_key->raw + FS_KEY_INDEX_OFFSET) & 0xff);

out:
	up_read(&keyring_key->sem);
	key_put(keyring_key);
	return res;
}
#endif

static int hwaa_check_support(struct inode *inode)
{
	int err = 0;
	u32 flags;

	if (!inode->i_sb->s_cop->get_hwaa_flags ||
		!inode->i_sb->s_cop->get_hwaa_attr)
		return -EOPNOTSUPP;
	if (!inode->i_crypt_info) {
		err = inode->i_sb->s_cop->get_hwaa_flags(inode, NULL, &flags);
	} else {
		/*
		 * The inode->i_crypt_info->ci_hw_enc_flag keeps sync with the
		 * flags in xattr_header. And it can not be changed once the
		 * file is opened.
		 */
		flags = (u32)(inode->i_crypt_info->ci_hw_enc_flag);
	}
	if (err) {
		pr_err("hwaa ino %lu get flags err %d\n", inode->i_ino, err);
	} else if (!(flags & HWAA_XATTR_ENABLE_FLAG)) {
		pr_info_once("hwaa ino %lu no support auth\n", inode->i_ino);
		err = -EOPNOTSUPP;
	}

	return err;
}

static uint8_t *hwaa_do_get_attr(struct inode *inode, size_t size)
{
	int err;
	uint8_t *wfek;

	wfek = kmalloc(size, GFP_NOFS);
	if (!wfek)
		return NULL;
	err = inode->i_sb->s_cop->get_hwaa_attr(inode, wfek, size);
	if (err == -ENODATA) {
		pr_err("hwaa ino %lu hwaa xattr is null\n", inode->i_ino);
		goto free_out;
	} else if (err != HWAA_ENCODED_WFEK_SIZE) {
		pr_err("hwaa ino %lu wrong encoded_wfek size %d\n",
			inode->i_ino, err);
		goto free_out;
	}
	return wfek;

free_out:
	kfree(wfek);
	return NULL;
}

static int hwaa_do_get_context(struct inode *inode, struct fscrypt_context *ctx)
{
	int err;

	err = fscrypt_initialize(inode->i_sb->s_cop->flags);
	if (err) {
		pr_err("hwaa ino %lu init fscrypt fail\n", inode->i_ino);
		return err;
	}
	err = inode->i_sb->s_cop->get_context(inode, ctx, sizeof(*ctx));
	if (err < 0) {
		if (!fscrypt_dummy_context_enabled(inode) ||
			inode->i_sb->s_cop->is_encrypted(inode))
			return err;
		/* Fake up a context for an unencrypted directory */
		if (memset_s(ctx, sizeof(*ctx), 0, sizeof(*ctx)) != EOK)
			return err;
		ctx->format = FS_ENCRYPTION_CONTEXT_FORMAT_V2;
		ctx->contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
		ctx->filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
		if (memset_s(ctx->master_key_descriptor, FS_KEY_DESCRIPTOR_SIZE,
			HWAA_KEY_DESC_STANDARD_FLAG,
			FS_KEY_DESCRIPTOR_SIZE) != EOK)
			return err;
	} else if (err != sizeof(*ctx)) {
		pr_err("hwaa ino %lu ctx size [%u : %lu]\n",
			inode->i_ino, err, sizeof(*ctx));
		return -EINVAL;
	}

	return 0;
}

static struct fscrypt_info *hwaa_get_fscrypt_info(struct fscrypt_context *ctx,
	struct inode *inode, const char **cipher_str, int *keysize)
{
	struct fscrypt_info *ci;
	int err;

	ci = kmem_cache_alloc(fscrypt_info_cachep, GFP_NOFS);
	if (!ci)
		return ERR_PTR(-ENOMEM);
	ci->ci_flags = ctx->flags;
	ci->ci_data_mode = ctx->contents_encryption_mode;
	ci->ci_filename_mode = ctx->filenames_encryption_mode;
	ci->ci_ctfm = NULL;
	ci->ci_gtfm = NULL;
	ci->ci_essiv_tfm = NULL;
	ci->ci_key = NULL;
	ci->ci_key_len = 0;
	ci->ci_key_index = -1; // inited as invalid
	if (memcpy_s(ci->ci_master_key, sizeof(ci->ci_master_key),
		ctx->master_key_descriptor,
		sizeof(ctx->master_key_descriptor)) != EOK) {
		put_crypt_info(ci);
		return ERR_PTR(-EINVAL);
	}
	err = determine_cipher_type(ci, inode, cipher_str, keysize);
	if (err) {
		pr_err("hwaa ino %lu cipher type fail\n", inode->i_ino);
		put_crypt_info(ci);
		return ERR_PTR(err);
	}
	return ci;
}

static int hwaa_do_get_fek(struct inode *inode, uint8_t *wfek, size_t size,
	uint8_t **fek, int *fek_len)
{
	int err;

	err = hwaa_get_fek(inode, wfek, size, fek, fek_len, GFP_NOFS);
	if (err) {
		pr_err("hwaa ino %lu get fek err %d\n", inode->i_ino, err);
		return err;
	}
	if ((*fek_len) > HWAA_FEK_SIZE_MAX) {
		pr_err("hwaa fek length too large %d\n", *fek_len);
		return -EINVAL;
	}

	return 0;
}

static int hwaa_do_set_cipher(struct inode *inode, const char *cipher_str,
	uint8_t *fek, int fek_len, int keysize, struct fscrypt_info *ci)
{
	struct crypto_skcipher *ctfm;
	int err;

	ctfm = crypto_alloc_skcipher(cipher_str, 0, 0); // 0 as default value
	if (!ctfm || IS_ERR(ctfm)) {
		err = ctfm ? PTR_ERR(ctfm) : -ENOMEM;
		pr_err("hwaa ino %lu alloc ctfm err %d\n", inode->i_ino, err);
		return err;
	}
	ci->ci_ctfm = ctfm;
	crypto_skcipher_clear_flags(ctfm, ~0);
	crypto_skcipher_set_flags(ctfm, CRYPTO_TFM_REQ_WEAK_KEY);
	err = crypto_skcipher_setkey(ctfm, fek, keysize);
	if (err) {
		pr_err("hwaa ino %lu setkey fail\n", inode->i_ino);
		return err;
	}
	if (S_ISREG(inode->i_mode) &&
		inode->i_sb->s_cop->is_inline_encrypted &&
		inode->i_sb->s_cop->is_inline_encrypted(inode)) {
		ci->ci_key = kzalloc(FS_MAX_KEY_SIZE, GFP_NOFS);
		if (!ci->ci_key)
			return -ENOMEM;
		ci->ci_key_len = fek_len;
		if (memcpy_s(ci->ci_key, FS_MAX_KEY_SIZE, fek, fek_len) != EOK)
			return -EINVAL;
	}
	if (S_ISREG(inode->i_mode) &&
		(ci->ci_data_mode == FS_ENCRYPTION_MODE_AES_128_CBC)) {
		err = init_essiv_generator(ci, fek, keysize);
		if (err) {
			pr_err("hwaa ino %lu alloc essiv err %d\n",
				inode->i_ino, err);
			return err;
		}
	}
	/* cached kmem may have dirty data */
	ci->ci_hw_enc_flag = (u8)(HWAA_XATTR_ENABLE_FLAG);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
	if (inode->i_crypt_info && (inode->i_crypt_info->ci_key_index >= 0) &&
	    	(inode->i_crypt_info->ci_key_index <= MAX_HISI_KEY_INDEX))
		ci->ci_key_index = inode->i_crypt_info->ci_key_index;
	else
		ci->ci_key_index = hwaa_get_key_index(ci->ci_master_key);
	if ((ci->ci_key_index < 0) || (ci->ci_key_index > MAX_HISI_KEY_INDEX)) {
		pr_err("ci_key_index: %d\n", ci->ci_key_index);
		return -EINVAL;
	}
#endif
	return 0;
}

/*
 * mainly copied from fscrypt_get_encryption_info
 *
 * Return:
 *  o -EAGAIN: file is not protected by HWAA
 *  o -EPERM: no permission to open the file
 *  o 0: SUCC
 *  o other errno: other errors
 */
int hwaa_get_context(struct inode *inode)
{
	struct fscrypt_info *ci = NULL;
	struct fscrypt_context ctx;
	const char *cipher_str = NULL;
	int keysize;
	uint8_t *encoded_wfek = NULL;
	uint8_t *fek = NULL;
	uint32_t fek_len;
	int err;
	if (inode->i_crypt_info && !inode->i_crypt_info->ci_hw_enc_flag)
		return 0;
	err = hwaa_check_support(inode);
	if (err)
		return err;
	encoded_wfek = hwaa_do_get_attr(inode, HWAA_ENCODED_WFEK_SIZE);
	if (!encoded_wfek)
		return -ENOMEM;
	if (inode->i_crypt_info) {
		err = hwaa_has_access(encoded_wfek, HWAA_ENCODED_WFEK_SIZE);
		goto free_encoded_wfek; // return anyway
	}
	err = hwaa_do_get_context(inode, &ctx);
	if (err)
		goto free_encoded_wfek;
	ci = hwaa_get_fscrypt_info(&ctx, inode, &cipher_str, &keysize);
	if (IS_ERR(ci)) {
		err = PTR_ERR(ci);
		goto free_encoded_wfek;
	}
	err = hwaa_do_get_fek(inode, encoded_wfek, HWAA_ENCODED_WFEK_SIZE,
		&fek, &fek_len);
	if (err)
		goto free_fek;
	err = hwaa_do_set_cipher(inode, cipher_str, fek, fek_len, keysize, ci);
	if (err)
		goto free_fek;
	if (cmpxchg(&inode->i_crypt_info, NULL, ci) == NULL)
		ci = NULL;
free_fek:
	kzfree(fek);
	if (err == -ENOKEY)
		err = 0;
	put_crypt_info(ci);
free_encoded_wfek:
	kfree(encoded_wfek);
	return err;
}
EXPORT_SYMBOL(hwaa_get_context);
#endif
