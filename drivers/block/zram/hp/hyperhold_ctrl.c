/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author:	He Biao <hebiao6@huawei.com>
 *		Wang Cheng Ke <wangchengke2@huawei.com>
 *		Wang Fa <fa.wang@huawei.com>
 *
 * Create: 2020-4-16
 *
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/atomic.h>
#include <linux/memcontrol.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/file.h>
#include <linux/dcache.h>
#include <linux/types.h>
#include <linux/syscalls.h> /* sys_open, sys_read; sys_close */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>

#ifdef CONFIG_HISI_DEBUG_FS
#include <linux/hisi/rdr_hisi_platform.h>
#endif

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"

#define HYPERHOLD_WDT_EXPIRE_DEFAULT 3600
#define PRE_EOL_INFO_OVER_VAL 2
#define LIFE_TIME_EST_OVER_VAL 8

#define F2FS_IOCTL_MAGIC	0xf5
#define F2FS_IOC_HP_PREALLOC	_IOWR(F2FS_IOCTL_MAGIC, 31, __u32)
#define HP_FILE_SZ		(2UL * 1024UL * 1024UL * 1024UL)
#define HP_PRE_ENABLE		2
#define HP_FILE_DEL		3
#define HP_PARTITION_PATH	"/dev/block/by-name/hyperhold"
#define HP_FILE_PATH		"/data/vendor/8ac9cd7ecba7097c176cfde5f1dbc544"

struct zs_ext_para {
	struct hyperhold_page_pool *pool;
	size_t alloc_size;
	bool fast;
	bool nofail;
};

struct hyperhold_cfg {
	atomic_t enable;
	atomic_t reclaim_in_enable;
	atomic_t watchdog_protect;
	int log_level;
	struct timer_list wdt_timer;
	unsigned long wdt_expire_s;
	struct hyperhold_stat *stat;
	struct workqueue_struct *reclaim_wq;
	struct zram *zram;
	sector_t start_sector;
	bool hyperhold_revoke;
};

static struct hyperhold_cfg global_settings;

void *hyperhold_malloc(size_t size, bool fast, bool nofail)
{
	void *mem = NULL;

	if (likely(fast)) {
		mem = kzalloc(size, GFP_ATOMIC);
		if (likely(mem || !nofail))
			return mem;
	}

	mem = kzalloc(size, GFP_NOIO);

	return mem;
}

void hyperhold_free(const void *mem)
{
	kfree(mem);
}

static struct page *hyperhold_alloc_page_common(
					void *data, gfp_t gfp)
{
	struct page *page = NULL;
	struct zs_ext_para *ext_para =
		(struct zs_ext_para *)data;

	if (ext_para->pool) {
		spin_lock(&ext_para->pool->page_pool_lock);
		if (!list_empty(&ext_para->pool->page_pool_list)) {
			page = list_first_entry(
				&ext_para->pool->page_pool_list,
				struct page, lru);
			list_del(&page->lru);
		}
		spin_unlock(&ext_para->pool->page_pool_lock);
	}

	if (!page) {
		if (ext_para->fast) {
			page = alloc_page(GFP_ATOMIC);
			if (likely(page))
				goto out;
		}
		if (ext_para->nofail)
			page = alloc_page(GFP_NOIO);
		else
			page = alloc_page(gfp);
	}
out:
	return page;
}

static size_t hyperhold_zsmalloc_parse(void *data)
{
	struct zs_ext_para *ext_para = (struct zs_ext_para *)data;

	return ext_para->alloc_size;
}

unsigned long hyperhold_zsmalloc(struct zs_pool *zs_pool,
		size_t size, struct hyperhold_page_pool *pool)
{
	unsigned long ret;
	gfp_t gfp = __GFP_DIRECT_RECLAIM | __GFP_KSWAPD_RECLAIM |
		__GFP_NOWARN | __GFP_HIGHMEM |	__GFP_MOVABLE;
#ifdef CONFIG_ZS_MALLOC_EXT
	struct zs_ext_para ext_para;

	ext_para.alloc_size = size;
	ext_para.pool = pool;
	ext_para.fast = true;
	ext_para.nofail = true;
	ret = zs_malloc(zs_pool, (size_t)(&ext_para), gfp);
	if (!ret)
		hh_print(HHLOG_ERR,
			 "alloc handle failed, size = %lu, gfp = %d\n",
			 size, gfp);

	return ret;
#else
	return zs_malloc(zs_pool, size, gfp);
#endif
}

