/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode_merge.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chenjinglong1@huawei.com maojingjing@huawei.com
 *         lousong@huawei.com qianjiaxing@huawei.com
 * Create: 2021-01-11
 *
 */

#include "hmdfs_merge_view.h"
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/fs_stack.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "authority/authentication.h"
#include "hmdfs_trace.h"

struct kmem_cache *hmdfs_dentry_merge_cachep;

struct dentry *hmdfs_get_fst_lo_d(struct dentry *dentry)
{
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct dentry *d = NULL;

	mutex_lock(&dim->comrade_list_lock);
	comrade = list_first_entry_or_null(&dim->comrade_list,
					   struct hmdfs_dentry_comrade, list);
	if (comrade)
		d = dget(comrade->lo_d);
	mutex_unlock(&dim->comrade_list_lock);
	return d;
}

struct dentry *hmdfs_get_lo_d(struct dentry *dentry, int dev_id)
{
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct dentry *d = NULL;

	mutex_lock(&dim->comrade_list_lock);
	list_for_each_entry(comrade, &dim->comrade_list, list) {
		if (comrade->dev_id == dev_id) {
			d = dget(comrade->lo_d);
			break;
		}
	}
	mutex_unlock(&dim->comrade_list_lock);
	return d;
}

static void update_inode_attr(struct inode *inode, struct dentry *child_dentry)
{
	struct inode *li = NULL;
	struct hmdfs_dentry_info_merge *cdi = hmdfs_dm(child_dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct hmdfs_dentry_comrade *fst_comrade = NULL;

	mutex_lock(&cdi->comrade_list_lock);
	fst_comrade = list_first_entry(&cdi->comrade_list,
				       struct hmdfs_dentry_comrade, list);
	list_for_each_entry(comrade, &cdi->comrade_list, list) {
		li = d_inode(comrade->lo_d);
		if (!li)
			continue;

		if (comrade == fst_comrade) {
			inode->i_atime = li->i_atime;
			inode->i_ctime = li->i_ctime;
			inode->i_mtime = li->i_mtime;
			inode->i_size = li->i_size;
			continue;
		}

		if (hmdfs_time_compare(&inode->i_mtime, &li->i_mtime) < 0)
			inode->i_mtime = li->i_mtime;
	}
	mutex_unlock(&cdi->comrade_list_lock);
}

static int get_num_comrades(struct dentry *dentry)
{
	struct list_head *pos;
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(dentry);
	int count = 0;

	mutex_lock(&dim->comrade_list_lock);
	list_for_each(pos, &dim->comrade_list)
		count++;
	mutex_unlock(&dim->comrade_list_lock);
	return count;
}

static struct inode *fill_inode_merge(struct super_block *sb,
				      struct inode *parent_inode,
				      struct dentry *child_dentry,
				      struct dentry *lo_d_dentry)
{
	struct dentry *fst_lo_d = NULL;
	struct hmdfs_inode_info *info = NULL;
	struct inode *inode = NULL;
	umode_t mode;

	if (lo_d_dentry) {
		fst_lo_d = lo_d_dentry;
		dget(fst_lo_d);
	} else {
		fst_lo_d = hmdfs_get_fst_lo_d(child_dentry);
	}
	if (!fst_lo_d) {
		inode = ERR_PTR(-EINVAL);
		goto out;
	}
	if (hmdfs_inode_type(parent_inode) == HMDFS_LAYER_ZERO)
		inode = hmdfs_iget_locked_root(sb, HMDFS_ROOT_MERGE, NULL,
					       NULL);
	else
		inode = hmdfs_iget5_locked_merge(sb, d_inode(fst_lo_d));
	if (!inode) {
		hmdfs_err("iget5_locked get inode NULL");
		inode = ERR_PTR(-ENOMEM);
		goto out;
	}
	if (!(inode->i_state & I_NEW))
		goto out;
	info = hmdfs_i(inode);
	if (hmdfs_inode_type(parent_inode) == HMDFS_LAYER_ZERO)
		info->inode_type = HMDFS_LAYER_FIRST_MERGE;
	else
		info->inode_type = HMDFS_LAYER_OTHER_MERGE;

	inode->i_uid = KUIDT_INIT((uid_t)1000);
	inode->i_gid = KGIDT_INIT((gid_t)1000);

	update_inode_attr(inode, child_dentry);
	mode = d_inode(fst_lo_d)->i_mode;
	/* remote symlink need to treat as regfile,
	 * the specific operation is performed by device_view.
	 * local symlink is managed by merge_view.
	 */
	if (hm_islnk(hmdfs_d(fst_lo_d)->file_type) &&
	    hmdfs_d(fst_lo_d)->device_id == 0) {
		inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
		inode->i_op = &hmdfs_symlink_iops_merge;
		inode->i_fop = &hmdfs_file_fops_merge;
		set_nlink(inode, 1);
	} else if (S_ISREG(mode)) { // Reguler file 0660
		inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
		inode->i_op = &hmdfs_file_iops_merge;
		inode->i_fop = &hmdfs_file_fops_merge;
		set_nlink(inode, 1);
	} else if (S_ISDIR(mode)) { // Directory 0771
		inode->i_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH;
		inode->i_op = &hmdfs_dir_iops_merge;
		inode->i_fop = &hmdfs_dir_fops_merge;
		set_nlink(inode, get_num_comrades(child_dentry) + 2);
	}

	unlock_new_inode(inode);
out:
	dput(fst_lo_d);
	return inode;
}

struct hmdfs_dentry_comrade *alloc_comrade(struct dentry *lo_d, int dev_id)
{
	struct hmdfs_dentry_comrade *comrade = NULL;

	// 文件只有一个 comrade，考虑 {comrade, list + list lock}
	comrade = kzalloc(sizeof(*comrade), GFP_KERNEL);
	if (unlikely(!comrade))
		return ERR_PTR(-ENOMEM);

	comrade->lo_d = lo_d;
	comrade->dev_id = dev_id;
	dget(lo_d);
	return comrade;
}

void link_comrade(struct list_head *onstack_comrades_head,
		  struct hmdfs_dentry_comrade *comrade)
{
	struct hmdfs_dentry_comrade *c = NULL;

	list_for_each_entry(c, onstack_comrades_head, list) {
		if (likely(c->dev_id != comrade->dev_id))
			continue;
		hmdfs_err("Redundant comrade of device %llu", c->dev_id);
		dput(comrade->lo_d);
		kfree(comrade);
		WARN_ON(1);
		return;
	}

	if (comrade_is_local(comrade))
		list_add(&comrade->list, onstack_comrades_head);
	else
		list_add_tail(&comrade->list, onstack_comrades_head);
}

/**
 * assign_comrades_unlocked - assign a child dentry with comrades
 *
 * We tend to setup a local list of all the comrades we found and place the
 * list onto the dentry_info to achieve atomicity.
 */
static void assign_comrades_unlocked(struct dentry *child_dentry,
				     struct list_head *onstack_comrades_head)
{
	struct hmdfs_dentry_info_merge *cdi = hmdfs_dm(child_dentry);

	mutex_lock(&cdi->comrade_list_lock);
	WARN_ON(!list_empty(&cdi->comrade_list));
	list_splice_init(onstack_comrades_head, &cdi->comrade_list);
	mutex_unlock(&cdi->comrade_list_lock);
}

static struct hmdfs_dentry_comrade *lookup_comrade(struct path lower_path,
						   const char *d_name,
						   int dev_id,
						   unsigned int flags)
{
	struct path path;
	struct hmdfs_dentry_comrade *comrade = NULL;
	int err;

	err = vfs_path_lookup(lower_path.dentry, lower_path.mnt, d_name, flags,
			      &path);
	if (err)
		return ERR_PTR(err);

	comrade = alloc_comrade(path.dentry, dev_id);
	path_put(&path);
	return comrade;
}

/**
 * conf_name_trans_nop - do nothing but copy
 *
 * WARNING: always check before translation
 */
static char *conf_name_trans_nop(struct dentry *d)
{
	return kstrndup(d->d_name.name, d->d_name.len, GFP_KERNEL);
}

/**
 * conf_name_trans_dir - conflicted name translation for directory
 *
 * WARNING: always check before translation
 */
static char *conf_name_trans_dir(struct dentry *d)
{
	int len = d->d_name.len - strlen(CONFLICTING_DIR_SUFFIX);

	return kstrndup(d->d_name.name, len, GFP_KERNEL);
}

/**
 * conf_name_trans_reg - conflicted name translation for regular file
 *
 * WARNING: always check before translation
 */
static char *conf_name_trans_reg(struct dentry *d, int *dev_id)
{
	int dot_pos, start_cpy_pos, num_len, i;
	int len = d->d_name.len;
	char *name = kstrndup(d->d_name.name, d->d_name.len, GFP_KERNEL);

	if (unlikely(!name))
		return NULL;

	// find the last dot if possible
	for (dot_pos = len - 1; dot_pos >= 0; dot_pos--) {
		if (name[dot_pos] == '.')
			break;
	}
	if (dot_pos == -1)
		dot_pos = len;

	// retrieve the conf sn (i.e. dev_id)
	num_len = 0;
	for (i = dot_pos - 1; i >= 0; i--) {
		if (name[i] >= '0' && name[i] <= '9')
			num_len++;
		else
			break;
	}

	*dev_id = 0;
	for (i = 0; i < num_len; i++)
		*dev_id = *dev_id * 10 + name[dot_pos - num_len + i] - '0';

	// move the file suffix( '\0' included) right after the file name
	start_cpy_pos =
		dot_pos - num_len - strlen(CONFLICTING_FILE_CONST_SUFFIX);
	memmove(name + start_cpy_pos, name + dot_pos, len - dot_pos + 1);
	return name;
}

int check_filename(const char *name, int len)
{
	int cmp_res = 0;

	if (len >= strlen(CONFLICTING_DIR_SUFFIX)) {
		cmp_res = strncmp(name + len - strlen(CONFLICTING_DIR_SUFFIX),
				  CONFLICTING_DIR_SUFFIX,
				  strlen(CONFLICTING_DIR_SUFFIX));
		if (cmp_res == 0)
			return DT_DIR;
	}

	if (len >= strlen(CONFLICTING_FILE_CONST_SUFFIX)) {
		int dot_pos, start_cmp_pos, num_len, i;

		for (dot_pos = len - 1; dot_pos >= 0; dot_pos--) {
			if (name[dot_pos] == '.')
				break;
		}
		if (dot_pos == -1)
			dot_pos = len;

		num_len = 0;
		for (i = dot_pos - 1; i >= 0; i--) {
			if (name[i] >= '0' && name[i] <= '9')
				num_len++;
			else
				break;
		}

		start_cmp_pos = dot_pos - num_len -
				strlen(CONFLICTING_FILE_CONST_SUFFIX);
		cmp_res = strncmp(name + start_cmp_pos,
				  CONFLICTING_FILE_CONST_SUFFIX,
				  strlen(CONFLICTING_FILE_CONST_SUFFIX));
		if (cmp_res == 0)
			return DT_REG;
	}

	return 0;
}

static int lookup_merge_normal(struct dentry *child_dentry, unsigned int flags)
{
	struct dentry *parent_dentry = dget_parent(child_dentry);
	struct hmdfs_dentry_info_merge *pdi = hmdfs_dm(parent_dentry);
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);
	struct hmdfs_dentry_comrade *comrade, *cc;
	struct path lo_p, path;
	LIST_HEAD(head);
	int ret = -ENOENT;
	int dev_id = -1;
	int ftype;
	char *lo_name;
	umode_t mode;

	ftype = check_filename(child_dentry->d_name.name,
			       child_dentry->d_name.len);
	if (ftype == DT_REG)
		lo_name = conf_name_trans_reg(child_dentry, &dev_id);
	else if (ftype == DT_DIR)
		lo_name = conf_name_trans_dir(child_dentry);
	else
		lo_name = conf_name_trans_nop(child_dentry);
	if (unlikely(!lo_name)) {
		ret = -ENOMEM;
		goto out;
	}

	ret = hmdfs_get_path_in_sb(child_dentry->d_sb, sbi->real_dst,
				   LOOKUP_DIRECTORY, &path);
	if (ret) {
		if (ret == -ENOENT)
			ret = -EINVAL;
		goto free;
	}
	lo_p.mnt = path.mnt;

	ret = -ENOENT;
	mutex_lock(&pdi->comrade_list_lock);
	list_for_each_entry(cc, &pdi->comrade_list, list) {
		if (ftype == DT_REG && cc->dev_id != dev_id)
			continue;

		lo_p.dentry = cc->lo_d;
		comrade = lookup_comrade(lo_p, lo_name, cc->dev_id, flags);
		if (IS_ERR(comrade)) {
			ret = ret ? PTR_ERR(comrade) : 0;
			continue;
		}

		mode = hmdfs_cm(comrade);
		if ((ftype == DT_DIR && !S_ISDIR(mode)) ||
		    (ftype == DT_REG && S_ISDIR(mode))) {
			destory_comrade(comrade);
			ret = ret ? PTR_ERR(comrade) : 0;
			continue;
		}

		ret = 0;
		link_comrade(&head, comrade);

		if (!S_ISDIR(mode))
			break;
	}
	mutex_unlock(&pdi->comrade_list_lock);

	assign_comrades_unlocked(child_dentry, &head);
	path_put(&path);
free:
	kfree(lo_name);
out:
	dput(parent_dentry);
	return ret;
}

