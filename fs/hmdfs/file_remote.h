// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * file_remote.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: houtao1@huawei.com
 * Create: 2021-01-14
 *
 */
#ifndef HMDFS_FILE_REMOTE_H
#define HMDFS_FILE_REMOTE_H

#include <linux/fs.h>
#include <linux/uio.h>

#include "hmdfs.h"
#include "comm/connection.h"

void hmdfs_remote_del_wr_opened_inode(struct hmdfs_peer *conn,
				      struct hmdfs_inode_info *info);

void hmdfs_remote_add_wr_opened_inode_nolock(struct hmdfs_peer *conn,
					     struct hmdfs_inode_info *info);

ssize_t hmdfs_file_write_iter_remote_nocheck(struct kiocb *iocb,
					     struct iov_iter *iter);

#endif
