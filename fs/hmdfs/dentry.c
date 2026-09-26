/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dentry.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: lizhipeng40@huawei.com maojingjing@huawei.com
 *         chenjinglong1@huawei.com
 * Create: 2021-01-11
 *
 */

#include <linux/ctype.h>
#include <linux/slab.h>

#include "comm/connection.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#include "hmdfs_merge_view.h"

#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/dentry_syncer.h"
#endif

extern struct kmem_cache *hmdfs_dentry_cachep;

void hmdfs_set_time(struct dentry *dentry, unsigned long time)
{
	struct hmdfs_dentry_info *d_info = dentry->d_fsdata;

	if (d_info)
		d_info->time = time;
}

unsigned long hmdfs_get_time(struct dentry *dentry)
{
	struct hmdfs_dentry_info *d_info = dentry->d_fsdata;

	if (d_info)
		return (unsigned long)d_info->time;
	return 0;
}

static int hmdfs_d_remote_revalidate(struct hmdfs_peer *conn,
				     struct dentry *target,
				     struct dentry *parent)
{
	unsigned int timeout = hmdfs_sb(target->d_sb)->dcache_timeout;
	unsigned long dentry_time = hmdfs_get_time(target);
	struct clearcache_item *item;

	item = hmdfs_find_cache_item(conn->device_id, parent);
	if (!item)
		return 0;
	kref_put(&item->ref, release_cache_item);

	if (cache_item_revalidate(READ_ONCE(conn->conn_time),
				  dentry_time, timeout))
		return 1;

	return 0;
}

static inline void lock_for_dname_cmp(struct dentry *dentry,
				      struct dentry *lower_dentry)
{
	if (dentry < lower_dentry) {
		spin_lock(&dentry->d_lock);
		spin_lock_nested(&lower_dentry->d_lock, DENTRY_D_LOCK_NESTED);
	} else {
		spin_lock(&lower_dentry->d_lock);
		spin_lock_nested(&dentry->d_lock, DENTRY_D_LOCK_NESTED);
	}
}

static inline void unlock_for_dname_cmp(struct dentry *dentry,
					struct dentry *lower_dentry)
{
	spin_unlock(&dentry->d_lock);
	spin_unlock(&lower_dentry->d_lock);
}

static int hmdfs_dev_d_revalidate(struct dentry *direntry, unsigned int flags)
{
	struct inode *dinode = NULL;
	struct hmdfs_inode_info *info = NULL;

#ifdef CONFIG_HMDFS_1_0
	dentry_syncer_wakeup();
#endif
	spin_lock(&direntry->d_lock);
	if (IS_ROOT(direntry)) {
		spin_unlock(&direntry->d_lock);
		return 1;
	}
	spin_unlock(&direntry->d_lock);

	dinode = d_inode(direntry);
	if (!dinode)
		return 0;

	info = hmdfs_i(dinode);
	if (info->inode_type == HMDFS_LAYER_SECOND_LOCAL ||
	    info->inode_type == HMDFS_LAYER_FIRST_DEVICE) {
		return 1;
	}
	if (info->conn && info->conn->status == NODE_STAT_ONLINE)
		return 1;

	return 0;
}

