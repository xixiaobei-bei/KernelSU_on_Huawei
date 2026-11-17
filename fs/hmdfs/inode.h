/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chenjinglong1@huawei.com
 * Create: 2021-01-19
 *
 */

#ifndef INODE_H
#define INODE_H

#include "hmdfs.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
#include <linux/iversion.h>
#endif

enum {
	HMDFS_REMOTE_INODE_NONE = 0,
	HMDFS_REMOTE_INODE_STASHING,
	HMDFS_REMOTE_INODE_RESTORING,
};

/*****************************************************************************
 * fid
 *****************************************************************************/

/* Bits for fid_flags */
enum {
	HMDFS_FID_NEED_OPEN = 0,
	HMDFS_FID_OPENING,
};

struct hmdfs_fid {
	__u64 ver;
	__u32 id;
};

/*
 * Cache file is stored in file like following format:
 *  ________________________________________________________________
 * |meta file info| remote file(s) path |        file content       |
 * |     head     |        path         |            data           |
 *                ↑                     ↑
 *             path_offs             data_offs
 */
struct hmdfs_cache_info {
	/* Path start offset in file (HMDFS_STASH_BLK_SIZE aligned) */
	__u32 path_offs;
	__u32 path_len;
	__u32 path_cnt;
	char *path_buf;
	/* Stricky remote file(hardlink)s' path, splitted by '\0' */
	char *path;
	/* Data start offset in file (HMDFS_STASH_BLK_SIZE aligned) */
	__u32 data_offs;
	/* # of pages need to be written to remote file during offline */
	atomic64_t to_write_pgs;
	/* # of pages written to remote file during offline */
	atomic64_t written_pgs;
	/* Stash file handler */
	struct file *cache_file;
};

/*****************************************************************************
 * inode info and it's inline helpers
 *****************************************************************************/

struct hmdfs_inode_info {
	struct inode *lower_inode; // for local/merge inode
	struct hmdfs_peer *conn;   // for remote inode
	struct kref ref;
	spinlock_t fid_lock;
	struct hmdfs_fid fid;
	unsigned long fid_flags;
	wait_queue_head_t fid_wq;
	__u8 inode_type; // deprecated: use ino system instead

	/* writeback list */
	struct list_head wb_list;

#ifdef CONFIG_HMDFS_1_0
	__u32 file_no;
	__u8 adapter_dentry_flag;
#endif
#ifdef CONFIG_HMDFS_ANDROID
	__u16 perm;
#endif
	/*
	 * lookup remote file will generate a local inode, this store the
	 * combination of remote inode number and generation in such situation.
	 * Conbined with 'conn->iid', the uniqueness of local inode can be
	 * determined.
	 */
	__u64 remote_ino;
	/*
	 * if this value is not ULLONG_MAX, it means that remote getattr syscall
	 * should return this value as inode size.
	 */
	__u64 getattr_isize;
	/*
	 * this value stores remote ctime, explicitly when remote file is opened
	 */
	struct hmdfs_time_t remote_ctime;
	/*
	 * this value stores the last time, aligned to dcache_precision, that
	 * remote file was modified. It should be noted that this value won't
	 * be effective if writecace_expire is set.
	 */
	struct hmdfs_time_t stable_ctime;
	/*
	 * If this value is set nonzero, pagecache should be truncated if the
	 * time that the file is opened is beyond the value. Furthermore,
	 * the functionality of stable_ctime won't be effective.
	 */
	unsigned long writecache_expire;
	/*
	 * This value record how many times the file has been written while file
	 * is opened. 'writecache_expire' will set in close if this value is
	 * nonzero.
	 */
	atomic64_t write_counter;
	/*
	 * will be linked to hmdfs_peer::wr_opened_inode_list
	 * if the remote inode is writable-opened. And using
	 * wr_opened_cnt to track possibly multiple writeable-open.
	 */
	struct list_head wr_opened_node;
	atomic_t wr_opened_cnt;
	spinlock_t stash_lock;
	unsigned int stash_status;
	struct hmdfs_cache_info *cache;
	/* link to hmdfs_peer::stashed_inode_list when stashing completes */
	struct list_head stash_node;
	/*
	 * The flush/fsync thread will hold the write lock while threads
	 * calling writepage will hold the read lock. We use rwlock to
	 * eliminate the cases that flush/fsync operations are done with
	 * re-dirtied pages remain dirty.
	 *
	 * Here is the explanation in detail:
	 *
	 * During `writepage()`, the state of a re-dirtied page will switch
	 * to the following states in sequence:
	 * s1: page dirty + tree dirty
	 * s2: page dirty + tree dirty <tag_pages_for_writeback>
	 * s3: page clean + tree dirty <clear_page_dirty_for_io>
	 * s4: page clean + tree clean + write back <set_page_writeback>
	 * s5: page dirty + tree dirty + write back <redirty_page_for_writepage>
	 * s6: page dirty + tree dirty <end_page_writeback>
	 *
	 * A page upon s4 will thus be ignored by the concurrent
	 * `do_writepages()` contained by `close()`, `fsync()`, making it's
	 * state inconsistent.
	 *
	 * To avoid such situation, we use per-file rwsems to prevent
	 * concurrent in-flight `writepage` during `close()` or `fsync()`.
	 *
	 * Minimal overhead is brought in since rsems allow concurrent
	 * `writepage` while `close()` or `fsync()` is natural to wait for
	 * in-flight `writepage()`s to complete.
	 *
	 * NOTE that in the worst case, a process may wait for wsem for TIMEOUT
	 * even if a signal is pending. But we've to wait there to iterate all
	 * pages and make sure that no dirty page should remain.
	 */
	struct rw_semaphore wpage_sem;

