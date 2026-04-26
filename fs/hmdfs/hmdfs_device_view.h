/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_device_view.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: lvhao13@huawei.com maojingjing@huawei.com
 * Create: 2021-01-11
 *
 */

#ifndef HMDFS_DEVICE_VIEW_H
#define HMDFS_DEVICE_VIEW_H

#include "hmdfs.h"

/*****************************************************************************
 * macro defination
 *****************************************************************************/

#define DEVICE_VIEW_ROOT "device_view"
#define MERGE_VIEW_ROOT	 "merge_view"
#define UPDATE_LOCAL_DST "/device_view/local/"

#define DEVICE_VIEW_LOCAL "local"

/*
 * in order to distinguish from vfs, we define our own bitmask, this should
 * covert to vfs bitmask while calling vfs apis
 */
#define HMDFS_LOOKUP_REVAL 0x1

enum HMDFS_FILE_TYPE {
	HM_REG = 0,
	HM_SYMLINK = 1,

	HM_MAX_FILE_TYPE = 0XFF
};

struct bydev_inode_info {
	struct inode *lower_inode;
	uint64_t ino;
};

struct hmdfs_dentry_info {
	struct path lower_path;
	unsigned long time;
	struct list_head cache_list_head;
	spinlock_t cache_list_lock;
	struct list_head remote_cache_list_head;
	struct mutex remote_cache_list_lock;
	__u8 file_type;
	__u8 dentry_type;
	uint64_t device_id;
	spinlock_t lock;
	struct mutex cache_pull_lock;
	bool async_readdir_in_progress;
};

struct hmdfs_lookup_ret {
	uint64_t i_size;
	uint64_t i_mtime;
	uint32_t i_mtime_nsec;
#ifdef CONFIG_HMDFS_1_0
	uint32_t fno;
#endif
	uint16_t i_mode;
	uint64_t i_ino;
};

struct hmdfs_getattr_ret {
	/*
	 * if stat->result_mask is 0, it means this remote getattr failed with
	 * look up, see details in hmdfs_server_getattr.
	 */
	struct kstat stat;
	uint32_t i_flags;
	uint64_t fsid;
};

extern int hmdfs_remote_getattr(struct hmdfs_peer *conn, struct dentry *dentry,
				unsigned int lookup_flags,
				struct hmdfs_getattr_ret **getattr_result);

/*****************************************************************************
 * local/remote inode/file operations
 *****************************************************************************/

extern const struct dentry_operations hmdfs_dops;
extern const struct dentry_operations hmdfs_dev_dops;

/* local device operation */
extern const struct inode_operations hmdfs_file_iops_local;
extern const struct file_operations hmdfs_file_fops_local;
extern const struct inode_operations hmdfs_dir_inode_ops_local;
extern const struct file_operations hmdfs_dir_ops_local;
extern const struct inode_operations hmdfs_symlink_iops_local;

/* remote device operation */
extern const struct inode_operations hmdfs_dev_file_iops_remote;
extern const struct file_operations hmdfs_dev_file_fops_remote;
extern const struct address_space_operations hmdfs_dev_file_aops_remote;
extern const struct inode_operations hmdfs_dev_dir_inode_ops_remote;
extern const struct file_operations hmdfs_dev_dir_ops_remote;
extern int hmdfs_dev_unlink_from_con(struct hmdfs_peer *conn,
				     struct dentry *dentry);
extern int hmdfs_dev_readdir_from_con(struct hmdfs_peer *con, struct file *file,
				      struct dir_context *ctx);
int hmdfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
int hmdfs_rmdir(struct inode *dir, struct dentry *dentry);
int hmdfs_create(struct inode *dir, struct dentry *dentry, umode_t mode,
		 bool want_excl);
int hmdfs_unlink(struct inode *dir, struct dentry *dentry);
int hmdfs_remote_unlink(struct hmdfs_peer *conn, struct dentry *dentry);
int hmdfs_rename(struct inode *old_dir, struct dentry *old_dentry,
		 struct inode *new_dir, struct dentry *new_dentry,
		 unsigned int flags);
loff_t hmdfs_file_llseek_local(struct file *file, loff_t offset, int whence);
ssize_t hmdfs_read_local(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos);
ssize_t hmdfs_write_local(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos);
int hmdfs_file_release_local(struct inode *inode, struct file *file);
int hmdfs_file_mmap_local(struct file *file, struct vm_area_struct *vma);
struct dentry *hmdfs_lookup(struct inode *parent_inode,
			    struct dentry *child_dentry, unsigned int flags);
struct dentry *hmdfs_lookup_local(struct inode *parent_inode,
				  struct dentry *child_dentry,
				  unsigned int flags);
