/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_local.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: weilongping@huawei.com
 * Create: 2020-03-26
 *
 */

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>

#include "hmdfs_client.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#include "hmdfs_merge_view.h"
#include "hmdfs_trace.h"

#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#endif

int hmdfs_file_open_local(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct path lower_path;
	struct super_block *sb = inode->i_sb;
	const struct cred *cred = hmdfs_sb(sb)->cred;
	struct hmdfs_file_info *gfi = kzalloc(sizeof(*gfi), GFP_KERNEL);

	if (!gfi) {
		err = -ENOMEM;
		goto out_err;
	}

	hmdfs_get_lower_path(file->f_path.dentry, &lower_path);
	lower_file = dentry_open(&lower_path, file->f_flags, cred);
	hmdfs_put_lower_path(&lower_path);
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		kfree(gfi);
	} else {
		gfi->lower_file = lower_file;
		file->private_data = gfi;
	}
out_err:
	return err;
}

int hmdfs_file_release_local(struct inode *inode, struct file *file)
{
	struct hmdfs_file_info *gfi = hmdfs_f(file);

#ifdef CONFIG_HMDFS_1_0
	if (hmdfs_i(inode)->adapter_dentry_flag == ADAPTER_PHOTOKIT_DENTRY_FLAG)
		hmdfs_adapter_update(inode, file);
#endif
	file->private_data = NULL;
	fput(gfi->lower_file);
	kfree(gfi);
	return 0;
}

ssize_t hmdfs_read_local(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos)
{
	struct file *lower_file = hmdfs_f(file)->lower_file;
	int err = vfs_read(lower_file, buf, count, ppos);

	if (err >= 0)
		file_inode(file)->i_atime = file_inode(lower_file)->i_atime;
	return err;
}

ssize_t hmdfs_write_local(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct file *lower_file = hmdfs_f(file)->lower_file;
	struct inode *inode = file_inode(file);
	struct inode *lower_inode = file_inode(lower_file);
	struct dentry *dentry = file_dentry(file);
	int err = vfs_write(lower_file, buf, count, ppos);

	if (err >= 0) {
		inode_lock(inode);
		i_size_write(inode, i_size_read(lower_inode));
		inode->i_atime = lower_inode->i_atime;
		inode->i_ctime = lower_inode->i_ctime;
		inode->i_mtime = lower_inode->i_mtime;
		if (!hmdfs_i_merge(hmdfs_i(inode)))
			update_inode_to_dentry(dentry, inode);
		inode_unlock(inode);
	}
	return err;
}

int hmdfs_fsync_local(struct file *file, loff_t start, loff_t end, int datasync)
{
	int err;
	struct file *lower_file = hmdfs_f(file)->lower_file;

	err = __generic_file_fsync(file, start, end, datasync);
	if (err)
		goto out;

	err = vfs_fsync_range(lower_file, start, end, datasync);
out:
	return err;
}

loff_t hmdfs_file_llseek_local(struct file *file, loff_t offset, int whence)
{
	int err = 0;
	struct file *lower_file = NULL;

	err = generic_file_llseek(file, offset, whence);
	if (err < 0)
		goto out;
	lower_file = hmdfs_f(file)->lower_file;
	err = generic_file_llseek(lower_file, offset, whence);
out:
	return err;
}

int hmdfs_file_mmap_local(struct file *file, struct vm_area_struct *vma)
{
	struct hmdfs_file_info *private_data = file->private_data;
	struct file *realfile = NULL;
	int ret;

	if (!private_data)
		return -EINVAL;

	realfile = private_data->lower_file;
	if (!realfile)
		return -EINVAL;

	if (!realfile->f_op->mmap)
		return -ENODEV;

	if (WARN_ON(file != vma->vm_file))
		return -EIO;

	vma->vm_file = get_file(realfile);
	ret = call_mmap(vma->vm_file, vma);
	if (ret)
		fput(realfile);
	else
		fput(file);

	file_accessed(file);

	return ret;
}

const struct file_operations hmdfs_file_fops_local = {
	.owner = THIS_MODULE,
	.llseek = hmdfs_file_llseek_local,
	.read = hmdfs_read_local,
	.write = hmdfs_write_local,
	.mmap = hmdfs_file_mmap_local,
	.open = hmdfs_file_open_local,
	.release = hmdfs_file_release_local,
	.fsync = hmdfs_fsync_local,
};

static int hmdfs_iterate_local(struct file *file, struct dir_context *ctx)
{
	int err = 0;
	loff_t start_pos = ctx->pos;
	struct file *lower_file = hmdfs_f(file)->lower_file;

	if (ctx->pos == -1)
		return 0;

	lower_file->f_pos = file->f_pos;
	err = iterate_dir(lower_file, ctx);
	file->f_pos = lower_file->f_pos;

	if (err < 0)
		ctx->pos = -1;

	trace_hmdfs_iterate_local(file->f_path.dentry, start_pos, ctx->pos,
				  err);
	return err;
}

int hmdfs_dir_open_local(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct dentry *dentry = file->f_path.dentry;
	struct path lower_path;
	struct super_block *sb = inode->i_sb;
	const struct cred *cred = hmdfs_sb(sb)->cred;
	struct hmdfs_file_info *gfi = kzalloc(sizeof(*gfi), GFP_KERNEL);

	if (!gfi)
		return -ENOMEM;

	if (IS_ERR_OR_NULL(cred)) {
		err = -EPERM;
		goto out_err;
	}
	hmdfs_get_lower_path(dentry, &lower_path);
	lower_file = dentry_open(&lower_path, file->f_flags, cred);
	hmdfs_put_lower_path(&lower_path);
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		goto out_err;
	} else {
		gfi->lower_file = lower_file;
		file->private_data = gfi;
	}
	return err;

out_err:
	kfree(gfi);
	return err;
}

static int hmdfs_dir_release_local(struct inode *inode, struct file *file)
{
	struct hmdfs_file_info *gfi = hmdfs_f(file);

	file->private_data = NULL;
	fput(gfi->lower_file);
	kfree(gfi);
	return 0;
}

const struct file_operations hmdfs_dir_ops_local = {
	.owner = THIS_MODULE,
	.iterate = hmdfs_iterate_local,
	.open = hmdfs_dir_open_local,
	.release = hmdfs_dir_release_local,
	.fsync = hmdfs_fsync_local,
};