/**
 * do_lookup_merge_root - lookup the root of the merge view(root/merge_view)
 *
 * It's common for a network filesystem to incur various of faults, so we
 * intent to show mercy for faults here, except faults reported by the local.
 */
static int do_lookup_merge_root(struct path path_dev,
				struct dentry *child_dentry, unsigned int flags)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);
	struct hmdfs_dentry_comrade *comrade;
	const int buf_len =
		max((int)HMDFS_CID_SIZE + 1, (int)sizeof(DEVICE_VIEW_LOCAL));
	char *buf = kzalloc(buf_len, GFP_KERNEL);
	struct hmdfs_peer *peer;
	LIST_HEAD(head);
	int ret;

	if (!buf)
		return -ENOMEM;

	// lookup real_dst/device_view/local
	memcpy(buf, DEVICE_VIEW_LOCAL, sizeof(DEVICE_VIEW_LOCAL));
	comrade = lookup_comrade(path_dev, buf, HMDFS_DEVID_LOCAL, flags);
	if (IS_ERR(comrade)) {
		ret = PTR_ERR(comrade);
		goto out;
	}
	link_comrade(&head, comrade);

	// lookup real_dst/device_view/cidxx
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(peer, &sbi->connections.node_list, list) {
		mutex_unlock(&sbi->connections.node_lock);
		memcpy(buf, peer->cid, HMDFS_CID_SIZE);
		comrade = lookup_comrade(path_dev, buf, peer->device_id, flags);
		if (IS_ERR(comrade))
			continue;

		link_comrade(&head, comrade);
		mutex_lock(&sbi->connections.node_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);

	assign_comrades_unlocked(child_dentry, &head);
	ret = 0;

out:
	kfree(buf);
	return ret;
}