unsigned long zram_zsmalloc(struct zs_pool *zs_pool,
				size_t size, gfp_t gfp)
{
#ifdef CONFIG_ZS_MALLOC_EXT
	unsigned long ret;
	struct zs_ext_para ext_para;

	if (!is_ext_pool(zs_pool))
		return zs_malloc(zs_pool, size, gfp);

	ext_para.alloc_size = size;
	ext_para.pool = NULL;
	ext_para.fast = false;
	ext_para.nofail = false;
	ret = zs_malloc(zs_pool, (size_t)(&ext_para), gfp);
	if (!ret && (gfp | GFP_NOIO) == GFP_NOIO)
		hh_print(HHLOG_ERR,
			 "alloc handle failed, size = %lu, gfp = %d\n",
			 size, gfp);

	return ret;
#else
	return zs_malloc(zs_pool, size, gfp);
#endif
}

struct page *hyperhold_alloc_page(
		struct hyperhold_page_pool *pool, gfp_t gfp,
		bool fast, bool nofail)
{
	struct zs_ext_para ext_para;

	ext_para.pool = pool;
	ext_para.fast = fast;
	ext_para.nofail = nofail;

	return hyperhold_alloc_page_common((void *)&ext_para, gfp);
}

void hyperhold_page_recycle(struct page *page,
				struct hyperhold_page_pool *pool)
{
	if (pool) {
		spin_lock(&pool->page_pool_lock);
		list_add(&page->lru, &pool->page_pool_list);
		spin_unlock(&pool->page_pool_lock);
	} else {
		__free_page(page);
	}
}

int hyperhold_loglevel(void)
{
	return global_settings.log_level;
}

static void hyperhold_wdt_expire_set(unsigned long expire)
{
	global_settings.wdt_expire_s = expire;
}

static void hyperhold_wdt_set_enable(bool en)
{
	atomic_set(&global_settings.watchdog_protect, en ? 1 : 0);
}

static bool hyperhold_wdt_enable(void)
{
	return !!atomic_read(&global_settings.watchdog_protect);
}

bool hyperhold_reclaim_in_enable(void)
{
	return !!atomic_read(&global_settings.reclaim_in_enable);
}

static void hyperhold_set_reclaim_in_disable(void)
{
	atomic_set(&global_settings.reclaim_in_enable, false);
}

static void hyperhold_set_reclaim_in_enable(bool en)
{
	del_timer_sync(&global_settings.wdt_timer);
	atomic_set(&global_settings.reclaim_in_enable, en ? 1 : 0);
	if (en && hyperhold_wdt_enable())
		mod_timer(&global_settings.wdt_timer,
			jiffies + msecs_to_jiffies(
			global_settings.wdt_expire_s * MSEC_PER_SEC));
}

bool hyperhold_enable(void)
{
	return !!atomic_read(&global_settings.enable);
}

static void hyperhold_set_enable(bool en)
{
	hyperhold_set_reclaim_in_enable(en);

	if (!hyperhold_enable())
		atomic_set(&global_settings.enable, en ? 1 : 0);
}

static void hyperhold_wdt_timeout(unsigned long data)
{
	hh_print(HHLOG_ERR,
		"hyperhold wdt is triggered! Hyperhold is disabled!\n");
	hyperhold_set_reclaim_in_disable();
}

static void hyperhold_close_bdev(struct block_device *bdev,
					struct file *backing_dev)
{
	if (bdev)
		blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);

	if (backing_dev)
		filp_close(backing_dev, NULL);
}

static struct file *hyperhold_open_bdev(const char *file_name)
{
	struct file *backing_dev = NULL;

	backing_dev = filp_open(file_name, O_RDWR | O_LARGEFILE, 0);
	if (unlikely(IS_ERR(backing_dev))) {
		hh_print(HHLOG_ERR, "open the %s failed! eno = %lld\n",
					file_name, PTR_ERR(backing_dev));
		backing_dev = NULL;
		return NULL;
	}

