
#include <keys/user-type.h>
#include <linux/scatterlist.h>
#include <uapi/linux/keyctl.h>
#include <crypto/skcipher.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/hmfs_fs.h>
#include "hmfs.h"
#include "xattr.h"
#include "sdp_internal.h"
#include "fscrypt_metadata.h"
#include <huawei_platform/hwdps/hwdps_defines.h>

#if F2FS_FS_SDP_ENCRYPTION
static void f2fs_put_crypt_info(struct fscrypt_info *ci)
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

	crypto_free_skcipher(ci->ci_ctfm);
	if (ci->ci_gtfm)
		crypto_free_aead(ci->ci_gtfm);
	if (ci->ci_essiv_tfm)
		crypto_free_cipher(ci->ci_essiv_tfm);
	kmem_cache_free(fscrypt_info_cachep, ci);
}

static void hmfs_sdp_rawkey_config(const struct user_key_payload *ukp, u8 *raw,
						int *size, bool filepubkey,
						unsigned char ver, int *res)
{
	struct fscrypt_sdp_key *mst_sdp = (struct fscrypt_sdp_key *)ukp->data;

	if (ver > INLINE_CRYPTO_V2) {
		if ((mst_sdp->sdpclass == FSCRYPT_SDP_ECE_CLASS ||
			mst_sdp->sdpclass == FSCRYPT_SDP_SECE_CLASS) &&
				mst_sdp->size == FS_AES_256_GCM_KEY_SIZE) {
			*size = mst_sdp->size;
			memcpy(raw, mst_sdp->raw, mst_sdp->size);
			*res = 0;
		}
	} else {
		if (mst_sdp->sdpclass == FSCRYPT_SDP_ECE_CLASS &&
			mst_sdp->size == FS_AES_256_GCM_KEY_SIZE) {
			*size = mst_sdp->size;
			memcpy(raw, mst_sdp->raw, mst_sdp->size);
			*res = 0;
		} else if (mst_sdp->sdpclass == FSCRYPT_SDP_SECE_CLASS &&
				(!filepubkey || mst_sdp->size == FS_SDP_ECC_PRIV_KEY_SIZE) &&
					mst_sdp->pubkeysize == FS_SDP_ECC_PUB_KEY_SIZE) {
			/*
			 * if filepubkey is ture, class private key doesn't need
			 * exist. if filepubkey is false, class private key must
			 * exist.
			 */
			*size = mst_sdp->pubkeysize + mst_sdp->size;
			memcpy(raw, mst_sdp->raw, mst_sdp->size);
			memcpy(raw + mst_sdp->size, mst_sdp->pubkey, mst_sdp->pubkeysize);
			*res = 0;
		}
	}
}

static int f2fs_do_get_keyring_payload(u8 *descriptor, u8 *raw, int *size,
						bool filepubkey,
						unsigned char ver)
{
	int res = -ENOKEY;
	struct key *keyring_key = NULL;
	const struct user_key_payload *ukp = NULL;
	struct fscrypt_key *mst_key = NULL;

	keyring_key = fscrypt_request_key(descriptor, FS_KEY_DESC_PREFIX,
			FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key))
		return PTR_ERR(keyring_key);

	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon)
		goto out;

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		/* key was revoked before we acquired its semaphore */
		res = -EKEYREVOKED;
		goto out;
	}

	if (ukp->datalen == sizeof(struct fscrypt_key)) {
		mst_key = (struct fscrypt_key *)ukp->data;
		if (mst_key->size == FS_AES_256_GCM_KEY_SIZE) {
			*size = mst_key->size;
			memcpy(raw, mst_key->raw, mst_key->size);
			res = 0;
		}
	} else if (ukp->datalen == sizeof(struct fscrypt_sdp_key)) {
		hmfs_sdp_rawkey_config(ukp, raw, size, filepubkey, ver, &res);
	}
out:
	up_read(&keyring_key->sem);
	key_put(keyring_key);
	return res;
}