// mkdir -p
static void lock_root_inode_shared(struct inode *root, bool *locked, bool *down)
{
	struct rw_semaphore *sem = &root->i_rwsem;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
#define RWSEM_READER_OWNED     (1UL << 0)
#define RWSEM_RD_NONSPINNABLE  (1UL << 1)
#define RWSEM_WR_NONSPINNABLE  (1UL << 2)
#define RWSEM_NONSPINNABLE     (RWSEM_RD_NONSPINNABLE | RWSEM_WR_NONSPINNABLE)
#define RWSEM_OWNER_FLAGS_MASK (RWSEM_READER_OWNED | RWSEM_NONSPINNABLE)
	struct task_struct *sem_owner =
		(struct task_struct *)(atomic_long_read(&sem->owner) &
				       ~RWSEM_OWNER_FLAGS_MASK);
#else
	struct task_struct *sem_owner = sem->owner;
#endif

	*locked = false;
	*down = false;

	if (sem_owner != current)
		return;

	// It's us that takes the wsem
	if (!inode_trylock_shared(root)) {
		downgrade_write(sem);
		*down = true;
	}
	*locked = true;
}

static void restore_root_inode_sem(struct inode *root, bool locked, bool down)
{
	if (!locked)
		return;

	inode_unlock_shared(root);
	if (down)
		inode_lock(root);
}

static int lookup_merge_root(struct inode *root_inode,
			     struct dentry *child_dentry, unsigned int flags)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);
	struct path path_dev;
	int ret = -ENOENT;
	int buf_len;
	char *buf = NULL;
	bool locked, down;

	// consider additional one slash and one '\0'
	buf_len = strlen(sbi->real_dst) + 1 + sizeof(DEVICE_VIEW_ROOT);
	if (buf_len > PATH_MAX)
		return -ENAMETOOLONG;

	buf = kmalloc(buf_len, GFP_KERNEL);
	if (unlikely(!buf))
		return -ENOMEM;

	sprintf(buf, "%s/%s", sbi->real_dst, DEVICE_VIEW_ROOT);
	lock_root_inode_shared(root_inode, &locked, &down);
	ret = hmdfs_get_path_in_sb(child_dentry->d_sb, buf, LOOKUP_DIRECTORY,
				   &path_dev);
	if (ret)
		goto free_buf;

	ret = do_lookup_merge_root(path_dev, child_dentry, flags);
	path_put(&path_dev);

