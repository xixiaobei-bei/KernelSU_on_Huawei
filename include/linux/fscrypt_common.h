/*
 * fscrypt_common.h: common declarations for per-file encryption
 *
 * Copyright (C) 2015, Google, Inc.
 *
 * Written by Michael Halcrow, 2015.
 * Modified by Jaegeuk Kim, 2015.
 */

#ifndef _LINUX_FSCRYPT_COMMON_H
#define _LINUX_FSCRYPT_COMMON_H

#include <linux/key.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/dcache.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <uapi/linux/fs.h>

#define FS_CRYPTO_BLOCK_SIZE		16

/* Encryption parameters */
#define FS_IV_SIZE                     16
#define FS_KEY_DERIVATION_NONCE_SIZE           64
#define FS_KEY_DERIVATION_IV_SIZE              16
#define FS_KEY_DERIVATION_TAG_SIZE             16
#define FS_KEY_DERIVATION_CIPHER_SIZE          (64 + 16) /* nonce + tag */
#define METADATA_BYTE_IN_KDF                   16

/**
 * Encryption context for inode
 *
 * Protector format:
 *  1 byte: Protector format (2 = this version)
 *  1 byte: File contents encryption mode
 *  1 byte: File names encryption mode
 *  1 byte: Flags
 *  8 bytes: Master Key descriptor
 *  80 bytes: Encryption Key derivation nonce (encrypted)
 *  12 bytes: IV
 */
struct fscrypt_context {
        u8 format;
        u8 contents_encryption_mode;
        u8 filenames_encryption_mode;
        u8 flags;
        u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
        u8 nonce[FS_KEY_DERIVATION_CIPHER_SIZE];
        u8 iv[FS_KEY_DERIVATION_IV_SIZE];
} __packed;

/*
 * A pointer to this structure is stored in the file system's in-core
 * representation of an inode.
 */
struct fscrypt_info {
	u8 ci_data_mode;
	u8 ci_filename_mode;
	u8 ci_flags;
	struct crypto_skcipher *ci_ctfm;
	struct crypto_aead *ci_gtfm;
	struct crypto_cipher *ci_essiv_tfm;
	u8 ci_master_key[FS_KEY_DESCRIPTOR_SIZE];
	void *ci_key;
	int ci_key_len;
	int ci_key_index;
	u8 ci_metadata[METADATA_BYTE_IN_KDF];
	u8 ci_hw_enc_flag;
};

#ifdef CONFIG_HWDPS
#define HWDPS_XATTR_NAME "hwdps"
#define HWDPS_XATTR_ENABLE_FLAG_NEW 0x0020
#define HWDPS_XATTR_ENABLE_FLAG 0x0010
#endif

static inline void *fscrypt_ci_key(struct inode *inode)
{
#if IS_ENABLED(CONFIG_FS_ENCRYPTION)
	return inode->i_crypt_info->ci_key;
#else
	return NULL;
#endif
}

static inline int fscrypt_ci_key_len(struct inode *inode)
{
#if IS_ENABLED(CONFIG_FS_ENCRYPTION)
	return inode->i_crypt_info->ci_key_len;
#else
	return 0;
#endif
}

static inline int fscrypt_ci_key_index(struct inode *inode)
{
#if IS_ENABLED(CONFIG_FS_ENCRYPTION)
	return inode->i_crypt_info->ci_key_index;
#else
	return -1;
#endif
}

static inline u8 *fscrypt_ci_metadata(struct inode *inode)
{
#if IS_ENABLED(CONFIG_FS_ENCRYPTION)
	return inode->i_crypt_info->ci_metadata;
#else
	return NULL;
#endif
}

struct fscrypt_ctx {
	union {
		struct {
			struct page *bounce_page;	/* Ciphertext page */
			struct page *control_page;	/* Original page  */
		} w;
		struct {
			struct bio *bio;
			struct work_struct work;
		} r;
		struct list_head free_list;	/* Free list */
	};
	u8 flags;				/* Flags */
};

/**
 * For encrypted symlinks, the ciphertext length is stored at the beginning
 * of the string in little-endian format.
 */
struct fscrypt_symlink_data {
	__le16 len;
	char encrypted_path[1];
} __packed;