	if (unlikely(!S_ISBLK(backing_dev->f_mapping->host->i_mode))) {
		hh_print(HHLOG_ERR, "%s isn't a blk device\n", file_name);
#ifdef CONFIG_HISI_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		hyperhold_close_bdev(NULL, backing_dev);
		return NULL;
	}

	return backing_dev;
}

sector_t hyperhold_get_start_sector(void)
{
	return global_settings.start_sector;
}

/*lint -e501*/
#ifdef CONFIG_HP_FILE
static void hyperhold_set_start_sector(sector_t start_sector)
{
	global_settings.start_sector  = start_sector;
}

static int hyperhold_get_bdev(const char *dir_name, struct block_device **bdev)
{
	int ret = 0;
	struct file *file = NULL;
	struct super_block *sb = NULL;

	file = filp_open(dir_name, O_RDONLY, S_IRWXU);
	if (IS_ERR_OR_NULL(file)) {
		hh_print(HHLOG_ERR, "open file %s error %ld", dir_name,
			 PTR_ERR(file));
		return -EINVAL;
	}

	sb = file->f_path.mnt->mnt_sb;
	if (!sb || sb->s_magic != F2FS_SUPER_MAGIC) {
		ret = -ENOENT;
		hh_print(HHLOG_ERR, "mount info error!\n");
		goto out;
	}

	hh_print(HHLOG_DEBUG, "path = %s fstype = %s\n", dir_name,
		 sb->s_type->name);
	*bdev = sb->s_bdev;
out:
	filp_close(file, NULL);
	return ret;
}

static int hyperhold_file_check(unsigned long long size)
{
	struct file *file = NULL;
	struct address_space *mapping = NULL;
	sector_t hp_addr;
	unsigned long start, end;

	file = filp_open(HP_FILE_PATH, O_CREAT | O_RDONLY | O_LARGEFILE,
			 S_IRUSR);
	if (unlikely(IS_ERR_OR_NULL(file))) {
		hh_print(HHLOG_ERR, "open hp file failed!\n");
		return -ENOENT;
	}

	mapping = file->f_mapping;
	if (unlikely(!mapping || !mapping->a_ops)) {
		hh_print(HHLOG_ERR, "No address mapping operations!\n");
		filp_close(file, NULL);
		return -ENOENT;
	}
	start = mapping->a_ops->bmap(mapping, 0);
	end = mapping->a_ops->bmap(mapping, (size >> PAGE_SHIFT) - 1);
	if (end - start != (size >> PAGE_SHIFT) - 1) {
		hh_print(HHLOG_ERR, "hyperhold file error!\n");
		filp_close(file, NULL);
		return -EINVAL;
	}
	hp_addr = start << BLOCK_SECTOR_SHIFT;
	hh_print(HHLOG_ERR, "file exits, get sector addr = %llu\n",
		 hp_addr); /* to change to DEBUG */
	hyperhold_set_start_sector(hp_addr);
	filp_close(file, NULL);

	return 0;
}

/*
 * This function has 2 steps to allocate the 2G hp file:
 * step 1: using filp_open to open(create) hp file
 * step 2: using ioctl to find and allocate hp file, and once we found that the
 * file is invalid, we re-preallocate the file.
 */
static int hyperhold_file_alloc(unsigned long long size)
{
	int ret;
	struct file *file = NULL;
	struct hp_file_cfg cfg;
	sector_t hp_addr;
	int flag = O_CREAT | O_RDONLY | O_LARGEFILE;

	/* distinguish the situation of allocating already by hp_pre_enable */
	if (hyperhold_get_start_sector()) {
		hh_print(HHLOG_ERR, "already got start sector!\n");
		return 0;
	}

	if (!sys_access(HP_FILE_PATH, 0))
		return hyperhold_file_check(size);

	file = filp_open(HP_FILE_PATH, flag, S_IRUSR);
	if (unlikely(IS_ERR_OR_NULL(file))) {
		hh_print(HHLOG_ERR, "open %s failed! ret = %ld\n", HP_FILE_PATH,
			 PTR_ERR(file));
		return -ENOENT;
	}
	cfg.pin = 1;
	cfg.len = size;
	cfg.addr = 0;

	/* set the hp file pinned and preallocate for it */
	ret = file->f_op->unlocked_ioctl(file, F2FS_IOC_HP_PREALLOC,
					 (unsigned long)&cfg);

	filp_close(file, NULL);

	if (unlikely(ret)) {
		sys_unlink(HP_FILE_PATH);
		return ret;
	} else {
		hp_addr = cfg.addr << BLOCK_SECTOR_SHIFT;
		hh_print(HHLOG_DEBUG, "get sector addr = %llu\n", hp_addr);
		hyperhold_set_start_sector(hp_addr);
	}
	return ret;
}