free_buf:
	kfree(buf);
	restore_root_inode_sem(root_inode, locked, down);
	return ret;
}

int init_hmdfs_dentry_info_merge(struct hmdfs_sb_info *sbi,
				 struct dentry *dentry)
{
	struct hmdfs_dentry_info_merge *info = NULL;

	info = kmem_cache_zalloc(hmdfs_dentry_merge_cachep, GFP_NOFS);
	if (!info)
		return -ENOMEM;

	info->ctime = jiffies;
	INIT_LIST_HEAD(&info->comrade_list);
	mutex_init(&info->comrade_list_lock);
	d_set_d_op(dentry, &hmdfs_dops_merge);
	dentry->d_fsdata = info;
	return 0;
}

static void update_dm(struct dentry *dst, struct dentry *src)
{
	struct hmdfs_dentry_info_merge *dmi_dst = hmdfs_dm(dst);
	struct hmdfs_dentry_info_merge *dmi_src = hmdfs_dm(src);
	LIST_HEAD(tmp_dst);
	LIST_HEAD(tmp_src);

	/* Mobilize all the comrades */
	mutex_lock(&dmi_dst->comrade_list_lock);
	mutex_lock(&dmi_src->comrade_list_lock);
	list_splice_init(&dmi_dst->comrade_list, &tmp_dst);
	list_splice_init(&dmi_src->comrade_list, &tmp_src);
	list_splice(&tmp_dst, &dmi_src->comrade_list);
	list_splice(&tmp_src, &dmi_dst->comrade_list);
	mutex_unlock(&dmi_src->comrade_list_lock);
	mutex_unlock(&dmi_dst->comrade_list_lock);
}

// do this in a map-reduce manner
struct dentry *hmdfs_lookup_merge(struct inode *parent_inode,
				  struct dentry *child_dentry,
				  unsigned int flags)
{
	bool create = flags & (LOOKUP_CREATE | LOOKUP_RENAME_TARGET);
	struct hmdfs_sb_info *sbi = hmdfs_sb(child_dentry->d_sb);
	struct hmdfs_inode_info *pii = hmdfs_i(parent_inode);
	struct inode *child_inode = NULL;
	struct dentry *ret_dentry = NULL;
	int err = 0;

	/*
	 * Internel flags like LOOKUP_CREATE should not pass to device view.
	 * LOOKUP_REVAL is needed because dentry cache in hmdfs might be stale
	 * after rename in lower fs. LOOKUP_FOLLOW is not needed because
	 * get_link is defined for symlink inode in merge_view.
	 * LOOKUP_DIRECTORY is not needed because merge_view can do the
	 * judgement that whether result is directory or not.
	 */
	flags = flags & LOOKUP_REVAL;

	child_dentry->d_fsdata = NULL;

	if (child_dentry->d_name.len > NAME_MAX) {
		err = -ENAMETOOLONG;
		goto out;
	}

	err = init_hmdfs_dentry_info_merge(sbi, child_dentry);
	if (unlikely(err))
		goto out;

	if (pii->inode_type == HMDFS_LAYER_ZERO)
		err = lookup_merge_root(parent_inode, child_dentry, flags);
	else
		err = lookup_merge_normal(child_dentry, flags);

	if (!err) {
		struct hmdfs_inode_info *info = NULL;

		child_inode = fill_inode_merge(parent_inode->i_sb, parent_inode,
					       child_dentry, NULL);
		ret_dentry = d_splice_alias(child_inode, child_dentry);
		if (IS_ERR(ret_dentry)) {
			clear_comrades(child_dentry);
			err = PTR_ERR(ret_dentry);
			goto out;
		}
		if (ret_dentry) {
			update_dm(ret_dentry, child_dentry);
			child_dentry = ret_dentry;
		}
		info = hmdfs_i(child_inode);
		if (info->inode_type == HMDFS_LAYER_FIRST_MERGE)
			hmdfs_root_inode_perm_init(child_inode);
		else
			check_and_fixup_ownership_remote(parent_inode,
							 child_dentry);

		goto out;
	}

	if ((err == -ENOENT) && create)
		err = 0;

out:
	hmdfs_trace_merge(trace_hmdfs_lookup_merge_end, parent_inode,
			  child_dentry, err);
	return err ? ERR_PTR(err) : ret_dentry;
}

static int hmdfs_getattr_merge(const struct path *path, struct kstat *stat,
			       u32 request_mask, unsigned int flags)
{
	int ret;
	struct path lower_path = {
		.dentry = hmdfs_get_fst_lo_d(path->dentry),
		.mnt = path->mnt,
	};

	if (unlikely(!lower_path.dentry)) {
		hmdfs_err("Fatal! No comrades");
		ret = -EINVAL;
		goto out;
	}

	ret = vfs_getattr(&lower_path, stat, request_mask, flags);
out:
	dput(lower_path.dentry);
	return ret;
}

