/*
 * stash.c
 *
 * Copyright (c) 2020-2028 Huawei Technologies Co., Ltd.
 * Author: houtao1@huawei.com
 * Author: luomeng12@huawei.com
 * Author: chengzhihao1@huawei.com
 * Create: 2020-12-27
 *
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/pagemap.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/errseq.h>
#include <linux/crc32.h>

#include "stash.h"
#include "comm/node_cb.h"
#include "comm/protocol.h"
#include "comm/connection.h"
#include "file_remote.h"
#include "hmdfs_dentryfile.h"
#include "authority/authentication.h"

/* Head magic used to identify a stash file */
#define HMDFS_STASH_FILE_HEAD_MAGIC 0xF7AB06C3
/* Head and path in stash file are aligned with HMDFS_STASH_BLK_SIZE */
#define HMDFS_STASH_BLK_SIZE 4096
#define HMDFS_STASH_BLK_SHIFT 12
#define HMDFS_STASH_PAGE_TO_SECTOR_SHIFT 3
#define HMDFS_STASH_DIR_NAME "stash"
#define HMDFS_STASH_FMT_DIR_NAME "v1"
#define HMDFS_STASH_WORK_DIR_NAME \
	(HMDFS_STASH_DIR_NAME "/" HMDFS_STASH_FMT_DIR_NAME)

#define HMDFS_STASH_FILE_NAME_LEN 20

#define HMDFS_STASH_FLUSH_CNT 2

#define HMDFS_STASH_PATH_LEN (HMDFS_CID_SIZE + HMDFS_STASH_FILE_NAME_LEN + 1)

struct hmdfs_cache_file_head {
	__le32 magic;
	__le32 crc_offset;
	__le64 ino;
	__le64 size;
	__le64 blocks;
	__le64 last_write_pos;
	__le64 ctime;
	__le32 ctime_nsec;
	__le32 change_detect_cap;
	__le64 ichange_count;
	__le32 path_offs;
	__le32 path_len;
	__le32 path_cnt;
	__le32 data_offs;
	/* Attention: expand new fields in here to compatible with old ver */
	__le32 crc32;
} __packed;

struct hmdfs_stash_work {
	struct hmdfs_peer *conn;
	struct list_head *list;
	struct work_struct work;
	struct completion done;
};

struct hmdfs_inode_tbl {
	unsigned int cnt;
	unsigned int max;
	uint64_t inodes[0];
};

struct hmdfs_stash_dir_context {
	struct dir_context dctx;
	char name[NAME_MAX + 1];
	struct hmdfs_inode_tbl *tbl;
};

struct hmdfs_restore_stats {
	unsigned int succeed;
	unsigned int fail;
	unsigned int keep;
	unsigned long long ok_pages;
	unsigned long long fail_pages;
};

struct hmdfs_stash_stats {
	unsigned int succeed;
	unsigned int donothing;
	unsigned int fail;
	unsigned long long ok_pages;
	unsigned long long fail_pages;
};

struct hmdfs_file_restore_ctx {
	struct hmdfs_peer *conn;
	struct path src_dir_path;
	struct path dst_root_path;
	char *dst;
	char *page;
	struct file *src_filp;
	uint64_t inum;
	uint64_t pages;
	unsigned int seq;
	unsigned int data_offs;
	/* output */
	bool keep;
};

struct hmdfs_copy_args {
	struct file *src;
	struct file *dst;
	void *buf;
	size_t buf_len;
	unsigned int seq;
	unsigned int data_offs;
	uint64_t inum;
};

struct hmdfs_copy_ctx {
	struct hmdfs_copy_args args;
	loff_t src_pos;
	loff_t dst_pos;
	/* output */
	size_t copied;
	bool eof;
};

struct hmdfs_rebuild_stats {
	unsigned int succeed;
	unsigned int total;
	unsigned int fail;
	unsigned int invalid;
};

struct hmdfs_check_work {
	struct hmdfs_peer *conn;
	struct work_struct work;
	struct completion done;
};

typedef int (*stash_operation_func)(struct hmdfs_peer *,
				    unsigned int,
				    struct path *,
				    const struct hmdfs_inode_tbl *,
				    void *);

static struct dentry *hmdfs_do_vfs_mkdir(struct dentry *parent,
					 const char *name, int namelen,
					 umode_t mode)
{
	struct inode *dir = d_inode(parent);
	struct dentry *child = NULL;
	int err;

	inode_lock_nested(dir, I_MUTEX_PARENT);

	child = lookup_one_len(name, parent, namelen);
	if (IS_ERR(child))
		goto out;

	if (d_is_positive(child)) {
		if (d_can_lookup(child))
			goto out;

		dput(child);
		child = ERR_PTR(-EINVAL);
		goto out;
	}

	err = vfs_mkdir(dir, child, mode);
	if (err) {
		dput(child);
		child = ERR_PTR(err);
		goto out;
	}

out:
	inode_unlock(dir);
	return child;
}

struct dentry *hmdfs_stash_new_work_dir(struct dentry *parent)
{
	struct dentry *base = NULL;
	struct dentry *work = NULL;

	base = hmdfs_do_vfs_mkdir(parent, HMDFS_STASH_DIR_NAME,
				   strlen(HMDFS_STASH_DIR_NAME), 0700);
	if (IS_ERR(base))
		return base;

	work = hmdfs_do_vfs_mkdir(base, HMDFS_STASH_FMT_DIR_NAME,
				  strlen(HMDFS_STASH_FMT_DIR_NAME), 0700);
	dput(base);

	return work;
}

static struct file *hmdfs_new_stash_file(struct path *d_path, const char *cid)
{
	struct dentry *parent = NULL;
	struct dentry *child = NULL;
	struct file *filp = NULL;
	struct path stash;
	int err;

	parent = hmdfs_do_vfs_mkdir(d_path->dentry, cid, strlen(cid), 0700);
	if (IS_ERR(parent)) {
		err = PTR_ERR(parent);
		hmdfs_err("mkdir error %d", err);
		goto mkdir_err;
	}

	child = vfs_tmpfile(parent, S_IFREG | 0600, 0);
	if (IS_ERR(child)) {
		err = PTR_ERR(child);
		hmdfs_err("new stash file error %d", err);
		goto tmpfile_err;
	}

	stash.mnt = d_path->mnt;
	stash.dentry = child;
	filp = dentry_open(&stash, O_LARGEFILE | O_WRONLY, current_cred());
	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		hmdfs_err("open stash file error %d", err);
		goto open_err;
	}

	dput(child);
	dput(parent);

	return filp;

open_err:
	dput(child);
tmpfile_err:
	dput(parent);
mkdir_err:
	return ERR_PTR(err);
}

static inline bool hmdfs_is_dir(struct dentry *child)
{
	return d_is_positive(child) && d_can_lookup(child);
}

static inline bool hmdfs_is_reg(struct dentry *child)
{
	return d_is_positive(child) && d_is_reg(child);
}

static void hmdfs_set_stash_file_head(const struct hmdfs_cache_info *cache,
				      uint64_t ino,
				      struct hmdfs_cache_file_head *head)
{
	long long blocks;
	unsigned int crc_offset;

	memset(head, 0, sizeof(*head));
	head->magic = cpu_to_le32(HMDFS_STASH_FILE_HEAD_MAGIC);
	head->ino = cpu_to_le64(ino);
	head->size = cpu_to_le64(i_size_read(file_inode(cache->cache_file)));
	blocks = atomic64_read(&cache->written_pgs) <<
			       HMDFS_STASH_PAGE_TO_SECTOR_SHIFT;
	head->blocks = cpu_to_le64(blocks);
	head->path_offs = cpu_to_le32(cache->path_offs);
	head->path_len = cpu_to_le32(cache->path_len);
	head->path_cnt = cpu_to_le32(cache->path_cnt);
	head->data_offs = cpu_to_le32(cache->data_offs);
	crc_offset = offsetof(struct hmdfs_cache_file_head, crc32);
	head->crc_offset = cpu_to_le32(crc_offset);
	head->crc32 = cpu_to_le32(crc32(0, head, crc_offset));
}

static int hmdfs_flush_stash_file_metadata(struct hmdfs_inode_info *info)
{
	struct hmdfs_cache_info *cache = NULL;
	struct hmdfs_peer *conn = info->conn;
	struct hmdfs_cache_file_head cache_head;
	size_t written;
	loff_t pos;
	unsigned int head_size;

	/* No metadata if no cache file info */
	cache = info->cache;
	if (!cache)
		return -EINVAL;

	if (strlen(cache->path) == 0) {
		long long to_write_pgs = atomic64_read(&cache->to_write_pgs);

		/* Nothing to stash. No need to flush meta data. */
		if (to_write_pgs == 0)
			return 0;

		hmdfs_err("peer 0x%x:0x%llx inode 0x%llx lost %lld pages due to no path",
			  conn->owner, conn->device_id,
			  info->remote_ino, to_write_pgs);
		return -EINVAL;
	}

	hmdfs_set_stash_file_head(cache, info->remote_ino, &cache_head);

	/* Write head */
	pos = 0;
	head_size = sizeof(cache_head);
	written = kernel_write(cache->cache_file, &cache_head, head_size, &pos);
	if (written != head_size) {
		hmdfs_err("stash peer 0x%x:0x%llx ino 0x%llx write head len %u err %zd",
			   conn->owner, conn->device_id, info->remote_ino,
			   head_size, written);
		return -EIO;
	}
	/* Write path */
	pos = (loff_t)cache->path_offs << HMDFS_STASH_BLK_SHIFT;
	written = kernel_write(cache->cache_file, cache->path, cache->path_len,
			       &pos);
	if (written != cache->path_len) {
		hmdfs_err("stash peer 0x%x:0x%llx ino 0x%llx write path len %u err %zd",
			   conn->owner, conn->device_id, info->remote_ino,
			   cache->path_len, written);
		return -EIO;
	}

	return 0;
}