static int hyperhold_disk_alloc(struct zram *zram)
{
	int ret;
	struct block_device *bdev = NULL;
	mm_segment_t oldfs;

	ret = hyperhold_get_bdev("/data/vendor", &bdev);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold_disk_alloc fail\n");
		goto out;
	}

	oldfs = get_fs();
	set_fs(get_ds());

	ret = hyperhold_file_alloc(HP_FILE_SZ);
	if (likely(!ret)) {
		down_write(&zram->init_lock);
		zram->bdev = bdev;
		zram->backing_dev = NULL;
		zram->nr_pages = HP_FILE_SZ >> PAGE_SHIFT;
		up_write(&zram->init_lock);
	}

	set_fs(oldfs);
out:
	return ret;
}

static void hyperhold_file_unlink(void)
{
	mm_segment_t old_fs;
	int ret;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_unlink(HP_FILE_PATH);
	set_fs(old_fs);

	if (ret)
		hh_print(HHLOG_ERR, "hp file unlink fail! ret = %d", ret);
}

static bool hyperhold_pre_enable(void)
{
	bool result = false;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (!sys_access(HP_PARTITION_PATH, 0)) {
		hh_print(HHLOG_ERR, "hyperhold partiton exist! \r\n");
		result = true;
		goto out;
	}

	if (!hyperhold_file_alloc(HP_FILE_SZ)) {
		hh_print(HHLOG_ERR, "hyperhold file check succ! \r\n");
		result = true;
		goto out;
	}
	hh_print(HHLOG_ERR, "hyperhold pre enable failed! \r\n");
out:
	set_fs(old_fs);
	return result;
}
#endif

static int hyperhold_bind(struct zram *zram, const char *file_name)
{
	struct file *backing_dev = NULL;
	struct inode *inode = NULL;
	unsigned long nr_pages;
	struct block_device *bdev = NULL;
	int err;

	if (hyperhold_get_start_sector()) {
		hh_print(HHLOG_ERR, "already got start sector!\n");
		return 0;
	}

	backing_dev = hyperhold_open_bdev(file_name);
	if (unlikely(!backing_dev))
		return -EINVAL;

	inode = backing_dev->f_mapping->host;

	bdev = bdgrab(I_BDEV(inode));
	err = blkdev_get(bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL, zram);
	if (unlikely(err < 0)) {
		hh_print(HHLOG_ERR, "%s blkdev_get failed! eno = %d\n",
					file_name, err);
#ifdef CONFIG_HISI_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		bdev = NULL;
		goto out;
	}

	nr_pages = (unsigned long)i_size_read(inode) >> PAGE_SHIFT;

	err = set_blocksize(bdev, PAGE_SIZE);
	if (unlikely(err)) {
		hh_print(HHLOG_ERR,
			"%s set blocksize failed! eno = %d\n", file_name, err);
#ifdef CONFIG_HISI_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		goto out;
	}

	down_write(&zram->init_lock);
	zram->bdev = bdev;
	zram->backing_dev = backing_dev;
	zram->nr_pages = nr_pages;
	up_write(&zram->init_lock);

	return 0;
out:
	hyperhold_close_bdev(bdev, backing_dev);

	return err;
}