static int hmdfs_setattr_merge(struct dentry *dentry, struct iattr *ia)
{
	struct inode *inode = d_inode(dentry);
	struct dentry *lower_dentry = hmdfs_get_fst_lo_d(dentry);
	struct inode *lower_inode = NULL;
	struct iattr lower_ia;
	unsigned int ia_valid = ia->ia_valid;
	int err = 0;
	kuid_t tmp_uid;

	if (!lower_dentry) {
		WARN_ON(1);
		err = -EINVAL;
		goto out;
	}

	lower_inode = d_inode(lower_dentry);
	memcpy(&lower_ia, ia, sizeof(lower_ia));
	if (ia_valid & ATTR_FILE)
		lower_ia.ia_file = hmdfs_f(ia->ia_file)->lower_file;
	lower_ia.ia_valid &= ~(ATTR_UID | ATTR_GID | ATTR_MODE);

	inode_lock(lower_inode);
	tmp_uid = hmdfs_override_inode_uid(lower_inode);

	err = notify_change(lower_dentry, &lower_ia, NULL);
	i_size_write(inode, i_size_read(lower_inode));
	inode->i_atime = lower_inode->i_atime;
	inode->i_mtime = lower_inode->i_mtime;
	inode->i_ctime = lower_inode->i_ctime;
	hmdfs_revert_inode_uid(lower_inode, tmp_uid);

	inode_unlock(lower_inode);

out:
	dput(lower_dentry);
	return err;
}

const struct inode_operations hmdfs_file_iops_merge = {
	.getattr = hmdfs_getattr_merge,
	.setattr = hmdfs_setattr_merge,
	.permission = hmdfs_permission,
};

int do_mkdir_merge(struct inode *parent_inode, struct dentry *child_dentry,
		   umode_t mode, struct inode *lo_i_parent,
		   struct dentry *lo_d_child)
{
	int ret = 0;
	struct super_block *sb = parent_inode->i_sb;
	struct inode *child_inode = NULL;

	ret = vfs_mkdir(lo_i_parent, lo_d_child, mode);
	if (ret)
		goto out;

	child_inode =
		fill_inode_merge(sb, parent_inode, child_dentry, lo_d_child);
	if (IS_ERR(child_inode)) {
		ret = PTR_ERR(child_inode);
		goto out;
	}

	d_add(child_dentry, child_inode);
	/* nlink should be increased with the joining of children */
	set_nlink(parent_inode, 2);
out:
	return ret;
}

int do_create_merge(struct inode *parent_inode, struct dentry *child_dentry,
		    umode_t mode, bool want_excl, struct inode *lo_i_parent,
		    struct dentry *lo_d_child)
{
	int ret = 0;
	struct super_block *sb = parent_inode->i_sb;
	struct inode *child_inode = NULL;

	ret = vfs_create(lo_i_parent, lo_d_child, mode, want_excl);
	if (ret)
		goto out;

	child_inode =
		fill_inode_merge(sb, parent_inode, child_dentry, lo_d_child);
	if (IS_ERR(child_inode)) {
		ret = PTR_ERR(child_inode);
		goto out;
	}

	d_add(child_dentry, child_inode);
	/* nlink should be increased with the joining of children */
	set_nlink(parent_inode, 2);
out:
	return ret;
}

int do_symlink_merge(struct inode *parent_inode, struct dentry *child_dentry,
		     const char *symname, struct inode *lower_parent_inode,
		     struct dentry *lo_d_child)
{
	int ret = 0;
	struct super_block *sb = parent_inode->i_sb;
	struct inode *child_inode = NULL;

	ret = vfs_symlink(lower_parent_inode, lo_d_child, symname);
	if (ret)
		goto out;

	child_inode =
		fill_inode_merge(sb, parent_inode, child_dentry, lo_d_child);
	if (IS_ERR(child_inode)) {
		ret = PTR_ERR(child_inode);
		goto out;
	}

	d_add(child_dentry, child_inode);
	fsstack_copy_attr_times(parent_inode, lower_parent_inode);
	fsstack_copy_inode_size(parent_inode, lower_parent_inode);
out:
	return ret;
}

int hmdfs_do_ops_merge(struct inode *i_parent, struct dentry *d_child,
		       struct dentry *lo_d_child, struct path path,
		       struct hmdfs_recursive_para *rec_op_para)
{
	int ret = 0;

	if (rec_op_para->is_last) {
		switch (rec_op_para->opcode) {
		case F_MKDIR_MERGE:
			ret = do_mkdir_merge(i_parent, d_child,
					     rec_op_para->mode,
					     d_inode(path.dentry), lo_d_child);
			break;
		case F_CREATE_MERGE:
			ret = do_create_merge(i_parent, d_child,
					      rec_op_para->mode,
					      rec_op_para->want_excl,
					      d_inode(path.dentry), lo_d_child);
			break;
		case F_SYMLINK_MERGE:
			ret = do_symlink_merge(i_parent, d_child,
					       rec_op_para->name,
					       d_inode(path.dentry),
					       lo_d_child);
			break;
		default:
			ret = -EINVAL;
			break;
		}
	} else {
		ret = vfs_mkdir(d_inode(path.dentry), lo_d_child,
				rec_op_para->mode);
	}
	if (ret)
		hmdfs_err("vfs_ops failed, ops %d, err = %d",
			  rec_op_para->opcode, ret);
	return ret;
}

int hmdfs_create_lower_dentry(struct inode *i_parent, struct dentry *d_child,
			      struct dentry *lo_d_parent, bool is_dir,
			      struct hmdfs_recursive_para *rec_op_para)
{
	struct hmdfs_sb_info *sbi = i_parent->i_sb->s_fs_info;
	struct hmdfs_dentry_comrade *new_comrade = NULL;
	struct dentry *lo_d_child = NULL;
	char *path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	char *absolute_path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	char *path_name = NULL;
	struct path path = { .mnt = NULL, .dentry = NULL };
	int ret = 0;

	if (unlikely(!path_buf || !absolute_path_buf)) {
		ret = -ENOMEM;
		goto out;
	}

	path_name = dentry_path_raw(lo_d_parent, path_buf, PATH_MAX);
	if (IS_ERR(path_name)) {
		ret = PTR_ERR(path_name);
		goto out;
	}
	if ((strlen(sbi->real_dst) + strlen(path_name) +
	     strlen(d_child->d_name.name) + 2) > PATH_MAX) {
		ret = -ENAMETOOLONG;
		goto out;
	}