/* Mainly from inode_wait_for_writeback() */
static void hmdfs_wait_remote_writeback_once(struct hmdfs_peer *conn,
					     struct hmdfs_inode_info *info)
{
	struct inode *inode = &info->vfs_inode;
	DEFINE_WAIT_BIT(wq, &inode->i_state, __I_SYNC);
	wait_queue_head_t *wq_head = NULL;
	bool in_sync = false;

	spin_lock(&inode->i_lock);
	in_sync = inode->i_state & I_SYNC;
	spin_unlock(&inode->i_lock);

	if (!in_sync)
		return;

	hmdfs_info("peer 0x%x:0x%llx ino 0x%llx wait for wb once",
		   conn->owner, conn->device_id, info->remote_ino);

	wq_head = bit_waitqueue(&inode->i_state, __I_SYNC);
	__wait_on_bit(wq_head, &wq, bit_wait, TASK_UNINTERRUPTIBLE);
}

static void hmdfs_reset_remote_write_err(struct hmdfs_peer *conn,
					 struct hmdfs_inode_info *info)
{
	struct address_space *mapping = info->vfs_inode.i_mapping;
	int flags_err;
	errseq_t old;
	int wb_err;

	flags_err = filemap_check_errors(mapping);

	old = errseq_sample(&mapping->wb_err);
	wb_err = errseq_check_and_advance(&mapping->wb_err, &old);
	if (flags_err || wb_err)
		hmdfs_warning("peer 0x%x:0x%llx inode 0x%llx wb error %d %d before stash",
			      conn->owner, conn->device_id, info->remote_ino,
			      flags_err, wb_err);
}

static bool hmdfs_is_mapping_clean(struct address_space *mapping)
{
	bool clean = false;

	/* b93b016313b3b ("page cache: use xa_lock") introduces i_pages */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	xa_lock_irq(&mapping->i_pages);
#else
	spin_lock_irq(&mapping->tree_lock);
#endif
	clean = !mapping_tagged(mapping, PAGECACHE_TAG_DIRTY) &&
		!mapping_tagged(mapping, PAGECACHE_TAG_WRITEBACK);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	xa_unlock_irq(&mapping->i_pages);
#else
	spin_unlock_irq(&mapping->tree_lock);
#endif
	return clean;
}

static int hmdfs_flush_stash_file_data(struct hmdfs_peer *conn,
				       struct hmdfs_inode_info *info)
{
	struct inode *inode = &info->vfs_inode;
	struct address_space *mapping = inode->i_mapping;
	bool all_clean = true;
	int err = 0;
	int i;

	/* Wait for the completion of write syscall */
	inode_lock(inode);
	inode_unlock(inode);

	all_clean = hmdfs_is_mapping_clean(mapping);
	if (all_clean) {
		hmdfs_reset_remote_write_err(conn, info);
		return 0;
	}

	/*
	 * No-sync_all writeback during offline may have not seen
	 * the setting of stash_status as HMDFS_REMOTE_INODE_STASHING
	 * and will call mapping_set_error() after we just reset
	 * the previous error. So waiting for these writeback once,
	 * and the following writeback will do local write.
	 */
	hmdfs_wait_remote_writeback_once(conn, info);

	/* Need to clear previous error ? */
	hmdfs_reset_remote_write_err(conn, info);

	/*
	 * 1. dirty page: do write back
	 * 2. writeback page: wait for its completion
	 * 3. writeback -> redirty page: do filemap_write_and_wait()
	 *    twice, so 2th writeback should not allow
	 *    writeback -> redirty transition
	 */
	for (i = 0; i < HMDFS_STASH_FLUSH_CNT; i++) {
		err = filemap_write_and_wait(mapping);
		if (err) {
			hmdfs_err("peer 0x%x:0x%llx inode 0x%llx #%d stash flush error %d",
				  conn->owner, conn->device_id,
				  info->remote_ino, i, err);
			return err;
		}
	}

	if (!hmdfs_is_mapping_clean(mapping))
		hmdfs_err("peer 0x%x:0x%llx inode 0x%llx is still dirty dt %d wb %d",
			  conn->owner, conn->device_id, info->remote_ino,
			  !!mapping_tagged(mapping, PAGECACHE_TAG_DIRTY),
			  !!mapping_tagged(mapping, PAGECACHE_TAG_WRITEBACK));

	return 0;
}

static int hmdfs_flush_stash_file(struct hmdfs_inode_info *info)
{
	int err;

	err = hmdfs_flush_stash_file_data(info->conn, info);
	if (!err)
		err = hmdfs_flush_stash_file_metadata(info);

	return err;
}

static int hmdfs_enable_stash_file(struct hmdfs_inode_info *info,
				   struct dentry *stash)
{
	char name[HMDFS_STASH_FILE_NAME_LEN];
	struct dentry *parent = NULL;
	struct inode *dir = NULL;
	struct dentry *child = NULL;
	int err = 0;
	bool retried = false;

	snprintf(name, sizeof(name), "0x%llx", info->remote_ino);

	parent = lock_parent(stash);
	dir = d_inode(parent);

lookup_again:
	child = lookup_one_len(name, parent, strlen(name));
	if (IS_ERR(child)) {
		err = PTR_ERR(child);
		child = NULL;
		hmdfs_err("lookup %s err %d", name, err);
		goto out;
	}

	if (d_is_positive(child)) {
		hmdfs_warning("%s exists (mode 0%o)",
			      name, d_inode(child)->i_mode);

		err = vfs_unlink(dir, child, NULL);
		if (err) {
			hmdfs_err("unlink %s err %d", name, err);
			goto out;
		}
		if (retried) {
			err = -EEXIST;
			goto out;
		}

		retried = true;
		dput(child);
		goto lookup_again;
	}

	err = vfs_link(stash, dir, child, NULL);
	if (err) {
		hmdfs_err("link stash file to %s err %d", name, err);
		goto out;
	}

out:
	unlock_dir(parent);
	if (child)
		dput(child);

	return err;
}

/* Return 1 if stash is done, 0 if nothing is stashed */
static int hmdfs_close_stash_file(struct hmdfs_peer *conn,
				  struct hmdfs_inode_info *info)
{
	struct file *cache_file = info->cache->cache_file;
	struct dentry *c_dentry = file_dentry(cache_file);
	struct inode *c_inode = d_inode(c_dentry);
	long long to_write_pgs = atomic64_read(&info->cache->to_write_pgs);
	int err;

	hmdfs_info("peer 0x%x:0x%llx inode 0x%llx stashed bytes %lld pages %lld",
		   conn->owner, conn->device_id, info->remote_ino,
		   i_size_read(c_inode), to_write_pgs);

	if (to_write_pgs == 0)
		return 0;

	err = vfs_fsync(cache_file, 0);
	if (!err)
		err = hmdfs_enable_stash_file(info, c_dentry);
	else
		hmdfs_err("fsync stash file err %d", err);

	return err < 0 ? err : 1;
}

static void hmdfs_del_file_cache(struct hmdfs_cache_info *cache)
{
	if (!cache)
		return;

	fput(cache->cache_file);
	kfree(cache->path_buf);
	kfree(cache);
}

static struct hmdfs_cache_info *
hmdfs_new_file_cache(struct hmdfs_peer *conn, struct hmdfs_inode_info *info)
{
	struct hmdfs_cache_info *cache = NULL;
	struct dentry *stash_dentry = NULL;
	int err;

	cache = kzalloc(sizeof(*cache), GFP_KERNEL);
	if (!cache)
		return ERR_PTR(-ENOMEM);

	atomic64_set(&cache->to_write_pgs, 0);
	atomic64_set(&cache->written_pgs, 0);
	cache->path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!cache->path_buf) {
		err = -ENOMEM;
		goto free_cache;
	}

	/* Need to handle "hardlink" ? */
	stash_dentry = d_find_any_alias(&info->vfs_inode);
	if (stash_dentry) {
		/* Needs full path in hmdfs, will be a device-view path */
		cache->path = dentry_path_raw(stash_dentry, cache->path_buf,
					      PATH_MAX);
		dput(stash_dentry);
		if (IS_ERR(cache->path)) {
			err = PTR_ERR(cache->path);
			hmdfs_err("peer 0x%x:0x%llx inode 0x%llx gen path err %d",
				  conn->owner, conn->device_id,
				  info->remote_ino, err);
			goto free_path;
		}
	} else {
		/* Write-opened file was closed before finding dentry */
		hmdfs_info("peer 0x%x:0x%llx inode 0x%llx no dentry found",
			   conn->owner, conn->device_id, info->remote_ino);
		cache->path_buf[0] = '\0';
		cache->path = cache->path_buf;
	}

	cache->path_cnt = 1;
	cache->path_len = strlen(cache->path) + 1;
	cache->path_offs = DIV_ROUND_UP(sizeof(struct hmdfs_cache_file_head),
					HMDFS_STASH_BLK_SIZE);
	cache->data_offs = cache->path_offs + DIV_ROUND_UP(cache->path_len,
					HMDFS_STASH_BLK_SIZE);
	cache->cache_file = hmdfs_new_stash_file(&conn->sbi->stash_work_dir,
						 conn->cid);
	if (IS_ERR(cache->cache_file)) {
		err = PTR_ERR(cache->cache_file);
		goto free_path;
	}

	return cache;