static void hyperhold_stat_init(struct hyperhold_stat *stat)
{
	int i;

	atomic64_set(&stat->reclaimin_cnt, 0);
	atomic64_set(&stat->reclaimin_bytes, 0);
	atomic64_set(&stat->reclaimin_pages, 0);
	atomic64_set(&stat->reclaimin_infight, 0);
	atomic64_set(&stat->batchout_cnt, 0);
	atomic64_set(&stat->batchout_bytes, 0);
	atomic64_set(&stat->batchout_pages, 0);
	atomic64_set(&stat->batchout_inflight, 0);
	atomic64_set(&stat->fault_cnt, 0);
	atomic64_set(&stat->hyperhold_fault_cnt, 0);
	atomic64_set(&stat->reout_pages, 0);
	atomic64_set(&stat->reout_bytes, 0);
	atomic64_set(&stat->zram_stored_pages, 0);
	atomic64_set(&stat->zram_stored_size, 0);
	atomic64_set(&stat->stored_pages, 0);
	atomic64_set(&stat->stored_size, 0);
	atomic64_set(&stat->notify_free, 0);
	atomic64_set(&stat->frag_cnt, 0);
	atomic64_set(&stat->mcg_cnt, 0);
	atomic64_set(&stat->ext_cnt, 0);
	atomic64_set(&stat->miss_free, 0);
	atomic64_set(&stat->mcgid_clear, 0);

	for (i = 0; i < HYPERHOLD_SCENARIO_BUTT; ++i) {
		atomic64_set(&stat->io_fail_cnt[i], 0);
		atomic64_set(&stat->alloc_fail_cnt[i], 0);
		atomic64_set(&stat->lat[i].total_lat, 0);
		atomic64_set(&stat->lat[i].max_lat, 0);
	}

	stat->record.num = 0;
	spin_lock_init(&stat->record.lock);
}

static bool hyperhold_global_setting_init(struct zram *zram)
{
	if (unlikely(global_settings.stat))
		return false;

	global_settings.log_level = HHLOG_ERR;
	global_settings.zram = zram;
	hyperhold_wdt_set_enable(true);
	hyperhold_set_enable(false);
	init_timer(&global_settings.wdt_timer);
	global_settings.wdt_timer.function = hyperhold_wdt_timeout;
	hyperhold_wdt_expire_set(HYPERHOLD_WDT_EXPIRE_DEFAULT);
	global_settings.stat = hyperhold_malloc(
				sizeof(struct hyperhold_stat), false, true);
	if (unlikely(!global_settings.stat)) {
		hh_print(HHLOG_ERR, "global stat allocation failed!\n");

		return false;
	}

	hyperhold_stat_init(global_settings.stat);
	global_settings.reclaim_wq = alloc_workqueue("hyperhold_reclaim",
							WQ_CPU_INTENSIVE, 0);
	if (unlikely(!global_settings.reclaim_wq)) {
		hh_print(HHLOG_ERR,
				"reclaim workqueue allocation failed!\n");
		hyperhold_free(global_settings.stat);
		global_settings.stat = NULL;

		return false;
	}

	return true;
}

static void hyperhold_global_setting_deinit(void)
{
	destroy_workqueue(global_settings.reclaim_wq);
	hyperhold_free(global_settings.stat);
	global_settings.stat = NULL;
	global_settings.zram = NULL;
}

struct workqueue_struct *hyperhold_get_reclaim_workqueue(void)
{
	return global_settings.reclaim_wq;
}

/*
 * This interface will be called when user set the ZRAM size.
 * Hyperhold init here.
 */
void hyperhold_init(struct zram *zram)
{
	int ret;
	mm_segment_t old_fs;

	if (global_settings.hyperhold_revoke)
		return;

	if (!hyperhold_global_setting_init(zram))
		return;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_access(HP_PARTITION_PATH, 0);
	set_fs(old_fs);

	if (!ret)
		ret = hyperhold_bind(zram, HP_PARTITION_PATH);
#ifdef CONFIG_HP_FILE
	else
		ret = hyperhold_disk_alloc(global_settings.zram);
#endif

	if (!ret) {
		zs_pool_enable_ext(zram->mem_pool, true,
					hyperhold_zsmalloc_parse);
		zs_pool_ext_malloc_register(zram->mem_pool,
					hyperhold_alloc_page_common);
		return;
	}
	hh_print(HHLOG_ERR,
			"bind storage device failed! %d\n", ret);
	hyperhold_global_setting_deinit();
}

