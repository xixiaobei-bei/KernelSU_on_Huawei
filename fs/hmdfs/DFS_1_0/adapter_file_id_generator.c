/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_file_id_generator.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 * Create: 2020-04-14
 *
 */

#include "adapter_file_id_generator.h"

#include <linux/bitops.h>
#include <linux/cred.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/xattr.h>

#include "authority/authentication.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"

#define FILE_ID_BIT	  32
#define TYPE_BIT	  6
#define HIGH_COUNT_BIT	  14
#define LOW_COUNT_BIT	  (FILE_ID_BIT - TYPE_BIT - HIGH_COUNT_BIT)
#define FILE_ID_XATTR_KEY "user.hmdfs_fno"

#define MAX_RETRY_TIMES		 30
#define RETRY_RANDOMIZE_INTERVAL 10

#define PATHWALK_BUFSIZE (64 * 1024)

static uint32_t current_id;
static const char default_datapoint[] = "/data/misc_ce/0/kdfs";
static const char id_base_path[] = ".file_id_links/";
DEFINE_SPINLOCK(id_lock);

enum adapter_id_async_task_type {
	INVALID = 0,
	CREATE_ID, // unused
	DELETE_ID,

	RENAME_FILE,
	RENAME_DIR,
};

static struct task_struct *async_thread;
static struct list_head async_q;
DEFINE_SPINLOCK(async_q_lock);

struct id_async_task {
	char path[PATH_MAX];
	uint32_t type;
	uint32_t id;
	struct hmdfs_sb_info *sbi;
	struct list_head list;
};

static uint32_t alloc_file_id(bool random)
{
	uint32_t id;

	spin_lock(&id_lock);
	do {
		if (random)
			current_id = get_random_u32();
		else
			current_id++;
	} while (current_id == INVALID_FILE_ID);

	id = current_id;
	spin_unlock(&id_lock);
	return id;
}

static int create_dir_if_not_exist(const char *dirpath, umode_t mode)
{
	int ret;
	struct dentry *dentry;
	struct path path;
	struct inode *inode;
#ifdef CONFIG_HMDFS_ANDROID
	kuid_t tmp_uid;
	const struct cred *orig_cred;
#endif

	dentry = kern_path_create(AT_FDCWD, dirpath, &path, LOOKUP_DIRECTORY);
	if (IS_ERR(dentry)) {
		ret = PTR_ERR(dentry);
		if (ret == -EEXIST) {
			hmdfs_debug("dirpath already exists");
			return 0;
		}
		return ret;
	}

	inode = path.dentry->d_inode;
#ifdef CONFIG_HMDFS_ANDROID
	tmp_uid = get_inode_uid(inode);
	orig_cred = hmdfs_override_fsids(false);
	if (!orig_cred) {
		ret = -ENOMEM;
		goto no_cred;
	}
	inode->i_uid = current_fsuid();
#endif
	ret = vfs_mkdir(inode, dentry, mode);
#ifdef CONFIG_HMDFS_ANDROID
	inode->i_uid = tmp_uid;
	hmdfs_revert_fsids(orig_cred);
no_cred:
#endif
	done_path_create(&path, dentry);

	return ret;
}

static int create_id_symlink(const char *src, const char *link)
{
	int ret;
	struct dentry *dentry;
	struct path path;
	char *parent_dir, *slash, *tmp_path, *cur, *parent_dir_tmp;
#ifdef CONFIG_HMDFS_ANDROID
	const struct cred *orig_cred;
	kuid_t tmp_uid;
#endif

	parent_dir = kstrdup(link, GFP_KERNEL);
	if (!parent_dir)
		return -ENOMEM;

	slash = strrchr(parent_dir, '/'); /* Assume link path is legal */
	if (!slash) {
		kfree(parent_dir);
		return -EINVAL;
	}
	*(slash + 1) = 0;

	tmp_path = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!tmp_path) {
		ret = -ENOMEM;
		goto out;
	}
	parent_dir_tmp = parent_dir;
	while ((cur = strsep(&parent_dir_tmp, "/"))) {
		if (!strlen(cur))
			continue;

		strcat(tmp_path, "/");
		strcat(tmp_path, cur);
		ret = create_dir_if_not_exist(tmp_path, 0771);
		if (ret) {
			hmdfs_err("create dir failed, err=%d", ret);
			goto out;
		}
	}

	dentry = kern_path_create(AT_FDCWD, link, &path, 0);
	if (IS_ERR(dentry)) {
		ret = PTR_ERR(dentry);
		hmdfs_err("create link failed, ret=%d", ret);
		goto out;
	}

	inode_lock_nested(path.dentry->d_parent->d_inode, I_MUTEX_PARENT);