	// The real inode shared with vfs. ALWAYS PUT IT AT THE BOTTOM.
	struct inode vfs_inode;
};

struct hmdfs_readdir_work {
	struct list_head head;
	struct dentry *dentry;
	struct hmdfs_peer *con;
	struct delayed_work dwork;
};

static inline struct hmdfs_inode_info *hmdfs_i(struct inode *inode)
{
	return container_of(inode, struct hmdfs_inode_info, vfs_inode);
}

static inline bool hmdfs_inode_is_stashing(const struct hmdfs_inode_info *info)
{
	const struct hmdfs_sb_info *sbi = hmdfs_sb(info->vfs_inode.i_sb);

	/* Refer to comments in hmdfs_stash_remote_inode() */
	return (hmdfs_is_stash_enabled(sbi) &&
		smp_load_acquire(&info->stash_status));
}

static inline void hmdfs_remote_fetch_fid(struct hmdfs_inode_info *info,
					  struct hmdfs_fid *fid)
{
	spin_lock(&info->fid_lock);
	*fid = info->fid;
	spin_unlock(&info->fid_lock);
}

/*****************************************************************************
 * ino allocator
 *****************************************************************************/

enum HMDFS_ROOT {
	HMDFS_ROOT_ANCESTOR = 1, // /
	HMDFS_ROOT_DEV,		 // /device_view
	HMDFS_ROOT_DEV_LOCAL,	 // /device_view/local
	HMDFS_ROOT_DEV_REMOTE,	 // /device_view/remote
	HMDFS_ROOT_MERGE,	 // /merge_view

	HMDFS_ROOT_INVALID,
};

// delete layer, directory layer, not overlay layer
enum HMDFS_LAYER_TYPE {
	HMDFS_LAYER_ZERO = 0,	   // /
	HMDFS_LAYER_FIRST_DEVICE,  // /device_view
	HMDFS_LAYER_SECOND_LOCAL,  // /device_view/local
	HMDFS_LAYER_SECOND_REMOTE, // /device_view/remote
	HMDFS_LAYER_OTHER_LOCAL,   // /device_view/local/xx
	HMDFS_LAYER_OTHER_REMOTE,  // /device_view/remote/xx

	HMDFS_LAYER_FIRST_MERGE, // /merge_view
	HMDFS_LAYER_OTHER_MERGE, // /merge_view/xxx

	HMDFS_LAYER_DFS_1_0,

	HMDFS_LAYER_INVALID,
};

struct inode *hmdfs_iget_locked_root(struct super_block *sb, uint64_t root_ino,
				     struct inode *lo_i,
				     struct hmdfs_peer *peer);
struct inode *hmdfs_iget_locked_dfs_1_0(struct super_block *sb,
					struct hmdfs_peer *peer);
struct inode *hmdfs_iget5_locked_merge(struct super_block *sb,
				       struct inode *fst_lo_i);

struct inode *hmdfs_iget5_locked_local(struct super_block *sb,
				       struct inode *lo_i);
struct hmdfs_peer;
struct inode *hmdfs_iget5_locked_remote(struct super_block *sb,
					struct hmdfs_peer *peer,
					uint64_t remote_ino);
int hmdfs_inode_type(struct inode *inode);

#endif // INODE_H