struct dentry *hmdfs_lookup_remote(struct inode *parent_inode,
				   struct dentry *child_dentry,
				   unsigned int flags);
int hmdfs_symlink_local(struct inode *dir, struct dentry *dentry,
			const char *symname);
int hmdfs_fsync_local(struct file *file, loff_t start, loff_t end,
		      int datasync);
int hmdfs_symlink(struct inode *dir, struct dentry *dentry,
		  const char *symname);
int hmdfs_fsync(struct file *file, loff_t start, loff_t end, int datasync);

/*****************************************************************************
 * common functions declaration
 *****************************************************************************/

static inline struct hmdfs_dentry_info *hmdfs_d(struct dentry *dentry)
{
	return dentry->d_fsdata;
}
/*
 * hmdfs_hash - generate a hashval for remote inode
 * @iid: id of remote device
 * @ino: remote inode number
 */
static inline bool hm_isreg(uint8_t file_type)
{
	return (file_type == HM_REG);
}

static inline bool hm_islnk(uint8_t file_type)
{
	return (file_type == HM_SYMLINK);
}
struct inode *fill_inode_remote(struct super_block *sb, struct hmdfs_peer *con,
				struct hmdfs_lookup_ret *lookup_result,
				struct inode *dir);
struct hmdfs_lookup_ret *get_remote_inode_info(struct hmdfs_peer *con,
					       struct dentry *dentry,
					       unsigned int flags);
void hmdfs_set_time(struct dentry *dentry, unsigned long time);
struct inode *fill_inode_local(struct super_block *sb,
			       struct inode *lower_inode);
struct inode *fill_root_inode(struct super_block *sb,
			      struct inode *lower_inode);
struct inode *fill_device_inode(struct super_block *sb,
				struct inode *lower_inode);
struct hmdfs_lookup_ret *hmdfs_lookup_by_con(struct hmdfs_peer *con,
					     struct dentry *dentry,
					     struct qstr *qstr,
					     unsigned int flags,
					     const char *relative_path);
char *hmdfs_connect_path(const char *path, const char *name);

char *hmdfs_get_dentry_relative_path(struct dentry *dentry);
char *hmdfs_get_dentry_absolute_path(const char *rootdir,
				     const char *relative_path);
int hmdfs_convert_lookup_flags(unsigned int hmdfs_flags,
			       unsigned int *vfs_flags);
static inline void hmdfs_get_lower_path(struct dentry *dent, struct path *pname)
{
	spin_lock(&hmdfs_d(dent)->lock);
	pname->dentry = hmdfs_d(dent)->lower_path.dentry;
	pname->mnt = hmdfs_d(dent)->lower_path.mnt;
	path_get(pname);
	spin_unlock(&hmdfs_d(dent)->lock);
}

static inline void hmdfs_put_lower_path(struct path *pname)
{
	path_put(pname);
}

static inline void hmdfs_put_reset_lower_path(struct dentry *dent)
{
	struct path pname;

	spin_lock(&hmdfs_d(dent)->lock);
	if (hmdfs_d(dent)->lower_path.dentry) {
		pname.dentry = hmdfs_d(dent)->lower_path.dentry;
		pname.mnt = hmdfs_d(dent)->lower_path.mnt;
		hmdfs_d(dent)->lower_path.dentry = NULL;
		hmdfs_d(dent)->lower_path.mnt = NULL;
		spin_unlock(&hmdfs_d(dent)->lock);
		path_put(&pname);
	} else {
		spin_unlock(&hmdfs_d(dent)->lock);
	}
}

static inline void hmdfs_set_lower_path(struct dentry *dent, struct path *pname)
{
	spin_lock(&hmdfs_d(dent)->lock);
	hmdfs_d(dent)->lower_path.dentry = pname->dentry;
	hmdfs_d(dent)->lower_path.mnt = pname->mnt;
	spin_unlock(&hmdfs_d(dent)->lock);
}

/* Only reg file for HMDFS_LAYER_OTHER_* support xattr */
static inline bool hmdfs_support_xattr(struct dentry *dentry)
{
	struct inode *inode = d_inode(dentry);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_dentry_info *gdi = hmdfs_d(dentry);

	if (info->inode_type != HMDFS_LAYER_OTHER_LOCAL &&
	    info->inode_type != HMDFS_LAYER_OTHER_REMOTE)
		return false;

	if (!S_ISREG(inode->i_mode))
		return false;

	if (hm_islnk(gdi->file_type))
		return false;

	return true;
}

int init_hmdfs_dentry_info(struct hmdfs_sb_info *sbi, struct dentry *dentry,
			   int dentry_type);

#endif