struct fscrypt_str {
	unsigned char *name;
	u32 len;
};

struct fscrypt_name {
	const struct qstr *usr_fname;
	struct fscrypt_str disk_name;
	u32 hash;
	u32 minor_hash;
	struct fscrypt_str crypto_buf;
};

#define FSTR_INIT(n, l)		{ .name = n, .len = l }
#define FSTR_TO_QSTR(f)		QSTR_INIT((f)->name, (f)->len)
#define fname_name(p)		((p)->disk_name.name)
#define fname_len(p)		((p)->disk_name.len)

/*
 * fscrypt superblock flags
 */
#define FS_CFLG_OWN_PAGES (1U << 1)

/*
 * crypto opertions for filesystems
 */
struct fscrypt_operations {
	unsigned int flags;
	const char *key_prefix;
	int (*get_context)(struct inode *, void *, size_t);
	int (*set_context)(struct inode *, const void *, size_t, void *);
	bool (*dummy_context)(struct inode *);
	bool (*is_encrypted)(struct inode *);
	bool (*is_inline_encrypted)(struct inode *);
	bool (*empty_dir)(struct inode *);
	unsigned int max_namelen;
	int (*get_keyinfo)(struct inode *, void *, int *);
	int (*is_file_sdp_encrypted)(struct inode *);
#ifdef CONFIG_HWDPS
	int (*set_hwdps_attr)(struct inode *, const void *, size_t, void *);
	int (*set_hwdps_flags)(struct inode *, void *, u32 *);
	int (*update_hwdps_attr)(struct inode *, const void *, size_t, void *);
	int (*get_hwdps_attr)(struct inode *, void *, size_t, u32, struct page *);
	int (*get_hwdps_flags)(struct inode *, void *, u32 *);
#endif
	bool (*encrypt_file_check)(struct inode *);
	enum encrypto_type (*get_encrypt_type)(struct inode *);
	int (*get_metadata_context)(struct inode *, void *, size_t, void *);
	int (*get_ex_metadata_context)(struct inode *, void *, size_t, void *);
	int (*set_ex_metadata_context)(struct inode *, const void *, size_t, void *);
	int (*is_file_ece_encrypted)(struct inode *);
	/*
	 * This interface is just for compatibility of F2FS.
	 * It should be deleted after F2FS optimise
	 */
	int (*open_metadata)(struct inode *, struct fscrypt_info *);
	void (*get_generate_nonce)(u8 *, struct inode *, size_t);
};

#ifndef F2FS_FS_SDP_ENCRYPTION
#define F2FS_FS_SDP_ENCRYPTION 1
#endif

#define CI_KEY_LEN_NEW 48
#define FILE_ENCRY_TYPE_BEGIN_BIT 8
#define FILE_ENCRY_TYPE_MASK 0xFF

enum encrypto_type {
	PLAIN = 0,
	CD,
	ECE,
	SECE,
};

extern int fscrypt_generate_metadata_nonce(u8 *nonce,
				struct inode *inode, size_t len);
extern int fscrypt_get_metadata(struct inode *inode,
				 struct fscrypt_info *ci_info);
extern int fscrypt_new_ece_metadata(struct inode *inode,
			struct fscrypt_info *ci_info, void *fs_data);
extern int fscrypt_new_sece_metadata(struct inode *inode,
			struct fscrypt_info *ci_info, void *fs_data);
extern int fscrypt_get_ece_metadata(struct inode *inode,
			struct fscrypt_info *ci_info, void *fs_data, bool create);
extern int fscrypt_get_sece_metadata(struct inode *inode,
			struct fscrypt_info *ci_info, void *fs_data, bool create);
extern int fscrypt_check_sece_metadata(struct inode *inode,
			struct fscrypt_info *ci_info, void *fs_data);
extern int fscrypt_vm_op_check(struct inode *inode);
extern int fscrypt_lld_protect(const struct request *request);

extern int rw_begin(struct file *file);
extern void rw_finish(int read_write, struct file *file);
extern void fbe3_lock_in(void);
extern void fbe3_unlock_in(void);

#endif	/* _LINUX_FSCRYPT_COMMON_H */
