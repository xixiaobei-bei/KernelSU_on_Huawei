/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_dentry.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 * Create: 2020-04-23
 *
 */

#include "adapter_file_id_generator.h"
#include "adapter_protocol.h"
#include "comm/transport.h"
#include "dentry_syncer.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"

int hmdfs_adapter_remote_readdir(struct hmdfs_peer *con, struct file *file,
				 struct dir_context *ctx)
{
	int err = 0;
	unsigned long long dev_id = con->iid;
	char *account = con->sbi->local_info.account;
	char *relative_path = NULL;
	char *path = NULL;
	int relative_path_len;
	int path_len;

	relative_path = hmdfs_get_dentry_relative_path(file->f_path.dentry);
	if (!relative_path) {
		err = -ENOMEM;
		goto out_get_relative_path;
	}

	relative_path_len = strlen(relative_path);
	path_len = strlen(account) + relative_path_len + 2;
	path = kzalloc(path_len, GFP_KERNEL);
	if (!path) {
		err = -ENOMEM;
		goto out_get_path;
	}

	if (relative_path_len != 1)
		snprintf(path, path_len, "/%s%s", account, relative_path);
	else
		snprintf(path, path_len, "/%s", account);

	err = dentry_syncer_iterate(file, dev_id, path, ctx);
	trace_hmdfs_adapter_remote_readdir(account, relative_path, err);

	kfree(path);
out_get_path:
	kfree(relative_path);
out_get_relative_path:
	return err;
}

struct hmdfs_lookup_ret *hmdfs_adapter_remote_lookup(struct hmdfs_peer *con,
						     const char *relative_path,
						     const char *d_name)
{
	int ret = 0;
	struct hmdfs_lookup_ret *lookup_ret = NULL;
	char *account = con->sbi->local_info.account;
	char *path = NULL;
	struct lookup_request *req = NULL;
	struct metabase *meta_result = NULL;
	int name_len = strlen(d_name);
	int relative_path_len = strlen(relative_path);
	int path_len = strlen(account) + relative_path_len + name_len + 2;

	lookup_ret = kzalloc(sizeof(*lookup_ret), GFP_KERNEL);
	if (!lookup_ret)
		goto out;

	path = kzalloc(path_len, GFP_KERNEL);
	if (!path) {
		kfree(lookup_ret);
		lookup_ret = NULL;
		goto out;
	}

	if (relative_path_len != 1)
		snprintf(path, path_len, "/%s%s%s", account, relative_path,
			 d_name);
	else
		snprintf(path, path_len, "/%s%s", account, d_name);

	req = kzalloc(sizeof(*req) + path_len, GFP_KERNEL);
	if (!req) {
		kfree(lookup_ret);
		lookup_ret = NULL;
		goto out_get_path;
	}

	req->device_id = con->iid;
	req->name_len = name_len;
	req->path_len = strlen(path) - req->name_len;
	memcpy(req->path, path, path_len);

	path_len += sizeof(*meta_result);
	meta_result = kzalloc(path_len, GFP_KERNEL);
	if (!meta_result) {
		kfree(lookup_ret);
		lookup_ret = NULL;
		goto out_get_meta;
	}
	ret = dentry_syncer_lookup(req, meta_result, path_len);
	if (ret != 0) {
		kfree(lookup_ret);
		lookup_ret = NULL;
	} else {
		lookup_ret->i_mode = (uint16_t)meta_result->mode;
		lookup_ret->i_size = meta_result->size;
		lookup_ret->i_mtime = meta_result->mtime;
		lookup_ret->i_mtime_nsec = 0;
		lookup_ret->fno = meta_result->fno;
	}

	kfree(meta_result);
out_get_meta:
	kfree(req);
out_get_path:
	kfree(path);
out:
	trace_hmdfs_adapter_remote_lookup(con, relative_path, d_name,
					  lookup_ret);
	return lookup_ret;
}