free_path:
	kfree(cache->path_buf);
free_cache:
	kfree(cache);
	return ERR_PTR(err);
}

static void hmdfs_init_stash_file_cache(struct hmdfs_peer *conn,
					struct hmdfs_inode_info *info)
{
	struct hmdfs_cache_info *cache = NULL;

	cache = hmdfs_new_file_cache(conn, info);
	if (IS_ERR(cache))
		/*
		 * Continue even creating stash info failed.
		 * We need to ensure there is no dirty pages
		 * after stash completes
		 */
		cache = NULL;

	/* Make write() returns */
	spin_lock(&info->stash_lock);
	info->cache = cache;
	info->stash_status = HMDFS_REMOTE_INODE_STASHING;
	spin_unlock(&info->stash_lock);
}

static void hmdfs_update_stash_stats(struct hmdfs_stash_stats *stats,
				     const struct hmdfs_cache_info *cache,
				     int err)
{
	unsigned long long ok_pages, fail_pages;

	if (cache) {
		ok_pages = err > 0 ? atomic64_read(&cache->written_pgs) : 0;
		fail_pages = atomic64_read(&cache->to_write_pgs) - ok_pages;
		stats->ok_pages += ok_pages;
		stats->fail_pages += fail_pages;
	}

	if (err > 0)
		stats->succeed++;
	else if (!err)
		stats->donothing++;
	else
		stats->fail++;
}

/* Return 1 if stash is done, 0 if nothing is stashed */
static int hmdfs_stash_remote_inode(struct hmdfs_inode_info *info,
				    struct hmdfs_stash_stats *stats)
{
	struct hmdfs_cache_info *cache = info->cache;
	struct hmdfs_peer *conn = info->conn;
	unsigned int status;
	int err = 0;

	hmdfs_info("stash peer 0x%x:0x%llx ino 0x%llx",
		   conn->owner, conn->device_id, info->remote_ino);

	err = hmdfs_flush_stash_file(info);
	if (!err)
		err = hmdfs_close_stash_file(conn, info);

	if (err <= 0)
		set_bit(HMDFS_FID_NEED_OPEN, &info->fid_flags);
	status = err > 0 ? HMDFS_REMOTE_INODE_RESTORING :
			   HMDFS_REMOTE_INODE_NONE;
	spin_lock(&info->stash_lock);
	info->cache = NULL;
	/*
	 * Use smp_store_release() to ensure order between HMDFS_FID_NEED_OPEN
	 * and HMDFS_REMOTE_INODE_NONE.
	 */
	smp_store_release(&info->stash_status, status);
	spin_unlock(&info->stash_lock);

	hmdfs_update_stash_stats(stats, cache, err);
	hmdfs_del_file_cache(cache);

	return err;
}

static void hmdfs_init_cache_for_stash_files(struct hmdfs_peer *conn,
					     struct list_head *list)
{
	const struct cred *old_cred = NULL;
	struct hmdfs_inode_info *info = NULL;

	/* For file creation under stash_work_dir */
	old_cred = hmdfs_override_creds(conn->sbi->cred);
	list_for_each_entry(info, list, stash_node)
		hmdfs_init_stash_file_cache(conn, info);
	hmdfs_revert_creds(old_cred);
}

static void hmdfs_init_stash_cache_work_fn(struct work_struct *base)
{
	struct hmdfs_stash_work *work =
		container_of(base, struct hmdfs_stash_work, work);

	hmdfs_init_cache_for_stash_files(work->conn, work->list);
	complete(&work->done);
}

static void hmdfs_init_cache_for_stash_files_by_work(struct hmdfs_peer *conn,
						     struct list_head *list)
{
	struct hmdfs_stash_work work = {
		.conn = conn,
		.list = list,
		.done = COMPLETION_INITIALIZER_ONSTACK(work.done),
	};

	INIT_WORK_ONSTACK(&work.work, hmdfs_init_stash_cache_work_fn);
	schedule_work(&work.work);
	wait_for_completion(&work.done);
}

static void hmdfs_stash_fetch_ready_files(struct hmdfs_peer *conn,
					  bool check, struct list_head *list)
{
	struct hmdfs_inode_info *info = NULL;

	spin_lock(&conn->wr_opened_inode_lock);
	list_for_each_entry(info, &conn->wr_opened_inode_list, wr_opened_node) {
		int status;

		/* Paired with *_release() in hmdfs_reset_stashed_inode() */
		status = smp_load_acquire(&info->stash_status);
		if (status == HMDFS_REMOTE_INODE_NONE) {
			list_add_tail(&info->stash_node, list);
			/*
			 * Prevent close() removing the inode from
			 * writeable-opened inode list
			 */
			hmdfs_remote_add_wr_opened_inode_nolock(conn, info);
			/* Prevent the inode from eviction */
			ihold(&info->vfs_inode);
		} else if (check && status == HMDFS_REMOTE_INODE_STASHING) {
			hmdfs_warning("peer 0x%x:0x%llx inode 0x%llx unexpected stash status %d",
				      conn->owner, conn->device_id,
				      info->remote_ino, status);
		}
	}
	spin_unlock(&conn->wr_opened_inode_lock);
}

static void hmdfs_stash_offline_prepare(struct hmdfs_peer *conn, int evt,
					unsigned int seq)
{
	LIST_HEAD(preparing);

	if (!hmdfs_is_stash_enabled(conn->sbi))
		return;

	mutex_lock(&conn->offline_cb_lock);

	hmdfs_stash_fetch_ready_files(conn, true, &preparing);

	if (list_empty(&preparing))
		goto out;

	hmdfs_init_cache_for_stash_files_by_work(conn, &preparing);
out:
	mutex_unlock(&conn->offline_cb_lock);
}

static void hmdfs_track_inode_locked(struct hmdfs_peer *conn,
				     struct hmdfs_inode_info *info)
{
	spin_lock(&conn->stashed_inode_lock);
	list_add_tail(&info->stash_node, &conn->stashed_inode_list);
	conn->stashed_inode_nr++;
	spin_unlock(&conn->stashed_inode_lock);
}

static void
hmdfs_update_peer_stash_stats(struct hmdfs_stash_statistics *stash_stats,
			      const struct hmdfs_stash_stats *stats)
{
	stash_stats->cur_ok = stats->succeed;
	stash_stats->cur_nothing = stats->donothing;
	stash_stats->cur_fail = stats->fail;
	stash_stats->total_ok += stats->succeed;
	stash_stats->total_nothing += stats->donothing;
	stash_stats->total_fail += stats->fail;
	stash_stats->ok_pages += stats->ok_pages;
	stash_stats->fail_pages += stats->fail_pages;
}

static void hmdfs_stash_remote_inodes(struct hmdfs_peer *conn,
				      struct list_head *list)
{
	const struct cred *old_cred = NULL;
	struct hmdfs_inode_info *info = NULL;
	struct hmdfs_inode_info *next = NULL;
	struct hmdfs_stash_stats stats;

	/* For file creation, write and relink under stash_work_dir */
	old_cred = hmdfs_override_creds(conn->sbi->cred);

	memset(&stats, 0, sizeof(stats));
	list_for_each_entry_safe(info, next, list, stash_node) {
		int err;

		list_del_init(&info->stash_node);

		err = hmdfs_stash_remote_inode(info, &stats);
		if (err > 0)
			hmdfs_track_inode_locked(conn, info);

		hmdfs_remote_del_wr_opened_inode(conn, info);
		if (err <= 0)
			iput(&info->vfs_inode);
	}
	hmdfs_revert_creds(old_cred);

	hmdfs_update_peer_stash_stats(&conn->stats.stash, &stats);
	hmdfs_info("peer 0x%x:0x%llx total stashed %u cur ok %u none %u fail %u",
		   conn->owner, conn->device_id, conn->stashed_inode_nr,
		   stats.succeed, stats.donothing, stats.fail);
}