static int hmdfs_d_revalidate(struct dentry *direntry, unsigned int flags)
{
	struct inode *dinode = NULL;
	struct hmdfs_inode_info *info = NULL;
	struct path lower_path, parent_lower_path;
	struct dentry *parent_dentry = NULL;
	struct dentry *parent_lower_dentry = NULL;
	struct dentry *lower_cur_parent_dentry = NULL;
	struct dentry *lower_dentry = NULL;
	int ret;

#ifdef CONFIG_HMDFS_1_0
	dentry_syncer_wakeup();
#endif

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	if (flags & (LOOKUP_CREATE | LOOKUP_RENAME_TARGET | LOOKUP_REVAL))
		return 0;

	dinode = d_inode(direntry);
	if (!dinode)
		return 0;

	/* remote dentry timeout */
	info = hmdfs_i(dinode);
	parent_dentry = dget_parent(direntry);
	if (info->conn) {
		ret = hmdfs_d_remote_revalidate(info->conn, direntry,
						parent_dentry);
		dput(parent_dentry);
		return ret;
	}

	hmdfs_get_lower_path(direntry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_cur_parent_dentry = dget_parent(lower_dentry);
	hmdfs_get_lower_path(parent_dentry, &parent_lower_path);
	parent_lower_dentry = parent_lower_path.dentry;
	if ((lower_dentry->d_flags & DCACHE_OP_REVALIDATE)) {
		ret = lower_dentry->d_op->d_revalidate(lower_dentry, flags);
		if (ret == 0)
			goto out;
	}

	spin_lock(&lower_dentry->d_lock);
	if (d_unhashed(lower_dentry)) {
		spin_unlock(&lower_dentry->d_lock);
		ret = 0;
		goto out;
	}
	spin_unlock(&lower_dentry->d_lock);

	if (parent_lower_dentry != lower_cur_parent_dentry) {
		ret = 0;
		goto out;
	}

	ret = 1;
	lock_for_dname_cmp(direntry, lower_dentry);
	if (!qstr_case_eq(&direntry->d_name, &lower_dentry->d_name))
		ret = 0;
	unlock_for_dname_cmp(direntry, lower_dentry);

out:
	hmdfs_put_lower_path(&parent_lower_path);
	dput(lower_cur_parent_dentry);
	hmdfs_put_lower_path(&lower_path);
	dput(parent_dentry);
	return ret;
}

static void hmdfs_dev_d_release(struct dentry *dentry)
{
	if (!dentry || !dentry->d_fsdata)
		return;

	switch (hmdfs_d(dentry)->dentry_type) {
	case HMDFS_LAYER_SECOND_LOCAL:
		hmdfs_clear_cache_dents(dentry, false);
		hmdfs_drop_remote_cache_dents(dentry);
		path_put(&(hmdfs_d(dentry)->lower_path));
		break;
	case HMDFS_LAYER_ZERO:
		hmdfs_put_reset_lower_path(dentry);
		break;
	case HMDFS_LAYER_FIRST_DEVICE:
		break;
	case HMDFS_LAYER_SECOND_REMOTE:
		hmdfs_clear_cache_dents(dentry, false);
		break;
	default:
		hmdfs_err("Unexpected dentry type %d",
			  hmdfs_d(dentry)->dentry_type);
		return;
	}

	kmem_cache_free(hmdfs_dentry_cachep, dentry->d_fsdata);
	dentry->d_fsdata = NULL;
}

static void hmdfs_d_release(struct dentry *dentry)
{
	if (!dentry || !dentry->d_fsdata)
		return;

	hmdfs_clear_cache_dents(dentry, false);
	hmdfs_drop_remote_cache_dents(dentry);
	hmdfs_put_reset_lower_path(dentry);
	kmem_cache_free(hmdfs_dentry_cachep, dentry->d_fsdata);
	dentry->d_fsdata = NULL;
}

static int hmdfs_cmp_ci(const struct dentry *dentry, unsigned int len,
			const char *str, const struct qstr *name)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(dentry->d_sb);

	if (name->len != len)
		return 1;

	if (!sbi->s_case_sensitive) {
		if (str_n_case_eq(name->name, str, len))
			return 0;
	} else {
		if (!strncmp(name->name, str, len))
			return 0;
	}
	return 1;
}

static int hmdfs_hash_ci(const struct dentry *dentry, struct qstr *qstr)
{
	const unsigned char *name = qstr->name;
	unsigned int len = qstr->len;
	unsigned long hash;
	struct hmdfs_sb_info *sbi = hmdfs_sb(dentry->d_sb);

	if (sbi->s_case_sensitive)
		return 0;

	hash = init_name_hash(dentry);
	while (len--)
		hash = partial_name_hash(tolower(*name++), hash);
	qstr->hash = end_name_hash(hash);
	return 0;
}

void clear_comrades_locked(struct list_head *comrade_list)
{
	struct hmdfs_dentry_comrade *cc, *nc;

	WARN_ON(!comrade_list);
	list_for_each_entry_safe(cc, nc, comrade_list, list) {
		dput(cc->lo_d);
		kfree(cc);
	}
	INIT_LIST_HEAD(comrade_list);
}

void clear_comrades(struct dentry *dentry)
{
	struct hmdfs_dentry_info_merge *cdi = hmdfs_dm(dentry);

	mutex_lock(&cdi->comrade_list_lock);
	clear_comrades_locked(&cdi->comrade_list);
	mutex_unlock(&cdi->comrade_list_lock);
}

/**
 * d_revalidate_merge - revalidate a merge dentry
 *
 * Always return 0 to invalidate a dentry for fault-tolerance.
 * The cost is acceptable for a overlay filesystem.
 */
static int d_revalidate_merge(struct dentry *direntry, unsigned int flags)
{
#ifdef CONFIG_HMDFS_1_0
	dentry_syncer_wakeup();
#endif
	return 0;
}

static void d_release_merge(struct dentry *dentry)
{
	if (!dentry || !dentry->d_fsdata)
		return;

	clear_comrades(dentry);
	kmem_cache_free(hmdfs_dentry_merge_cachep, dentry->d_fsdata);
	dentry->d_fsdata = NULL;
}

const struct dentry_operations hmdfs_dops_merge = {
	.d_revalidate = d_revalidate_merge,
	.d_release = d_release_merge,
};

const struct dentry_operations hmdfs_dev_dops = {
	.d_revalidate = hmdfs_dev_d_revalidate,
	.d_release = hmdfs_dev_d_release,
};

const struct dentry_operations hmdfs_dops = {
	.d_revalidate = hmdfs_d_revalidate,
	.d_release = hmdfs_d_release,
	.d_compare = hmdfs_cmp_ci,
	.d_hash = hmdfs_hash_ci,
};
