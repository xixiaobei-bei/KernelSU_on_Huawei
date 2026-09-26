/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_dentryfile.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lousong@huawei.com
 * Create: 2020-04-15
 *
 */

#ifndef HMDFS_DENTRYFILE_H
#define HMDFS_DENTRYFILE_H

#include "hmdfs.h"
#include <linux/namei.h>

/* use for escape from hmdfs file system, hmdfs hide follow names */
#define CURRENT_DIR "."
#define PARENT_DIR  ".."

/* local dentry cache data */
#define DENTRY_FILE_XATTR_NAME "user.hmdfs_cache"

#define DENTRY_FILE_NAME_RETRY 10

#define MAX_BUCKET_LEVEL 63
#define BUCKET_BLOCKS	 2
#define MAX_DIR_BUCKETS	 (1 << ((MAX_BUCKET_LEVEL / 2) - 1))

#define CONFLICTING_FILE_CONST_SUFFIX "_conflict_dev"
#define CONFLICTING_FILE_SUFFIX	      "_conflict_dev%u"
#define CONFLICTING_DIR_SUFFIX	      "_remote_directory"

#define POS_BIT_NUM	 64
#define DEV_ID_BIT_NUM	 16
#define GROUP_ID_BIT_NUM 39
#define OFFSET_BIT_NUM	 8
#define OFFSET_BIT_MASK	 0xFF

#define DEFAULT_DCACHE_TIMEOUT	 30
#define DEFAULT_DCACHE_PRECISION 10
#define DEFAULT_DCACHE_THRESHOLD 1000
#define HMDFS_STALE_REMOTE_ISIZE ULLONG_MAX

/* Seconds per-week */
#define MAX_DCACHE_TIMEOUT 604800

struct hmdfs_iterate_callback {
	struct dir_context ctx;
	struct dir_context *caller;
	int result;
	struct rb_root *root;
};

#ifdef CONFIG_HMDFS_1_0
#define INVALID_FILE_ID 0
#endif

/*
 * 4096 = version(1) + bitmap(10) + reserved(5)
 *        + nsl(80 * 43) + filename(80 * 8)
 */
#define DENTRYGROUP_SIZE       4096
#define DENTRY_NAME_LEN	       8
#define DENTRY_RESERVED_LENGTH 3
#define DENTRY_PER_GROUP       80
#define DENTRY_BITMAP_LENGTH   10
#define DENTRY_GROUP_RESERVED  5
#define DENTRYGROUP_HEADER     4096

struct hmdfs_dentry {
	__le32 hash;
	__le16 i_mode;
	__le16 namelen;
	__le64 i_size;
	/* modification time */
	__le64 i_mtime;
	/* modification time in nano scale */
	__le32 i_mtime_nsec;
	/* combination of inode number and generation */
	__le64 i_ino;
	__le32 i_flag;
	/* reserved bytes for long term extend, total 43 bytes */
	__u8 reserved[DENTRY_RESERVED_LENGTH];
} __packed;

/* 4K/51 Bytes = 80 dentries for per dentrygroup */
struct hmdfs_dentry_group {
	__u8 dentry_version; /* dentry version start from 1 */
	__u8 bitmap[DENTRY_BITMAP_LENGTH];
	struct hmdfs_dentry nsl[DENTRY_PER_GROUP];
	__u8 filename[DENTRY_PER_GROUP][DENTRY_NAME_LEN];
	__u8 reserved[DENTRY_GROUP_RESERVED];
} __packed;

/**
 * The content of 1st 4k block in dentryfile.dat.
 * Used for check whether the dcache can be used directly or
 * need to rebuild.
 *
 * Since the ctime has 10ms or less precision, if the dcache
 * rebuild at the same time of the dentry inode ctime, maybe
 * non-consistent in dcache.
 *	eg: create 1.jpg 2.jpg 3.jpg
 *	    dcache rebuild may only has 1.jpg 2.jpg
 * So, we need use these time to verify the dcache.
 */
struct hmdfs_dcache_header {
	/* The time of dcache rebuild */
	__le64 dcache_crtime;
	__le64 dcache_crtime_nsec;

	/* The directory inode ctime when dcache rebuild */
	__le64 dentry_ctime;
	__le64 dentry_ctime_nsec;

	/* The dentry count */
	__le64 num;

	/* The case sensitive */
	__u8 case_sensitive;
} __packed;

static inline loff_t get_dentry_group_pos(unsigned int bidx)
{
	return ((loff_t)bidx) * DENTRYGROUP_SIZE + DENTRYGROUP_HEADER;
}