static int f2fs_do_get_keyindex(u8 *descriptor, int *keyindex,
					unsigned char ver)
{
	int res = -ENOKEY;
	struct key *keyring_key = NULL;
	const struct user_key_payload *ukp;

	keyring_key = fscrypt_request_key(descriptor, FS_KEY_DESC_PREFIX,
			FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key))
		return PTR_ERR(keyring_key);

	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon)
		goto out;

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		/* key was revoked before we acquired its semaphore */
		res = -EKEYREVOKED;
		goto out;
	}

	if (ver > INLINE_CRYPTO_V2) {
		struct fscrypt_sdp_key *master_key = NULL;
		if (ukp->datalen == sizeof(struct fscrypt_sdp_key)) {
			master_key = (struct fscrypt_sdp_key *)ukp->data;
			if (master_key->size == FS_AES_256_GCM_KEY_SIZE) {
				*keyindex = (int)(*(master_key->raw +
							FS_KEY_INDEX_OFFSET) &
								0xff);
				res = 0;
			}
		}
	} else {
		struct fscrypt_key *master_key = NULL;
		if (ukp->datalen == sizeof(struct fscrypt_key)) {
			master_key = (struct fscrypt_key *)ukp->data;
			if (master_key->size == FS_AES_256_GCM_KEY_SIZE) {
				*keyindex = (int)(*(master_key->raw +
							FS_KEY_INDEX_OFFSET) &
								0xff);
				res = 0;
			}
		}
	}

out:
	up_read(&keyring_key->sem);
	key_put(keyring_key);
	return res;
}

static int hmfs_determine_cipher_type(struct fscrypt_info *ci,
						   const char **cipher_str_ret,
						   int *keysize_ret)
{
	if (ci->ci_data_mode != FS_ENCRYPTION_MODE_AES_256_XTS)
		return -ENOKEY;

	*cipher_str_ret = "xts(aes)";
	*keysize_ret = FS_AES_256_XTS_KEY_SIZE;
	return 0;
}

static int hmfs_derive_ctfm_from_fek(struct fscrypt_info *crypt_info, u8 *raw_key)
{
	int res = -ENOKEY;
	const char *cipher_str = NULL;
	struct crypto_skcipher *ctfm = NULL;
	int keysize = 0;

	res = hmfs_determine_cipher_type(crypt_info, &cipher_str, &keysize);
	if (res)
		goto out;

	ctfm = crypto_alloc_skcipher(cipher_str, 0, 0);
	if (IS_ERR(ctfm)) {
		res = PTR_ERR(ctfm);
		goto out;
	}

	crypto_skcipher_clear_flags(ctfm, ~0);
	crypto_skcipher_set_flags(ctfm, CRYPTO_TFM_REQ_WEAK_KEY);
	res = crypto_skcipher_setkey(ctfm, raw_key, keysize);
	if (res)
		goto out;

	kzfree(crypt_info->ci_key);
	crypt_info->ci_key = kzalloc((size_t)FS_MAX_KEY_SIZE, GFP_NOFS);
	if (!crypt_info->ci_key) {
		res = -ENOMEM;
		goto out;
	}

	crypt_info->ci_key_len = keysize;
	/*lint -save -e732 -e747*/
	memcpy(crypt_info->ci_key, raw_key, crypt_info->ci_key_len);
	/*lint -restore*/

	if (crypt_info->ci_ctfm)
		crypto_free_skcipher(crypt_info->ci_ctfm);
	crypt_info->ci_ctfm = ctfm;
	ctfm = NULL;
	res = 0;
out:
	if (ctfm)
		crypto_free_skcipher(ctfm);
	return res;
}

