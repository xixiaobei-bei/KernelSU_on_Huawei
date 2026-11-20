/* SPDX-License-Identifier: GPL-2.0
 *
 * adapter_inode.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 * Create: 2020-11-12
 *
 */

#include "adapter_client.h"
#include "hmdfs.h"

static int client_remote_set_fsize(struct hmdfs_peer *con, loff_t size,
				   uint64_t device_id, __u32 fno)
{
	int err;
	struct adapter_set_fsize_request *data;
	struct adapter_sendmsg sm = {
		.operations = SET_FSIZE_REQUEST,
		.timeout = MSG_DEFAULT_TIMEOUT,
		.len = sizeof(*data),
	};

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	sm.data = data;
	data->req_info.src_dno = device_id;
	data->req_info.fno = fno;
	data->lock_id = 0;
	data->size = size;

	err = client_sendmessage_request(con, &sm);
	kfree(data);
	return err;
}

static int adapter_setattr(struct dentry *dentry, struct iattr *ia)
{
	int err = 0;
	struct hmdfs_inode_info *info = hmdfs_i(d_inode(dentry));
	struct hmdfs_peer *conn = info->conn;
	struct inode *inode = d_inode(dentry);
	struct hmdfs_sb_info *sbi = inode->i_sb->s_fs_info;
	uint64_t device_id = sbi->local_info.iid;
	__u32 file_id = info->file_no;

	if (ia->ia_valid & ATTR_SIZE) {
		err = inode_newsize_ok(inode, ia->ia_size);
		if (err) {
			hmdfs_err("ino %lu can't truncate to given size",
				  inode->i_ino);
			goto out;
		}
		err = client_remote_set_fsize(conn, ia->ia_size, device_id,
					      file_id);
		if (err) {
			hmdfs_err("send msg failed, err=%d", err);
			goto out;
		}
		truncate_setsize(inode, ia->ia_size);
	}
out:
	return err;
}

const struct inode_operations hmdfs_adapter_remote_file_iops = {
	.setattr = adapter_setattr,
	.permission = hmdfs_permission,
};