#ifdef CONFIG_HMDFS_ANDROID
	orig_cred = hmdfs_override_fsids(false);
	if (!orig_cred) {
		ret = -ENOMEM;
		goto no_cred;
	}
	tmp_uid = get_inode_uid(path.dentry->d_inode);
	path.dentry->d_inode->i_uid = current_fsuid();
#endif
	ret = vfs_symlink(path.dentry->d_inode, dentry, src);
#ifdef CONFIG_HMDFS_ANDROID
	path.dentry->d_inode->i_uid = tmp_uid;
	hmdfs_revert_fsids(orig_cred);
no_cred:
#endif
	inode_unlock(path.dentry->d_parent->d_inode);
	done_path_create(&path, dentry);
	trace_hmdfs_create_id_symlink(src, link, ret);

out:
	kfree(tmp_path);
	kfree(parent_dir);
	return ret;
}

static int delete_id_symlink(const char *linkpath)
{
	int ret;
	struct path path;
#ifdef CONFIG_HMDFS_ANDROID
	kuid_t tmp_uid;
#endif
	ret = kern_path(linkpath, 0, &path);
	if (ret) {
		hmdfs_err("cannot find linkpath, ret=%d", ret);
		return ret;
	}

	inode_lock_nested(path.dentry->d_parent->d_inode, I_MUTEX_PARENT);
#ifdef CONFIG_HMDFS_ANDROID
	tmp_uid = get_inode_uid(path.dentry->d_parent->d_inode);
	path.dentry->d_parent->d_inode->i_uid = current_fsuid();
#endif
	ret = vfs_unlink(path.dentry->d_parent->d_inode, path.dentry, NULL);
#ifdef CONFIG_HMDFS_ANDROID
	path.dentry->d_parent->d_inode->i_uid = tmp_uid;
#endif
	inode_unlock(path.dentry->d_parent->d_inode);
	path_put(&path);

	trace_hmdfs_delete_id_symlink(linkpath, ret);
	return ret;
}

static void convert_src_mnt_path(struct hmdfs_sb_info *sbi, const char *from,
				 char *to, bool to_mntpath)
{
	const char *relative, *from_prefix, *to_prefix;

	if (to_mntpath) {
		from_prefix = sbi->local_src;
		to_prefix = sbi->local_dst;
	} else {
		from_prefix = sbi->local_dst;
		to_prefix = sbi->local_src;
	}

	if (strncmp(from, from_prefix, strlen(from_prefix))) {
		hmdfs_warning("orig path unmatched");
		memcpy(to, from, strlen(from));
		return;
	}
	relative = from + strlen(from_prefix);
	if (*relative == '/')
		relative++;
	snprintf(to, PATH_MAX, "%s/%s", to_prefix, relative);
}

int hmdfs_adapter_generate_file_id(struct hmdfs_sb_info *sbi, const char *dir,
				   const char *name, uint32_t *out_id)
{
	char *linkpath = NULL;
	char *destpath = NULL;
	uint32_t id = INVALID_FILE_ID;
	int ret = 0, retry = 0;

	linkpath = kzalloc(PATH_MAX, GFP_KERNEL);
	destpath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!linkpath || !destpath) {
		ret = -ENOMEM;
		goto out;
	}

	ret = snprintf(destpath, PATH_MAX, "%s/%s/%s", sbi->local_dst, dir,
		       name);
	if (ret >= PATH_MAX) {
		ret = -ENAMETOOLONG;
		goto out;
	}

	while (retry < MAX_RETRY_TIMES) {
		if (retry > 0 && retry % RETRY_RANDOMIZE_INTERVAL == 0)
			id = alloc_file_id(true);
		else
			id = alloc_file_id(false);

		hmdfs_adapter_convert_file_id_to_path(id, linkpath);
		ret = create_id_symlink(destpath, linkpath);
		if (!ret)
			break;
		hmdfs_debug("id %u used, retry %d", id, retry);
		retry++;
	}

	if (retry >= MAX_RETRY_TIMES) {
		hmdfs_err("failed to find unused file id! %d", ret);
		id = INVALID_FILE_ID;
		ret = -ENOSPC;
		goto out;
	}
