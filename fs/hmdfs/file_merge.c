/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_merge.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lizhipeng40@huawei.com maojingjing@huawei.com
 *         qianjiaxing@huawei.com chenjinglong1@huawei.com
 * Create: 2021-01-11
 *
 */

#include "hmdfs_merge_view.h"

#include <linux/file.h>

#include "hmdfs.h"
#include "hmdfs_trace.h"

struct hmdfs_iterate_callback_merge {
	struct dir_context ctx;
	struct dir_context *caller;
	/*
	 * Record the return value of 'caller->actor':
	 *
	 * -EINVAL, buffer is exhausted
	 * -EINTR, current task is pending
	 * -EFAULT, something is wrong
	 * 0, success and can do more
	 */
	int result;
	struct rb_root *root;
	uint64_t dev_id;
};

struct hmdfs_cache_entry {
	struct rb_node rb_node;
	int name_len;
	char *name;
	int file_type;
};

struct hmdfs_cache_entry *allocate_entry(const char *name, int namelen,
					 int d_type)
{
	struct hmdfs_cache_entry *data;

	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return ERR_PTR(-ENOMEM);

	data->name = kstrndup(name, namelen, GFP_KERNEL);
	if (!data->name) {
		kfree(data);
		return ERR_PTR(-ENOMEM);
	}

	data->name_len = namelen;
	data->file_type = d_type;

	return data;
}

int insert_filename(struct rb_root *root, struct hmdfs_cache_entry **new_entry)
{
	struct rb_node *parent = NULL;
	struct rb_node **new_node = &(root->rb_node);
	int cmp_res = 0;
	struct hmdfs_cache_entry *data = *new_entry;

	while (*new_node) {
		struct hmdfs_cache_entry *entry = container_of(
			*new_node, struct hmdfs_cache_entry, rb_node);
		parent = *new_node;

		if (data->name_len < entry->name_len)
			cmp_res = -1;
		else if (data->name_len > entry->name_len)
			cmp_res = 1;
		else
			cmp_res = strncmp(data->name, entry->name,
					  data->name_len);

		if (!cmp_res) {
			kfree(data->name);
			kfree(data);
			*new_entry = entry;
			return entry->file_type;
		}

		if (cmp_res < 0)
			new_node = &((*new_node)->rb_left);
		else if (cmp_res > 0)
			new_node = &((*new_node)->rb_right);
	}

	rb_link_node(&data->rb_node, parent, new_node);
	rb_insert_color(&data->rb_node, root);

	return 0;
}

static void recursive_delete(struct rb_node *node)
{
	struct hmdfs_cache_entry *entry = NULL;

	if (!node)
		return;

	recursive_delete(node->rb_left);
	recursive_delete(node->rb_right);

	entry = container_of(node, struct hmdfs_cache_entry, rb_node);
	kfree(entry->name);
	kfree(entry);
}

static void destroy_tree(struct rb_root *root)
{
	if (!root)
		return;
	recursive_delete(root->rb_node);
	root->rb_node = NULL;
}

static void delete_filename(struct rb_root *root,
			    struct hmdfs_cache_entry *data)
{
	struct rb_node **node = &(root->rb_node);
	struct hmdfs_cache_entry *entry = NULL;
	int cmp_res = 0;

	while (*node) {
		entry = container_of(*node, struct hmdfs_cache_entry, rb_node);
		if (data->name_len < entry->name_len)
			cmp_res = -1;
		else if (data->name_len > entry->name_len)
			cmp_res = 1;
		else
			cmp_res = strncmp(data->name, entry->name,
					  data->name_len);

		if (!cmp_res)
			goto found;

		if (cmp_res < 0)
			node = &((*node)->rb_left);
		else if (cmp_res > 0)
			node = &((*node)->rb_right);
	}
	return;

found:
	rb_erase(*node, root);
	kfree(entry->name);
	kfree(entry);
}