static int f2fs_do_get_fek(u8 *descriptor, u8 *nonce, u8 *fek, u8 *iv, int enc,
					unsigned char ver)
{
	int res = 0;
	struct crypto_aead *tfm = NULL;
	u8 raw[FS_MAX_KEY_SIZE];
	int size;

	tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm))
		return (int)PTR_ERR(tfm);

	res = f2fs_do_get_keyring_payload(descriptor, raw, &size, false, ver);
	if (res)
		goto out;

	res = fscrypt_set_gcm_key(tfm, raw);
	if (res)
		goto out;

	if (enc)
		res = fscrypt_derive_gcm_key(tfm, fek, nonce, iv, 1);
	else
		res = fscrypt_derive_gcm_key(tfm, nonce, fek, iv, 0);
out:
	crypto_free_aead(tfm);
	memzero_explicit(raw, (size_t)FS_MAX_KEY_SIZE);
	return res;
}

static int hmfs_sdp_keyindex_config(unsigned char ver,
						struct fscrypt_context *ctx,
						struct f2fs_sdp_fscrypt_context *sdp_ctx,
						struct fscrypt_info *crypt_info,
						int *keyindex)
{
	int res = 0;

	if (ver >= INLINE_CRYPTO_V2) {
		if (ver > INLINE_CRYPTO_V2)
			res = f2fs_do_get_keyindex(
				sdp_ctx->master_key_descriptor, keyindex, ver);
		else
			res = f2fs_do_get_keyindex(ctx->master_key_descriptor,
							keyindex, ver);
		if (res)
			return res;

		crypt_info->ci_key_index = *keyindex;
		/* keyindex range from 0 to 31 */
		if (crypt_info->ci_key_index < 0 || crypt_info->ci_key_index > 31) {
			pr_err("ece_key %s: %d\n", __func__, crypt_info->ci_key_index);
			return -EINVAL;
		}
	}

	return res;
}

static int
f2fs_get_ece_crypt_info_from_context(struct inode *inode,
							struct fscrypt_context *ctx,
							struct f2fs_sdp_fscrypt_context *sdp_ctx,
							struct fscrypt_info *crypt_info,
							int inherit_key, void *fs_data)
{
	int res;
	u32 flag = 0;
	u8  iv[FS_KEY_DERIVATION_IV_SIZE] = { 0x00 };
	u8  fek[FS_KEY_DERIVATION_CIPHER_SIZE];
	int keyindex;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);

	if (inherit_key) {
		if (sb->encryption_ver > INLINE_CRYPTO_V2)
			return -EINVAL;
		res = f2fs_do_get_fek(ctx->master_key_descriptor, ctx->nonce,
					fek, ctx->iv, 0, sb->encryption_ver);
		if (res)
			return res;
		memcpy(iv, ctx->iv, FS_KEY_DERIVATION_IV_SIZE);
	} else {
		res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
		if (res)
			return res;
		if (F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag)) {
			res = f2fs_do_get_fek(sdp_ctx->master_key_descriptor,
						sdp_ctx->nonce, fek, sdp_ctx->iv,
						0, sb->encryption_ver);
			if (res)
				return res;
		} else {
			get_random_bytes(fek, FS_KEY_DERIVATION_NONCE_SIZE);
			get_random_bytes(iv, FS_KEY_DERIVATION_IV_SIZE);
		}
	}

	res = hmfs_sdp_keyindex_config(sb->encryption_ver, ctx, sdp_ctx,
					crypt_info, &keyindex);
	if (res)
		return res;

	res = hmfs_derive_ctfm_from_fek(crypt_info, fek);
	if (res) {
		pr_err("hmfs_sdp %s: error %d (inode %lu) get file key fail!\n",
				__func__, res, inode->i_ino);
		return res;
	}

	if (F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag))
		return 0;

	memcpy(sdp_ctx->iv, iv,  FS_KEY_DERIVATION_IV_SIZE);
	res = f2fs_do_get_fek(sdp_ctx->master_key_descriptor, sdp_ctx->nonce,
				fek, sdp_ctx->iv, 1, sb->encryption_ver);
	if (res) {
		pr_err("hmfs_sdp %s: error %d (inode %lu) encrypt ece key failed!\n",
				__func__, res, inode->i_ino);
		return res;
	}

	return 0;
}

