/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: filesystem cryption metadata mechanism
 * Author: 	laixinyi@hisilicon.com
 *		hebiao6@huawei.com
 * Create: 	2020-12-16
 */


#ifndef __FSCRYPT_METADATA_H__
#define __FSCRYPT_METADATA_H__

#include <linux/types.h>
#include <crypto/kpp.h>
#include <linux/fs.h>
#include <linux/fscrypt_common.h>

#include "hmfs.h"


int hmfs_get_metadata_context(struct inode *inode, void *ctx, size_t len,
			      void *fs_data);

int hmfs_get_ex_metadata_context(struct inode *inode, void *ctx, size_t len,
				  void *fs_data);

int hmfs_set_ex_metadata_context(struct inode *inode, const void *ctx,
				  size_t len, void *fs_data);

bool hmfs_encrypt_file_check(struct inode *inode);
enum encrypto_type hmfs_get_encrypt_type(struct inode *inode);

#endif

