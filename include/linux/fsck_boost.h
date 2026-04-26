/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: prefetch the meta data of userdata partion to speed up the
 *              process of resize/fsck/mount.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2019-10-11
 */
#ifndef KERNEL_FSCK_BOOST_H
#define KERNEL_FSCK_BOOST_H

#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>

#define SECTOR_LOG_SIZE  9
#define SECTORS_PER_GB (1 << (30 - SECTOR_LOG_SIZE))

#define PREFETCH_SEGMENT_SIZE (2 * 1024 * 1024 / PAGE_SIZE)

#define FSCK_BOOST_RUNNING 1
#define FSCK_BOOST_STOPPED 2

struct fsck_boost_info {
	struct gendisk *disk;
	int partno;
	int done;
	int seg_num;
};

void fsck_boost_start(struct gendisk *disk,
	const struct partition_meta_info *info,
	int partno, sector_t len);

void fsck_boost_end(const struct block_device *bdev);

struct gendisk *is_fsck_boost_done(const struct block_device *bdev);

void stop_fsck_boost(void);
#endif