static int f2fs_get_sdp_ece_crypt_info(struct inode *inode, void *fs_data)
{
	struct fscrypt_context ctx;
	struct f2fs_sdp_fscrypt_context sdp_ctx;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	struct fscrypt_info *crypt_info = NULL;
	int inherit_key = 0;
	int res;
	u32 flag;

	res = sb->s_sdp_cop->get_sdp_context(inode, &sdp_ctx,
			sizeof(sdp_ctx), fs_data);
	if (res != sizeof(sdp_ctx))
		return -EINVAL;

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return -EINVAL;

	crypt_info = kmem_cache_alloc(fscrypt_info_cachep, GFP_NOFS);
	if (!crypt_info)
		return -ENOMEM;

	crypt_info->ci_flags = sdp_ctx.flags;
	crypt_info->ci_data_mode = sdp_ctx.contents_encryption_mode;
	crypt_info->ci_filename_mode = sdp_ctx.filenames_encryption_mode;
	crypt_info->ci_ctfm = NULL;
	crypt_info->ci_gtfm = NULL;
	crypt_info->ci_essiv_tfm = NULL;
	crypt_info->ci_key = NULL;
	crypt_info->ci_key_len = 0;
	crypt_info->ci_key_index = -1;
	memcpy(crypt_info->ci_master_key, sdp_ctx.master_key_descriptor,
		sizeof(crypt_info->ci_master_key));

	res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
	if (res)
		goto out;
	if (!F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag)
	    && i_size_read(inode))
		inherit_key = 1;

	res = f2fs_get_ece_crypt_info_from_context(inode, &ctx,
		&sdp_ctx, crypt_info, inherit_key, fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: get cypt info failed\n", __func__);
		goto out;
	}
	crypt_info->ci_hw_enc_flag  = F2FS_XATTR_SDP_ECE_ENABLE_FLAG;

	res = fscrypt_get_ece_metadata(
		inode, crypt_info, fs_data,
		!F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag));
	if (res)
		goto out;

	if (cmpxchg(&inode->i_crypt_info, NULL, crypt_info) == NULL)
		crypt_info = NULL;

	if (F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag))
		goto out;

	/* should set sdp context back for getting the nonce */
	res = sb->s_sdp_cop->update_sdp_context(inode, &sdp_ctx,
		sizeof(struct f2fs_sdp_fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) set sdp ctx failed res %d.\n",
				__func__, inode->i_ino, res);
		goto out;
	}

	res = hmfs_inode_set_sdp_encryption_flags(inode, fs_data,
			F2FS_XATTR_SDP_ECE_ENABLE_FLAG);
	if (res) {
		pr_err("hmfs_sdp %s: set sdp crypt flag failed! need to check!\n",
				__func__);
		goto out;
	}

	/* clear the ce crypto info */
	memset(ctx.nonce, 0, FS_KEY_DERIVATION_CIPHER_SIZE);
	memset(ctx.iv, 0, FS_KEY_DERIVATION_IV_SIZE);

	res = sb->s_sdp_cop->update_context(inode, &ctx,
		sizeof(struct fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) update ce ctx failed res %d.\n",
				__func__, inode->i_ino, res);
		goto out;
	}

out:
	if (res == -ENOKEY)
		res = 0;
	f2fs_put_crypt_info(crypt_info);
	return res;
}

