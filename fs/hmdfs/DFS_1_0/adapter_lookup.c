/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_lookup.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 * Create: 2020-04-18
 *
 */

#include "adapter.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"

static int client_remote_delete(struct hmdfs_peer *con, __u32 fno,
				struct dentry *dentry)
{
	int ret = 0;
	char *relative_dir_path = NULL;
	struct adapter_req_info *req_info =
		kzalloc(sizeof(*req_info), GFP_KERNEL);
	struct adapter_sendmsg sm = {
		.data = req_info,
		.len = sizeof(*req_info),
		.operations = DELETE_REQUEST,
		.timeout = MSG_DEFAULT_TIMEOUT,
	};

	if (!req_info)
		return -ENOMEM;
	req_info->fno = fno;
	req_info->src_dno = con->sbi->local_info.iid;

	ret = client_sendmessage_request(con, &sm);
	if (ret == 0) {
		relative_dir_path =
			hmdfs_get_dentry_relative_path(dentry->d_parent);
		if (!relative_dir_path) {
			ret = -ENOMEM;
			goto out;
		}
		hmdfs_adapter_remove(con->sbi, relative_dir_path,
				     dentry->d_name.name);
		kfree(relative_dir_path);
	}

out:
	kfree(req_info);
	return ret;
}

int hmdfs_adapter_remote_unlink(struct hmdfs_peer *con, struct dentry *dentry)
{
	int ret = 0;
	struct hmdfs_inode_info *info = hmdfs_i(dentry->d_inode);
	__u32 fno = info->file_no;

	ret = client_remote_delete(con, fno, dentry);
	return ret;
}
