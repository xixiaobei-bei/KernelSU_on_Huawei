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
	int ret = -ENOMEM;
	char *account = con->sbi->local_info.account;
	int name_len = strlen(d_name);
	int relative_path_len = strlen(relative_path);
	int path_len = strlen(account) + relative_path_len + name_len + 2;
	char *path = kzalloc(path_len, GFP_KERNEL);
	struct metabase *meta_result =
		kzalloc(path_len + sizeof(*meta_result), GFP_KERNEL);
	struct lookup_request *req =
		kzalloc(sizeof(*req) + path_len, GFP_KERNEL);
	struct hmdfs_lookup_ret *lookup_ret =
		kzalloc(sizeof(*lookup_ret), GFP_KERNEL);

	if (!lookup_ret || !path || !req || !meta_result)
		goto out;

	if (relative_path_len != 1)
		snprintf(path, path_len, "/%s%s%s", account, relative_path,
			 d_name);
	else
		snprintf(path, path_len, "/%s%s", account, d_name);

	req->device_id = con->iid;
	req->name_len = name_len;
	req->path_len = strlen(path) - req->name_len;
	memcpy(req->path, path, path_len);

	path_len += sizeof(*meta_result);
	ret = dentry_syncer_lookup(req, meta_result, path_len);
	if (!ret) {
		lookup_ret->i_mode = (uint16_t)meta_result->mode;
		lookup_ret->i_size = meta_result->size;
		lookup_ret->i_mtime = meta_result->mtime;
		lookup_ret->i_mtime_nsec = 0;
		lookup_ret->fno = meta_result->fno;
	}

out:
	kfree(meta_result);
	kfree(req);
	kfree(path);
	if (ret) {
		kfree(lookup_ret);
		lookup_ret = NULL;
	}
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

static void update_req_from_lower_inode(struct metabase *req,
					struct inode *lower_inode)
{
	req->ctime = lower_inode->i_ctime.tv_sec;
	req->mtime = lower_inode->i_mtime.tv_sec;
	req->atime = lower_inode->i_atime.tv_sec;
	req->size = lower_inode->i_size;
	req->uid = lower_inode->i_uid.val;
	req->gid = lower_inode->i_gid.val;
	req->nlink = lower_inode->i_nlink;
	req->rdev = lower_inode->i_rdev;
	req->dno = 0; // Unused, won't be updated
	req->fno = 0; // Unused, won't be updated
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
	struct metabase *req = NULL;
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

	req = kzalloc(sizeof(*req) + path_len, GFP_KERNEL);
	if (!req) {
		err = -ENOMEM;
		goto out_get_req;
	}
	update_req_from_lower_inode(req, lower_inode);
	req->mode = inode->i_mode;
	req->name_len = strlen(name);
	req->path_len = strlen(path) - req->name_len - 1;
	memcpy(req->path, path, path_len);

	trace_hmdfs_adapter_update(account, relative_path, name);
	dentry_syncer_update(req);

	kfree(req);
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
	struct _rename_request *req = NULL;
	int old_dlen = strlen(old_path);
	int new_dlen = strlen(new_path);
	int old_nlen = strlen(old_name);
	int new_nlen = strlen(new_name);
	int account_len = strlen(account);
	int length = old_dlen + new_dlen + old_nlen + new_nlen +
		     account_len * 2 + 3;
	int real_len;
	char *path = kzalloc(length, GFP_KERNEL);

	if (!path)
		return -ENOMEM;

	if (old_dlen != 1 && new_dlen != 1)
		snprintf(path, length, "/%s%s%s/%s%s%s", account, old_path,
			 old_name, account, new_path, new_name);
	else if (old_dlen == 1 && new_dlen != 1)
		snprintf(path, length, "/%s%s/%s%s%s", account, old_name,
			 account, new_path, new_name);
	else if (old_dlen != 1 && new_dlen == 1)
		snprintf(path, length, "/%s%s%s/%s%s", account, old_path,
			 old_name, account, new_name);
	else
		snprintf(path, length, "/%s%s/%s%s", account, old_name, account,
			 new_name);

	real_len = strlen(path);
	req = kzalloc(sizeof(*req) + real_len + 1, GFP_KERNEL);
	if (!req) {
		kfree(path);
		return -ENOMEM;
	}

	req->is_dir = is_dir;
	req->flags = 0;
	req->oldparent_len = account_len + 1 + (old_dlen == 1 ? 0 : old_dlen);
	req->oldname_len = old_nlen;
	req->newparent_len = account_len + 1 + (new_dlen == 1 ? 0 : new_dlen);
	req->newname_len = new_nlen;
	memcpy(req->path, path, real_len + 1);

	trace_hmdfs_adapter_rename(account, old_path, old_name, new_path,
				   new_name, is_dir);
	dentry_syncer_rename(req);

	kfree(path);
	kfree(req);
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