static void rename_conflicting_file(char *dentry_name, int *len,
				    unsigned int dev_id)
{
	int i = *len - 1;
	int dot_pos = -1;
	char *buffer;

	buffer = kzalloc(DENTRY_NAME_MAX_LEN, GFP_KERNEL);
	if (!buffer)
		return;

	while (i >= 0) {
		if (dentry_name[i] == '/')
			break;
		if (dentry_name[i] == '.') {
			// TODO: 这个修改同步到 CT01
			dot_pos = i;
			break;
		}
		i--;
	}

	if (dot_pos == -1) {
		snprintf(dentry_name + *len, DENTRY_NAME_MAX_LEN - *len,
			 CONFLICTING_FILE_SUFFIX, dev_id);
		goto done;
	}

	for (i = 0; i < *len - dot_pos; i++)
		buffer[i] = dentry_name[i + dot_pos];

	buffer[i] = '\0';
	snprintf(dentry_name + dot_pos, DENTRY_NAME_MAX_LEN - dot_pos,
		 CONFLICTING_FILE_SUFFIX, dev_id);
	strcat(dentry_name, buffer);

done:
	*len = strlen(dentry_name);
	kfree(buffer);
}

static void rename_conflicting_directory(char *dentry_name, int *len)
{
	snprintf(dentry_name + *len, DENTRY_NAME_MAX_LEN - *len,
		 CONFLICTING_DIR_SUFFIX);
	*len += strlen(CONFLICTING_DIR_SUFFIX);
}

static int hmdfs_actor_merge(struct dir_context *ctx, const char *name,
			     int namelen, loff_t offset, u64 ino,
			     unsigned int d_type)
{
	int ret = 0;
	int insert_res = 0;
	int max_devid_len = 2;
	char *dentry_name = NULL;
	int dentry_len = namelen;
	struct hmdfs_cache_entry *cache_entry = NULL;
	struct hmdfs_iterate_callback_merge *iterate_callback_merge = NULL;
	struct dir_context *org_ctx = NULL;

	if (hmdfs_file_type(name) != HMDFS_TYPE_COMMON)
		return 0;

	if (namelen > NAME_MAX)
		return -EINVAL;
	dentry_name = kzalloc(NAME_MAX + 1, GFP_KERNEL);
	if (!dentry_name)
		return -ENOMEM;

	strncpy(dentry_name, name, dentry_len);

	cache_entry = allocate_entry(dentry_name, dentry_len, d_type);
	if (IS_ERR(cache_entry)) {
		ret = PTR_ERR(cache_entry);
		goto done;
	}

	iterate_callback_merge =
		container_of(ctx, struct hmdfs_iterate_callback_merge, ctx);
	insert_res =
		insert_filename(iterate_callback_merge->root, &cache_entry);
	if (d_type == DT_DIR && insert_res == DT_DIR) {
		goto done;
	} else if (d_type == DT_DIR && insert_res == DT_REG) {
		if (strlen(CONFLICTING_DIR_SUFFIX) > NAME_MAX - dentry_len) {
			ret = -ENAMETOOLONG;
			goto delete;
		}
		rename_conflicting_directory(dentry_name, &dentry_len);
		cache_entry->file_type = DT_DIR;
	} else if (d_type == DT_REG && insert_res > 0) {
		if (strlen(CONFLICTING_FILE_SUFFIX) + max_devid_len >
		    NAME_MAX - dentry_len) {
			ret = -ENAMETOOLONG;
			goto delete;
		}
		rename_conflicting_file(dentry_name, &dentry_len,
					iterate_callback_merge->dev_id);
	}

	org_ctx = iterate_callback_merge->caller;
	ret = org_ctx->actor(org_ctx, dentry_name, dentry_len, org_ctx->pos,
			     ino, d_type);
	/*
	 * Record original return value, so that the caller can be aware of
	 * different situations.
	 */
	iterate_callback_merge->result = ret;
	ret = ret == 0 ? 0 : 1;
	if (ret && d_type == DT_DIR && insert_res == DT_REG &&
	    cache_entry->file_type == DT_DIR)
		cache_entry->file_type = DT_REG;

delete:
	if (ret && !insert_res)
		delete_filename(iterate_callback_merge->root, cache_entry);
done:
	kfree(dentry_name);
	return ret;
}