static inline unsigned int get_dentry_group_cnt(struct inode *inode)
{
	loff_t size = i_size_read(inode);

	return size >= DENTRYGROUP_HEADER ?
		       (size - DENTRYGROUP_HEADER) / DENTRYGROUP_SIZE :
		       0;
}

#define DENTRY_NAME_MAX_LEN (DENTRY_PER_GROUP * DENTRY_NAME_LEN)
#define BITS_PER_BYTE	    8
#define HMDFS_SLOT_LEN_BITS 3
#define get_dentry_slots(x) (((x) + BITS_PER_BYTE - 1) >> HMDFS_SLOT_LEN_BITS)

#define INUNUMBER_START 10000000

#ifdef CONFIG_HMDFS_ANDROID
#define DENTRY_FILE_PERM 0660
#else
#define DENTRY_FILE_PERM 0666
#endif

struct hmdfs_dcache_lookup_ctx {
	struct hmdfs_sb_info *sbi;
	const struct qstr *name;
	struct file *filp;
	__u32 hash;

	/* for case sensitive */
	unsigned int bidx;
	struct hmdfs_dentry_group *page;

	/* for case insensitive */
	struct hmdfs_dentry *insense_de;
	unsigned int insense_bidx;
	struct hmdfs_dentry_group *insense_page;
};

extern void hmdfs_init_dcache_lookup_ctx(struct hmdfs_dcache_lookup_ctx *ctx,
					 struct hmdfs_sb_info *sbi,
					 const struct qstr *qstr,
					 struct file *filp);

int create_dentry(struct dentry *child_dentry, struct inode *inode,
		  struct file *file, struct hmdfs_sb_info *sbi);
int read_dentry(struct hmdfs_sb_info *sbi, char *file_name,
		struct dir_context *ctx);
struct hmdfs_dentry *hmdfs_find_dentry(struct dentry *child_dentry,
				       struct hmdfs_dcache_lookup_ctx *ctx);
void hmdfs_delete_dentry(struct dentry *d, struct file *filp);
int hmdfs_rename_dentry(struct dentry *old_dentry, struct dentry *new_dentry,
			struct file *old_filp, struct file *new_filp);
int get_inonumber(void);
struct file *create_local_dentry_file_cache(struct hmdfs_sb_info *sbi);
int update_inode_to_dentry(struct dentry *child_dentry, struct inode *inode);
struct file *cache_file_persistent(struct hmdfs_peer *con, struct file *filp,
			   const char *relative_path, bool server);

#define HMDFS_TYPE_COMMON	0
#define HMDFS_TYPE_DOT		1
#define HMDFS_TYPE_DENTRY	2
#define HMDFS_TYPE_DENTRY_CACHE 3
int hmdfs_file_type(const char *name);

unsigned long hmdfs_set_pos(unsigned long dev_id, unsigned long group_id,
			    unsigned long offset);

struct getdents_callback_real {
	struct dir_context ctx;
	struct path *parent_path;
	loff_t num;
	struct file *file;
	struct hmdfs_sb_info *sbi;
	const char *dir;
};

struct file *hmdfs_server_rebuild_dents(struct hmdfs_sb_info *sbi,
					struct path *path, loff_t *num,
					const char *dir);

#define DCACHE_LIFETIME 30

struct clearcache_item {
	uint64_t dev_id;
	struct file *filp;
	unsigned long time;
	struct list_head list;
	struct kref ref;
	struct hmdfs_dentry_info *d_info;
};

void hmdfs_add_remote_cache_list(struct hmdfs_peer *con, const char *dir_path);

struct remotecache_item {
	struct hmdfs_peer *con;
	struct list_head list;
	__u8 drop_flag;
};

#define HMDFS_CFN_CID_SIZE 65
#define HMDFS_SERVER_CID   ""

struct cache_file_node {
	struct list_head list;
	struct hmdfs_sb_info *sbi;
	char *relative_path;
	u8 cid[HMDFS_CFN_CID_SIZE];
	refcount_t ref;
	bool server;
	struct file *filp;
};

struct cache_file_item {
	struct list_head list;
	const char *name;
};

struct cache_file_callback {
	struct dir_context ctx;
	const char *dirname;
	struct hmdfs_sb_info *sbi;
	bool server;
	struct list_head list;
};

int hmdfs_drop_remote_cache_dents(struct dentry *dentry);
void hmdfs_send_drop_push(struct hmdfs_peer *con, const char *path);
void hmdfs_mark_drop_flag(uint64_t device_id, struct dentry *dentry);
void hmdfs_clear_drop_flag(struct dentry *dentry);
void delete_in_cache_file(uint64_t dev_id, struct dentry *dentry);
void create_in_cache_file(uint64_t dev_id, struct dentry *dentry);
struct clearcache_item *hmdfs_find_cache_item(uint64_t dev_id,
					      struct dentry *dentry);