static int f2fs_do_get_sece_fek(struct f2fs_sdp_fscrypt_context *ctx, u8 *fek,
				u8 haspubkey, unsigned char ver)
{
	int res = -ENOKEY;
	struct crypto_aead *tfm = NULL;
	u8 raw[FS_SDP_ECC_PRIV_KEY_SIZE + FS_SDP_ECC_PUB_KEY_SIZE];
	u8 shared_key[FS_AES_256_GCM_KEY_SIZE];
	int size;
	int offset_len;

	tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm))
		return (int)PTR_ERR(tfm);

	res = f2fs_do_get_keyring_payload(ctx->master_key_descriptor, raw,
						&size, haspubkey == 1, ver);
	if (res)
		goto out;

	if (!haspubkey) {
		offset_len = (size == FS_SDP_ECC_PUB_KEY_SIZE) ? 0 : FS_SDP_ECC_PRIV_KEY_SIZE;
		res = hmfs_get_file_pubkey_shared_secret(ECC_CURVE_NIST_P256,
				raw + offset_len, FS_SDP_ECC_PUB_KEY_SIZE,
				ctx->file_pub_key, FS_SDP_ECC_PUB_KEY_SIZE,
				shared_key, FS_AES_256_GCM_KEY_SIZE);
	}
	else
		res = hmfs_get_shared_secret(ECC_CURVE_NIST_P256,
				raw, FS_SDP_ECC_PRIV_KEY_SIZE,
				ctx->file_pub_key, FS_SDP_ECC_PUB_KEY_SIZE,
				shared_key, FS_AES_256_GCM_KEY_SIZE);
	if (res)
		goto out;

	res = fscrypt_set_gcm_key(tfm, shared_key);
	if (res)
		goto out;

	if (haspubkey)
		res = fscrypt_derive_gcm_key(tfm, ctx->nonce, fek, ctx->iv, 0);
	else
		res = fscrypt_derive_gcm_key(tfm, fek, ctx->nonce, ctx->iv, 1);
out:
	crypto_free_aead(tfm);
	memzero_explicit(raw, (size_t)(FS_SDP_ECC_PRIV_KEY_SIZE
				+ FS_SDP_ECC_PUB_KEY_SIZE));
	memzero_explicit(shared_key, (size_t)(FS_AES_256_GCM_KEY_SIZE));
	return res;
}

static inline int f2fs_do_set_sece_fek(struct f2fs_sdp_fscrypt_context *sdp_ctx,
						u8 *fek, unsigned char ver)
{
	return f2fs_do_get_sece_fek(sdp_ctx, fek, 0, ver);
}

static int hmfs_enabled_sece_get_fek(unsigned char ver,
					struct f2fs_sdp_fscrypt_context *sdp_ctx,
					u8 *fek)
{
	int res;

	if (ver > INLINE_CRYPTO_V2)
		res = f2fs_do_get_fek(sdp_ctx->master_key_descriptor,
					sdp_ctx->nonce, fek, sdp_ctx->iv, 0, ver);
	else
		res = f2fs_do_get_sece_fek(sdp_ctx, fek, 1, ver);

	return res;
}

static int
f2fs_get_sece_crypt_info_from_context(struct inode *inode,
						struct fscrypt_context *ctx,
						struct f2fs_sdp_fscrypt_context *sdp_ctx,
						struct fscrypt_info *crypt_info,
						int inherit_key, void *fs_data)
{
	int res = 0;
	u8  iv[FS_KEY_DERIVATION_IV_SIZE] = { 0x00 };
	u8  fek[FS_KEY_DERIVATION_CIPHER_SIZE];
	u32 flag = 0;
	int keyindex;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	unsigned char ver = sb->encryption_ver;

	if (inherit_key) {
		if (ver > INLINE_CRYPTO_V2)
			return -EINVAL;
		res = f2fs_do_get_fek(ctx->master_key_descriptor, ctx->nonce,
					fek, ctx->iv, 0, ver);
		if (res)
			goto out;
		memcpy(iv, ctx->iv, FS_KEY_DERIVATION_IV_SIZE);
	} else {
		res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
		if (res)
			goto out;
		if (F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag)) {
			res = hmfs_enabled_sece_get_fek(ver, sdp_ctx, fek);
			if (res)
				goto out;
		} else {
			get_random_bytes(fek, FS_KEY_DERIVATION_NONCE_SIZE);
			get_random_bytes(iv, FS_KEY_DERIVATION_IV_SIZE);
		}
	}

	res = hmfs_sdp_keyindex_config(ver, ctx, sdp_ctx, crypt_info,
					&keyindex);
	if (res)
		goto out;

	res = hmfs_derive_ctfm_from_fek(crypt_info, fek);
	if (res) {
		pr_err("hmfs_sdp %s: error %d (inode %lu) get file key fail!\n",
				__func__, res, inode->i_ino);
		goto out;
	}

	if (F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag))
		goto out;

	memcpy(sdp_ctx->iv, iv,  FS_KEY_DERIVATION_IV_SIZE);
	if (sb->encryption_ver > INLINE_CRYPTO_V2)
		res = f2fs_do_get_fek(sdp_ctx->master_key_descriptor,
					sdp_ctx->nonce, fek, sdp_ctx->iv, 1, ver);
	else
		res = f2fs_do_set_sece_fek(sdp_ctx, fek, ver);
	if (res) {
		pr_err("hmfs_sdp %s: error %d (inode %lu) encrypt ece key failed\n",
				__func__, res, inode->i_ino);
	}
