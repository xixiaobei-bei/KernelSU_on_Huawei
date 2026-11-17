/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * stash.h
 *
 * Copyright (c) 2020-2028 Huawei Technologies Co., Ltd.
 * Author: houtao1@huawei.com
 * Create: 2020-12-27
 *
 */

#ifndef HMDFS_STASH_H
#define HMDFS_STASH_H

#include "hmdfs.h"
#include "hmdfs_client.h"

extern void hmdfs_stash_add_node_evt_cb(void);

extern void hmdfs_exit_stash(struct hmdfs_sb_info *sbi);
extern int hmdfs_init_stash(struct hmdfs_sb_info *sbi);

extern int hmdfs_stash_writepage(struct hmdfs_peer *conn,
				 struct hmdfs_writepage_context *ctx);

extern void hmdfs_remote_init_stash_status(struct hmdfs_peer *conn,
					   struct inode *inode, umode_t mode);

#endif