static int hyperhold_set_enable_init(bool en)
{
	int ret;

	if (hyperhold_enable() || !en)
		return 0;

	if (!global_settings.stat) {
		hh_print(HHLOG_ERR, "global_settings.stat is null!\n");

		return -EINVAL;
	}

	ret = hyperhold_manager_init(global_settings.zram);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "init manager failed! %d\n", ret);

		return -EINVAL;
	}

	ret = hyperhold_schedule_init();
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "init schedule failed! %d\n", ret);
		hyperhold_manager_deinit(global_settings.zram);

		return -EINVAL;
	}

	return 0;
}

struct hyperhold_stat *hyperhold_get_stat_obj(void)
{
	return global_settings.stat;
}

static int hyperhold_health_check(void)
{
#ifdef CONFIG_MAS_BLK
	int ret;
	u8 pre_eol_info = PRE_EOL_INFO_OVER_VAL;
	u8 life_time_est_a = LIFE_TIME_EST_OVER_VAL;
	u8 life_time_est_b = LIFE_TIME_EST_OVER_VAL;

	if (unlikely(!global_settings.zram)) {
		hh_print(HHLOG_ERR, "zram is null!\n");

		return -EFAULT;
	}

	ret = blk_dev_health_query(global_settings.zram->bdev, &pre_eol_info,
		&life_time_est_a, &life_time_est_b);
	if (ret) {
		hh_print(HHLOG_ERR, "query health err %d!\n", ret);

		return ret;
	}

	if ((pre_eol_info >= PRE_EOL_INFO_OVER_VAL) ||
		(life_time_est_a >= LIFE_TIME_EST_OVER_VAL) ||
		(life_time_est_b >= LIFE_TIME_EST_OVER_VAL)) {
		hh_print(HHLOG_ERR, "over life time uesd %u %u %u\n",
			pre_eol_info, life_time_est_a, life_time_est_b);

		return -EPERM;
	}

	hh_print(HHLOG_DEBUG, "life time uesd %u %u %u\n",
			pre_eol_info, life_time_est_a, life_time_est_b);
#endif

	return 0;
}

ssize_t hyperhold_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "val is error!\n");

		return -EINVAL;
	}

#ifdef CONFIG_HP_FILE
	if (val == HP_PRE_ENABLE) {
		if (hyperhold_pre_enable()) {
			global_settings.hyperhold_revoke = false;
			return len;
		} else {
			return -EINVAL;
		}
	}
	if (val == HP_FILE_DEL) {
		hh_print(HHLOG_ERR, "deleting hp file!\n");
		hyperhold_file_unlink();
		global_settings.hyperhold_revoke = true;
		return len;
	}
#endif
	/* hyperhold must be close when over 70% life time used */
	if (hyperhold_health_check())
		val = false;

	if (hyperhold_set_enable_init(!!val))
		return -EINVAL;

	hyperhold_set_enable(!!val);

	return len;
}

ssize_t hyperhold_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hyperhold %s reclaim_in %s\n",
		hyperhold_enable() ? "enable" : "disable",
		hyperhold_reclaim_in_enable() ? "enable" : "disable");
}

#ifdef CONFIG_HISI_DEBUG_FS
static void hyperhold_loglevel_set(int level)
{
	global_settings.log_level = level;
}

static bool ft_get_val(const char *buf, const char *token, unsigned long *val)
{
	int ret = -EINVAL;
	char *str = strstr(buf, token);

	if (str)
		ret = kstrtoul(str + strlen(token), 0, val);

	return ret == 0;
}

ssize_t hyperhold_ft_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len)
{
	char *type_buf = NULL;
	unsigned long val;

	if (!hyperhold_enable())
		return len;

	type_buf = strstrip((char *)buf);
	if (ft_get_val(type_buf, "idle", &val)) {
		memcg_idle_count(global_settings.zram);
		goto out;
	}
	if (ft_get_val(type_buf, "loglevel=", &val)) {
		hyperhold_loglevel_set((int)val);
		goto out;
	}

	if (ft_get_val(type_buf, "watchdog=", &val)) {
		if (val)
			hyperhold_wdt_expire_set(val);
		hyperhold_wdt_set_enable(!!val);
	}
out:
	return len;
}