struct hmdfs_file_info *
get_next_hmdfs_file_info(struct hmdfs_file_info *fi_head, int device_id)
{
	struct hmdfs_file_info *fi_iter = NULL;
	struct hmdfs_file_info *fi_result = NULL;

	mutex_lock(&fi_head->comrade_list_lock);
	list_for_each_entry_safe(fi_iter, fi_result, &(fi_head->comrade_list),
				  comrade_list) {
		if (fi_iter->device_id == device_id)
			break;
	}
	mutex_unlock(&fi_head->comrade_list_lock);

	return fi_result != fi_head ? fi_result : NULL;
}

struct hmdfs_file_info *get_hmdfs_file_info(struct hmdfs_file_info *fi_head,
					    int device_id)
{
	struct hmdfs_file_info *fi_iter = NULL;

	mutex_lock(&fi_head->comrade_list_lock);
	list_for_each_entry(fi_iter, &(fi_head->comrade_list), comrade_list) {
		if (fi_iter->device_id == device_id) {
			mutex_unlock(&fi_head->comrade_list_lock);
			return fi_iter;
		}
	}
	mutex_unlock(&fi_head->comrade_list_lock);

	return NULL;
}

int hmdfs_iterate_merge(struct file *file, struct dir_context *ctx)
{
	int err = 0;
	struct hmdfs_file_info *fi_head = hmdfs_f(file);
	struct hmdfs_file_info *fi_iter = NULL;
	struct file *lower_file_iter = NULL;
	loff_t start_pos = ctx->pos;
	unsigned long device_id = ((unsigned long)(ctx->pos) << 1) >>
				  (POS_BIT_NUM - DEV_ID_BIT_NUM);
	struct hmdfs_iterate_callback_merge ctx_merge = {
		.ctx.actor = hmdfs_actor_merge,
		.caller = ctx,
		.root = &fi_head->root,
		.dev_id = device_id
	};

	/* pos = -1 indicates that all devices have been traversed
	 * or an error has occurred.
	 */
	if (ctx->pos == -1)
		return 0;

	fi_iter = get_hmdfs_file_info(fi_head, device_id);
	if (!fi_iter) {
		fi_iter = get_next_hmdfs_file_info(fi_head, device_id);
		// dev_id is changed, parameter is set 0 to get next file info
		if (fi_iter)
			ctx_merge.ctx.pos =
				hmdfs_set_pos(fi_iter->device_id, 0, 0);
	}
	while (fi_iter) {
		ctx_merge.dev_id = fi_iter->device_id;
		device_id = ctx_merge.dev_id;
		lower_file_iter = fi_iter->lower_file;
		lower_file_iter->f_pos = file->f_pos;
		err = iterate_dir(lower_file_iter, &ctx_merge.ctx);
		file->f_pos = lower_file_iter->f_pos;
		ctx->pos = file->f_pos;

		if (err)
			goto done;
		/*
		 * ctx->actor return nonzero means buffer is exhausted or
		 * something is wrong, thus we should not continue.
		 */
		if (ctx_merge.result)
			goto done;
		fi_iter = get_next_hmdfs_file_info(fi_head, device_id);
		if (fi_iter) {
			file->f_pos = hmdfs_set_pos(fi_iter->device_id, 0, 0);
			ctx->pos = file->f_pos;
		}
	}
done:
	trace_hmdfs_iterate_merge(file->f_path.dentry, start_pos, ctx->pos,
				  err);
	return err;
}