	sprintf(absolute_path_buf, "%s%s/%s", sbi->real_dst, path_name,
		d_child->d_name.name);

	if (is_dir)
		lo_d_child = kern_path_create(AT_FDCWD, absolute_path_buf,
					      &path, LOOKUP_DIRECTORY);
	else
		lo_d_child = kern_path_create(AT_FDCWD, absolute_path_buf,
					      &path, 0);
	if (IS_ERR(lo_d_child)) {
		ret = PTR_ERR(lo_d_child);
		goto out;
	}
	// to ensure link_comrade after vfs_mkdir succeed
	ret = hmdfs_do_ops_merge(i_parent, d_child, lo_d_child, path,
				 rec_op_para);
	if (ret)
		goto out_put;
	new_comrade = alloc_comrade(lo_d_child, HMDFS_DEVID_LOCAL);
	if (IS_ERR(new_comrade)) {
		ret = PTR_ERR(new_comrade);
		goto out_put;
	} else {
		link_comrade_unlocked(d_child, new_comrade);
	}

out_put:
	done_path_create(&path, lo_d_child);
out:
	kfree(absolute_path_buf);
	kfree(path_buf);
	return ret;
}

static int create_lo_d_parent_recur(struct dentry *d_parent,
				    struct dentry *d_child, umode_t mode,
				    struct hmdfs_recursive_para *rec_op_para)
{
	struct dentry *lo_d_parent, *d_pparent;
	int ret = 0;

	lo_d_parent = hmdfs_get_lo_d(d_parent, HMDFS_DEVID_LOCAL);
	if (!lo_d_parent) {
		d_pparent = dget_parent(d_parent);
		ret = create_lo_d_parent_recur(d_pparent, d_parent,
					       d_inode(d_parent)->i_mode,
					       rec_op_para);
		dput(d_pparent);
		if (ret)
			goto out;
		lo_d_parent = hmdfs_get_lo_d(d_parent, HMDFS_DEVID_LOCAL);
		if (!lo_d_parent) {
			ret = -ENOENT;
			goto out;
		}
	}
	rec_op_para->is_last = false;
	rec_op_para->mode = mode;
	ret = hmdfs_create_lower_dentry(d_inode(d_parent), d_child, lo_d_parent,
					true, rec_op_para);
out:
	dput(lo_d_parent);
	return ret;
}

int create_lo_d_child(struct inode *i_parent, struct dentry *d_child,
		      bool is_dir, struct hmdfs_recursive_para *rec_op_para)
{
	struct dentry *d_pparent, *lo_d_parent, *lo_d_child;
	struct dentry *d_parent = dget_parent(d_child);
	int ret = 0;
	mode_t d_child_mode = rec_op_para->mode;

	lo_d_parent = hmdfs_get_lo_d(d_parent, HMDFS_DEVID_LOCAL);
	if (!lo_d_parent) {
		d_pparent = dget_parent(d_parent);
		ret = create_lo_d_parent_recur(d_pparent, d_parent,
					       d_inode(d_parent)->i_mode,
					       rec_op_para);
		dput(d_pparent);
		if (unlikely(ret)) {
			lo_d_child = ERR_PTR(ret);
			goto out;
		}
		lo_d_parent = hmdfs_get_lo_d(d_parent, HMDFS_DEVID_LOCAL);
		if (!lo_d_parent) {
			lo_d_child = ERR_PTR(-ENOENT);
			goto out;
		}
	}
	rec_op_para->is_last = true;
	rec_op_para->mode = d_child_mode;
	ret = hmdfs_create_lower_dentry(i_parent, d_child, lo_d_parent, is_dir,
					rec_op_para);

out:
	dput(d_parent);
	dput(lo_d_parent);
	return ret;
}

void hmdfs_init_recursive_para(struct hmdfs_recursive_para *rec_op_para,
			       int opcode, mode_t mode, bool want_excl,
			       const char *name)
{
	rec_op_para->is_last = true;
	rec_op_para->opcode = opcode;
	rec_op_para->mode = mode;
	rec_op_para->want_excl = want_excl;
	rec_op_para->name = name;
}

int hmdfs_mkdir_merge(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	int ret = 0;
	struct hmdfs_recursive_para *rec_op_para = NULL;

	// confict_name  & file_type is checked by hmdfs_mkdir_local
	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		ret = -EACCES;
		goto out;
	}
	rec_op_para = kmalloc(sizeof(*rec_op_para), GFP_KERNEL);
	if (!rec_op_para) {
		ret = -ENOMEM;
		goto out;
	}

	hmdfs_init_recursive_para(rec_op_para, F_MKDIR_MERGE, mode, false,
				  NULL);
	ret = create_lo_d_child(dir, dentry, true, rec_op_para);
out:
	hmdfs_trace_merge(trace_hmdfs_mkdir_merge, dir, dentry, ret);
	if (ret)
		d_drop(dentry);
	kfree(rec_op_para);
	return ret;
}

int hmdfs_create_merge(struct inode *dir, struct dentry *dentry, umode_t mode,
		       bool want_excl)
{
	struct hmdfs_recursive_para *rec_op_para = NULL;
	int ret = 0;

	rec_op_para = kmalloc(sizeof(*rec_op_para), GFP_KERNEL);
	if (!rec_op_para) {
		ret = -ENOMEM;
		goto out;
	}
	hmdfs_init_recursive_para(rec_op_para, F_CREATE_MERGE, mode, want_excl,
				  NULL);
	// confict_name  & file_type is checked by hmdfs_create_local
	ret = create_lo_d_child(dir, dentry, false, rec_op_para);
out:
	hmdfs_trace_merge(trace_hmdfs_create_merge, dir, dentry, ret);
	if (ret)
		d_drop(dentry);
	kfree(rec_op_para);
	return ret;
}