ssize_t hyperhold_ft_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t size = 0;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	size += scnprintf(buf + size, PAGE_SIZE,
			"Hyperhold enable: %s\n",
			hyperhold_enable() ? "Yes" : "No");
	size += scnprintf(buf + size, PAGE_SIZE - size,
			"Hyperhold watchdog enable: %s\n",
			hyperhold_wdt_enable() ? "Yes" : "No");
	size += scnprintf(buf + size, PAGE_SIZE - size,
				"Hyperhold watchdog expire(s): %lu\n",
				global_settings.wdt_expire_s);
	size += scnprintf(buf + size, PAGE_SIZE - size,
				"Hyperhold log level: %d\n",
				hyperhold_loglevel());
	if (stat)
		size += scnprintf(buf + size, PAGE_SIZE - size,
				"Hyperhold mcgid clear: %ld\n",
				atomic64_read(&stat->mcgid_clear));

	return size;
}

#define TEST_BUF_LEN 5

static int hyperhold_write_test_thread(void *data)
{
	int fd = -1;
	int ret;
	ssize_t cnt;
	mm_segment_t old_fs;
	char data_buf[] = "hhhhh";

	hh_print(HHLOG_ERR, "%s started\n", __func__);
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	/* 2. open file */
	fd = (int)sys_open(HP_FILE_PATH, O_WRONLY, S_IRUSR | S_IWUSR | S_IWGRP);
	if (fd < 0) {
		hh_print(HHLOG_ERR, "%s: open hyperhold_file failed, fd = %d\n",
			 __func__, fd);
		ret = fd;
		goto out;
	}

	/* 3. write data */
	ret = (int)sys_lseek((unsigned int)fd, 0, SEEK_SET);
	if (ret < 0) {
		hh_print(HHLOG_ERR, "%s: sys_lseek fail, ret = %d\n", __func__,
			 ret);
		goto out;
	}

	cnt = sys_write((unsigned int)fd, (char __user *)data_buf,
			TEST_BUF_LEN);
	if (cnt < TEST_BUF_LEN) {
		hh_print(HHLOG_ERR,
			 "%s: write hyperhold_file failed, %ld completed\n",
			 __func__, cnt);
		ret = -EINVAL;
		goto out;
	}
	ret = sys_fsync((unsigned int)fd);
	if (ret < 0) {
		hh_print(HHLOG_ERR, "%s: sys_fsync failed, ret = %d\n",
			 __func__, ret);
		goto out;
	}
	ret = 0;
	hh_print(HHLOG_ERR, "hyperhold_write_test success\n");
out:
	if (fd >= 0)
		sys_close((unsigned int)fd);
	set_fs(old_fs);
	return ret;
}

static int hyperhold_unlink_test_thread(void *data)
{
	int ret;

	ret = sys_unlink(HP_FILE_PATH);
	if (unlikely(ret))
		hh_print(HHLOG_ERR, "unlink hp file failed!, ret = %d\n", ret);
	else
		hh_print(HHLOG_ERR, "hyperhold_unlink_test success\n");
	return ret;
}

static struct task_struct *_task;
static struct task_struct *_task_1;

void hyperhold_start_selinux_test(void)
{
	_task = kthread_run(hyperhold_write_test_thread, NULL,
			    "hyperhold_write_test_thread");
	if (!IS_ERR(_task))
		hh_print(HHLOG_ERR, "kthread_create done\n");
	else
		hh_print(HHLOG_ERR, "kthread_create error\n");
	_task_1 = kthread_run(hyperhold_unlink_test_thread, NULL,
			      "hyperhold_unlink_test_thread");
	if (!IS_ERR(_task_1))
		hh_print(HHLOG_ERR, "kthread_create done\n");
	else
		hh_print(HHLOG_ERR, "kthread_create error\n");
}

void hyperhold_stop_selinux_test(void)
{
	int ret;

	if (_task) {
		ret = kthread_stop(_task);
		hh_print(HHLOG_ERR, "kthread_stop ret = %d\n", ret);
		_task = NULL;
	}
	if (_task_1) {
		ret = kthread_stop(_task_1);
		hh_print(HHLOG_ERR, "kthread_stop ret = %d\n", ret);
		_task_1 = NULL;
	}
}
#endif
/*lint +e501*/

static int __init hyperhold_module_init(void)
{
	global_settings.start_sector = 0;
	return 0;
}
module_init(hyperhold_module_init);