out:
	*out_id = id;
	kfree(destpath);
	kfree(linkpath);
	return ret;
}

static int __rebuild_file_id_linkfile(const char *path, uint32_t id)
{
	int ret;
	char *linkpath = NULL;
	char *destpath = NULL;

	destpath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!destpath) {
		ret = -ENOMEM;
		goto out;
	}

	linkpath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!linkpath) {
		ret = -ENOMEM;
		goto out;
	}

	hmdfs_adapter_convert_file_id_to_path(id, linkpath);
	ret = snprintf(destpath, PATH_MAX, "%s", path);
	if (ret >= PATH_MAX) {
		ret = -ENAMETOOLONG;
		goto out;
	}
	ret = create_id_symlink(destpath, linkpath);
out:
	kfree(destpath);
	kfree(linkpath);
	return ret;
}

int hmdfs_adapter_remove_file_id(uint32_t id)
{
	int ret;
	char *path;

	path = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!path)
		return -ENOMEM;

	hmdfs_adapter_convert_file_id_to_path(id, path);
	ret = delete_id_symlink(path);
	if (ret == -ENOENT)
		ret = 0; // already removed

	kfree(path);
	return ret;
}

int hmdfs_persist_file_id(struct dentry *dentry, uint32_t id)
{
	int err;

	if (!d_inode(dentry))
		return -EINVAL;

	err = __vfs_setxattr(dentry, d_inode(dentry), FILE_ID_XATTR_KEY, &id,
			     sizeof(id), XATTR_CREATE);
	if (err && err != -EEXIST)
		hmdfs_err("failed to setxattr, err=%d", err);

	return err;
}

uint32_t hmdfs_read_file_id(struct inode *inode)
{
	uint32_t ret = INVALID_FILE_ID;
	struct dentry *dentry = d_find_alias(inode);

	if (!dentry)
		return ret;

	if (__vfs_getxattr(dentry, inode, FILE_ID_XATTR_KEY, &ret,
			   sizeof(ret)) < 0)
		ret = INVALID_FILE_ID;

	dput(dentry);
	return ret;
}

static void rename_link_dest(const char *pathname)
{
	uint32_t id;
	struct path path;

	if (kern_path(pathname, LOOKUP_FOLLOW, &path)) {
		hmdfs_warning("cannot find path");
		return;
	}

	id = hmdfs_i(d_inode(path.dentry))->file_no;
	path_put(&path);
	if (id == INVALID_FILE_ID) {
		hmdfs_warning("invalid fileid");
		return;
	}

	hmdfs_adapter_remove_file_id(id);
	__rebuild_file_id_linkfile(pathname, id);
}

struct walk_callback {
	struct dir_context ctx;
	char *buf;
	int buflen;
	int reclen;
};

struct walk_namedata {
	char type;
	char name[];
};

struct walk_dir {
	char path[PATH_MAX];
	struct list_head list;
};

static int walk_fillbuf(struct dir_context *ctx, const char *name, int namlen,
			loff_t offset, u64 ino, unsigned int d_type)
{
	struct walk_callback *wcb =
		container_of(ctx, struct walk_callback, ctx);
	struct walk_namedata *data = (void *)wcb->buf + wcb->reclen;

	if (wcb->buflen - wcb->reclen <
	    sizeof(struct walk_namedata) + namlen + 1)
		return -EFAULT;

	switch (d_type) {
	case DT_DIR:
	case DT_REG:
	case DT_LNK:
		data->type = (char)d_type;
		break;
	default:
		return 0;
	}

	strncpy(data->name, name, namlen);
	data->name[namlen] = '\0';

	wcb->reclen += sizeof(struct walk_namedata) + namlen + 1;
	return 0;
}