int do_dir_open_merge(struct file *file, const struct cred *cred,
		      struct hmdfs_file_info *fi_head)
{
	int ret = -EINVAL;
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(file->f_path.dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct hmdfs_file_info *fi = NULL;
	struct path lo_p = { .mnt = file->f_path.mnt };
	struct file *lower_file = NULL;

	if (IS_ERR_OR_NULL(cred))
		return ret;

	mutex_lock(&dim->comrade_list_lock);
	list_for_each_entry(comrade, &(dim->comrade_list), list) {
		fi = kzalloc(sizeof(*fi), GFP_KERNEL);
		if (!fi) {
			ret = ret ? -ENOMEM : 0;
			continue; // allow some dir to fail to open
		}
		lo_p.dentry = comrade->lo_d;
		// make sure that dentry will not be dentry_kill before open
		dget(lo_p.dentry);
		if (unlikely(d_is_negative(lo_p.dentry))) {
			hmdfs_info("dentry is negative, try again");
			kfree(fi);
			dput(lo_p.dentry);
			continue;  // skip this device
		}
		lower_file = dentry_open(&lo_p, file->f_flags, cred);
		dput(lo_p.dentry);
		if (IS_ERR(lower_file)) {
			kfree(fi);
			continue;
		}
		ret = 0;
		fi->device_id = comrade->dev_id;
		fi->lower_file = lower_file;
		mutex_lock(&fi_head->comrade_list_lock);
		list_add_tail(&fi->comrade_list, &fi_head->comrade_list);
		mutex_unlock(&fi_head->comrade_list_lock);
	}
	mutex_unlock(&dim->comrade_list_lock);
	return ret;
}

int hmdfs_dir_open_merge(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct hmdfs_file_info *fi = NULL;

	fi = kzalloc(sizeof(*fi), GFP_KERNEL);
	if (!fi)
		return -ENOMEM;

	file->private_data = fi;
	fi->root = RB_ROOT;
	mutex_init(&fi->comrade_list_lock);
	INIT_LIST_HEAD(&fi->comrade_list);

	ret = do_dir_open_merge(file, hmdfs_sb(inode->i_sb)->cred, fi);
	if (ret)
		kfree(fi);

	return ret;
}

int hmdfs_dir_release_merge(struct inode *inode, struct file *file)
{
	struct hmdfs_file_info *fi_head = hmdfs_f(file);
	struct hmdfs_file_info *fi_iter = NULL;
	struct hmdfs_file_info *fi_temp = NULL;

	mutex_lock(&fi_head->comrade_list_lock);
	list_for_each_entry_safe(fi_iter, fi_temp, &(fi_head->comrade_list),
				  comrade_list) {
		list_del_init(&(fi_iter->comrade_list));
		fput(fi_iter->lower_file);
		kfree(fi_iter);
	}
	mutex_unlock(&fi_head->comrade_list_lock);
	destroy_tree(&fi_head->root);
	file->private_data = NULL;
	kfree(fi_head);

	return 0;
}

const struct file_operations hmdfs_dir_fops_merge = {
	.owner = THIS_MODULE,
	.iterate = hmdfs_iterate_merge,
	.open = hmdfs_dir_open_merge,
	.release = hmdfs_dir_release_merge,
};

int hmdfs_file_open_merge(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct path lo_p = { .mnt = file->f_path.mnt };
	struct super_block *sb = inode->i_sb;
	const struct cred *cred = hmdfs_sb(sb)->cred;
	struct hmdfs_file_info *gfi = NULL;
	struct dentry *parent = NULL;

	lo_p.dentry = hmdfs_get_fst_lo_d(file->f_path.dentry);
	if (!lo_p.dentry) {
		err = -EINVAL;
		goto out_err;
	}

	gfi = kzalloc(sizeof(*gfi), GFP_KERNEL);
	if (!gfi) {
		err = -ENOMEM;
		goto out_err;
	}

	parent = dget_parent(file->f_path.dentry);
	lower_file = dentry_open(&lo_p, file->f_flags, cred);
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		kfree(gfi);
	} else {
		gfi->lower_file = lower_file;
		file->private_data = gfi;
	}
	dput(parent);
out_err:
	dput(lo_p.dentry);
	return err;
}

int hmdfs_file_flush_merge(struct file *file, fl_owner_t id)
{
	struct hmdfs_file_info *gfi = hmdfs_f(file);
	struct file *lower_file = gfi->lower_file;

	if (lower_file->f_op->flush)
		return lower_file->f_op->flush(lower_file, id);

	return 0;
}

/* Transparent transmission of parameters to device_view level,
 * so file operations are same as device_view local operations.
 */
const struct file_operations hmdfs_file_fops_merge = {
	.owner = THIS_MODULE,
	.llseek = hmdfs_file_llseek_local,
	.read = hmdfs_read_local,
	.write = hmdfs_write_local,
	.mmap = hmdfs_file_mmap_local,
	.open = hmdfs_file_open_merge,
	.flush = hmdfs_file_flush_merge,
	.release = hmdfs_file_release_local,
	.fsync = hmdfs_fsync_local,
};
