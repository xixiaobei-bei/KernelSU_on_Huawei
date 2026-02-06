/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_dentry_limitation.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lvhao13@huawei.com
 * Create: 2020-08-31
 *
 */

#include "adapter_dentry_limitation.h"

#include <linux/xattr.h>

#include "hmdfs_dentryfile.h"

#define DIRECTORY_TYPE_XATTR_KEY "user.hmdfs_directory_type"

uint8_t hmdfs_adapter_read_dentry_flag(struct inode *inode)
{
	uint8_t ret = ADAPTER_OTHER_DENTRY_FLAG;
	struct dentry *dentry = d_find_alias(inode);

	if (!dentry)
		return ret;

	if (__vfs_getxattr(dentry, inode, DIRECTORY_TYPE_XATTR_KEY, &ret,
			   sizeof(ret)) < 0)
		ret = ADAPTER_OTHER_DENTRY_FLAG;

	dput(dentry);
	return ret;
}

int hmdfs_adapter_persist_dentry_flag(struct dentry *dentry,
				      __u8 directory_type)
{
	int err;

	if (!d_inode(dentry))
		return -EINVAL;

	err = __vfs_setxattr(dentry, d_inode(dentry), DIRECTORY_TYPE_XATTR_KEY,
			     &directory_type, sizeof(directory_type),
			     XATTR_CREATE);
	if (err && err != -EEXIST)
		hmdfs_err("failed to setxattr, err=%d", err);

	return err;
}

int adapter_identify_dentry_flag(struct inode *dir, struct dentry *dentry,
				 struct dentry *lower_dentry)
{
	if (hmdfs_adapter_read_dentry_flag(hmdfs_i(dir)->lower_inode) ==
	    ADAPTER_PHOTOKIT_DENTRY_FLAG) {
		hmdfs_adapter_persist_dentry_flag(lower_dentry,
						  ADAPTER_PHOTOKIT_DENTRY_FLAG);
		hmdfs_i(d_inode(dentry))->adapter_dentry_flag =
			ADAPTER_PHOTOKIT_DENTRY_FLAG;
		return ADAPTER_PHOTOKIT_DENTRY_FLAG;
	}
	hmdfs_i(d_inode(dentry))->adapter_dentry_flag =
		ADAPTER_OTHER_DENTRY_FLAG;
	hmdfs_adapter_persist_dentry_flag(lower_dentry,
					  ADAPTER_OTHER_DENTRY_FLAG);
	return ADAPTER_OTHER_DENTRY_FLAG;
}