int hmdfs_adapter_create(struct hmdfs_sb_info *sbi, struct dentry *dentry,
			 const char *relative_dir_path)
{
	__u64 device_id = sbi->local_info.iid;
	struct path lower_path;
	struct dentry *lower_dentry = NULL;
	struct inode *lower_inode = NULL;
	__u32 fno = INVALID_FILE_ID;
	char *account = sbi->local_info.account;
	struct metabase *req_create = NULL;
	int name_len = strlen(dentry->d_name.name);
	int relative_dir_len = strlen(relative_dir_path);
	char *path;
	int is_symlink;
	int src_path_len;
	int path_len = strlen(account) + relative_dir_len + name_len + 2;

	hmdfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_inode = lower_dentry->d_inode;
	is_symlink = S_ISLNK(lower_inode->i_mode);
	src_path_len = is_symlink ? strlen(sbi->local_src) : 0;
	if (!S_ISDIR(d_inode(dentry)->i_mode)) {
		hmdfs_adapter_generate_file_id(sbi, relative_dir_path,
					       dentry->d_name.name, &fno);
		if (fno != INVALID_FILE_ID &&
		    hmdfs_persist_file_id(lower_dentry, fno)) {
			hmdfs_adapter_remove_file_id_async(sbi, fno);
			fno = INVALID_FILE_ID;
		}
		if (fno == INVALID_FILE_ID) {
			hmdfs_err("failed to alloc file_id!");
			hmdfs_put_lower_path(&lower_path);
			return -EBADF;
		}
		hmdfs_i(d_inode(dentry))->file_no = fno;
	}
	hmdfs_put_lower_path(&lower_path);
	path = kzalloc(path_len, GFP_KERNEL);
	if (!path)
		return -ENOMEM;

	if (relative_dir_len != 1)
		snprintf(path, path_len, "/%s%s%s", account, relative_dir_path,
			 dentry->d_name.name);
	else
		snprintf(path, path_len, "/%s%s", account, dentry->d_name.name);

	req_create = kzalloc(sizeof(*req_create) + path_len + src_path_len,
			     GFP_KERNEL);
	if (!req_create) {
		kfree(path);
		return -ENOMEM;
	}
	req_create->ctime = lower_inode->i_ctime.tv_sec;
	req_create->mtime = lower_inode->i_mtime.tv_sec;
	req_create->atime = lower_inode->i_atime.tv_sec;
	req_create->size = lower_inode->i_size;
	req_create->mode = d_inode(dentry)->i_mode;
	req_create->uid = lower_inode->i_uid.val;
	req_create->gid = lower_inode->i_gid.val;
	req_create->nlink = lower_inode->i_nlink;
	req_create->rdev = lower_inode->i_rdev;
	req_create->dno = S_ISDIR(d_inode(dentry)->i_mode) ? 0 : device_id;
	req_create->fno = fno;
	req_create->name_len = name_len;
	req_create->path_len = strlen(path) - name_len;
	memcpy(req_create->path, path, path_len);
	req_create->is_symlink = is_symlink;
	req_create->src_path_len = src_path_len;
	req_create->relative_dir_len =
		relative_dir_len == 1 ? 0 : relative_dir_len;
	if (src_path_len)
		strncat(req_create->path, sbi->local_src, src_path_len);

	trace_hmdfs_adapter_create(account, relative_dir_path,
				   dentry->d_name.name);
	dentry_syncer_create(req_create);

	kfree(path);
	kfree(req_create);
	return 0;
}

int hmdfs_adapter_update(struct inode *inode, struct file *file)
{
	int err = 0;
	struct inode *lower_inode = file_inode(hmdfs_f(file)->lower_file);
	struct dentry *dentry = file->f_path.dentry;
	struct hmdfs_sb_info *sbi = inode->i_sb->s_fs_info;
	char *account = sbi->local_info.account;
	const char *name = dentry->d_name.name;
	char *relative_path = NULL;
	struct metabase *up_req = NULL;
	int relative_path_len = 0;
	int path_len = 0;
	char *path = NULL;

	relative_path = hmdfs_get_dentry_relative_path(dentry);
	if (!relative_path)
		return -ENOMEM;

	relative_path_len = strlen(relative_path);
	path_len = strlen(account) + relative_path_len + 2;

	path = kzalloc(path_len, GFP_KERNEL);
	if (!path) {
		err = -ENOMEM;
		goto out_get_path;
	}

	snprintf(path, path_len, "/%s%s", account, relative_path);

	up_req = kzalloc(sizeof(*up_req) + path_len, GFP_KERNEL);
	if (!up_req) {
		err = -ENOMEM;
		goto out_get_req;
	}
	up_req->ctime = lower_inode->i_ctime.tv_sec;
	up_req->mtime = lower_inode->i_mtime.tv_sec;
	up_req->atime = lower_inode->i_atime.tv_sec;
	up_req->size = lower_inode->i_size;
	up_req->mode = inode->i_mode;
	up_req->uid = lower_inode->i_uid.val;
	up_req->gid = lower_inode->i_gid.val;
	up_req->nlink = lower_inode->i_nlink;
	up_req->rdev = lower_inode->i_rdev;
	up_req->dno = 0; // Unused, won't be updated
	up_req->fno = 0; // Unused, won't be updated
	up_req->name_len = strlen(name);
	up_req->path_len = strlen(path) - up_req->name_len - 1;
	memcpy(up_req->path, path, path_len);

	trace_hmdfs_adapter_update(account, relative_path, name);
	dentry_syncer_update(up_req);

	kfree(up_req);
out_get_req:
	kfree(path);
out_get_path:
	kfree(relative_path);
	return err;
}