static void hmdfs_stash_offline_do_stash(struct hmdfs_peer *conn, int evt,
					 unsigned int seq)
{
	struct hmdfs_inode_info *info = NULL;
	LIST_HEAD(preparing);
	LIST_HEAD(stashing);

	if (!hmdfs_is_stash_enabled(conn->sbi))
		return;

	/* release seq_lock to prevent blocking no-offline sync cb */
	mutex_unlock(&conn->seq_lock);
	/* acquire offline_cb_lock to serialized with offline sync cb */
	mutex_lock(&conn->offline_cb_lock);

	hmdfs_stash_fetch_ready_files(conn, false, &preparing);
	if (!list_empty(&preparing))
		hmdfs_init_cache_for_stash_files(conn, &preparing);

	spin_lock(&conn->wr_opened_inode_lock);
	list_for_each_entry(info, &conn->wr_opened_inode_list, wr_opened_node) {
		int status = READ_ONCE(info->stash_status);
		if (status == HMDFS_REMOTE_INODE_STASHING)
			list_add_tail(&info->stash_node, &stashing);
	}
	spin_unlock(&conn->wr_opened_inode_lock);

	if (list_empty(&stashing))
		goto unlock;

	hmdfs_stash_remote_inodes(conn, &stashing);

unlock:
	mutex_unlock(&conn->offline_cb_lock);
	mutex_lock(&conn->seq_lock);
}

static struct hmdfs_inode_info *
hmdfs_lookup_stash_inode(struct hmdfs_peer *conn, uint64_t inum)
{
	struct hmdfs_inode_info *info = NULL;

	list_for_each_entry(info, &conn->stashed_inode_list, stash_node) {
		if (info->remote_ino == inum)
			return info;
	}

	return NULL;
}

static void hmdfs_untrack_stashed_inode(struct hmdfs_peer *conn,
					struct hmdfs_inode_info *info)
{
	list_del_init(&info->stash_node);
	iput(&info->vfs_inode);

	conn->stashed_inode_nr--;
}

static void hmdfs_reset_stashed_inode(struct hmdfs_peer *conn,
				      struct hmdfs_inode_info *info)
{
	struct inode *ino = &info->vfs_inode;

	/*
	 * For updating stash_status after iput()
	 * in hmdfs_untrack_stashed_inode()
	 */
	ihold(ino);
	hmdfs_untrack_stashed_inode(conn, info);
	/*
	 * Ensure the order of stash_node and stash_status:
	 * only update stash_status to NONE after removal of
	 * stash_node is completed.
	 */
	smp_store_release(&info->stash_status,
			  HMDFS_REMOTE_INODE_NONE);
	iput(ino);
}

static void hmdfs_drop_stashed_inodes(struct hmdfs_peer *conn)
{
	struct hmdfs_inode_info *info = NULL;
	struct hmdfs_inode_info *next = NULL;

	if (list_empty(&conn->stashed_inode_list))
		return;

	hmdfs_warning("peer 0x%x:0x%llx drop unrestorable file %u",
		      conn->owner, conn->device_id, conn->stashed_inode_nr);

	list_for_each_entry_safe(info, next,
				 &conn->stashed_inode_list, stash_node) {
		hmdfs_warning("peer 0x%x:0x%llx inode 0x%llx unrestorable status %u",
			      conn->owner, conn->device_id, info->remote_ino,
			      READ_ONCE(info->stash_status));

		hmdfs_reset_stashed_inode(conn, info);
	}
}

static struct file *hmdfs_open_stash_dir(struct path *d_path, const char *cid)
{
	int err = 0;
	struct dentry *parent = d_path->dentry;
	struct inode *dir = d_inode(parent);
	struct dentry *child = NULL;
	struct path peer_path;
	struct file *filp = NULL;

	inode_lock_nested(dir, I_MUTEX_PARENT);
	child = lookup_one_len(cid, parent, strlen(cid));
	if (!IS_ERR(child)) {
		if (!hmdfs_is_dir(child)) {
			if (d_is_positive(child)) {
				hmdfs_err("invalid stash dir mode 0%o", d_inode(child)->i_mode);
				err = -EINVAL;
			} else {
				err = -ENOENT;
			}
			dput(child);
		}
	} else {
		err = PTR_ERR(child);
		hmdfs_err("lookup stash dir err %d", err);
	}
	inode_unlock(dir);

	if (err)
		return ERR_PTR(err);

	peer_path.mnt = d_path->mnt;
	peer_path.dentry = child;
	filp = dentry_open(&peer_path, O_RDONLY | O_DIRECTORY, current_cred());
	if (IS_ERR(filp))
		hmdfs_err("open err %d", (int)PTR_ERR(filp));

	dput(child);

	return filp;
}

static int hmdfs_new_inode_tbl(struct hmdfs_inode_tbl **tbl)
{
	struct hmdfs_inode_tbl *new = NULL;

	new = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	new->cnt = 0;
	new->max = (PAGE_SIZE - offsetof(struct hmdfs_inode_tbl, inodes)) /
		   sizeof(new->inodes[0]);
	*tbl = new;

	return 0;
}

static int hmdfs_parse_stash_file_name(struct dir_context *dctx,
					const char *name,
					int namelen,
					unsigned int d_type,
					uint64_t *stash_inum)
{
	struct hmdfs_stash_dir_context *ctx = NULL;
	int err;

	if (d_type != DT_UNKNOWN && d_type != DT_REG)
		return 0;
	if (namelen > NAME_MAX)
		return 0;

	ctx = container_of(dctx, struct hmdfs_stash_dir_context, dctx);
	memcpy(ctx->name, name, namelen);
	ctx->name[namelen] = '\0';
	err = kstrtoull(ctx->name, 16, stash_inum);
	if (err) {
		hmdfs_err("unexpected stash file err %d", err);
		return 0;
	}
	return 1;
}

static int hmdfs_has_stash_file(struct dir_context *dctx, const char *name,
				int namelen, loff_t offset,
				u64 inum, unsigned int d_type)
{
	struct hmdfs_stash_dir_context *ctx = NULL;
	uint64_t stash_inum;
	int err;

	ctx = container_of(dctx, struct hmdfs_stash_dir_context, dctx);
	err = hmdfs_parse_stash_file_name(dctx, name, namelen,
					   d_type, &stash_inum);
	if (!err)
		return 0;

	ctx->tbl->cnt++;
	return 1;
}

static int hmdfs_fill_stash_file(struct dir_context *dctx, const char *name,
				 int namelen, loff_t offset,
				 u64 inum, unsigned int d_type)
{
	struct hmdfs_stash_dir_context *ctx = NULL;
	uint64_t stash_inum;
	int err;

	ctx = container_of(dctx, struct hmdfs_stash_dir_context, dctx);
	err = hmdfs_parse_stash_file_name(dctx, name, namelen,
					   d_type, &stash_inum);
	if (!err)
		return 0;
	if (ctx->tbl->cnt >= ctx->tbl->max)
		return 1;

	ctx->tbl->inodes[ctx->tbl->cnt++] = stash_inum;

	return 0;
}

static int hmdfs_del_stash_file(struct dentry *parent, struct dentry *child)
{
	struct inode *dir = d_inode(parent);
	int err = 0;

	/* Prevent d_delete() from calling dentry_unlink_inode() */
	dget(child);

	inode_lock_nested(dir, I_MUTEX_PARENT);
	err = vfs_unlink(dir, child, NULL);
	if (err)
		hmdfs_err("remove stash file err %d", err);
	inode_unlock(dir);

	dput(child);

	return err;
}

static inline bool hmdfs_is_node_offlined(const struct hmdfs_peer *conn,
					  unsigned int seq)
{
	/*
	 * open()/fsync() may fail due to "status = NODE_STAT_OFFLINE"
	 * in hmdfs_disconnect_node().
	 * Pair with smp_mb() in hmdfs_disconnect_node() to ensure
	 * getting the newest event sequence.
	 */
	smp_mb__before_atomic();
	return hmdfs_node_evt_seq(conn) != seq;
}

static int hmdfs_verify_restore_file_head(struct hmdfs_file_restore_ctx *ctx,
				    const struct hmdfs_cache_file_head *head)
{
	struct inode *inode = file_inode(ctx->src_filp);
	struct hmdfs_peer *conn = ctx->conn;
	unsigned int crc, read_crc, crc_offset;
	loff_t path_offs, data_offs, isize;
	int err = 0;

	if (le32_to_cpu(head->magic) != HMDFS_STASH_FILE_HEAD_MAGIC) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid magic: got 0x%x, exp 0x%x",
			  conn->owner, conn->device_id, ctx->inum,
			  le32_to_cpu(head->magic),
			  HMDFS_STASH_FILE_HEAD_MAGIC);
		goto out;
	}

	crc_offset = le32_to_cpu(head->crc_offset);
	read_crc = le32_to_cpu(*((__le32 *)((char *)head + crc_offset)));
	crc = crc32(0, head, crc_offset);
	if (read_crc != crc) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid crc: got 0x%x, exp 0x%x",
			  conn->owner, conn->device_id, ctx->inum,
			  read_crc, crc);
		goto out;
	}

	if (le64_to_cpu(head->ino) != ctx->inum) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid ino: got %llu, exp %llu",
			  conn->owner, conn->device_id, ctx->inum,
			  le64_to_cpu(head->ino), ctx->inum);
		goto out;
	}

	path_offs = (loff_t)le32_to_cpu(head->path_offs) <<
		    HMDFS_STASH_BLK_SHIFT;
	if (path_offs <= 0 || path_offs >= i_size_read(inode)) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid path_offs %d, stash file size %llu",
			  conn->owner, conn->device_id, ctx->inum,
			  le32_to_cpu(head->path_offs), i_size_read(inode));
		goto out;
	}

	data_offs = (loff_t)le32_to_cpu(head->data_offs) <<
		    HMDFS_STASH_BLK_SHIFT;
	if (path_offs >= data_offs) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid data_offs %d, path_offs %d",
			  conn->owner, conn->device_id, ctx->inum,
			  le32_to_cpu(head->data_offs),
			  le32_to_cpu(head->path_offs));
		goto out;
	}
	if (data_offs <= 0 || data_offs >= i_size_read(inode)) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid data_offs %d, stash file size %llu",
			  conn->owner, conn->device_id, ctx->inum,
			  le32_to_cpu(head->data_offs), i_size_read(inode));
		goto out;
	}

	isize = le64_to_cpu(head->size);
	if (isize != i_size_read(inode)) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid isize: got %llu, exp %llu",
			  conn->owner, conn->device_id, ctx->inum,
			  le64_to_cpu(head->size), i_size_read(inode));
		goto out;
	}

	if (le32_to_cpu(head->path_cnt) < 1) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid path_cnt %d",
			  conn->owner, conn->device_id, ctx->inum,
			  le32_to_cpu(head->path_cnt));
		goto out;
	}