out:
	memzero_explicit(fek, (size_t)FS_MAX_KEY_SIZE);
    return res;
}

static int f2fs_get_sdp_sece_crypt_info(struct inode *inode, void *fs_data)
{
	struct fscrypt_context ctx;
	struct f2fs_sdp_fscrypt_context sdp_ctx;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	struct fscrypt_info *crypt_info = NULL;
	int inherit_key = 0;
	int res;
	u32 flag;

	res = sb->s_sdp_cop->get_sdp_context(inode, &sdp_ctx,
			sizeof(sdp_ctx), fs_data);
	if (res != sizeof(sdp_ctx))
		return -EINVAL;

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return -EINVAL;

	crypt_info = kmem_cache_alloc(fscrypt_info_cachep, GFP_NOFS);
	if (!crypt_info)
		return -ENOMEM;

	crypt_info->ci_flags = sdp_ctx.flags;
	crypt_info->ci_data_mode = sdp_ctx.contents_encryption_mode;
	crypt_info->ci_filename_mode = sdp_ctx.filenames_encryption_mode;
	crypt_info->ci_ctfm = NULL;
	crypt_info->ci_gtfm = NULL;
	crypt_info->ci_essiv_tfm = NULL;
	crypt_info->ci_key = NULL;
	crypt_info->ci_key_len = 0;
	crypt_info->ci_key_index = -1;
	memcpy(crypt_info->ci_master_key, sdp_ctx.master_key_descriptor,
		sizeof(crypt_info->ci_master_key));

	res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
	if (res)
		goto out;
	if (!F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag)
	    && i_size_read(inode))
		inherit_key = 1;

	res = f2fs_get_sece_crypt_info_from_context(inode, &ctx,
		&sdp_ctx, crypt_info, inherit_key, fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: get cypt info failed\n", __func__);
		goto out;
	}
	crypt_info->ci_hw_enc_flag  = F2FS_XATTR_SDP_SECE_ENABLE_FLAG;

	res = fscrypt_get_sece_metadata(
		inode, crypt_info, fs_data,
		!F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag));
	if (res)
		goto out;

	if (cmpxchg(&inode->i_crypt_info, NULL, crypt_info) == NULL)
		crypt_info = NULL;

	if (F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag))
		goto out;

	/*should set sdp context back for getting the nonce*/
	res = sb->s_sdp_cop->update_sdp_context(inode, &sdp_ctx,
		sizeof(struct f2fs_sdp_fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) set sdp ctx failed res %d.\n",
				__func__, inode->i_ino, res);
		goto out;
	}

	res = hmfs_inode_set_sdp_encryption_flags(inode, fs_data,
			F2FS_XATTR_SDP_SECE_ENABLE_FLAG);
	if (res) {
		pr_err("hmfs_sdp %s:set sdp crypt flag failed! need to check!\n",
				__func__);
		goto out;
	}

	/*clear the ce crypto info*/
	memset(ctx.nonce, 0, FS_KEY_DERIVATION_CIPHER_SIZE);
	memset(ctx.iv, 0, FS_KEY_DERIVATION_IV_SIZE);

	res = sb->s_sdp_cop->update_context(inode, &ctx,
		sizeof(struct fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) update ce ctx failed res %d.\n",
				__func__, inode->i_ino, res);
		goto out;
	}

