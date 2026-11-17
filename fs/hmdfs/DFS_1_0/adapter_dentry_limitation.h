/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_dentry_limitation.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lvhao13@huawei.com
 * Create: 2020-08-31
 *
 */

#ifndef HMDFS_ADAPTER_DIRECTORY_LIMITATION_H
#define HMDFS_ADAPTER_DIRECTORY_LIMITATION_H

#include <linux/fs.h>

#define ADAPTER_PHOTOKIT_DENTRY_FLAG 1
#define ADAPTER_OTHER_DENTRY_FLAG    0

uint8_t hmdfs_adapter_read_dentry_flag(struct inode *inode);
int hmdfs_adapter_persist_dentry_flag(struct dentry *dentry,
				      __u8 directory_type);

int adapter_identify_dentry_flag(struct inode *dir, struct dentry *dentry,
				 struct dentry *lower_dentry);
#endif