int do_rmdir_merge(struct inode *dir, struct dentry *dentry)
{
	int ret = 0;
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct dentry *lo_d = NULL;
	struct dentry *lo_d_dir = NULL;
	struct inode *lo_i_dir = NULL;

	//TODO: 当前只删本地，因不会影响到图库场景
	//TODO：图库重启清除软连接？或者什么场景会删除
	//TODO: remove 调用同时删除空目录以及非空目录，结果不一致
	//TODO: 如果校验会不会有并发问题？就算锁，也只能锁自己
	mutex_lock(&dim->comrade_list_lock);
	list_for_each_entry(comrade, &(dim->comrade_list), list) {
		lo_d = comrade->lo_d;
		lo_d_dir = lock_parent(lo_d);
		lo_i_dir = d_inode(lo_d_dir);
		//TODO: 部分成功，lo_d确认
		ret = vfs_rmdir(lo_i_dir, lo_d);
		unlock_dir(lo_d_dir);
		if (ret)
			break;
	}
	mutex_unlock(&dim->comrade_list_lock);
	hmdfs_trace_merge(trace_hmdfs_rmdir_merge, dir, dentry, ret);
	return ret;
}

int hmdfs_rmdir_merge(struct inode *dir, struct dentry *dentry)
{
	int ret = 0;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		ret = -EACCES;
		goto out;
	}

	ret = do_rmdir_merge(dir, dentry);
	if (ret) {
		hmdfs_err("rm dir failed:%d", ret);
		goto out;
	}

	d_drop(dentry);
out:
	hmdfs_trace_merge(trace_hmdfs_rmdir_merge, dir, dentry, ret);
	return ret;
}

int do_unlink_merge(struct inode *dir, struct dentry *dentry)
{
	int ret = 0;
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(dentry);
	struct hmdfs_dentry_comrade *comrade = NULL;
	struct dentry *lo_d = NULL;
	struct dentry *lo_d_dir = NULL;
	struct inode *lo_i_dir = NULL;
	// TODO：文件场景  list_first_entry
	mutex_lock(&dim->comrade_list_lock);
	list_for_each_entry(comrade, &(dim->comrade_list), list) {
		lo_d = comrade->lo_d;
		lo_d_dir = lock_parent(lo_d);
		lo_i_dir = d_inode(lo_d_dir);
		ret = vfs_unlink(lo_i_dir, lo_d, NULL); // lo_d GET
		unlock_dir(lo_d_dir);
		if (ret)
			break;
	}
	mutex_unlock(&dim->comrade_list_lock);

	return ret;
}

int hmdfs_unlink_merge(struct inode *dir, struct dentry *dentry)
{
	int ret = 0;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		ret = -EACCES;
		goto out;
	}

	ret = do_unlink_merge(dir, dentry);
	if (ret) {
		hmdfs_err("unlink failed:%d", ret);
		goto out;
	}

	d_drop(dentry);
out:
	return ret;
}