static void walk_and_rebuild_symlink(struct hmdfs_sb_info *sbi,
				     const char *parent)
{
	int ret;
	struct walk_callback walk_cb = {
		.ctx.actor = walk_fillbuf,
		.buflen = PATHWALK_BUFSIZE,
		.reclen = 0,
	};
	struct list_head dir_list = LIST_HEAD_INIT(dir_list);
	struct walk_dir *cur_dir, *tmp;
	char *path_buf, *src_path;

	walk_cb.buf = kzalloc(walk_cb.buflen, GFP_KERNEL);
	src_path = kzalloc(PATH_MAX, GFP_KERNEL);
	path_buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!path_buf || !src_path || !walk_cb.buf)
		goto out;

	cur_dir = kzalloc(sizeof(*cur_dir), GFP_KERNEL);
	if (!cur_dir)
		goto out;

	strcpy(cur_dir->path, parent);
	list_add_tail(&cur_dir->list, &dir_list);

	while (!list_empty(&dir_list)) {
		cur_dir = list_first_entry(&dir_list, struct walk_dir, list);
		convert_src_mnt_path(sbi, cur_dir->path, src_path, false);
		walk_cb.ctx.pos = 0;
		do {
			int readlen = 0;
			struct walk_namedata *data;

			walk_cb.reclen = 0;
			ret = read_dentry(sbi, src_path, &walk_cb.ctx);
			if (ret < 0) {
				hmdfs_err("read_dentry failed! err=%d", ret);
				goto out_walk_failed;
			}
			if (!walk_cb.reclen)
				break;
			data = (struct walk_namedata *)walk_cb.buf;
process_one_data:
			if (data->type == DT_DIR) {
				tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
				if (!tmp)
					goto out_walk_failed;
				snprintf(tmp->path, PATH_MAX, "%s/%s",
					 cur_dir->path, data->name);
				list_add_tail(&tmp->list, &dir_list);
			} else {
				snprintf(path_buf, PATH_MAX, "%s/%s",
					 cur_dir->path, data->name);
				rename_link_dest(path_buf);
			}
			readlen += sizeof(*data) + strlen(data->name) + 1;
			data = (struct walk_namedata *)(walk_cb.buf + readlen);
			if (readlen < walk_cb.reclen)
				goto process_one_data;
		} while (ret > 0);
		list_del(&cur_dir->list);
		kfree(cur_dir);
	}
	goto out;

out_walk_failed:
	list_for_each_entry_safe(cur_dir, tmp, &dir_list, list) {
		list_del(&cur_dir->list);
		kfree(cur_dir);
	}
out:
	kfree(path_buf);
	kfree(src_path);
	kfree(walk_cb.buf);
}

static void process_file_id_delete_task(struct id_async_task *tsk)
{
	hmdfs_adapter_remove_file_id(tsk->id);
}

static void process_file_rename_task(struct id_async_task *tsk)
{
	rename_link_dest(tsk->path);
}

static void process_dir_rename_task(struct id_async_task *tsk)
{
	walk_and_rebuild_symlink(tsk->sbi, tsk->path);
}

typedef void (*async_task_executor)(struct id_async_task *tsk);
static const async_task_executor id_task_executors[] = {
	[DELETE_ID] = process_file_id_delete_task,
	[RENAME_FILE] = process_file_rename_task,
	[RENAME_DIR] = process_dir_rename_task,
};

static int async_task_loop(void *data)
{
	struct id_async_task *tsk;

	while (!kthread_should_stop()) {
		while (1) {
			spin_lock(&async_q_lock);
			if (list_empty(&async_q)) {
				spin_unlock(&async_q_lock);
				break;
			}
			tsk = list_first_entry(&async_q, struct id_async_task,
					       list);
			spin_unlock(&async_q_lock);

			if (id_task_executors[tsk->type]) {
				const struct cred *orig_cred =
					hmdfs_override_creds(tsk->sbi->cred);
				const struct cred *reverted_cred =
					hmdfs_override_fsids(false);

				if (unlikely(!reverted_cred)) {
					const int min = 1000;
					const int max = 2000;

					hmdfs_revert_creds(orig_cred);
					usleep_range(min, max);
					continue;
				}

				id_task_executors[tsk->type](tsk);
				trace_hmdfs_adapter_file_id_gen_process_task(
					tsk->type, tsk->path, tsk->id);

				hmdfs_revert_fsids(reverted_cred);
				hmdfs_revert_creds(orig_cred);
			}
			spin_lock(&async_q_lock);
			list_del(&tsk->list);
			spin_unlock(&async_q_lock);
			kfree(tsk);
		}

		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock(&async_q_lock);
		if (list_empty(&async_q)) {
			spin_unlock(&async_q_lock);
			schedule();
		} else {
			spin_unlock(&async_q_lock);
		}
		set_current_state(TASK_RUNNING);
	}

	hmdfs_info("thread exit");
	async_thread = NULL;
	return 0;
}

