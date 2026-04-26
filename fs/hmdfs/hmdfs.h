/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: weilongping@huawei.com
 * Create: 2020-03-26
 *
 */

#ifndef HMDFS_H
#define HMDFS_H

#include <linux/fs.h>
#include <linux/kfifo.h>
#include <linux/kobject.h>
#include <linux/kref.h>
#include <linux/sched.h>
#include <linux/version.h>

#include "comm/protocol.h"
#include "comm/fault_inject.h"

#if KERNEL_VERSION(4, 15, 0) < LINUX_VERSION_CODE
#define hmdfs_time_t	   timespec64
#define hmdfs_time_compare timespec64_compare
#define hmdfs_time_add	   timespec64_add
#else
#define hmdfs_time_t	   timespec
#define hmdfs_time_compare timespec_compare
#define hmdfs_time_add	   timespec_add
#endif

#define HMDFS_PAGE_SIZE	  4096
#define HMDFS_PAGE_OFFSET 12

/* max xattr value size, not include '\0' */
#define HMDFS_XATTR_SIZE_MAX	4096
/* max listxattr response size, include '\0' */
#define HMDFS_LISTXATTR_SIZE_MAX 4096

// 20 digits +'\0', Converted from a u64 integer
#define HMDFS_ACCOUNT_HASH_MAX_LEN 21
#define CTRL_PATH_MAX_LEN	   21

#define HMDFS_SUPER_MAGIC 0x20200302

#define DEFAULT_WRITE_CACHE_TIMEOUT 30
#define DEFAULT_SRV_REQ_MAX_ACTIVE 16

#define HMDFS_INODE_INVALID_FILE_ID	(1U << 31)
#define HMDFS_FID_VER_BOOT_COOKIE_SHIFT	15

/* According to task_struct instead of workqueue_struct */
#define HMDFS_WQ_NAME_LEN 16

#define HMDFS_DEF_WB_TIMEOUT_MS 60000
#define HMDFS_MAX_WB_TIMEOUT_MS 900000

#define HMDFS_READPAGES_NR_MAX	32

enum {
	HMDFS_FEATURE_READPAGES		= 1ULL << 0,
	HMDFS_FEATURE_READPAGES_OPEN	= 1ULL << 1,
	HMDFS_ATOMIC_OPEN		= 1ULL << 2,
};

struct client_statistic;
struct server_statistic;
struct hmdfs_writeback;
struct hmdfs_server_writeback;
struct hmdfs_syncfs_info {
	wait_queue_head_t wq;
	atomic_t wait_count;
	int remote_ret;
	unsigned long long version;

	/* Protect version in concurrent operations */
	spinlock_t v_lock;
	/*
	 * Serialize hmdfs_sync_fs() process:
	 *  |<- pending_list ->|   exexuting    |<-  wait_list  ->|
	 *   syncfs_1 syncfs_2     (syncfs_3)    syncfs_4 syncfs_5
	 *
	 * Abandon syncfs processes in pending_list after syncfs_3 finished;
	 * Pick the last syncfs process in wait_list after syncfs_3 finished;
	 */
	bool is_executing;
	/* syncfs process arriving after current exexcuting syncfs */
	struct list_head wait_list;
	/* syncfs process arriving before current exexcuting syncfs */
	struct list_head pending_list;
	spinlock_t list_lock;
};

struct hmdfs_sb_info {
	/* list for all registered superblocks */
	struct list_head list;
	struct mutex umount_mutex;

	struct kobject kobj;
	struct completion s_kobj_unregister;
	struct super_block *sb;
	struct super_block *lower_sb;
	/* from mount, which is root */
	const struct cred *cred;
	/* from update cmd, expected to be system */
	const struct cred *system_cred;
	struct {
		struct mutex node_lock;
		struct list_head node_list;
		atomic_t conn_seq;
		unsigned long recent_ol;
	} connections;
	char *local_dst;
	char *real_dst;
	struct {
		uint64_t iid;
		char account[HMDFS_ACCOUNT_HASH_MAX_LEN];
	} local_info;
	char *local_src;
	char *cache_dir;
	/* seq number for hmdfs super block */
	unsigned int seq;

	/*
	 * This value indicate how long the pagecache stay valid(in seconds) in
	 * client if metadate(except iversion) is equal to server. This
	 * functionality is disabled if this value is 0.
	 */
	unsigned int write_cache_timeout;
	unsigned int dcache_timeout;
	unsigned int dcache_precision;
	unsigned long dcache_threshold;
	struct list_head client_cache;
	struct list_head server_cache;
	struct list_head to_delete;
	struct mutex cache_list_lock;

	/* local operation time statistic */
	struct server_statistic *s_server_statis;

	/* client statistic */
	struct client_statistic *s_client_statis;

	/* TIMEOUT of each command */
	struct kobject s_cmd_timeout_kobj;
	struct completion s_timeout_kobj_unregister;
	unsigned int s_cmd_timeout[F_SIZE];

	/* For case sensitive */
	bool s_case_sensitive;

	/* For features supporting */
	u64 s_features;

	/* For merge & device view */
	unsigned int s_merge_switch;
	/* For writeback */
	struct hmdfs_writeback *h_wb;
	/* For server writeback */
	struct hmdfs_server_writeback *h_swb;

	/* syncfs info */
	struct hmdfs_syncfs_info hsi;

