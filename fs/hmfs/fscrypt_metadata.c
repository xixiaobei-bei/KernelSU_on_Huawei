/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: filesystem cryption metadata mechanism
 * Author: 	laixinyi@hisilicon.com
 *		hebiao6@huawei.com
 * Create: 	2020-12-16
 */

#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <linux/random.h>
#include "xattr.h"
#include "sdp_internal.h"
#include "fscrypt_metadata.h"

int hmfs_get_metadata_context(struct inode *inode, void *ctx,
					size_t len, void *fs_data)
{
	return hmfs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION_METADATA,
			     F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, ctx, len,
			     fs_data);
}

int hmfs_get_ex_metadata_context(struct inode *inode, void *ctx,
					size_t len, void *fs_data)
{
	int ret = hmfs_getxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION_METADATA,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, ctx, len,
				fs_data);
	if (unlikely(ret != len))
		pr_err("%s: error %d ino = %llu\r\n", __func__,
					ret, inode->i_ino);

	return ret;
}

int hmfs_set_ex_metadata_context(struct inode *inode, const void *ctx,
				  size_t len, void *fs_data)
{
	int ret = hmfs_setxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION_METADATA,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, ctx, len,
				fs_data, XATTR_CREATE);
	if (ret == -EEXIST)
		ret = hmfs_setxattr(inode, F2FS_XATTR_INDEX_ENCRYPTION_METADATA,
				F2FS_XATTR_NAME_ENCRYPTION_CONTEXT, ctx, len,
				fs_data, XATTR_REPLACE);
	if (unlikely(ret))
		pr_err("%s: error %d ino = %llu\r\n", __func__,
					ret, inode->i_ino);

	return ret;
}

enum encrypto_type hmfs_get_encrypt_type(struct inode *inode)
{
	struct fscrypt_info *ci_info = inode->i_crypt_info;
	if (!ci_info)
		return PLAIN;
	if (ci_info->ci_hw_enc_flag == F2FS_XATTR_SDP_ECE_ENABLE_FLAG)
		return ECE;
	if (ci_info->ci_hw_enc_flag == F2FS_XATTR_SDP_SECE_ENABLE_FLAG)
		return SECE;
	return CD;
}

bool hmfs_encrypt_file_check(struct inode *inode)
{
	struct fscrypt_info *ci_info = NULL;
	struct f2fs_sb_info *sb = NULL;

	if (unlikely(!inode))
		return false;
	sb = F2FS_I_SB(inode);
	ci_info = inode->i_crypt_info;

	/* If ECE && Lock Screen return true, otherwise, return false */
	if ((sb->encryption_ver > 2) && ci_info) {
		if ((ci_info->ci_hw_enc_flag ==
		     F2FS_XATTR_SDP_ECE_ENABLE_FLAG) &&
		    hmfs_inode_check_sdp_keyring(ci_info->ci_master_key, 0,
						 sb->encryption_ver)) {
			pr_err("%s: ECE file check sdp keyring failed! \n",
			       __func__);
			return true;
		}
	}
	return false;
}

