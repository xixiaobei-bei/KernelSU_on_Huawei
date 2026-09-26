/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Internal functions for per-f2fs sdp metadtata
 * Author: LAI Xinyi
 * Create: 2020-02-12
 */

#ifndef __SDP_METADATA_H__
#define __SDP_METADATA_H__

#include <linux/types.h>
#include <crypto/kpp.h>
#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <linux/fscrypt_common.h>

#include "f2fs.h"
#include "sdp_internal.h"

#define FPUBKEY_LEN PUBKEY_LEN

void generate_nonce(u8 *nonce, struct inode *inode, size_t len);
int f2fs_get_metadata_context(struct inode *inode, void *ctx, size_t len,
			      void *fs_data);
int f2fs_get_sdp_metadata_context(struct inode *inode, void *ctx, size_t len,
				  void *fs_data);
int f2fs_set_sdp_metadata_context(struct inode *inode, const void *ctx,
				  size_t len, void *fs_data);
int f2fs_update_sdp_metadata_context(struct inode *inode, const void *ctx,
				     size_t len, void *fs_data);

int fscrypt_open_metadata_config(struct inode *inode,
				 struct fscrypt_info *ci_info);
int fscrypt_open_sece_metadata_config(struct inode *inode,
				      struct fscrypt_info *ci_info,
				      void *fs_data);
int f2fs_get_sdp_ece_metadata(struct inode *inode, struct f2fs_sb_info *sb,
			      struct fscrypt_info *crypt_info, void *fs_data,
			      int flag);
int f2fs_get_sdp_sece_metadata(struct inode *inode, struct f2fs_sb_info *sb,
			       struct fscrypt_info *crypt_info, void *fs_data,
			       u32 flag);
int f2fs_update_metadata_sdp_crypto(struct inode *inode, void *fs_data,
				    struct f2fs_sdp_fscrypt_context *sdp_ctx);

bool f2fs_encrypt_file_check(struct inode *inode);
enum encrypto_type f2fs_get_encrypt_type(struct inode *inode);


#endif