int hmdfs_adapter_rename(struct hmdfs_sb_info *sbi, const char *old_path,
			 struct dentry *old_dentry, const char *new_path,
			 struct dentry *new_dentry)
{
	char *account = sbi->local_info.account;
	int is_dir = S_ISDIR(old_dentry->d_inode->i_mode);
	const char *old_name = old_dentry->d_name.name;
	const char *new_name = new_dentry->d_name.name;
	struct _rename_request *rename_req = NULL;
	int dir_old_len = strlen(old_path);
	int dir_new_len = strlen(new_path);
	int old_name_len = strlen(old_name);
	int new_name_len = strlen(new_name);
	int account_len = strlen(account);
	int length = dir_old_len + dir_new_len + old_name_len + new_name_len +
		     account_len * 2 + 3;
	int real_len;
	char *path = kzalloc(length, GFP_KERNEL);

	if (!path)
		return -ENOMEM;

	if (dir_old_len != 1 && dir_new_len != 1)
		snprintf(path, length, "/%s%s%s/%s%s%s", account, old_path,
			 old_name, account, new_path, new_name);
	else if (dir_old_len == 1 && dir_new_len != 1)
		snprintf(path, length, "/%s%s/%s%s%s", account, old_name,
			 account, new_path, new_name);
	else if (dir_old_len != 1 && dir_new_len == 1)
		snprintf(path, length, "/%s%s%s/%s%s", account, old_path,
			 old_name, account, new_name);
	else
		snprintf(path, length, "/%s%s/%s%s", account, old_name, account,
			 new_name);

	real_len = strlen(path);
	rename_req = kzalloc(sizeof(*rename_req) + real_len + 1, GFP_KERNEL);
	if (!rename_req) {
		kfree(path);
		return -ENOMEM;
	}

	rename_req->is_dir = is_dir;
	rename_req->flags = 0;
	rename_req->oldparent_len =
		account_len + 1 + (dir_old_len == 1 ? 0 : dir_old_len);
	rename_req->oldname_len = old_name_len;
	rename_req->newparent_len =
		account_len + 1 + (dir_new_len == 1 ? 0 : dir_new_len);
	rename_req->newname_len = new_name_len;
	memcpy(rename_req->path, path, real_len + 1);

	trace_hmdfs_adapter_rename(account, old_path, old_name, new_path,
				   new_name, is_dir);
	dentry_syncer_rename(rename_req);

	kfree(path);
	kfree(rename_req);
	return 0;
}

int hmdfs_adapter_remove(struct hmdfs_sb_info *sbi,
			 const char *relative_dir_path, const char *name)
{
	char *account = sbi->local_info.account;
	struct remove_request *remove_req = NULL;
	int relative_dir_len = strlen(relative_dir_path);
	int name_len = strlen(name);
	int path_len = strlen(account) + relative_dir_len + name_len + 2;
	int real_len;
	char *path = kzalloc(path_len, GFP_KERNEL);

	if (!path)
		return -ENOMEM;

	if (relative_dir_len != 1)
		snprintf(path, path_len, "/%s%s%s", account, relative_dir_path,
			 name);
	else
		snprintf(path, path_len, "/%s%s", account, name);

	real_len = strlen(path);
	remove_req = kzalloc(sizeof(*remove_req) + real_len + 1, GFP_KERNEL);
	if (!remove_req) {
		kfree(path);
		return -ENOMEM;
	}

	remove_req->name_len = name_len;
	remove_req->path_len = real_len - name_len;
	memcpy(remove_req->path, path, real_len + 1);

	trace_hmdfs_adapter_delete(account, relative_dir_path, name);
	dentry_syncer_remove(remove_req);

	kfree(path);
	kfree(remove_req);
	return 0;
}