out:
	return err;
}

static int hmdfs_get_restore_file_metadata(struct hmdfs_file_restore_ctx *ctx)
{
	struct hmdfs_cache_file_head head;
	struct hmdfs_peer *conn = ctx->conn;
	unsigned int head_size, read_size, head_crc_offset;
	loff_t pos;
	ssize_t rd;
	int err = 0;

	head_size = sizeof(struct hmdfs_cache_file_head);
	memset(&head, 0, head_size);
	/* Read part head */
	pos = 0;
	read_size = offsetof(struct hmdfs_cache_file_head, crc_offset) +
		    sizeof(head.crc_offset);
	rd = kernel_read(ctx->src_filp, &head, read_size, &pos);
	if (rd != read_size) {
		err = rd < 0 ? rd : -ENODATA;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx read part head err %d",
			  conn->owner, conn->device_id, ctx->inum, err);
		goto out;
	}
	head_crc_offset = le32_to_cpu(head.crc_offset);
	if (head_crc_offset + sizeof(head.crc32) < head_crc_offset ||
	    head_crc_offset + sizeof(head.crc32) > head_size) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx got bad head: Too long crc_offset %u which exceeds head size %u",
			  conn->owner, conn->device_id, ctx->inum,
			  head_crc_offset, head_size);
		goto out;
	}

	/* Read full head */
	pos = 0;
	read_size = le32_to_cpu(head.crc_offset) + sizeof(head.crc32);
	rd = kernel_read(ctx->src_filp, &head, read_size, &pos);
	if (rd != read_size) {
		err = rd < 0 ? rd : -ENODATA;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx read full head err %d",
			  conn->owner, conn->device_id, ctx->inum, err);
		goto out;
	}

	err = hmdfs_verify_restore_file_head(ctx, &head);
	if (err)
		goto out;

	ctx->pages = le64_to_cpu(head.blocks) >>
		     HMDFS_STASH_PAGE_TO_SECTOR_SHIFT;
	ctx->data_offs = le32_to_cpu(head.data_offs);
	/* Read path */
	read_size = min_t(unsigned int, le32_to_cpu(head.path_len), PATH_MAX);
	pos = (loff_t)le32_to_cpu(head.path_offs) << HMDFS_STASH_BLK_SHIFT;
	rd = kernel_read(ctx->src_filp, ctx->dst, read_size, &pos);
	if (rd != read_size) {
		err = rd < 0 ? rd : -ENODATA;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx read path err %d",
			  conn->owner, conn->device_id, ctx->inum, err);
		goto out;
	}
	if (strnlen(ctx->dst, read_size) >= read_size) {
		err = -EUCLEAN;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx read path not end with \\0",
			  conn->owner, conn->device_id, ctx->inum);
		goto out;
	}
	/* TODO: Pick a valid path from all paths */

out:
	return err;
}

static int hmdfs_open_restore_dst_file(struct hmdfs_file_restore_ctx *ctx,
				       unsigned int rw_flag, struct file **filp)
{
	struct hmdfs_peer *conn = ctx->conn;
	struct file *dst = NULL;
	int err = 0;

	err = hmdfs_get_restore_file_metadata(ctx);
	if (err)
		goto out;

	/* Error comes from connection or server ? */
	dst = file_open_root(ctx->dst_root_path.dentry, ctx->dst_root_path.mnt,
			     ctx->dst, O_LARGEFILE | rw_flag, 0);
	if (IS_ERR(dst)) {
		err = PTR_ERR(dst);
		hmdfs_err("open remote file ino 0x%llx err %d", ctx->inum, err);
		if (hmdfs_is_node_offlined(conn, ctx->seq))
			err = -ESHUTDOWN;
		goto out;
	}

	*filp = dst;
out:
	return err;
}

static bool hmdfs_need_abort_restore(struct hmdfs_file_restore_ctx *ctx,
				     struct hmdfs_inode_info *pinned,
				     struct file *opened_file)
{
	struct hmdfs_inode_info *opened = hmdfs_i(file_inode(opened_file));

	if (opened->inode_type != HMDFS_LAYER_OTHER_REMOTE)
		goto abort;

	if (opened == pinned)
		return false;

abort:
	hmdfs_warning("peer 0x%x:0x%llx inode 0x%llx invalid remote file",
		      ctx->conn->owner, ctx->conn->device_id, ctx->inum);
	hmdfs_warning("got: peer 0x%x:0x%llx inode 0x%llx type %d status %d",
		      opened->conn ? opened->conn->owner : 0,
		      opened->conn ? opened->conn->device_id : 0,
		      opened->remote_ino, opened->inode_type,
		      opened->stash_status);
	hmdfs_warning("pinned: peer 0x%x:0x%llx inode 0x%llx type %d status %d",
		      pinned->conn->owner, pinned->conn->device_id,
		      pinned->remote_ino, pinned->inode_type,
		      pinned->stash_status);
	return true;
}

static void hmdfs_init_copy_args(const struct hmdfs_file_restore_ctx *ctx,
				 struct file *dst, struct hmdfs_copy_args *args)
{
	args->src = ctx->src_filp;
	args->dst = dst;
	args->buf = ctx->page;
	args->buf_len = PAGE_SIZE;
	args->seq = ctx->seq;
	args->data_offs = ctx->data_offs;
	args->inum = ctx->inum;
}

static ssize_t hmdfs_write_dst(struct hmdfs_peer *conn, struct file *filp,
			       void *buf, size_t len, loff_t pos)
{
	mm_segment_t old_fs;
	struct kiocb kiocb;
	struct iovec iov;
	struct iov_iter iter;
	ssize_t wr;
	int err = 0;

	file_start_write(filp);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	init_sync_kiocb(&kiocb, filp);
	kiocb.ki_pos = pos;

	iov.iov_base = buf;
	iov.iov_len = len;
	iov_iter_init(&iter, WRITE, &iov, 1, len);

	wr = hmdfs_file_write_iter_remote_nocheck(&kiocb, &iter);

	set_fs(old_fs);

	file_end_write(filp);

	if (wr != len) {
		struct hmdfs_inode_info *info = hmdfs_i(file_inode(filp));

		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx short write ret %zd exp %zu",
			  conn->owner, conn->device_id, info->remote_ino,
			  wr, len);
		err = wr < 0 ? (int)wr : -EFAULT;
	}

	return err;
}

static int hmdfs_rd_src_wr_dst(struct hmdfs_peer *conn,
			       struct hmdfs_copy_ctx *ctx)
{
	const struct hmdfs_copy_args *args = NULL;
	int err = 0;
	loff_t rd_pos;
	ssize_t rd;

	ctx->eof = false;
	ctx->copied = 0;

	args = &ctx->args;
	rd_pos = ctx->src_pos;
	rd = kernel_read(args->src, args->buf, args->buf_len, &rd_pos);
	if (rd < 0) {
		err = (int)rd;
		hmdfs_err("peer 0x%x:0x%llx ino 0x%llx short read err %d",
			  conn->owner, conn->device_id, args->inum, err);
		goto out;
	} else if (rd == 0) {
		ctx->eof = true;
		goto out;
	}

	err = hmdfs_write_dst(conn, args->dst, args->buf, rd, ctx->dst_pos);
	if (!err)
		ctx->copied = rd;
	else if (hmdfs_is_node_offlined(conn, args->seq))
		err = -ESHUTDOWN;
out:
	return err;
}