out:
	if (res == -ENOKEY)
		res = 0;
	f2fs_put_crypt_info(crypt_info);
	return res;
}

int hmfs_change_to_sdp_crypto(struct inode *inode, void *fs_data)
{
	struct fscrypt_info *ci_info = inode->i_crypt_info;
	struct fscrypt_context ctx;
	struct f2fs_sdp_fscrypt_context sdp_ctx;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	int res, inherit = 0;
	u32 flag;

	if (!ci_info)
		return -EINVAL;

	res = sb->s_sdp_cop->get_sdp_context(inode, &sdp_ctx,
			sizeof(sdp_ctx), fs_data);
	if (res != sizeof(sdp_ctx))
		return -EINVAL;

	res = inode->i_sb->s_cop->get_context(inode, &ctx,
			sizeof(ctx));
	if (res != sizeof(ctx))
		return -EINVAL;

	/* file is not null, ece should inherit ce nonece iv,
	 * sece also support
	 */
	if (i_size_read(inode))
		inherit = 1;

	if (sdp_ctx.sdpclass == FSCRYPT_SDP_ECE_CLASS)
		res = f2fs_get_ece_crypt_info_from_context(inode, &ctx,
				&sdp_ctx, ci_info, inherit, fs_data);
	else if (sdp_ctx.sdpclass == FSCRYPT_SDP_SECE_CLASS)
		res = f2fs_get_sece_crypt_info_from_context(inode, &ctx,
				&sdp_ctx, ci_info, inherit, fs_data);
	else
		res = -EINVAL;
	if (res) {
		pr_err("hmfs_sdp %s: get cypt info failed\n", __func__);
		return res;
	}

	memcpy(ci_info->ci_master_key, sdp_ctx.master_key_descriptor,
		FS_KEY_DESCRIPTOR_SIZE);
	ci_info->ci_flags = sdp_ctx.flags;
	res = sb->s_sdp_cop->update_sdp_context(inode, &sdp_ctx,
		sizeof(struct f2fs_sdp_fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) set sdp ctx failed, res(%d)\n",
				__func__, inode->i_ino, res);
		goto out;
	}

	switch (sdp_ctx.sdpclass) {
	case FSCRYPT_SDP_ECE_CLASS:
		res = fscrypt_new_ece_metadata(inode, ci_info, fs_data);
		break;
	case FSCRYPT_SDP_SECE_CLASS:
		res = fscrypt_new_sece_metadata(inode, ci_info, fs_data);
		break;
	default:
		break;
	}
	if (res)
		goto out;

	res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
	if (res)
		goto out;

	if (HMFS_INODE_IS_CONFIG_SDP_ECE_ENCRYPTION(flag)) {
		res = hmfs_inode_set_sdp_encryption_flags(inode,
			fs_data, F2FS_XATTR_SDP_ECE_ENABLE_FLAG);
		ci_info->ci_hw_enc_flag = F2FS_XATTR_SDP_ECE_ENABLE_FLAG;
	} else {
		res = hmfs_inode_set_sdp_encryption_flags(inode,
			fs_data, F2FS_XATTR_SDP_SECE_ENABLE_FLAG);
		ci_info->ci_hw_enc_flag = F2FS_XATTR_SDP_SECE_ENABLE_FLAG;
	}
	if (res) {
		pr_err("hmfs_sdp %s: set crypt flags failed! need to check!\n",
				__func__);
		goto out;
	}

	/* clear the ce crypto info */
	memset(ctx.nonce, 0, FS_KEY_DERIVATION_CIPHER_SIZE);
	memset(ctx.iv, 0, FS_KEY_DERIVATION_IV_SIZE);

	res = sb->s_sdp_cop->update_context(inode, &ctx,
		sizeof(struct fscrypt_context), fs_data);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) update ce ctx failed res(%d)\n",
				__func__, inode->i_ino, res);
		goto out;
	}

	return 0;