int hmdfs_symlink_merge(struct inode *dir, struct dentry *dentry,
			const char *symname)
{
	int ret = 0;
	struct hmdfs_recursive_para *rec_op_para = NULL;

	if (hmdfs_file_type(dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		ret = -EACCES;
		goto out;
	}

	rec_op_para = kmalloc(sizeof(*rec_op_para), GFP_KERNEL);
	if (!rec_op_para) {
		ret = -ENOMEM;
		goto out;
	}
	hmdfs_init_recursive_para(rec_op_para, F_SYMLINK_MERGE, 0, false,
				  symname);
	ret = create_lo_d_child(dir, dentry, false, rec_op_para);

out:
	trace_hmdfs_symlink_merge(dir, dentry, ret);
	if (ret)
		d_drop(dentry);
	kfree(rec_op_para);
	return ret;
}

int do_rename_merge(struct inode *old_dir, struct dentry *old_dentry,
		    struct inode *new_dir, struct dentry *new_dentry,
		    unsigned int flags)
{
	int ret = 0;
	struct hmdfs_sb_info *sbi = (old_dir->i_sb)->s_fs_info;
	struct hmdfs_dentry_info_merge *dim = hmdfs_dm(old_dentry);
	struct hmdfs_dentry_comrade *comrade = NULL, *new_comrade = NULL;
	struct path lo_p_new = { .mnt = NULL, .dentry = NULL };
	struct inode *lo_i_old_dir = NULL, *lo_i_new_dir = NULL;
	struct dentry *lo_d_old_dir = NULL, *lo_d_old = NULL,
		      *lo_d_new_dir = NULL, *lo_d_new = NULL;
	struct dentry *d_new_dir = NULL;
	char *path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	char *abs_path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	char *path_name = NULL;

	/* TODO: Will WPS rename a temporary file to another directory?
	 * could flags with replace bit result in rename ops
	 * cross_devices?
	 * currently does not support replace flags.
	 */
	if (flags & ~RENAME_NOREPLACE) {
		ret = -EINVAL;
		goto out;
	}

	if (unlikely(!path_buf || !abs_path_buf)) {
		ret = -ENOMEM;
		goto out;
	}

	list_for_each_entry(comrade, &dim->comrade_list, list) {
		lo_d_old = comrade->lo_d;
		d_new_dir = d_find_alias(new_dir);
		lo_d_new_dir = hmdfs_get_lo_d(d_new_dir, comrade->dev_id);
		dput(d_new_dir);

		if (!lo_d_new_dir)
			continue;
		path_name = dentry_path_raw(lo_d_new_dir, path_buf, PATH_MAX);
		dput(lo_d_new_dir);
		if (IS_ERR(path_name)) {
			ret = PTR_ERR(path_name);
			continue;
		}

		if (strlen(sbi->real_dst) + strlen(path_name) +
		    strlen(new_dentry->d_name.name) + 2 > PATH_MAX) {
			ret = -ENAMETOOLONG;
			goto out;
		}

		snprintf(abs_path_buf, PATH_MAX, "%s%s/%s", sbi->real_dst,
			 path_name, new_dentry->d_name.name);
		if (S_ISDIR(d_inode(old_dentry)->i_mode))
			lo_d_new = kern_path_create(AT_FDCWD, abs_path_buf,
						    &lo_p_new,
						    LOOKUP_DIRECTORY);
		else
			lo_d_new = kern_path_create(AT_FDCWD, abs_path_buf,
						    &lo_p_new, 0);
		if (IS_ERR(lo_d_new))
			continue;

		lo_d_new_dir = dget_parent(lo_d_new);
		lo_i_new_dir = d_inode(lo_d_new_dir);
		lo_d_old_dir = dget_parent(lo_d_old);
		lo_i_old_dir = d_inode(lo_d_old_dir);

		ret = vfs_rename(lo_i_old_dir, lo_d_old, lo_i_new_dir, lo_d_new,
				 NULL, flags);
		new_comrade = alloc_comrade(lo_p_new.dentry, comrade->dev_id);
		if (IS_ERR(new_comrade)) {
			ret = PTR_ERR(new_comrade);
			goto no_comrade;
		}

		link_comrade_unlocked(new_dentry, new_comrade);
no_comrade:
		done_path_create(&lo_p_new, lo_d_new);
		dput(lo_d_old_dir);
		dput(lo_d_new_dir);
	}
out:
	kfree(abs_path_buf);
	kfree(path_buf);
	return ret;
}

int hmdfs_rename_merge(struct inode *old_dir, struct dentry *old_dentry,
		       struct inode *new_dir, struct dentry *new_dentry,
		       unsigned int flags)
{
	char *old_dir_buf = NULL;
	char *new_dir_buf = NULL;
	char *old_dir_path = NULL;
	char *new_dir_path = NULL;
	struct dentry *old_dir_dentry = NULL;
	struct dentry *new_dir_dentry = NULL;
	int ret = 0;

	if (hmdfs_file_type(old_dentry->d_name.name) != HMDFS_TYPE_COMMON ||
	    hmdfs_file_type(new_dentry->d_name.name) != HMDFS_TYPE_COMMON) {
		ret = -EACCES;
		goto rename_out;
	}
	old_dir_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	new_dir_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!old_dir_buf || !new_dir_buf) {
		ret = -ENOMEM;
		goto rename_out;
	}

	new_dir_dentry = d_find_alias(new_dir);
	if (!new_dir_dentry) {
		ret = -EINVAL;
		goto rename_out;
	}

	old_dir_dentry = d_find_alias(old_dir);
	if (!old_dir_dentry) {
		ret = -EINVAL;
		dput(new_dir_dentry);
		goto rename_out;
	}

	old_dir_path = dentry_path_raw(old_dir_dentry, old_dir_buf, PATH_MAX);
	new_dir_path = dentry_path_raw(new_dir_dentry, new_dir_buf, PATH_MAX);
	dput(new_dir_dentry);
	dput(old_dir_dentry);
	if (strcmp(old_dir_path, new_dir_path)) {
		ret = -EPERM;
		goto rename_out;
	}

	trace_hmdfs_rename_merge(old_dir, old_dentry, new_dir, new_dentry,
				 flags);
	ret = do_rename_merge(old_dir, old_dentry, new_dir, new_dentry, flags);

	if (ret != 0)
		d_drop(new_dentry);

	if (S_ISREG(old_dentry->d_inode->i_mode) && !ret)
		d_invalidate(old_dentry);

rename_out:
	hmdfs_trace_rename_merge(old_dir, old_dentry, new_dir, new_dentry, ret);
	kfree(old_dir_buf);
	kfree(new_dir_buf);
	return ret;
}

static const char *hmdfs_get_link_merge(struct dentry *dentry,
					struct inode *inode,
					struct delayed_call *done)
{
	const char *link = NULL;
	struct dentry *lower_dentry = NULL;
	struct inode *lower_inode = NULL;

	if (!dentry) {
		hmdfs_err("dentry NULL");
		link = ERR_PTR(-ECHILD);
		goto link_out;
	}

	lower_dentry = hmdfs_get_fst_lo_d(dentry);
	if (!lower_dentry) {
		WARN_ON(1);
		link = ERR_PTR(-EINVAL);
		goto out;
	}
	lower_inode = d_inode(lower_dentry);
	if (!lower_inode->i_op || !lower_inode->i_op->get_link) {
		hmdfs_err("lower inode hold no operations");
		link = ERR_PTR(-EINVAL);
		goto out;
	}

	link = lower_inode->i_op->get_link(lower_dentry, lower_inode, done);
	if (IS_ERR_OR_NULL(link))
		goto out;
	fsstack_copy_attr_atime(inode, lower_inode);
out:
	dput(lower_dentry);
	trace_hmdfs_get_link_merge(inode, dentry, PTR_ERR_OR_ZERO(link));
link_out:
	return link;
}

const struct inode_operations hmdfs_symlink_iops_merge = {
	.get_link = hmdfs_get_link_merge,
	.permission = hmdfs_permission,
};

const struct inode_operations hmdfs_dir_iops_merge = {
	.lookup = hmdfs_lookup_merge,
	.mkdir = hmdfs_mkdir_merge,
	.create = hmdfs_create_merge,
	.rmdir = hmdfs_rmdir_merge,
	.unlink = hmdfs_unlink_merge,
	.symlink = hmdfs_symlink_merge,
	.rename = hmdfs_rename_merge,
	.permission = hmdfs_permission,
};