static int hmdfs_copy_src_to_dst(struct hmdfs_peer *conn,
				 const struct hmdfs_copy_args *args)
{
	int err = 0;
	struct file *src = NULL;
	struct hmdfs_copy_ctx ctx;
	loff_t seek_pos, data_init_pos;
	loff_t src_size;

	ctx.args = *args;

	src = ctx.args.src;
	data_init_pos = (loff_t)ctx.args.data_offs << HMDFS_STASH_BLK_SHIFT;
	seek_pos = data_init_pos;
	src_size = i_size_read(file_inode(src));
	while (true) {
		loff_t data_pos;

		data_pos = vfs_llseek(src, seek_pos, SEEK_DATA);
		if (data_pos > seek_pos) {
			seek_pos = data_pos;
			continue;
		} else if (data_pos < 0) {
			if (data_pos == -ENXIO) {
				loff_t src_blks = file_inode(src)->i_blocks;

				hmdfs_info("peer 0x%x:0x%llx ino 0x%llx end at 0x%llx (sz 0x%llx blk 0x%llx)",
					   conn->owner, conn->device_id,
					   args->inum, seek_pos,
					   src_size, src_blks);
			} else {
				err = (int)data_pos;
				hmdfs_err("peer 0x%x:0x%llx ino 0x%llx seek pos 0x%llx err %d",
					  conn->owner, conn->device_id,
					  args->inum, seek_pos, err);
			}
			break;
		}

		hmdfs_debug("peer 0x%x:0x%llx ino 0x%llx seek to 0x%llx",
			    conn->owner, conn->device_id, args->inum, data_pos);

		ctx.src_pos = data_pos;
		ctx.dst_pos = data_pos - data_init_pos;
		err = hmdfs_rd_src_wr_dst(conn, &ctx);
		if (err || ctx.eof)
			break;

		seek_pos += ctx.copied;
		if (seek_pos >= src_size)
			break;
	}

	return err;
}

static int hmdfs_restore_src_to_dst(struct hmdfs_file_restore_ctx *ctx,
				    struct file *dst)
{
	struct file *src = ctx->src_filp;
	struct hmdfs_copy_args args;
	int err;

	hmdfs_init_copy_args(ctx, dst, &args);
	err = hmdfs_copy_src_to_dst(ctx->conn, &args);
	if (err)
		goto out;

	err = vfs_fsync(dst, 0);
	if (err) {
		hmdfs_err("fsync remote file ino 0x%llx err %d", ctx->inum, err);
		if (hmdfs_is_node_offlined(ctx->conn, ctx->seq))
			err = -ESHUTDOWN;
	}

out:
	if (err)
		truncate_inode_pages(file_inode(dst)->i_mapping, 0);

	/* Remove the unnecessary cache */
	invalidate_mapping_pages(file_inode(src)->i_mapping, 0, -1);

	return err;
}


static int hmdfs_restore_file(struct hmdfs_file_restore_ctx *ctx)
{
	struct hmdfs_peer *conn = ctx->conn;
	uint64_t inum = ctx->inum;
	struct hmdfs_inode_info *pinned_info = NULL;
	struct file *dst_filp = NULL;
	int err = 0;
	bool keep = false;

	hmdfs_info("peer 0x%x:0x%llx ino 0x%llx do restore",
		   conn->owner, conn->device_id, inum);

	pinned_info = hmdfs_lookup_stash_inode(conn, inum);
	if (pinned_info) {
		unsigned int status = READ_ONCE(pinned_info->stash_status);
		if (status != HMDFS_REMOTE_INODE_RESTORING) {
			hmdfs_err("peer 0x%x:0x%llx ino 0x%llx invalid status %u",
				  conn->owner, conn->device_id, inum, status);
			err = -EINVAL;
			goto clean;
		}
	} else {
		hmdfs_warning("peer 0x%x:0x%llx ino 0x%llx doesn't being pinned",
			      conn->owner, conn->device_id, inum);
		err = -EINVAL;
		goto clean;
	}

	set_bit(HMDFS_FID_NEED_OPEN, &pinned_info->fid_flags);
	err = hmdfs_open_restore_dst_file(ctx, O_RDWR, &dst_filp);
	if (err) {
		if (err == -ESHUTDOWN)
			keep = true;
		goto clean;
	}

	if (hmdfs_need_abort_restore(ctx, pinned_info, dst_filp))
		goto abort;

	err = hmdfs_restore_src_to_dst(ctx, dst_filp);
	if (err == -ESHUTDOWN)
		keep = true;
abort:
	fput(dst_filp);
clean:
	if (pinned_info && !keep)
		hmdfs_reset_stashed_inode(conn, pinned_info);
	ctx->keep = keep;

	hmdfs_info("peer 0x%x:0x%llx ino 0x%llx restore err %d keep %d",
		   conn->owner, conn->device_id, inum, err, ctx->keep);

	return err;
}

static int hmdfs_init_file_restore_ctx(struct hmdfs_peer *conn,
				       unsigned int seq, struct path *src_dir,
				       struct hmdfs_file_restore_ctx *ctx)
{
	struct hmdfs_sb_info *sbi = conn->sbi;
	struct path dst_root;
	char *dst = NULL;
	char *page = NULL;
	int err = 0;

	err = hmdfs_get_path_in_sb(sbi->sb, sbi->real_dst, LOOKUP_DIRECTORY,
				   &dst_root);
	if (err)
		return err;

	dst = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!dst) {
		err = -ENOMEM;
		goto put_path;
	}

	page = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!page) {
		err = -ENOMEM;
		goto free_dst;
	}

	ctx->conn = conn;
	ctx->src_dir_path = *src_dir;
	ctx->dst_root_path = dst_root;
	ctx->dst = dst;
	ctx->page = page;
	ctx->seq = seq;

	return 0;
free_dst:
	kfree(dst);
put_path:
	path_put(&dst_root);
	return err;
}

static void hmdfs_exit_file_restore_ctx(struct hmdfs_file_restore_ctx *ctx)
{
	path_put(&ctx->dst_root_path);
	kfree(ctx->dst);
	kfree(ctx->page);
}

static struct file *hmdfs_open_stash_file(struct path *p_path, char *name)
{
	struct dentry *parent = NULL;
	struct inode *dir = NULL;
	struct dentry *child = NULL;
	struct file *filp = NULL;
	struct path c_path;
	int err = 0;

	parent = p_path->dentry;
	dir = d_inode(parent);
	inode_lock_nested(dir, I_MUTEX_PARENT);
	child = lookup_one_len(name, parent, strlen(name));
	if (!IS_ERR(child) && !hmdfs_is_reg(child)) {
		if (d_is_positive(child)) {
			hmdfs_err("invalid stash file (mode 0%o)",
				  d_inode(child)->i_mode);
			err = -EINVAL;
		} else {
			hmdfs_err("missing stash file");
			err = -ENOENT;
		}
		dput(child);
	} else if (IS_ERR(child)) {
		err = PTR_ERR(child);
		hmdfs_err("lookup stash file err %d", err);
	}
	inode_unlock(dir);

	if (err)
		return ERR_PTR(err);

	c_path.mnt = p_path->mnt;
	c_path.dentry = child;
	filp = dentry_open(&c_path, O_RDONLY | O_LARGEFILE, current_cred());
	if (IS_ERR(filp))
		hmdfs_err("open stash file err %d", (int)PTR_ERR(filp));

	dput(child);

	return filp;
}

static void hmdfs_update_restore_stats(struct hmdfs_restore_stats *stats,
				       bool keep, uint64_t pages, int err)
{
	if (!err) {
		stats->succeed++;
		stats->ok_pages += pages;
	} else if (keep) {
		stats->keep++;
	} else {
		stats->fail++;
		stats->fail_pages += pages;
	}
}

static int hmdfs_restore_files(struct hmdfs_peer *conn,
			       unsigned int seq, struct path *dir,
			       const struct hmdfs_inode_tbl *tbl,
			       void *priv)
{
	unsigned int i;
	struct hmdfs_file_restore_ctx ctx;
	int err = 0;
	struct hmdfs_restore_stats *stats = priv;

	err = hmdfs_init_file_restore_ctx(conn, seq, dir, &ctx);
	if (err)
		return err;

	for (i = 0; i < tbl->cnt; i++) {
		char name[HMDFS_STASH_FILE_NAME_LEN];
		struct file *filp = NULL;

		snprintf(name, sizeof(name), "0x%llx", tbl->inodes[i]);
		filp = hmdfs_open_stash_file(dir, name);
		/* Continue to restore if any error */
		if (IS_ERR(filp)) {
			stats->fail++;
			continue;
		}

		ctx.inum = tbl->inodes[i];
		ctx.src_filp = filp;
		ctx.keep = false;
		ctx.pages = 0;
		err = hmdfs_restore_file(&ctx);
		hmdfs_update_restore_stats(stats, ctx.keep, ctx.pages, err);

		if (!ctx.keep)
			hmdfs_del_stash_file(dir->dentry,
					     file_dentry(ctx.src_filp));
		fput(ctx.src_filp);

		/* Continue to restore */
		if (err == -ESHUTDOWN)
			break;
		err = 0;
	}

	hmdfs_exit_file_restore_ctx(&ctx);

	return err;
}

static bool hmdfs_is_valid_stash_status(struct hmdfs_inode_info *inode_info,
					uint64_t ino)
{
	return (inode_info->inode_type == HMDFS_LAYER_OTHER_REMOTE &&
		inode_info->stash_status == HMDFS_REMOTE_INODE_RESTORING &&
		inode_info->remote_ino == ino);
}