bool hmdfs_cache_revalidate(unsigned long conn_time, uint64_t dev_id,
			    struct dentry *dentry);
void hmdfs_remove_cache_filp(struct hmdfs_peer *con, struct dentry *dentry);
int hmdfs_add_cache_list(uint64_t dev_id, struct dentry *dentry,
			 struct file *filp);
int hmdfs_clear_cache_dents(struct dentry *dentry, bool remove_cache);

int hmdfs_root_unlink(uint64_t device_id, struct path *root_path,
		      const char *unlink_dir, const char *unlink_name);
struct dentry *hmdfs_root_mkdir(uint64_t device_id, const char *local_dst_path,
				const char *mkdir_dir, const char *mkdir_name,
				umode_t mode);
struct dentry *hmdfs_root_create(uint64_t device_id, const char *local_dst_path,
				 const char *create_dir,
				 const char *create_name,
				 umode_t mode, bool want_excl);
int hmdfs_root_rmdir(uint64_t device_id, struct path *root_path,
		     const char *rmdir_dir, const char *rmdir_name);
int hmdfs_root_rename(struct hmdfs_sb_info *sbi, uint64_t device_id,
		      const char *oldpath, const char *oldname,
		      const char *newpath, const char *newname,
		      unsigned int flags);

int hmdfs_get_path_in_sb(struct super_block *sb, const char *name,
			 unsigned int flags, struct path *path);

int hmdfs_wlock_file(struct file *filp, loff_t start, loff_t len);
int hmdfs_rlock_file(struct file *filp, loff_t start, loff_t len);
int hmdfs_unlock_file(struct file *filp, loff_t start, loff_t len);
long cache_file_truncate(struct hmdfs_sb_info *sbi, const struct path *path,
			 loff_t length);
ssize_t cache_file_read(struct hmdfs_sb_info *sbi, struct file *filp, void *buf,
			size_t count, loff_t *pos);
ssize_t cache_file_write(struct hmdfs_sb_info *sbi, struct file *filp,
			 const void *buf, size_t count, loff_t *pos);
int hmdfs_metainfo_read(struct hmdfs_sb_info *sbi, struct file *filp,
			void *buffer, int buffersize, int bidx);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)
typedef int hmdfs_vm_fault_t;
#else
typedef vm_fault_t hmdfs_vm_fault_t;
#endif

bool get_remote_dentry_file(struct dentry *dentry, struct hmdfs_peer *con);
void get_remote_dentry_file_sync(struct dentry *dentry, struct hmdfs_peer *con);

void release_cache_item(struct kref *ref);
void remove_cache_item(struct clearcache_item *item);

void hmdfs_cfn_load(struct hmdfs_sb_info *sbi);
void hmdfs_cfn_destroy(struct hmdfs_sb_info *sbi);
struct cache_file_node *find_cfn(struct hmdfs_sb_info *sbi, const char *cid,
				 const char *path, bool server);
void release_cfn(struct cache_file_node *cfn);
void destroy_cfn(struct hmdfs_sb_info *sbi);
void remove_cfn(struct cache_file_node *cfn);
int delete_dentry_file(struct file *filp);
struct file *hmdfs_server_cache_revalidate(struct hmdfs_sb_info *sbi,
					   const char *recvpath,
					   struct path *path);
int write_header(struct file *filp, struct hmdfs_dcache_header *header);

static inline struct list_head *get_list_head(struct hmdfs_sb_info *sbi,
					      bool server)
{
	return ((server) ? &(sbi)->server_cache : &(sbi)->client_cache);
}

/*
 * generate_u64_ino - generate a new 64 bit inode number
 *
 * @ino: origin 32 bit inode number
 * @generation: origin 32 bit inode generation
 *
 * We need both remote inode number and generation to ensure the uniqueness of
 * the local inode, thus we store inode->i_ino in lower 32 bits, and
 * inode->i_generation in higher 32 bits.
 */
static inline uint64_t generate_u64_ino(unsigned long ino,
					unsigned int generation)
{
	return (uint64_t)ino | ((uint64_t)generation << 32);
}

static inline bool cache_item_revalidate(unsigned long conn_time,
					 unsigned long item_time,
					 unsigned int timeout)
{
	return time_before_eq(jiffies, item_time + timeout * HZ) &&
	       time_before_eq(conn_time, item_time);
}

#endif
