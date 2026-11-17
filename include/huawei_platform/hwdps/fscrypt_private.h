/* SPDX-License-Identifier: GPL-2.0 */
/*
 * fscrypt_private.h
 *
 * Copyright (C) 2015, Google, Inc.
 *
 * This contains encryption key functions.
 *
 * Written by Michael Halcrow, Ildar Muslukhov, and Uday Savagaonkar, 2015.
 */

#ifndef _FSCRYPT_PRIVATE_H
#define _FSCRYPT_PRIVATE_H

#define __FS_HAS_ENCRYPTION 1
#include <linux/fscrypt.h>
#include <crypto/hash.h>

#define FS_IV_SIZE                      16
#define FS_ENCRYPTION_CONTEXT_FORMAT_V2		2

typedef enum {
	FS_DECRYPT = 0,
	FS_ENCRYPT,
} fscrypt_direction_t;

#define FS_CTX_REQUIRES_FREE_ENCRYPT_FL		0x00000001
#define FS_CTX_HAS_BOUNCE_BUFFER_FL		0x00000002
#define FS_AES_256_GCM_KEY_SIZE         32

struct fscrypt_completion_result {
        struct completion completion;
        int res;
};

#define DECLARE_FS_COMPLETION_RESULT(ecr) \
        struct fscrypt_completion_result ecr = { \
                COMPLETION_INITIALIZER_ONSTACK((ecr).completion), 0 }

static inline bool fscrypt_valid_enc_modes(u32 contents_mode,
					   u32 filenames_mode)
{
	if (contents_mode == FS_ENCRYPTION_MODE_AES_128_CBC &&
	    filenames_mode == FS_ENCRYPTION_MODE_AES_128_CTS)
		return true;

	if (contents_mode == FS_ENCRYPTION_MODE_AES_256_XTS &&
	    filenames_mode == FS_ENCRYPTION_MODE_AES_256_CTS)
		return true;

	return false;
}

/* crypto.c */
/* expose fscrypt_info_cachep to f2fs */
// extern struct kmem_cache *fscrypt_info_cachep;
extern int fscrypt_initialize(unsigned int cop_flags);
extern int fscrypt_do_page_crypto(const struct inode *inode,
				  fscrypt_direction_t rw, u64 lblk_num,
				  struct page *src_page,
				  struct page *dest_page,
				  unsigned int len, unsigned int offs,
				  gfp_t gfp_flags);
extern struct page *fscrypt_alloc_bounce_page(struct fscrypt_ctx *ctx,
					      gfp_t gfp_flags);
extern const struct dentry_operations fscrypt_d_ops;

extern void __printf(3, 4) __cold
fscrypt_msg(struct super_block *sb, const char *level, const char *fmt, ...);

#define fscrypt_warn(sb, fmt, ...)		\
	fscrypt_msg(sb, KERN_WARNING, fmt, ##__VA_ARGS__)
#define fscrypt_err(sb, fmt, ...)		\
	fscrypt_msg(sb, KERN_ERR, fmt, ##__VA_ARGS__)

/* fname.c */
extern int fname_encrypt(struct inode *inode, const struct qstr *iname,
			 u8 *out, unsigned int olen);
extern bool fscrypt_fname_encrypted_size(const struct inode *inode,
					 u32 orig_len, u32 max_len,
					 u32 *encrypted_len_ret);

/* keyinfo.c */
extern void __exit fscrypt_essiv_cleanup(void);

#endif /* _FSCRYPT_PRIVATE_H */