static int hmdfs_rebuild_stash_list(struct hmdfs_peer *conn,
				    unsigned int seq,
				    struct path *dir,
				    const struct hmdfs_inode_tbl *tbl,
				    void *priv)
{
	struct hmdfs_file_restore_ctx ctx;
	unsigned int i;
	int err;
	struct hmdfs_rebuild_stats *stats = priv;

	err = hmdfs_init_file_restore_ctx(conn, seq, dir, &ctx);
	if (err)
		return err;

	stats->total += tbl->cnt;

	for (i = 0; i < tbl->cnt; i++) {
		char name[HMDFS_STASH_FILE_NAME_LEN];
		struct file *src_filp = NULL;
		struct file *dst_filp = NULL;
		struct hmdfs_inode_info *inode_info = NULL;
		bool is_valid = true;

		snprintf(name, sizeof(name), "0x%llx", tbl->inodes[i]);
		src_filp = hmdfs_open_stash_file(dir, name);
		if (IS_ERR(src_filp)) {
			stats->fail++;
			continue;
		}
		ctx.inum = tbl->inodes[i];
		ctx.src_filp = src_filp;

		/* No need to track the open which only needs meta info */
		err = hmdfs_open_restore_dst_file(&ctx, O_RDONLY, &dst_filp);
		if (err) {
			fput(src_filp);
			if (err == -ESHUTDOWN)
				break;
			stats->fail++;
			err = 0;
			continue;
		}

		inode_info = hmdfs_i(file_inode(dst_filp));
		is_valid = hmdfs_is_valid_stash_status(inode_info,
						       ctx.inum);
		if (is_valid) {
			stats->succeed++;
		} else {
			hmdfs_err("peer 0x%x:0x%llx inode 0x%llx invalid state: type: %d, status: %u, inode: %llu",
				  conn->owner, conn->device_id, ctx.inum,
				  inode_info->inode_type,
				  READ_ONCE(inode_info->stash_status),
				  inode_info->remote_ino);
			stats->invalid++;
		}

		fput(ctx.src_filp);
		fput(dst_filp);
	}

	hmdfs_exit_file_restore_ctx(&ctx);
	return err;
}

static int hmdfs_iter_stash_file(struct hmdfs_peer *conn,
				 unsigned int seq,
				 struct file *filp,
				 stash_operation_func op,
				 void *priv)
{
	int err = 0;
	struct hmdfs_stash_dir_context ctx = {
		.dctx.actor = hmdfs_fill_stash_file,
	};
	struct hmdfs_inode_tbl *tbl = NULL;
	struct path dir;

	err = hmdfs_new_inode_tbl(&tbl);
	if (err)
		goto out;

	dir.mnt = filp->f_path.mnt;
	dir.dentry = file_dentry(filp);

	ctx.tbl = tbl;
	ctx.dctx.pos = 0;
	do {
		tbl->cnt = 0;
		err = iterate_dir(filp, &ctx.dctx);
		if (err || !tbl->cnt) {
			if (err)
				hmdfs_err("iterate stash dir err %d", err);
			break;
		}
		err = op(conn, seq, &dir, tbl, priv);
	} while (!err);

out:
	kfree(tbl);
	return err;
}

static void hmdfs_rebuild_check_work_fn(struct work_struct *base)
{
	struct hmdfs_check_work *work =
		container_of(base, struct hmdfs_check_work, work);
	struct hmdfs_peer *conn = work->conn;
	struct hmdfs_sb_info *sbi = conn->sbi;
	struct file *filp = NULL;
	const struct cred *old_cred = NULL;
	struct hmdfs_stash_dir_context ctx = {
		.dctx.actor = hmdfs_has_stash_file,
	};
	struct hmdfs_inode_tbl tbl;
	int err;

	old_cred = hmdfs_override_creds(sbi->cred);
	filp = hmdfs_open_stash_dir(&sbi->stash_work_dir, conn->cid);
	if (IS_ERR(filp))
		goto out;

	memset(&tbl, 0, sizeof(tbl));
	ctx.tbl = &tbl;
	err = iterate_dir(filp, &ctx.dctx);
	if (!err && ctx.tbl->cnt > 0)
		conn->need_rebuild_stash_list = true;

	fput(filp);
out:
	hmdfs_revert_creds(old_cred);
	hmdfs_info("peer 0x%x:0x%llx %sneed to rebuild stash list",
		   conn->owner, conn->device_id,
		   conn->need_rebuild_stash_list ? "" : "don't ");
	complete(&work->done);
}

static void hmdfs_stash_add_do_check(struct hmdfs_peer *conn, int evt,
				     unsigned int seq)
{
	struct hmdfs_sb_info *sbi = conn->sbi;
	struct hmdfs_check_work work = {
		.conn = conn,
		.done = COMPLETION_INITIALIZER_ONSTACK(work.done),
	};

	if (!hmdfs_is_stash_enabled(sbi))
		return;

	INIT_WORK_ONSTACK(&work.work, hmdfs_rebuild_check_work_fn);
	schedule_work(&work.work);
	wait_for_completion(&work.done);
}

static void
hmdfs_update_peer_rebuild_stats(struct hmdfs_rebuild_statistics *rebuild_stats,
				const struct hmdfs_rebuild_stats *stats)
{
	rebuild_stats->cur_ok = stats->succeed;
	rebuild_stats->cur_fail = stats->fail;
	rebuild_stats->cur_invalid = stats->invalid;
	rebuild_stats->total_ok += stats->succeed;
	rebuild_stats->total_fail += stats->fail;
	rebuild_stats->total_invalid += stats->invalid;
}

/* rebuild stash inode list */
static void hmdfs_stash_online_prepare(struct hmdfs_peer *conn, int evt,
				       unsigned int seq)
{
	struct hmdfs_sb_info *sbi = conn->sbi;
	struct file *filp = NULL;
	const struct cred *old_cred = NULL;
	int err;
	struct hmdfs_rebuild_stats stats;

	if (!hmdfs_is_stash_enabled(sbi) ||
	    !conn->need_rebuild_stash_list)
		return;

	/* release seq_lock to prevent blocking no-online sync cb */
	mutex_unlock(&conn->seq_lock);
	old_cred = hmdfs_override_creds(sbi->cred);
	filp = hmdfs_open_stash_dir(&sbi->stash_work_dir, conn->cid);
	if (IS_ERR(filp))
		goto out;

	memset(&stats, 0, sizeof(stats));
	err = hmdfs_iter_stash_file(conn, seq, filp,
				    hmdfs_rebuild_stash_list, &stats);
	if (err == -ESHUTDOWN) {
		hmdfs_info("peer 0x%x:0x%llx offline again during rebuild",
			   conn->owner, conn->device_id);
	} else {
		WRITE_ONCE(conn->need_rebuild_stash_list, false);
		if (err)
			hmdfs_warning("partial rebuild fail err %d", err);
	}

	hmdfs_update_peer_rebuild_stats(&conn->stats.rebuild, &stats);
	hmdfs_info("peer 0x%x:0x%llx rebuild stashed-file total %u succeed %u fail %u invalid %u",
		   conn->owner, conn->device_id, stats.total, stats.succeed,
		   stats.fail, stats.invalid);
	fput(filp);
out:
	conn->stats.rebuild.time++;
	hmdfs_revert_creds(old_cred);
	if (!READ_ONCE(conn->need_rebuild_stash_list)) {
		/*
		 * Use smp_mb__before_atomic() to ensure order between
		 * writing @conn->need_rebuild_stash_list and
		 * reading conn->rebuild_inode_status_nr.
		 */
		smp_mb__before_atomic();
		/*
		 * Wait until all inodes finish rebuilding stash status before
		 * accessing @conn->stashed_inode_list in restoring.
		 */
		wait_event(conn->rebuild_inode_status_wq,
			   !atomic_read(&conn->rebuild_inode_status_nr));
	}
	mutex_lock(&conn->seq_lock);
}

static void
hmdfs_update_peer_restore_stats(struct hmdfs_restore_statistics *restore_stats,
				const struct hmdfs_restore_stats *stats)
{
	restore_stats->cur_ok = stats->succeed;
	restore_stats->cur_fail = stats->fail;
	restore_stats->cur_keep = stats->keep;
	restore_stats->total_ok += stats->succeed;
	restore_stats->total_fail += stats->fail;
	restore_stats->total_keep += stats->keep;
	restore_stats->ok_pages += stats->ok_pages;
	restore_stats->fail_pages += stats->fail_pages;
}

static void hmdfs_stash_online_do_restore(struct hmdfs_peer *conn, int evt,
					  unsigned int seq)
{
	struct hmdfs_sb_info *sbi = conn->sbi;
	struct file *filp = NULL;
	const struct cred *old_cred = NULL;
	struct hmdfs_restore_stats stats;
	int err = 0;

	if (!hmdfs_is_stash_enabled(sbi) || conn->need_rebuild_stash_list) {
		if (conn->need_rebuild_stash_list)
			hmdfs_info("peer 0x%x:0x%llx skip restoring due to rebuild-need",
				   conn->owner, conn->device_id);
		return;
	}

	/* release seq_lock to prevent blocking no-online sync cb */
	mutex_unlock(&conn->seq_lock);
	/* For dir iteration, file read and unlink */
	old_cred = hmdfs_override_creds(conn->sbi->cred);

	memset(&stats, 0, sizeof(stats));
	filp = hmdfs_open_stash_dir(&sbi->stash_work_dir, conn->cid);
	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		goto out;
	}

	err = hmdfs_iter_stash_file(conn, seq, filp,
				    hmdfs_restore_files, &stats);

	fput(filp);