out:
	fscrypt_put_encryption_info(inode);
	return res;
}

static int f2fs_get_sdp_crypt_info(struct inode *inode, void *fs_data)
{
	int res = 0;
	struct fscrypt_info *ci_info = inode->i_crypt_info;
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	u32 flag = 0;

	res = hmfs_inode_get_sdp_encrypt_flags(inode, fs_data, &flag);
	if (res)
		return res;

	if (ci_info && F2FS_INODE_IS_ENABLED_SDP_ENCRYPTION(flag)) {
		if (F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag)) {
			res = fscrypt_check_sece_metadata(inode, ci_info,
								fs_data);
			if (unlikely(res)) {
				pr_err("hmfs_sdp %s: fscrypt_open_sece_metadata_config failed, res = %d\n",
					__func__, res);
				return res;
			}
		}
		return hmfs_inode_check_sdp_keyring(
			ci_info->ci_master_key, 1, sb->encryption_ver);
	}

	/* means should change from ce to sdp crypto */
	if (ci_info && HMFS_INODE_IS_CONFIG_SDP_ENCRYPTION(flag))
		return hmfs_change_to_sdp_crypto(inode, fs_data);

	if (HMFS_INODE_IS_CONFIG_SDP_ECE_ENCRYPTION(flag))
		return f2fs_get_sdp_ece_crypt_info(inode, fs_data);

	return f2fs_get_sdp_sece_crypt_info(inode, fs_data);
}

int hmfs_get_crypt_keyinfo(struct inode *inode, void *fs_data, int *flag)
{
	int res;
	u32 sdpflag;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	/* 0 for getting original ce crypt info, otherwise be 1 */
	*flag = 0;
#ifdef CONFIG_HWDPS
	if (!inode->i_crypt_info || (inode->i_crypt_info &&
		((inode->i_crypt_info->ci_hw_enc_flag &
		HWDPS_ENABLE_FLAG) != 0))) {
		res = hwdps_get_context(inode);
		if (res == -EOPNOTSUPP)
			goto get_sdp_encryption_info;
		else if (res) {
			*flag = 1; // enabled
			return -EACCES;
		} else {
			*flag = 1; // enabled
			return 0;
		}
	}
get_sdp_encryption_info:
#endif
	if (!S_ISREG(inode->i_mode))
		return 0;

	down_write(&F2FS_I(inode)->i_sdp_sem);
	res = sbi->s_sdp_cop->get_sdp_encrypt_flags(inode, fs_data, &sdpflag);
	if (res)
		goto unlock;

	if (!test_hw_opt(sbi, SDP_ENCRYPT)) {
		/* get sdp crypt info failed since not enable SDP_ENCRYPT */
		if (F2FS_INODE_IS_SDP_ENCRYPTED(sdpflag))
			res = -ENOKEY;
		/* should get original ce crypt info */
		goto unlock;
	}

	/* get sdp crypt info */
	if (F2FS_INODE_IS_SDP_ENCRYPTED(sdpflag)) {
		*flag = 1;
		res = f2fs_get_sdp_crypt_info(inode, fs_data);
		if (res)
			pr_err("hmfs_sdp %s: inode(%lu) get keyinfo failed res(%d)\n",
					__func__, inode->i_ino, res);
	}
unlock:
	up_write(&F2FS_I(inode)->i_sdp_sem);
	return res;
}

int hmfs_is_file_sdp_encrypted(struct inode *inode)
{
	u32 flag;

	if (F2FS_I_SB(inode)->s_sdp_cop->get_sdp_encrypt_flags(inode, NULL, &flag))
		return 0;
	return F2FS_INODE_IS_SDP_ENCRYPTED(flag);
}

#endif