static struct id_async_task *add_id_task(struct hmdfs_sb_info *sbi,
					 uint32_t type, const char *path,
					 uint32_t id)
{
	struct id_async_task *tsk = NULL;

	if (!async_thread) {
		hmdfs_err("async thread already stopped, cannot add more task");
		return NULL;
	}

	tsk = kzalloc(sizeof(*tsk), GFP_KERNEL);
	if (!tsk)
		return NULL;

	tsk->sbi = sbi;
	tsk->id = id;
	tsk->type = type;
	if (path)
		strncpy(tsk->path, path, PATH_MAX);

	spin_lock(&async_q_lock);
	list_add_tail(&tsk->list, &async_q);
	spin_unlock(&async_q_lock);

	trace_hmdfs_adapter_file_id_gen_add_task(tsk->type, tsk->path, tsk->id);
	wake_up_process(async_thread);
	return tsk;
}

int hmdfs_adapter_remove_file_id_async(struct hmdfs_sb_info *sbi, uint32_t id)
{
	if (!add_id_task(sbi, DELETE_ID, NULL, id)) {
		hmdfs_err("failed to add task!");
		return -ENOSPC;
	}
	return 0;
}

int hmdfs_adapter_add_rename_task(struct hmdfs_sb_info *sbi,
				  struct path *lower_parent_path,
				  const char *newname, uint32_t id,
				  umode_t mode)
{
	int ret = 0;
	char *buf = NULL;
	char *dir;
	char *newpath = NULL;
	uint32_t type;

	if (S_ISDIR(mode))
		type = RENAME_DIR;
	else
		type = RENAME_FILE;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	newpath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!buf || !newpath) {
		ret = -ENOMEM;
		goto out;
	}

	dir = d_path(lower_parent_path, buf, PATH_MAX);
	if (IS_ERR(dir)) {
		ret = PTR_ERR(dir);
		goto out;
	}

	if (strlen(dir) + strlen(newname) + 2 > PATH_MAX) {
		ret = -ENAMETOOLONG;
		goto out;
	}
	convert_src_mnt_path(sbi, dir, newpath, true);
	strcat(newpath, "/");
	strcat(newpath, newname);

	if (!add_id_task(sbi, type, newpath, id)) {
		hmdfs_err("failed to add task!");
		ret = -ENOSPC;
	}

out:
	kfree(newpath);
	kfree(buf);
	return ret;
}

/**
 * Convert file id to absolute path.
 * The caller must guarantee that the given buffer has enough space.
 */
void hmdfs_adapter_convert_file_id_to_path(uint32_t id, char *buf)
{
	uint32_t type = id >> (FILE_ID_BIT - TYPE_BIT);
	uint32_t high_bit =
		(id & GENMASK(FILE_ID_BIT - TYPE_BIT - 1, LOW_COUNT_BIT)) >>
		LOW_COUNT_BIT;
	uint32_t low_bit = id & GENMASK(LOW_COUNT_BIT - 1, 0);

	sprintf(buf, "%s/%s%x/%x/%x_0", default_datapoint, id_base_path, type,
		high_bit, low_bit);
}

int hmdfs_adapter_file_id_generator_init(void)
{
	int err = 0;

	current_id = get_random_u32();
	INIT_LIST_HEAD(&async_q);

	async_thread = kthread_run(async_task_loop, NULL, "fid_async_t");
	if (IS_ERR(async_thread)) {
		err = PTR_ERR(async_thread);
		async_thread = NULL;
		hmdfs_err("init async thread fail! err=%d", err);
	}
	return err;
}

void hmdfs_adapter_file_id_generator_cleanup(void)
{
	struct id_async_task *tsk, *tmp;

	current_id = 0;

	if (async_thread)
		kthread_stop(async_thread);
	async_thread = NULL;

	spin_lock(&async_q_lock);
	list_for_each_entry_safe(tsk, tmp, &async_q, list) {
		list_del(&tsk->list);
		kfree(tsk);
	}
	spin_unlock(&async_q_lock);
}
