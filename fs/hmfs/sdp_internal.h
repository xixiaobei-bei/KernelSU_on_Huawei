/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Internal functions for per-f2fs sdp(sensitive data protection).
 * Create: 2017-11-10
 * History: 2020-10-10 add hwdps
 */
#ifndef __SDP_INTERNAL_H__
#define __SDP_INTERNAL_H__

#include <linux/fscrypt_common.h>
#include <linux/types.h>
#include <crypto/kpp.h>
#include <crypto/ecdh.h>

#if F2FS_FS_SDP_ENCRYPTION

#define DEBUG_F2FS_FS_SDP_ENCRYPTION	(0)
#if DEBUG_F2FS_FS_SDP_ENCRYPTION
#define pr_sdp_info pr_info
#else
#define pr_sdp_info(fmt, ...) \
	no_printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#endif

#define F2FS_XATTR_SDP_ECE_ENABLE_FLAG	(0x01)
#define HMFS_XATTR_SDP_ECE_CONFIG_FLAG	(0x02)
#define F2FS_XATTR_SDP_SECE_ENABLE_FLAG	(0x04)
#define HMFS_XATTR_SDP_SECE_CONFIG_FLAG	(0x08)

#define F2FS_INODE_IS_SDP_ENCRYPTED(flag) (flag & 0x0f)

#define HMFS_INODE_IS_CONFIG_SDP_ECE_ENCRYPTION(flag)	\
	((flag) & (HMFS_XATTR_SDP_ECE_CONFIG_FLAG))
#define HMFS_INODE_IS_CONFIG_SDP_SECE_ENCRYPTION(flag)	\
	((flag) & (HMFS_XATTR_SDP_SECE_CONFIG_FLAG))
#define HMFS_INODE_IS_CONFIG_SDP_ENCRYPTION(flag)	\
	(((flag) & (HMFS_XATTR_SDP_SECE_CONFIG_FLAG))	\
	 || ((flag) & (HMFS_XATTR_SDP_ECE_CONFIG_FLAG)))
#define F2FS_INODE_IS_ENABLED_SDP_ECE_ENCRYPTION(flag)	\
	((flag) & (F2FS_XATTR_SDP_ECE_ENABLE_FLAG))
#define F2FS_INODE_IS_ENABLED_SDP_SECE_ENCRYPTION(flag)	\
	((flag) & (F2FS_XATTR_SDP_SECE_ENABLE_FLAG))
#define F2FS_INODE_IS_ENABLED_SDP_ENCRYPTION(flag)	\
	(((flag) & (F2FS_XATTR_SDP_ECE_ENABLE_FLAG))	\
	 || ((flag) & (F2FS_XATTR_SDP_SECE_ENABLE_FLAG)))

#define FSCRYPT_INVALID_CLASS		(0)
#define FSCRYPT_CE_CLASS			(1)
#define FSCRYPT_SDP_ECE_CLASS		(2)
#define FSCRYPT_SDP_SECE_CLASS		(3)
#define FSCRYPT_DPS_CLASS		4

#define FS_SDP_ECC_PUB_KEY_SIZE			(64)
#define FS_SDP_ECC_PRIV_KEY_SIZE		(32)
#ifndef FS_AES_256_GCM_KEY_SIZE
#define FS_AES_256_GCM_KEY_SIZE			(32)
#endif
#define FS_AES_256_CBC_KEY_SIZE			(32)
#define FS_AES_256_CTS_KEY_SIZE			(32)
#define FS_AES_256_XTS_KEY_SIZE			(64)
#ifndef FS_ENCRYPTION_CONTEXT_FORMAT_V2
#define FS_ENCRYPTION_CONTEXT_FORMAT_V2	(2)
#endif

#define FS_KEY_INDEX_OFFSET				(63)
#define INLINE_CRYPTO_V2				2

struct fscrypt_sdp_key {
	u32 version;
	u32 sdpclass;
	u32 mode;
	u8 raw[FS_MAX_KEY_SIZE];
	u32 size;
	u8 pubkey[FS_MAX_KEY_SIZE];
	u32 pubkeysize;
} __packed;

struct f2fs_sdp_fscrypt_context {
	u8 version;
	u8 sdpclass;
	u8 format;
	u8 contents_encryption_mode;
	u8 filenames_encryption_mode;
	u8 flags;
	u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
	u8 nonce[FS_KEY_DERIVATION_CIPHER_SIZE];
	u8 iv[FS_KEY_DERIVATION_IV_SIZE];
	u8 file_pub_key[FS_SDP_ECC_PUB_KEY_SIZE];
} __packed;

/*
 * policy.c internal functions
 */
int hmfs_inode_get_sdp_encrypt_flags(struct inode *inode, void *fs_data,
									 u32 *flag);
int hmfs_inode_set_sdp_encryption_flags(struct inode *inode, void *fs_data,
										u32 sdp_enc_flag);
int hmfs_inode_check_sdp_keyring(u8 *descriptor, int enforce,
					unsigned char ver);

/*
 * keyinfo.c internal functions
 */
int hmfs_change_to_sdp_crypto(struct inode *inode, void *fs_data);

/*
 * ecdh.c internal functions
 */

/**
 * hmfs_get_file_pubkey_shared_secret() - Use ECDH to generate file public key
 * and shared secret.
 *
 * @cuive_id:          Curve id, only ECC_CURVE_NIST_P256 now
 * @dev_pub_key:       Device public key
 * @dev_pub_key_len:   The length of @dev_pub_key in bytes
 * @file_pub_key[out]: File public key to be generated
 * @file_pub_key_len:  The length of @file_pub_key in bytes
 * @shared secret[out]:Compute shared secret with file private key
 * and device public key
 * @shared_secret_len: The length of @shared_secret in bytes
 *
 * Return: 0 for success, error code in case of error.
 *         The contents of @file_pub_key and @shared_secret are
 *         undefined in case of error.
 */
int hmfs_get_file_pubkey_shared_secret(unsigned int curve_id, const u8 *dev_pub_key,
								  unsigned int dev_pub_key_len,
								  u8 *file_pub_key,
								  unsigned int file_pub_key_len,
								  u8 *shared_secret,
								  unsigned int shared_secret_len);

/**
 * hmfs_get_shared_secret() - Use ECDH to generate shared secret.
 *
 * @cuive_id:          Curve id, only ECC_CURVE_NIST_P256 now
 * @dev_privkey:       Device private key
 * @dev_privkey_len:   The length of @dev_privkey in bytes
 * @file_pub_key:      File public key
 * @file_pub_key_len:  The length of @file_pub_key in bytes
 * @shared secret[out]:Compute shared secret with file public key and device
 *                     private key
 * @shared_secret_len: The length of @shared_secret in bytes
 *
 * Return: 0 for success, error code in case of error.
 *         The content of @shared_secret is undefined in case of error.
 */
int hmfs_get_shared_secret(unsigned int curve_id, const u8 *dev_privkey,
					  unsigned int dev_privkey_len, const u8 *file_pub_key,
					  unsigned int file_pub_key_len, u8 *shared_secret,
					  unsigned int shared_secret_len);
#endif
#endif

