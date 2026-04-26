/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_merge_view.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chenjinglong1@huawei.com
 * Create: 2021-01-11
 *
 */

#ifndef HMDFS_MERGE_VIEW_H
#define HMDFS_MERGE_VIEW_H

#include "hmdfs.h"

#include "comm/connection.h"
#include <linux/slab.h>

/*****************************************************************************
 * Dentires for merge view and their comrades.
 * A dentry's lower dentry is named COMRADE.
 *****************************************************************************/

struct hmdfs_dentry_info_merge {
	unsigned long ctime;
	// For the merge view to link dentries with same names
	struct mutex comrade_list_lock;
	struct list_head comrade_list;
};

struct hmdfs_dentry_comrade {
	uint64_t dev_id;
	struct dentry *lo_d;
	struct list_head list;
};

enum FILE_CMD_MERGE {
	F_MKDIR_MERGE = 0,
	F_CREATE_MERGE = 1,
	F_SYMLINK_MERGE = 2,
};

struct hmdfs_recursive_para {
	bool is_last;
	int opcode;
	umode_t mode;
	bool want_excl;
	const char *name;
};
static inline struct hmdfs_dentry_info_merge *hmdfs_dm(struct dentry *dentry)
{
	return dentry->d_fsdata;
}

static inline umode_t hmdfs_cm(struct hmdfs_dentry_comrade *comrade)
{
	return d_inode(comrade->lo_d)->i_mode;
}

static inline bool comrade_is_local(struct hmdfs_dentry_comrade *comrade)
{
	return comrade->dev_id == HMDFS_DEVID_LOCAL;
}

struct dentry *hmdfs_lookup_merge(struct inode *parent_inode,
				  struct dentry *child_dentry,
				  unsigned int flags);

struct hmdfs_dentry_comrade *alloc_comrade(struct dentry *lo_d, int dev_id);

void link_comrade(struct list_head *onstack_comrades_head,
		  struct hmdfs_dentry_comrade *comrade);

static inline void destory_comrade(struct hmdfs_dentry_comrade *comrade)
{
	dput(comrade->lo_d);
	kfree(comrade);
}

void clear_comrades(struct dentry *dentry);

static inline void link_comrade_unlocked(struct dentry *dentry,
					 struct hmdfs_dentry_comrade *comrade)
{
	mutex_lock(&hmdfs_dm(dentry)->comrade_list_lock);
	link_comrade(&hmdfs_dm(dentry)->comrade_list, comrade);
	mutex_unlock(&hmdfs_dm(dentry)->comrade_list_lock);
}

void clear_comrades_locked(struct list_head *comrade_list);

#define for_each_comrade_locked(_dentry, _comrade)                             \
	list_for_each_entry(_comrade, &(hmdfs_dm(_dentry)->comrade_list), list)

#define hmdfs_trace_merge(_trace_func, _parent_inode, _child_dentry, err)      \
	{                                                                      \
		struct hmdfs_dentry_comrade *comrade;                          \
		struct hmdfs_dentry_info_merge *dm = hmdfs_dm(_child_dentry);  \
		_trace_func(_parent_inode, _child_dentry, err);                \
		if (likely(dm)) {                                              \
			mutex_lock(&dm->comrade_list_lock);                    \
			for_each_comrade_locked(_child_dentry, comrade)        \
				trace_hmdfs_show_comrade(_child_dentry,        \
							 comrade->lo_d,        \
							 comrade->dev_id);     \
			mutex_unlock(&dm->comrade_list_lock);                  \
		}                                                              \
	}

#define hmdfs_trace_rename_merge(olddir, olddentry, newdir, newdentry, err)    \
	{                                                                      \
		struct hmdfs_dentry_comrade *comrade;                          \
		trace_hmdfs_rename_merge(olddir, olddentry, newdir, newdentry, \
					 err);                                 \
		mutex_lock(&hmdfs_dm(olddentry)->comrade_list_lock);           \
		for_each_comrade_locked(olddentry, comrade)                    \
			trace_hmdfs_show_comrade(olddentry, comrade->lo_d,     \
						 comrade->dev_id);             \
		mutex_unlock(&hmdfs_dm(olddentry)->comrade_list_lock);         \
		mutex_lock(&hmdfs_dm(newdentry)->comrade_list_lock);           \
		for_each_comrade_locked(newdentry, comrade)                    \
			trace_hmdfs_show_comrade(newdentry, comrade->lo_d,     \
						 comrade->dev_id);             \
		mutex_unlock(&hmdfs_dm(newdentry)->comrade_list_lock);         \
	}

/*****************************************************************************
 * Helper functions abstarcting out comrade
 *****************************************************************************/

static inline bool hmdfs_i_merge(struct hmdfs_inode_info *hii)
{
	__u8 t = hii->inode_type;
	return t == HMDFS_LAYER_FIRST_MERGE || t == HMDFS_LAYER_OTHER_MERGE;
}

struct dentry *hmdfs_get_lo_d(struct dentry *dentry, int dev_id);
struct dentry *hmdfs_get_fst_lo_d(struct dentry *dentry);

/*****************************************************************************
 * Inode operations for the merge view
 *****************************************************************************/

extern const struct inode_operations hmdfs_file_iops_merge;
extern const struct file_operations hmdfs_file_fops_merge;
extern const struct inode_operations hmdfs_symlink_iops_merge;
extern const struct inode_operations hmdfs_dir_iops_merge;
extern const struct file_operations hmdfs_dir_fops_merge;
extern const struct dentry_operations hmdfs_dops_merge;

/*****************************************************************************
 * dentry cache for the merge view
 *****************************************************************************/
extern struct kmem_cache *hmdfs_dentry_merge_cachep;

#endif // HMDFS_MERGE_H