out:
	hmdfs_revert_creds(old_cred);

	/* offline again ? */
	if (err != -ESHUTDOWN)
		hmdfs_drop_stashed_inodes(conn);

	hmdfs_update_peer_restore_stats(&conn->stats.restore, &stats);
	hmdfs_info("peer 0x%x:0x%llx restore stashed-file ok %u fail %u keep %u",
		   conn->owner, conn->device_id,
		   stats.succeed, stats.fail, stats.keep);

	mutex_lock(&conn->seq_lock);
}

static void hmdfs_stash_del_do_cleanup(struct hmdfs_peer *conn, int evt,
				       unsigned int seq)
{
	struct hmdfs_inode_info *info = NULL;
	struct hmdfs_inode_info *next = NULL;
	unsigned int preparing;

	if (!hmdfs_is_stash_enabled(conn->sbi))
		return;

	/* Async cb is cancelled */
	preparing = 0;
	list_for_each_entry_safe(info, next, &conn->wr_opened_inode_list,
				 wr_opened_node) {
		int status = READ_ONCE(info->stash_status);

		if (status == HMDFS_REMOTE_INODE_STASHING) {
			struct hmdfs_cache_info *cache = NULL;

			spin_lock(&info->stash_lock);
			cache = info->cache;
			info->cache = NULL;
			info->stash_status = HMDFS_REMOTE_INODE_NONE;
			spin_unlock(&info->stash_lock);

			hmdfs_remote_del_wr_opened_inode(conn, info);
			hmdfs_del_file_cache(cache);
			/* put inode after all access are completed */
			iput(&info->vfs_inode);
			preparing++;
		}
	}
	hmdfs_info("release %u preparing inodes", preparing);

	hmdfs_info("release %u pinned inodes", conn->stashed_inode_nr);
	if (list_empty(&conn->stashed_inode_list))
		return;

	list_for_each_entry_safe(info, next,
				 &conn->stashed_inode_list, stash_node)
		hmdfs_untrack_stashed_inode(conn, info);
}

void hmdfs_exit_stash(struct hmdfs_sb_info *sbi)
{
	if (!sbi->s_offline_stash)
		return;

	if (sbi->stash_work_dir.dentry) {
		path_put(&sbi->stash_work_dir);
		sbi->stash_work_dir.dentry = NULL;
	}
}

int hmdfs_init_stash(struct hmdfs_sb_info *sbi)
{
	int err = 0;
	struct path parent;
	struct dentry *child = NULL;

	if (!sbi->s_offline_stash)
		return 0;

	err = kern_path(sbi->cache_dir, LOOKUP_FOLLOW | LOOKUP_DIRECTORY,
			&parent);
	if (err) {
		hmdfs_err("invalid cache dir err %d", err);
		goto out;
	}

	child = hmdfs_stash_new_work_dir(parent.dentry);
	if (!IS_ERR(child)) {
		sbi->stash_work_dir.mnt = mntget(parent.mnt);
		sbi->stash_work_dir.dentry = child;
	} else {
		err = PTR_ERR(child);
		hmdfs_err("create stash work dir err %d", err);
	}

	path_put(&parent);
out:
	return err;
}

static int hmdfs_stash_write_local_file(struct hmdfs_peer *conn,
					struct hmdfs_inode_info *info,
					struct hmdfs_writepage_context *ctx,
					struct hmdfs_cache_info *cache)
{
	struct page *page = ctx->page;
	const struct cred *old_cred = NULL;
	void *buf = NULL;
	loff_t pos;
	unsigned int flags;
	ssize_t written;
	int err = 0;

	buf = kmap(page);
	pos = (loff_t)page->index << PAGE_SHIFT;
	/* enable NOFS for memory allocation */
	flags = memalloc_nofs_save();
	old_cred = hmdfs_override_creds(conn->sbi->cred);
	pos += cache->data_offs << HMDFS_STASH_BLK_SHIFT;
	written = kernel_write(cache->cache_file, buf, ctx->count, &pos);
	hmdfs_revert_creds(old_cred);
	memalloc_nofs_restore(flags);
	kunmap(page);

	if (written != ctx->count) {
		hmdfs_err("stash peer 0x%x:0x%llx ino 0x%llx page 0x%lx data_offs 0x%x len %u err %zd",
			  conn->owner, conn->device_id, info->remote_ino,
			  page->index, cache->data_offs, ctx->count, written);
		err = -EIO;
	}

	return err;
}

int hmdfs_stash_writepage(struct hmdfs_peer *conn,
			  struct hmdfs_writepage_context *ctx)
{
	struct inode *inode = ctx->page->mapping->host;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_cache_info *cache = NULL;
	int err;

	/* e.g. fail to create stash file */
	cache = info->cache;
	if (!cache)
		return -EIO;

	err = hmdfs_stash_write_local_file(conn, info, ctx, cache);
	if (!err) {
		hmdfs_client_writepage_done(info, ctx);
		atomic64_inc(&cache->written_pgs);
		put_task_struct(ctx->caller);
		kfree(ctx);
	}
	atomic64_inc(&cache->to_write_pgs);

	return err;
}

static void hmdfs_stash_rebuild_status(struct hmdfs_peer *conn,
				       struct inode *inode)
{
	char *path_str = NULL;
	struct hmdfs_inode_info *info = NULL;
	const struct cred *old_cred = NULL;
	struct path path;
	struct path *stash_path = NULL;
	int err = 0;

	path_str = kmalloc(HMDFS_STASH_PATH_LEN, GFP_KERNEL);
	if (!path_str) {
		err = -ENOMEM;
		return;
	}

	info = hmdfs_i(inode);
	err = snprintf(path_str, HMDFS_STASH_PATH_LEN, "%s/0x%llx",
		       conn->cid, info->remote_ino);
	if (err >= HMDFS_STASH_PATH_LEN) {
		kfree(path_str);
		hmdfs_err("peer 0x%x:0x%llx inode 0x%llx too long name len",
			  conn->owner, conn->device_id, info->remote_ino);
		return;
	}
	old_cred = hmdfs_override_creds(conn->sbi->cred);
	stash_path = &conn->sbi->stash_work_dir;
	err = vfs_path_lookup(stash_path->dentry, stash_path->mnt,
			      path_str, 0, &path);
	hmdfs_revert_creds(old_cred);
	if (!err) {
		if (hmdfs_is_reg(path.dentry)) {
			WRITE_ONCE(info->stash_status,
				   HMDFS_REMOTE_INODE_RESTORING);
			ihold(&info->vfs_inode);
			hmdfs_track_inode_locked(conn, info);
		} else {
			hmdfs_info("peer 0x%x:0x%llx inode 0x%llx unexpected stashed file mode 0%o",
				    conn->owner, conn->device_id,
				    info->remote_ino,
				    d_inode(path.dentry)->i_mode);
		}

		path_put(&path);
	} else if (err && err != -ENOENT) {
		hmdfs_err("peer 0x%x:0x%llx inode 0x%llx find %s err %d",
			   conn->owner, conn->device_id, info->remote_ino,
			   path_str, err);
	}

	kfree(path_str);
}

static inline bool
hmdfs_need_rebuild_inode_stash_status(struct hmdfs_peer *conn, umode_t mode)
{
	return hmdfs_is_stash_enabled(conn->sbi) &&
	       READ_ONCE(conn->need_rebuild_stash_list) &&
	       (S_ISREG(mode) || S_ISLNK(mode));
}

void hmdfs_remote_init_stash_status(struct hmdfs_peer *conn,
				    struct inode *inode, umode_t mode)
{
	if (!hmdfs_need_rebuild_inode_stash_status(conn, mode))
		return;

	atomic_inc(&conn->rebuild_inode_status_nr);
	/*
	 * Use smp_mb__after_atomic() to ensure order between writing
	 * @conn->rebuild_inode_status_nr and reading
	 * @conn->need_rebuild_stash_list.
	 */
	smp_mb__after_atomic();
	if (READ_ONCE(conn->need_rebuild_stash_list))
		hmdfs_stash_rebuild_status(conn, inode);
	if (atomic_dec_and_test(&conn->rebuild_inode_status_nr))
		wake_up(&conn->rebuild_inode_status_wq);
}

static struct hmdfs_node_cb_desc stash_cb[] = {
	{
		.evt = NODE_EVT_OFFLINE,
		.sync = true,
		.min_version = DFS_2_0,
		.fn = hmdfs_stash_offline_prepare,
	},
	{
		.evt = NODE_EVT_OFFLINE,
		.sync = false,
		.min_version = DFS_2_0,
		.fn = hmdfs_stash_offline_do_stash,
	},
	/* Don't known peer version yet, so min_version is 0 */
	{
		.evt = NODE_EVT_ADD,
		.sync = true,
		.fn = hmdfs_stash_add_do_check,
	},
	{
		.evt = NODE_EVT_ONLINE,
		.sync = false,
		.min_version = DFS_2_0,
		.fn = hmdfs_stash_online_prepare,
	},
	{
		.evt = NODE_EVT_ONLINE,
		.sync = false,
		.min_version = DFS_2_0,
		.fn = hmdfs_stash_online_do_restore,
	},
	{
		.evt = NODE_EVT_DEL,
		.sync = true,
		.min_version = DFS_2_0,
		.fn = hmdfs_stash_del_do_cleanup,
	},
};

void __init hmdfs_stash_add_node_evt_cb(void)
{
	hmdfs_node_add_evt_cb(stash_cb, ARRAY_SIZE(stash_cb));
}

