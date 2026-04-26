/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: prefetch the meta data of userdata partion to speed up the
 *              process of resize/fsck/mount.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2019-10-11
 */
#include <linux/fsck_boost.h>

#include <linux/mm.h>
#include <linux/magic.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/f2fs_fs.h>
#include <linux/completion.h>
#include <linux/hisi/hisi_cmdline_parse.h>

#define FSCK_BOOST_USERDATA_SIZE_THRESHOLD 32
#define FSCK_BOOST_SEG_NUM1 123
#define FSCK_BOOST_SEG_NUM2 57

static struct fsck_boost_info g_fbi;
static DEFINE_MUTEX(g_fbi_lock);
static DECLARE_COMPLETION(g_fb_complete);

void stop_fsck_boost(void)
{
	mutex_lock(&g_fbi_lock);
	g_fbi.done = FSCK_BOOST_STOPPED;
	mutex_unlock(&g_fbi_lock);
}

static int is_fsck_boost_stopped(void)
{
	int result;

	mutex_lock(&g_fbi_lock);
	if (g_fbi.done == FSCK_BOOST_STOPPED)
		result = 1;
	else
		result = 0;
	mutex_unlock(&g_fbi_lock);

	return result;
}

static int get_fbi(struct gendisk **disk, int *partno)
{
	mutex_lock(&g_fbi_lock);
	if (g_fbi.done && g_fbi.disk) {
		*disk   = g_fbi.disk;
		*partno = g_fbi.partno;
		mutex_unlock(&g_fbi_lock);
		return 0;
	}
	mutex_unlock(&g_fbi_lock);

	pr_err_ratelimited("fsck_boost is not running\n");

	return -1;
}

struct gendisk *is_fsck_boost_done(const struct block_device *bdev)
{
	int err;
	int partno;
	struct gendisk *disk = NULL;

	err = get_fbi(&disk, &partno);
	if (err)
		return NULL;

	if (strcmp(disk->disk_name, bdev->bd_disk->disk_name)
			|| partno != bdev->bd_part->partno) {
		pr_err("%s: %s%d not match %s%d\n", __func__,
				disk->disk_name, partno,
				bdev->bd_disk->disk_name,
				bdev->bd_part->partno);
		return NULL;
	}

	return disk;
}

static int read_pagecaches(void *info)
{
	int i;
	int err;
	int partno;

	struct gendisk *disk = NULL;
	struct block_device *bdev = NULL;
	struct address_space *mapping = NULL;

	err = get_fbi(&disk, &partno);
	if (err)
		return -1;

	bdev = bdget_disk(disk, partno);
	if (!bdev) {
		pr_err("%s bdget fail!\n", __func__);
		return -1;
	}

	err = blkdev_get(bdev, FMODE_READ, NULL);
	if (err) {
		pr_err("%s blkdev_get fail!\n", __func__);
		return err;
	}

	mapping = bdev->bd_inode->i_mapping;

	pr_info("%s: begin read meta, total %d segments\n",
			__func__, g_fbi.seg_num);

	for (i = 0; i < g_fbi.seg_num; i++) {
		if (is_fsck_boost_stopped())
			break;

		err = force_page_cache_readahead_abs(mapping,
				PREFETCH_SEGMENT_SIZE * i,
				PREFETCH_SEGMENT_SIZE);
		if (err < 0)
			pr_err("%s: readahead err is %d\n", __func__, err);
	}
	pr_info("%s: end read meta %d segments\n", __func__, i);

	wait_for_completion(&g_fb_complete);

	mutex_lock(&g_fbi_lock);
	if (g_fbi.disk) {
		invalidate_mapping_pages(mapping, 0, -1);
		blkdev_put(bdev, FMODE_READ);
		g_fbi.disk = NULL;
		pr_info("fsck_boost end\n");
	}
	mutex_unlock(&g_fbi_lock);

	return 0;
}

static void run_fsck_boost(void)
{
	struct task_struct *thread = NULL;
	struct cpumask slow_cpus;

	thread = kthread_create(read_pagecaches, NULL, "fsck_boost");
	if (IS_ERR_OR_NULL(thread)) {
		pr_err("%s: create thread fail!\n", __func__);
		return;
	}

#ifdef CONFIG_HISI_EAS_SCHED
	hisi_get_slow_cpus(&slow_cpus);
	kthread_bind_mask(thread, &slow_cpus);
	pr_info("%s : bind to slow cpu\n", __func__);
#endif

	wake_up_process(thread);
}

void fsck_boost_start(struct gendisk *disk,
	const struct partition_meta_info *info,
	int partno, sector_t len)
{
	if (!info || strcmp(info->volname, "userdata"))
		return;

	if (get_boot_into_recovery_flag()) {
		pr_err_once("%s : booting into recovery, no need to run\n",
			__func__);
		return;
	}

	mutex_lock(&g_fbi_lock);
	if (g_fbi.done) {
		pr_err("%s: fsck boost done is %d!\n",
				__func__, g_fbi.done);
		mutex_unlock(&g_fbi_lock);
		return;
	}
	g_fbi.partno = partno;
	g_fbi.disk   = disk;
	g_fbi.done   = FSCK_BOOST_RUNNING;

	/*
	 * If userdata partition's size is less than 32 GB, its meta area
	 * has 57 segments. If userdata partition's size is more than 32 GB,
	 * its meta area has 123 segments.
	 */
	if (len / SECTORS_PER_GB > FSCK_BOOST_USERDATA_SIZE_THRESHOLD)
		g_fbi.seg_num = FSCK_BOOST_SEG_NUM1;
	else
		g_fbi.seg_num = FSCK_BOOST_SEG_NUM2;

	mutex_unlock(&g_fbi_lock);

	run_fsck_boost();
}

void fsck_boost_end(const struct block_device *bdev)
{
	if (!is_fsck_boost_done(bdev))
		return;

	complete(&g_fb_complete);
}