	/* To bridge the userspace utils */
	struct kfifo notify_fifo;
	spinlock_t notify_fifo_lock;
	struct hmdfs_fault_inject fault_inject;

	/* For reboot detect */
	uint64_t boot_cookie;
	/* offline process */
	unsigned int async_cb_delay;
	/* For server handle requests */
	unsigned int async_req_max_active;
	/* stash dirty pages during offline */
	bool s_offline_stash;

	/* Timeout (ms) to retry writing remote pages */
	unsigned int wb_timeout_ms;

	struct path stash_work_dir;
	/* dentry cache */
	bool s_dentry_cache;

	/* msgs that are waiting for remote */
	struct list_head async_readdir_msg_list;
	/* protect async_readdir_msg_list */
	spinlock_t async_readdir_msg_lock;
	/* async readdir work that are queued but not finished */
	struct list_head async_readdir_work_list;
	/* protect async_readdir_work_list */
	spinlock_t async_readdir_work_lock;
	/* wait for async_readdir_work_list to be empty in umount */
	wait_queue_head_t async_readdir_wq;
	/* don't allow async readdir */
	bool async_readdir_prohibit;
};

static inline struct hmdfs_sb_info *hmdfs_sb(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline bool hmdfs_is_stash_enabled(const struct hmdfs_sb_info *sbi)
{
	return sbi->s_offline_stash;
}

struct setattr_info {
	loff_t size;
	unsigned int valid;
	umode_t mode;
	kuid_t uid;
	kgid_t gid;
	long long atime;
	long atime_nsec;
	long long mtime;
	long mtime_nsec;
	long long ctime;
	long ctime_nsec;
};

struct hmdfs_file_info {
	union {
		struct {
			struct rb_root root;
			struct mutex comrade_list_lock;
		};
		struct {
			struct file *lower_file;
			int device_id;
		};
	};
	struct list_head comrade_list;
};

static inline struct hmdfs_file_info *hmdfs_f(struct file *file)
{
	return file->private_data;
}

// Almost all the source files want this, so...
#include "inode.h"

/* locking helpers */
static inline struct dentry *lock_parent(struct dentry *dentry)
{
	struct dentry *dir = dget_parent(dentry);

	inode_lock_nested(d_inode(dir), I_MUTEX_PARENT);
	return dir;
}

static inline void unlock_dir(struct dentry *dir)
{
	inode_unlock(d_inode(dir));
	dput(dir);
}

extern uint64_t path_hash(const char *path, int len, bool case_sense);
extern int vfs_path_lookup(struct dentry *dentry, struct vfsmount *mnt,
			   const char *name, unsigned int flags,
			   struct path *path);
extern ssize_t hmdfs_remote_listxattr(struct dentry *dentry, char *buffer,
				      size_t size);

int check_filename(const char *name, int len);

int hmdfs_permission(struct inode *inode, int mask);

int hmdfs_parse_options(struct hmdfs_sb_info *sbi, const char *data);

/* Refer to comments in hmdfs_request_work_fn() */
#define HMDFS_SERVER_CTX_FLAGS (PF_KTHREAD | PF_WQ_WORKER | PF_NPROC_EXCEEDED)

static inline bool is_current_hmdfs_server_ctx(void)
{
	return ((current->flags & HMDFS_SERVER_CTX_FLAGS) ==
		HMDFS_SERVER_CTX_FLAGS);
}

extern uint64_t hmdfs_gen_boot_cookie(void);

static inline bool str_n_case_eq(const char *s1, const char *s2, size_t len)
{
	return !strncasecmp(s1, s2, len);
}

static inline bool qstr_case_eq(const struct qstr *q1, const struct qstr *q2)
{
	return q1->len == q2->len && str_n_case_eq(q1->name, q2->name, q2->len);
}

/*****************************************************************************
 * log print helpers
 *****************************************************************************/
__printf(4, 5) void __hmdfs_log(const char *level, const bool ratelimited,
				const char *function, const char *fmt, ...);
#define hmdfs_err(fmt, ...)	\
	__hmdfs_log(KERN_ERR, false, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_warning(fmt, ...) \
	__hmdfs_log(KERN_WARNING, false, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_info(fmt, ...) \
	__hmdfs_log(KERN_INFO, false, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_err_ratelimited(fmt, ...)	\
	__hmdfs_log(KERN_ERR, true, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_warning_ratelimited(fmt, ...) \
	__hmdfs_log(KERN_WARNING, true, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_info_ratelimited(fmt, ...) \
	__hmdfs_log(KERN_INFO, true, __func__, fmt, ##__VA_ARGS__)
#ifdef CONFIG_HMDFS_FS_DEBUG
#define hmdfs_debug(fmt, ...) \
	__hmdfs_log(KERN_DEBUG, false, __func__, fmt, ##__VA_ARGS__)
#define hmdfs_debug_ratelimited(fmt, ...) \
	__hmdfs_log(KERN_DEBUG, true, __func__, fmt, ##__VA_ARGS__)
#else
#define hmdfs_debug(fmt, ...)       ((void)0)
#define hmdfs_debug_ratelimited(fmt, ...)       ((void)0)
#endif

/*****************************************************************************
 * inode/file operations declartion
 *****************************************************************************/
extern const struct inode_operations hmdfs_device_ops;
extern const struct inode_operations hmdfs_root_ops;
extern const struct file_operations hmdfs_root_fops;
extern const struct file_operations hmdfs_device_fops;

#endif // HMDFS_H
